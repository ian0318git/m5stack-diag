/* $Id: nim_dm.c,v 1.4 2018/05/18 09:25:02 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/dreamliner/nim_dm.c,v $
 *------------------------------------------------------------------
 *
 * nim_dm.c - This file supports Dreamliner.
 *
 * Christine Wen -- Jan. 2014
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*------------------------------------------------------------------
 * DM kernel driver for all dm instances
 *
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/sysctl.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/kdev_t.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <asm/atomic.h>
#include "nim_dm_ioctl.h"
#include "nim_dm.h"

//#define DEBUG 1

MODULE_AUTHOR("Cisco Systems");
MODULE_DESCRIPTION("DM PCIE Driver");
MODULE_LICENSE("GPL");

unsigned int dm_drv_log_level = DM_DRV_LOG_LEVEL_ALL;
module_param(dm_drv_log_level, uint, S_IRUGO|S_IWUSR|S_IWGRP);

char nim_dm_driver_name[] = "nim_dm";

/*
 * global dm device instance table for all slot/bay
 */
static dm_dev_t *g_dm_devs[DM_MAX_SLOT][DM_MAX_BAY];

/*
 * global pcie port mapping table for all slot/bay
 */
static pcie_mapping_t pcie_mapping_table[DM_MAX_SLOT][DM_MAX_BAY];

/*
 * flag for pcie port mapping set and pcie driver loaded
 */
static int pcie_mapping_driver_initialized;

/*
 * major device number of main and sub dev
 */
static int cdev_major;

/*
 * tracking users of the main cdev, only one user is allowed
 */
static atomic_t main_cdev_user = ATOMIC_INIT(0);

/*
 * char device of the main cdev
 */
static struct cdev main_cdev;

/*
 *  dm class for sys file
 */
static struct class *dm_class;

/*
 * device for main cdev sys attribute
 */
static struct device *dm_main_sysdev;

/*
 * the main cdev structure, which is used for global configuration
 * for now, only pcie port mapping is defined
 */
static struct file_operations dm_main_cdev_fops = {
    .owner = THIS_MODULE,
    .open = dm_main_open,
    .release = dm_main_release,
#if defined(HAVE_COMPAT_IOCTL)
    .compat_ioctl  = dm_main_ioctl_wrapper,
#endif
#if defined(HAVE_UNLOCKED_IOCTL)
    .unlocked_ioctl  = dm_main_ioctl_wrapper,
#else
    .ioctl = dm_main_ioctl,
#endif
};

/*
 * the sub cdev structure, which is for slot/bay specific dm instance
 * configuration and operations.
 * each pp instance is bound to a sub cdev through dm_dev_t
 */
static struct file_operations dm_sub_cdev_fops = {
    .owner = THIS_MODULE,
    .mmap   = dm_sub_mmap,
    .open = dm_sub_open,
    .release = dm_sub_release,
    .poll = dm_sub_poll,
#if defined(HAVE_COMPAT_IOCTL)
    .compat_ioctl  = dm_sub_ioctl_wrapper,
#endif
#if defined(HAVE_UNLOCKED_IOCTL)
    .unlocked_ioctl  = dm_sub_ioctl_wrapper,
#else
    .ioctl = dm_sub_ioctl,
#endif

};

/*
 * pci id table to identify supported hardware chip
 * one entry for one chip model
 *
 */
static struct pci_device_id dm_pci_tbl[] = {
    /* for NIM-ES2-8 and NIM-ES2-8-P */
    { PCI_DEVICE(DM_PCIE_VENDOR_ID_MARVELL, DM_PCIE_DEVICE_ID_98DX3133) },

    /* for NIM-ES2-4 */
    { PCI_DEVICE(DM_PCIE_VENDOR_ID_MARVELL, DM_PCIE_DEVICE_ID_98DX3033B) },
    { 0 },
};

/*
 * pci driver to manage all slot/bay pp devices
 * no need to support power management
 */
static struct pci_driver pcie_dm_driver = {
    .name = nim_dm_driver_name,
    .id_table = dm_pci_tbl,
    .probe = dm_probe,
    .remove = dm_remove,
};

/*
 * sys fs attributs of main cdev
 */
/* newly kernel cannot assign persmission 666, using 0444 as default */
static DEVICE_ATTR(pcie_mapping, 0444, 
                   show_main_cdev_attr, store_main_cdev_attr);

/*
 * sys fs attributes of sub cdev
 */
static DEVICE_ATTR(dm_dev_info, S_IRUGO,
                   show_sub_cdev_attr, NULL);




static int
dm_main_open(struct inode *inode, struct file *filp)
{
    /*
     * only one user of the main cdev is allowed
     */
    if(atomic_inc_return(&main_cdev_user) > 1) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to open a busy dm main cdev.\n");

        atomic_dec(&main_cdev_user);
        return -EBUSY;
    }
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "The dm main cdev is opened.\n");
#endif
    return 0;
}

static int
dm_main_release(struct inode *inode, struct file *filp)
{
    /*
     * the main cdev is released when the owner
     * dm instance is plugged out. So it is avaible for
     * other instance.
     */
    atomic_dec(&main_cdev_user);
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "One user releases the dm main cdev.\n");
#endif
    return 0;
}

static int
dm_main_set_pcie_mapping(pcie_mapping_array_t *map_array)
{
    int i = 0, cnt = 0;
    pcie_switch_port_mapping_t *map_ents;
    uint32_t slot, bay;

    map_ents = (pcie_switch_port_mapping_t *)map_array->ents;

    for (i=0; i < map_array->num; i++) {
        slot = map_ents[i].slot;
        bay = map_ents[i].bay;

        if(slot >= DM_MAX_SLOT || bay >= DM_MAX_BAY) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The pcie mapping entry %d has wrong slot/bay. "
                       "Slot = %u, Bay = %u, Sec bus = %u, Sub bus = %u\n",
                       i, slot, bay,
                       map_ents[i].sec_bus, map_ents[i].sub_bus);

            continue;
        }
        /*
         * a mapping entry for a slot/bay instance is configured only once
         */
        if(pcie_mapping_table[slot][bay].flag & PCIE_MAPPING_VALID_BIT) {
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The pcie mapping entry %d has been mapped "
                       "for slot/bay(%u/%u), old pcie port(%u, %u), "
                       "new pcie port(%u, %u)\n",
                       i, slot, bay,
                       pcie_mapping_table[slot][bay].sec_bus,
                       pcie_mapping_table[slot][bay].sub_bus,
                       map_ents[i].sec_bus, map_ents[i].sub_bus);
#endif
            continue;
        }

        pcie_mapping_table[slot][bay].sec_bus = map_ents[i].sec_bus;
        pcie_mapping_table[slot][bay].sub_bus = map_ents[i].sub_bus;
        pcie_mapping_table[slot][bay].flag |= PCIE_MAPPING_VALID_BIT;

        cnt++;
#ifdef DEBUG
        dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                   "The mapping of PCIe port(%u, %u) to slot/bay(%u/%u) "
                   "is set up.\n",
                   map_ents[i].sec_bus, map_ents[i].sub_bus, slot, bay);
#endif
    }
    return cnt;
}

static int
dm_main_ioctl(struct inode *inode, struct file *filp,
              unsigned int cmd, unsigned long arg)
{
    int ret = 0, cnt = 0;
    pcie_mapping_array_t map_array;
    pcie_switch_port_mapping_t *map_ents;

    if (_IOC_TYPE(cmd) != DM_MAIN_IOC_MAGIC) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "The dm main dev receives wrong cmd(%u,%c).\n",
                   cmd, _IOC_TYPE(cmd));

        return -ENOTTY;
    }

    switch(cmd) {
        case DM_MAIN_SET_PCIE_MAPPING:
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "Dm main cdev receives a DM_MAIN_SET_PCIE_MAPPING "
                       "io cmd.\n");
#endif
            /*
             * for now, all mapping entries are configured at one time.
             */
            if(pcie_mapping_driver_initialized){
                dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                           "pcie mapping array has been set.\n");

                return ret;
            }

            ret = copy_from_user(&map_array, (void*)arg,
                                 sizeof(pcie_mapping_array_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "Fail to copy pcie mapping array from user space. "
                           "Error code = %d\n", ret);

                return ret;
            }

            map_ents = kmalloc(sizeof(pcie_switch_port_mapping_t) *
                               map_array.num, GFP_KERNEL);
            if(map_ents == NULL) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "Fail to alloc memory for pcie mapping entries. "
                           "Error code = %d\n", ret);
                return ret;
            }

            ret = copy_from_user(map_ents, (void *)map_array.ents,
                                 sizeof(pcie_switch_port_mapping_t) *
                                 map_array.num);
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "Fail to copy pcie mapping entries from user space. "
                           "Error code = %d\n", ret);

                return ret;
            }

            map_array.ents = (unsigned long long)map_ents;
            cnt = dm_main_set_pcie_mapping(&map_array);

            kfree(map_ents);

            /*
             * To avoid multiple threads synchronization issues. PCIe  driver
             * is registered to kernel PCI core after invalid PCIe port mappings
             * are set since the driver need mapping information to bind each
             * detected PCIe device to a specific slot/bay.
             */
            if(cnt > 0 && !pcie_mapping_driver_initialized) {
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO, "\n*** DBG : pci_register_driver\n");
#endif
                ret = pci_register_driver(&pcie_dm_driver);
                if (ret) {
                    dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                               "Fail to load the pcie dm driver. "
                               "Error code = %d\n", ret);
                    return ret;
                }
                pcie_mapping_driver_initialized = 1;
            }

            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "Dm pcie driver is registered to kernel.\n");

            break;

        default:
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The dm main cdev receives unknown io cmd(%u).\n",
                       cmd);
            return -ENOTTY;
    }
    return ret;
}

#if defined(HAVE_COMPAT_IOCTL)
static long dm_main_ioctl_wrapper (struct file *filp, unsigned int cmd,
                                   unsigned long arg)
{
    return ((long)dm_main_ioctl(NULL, filp, cmd, arg));
}
#endif

static ssize_t show_main_cdev_attr(struct device *dev,
                                   struct device_attribute *attr,
                                   char *buf)
{
    int i = 0, j=0;
    int len = 0;

    len += sprintf(buf+len, "PCIe port mapping:\n"
                       "SLOT  BAY   SEC   SUB   FLAG\n");

    for(i = 0; i <  DM_MAX_SLOT; i++) {
        for(j = 0; j < DM_MAX_BAY; j++) {
            len += sprintf(buf+len, "%-4u  %-4u  %-4u  %-4u  %-4llu\n",
                           i, j,
                           pcie_mapping_table[i][j].sec_bus,
                           pcie_mapping_table[i][j].sub_bus,
                           pcie_mapping_table[i][j].flag);
        }
    }

    return len;
}

static ssize_t store_main_cdev_attr(struct device *dev,
                                    struct device_attribute *attr,
                                    const char *buf,
                                    size_t count)
{
    unsigned int slot, bay;
    unsigned int sec_bus, sub_bus;
    unsigned long long flag;

    sscanf(buf, "%u %u %u %u %llu",
           &slot, &bay, &sec_bus, &sub_bus, &flag);

    if((slot < DM_MAX_SLOT) && (bay < DM_MAX_BAY)) {
        pcie_mapping_table[slot][bay].sec_bus = sec_bus;
        pcie_mapping_table[slot][bay].sub_bus = sub_bus;
        pcie_mapping_table[slot][bay].flag = flag;
    }

    return count;
}

static int
dm_create_main_cdev(void)
{
    int ret = 0;
    dev_t devno;

    /*
     * dynamically allocate major and minor device numbers
     * for main and all sub cdev
     */
    ret = alloc_chrdev_region(&devno, 0, DM_MAX_SLOT*DM_MAX_BAY+1,
                              nim_dm_driver_name);
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to alloc device number. "
                   "Error code = %d\n", ret);
        goto _exit_no_chrdev_region;
    }

    cdev_major = MAJOR(devno);

    cdev_init(&main_cdev, &dm_main_cdev_fops);
    main_cdev.owner = THIS_MODULE;

    /*
     * the minor device number of main cdev is DM_MAX_SLOT*DM_MAX_BAY
     */
    ret = cdev_add(&main_cdev, MKDEV(cdev_major, DM_MAX_SLOT*DM_MAX_BAY), 1);
    if(ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the main cdev: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, DM_MAX_SLOT*DM_MAX_BAY, ret);

        goto _exit_chrdev_region_alloced;
    }

    dm_class = class_create(THIS_MODULE, nim_dm_driver_name);
    if (IS_ERR(dm_class)) {
        ret = PTR_ERR(dm_class);
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the main cdev class: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, DM_MAX_SLOT*DM_MAX_BAY, ret);

        goto _exit_cdev_added;
    }

    dm_main_sysdev = device_create(dm_class,
                                   NULL,
                                   MKDEV(cdev_major, DM_MAX_SLOT*DM_MAX_BAY),
                                   NULL,
                                   nim_dm_driver_name);

    if (IS_ERR(dm_main_sysdev)) {
        ret = PTR_ERR(dm_main_sysdev);
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the main cdev sys device: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, DM_MAX_SLOT*DM_MAX_BAY, ret);

        goto _exit_class_created;
    }

    ret = device_create_file(dm_main_sysdev, &dev_attr_pcie_mapping);
    if(ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the main cdev sys attribute: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, DM_MAX_SLOT*DM_MAX_BAY, ret);

        goto _exit_device_created;
    }
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "The main cdev is created: "
               "cdev_major=%u, cdev_minor=%u\n",
               cdev_major, DM_MAX_SLOT*DM_MAX_BAY);
#endif
    return ret;

_exit_device_created:
    device_destroy(dm_class, MKDEV(cdev_major, DM_MAX_SLOT*DM_MAX_BAY));
_exit_class_created:
    class_destroy(dm_class);
    dm_class = NULL;
_exit_cdev_added:
    cdev_del(&main_cdev);
_exit_chrdev_region_alloced:
    unregister_chrdev_region(devno, DM_MAX_SLOT*DM_MAX_BAY+1);
    cdev_major = 0;
_exit_no_chrdev_region:

    return ret;
}

static void
dm_enable_hw_int(dm_dev_t *dm_dev)
{
    volatile uint32_t *base;
    volatile uint32_t *reg;

    base = dm_dev->pp_dev->config.base;

    /*
     * configure switch MG and GPIO interrupts to be routed to the PCIe
     */

    reg = (uint32_t *)((unsigned long)base +
                        DM_PCIE_EP_INT_MASK_HIGH_REG);

    *reg  = ((1 << DM_PCIE_SWITCH_MG_INT_SHIFT) |
             (1 << DM_PCIE_GPIOLO_INT_SHIFT));

    /*
     * enable POE(GPIO PIN25) and PHY(GPIO PIN26) low level interrupt
     * 1. change the polarity of active level
     * 2. clear previous interrupt status
     * 3. enable the level interrupt
     */
    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_DATA_IN_POLARITY_REG);
    *reg |= ((1 << DM_GPIOHI_POE_PIN25_INT_SHIFT) |
             (1 << DM_GPIOHI_PHY_PIN26_INT_SHIFT));

#ifdef YWEN
    /* clear PHY and POE GPIO interrupt status */
    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_INT_CAUSE_REG);
    *reg &= ~((1 << DM_GPIOHI_POE_PIN25_INT_SHIFT) |
	     (1 << DM_GPIOHI_PHY_PIN26_INT_SHIFT));

    /* enable PHY and POE GPIO interrupts */
    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_INT_LEVEL_MASK_REG);
    *reg |= ((1 << DM_GPIOHI_POE_PIN25_INT_SHIFT) |
             (1 << DM_GPIOHI_PHY_PIN26_INT_SHIFT));
#endif
}

static void
dm_disable_hw_int(dm_dev_t *dm_dev)
{
    volatile uint32_t *base;
    volatile uint32_t *reg;

    base = dm_dev->pp_dev->config.base;

    /*
     * disable POE(GPIO PIN25) and PHY(GPIO PIN26) low level interrupt
     * 1. restore the polarity of active level
     * 2. clear previous interrupt status
     * 3. disable the level interrupt
     */
    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_DATA_IN_POLARITY_REG);
    *reg &= ~((1 << DM_GPIOHI_POE_PIN25_INT_SHIFT) |
              (1 << DM_GPIOHI_PHY_PIN26_INT_SHIFT));

    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_INT_CAUSE_REG);
    *reg &= ~((1 << DM_GPIOHI_POE_PIN25_INT_SHIFT) |
              (1 << DM_GPIOHI_PHY_PIN26_INT_SHIFT));


    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_INT_LEVEL_MASK_REG);
    *reg &= ~((1 << DM_GPIOHI_POE_PIN25_INT_SHIFT) |
              (1 << DM_GPIOHI_PHY_PIN26_INT_SHIFT));

    /*
     * de-configure switch MG and GPIO interrupts to be routed to the PCIe
     */

    reg = (uint32_t *)((unsigned long)base +
                        DM_PCIE_EP_INT_MASK_HIGH_REG);
    *reg = 0;

}


static int
dm_sub_open(struct inode *inode, struct file *filp)
{

    dm_dev_t *dm_dev;

    dm_dev = container_of(inode->i_cdev, dm_dev_t, cdev);

    /*
     * only one user of a slot/bay specific sub cdev is allowed.
     */
    if(atomic_inc_return(&dm_dev->users) > 1) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to open a busy dm sub cdev(%u/%u).\n",
                   dm_dev->slot, dm_dev->bay);

        atomic_dec(&dm_dev->users);
        return -EBUSY;
    }

    filp->private_data = dm_dev;
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "A dm sub cdev(%u/%u) is opened.\n",
               dm_dev->slot, dm_dev->bay);
#endif
    return 0;
}

static int
dm_sub_release(struct inode *inode, struct file *filp)
{

    dm_dev_t *dm_dev;

    /*
     * the sub cdev is released when the according dm
     * instance is plugged out.
     */
    dm_dev = filp->private_data;
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "A dm sub cdev(%u/%u) is released.\n",
               dm_dev->slot, dm_dev->bay);
#endif
    dm_dev->flag &= ~DM_DEV_FLAG_CONFIG_MAP_BIT;
    dm_dev->flag &= ~DM_DEV_FLAG_PPREGS_MAP_BIT;

    if(dm_dev->flag & DM_DEV_FLAG_INT_EN_BIT) {
        dm_disable_hw_int(dm_dev);
        disable_irq(dm_dev->pp_dev->pdev->irq);
    }

    if(dm_dev->flag & DM_DEV_FLAG_INT_BIT) {
        free_irq(dm_dev->pp_dev->pdev->irq, dm_dev);
        dm_dev->flag &= ~DM_DEV_FLAG_INT_BIT;
    }

    sema_init(&dm_dev->sem, 0);

    if(waitqueue_active(&dm_dev->wq)) {
        dm_dev->event_pending = 1;
        wake_up_interruptible(&dm_dev->wq);
    }

    atomic_dec(&dm_dev->users);

    return 0;
}

static irqreturn_t
dm_sub_isr(int irq, void *arg)
{
    dm_dev_t *dm_dev = arg;

    /*
     * wake up user space ISR routine
     */
    up(&dm_dev->sem);

    return IRQ_HANDLED;
}

static int
dm_sub_ioctl(struct inode *inode, struct file *filp,
              unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    dm_dev_t *dm_dev = filp->private_data;

    pcie_config_reg_t conf_reg;
    intrline_to_vector_t int_vec;
    pcie_find_dev_t find_dev;
    config_ppregs_size_t config_ppregs;
    read_write_data_t rw_data;
    dma_mem_t dma_mem;
    void *vaddr = NULL;
    dma_addr_t dma_handle = 0;
    unsigned long base;

    if (_IOC_TYPE(cmd) != DM_SUB_IOC_MAGIC) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "The dm sub cdev(%u/%u) receives wrong cmd(%u,%c).\n",
                   dm_dev->slot, dm_dev->bay, cmd, _IOC_TYPE(cmd));

        return -ENOTTY;
    }

    switch(cmd) {
        case DM_SUB_IOC_INTCONNECT:
            if(dm_dev->flag & DM_DEV_FLAG_INT_BIT) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) has installed ISR(%u).\n",
                           dm_dev->slot, dm_dev->bay, (unsigned int )arg);
                return -EPERM;
            }

            /*
             * one sub cdev is bound to one pp. So the arg is checked
             * against related pp irq
             */
            if((unsigned int )arg != dm_dev->pp_dev->pdev->irq) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) requests wrong irq(%u, %u).\n",
                           dm_dev->slot, dm_dev->bay,
                           (unsigned int )arg, dm_dev->pp_dev->pdev->irq);
                return -EPERM;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_INTCONNECT "
                       "io cmd(%u).\n",
                       dm_dev->slot, dm_dev->bay, (unsigned int )arg);
#endif
            /*
             * MSI interrupt is not shared?
             */
            ret = request_irq((unsigned int )arg, (irq_handler_t)dm_sub_isr,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
                              IRQF_DISABLED, nim_dm_driver_name,
#else 
                              0x0 , nim_dm_driver_name,
#endif 
                              dm_dev);
            if(ret)
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fail to install ISR(%u).\n",
                           dm_dev->slot, dm_dev->bay, (unsigned int )arg);
            /*
             * the irq is disabled by default.
             */
            disable_irq((unsigned int )arg);

            dm_dev->flag |= DM_DEV_FLAG_INT_BIT;

            break;

        case DM_SUB_IOC_INTENABLE:
            if((unsigned int )arg != dm_dev->pp_dev->pdev->irq) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) enables wrong irq(%u, %u).\n",
                           dm_dev->slot, dm_dev->bay,
                           (unsigned int )arg, dm_dev->pp_dev->pdev->irq);
                return -EPERM;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_INTENABLE "
                       "io cmd(%u).\n",
                       dm_dev->slot, dm_dev->bay, (unsigned int )arg);
#endif
            enable_irq((unsigned int )arg);

            dm_enable_hw_int(dm_dev);

            dm_dev->flag |= DM_DEV_FLAG_INT_EN_BIT;

            break;

        case DM_SUB_IOC_INTDISABLE:
            if((unsigned int )arg != dm_dev->pp_dev->pdev->irq) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) disables wrong irq(%u, %u).\n",
                           dm_dev->slot, dm_dev->bay,
                           (unsigned int )arg, dm_dev->pp_dev->pdev->irq);
                return -EPERM;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_INTDISABLE "
                       "io cmd(%u).\n",
                       dm_dev->slot, dm_dev->bay, (unsigned int )arg);
#endif
            dm_disable_hw_int(dm_dev);

            disable_irq((unsigned int )arg);

            dm_dev->flag &= ~DM_DEV_FLAG_INT_EN_BIT;

            break;

        case DM_SUB_IOC_WAIT:
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_WAIT "
                       "io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            base = (unsigned long)dm_dev->pp_dev->config.base;
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) is waiting hardware interrupt"
                       "(%#x, %#x, %#x)\n",
                       dm_dev->slot, dm_dev->bay,
                       *(unsigned int*)(base + DM_GPIOLO_DATA_IN_REG),
                       *(unsigned int*)(base + DM_MAIN_INT_CAUSE_LOW_REG),
                       *(unsigned int*)(base + DM_MAIN_INT_CAUSE_HI_REG));
#endif
            /*
             * for user space ISR routine waiting for hw interrupt
             */
            if (down_interruptible(&dm_dev->sem)) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) DM_SUB_IOC_WAIT "
                           "is interrupted.\n",
                           dm_dev->slot, dm_dev->bay);
                return -ERESTARTSYS;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives hardware interrupt"
                       "(%#x, %#x, %#x)\n",
                       dm_dev->slot, dm_dev->bay,
                       *(unsigned int*)(base + DM_GPIOLO_DATA_IN_REG),
                       *(unsigned int*)(base + DM_MAIN_INT_CAUSE_LOW_REG),
                       *(unsigned int*)(base + DM_MAIN_INT_CAUSE_HI_REG));
#endif
            break;

        case DM_SUB_IOC_INTDISCONNECT:
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_INTDISCONNECT "
                       "io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            /*
             * for user space signal ISR routine to exit
             */
            if(dm_dev->flag & DM_DEV_FLAG_INT_BIT) {
                free_irq(dm_dev->pp_dev->pdev->irq, dm_dev);
                dm_dev->flag &= ~DM_DEV_FLAG_INT_BIT;
            }

            up(&dm_dev->sem);

            break;

        case DM_SUB_IOC_PCIECONFIGWRITEREG:
            ret = copy_from_user(&conf_reg, (void*)arg,
                                 sizeof(pcie_config_reg_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy config reg "
                           "from user space(%p) for "
                           "DM_SUB_IOC_PCIECONFIGWRITEREG io cmd. "
                           "Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            /*
             * one sub cdev is bound to one pp. So the arg is checked
             * against related pp bus, dev and func no.
             */
            if(conf_reg.bus != dm_dev->pp_dev->pdev->bus->number ||
               conf_reg.dev != PCI_SLOT(dm_dev->pp_dev->pdev->devfn) ||
               conf_reg.func != PCI_FUNC(dm_dev->pp_dev->pdev->devfn)) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) with bus/dev/func"
                           "(%u/%u/%u) don't match DM_SUB_IOC_PCIECONFIGWRITEREG "
                           "io cmd(%u/%u/%u)\n",
                           dm_dev->slot, dm_dev->bay,
                           dm_dev->pp_dev->pdev->bus->number,
                           PCI_SLOT(dm_dev->pp_dev->pdev->devfn),
                           PCI_FUNC(dm_dev->pp_dev->pdev->devfn),
                           conf_reg.bus, conf_reg.dev, conf_reg.func);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_PCIECONFIGWRITEREG io cmd(%u, %u).\n",
                        dm_dev->slot, dm_dev->bay, conf_reg.reg, conf_reg.data);
#endif
            ret = pci_write_config_dword(dm_dev->pp_dev->pdev, conf_reg.reg,
                                         conf_reg.data);
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to write config "
                           "reg(%u, %u). Error code = %d\n",
                           dm_dev->slot, dm_dev->bay,
                           conf_reg.reg, conf_reg.data, ret);

                return ret;
            }

            break;

        case DM_SUB_IOC_PCIECONFIGREADREG:
            ret = copy_from_user(&conf_reg, (void*)arg,
                                 sizeof(pcie_config_reg_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy config reg "
                           "from user space(%p) for "
                           "DM_SUB_IOC_PCIECONFIGREADREG io cmd. "
                           "Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            if(conf_reg.bus != dm_dev->pp_dev->pdev->bus->number ||
               conf_reg.dev != PCI_SLOT(dm_dev->pp_dev->pdev->devfn) ||
               conf_reg.func != PCI_FUNC(dm_dev->pp_dev->pdev->devfn)) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) with bus/dev/func"
                           "(%u/%u/%u) don't match DM_SUB_IOC_PCIECONFIGREADREG "
                           "io cmd(%u/%u/%u)\n",
                           dm_dev->slot, dm_dev->bay,
                           dm_dev->pp_dev->pdev->bus->number,
                           PCI_SLOT(dm_dev->pp_dev->pdev->devfn),
                           PCI_FUNC(dm_dev->pp_dev->pdev->devfn),
                           conf_reg.bus, conf_reg.dev, conf_reg.func);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_PCIECONFIGREADREG io cmd(%u).\n",
                        dm_dev->slot, dm_dev->bay, conf_reg.reg);
#endif
            ret = pci_read_config_dword(dm_dev->pp_dev->pdev, conf_reg.reg,
                                        &conf_reg.data);
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to read config reg(%u). "
                           "Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, conf_reg.reg, ret);

                return ret;
            }

            ret = copy_to_user((void*)arg, &conf_reg, sizeof(pcie_config_reg_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy config reg to "
                           "user space(%p) for DM_SUB_IOC_PCIECONFIGREADREG io "
                           "cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            break;

        case DM_SUB_IOC_GETINTVEC:
            ret = copy_from_user(&int_vec, (void*)arg,
                                 sizeof(intrline_to_vector_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy "
                           "intrline_to_vector_t from user space(%p) "
                           "for DM_SUB_IOC_GETINTVEC io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_GETINTVEC "
                       "io cmd(%u).\n",
                       dm_dev->slot, dm_dev->bay, int_vec.intrline);
#endif
            /*
             * irq is allocated when enabling MSI interrupt
             */
            int_vec.vector = dm_dev->pp_dev->pdev->irq;

            ret = copy_to_user((void*)arg, &int_vec,
                               sizeof(intrline_to_vector_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy "
                           "intrline_to_vector_t to user space(%p) "
                           "for DM_SUB_IOC_GETINTVEC io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            break;

        case DM_SUB_IOC_FIND_DEV:
            ret = copy_from_user(&find_dev, (void*)arg, sizeof(pcie_find_dev_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to pcie_find_dev_t "
                           "from user space(%p) for DM_SUB_IOC_FIND_DEV io "
                           "cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            if(find_dev.vendorId != dm_dev->pp_dev->pdev->vendor ||
               find_dev.devId != dm_dev->pp_dev->pdev->device) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) with vendorId/devId"
                           "(%u/%u) don't match DM_SUB_IOC_FIND_DEV io "
                           "cmd(%u/%u/%u)\n",
                           dm_dev->slot, dm_dev->bay,
                           dm_dev->pp_dev->pdev->vendor,
                           dm_dev->pp_dev->pdev->device,
                           find_dev.vendorId, find_dev.devId, find_dev.instance);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives DM_SUB_IOC_FIND_DEV "
                       "io cmd (%u/%u/%u).\n",
                       dm_dev->slot, dm_dev->bay,
                       find_dev.vendorId, find_dev.devId, find_dev.instance);
#endif
            find_dev.bus = dm_dev->pp_dev->pdev->bus->number;
            find_dev.dev = PCI_SLOT((dm_dev->pp_dev->pdev->devfn));
            find_dev.func = PCI_FUNC((dm_dev->pp_dev->pdev->devfn));

            ret = copy_to_user((void*)arg, &find_dev, sizeof(pcie_find_dev_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy "
                           "pcie_find_dev_t to user space(%p) "
                           "for DM_SUB_IOC_FIND_DEV io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            break;

        case DM_SUB_IOC_CONFIG_PPREGS_SIZE:
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_CONFIG_PPREGS_SIZE io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            /*
             * both config size and ppregs size should be page size alignment.
             */
            config_ppregs.config_size = dm_dev->pp_dev->config.size;
            config_ppregs.ppregs_size = dm_dev->pp_dev->ppregs.size;

            ret = copy_to_user((void*)arg, &config_ppregs,
                               sizeof(config_ppregs_size_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy "
                           "config_ppregs_size_t to user space(%p) "
                           "for DM_SUB_IOC_GETINTVEC io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            break;

        case DM_SUB_IOC_U64_READ:
            /*
             * only meaningful to 32bit user space app
             */
            ret = copy_from_user(&rw_data, (void*)arg, sizeof(read_write_data_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy u64 data info "
                           "from user space(%p) for DM_SUB_IOC_U64_READ io cmd. "
                           "Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_U64_READ io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            if(rw_data.type == READ_WRITE_TYPE_CONFIG)
                rw_data.data = *(unsigned long long *)
                                ((unsigned long)dm_dev->pp_dev->config.base +
                                 (unsigned long)rw_data.offset);
            else
                rw_data.data = *(unsigned long long *)
                                ((unsigned long)dm_dev->pp_dev->ppregs.base +
                                 (unsigned long)rw_data.offset);

            ret = copy_to_user((void*)arg, &rw_data, sizeof(read_write_data_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy u64 data info "
                           "to user space(%p) for DM_SUB_IOC_U64_READ io cmd. "
                           "Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }

            break;

        case DM_SUB_IOC_U64_WRITE:
            /*
             * only meaningful to 32bit user space app
             */
            ret = copy_from_user(&rw_data, (void*)arg, sizeof(read_write_data_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy u64 data info "
                           "from user space(%p) for DM_SUB_IOC_U64_WRITE "
                           "io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_U64_WRITE io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            if(rw_data.type == READ_WRITE_TYPE_CONFIG)
                *(unsigned long long *)
                ((unsigned long)dm_dev->pp_dev->config.base +
                 (unsigned long)rw_data.offset) = rw_data.data;
            else
                *(unsigned long long *)
                ((unsigned long)dm_dev->pp_dev->ppregs.base +
                 (unsigned long)rw_data.offset) = rw_data.data;

            break;

        case DM_SUB_IOC_ALLOC_DMA_MEM:
            /*
             * the memory allocated should be page size alignment
             */
            ret = copy_from_user(&dma_mem, (void*)arg, sizeof(dma_mem_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy dma mem info "
                           "from user space(%p) for DM_SUB_IOC_ALLOC_DMA_MEM "
                           "io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_ALLOC_DMA_MEM io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            vaddr = pci_alloc_consistent(dm_dev->pp_dev->pdev, dma_mem.size,
                                         &dma_handle);
            /*
             * bus address should be 32bit
             * virtual address can be 64bit
             */
            if((vaddr == NULL) ||
               (dma_handle == 0) || ((dma_handle >> 32) != 0)) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to allocate dma mem "
                           "of size(%u) Error info = (%p, %#llx)\n",
                           dm_dev->slot, dm_dev->bay,
                           dma_mem.size, vaddr, (unsigned long long)dma_handle);

                return -EPERM;
            }

            dma_mem.kernel_addr = (unsigned long long)vaddr;
            dma_mem.bus_addr = (unsigned int)dma_handle;

            ret = copy_to_user((void*)arg, &dma_mem, sizeof(dma_mem_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy dma mem info "
                           "to user space(%p) for DM_SUB_IOC_ALLOC_DMA_MEM "
                           "io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) allocate dma mem of size"
                       "(%#x, %p, %#lx)\n",
                       dm_dev->slot, dm_dev->bay,
                       dma_mem.size, vaddr, (unsigned long)dma_handle);
#endif
            break;

        case DM_SUB_IOC_FREE_DMA_MEM:
            ret = copy_from_user(&dma_mem, (void*)arg, sizeof(dma_mem_t));
            if(ret) {
                dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                           "The dm sub cdev(%u/%u) fails to copy dma mem info "
                           "from user space(%p) for DM_SUB_IOC_FREE_DMA_MEM "
                           "io cmd. Error code = %d\n",
                           dm_dev->slot, dm_dev->bay, (void*)arg, ret);

                return ret;
            }
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_FREE_DMA_MEM io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            pci_free_consistent(dm_dev->pp_dev->pdev, dma_mem.size,
                                (void *)(unsigned long)dma_mem.kernel_addr,
                                (unsigned long)dma_mem.bus_addr);
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) free dma mem of size"
                       "(%#x, %#llx, %#x)\n",
                       dm_dev->slot, dm_dev->bay,
                       dma_mem.size, dma_mem.kernel_addr, dma_mem.bus_addr);
#endif
            break;

        case DM_SUB_IOC_EVENT:
            /*
             * support user space evSelect() for cpss lib.
             */
#ifdef DEBUG
            dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                       "The dm sub cdev(%u/%u) receives "
                       "DM_SUB_IOC_EVENT io cmd.\n",
                       dm_dev->slot, dm_dev->bay);
#endif
            dm_dev->event_pending = 1;
            wake_up_interruptible(&dm_dev->wq);

            break;

        default:
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The dm sub cdev(%u/%u) receives unknown io cmd(%u).\n",
                       dm_dev->slot, dm_dev->bay, cmd);

            return -ENOTTY;
    }
    return ret;
}

#if defined(HAVE_COMPAT_IOCTL)
static long dm_sub_ioctl_wrapper (struct file *filp, unsigned int cmd,
                                  unsigned long arg)
{
    return ((long)dm_sub_ioctl(NULL, filp, cmd, arg));
}
#endif


static int
dm_sub_mmap(struct file * filp, struct vm_area_struct *vma)
{
    int ret = 0;
    unsigned long size;
    unsigned long pfn;

    dm_dev_t *dm_dev = filp->private_data;

    vma->vm_flags |= VM_IO;
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    /*
     * Although these two register areas are not continuous physically,
     * the driver makes an assumption that all register areas are continuous
     * from the user space client point of view. The logical memory layout is
     * 1. config space, offset: 0, size: pp_dev->config.size
     * 2. ppregs space, offset: config space size, size: pp_dev->ppregs.size
     * 3. dma space, offset: config space size plus ppregs size, size: not fixed
     * Note: dma space is not a mapped pp register area, it is just for the
     * support of dma kernel memory address mapped to user space address
     */
    if ((((vma->vm_pgoff) << PAGE_SHIFT) == 0) &&
        !(dm_dev->flag & DM_DEV_FLAG_CONFIG_MAP_BIT)) {
#ifdef DEBUG
        dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                   "The dm sub cdev(%u/%u) maps config space of the dev "
                   "(%lx, %lx, %lx, %lx, %lx).\n",
                   dm_dev->slot, dm_dev->bay,
                   vma->vm_start, vma->vm_end, (vma->vm_pgoff) << PAGE_SHIFT,
                   (unsigned long)dm_dev->pp_dev->config.phys,
                   (unsigned long)dm_dev->pp_dev->config.size);
#endif
        size = dm_dev->pp_dev->config.size;
        if(size > vma->vm_end - vma->vm_start)
            size = vma->vm_end - vma->vm_start;

        ret = remap_pfn_range(vma, vma->vm_start,
                              (dm_dev->pp_dev->config.phys) >> PAGE_SHIFT,
                              size, vma->vm_page_prot);
        if(ret) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The dm sub cdev(%u/%u) fails to map config space of "
                       "the dev (%lx, %lx, %lx, %lx, %lx).\n",
                       dm_dev->slot, dm_dev->bay,
                       vma->vm_start, vma->vm_end, (vma->vm_pgoff) << PAGE_SHIFT,
                       (unsigned long)dm_dev->pp_dev->config.phys,
                       (unsigned long)dm_dev->pp_dev->config.size);
            return -EAGAIN;
        }
        dm_dev->flag |= DM_DEV_FLAG_CONFIG_MAP_BIT;

    } else if ((((vma->vm_pgoff)<<PAGE_SHIFT) == dm_dev->pp_dev->config.size) &&
               !(dm_dev->flag & DM_DEV_FLAG_PPREGS_MAP_BIT)) {
#ifdef DEBUG
        dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                   "The dm sub cdev(%u/%u) maps ppregs space of the dev "
                   "(%lx, %lx, %lx, %lx, %lx).\n",
                   dm_dev->slot, dm_dev->bay,
                   vma->vm_start, vma->vm_end, (vma->vm_pgoff) << PAGE_SHIFT,
                   (unsigned long)dm_dev->pp_dev->ppregs.phys,
                   (unsigned long)dm_dev->pp_dev->ppregs.size);
#endif
        size = dm_dev->pp_dev->ppregs.size;
        if(size > vma->vm_end - vma->vm_start)
            size = vma->vm_end - vma->vm_start;

        ret = remap_pfn_range(vma, vma->vm_start,
                              (dm_dev->pp_dev->ppregs.phys) >> PAGE_SHIFT,
                              size, vma->vm_page_prot);
        if(ret) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The dm sub cdev(%u/%u) fails to map ppregs space of "
                       "the dev (%lx, %lx, %lx, %lx, %lx).\n",
                       dm_dev->slot, dm_dev->bay,
                       vma->vm_start, vma->vm_end, (vma->vm_pgoff) << PAGE_SHIFT,
                       (unsigned long)dm_dev->pp_dev->ppregs.phys,
                       (unsigned long)dm_dev->pp_dev->ppregs.size);
            return -EAGAIN;
        }
        dm_dev->flag |= DM_DEV_FLAG_PPREGS_MAP_BIT;

    } else {
#ifdef DEBUG
        dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                   "The dm sub cdev(%u/%u) maps dma space of the dev "
                   "(%lx, %lx, %llx).\n",
                   dm_dev->slot, dm_dev->bay,
                   vma->vm_start, vma->vm_end, (((vma->vm_pgoff) << PAGE_SHIFT) -
                   dm_dev->pp_dev->config.size - dm_dev->pp_dev->ppregs.size));
#endif
        pfn = (((vma->vm_pgoff) << PAGE_SHIFT) -
               (dm_dev->pp_dev->config.size +
                dm_dev->pp_dev->ppregs.size)) >> PAGE_SHIFT;
        size = vma->vm_end - vma->vm_start;

        ret = remap_pfn_range(vma, vma->vm_start, pfn,
                              size, vma->vm_page_prot);
        if(ret) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The dm sub cdev(%u/%u) fails to map dma space of the dev "
                       "(%lx, %lx, %llx).\n",
                       dm_dev->slot, dm_dev->bay,
                       vma->vm_start, vma->vm_end,
                       (((vma->vm_pgoff) << PAGE_SHIFT) -
                        dm_dev->pp_dev->config.size -
                        dm_dev->pp_dev->ppregs.size));

            return -EAGAIN;
        }
    }

    return ret;
}

/*
 * support user space evSelect() for cpss lib.
 */
static unsigned int
dm_sub_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    dm_dev_t *dm_dev = filp->private_data;

    poll_wait(filp, &dm_dev->wq, wait);
    if(dm_dev->event_pending)
    {
        mask = (POLLIN | POLLRDNORM);
        dm_dev->event_pending = 0;
    }
    return mask;
}

static ssize_t show_sub_cdev_attr(struct device *dev,
                                  struct device_attribute *attr,
                                  char *buf)
{
    unsigned int len = 0;
    dm_dev_t *dm_dev = dev_get_drvdata(dev);

    len += sprintf(buf+len, "DM slot/bay: %u/%u\n",
                   dm_dev->slot, dm_dev->bay);

    len += sprintf(buf+len, "DM flag: 0x%llx\n",
                   dm_dev->flag);

    len += sprintf(buf+len, "DM event_pending: %u\n",
                       dm_dev->event_pending);

    len += sprintf(buf+len,
                  "DM config register:\n"
                  "  Phy Addr : 0x%llx\n"
                  "  Virt Addr: 0x%p\n"
                  "  Size     : 0x%llx\n"
                  "DM pp register:\n"
                  "  Phy Addr : 0x%llx\n"
                  "  Virt Addr: 0x%p\n"
                  "  Size     : 0x%llx\n"
                  "DM MSI interrupt:\n"
                  "  IRQ      : 0x%x\n",
                  dm_dev->pp_dev->config.phys,
                  dm_dev->pp_dev->config.base,
                  dm_dev->pp_dev->config.size,
                  dm_dev->pp_dev->ppregs.phys,
                  dm_dev->pp_dev->ppregs.base,
                  dm_dev->pp_dev->ppregs.size,
                  dm_dev->pp_dev->pdev->irq);

    return len;
}

static int
dm_create_sub_cdev(dm_dev_t *dm_dev)
{
    int ret = 0;
    /*
     * each dm instance specific sub cdev assigned a
     * minor device number according to its slot/bay
     */
    int cdev_minor = dm_dev->slot*DM_MAX_BAY + dm_dev->bay;

    atomic_set(&dm_dev->users, 0);
    sema_init(&dm_dev->sem, 0);

    init_waitqueue_head(&dm_dev->wq);
    dm_dev->event_pending = 0;

    cdev_init(&dm_dev->cdev, &dm_sub_cdev_fops);
    dm_dev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&dm_dev->cdev, MKDEV(cdev_major, cdev_minor), 1);
    if(ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the sub cdev: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, cdev_minor, ret);
    }

    dm_dev->cdev_minor = cdev_minor;

    dm_dev->dm_sub_sysdev = device_create(dm_class,
                                          NULL,
                                          MKDEV(cdev_major, cdev_minor),
                                          NULL,
                                          "%s_%u_%u",
                                          nim_dm_driver_name,
                                          dm_dev->slot, dm_dev->bay);
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "create the sub cdev sys device:%s_%u_%u\n",
                                          nim_dm_driver_name,
                                          dm_dev->slot, dm_dev->bay);
#endif
    if (IS_ERR(dm_dev->dm_sub_sysdev)) {
        ret = PTR_ERR(dm_dev->dm_sub_sysdev);
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the sub cdev sys device: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, cdev_minor, ret);

        cdev_del(&dm_dev->cdev);
        return ret;
    }

    ret = device_create_file(dm_dev->dm_sub_sysdev, &dev_attr_dm_dev_info);
    if(ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the sub cdev sys attribute: "
                   "cdev_major=%u, cdev_minor=%u, "
                   "error code=%d\n",
                   cdev_major, cdev_minor, ret);

        device_destroy(dm_class, MKDEV(cdev_major, dm_dev->cdev_minor));
        cdev_del(&dm_dev->cdev);
        return ret;

    }

    dev_set_drvdata(dm_dev->dm_sub_sysdev, dm_dev);

    return ret;
}

static int
dm_get_slot_bay_by_busno(uint8_t busno, uint8_t *slot, uint8_t *bay)
{
    int i = 0,j = 0;
    for(i=0; i<DM_MAX_SLOT; i++) {
        for(j=0; j<DM_MAX_BAY; j++) {
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "\npcie_mapping_table[i][j].flag = %llu, PCIE_MAPPING_VALID_BIT = %x\n"
               "busno = %x >= pcie_mapping_table[%d][%d].sec_bus = %x \n"
               "busno = %x <= pcie_mapping_table[%d][%d].sub_bus = %x \n",
                                          pcie_mapping_table[i][j].flag,PCIE_MAPPING_VALID_BIT,
                                          busno, i, j,pcie_mapping_table[i][j].sec_bus,
                                          busno, i, j,pcie_mapping_table[i][j].sub_bus
                                           );
#endif
           /* CSCur57907 : Dreamliner + Thule can't boot up on Sword system
            * The example for PCIe bus tree,check the sub_bus follow the rule in turn
            * +-04.0-[04-38]----00.0-[05-38]--+-01.0-[06-26]----00.0
            * |                               +-02.0-[27-2f]----00.0
            * |                               \-03.0-[30-38]--      
            */   
            if(j == 0) {
                if((pcie_mapping_table[i][j].flag & PCIE_MAPPING_VALID_BIT) &&
                    (busno == pcie_mapping_table[i][j].sec_bus) &&
                    (busno <= pcie_mapping_table[i][j].sub_bus)) {
                    *slot = i;
                    *bay = j;
                    return 0;
                }
            } else {
                if((pcie_mapping_table[i][j].flag & PCIE_MAPPING_VALID_BIT) &&
                    (busno == pcie_mapping_table[i][j].sec_bus) &&
                    (busno <= pcie_mapping_table[i][j].sub_bus)) {
                    *slot = i;
                    *bay = j;
                    return 0;
                }
            }             
        }
    }
    return -1;
}

static inline dm_dev_t **
dm_get_pdev_by_slot_bay(uint8_t slot, uint8_t bay)
{
    return &g_dm_devs[slot][bay];
}

static void
dm_config_switch_chip(dm_dev_t *dm_dev)
{
    volatile uint32_t *base;
    volatile uint32_t *base1;
    volatile uint32_t *reg;
    base = dm_dev->pp_dev->config.base;
    base1 = dm_dev->pp_dev->ppregs.base;

    /*
     * configure PCIe access window0 to map BAR1 to PP reg area
     */
    reg = (uint32_t *)((unsigned long)base +
                       DM_PCIE_ACCESS_WIN0_CONTROL_REG);
    *reg = 0x03ff00c1;

    reg = (uint32_t *)((unsigned long)base +
                       DM_PCIE_ACCESS_WIN0_BASE_REG);
    *reg = (uint32_t)dm_dev->pp_dev->ppregs.phys;

    /*
     * check if MSI interrupt is enabled. if not, enable it.
     */
    reg = (uint32_t *)((unsigned long)base +
                       DM_PCIE_MSI_MSG_CONTROL_REG);

    if(!(*reg & (0x1 << DM_PCIE_MSI_ENABLE_SHIFT))) {

        *reg |= (0x1 << DM_PCIE_MSI_ENABLE_SHIFT);
    }

    /*
     * configure MPP pins for GPIO PIN 24/25/26/27/30
     */
    reg = (uint32_t *)((unsigned long)base +
                       DM_MPP_CONTROL_3_REG);
    *reg &= 0xF0FF0000;

    /*
     * configure POE(GPIO PIN25), PHY(GPIO PIN26) and GPIO PIN20 as input
     * GPIO PIN24, PIN27 and PIN30 as output. 
     */
    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_DATA_OUT_EN_REG);
    *reg &= ~((0x1 << DM_GPIOHI_POE_PIN24_RST_SHIFT) |
	      (0x1 << DM_GPIOHI_PHY_PIN27_RST_SHIFT) |
	      (0x1 << DM_GPIOHI_MODULE_RD_PIN30_SHIFT));

    /* take PHY and POE out of reset */
    reg = (uint32_t *)((unsigned long)base +
                       DM_GPIOLO_DATA_OUT_REG);
    *reg |= ((0x1 << DM_GPIOHI_POE_PIN24_RST_SHIFT) |
	     (0x1 << DM_GPIOHI_PHY_PIN27_RST_SHIFT));

    /*
     * configure GPP1 and GPP2 as input to avoid conflict
     * with GPIO PIN25 and GPIO PIN26
     */
    reg = (uint32_t *)((unsigned long)base1 +
                       DM_GPP_IO_CONTROL_REG);
    *reg &= ~((0x1 << DM_GPP1_IO_CONTROL_SHIFT) |
              (0x1 << DM_GPP2_IO_CONTROL_SHIFT));

    /* configure I/O selectors register */
    reg = (uint32_t *)((unsigned long)base1 + 0xb0);
    *reg |= 0x8;
}

static int
dm_probe (struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int ret = 0;
    uint8_t slot, bay;
    pp_dev_t *pp_dev;
    dm_dev_t *dm_dev, **dm_dev_p;
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,"\n*** DBG :dm_probe\n");
#endif

    ret = pci_enable_device(pdev);
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Cannot enable pcie device for dm\n");
        ret = -ENODEV;
        goto _exit_no_pcie;
    }

    if(dm_get_slot_bay_by_busno(pdev->bus->number, &slot, &bay)) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to map the dev to slot/bay.\n");
        ret = -EPERM;
        goto _exit_was_enabled;
    }

    dm_dev_p = dm_get_pdev_by_slot_bay(slot, bay);

    if((dm_dev = *dm_dev_p) != NULL) {
        if(dm_dev->flag & DM_DEV_FLAG_ACTIVE_BIT) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "The device is not released.\n");
            ret = -EPERM;
            goto _exit_was_enabled;
        }
        pp_dev = dm_dev->pp_dev;
    } else {
        pp_dev = kzalloc(sizeof(pp_dev_t), GFP_ATOMIC);
        if (!pp_dev) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "Cannot allocate memory for pp object.\n");
            ret = -ENOMEM;
            goto _exit_was_enabled;
        }

        dm_dev = kzalloc(sizeof(dm_dev_t), GFP_ATOMIC);
        if (!dm_dev) {
            dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                       "Cannot allocate memory for dm object\n");
            kfree(pp_dev);
            ret = -ENOMEM;
            goto _exit_was_enabled;
        }
        dm_dev->pp_dev = pp_dev;
        dm_dev->slot = slot;
        dm_dev->bay = bay;
        *dm_dev_p = dm_dev;
    }
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "\ndm_dev->slot(%u) dm_dev->bay (%u)\n", dm_dev->slot, dm_dev->bay);
#endif

    ret = pci_request_regions(pdev, nim_dm_driver_name);
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Failed to get regions.\n");
        ret = -EINVAL;
        goto _exit_was_enabled;
    }

    pci_set_master(pdev);
    ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to set the DMA mask(%d).\n", ret);
        ret = -EPERM;
        goto _exit_was_regioned;
    }

    ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to set the consistent DMA mask(%d).\n", ret);
        ret = -EPERM;
        goto _exit_was_regioned;
    }


    pp_dev->config.phys = pci_resource_start(pdev, DM_PCIE_CONFIG_BAR);
    pp_dev->config.size = pci_resource_len(pdev, DM_PCIE_CONFIG_BAR);
    pp_dev->config.base = ioremap_nocache(pp_dev->config.phys,
                                          pp_dev->config.size);

    if (!pp_dev->config.base) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Failed to map dm config registers "
                   "into kernel virtual space.\n");
        ret = -ENOMEM;
        goto _exit_was_regioned;
    }

    pp_dev->ppregs.phys = pci_resource_start(pdev, DM_PCIE_PPREGS_BAR);
    pp_dev->ppregs.size = pci_resource_len(pdev, DM_PCIE_PPREGS_BAR);
    pp_dev->ppregs.base = ioremap_nocache(pp_dev->ppregs.phys,
                                          pp_dev->ppregs.size);

    if (!pp_dev->ppregs.base) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Failed to map dm pp registers "
                   "into kernel virtual space\n");
        ret = -ENOMEM;
        goto _exit_config_mapped;
    }

    /*
     * configure the pp chip. if necessary, enable MSI.
     */
    dm_config_switch_chip(dm_dev);

    ret = pci_enable_msi(pdev);
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to enable pp MSI interrupt. Error = %d\n", ret);
        ret = -EPERM;
        goto _exit_all_mapped;
    }


    pp_dev->pdev = pdev;
    pci_set_drvdata(pdev, dm_dev);
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
               "DM config register:\n"
               "  Phy Addr : 0x%llx\n"
               "  Virt Addr: 0x%p\n"
               "  Size     : 0x%llx\n"
               "DM pp register:\n"
               "  Phy Addr : 0x%llx\n"
               "  Virt Addr: 0x%p\n"
               "  Size     : 0x%llx\n"
               "DM MSI interrupt:\n"
               "  IRQ      : 0x%x\n",
               pp_dev->config.phys,
               pp_dev->config.base,
               pp_dev->config.size,
               pp_dev->ppregs.phys,
               pp_dev->ppregs.base,
               pp_dev->ppregs.size,
               pdev->irq);
#endif
    if (dm_create_sub_cdev(dm_dev)) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to create the sub cdev.\n");
        ret = -EPERM;
        goto _exit_msi_enabled;
    }

    dm_dev->flag |= DM_DEV_FLAG_ACTIVE_BIT;
    return ret;

_exit_msi_enabled:
    pci_disable_msi(pdev);
_exit_all_mapped:
    iounmap(pp_dev->ppregs.base);
_exit_config_mapped:
    iounmap(pp_dev->config.base);
_exit_was_regioned:
    pci_release_regions(pdev);
_exit_was_enabled:
    pci_disable_device(pdev);
_exit_no_pcie:

    return ret;

}

static void
dm_remove(struct pci_dev *pdev)
{
    dm_dev_t *dm_dev = NULL;

    dm_dev = pci_get_drvdata(pdev);
    if(dm_dev != NULL) {
#ifdef DEBUG
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "DM device (%u/%u)(pcie bus no %u) is removed.\n",
                   dm_dev->slot, dm_dev->bay, pdev->bus->number);
#endif
        device_remove_file(dm_dev->dm_sub_sysdev, &dev_attr_dm_dev_info);
        device_destroy(dm_class, MKDEV(cdev_major, dm_dev->cdev_minor));
        cdev_del(&dm_dev->cdev);
        if(dm_dev->flag & DM_DEV_FLAG_INT_BIT)
            free_irq(pdev->irq, dm_dev);

        iounmap(dm_dev->pp_dev->ppregs.base);
        iounmap(dm_dev->pp_dev->config.base);
    } else {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
                   "Fail to get the stored dm dev.\n");
        dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
                   "DM device (pcie bus no %u) is removed.\n",
                   pdev->bus->number);
    }

    pci_disable_msi(pdev);
    pci_release_regions(pdev);
    pci_disable_device(pdev);

    if(dm_dev != NULL)
        dm_dev->flag = 0;

    return;
}


static int __init nim_dm_init (void)
{
    int ret = 0;
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
           "dm kernel driver is loaded. "
           "Date "__DATE__" "__TIME__"\n");
#endif
    ret = dm_create_main_cdev();
    if (ret) {
        dm_drv_log(DM_DRV_LOG_LEVEL_ERROR,
               "Fail to create dm main cdev. "
               "Error code = %d\n", ret);

        return ret;
    }

    return (ret);
}

static void __exit nim_dm_exit (void)
{
    int i=0, j=0;
#ifdef DEBUG
    dm_drv_log(DM_DRV_LOG_LEVEL_INFO,
           "dm kernel driver is unloaded. "
           "Date "__DATE__" "__TIME__"\n");
#endif
    if (pcie_mapping_driver_initialized)
        pci_unregister_driver(&pcie_dm_driver);

    for (i=0; i<DM_MAX_SLOT; i++) {
        for (j=0; j<DM_MAX_BAY; j++) {
            if(g_dm_devs[i][j] != NULL &&
               g_dm_devs[i][j]->pp_dev != NULL) {
                kfree(g_dm_devs[i][j]->pp_dev);
                kfree(g_dm_devs[i][j]);
                g_dm_devs[i][j] = NULL;
            }
        }
    }

    device_remove_file(dm_main_sysdev, &dev_attr_pcie_mapping);
    device_destroy(dm_class, MKDEV(cdev_major, DM_MAX_SLOT*DM_MAX_BAY));
    class_destroy(dm_class);
    dm_class = NULL;
    cdev_del(&main_cdev);
    unregister_chrdev_region(MKDEV(cdev_major, 0), DM_MAX_SLOT*DM_MAX_BAY+1);
    cdev_major = 0;

    return;
}

module_init(nim_dm_init);
module_exit(nim_dm_exit);

/******** History ********
$Log: nim_dm.c,v $
Revision 1.4  2018/05/18 09:25:02  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.3.20.4  2017/10/13 03:29:21  alpeng
remove useless msg, since there is no failure for victory and neptune

Revision 1.3.20.3  2017/06/14 06:43:57  alpeng
support 4.9 kernel

Revision 1.3.20.2  2016/11/14 09:35:07  alpeng
before rommon, using equal instead of large equal for secondary bus checking

Revision 1.3.20.1  2016/10/28 08:27:48  alpeng
update file permission for kernel restriction, add is_neptune

Revision 1.3  2015/02/27 10:02:34  iachang

Add support dreamliner NIM

Revision 1.2.8.1  2015/02/14 07:36:35  iachang
Dreamliner Diag sync with main trunk.

Revision 1.2.6.1  2015/02/05 13:52:02  iachang
Fixed Juno NIM 3 PCIe mapping issue

Revision 1.2  2014/11/06 01:55:05  iachang
CSCur57907 : Fixed Dreamliner + Thule can't boot up on Sword issue

Revision 1.1  2014/09/23 20:37:13  ywen
Add support for Dreamliner.


$Endlog$
*/

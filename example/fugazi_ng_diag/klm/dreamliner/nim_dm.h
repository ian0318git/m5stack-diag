/* $Id: nim_dm.h,v 1.1 2014/09/23 20:37:21 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/dreamliner/nim_dm.h,v $
 *------------------------------------------------------------------
 *
 * nim_dm.h - This file supports Dreamliner.
 *
 * Christine Wen -- Jan. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*------------------------------------------------------------------
 * DM kernel driver related definitions
 *
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __NIM_DM_H__
#define __NIM_DM_H__

/*
 * Use this macro for dm kernel driver logging.
 */
extern unsigned int dm_drv_log_level;
#define DM_DRV_LOG_LEVEL_NONE         0x00000000
#define DM_DRV_LOG_LEVEL_ERROR        0x00000001
#define DM_DRV_LOG_LEVEL_INFO         0x00000002
#define DM_DRV_LOG_LEVEL_DEBUG        0x00000004
#define DM_DRV_LOG_LEVEL_ALL          0x00000100

#define dm_drv_log(level, format, a...) \
do { \
    if (dm_drv_log_level != DM_DRV_LOG_LEVEL_NONE) { \
        if (((level & dm_drv_log_level) == DM_DRV_LOG_LEVEL_ERROR) || \
            (dm_drv_log_level & DM_DRV_LOG_LEVEL_ALL)) { \
            printk(KERN_ERR "%s: " format, __FUNCTION__, ## a); \
         } else if (level & dm_drv_log_level) { \
            printk(KERN_INFO "%s: " format, __FUNCTION__, ## a); \
        } \
    } \
} while(0)


#define DM_PCIE_VENDOR_ID_MARVELL     0x11AB
/*
 * for NIM-ES2-8 and NIM-ES2-8-P
 */
#define DM_PCIE_DEVICE_ID_98DX3133    0xE61E
/*
 * for NIM-ES2-4 and NIM-ES2-4-P
 */
#define DM_PCIE_DEVICE_ID_98DX3033B   0xE75A

#define DM_PCIE_CONFIG_BAR   0
#define DM_PCIE_PPREGS_BAR   2

#define DM_PCIE_ACCESS_WIN0_CONTROL_REG    0x41820
#define DM_PCIE_ACCESS_WIN0_BASE_REG       0x41824
#define DM_PCIE_MSI_MSG_CONTROL_REG        0x40050
#define DM_PCIE_EP_INT_MASK_HIGH_REG       0x2021C

#define DM_PCIE_MSI_ENABLE_SHIFT           16
#define DM_PCIE_SWITCH_MG_INT_SHIFT        23
#define DM_PCIE_GPIOLO_INT_SHIFT           6

#define DM_MPP_CONTROL_3_REG               0x1000C
#define DM_GPP_IO_CONTROL_REG              0x18001C8

#define DM_GPP1_IO_CONTROL_SHIFT           1
#define DM_GPP2_IO_CONTROL_SHIFT           2

#define DM_GPIOLO_DATA_OUT_REG             0x10100
#define DM_GPIOLO_DATA_OUT_EN_REG          0x10104
#define DM_GPIOLO_DATA_IN_POLARITY_REG     0x1010C
#define DM_GPIOLO_DATA_IN_REG              0x10110
#define DM_GPIOLO_INT_CAUSE_REG            0x10114
#define DM_GPIOLO_INT_LEVEL_MASK_REG       0x1011C

#define DM_GPIOHI_POE_PIN24_RST_SHIFT      24
#define DM_GPIOHI_POE_PIN25_INT_SHIFT      25
#define DM_GPIOHI_PHY_PIN26_INT_SHIFT      26
#define DM_GPIOHI_PHY_PIN27_RST_SHIFT      27
#define DM_GPIOHI_MODULE_RD_PIN30_SHIFT    30


#define DM_MAIN_INT_CAUSE_LOW_REG          0x20200
#define DM_MAIN_INT_CAUSE_HI_REG           0x20210


#define DM_MAX_SLOT    6
#define DM_MAX_BAY     5


typedef struct mem_region_s {
    /*
     * physical address
     */
    resource_size_t phys;

    /*
     * size of the memory area
     */
    resource_size_t size;

    /*
     * kernel virtual address
     */
    void *base;
} mem_region_t;

typedef struct pp_dev_s {
    struct pci_dev *pdev;

    /*
     * Configuration space
     */
    mem_region_t    config;

    /*
     * PP registers space
     */
    mem_region_t    ppregs;
} pp_dev_t;

#define DM_DEV_FLAG_ACTIVE_BIT        0x01
#define DM_DEV_FLAG_INT_BIT           0x02
#define DM_DEV_FLAG_INT_EN_BIT        0x04
#define DM_DEV_FLAG_CONFIG_MAP_BIT    0x08
#define DM_DEV_FLAG_PPREGS_MAP_BIT    0x10
typedef struct dm_dev_s {
    pp_dev_t *pp_dev;

    /*
     * char device of specific slot/bay dm instance
     */
    struct cdev cdev;

    /*
     * minor device number of sub cdev
     */
    int cdev_minor;

    /*
     * tracking users of the sub cdev, only one user is allowed
     */
    atomic_t users;

    /*
     * used for interrupt waking up user space process
     */
    struct semaphore sem;

    /*
     * used for sub cdev sys attribute
     */
    struct device *dm_sub_sysdev;

    /*
     * cpss event
     *
     * for poll method wait queue
     */
    wait_queue_head_t wq;

    /*
     * flag for event occurrence
     */
    uint32_t event_pending;

    uint32_t slot;
    uint32_t bay;

    uint64_t flag;
} dm_dev_t;

#define PCIE_MAPPING_VALID_BIT    0x01
typedef struct pcie_mapping_s {
    /*
     * secondary bus number of PCIe switch port of specific slot/bay
     */
    uint32_t sec_bus;

    /*
     * subordinate bus number of PCIe switch port of specific slot/bay
     */
    uint32_t sub_bus;

    uint64_t flag;
} pcie_mapping_t;


/*
 * the main cdev related functions, which is for global configuration
 */
static int dm_create_main_cdev(void);
static int dm_main_open(struct inode *inode, struct file *filp);
static int dm_main_release(struct inode *inode, struct file *filp);
static int dm_main_ioctl(struct inode *inode, struct file *filp,
                         unsigned int cmd, unsigned long arg);
#if defined(HAVE_COMPAT_IOCTL)
static long dm_main_ioctl_wrapper(struct file *filp, unsigned int cmd,
                                  unsigned long arg);
#endif

/*
 * the sub cdev related functions, which is for slot/bay specific dm instance
 * configuration and operations
 */
static int dm_create_sub_cdev(dm_dev_t *dm_dev);
static int dm_sub_open(struct inode *inode, struct file *filp);
static int dm_sub_release(struct inode *inode, struct file *filp);
static int dm_sub_mmap(struct file *filp, struct vm_area_struct *vma);
static unsigned int dm_sub_poll(struct file *filp, poll_table *wait);
static int dm_sub_ioctl(struct inode *inode, struct file *filp,
                        unsigned int cmd, unsigned long arg);
#if defined(HAVE_COMPAT_IOCTL)
static long dm_sub_ioctl_wrapper(struct file *filp, unsigned int cmd,
                                 unsigned long arg);
#endif

/*
 * pcie driver to detect hot plugged pp devices
 */
static int dm_probe (struct pci_dev *pdev, const struct pci_device_id *ent);
static void dm_remove(struct pci_dev *pdev);

/*
 * the main cdev sys attribute callback APIs
 */
static ssize_t show_main_cdev_attr(struct device *dev,
                                   struct device_attribute *attr,
                                   char *buf);
static ssize_t store_main_cdev_attr(struct device *dev,
                                    struct device_attribute *attr,
                                    const char *buf,
                                    size_t count);

/*
 * the sub cdev sys attribute callback APIs
 */
static ssize_t show_sub_cdev_attr(struct device *dev,
                                  struct device_attribute *attr,
                                  char *buf);




#endif /* _NIM_DM_H_ */

/******** History ********
$Log: nim_dm.h,v $
Revision 1.1  2014/09/23 20:37:21  ywen
Add support for Dreamliner.


$Endlog$
*/

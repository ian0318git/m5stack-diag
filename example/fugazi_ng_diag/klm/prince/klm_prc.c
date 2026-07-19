/* $Id: klm_prc.c,v 1.2 2017/07/18 08:48:45 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/prince/klm_prc.c,v $
 ***********************************************************************
 * File Name: klm_prc.c
 *
 * Description: This KLM contains DMA and mmap() support.
 *
 * Xiaoying Zhang -- Dec. 2012
 *
 * Copyright (c)2012 - 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/cdev.h>  /* for cdev struct */
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#include "../../common/src/prince/pcmap.h"
#include "../../common/src/prince/prince_reg.h"
#include "../../common/src/prince/prince_ge_mac.h"
#include "../../common/src/prince/dev_phy_88e1512.h"

#define MMAP_MAX 1
#define KLM_NAME "prc"

#define MAC_MDIO_INTR_MASK       0x00000001

struct klm_dev_t {
    struct cdev cdev;
};

static dev_t dev = 0;
static struct klm_dev_t mmap_devs[MMAP_MAX];
static int klm_major = 0;
static sys_csr_reg_t *sys_csr_reg_p = NULL;
static ge_mac_reg_t *ge_mac_reg_p = NULL;
static ge_dma_reg_t *ge_dma_reg_p = NULL;
static prince_scc_regs_t *scc_reg_p = NULL;

dma_addr_t phy_addr_ge_rx_buf;
void * cpu_addr_ge_rx_buf;
dma_addr_t phy_addr_ge_tx_buf;
void * cpu_addr_ge_tx_buf;
dma_addr_t phy_addr_ge_rxbd;
void * cpu_addr_ge_rxbd;
dma_addr_t phy_addr_ge_txbd;
void * cpu_addr_ge_txbd;
dma_addr_t phy_addr_scc_rx_buf;
void * cpu_addr_scc_rx_buf;
dma_addr_t phy_addr_scc_tx_buf;
void * cpu_addr_scc_tx_buf;
dma_addr_t phy_addr_scc_rxbd;
void * cpu_addr_scc_rxbd;
dma_addr_t phy_addr_scc_txbd;
void * cpu_addr_scc_txbd;

static int klm_open (struct inode *inode, struct file *file)
{
    return 0;
}

static int klm_release (struct inode *inode, struct file *file) 
{
    return 0;
}

// ioctl - I/O control
static long klm_ioctl(struct file *file,
        unsigned int cmd, unsigned long arg) 
{
    int retval = 0;

    switch ( cmd ) {
    case GET_GE_DMA_RX_BUF_PHYS:
        *(ulong *)arg = (ulong)phy_addr_ge_rx_buf;
        break;
    case GET_GE_DMA_TX_BUF_PHYS:
        *(ulong *)arg = (ulong)phy_addr_ge_tx_buf;
        break;
    case GET_GE_DMA_RXBD_PHYS:
        *(ulong *)arg = (ulong)phy_addr_ge_rxbd;
        break;
    case GET_GE_DMA_TXBD_PHYS:
        *(ulong *)arg = (ulong)phy_addr_ge_txbd;
        break;
    case GET_SCC_DMA_RX_BUF_PHYS:
        *(ulong *)arg = (ulong)phy_addr_scc_rx_buf;
        break;
    case GET_SCC_DMA_TX_BUF_PHYS:
        *(ulong *)arg = (ulong)phy_addr_scc_tx_buf;
        break;
    case GET_SCC_DMA_RXBD_PHYS:
        *(ulong *)arg = (ulong)phy_addr_scc_rxbd;
        break;
    case GET_SCC_DMA_TXBD_PHYS:
        *(ulong *)arg = (ulong)phy_addr_scc_txbd;
        break;
    case ENABLE_IRQ:
        printk(KERN_INFO "irq %u enabled\n", (unsigned int)arg);
        enable_irq((unsigned int)arg);
        break;
    case DISABLE_IRQ:
        printk(KERN_INFO "irq %u disabled\n", (unsigned int)arg);
        disable_irq_nosync((unsigned int)arg);
        break;
    default:
        retval = -EINVAL;
        break;
    }

    return retval;
}

static int klm_mmap(struct file * filp, struct vm_area_struct * vma)
{
    int ret;

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    vma->vm_flags |= VM_RESERVED;

    ret = remap_pfn_range(vma,
                          vma->vm_start,
                          vma->vm_pgoff, /*if use vm_pgoff, don't need to shift by PAGE_SHIFT */ 
                          vma->vm_end-vma->vm_start,
                          vma->vm_page_prot);

    if (ret < 0) {
        printk("kernel asic_utils: remap pfn range failed.");
        return -EIO;
    }

    return 0;
}

static int klm_dma_alloc(void)
{
    cpu_addr_ge_rx_buf = dma_alloc_coherent(NULL, 
                PRINCE_GE_DMA_RXBD_BUF_SIZE * PRINCE_GE_DMA_RXBD_NUM, 
                &phy_addr_ge_rx_buf, 
                GFP_DMA);
    if (cpu_addr_ge_rx_buf == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for GE DMA Rx Buffer");
        return -EAGAIN;
    }
    printk (KERN_NOTICE "allocate 0x%x x 0x%x for GE DMA Rx Buffer",
        PRINCE_GE_DMA_RXBD_BUF_SIZE, PRINCE_GE_DMA_RXBD_NUM);

    cpu_addr_ge_tx_buf = dma_alloc_coherent(NULL, 
                PRINCE_GE_DMA_TXBD_BUF_MAX, 
                &phy_addr_ge_tx_buf, 
                GFP_DMA);
    if (cpu_addr_ge_tx_buf == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for GE DMA Tx Buffer");
        return -EAGAIN;
    }

    cpu_addr_ge_rxbd = dma_alloc_coherent(NULL, 
                PRINCE_GE_DMA_RXBD_NUM * BYTES_PER_BD, 
                &phy_addr_ge_rxbd, 
                GFP_DMA);
    if (cpu_addr_ge_rxbd == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for GE DMA RXBD");
        return -EAGAIN;
    }

    cpu_addr_ge_txbd = dma_alloc_coherent(NULL, 
                PRINCE_GE_DMA_TXBD_NUM * BYTES_PER_BD * PRINCE_GE_DMA_TXBD_TYPE, 
                &phy_addr_ge_txbd, 
                GFP_DMA);
    if (cpu_addr_ge_txbd == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for GE DMA TXBD");
        return -EAGAIN;
    }

    cpu_addr_scc_rx_buf = dma_alloc_coherent(NULL, 
                PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
                &phy_addr_scc_rx_buf, 
                GFP_DMA);
    if (cpu_addr_scc_rx_buf == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for SCC DMA Rx Buffer");
        return -EAGAIN;
    }

    cpu_addr_scc_tx_buf = dma_alloc_coherent(NULL, 
                PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
                &phy_addr_scc_tx_buf, 
                GFP_DMA);
    if (cpu_addr_scc_tx_buf == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for SCC DMA Tx Buffer");
        return -EAGAIN;
    }

    cpu_addr_scc_rxbd = dma_alloc_coherent(NULL, 
                PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
                &phy_addr_scc_rxbd, 
                GFP_DMA);
    if (cpu_addr_scc_rxbd == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for SCC DMA RXBD");
        return -EAGAIN;
    }

    cpu_addr_scc_txbd = dma_alloc_coherent(NULL, 
                PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM, 
                &phy_addr_scc_txbd, 
                GFP_DMA);
    if (cpu_addr_scc_txbd == NULL) {
        printk (KERN_NOTICE "Error failed to allocate memory for SCC DMA TXBD");
        return -EAGAIN;
    }

    return 0;
}

static void klm_dma_free(void)
{
    dma_free_coherent(NULL, 
        PRINCE_GE_DMA_RXBD_BUF_SIZE * PRINCE_GE_DMA_RXBD_NUM, 
        cpu_addr_ge_rx_buf, 
        phy_addr_ge_rx_buf);

    dma_free_coherent(NULL, 
        PRINCE_GE_DMA_TXBD_BUF_MAX, 
        cpu_addr_ge_tx_buf, 
        phy_addr_ge_tx_buf);

    dma_free_coherent(NULL, 
        PRINCE_GE_DMA_RXBD_NUM * BYTES_PER_BD,
        cpu_addr_ge_rxbd, 
        phy_addr_ge_rxbd);

    dma_free_coherent(NULL, 
        PRINCE_GE_DMA_TXBD_NUM * BYTES_PER_BD * PRINCE_GE_DMA_TXBD_TYPE,
        cpu_addr_ge_txbd, 
        phy_addr_ge_txbd);

    dma_free_coherent(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM,
        cpu_addr_scc_rx_buf, 
        phy_addr_scc_rx_buf);

    dma_free_coherent(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM,
        cpu_addr_scc_tx_buf, 
        phy_addr_scc_tx_buf);

    dma_free_coherent(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM,
        cpu_addr_scc_rxbd, 
        phy_addr_scc_rxbd);

    dma_free_coherent(NULL, 
        PRINCE_SCC_BUF_SIZE * PRINCE_SCC_BUF_NUM,
        cpu_addr_scc_txbd, 
        phy_addr_scc_txbd);
}

/* Interrupt handler for GE DMA interrupt */
static irqreturn_t
ge_dma_isr(int irq, void *dev_id, struct pt_regs *pt)
{
    /* Clear the interrupt */
    printk("`");
    ge_dma_reg_p->ge_intr_sts |= ge_dma_reg_p->ge_intr_sts;

    return IRQ_HANDLED;
}

/* Interrupt handler for GE MAC interrupt */
static irqreturn_t
ge_mac_isr(int irq, void *dev_id, struct pt_regs *pt)
{
    /* Clear the interrupt */
    ge_mac_reg_p->intr_ctrl.intr_sts &= ~MAC_MDIO_INTR_MASK;
    ge_mac_reg_p->intr_ctrl.intr_clr |= MAC_MDIO_INTR_MASK;

    return IRQ_HANDLED;
}

/* Interrupt handler for SCC Management interrupt */
static irqreturn_t
scc_mgmt_isr(int irq, void *dev_id, struct pt_regs *pt)
{
    int channel_num = 4;
    int i;
    ushort data = 0;

    printk("$");

    /* Clear the interrupt */
    for (i = 0; i < channel_num; i++) {
        data = scc_reg_p->serial_itf[i].modem_intr_status;
    }

    return IRQ_HANDLED;
}

/* Interrupt handler for SCC NetIO interrupt */
static irqreturn_t
scc_netio_isr(int irq, void *dev_id, struct pt_regs *pt)
{
    int channel_num = 4;
    int i;

    /* Clear the interrupt */
    for (i = 0; i < channel_num; i++) {
        scc_reg_p->proto_regs_hp1[i].intr_status |= scc_reg_p->proto_regs_hp1[i].intr_status;
        scc_reg_p->proto_regs_hp2[i].intr_status |= scc_reg_p->proto_regs_hp2[i].intr_status;
        scc_reg_p->proto_regs_lp[i].intr_status |= scc_reg_p->proto_regs_lp[i].intr_status;
    }

    return IRQ_HANDLED;
}

static int mac_mdio_read (ushort offset, ushort *reg_data)
{
    int i, j;

    ge_mdio_cfg_reg_t *mdio_reg_p = &ge_mac_reg_p->mdio_cfg;

    /* Write MDIO Configuration Word 1 */
    mdio_reg_p->mdio_cfg_wd_1 = ((1 << MDIO_INIT_SHIFT) & MAC_MDIO_INIT ) |
        ((2 << MDIO_TX_OP_SHIFT) & MAC_MDIO_TX_OP ) |
        ((offset << MDIO_TX_REGAD_SHIFT) & MAC_MDIO_TX_REGAD ) |
        ((PRINCE_PHY_ADDR << MDIO_TX_PHYAD_SHIFT) & MAC_MDIO_TX_PHYAD );

    /* Polling for ready status, total wait time is 20ms */
    for (i = 0; i < MDIO_WAIT_LOOP; i++) {
        /* When the MDIO Ready is re-asserted the read data is ready to be read. */
        if ((mdio_reg_p->mdio_cfg_wd_1 & MAC_MDIO_READY)) {
            break;
        }
        for (j = 0; j < MDIO_WAIT_LOOP * 10; j++);
    }

    if (i == MDIO_WAIT_LOOP) {
        printk("Timeout waiting for MDIO read to complete.\n");
        return (1);
    }

    *reg_data = mdio_reg_p->mdio_rx_data & MAC_MDIO_RD_DATA >> MDIO_RD_DATA_SHIFT;

    return (0);
}

static int mac_mdio_write (ushort offset, ushort *reg_data)
{
    int i, j;

    ge_mdio_cfg_reg_t *mdio_reg_p = &ge_mac_reg_p->mdio_cfg;

    /* Write data to TX reg first */
    mdio_reg_p->mdio_tx_data = (*reg_data << MDIO_WR_DATA_SHIFT) & MAC_MDIO_RD_DATA;

    /* Write MDIO Configuration Word 1 */
    mdio_reg_p->mdio_cfg_wd_1 = ((1 << MDIO_INIT_SHIFT) & MAC_MDIO_INIT ) |
        ((1 << MDIO_TX_OP_SHIFT) & MAC_MDIO_TX_OP ) |
        ((offset << MDIO_TX_REGAD_SHIFT) & MAC_MDIO_TX_REGAD ) |
        ((PRINCE_PHY_ADDR << MDIO_TX_REGAD_SHIFT) & MAC_MDIO_TX_PHYAD );

    /* Polling for ready status, total wait time is 20ms */
    for (i = 0; i < MDIO_WAIT_LOOP; i++) {
        /* When the MDIO Ready is re-asserted, the transaction has completed. */
        if ((mdio_reg_p->mdio_cfg_wd_1 & MAC_MDIO_READY)) {
            break;
        }
        for (j = 0; j < MDIO_WAIT_LOOP * 10; j++);
    }

    if (i == MDIO_WAIT_LOOP) {
        printk("Timeout waiting for MDIO write to complete.\n");
        return (1);
    }

    return (0);
}

static int phy_read_reg(ushort page, ushort offset, ushort *data)
{
    int retval;

    /* Set page */
    if (mac_mdio_write( MRV88E1512_PAGE_ADDRESS_REG, &page)) {
        printk("Failed to set page %d.", page);  
        return (1);
    }

    retval = mac_mdio_read(offset, data);
    if (retval != 0) {
        printk("phy smi read failed. phy_addr = %d, "
               "page = %d, reg = %#x\n",
               PRINCE_PHY_ADDR, page, offset);
    }
    return (retval);
}

static int phy_write_reg(ushort page, ushort offset, ushort *data)
{
    int retval;

    /* Set page */
    if (mac_mdio_write(MRV88E1512_PAGE_ADDRESS_REG, &page)) {
        printk("Failed to set page %d.", page);
        return (1);
    }

    retval = mac_mdio_write(offset, data);
    if (retval != 0) {
        printk("phy smi write failed. phy_addr = %d,"
               "page = %d, reg = %#x\n",
               PRINCE_PHY_ADDR, page, offset);
    }
    return (retval);
}

/* Interrupt handler for MVL PHY interrupt */
static irqreturn_t
mvl_phy_isr(int irq, void *dev_id, struct pt_regs *pt)
{
    ushort data_copper;
    ushort data_fiber;

    printk("#");

    /* Read copper */
    phy_read_reg(MRV88E1512_REG_PAGE_3, 
                 MRV88E1512_LED_TIMER_CTRL, &data_copper);

    /* Read fiber */
    phy_read_reg(MRV88E1512_REG_PAGE_1, 
                 MRV88E1512_SPECIFIC_CONTROL2_REG, &data_fiber);

    if (data_copper & MRV88E1512_FIBER_FORCE_INT) {
        /* INTn active low */
        data_copper &= ~MRV88E1512_FIBER_FORCE_INT;
    }
    if (data_fiber & MRV88E1512_FIBER_FORCE_INT) {
        /* INTn active low */
        data_fiber &= ~MRV88E1512_FIBER_FORCE_INT;
    }
    phy_write_reg(MRV88E1512_REG_PAGE_3, 
                  MRV88E1512_LED_TIMER_CTRL, &data_copper);

    phy_write_reg(MRV88E1512_REG_PAGE_1, 
                  MRV88E1512_SPECIFIC_CONTROL2_REG, &data_fiber);


    return IRQ_HANDLED;
}

static int klm_isr_install(void)
{
    sys_csr_reg_p = (sys_csr_reg_t *)
                   ioremap_nocache(ZYNC_SYSTEM_CSR_BASE, 0x1000);

    ge_mac_reg_p = (ge_mac_reg_t *)
                   ioremap_nocache(ZYNC_SYSTEM_CSR_BASE+ZYNC_GE_MAC_CSR_OFFSET, 0x1000);

    ge_dma_reg_p = (ge_dma_reg_t *)
                   ioremap_nocache(ZYNC_SYSTEM_CSR_BASE+ZYNC_GE_DMA_CSR_OFFSET, 0x1000);

    scc_reg_p = (prince_scc_regs_t *)
                   ioremap_nocache(ZYNC_SCC_CSR_BASE, ZYNC_SCC_CSR_LENGTH);

    /* install GE MAC interrupt */
    if (request_irq(GE_MAC_INTR_ID, (void *)ge_mac_isr,
                    IRQF_DISABLED, "GE MAC ISR", NULL)) {
        printk(KERN_WARNING "unable to install GE MAC handler\n");
        return -EAGAIN;
    }

    /* install SCC Network interrupt */
    if (request_irq(SCC_NETIO_INTR_ID, (void *)scc_netio_isr,
                    IRQF_DISABLED, "SCC NETIO ISR", NULL)) {
        printk(KERN_WARNING "unable to install SCC NetIO handler\n");
        return -EAGAIN;
    }

    /* install SCC Management interrupt */
    if (request_irq(SCC_MGMT_INTR_ID, (void *)scc_mgmt_isr,
                    IRQF_DISABLED, "SCC MGMT ISR", NULL)) {
        printk(KERN_WARNING "unable to install SCC MGMT handler\n");
        return -EAGAIN;
    }
#if 0
    /* Enable interrupts on LED2 pin */
    phy_read_reg(MRV88E1512_REG_PAGE_3, 
                 MRV88E1512_LED_TIMER_CTRL, &data);

    data |= MRV88E1512_P3_R18_INT_EN;
    phy_write_reg(MRV88E1512_REG_PAGE_3, 
                  MRV88E1512_LED_TIMER_CTRL, &data);

    /* install Marvell PHY interrupt */
    if (request_irq(MVL_PHY_INTR_ID, (void *)mvl_phy_isr,
                    IRQF_DISABLED, "MVL PHY ISR", NULL)) {
        printk(KERN_WARNING "unable to install MVL PHY handler\n");
        return -EAGAIN;
    }

    disable_irq_nosync(MVL_PHY_INTR_ID);
#endif

    return 0;
}

static void klm_isr_free(void)
{
    iounmap((void *)ge_dma_reg_p);
    iounmap((void *)ge_mac_reg_p);
    iounmap((void *)scc_reg_p);

    free_irq(GE_MAC_INTR_ID, (void *)NULL);
    free_irq(SCC_MGMT_INTR_ID, (void *)NULL);
    free_irq(SCC_NETIO_INTR_ID, (void *)NULL);
}

// define which file operations are supported
struct file_operations klm_fops = {
    .owner  =   THIS_MODULE,
    .llseek =   NULL,
    .readdir=   NULL,
    .poll   =   NULL,
    .unlocked_ioctl  =   klm_ioctl,
    .mmap   =   klm_mmap,
    .open   =   klm_open,
    .flush  =   NULL,
    .release=   klm_release,
    .fsync  =   NULL,
    .fasync =   NULL,
    .lock   =   NULL,
};

// initialize module
static int __init klm_init_module (void) {

    int result;
    int err, devno, i;

    printk(KERN_ALERT "KERNAL_VERSION %d\n", KERNEL_VERSION(2,6,11));
    printk(KERN_ALERT "LINUX_VERSION_CODE %d\n\n", LINUX_VERSION_CODE);

    if (klm_isr_install()) {
        printk(KERN_NOTICE "Failed to install ISR %d\n",
               klm_major);
        return -EAGAIN;
    }

    result = alloc_chrdev_region(&dev, 0, 1, KLM_NAME);
    klm_major = MAJOR(dev);

    if (result < 0) {
        printk(KERN_WARNING "unable to get major %d\n",
               klm_major);
        return result;
    }

    for (i = 0; i < MMAP_MAX; i++) {
        devno = MKDEV(klm_major, i);
        cdev_init(&mmap_devs[i].cdev, &klm_fops);
        mmap_devs[i].cdev.owner = THIS_MODULE;
        mmap_devs[i].cdev.ops = &klm_fops;
        err = cdev_add (&mmap_devs[i].cdev, devno, 1);

        if (err) {
            printk (KERN_NOTICE "Error %d adding klm", err);
            return -EAGAIN;
        }
    }

    if (klm_dma_alloc() != 0) {
        printk(KERN_NOTICE "Failed to allocate DMA memory %d\n",
               klm_major);
        return -EAGAIN;
    }

    return 0;
}

static void __exit klm_cleanup_module (void) {

    int i;
    dev_t devno;

    klm_dma_free();

    for (i = 0; i < MMAP_MAX; i++) {
        devno = MKDEV(klm_major, i);
        cdev_del(&mmap_devs[i].cdev);
    }
    unregister_chrdev_region(dev, MMAP_MAX);

    klm_isr_free();
}

module_init(klm_init_module);
module_exit(klm_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Device Driver klm");

/******** History ********
$Log: klm_prc.c,v $
Revision 1.2  2017/07/18 08:48:45  iachang
Prince FPGA Enhanced Feature, Support HP1, HP2, and LP.

Revision 1.1  2013/11/13 06:02:05  xiaoyizh
Initial check in for Prince klm.


$Endlog$
*/

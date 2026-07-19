/* $Id: klm_p1021.c,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 **********************************************************************
 * File Name: klm_p1021.c
 *
 * Description: This KLM contains the interrupt handling and mmap() support.
 *
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/cdev.h>  /* for cdev struct */
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/of.h>
#include <asm/uaccess.h> /* for copy_from_user & copy_to_user */
#include "../apps/patriot_main.h"  /* Bitbake copy this patriot_main.h to the same dir */
#include "../apps/router_if.h"
#include "../apps/sgmii_defs.h"
#include "../apps/p1021_immap.h"
#include "../apps/p1021_etsec.h"
#include "../apps/patriot_intr.h"
#include "klm_defs.h"

//#define DEBUG    1
#define MMAP_MAX 1

#define KLM_NAME "p1021"

sm_patriot_eth_intr_iface_t intr_iface_gl;
EXPORT_SYMBOL(intr_iface_gl);

patriot_intr_dev_t *glob_intr_dev;
EXPORT_SYMBOL(glob_intr_dev);


static unsigned long ADRSPC_PQUICC_IMEMB_K;
unsigned long patriot_ccsr_base;
struct klm_dev_t {
    struct cdev cdev;
};

static dev_t dev = 0;
static struct klm_dev_t mmap_devs[MMAP_MAX];
static int klm_major = 0;



static ssize_t klm_patriot_read (struct file *file, char __user *buf,
				 size_t count, loff_t *f_pos);
static ssize_t klm_patriot_write (struct file *file, const char __user *buf,
				  size_t count, loff_t *f_pos);


static int klm_open (struct inode *inode, struct file *file)
{
    return 0;
}

static int klm_release (struct inode *inode, struct file *file) 
{
    return 0;
}

// ioctl - I/O control
static int klm_ioctl(struct inode *inode, struct file *file,
		     unsigned int cmd, unsigned long arg) 
{
    int retval;
    patriot_msg_t msg;
    patriot_msg_t *msg_p; 

    msg_p = (patriot_msg_t *) arg;

    if (copy_from_user (&msg, (void *)msg_p, sizeof(patriot_msg_t))) {
	printk("\ncopy from user failed\n");
	return -EINVAL;
    }
    
    asm volatile("msync");
    
    switch ( cmd ) {
    case IOCTL_GET_FPGA_INTR:
#ifdef DEBUG	
	printk("\n%s: cmd = %d\n", __FUNCTION__, cmd);
	printk("\nglob_intr_dev = 0x%08x", glob_intr_dev);
	printk("\nglob_intr_dev->fpga_intr_cnt =  0x%08x",
	       glob_intr_dev->fpga_intr_cnt);
#endif	
	msg.data[0] = glob_intr_dev->fpga_intr_cnt;
	if (copy_to_user((int *)msg_p, (int *)&msg, sizeof(patriot_msg_t))) {
	    return -EFAULT;
	}
	break;
    case IOCTL_CLEAR_FPGA_INTR:
	glob_intr_dev->fpga_intr_cnt = 0;
#ifdef DEBUG	
	printk("\nglob_intr_dev->fpga_intr_cnt after clear = 0x%02x",
	       glob_intr_dev->fpga_intr_cnt);
#endif	
        break;
    case IOCTL_GET_FRAMER_INTR:
#ifdef DEBUG
	printk("\n%s: cmd = %d\n", __FUNCTION__, cmd);
	printk("\nglo_intr_dev = 0x%08x", glob_intr_dev);
	printk("\nglob_intr_dev->framer_intr_cnt =  0x%08x",
	       glob_intr_dev->framer_intr_cnt);
#endif
	msg.data[0] = glob_intr_dev->framer_intr_cnt;
	msg.data[1] = glob_intr_dev->framer_intr_bit;
	if (copy_to_user((int *)msg_p, (int *)&msg, sizeof(patriot_msg_t))) {
	    return -EFAULT;
	}
	break;
    case IOCTL_CLEAR_FRAMER_INTR:
    	glob_intr_dev->framer_intr_cnt = 0;
    	glob_intr_dev->framer_intr_bit = 0;
#ifdef DEBUG
	printk("\nglob_intr_dev->framer_intr_cnt after clear = 0x%02x",
		   glob_intr_dev->framer_intr_cnt);
#endif
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
			   /*if use vm_pgoff, don't need to shift by PAGE_SHIFT*/ 
                          vma->vm_pgoff,
                          vma->vm_end-vma->vm_start,
                          vma->vm_page_prot);

    if (ret < 0) {
        printk("kernel asic_utils: remap pfn range failed.");
        return -EIO;
    }

    return 0;
}

// define which file operations are supported
struct file_operations klm_fops = {
	.owner	=	THIS_MODULE,
	.llseek	=	NULL,
	.readdir=	NULL,
	.read   =       klm_patriot_read,
        .write  =       klm_patriot_write,
	.poll	=	NULL,
	.ioctl	=	klm_ioctl,
        .mmap   =       klm_mmap,
	.open	=	klm_open,
	.flush	=	NULL,
	.release=	klm_release,
	.fsync	=	NULL,
	.fasync	=	NULL,
	.lock	=	NULL,
};



/***********************************************************************
 * Name: etsec_ck_ievent
 *
 * Description: Will check ievent for pending status, save
 *    info, then clear pending bit
 *    We may enter this function as a result of an interrupt
 *    or a call from a poll routine. 
 *
 *    Transmit interrupts are set whenever TXB or TXF is set;
 *    to clear this hardware interrupt, must clear both bits.
 *    Receive interrupts are set whenever RXB or RXF is set;
 *    to clear this hardware interrupt, must clear both bits.
 *    Error and diagnostic interrupts are set whenever bits
 *    MAG, GTSC, GRSC, TXC, RXC, BABR, BABT, LC, CRL, FIR, FIQ,
 *    DPE, PERR, EBERR, TXE, XFUN, BSY are set.  Must clear
 *    all of these bits to clear a hardware/diagnostic interrupt.
 *
 * Input: etsec_num
 *        mode
 *
 * Output: none
 *
 ***********************************************************************
 */
irqreturn_t etsec2_intr_hndlr_test (void)
{

    int tsec_ev, dummy;
    volatile ccsr_tsec_t *tsec_reg;
    int etsec_num = ETSEC2;

    ADRSPC_PQUICC_IMEMB_K = (unsigned long)patriot_ccsr_base;

    tsec_reg = (volatile ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB_K +
					ETSEC2_GROUP0_OFFSET);

    tsec_ev = tsec_reg->ievent;

#ifdef DEBUG
	printk("\nIn eTSEC%d_intr_hndlr(), ievent @%#.8x = %#.8x\n",
	       etsec_num, &tsec_reg->ievent, tsec_ev);
#endif        

    if (tsec_ev & TSEC_IEVENT_TXF) {
        tsec_reg->ievent = TSEC_IEVENT_TXF | TSEC_IEVENT_TXB;
	intr_iface_gl.eth_tx_intr_cnt++;
    }
#ifdef DEBUG
    printk("\n after TSEC_IEVENT_TXF\n");
#endif   
    if (tsec_ev & TSEC_IEVENT_RXFO) {
        tsec_reg->ievent = TSEC_IEVENT_RXFO | TSEC_IEVENT_RXBO;
	    intr_iface_gl.eth_rx_intr_cnt++;
    }
#ifdef DEBUG    
    printk("\n after TSEC_IEVENT_RXFO, intr_iface_gl.eth_rx_intr_cnt = %d\n",
	   intr_iface_gl.eth_rx_intr_cnt);
#endif    
    if (tsec_ev & TSEC_ERR_IEVENTS) {
        tsec_reg->ievent = (tsec_ev & TSEC_ERR_IEVENTS);
        if (tsec_ev & (TSEC_IEVENT_RXC | TSEC_IEVENT_TXC)) {
            dummy = tsec_reg->tctrl;
            return IRQ_HANDLED;
        }
        
	    intr_iface_gl.eth_rx_intr_cnt++;
        printk("\n*** Unexpected eTSEC%d event status, "
               "ievent @%#.8x=%#.8x ***\n",
               etsec_num, (unsigned int)&tsec_reg->ievent, tsec_ev);
    }

#ifdef DEBUG_INTR
    /* used when etsec1 interrupt was being handled by etsec2
     * interrupt handler.  Problem turned out to be that when
     * the processor internal interrupts was being programmed,
     * IIVPR13 was initialized to a wrong vector (22)
     */
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(1, ETSEC_GROUP0);
    printf("\neTSEC1 ievent @%#.8x = %#.8x, imask %#.8x\n",
           &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    tsec_ev = tsec_reg->ievent;
    if (tsec_ev) {
        tsec_reg->ievent = TSEC_IEVENTS;
        printf("eTSEC1 ievent @%#.8x = %#.8x, imask %#.8x\n",
               &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    }
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(2, ETSEC_GROUP0);
    printf("eTSEC2 ievent @%#.8x = %#.8x, imask %#.8x\n",
           &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    tsec_ev = tsec_reg->ievent;
    if (tsec_ev) {
        tsec_reg->ievent = TSEC_IEVENTS;
        printf("eTSEC2 ievent @%#.8x = %#.8x, imask %#.8x\n",
               &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    }
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(3, ETSEC_GROUP0);
    printf("eTSEC3 ievent @%#.8x = %#.8x, imask %#.8x\n",
           &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    tsec_ev = tsec_reg->ievent;
    if (tsec_ev) {
        tsec_reg->ievent = TSEC_IEVENTS;
        printf("eTSEC3 ievent @%#.8x = %#.8x, imask %#.8x\n",
               &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    }
#endif
    
    return IRQ_HANDLED;
}


/***********************************************************************
 * Name: hdlc_intr_hndlr
 *
 * Description: HDLC interrupt handler
 *
 * Input: none
 *
 * Output: none
 *
 ***********************************************************************
 */
void hdlc_intr_hndlr (void)
{

    unsigned int *ucce_p;

    ucce_p = (unsigned int *)&((immap_t *)ADRSPC_PQUICC_IMEMB_K)->qe.ucc3.mode.fast.reg.ucce;

    if ((*ucce_p & RXF) != 0) {
	hdlc_rx_frames++;
	/* Clear RXF flag to allow a new event to be detected */
	*ucce_p = RXF;	
    }
    return;
        
}


// initialize module
static int __init klm_init_module (void) {

    int result;
    int err, devno, i;
    patriot_intr_dev_t *intr_dev;
    
    printk(KERN_ALERT "KERNEL_VERSION %d\n", KERNEL_VERSION(2,6,32));
    printk(KERN_ALERT "LINUX_VERSION_CODE %d\n\n", LINUX_VERSION_CODE);

    patriot_ccsr_base = (unsigned long)ioremap(0xFFE00000, 0x100000);
    printk("\npatriot_ccsr_base = 0x%08lx", patriot_ccsr_base);

    intr_dev = kzalloc(sizeof(patriot_intr_dev_t), GFP_KERNEL);
    if (!intr_dev) {
	return -ENOMEM;
    }

    intr_dev->fpga_get_intr_cnt = NULL;
    intr_dev->fpga_clear_intr_cnt = NULL;
    intr_dev->framer_get_intr_cnt = NULL;
    intr_dev->framer_clear_intr_cnt = NULL;

    glob_intr_dev = intr_dev;
    /* install ETSEC2 TX interrupt */
    if (request_irq(INTERNAL_IRQ_BASE + ETSEC2_GROUP0_TX_INTR,
		    (void *)etsec2_intr_hndlr_test,
		    IRQF_DISABLED, "ETSEC2 INT", NULL)) {
	printk(KERN_WARNING "unable to install ETSEC2 TX intr handler\n");
	return -EAGAIN;
    }

    /* install ETSEC2 RX interrupt */
    if (request_irq(INTERNAL_IRQ_BASE + ETSEC2_GROUP0_RX_INTR,
		    (void *)etsec2_intr_hndlr_test,
		    IRQF_DISABLED, "ETSEC2 INT", NULL)) {
	printk(KERN_WARNING "unable to install ETSEC2 RX intr handler\n");
	return -EAGAIN;
    }

    /* install ETSEC2 ERROR interrupt */
    if (request_irq(INTERNAL_IRQ_BASE + ETSEC2_GROUP0_ERR_INTR,
		    (void *)etsec2_intr_hndlr_test,
		    IRQF_DISABLED, "ETSEC2 INT", NULL)) {
	printk(KERN_WARNING "unable to install ETSEC2 Error intr handler\n");
	return -EAGAIN;
    }
    
    result = alloc_chrdev_region(&dev, 0, 1, KLM_NAME);
    klm_major = MAJOR(dev);
        
    if (result < 0) {
        printk(KERN_WARNING "unable to get major %d\n",
               klm_major);
        return -EAGAIN;
    }

    for (i=0;i<MMAP_MAX;i++) {
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

    return 0;
}

// read function for userspace
static ssize_t klm_patriot_read (struct file *file, char __user *buf,
				 size_t count, loff_t *f_pos)
{
    sm_patriot_eth_intr_iface_t intr_iface;

    if (copy_from_user(&intr_iface, (int *)buf, count))
        return -EFAULT;

    if (copy_to_user((int *)buf, (int *)&intr_iface_gl, count))
        return -EFAULT;

    return count;
}


// write function for userspace
static ssize_t klm_patriot_write (struct file *file, const char __user *buf,
				  size_t count, loff_t *f_pos)
{
    sm_patriot_eth_intr_iface_t intr_iface;

    if  (copy_from_user(&intr_iface, (int *)buf, count))
        return -EFAULT;

    memcpy(&intr_iface_gl, &intr_iface, count);
        
    return count;

}

static void __exit klm_cleanup_module (void) {

    int i;
    dev_t devno;

    for (i=0;i<MMAP_MAX;i++) {
        devno = MKDEV(klm_major, i);
        cdev_del(&mmap_devs[i].cdev);
    }
    unregister_chrdev_region(dev, MMAP_MAX);

    free_irq(INTERNAL_IRQ_BASE + ETSEC2_GROUP0_TX_INTR, (void *)NULL);
    free_irq(INTERNAL_IRQ_BASE + ETSEC2_GROUP0_RX_INTR, (void *)NULL);
    free_irq(INTERNAL_IRQ_BASE + ETSEC2_GROUP0_ERR_INTR, (void *)NULL);
    free_irq(INTERNAL_IRQ_BASE + QE_LOW_INTR, (void *)NULL);
    iounmap((volatile void *)patriot_ccsr_base);
    kfree(glob_intr_dev);
}

module_init(klm_init_module);
module_exit(klm_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Device Driver klm");

/******** History ********
$Log: klm_p1021.c,v $
Revision 1.1  2014/03/25 02:12:43  huanngo
Adding patriot_linux directory to ng_diag code tree

Revision 1.3  2014/03/06 01:56:52  steja
1. added cli command for margining patriot voltage
2. enhance framer interrupt and ecc memory test timing

Revision 1.2  2012/05/08 23:52:56  huanngo
Support SM Patriot on ngd main code tree

Revision 1.1.4.8  2012/04/12 18:37:02  huanngo
Clean up and cosmetic changes

Revision 1.1.4.7  2012/03/27 07:45:06  steja
Fix Warning compilation

Revision 1.1.4.6  2012/03/13 13:24:39  steja
Supoort Framer Interrupt

Revision 1.1.4.5  2012/02/06 22:30:16  huanngo
Update to not use bitbake to compile, use make with local kernel

Revision 1.1.4.4  2012/01/09 23:06:19  huanngo
Support on xformers mips and informers and clean up

Revision 1.1.4.3  2011/12/21 23:46:32  huanngo
Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure

Revision 1.1.4.2  2011/08/18 19:43:27  huanngo
Update code to patriot2-branch

Revision 1.1.2.5  2011/08/06 00:17:41  huanngo
Update code for Patriot

Revision 1.1.2.4  2011/07/19 06:11:35  huanngo
Update code per code review comments

Revision 1.1.2.3  2011/07/08 00:08:56  huanngo
Clean up code

Revision 1.1.2.2  2011/06/28 06:27:56  huanngo
Update code to support Patriot SM

Revision 1.1.2.1  2011/05/02 23:33:35  huanngo
Update code to support Patriot module side

$Endlog$
*/




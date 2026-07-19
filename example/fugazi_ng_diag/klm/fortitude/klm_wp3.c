/* $Id: klm_wp3.c,v 1.2 2012/03/28 00:38:26 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/fortitude/klm_wp3.c,v $
 ***********************************************************************
 * File Name: klm_mp3.c
 *
 * Description: This KLM contains the interrupt handling and mmap() support.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c)2011 - 2012 by Cisco Systems, Inc.
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

#include "../../common/src/fortitude/pcmap.h"
#include "../../common/src/fortitude/fortitude_fpga.h"

#define MMAP_MAX 1
#define BLK_SIZE 0
#define KLM_NAME "wp3"

struct klm_dev_t {
    struct cdev cdev;
};

static dev_t dev = 0;
static struct klm_dev_t mmap_devs[MMAP_MAX];
static int klm_major = 0;

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
    
    switch ( cmd ) {
    default:
            retval = -EINVAL;
            break;
                    
    }
    return retval;
}

static int klm_mmap(struct file * filp, struct vm_area_struct * vma)
{
    int ret;

#ifdef DEBUG
    unsigned int virt_addr;

    virt_addr = (unsigned long)ioremap_nocache(NPU_RIF_BASE, 0x10000);
    printk("vm_pgoff = %#x\n", vma->vm_pgoff);
    printk("virt_addr = %#x, value= %#x\n", virt_addr, *(unsigned int *)(virt_addr+0xc2dc));
#endif

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    vma->vm_flags |= VM_RESERVED;

    ret = remap_pfn_range(vma,
                          vma->vm_start,
                          //NPU_RIF_BASE >> PAGE_SHIFT, /* if we use hard code value, need to shift */
                          vma->vm_pgoff, /*if use vm_pgoff, don't need to shift by PAGE_SHIFT */ 
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
	.poll	=	NULL,
	.ioctl	=	klm_ioctl,
        .mmap   =       klm_mmap,
	.open	=	klm_open,
	.flush	=	NULL,
	.release=	klm_release,
	.fsync	=	NULL,
	.fasync	=	NULL,
	.lock	=	NULL,
        //	.readv	=	NULL,
        //	.writev	=	NULL,
};

/* interrupt handler for NPU INT1 interrupt */
static irqreturn_t
npu_int1_isr(int irq, void *dev_id, struct pt_regs *pt)
{
    fpga_reg_t *fpga_reg;
    unsigned char temp;

    /* the minimum map size is 4KB (page size) */
    fpga_reg = (fpga_reg_t *)ioremap_nocache(FPGA_BASE+FPGA_GENERAL_REG_BASE, 0x1000);

    /* after the test, clear the interrupt from FPGA to NPU */
    temp = fpga_reg->fpga_int_event;
#ifndef DEBUG
    printk("\nfpga_reg @ %#x, fpga_int_event = %#x, fpga_int_diag_test =%#x\n", 
     (unsigned int)fpga_reg, fpga_reg->fpga_int_event, fpga_reg->fpga_int_diag_test);
#endif

    fpga_reg->fpga_int_event = temp; 

    iounmap(fpga_reg);

    return IRQ_HANDLED;
}


// initialize module
static int __init klm_init_module (void) {

    int result;
    int err, devno, i;

    printk(KERN_ALERT "KERNAL_VERSION %d\n", KERNEL_VERSION(2,6,11));
    printk(KERN_ALERT "LINUX_VERSION_CODE %d\n\n", LINUX_VERSION_CODE);

    /* install NPU INT1 interrupt */
    if (request_irq(WINPATH_IRQ_BASE + NPU_INT1, (void *)npu_int1_isr,
		    IRQF_DISABLED, "NPU INT1 ISR", NULL)) {
	printk(KERN_WARNING "unable to install NPU INT1 handler\n");
	return -EAGAIN;
    }

    result = alloc_chrdev_region(&dev, 0, 1, KLM_NAME);
    klm_major = MAJOR(dev);
        
    if (result < 0) {
        printk(KERN_WARNING "unable to get major %d\n",
               klm_major);
        return result;
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
    /*
      mknod /dev/wp3 major 0
      wp3 is open for device with minor 0
    */
    return 0;
}


static void __exit klm_cleanup_module (void) {

    int i;
    dev_t devno;

    for (i=0;i<MMAP_MAX;i++) {
        devno = MKDEV(klm_major, i);
        cdev_del(&mmap_devs[i].cdev);
    }
    unregister_chrdev_region(dev, MMAP_MAX);

    free_irq(WINPATH_IRQ_BASE + NPU_INT1, (void *)NULL);    
}

module_init(klm_init_module);
module_exit(klm_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Device Driver klm");

/******** History ********
$Log: klm_wp3.c,v $
Revision 1.2  2012/03/28 00:38:26  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:11  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: addr_vtop.c,v 1.3 2015/11/06 01:51:03 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/addr_vtop.c,v $
 *-----------------------------------------------------------------------------
 * File: addr_vtop.c  Walk the page table. Convert given userspace
 *                    virtual address to physical address
 *
 * August 2010, naorlin
 *
 * Copyright (c) 2011-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>         //struct page
#include <asm/pgtable.h>      //pagetable structs & walking functs
#include <linux/fs.h>         //module stuff, must go before cdev
#include <linux/cdev.h>       //cdev struct
#include <linux/sched.h>      //task_struct
#include <linux/device.h>     //class_create to dynmically create /dev/node
#ifdef CVMX_SDK_1_7
#include <asm-mips/current.h> //get_current()
#else // SDK 2.0 need these
#if defined (X86) || defined (ARM)
#include <linux/uaccess.h>  /*copy_to_user */
#include <asm/pgalloc.h>    /* pte_ */
#include <asm/io.h>         /* page_to_phys */
#else
#include <asm/current.h>      //get_current()
#endif /*x86 or ARM*/

#endif 
//#include <linux/uaccess.h>


/*
 * invalid_pmd_table is used in pud_none. Need to define this dummy to compile.
 */
#define __page_aligned(order) __attribute__((__aligned__(PAGE_SIZE<<order)))
#if !defined (X86) && !defined (ARM)
pmd_t invalid_pmd_table[PTRS_PER_PMD] __page_aligned(PMD_ORDER);
#endif

static struct cdev vtop_dev;
static dev_t devno;
static struct class *dev_class;
static struct device *dev_vtop;
static int major;

/*
 * User virtual address is passed in buf. This function finds the
 * page frame number of the physical page this virtual address is
 * mapped to and returned back in buf to the user program.
 * Note: This driver deals with a kernel with 4 level page table which
 * has the pud. A three level page table kernel does not have pud.
 */
static ssize_t 
addr_vtop_read(struct file *file, char __user *buf, size_t count,
	  loff_t *fpos)
{
    unsigned long user_virt_addr = 0;
    unsigned long phys_addr;	
	
    if (!copy_from_user(&user_virt_addr, buf, sizeof(unsigned long))) {
        //walk the page table to match given virt address with a phys one
        struct mm_struct *mm = current->mm;

	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	struct page *page;

	pgd = pgd_offset(mm, user_virt_addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) 
	    goto out;
		
	pud = pud_offset(pgd, user_virt_addr);  
	if (pud_none(*pud) || pud_bad(*pud))
	    goto out;	
      
	pmd = pmd_offset(pud, user_virt_addr);  
	if (pmd_none(*pmd) || pmd_bad(*pmd))
	    goto out;
#if defined (X86) || defined (ARM)
        pte = pte_offset_kernel(pmd, user_virt_addr);
#else
	pte = pte_offset(pmd, user_virt_addr);
#endif
	if (!pte)
	   goto out;
		
	page = pte_page(*pte); 
		
	/* This is the physical address of the page which is specified
	 * by PAGE_SIZE (usually is 4K). This address is passed back
	 * out via buf.
	 */
	phys_addr = (unsigned long)page_to_phys(page);	

	if (copy_to_user(buf, &phys_addr, sizeof(unsigned long))) {
	    printk(KERN_ALERT "addr_vtop.ko: copy_to_user failed.\n");
	    return -EFAULT;
	}

	return count;

out:
        printk(KERN_ALERT "addr_vtop.ko: page tables bad or not found.\n");
	return -EFAULT;
    }
    else {
        printk(KERN_ALERT "addr_vtop.ko: copy_from_user failed.\n");
	return -EFAULT;
    }
}

// open function - called when the "file" /dev/addr_vtop is open in userspace  
static int
addr_vtop_open (struct inode *inode, struct file *file)
{
    return 0;
}

// close function - called when the "file" /dev/addr_vtop is closed in userspace  
static int 
addr_vtop_release (struct inode *inode, struct file *file)
{
    return 0;
}

// define which file operations are supported
struct file_operations addr_vtop_fops = {
    .owner   = THIS_MODULE,
    .open    = addr_vtop_open,
    .write   = NULL,
    .read    = addr_vtop_read,
    .release = addr_vtop_release,
};       

// initialize module
static int __init addr_vtop_init_module (void) 
{
    int result;
    int err;
    void *ptr_err;
       
    result = alloc_chrdev_region(&devno, 0, 1, "addr_vtop");
    if (result < 0) {
        printk(KERN_WARNING "unable to get device number\n");
	return result;
    }

    cdev_init(&vtop_dev, &addr_vtop_fops);
    vtop_dev.owner = THIS_MODULE;
    vtop_dev.ops = &addr_vtop_fops;
    err = cdev_add (&vtop_dev, devno, 1);

    /* Fail gracefully if need be */
    if (err) {
        printk (KERN_ALERT "Error %d adding addr_vtop\n", err);
	return -EAGAIN;
    }
    major = MAJOR(devno);
    //    printk(KERN_NOTICE "\n%s() mknod /dev/addr_vtop c %d 0\n\n", __FUNCTION__,MAJOR(devno));
    dev_class = class_create(THIS_MODULE, "addr_vtop");
    if (IS_ERR(ptr_err = dev_class))
        goto err2;
    
    dev_vtop = device_create(dev_class, NULL, MKDEV(major, 0), NULL, "addr_vtop");
    if (IS_ERR(ptr_err = dev_vtop))
        goto err;
    
    return 0;
 err:
    class_destroy(dev_class);
 err2:
    cdev_del (&vtop_dev);
    unregister_chrdev_region((devno), 1);
    return  PTR_ERR(ptr_err);


}

// close and cleanup module
static void __exit addr_vtop_cleanup_module (void)
{
    //    printk(KERN_NOTICE "\n%s() called.\n\n", __FUNCTION__);
    major = MAJOR(devno);
    device_destroy(dev_class, MKDEV(major,0));
    class_destroy(dev_class);
    
    cdev_del (&vtop_dev);
    unregister_chrdev_region(devno, 1);
}

module_init(addr_vtop_init_module);
module_exit(addr_vtop_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Userspace virt_to_phys");

/******** History ******** 
$Log: addr_vtop.c,v $
Revision 1.3  2015/11/06 01:51:03  xiaoyizh
Add ARM support.

Revision 1.2  2012/03/28 00:38:25  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

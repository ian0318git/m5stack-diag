/* $Id: addr_vtop.c,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: addr_vtop.c
 *
 * Description: Walk the page table. Convert given userspace
 *                    virtual address to physical address
 *
 * Port from Vindicator   
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>         //struct page
#include <asm/pgtable.h>      //pagetable structs & walking functs
#include <linux/fs.h>         //module stuff, must go before cdev
#include <linux/cdev.h>       //cdev struct
#include <linux/sched.h>      //task_struct
#include <linux/uaccess.h>  /*copy_to_user */
#include <asm/pgalloc.h>    /* pte_ */
#include <asm/io.h>         /* page_to_phys */

/*
 * invalid_pmd_table is used in pud_none. Need to define this dummy to compile.
 */
#define __page_aligned(order) __attribute__((__aligned__(PAGE_SIZE<<order)))


static struct cdev vtop_dev;
static dev_t devno;

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

        pte = pte_offset_kernel(pmd, user_virt_addr);

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
    
    printk(KERN_NOTICE "\n%s() mknod /dev/addr_vtop c %d 0\n\n", __FUNCTION__,MAJOR(devno));
    return 0;
}

// close and cleanup module
static void __exit addr_vtop_cleanup_module (void)
{
    printk(KERN_NOTICE "\n%s() called.\n\n", __FUNCTION__);
    cdev_del (&vtop_dev);
    unregister_chrdev_region(devno, 1);
}

module_init(addr_vtop_init_module);
module_exit(addr_vtop_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Userspace virt_to_phys");

/******** History ********/ 
/*------------------------------------------------------------------------------
 * $Log: addr_vtop.c,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.3  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.2  2011/08/18 19:43:27  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.3  2011/07/19 06:11:35  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.2  2011/07/08 00:08:56  huanngo
 * Clean up code
 *
 * Revision 1.1.2.1  2011/05/21 01:01:49  huanngo
 * Support memory test, I2C interface and Montavista Linux
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

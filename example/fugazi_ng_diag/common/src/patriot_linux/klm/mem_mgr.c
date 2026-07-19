/* $Id: mem_mgr.c,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: mem_mgr.c
 *
 * Description: memory management klm
 *
 *      
 * Original author mcharon
 * Copyright (c)2011-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/cdev.h>  /* for cdev struct */
#include <linux/ioctl.h>
#include <linux/uaccess.h>  /*copy_to_user */
#include <asm/io.h>  /*virt_to_phys */
#include "klm_defs.h"
#include "mem_mgr.h"

//#define MEM_DEBUG
#define MEM_MGR_NAME "mem_mgr"
static mem_info_t mem_list[MAX_LIST_SIZE];
static struct cdev mem_devs;
static int mem_mgr_major = 0;

static void free_mem_list(void)
{
    int i;
    for (i=0;i<MAX_LIST_SIZE;i++) {
        if (mem_list[i].in_use) {
            mem_list[i].in_use = 0;
#ifdef MEM_DEBUG
            printk("freeing mem entry%d @%p; size %#x\n", i,
                   (void *)mem_list[i].kernel_virt_addr, mem_list[i].size);
#endif
            if (mem_list[i].size > MEM_MGR_MAX_KMALLOC_SIZE) {
                printk("mem_mgr_drv: free memory whose size > 128K currently");
		printk(" not supported\n");
            } else {
                kfree((void *)mem_list[i].kernel_virt_addr);
            }
            mem_list[i].kernel_virt_addr = 0;
                        
        }
    }
}

static int mem_mgr_open (struct inode *inode, struct file *file)
{
    return 0;
}

// close function - called when the "file" /dev/mem_mgr is closed in userspace  
static int mem_mgr_release (struct inode *inode, struct file *file)
{
    free_mem_list();
    return 0;
}

/* called by user space free_nm */
static ssize_t mem_mgr_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *fpos)
{
    mem_info_t mem_info;
    int i;

    if (copy_from_user(&mem_info, (int *)buf, sizeof(mem_info_t))) {
            printk("unable to get memory address to free\n");
            return -EFAULT;
        } 
        if (mem_info.kernel_virt_addr) {
            for (i=0;i<MAX_LIST_SIZE;i++) {
                if (mem_list[i].kernel_virt_addr ==
                    mem_info.kernel_virt_addr) {
#ifdef MEM_DEBUG
                    printk("freeing entry %d @%p; size = %d\n",
                           i, (void *)mem_info.kernel_virt_addr, mem_info.size);
#endif
                    mem_list[i].in_use = 0;
                    if (mem_info.size > MEM_MGR_MAX_KMALLOC_SIZE) {
                        printk("mem_mgr_drv: free memory whose size > 128K");
			printk(" currently not supported\n");
                    } else {
                        kfree((void *)mem_info.kernel_virt_addr);
                    }
                    mem_list[i].kernel_virt_addr = 0;
                   break;
                }
            }
        } else {
            printk("unable to free memory..\n");
            return -EFAULT;
        }

    return count;
}

/* called by user space malloc_nm */
static ssize_t mem_mgr_read(struct file *file, char __user *buf, size_t count,
			    loff_t *fpos)
{
    mem_info_t mem_info;
    int i;
#ifdef MEM_DEBUG    
    printk("\nmem_mgr.c sizeof(mem_info_t) = %d", sizeof(mem_info_t));
#endif    
    if (copy_from_user(&mem_info, (int *)buf, sizeof(mem_info_t)))
        return -EFAULT;
#ifdef MEM_DEBUG    
    printk("\nmem_mgr.c mem_info.size = %d", mem_info.size);
#endif    
    if (mem_info.size > MEM_MGR_MAX_KMALLOC_SIZE) {
	printk("mem_mgr.c currently not supporting allocating memory ");
	printk("whose size > 128K\n");
        } else {
	mem_info.kernel_virt_addr = (unsigned long)kmalloc(mem_info.size,
							   GFP_KERNEL);
	mem_info.phy_addr = (unsigned long)
	    (virt_to_phys((void *)mem_info.kernel_virt_addr));
#ifdef MEM_DEBUG	    
	printk("\nphy_addr = 0x%08x, kernel_virt_addr = 0x%08x,mem size = 0x%08x",
		mem_info.phy_addr, mem_info.kernel_virt_addr, mem_info.size);
#endif	    
        }

        if (!mem_info.kernel_virt_addr) {
            printk(KERN_ALERT "inside mem_mgr.ko:  unable to kmalloc\n");
            return -EFAULT;
        }

        memset((void *)mem_info.kernel_virt_addr, 0, mem_info.size);

        if (copy_to_user((int *)buf, &mem_info, sizeof(mem_info_t)))
            return -EFAULT;
        
        for (i=0;i<MAX_LIST_SIZE;i++) {
            if (!mem_list[i].in_use) {
                mem_list[i].kernel_virt_addr =  mem_info.kernel_virt_addr;
                mem_list[i].in_use = 1;
                mem_list[i].size = mem_info.size;
#ifdef MEM_DEBUG
                printk("allocating mem entry %d @%p\n", i,
		       (void *)mem_info.kernel_virt_addr);
#endif
                break;
            }
        }

        return count;
}

// define which file operations are supported
struct file_operations mem_mgr_fops = {
	.owner	=	THIS_MODULE,
	.llseek	=	NULL,
	.poll	=	NULL,
	.open	=	mem_mgr_open,
	.flush	=	NULL,
        .write  =       mem_mgr_write,
        .read   =       mem_mgr_read,
	.release=	mem_mgr_release,
	.fsync	=	NULL,
	.fasync	=	NULL,
	.lock	=	NULL,
};

// initialize module
static int __init mem_mgr_init_module (void) {

        int result;
        dev_t dev = 0;
        int err, devno, i;
        
        result = alloc_chrdev_region(&dev, 0, 1, MEM_MGR_NAME);
        mem_mgr_major = MAJOR(dev);
        
	if (result < 0) {
            printk(KERN_WARNING "unable to get major %d\n",
                   mem_mgr_major);
            return result;
	}

        devno = MKDEV(mem_mgr_major, 0);

        cdev_init(&mem_devs, &mem_mgr_fops);
	mem_devs.owner = THIS_MODULE;
	mem_devs.ops = &mem_mgr_fops;
	err = cdev_add (&mem_devs, devno, 1);

	/* Fail gracefully if need be */
	if (err) {
		printk (KERN_NOTICE "Error %d adding mem_mgr", err);
                return -EAGAIN;
        }

        printk(KERN_ALERT "\n mknod /dev/mem_mgr c %d 0\n\n",
               mem_mgr_major);

        for (i=0;i<MAX_LIST_SIZE;i++) {
            mem_list[i].in_use = 0;
            mem_list[i].kernel_virt_addr = 0;
        }

	return 0;
}

// close and cleanup module
static void __exit mem_mgr_cleanup_module (void) {

    int scull_minor = 0;
    dev_t devno = MKDEV(mem_mgr_major, scull_minor);

    cdev_del (&mem_devs);
    unregister_chrdev_region(devno, 1);

}

module_init(mem_mgr_init_module);
module_exit(mem_mgr_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Device Driver Template with MMAP");

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: mem_mgr.c,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2012/09/06 00:00:16  huanngo
 * After malloc, memset the menory to 0
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.6  2012/04/12 18:37:03  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.5  2012/03/27 07:45:06  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.4  2011/11/24 00:40:02  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.3  2011/10/07 01:12:09  huanngo
 * Remove unwanted printing
 *
 * Revision 1.1.4.2  2011/08/18 19:43:28  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.2  2011/08/06 00:17:41  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.1  2011/07/21 20:06:27  huanngo
 * Add support for mem_mgr.ko
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

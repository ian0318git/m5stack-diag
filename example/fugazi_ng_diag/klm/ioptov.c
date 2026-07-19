/* $Id: ioptov.c,v 1.2 2012/03/28 00:38:25 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/ioptov.c,v $
 *-----------------------------------------------------------------------------
 * File: ioptov.c  Convert IO physical address to userspace virtual address.
 *                 Physical address must be aligned with Linux PAGE_SIZE.
 *
 * August 2010, Paul Tong
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>    // must go before cdev.h
#include <linux/cdev.h>

static struct cdev ioptov_dev;
static dev_t devno;
static struct class *dev_class;
static struct device *dev_ptov;
static int major;

static int
ioptov_mmap(struct file * filp, struct vm_area_struct * vma)
{
    int ret;
    //    printk("vm_start = %p vm_pgoff= %p\n", vma->vm_start, vma->vm_pgoff);
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    vma->vm_flags |= VM_RESERVED;
    
    ret = remap_pfn_range(vma,
			  vma->vm_start,
			  vma->vm_pgoff,
			  (vma->vm_end - vma->vm_start),
			  vma->vm_page_prot);
    if (ret < 0) {
        printk(KERN_WARNING "ioptov klm remap pfn range failed.");
	return -EIO;
    }
		
    return 0;
}

static int ioptov_open (struct inode *inode, struct file *file)
{
    return 0;
}

static int ioptov_release (struct inode *inode, struct file *file)
{
    return 0;
}

// define which file operations are supported
struct file_operations ioptov_fops = {
    .owner	=	THIS_MODULE,
    .open	=	ioptov_open,
    .mmap       =       ioptov_mmap,
    .release    =	ioptov_release,
};

static int __init ioptov_init_module (void)
{
    int result;
    int err;
       
    result = alloc_chrdev_region(&devno, 0, 1, "ioptov");
    if (result < 0) {
        printk(KERN_WARNING "ioptov klm unable to get device number\n");
	return result;
    }

    cdev_init(&ioptov_dev, &ioptov_fops);
    ioptov_dev.owner = THIS_MODULE;
    ioptov_dev.ops = &ioptov_fops;
    err = cdev_add (&ioptov_dev, devno, 1);

    if (err) {
        printk (KERN_WARNING "Error %d adding ioptov\n", err);
	return -EAGAIN;
    }
    
    //    printk(KERN_NOTICE "\n%s() mknod /dev/ioptov c %d 0\n\n", __FUNCTION__,MAJOR(devno));
    dev_class = class_create(THIS_MODULE, "ioptov");
    if (IS_ERR(ptr_err = dev_class))
        goto err2;
    
    dev_ptv = device_create(dev_class, NULL, MKDEV(major, 0), NULL, "ioptov");
    if (IS_ERR(ptr_err = dev_ptv))
        goto err;
    
    return 0;
 err:
    class_destroy(dev_class);
 err2:
    cdev_del (&ioptov_dev);
    unregister_chrdev_region((devno), 1);
    return  PTR_ERR(ptr_err);

}

// close and cleanup module
static void __exit ioptov_cleanup_module (void) {
    //    printk ("\n%s()\n", __FUNCTION__);
    int major;
    major = MAJOR(devno);
    class_destroy(dev_class, MKDEV(major,0));
    class_destroy(dev_class);
    
    cdev_del (&ioptov_dev);
    unregister_chrdev_region(devno, 1);
}

module_init(ioptov_init_module);
module_exit(ioptov_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IO physical address to user virtual address mapping");

/******** History ******** 
$Log: ioptov.c,v $
Revision 1.2  2012/03/28 00:38:25  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

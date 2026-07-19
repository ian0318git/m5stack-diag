/* $Id: diag_timer.c,v 1.2 2012/03/28 00:38:25 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/diag_timer.c,v $
 *-----------------------------------------------------------------------------
 * File: diag_timer.c
 *
 * March. 2008, mcharon
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 * example code on how to dynamically create device node
 * http://www.linuxquestions.org/questions/linux-newbie-8/create-a-device-file-using-udev-738448/
 *
 *-----------------------------------------------------------------------------
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>  /* for cdev struct */
#include <linux/uaccess.h>  /*copy_to_user */

#include "klm_defs.h"

#define KLM_DIAG_TIMER_NAME "diag_timer"
#define MAX_DIAGTIMER 1
struct diag_timer_dev_t {
    struct cdev cdev;
};
static struct class *dev_class;
static struct device *dev_timer;
static int major;
static dev_t dev = 0;
static struct diag_timer_dev_t diag_timer_devs[MAX_DIAGTIMER];;

static ssize_t diag_timer_write (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    unsigned int delay;

    if  (copy_from_user(&delay, (int *)buf, sizeof(delay)))
        return -EFAULT;

    if (delay < (MAX_UDELAY_MS * 1000)) {
        udelay(delay);
    } else {
        delay = delay/1000;
        mdelay(delay);
    }
    return count;
}
static ssize_t diag_timer_read (struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    barrier();
    mb();
    return count;
}

static int diag_timer_open (struct inode *inode, struct file *file)
{
    return 0;
}

static int diag_timer_release (struct inode *inode, struct file *file)
{
    return 0;
}

// define which file operations are supported
struct file_operations diag_timer_fops = {
	.owner	=	THIS_MODULE,
	.llseek	=	NULL,
	.read   =	diag_timer_read,
        .write   =	diag_timer_write,
	.poll	=	NULL,
	.open	=	diag_timer_open,
	.flush	=	NULL,
	.release=	diag_timer_release,
	.fsync	=	NULL,
	.fasync	=	NULL,
	.lock	=	NULL,
};

// initialize module
static int __init diag_timer_init_module (void) {

    int result, devno;
    int err, i;
    void *ptr_err;
    
    result = alloc_chrdev_region(&dev,  /* get device number containing major/minor info */
                                 0 /*firstminor */,
                                 MAX_DIAGTIMER /*count */,
                                 KLM_DIAG_TIMER_NAME /* name */);
        
    if (result < 0) {
        printk(KERN_WARNING "unable to get major \n");
        return result;
    }

    major = MAJOR(dev);
    
    for (i=0;i<MAX_DIAGTIMER;i++) {
        cdev_init(&diag_timer_devs[i].cdev, &diag_timer_fops);
        diag_timer_devs[i].cdev.owner = THIS_MODULE;
        diag_timer_devs[i].cdev.ops = &diag_timer_fops;
        devno = MKDEV(major, dev+i);
        err = cdev_add (&diag_timer_devs[i].cdev, devno, 1);

        if (err) {
            printk (KERN_NOTICE "Error %d adding diag_timer", err);
            return -EAGAIN;
        }
    }

    dev_class = class_create(THIS_MODULE, KLM_DIAG_TIMER_NAME);
    if (IS_ERR(ptr_err = dev_class))
        goto err2;
    
    dev_timer = device_create(dev_class, NULL, MKDEV(major, 0), NULL, KLM_DIAG_TIMER_NAME);
    if (IS_ERR(ptr_err = dev_timer))
        goto err;
    
    /*
mknod /dev/diag_timer0 major 0
mknod /dev/diag_timer1 major 1
diag_timer0 is open for device with minor 0
diag_timer1 is open for device with minor 1
    */
    return 0;
 err:
    class_destroy(dev_class);
 err2:
    for (i=0;i<MAX_DIAGTIMER;i++) {
        cdev_del(&diag_timer_devs[i].cdev);
    }
    unregister_chrdev_region((dev), MAX_DIAGTIMER);
    return  PTR_ERR(ptr_err);
}


static void __exit diag_timer_cleanup_module (void) {

    int i;

    device_destroy(dev_class, MKDEV(major, 0));
    class_destroy(dev_class);
        
    for (i=0;i<MAX_DIAGTIMER;i++) {
        cdev_del(&diag_timer_devs[i].cdev);
    }
    unregister_chrdev_region((dev), MAX_DIAGTIMER);
    dev = 0;
    
    
}

module_init(diag_timer_init_module);
module_exit(diag_timer_cleanup_module);
MODULE_AUTHOR("");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Device Driver diag_timer");

/******** History ******** 
$Log: diag_timer.c,v $
Revision 1.2  2012/03/28 00:38:25  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

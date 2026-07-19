/* $Id: max4422.c,v 1.1 2012/07/11 23:33:11 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/fortitude/max4422.c,v $
 ***********************************************************************
 * File Name: max4422.c
 *
 * Description:  Maxim DS4422/4424 I2C Chip Driver
 *               Port from SW team.
 *
 * Christine Wen -- June 2012
 *
 * Copyright (c)2011 - 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/bcd.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>

#include <asm/uaccess.h>
#include <asm/system.h>

#define SLAVE_ADDR1 0x20
#define SLAVE_ADDR2 0x30
#define SLAVE_ADDR3 0xA0
#define SLAVE_ADDR4 0xE0

#define MEM_ADDR1 0xF8
#define MEM_ADDR2 0xF9
#define MEM_ADDR3 0xFA
#define MEM_ADDR4 0xFB

/*
 Functions declaration
I2C_CLIENT_INSMOD_1(max4422);
 */
static unsigned short normal_i2c[] = { SLAVE_ADDR2, I2C_CLIENT_END };
static unsigned short ignore = I2C_CLIENT_END;

static int max4422_attach_adapter(struct i2c_adapter *adapter);
static int max4422_detect(struct i2c_adapter *adapter, int address, int kind);
static int max4422_detach_client(struct i2c_client *client);

static struct i2c_client_address_data addr_data = {
        .normal_i2c = normal_i2c,
        .probe      = &ignore,
        .ignore     = &ignore,
};

/*
 * Driver data (common to all clients)
 */
static struct i2c_driver max4422_driver = {
        .driver = {
                .name   = "max4422",
        },
        .attach_adapter = max4422_attach_adapter,
        .detach_client  = max4422_detach_client,
};


#define show_reg(reg_name, reg_address) \
static ssize_t show_##reg_name(struct device *dev, \
			       struct device_attribute *attr, \
			       char *buf) \
{ \
	s32 ret_val = 0; \
	struct i2c_client *client = to_i2c_client(dev); \
	ret_val = i2c_smbus_read_byte_data(client, reg_address); \
	return (ret_val); \
}
	
#define set_reg(reg_name, reg_address) \
static ssize_t set_##reg_name(struct device *dev, \
			       struct device_attribute *attr, \
			       const char *buf, size_t count) \
{ \
	s32 ret_val = 0; \
	struct i2c_client *client = to_i2c_client(dev); \
	uint32_t temp_val = simple_strtoul(buf, NULL, 16); \
	ret_val = i2c_smbus_write_byte_data(client, reg_address,temp_val); \
	return (ret_val); \
}
	
show_reg(reg_out1, MEM_ADDR1);
show_reg(reg_out2, MEM_ADDR2);
show_reg(reg_out3, MEM_ADDR3);
show_reg(reg_out4, MEM_ADDR4);

set_reg(reg_out1, MEM_ADDR1);
set_reg(reg_out2, MEM_ADDR2);
set_reg(reg_out3, MEM_ADDR3);
set_reg(reg_out4, MEM_ADDR4);

static DEVICE_ATTR(reg_out1, S_IWUGO | S_IRUGO, show_reg_out1, set_reg_out1);
static DEVICE_ATTR(reg_out2, S_IWUGO | S_IRUGO, show_reg_out2, set_reg_out2);
static DEVICE_ATTR(reg_out3, S_IWUGO | S_IRUGO, show_reg_out3, set_reg_out3);
static DEVICE_ATTR(reg_out4, S_IWUGO | S_IRUGO, show_reg_out4, set_reg_out4);


static int max4422_attach_adapter(struct i2c_adapter *adapter)
{
        int val = 0;
	printk(KERN_DEBUG "Finding the I2C Adapter ...\n");
        val = i2c_probe(adapter, &addr_data, max4422_detect);
        return val;
}


/*
 * The following function does more than just detection. If detection
 * succeeds, it also registers the new chip.
 */
static int max4422_detect(struct i2c_adapter *adapter, int address, int kind)
{
	int ret_val = 0;
	struct i2c_client *new_client = NULL;

	printk(KERN_DEBUG "Registering the I2C Chip @ Address -> 0x%02x\n", address);
	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		printk(KERN_DEBUG "Unsupported Func: I2C_FUNC_SMBUS_BYTE_DATA\n");
		goto _exit;
	}
	new_client = kmalloc(sizeof(*new_client), GFP_KERNEL);
	if(new_client == NULL) {
		printk(KERN_DEBUG "Failed to allocate memory for I2C Client!!\n");
		goto _exit;
	}
	memset(new_client, 0, sizeof(*new_client));

	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->flags = 0;
	new_client->driver = &max4422_driver;
	strncpy(new_client->name, "max4422", I2C_NAME_SIZE);
	
	ret_val = i2c_attach_client(new_client);
	if(ret_val != 0) {
		printk(KERN_DEBUG "Failed to attach I2C Client!!\n");
		goto _exit;
	}

	printk(KERN_DEBUG "Creating Device Files.....\n");
	device_create_file(&new_client->dev,
			   &dev_attr_reg_out1);
	device_create_file(&new_client->dev,
			   &dev_attr_reg_out2);
	device_create_file(&new_client->dev,
			   &dev_attr_reg_out3);
	device_create_file(&new_client->dev,
			   &dev_attr_reg_out4);


	_exit:
		if(ret_val != 0) {
			if(new_client != NULL) {
				kfree(new_client);
			}
		}	
	return ret_val;
}


static int max4422_detach_client(struct i2c_client *client)
{
	uint32_t ret_val;
	printk(KERN_DEBUG "Detaching the Client ...\n");
	ret_val = i2c_detach_client(client);
	if(ret_val != 0) {
		printk(KERN_DEBUG "Failed to detach I2C Client!!\n");
		return ret_val;
	}
	if(client) 
		kfree(client);
	return 0;
}


static int __init max4422_init(void)
{
	int err = 0;
	printk(KERN_DEBUG "Initializing Maxim I2C Chip Driver \n");
	err = i2c_add_driver(&max4422_driver);
	return err;
}


static void __exit max4422_exit(void)
{
	i2c_del_driver(&max4422_driver);
	printk(KERN_DEBUG "Exiting Maxim I2C Chip Driver \n");
}


MODULE_AUTHOR("Desh");
MODULE_DESCRIPTION("Maxim 4422/4424 Chip driver");
MODULE_LICENSE("GPL");

module_init(max4422_init);
module_exit(max4422_exit);

/******** History ********
$Log: max4422.c,v $
Revision 1.1  2012/07/11 23:33:11  ywen
Add I2C driver.


$Endlog$
*/

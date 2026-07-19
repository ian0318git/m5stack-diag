/* $Id: pmc-adapter.c,v 1.1 2012/07/11 23:33:10 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/fortitude/pmc-adapter.c,v $
 ***********************************************************************
 * File Name: pmc-adapter.c
 *
 * Description:  PMC I2C Host Controller Driver
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
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include "../../common/src/fortitude/pcmap.h"

#define I2C_STATUS_TIMEOUT 0x900
#define WAIT_FOR_ACK 200
#define EXIT_I2C 25

static uint32_t pmc_i2c_xfer(struct i2c_adapter *adap, unsigned short addr,
                            unsigned short flags, char read_write,
                            uint8_t command, int size,
                            union i2c_smbus_data *data);
static uint32_t pmc_i2c_func(struct i2c_adapter *adapter);
static uint32_t status_poll_ack(uint32_t expected_value, uint32_t index, uint32_t time_out);

static struct i2c_algorithm smbus_algorithm = {
	.smbus_xfer	= pmc_i2c_xfer,
	.functionality	= pmc_i2c_func,
};

static struct i2c_adapter pmc_adapter = {
	.owner	= THIS_MODULE,
	.class	= I2C_CLASS_HWMON,
	.algo	= &smbus_algorithm,
	.name	= "PMC I2C Bus Driver",
};

static uint32_t pmc_i2c_func(struct i2c_adapter *adapter){
	return (I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE |
		I2C_FUNC_SMBUS_BYTE_DATA);
};

#if 0
typedef struct pmc_reg_set
{
	volatile uint32_t i2c_cfg;      /* 0x0:R/W: Config Register */
	volatile uint32_t i2c_clkdiv;   /* 0x4:R/W: Clock divide for slave */
	volatile uint32_t i2c_dev_addr; /* 0x8:R/W: Slave device address */
	volatile uint32_t i2c_addr;     /* 0xC:R/W: Address to send */
	volatile uint32_t i2c_data_out; /* 0x10:R/W: Write data to slave */
	volatile uint32_t i2c_data_in;  /* 0x14:R: Read data from slave */
	volatile uint32_t i2c_status;   /* 0x18:R: Master status */
	volatile uint32_t i2c_sden;     /* 0x1C:R/W: Activate RD/WR Command to slave */
	volatile uint32_t i2c_bytcnt;   /* 0x20:R/W: Byte count length of command */
} pset;
#endif

static uint32_t status_poll_ack(uint32_t expected_value, uint32_t index, uint32_t time_out)
{

	uint32_t status = 0;
	uint32_t masked_status = 0;
	uint32_t cnt = 0;

	status = *(uint32_t volatile *) (MAP_I2C_STATUS + NPU_RIF_BASE + UNCACHE_ADDR_MSK);
	masked_status = status & expected_value;

	while ((masked_status != expected_value) && (cnt != time_out)){
		cnt++;
		status = *(uint32_t volatile *) (MAP_I2C_STATUS + NPU_RIF_BASE + UNCACHE_ADDR_MSK);
		masked_status = status & expected_value;
	}
	if(cnt == time_out){
		printk(KERN_DEBUG "!!! I2C STATUS TIMEOUT !!! read %x, expected %x, index %x \n", status,
			expected_value, index);
	}
	printk(KERN_DEBUG "Read Status : 0x%02x Expected Val: 0x%02x Index: 0x%02x\n", status, expected_value, index);
	return (status & 0x8);
}

static uint32_t pmc_i2c_xfer(struct i2c_adapter *adap, unsigned short addr,
			    unsigned short flags, char read_write,
			    uint8_t command, int size,
			    union i2c_smbus_data *data)
{
	uint32_t bad_ack = 0;
	uint32_t cnt = 0;
	uint32_t data_read = 0; 
	
	*(uint32_t *) (MAP_I2C_CFG + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x1A;
	*(uint32_t *) (MAP_I2C_CLKDIV + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x14A; /* At 400MHz Internal Sys Clock, 300KHz*/
	*(uint32_t *) (MAP_I2C_DEV_ADDR + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = addr; /* Slave Address*/
	switch(size) {

		case I2C_SMBUS_BYTE_DATA:
			if(read_write == I2C_SMBUS_READ) {
        			do {
					cnt++;
					*(uint32_t *) (MAP_I2C_BYTCNT + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x00;
					*(uint32_t *) (MAP_I2C_DATA_OUT + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = command;
					/* Memory Address on I2C Chip*/
					*(uint32_t *) (MAP_I2C_SDEN + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x00;
					bad_ack = status_poll_ack (0x02, 0x03, I2C_STATUS_TIMEOUT);
					*(uint32_t *) (MAP_I2C_BYTCNT + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x00;
					*(uint32_t *) (MAP_I2C_SDEN + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x01;
					bad_ack = bad_ack | status_poll_ack(0x07,0x4,I2C_STATUS_TIMEOUT);
					data_read = *(uint32_t *) (MAP_I2C_DATA_IN + NPU_RIF_BASE + UNCACHE_ADDR_MSK);
					bad_ack = bad_ack | status_poll_ack(0x03,0x5,I2C_STATUS_TIMEOUT);
				} while( (bad_ack != 0) && (cnt != WAIT_FOR_ACK) );
			} else {
        			do {
					cnt++;
					*(uint32_t *) (MAP_I2C_BYTCNT + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x01;
					*(uint32_t *) (MAP_I2C_DATA_OUT + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = command;
					/* Memory Address on I2C Chip*/
					*(uint32_t *) (MAP_I2C_SDEN + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = 0x00;
					bad_ack = status_poll_ack (0x02, 0x01, I2C_STATUS_TIMEOUT);

					*(uint32_t *) (MAP_I2C_DATA_OUT + NPU_RIF_BASE + UNCACHE_ADDR_MSK) = data->byte;
					bad_ack = bad_ack | status_poll_ack(0x03,0x2,I2C_STATUS_TIMEOUT);
				} while( (bad_ack != 0) && (cnt != WAIT_FOR_ACK) );
			} 
			break;
		default:
			return 0;
	}
        if(cnt == WAIT_FOR_ACK) {
		printk(KERN_DEBUG "Error: NO ACK Received from the Slave Device\n");
		return (-EINVAL); 
	} else if(read_write == I2C_SMBUS_READ) {
		printk(KERN_CRIT "0x%02x\n", data_read);
		return (EXIT_I2C);
	} else
		return (EXIT_I2C);
}

static int __init i2c_pmc_init(void)
{
	printk(KERN_DEBUG "Initializing PMC's I2C Host Controller\n"); 
	i2c_add_adapter(&pmc_adapter);
	return 0;
}

static void __exit i2c_pmc_exit(void)
{
	printk(KERN_DEBUG "Exiting PMC's I2C Host Controller\n"); 
	i2c_del_adapter(&pmc_adapter);
}

MODULE_AUTHOR("Desh");
MODULE_DESCRIPTION("PMC I2C Bus Driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");

module_init(i2c_pmc_init);
module_exit(i2c_pmc_exit);

/******** History ********
$Log: pmc-adapter.c,v $
Revision 1.1  2012/07/11 23:33:10  ywen
Add I2C driver.


$Endlog$
*/

/* $Id: gpio_I2C.c,v 1.2 2017/07/28 07:58:47 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/gpio_I2C.c,v $
 *------------------------------------------------------------------
 * gpio_I2C.c
 *      USe GPIO to generate I2C signlas
 *
 * Jul 2012, Smita Rane
 *
 * Copyright (c)2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "lsi_sp27xx_reg.h"
#include "libgpio.h"
#include "libgeneric.h"

uint32_t i2c_scl = 0;
uint32_t i2c_sda = 0;
uint32_t delay = DFLT_I2C_CLK_CYCLE; /* minimum cycle duration, (max freq) */

/* basic protocol */
inline void i2c_scl_high(void)
{
	__SP_GPIO_SET_HIGH(i2c_scl);
}

inline void i2c_scl_low(void)
{
	__SP_GPIO_SET_LOW(i2c_scl);
}

inline void i2c_sda_high(void)
{
	__SP_GPIO_SET_HIGH(i2c_sda);
}

inline void i2c_sda_low(void)
{
	__SP_GPIO_SET_LOW(i2c_sda);
}

inline uint8_t i2c_sda_read(void)
{
	uint32_t data;

	__SP_GPIO_DATA_READ(data);
	return (uint8_t)((data&i2c_sda) == i2c_sda);
}

inline void sp_GPIOasI2C_start(void)
{
	i2c_sda_high();
	lsi_mg_delay(delay/2);
	i2c_scl_high();
	lsi_mg_delay(delay/2);
	i2c_sda_low();
	lsi_mg_delay(delay/2);
	i2c_scl_low();
	lsi_mg_delay(delay/2);
}

inline void sp_GPIOasI2C_stop(void)
{
	i2c_scl_high();
	lsi_mg_delay(delay/2);
	i2c_sda_high();
	lsi_mg_delay(delay/2);
}

uint32_t sp_GPIOasI2C_Init(uint32_t gpio_for_scl, uint32_t gpio_for_sda, uint32_t scl_cycle)
{
	/* Initialize the PPB GPIOs */
	//sp_InitGPIO();

	if((gpio_for_scl>7)||(gpio_for_sda>7)||(gpio_for_scl==gpio_for_sda))
	{
		return ERROR;
	}

	if(scl_cycle == 0)
	{
		delay = DFLT_I2C_CLK_CYCLE;
		delay = 0;
	}
	else
	{
		delay = scl_cycle;
	}

	delay = scl_cycle;

	i2c_scl = (0x1<<gpio_for_scl);
	i2c_sda = (0x1<<gpio_for_sda);

	/* Set i2c_sda and i2c_scl to output data */
	__SP_GPIO_OUT(i2c_scl|i2c_sda);

	/* Set the clock and data lines to HIGH */
	i2c_sda_high();
	i2c_scl_high();

	lsi_mg_delay(10000);

	return SUCCESS;
}

void sp_GPIOasI2C_byte_transmit(uint8_t byte)
{
	uint32_t i = 8;

	while(i>0)
	{
		if((byte>>(i-1))&0x1)
		{
			i2c_sda_high();
		}
		else
		{
			i2c_sda_low();
		}

		lsi_mg_delay(delay/2);
		i2c_scl_high();
		lsi_mg_delay(delay);
		i2c_scl_low();
		lsi_mg_delay(delay/4);

		i--;
	}

	i2c_sda_low();

	/* processing ack */
	lsi_mg_delay(delay/4);
	i2c_scl_high();
	lsi_mg_delay(delay*2);
	i2c_scl_low();
}

uint8_t sp_GPIOasI2C_byte_receive(void)
{
	uint32_t i = 8;
	uint8_t read_data = 0;

	/* change direction of SDA temporarily */
	__SP_GPIO_IN(i2c_sda);

	while(i>0)
	{
		read_data = (read_data<<1);
		read_data |= i2c_sda_read();

		i2c_scl_high();
		lsi_mg_delay(delay);

		i2c_scl_low();
		lsi_mg_delay(delay/2);
		lsi_mg_delay(delay/4);

		i--;
	}

	i2c_sda_low();
	__SP_GPIO_OUT(i2c_sda);

	lsi_mg_delay(delay/4);
	i2c_scl_high();
	lsi_mg_delay(delay*2);
	i2c_scl_low();
	lsi_mg_delay(delay/2);

	return read_data;
}

void dac_i2c_random_write(uint8_t addr, uint8_t reg_num, uint16_t data)
{
	sp_GPIOasI2C_start();
	sp_GPIOasI2C_byte_transmit(addr&0xFE);
	sp_GPIOasI2C_byte_transmit(reg_num);
	sp_GPIOasI2C_byte_transmit((data>>8)&0xFF);
	sp_GPIOasI2C_stop();

	sp_GPIOasI2C_start();
	sp_GPIOasI2C_byte_transmit(addr&0xFE);
	sp_GPIOasI2C_byte_transmit(reg_num);
	sp_GPIOasI2C_byte_transmit(data&0xFF);
	sp_GPIOasI2C_stop();
}

uint16_t dac_i2c_random_read(uint8_t addr, uint8_t reg_num)
{
	uint16_t read_data = 0;

	sp_GPIOasI2C_start();
	sp_GPIOasI2C_byte_transmit(addr&0xFE);
	sp_GPIOasI2C_byte_transmit(reg_num);
	sp_GPIOasI2C_start();
	sp_GPIOasI2C_byte_transmit((addr&0xFE)|0x1);
	read_data = (uint16_t)(sp_GPIOasI2C_byte_receive()&0xFF);
	sp_GPIOasI2C_stop();

	read_data = (read_data<<8);

	sp_GPIOasI2C_start();
	sp_GPIOasI2C_byte_transmit(addr&0xFE);
	sp_GPIOasI2C_byte_transmit(reg_num);
	sp_GPIOasI2C_start();
	sp_GPIOasI2C_byte_transmit((addr&0xFE)|0x1);
	read_data |= (uint16_t)(sp_GPIOasI2C_byte_receive()&0xFF);
	sp_GPIOasI2C_stop();

	return read_data;
}

uint32_t dac_i2c_seq_write(uint8_t addr, uint8_t reg_num, uint16_t* buf)
{
	return 0;
}

uint32_t dac_i2c_seq_read(uint8_t addr, uint8_t reg_num, uint16_t* buf)
{
	return 0;
}

/******** History ********
$Log: gpio_I2C.c,v $
Revision 1.2  2017/07/28 07:58:47  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/12/24 00:09:01  srane
Cleanup the ^M and do not init gpio.

Revision 1.1  2012/07/17 20:45:24  srane
Add GPIO I2C support.


$Endlog$
*/

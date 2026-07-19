/* $Id: i2c_api.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/i2c_api.c,v $
 *------------------------------------------------------------------
 * Filename: i2c_api.c
 *
 * Description: Transformers (CPU) I2C API supports.
 *
 * Copyright (c) 2014-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/* #include <stdio.h> */
/* #include <string.h> */

#include "endians.h"
#include "common.h"
#include "error.h"
#include "types.h"
/* #include "time.h" */

#include "proto.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "mon_plat_defs.h"
#include "cross_platform.h" 

#include "pca9545a.h"
#include "dev_object.h"
#include "goofy_i2c.h"
#include "dev_at24c0n.h"	/* 256-byte EEPROM special handling */
#include "i2c_address.h"
#include "dash_fpga.h"

/* #define USE_OLD_GEPHY_ADDR  * */
/* #define SKIP_GEPHY2_3  * */
extern unsigned long dash_fpga;
extern unsigned char i2c_debug;

#ifndef LINUX_APP
static pid_t
getpid(void)
{
    return((pid_t)(0x1234));
}
#endif /* LINUX */

extern uint8_t plat_get_goofy_i2c_num (uint32_t i2c_bus_num);
extern int goofy_i2c_reset(uint32_t plat_i2c_id);
extern int goofy_i2c_init(uint32_t plat_i2c_id, uint32_t op_spd);
extern int goofy_i2c_rd(uint32_t plat_i2c_id, uint32_t slv_addr, 
			uint32_t sub_addr_sz, uint32_t reg_addr,
			uint32_t data_len, uchar *data_buf);
extern int goofy_i2c_wr(uint32_t plat_i2c_id, uint32_t slv_addr, 
			uint32_t sub_addr_sz, uint32_t reg_addr, 
			uint32_t data_len, uchar *data_buf);
extern int goofy_i2c_dma_wr(uint32_t plat_i2c_id, uint32_t slv_addr,
			    uint32_t sub_addr_sz, uint32_t reg_addr, 
			    uint32_t data_len, uchar *data_buf);

static n2g_i2c_states_t *get_n2g_i2c_states_table(uint8_t i2c_bus,
						  uint8_t i2c_dev);
extern boolean has_i2c_mux(void);


/* temporary value. Needs to be removed after the values are ready */
#define TEMP_X	0		/* I2C seq read offset size */
#define TEMP_Y	0		/* I2C seq write offset size */

/*********************************************************************
 *              Global variables
 *********************************************************************
 */
static char *i2c_err[] =
    {
        "OK",
        "BUSY",
        "time out",
        "RC_I2C_DMA_ADDR_NOT_64ALIGN",
        "no slave device ack",
        "no slave sub_addr device ack",
        "RC_I2C_BUS_ERR",
        "unknown error",
        "\0",
    };


/*********************************************************************
 *		I2C devices characteristics tables.
 *********************************************************************
 */
/* CPU I2C controller 1 devices */
static n2g_i2c_dev_t n2g_i2c1_clk1 =
	{CPU_I2C1, MB_I2C_ADDR_CLK1, 0, 0, 0};	/* CLK1 */
static n2g_i2c_dev_t n2g_i2c1_clk2 =
	{CPU_I2C1, MB_I2C_ADDR_CLK2, 1, 1, 0};	/* CLK2 */
static n2g_i2c_dev_t n2g_i2c1_ps_cookie =
	{CPU_I2C1, PSU_I2C_ADDR_COOKIE, 1, 1, 0};	/* PS Cookie */
static n2g_i2c_dev_t n2g_i2c1_ps_obfl =
	{CPU_I2C1, PSU_I2C_ADDR_OBFL, TEMP_X, TEMP_Y, 0};	/* PS OBFL */
static n2g_i2c_dev_t n2g_i2c1_ps_temp =
	{CPU_I2C1, PSU_I2C_ADDR_TEMP, 1, 1, 0};	/* PS Temperature sensor */
static n2g_i2c_dev_t n2g_i2c1_usb =
	{CPU_I2C1, USB_I2C_ADDR, 0, 2, 0};		/* USB Console */
static n2g_i2c_dev_t n2g_i2c1_usb_r =
	{CPU_I2C1, USB_R_I2C_ADDR, 0, 1, 0};	/* USB Console Read/Write */

static n2g_i2c_dev_t n2g_i2c1_mb_temp =
	{CPU_I2C1, MB_I2C_ADDR_MB_TEMP, 1, 1, 0};	/* Motherboard Temp Sensor */

/* IOFPGA I2C device */
static n2g_i2c_dev_t n2g_i2c0_quack =
	{IOFPGA_I2C, MB_I2C_ADDR_ACT2, 0, 0, 0};	/* Quack */


/*********************************************************************
 *		I2C device state tables.
 *********************************************************************
 */
/*
 * CPU South Bridge I2C devices table
 * (in the same order as in MB_I2C1_DEVICE enum)
 */
static n2g_i2c_states_t i2c_mb1_state[MB_I2C_1_INVALID] = {
    {0, &n2g_i2c1_clk1,  N2G_I2C_IDLE},		/* CLK1 */
    {0, &n2g_i2c1_clk2,  N2G_I2C_IDLE},		/* CLK2 */
    {0, &n2g_i2c1_ps_cookie, N2G_I2C_IDLE},	/* PS1 Cookie */
    {0, &n2g_i2c1_ps_obfl,   N2G_I2C_IDLE},	/* PS1 OBFL */
    {0, &n2g_i2c1_ps_temp,   N2G_I2C_IDLE},	/* PS1 Temperature Sensor */
    {0, &n2g_i2c1_ps_cookie, N2G_I2C_IDLE},	/* PS2 Cookie */
    {0, &n2g_i2c1_ps_obfl,   N2G_I2C_IDLE},	/* PS2 OBFL */
    {0, &n2g_i2c1_ps_temp,   N2G_I2C_IDLE},	/* PS2 Temperature Sensor */
    {0, &n2g_i2c1_usb,	N2G_I2C_IDLE},		/* USB Console */
    {0, &n2g_i2c1_usb_r, N2G_I2C_IDLE},		/* USB Console Read/Write */
    {0, &n2g_i2c1_mb_temp, N2G_I2C_IDLE},	/* Mother Board Temp Sensor */
};

/* IO FPGA I2C devices table */
 static n2g_i2c_states_t i2c_iofpga_state[IOFPGA_I2C_INVALID] = {
    {0, &n2g_i2c0_quack, N2G_I2C_IDLE},		/* Motherboard Quack */
    
 };

char *
i2c_err_str (int num)
{
    if (num < RC_I2C_UNKNOWN)
        return i2c_err[num];
    return i2c_err[RC_I2C_UNKNOWN];
}

/*********************************************************************
 *
 * Function:	n2g_i2c_init
 *
 * Description:	N2G I2C API for init. This API only initialize the controller,
 *		not the I2C devices, except Goofy port 5 1:4 Mux.
 *
 * Inputs:	i2c_p	- Pointer to the N2G I2C API interface struct. Fields
 *			  needed in the struct are:
 *				i2c_bus_type, i2c_speed.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_init(n2g_i2c_if_t *i2c_p)
{
    uint32_t rc = PASSED;

    /* Call the lower device driver */
    switch(i2c_p->i2c_bus_type) {
    case CPU_I2C0:
	break;
    case IOFPGA_I2C:
       	//rc = iofpga_i2c_init(i2c_p->i2c_speed);
        return PASSED;
	break;
    }

    return(rc);
}


/*********************************************************************
 *
 * Function:	n2g_i2c_open
 *
 * Description:	legacy code. not used.
 *	       
 */
uint32_t n2g_i2c_open(n2g_i2c_if_t *i2c_p)
{
    return(PASSED);

}

/********************************************************************
 *
 * Function:	n2g_i2c_close
 *
 * Description:	legacy code. not used.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_close(n2g_i2c_if_t *i2c_p)
{
    return PASSED;
}

/*********************************************************************
 *
 * Function:	n2g_i2c_read
 *
 * Description:	N2G Generic I2C Read API.
 *
 * Inputs:	i2c_p	- Pointer to the N2G I2C API interface struct. Fields
 *			  needed in the struct are:
 *			  i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_DEV - Invalid device address.
 *		E_I2C_NOT_LOCKED - Device not locked by any process.
 *		E_I2C_LOCKED - Device is locked by another process.
 *		E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *		Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_read(n2g_i2c_if_t *i2c_p)
{
    uint32_t rc = 0;
    unsigned long addr = 0;
    n2g_i2c_states_t *state_p;	/* pointer to the state struct */
    
    /* Call the lower device driver */
    switch(i2c_p->i2c_bus_type) {
    case CPU_I2C0:
        printf("CPU_I2C0 not suported i2c_api.c line %d\n", __LINE__);
        assert(0);
	break;
    case CPU_I2C1:
        state_p = get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);
	break;

    case IOFPGA_I2C:
        /*
        printf("i2c_api.c n2g_i2c_read: %d: IOFGPA_I2C\n",  __LINE__);
        printf("i2c_dev %#x, rd_hd_size %d, offset %#x, size %#x \n\n",
               i2c_p->i2c_dev, i2c_p->rd_hd_size,
               i2c_p->offset, i2c_p->size);
        */
        addr = get_platform_i2c_addr(i2c_p->i2c_ctrl);

        rc = gfy_i2c_rd((goofy_i2c_t *)addr, i2c_p->mux, i2c_p->i2c_dev,
                        i2c_p->offset,
                        i2c_p->sub_addr_len,
                        i2c_p->size,
                        (unsigned char *) i2c_p->buf);
     	/* printf("i2c_p-> buf = 0x%X\n", *i2c_p->buf); */
        if(i2c_debug) {
		    printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
		    printf("i2c_if_p->mux %d\n", i2c_p->mux);
		    printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
		    printf("i2c_if_p->offset 0x%X\n", i2c_p->offset);
		    printf("i2c_if_p->buf 0x%X\n", *i2c_p->buf);
        }
	break;

    default:
        printf("not suported i2c_api.c %d line %d\n", i2c_p->i2c_bus_type, __LINE__);
        assert(0);
	break;
    } /* endof switch */

    /* According to I2C specification, tBUF - "bus free time between a STOP and
     * START condition" is 4.7 us minimum for Standard-mode
     */
    wastetime(I2C_BUS_FREE_TIME);
    return (rc);
}

/*********************************************************************
 *
 * Function:	n2g_i2c_write
 *
 * Description:	N2G Generic I2C Write API.
 *
 * Inputs:	i2c_p	- Pointer to the N2G I2C API interface struct. Fields
 *			  needed in the struct are:
 *			  i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_DEV - Invalid device address.
 *		E_I2C_NOT_LOCKED - Device not locked by any process.
 *		E_I2C_LOCKED - Device is locked by another process.
 *		E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *		Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_write(n2g_i2c_if_t *i2c_p)
{

    n2g_i2c_states_t *state_p; /* pointer to the state struct */
    unsigned long addr;
    uint rc;

    /* Call the lower device driver */
    switch(i2c_p->i2c_bus_type) {
    case CPU_I2C0:
        printf("not suported i2c_api.c n2g_i2c_write: line %d\n", __LINE__);
        assert(0);
	break;
    case CPU_I2C1:
        state_p = get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);
	break;

    case IOFPGA_I2C:
        //        printf("i2c_api.c: ofset %#x %d\n", i2c_p->offset, __LINE__);
        addr = get_platform_i2c_addr(i2c_p->i2c_ctrl);
        rc = gfy_i2c_wr((goofy_i2c_t *)addr, i2c_p->mux, i2c_p->i2c_dev,
                        i2c_p->offset,
                        i2c_p->sub_addr_len,
                        i2c_p->size,
                        (unsigned char *)i2c_p->buf);
        if(i2c_debug) {
		    printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
		    printf("i2c_if_p->mux %d\n", i2c_p->mux);
		    printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
		    printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
		    printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
        }
	break;
    default:
        printf("not suported i2c_api.c n2g_i2c_write: %d line %d\n",
                   i2c_p->i2c_bus_type, __LINE__);
        assert(0);
	break;
    
    } /* endof switch */


    /* According to I2C specification, tBUF - "bus free time between a STOP and
     * START condition" is 4.7 us minimum for Standard-mode
     */
    wastetime(I2C_BUS_FREE_TIME);
    return (rc);
}

/*********************************************************************
 *
 * Function:	get_n2g_i2c_states_table
 *
 * Description:	Get N2G I2C device table pointer.
 *
 * Inputs:	i2c_bus - N2G_I2C_BUS in n2g_i2c.h
 *		i2c_dev - MB_I2C_DEVICE in n2g_i2c.h.
 *
 * Outputs:	Pointer to the N2G I2C table of requested device.
 *		NULL if not a valid device.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static n2g_i2c_states_t * get_n2g_i2c_states_table(uint8_t i2c_bus, uint8_t i2c_dev)
{

    printf("i2c_bus %d i2c_dev %#x\n", i2c_bus, i2c_dev);
    switch(i2c_bus) {
    case CPU_I2C0:
        printf("not suported i2c_api.c line %d\n", __LINE__);
	break;
    case CPU_I2C1:
	/* ICH9 - Southbridge */
	if (i2c_dev >= MB_I2C_1_INVALID) {
	    /* Invalid I2C device */
	    return(NULL);
	} else {
	    return(&i2c_mb1_state[i2c_dev]);
	}
	break;
    case IOFPGA_I2C:
        if (i2c_dev >= IOFPGA_I2C_INVALID) {
            /* Invalid I2C device */
            return(NULL);
        } else {
            return(&i2c_iofpga_state[i2c_dev]);
        }
        break;
    default:
	/* Invalid I2C bus number requested */
        assert(!"i2c_api.c : states table is null\n");
	return(NULL);
	break;
    } /* endof i2c_bus */

    return (NULL);
}


/*-------------------------------------------------
 * $Log: i2c_api.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */


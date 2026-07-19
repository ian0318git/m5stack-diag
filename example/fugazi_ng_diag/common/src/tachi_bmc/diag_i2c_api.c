/* $Id: diag_i2c_api.c,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_api.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_api.c - I2C API Supports
 * 
 * July 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "error.h"
#include "types.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "diag_fpga_i2c.h"
#include "diag_i2c_api.h"
#include "diag_i2c_lib.h"

uint32_t n2g_i2c_reset(n2g_i2c_if_t *);
uint32_t n2g_i2c_init(n2g_i2c_if_t *);
uint32_t n2g_i2c_open(n2g_i2c_if_t *);
uint32_t n2g_i2c_close(n2g_i2c_if_t *);
uint32_t n2g_i2c_read(n2g_i2c_if_t *);
uint32_t n2g_i2c_write(n2g_i2c_if_t *);

static n2g_i2c_states_t *get_n2g_i2c_states_table(uint8_t, uint8_t);

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
 *      I2C devices characteristics tables.
 *********************************************************************
 */
/* FPGA I2C Controller */
static n2g_i2c_dev_t n2g_fpga_i2c_act2 =
    {IOFPGA_I2C, MB_I2C_ADDR_ACT2, 0, 0, 0};  /* ACT2 */


/* IO FPGA I2C devices table */
static n2g_i2c_states_t i2c_iofpga_state[IOFPGA_I2C_INVALID] = {
    {0, &n2g_fpga_i2c_act2, N2G_I2C_IDLE}, /* ACT2 */
};


/*********************************************************************
 *
 * Function:	n2g_i2c_reset
 *
 * Description: API for N2G I2C controller reset. This API does not
 *		reset individual I2C device.
 *		
 *		The I2C controller state is not checked, so that if the I2C
 *		controller is busy with other device, it will be yanked out
 *		of the busy state, and enters idle state.
 *
 * Inputs:	i2c_p	- Pointer to the N2G I2C API interface struct.
 *			  i2c_bus_type and i2c_dev are needed for this struct.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_DEV - I2C device is not a valid device.
 *		Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_reset (n2g_i2c_if_t *i2c_p)
{
    n2g_i2c_states_t *state_p; /* pointer to the state struct */
    uint32_t rc;

    /* Get the state table of the I2C device */
    state_p = get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);

    if (state_p == NULL) {
        return(E_I2C_INV_DEV);
    }

    /* Call the lower device driver to reset its controller */
    switch(i2c_p->i2c_bus_type) {
    case IOFPGA_I2C:
        printf("%s: Doesn't support reset\n", __FUNCTION__);
        break;
    default: 
        rc = E_I2C_INV_DEV;
        break;
    }

    /* Reset the device to unlocked and idle state */
    state_p->state = N2G_I2C_IDLE;
    state_p->pid = 0;
    
    return (rc);
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
uint32_t n2g_i2c_init (n2g_i2c_if_t *i2c_p)
{
    uint32_t rc = PASSED;

    /* Call the lower device driver */
    switch(i2c_p->i2c_bus_type) {
    case IOFPGA_I2C:
        return (PASSED);
    }
    
    return (rc);
}


/*********************************************************************
 *
 * Function:	n2g_i2c_open
 *
 * Description:	legacy code. not used.
 *	       
 */
uint32_t n2g_i2c_open (n2g_i2c_if_t *i2c_p)
{
    return (PASSED);
}


/*********************************************************************
 *
 * Function:	n2g_i2c_close
 *
 * Description:	legacy code. not used.
 *	       
 */
uint32_t n2g_i2c_close (n2g_i2c_if_t *i2c_p)
{
    return (PASSED);
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
uint32_t n2g_i2c_read (n2g_i2c_if_t *i2c_p)
{
    int rc;
    int i2c_ctrl_addr;

    switch (i2c_p->i2c_bus_type) {
    case CPU_I2C0:
    case CPU_I2C1:
    case CPU_I2C2:
    case CPU_I2C3:
    case CPU_I2C4:
    case CPU_I2C5:
    case CPU_I2C6:
    case CPU_I2C7:
        rc = diag_i2c_read(i2c_p->i2c_bus_type, i2c_p->i2c_dev, i2c_p->offset, 
                           i2c_p->size, (unsigned char *)i2c_p->buf); 
        if (rc) {
            return (E_I2C_INV_DEV);
        }
        break;
    case IOFPGA_I2C:
        /* Call FPGA I2C Read */
        i2c_ctrl_addr = diag_fpga_get_i2c_ctrl_addr(i2c_p->i2c_ctrl);
#ifdef FOXCONN_FPGA
        rc = diag_fxn_fpga_i2c_read(i2c_ctrl_addr, i2c_p->mux, i2c_p->i2c_dev,
                                    i2c_p->offset, i2c_p->sub_addr_len, i2c_p->size,
                                    (unsigned char *)i2c_p->buf);
#else
        rc = diag_fpga_i2c_read(i2c_ctrl_addr, i2c_p->mux, i2c_p->i2c_dev,
                                i2c_p->offset, i2c_p->sub_addr_len, i2c_p->size,
                                (unsigned char *)i2c_p->buf);
#endif

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
        }
        break;
    default:
        printf("%s: Bus Type not supported (%d)\n", __FUNCTION__, 
               i2c_p->i2c_bus_type);
        return (E_I2C_INV_DEV);
    }
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
uint32_t n2g_i2c_write (n2g_i2c_if_t *i2c_p)
{
    int rc;
    int i2c_ctrl_addr;

    switch (i2c_p->i2c_bus_type) {
    case CPU_I2C0:
    case CPU_I2C1:
    case CPU_I2C2:
    case CPU_I2C3:
    case CPU_I2C4:
    case CPU_I2C5:
    case CPU_I2C6:
    case CPU_I2C7:
        rc = diag_i2c_write(i2c_p->i2c_bus_type, i2c_p->i2c_dev, i2c_p->offset, 
                            i2c_p->size, (unsigned char *)i2c_p->buf); 
        if (rc) {
            return (E_I2C_INV_DEV);
        }
        break;

    case IOFPGA_I2C:
        /* Call FPGA I2C write */
        i2c_ctrl_addr = diag_fpga_get_i2c_ctrl_addr(i2c_p->i2c_ctrl);

#ifdef FOXCONN_FPGA
        rc = diag_fxn_fpga_i2c_write(i2c_ctrl_addr, i2c_p->mux, i2c_p->i2c_dev,
                                i2c_p->offset, i2c_p->sub_addr_len, i2c_p->size,
                                (unsigned char *)i2c_p->buf);
#else
        rc = diag_fpga_i2c_write(i2c_ctrl_addr, i2c_p->mux, i2c_p->i2c_dev,
                                i2c_p->offset, i2c_p->sub_addr_len, i2c_p->size,
                                (unsigned char *)i2c_p->buf);

#endif
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->i2c_siz 0x%X\n", i2c_p->size);
            printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
        }
        break;
    default:
        printf("%s: Bus Type not supported (%d)\n", __FUNCTION__, 
               i2c_p->i2c_bus_type);
        return (E_I2C_INV_DEV);
    }
    return (rc);
}


/*********************************************************************
 *
 * Function:    get_n2g_i2c_states_table
 *
 * Description: Get N2G I2C device table pointer.
 *
 * Inputs:  i2c_bus - N2G_I2C_BUS in n2g_i2c.h
 *      i2c_dev - MB_I2C_DEVICE in n2g_i2c.h.
 *
 * Outputs: Pointer to the N2G I2C table of requested device.
 *      NULL if not a valid device.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static n2g_i2c_states_t *get_n2g_i2c_states_table (uint8_t i2c_bus, 
                                                   uint8_t i2c_dev)
{
    printf("i2c_bus %d i2c_dev %#x\n", i2c_bus, i2c_dev);

    switch (i2c_bus) {
    case IOFPGA_I2C:
        return (&i2c_iofpga_state[i2c_dev]);
        break;            
    default:
        printf("%s: state table is null (%d)\n", __FUNCTION__, i2c_bus);
        break;
    }
    
    return (NULL);
}

char *
i2c_err_str (int num)
{
    if (num < RC_I2C_UNKNOWN)
        return i2c_err[num];
    return i2c_err[RC_I2C_UNKNOWN];
}

/*---------------------------------------------------------------
$Log: diag_i2c_api.c,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.5  2015/09/18 06:58:54  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.4  2015/08/27 01:24:26  alpeng
update i2c utils; add ngio init on linux_main.c

Revision 1.1.2.3  2015/08/21 10:38:30  benchen2
Add foxconn FPGA I2C R/W Function

Revision 1.1.2.2  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/

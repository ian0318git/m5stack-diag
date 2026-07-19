/* $Id: tam_act2_api_drv_support.c,v 1.1 2020/08/19 09:50:05 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/tam_act2_api_drv_support.c,v $
 *------------------------------------------------------------------
 * 
 * tam_act2_api_drv_support.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "nmc93c46.h"
#include "common.h"
#include "menu.h"
#include "proto.h"
#include "n2g_api_rc.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "platform_i2c.h"
#include "smart_cookie.h"
#include "act2_utils.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "defs.h"
#include "error.h"
#include "tam_act2_api_drv_support.h"
#include "tam_aikido_mailbox.h"
#include "tam_library.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include "highrise_cpld_api.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/
/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern int act2_i2c_debug;
extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag;  
extern int get_i2c_fd(int i2c_bus);
extern void * tam_act2_get_n2g_i2c_if(void);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
static int set_i2c_slave_addr(int fp, int addr);
int is_tam_aikido_mbox_on(void);
int is_tam_aikido_on(void);
/*
 * Function: set_i2c_slave_addr()
 * Set ACT2 slave address
 *
 * Input: addr - I2C slave address
 *
 * Output: SUCCEED if set I2C slave address successfully
 */
static
int set_i2c_slave_addr(int fp, int addr)
{
    if (ioctl(fp, I2C_SLAVE, addr) < 0) {
        printf("%s:ioctl(I2C_SLAVE, 0x%x)", __FUNCTION__, addr);
        return (FAILED);
    }

    return (SUCCEED);
}



/*
 * Function: tam_lib_platform_write 
 *
 * This function perform write access to ACT2 device. Tam libary location:
 * /auto/sp-engops/diags/pld/act2lite/x86/victory/tam_library_w_standalone_victory_x86_64.a
 *
 * Inputs: ptr to platform_opaque_handle(module) -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: number of bytes written
 *         -1 error
 */
int32_t
tam_lib_platform_write(void *platform_opaque_handle,
                       uint8_t * send_buffer, uint32_t length)
{
    int res, ix;
    int fp = -1;
    uint16_t dev_addr; 
    n2g_i2c_if_t *n2g_i2c_act;

    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();
    fp = get_i2c_fd(n2g_i2c_act->i2c_bus_type);
    if (fp == -1) {
        printf("[%s]:%d, Error: failed to get fp for I2C_BUS[%d]",
                __FUNCTION__, __LINE__, n2g_i2c_act->i2c_bus_type);
        return (FALSE);
    }
    dev_addr = n2g_i2c_act->i2c_dev;

    /*
     * Print one space to solve the corrupt printf buffer, 
     * otherwise weird chars will be printed out.
     */
    printf(" ");
    fflush(stdout);

    if (act2_i2c_debug) {
        printf("[%s]:%d fp:%d, dev_addr:0x%x, bus:%d\n", 
                __FUNCTION__, __LINE__, fp, dev_addr, n2g_i2c_act->i2c_bus_type);

        printf("\n Inside tam_lib_platform_write. length = 0x%x \n",
               length);
        for (ix = 0; ix < length; ix++) {
            printf(" %02x", send_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }

    usleep(ACT_RW_DELAY);

    for (ix = 0; ix < ACT_RETRY; ix++) {
        res = set_i2c_slave_addr(fp, dev_addr);
        if (res != SUCCEED) {
            printf("\n[%s]:%d, Error: unable to connect dev:%#x with rc:%d",
                    __FUNCTION__, __LINE__, dev_addr, res);
            return (FAILED);
        }
        res = write(fp, send_buffer, length);
        if (res == -1) {
            msleep(100);
            continue;
        } else {
            break;
        }
    }
    if (ix >= ACT_RETRY) {
        printf("\n *** ERROR: unable to write from ACT2 device, error no is %d\n", 
              errno);
        printf("Something went wrong with write()! Error description is: %s\n", 
              strerror(errno));
        fflush(stdout);
        return (-1);
    }
    usleep(ACT_RW_DELAY);
    return (length);
}

/*
 * Function: tam_lib_platform_read 
 *
 * This function perform read access to ACT2 device. Tam libary location:
 * /auto/sp-engops/diags/pld/act2lite/x86/victory/tam_library_w_standalone_victory_x86_64.a
 *
 * Inputs: ptr to module(platform_opaque_handle) -
 *         min_time - min time to wait (for future use)
 *         max_time - max time to wait (for future use)
 *         ptr to status_buf(receive_buf) -
 *         length - number of bytes to read.
 *         bytes_actually_read - number of bytes actually read.
 *
 * Output: TAM_RC_OK successfully
 *         -1   failed
 */
tam_lib_status_t
tam_lib_platform_read(void *platform_opaque_handle,
                      uint32_t min_time,
                      uint32_t max_time,
                      uint8_t * read_buffer,
                      uint16_t bytes_to_read,
                      uint16_t * bytes_actually_read)
{
    int res, ix;
    int fp = -1;
    uint16_t dev_addr; 
    n2g_i2c_if_t *n2g_i2c_act;

    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();
    fp = get_i2c_fd(n2g_i2c_act->i2c_bus_type);
    if (fp == -1) {
        printf("[%s]:%d, Error: failed to get fp for I2C_BUS[%d]",
                __FUNCTION__, __LINE__, n2g_i2c_act->i2c_bus_type);
        return (FALSE);
    }
    dev_addr = n2g_i2c_act->i2c_dev;

    usleep(ACT_RW_DELAY);

    /* below comments is leveraged from overlord*/
    /* The chip is not accessed until 5 seconds after the chip Power On/Reset 
       sequence completes. */

    res = set_i2c_slave_addr(fp, dev_addr);
    if (res != SUCCEED) {
        printf("\n[%s]:%d, Error: unable to connect dev:%#x with rc:%d",
                __FUNCTION__, __LINE__, dev_addr, res);
        return (FAILED);
    }

    for (ix = 0; ix < ACT_RETRY; ix++) {
        res = read(fp, read_buffer, bytes_to_read);     // was bytes_to_read
        if (res == -1) {
            msleep(100);
            ix ++;
            continue;
        } else {
            break;
        }
    }

    if (ix >= ACT_RETRY) {
        printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n", 
                errno);
        printf("Something went wrong with read()! Error description is: %s\n", 
                strerror(errno));
        fflush(stdout);
        return (-1);
    }

    if (act2_i2c_debug) {
        printf("[%s]:%d fp:%d, dev_addr:0x%x, bus:%d\n", 
                __FUNCTION__, __LINE__, fp, dev_addr, n2g_i2c_act->i2c_bus_type);

        printf("\n Inside tam_lib_platform_read. length = 0x%x \n",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_read; ix++) {
            printf(" %02x", read_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }

    usleep(ACT_RW_DELAY);
    *bytes_actually_read = bytes_to_read;

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : is_tam_aikido_mbox_on
 * Description: Return TRUE if mailbox in Aikido is turned on
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_tam_aikido_mbox_on (void)
{
    return (aikido_mailbox_flag);
}


/*-------------------------------------------------------------------
 *
 * Function : is_tam_aikido_on
 * Description: Return TRUE if ACT2 chip is Aikido
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_tam_aikido_on (void)
{
    int ret = FALSE;

    if ((aikido_act2_flag == TRUE) || (aikido_mailbox_flag == TRUE)) {
        ret = TRUE;
    }

    return (ret);
}


/*-------------------------------------------------------------------
 *
 * Function: reset_plat_dev
 * reset device on control plan
 *
 * Input: bit mask representing device to be reset
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void reset_plat_dev(unsigned int mask)
{
    (void)hr_cpld_reset_act2();
}

/*-------------------------------------------------------------------
 *
 * Function: unreset_plat_dev
 * unreset device on control plan
 *
 * Input: bit mask representing device to be reset
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void unreset_plat_dev(unsigned int mask)
{
    (void)hr_cpld_unreset_act2();
}


/*
 * Function: tam_lib_platform_mbx_write 
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary. 
 *
 * Inputs: ptr to platform_opaque_handle(module) -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: number of bytes written
 *         -1 error
 */
int32_t
tam_lib_platform_mbx_write(void *platform_opaque_handle,
                           uint16_t bytes_to_send,
                           uint8_t *send_buffer)
{
    return bytes_to_send;
}

/*
 * Function: tam_lib_platform_mbx_read 
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary.
 *
 * Inputs: ptr to module(platform_opaque_handle) -
 *         min_time - min time to wait (for future use)
 *         max_time - max time to wait (for future use)
 *         ptr to status_buf(receive_buf) -
 *         length - number of bytes to read.
 *         bytes_actually_read - number of bytes actually read.
 *
 * Output: TAM_RC_OK successfully
 *         -1   failed
 */
tam_lib_status_t
tam_lib_platform_mbx_read(void *platform_opaque_handle,
                          uint16_t bytes_to_send,
                          uint8_t *send_buffer,
                          uint16_t bytes_to_read,
                          uint8_t *read_buffer,
                          uint16_t *bytes_actually_read)
{
    return TAM_RC_OK;
}

/*-------------------------------------------------
 * $Log: tam_act2_api_drv_support.c,v $
 * Revision 1.1  2020/08/19 09:50:05  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

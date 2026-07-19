 /* $Id: tam_act2_api_drv_support.c,v 1.3 2018/08/14 22:47:13 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/tam_act2_api_drv_support.c,v $
 *------------------------------------------------------------------
 *
 * tam_act2_api_drv_support.c :
 *
 * This file provides the functions to support the API developed for ACT2 chip.
 *
 * DEC 2014 - Ian Chang
 * Ported by Kody Ko
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "common.h"
#include "menu.h"
#include "proto.h"
#include "n2g_api_rc.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "platform_i2c.h"
#include "nmc93c46.h"           /* pvdm need pas_managemtn in sc_context struct */
#include "smart_cookie.h"
#include "act2_utils.h"
#include "diag_fpga.h"
#include "diag_i2c_addr.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "defs.h"
#include "error.h"
#include "tam_act2_api_drv_support.h"
#include "tam_library.h"
#include "platform_cpu.h"
#include "diag_fpga.h"
#include "diag_common.h"


/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int diagact2_lib_initialize(int addr);
int tam_act2_i2c_initialize(void);

/*
 * Hardware supports 512 byte fifo.
 * Currently act2 library sends at most 50 bytes
 */
extern int act2_i2c_debug;

/*
 * Function: act2_drv_read
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 *
 * Inputs: ptr to module -
 *         ptr to status_buf -
 *         length - number of bytes to read.
 *
 * Output: PASSED/FAILED
 */
int
act2_drv_read (void *module, char *receive_buf, unsigned int length)
{
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;
    int ret_val = 0;
    int size;
    uint ix;

    memset(receive_buf,0xCC,length);
    
    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();

    assert(n2g_i2c_act);

    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));
    
    i2c_if.buf = receive_buf;
    size = (int)length;

    if (act2_i2c_debug) {
        printf(" act2_api_drv_support.c read :i2c_ctrl %d; addr %#x; mux %d; len %d\n",
               i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, length);
    }

    while (size) {
        if (size > SEGMENT_I2C_READ) {
            i2c_if.size = SEGMENT_I2C_READ;
        } else {
            i2c_if.size = size;
        }
        size -= i2c_if.size;
        
        for (ix = 0; ix < ACT_RETRY; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != RC_I2C_OP_OK) {
                msleep(ACT_DELAY);
                continue;
            } else {
                break;
            }
        }
        if (ret_val != RC_I2C_OP_OK) {
            cterr('f',0,"READ ERROR in act2_drv_read: error %d", ret_val);
            return(0);
        }
        //         msleep(1000);
        i2c_if.buf += SEGMENT_I2C_READ;
    } 

    if (act2_i2c_debug) {
        printf("\n");
        printf("\n ----- receive buf within act2_drv_read --------\n");
        for (ix =0; ix < length; ix++) {
            printf(" %02x ", receive_buf[ix] & 0xFF);
            if (ix && (ix % SEGMENT_I2C_READ == 0))
                printf("\n"); 
        }
        printf("\n ----------------------------------------------\n");
    }

    return(length);
}

/*
 * Function: act2_drv_write
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 *
 * Inputs: ptr to module -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: PASSED/FAILED
 */
int
act2_drv_write (void *module, char *send_buf, unsigned int length)
{
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;
    int ret_val = 0;

    if (!length) {
        cterr('f',0," Error in act2_drv_write. length = %d", length);
        return(FAILED);
    }

    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();

    assert(n2g_i2c_act);
    
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));
    i2c_if.size = length;
    i2c_if.buf = send_buf;
    if (act2_i2c_debug) {
       uint ix;
       printf(" act2_api_drv_support.c write :i2c_ctrl %d; addr %#x; mux %d; len %d\n",
           i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, i2c_if.size);

        printf("\n");
        printf("\n ----- write buf within act2_drv_write --------\n");
        for (ix =0; ix < length; ix++) {
            printf(" %02x ", send_buf[ix]);
            if (ix && (ix % SEGMENT_I2C_READ == 0))
                printf("\n"); 
        }
        printf("\n ----------------------------------------------\n");
    }
    ret_val = n2g_i2c_write(&i2c_if);
    if (ret_val == RC_I2C_SLV_NACK) {
        printf("ACT2 NACK, retry\n");
        return(0);
    } else if (ret_val != RC_I2C_OP_OK) {
        cterr('f',0," ERROR in act2_drv_write");
        return(0);
    } else {
        return(length); // No error detected 
    }
  
}
/*
 * Function: tam_act2_i2c_initialize()
 *
 * This function initializes the library by
 * setting the I2C bus number that the device is on
 * and which is used for subsequent library calls.
 *
 * Input: None
 * 
 * Output: PASSED if initialized successfully, FAILED otherwise
 */
int tam_act2_i2c_initialize (void)
{
    return (diagact2_lib_initialize(MB_I2C_ADDR_ACT2));
}

/*
 * Function: diagact2_lib_initialize()
 *
 * This function initializes the library by
 * setting the I2C bus number that the device is on
 * and which is used for subsequent library calls.
 *
 * Input: i2c_adapter - /dev/i2c-<bus number>
 *
 * Output: PASSED if initialized successfully, FAILED otherwise
 */
int diagact2_lib_initialize (int addr)
{
    n2g_i2c_if_t *i2c_if;
    
    /*
     * init i2c_if for I2C
     */
    i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ZERO, I2C_MUX_ZERO, addr);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }
    if (addr == MB_I2C_ADDR_ACT2) {
        addr = MB_I2C_ADDR_ACT2;
    }
    if (act2_i2c_debug) {
        printf("\nI2C addr: 0x%x ; %s\n",addr, __FUNCTION__);
    }

    return (PASSED);
}

/*
 * Function: tam_lib_platform_write 
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/x86/victory/tam_library_w_standalone_victory_x86_64.a

 *
 * Inputs: ptr to platform_opaque_handle(module) -
 *         ptr to send_buf -
 *         length - number of bytes to write.
 *
 * Output: number of bytes written
 *         -1 error
 */

int32_t tam_lib_platform_write(void *platform_opaque_handle,
                       uint8_t * send_buf, uint32_t length)
{
    int ret;
    uint16_t ix;
    /* The chip is not accessed until 5 seconds after the chip Power On/Reset 
       sequence completes. */

    if (act2_i2c_debug) {
        printf("tam_lib_platform_write data length %d\n", length);
        for (ix =0; ix < length; ix++) {
            printf("%02X ", send_buf[ix]);
        }
        printf("\n");
    }    


    for (ix = 0; ix < 50; ix++) {
        ret = act2_drv_write(platform_opaque_handle, (char *)send_buf, length); 
        if (ret == length) {
            break;
        }
        msleep(100);
    }
    if (ix >= 50) {
        printf("\n!!!! msleep %d over Max mini-seconds: %s !!!!",ix * 100,
                __FUNCTION__);
    } else if (ix > 0) {
        printf("\nmsleep %d min-seconds: %s",ix * 100,__FUNCTION__);
    }

    return (ret == length)? length : -1;
}

/*
 * Function: tam_lib_platform_read 
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
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
tam_lib_status_t tam_lib_platform_read (void *platform_opaque_handle,
                      uint32_t min_time,
                      uint32_t max_time,
                      uint8_t * receive_buf,
                      uint16_t length,
                      uint16_t * bytes_actually_read)
{
    uint16_t ret, ix;
    /* The chip is not accessed until 5 seconds after the chip Power On/Reset 
       sequence completes. */
    for (ix = 0; ix < 50; ix++) {
        ret = act2_drv_read(platform_opaque_handle, (char *)receive_buf, length); 
        if (ret != FALSE) {
            break;
        }
        msleep(100);
    }
    if (ix >= 50) {
        printf("\n!!!! msleep %d over Max mini-seconds: %s !!!!",ix * 100,
                __FUNCTION__);
    } else if (ix > 0) {
        printf("\nmsleep %d min-seconds: %s",ix * 100,__FUNCTION__);
    }

    *bytes_actually_read = ret;
    if (act2_i2c_debug) {
        printf("tam_lib_platform_read ret %d bytes_actually_read %d\n", 
	        ret, *bytes_actually_read);
        for (ix =0; ix < length; ix++) {
            printf("%02X ", receive_buf[ix]);
        }
        printf("\n");
    }    
    return (ret) ? TAM_RC_OK : -1;
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
    /* Act2 initialization reset sequence */
    /* DSL SKU reset the ACT2 */
    fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_ACT2_RST_L, TRUE,
                          WAITTIME_5_MS);
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
    /* Act2 initialization reset sequence */
    /* un-reset the ACT2 */
    /* DOC-699608: The chip is not accessed until 5 seconds after  */ 
    /* the chip Power On/Reset sequence or after the chip is Reset; */
    fpga_reset_api(FPGA_EXTER_DEV_RST_REG, FPGA_ACT2_RST_L, FALSE,
                   SLEEP_5S);
}

/*
 *------------------------------------------------------------------
 * $Log: tam_act2_api_drv_support.c,v $
 * Revision 1.3  2018/08/14 22:47:13  iachang
 * CSCvk51378: Supportted Act2 SUDI 2099, please refer to Viper Makefile.
 *
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.4  2018/06/14 01:28:46  harrchan
 * Remove Aikido option and Aikido keyword in whole source code
 *
 * Revision 1.1.2.3  2018/06/01 08:13:21  olin2
 * Enhance ACT2 I2C write
 *
 * Revision 1.1.2.2  2018/04/10 06:23:12  harrchan
 * Fix the bug of tam lib read/write
 *
 * Revision 1.1.2.1  2018/02/27 08:06:53  harrchan
 * Initial viper application code base
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
 

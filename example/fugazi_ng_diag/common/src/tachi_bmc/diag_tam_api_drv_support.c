/* $Id: diag_tam_api_drv_support.c,v 1.2 2016/04/20 11:37:00 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_tam_api_drv_support.c,v $
 *------------------------------------------------------------------
 *
 * diag_tam_api_drv_support.c - TAM API Driver support
 * 
 * July 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "proto.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "n2g_api_rc.h"
#include "diag_tam_api_drv_support.h"
#include "diag_i2c_api.h"
#include <tam_library.h>
#include "diag_fpga_lib.h"
#include "queryflags.h"

int32_t tam_lib_platform_write(void *, uint8_t *, uint32_t);
tam_lib_status_t tam_lib_platform_read(void *, uint32_t, uint32_t, uint8_t *, uint16_t , 
                                       uint16_t *);
void reset_plat_dev(unsigned int);
void unreset_plat_dev(unsigned int);
void reset_isp_dev(unsigned int);
void unreset_isp_dev(unsigned int);

int act2_drv_write(void *, char *, unsigned int);
int act2_drv_read(void *, char *, unsigned int);

extern void *tam_act2_get_n2g_i2c_if(void);

extern int act2_i2c_debug;

void reset_plat_dev (unsigned int mask)
{
	diag_fpga_ext_reset(FPGA_ACT2_RESET);
}

void unreset_plat_dev (unsigned int mask)
{
	diag_fpga_ext_unreset(FPGA_ACT2_RESET);
}

void reset_isp_dev (unsigned int mask)
{
    diag_fpga_reg_write(mask, DC_ACT2_RESET);
}
void unreset_isp_dev (unsigned int mask)
{
    diag_fpga_reg_write(mask, DC_ACT2_UNREST);
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

int32_t tam_lib_platform_write (void *platform_opaque_handle,
                                uint8_t *send_buf, uint32_t length)
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


    for (ix = 0; ix < ACT2_RW_RETRY; ix++) {
        ret = act2_drv_write(platform_opaque_handle, (char *)send_buf, length); 
        if (ret == length) {
            break;
        }
        msleep(100);
    }
    if (ix >= ACT2_RW_RETRY) {
        printf("\n!!!! msleep %d over Max mini-seconds: %s !!!!",ix * 100,
                __FUNCTION__);
    } else if (ix > 0) {
        printf("\nmsleep %d min-seconds: %s",ix * 100,__FUNCTION__);
    }

    return (ret == length)? length : -1;
}


/*
 * Function: tam_lib_status_t 
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
tam_lib_status_t
tam_lib_platform_read (void *platform_opaque_handle,
                       uint32_t min_time,
                       uint32_t max_time,
                       uint8_t *receive_buf, 
                       uint16_t length,
                       uint16_t * bytes_actually_read)
{
    uint16_t ret, ix;

    /* The chip is not accessed until 5 seconds after the chip Power On/Reset 
     * sequence completes. */

    for (ix = 0; ix < ACT2_RW_RETRY; ix++) {
        ret = act2_drv_read(platform_opaque_handle, (char *)receive_buf, length);
        if (ret != FALSE) {
            break;
        }

        msleep(100);
    }

    if (ix >= ACT2_RW_RETRY) {
        printf("\n!!!! msleep %d over Max mini-seconds: %s !!!!",ix * 100,
                                                                 __FUNCTION__);        
    } else if (ix > 0) {
        printf("\nmsleep %d min-seconds: %s",ix * 100,__FUNCTION__);
    }

    *bytes_actually_read = ret;

    if (act2_i2c_debug) {
        printf("%s: ret %d bytes_actually_read %d\n", __FUNCTION__, 
                ret, *bytes_actually_read);
        for (ix =0; ix < length; ix++) {
            printf("%02X ", receive_buf[ix]);
        }
        printf("\n");
    }

    return (ret) ? TAM_RC_OK : -1;
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
int act2_drv_write (void *module, char *send_buf, unsigned int length)
{
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;
    int ret_val = 0;

    if (!length) {
        printf("%s: Error in act2_drv_write. length = %d\n", __FUNCTION__, length);
        return (FAILED);
    }

    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();

    if (n2g_i2c_act == NULL) {
        printf("%s: n2g_i2c_act returns NULL\n", __func__);
        return (FAILED);
    }
    
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));

    i2c_if.size = length;
    i2c_if.buf = send_buf;

    if (act2_i2c_debug) {
        uint ix;

        printf("%s:i2c_ctrl %d; addr %#x; mux %d; len %d\n", __FUNCTION__, 
               i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, i2c_if.size);

        printf("\n");
        printf("\n ----- write buf within act2_drv_write --------\n");
        for (ix =0; ix < length; ix++) {
            printf(" %02x ", send_buf[ix]);
            if (ix && (ix % SEGMENT_I2C_READ == 0)) {
                printf("\n");
            }
        }
        printf("\n ----------------------------------------------\n");
    }

    ret_val = n2g_i2c_write(&i2c_if);
    if (ret_val != RC_I2C_OP_OK) {
        printf("%s: ERROR in ACT2 write\n", __FUNCTION__);
        return (FAILED);
    }
    
    return (length);
}


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
int act2_drv_read (void *module, char *receive_buf, unsigned int length)
{
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;
    int ret_val = 0;
    int size;
    uint ix;

    memset(receive_buf,0xCC,length);
    
    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();

    if (n2g_i2c_act == NULL) {
        printf("%s: n2g_i2c_act returns NULL\n", __func__);
        return (FAILED);
    }
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));

    i2c_if.buf = receive_buf;
    size = (int)length;

    if (act2_i2c_debug) {
        printf("%s :i2c_ctrl %d; addr %#x; mux %d; len %d\n", __FUNCTION__, 
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
            printf("%s: READ Error: Error %d\n", __FUNCTION__, ret_val);
            return (0);
        }

        i2c_if.buf += SEGMENT_I2C_READ;
    }

    if (act2_i2c_debug) {
        printf("\n");
        printf("\n ----- receive buf within %s --------\n", __FUNCTION__);
        for (ix =0; ix < length; ix++) {
            printf(" %02x ", receive_buf[ix] & 0xFF);
            if (ix && (ix % SEGMENT_I2C_READ == 0)) {
                printf("\n");
            }
        }

        printf("\n ----------------------------------------------\n");
    }
    return (length);
}

/*---------------------------------------------------------------
$Log: diag_tam_api_drv_support.c,v $
Revision 1.2  2016/04/20 11:37:00  benchen2
merge tachi to main trunk

Revision 1.1.2.6  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.1.2.5  2016/01/26 06:27:55  benchen2
add daughter card ACT2 programming

Revision 1.1.2.4  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.3  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.2  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/


/* $Id: act2_api_drv_support.c,v 1.3 2014/02/13 19:54:41 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/act2_api_drv_support.c,v $
 *----------------------------------------------------------------------------
 * act2_api_drv_support.c
 *
 * This file provides the functions to support the API developed for ACT2 chip.
 *
 * May 2011: Alan O'Sullivan
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *----------------------------------------------------------------------------
 */

#include "types.h"
#include "common.h"
#include "menu.h"
#include "proto.h"
#include "n2g_api_rc.h"
#include "i2c_api.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "platform_i2c.h"
#include "act2_api_drv_support.h"
#include "nmc93c46.h"  /* pvdm need pas_managemtn in sc_context struct */
#include "smart_cookie.h"
#include "act2_utils.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* hardware supports 512 byte fifo. currently act2 library sends at most 50 bytes
 */
#define SEGMENT_I2C_READ   511
//#define SEGMENT_I2C_READ   30
#define ACT_RETRY  30
#define ACT_DELAY 200
extern int act2_i2c_debug;
/*
 * Function: act2_drv_write
 *
 * This function does the write to the Ruby/ACT2 device. Reggio FPGA is the master.
 * Calls are made to this function from within the act2 libary located at.
 * ../common/chips/lib/act2/lib/lib_act2.a
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
    
    //    assert(con_p);
    if (!length) {
        cterr('f',0," Error in act2_drv_write. length = %d", length);
        return(FAILED);
    }

    n2g_i2c_act = (n2g_i2c_if_t *)act2_get_n2g_i2c_if();

    assert(n2g_i2c_act);
    
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));
    i2c_if.size = length;
    i2c_if.buf = send_buf;
    if (act2_i2c_debug) {
        printf(" act2_api_drv_support.c write :i2c_ctrl %d; addr %#x; mux %d; len %d\n",
           i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, i2c_if.size);
    }
    ret_val = n2g_i2c_write(&i2c_if);
    if (ret_val != RC_I2C_OP_OK) {
        cterr('f',0," ERROR in act2_drv_write");
        return(0);
    } else {
        //      msleep(200);
        int i;
        if (act2_i2c_debug) {
            printf("\n");
            printf("\n ----- write buf within act2_drv_write --------\n");
            for (i =0; i < length; i++) {
                printf(" %02x ",     i2c_if.buf[i] & 0xff);
                if (i && (i % SEGMENT_I2C_READ == 0))
                    printf("\n"); 
            }
            printf("\n ----------------------------------------------\n");
        }
        return(length); // No error detected 
    }
  
}

/*
 * Function: act2_drv_read
 *
 * This function does the read from the Ruby/ACT2 device. Reggio FPGA is the master.
 * Calls are made to this function from within the act2 libary located at.
 * ../common/chips/lib/act2/lib/lib_act2.a
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
    uint i;

    memset(receive_buf,0xCC,length);
    
    n2g_i2c_act = (n2g_i2c_if_t *)act2_get_n2g_i2c_if();

    assert(n2g_i2c_act);

    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));
    
    i2c_if.buf = receive_buf;
    size = (int)length;

#if 0
    printf(" act2_api_drv_support.c read :i2c_ctrl %d; addr %#x; mux %d; len %d\n",
           i2c_if.i2c_ctrl, i2c_if.i2c_dev, i2c_if.mux, length);
#endif

    while (size) {
        if (size > SEGMENT_I2C_READ) {
            i2c_if.size = SEGMENT_I2C_READ;
        } else {
            i2c_if.size = size;
        }
        size -= i2c_if.size;
        
        for (i = 0; i < ACT_RETRY; i++) {
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
        for (i =0; i < length; i++) {
            printf(" %02x ", receive_buf[i] & 0xFF);
            if (i && (i % SEGMENT_I2C_READ == 0))
                printf("\n"); 
        }
        printf("\n ----------------------------------------------\n");
    }

    return(length);
}


/******** History ******** 
$Log: act2_api_drv_support.c,v $
Revision 1.3  2014/02/13 19:54:41  mcharon
add debug flag for act2

Revision 1.2  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:35  alpeng
moving overlord common code from x86

Revision 1.4  2012/11/07 18:21:17  mcharon
cleanup


Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/

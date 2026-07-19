/* $Id: tam_act2_api_drv_support.c,v 1.5 2019/08/06 06:56:10 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/tam_act2_api_drv_support.c,v $
 *------------------------------------------------------------------
 * FILE NAME : tam_act2_api_drv_support.c
 *
 * This file provides the functions to support the API developed for ACT2 chip.
 *
 * DEC 2014 - Ian Chang
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "platform_i2c.h"
#include "nmc93c46.h"  /* pvdm need pas_managemtn in sc_context struct */
#include "smart_cookie.h"
#include "act2_utils.h"
#include "tam_library.h"
#include "tam_aikido_mailbox.h"       

/* hardware supports 512 byte fifo. currently act2 library sends at most 50 bytes
 */
#define SEGMENT_I2C_READ   511
#define ACT_RETRY  30
#define ACT_DELAY 200
extern int act2_i2c_debug;
extern void *tam_act2_get_n2g_i2c_if(void);
extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag;
extern unsigned int aikido_spi_write(unsigned int, unsigned int, 
                              unsigned int, unsigned int, 
                              unsigned char *); 
extern unsigned int aikido_spi_read(unsigned int, unsigned int, 
                              unsigned int, unsigned int, 
                              unsigned char *); 

int is_tam_aikido_mbox_on(void);
int is_tam_aikido_on(void);

/*-------------------------------------------------------------------
 *
 * Function : aikido_spi_write
 * Description: a weak func for older platforms. 
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
unsigned int aikido_spi_write (unsigned int var1, unsigned int var2,
                               unsigned int var3, unsigned int var4,
                               unsigned char *var5)
    __attribute__((weak, alias("__aikido_spi_write"))); 
unsigned int __aikido_spi_write (unsigned int var1, unsigned int var2,
                        unsigned int var3, unsigned int var4,
                        unsigned char *var5)
{
    printf("%s : is not support \n", __FUNCTION__); 
    return (-1); 
}

/*-------------------------------------------------------------------
 *
 * Function : aikido_spi_read
 * Description: a weak func for older platforms. 
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
unsigned int aikido_spi_read (unsigned int var1, unsigned int var2,
                              unsigned int var3, unsigned int var4,
                              unsigned char *var5)
    __attribute__((weak, alias("__aikido_spi_read"))); 
unsigned int __aikido_spi_read (unsigned int var1, unsigned int var2,
                                unsigned int var3, unsigned int var4,
                                unsigned char *var5)
{
    printf("%s : is not support \n", __FUNCTION__); 
    return (-1); 
}

/*
 * Function: tam_act2_i2c_initialize
 *
 * A blank function, it is used for tam_act2_util, we dont need it 
 *
 * Inputs: NONE
 * Output: PASSED
 */
int tam_act2_i2c_initialize(void)
{
    return (PASSED); 

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

    if (is_tam_aikido_on()) {
        printf("AIKIDO is using mbox, never come here \n"); 
        cterr('f',0,"READ ERROR in act2_drv_write"); 
        return (0);  /* error */
    } else {

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
        if (ret_val != RC_I2C_OP_OK) {
            cterr('f',0," ERROR in act2_drv_write");
            return(0);
        } else {
            return(length); // No error detected 
        }
  
   } /* is_tam_aikido_on */
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

int32_t
tam_lib_platform_write (void *platform_opaque_handle, uint8_t *send_buf, 
			uint32_t length)
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


    if (is_tam_aikido_on()) {
        printf("AIKIDO is using mbox, never come here \n"); 
        cterr('f',0,"READ ERROR in act2_drv_read"); 
        return (0);  /* error */
    } else {

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
        
            msleep(ACT_DELAY); //afix a8
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
    } /*  is_tam_aikido_on */
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
		       uint16_t *bytes_actually_read)
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


#ifdef AIKIDO_ACT2
/*
 * Function: tam_lib_platform_mbx_write 
 *
 * This function does the write to the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_library_w_standalone_tsn_diag.a
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
    unsigned char *ptr; 
    unsigned int address, ix, ret_val; 

    if (act2_i2c_debug) {
        printf("\n\n Inside tam_lib_platform_mbx_write. length = 0x%x \n",
               bytes_to_send);
        for (ix = 0; ix < bytes_to_send; ix++) {
            printf(" %02x", send_buffer[ix]);
            if ((ix != 0) && (ix % 16 == 0)) {
                printf("\n");
            }
        }
        printf("\nend of dump write buf\n");
        fflush(stdout);
    }

    ptr = send_buffer; 
 
    /* address = send_buf[0] << 24 | send_buf[1] << 16 | 
     *           send_buf[2] << 8  | send_buf[3] ; 
     */
    for (ix = 0; ix < 4; ix++) {
        address = address << 8;
        address |= *ptr; 
        ptr++; 
    }

    address = address & 0x00FFFFFF; /* follow tsn */

    /* aikido_spi_write(size, address, spi_op, is_address_field, write_buf) */
    /* size : bytes_to_send -1 */ 
    /* spi_op : 0x2 for write, don't care */ 
    /* is_address_field : TRUE */
    /* ptr : the remaining is the data, send_buf[4],[5],[6],...*/
    if (act2_i2c_debug) {
       printf("\n=== start spi write===\n"); 
    }
    /* -4 bytes, address; -1 byte, fpga spec */
    ret_val = aikido_spi_write(bytes_to_send - 1 - 4, address, 0x2,
                               TRUE, ptr); 
    if (act2_i2c_debug) {
       printf("\n ===end spi write===\n"); 
    }
        
    if (ret_val != PASSED) { 
        return(-1); /* return -1, follow tsn */
    }

    return(bytes_to_send); /* pass, return length, follow tsn */
}

/*
 * Function: tam_lib_platform_mbx_read 
 *
 * This function does the read from the ACT2 device. The FPGA is the master.
 * Calls are made to this function from within the act2 tam libary located at.
 * /auto/sp-engops/diags/pld/act2lite/ARM/TSN/tam_library_w_standalone_tsn_diag.a
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
    unsigned int address, ix, jx;
    unsigned int byte_count = 0, byte_shift = 0;
    unsigned char *ptr, buf[3000]; 

    if (act2_i2c_debug) {
        printf("\n\n Inside tam_lib_platform_mbx_read. length = 0x%x \n",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_send; ix++) {
            printf(" %02x", send_buffer[ix]);
        }
        fflush(stdout);
    }

    ptr = send_buffer; 
 
    /* send_buf first 4 bytes is address 
     * address = send_buf[0] << 24 | send_buf[1] << 16 | 
     *           send_buf[2] << 8  | send_buf[3] ; 
     * address = address & 0x00FFFFFF //following tsn 
     * byte_shift = address % 4; // aligned 4 bytes. 
     * address -= address % 4;  // aligned 4 bytes 
     * byte_count = bytes_to_read - 4 + byte_shift;  // aligned 4 bytes 
     *
     */
    for (ix = 0; ix < 4; ix++) {
        address = address << 8;
        address |= *ptr; 
        ptr++; 
    } 

    address = address & 0x00FFFFFF; /* following tsn */
    byte_shift = address % 4; /* aligned 4 bytes. */
    address -= address % 4;  /* aligned 4 bytes */

    /* address is aligned, need to read more bytes for read full data */
    byte_count = bytes_to_read - 4 + byte_shift;  // 8 -4(addr) + aligned 

    
    /* Read buffer header FF FF FF FF, tsn */
    for (jx = 0; jx < 4; jx++) {
        read_buffer[jx] = 0xFF;
    }

    /* size, address, op = don't care, is_addr = 1(true), buf */
    /* byte_count - 1, due to fpga definition, 
     * read 1 byte size is 0, read 100 bytes size is 99, and so on. */
    if (act2_i2c_debug) {
       printf("\n=== start spi read=== \n"); 
    }
    aikido_spi_read(byte_count - 1, address, 0, 1, buf); // -1 for fpga
    if (act2_i2c_debug) {
       printf("\n=== end spi read=== \n"); 
    }

    for (jx = 0; jx < byte_count - byte_shift; jx++) {
        read_buffer[4 + jx] = buf[jx + byte_shift];  /*jx + byte shift */
    }

    *bytes_actually_read = bytes_to_read; 

    if (act2_i2c_debug) {
        printf("\n Actually read length = 0x%x : ", bytes_to_read); 
        for (ix = 0; ix < bytes_to_read ; ix++) { 
            if (ix % 16 == 0) {
                printf("\n ");
            }
            printf(" %02x", read_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }

    return (TAM_RC_OK); 
}
#endif /* -DAIKIDO_ACT2 */

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
 * Function : tam_aikido_reset_utilty
 * Description: an utility for soft reset aikido and check status.
 * INPUT:  None
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
void tam_aikido_reset_utilty (void) 
{
    unsigned int size, address, op, is_addr, ix, tmp = 0;
    unsigned char buf[32]; 

    size = 4 - 1; 
    address = AIKIDO_TAM_RESOURCE;  
    op = 0; 
    is_addr = 1; 
    buf[0] = AIKIDO_TAM_RESET; 
    buf[1] = 0; 
    buf[2] = 0; 
    buf[3] = 0; 
    
    printf("Note: Aikido Reset is able to unreset automatically \n"); 
    printf("Reset: write 0x37@0x2000 \n"); 
    aikido_spi_write(size, address, op, is_addr, buf); 
    /* Aikido team suggest take 1 sec. delay when issue the reset.
     */
    msleep(TAM_AIKIDO_RESET_DELAY);

    /* polling status register for bit16 and bit19, 
     * soft reset event and fw ready, respectively. 
     */
    address = AIKIDO_TAM_STATUS; /* 200C */
    size = 4 - 1; 
    op = 0; 
    is_addr = 1; 
    aikido_spi_read(size, address, op, is_addr, buf); 
    printf("Check reset result: read debug register 0x200C : "); 
    for (ix = 0; ix <= size; ix++) {
        printf("%02x ", buf[ix]); 
    }

    tmp = buf[2]; /* buf[2] for bit[19:16] */
    if (tmp << 16 & AIKIDO_TAM_READY) {
        printf("\nAIKIDO FW is ready after reset \n"); 
    } else {
        printf("\nError : Status reg bit[19:16] = 0x%x\n", tmp); 
        printf("Bit19-fw ready or Bit16-soft reset event is not set\n"); 
    }

    return; 

}

/*
 *------------------------------------------------------------------
 * $Log: tam_act2_api_drv_support.c,v $
 * Revision 1.5  2019/08/06 06:56:10  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.4  2018/08/25 01:01:01  ptong
 * Turn on 2099 SUDI support. Bump diag version to 4.3.0
 *
 * Revision 1.3  2018/05/28 07:31:29  alpeng
 * Change back to use 2029-SUDI TAM lib from 2099-SUDI TAM lib until IOS is ready.
 *
 *
 * Revision 1.2  2015/02/13 02:33:38  iachang
 * Supported Act2 TAM library
 *
 * Revision 1.1.4.3  2015/01/17 02:39:40  iachang
 * Upgrade TAM library. Built-Jan 9 2015
 *
 * Revision 1.1.4.2  2014/12/17 08:30:12  hondwang
 * sync with maintrunk tag ovld-juno-tag-121714
 *
 * Revision 1.1.2.3  2014/12/15 03:16:17  iachang
 * Refer TAM lib. headfile from /auto/sp-engops/diags/pld/act2lite/x86/victory
 *
 * Revision 1.1.2.2  2014/12/12 08:44:29  iachang
 * Modify the function description
 *
 * Revision 1.1.2.1  2014/12/11 09:52:12  iachang
 * Supported Longer PID With TAM lib.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
 

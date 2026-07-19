/* $Id: tam_act2_api_drv_support.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/tam_act2_api_drv_support.c,v $
 *------------------------------------------------------------------
 *
 * tam_act2_api_drv_support.c :
 *
 * This file provides the functions to support the API developed for ACT2 chip.
 *
 * DEC 2014 - Ian Chang
 * Ported by Kody Ko
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
#include "diag_fpga.h"
#include "diag_common.h"
#include "dash_fpga.h"
#include "diag_cpld_lib.h"
#include "cookie_4.h"
#include "cross_platform.h"
#include "tam_aikido_mailbox.h"


/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int diagact2_lib_initialize(int addr);
int tam_act2_i2c_initialize(void);
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


/*
 * Hardware supports 512 byte fifo.
 * Currently act2 library sends at most 50 bytes
 */
extern int act2_i2c_debug;

static sc_context *con, cont;
static dev_if_info_t dev_if;


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

    if (is_tam_aikido_mbox_on()) {
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

    if (is_tam_aikido_mbox_on()) {
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
        if (ret_val == RC_I2C_SLV_NACK) {
            printf("ACT2 NACK, retry\n");
            return(0);
        } else if (ret_val != RC_I2C_OP_OK) {
            cterr('f',0," ERROR in act2_drv_write");
            return(0);
        } else {
            return(length); // No error detected 
        }
    }  /* is_tam_aikido_on */  
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
    if (is_tam_aikido_mbox_on()) {
        printf("AIKIDO is using mbox, never come here \n");
        cterr('f',0,"READ ERROR in tam_act2_i2c_initialize");
        return (FAILED);
    }

    if (is_tam_aikido_on()) {
        return (diagact2_lib_initialize(MB_I2C_ADDR_AIKIDO));
    } else {
        return (diagact2_lib_initialize(MB_I2C_ADDR_ACT2));
    }
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
    uchar cookie[COOKIE_SIZE_512];

    con = &cont;
    con->slot = 0;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    plat_init_smart_eeprom_context(con, MOTHER_BOARD, 0, cookie);
    act2_init_cont(con);
    
    /*
     * init i2c_if for I2C
     */
    i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ZERO, I2C_MUX_ZERO, addr);
    if (i2c_if == NULL) {
        printf("%s: Failed to get I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
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
        }
        printf("\n");
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

    /* to avoid effect previous running projects, 
     * we skip aikido here instead of common code; 
     * Since SW suggest do not reset AIKIDO, 
     * it is ready after rommon boot up. 
     */
    if ((is_tam_aikido_on() == TRUE) && (mask == FPGA_RST_ACT2)) {
        return ;
    }

    /* Need to be removed after no ACT2 HW */
    /* Act2 initialization reset sequence */
    cpld_reset_api(FPGA_DEV_RST_CTRL, FPGA_RST_ACT2, TRUE,
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
    /* to avoid effect previous running projects, 
     * we skip aikido here instead of common code; 
     * Since SW suggest do not reset AIKIDO, 
     * it is ready after rommon boot up. 
     */
    if ((is_tam_aikido_on() == TRUE) && (mask == FPGA_RST_ACT2)) {
        return ;
    }

    /* Need to be removed after no ACT2 HW */
    /* Act2 initialization reset sequence */
    /* un-reset the ACT2 */
    /* DOC-699608: The chip is not accessed until 5 seconds after  */ 
    /* the chip Power On/Reset sequence or after the chip is Reset; */
    cpld_reset_api(FPGA_DEV_RST_CTRL, FPGA_RST_ACT2, FALSE,
                   SLEEP_5S);
}


/*-------------------------------------------------------------------
 *
 * Function: phoenix_show_aikido_fpga_ver
 *
 *   Show Aikido FPGA version.
 *
 * Input: None
 * Output: None
 *
 *-------------------------------------------------------------------
 */
int phoenix_show_aikido_fpga_ver(void)
{
    uint16_t tmp;
    int ret_val;
    tam_lib_status_t status;
    void *tam_handle = NULL;

    aikido_act2_flag = TRUE;
    aikido_mailbox_flag = TRUE;

    con = &cont;
    con->slot = 0;
    con->dev_if_p = &dev_if;

    act2_init_cont((void *) con);

    reset_plat_dev(FPGA_RST_ACT2);
    unreset_plat_dev(FPGA_RST_ACT2);
    msleep(200);

    if (is_tam_aikido_mbox_on() == TRUE) {
        ret_val = tam_lib_device_open_mailbox((void *)&cont, MBX_USE_INTERRUPT,
                                              MBX_MSG_SIZE, MBX_REG_BASE_ADDR,
                                              &tam_handle);
        if (ret_val != TAM_RC_OK) {
            printf("%s: ERROR: Can't initialize Mailbox. Status: %#x\n",
                    __func__, ret_val);
            return (FAILED);
        }
    } else {
        ret_val = tam_lib_device_open((void *)&cont, 259, &tam_handle);
        if (ret_val != TAM_RC_OK) {
            printf("%s: ERROR: Can't open handler. Status: %#x\n",
                    __func__, ret_val);
            return (FAILED);
        }
    }

    status = tam_lib_get_aikido_fpga_version(tam_handle, &tmp);
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_get_chip_info failed with status 0x%x\n", status);
        return (FAILED);
    } else {
        printf("\nAikido FPGA Version: v%d (0x%x)\n", tmp, tmp);
        return (PASSED);
    }
}

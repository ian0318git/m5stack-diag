/* $Id: tam_act2_api_drv_support.c,v 1.6 2018/11/23 08:49:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/tam_act2_api_drv_support.c,v $
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
#include "platform_fpga.h"
#include "i2c_address.h"
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
#include "platform_cpu.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"
#include "cross_platform.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/
/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern void *tam_act2_get_n2g_i2c_if(void);
extern int fpga_read_32_reg(uint, uint *);
extern int tsn_get_devbus_baseaddr(int, uint *);
/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
static int set_i2c_slave_addr(int addr);
int diagact2_lib_initialize(char *i2c_adapter, int addr);
int tam_act2_i2c_initialize(void);
int is_tam_aikido_mbox_on(void);
int is_tam_aikido_on(void);
void reset_tam_aikido_dev(void);

int act2_drv_write(void *, char *, unsigned int);
int act2_drv_read(void *, char *, unsigned int);

/*
 * Hardware supports 512 byte fifo.
 * Currently act2 library sends at most 50 bytes
 */
extern int act2_i2c_debug;
static int i2c_adapter_fd;
static boolean lib_initialized = FALSE;
extern boolean aikido_act2_flag;
extern boolean aikido_mailbox_flag;  
/*
 * Function: set_i2c_slave_addr()
 * Set ACT2 slave address
 *
 * Input: addr - I2C slave address
 *
 * Output: SUCCEED if set I2C slave address successfully
 */
static
int set_i2c_slave_addr(int addr)
{
    if (ioctl(i2c_adapter_fd, I2C_SLAVE, addr) < 0) {
        printf("%s:ioctl(I2C_SLAVE, 0x%x)", __FUNCTION__, addr);
        return (FAILED);
    }

    return (SUCCEED);
}

/*
 * Function: setact2_i2c_timeout()
 * Extend I2C kernel driver timeout time for ACT2 programming.
 *
 * Input: is_act2 - ACT2 Programming
 *
 * Output: SUCCEED if set I2C timeout successfully
 */
static int setact2_i2c_timeout(boolean is_act2)
{
    if (ioctl(i2c_adapter_fd, I2C_ACT2_TIMEOUT, is_act2) < 0) {
        printf("%s:ioctl(I2C_ACT2_TIMEOUT,%d)", __FUNCTION__, is_act2);
        return (FAILED);
    }

    return (SUCCEED);
}

/*-------------------------------------------------------------------
 *
 * Function: diag_set_i2c_timeout
 * Recovery I2C kernel driver timeout time
 *
 * Input: NONE
 * Output: NONE
 *
 *-------------------------------------------------------------------
 */
void diag_set_i2c_timeout()
{
	char i2c_adapter[] = "/dev/i2c-0";

    /*
     * Open /dev/i2c-%d adapter device
     */
    i2c_adapter_fd = open(i2c_adapter, O_RDWR);
    if (i2c_adapter_fd < 0) {
        cterr('f', 0, "%s:open(%s)", __FUNCTION__, i2c_adapter);
    }

    /* Restore I2C kernel driver timeout to HZ for other I2C tests */
    if (setact2_i2c_timeout(FALSE) != SUCCEED) {
        cterr('f', 0, "\n ERROR: Cannot set i2c timeout.");
    }

    close (i2c_adapter_fd);
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
    if (is_tam_aikido_on() == TRUE) {
        return (diagact2_lib_initialize(TAM_I2C_ADAPTER, MB_I2C_ADDR_AIKIDO_ACT2));
    } else {
        return (diagact2_lib_initialize(TAM_I2C_ADAPTER, MB_I2C_ADDR_ACT2));
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
int diagact2_lib_initialize(char *i2c_adapter, int addr)
{
    /*
     * Open /dev/i2c-%d adapter device 
     */
    i2c_adapter_fd = open(i2c_adapter, O_RDWR);
    if (i2c_adapter_fd < 0) {
        printf("%s:open(%s)", __FUNCTION__, i2c_adapter);
        lib_initialized = FALSE;
        return (FAILED);
    }
    if (addr == MB_I2C_ADDR_ACT2) {
        if (aikido_act2_flag == TRUE) {
            addr = MB_I2C_ADDR_AIKIDO_ACT2;     
        } else {
            addr = MB_I2C_ADDR_ACT2;
        }
    }
    if (act2_i2c_debug) {
        printf("\nI2C addr: 0x%x ; %s\n",addr, __FUNCTION__);
    }
    /*
     * Set the Slave I2C address for the accesses 
     */
    if (set_i2c_slave_addr(addr) != SUCCEED) {
        printf("\n ERROR: Cannot set slave address.");
        close(i2c_adapter_fd);
        lib_initialized = FALSE;
        return (FAILED);
    }

    lib_initialized = TRUE;
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
int act2_drv_write (void *module, char *send_buf, unsigned int length)
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
    if (ret_val != RC_I2C_OP_OK) {
        if (ret_val == RC_I2C_SLV_NACK) {
            return (0);  /* Check Act2 NACK and return */   
        }
        cterr('f',0," ERROR in act2_drv_write");
        return(0);
    } else {
        return(length); // No error detected 
    }
  
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
tam_lib_platform_write(void *platform_opaque_handle,
                       uint8_t * send_buffer, uint32_t length)
{
    int res;
    int ix;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;


    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();
    if (n2g_i2c_act == NULL) {
        printf("%s: n2g_i2c_act returns NULL\n", __func__);
        return (FAILED);
    }
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));

    /*
     * Print one space to solve the corrupt printf buffer, if this is not
     * added, then there will be some weird chars print out.
     */
    printf(" ");
    fflush(stdout);

    if (act2_i2c_debug) {
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

        if (i2c_if.i2c_bus_type == PLUG_FPGA) {
            res = act2_drv_write(platform_opaque_handle, (char *)send_buffer, length);
        } else {
        res = write(i2c_adapter_fd, send_buffer, length);
        }
        
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
tam_lib_platform_read(void *platform_opaque_handle,
                      uint32_t min_time,
                      uint32_t max_time,
                      uint8_t * read_buffer,
                      uint16_t bytes_to_read,
                      uint16_t * bytes_actually_read)
{
    int res, ix;
    uint8_t count = 0;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *n2g_i2c_act;


    n2g_i2c_act = (n2g_i2c_if_t *)tam_act2_get_n2g_i2c_if();
    if (n2g_i2c_act == NULL) {
        printf("%s: n2g_i2c_act returns NULL\n", __func__);
        return (FAILED);
    }
    memcpy(&i2c_if, n2g_i2c_act, sizeof(i2c_if));

    usleep(ACT_RW_DELAY);

retry:

    if (i2c_if.i2c_bus_type == PLUG_FPGA) {
        res = act2_drv_read(platform_opaque_handle, (char *)read_buffer, bytes_to_read);
    } else {
    res = read(i2c_adapter_fd, read_buffer, bytes_to_read);     // was bytes_to_read
    }

   if (res == -1) {
        if (count > ACT_RETRY) {
            printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n", 
                   errno);
            printf("Something went wrong with read()! Error description is: %s\n", 
                   strerror(errno));
            fflush(stdout);
            return (-1);
        } else {
            count++;
            msleep(100);
            goto retry;
        }
    }

    if (act2_i2c_debug) {
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
    int ix = 0;;
    uint32_t offset = 0;
    uint32_t aikido_devbus_baseaddr = 0;
    uint16_t byte_count = 0;
    uint tx_reg_offset = 0;
    uint tx_data = 0;

    /* Get base addr. of device bus. */
    if (tsn_get_devbus_baseaddr(TSN_AIKIDO_DEVBUS_NUM, &aikido_devbus_baseaddr) != PASSED) {
        printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
               __FUNCTION__, __LINE__, TSN_AIKIDO_DEVBUS_NUM);
        return (-1);
    }
    offset |= aikido_devbus_baseaddr;

    if (act2_i2c_debug) {
        printf("\n\n Inside tam_lib_platform_spi_write. length = 0x%x \n",
               bytes_to_send);
        for (ix = 0; ix < bytes_to_send; ix++) {
            printf(" %02x", send_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }

    usleep(ACT_RW_DELAY);
    tx_reg_offset = (send_buffer[0] << 24 |
                     send_buffer[1] << 16 |
                     send_buffer[2] << 8 |
                     send_buffer[3]);
    tx_reg_offset  = tx_reg_offset & 0x00FFFFFF;
    byte_count = bytes_to_send - 4;
    for (ix = 0; byte_count > 0; ix++) {
        fflush(stdout);
        tx_data = (send_buffer[4 + (ix * 4) ] |
                   send_buffer[4 + (ix * 4) + 1] << 8 |
                   send_buffer[4 + (ix * 4) + 2] << 16 |
                   send_buffer[4 + (ix * 4) + 3] << 24);
        if (tsn_mem_write32(offset + tx_reg_offset + (ix * 4) , tx_data) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, 
        			offset + ix);
            return (-1);
    	}
        byte_count -= 4;
    }
    return bytes_to_send;
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
    int ix = 0, jx = 0;
    uint rx_buffer = 0;
    uchar rx_buffer_tmp[TAM_SPI_READ_BUF];
    uint32_t offset = 0;
    uint32_t aikido_devbus_baseaddr = 0;
    uint16_t tx_reg_offset = 0;
    uint16_t byte_count = 0, byte_shift = 0;
   
    /* Get base addr. of device bus. */
    if (tsn_get_devbus_baseaddr(TSN_AIKIDO_DEVBUS_NUM, &aikido_devbus_baseaddr) != PASSED) {
        printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
               __FUNCTION__, __LINE__, TSN_AIKIDO_DEVBUS_NUM);
        return (-1);
    }
    offset |= aikido_devbus_baseaddr;
 
    if (act2_i2c_debug) {
        printf("\n\n Inside tam_lib_platform_spi_read. length = 0x%x \n",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_send; ix++) {
            printf(" %02x", send_buffer[ix]);
        }
        fflush(stdout);
    }
    tx_reg_offset = (send_buffer[0] << 24 |
                     send_buffer[1] << 16 |
                     send_buffer[2] << 8 |
                     send_buffer[3]);
    tx_reg_offset  = tx_reg_offset & 0x00FFFFFF;
    /* Read buffer header FF FF FF FF*/
    for (jx = 0; jx < 4; jx++) {
        read_buffer[jx] = 0xFF;
    }
    byte_shift = tx_reg_offset % 4;
    tx_reg_offset -= (tx_reg_offset % 4);

    byte_count = bytes_to_read - 4 + byte_shift;
    for (ix = 0; byte_count > 0; ix++) {
	    if (tsn_mem_read32(offset + tx_reg_offset + (ix * 4), &rx_buffer) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, 
        			offset + ix);
            return (-1);
    	}
        for (jx = 0; jx < 4; jx++) {
            rx_buffer_tmp[(ix * 4) + jx] = (rx_buffer >> (8 * jx)) & 0xff;
        }
        if (byte_count >= 4) {
            byte_count -= 4;
        } else {
            byte_count = 0;
        }
    }
    for (ix = 0; ix < bytes_to_read - 4; ix++) {
        read_buffer[4 + ix] = rx_buffer_tmp[byte_shift + ix] & 0xff;
    }
    if (act2_i2c_debug) {
        printf("\n Actually read length = 0x%x : ",
               bytes_to_read);
        for (ix = 0; ix < bytes_to_read; ix++) {
            printf(" %02x", read_buffer[ix]);
        }
        printf("\n");
        fflush(stdout);
    }
    *bytes_actually_read = bytes_to_read;
    return TAM_RC_OK;
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
 * Function : reset_tam_aikido_dev
 * Description: This function  resets Aikido device
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 *-------------------------------------------------------------------
 */
void reset_tam_aikido_dev (void)
{
    /* Writing a 0x37 will cause the TAM (MSS subsystem) to reinitialize 
       via a TAM_SOFT_RESET_IRQ */
    uint32_t offset = 0;
    uint32_t aikido_devbus_baseaddr = 0;
    
    /* Get base addr. of device bus. */
    if (tsn_get_devbus_baseaddr(TSN_AIKIDO_DEVBUS_NUM, &aikido_devbus_baseaddr) != PASSED) {
        printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
               __FUNCTION__, __LINE__, TSN_AIKIDO_DEVBUS_NUM);
    }
    offset |= aikido_devbus_baseaddr + AIKIDO_TAM_RESOURCE;

    if (tsn_mem_write32(offset , AIKIDO_TAM_RESET) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, offset);
    }
    
    /* Aikido team suggest take 1 sec. delay when issue the reset.
       There is no register indicate the soft reset complete */
    msleep(TAM_AIKIDO_RESET_DELAY);
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
    if (is_tam_aikido_on() == TRUE) {
		reset_tam_aikido_dev();
    } else {
        /* Act2 initialization reset sequence */
        /* DSL SKU reset the ACT2 */
        fpga_reset_32_api(FPGA_LPC_EXT_DEV_RST_REG, FPGA_ACT2_RST_L, TRUE,
                          WAITTIME_5_MS);
        }
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
    if (is_tam_aikido_on() == TRUE) {
	    /* Do nothing */
    } else {
        /* Act2 initialization reset sequence */
        /* DSL SKU un-reset the ACT2 */
        fpga_reset_32_api(FPGA_LPC_EXT_DEV_RST_REG, FPGA_ACT2_RST_L, FALSE,
                          WAITTIME_20_MS);
    }
}

/*
 *------------------------------------------------------------------
 * $Log: tam_act2_api_drv_support.c,v $
 * Revision 1.6  2018/11/23 08:49:53  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.5  2018/09/21 02:51:01  iachang
 * CSCvm54395: Upgraded TAM lib. to V3.3.27 and supported Act2 SUDI 2099
 *
 * Revision 1.4.54.2  2018/10/25 02:49:04  iachang
 * Check Act2 NACK status
 *
 * Revision 1.4.54.1  2018/10/15 06:53:08  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.4  2018/02/09 09:56:55  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.3  2018/01/23 11:38:19  steja
 * Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)
 *
 * Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
 * prepare merge star-branch-c9xx to main trunk
 *
 * Revision 1.2.12.2  2017/12/28 03:11:51  shjung
 * Fixed cookie alter issue
 *
 * Revision 1.2.12.1  2017/10/20 11:42:40  steja
 * Sync Gfast  with the latest main trunk
 *
 * Revision 1.2.4.3  2017/09/08 01:36:19  harrchan
 * Support discrete ACT2 offline test
 *
 * Revision 1.2.4.2  2017/08/17 12:19:44  hondwang
 * Set AIKIDO ACT2 by default
 *
 * Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:50  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:21  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/21 09:17:41  iachang
 * clean up code
 *
 * Revision 1.1.6.2  2017/07/20 13:38:08  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.6.4.1  2017/09/21 11:42:44  shjung
 * Do not initialize I2C adapter as file descriptor is never closed
 * 
 * Revision 1.1.4.6.2.2  2017/07/06 07:01:07  iachang
 * Upgrade TAM lib. to V2.10.2
 * Changed tam_lib_platform_spi_r/w() function name to tam_lib_platform_mbx_r/w()
 *
 * Revision 1.1.4.6.6.2  2017/06/16 13:03:34  tirawan
 * I2C driver modification to support ACT2 cookie programming
 *
 * Revision 1.1.4.6.6.1  2017/06/13 14:10:31  hondwang
 * Add ACT2 function and compiler pass
 *
 * Revision 1.1.4.6.2.1  2017/06/12 07:27:26  iachang
 * Correct the TAM lib. path : .../TSN/HA_roots/tam_library.h
 *
 * Revision 1.1.4.6  2016/11/30 03:12:44  steja
 * Fix Aikido get base address
 *
 * Revision 1.1.4.5  2016/09/13 14:35:47  steja
 * Commit Aikido / TAM Mailbox code
 *
 * Revision 1.1.4.4  2016/09/01 06:36:18  iachang
 * Supported Aikido cookie access
 * Supported Aikido ACT-2 utilities and programming
 *
 * Revision 1.1.4.3  2016/08/09 09:47:54  iachang
 * Supported FPGA/Aikido firmware upgrade.
 *
 * Revision 1.1.4.2  2016/06/30 06:22:51  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.7  2016/05/30 02:31:34  palin2
 * Updated code after ACT2 bring up.
 *
 * Revision 1.1.2.6  2016/05/26 10:46:11  steja
 * Optimize Reset and Unreset MB I2C
 *
 * Revision 1.1.2.5  2016/05/24 01:18:11  palin2
 * Updated Thermal sensor and ACT2 chip I2C bus number based on P1A HW changes
 *
 * Revision 1.1.2.4  2016/04/14 13:47:39  steja
 * Fix Cookie & ACT2 Programming
 *
 * Revision 1.1.2.3  2016/04/14 06:09:49  palin2
 * 1. Removed cpld.c and cpld.h because TSN don't have CPLD.
 * 2. Linked related function to correct FPGA one.
 *
 * Revision 1.1.2.2  2016/04/11 14:12:27  steja
 * Update code i2c utility for bringup
 *
 * Revision 1.1.2.1  2016/03/24 10:35:04  steja
 * Add Cookie and Act2 programming
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
 

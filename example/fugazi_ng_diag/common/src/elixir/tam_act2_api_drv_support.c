/* $Id: tam_act2_api_drv_support.c,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/tam_act2_api_drv_support.c,v $
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
#include "diag_moka_fpga_lib.h"
#include "diag_i2c_lib.h"
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
#include "diag_cpu_lib.h"
#include "plug_slot.h"
#include "diag_sirius_fpga_lib.h"
#include "cross_platform.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/
/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern void *tam_act2_get_n2g_i2c_if(void);
extern int fpga_read_32_reg(uint, uint *);
extern int plat_get_devbus_baseaddr(int, uint *);
/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
static int set_i2c_slave_addr(int addr);
int diagact2_lib_initialize(char *i2c_adapter, int addr);
int diagact2_close_i2c_adapter(void);
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

int diagact2_close_i2c_adapter(void)
{
    close(i2c_adapter_fd);
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
        return (FAILED);
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
        return (0);
    } else {
        return (length); // No error detected 
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
            return (0);
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

    return (length);
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
    int ix = 0;;
    uint32_t offset = 0;
    uint32_t aikido_devbus_baseaddr = 0;
    uint16_t byte_count = 0;
    uint tx_reg_offset = 0;
    uint tx_data = 0;

    /* Get base addr. of device bus. */
    if (plat_get_devbus_baseaddr(PLAT_AIKIDO_DEVBUS_NUM, &aikido_devbus_baseaddr) != PASSED) {
        printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
               __FUNCTION__, __LINE__, PLAT_AIKIDO_DEVBUS_NUM);
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
        if (plat_mem_write32(offset + tx_reg_offset + (ix * 4) , tx_data) != PASSED) {
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
    int ix = 0, jx = 0;
    uint rx_buffer = 0;
    uchar rx_buffer_tmp[TAM_SPI_READ_BUF];
    uint32_t offset = 0;
    uint32_t aikido_devbus_baseaddr = 0;
    uint16_t tx_reg_offset = 0;
    uint16_t byte_count = 0, byte_shift = 0;
   
    /* Get base addr. of device bus. */
    if (plat_get_devbus_baseaddr(PLAT_AIKIDO_DEVBUS_NUM, &aikido_devbus_baseaddr) != PASSED) {
        printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
               __FUNCTION__, __LINE__, PLAT_AIKIDO_DEVBUS_NUM);
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
	    if (plat_mem_read32(offset + tx_reg_offset + (ix * 4), &rx_buffer) != PASSED) {
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
    if (plat_get_devbus_baseaddr(PLAT_AIKIDO_DEVBUS_NUM, &aikido_devbus_baseaddr) != PASSED) {
        printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
               __FUNCTION__, __LINE__, PLAT_AIKIDO_DEVBUS_NUM);
    }
    offset |= aikido_devbus_baseaddr + AIKIDO_TAM_RESOURCE;

    if (plat_mem_write32(offset , AIKIDO_TAM_RESET) != PASSED) {
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

/*-------------------------------------------------
 * $Log: tam_act2_api_drv_support.c,v $
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.3  2021/07/30 06:22:00  harrchan
 * Closing i2c adapter after leaving the cookie menu
 *
 * Revision 1.1.2.2  2021/07/01 02:39:27  harrchan
 * Add Aikido-SPI-ACT2 option into menu so that users can choose SPI interface to programming ACT2.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

/* $Id: platform_spi.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_spi.c,v $
 *------------------------------------------------------------------
 * platform_aikido_spi.c - Contains code to support FPGA SPI interface 
 * 
 * Aug 2018, Alan Peng
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <error.h>
#include <sys/mman.h>
#include "common.h"
#include "proto.h"
#include "linux_api.h"
#include "queryflags.h"
#include "diag_fpga_upgrade.h"
#include "dash_fpga.h" /* get_platform_aikido_base */
#include "nvmonvars.h"
#include "diag_fpga_lib.h"


/* Function prototypes */
extern boolean aikido_mailbox_flag;
extern boolean aikido_act2_flag;
extern int act2_i2c_debug; 
void aikido_flag_mailbox(void); 
void aikido_flag_act2(void); 

void aikido_spi_read_util(void);
void aikido_spi_write_util(void);
static prom_t * init_aikido_spi_addr(void);
unsigned int aikido_spi_read(unsigned int, unsigned int,
                              unsigned int, unsigned int, 
                              unsigned char *); 
unsigned int general_fpga_spi_read(prom_t *, 
                                    unsigned int, unsigned int, 
                                    unsigned int, unsigned int,
                                    unsigned char *); 
unsigned int aikido_spi_write(unsigned int, unsigned int,
                              unsigned int, unsigned int, 
                              unsigned char *); 
unsigned int general_fpga_spi_write(prom_t *, 
                                    unsigned int, unsigned int, 
                                    unsigned int, unsigned int,
                                    unsigned char *); 



/*-------------------------------------------------------------------
 *
 * Function: aikido_flag_mailbox
 * 
 * This function will toggle mail box flag between true/false
 *
 * Input: none.
 *
 * Output: none.
 *
 *-------------------------------------------------------------------
 */
void aikido_flag_mailbox (void)
{
    if (aikido_mailbox_flag == FALSE) {
        printf("\naikido_mailbox_flag = TRUE\n"); 
        aikido_mailbox_flag = TRUE; 
    } else {
        printf("\naikido_mailbox_flag = FALSE\n"); 
        aikido_mailbox_flag = FALSE;
    }
}

/*-------------------------------------------------------------------
 *
 * Function: aikido_flag_act2
 * 
 * This function will toggle act2 flag between true/false
 *
 * Input: none.
 *
 * Output: none.
 *
 *-------------------------------------------------------------------
 */
void aikido_flag_act2 (void)
{
    if (aikido_act2_flag  == FALSE) {
        printf("\naikido_act2_flag = TRUE\n"); 
        aikido_act2_flag    = TRUE; 
    } else {
        printf("\naikido_act2_flag = FALSE\n"); 
        aikido_act2_flag    = FALSE;
    }
}

/*-------------------------------------------------------------------
 *
 * Function: aikido_spi_read_util
 * 
 * This function is spi read util for read aikido via spi
 *
 * Input: none.
 *
 * Output: none.
 *
 *-------------------------------------------------------------------
 */
void aikido_spi_read_util (void) 
{
    unsigned int size, address, op, is_addr, ix; 
    unsigned char buf[3000]; 

    printf("Make sure address is 4-bytes aligned \n"); 
    address = gethex_answer("Enter address", 0, 0, 0xffff);
    size = getdec_answer("Enter size", 1, 1, 3000);  

    op = 0; 
    is_addr = 1; 
    
    size -= 1; /* if user want to read 1 byte, size must be equal to 0 */

    aikido_spi_read(size, address, op, is_addr, buf); 
    for (ix = 0; ix <= size; ix++) { 
        if (ix % 16 == 0) {
            printf("\n"); 
        }
        printf("%02x ", buf[ix]);  
    }
}


/*-------------------------------------------------------------------
 *
 * Function: aikido_spi_write_util
 * 
 * This function is spi write util for write aikido via spi
 *
 * Input: none.
 *
 * Output: none.
 *
 *-------------------------------------------------------------------
 */
void aikido_spi_write_util (void) 
{
    unsigned int size, address, op, is_addr, ix; 
    unsigned char buf[3000]; 

    printf("Make sure address is 4-bytes aligned \n"); 
    printf("address start from 0\n"); 
    address = gethex_answer("Enter address", 0, 0, 0xffff);
    size = getdec_answer("Enter size", 1, 1, 3000); 
    for (ix = 0; ix < size; ix++) {
        buf[ix] = gethex_answer("Enter data", 0, 0, 0xffff);
    }
 
    op = 0;  /* no need to deal with op */
    is_addr = 1; 
    
    size -= 1; /* if user want to read 1 byte, size must be equal to 0 */

    aikido_spi_write(size, address, op, is_addr, buf); 
}

/*-------------------------------------------------------------------
 *
 * Function: init_aikido_spi_addr
 * 
 * This function will initialize the spi_prom_reg data stricture for
 * use in this file.
 *
 * Input: none.
 *
 * Output: Pointer to the spi prom register file
 *
 *-------------------------------------------------------------------
 */
static prom_t *
init_aikido_spi_addr (void)
{ 
    prom_t *aikido_spi; 
    aikido_spi = (prom_t *)get_platform_aikido_addr(); 

    return aikido_spi; 
}

/*-------------------------------------------------------------------
 *
 * Function: aikido_spi_write
 * 
 * This function will get aikido spi address ptr from fpga and 
 * fill write with address on spi control field. 
 *
 * Input: size - write data size 
 *        address - register address 
 *        spi_op - opreation field, we use 0x2 as write op, and left 
 *                 spi_op for future reference.
 *        is_address_field - fpga spi perform read/write with address
 *                           or non-addr type. we keep this parm for 
 *                           future reference. 
 *        write_buf - pointer to write buffer. 
 *
 * Output: PASSED, based on HW spec, there is no error check method. 
 *
 *-------------------------------------------------------------------
 */
unsigned int aikido_spi_write (unsigned int size, unsigned int address,
                              unsigned int spi_op, unsigned int is_address_field, 
                              unsigned char *write_buf)
{
    prom_t *aikido_spi_addr;
    unsigned int control; 

    /* get aikido spi base addr from host FPGA */
    aikido_spi_addr = init_aikido_spi_addr();
  
    /* 0x13 for control : 
     * bit0: use address field, bit1=1 wrtie, direction is host to flash
     * bit4: baud rate divisor, default to 1 */
    if (is_address_field == TRUE) {
        control = 0x13; /* bit[1:0] = write/address field */
    } else {
        control = 0x12; /* bit[1:0] = write/non-address field */
    }

    /* write op 0x02, so far hw only support write/read commands */ 
    general_fpga_spi_write(aikido_spi_addr, size, address, 0x02, control, write_buf); 

    return (PASSED); 
}

/*-------------------------------------------------------------------
 *
 * Function: general_fpga_spi_write
 * 
 * This function is a general spi write function. Despite AIKIDO
 * it could be used for the other SPI device on FPGA. 
 *
 * Input: spi_addr - a pointer to FPGA spi address field. 
 *        size - write data size 
 *        address - register address 
 *        spi_op - opreation field, we use 0x2 as write op, and left 
 *                 spi_op for future reference.
 *        ctrl - to spi control offset. 
 *        buf - pointer to write buffer. 
 *
 * Output: PASSED, based on HW spec, there is no error check method. 
 *
 *-------------------------------------------------------------------
 */
unsigned int general_fpga_spi_write (prom_t * spi_addr, 
                                    unsigned int size, unsigned int address,
                                    unsigned int spi_op, unsigned int ctrl, 
                                    unsigned char *buf)
{
    unsigned int ix, status, buf_ptr = 0;  

wr_size_exceed:

    /* check read fifo empty or not */ 
    /* read data register, make sure data register is clean */
    if (act2_i2c_debug) { 
        printf(" spi write :clean up read fifo \n"); 
    }
    if (!is_read_fifo_empty(spi_addr)) {
	printf("%s:SPI PROM Read Fifo is not empty\n", __FUNCTION__);
	return(FAILED);
    }

    /* spi offset elements 
     * 0:control, 4:status, 8:size, c:data, 10: opcode_addr */
    /* opcode_addr : 31:24 = op code , 23:0 = address */
    if (act2_i2c_debug) { 
        printf(" spi write :filled in size and op code\n"); 
    }

    if (size > 0xFF) { 
        spi_addr->size = 0xFF; /* write 256 bytes */
        size -= 256; 
    } else {
        spi_addr->size = size; 
        size -= size; /* exit */
    }
    spi_addr->opcode_addr = (spi_op << 24 | address); /* 0x02 << 24 | addr */
 
    /* filled in data */ 
    if (act2_i2c_debug) { 
        printf(" spi write :filled in data : ");
    }

    for (ix = buf_ptr; ix <= (spi_addr->size + buf_ptr); ix++) {
        spi_addr->data = buf[ix]; 
        if (act2_i2c_debug) { 
            if (ix % 16 == 0) {
                printf("\n ");
            }
            printf("%02x ", buf[ix]); 
        }
    } 
    if (act2_i2c_debug) { 
        printf("\n");
    }

    status = spi_addr->status; 
    if (status & SPI_STATUS_WR_FIFO_EMPT) {
        printf("spi write : write fifo is empty, status = 0x%x\n", status); 
    } else { 
        if (act2_i2c_debug) {
            printf("spi write : write fifo is not empty, good \n"); 
        }
    }
  
    if (act2_i2c_debug) { 
        printf(" spi write trigger ctrl :\n"); 
        printf(" size        - 0x%x \n", spi_addr->size); 
        printf(" opcode_addr - 0x%08x\n", spi_addr->opcode_addr); 
        printf(" status      - 0x%x \n", status); 
        printf(" ctrl        - 0x%x \n", ctrl); 
    }

    /* bit0: use address field, bit1=0 read, direction is to master */
    /* bit4: baud rate divisor, default to 1 */
    spi_addr->control = ctrl;  /* write, ctrl = 0x13 */

    /* Check if operation completed */
    /* read status register and clear done bit on bit15 */
    if (act2_i2c_debug) { 
        printf(" spi write check op done \n"); 
    }
    if (!is_rd_wr_op_done(spi_addr)) {
        printf("%s: read/write operation not done\n", __FUNCTION__);
        return (FAILED);
    }

    if (size != 0) {
        address += 256; 
        buf_ptr += 256; 
        goto wr_size_exceed;
    }

    return (PASSED); 
}


/*-------------------------------------------------------------------
 *
 * Function: aikido_spi_read
 * 
 * This function will get aikido spi address ptr from fpga and 
 * fill read with address on spi control field. 
 *
 * Input: size - read data size 
 *        address - register address 
 *        spi_op - opreation field, we use 0x3 as read op, and left 
 *                 spi_op for future reference.
 *        is_address_field - fpga spi perform read/write with address
 *                           or non-addr type. we keep this parm for 
 *                           future reference. 
 *        buf - pointer to read data buffer. 
 *
 * Output: PASSED, based on HW spec, there is no error check method. 
 */
unsigned int aikido_spi_read (unsigned int size, unsigned int address,
                              unsigned int spi_op, unsigned int is_address_field, 
                              unsigned char *buf)
{
    prom_t *aikido_spi_addr;
    unsigned int control; 

    /* get aikido spi base addr from host FPGA */
    aikido_spi_addr = init_aikido_spi_addr();
  
    /* 0x11 for control : 
     * bit0: use address field, bit1=0 read, direction is to master 
     * bit4: baud rate divisor, default to 1 */
    /* it seems none-address field read nothing */
    if (is_address_field == TRUE) {
        control = 0x11;  /* bit[1:0] = read/address field */
    } else {
        control = 0x10;  /* bit[1:0] = read/non-address field */
    }

    /* read op 0x03, so far hw only support write/read commands */ 
    general_fpga_spi_read(aikido_spi_addr, size, address, 0x03, control, buf); 

    return (PASSED); 
}


/*-------------------------------------------------------------------
 *
 * Function: general_fpga_spi_read
 * 
 * This function is a general spi read function. Despite AIKIDO
 * it could be used for the other SPI device on FPGA. 
 *
 * Input: spi_addr - a pointer to FPGA spi address field. 
 *        size - write data size 
 *        address - register address 
 *        spi_op - opreation field, we use 0x2 as write op, and left 
 *                 spi_op for future reference.
 *        ctrl - to spi control offset. 
 *        buf - pointer to write buffer. 
 *
 * Output: PASSED, based on HW spec, there is no error check method. 
 *
 *-------------------------------------------------------------------
 */
unsigned int general_fpga_spi_read (prom_t * spi_addr, 
                                    unsigned int size, unsigned int address,
                                    unsigned int spi_op, unsigned int ctrl, 
                                    unsigned char *buf)
{
    int ix, buf_ptr = 0, size_tmp; 

    size_tmp = size; 

rd_size_exceed:

    if (act2_i2c_debug) { 
        printf(" spi read : clean read fifo \n"); 
    }
    /* check read fifo empty or not */ 
    if (!is_read_fifo_empty(spi_addr)) {
	printf("%s:SPI PROM Read Fifo is not empty\n", __FUNCTION__);
	return(FAILED);
    }

    /* spi offset elements 
     * 0:control, 4:status, 8:size, c:data, 10: opcode_addr */
    /* opcode_addr : 31:24 = op code , 23:0 = address */
    if (size > 0xFF) {
        spi_addr->size = 0xFF;
        size -= 256; 
    } else { 
        spi_addr->size = size; 
        size -= size; 
    }
    spi_addr->opcode_addr = (spi_op << 24 | address);
 
    /* bit0: use address field, bit1=0 read, direction is to master */
    /* bit4: baud rate divisor, default to 1 */
    if (act2_i2c_debug) { 
        printf(" spi read trigger ctrl :\n"); 
        printf(" size        - 0x%x  \n", spi_addr->size); 
        printf(" opcode_addr - 0x%08x\n", spi_addr->opcode_addr); 
        printf(" ctrl        - 0x%x  \n", ctrl); 
    }
    spi_addr->control = ctrl; 

    if (act2_i2c_debug) { 
        printf(" spi read : check wd wr op done  \n"); 
    }
    /* Check if operation completed */
    if (!is_rd_wr_op_done(spi_addr)) {
        printf("%s: read/write operation not done\n", __FUNCTION__);
        return (FAILED);
    }

    if (act2_i2c_debug) { 
        printf("\nstart of partial red\n"); 
    }
    /* size = 0 means 1 btye, size = 3 means 4 bytes, 
     * size = 0xff means 256 bytes; using <= to alias the spec. */
    for (ix = buf_ptr; ix <= (spi_addr->size + buf_ptr); ix++) {
        buf[ix] = spi_addr->data; 
        if (act2_i2c_debug) { 
            if (ix % 16 == 0) {
                printf("\n"); 
            }
            printf("%02x ", buf[ix]);  
        }
    } 

    if (act2_i2c_debug) { 
        printf("\nend of partial red\n"); 
    }

    if (act2_i2c_debug) { 
        if (size == 0) { 
        printf(" total spi read : dump buf   \n"); 
        for (ix = 0; ix <= size_tmp; ix++) { 
            if (ix % 16 == 0) {
                printf("\n"); 
            }
            printf("%02x ", buf[ix]);  
        }
        }
    }

    if (size != 0) {
        address += 256; 
        buf_ptr += 256; 
        goto rd_size_exceed;
    }

    return (PASSED); 
}


/*-------------------------------------------------------------------
 *
 * Function: aikido_reg_test
 *
 * Aikido register test.
 *
 * Input: addr - register address
 *        ptn - pattern for read/write test, pattern size 4 bytes.
 *
 * Output: PASSED / FAILED
 *
 *-------------------------------------------------------------------
 */
int aikido_reg_test(unsigned int addr, unsigned char *ptn)
{
    unsigned int size = 4, op = 0, is_addr = 1;
    unsigned char buf[4], buf_orig[4];
    int ix;

    size -= 1; /* if user want to read 1 byte, size must be equal to 0 */

    /* Store original value */
    aikido_spi_read(size, addr, op, is_addr, buf_orig);

    /* Write test pattern data to target register */
    aikido_spi_write(size, addr, op, is_addr, ptn);

    /* Read data from target register */
    aikido_spi_read(size, addr, op, is_addr, buf);

    /* Restore original value */
    aikido_spi_write(size, addr, op, is_addr, buf_orig);

    /* Compare data */
    for (ix = 0; ix < size+1; ix++) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Read buf[%d] = 0x%x, expected ptn[%d] = 0x%x\n",
                    ix, buf[ix], ix, ptn[ix]);
        }
        if (buf[ix] != ptn[ix]) {
            return (FAILED);
        }
    }
    return (PASSED);
}

/*------------------- End of File ----------------------*/


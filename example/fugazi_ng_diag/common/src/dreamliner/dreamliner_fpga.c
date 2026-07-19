/* $Id: dreamliner_fpga.c,v 1.2 2015/02/27 10:02:19 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/dreamliner_fpga.c,v $
 *------------------------------------------------------------------
 *
 * dreamliner_fpga.c - This file contains FPGA functions for Dreamliner POE 
 *                     daughtercard.
 *
 * Christine Wen -- Jan. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "common_utils.h"
#include "proto.h"
#include "strings.h"
#include "queryflags.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h> 
#include <unistd.h>

#include "dreamliner.h"
#include "dreamliner_ge_switch.h"
#include "dreamliner_fpga.h"

extern unsigned char dreamliner_fpga_array_fw[];
extern const unsigned int dreamliner_fpga_array_size;

static int peek_fpga_reg();
static int poke_fpga_reg();
static int show_fpga_timestamp();
static int poe_in_reset();
static int poe_out_of_reset();
static int read_spi_util();
static int write_spi_util();
static int fpga_reg_read(ulong offset, int size, ulong *buf, void *param);
static int fpga_reg_write(ulong offset, int size, ulong data, void *param);
static int spi_flash_image_upgrade();

static reg_info_t_ext reg_ext = {2, fpga_reg_read, fpga_reg_write, 0};

static reg_info_t fpga_reg_table[] =
{
/*  Register name,		Offset,		Type, Size,
 *		Mask, Reset Value
 */
    {"SMI_WR_ADDR_LSB",           0x01,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffff, 0},
    {"SMI_WR_DATA_MSB",           0x02,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffff, 0},
    {"SMI_WR_DATA_LSB",           0x03,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffff, 0},
    {"SMI_RD_ADDR_LSB",           0x05,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffff, 0},
};


/* submenu for FPGA utilities */
submenu_xtable_t fpga_util_submenu_table[] = {
    {"peek FPGA register",  
     (PFT)peek_fpga_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"poke FPGA register",  
     (PFT)poke_fpga_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"show FPGA timestamp",  
     (PFT)show_fpga_timestamp,     0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"put POE in reset",  
     (PFT)poe_in_reset,            0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"take POE out of reset",  
     (PFT)poe_out_of_reset,        0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"read from SPI flash",  
     (PFT)read_spi_util,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"write to SPI flash",  
     (PFT)write_spi_util,          0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Upgrade FPGA image",  
     (PFT)spi_flash_image_upgrade, 0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
};

#define FPGA_UTIL_SUBMENU_TABLE_SIZE (sizeof(fpga_util_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_util_primary_items[FPGA_UTIL_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t fpga_util_secondary_items[FPGA_UTIL_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t fpga_util_menu = {
    "FPGA Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fpga_util_primary_items,
};
menuinfo_t *fpga_submenup = &fpga_util_menu;

/*********************************************************************
 *
 * Function: fpga_utils()
 *
 * Description: Build the primary & secondary submenus for the
 * FPGA utility menu. 
 *
 * Inputs: none       
 * Outputs: PASSED
 *
 *********************************************************************
 */
int 
fpga_utils ()
{
    build_primary_submenu(fpga_util_submenu_table, 
			  FPGA_UTIL_SUBMENU_TABLE_SIZE,
			  "FPGA Utility", &fpga_submenup);
    build_secondary_submenu(fpga_util_submenu_table,
			    FPGA_UTIL_SUBMENU_TABLE_SIZE,
			    fpga_util_secondary_items);

    menu(&fpga_util_menu, fpga_util_secondary_items, '\0');
    
    return PASSED;
}


/******************************************************************************
 *
 * Function   :	peek_fpga_reg
 * Description:	utility to peek a FPGA register.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
peek_fpga_reg ()
{
    uint32_t reg_addr;
    uint16_t data;
    int ret;

    reg_addr = gethex_answer("Enter FPGA internal register number: ", 0, 0, 0x1f);

    ret = smi1_read_reg(reg_addr, &data);
    if (ret == PASSED)
	printf("FPGA register value@%#x = %#x\n", reg_addr, data);
    return (ret);
}

/******************************************************************************
 *
 * Function   :	poke_fpga_reg
 * Description:	utility to poke a FPGA register.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
poke_fpga_reg ()
{
    uint32_t reg_addr;
    uint16_t data;

    reg_addr = gethex_answer("Enter FPGA internal register number: ", 0, 0, 0x1f);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);

    return (smi1_write_reg(reg_addr, data));
}

/******************************************************************************
 *
 * Function   :	poe_in_reset
 * Description:	utility to put POE controllers in reset.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
poe_in_reset ()
{
    uint16_t data;

    smi1_read_reg(FPGA_POE_RESET, &data);
    data &= ~FPGA_POE_RESET_BIT;
    return (smi1_write_reg(FPGA_POE_RESET, data));
}

/******************************************************************************
 *
 * Function   :	poe_out_of_reset
 * Description:	utility to take POE controllers out of reset.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
poe_out_of_reset ()
{
    uint16_t data;

    smi1_read_reg(FPGA_POE_RESET, &data);
    data |= FPGA_POE_RESET_BIT;
    return (smi1_write_reg(FPGA_POE_RESET, data));
}

/******************************************************************************
 *
 * Function   :	show_fpga_timestamp
 * Description:	utility to display FPGA timestamp.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
show_fpga_timestamp ()
{
    uint16_t data;
    uchar year, month, day, ver;

    if (smi1_read_reg(FPGA_DATE, &data) != PASSED)
	return FAILED;
    year = (data >> 8) & 0xff;
    month = data & 0xff;

    if (smi1_read_reg(FPGA_DAY_REV, &data) != PASSED)
	return FAILED;
    day  = (data >> 8) & 0xff;
    ver = data & 0xff;

    printf("The FPGA is built at %x/%x/20%x, version: %d\n", month, day, year, ver);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	fpga_reg_read
 * Description:	Wrapper for fpga register test. 
 * Inputs: offset - register offset
 *	   size - Number of bytes to be read. fpga registers are 
 *                4 bytes access
 *	   buf  - points to the data buffer to hold read data.
 *	   param - Pointer to parameter
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
fpga_reg_read (ulong offset, int size, ulong *buf, void *param)
{
    return (smi1_read_reg((int32_t)offset, (uint16_t *)buf));
}


/******************************************************************************
 *
 * Function   :	fpga_reg_read
 * Description:	Wrapper for fpga register test. 
 * Inputs: offset - register offset
 *	   size - Number of bytes to be read. FPGA registers 
 *                are 4 bytes access
 *	   data  - write data
 *	   param - Pointer to parameter
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
fpga_reg_write (ulong offset, int size, ulong data, void *param)
{
    return (smi1_write_reg((uint32_t)offset, (uint16_t)data));
}

/******************************************************************************
 *
 * Function   :	fpga_reg_test
 * Description:	FPGA registers test through SMI interface.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
fpga_reg_test (void)
{
    prpass(testpass, "FPGA Register,");
    cterr_setup();
    cterr_add_component("Marvell xCat2 switch", 
			"Dreamliner FPGA");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check SMI1 interface from xCat2 switch");
    return (register_tests(0, &fpga_reg_table[0])); 
}


/******************************************************************************
 *
 * Function   :	dl_read_i2c
 * Description:	read POE controller registers through SMI interface.
 * Inputs     :	i2c_addr - POE controller I2C address
 *              reg_addr - POE controller register address
 *              size     - data size to be read
 *              reg_data - point to uchar which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
dl_read_i2c(uchar i2c_addr, uchar reg_addr, uint size, uchar *reg_data)
{
    uint16_t status;
    uint16_t data;
    int i;

    /* poll for SMI interface ready bit */
    for (i=0; i < FPGA_SMI_READY_RETRY; i++) {
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_READY)
	    break;
    }

    if (i == FPGA_SMI_READY_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI interface READY bit");
	return FAILED;
    }

    /* check for READ DONE bit, it should be cleared before the read */
    if  (status & FPGA_SMI_RD_DONE) {
	/* clear READ DONE bit */
	if (smi1_write_reg(FPGA_SMI_STATUS, 0x0f) != PASSED)
	    return FAILED;
    }

    /* set read address */
    data = reg_addr;
    if (smi1_write_reg(FPGA_SMI_RD_ADDR_LSB, data))    
	return FAILED;

    data = i2c_addr;
    if (smi1_write_reg(FPGA_SMI_RD_ADDR_MSB, data))    
	return FAILED;

    /* poll for READ DONE bit */
    for (i=0; i < FPGA_SMI_RD_RETRY; i++) {
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_RD_DONE)
	    break;
	msleep(1);
    }

    if (i == FPGA_SMI_RD_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI READ DONE bit");
	return FAILED;
    }

    /* read data */
    if (smi1_read_reg(FPGA_SMI_RD_DATA_LSB, &data))    
	return FAILED;
    *reg_data = data & 0xff;

    /* clear READ DONE bit */
    if (smi1_write_reg(FPGA_SMI_STATUS, 0x0f) != PASSED)
	return FAILED;

    /* check SMI interface ready bit */
    if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	return FAILED;
    if ((status & FPGA_SMI_READY) != FPGA_SMI_READY) {
	cterr('f', 0, "SMI interface READY bit is not set after read operation");
	return FAILED;
    }
	
    return PASSED;
}

/******************************************************************************
 *
 * Function   :	dl_write_i2c
 * Description:	write to POE controller registers through SMI interface.
 * Inputs     :	i2c_addr - POE controller I2C address
 *              reg_addr - POE controller register address
 *              size     - data size to write
 *              reg_data - point to uchar which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
dl_write_i2c(uchar i2c_addr, uchar reg_addr, uint size, uchar *reg_data)
{
    uint16_t status;
    uint16_t data;
    int i;

    /* poll for SMI interface ready bit */
    for (i=0; i < FPGA_SMI_READY_RETRY; i++) {
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_READY)
	    break;
    }

    if (i == FPGA_SMI_READY_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI interface READY bit");
	return FAILED;
    }

    /* set write data */
    if (smi1_write_reg(FPGA_SMI_WR_DATA_LSB, (uint16_t)*reg_data))    
	return FAILED;

    /* set write address */
    data = reg_addr;
    if (smi1_write_reg(FPGA_SMI_WR_ADDR_LSB, data))    
	return FAILED;

    data = i2c_addr;
    if (smi1_write_reg(FPGA_SMI_WR_ADDR_MSB, data))    
	return FAILED;

    /* poll for WRITE DONE bit */
    for (i=0; i < FPGA_SMI_WR_RETRY; i++) {
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_WR_DONE)
	    break;
    }

    if (i == FPGA_SMI_WR_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI WRITE DONE bit");
	return FAILED;
    }

    /* clear WRITE DONE bit */
    if (smi1_write_reg(FPGA_SMI_STATUS, 0x0f) != PASSED)
	    return FAILED;

    /* check SMI interface ready bit */
    if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	return FAILED;
    if ((status & FPGA_SMI_READY) != FPGA_SMI_READY) {
	cterr('f', 0, "SMI interface READY bit is not set after read operation");
	return FAILED;
    }
	
    return PASSED;
}

/******************************************************************************
 *
 * Function   :	dl_read_spi
 * Description:	read SPI flash through SMI interface.
 * Inputs     :	spi_addr - SPI address
 *              opcode   - opcode
 *              size     - data size to be read (4 bytes max)
 *              mem_data - point to uint32_t which holds memory value
 *              addr_incl- include spi_addr or not 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
dl_read_spi (uint spi_addr, uchar opcode, uint size, uint32_t *mem_data, boolean addr_incl)
{
    uint16_t status;
    uint16_t data;
    int i;

    /* poll for SMI interface ready bit */
    for (i=0; i < FPGA_SMI_READY_RETRY; i++) {
	/* FPGA port is fixed to 12 */
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_READY)
	    break;
    }

    if (i == FPGA_SMI_READY_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI interface READY bit");
	return FAILED;
    }

    /* set opcode */
    data = (opcode << 8);
    if (smi1_write_reg(FPGA_SMI_WR_DATA_MSB, data))    
	return FAILED;

    /* set read address */
    data = spi_addr & 0xffff;
    if (smi1_write_reg(FPGA_SMI_RD_ADDR_LSB, data))    
	return FAILED;

    data = SPI_ACCESS | (size << LEN_SHIFT) | ((spi_addr >> 16) & 0xff);
    if (addr_incl == TRUE)
	data |= SPI_ADDR_PRESENT ;

    if (smi1_write_reg(FPGA_SMI_RD_ADDR_MSB, data))    
	return FAILED;

    /* poll for READ DONE bit */
    for (i=0; i < FPGA_SMI_RD_RETRY; i++) {
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_RD_DONE)
	    break;
    }

    if (i == FPGA_SMI_RD_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI READ DONE bit");
	return FAILED;
    }

    /* read data */
    *mem_data = 0;
    if (size > 2) {
	if (smi1_read_reg(FPGA_SMI_RD_DATA_MSB, &data) != PASSED)    
	    return FAILED;
	    *mem_data = (data << 16) & 0x00ff0000;
    } 

    if (smi1_read_reg(FPGA_SMI_RD_DATA_LSB, &data) != PASSED)    
	return FAILED;
    if(size == 1)
	*mem_data = data & 0x00ff;
    else
	*mem_data |= data;

    /* clear READ DONE bit */
    if (smi1_write_reg(FPGA_SMI_STATUS, 0x0f) != PASSED)
	    return FAILED;

    /* check SMI interface ready bit */
    if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	return FAILED;
    if ((status & FPGA_SMI_READY) != FPGA_SMI_READY) {
	cterr('f', 0, "SMI interface READY bit is not set after read operation");
	return FAILED;
    }
	
    return PASSED;
}

/******************************************************************************
 *
 * Function   :	dl_write_spi
 * Description:	write SPI flash through SMI interface.
 * Inputs     :	spi_addr - SPI address
 *              opcode   - opcode
 *              size     - data size to write (3 bytes max)
 *              mem_data - data to write
 *              addr_incl- include spi_addr or not 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
dl_write_spi (uint spi_addr, uchar opcode, uint size, uint32_t mem_data, boolean addr_incl)
{
    uint16_t status;
    uint16_t data;
    int i;

    /* poll for SMI interface ready bit */
    for (i=0; i < FPGA_SMI_READY_RETRY; i++) {
	/* FPGA port is fixed to 12 */
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_READY)
	    break;
    }

    if (i == FPGA_SMI_READY_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI interface READY bit");
	return FAILED;
    }

    /* set write data */	
    data = mem_data & 0xffff;
    if (smi1_write_reg(FPGA_SMI_WR_DATA_LSB, data))    
	return FAILED;

    if (size > 2) {
	data = ((mem_data >> 16) & 0xff) | (opcode << 8);
    } else {
	data = (opcode << 8);
    }
    if (smi1_write_reg(FPGA_SMI_WR_DATA_MSB, data))    
	return FAILED;

    /* set write address */
    data = spi_addr & 0xffff;
    if (smi1_write_reg(FPGA_SMI_WR_ADDR_LSB, data))    
	return FAILED;

    data = SPI_ACCESS | (size << LEN_SHIFT) | ((spi_addr >> 16) & 0xff);
    if (addr_incl == TRUE)
	data |= SPI_ADDR_PRESENT ;

    if (smi1_write_reg(FPGA_SMI_WR_ADDR_MSB, data))    
	return FAILED;

    /* poll for WRITE DONE bit */
    for (i=0; i < FPGA_SMI_WR_RETRY; i++) {
	if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	    return FAILED;
	if (status & FPGA_SMI_WR_DONE)
	    break;
    }

    if (i == FPGA_SMI_WR_RETRY) {
	cterr('f', 0, "Timeout waiting for SMI WRITE DONE bit");
	return FAILED;
    }

    /* clear WRITE DONE bit */
    if (smi1_write_reg(FPGA_SMI_STATUS, 0x0f) != PASSED)
	    return FAILED;

    /* check SMI interface ready bit */
    if (smi1_read_reg(FPGA_SMI_STATUS, &status) != PASSED)
	return FAILED;
    if ((status & FPGA_SMI_READY) != FPGA_SMI_READY) {
	cterr('f', 0, "SMI interface READY bit is not set after read operation");
	return FAILED;
    }
	
    return PASSED;
}

/******************************************************************************
 *
 * Function   :	read_spi_util
 * Description:	utility to read from SPI flash.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
read_spi_util ()
{
    uint32_t spi_addr;
    uchar opcode;
    uint size;
    uint32_t data;
    boolean addr_incl;
    int ret;

    spi_addr = gethex_answer("Enter SPI address: ", 0, 0, 0x7fffff);
    opcode = gethex_answer("Enter opcode: ", 0, 0, 0xff);
    size = gethex_answer("Enter size: ", 1, 1, 0x3);
    addr_incl = gethex_answer("is spi_addr included? 1(Yes) or 0(No)): ", 0, 0, 1);

    ret = dl_read_spi(spi_addr, opcode, size, &data, addr_incl);
    if (ret == PASSED) {
	printf("The read back data are: %#x\n", data);
    }

    return (ret);
}

/******************************************************************************
 *
 * Function   :	write_spi_util
 * Description:	utility to write to SPI flash.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
write_spi_util ()
{
    uint32_t spi_addr;
    uchar opcode;
    uint size;
    uint32_t data;
    boolean addr_incl;

    spi_addr = gethex_answer("Enter SPI address: ", 0, 0, 0x7fffff);
    opcode = gethex_answer("Enter opcode: ", 0, 0, 0xff);
    size = gethex_answer("Enter size: ", 0, 0, 0x3);
    addr_incl = gethex_answer("is spi_addr included? 1(Yes) or 0(No)): ", 0, 0, 1);
    if (size != 0)
	data = gethex_answer("Enter data: ", 0, 0, 0xffffff);

    return (dl_write_spi(spi_addr, opcode, size, data, addr_incl));
}


/******************************************************************************
 *
 * Function   :	spi_wait_for_rdy
 * Description:	wait for the SPI flash to be ready for the write/erase.
 * Inputs     :	none
 * Outputs    : TRUE/FALSE
 *
 ******************************************************************************
 */
static boolean
spi_wait_for_rdy ()
{
    uint32_t data;
    int count = 0;

    do {
	/* read status register */
	if (dl_read_spi(0, SPI_READ_STATUS, 1, &data, FALSE) == FAILED) {
	    cterr('f',0,"Failed to read SPI flash status register.");
	    return (FALSE);
	}
	
	/* return TRUE if the SPI flash is ready */
	if (!(data & 0x01)) {
	    return (TRUE);
	}

	usleep(10);
	count++;
    } while (count < SPI_MAX_RETRIES);

    cterr('f', 0, "Timeout waiting for the SPI status to be ready.");
    return (FALSE);
}


/******************************************************************************
 *
 * Function   :	spi_protect_sector
 * Description:	protect a specific sector within the SPI flash
 * Inputs     :	sector_addr
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_protect_sector (uint32_t sector_addr)
{
    if (dl_write_spi(0, SPI_WRITE_ENABLE, 0, 0, FALSE) == FAILED) {
	cterr('f',0,"Failed to enable write on SPI flash");
	return (FAILED);
    }

    if (dl_write_spi(sector_addr, SPI_PROTECT_SECTOR, 0, 0, TRUE) == FAILED) {
	cterr('f',0,"Failed to protect sector %#x on SPI flash", sector_addr);
	return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	spi_unprotect_sector
 * Description:	unprotect a specific sector within the SPI flash
 * Inputs     :	sector_addr
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_unprotect_sector (uint32_t sector_addr)
{
    if (dl_write_spi(0, SPI_WRITE_ENABLE, 0, 0, FALSE) == FAILED) {
	cterr('f',0,"Failed to enable write on SPI flash");
	return (FAILED);
    }

    if (dl_write_spi(sector_addr, SPI_UNPROTECT_SECTOR, 0, 0, TRUE) == FAILED) {
	cterr('f',0,"Failed to unprotect sector %#x on SPI flash", sector_addr);
	return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	spi_erase_sector
 * Description:	Erase a sector (64KB) within the SPI flash.
 * Inputs     :	sector_addr
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_erase_sector (uint32_t sector_addr)
{
    if (dl_write_spi(0, SPI_WRITE_ENABLE, 0, 0, FALSE) == FAILED) {
	cterr('f',0,"Failed to enable write on SPI flash");
	return (FAILED);
    }

    if (dl_write_spi(sector_addr, SPI_ERASE_SECTOR, 0, 0, TRUE) == FAILED) {
	cterr('f',0,"Failed to erase sector %#x on SPI flash", sector_addr);
	return (FAILED);
    }
    
    if (spi_wait_for_rdy() == FALSE)
	return (FAILED);

    return (PASSED);
} 


/******************************************************************************
 *
 * Function   :	spi_erase_subsector
 * Description:	Erase a subsector (4KB) within the SPI flash.
 * Inputs     :	sector_addr
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_erase_subsector (uint32_t sector_addr)
{
    if (dl_write_spi(0, SPI_WRITE_ENABLE, 0, 0, FALSE) == FAILED) {
	cterr('f',0,"Failed to enable write on SPI flash");
	return (FAILED);
    }

    if (dl_write_spi(sector_addr, SPI_ERASE_SUBSECTOR, 0, 0, TRUE) == FAILED) {
	cterr('f',0,"Failed to erase subsector %#x on SPI flash", sector_addr);
	return (FAILED);
    }
    
    if (spi_wait_for_rdy() == FALSE)
	return (FAILED);

    return (PASSED);
} 


/******************************************************************************
 *
 * Function   :	spi_verify_bytes
 * Description:	Read a length of data from the SPI flash and compare them
 *              with the specific string.
 * Inputs     :	addr
 *              len
 *              val_array
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_verify_bytes (uint32_t addr, uint32_t len, uint8_t *val_array)
{
    uint8_t *val = val_array;
    uint8_t byteval;
    uint32_t data;
    int read_len;

    while (len > 0) {
	if (len > 1)
	    read_len = 2;
	else
	    read_len = 1;

	if (dl_read_spi(addr, SPI_FAST_READ_BYTE, read_len+1, &data, TRUE) == FAILED) {
	    cterr('f',0,"Failed to read %d bytes @ %#x from SPI flash.",
		  read_len, addr);
	    return (FAILED);
	}	    

	byteval = (uint8_t)((data >> 8) & 0xff);
	if (byteval != *val) {
	    cterr('f',0,"Read SPI addr %#x value %#x, expect %#x",
		  addr, byteval, *val);
	    return (FAILED);
	}

	val++;

	if (read_len > 1) {
	    byteval = (uint8_t)((data >> 16) & 0xff);
	    if (byteval != *val) {
		cterr('f',0,"Read SPI addr %#x value %#x, expect %#x",
		      addr, byteval, *val);
		return (FAILED);
	    }
	    val++;
	}
	addr += read_len;
	len -= read_len;
    }
    return (PASSED);
}
	    

/******************************************************************************
 *
 * Function   :	spi_write_bytes
 * Description:	write a length of data to the SPI flash.
 * Inputs     :	addr
 *              len
 *              val_array
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_write_bytes (uint32_t addr, uint32_t len, uint8_t *val_array)
{
    uint8_t *val = val_array;
    uint32_t data;
    int write_len, align_len;

    while (len > 0) {
	write_len = (len > 3) ? 3 : (len);
	
	/* check address boundary, need to be page aligned (256B) */
	align_len = SPI_PAGE_SIZE - (addr % SPI_PAGE_SIZE);
	if (write_len > align_len)
	    write_len = align_len;

	data = 0;
	switch (write_len) {
	case 3:
	    data += *(val + 2) << 16;
	case 2:
	    data += *(val + 1) << 8;
	case 1:
	default:
	    data += *(val);
	}

	if (dl_write_spi(0, SPI_WRITE_ENABLE, 0, 0, FALSE) == FAILED) {
	    cterr('f',0,"Failed to enable write on SPI flash");
	    return (FAILED);
	}

	if (dl_write_spi(addr, SPI_PROGRAM_BYTE, write_len, data, TRUE) == FAILED) {
	    cterr('f',0,"Failed to write %d bytes %#x  @ %#x to SPI flash.", 
		  write_len, data, addr);
	    return (FAILED);
	}

	/* wait for device ready */
	if (spi_wait_for_rdy() == FALSE)
	    return (FAILED);

	val += write_len;
	addr += write_len;
	len -= write_len;
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	spi_flash_test
 * Description:	test a subsector(4KB) within the SPI flash.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
spi_flash_test ()
{
    uint32_t test_offset = SPI_FLASH_TEST_SECTOR_OFFSET;
    uint32_t test_size = SPI_FLASH_TEST_LENGTH;
    int i;
    uint8_t *data_array;

    cterr_setup();
    cterr_add_component("SPI flash", 
			"SPI controller within the FPGA");
    cterr_add_debug("Check SPI flash",
		    "Check SPI controller within the FPGA");

    data_array = (uint8_t *)malloc(test_size);
    if (data_array == NULL) {
	cterr('f', 0, "Failed to allocate memory");
	return (FAILED);
    }

    for (i = 0; i < test_size; i++) {
	data_array[i] = 0xff;
    }

    if (spi_unprotect_sector(test_offset) == FAILED) {
	free(data_array);
	return (FAILED);
    }

    /* next erase 4KB subsector */
    prpass(testpass, "SPI flash erase sector ");
    if (spi_erase_subsector(test_offset) == FAILED) {
	free(data_array);
	return (FAILED);
    }

    sleep(2);

    /* check sector erase */
    prpass(testpass, "Check SPI flash sector erase \n");
    if (spi_verify_bytes(test_offset, test_size, data_array) == FAILED) {
	free(data_array);
	return (FAILED);
    }

    /* write data pattern to SPI flash */
    prpass(testpass, "SPI flash write sector \n");
    for (i = 0; i < test_size; i++) {
	data_array[i] = i;
    }

    if (spi_write_bytes(test_offset, test_size, data_array) == FAILED) {
	free(data_array);
	return (FAILED);
    }

    /* Now read the test data pattern from sector and verify */
    prpass(testpass, "SPI flash read sector \n");
    if (spi_verify_bytes(test_offset, test_size, data_array) == FAILED) {
	free(data_array);
	return (FAILED);
    }

    if (spi_protect_sector(test_offset) == FAILED) {
	free(data_array);
	return (FAILED);
    }

    free(data_array);
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	spi_flash_image_upgrade
 * Description: utility to upgrade the FPGA image within the SPI flash.
 * Inputs     :	none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
spi_flash_image_upgrade ()
{
    uint32_t image_offset = SPI_FLASH_UPDATE_IMAGE_ADDR;
    uint8_t *dl_image_fw = dreamliner_fpga_array_fw;
    int dl_image_size = dreamliner_fpga_array_size;
    uint32_t remaining_len, bs_offset, sector_addr, data_len;

    cterr_add_component("SPI flash", 
			"SPI controller within the FPGA");
    cterr_add_debug("Check SPI flash",
		    "Check SPI controller within the FPGA");

    prpass(testpass, "Upgrade the secondary FPGA image. ");
    remaining_len = dl_image_size;
    bs_offset = 0;
    sector_addr = image_offset;

    while (remaining_len > 0) {
	/* unprotect current sector */
	if (spi_unprotect_sector(sector_addr) == FAILED) 
	    return (FAILED);

	/* erase 64KB sector */
	if (spi_erase_sector(sector_addr) == FAILED) 
	    return (FAILED);

	if (remaining_len >= SPI_FLASH_SECTOR_SIZE) {
	    data_len = SPI_FLASH_SECTOR_SIZE;
	} else {
	    data_len = remaining_len;
	}

	prpass(testpass, "program sector @ %#x\n ", sector_addr);
	/* write data to SPI flash */
	if (spi_write_bytes(sector_addr, data_len, 
			    &dl_image_fw[bs_offset]) == FAILED) 
	    return (FAILED);

	/* protect current sector */
	if (spi_protect_sector(sector_addr) == FAILED) 
	    return (FAILED);

	remaining_len -= data_len;
	bs_offset += SPI_FLASH_SECTOR_SIZE;
	sector_addr += SPI_FLASH_SECTOR_SIZE;
    }

    prpass(testpass, "Verify the FPGA image programmed. ");
    for (remaining_len = dl_image_size, bs_offset = 0, 
	 sector_addr = image_offset; remaining_len > 0; ) {
	if (remaining_len >= SPI_FLASH_SECTOR_SIZE) {
	    data_len = SPI_FLASH_SECTOR_SIZE;
	} else {
	    data_len = remaining_len;
	}

	/* Now read the data from sector and verify */
	if (spi_verify_bytes(sector_addr, data_len, 
			     &dl_image_fw[bs_offset]) == FAILED)
	    return (FAILED);

	remaining_len -= data_len;
	bs_offset += SPI_FLASH_SECTOR_SIZE;
	sector_addr += SPI_FLASH_SECTOR_SIZE;
    }

    prpass(testpass, "Finish FPGA image upgrade! ");

    return (PASSED);
}

/*
 *------------------------------------------------------------------
 * $Log: dreamliner_fpga.c,v $
 * Revision 1.2  2015/02/27 10:02:19  iachang
 *
 * Add support dreamliner NIM
 *
 * Revision 1.1.6.2  2015/02/14 07:13:51  iachang
 * Dreamliner Diag sync with main trunk.
 *
 * Revision 1.1.4.3  2015/02/06 14:58:47  iachang
 * FPGA Image 7/22/2014, version: 2
 *
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 *
 * Revision 1.1.2.1  2014/12/02 08:04:10  iachang
 * Dreamliner Diag initial check-in.
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

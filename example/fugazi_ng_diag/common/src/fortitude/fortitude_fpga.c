/* $Id: fortitude_fpga.c,v 1.24 2019/06/03 09:07:40 meho Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/fortitude_fpga.c,v $
 *------------------------------------------------------------------
 *
 * fortitude_fpga.c - This file contains functions for Fortitude FPGA.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "common.h"
#include "types.h" 
#include "common_utils.h"
#include "defs.h"
#include "error.h"
#include "nvmonvars.h"
#include "pcmap.h"
#include "fortitude_fpga.h"
#include "fortitude.h"

/* FPGA for P1A and P1B hw */
extern const unsigned char fortitude_image_lx25_fw[];
extern const int fortitude_image_lx25_fw_size;
extern const unsigned char fortitude_image_lx9_fw[];
extern const int fortitude_image_lx9_fw_size;
extern const unsigned char fortitude_image_lx4_fw[];
extern const int fortitude_image_lx4_fw_size;

/* FPGA for P1C and later hw */
extern const unsigned char fortitude_2p_fpga_ugd_fw[];
extern const int fortitude_2p_fpga_ugd_fw_size;
extern const unsigned char fortitude_4p_fpga_ugd_fw[];
extern const int fortitude_4p_fpga_ugd_fw_size;
extern const unsigned char fortitude_8p_fpga_ugd_fw[];
extern const int fortitude_8p_fpga_ugd_fw_size;
extern const unsigned char fortitude_2p_fpga_gld_fw[];
extern const int fortitude_2p_fpga_gld_fw_size;
extern const unsigned char fortitude_4p_fpga_gld_fw[];
extern const int fortitude_4p_fpga_gld_fw_size;
extern const unsigned char fortitude_8p_fpga_gld_fw[];
extern const int fortitude_8p_fpga_gld_fw_size;

static int tdm_rd(ulong addr, int size, ulong *buf, void *param);
static int tdm_wr(ulong addr, int size, ulong data, void *param);

static tdm_rate[4] = {2, 8, 16, 32};

static reg_info_t_ext reg_ext = {4, tdm_rd, tdm_wr, 0};

static reg_info_t tdmsw64_reg_table[] =
{
/*  Register name,		Offset,		Type, Size,
 *		Mask, Reset Value
 */
    {"TDMSW64 stream enable(63-32)",           0x00,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"TDMSW64 stream enable(31-00)",           0x04,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"TDMSW64 stream rate(63-48)",             0x10,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0x0cffff00, 0x55aaaa55},
    {"TDMSW64 stream rate(47-32)",             0x14,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0x55555555},
    {"TDMSW64 stream rate(31-16)",             0x18,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"TDMSW64 stream rate(15-00)",             0x1c,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"TDMSW64 stream lpbk(63-32)",             0x20,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"TDMSW64 stream lpbk(31-00)",             0x24,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"TDMSW64 control",                        0x2c,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0x10, 0},
    {"NGVM TDM control",                       0x30,
     READ_ONLY | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0x07, 0},
    {"End",			0, 0, {0}, 0, 0},
};

static reg_info_t fpga_general_reg_table[] =
{
/*  Register name,		Offset,		Type, Size,
 *		Mask, Reset Value
 */
    {"FPGA_REV_HOUR",           0x00,
     READ_ONLY | SAVE_RESTORE, 1, 0x00, 0},
    {"FPGA_REV_DAY",            0x01,
     READ_ONLY | SAVE_RESTORE, 1, 0x00, 0},
    {"FPGA_REV_MON",            0x02,
     READ_ONLY | SAVE_RESTORE, 1, 0x00, 0},
    {"FPGA_REV_YEAR",           0x03,
     READ_ONLY | SAVE_RESTORE, 1, 0x00, 0},
    {"MISC control",            0x04,
     READ_ONLY | SAVE_RESTORE, 1, 0x00, 0x03},
    {"Board ID",                0x05,
     READ_ONLY | SAVE_RESTORE, 1, 0x00, 0x0},
    {"FPGA int event",          0x08,
     READ_ONLY | SAVE_RESTORE, 1, 0x7f, 0},
    {"FPGA int diag test",      0x09,
     READ_ONLY | SAVE_RESTORE, 1, 0x7f, 0},
    {"FPGA int event ena",      0x0c,
     READ_ONLY | SAVE_RESTORE, 1, 0x7f, 0},
    {"TDMSW cmd status",        0x10,
     READ_ONLY | SAVE_RESTORE, 1, 0x03, 0},
    {"TDMSW address(low)",      0x14,
     READ_WRITE | SAVE_RESTORE, 1, 0xff, 0},
    {"TDMSW address(high)",     0x15,
     READ_WRITE | SAVE_RESTORE, 1, 0xff, 0},
    {"TDM PLL ctrl status1",    0x1c,
     READ_ONLY | SAVE_RESTORE,  1, 0x03, 0x0},
    {"TDM PLL ctrl status2",    0x1d,
     READ_ONLY | SAVE_RESTORE,  1, 0x17, 0x0},
    {"LED control1",            0x20,
     READ_WRITE | SAVE_RESTORE, 1, 0x77, 0},
    {"LED control2",            0x21,
     READ_WRITE | SAVE_RESTORE, 1, 0x77, 0},
    {"LED control3",            0x22,
     READ_WRITE | SAVE_RESTORE, 1, 0x77, 0},
    {"LED control4",            0x23,
     READ_WRITE | SAVE_RESTORE, 1, 0x77, 0},
    {"PMC mode status11",       0x24,
     READ_ONLY | SAVE_RESTORE, 1, 0xff, 0},
    {"PMC mode status12",       0x25,
     READ_ONLY | SAVE_RESTORE, 1, 0xff, 0},
    {"PMC mode status13",       0x26,
     READ_ONLY | SAVE_RESTORE, 1, 0x01, 0},
    {"PMC mode status21",       0x28,
     READ_ONLY | SAVE_RESTORE, 1, 0xff, 0},
    {"PMC mode status22",       0x29,
     READ_ONLY | SAVE_RESTORE, 1, 0xff, 0},
    {"PMC mode status23",       0x2a,
     READ_ONLY | SAVE_RESTORE, 1, 0xff, 0},
    {"PMC mode status24",       0x2b,
     READ_ONLY | SAVE_RESTORE, 1, 0xff, 0},
    {"DS0 dump control",        0x2c,
     READ_ONLY | SAVE_RESTORE, 1, 0x01, 0},
    {"End",			0, 0, {0}, 0, 0},
};


/**********************************************************************
 *
 * Function: tdm_rd()
 *
 * TDMSW registers can only be accessed in 4 bytes width. But the NPU 
 * local bus access to FPGA is only 1 byte. We need to use indirect access
 * to read/write TDMSW registers.
 *
 * Input : addr - offset of register to be read.
 *	   size - Number of bytes to be read. TDMSW registers are 4 bytes
 *	   buf  - points to the data buffer to hold read data.
 *	   param - Pointer to parameter
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
tdm_rd (ulong addr, int size, ulong *buf, void *param)
{
    fpga_reg_t *fpga_reg;
    int i;
    uint8_t *data_buf = (uint8_t *)buf;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);
    
    /* make sure TDMSW is available to access */
    for (i = 0; i < TDMSW_CMD_WAIT; i++) {
	if (fpga_reg->tdmsw_cmd_status & TDMSW_BUSY) {
	    usleep(10);
	} else {
	    break;
	}
    }

    if (i == TDMSW_CMD_WAIT) {
	cterr('f', 0, "TDMSW is busy! Can not access.");
	return (FAILED);
    }

    fpga_reg->tdmsw_adr[0] = addr & 0xff;
    fpga_reg->tdmsw_adr[1] = (addr >> 8) & 0xff;

    /* send read command */
    fpga_reg->tdmsw_cmd_status = TDMSW_CMD_READ | TDMSW_CMD_GO;

    /* wait for read command to complete - 50ms */
    for (i = 0; i < TDMSW_CMD_WAIT; i++) {
	if (fpga_reg->tdmsw_cmd_status & TDMSW_BUSY) {
	    usleep(10);
	} else {
	    break;
	}
    }
    
    if (i == TDMSW_CMD_WAIT) {
	cterr('f', 0, "TDMSW register@%#x read timeout!", addr);
	return (FAILED);
    }

    for (i = 0; i < size; i++) {
	/* NPU is big endian, need to swap the bytes */
	*data_buf = fpga_reg->tdmsw_data[size - 1 - i];
	data_buf++;
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: tdm_wr()
 *
 * TDMSW registers can only be accessed in 4 bytes width. But the NPU 
 * local bus access to FPGA is only 1 byte. We need to use indirect access
 * to read/write TDMSW registers.
 *
 * Input : addr - offset of register to be written.
 *	   size - Number of bytes to write. TDMSW registers are 4 bytes
 *	   data - Write data.
 *	   param - Pointer to parameter
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
tdm_wr (ulong addr, int size, ulong data, void *param)
{
    fpga_reg_t *fpga_reg;
    int i;
    ulong rd_data;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    /* make sure TDMSW is available to access */
    for (i = 0; i < TDMSW_CMD_WAIT; i++) {
	if (fpga_reg->tdmsw_cmd_status & TDMSW_BUSY) {
	    usleep(10);
	} else {
	    break;
	}
    }

    if (i == TDMSW_CMD_WAIT) {
	cterr('f', 0, "TDMSW is busy! Can not access.");
	return (FAILED);
    }

    for (i = 0; i < size; i++) {
	fpga_reg->tdmsw_data[i] = (data >> (i * 8)) & 0xff;
    }

    fpga_reg->tdmsw_adr[0] = addr & 0xff;
    fpga_reg->tdmsw_adr[1] = (addr >> 8) & 0xff;

    /* send write command */
    fpga_reg->tdmsw_cmd_status = TDMSW_CMD_WRITE | TDMSW_CMD_GO;

    if ((NVRAM)->diagflag & D_VERBOSE)
	printf("send tdm_wr(), addr = %#x, data = %#x, fpga_reg->tdmsw_cmd_status = %#x\n", addr, data, fpga_reg->tdmsw_cmd_status);

    /* wait for write command to complete - 50ms */
    for (i = 0; i < TDMSW_CMD_WAIT; i++) {
	if (fpga_reg->tdmsw_cmd_status & TDMSW_BUSY) {
	    usleep(10);
	} else {
	    break;
	}
    }
    if (i == TDMSW_CMD_WAIT) {
	cterr('f', 0, "TDMSW register@%#x write timeout!", addr);
	return (FAILED);
    }

    /* read back to make sure write complete and successful. */
    if (tdm_rd(addr, size, &rd_data, param) == FAILED) {
	cterr('f', 0, "read from TDMSW register@%#x failed!", addr);
	return (FAILED);
    }

    if (data != rd_data) {
	cterr('f', 0, "Data mismatch @ %#x! read = %#x, expect = %#x", 
	      addr, rd_data, data);
	return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE)
	printf("send tdm_wr(), addr = %#x, rd_data = %#x\n", addr, rd_data);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fpga_general_reg_test()
 *
 * This function will test FPGA general registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_general_reg_test ()
{
    ulong base_addr;

    base_addr = get_fpga_base() + FPGA_GENERAL_REG_BASE;

    return (register_tests(base_addr, &fpga_general_reg_table[0]));
}


/**********************************************************************
 *
 * Function: fpga_tdmsw_reg_test()
 *
 * This function will test FPGA TDMSW registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_tdmsw_reg_test ()
{
    ulong base_addr;

    base_addr = TDMSW64_REG_BASE;

    return (register_tests(base_addr, &tdmsw64_reg_table[0]));
}

/**********************************************************************
 *
 * Function: fpga_reg_test()
 *
 * wrapper function for FPGA registers test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_reg_test ()
{
    printf("FPGA general registers test.\n");
    if (fpga_general_reg_test() == PASSED) {
	printf("FPGA TDMSW registers test.\n");
	if (fpga_tdmsw_reg_test() == PASSED) {
	    return (PASSED);
	}
    }
    cterr('f', 0, "FPGA register test failed.");
    return (FAILED);
}


/**********************************************************************
 *
 * Function: fortitude_led_test()
 *
 * This function will test the leds on the Fortitude NGWIC
 *
 * Input : None
 *
 * Output: Always return PASSED to avoid compilation warning
 *
 **********************************************************************
 */
int
fortitude_led_test ()
{
    fpga_reg_t *fpga_reg;
    uint8_t val;
    int i, j, shift, reg_num, port_num, led_num, led_start;

    prpass(testpass, "Fortitude LED test");

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    port_num = get_num_ports();
    if (port_num == 1) {
	reg_num = 1;
	led_num = 1;
    } else if (port_num == 8) {
	reg_num = 4;
	led_num = 2;
    } else {
	/* for 2 and 4 port SKUs */
	reg_num = 2;
	led_num = 2;
    }

    /* turn all leds off */
    val = 0;
    for (i = 0; i < reg_num; i++) {
	fpga_reg->led_ctrl[i] = val;
    }
    sleep(1);

    /* turn all leds yellow */
    val = PORT0_LED_LP_ON | PORT0_LED_AL_ON | PORT1_LED_LP_ON | PORT1_LED_AL_ON;
    for (i = 0; i < reg_num; i++) {
	fpga_reg->led_ctrl[i] = val;
    }
    sleep(1);

    /* turn all leds green */
    val = PORT0_LED_CD_ON | PORT1_LED_CD_ON;
    for (i = 0; i < reg_num; i++) {
	fpga_reg->led_ctrl[i] = val;
    }
    sleep(1);

    /* turn all leds off */
    val = 0;
    for (i = 0; i < reg_num; i++) {
	fpga_reg->led_ctrl[i] = val;
    }
    sleep(1);

    /* For 2 port SKU, the LEDs are mapped to port 0 and port 3. */ 
    for (i = 0; i < reg_num; i++) {
	if (port_num == 2) {
	    if (i == 1) {
		led_start = 1;
		led_num = 2;
	    } else {
		led_start = 0;
		led_num = 1;
	    }
	} else
	    led_start = 0;

	for (j = led_start; j < led_num; j++) {
	    shift = j * LED_PORT_SHIFT;
	    /* turn LP, AL leds yellow */
	    val = ((PORT0_LED_LP_ON << shift) | (PORT0_LED_AL_ON << shift));
	    fpga_reg->led_ctrl[i] = val;
	    sleep(1);
	    /* turn CD led green, AL led yellow */
	    val = ((PORT0_LED_CD_ON << shift) | (PORT0_LED_AL_ON << shift));
	    fpga_reg->led_ctrl[i] = val;
	    sleep(1);
	    /* turn LP led yellow */
	    val = PORT0_LED_LP_ON << shift;
	    fpga_reg->led_ctrl[i] = val;
	    sleep(1);
	    /* turn AL led yellow */
	    val = PORT0_LED_AL_ON << shift;
	    fpga_reg->led_ctrl[i] = val;
	    sleep(1);
	    /* turn CD led green */
	    val = PORT0_LED_CD_ON << shift;
	    fpga_reg->led_ctrl[i] = val;
	    sleep(1);

	    fpga_reg->led_ctrl[i] = 0;
	}
    }
    return(PASSED);
}


/**********************************************************************
 *
 * Function: fpga_intr_test()
 *
 * This function will test INT1 from FPGA to NPU
 *
 * Input : None
 *
 * Output: PASSED/FAILED 
 *
 **********************************************************************
 */
int
fpga_intr_test ()
{
    fpga_reg_t *fpga_reg;
    int i;

    prpass(testpass, "FPGA interrupt test");

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    /* clear all the pending interrupt bits first */
    fpga_reg->fpga_int_event = 0xff;

    /* enable TDMSW_FSYNC_MISS_ERR interrupt for diag test */
    fpga_reg->fpga_int_event_ena = TDMSW_FSYNC_MISS_ERR;

    /* trigger INT1 from FPGA to NPU */
    fpga_reg->fpga_int_diag_test = TDMSW_FSYNC_MISS_ERR;

    /* wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
	if (fpga_reg->fpga_int_diag_test == 0) {
	    break;
	} else {
	    usleep(100);
	}
    }
#ifndef DEBUG
    printf ("fpga_int_diag_test = %#x, fpga_int_event = %#x, fpga_int_event_ena = %#x\n", 
	    fpga_reg->fpga_int_diag_test, fpga_reg->fpga_int_event, fpga_reg->fpga_int_event_ena);

#endif

    /* disable TDMSW_FSYNC_MISS_ERR interrupt after the test */
    fpga_reg->fpga_int_event_ena &= (~TDMSW_FSYNC_MISS_ERR);

    if (i == 5000) {
	printf("Timeout waiting for interrupt to be cleared. fpga_int_diag_test = %#x\n", 
	       fpga_reg->fpga_int_diag_test);
	return (FAILED);
    } else {
	return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: fpga_get_rev()
 *
 * This function will get FPGA revision.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_get_rev ()
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    /* fpga_rev[0] for hour, fpga_rev[1] for day, 
       fpga_rev[2] for month, fpga_rev[3] for year. */
    printf("Fortitude FPGA was built at %x/%x/%x at %x o'clock\n", 
	   fpga_reg->fpga_rev[2], fpga_reg->fpga_rev[1],
	   fpga_reg->fpga_rev[3], fpga_reg->fpga_rev[0]);
    printf("Fortitude FPGA Image Revision: %x.%x.%x (%s)\n", 
	   fpga_reg->fpga_ver[2], fpga_reg->fpga_ver[1],
	   fpga_reg->fpga_ver[0], 
	   (fpga_reg->fpga_ver[3]&0x01)?"Debug Image":"Official Release Image");
}


/**********************************************************************
 *
 * Function: fpga_set_framer_txhiz()
 *
 * This function will set Framer transceiver in High Impedence State.
 *
 * Input : ena - TRUE for enable, FALSE for disable
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_set_framer_txhiz (int ena)
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    if (ena == TRUE) {
	fpga_reg->misc_ctl |= FRAMER_TXHIZ;
    } else {
	fpga_reg->misc_ctl &= ~FRAMER_TXHIZ;
    }
}

/**********************************************************************
 *
 * Function: fpga_reset_framer()
 *
 * This function will reset Framer from FPGA.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_reset_framer ()
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    fpga_reg->misc_ctl |= FRAMER_RST;
}

/**********************************************************************
 *
 * Function: fpga_unreset_framer()
 *
 * This function will take Framer out of reset from FPGA.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_unreset_framer ()
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    fpga_reg->misc_ctl &= (~FRAMER_RST);
}

/**********************************************************************
 *
 * Function: fpga_reset_tdm_pll()
 *
 * This function will reset TDM PLL from FPGA.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_reset_tdm_pll ()
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    fpga_reg->misc_ctl |= TDM_PLL_RST;
}


/**********************************************************************
 *
 * Function: fpga_unreset_tdm_pll()
 *
 * This function will take TDM PLL out of reset from FPGA.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_unreset_tdm_pll ()
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    fpga_reg->misc_ctl &= (~TDM_PLL_RST);
}

/**********************************************************************
 *
 * Function: fpga_set_ctclk_src()
 *
 * This function will set CTCLK src to be either 8KHz or 2MHz.
 *
 * Input : CTCLK_SRC_8K or CCLK_SRC_2M
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_set_ctclk_src (ctclk_src clk)
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    if (clk == CTCLK_SRC_2M)
	fpga_reg->misc_ctl |= CTC_SRC_2M;
    else 
	fpga_reg->misc_ctl &= (~CTC_SRC_2M);
}

/**********************************************************************
 *
 * Function: fpga_set_nor_flash_a23()
 *
 * This function will set NOR flash address23 for FPGA golden image address 
 * range or upgrade image address range. 
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_set_nor_flash_a23 ()
{
    fpga_reg_t *fpga_reg;
    int choice;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    choice = gethex_answer("\nPlease select 0: Golden image address range \n"
                           "              1: Upgrade image address range",
			   0, 0, 1);

    if (choice == 0) 
	fpga_reg->misc_ctl |= NOR_FLASH_A23_INV;
    else 
	fpga_reg->misc_ctl &= (~NOR_FLASH_A23_INV);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fpga_config_tdm_pll()
 *
 * This function will configure TDM PLL if the framer is configured in 
 * slave mode.
 *
 * Input : CLK_MASTER or CLK_SLAVE
 *
 * Output: None
 *
 **********************************************************************
 */
void 
fpga_config_tdm_pll (frmr_clk_mode clk_mode)
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    if (clk_mode = CLK_SLAVE) 
	fpga_reg->pll_ctrl_status[0] = RSYNC_FREQ_8K;
    else
	fpga_reg->pll_ctrl_status[0] = 0;
}

/**********************************************************************
 *
 * Function: fpga_check_tdm_pll()
 *
 * This function will check TDM PLL lock and reference failed status 
 * if the framer is configured in slave mode.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_check_tdm_pll ()
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    printf("\npll_ctrl_status[0] = %#x, pll_ctrl_status[1] = %#x\n", fpga_reg->pll_ctrl_status[0], fpga_reg->pll_ctrl_status[1]);

    if (fpga_reg->pll_ctrl_status[1] & TDMPLL_REF_FAIL) {
	cterr('f',0,"Detect TDM PLL reference clock failed.");
	return (FAILED);
    }

    if (!(fpga_reg->pll_ctrl_status[1] & TDMPLL_LOCK)) {
	cterr('f',0,"TDM PLL is not in lock status.");
	return (FAILED);
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: fpga_reset_tdmsw()
 *
 * This function will reset TDMSW from FPGA.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_reset_tdmsw ()
{
    ulong reg_addr, reg_data;

    reg_addr = TDMSW64_REG_BASE + TDMSW64_CTL_OFFSET;
    if (tdm_rd(reg_addr, 4, &reg_data, 0) == FAILED) {
	return (FAILED);
    }

    return (tdm_wr(reg_addr, 4, (reg_data | TDMSW_RST), 0));
}

/**********************************************************************
 *
 * Function: fpga_unreset_tdmsw()
 *
 * This function will take TDMSW out of reset from FPGA.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_unreset_tdmsw ()
{
    ulong reg_addr, reg_data;

    reg_addr = TDMSW64_REG_BASE + TDMSW64_CTL_OFFSET;
    if (tdm_rd(reg_addr, 4, &reg_data, 0) == FAILED) {
	return (FAILED);
    
    }
    return (tdm_wr(reg_addr, 4, (reg_data & ~TDMSW_RST), 0));
}


/**********************************************************************
 *
 * Function: set_tdmsw_lpbk()
 *
 * This function will configure the specific stream in TDMSW in lpbk or not.
 *
 * Input : stream - the stream number (0-63) to configure.
 *         If stream = TDMSW64_NUM_TDM_STREAM or TDMSW16_NUM_TDM_STREAM, 
 *         configure all the streams.
 *         lpbk - TRUE for lpbk, FALSE for no lpbk
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
set_tdmsw_lpbk (int stream, int lpbk)
{
    ulong lpbk_addr;
    ulong reg_data;
    int str;

    if (stream == TDMSW64_NUM_TDM_STREAM) {
	if (lpbk == TRUE) {
	    reg_data = 0xffff;
	} else {
	    reg_data = 0;
	}
	lpbk_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_LPBK63_32_OFFSET;
	if (tdm_wr(lpbk_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
	lpbk_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_LPBK31_00_OFFSET;
	if (tdm_wr(lpbk_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    } else if ((stream == TDMSW16_NUM_TDM_STREAM) && (get_num_ports() < 4)) {
	if (lpbk == TRUE) {
	    reg_data = 0x00ff;
	} else {
	    reg_data = 0;
	}
	lpbk_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_LPBK31_00_OFFSET;
	if (tdm_wr(lpbk_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}	
    } else {
	if (stream >= 32) {
	    lpbk_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_LPBK63_32_OFFSET;
	    str = stream - 32;
	} else {
	    lpbk_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_LPBK31_00_OFFSET;
	    str = stream;
	} 
	
	if (tdm_rd(lpbk_addr, 4, &reg_data, 0) == FAILED) {
	    return (FAILED);
	}

	if (lpbk == TRUE) {
	    reg_data |= (1 << str);
	} else {
	    reg_data &= ~(1 << str);
	}

	if (tdm_wr(lpbk_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_tdmsw_stream_rate()
 *
 * This function will configure the rate for the specific stream in TDMSW.
 *
 * Input : stream - the stream number (0-63) to configure, if stream = 64, 
 *                  then configure all 64 streams.
 *         rate - TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
set_tdmsw_stream_rate (int stream, stream_rate rate)
{
    ulong rate_addr;
    ulong reg_data;
    int str, i;

    if (stream == TDMSW64_NUM_TDM_STREAM) {
	reg_data = rate;
	for (i = 1; i < 16; i++) 
	    reg_data |= (rate << (i*2));

	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE15_00_OFFSET;
	if (tdm_wr(rate_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE31_16_OFFSET;
	if (tdm_wr(rate_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE47_32_OFFSET;
	if (tdm_wr(rate_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE63_48_OFFSET;
	if (tdm_wr(rate_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    } else if ((stream == TDMSW16_NUM_TDM_STREAM) && (get_num_ports() < 4)) {
	reg_data = rate;
	for (i = 1; i < 16; i++) 
	    reg_data |= (rate << (i*2));

	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE15_00_OFFSET;
	if (tdm_wr(rate_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    } else {
	if ((stream / 16) == 1) {
	    /* 16 <= stream <= 31 */
	    rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE31_16_OFFSET;
	    str = stream - 16;
	} else if ((stream / 16) == 2) {
	    /* 32 <= stream <= 47 */
	    rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE47_32_OFFSET;
	    str = stream - 32;
	} else if ((stream / 16) == 3) {
	    /* 48 <= stream <= 63 */
	    rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE63_48_OFFSET;
	    str = stream - 48;
	} else {
	    /* 0 <= stream <= 15 */
	    rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE15_00_OFFSET;
	    str = stream;
	}
	
	if (tdm_rd(rate_addr, 4, &reg_data, 0) == FAILED) {
	    return (FAILED);
	}
	
	reg_data &= ~(0x11 << (str*2));
	reg_data |= (rate << (str*2));
	
	if (tdm_wr(rate_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    }
    return (PASSED);
}

/*******************************************************************
 *
 * Function: get_tdmsw_stream_rate()
 *
 * Description:
 *      This function return the TDM rate of the given stream.
 *
 * Input :
 *      stream - stream number to get the rate
 *
 * Output: TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M
 *          or -1 if FAILED.
 *
 *******************************************************************
 */
static int 
get_tdmsw_stream_rate (int stream)
{
    ulong rate_addr;
    ulong reg_data;
    int str;

    if ((stream / 16) == 1) {
	/* 16 <= stream <= 31 */
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE31_16_OFFSET;
	str = stream - 16;
    } else if ((stream / 16) == 2) {
	/* 32 <= stream <= 47 */
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE47_32_OFFSET;
	str = stream - 32;
    } else if ((stream / 16) == 3) {
	/* 48 <= stream <= 63 */
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE63_48_OFFSET;
	str = stream - 48;
    } else {
    	/* 0 <= stream <= 15 */
	rate_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_RATE15_00_OFFSET;
	str = stream;
    }

    if (tdm_rd(rate_addr, 4, &reg_data, 0) == FAILED) {
	return (-1);
    }

    return ((reg_data >> (str*2)) & 0x0003);
}

/**********************************************************************
 *
 * Function: set_tdmsw_stream_ena()
 *
 * This function will enable/disable the specific stream in TDMSW.
 *
 * Input : stream - the stream number (0-63) to configure
 *         If stream = TDMSW64_NUM_TDM_STREAM or TDMSW16_NUM_TDM_STREAM, 
 *         configure all the streams.
 *         ena - TRUE for enable, FALSE for disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
set_tdmsw_stream_ena (int stream, int ena)
{
    ulong enbl_addr;
    ulong reg_data;
    int str;

    if (stream == TDMSW64_NUM_TDM_STREAM) {
	if (ena == TRUE) {
	    reg_data = 0xffff;
	} else {
	    reg_data = 0;
	}
	enbl_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_ENBL63_32_OFFSET;
	if (tdm_wr(enbl_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
	enbl_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_ENBL31_00_OFFSET;
	if (tdm_wr(enbl_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    }  else if ((stream == TDMSW16_NUM_TDM_STREAM) && (get_num_ports() < 4)) {
	if (ena == TRUE) {
	    reg_data = 0x00ff;
	} else {
	    reg_data = 0;
	}
	enbl_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_ENBL31_00_OFFSET;
	if (tdm_wr(enbl_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    } else {
	if (stream >= 32) {
	    enbl_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_ENBL63_32_OFFSET;
	    str = stream - 32;
	} else {
	    enbl_addr = TDMSW64_REG_BASE + TDMSW64_STREAM_ENBL31_00_OFFSET;
	    str = stream;
	} 
    
	if (tdm_rd(enbl_addr, 4, &reg_data, 0) == FAILED) {
	    return (FAILED);
	}

	if (ena == TRUE) {
	    reg_data |= (1 << str);
	} else {
	    reg_data &= ~(1 << str);
	}

	if (tdm_wr(enbl_addr, 4, reg_data, 0) == FAILED) {
	    return (FAILED);
	}
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_ngvm_tdm_clock()
 *
 * This function will configure TDM clock and TDM frame sync format for
 * the NGVM TDM bus.
 *
 * Input : tdm_format - can be D16C16, D8C8, STBUS, H100, IOM2, tri-state
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
set_ngvm_tdm_clock (uchar tdm_format)
{
    ulong ngvm_tdm_addr;

    ngvm_tdm_addr = TDMSW64_REG_BASE + NGVMTDM_CTL_OFFSET;

    return (tdm_wr(ngvm_tdm_addr, 4, tdm_format & 0x07, 0));
}


/*******************************************************************
 *
 * Function: show_tdmsw_regs
 *
 * Description: This function will display the contents of the
 *              Fortitude FPGA TDMSW registers
 *
 * Input  : None
 *
 * Output : Always return PASSED to avoid compilation warning
 *
 *******************************************************************
 */
int
show_tdmsw_regs ()
{
    tdmsw64_reg_t *tdmsw_reg;
    ulong reg_data;

    tdmsw_reg = (tdmsw64_reg_t *)(get_fpga_base() + TDMSW64_REG_BASE);

    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_ENBL63_32_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM switch ENBL Register(63-32)  @%#.8x = %#.8x",
	       &tdmsw_reg->enbl_63_32, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_ENBL31_00_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM switch ENBL Register(31-00)  @%#.8x = %#.8x",
	       &tdmsw_reg->enbl_31_00, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_RATE63_48_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM stream Rate Register(63-48) @%#.8x = %#.8x",
	       &tdmsw_reg->rate_63_48, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_RATE47_32_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM stream Rate Register(47_32) @%#.8x = %#.8x",
	       &tdmsw_reg->rate_47_32, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_RATE31_16_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM stream Rate Register(31-16) @%#.8x = %#.8x",
	       &tdmsw_reg->rate_31_16, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_RATE15_00_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM stream Rate Register(15-00) @%#.8x = %#.8x",
	       &tdmsw_reg->rate_15_00, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_LPBK63_32_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM stream Lpbk Register(63-32) @%#.8x = %#.8x",
	       &tdmsw_reg->lpbk_63_32, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_STREAM_LPBK31_00_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM stream Lpbk Register(31-00) @%#.8x = %#.8x",
	       &tdmsw_reg->lpbk_31_00, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+TDMSW64_CTL_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nTDM Control Register     @%#.8x = %#.8x",
	       &tdmsw_reg->tdmsw64_ctl, reg_data);
    }
    if (tdm_rd(TDMSW64_REG_BASE+NGVMTDM_CTL_OFFSET, 4, &reg_data, 0) == PASSED) {
	printf("\nNGVM TDM Control Register     @%#.8x = %#.8x",
	       &tdmsw_reg->ngvmtdm_ctl, reg_data);
    }

    return(PASSED);
}

/*****************************************************************
 *
 * Function: tdmsw_peek_reg()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              TDMSW registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
tdmsw_peek_reg ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;

    base_addr = TDMSW64_REG_BASE;

    offset = gethex_answer("\nEnter TDMSW register offset[0x00 to 0x30]:",
			   0, 0, 0x30);

    /* all the TDMSW registers are 4 bytes aligned */
    offset &= 0xfc;

    if (tdm_rd((base_addr + offset), 4, &reg_data, 0) == PASSED) {
	printf("\n register value @ offset %#x = %#.8x ", 
	       (base_addr+offset), reg_data);
	return PASSED;
    } else {
	return FAILED;
    }
}

/*****************************************************************
 *
 * Function: tdmsw_poke_reg()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              TDMSW registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
tdmsw_poke_reg ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;

    base_addr = TDMSW64_REG_BASE;

    offset = gethex_answer("\nEnter TDMSW register offset[0x00 to 0x30]:",
			   0, 0, 0x30);

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
			     0, 0, 0xffffffff);
   
    /* all the TDMSW registers are 4 bytes aligned */
    offset &= 0xfc;

    if (tdm_wr((base_addr + offset), 4, reg_data, 0)== PASSED) {
	if (tdm_rd((base_addr + offset), 4, &reg_data, 0) == PASSED) {
	    printf("\n register value @ offset %#x = %#.8x ", 
		   (base_addr+offset), reg_data);
	    return PASSED;
	} 
    }
    return FAILED;
}


/*****************************************************************
 *
 * Function: tdmsw_peek_conn_mem()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              TDMSW connection memory.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
tdmsw_peek_conn_mem ()
{
    int i;
    ushort offset;
    ulong mem_data;

    offset = gethex_answer("\nEnter TDMSW memory offset[0x0 to 0x7FFC]:",
			   0, 0, 0x7ffc);

    /* all the TDMSW connection memory locations are 4 bytes aligned */
    offset &= 0x7ffc;

    for (i=0; i<20; i++) {
        if (tdm_rd(offset, 4, &mem_data, 0) == PASSED) {
            printf("\n connection memory @ offset %#x = %#.8x ", offset, 
                   mem_data);
	    return PASSED;
        } else {
	    return FAILED;
        }
        offset += 4;
    }
    return PASSED;
}

/*****************************************************************
 *
 * Function: tdmsw_poke_conn_mem()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              TDMSW connection memory.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
tdmsw_poke_conn_mem ()
{
    ushort offset;
    ulong mem_data;

    offset = gethex_answer("\nEnter TDMSW memory offset[0x0 to 0x7FFC]:",
			   0, 0, 0x7ffc);

    mem_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
			     0, 0, 0xffffffff);
   
    /* all the TDMSW registers are 4 bytes aligned */
    offset &= 0x7ffc;

    if (tdm_wr(offset, 4, mem_data, 0) == PASSED) {
	if (tdm_rd(offset, 4, &mem_data, 0) == PASSED) {
	    printf("\n connection memory @ offset %#x = %#.8x ", 
		   offset, mem_data);
	    return PASSED;
	} 
    }
    return FAILED;
}


/*****************************************************************
 *
 * Function: fpga_peek_reg()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA general registers.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int 
fpga_peek_reg ()
{
    ulong base_addr;
    ushort offset;
    uchar reg_data;
    uchar *reg_p;

    base_addr = (get_fpga_base() + FPGA_GENERAL_REG_BASE);

    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x8f]:",
			   0, 0, 0x8f);

    reg_p = (uchar *)(base_addr + offset);
    reg_data = *reg_p;
    printf("\n register value @%#x = %#x ", (base_addr+offset), reg_data);
    return PASSED;
}

/*****************************************************************
 *
 * Function: fpga_poke_reg()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              FPGA general registers.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int 
fpga_poke_reg ()
{
    ulong base_addr;
    ushort offset;
    uchar reg_data;
    uchar *reg_p;

    base_addr = (get_fpga_base() + FPGA_GENERAL_REG_BASE);

    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x8f]:",
			   0, 0, 0x8f);
    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFF]:", 
			     0, 0, 0xff);
   
    reg_p = (uchar *)(base_addr + offset);
    *reg_p = reg_data;
    printf("\n register value @%#x = %#x ", (base_addr+offset), *reg_p);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: fpga_peek_dump_mem()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA DS0 dump memory location.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int 
fpga_peek_dump_mem ()
{
    ulong base_addr;
    ushort offset;
    uchar mem_data;
    uchar *mem_p;

    base_addr = (get_fpga_base() + DS0_DUMP_BUFFER_BASE);

    offset = gethex_answer("\nEnter FPGA dump memory offset[0x00 to 0x1FFF]:",
			   0, 0, 0x1fff);

    mem_p = (uchar *)(base_addr + offset);
    mem_data = *mem_p;
    printf("\n DS0 dump memory value @%#x = %#x ", (base_addr+offset), mem_data);
    return PASSED;
}


/*****************************************************************
 *
 * Function: rd_verify_wr_conn_mem
 *
 * Description:
 *      This function will perform a read-verify-write test
 *      through memory using word accesses.
 *	Algorithm:
 *	Read the contents of 'addr' and compare against 'expval'.
 *	If compare OK, write with 'wrval'.  Cycle through memory
 *	for 'size' int in the direction, and using the address
 *	offset, specified in 'dir_val' (direction is a function
 *	of whether the offset value is positive or negative).
 *
 * Input :
 *	size - number of bytes
 *	rd_mask - read mask
 *	wrval - write value
 *	expval - expected value
 *	dir_val - direction to go through memory and by how much
 *	     1 = increment through memory, one word at a time
 *	    -1 = decrement through memory, one word at a time
 *	caddr - pointer to start address of memory test
 *
 * Output:
 *	1 - FAILED, 0 - PASSED
 *
 *****************************************************************
 */
static int
rd_verify_wr_conn_mem (int size, ulong rd_mask, ulong wrval, ulong expval,
		       int direction, ulong caddr)
{
    int count;
    ulong offset, rdval, mem_addr;

    if ((NVRAM)->diagflag & D_VERBOSE)
        printf("\nrd_verify_wr from %#x %s for %#.8x with %#.8x, bkgnd %#.8x\n",
	       caddr, direction == MEM_INCREMENT_1 ? "up" : "down",
	       size, wrval, expval);

    offset = 0;
    for (count = 0; count < size/4; count++) {
	if (direction == MEM_INCREMENT_1) {
	    mem_addr = caddr + offset;
	} else {  /* MEM_DECREMENT_1 */
	    mem_addr = caddr - offset;
	}
	if (tdm_rd(mem_addr, 4, &rdval, 0) == FAILED) {
	    cterr('f',0,"Failed to read from TDM connection memory @ %#x", 
		  mem_addr);
	    return (FAILED);
	}
	rdval &= rd_mask;
	if (rdval != expval) {
	    cterr('f', 0, "%s conn mem err @ %#x, read %#.8x, expect %#.8x",
		  direction == MEM_INCREMENT_1 ? "inc" : "dec", mem_addr, 
		  rdval, expval);	    
	    return(FAILED);
	}
	if (tdm_wr(mem_addr, 4, wrval, 0) == FAILED) {
	    cterr('f',0,"Failed to write to TDM connection memory @ %#x", 
		  mem_addr);
	    return (FAILED);
	}
	offset += 4;
    }
    return(PASSED);
}


/*****************************************************************
 *
 * Function: fpga_conn_mem_test()
 *
 * Description:
 *	This function will test the tdm connection memory.
 *	The connection memory maps input stream/timeslot to
 *	output stream/timeslot.  The connection memory is
 *	addressed using the output stream/timeslot number.
 *	the content of the output stream/timeslot address
 *	includes the input stream/timeslot number as well
 *	as other control bits.
 *
 *	Will test using 8Mbps speed.
 *
 *	for 2Mbps stream:
 *	    64 streams, 32 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) + (o_timeslot * 4 * 4)
 *
 *	for 8Mbps stream:
 *	    64 streams, 128 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) + (o_timeslot * 4)
 *
 *	for 16Mbps stream:
 *	    64 streams, 256 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) 
 *                      + ((o_timeslot%2) * 128 * 4) + ((o_timeslot>>1) * 4
 *
 *	for 32Mbps stream:
 *	    64 streams, 512 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) 
 *                      + ((o_timeslot%4) * 128 * 4) + ((o_timeslot>>2) * 4
 *
 *	connection memory map is as follows:
 *	    stream  0, timeslot 0 - 127, addr offset 0x0000
 *	    stream  1, timeslot 0 - 127, addr offset 0x0200
 *		. 	.	.
 *	    stream 30, timeslot 0 - 127, addr offset 0x3c00
 *	    stream 31, timeslot 0 - 127, addr offset 0x3e00
 *              .       .       .
 *          stream 32, timeslot 0 - 127, addr offset 0x4000
 *              .       .       .
 *          stream 63, timeslot 0 - 127, addr offset 0x7e00
 *
 *	the connection memory must be accessed as an int.
 *
 * Input : None
 *
 * Output:
 *	1 - FAILED, 0 - PASSED
 *
 *****************************************************************
 */
static int
fpga_conn_mem_test ()
{
    ulong bkgnd, forgnd, cid, rdval;
    int retval = PASSED;
    uint count, i, stream, timeslot, conn_mem_size, tdm_num;
    int buf[5] = {0x00005aa5, 0x00003cc3, 0x00006996, 0x00000f0f, 0x00000000};
    ulong caddr, offset;

    prpass(testpass, "FPGA Connection Memory test");

    offset = 0;
    if (get_num_ports() < 4) {
	conn_mem_size = TDMSW16_CONN_MEM_SIZE;
	tdm_num = TDMSW16_NUM_TDM_STREAM;
    } else {
	conn_mem_size = TDMSW64_CONN_MEM_SIZE;
	tdm_num = TDMSW64_NUM_TDM_STREAM;
    }

#ifdef DEBUG
    printf("conn_mem_size = %#x, port_num = %d\n", conn_mem_size, get_num_ports());
#endif

    /* disable all output enables */
    if (set_tdmsw_stream_ena(tdm_num, FALSE) == FAILED) {
	cterr('f',0,"Failed to disable 64 TDM streams");
	return (FAILED);
    }

    /*
     * initialize connection memory
     * increment through memory
     */
    forgnd = TDMSW64_CM_PASSWORD | TDMSW64_CM_FORCEBYTE | 
	     TDMSW64_CM_ODRV | TDMSW64_CM_FORCELSB;
    for (i = 0; i < conn_mem_size/4; i++) {
	if (tdm_wr(offset, 4, forgnd, 0) == FAILED) {
	    cterr('f',0,"Failed to write to TDM connection memory @ offset %#x", 
		  offset);
	    return (FAILED);
	}
	offset += 4;
    }

    /*
     * connection memory test
     * a modified March C memory test
     */
    for (count = 0; count < 5; count++) {
        prpass(testpass, "TDM connection memory march test %d ", count);
	bkgnd = forgnd & TDMSW64_CONN_MEM_DATA_MASK;
	forgnd = buf[count] | TDMSW64_CM_PASSWORD;
	caddr = 0;
	/* increment through memory */
	if (rd_verify_wr_conn_mem(conn_mem_size, 
				  TDMSW64_CONN_MEM_DATA_MASK, 
				  forgnd, bkgnd, MEM_INCREMENT_1, caddr)) {
	    retval = FAILED;
	    break;
	}
	bkgnd = forgnd & TDMSW64_CONN_MEM_DATA_MASK;
	forgnd = (~forgnd & TDMSW64_CONN_MEM_PW_MASK) | TDMSW64_CM_PASSWORD;
	caddr = conn_mem_size - 4;
	if (rd_verify_wr_conn_mem(conn_mem_size, 
				  TDMSW64_CONN_MEM_DATA_MASK, 
				  forgnd, bkgnd, MEM_DECREMENT_1, caddr)) {
	    retval = FAILED;
	    break;
	}
    }
    if (retval == PASSED) {
        /*
         * connection memory address test
         * addr (output stream/timeslot) = input stream/timeslot
         * increment through memory (size * 4 because each
         * stream/timeslot is accessed as an int)
         */
        prpass(testpass, "TDM connection memory address test ");
	offset = 0;
        for (stream = 0; stream < tdm_num; stream++) {
	    for (timeslot = 0; timeslot < NUM_8M_TIMESLOTS; timeslot++) {
		cid = stream * 128 + timeslot;
#ifdef DEBUG
		printf("stream = %d, ts = %d, cid = %#x, offset = %#x\n", 
		       stream, timeslot, cid, offset);
#endif
		if (tdm_wr(offset, 4, cid | TDMSW64_CM_PASSWORD, 0) 
		    == FAILED) {
		    cterr('f',0,"Failed to write to TDM connection memory @ %#x", 
			  offset);
		    return (FAILED);
		}
		offset += 4;
	    }
        }
	offset = 0;
        for (stream = 0; stream < tdm_num; stream++) {
	    for (timeslot = 0; timeslot < NUM_8M_TIMESLOTS; timeslot++) {
	        bkgnd = (stream * 128) | timeslot | TDMSW64_CM_PASSWORD;
		if (tdm_rd(offset, 4, &rdval, 0) == FAILED) {
		    cterr('f',0,"Failed to read from TDM connection memory @ %#x", 
			  offset);
		    return (FAILED);
		}
		offset += 4;
		rdval &= TDMSW64_CONN_MEM_DATA_MASK;
	        if (rdval != bkgnd) {
		    cterr('f', 0, "conn mem addr err @%#.8x, "
			"stream %.2x, tslot %.2x, read %#.8x, expect %#.8x",
			 offset, stream, timeslot, rdval, bkgnd);
		    return (FAILED);
	        }
	    }
	}
    }
    return (retval);
}


/**********************************************************************
 *
 * Function: fpga_mem_test()
 *
 * wrapper function for FPGA connection memory and DS0 dump memory test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fpga_mem_test ()
{
    prpass(testpass, "FPGA connection memory test");
    if (fpga_conn_mem_test() == PASSED) {
	return (PASSED);
    }
    cterr('f', 0, "FPGA memory test failed.");
    return (FAILED);
}

/*******************************************************************
 *
 * Function:
 *      tdm_get_mem_offset
 *
 * Description:
 *      This function will return the tdm connection memory
 *      address offset for the given output stream, tslot, rate
 *	The equations are specified in the Fortitude FPGA
 *	Hardware Functional Specification, EDCS-1106187. 
 *
 * Input  :
 *      stream   - Stream number to get connection memory offset
 *      timeslot - DS0 number of the stream
 *      rate     - TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M
 *
 * Output :
 *      cm_offset or -1 if FAILED
 *
 *******************************************************************
 */
static int
tdm_get_mem_offset (uint stream, uint tslot, stream_rate rate)
{
    int cm_offset;

    cm_offset = -1;
    if (stream >= TDMSW64_NUM_TDM_STREAM)
	return (-1);

    switch (rate) {
	case TDM_STREAM_2M:
	    if (tslot < NUM_2M_TIMESLOTS) {
		cm_offset = ((stream * 128) + (tslot * 4)) * 4;
	    }
	    break;

	case TDM_STREAM_8M:
	    if (tslot < NUM_8M_TIMESLOTS) {
		cm_offset = ((stream * 128) + tslot) * 4;
	    }
	    break;

	case TDM_STREAM_16M:
	    if (tslot < NUM_16M_TIMESLOTS) {
		cm_offset = ((stream*128) + (tslot%2)*128 + (tslot>>1)) * 4;
	    }
	    break;

	case TDM_STREAM_32M:
	    if (tslot < NUM_32M_TIMESLOTS) {
		cm_offset = ((stream*128) + (tslot%4)*128 + (tslot>>2)) * 4;
	    }
	    break;

	default:
	    break;
    }

    return(cm_offset);
}

/*******************************************************************
 *
 * Function:
 *      tdm_get_input_cid
 *
 * Description:
 *      This function will return the input CID for the given input
 *      stream, input time slot and rate.
 *
 * Input  :
 *      stream   - Input Stream number
 *      timeslot - DS0 number of the input stream
 *      rate     - TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M
 *
 * Output :
 *      input CID on success or -1 if FAILED
 *
 *******************************************************************
 */
static int
tdm_get_input_cid (uint stream, uint tslot, stream_rate rate)
{
    int cid;

    cid = -1;
    if (stream >= TDMSW64_NUM_TDM_STREAM)
	return (-1);

    switch (rate) {
	case TDM_STREAM_2M:
	    if (tslot < NUM_2M_TIMESLOTS) {
		cid = (stream * 128) + (tslot * 4) + 3;
	    }
	    break;

	case TDM_STREAM_8M:
	    if (tslot < NUM_8M_TIMESLOTS) {
		cid = (stream * 128) + tslot;
	    } 
	    break;

	case TDM_STREAM_16M:
	    if (tslot < NUM_16M_TIMESLOTS) {
		cid = (stream * 128) + (tslot%2)*128 + (tslot>>1);
	    }
	    break;

	case TDM_STREAM_32M:
	    if (tslot < NUM_32M_TIMESLOTS) {
		cid = (stream * 128) + (tslot%4)*128 + (tslot>>2);
	    }
	    break;

	default:
	    break;
    }
    return(cid);
}

/*******************************************************************
 *
 * Function:
 *	tdm_stream_connect
 *
 * Description:
 *	This function will make the output stream to input
 *	stream connection for the specified tdm stream.  The
 *	output stream/timeslot indexes into the connection
 *	memory.  The input stream/timeslot value is contained
 *	in the connection memory location indexed to by the
 *	output stream/timeslot.
 * Notes:
 *	The rate of input and output streams are determined dynamically
 *	by reading the rate register, so make sure the streams' rates are
 *	configured correctly before calling this function since that
 *	defines the maximum number of timeslots which can be connected.
 *
 * Input  : 
 *      istrm  - input stream
 *      itslot - input timeslot
 *      ostrm  - output stream
 *      otslot - output timeslot
 *      connect_sel - mode select: TDMSW64_CM_FORCEBYTE, TDMSW64_CM_FORCELSB,
 *				TDMSW64_CM_ODRV
 *
 * Output : PASSED of connect successfully, FAILED otherwise
 *
 *******************************************************************
 */
static int
tdm_stream_connect (int istrm, int itslot, int ostrm, int otslot, int connect_sel)
{
    int offset, orate, irate, input_cid;
    ulong rd_val;

    /* Determine output stream rate and connection memory offset */
    orate = get_tdmsw_stream_rate(ostrm);
    if (orate == -1) {
	cterr('f', 0, "Failed to set rate for stream %d", ostrm);
	return(FAILED);
    }
    offset = tdm_get_mem_offset(ostrm, otslot, orate);
    if (offset == -1) {
	cterr('f', 0, "Failed to get connection memory offset. "
	      "output stream %d/tslot %d/rate %dMbps", 
	      ostrm, otslot, tdm_rate[orate]);
	return(FAILED);
    }
    irate = get_tdmsw_stream_rate(istrm);
    if (irate == -1) {
	cterr('f', 0, "Failed to set rate for stream %d", istrm);
	return(FAILED);
    }
    input_cid = tdm_get_input_cid (istrm, itslot, irate);
    if (input_cid == -1) {
	cterr('f', 0, "Failed to get input cid. "
	      "input stream %d/tslot %d/rate %dMbps", 
	      istrm, itslot, tdm_rate[irate]);
	return(FAILED);
    }

    /* Write input CID to the connection memory */
    if (tdm_wr(offset, 4, TDMSW64_CM_PASSWORD|connect_sel|input_cid, 0)
	== FAILED) {
	cterr('f',0,"Failed to write to TDM connection memory @ offset %#x", 
	      offset);
	return (FAILED);
    }
    
    if (((NVRAM)->diagflag & D_VERBOSE) && (istrm != ostrm) && 
	(itslot != otslot)) {
	tdm_rd(offset, 4, &rd_val, 0);
	printf("stream %.2d: in stream %.2d/ts %.3d, out stream %.2d/ts %.3d, "
	       "connection memory @ offset %#.8x = %#.4x\n", 
	       ostrm, istrm, itslot, ostrm, otslot, offset, rd_val);
    }

    return(PASSED);
}

/*******************************************************************
 *
 * Function:
 *	tdm_stream_disconnect
 *
 * Description:
 *	This function will disable one output
 *	stream/timeslot connection of the tdm
 *	
 *
 * Input  : 
 *	output stream
 *	output timeslot
 *
 * Output :
 *	1 - FAILED, 0 - PASSED
 *
 *******************************************************************
 */
static int
tdm_stream_disconnect (int ostrm, int otslot)
{
    int offset, orate;
    ulong rd_val;

    /* Determine output stream rate and connection memory offset */
    orate = get_tdmsw_stream_rate(ostrm);
    if (orate == -1) {
	cterr('f', 0, "Failed to set rate for stream %d", ostrm);
	return(FAILED);
    }
    offset = tdm_get_mem_offset(ostrm, otslot, orate);
    if (offset == -1) {
	cterr('f', 0, "Failed to get connection memory offset. "
	      "output stream %d/tslot %d/rate %dMbps", 
	      ostrm, otslot, tdm_rate[orate]);
	return(FAILED);
    }
    /* Write to the connection memory */
    if (tdm_wr(offset, 4, TDMSW64_CM_PASSWORD, 0) == FAILED) {
	cterr('f',0,"Failed to write to TDM connection memory @ offset %#x", 
	      offset);
	return (FAILED);
    }
    usleep(3000);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
	tdm_rd(offset, 4, &rd_val, 0);
	printf("stream %.2d: out stream %.2d/ts %.3d, "
	       "connection memory @ offset %#.8x = %#.4x\n",
	       ostrm, ostrm, otslot, offset, rd_val);
    }

    return(PASSED);
}

/*******************************************************************
 *
 * Function:
 *      tdm_disconnect
 *
 * Description:
 *      This function will disable multiple output stream
 *      connections of the tdm, for the specified number of timeslots.
 *
 * Input  :
 *      start_stream
 *      num_streams
 *	start_timeslot
 *	num_timeslots
 *
 * Output :
 *      1 - FAILED, 0 - PASSED
 *
 *******************************************************************
 */
static int
tdm_disconnect (int stream_from, int num_streams,
		int tslot_from, int num_tslots)
{
    int i, conn_mem_size;
    ulong offset = 0;

    if (get_num_ports() < 4)
	conn_mem_size = TDMSW16_CONN_MEM_SIZE;
    else 
	conn_mem_size = TDMSW64_CONN_MEM_SIZE;

    for (i = 0; i < conn_mem_size/4; i++) {
	if (tdm_wr(offset, 4, TDMSW64_CM_PASSWORD, 0) == FAILED) {
	    cterr('f',0,"Failed to write to TDM connection memory @ offset %#x", 
		  offset);
	    return (FAILED);
	}
	offset += 4;
    }

    return(PASSED);
}


/*******************************************************************
 *
 * Function:
 *      tdm_cleanup
 *
 * Description:
 *      This function will put the tdm in normal operation
 *      mode and disable the outputs
 *
 * Input  :
 *
 * Output : PASSED/FAILED
 *
 *******************************************************************
 */
int
tdm_cleanup ()
{
    uint stream, tdm_num;

    if (get_num_ports() < 4) {
	tdm_num = TDMSW16_NUM_TDM_STREAM;
    } else {
	tdm_num = TDMSW64_NUM_TDM_STREAM;
    }

    /* set rate to 8Mbps to allow a single call to disconnect */
    if (set_tdmsw_stream_rate(tdm_num, TDM_STREAM_8M) == FAILED) {
	cterr('f',0,"Failed to configure rate for 64 TDM streams");
	return (FAILED);
    }

    /* disable tdm stream outputs, do not drive the timeslots */
    if (tdm_disconnect(0, tdm_num, 0, NUM_8M_TIMESLOTS)
	== FAILED) {
	cterr('f',0,"Failed to disconnect all TDM streams.");
	return (FAILED);
    }

    /* disable output enables */
    if (set_tdmsw_stream_ena(tdm_num, FALSE) == FAILED) {
	cterr('f',0,"Failed to disable TDM streams");
	return (FAILED);
    }
    /* put tdm into normal operation mode */
    if (set_tdmsw_lpbk(tdm_num, FALSE) == FAILED) {
	cterr('f',0,"Failed to disable lpbk for TDM streams");
	return (FAILED);
    }

    /* set TDM streams in default rate 
       For TDMSW64:
       stream 0 - 31: 2Mbps
       stream 52 - 59: 16Mbps
       stream 61: 8Mbps
       For TDMSW16:
       stream 0 - 7: 2Mbps
       stream 8 - 11: 8Mbps
       stream 13: 8Mbps
    */
    if (get_num_ports() < 4) {
	for (stream = 0; stream < 8; stream++) {
	    if (set_tdmsw_stream_rate(stream, TDM_STREAM_2M) == FAILED) {
		cterr('f',0,"Failed to configure rate for TDM stream %d", stream);
		return (FAILED);
	    }
	}
	for (stream = 8; stream < 12; stream++) {
	    if (set_tdmsw_stream_rate(stream, TDM_STREAM_8M) == FAILED) {
		cterr('f',0,"Failed to configure rate for TDM stream %d", stream);
		return (FAILED);
	    }
	}
	if (set_tdmsw_stream_rate(13, TDM_STREAM_8M) == FAILED) {
	    cterr('f',0,"Failed to configure rate for TDM stream 13");
	    return (FAILED);
	}
    } else {
	for (stream = 0; stream < 32; stream++) {
	    if (set_tdmsw_stream_rate(stream, TDM_STREAM_2M) == FAILED) {
		cterr('f',0,"Failed to configure rate for TDM stream %d", stream);
		return (FAILED);
	    }
    }

	for (stream = 52; stream < 60; stream++) {
	    if (set_tdmsw_stream_rate(stream, TDM_STREAM_16M) == FAILED) {
		cterr('f',0,"Failed to configure rate for TDM stream %d", stream);
		return (FAILED);
	    }
	}
	if (set_tdmsw_stream_rate(61, TDM_STREAM_8M) == FAILED) {
	    cterr('f',0,"Failed to configure rate for TDM stream 61");
	    return (FAILED);
	}
    }

    /* allow tdm switch to free run */
    if (fpga_reset_tdmsw() == FAILED) {
	cterr('f',0,"Failed to reset TDMSW");
	return (FAILED);
    }	

    if (fpga_unreset_tdmsw() == FAILED) {
	cterr('f',0,"Failed to take TDMSW out of reset");
	return (FAILED);
    }	

    /* allow tdm time to stabilize */
    usleep(5000);
}


/**********************************************************************
 *
 * Function: set_tdmsw_lpbk_test()
 *
 * This function will do all the neccessary setup within TDMSW for 
 * either TDM loopback test or framer loopback tests.
 * Test will be performed at 2Mbps, each timeslot
 * tested individually, then all timeslots at the same time.
 *
 * For this test, the framer is not put into clock master mode.
 * When in clock master mode, the hdlc is tied directly to the
 * framer and does not need to go through the tdm switch.
 * This test is performed in clock slave mode (hdlc access to
 * framer through tdm) since diags cannot test in clock master
 * mode due to inability to get a recovered clock from the PMC
 * without performing a back-to-back transfer).
 *
 * Input : 
 *	   port
 *	   mode - CMQ_MODE_T1, CMQ_MODE_E1
 *         lpbk - TRUE: TDM loopback
 *                FALSE: TDM not in loopback mode
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
set_tdmsw_lpbk_test(int port, int mode, int lpbk)
{
    int stream, npu_stream, framer_stream;
    int ts_cnt, num_ts, ts_start;
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    npu_stream = port * 4;
    framer_stream = npu_stream + 1;
    ts_start = 0;
    /* TDMSW loopback only supports E1 mode. BP clock 2.048M. */
    num_ts = NUM_2M_TIMESLOTS;

    printf("\nnpu_stream = %d, framer_stream = %d, ts_start = %d, num_ts = %d\n",
	   npu_stream, framer_stream, ts_start, num_ts);

    fpga_reset_tdmsw();
    usleep(1000);
    fpga_unreset_tdmsw();
    usleep(1000);

    if (set_tdmsw_stream_rate(npu_stream, TDM_STREAM_2M) == FAILED) {
	cterr('f',0,"Failed to configure rate for TDM stream %d", 
	      npu_stream);
	return (FAILED);
    }  
    if (set_tdmsw_stream_rate(framer_stream, TDM_STREAM_2M) == FAILED) {
	cterr('f',0,"Failed to configure rate for TDM stream %d", 
	      framer_stream);
	return (FAILED);
    }   

    if (lpbk == TRUE) {
	if (set_tdmsw_lpbk(framer_stream, TRUE) == FAILED) {
	    cterr('f',0,"Failed to enable lpbk for TDM stream %d", 
		  framer_stream);
	    return (FAILED);		
	}  
    }

    /* enable output stream */
    if (set_tdmsw_stream_ena(npu_stream, TRUE) == FAILED) {
	cterr('f',0,"Failed to enable stream %d", npu_stream);
	return (FAILED);
    }

    if (set_tdmsw_stream_ena(framer_stream, TRUE) == FAILED) {
	cterr('f',0,"Failed to enable stream %d", framer_stream);
	return (FAILED);
    }	

    /*
     * map connection memory from hdlc stream to framer data and signal 
     * streams. only testing data stream since signal stream does not 
     * loopback the data. test each 2 Mbps timeslots individually.
     */ 
    for(ts_cnt = ts_start; ts_cnt < num_ts; ts_cnt++) {
	if (tdm_stream_connect(npu_stream, ts_cnt, framer_stream, ts_cnt,
			       TDMSW64_CM_ODRV) == FAILED) {
	    cterr('f',0,"Failed to connect input stream %d with output "
		  "stream %d", npu_stream, framer_stream);
	    return FAILED;
	}
	
	if (tdm_stream_connect(framer_stream, ts_cnt, npu_stream, ts_cnt,
			       TDMSW64_CM_ODRV) == FAILED) {
	    cterr('f',0,"Failed to connect input stream %d with output "
		  "stream %d", framer_stream, npu_stream);
	    return FAILED;
	}
    }
}

int get_cid (int rate, int strm, int ds_num)
{
    if (rate == TDM_STREAM_2M)
        return ((strm * 128) + (ds_num * 4) + 3);
    else if (rate == TDM_STREAM_16M) {
        return ((strm * 128) + ((ds_num % 2)* 128) + (ds_num >> 1));
    } else {
        printf("%s(): Stream rate %d not supported", __FUNCTION__, rate);
        return (FAILED);
    }
}

int get_addr (int rate, int strm, int ds_num)
{
    if (rate == TDM_STREAM_2M)
        return ((strm * 128) + (ds_num * 4)) * 4;
    else if (rate == TDM_STREAM_16M) {
        return (((strm * 128) + ((ds_num % 2)* 128) + (ds_num >> 1))*4);
    } else {
        printf("%s(): Stream rate %d not supported", __FUNCTION__, rate);
        return (FAILED);
    }
}

int set_tdm_stream_lpbk (int stream)
{
    ulong offset, mem_data;
    int i, cid;

    fpga_reset_tdmsw();
    usleep(1000);
    fpga_unreset_tdmsw();
    usleep(1000);

    /* Set up connection memory to do the loopback */
    if (set_tdmsw_stream_rate(stream, TDM_STREAM_16M) == FAILED) {
	cterr('f',0,"Failed to configure rate for TDM stream %d", 
	      stream);
	return (FAILED);
    } 
    offset = get_addr(TDM_STREAM_16M, stream, 0);
    printf("\n Start offset in connection memory for stream %d = 0x%x\n",
           stream, offset);
    for (i=0; i < 256; i++) {

        cid = get_cid(TDM_STREAM_16M, stream, i);
        mem_data = ((0xCAC << 20) | (1 << 15) | cid);
        offset = get_addr(TDM_STREAM_16M, stream, i);
        if (tdm_wr(offset, 4, mem_data, 0) == FAILED) {
            cterr('f', 0, "Write to connection memory at offset 0x%x with data 0x%x"
                   " failed", offset, mem_data);
            return (FAILED);
        }
    }
    /* enable output stream */
    if (set_tdmsw_stream_ena(stream, TRUE) == FAILED) {
        cterr('f',0,"Failed to enable stream %d", stream);
        return (FAILED);
    }
}

int config_ngvm_tdm_lpbk (void)
{
    int ngvm_str1, ngvm_str2;

    if (get_num_ports() < 4) {
        ngvm_str1 = TDMSW16_NGVM_STREAM_1;
        ngvm_str2 = TDMSW16_NGVM_STREAM_2;
        printf("\n Use stream 8, 10\n");
    } else {
        ngvm_str1 = TDMSW64_NGVM_STREAM_1;
        ngvm_str2 = TDMSW64_NGVM_STREAM_2;
        printf("\n Use stream 52, 56\n");
    }
    if (set_tdm_stream_lpbk(ngvm_str1) == FAILED)
        return (FAILED);
    if (set_tdm_stream_lpbk(ngvm_str2) == FAILED)
        return (FAILED);
}

/*****************************************************************
 *
 * Function: tdmsw_force_byte_test()
 *
 * Description:
 *	This function will test the force byte feature of the
 *	tdm connection memory as well as the DS0 dump feature.
 *	This test uses the LS 8 bits of connection memory
 *	for data generation and reads the contents of DS0 dump
 *	buffer for data verification.
 *	TDMSW64_CM_FORCEBYTE, when set, forces the LS 8 bits of
 *	connection memory to be output to stream out instead
 *	of the data coming in from the input stream.
 *	Since force byte affects the output, we must connect
 *	the test tdm stream with the DS0 dump stream (#61)
 *      so that we can see the actual data that is being sent out.  
 *
 *	Algorithm:
 *	Initialize and verify contents of DS0 dump buffer.
 *	Then setup connection memory.  Then enable tdm out.
 *	Now read the contents of
 *	DS0 dump buffer and verify that it is modified by the
 *	contents of connection memory.  The test is performed
 *	on stream 0, all 32 timeslots to stream 61, all 128 timeslot.
 *
 * Input :
 *	None
 *
 * Output:
 *	1 - FAILED, 0 - PASSED
 *
 *****************************************************************
 */
int
tdmsw_force_byte_test ()
{
    int istrm, itslot, ostrm, otslot, offset, i;
    ulong base_addr;
    uchar *data_ptr;
    fpga_reg_t *fpga_reg;
    uchar rd_data;
    uchar data_pattern = 0xaa;

    prpass(testpass, "TDMSW force byte test ");

    base_addr = get_fpga_base();
    data_ptr = (uchar *)(base_addr + DS0_DUMP_BUFFER_BASE);
    fpga_reg = (fpga_reg_t *)(base_addr + FPGA_GENERAL_REG_BASE);

    istrm = 0;
    itslot = 0;
    otslot = 0;

    if (get_num_ports() < 4)
	ostrm = TDM16_DS0_DUMP_STREAM;
    else
	ostrm = TDM64_DS0_DUMP_STREAM;

    if (set_tdmsw_stream_rate(istrm, TDM_STREAM_8M) == FAILED) {
	cterr('f',0,"Failed to configure rate for TDM stream %d", 
	      istrm);
	return (FAILED);
    } 

    if (set_tdmsw_stream_rate(ostrm, TDM_STREAM_8M) == FAILED) {
	cterr('f',0,"Failed to configure rate for TDM stream %d", 
	      ostrm);
	return (FAILED);
    } 

    /* connect istrm/itslot with ostrm/otslot */
    offset = ((ostrm * 128) + otslot) * 4;
    if (tdm_wr(offset, 4, TDMSW64_CM_PASSWORD | TDMSW64_CM_FORCEBYTE |
	       TDMSW64_CM_ODRV | data_pattern, 0) == FAILED) {
	cterr('f',0,"Failed to write to TDM connection memory @ %#x", 
	      (base_addr + offset));
	return (FAILED);
    }
    usleep(30000);

    /* enable output stream */
    if (set_tdmsw_stream_ena(istrm, TRUE) == FAILED) {
	cterr('f',0,"Failed to enable stream %d", istrm);
	return (FAILED);
    }

    if (set_tdmsw_stream_ena(ostrm, TRUE) == FAILED) {
	cterr('f',0,"Failed to enable stream %d", ostrm);
	return (FAILED);
    }    

    /* start DS0 dump */
    fpga_reg->ds0_dump_ctrl = DS0_DUMP_GO | DS0_DUMP_TOP_RDY | DS0_DUMP_BOT_RDY;

    /* read from DS0 dump buffer to verify the data */
    for (i = 0; i < 1000; i++) {
	if ((fpga_reg->ds0_dump_ctrl & DS0_DUMP_TOP_RDY) &&
	    (fpga_reg->ds0_dump_ctrl & DS0_DUMP_BOT_RDY))
	    break;
	usleep(5000);
    }
    
    if (i == 1000) {
	cterr('f',0,"Timeout waiting for DS0 dump buffer ready, ds0_dump_ctrl=%#x", fpga_reg->ds0_dump_ctrl);
	return (FAILED);
    }

    data_ptr = (uchar *)(base_addr + DS0_DUMP_BUFFER_BASE);
    for (i = 0; i < DS0_DUMP_BUFFER_SIZE; i++) {
	rd_data = *data_ptr;
	if (rd_data != data_pattern) {
	    cterr('f',0,"DS0 dump buffer @%#x contains mismatch data. "
		  "expect = %#x, read = %#x", data_ptr, data_pattern, rd_data);
	    return (FAILED);
	}
	data_ptr++;
    }
    
    /* reset RDY bit and stop dump */
    fpga_reg->ds0_dump_ctrl = DS0_DUMP_TOP_RDY | DS0_DUMP_BOT_RDY;

    printf("\nTest passes! Doing cleanup!\n");

    return (tdm_cleanup());
}


/*****************************************************************
 *
 * Function: generate_nclock()
 *
 * Description: This function generates number of clocks from 
 *              NPU GPIO pins.
 *
 * Input: gpvr_ptr - pointer to NPU GPIO register
 *        n - number of clocks
 *
 * Output: void
 *
 *****************************************************************/
static void
generate_nclock (unsigned int *gpvr_ptr, int n)
{
    int ix, i;
   
    for (ix = 0; ix < n; ix++) {
	*gpvr_ptr &= ~FPGA_CCLK;
	for (i = 0; i < 10; i++)
	    ;
 
	*gpvr_ptr |= FPGA_CCLK;
	for (i = 0; i < 10; i++)
	    ;
     }
   
    return;
}


/*****************************************************************
 *
 * Function: fortitude_fpga_download()
 *
 * Description: This function downloads Fortitude PFGA image through
  *             NPU GPIO pins.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int
fortitude_fpga_download () 
{
    uint ctr,cunt, port;
    int ix, iz;
    unsigned long gpdr_addr, gpvr_addr;
    unsigned int *gpdr_ptr, *gpvr_ptr, *pad_ptr;
    unsigned char *fortitude_image_fw;
    int fortitude_image_fw_size;

    printf("\nStart FPGA download!\n");

    gpdr_addr = get_npu_rif_base() + NPU_GPDR_OFFSET;
    gpvr_addr = get_npu_rif_base() + NPU_GPVR_OFFSET;
    pad_ptr = (unsigned int *)(get_npu_rif_base() + 0x8044);

    gpdr_ptr = (unsigned int *)gpdr_addr;
    gpvr_ptr = (unsigned int *)gpvr_addr;

    /* set GPIO output pins */
    *gpdr_ptr = 0x11c0;
    usleep(1000);

    port = get_num_ports();
    if (port == 8) {
	fortitude_image_fw = (unsigned char *)fortitude_image_lx25_fw;
	fortitude_image_fw_size = fortitude_image_lx25_fw_size;
    } else if (port == 4) {
	fortitude_image_fw = (unsigned char *)fortitude_image_lx9_fw;
	fortitude_image_fw_size = fortitude_image_lx9_fw_size;
    } else {
	/* for 1 or 2 ports SKUs */
	fortitude_image_fw = (unsigned char *)fortitude_image_lx4_fw;
	fortitude_image_fw_size = fortitude_image_lx4_fw_size;
    }

    printf("\ngpdr_addr @%#x = %#x, gpvr_addr @%#x = %#x, pad_ptr @%#x = %#x\n", 
	   gpdr_addr, *gpdr_ptr, gpvr_addr, *gpvr_ptr, (ulong)pad_ptr, *pad_ptr);

    printf("fortitude_image_fw @ %#x = %#x, fortitude_image_fw_size = %d\n", 
	   (ulong )fortitude_image_fw, *fortitude_image_fw, fortitude_image_fw_size);

    /* clear config memory */
    *gpvr_ptr &= ~FPGA_PROG_L;
    /* Don't keep PROG_L low more then 500ns */
    usleep(300);
    
    *gpvr_ptr |= FPGA_PROG_L;

    /* check memory is clear. wait time is 100ms. */
    for (ix = 0; ix < 10000; ix++) {
        if (*gpvr_ptr & FPGA_INIT) {
            break;
        }
        usleep(10);
    }

    if (ix == 10000) {
        printf("Timeout! Internal memory is not clear\n");
	return (FAILED);
    }
    
    /* According to xapp098.pdf, need to delay 55us after INIT goes high */
    usleep(80);
   
    /* Configuration */
    for (ctr = 0; ctr < fortitude_image_fw_size; ctr++) {
	for (cunt = 0; cunt < 8; cunt++) {
            /* DIN connects to GPIO pin 8 within NPU.
             * Data will be shifted out MSB first on gpio pin 8.
	     */
	    *gpvr_ptr &= 0xfffffeff;
            *gpvr_ptr |= (ushort)((fortitude_image_fw[ctr] << cunt) & 0x80) << 1;
            generate_nclock(gpvr_ptr, 1);
        }
    }       

    for (ix = 0; ix < 10000; ix++) {
	/* keep sending data in order to get DONE bit. */
	*gpvr_ptr |= (ushort)0x01 << 8;
	generate_nclock(gpvr_ptr, 1);

        if (*gpvr_ptr & FPGA_DONE) {
	    /* generate 8 more clock after getting DONE bit */
	    for (ix = 0; ix < 8; ix++) {
		*gpvr_ptr |= 0x01 << 8;
		generate_nclock(gpvr_ptr, 1);
	    }
            break;
        }
    }

    if (ix == 10000) {
	printf("\ngpdr_addr @%#x = %#x, gpvr_addr @%#x = %#x\n", 
	       gpdr_addr, *gpdr_ptr, gpvr_addr, *gpvr_ptr);
        printf("ERROR! FPGA download failed. Can not get DONE bit.\n");
        return (FAILED);
    } else {
        printf("FPGA download is successful.\n");
        return (PASSED);
    }
}


/*****************************************************************
 *
 * Function: fortitude_fpga_upgrade_secondary()
 *
 * Description: This function upgrades secondary Fortitude PFGA image in
  *             the SPI flash.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int
fortitude_fpga_upgrade_secondary () 
{
    uint port, i, write_size;
    unsigned char *fortitude_image_fw, *fpga_data;
    int fortitude_image_fw_size;
    ulong mb_ctrl_addr, mb_ctrl_data, fpga_addr;
    unsigned char wr_status;

    printf("\nStart secondary FPGA upgrade!\n");

    port = get_num_ports();
    if (port == 8) {
	fortitude_image_fw = (unsigned char *)fortitude_8p_fpga_ugd_fw;
	fortitude_image_fw_size = fortitude_8p_fpga_ugd_fw_size;
    } else if (port == 4) {
	fortitude_image_fw = (unsigned char *)fortitude_4p_fpga_ugd_fw;
	fortitude_image_fw_size = fortitude_4p_fpga_ugd_fw_size;
    } else {
	/* for 1 or 2 ports SKUs */
	fortitude_image_fw = (unsigned char *)fortitude_2p_fpga_ugd_fw;
	fortitude_image_fw_size = fortitude_2p_fpga_ugd_fw_size;
    }

    mb_ctrl_addr = FPGA_MB_REG_BASE + MB_CTRL_OFFSET;
    if (tdm_rd(mb_ctrl_addr, 4, &mb_ctrl_data, 0) == FAILED) {
	return (FAILED);
    }

    /* 1. before writing the upgrade image to spi flash, reset bit 0 
       of mb_ctrl_reg to invalidate the header register. */
    mb_ctrl_data &= 0xffffffe;
    if (tdm_wr(mb_ctrl_addr, 4, mb_ctrl_data, 0) == FAILED) {
	return (FAILED);
    }

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	printf("Failed to enable SPI flash write.\n");
	return (FAILED);
    }

    /* 2. Before programming, set status register BP2:BP0=000 to make 
       all secotrs unprotected. */
    wr_status = 0x00;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
	    printf("Failed to write to SPI flash status register.\n");
	    return (FAILED);
    }

    /* 3. write the upgrade image to spi flash */
    fpga_addr = SECONDARY_FPGA_IMAGE_START_ADDR;
    fpga_data = fortitude_image_fw;
    for (i = fortitude_image_fw_size; i > 0; ) {
#ifdef DEBUG
	printf("i = %#x, fpga_addr = %#x\n", i, fpga_addr);
#endif
	if (i >= FPGA_SPI_PAGE_SIZE)
	    write_size = FPGA_SPI_PAGE_SIZE;
	else
	    write_size = i;

	if (!(fpga_addr & 0xffff)) {
#ifdef DEBUG
	    printf("sector erase. fpga_addr = %#x\n", fpga_addr);
#endif
	    /* enable SPI flash write */
	    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
		printf("Failed to enable SPI flash write.\n");
		return (FAILED);
	    }

	    /* erase sector (64KB) */
	    if (spi_flash_write(SECTOR_ERASE, fpga_addr & 0x1f0000, 0, NULL) 
		== FAILED) {
		printf("Failed to erase sector.\n");
		return (FAILED);
	    }
	}

	/* enable SPI flash write */
	if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	    printf("Failed to enable SPI flash write.\n");
	    return (FAILED);
	}

	if (spi_flash_write(PAGE_PROGRAM, fpga_addr, write_size,
			    fpga_data) == FAILED) {
	    printf("Failed to write to SPI flash.\n");
	    return (FAILED);
	}
	i -= FPGA_SPI_PAGE_SIZE;
	fpga_addr += write_size;
	fpga_data += write_size;
    }

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	printf("Failed to enable SPI flash write.\n");
	return (FAILED);
    }

    /* 4. After programming, set status register BP2:BP0=111 to protect 
       all sectors. */
    wr_status = 0x1c;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
	    printf("Failed to write to SPI flash status register.\n");
	    return (FAILED);
    }

    printf("\nFinish secondary FPGA upgrade!\n");

#ifdef DEBUG
    /* 5. set bit 1 of mb_ctrl_reg to reset the Reconfiguration FSM. 
       We can choose to reset automatically or let users to do the reset
       after the FPGA upgrade. */
    mb_ctrl_data |= 0x3;
    if (tdm_wr(mb_ctrl_addr, 4, mb_ctrl_data, 0) == FAILED) {
	return (FAILED);
    }
#endif

    return (PASSED);
}

/*****************************************************************
 *
 * Function: fortitude_fpga_upgrade_golden()
 *
 * Description: This function upgrades golden Fortitude PFGA image in
  *             the SPI flash.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int
fortitude_fpga_upgrade_golden () 
{
    uint port, i, write_size;
    unsigned char *fortitude_image_fw, *fpga_data;
    int fortitude_image_fw_size;
    ulong mb_ctrl_addr, mb_ctrl_data, fpga_addr;
    unsigned char wr_status;

    printf("\nStart golden FPGA upgrade!\n");

    port = get_num_ports();
    if (port == 8) {
	fortitude_image_fw = (unsigned char *)fortitude_8p_fpga_gld_fw;
	fortitude_image_fw_size = fortitude_8p_fpga_gld_fw_size;
    } else if (port == 4) {
	fortitude_image_fw = (unsigned char *)fortitude_4p_fpga_gld_fw;
	fortitude_image_fw_size = fortitude_4p_fpga_gld_fw_size;
    } else {
	/* for 1 or 2 ports SKUs */
	fortitude_image_fw = (unsigned char *)fortitude_2p_fpga_gld_fw;
	fortitude_image_fw_size = fortitude_2p_fpga_gld_fw_size;
    }

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	printf("Failed to enable SPI flash write.\n");
	return (FAILED);
    }

    /* 1. Before programming, set status register BP2:BP0=101 to make 
       lower half(16 secotrs) unprotected. */
    wr_status = 0x14;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
	    printf("Failed to write to SPI flash status register.\n");
	    return (FAILED);
    }

    /* 2. write the golden image to spi flash */
    fpga_addr = GOLDEN_FPGA_IMAGE_START_ADDR;
    fpga_data = fortitude_image_fw;
    for (i = fortitude_image_fw_size; i > 0; ) {
#ifdef DEBUG
	printf("i = %#x, fpga_addr = %#x\n", i, fpga_addr);
#endif
	if (i >= FPGA_SPI_PAGE_SIZE)
	    write_size = FPGA_SPI_PAGE_SIZE;
	else
	    write_size = i;

	if (!(fpga_addr & 0xffff)) {
#ifdef DEBUG
	    printf("sector erase. fpga_addr = %#x\n", fpga_addr);
#endif
	    /* enable SPI flash write */
	    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
		printf("Failed to enable SPI flash write.\n");
		return (FAILED);
	    }

	    /* erase sector (64KB) */
	    if (spi_flash_write(SECTOR_ERASE, fpga_addr & 0x1f0000, 0, NULL) 
		== FAILED) {
		printf("Failed to erase sector.\n");
		return (FAILED);
	    }
	}

	/* enable SPI flash write */
	if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	    printf("Failed to enable SPI flash write.\n");
	    return (FAILED);
	}

	if (spi_flash_write(PAGE_PROGRAM, fpga_addr, write_size,
			    fpga_data) == FAILED) {
	    printf("Failed to write to SPI flash.\n");
	    return (FAILED);
	}
	i -= FPGA_SPI_PAGE_SIZE;
	fpga_addr += write_size;
	fpga_data += write_size;
    }

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	printf("Failed to enable SPI flash write.\n");
	return (FAILED);
    }

    /* 3. After programming, set status register BP2:BP0=111 to protect 
       all sectors. */
    wr_status = 0x1c;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
	    printf("Failed to write to SPI flash status register.\n");
	    return (FAILED);
    }

    printf("\nFinish golden FPGA upgrade!\n");

    return (PASSED);
}


int 
spi_flash_read (int opcode, ulong addr, int size, uint8_t *rd_data)
{
    fpga_spi_reg_t *fpga_reg;
    uchar spi_data;
    int i;

    fpga_reg = (fpga_spi_reg_t *)(get_fpga_base() + FPGA_SPI_REG_BASE);

    /* clear fpga_spi_stat register and wait for read FIFO empty. */
    if ((fpga_reg->fpga_spi_stat[0] & FPGA_SPI_WR_FIFO_ORUN) 
	== FPGA_SPI_WR_FIFO_ORUN) {
	fpga_reg->fpga_spi_stat[0] = FPGA_SPI_WR_FIFO_ORUN;
    }
    if ((fpga_reg->fpga_spi_stat[1] & FPGA_SPI_DONE) 
	== FPGA_SPI_DONE) {
	fpga_reg->fpga_spi_stat[1] = FPGA_SPI_DONE;
    }

    for (i = 0; i < 1000; i++) {
	if (fpga_reg->fpga_spi_stat[0] & FPGA_SPI_RD_FIFO_EMPTY)
	    break;
	msleep(1);
    }

    if (i == 1000) {
	printf("Time out waiting for read FIFO empty.\n");
	return (FAILED);
    }

    /* set read data size */
    fpga_reg->fpga_spi_rd_size[0] = size - 1;

    /* set read address and opcode */
    fpga_reg->fpga_spi_addr[3] = opcode;
    fpga_reg->fpga_spi_addr[0] = addr & 0xff;
    fpga_reg->fpga_spi_addr[1] = (addr >> 8) & 0xff;
    fpga_reg->fpga_spi_addr[2] = (addr >> 16) & 0xff;

    /* set bit 0 and bit 2 of fpga_spi_ctrl register per
       requirement of the opcode used. */
    spi_data = fpga_reg->fpga_spi_ctrl[0];

    if (opcode == RD_DATA_BYTES)
	spi_data |= FPGA_SPI_ADDR_EN;
    else
	spi_data &= (~FPGA_SPI_ADDR_EN);

    /* write 0 to bit 1 for read */
    spi_data &= 0xf9;
    fpga_reg->fpga_spi_ctrl[0] = spi_data;

    /* wait for spi flash read to complete by polling DONE bit
       in fpga_spi_stat register */
    for (i = 0; i < 600; i++) {
	if ((fpga_reg->fpga_spi_stat[1] & FPGA_SPI_DONE) 
	    == FPGA_SPI_DONE) {
	    break;
	}
	msleep(1);
    }

    if (i == 600) {
	printf("Time out waiting for read to complete.\n");
	return (FAILED);
    }

    /* read data */
    for (i = 0; i < size; i++) {
	*rd_data++ = fpga_reg->fpga_spi_data[0];
    }

    fpga_reg->fpga_spi_stat[1] = FPGA_SPI_DONE;

    return (PASSED);
}

int 
spi_flash_write (int opcode, ulong addr, int size, uint8_t *wr_data)
{
    fpga_spi_reg_t *fpga_reg;
    uchar status_reg;
    int i;

    fpga_reg = (fpga_spi_reg_t *)(get_fpga_base() + FPGA_SPI_REG_BASE);

    /* clear fpga_spi_stat register. */
    if ((fpga_reg->fpga_spi_stat[0] & FPGA_SPI_WR_FIFO_ORUN) 
	== FPGA_SPI_WR_FIFO_ORUN) {
	fpga_reg->fpga_spi_stat[0] = FPGA_SPI_WR_FIFO_ORUN;
    }
    if ((fpga_reg->fpga_spi_stat[1] & FPGA_SPI_DONE) 
	== FPGA_SPI_DONE) {
	fpga_reg->fpga_spi_stat[1] = FPGA_SPI_DONE;
    }

    /* poll "Write In Progress" bit by reading SPI flash status 
       register and wait bit 0 to be 0. */
    for (i = 0; i < 3000; i++) {
	if (spi_flash_read(RD_STATUS, 0, 1, &status_reg) == FAILED) {
	    printf("Failed to read SPI flash status register.\n");
	    return (FAILED);
	}
	if (!(status_reg & RDSR_WIP))
	    break;
	msleep(1);
    }

    if (i == 3000) {
	printf("Time out waiting for SPI flash ready.\n");
	return (FAILED);
    }

    /* check for write enable bit in SPI flash status register */
    if (opcode != WRITE_ENABLE) {
	if (!(status_reg & RDSR_WEL)) {
	    printf("SPI flash write is not enabled.\n");
	    return (FAILED);
	}
    }

    /* set write address and opcode */
    fpga_reg->fpga_spi_addr[3] = opcode;
	    
    if ((opcode == SECTOR_ERASE) || (opcode == PAGE_PROGRAM)) {
	fpga_reg->fpga_spi_addr[0] = addr & 0xff;
	fpga_reg->fpga_spi_addr[1] = (addr >> 8) & 0xff;
	fpga_reg->fpga_spi_addr[2] = (addr >> 16) & 0xff;
    }

    if ((opcode == PAGE_PROGRAM) || (opcode == WR_STATUS)) {
	/* write the data to the write FIFO */
	for (i = 0; i < size; i++) {
	    fpga_reg->fpga_spi_data[0] = *wr_data++;
	}
    }

    /* set bit 1 of fpga_spi_ctrl register to 1 for write operation 
       and set bit 0 and bit 2 per requirement of the opcode used. */
    /* FPGA code will block multiple writes to fpga_spi_ctrl
     * if the SPI master is busy to prevent any data corruption.
     */
    uchar spi_data;
    spi_data = fpga_reg->fpga_spi_ctrl[0];

    if ((opcode == SECTOR_ERASE) || (opcode == PAGE_PROGRAM)) {
	    spi_data |= FPGA_SPI_ADDR_EN;
    } else {
	    spi_data &= (~FPGA_SPI_ADDR_EN);
    }	

    fpga_reg->fpga_spi_ctrl[0] |= (spi_data | FPGA_SPI_WRITE);

    /* wait for spi flash write to complete by polling DONE bit
       in fpga_spi_stat register */
    for (i = 0; i < 6000; i++) {
	if ((fpga_reg->fpga_spi_stat[1] & FPGA_SPI_DONE) 
	    == FPGA_SPI_DONE) {
	    fpga_reg->fpga_spi_stat[1] = FPGA_SPI_DONE;
	    break;
	}
	msleep(1);
    }

    if (i == 6000) {
	printf("Time out waiting for write to complete.\n");
	return (FAILED);
    }
    
    /* check bit 2 of fpga_spi_stat register to make sure 
       write FIFO is empty. If not, error has occured. */
    if (!(fpga_reg->fpga_spi_stat[0] & FPGA_SPI_WR_FIFO_EMPTY)) {
	printf("Error: SPI flash write FIFO is not empty.\n");
	return (FAILED);
    }

    /* clear fpga_spi_stat register. */
    if ((fpga_reg->fpga_spi_stat[0] & FPGA_SPI_WR_FIFO_ORUN) 
	== FPGA_SPI_WR_FIFO_ORUN) {
	fpga_reg->fpga_spi_stat[0] = FPGA_SPI_WR_FIFO_ORUN;
    }

    return (PASSED);  
}


int 
peek_spi_flash ()
{
    int opcode, size, i;
    ulong addr;
    uint8_t data[4];

    opcode = gethex_answer("\nEnter opcode to access SPI flash[03, 05, 9F]:",
			   0x03, 0x03, 0x9f);
    if ((opcode != RD_IDENTIFICATION) && (opcode != RD_STATUS)
	&& (opcode != RD_DATA_BYTES)) {
	printf("\nWrong opcode to access SPI flash!\n");
	return (FAILED);
    }

    if (opcode == RD_IDENTIFICATION) {
	size = 4;
    } else {
	size = 1;
    }

    if ((opcode == RD_DATA_BYTES) || (opcode == RD_DATA_BYTES_HIGH_SPEED)) {
	addr = gethex_answer("\nEnter address to access SPI flash"
			     "[0x100000 - 0x200000]:", 
			     0x100000, 0x100000, 0x200000);
    } else {
	addr = 0;
    }

    if (spi_flash_read(opcode, addr, size, data) == FAILED) {
	printf("\nFailed to read from SPI flash.\n");
	return (FAILED);
    }

    for (i = 0; i < size; i++) {
	printf("\nData = %#x", data[i]);
    }

    return (PASSED);
}


#ifdef DEBUG   
int 
poke_spi_flash ()
{
    ulong addr;
    uint8_t data;
    unsigned char wr_status;
    
    addr = gethex_answer("\nEnter address to access SPI flash"
			 "[0x100000 - 0x1fffff]:", 
			 0x100000, 0x100000, 0x1fffff);
    
    data = gethex_answer("\nEnter data to write to SPI flash[0 - 0xff]:",
			 0, 0, 0xff);

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	printf("Failed to enable SPI flash write.\n");
	return (FAILED);
    }

    /* Before poking SPI flash, set status register BP2:BP0=000 to make 
       all secotrs unprotected. */
    wr_status = 0x00;
    if (spi_flash_write(WR_STATUS, 0, 1, &wr_status) == FAILED) {
	    printf("Failed to write to SPI flash status register.\n");
	    return (FAILED);
    }

    /* erase sector (64KB) */
    if (spi_flash_write(SECTOR_ERASE, addr & 0x1f0000, 0, NULL) == FAILED) {
	printf("Failed to erase sector.\n");
	return (FAILED);
    }

     /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
	printf("Failed to enable SPI flash write.\n");
	return (FAILED);
    }   
    
    if (spi_flash_write(PAGE_PROGRAM, addr, 1, &data) == FAILED) {
	printf("Failed to write to SPI flash.\n");
	return (FAILED);
    }

    return (PASSED);
}
#endif

/*******************************************************************
 *
 * Function: show_mb_regs
 *
 * Description: This function will display the contents of the
 *              Fortitude FPGA Multiboot registers
 *
 * Input  : None
 *
 * Output : Always return PASSED to avoid compilation warning
 *
 *******************************************************************
 */
int
show_mb_regs ()
{
    fpga_mb_reg_t *tdmsw_reg;
    ulong reg_data;

    tdmsw_reg = (fpga_mb_reg_t *)(get_fpga_base() + FPGA_MB_REG_BASE);

    if (tdm_rd(FPGA_MB_REG_BASE+MB_CTRL_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot Control Register  @%#.8x = %#.8x",
	       &tdmsw_reg->mb_ctrl, reg_data);
    }
    if (tdm_rd(FPGA_MB_REG_BASE+MB_STAT_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot Status Register  @%#.8x = %#.8x",
	       &tdmsw_reg->mb_stat, reg_data);
    }
    if (tdm_rd(FPGA_MB_REG_BASE+MB_HDR_ID_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot Header ID Register @%#.8x = %#.8x",
	       &tdmsw_reg->mb_hdr_id, reg_data);
    }
    if (tdm_rd(FPGA_MB_REG_BASE+MB_HDR_DATE_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot Header Date Register @%#.8x = %#.8x",
	       &tdmsw_reg->mb_hdr_date, reg_data);
    }
    if (tdm_rd(FPGA_MB_REG_BASE+MB_HDR_FLAG_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot Header Flag Register @%#.8x = %#.8x",
	       &tdmsw_reg->mb_hdr_flag, reg_data);
    }
    if (tdm_rd(FPGA_MB_REG_BASE+MB_HDR_MAGIC_NUM_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot Header Magic Num Register @%#.8x = %#.8x",
	       &tdmsw_reg->mb_hdr_magic_num, reg_data);
    }
    if (tdm_rd(FPGA_MB_REG_BASE+MB_HISTORY_OFFSET, 4, 
	       &reg_data, 0) == PASSED) {
	printf("\nMultiboot State History Register @%#.8x = %#.8x",
	       &tdmsw_reg->mb_history, reg_data);
    }

    return(PASSED);
}

/*****************************************************************
 *
 * Function: mb_peek_reg()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA Multiboot registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
mb_peek_reg ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;

    base_addr = FPGA_MB_REG_BASE;

    offset = gethex_answer("\nEnter Multiboot register offset[0x00 to 0x18]:",
			   0, 0, 0x18);

    /* all the Multiboot registers are 4 bytes aligned */
    offset &= 0xfc;

    if (tdm_rd((base_addr + offset), 4, &reg_data, 0) == PASSED) {
	printf("\n register value @ offset %#x = %#.8x ", 
	       (base_addr+offset), reg_data);
	return PASSED;
    } else {
	return FAILED;
    }
}

/*****************************************************************
 *
 * Function: mb_poke_reg()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              FPGA Multiboot registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
mb_poke_reg ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;

    base_addr = FPGA_MB_REG_BASE;

    offset = gethex_answer("\nEnter Multiboot register offset[0x00 to 0x18]:",
			   0, 0, 0x18);

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
			     0, 0, 0xffffffff);
   
    /* all the Multiboot registers are 4 bytes aligned */
    offset &= 0xfc;

    if (tdm_wr((base_addr + offset), 4, reg_data, 0)== PASSED) {
	if (tdm_rd((base_addr + offset), 4, &reg_data, 0) == PASSED) {
	    printf("\n register value @ offset %#x = %#.8x ", 
		   (base_addr+offset), reg_data);
	    return PASSED;
	} 
    }
    return FAILED;
}

/*****************************************************************
 *
 * Function: spi_peek_reg()
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA SPI flash registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
spi_peek_reg ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    uchar *reg_p;

    base_addr = (get_fpga_base() + FPGA_SPI_REG_BASE);

    offset = gethex_answer("\nEnter SPI flash register offset[0x00 to 0x13]:",
			   0, 0, 0x13);

    reg_p = (uchar *)(base_addr + offset);
    reg_data = *reg_p;
    
    printf("\n register value @%#x = %#x ", (base_addr+offset), reg_data);
    return PASSED;
}

/*****************************************************************
 *
 * Function: spi_poke_reg()
 *
 * Description: This function performs a write to NPU memory-mapped
 *              FPGA SPI flash registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int 
spi_poke_reg ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    uchar *reg_p;

    base_addr = (get_fpga_base() + FPGA_SPI_REG_BASE);

    offset = gethex_answer("\nEnter SPI flash register offset[0x00 to 0x13]:",
			   0, 0, 0x13);

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFF]:", 
			     0, 0, 0xff);
   
    reg_p = (uchar *)(base_addr + offset);
    *reg_p = reg_data;
    printf("\n register value @%#x = %#x ", (base_addr+offset), *reg_p);

    return (PASSED);
}


/*****************************************************************
 *
 * Function: is_board_t1_mode()
 *
 * Description: This function checks the board is in T1 or E1 mode
 *              by reading the FPGA snoop register.  
 *
 * Input: None
 *
 * Output: TRUE  - T1 mode
 *         FALSE - E1 mode
 *
 *****************************************************************/
int
is_board_t1_mode () 
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);
#ifdef DEBUG
    printf("\npmc_mode_status1[0] = %#x, pmc_mode_status1[1] = %#x, pmc_mode_status1[2] = %#x", fpga_reg->pmc_mode_status1[0], fpga_reg->pmc_mode_status1[1], fpga_reg->pmc_mode_status1[2]);
    printf("\npmc_mode_status2[0] = %#x, pmc_mode_status2[1] = %#x, pmc_mode_status2[2] = %#x, pmc_mode_status2[3] = %#x\n", fpga_reg->pmc_mode_status2[0], fpga_reg->pmc_mode_status2[1], fpga_reg->pmc_mode_status2[2], fpga_reg->pmc_mode_status2[3]);
#endif
    if (fpga_reg->pmc_mode_status1[2] & SNOOP_E1) {
	return (FALSE);
    } else {
	return (TRUE);
    }
}

/*****************************************************************
 *
 * Function: is_port_clk_master_mode()
 *
 * Description: This function checks the port is in PMC clock master 
 *              or slave mode by reading the FPGA snoop register.  
 *
 * Input: port_num
 *
 * Output: TRUE  - clock master mode
 *         FALSE - clock slave mode
 *
 *****************************************************************/
int
is_port_clk_master_mode (int port_num) 
{
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    if (fpga_reg->pmc_mode_status1[0] & (PORT0_PMC_CLK_MASTER << port_num)) {
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/******** History ********
$Log: fortitude_fpga.c,v $
Revision 1.24  2019/06/03 09:07:40  meho
Writing data to fpga_spi_ctrl register once at a time.

Revision 1.23  2013/09/06 22:15:53  ywen
Add peek/poke utility for new added FPGA registers.

Revision 1.22  2013/02/15 17:50:43  ywen
Fix a LED test issue.

Revision 1.21  2013/02/13 23:55:39  ywen
Add code to display FPGA version in the utility.

Revision 1.20  2013/01/30 21:12:13  ywen
Add peek utility for secure boot registers.

Revision 1.19  2012/12/18 17:39:27  ywen
mask off the utility to poke the SPI flash for safety purpose.

Revision 1.18  2012/10/31 21:28:01  ywen
Add utility to upgrade golden FPGA image.

Revision 1.17  2012/10/04 21:38:29  ywen
Add code to support different HW revision.

Revision 1.16  2012/10/02 22:42:16  ywen
- Add support for host UART test.
- Add utility to set NOR flash address 23 for secure boot.

Revision 1.15  2012/09/25 22:36:43  ywen
- Add SPI flash peek/poke utility
- Update SPI registers access based on the latest FPGA design.

Revision 1.14  2012/09/10 06:02:28  srane
return failure for config_ngvm_tdm_lpbk().

Revision 1.13  2012/08/29 20:07:25  ywen
- Add utility to upgrade FPGA image for P1C and later builds.
- Add peek/poke utilities for FPGA multiboot registers.

Revision 1.12  2012/08/15 15:55:05  srane
Add NGVM TDM streams for TDMSW6.

Revision 1.11  2012/07/26 20:40:49  ywen
Fixed loopback test issue in slave mode for 1 port SKU.

Revision 1.10  2012/07/23 06:54:53  srane
- Display more locations in connection memory.
- Routine to set connection memory to do lpbk for specified TDM stream.

Revision 1.9  2012/06/25 21:24:55  ywen
Support LED test for 2 port SKU.

Revision 1.8  2012/06/13 17:54:34  ywen
Add support for TDMSW16 and 2 port SKU.

Revision 1.7  2012/06/07 23:30:16  ywen
Add support for 1/2/4/8 port SKUs.

Revision 1.6  2012/05/15 23:30:56  ywen
Add code to get PID from kernel file and parse the port number from that.

Revision 1.5  2012/04/20 21:20:24  ywen
Improve FPGA force byte test.

Revision 1.4  2012/04/02 21:06:29  ywen
Make FPGA TDMSW loopback test work.

Revision 1.3  2012/04/02 17:52:04  ywen
Add workaround in FPGA download for the issue cause by Fortitude TFTP download.

Revision 1.2  2012/03/28 00:38:16  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: platform_mcu.c,v 1.11 2017/07/10 02:27:50 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/platform_mcu.c,v $
 *------------------------------------------------------------------
 * Filename   : platform_mcu.c
 *
 * Description: Overlord Environment MCU Utilities.
 *
 * Ported from Informers, and original author is Ling Lee.
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "endians.h"
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "strings.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "byteswap.h"
#include "nvmonvars.h"
#include "n2g_api_rc.h"
#include "dash_fpga.h" /*this includes platform_mcu.h*/
#include "i2c_address.h"
#include "queryflags.h"
#include "platform_intr_test.h"
#include "platform_env.h"
#include "platform_vtg_mntr.h"

/*
 * Global defines
 */
//#define FLASH_RD_WR_DELAY 50
#define FLASH_RD_WR_DELAY 50
#define MCU_DEBUG
#define MCU_BAUDRATE 9600

/* Function prototypes */
#ifdef MCU_DEBUG
static int mcu_display_reg(void);
static int mcu_show_ver(void);
static int display_status_reg(void);
static int mcu_show_fw_ver(void);
#endif

static int mcu_select(int);
static void env_mcu_intr_hndlr(int irq, void *arg);
static int erase_flash_status_check(void);
static int read_status_reg(uchar *, int);
static int init_env_mcu(void);
static int program_mcu_flash(void);
static int read_check_mcu_flash(void);
static int env_mcu_id_check(void);
static int reset_start_mcu_flash(void);
static int select_firmware_type(void);
static void enable_intr(void);
static void disable_intr(void);
#define DEFAULT_IRQ INTR_ENV_MCU
/* Global variables */
static mb_iofpga_mcu_regs_t *IOFPGA_MCU_REGS;  /* used to download firmware */
static int irq = DEFAULT_IRQ;
static int (*get_version)(uint16_t *);

static int init_mcu_done;
static int show = 0;
static uint32_t rx_size = 0;
extern const unsigned char eprom[];
extern const unsigned long eprom_start;
extern const unsigned long eprom_length;
extern const unsigned char overlord_env_mcu_revision[];

extern const unsigned char vm_eprom[];
extern const unsigned long vm_eprom_start;
extern const unsigned long vm_eprom_length;
extern const unsigned char overlord_vmon_mcu_revision[];
/* r5f 21262 */

static uchar env_mcu_rx_buf[ENV_MCU_MAX_DATA_SIZE];
static uchar *env_mcu_rx_buf_p;
static volatile uint32_t intr_cnt ;
static uchar *program_data_p = 0;
static int program_data_length = 0;
static uchar *program_data_revision;
static uint program_eprom_start_addr;

static uchar *cmp_ptr;
static boolean isr_data_chk = FALSE;
static boolean isr_err_found = FALSE;


/*
 * Environmental MCU Download Control Unit Menu
 */
static submenu_xtable_t env_menu_table[] = {
    {"Select Env MCU (default Voltage MCU)", (PFT)mcu_select, 1,
	0, (type_t(*)())0, 0, (PFT)mcu_select, 1},
#ifdef MCU_DEBUG
    {"Display FPGA MCU registers", (PFT)mcu_display_reg, 0,
	0, (type_t(*)())0, 0, (PFT)mcu_display_reg, 0},
    {"Init MCU", (PFT)init_env_mcu, 0,
	0, (type_t(*)())0, 0, (PFT)init_env_mcu, 0},
    {"Show MCU Boot Loader Version Number", (PFT)mcu_show_ver, 0,
	0, (type_t(*)())0, 0, (PFT)mcu_show_ver, 0},
    {"Display status reg", (PFT)display_status_reg, 0,
	0, (type_t(*)())0, 0, (PFT)display_status_reg, 0},

    {"ID check", (PFT)env_mcu_id_check, 0,
	0, (type_t(*)())0, 0, (PFT)env_mcu_id_check, 0},
    {"Show MCU Firmware Version Number", (PFT)mcu_show_fw_ver, 0,
	0, (type_t(*)())0, 0, (PFT)mcu_show_fw_ver, 0},
#endif
    {"Program MCU flash", (PFT)program_mcu_flash, 0,
	0, (type_t(*)())0, 0, (PFT)program_mcu_flash, 0},
    {"Read and check MCU flash", (PFT)read_check_mcu_flash, 0,
	0, (type_t(*)())0, 0, (PFT)read_check_mcu_flash, 0},
    {"Reset and Start MCU flash program", (PFT)reset_start_mcu_flash, 0,
	0, (type_t(*)())0, 0, (PFT)reset_start_mcu_flash, 0},
};

#define ENV_MENU_TABLE_SIZE (sizeof(env_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t env_menu_primary_items[ENV_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t env_menu_secondary_items[ENV_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo envdiag = {
    "MCU Download Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    env_menu_primary_items,
};

static struct menuinfo *envdiagp = &envdiag;

/*******************************************************************************
 *
 * function   : build_env_mcu_menu
 * Description:	Build Environmental Control Unit menu.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_env_mcu_menu (void)
{
    mcu_select(0);
    
    build_primary_submenu(env_menu_table, ENV_MENU_TABLE_SIZE,
			  "Environmental Control Unit Utility Menu", &envdiagp);
    build_secondary_submenu(env_menu_table, ENV_MENU_TABLE_SIZE,
			    env_menu_secondary_items);

    menu(&envdiag, env_menu_secondary_items, 0);
    
    if (irq) {
        free_irq(irq, NULL);
        irq = DEFAULT_IRQ;
        printf("irq %d %d\n", irq, __LINE__);
    }
}

static
int mcu_select (int query)
{
    int ans = 1;
    
    if (query) {
        printf("select '0' for env mcu (default).\n");
        printf("select '1' for voltage mcu.\n");
        ans = getdec_answer("Enter > ", 0, 0, 1);
    }

    if (irq) {
        free_irq(irq, NULL);
        irq = DEFAULT_IRQ;
    }
 
    env_mcu_rx_buf_p = env_mcu_rx_buf;
    
    if (ans == 0) {
        printf("Env MCU selected \n");
        IOFPGA_MCU_REGS = (mb_iofpga_mcu_regs_t *)get_platform_env_mcu_base(CP);
        irq = INTR_ENV_MCU;
        get_version = env_get_version;
    }
    if (ans == 1) {
        printf("Votage Monitor MUC selected\n");
        IOFPGA_MCU_REGS = (mb_iofpga_mcu_regs_t *)get_platform_vm_base(CP);
        irq = INTR_VM_MCU;
        get_version = vtg_get_version;

    }

    request_irq(irq, env_mcu_intr_hndlr, 0, (void *)env_mcu_rx_buf_p);     
    init_mcu_done = FALSE;
    intr_cnt = 0;

    return (PASSED);
}

static void
enable_intr (void)
{
    switch (irq) {
    case INTR_ENV_MCU:
        enable_platform_mcu_intr(0);
        break;
    case INTR_VM_MCU:
        enable_platform_vm_mcu_intr(0);
        break;
    default:
        assert(!"invalide irq type");
    }

}

static void
disable_intr (void)
{
    switch (irq) {
    case INTR_ENV_MCU:
        disable_platform_mcu_intr(0);
        break;
    case INTR_VM_MCU:
        disable_platform_vm_mcu_intr(0);
        break;
    default:
        assert(!"invalide irq type");
    }

}

/*******************************************************************************
 *
 * Function   : env_mcu_tx
 * Description:	MCU transmit data.
 * Inputs     : data - transmit data
 *              size - transmit data size
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int env_mcu_tx (volatile uchar *data, int size)
{
    intr_cnt = 0;
    memset(env_mcu_rx_buf, 0, sizeof(env_mcu_rx_buf));
    while (size) {
	IOFPGA_MCU_REGS->mcu_dnld_data = *data++;
	size--;
	
	/* wait for trasmit done bit */
        if (!pstat_le((uint *)&IOFPGA_MCU_REGS->mcu_dnld_status, BW_16BITS, EQUAL,
		   ENV_MCU_TX_DONE, ENV_MCU_TX_DONE, 700, NULL)) {
	    printf("MCU TX done bit is not set, status: 0x%x\n",
		   IOFPGA_MCU_REGS->mcu_dnld_status);
	    return (FAILED);
	}
	

	/* clear the transmit done bit */
	IOFPGA_MCU_REGS->mcu_dnld_status = ENV_MCU_TX_DONE;

	/* check if transmit done bit is cleared */
	if (IOFPGA_MCU_REGS->mcu_dnld_status & ENV_MCU_TX_DONE) {
	    printf("MCU transmit done bit is not cleared, status: 0x%x\n",
		   IOFPGA_MCU_REGS->mcu_dnld_status);
	    return (FAILED);
	}
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : env_mcu_read_rx_data
 * Description:	To read data from Environmental MCU.
 * Inputs     : expect_size - expect receive bytesntr
 * Outputs    : PASSED/FAILED.
 *
 *******************************************************************************
 */
static int env_mcu_read_rx_data (int expect_size, int time_period)
{
    int timeout = 0;
    volatile uint *ptr = (volatile uint *)&intr_cnt;
    /* read_data(expect_size, time_period); */

    while (timeout++ < time_period) {
	if ((*ptr >= expect_size) || (isr_err_found == TRUE)) {
	    break;
	}
	msleep(1);
    }

    if ((timeout >= time_period) || (isr_err_found == TRUE)) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s timeout. Got %d bytes. Expect %d bytes. %s error\n", 
	       __FUNCTION__, intr_cnt, expect_size,
	       isr_err_found ? "Has" : "No");
            printf("MCU dnld intr en = %#x. "
                   "MCU dnld stat = %#x\n",
                   IOFPGA_MCU_REGS->mcu_dnld_intr_enable,
                   IOFPGA_MCU_REGS->mcu_dnld_status);
        }
	return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function:	clear_status_reg
 *
 * Description:	Clear Environmental MCU Status Register
 *
 * Inputs:	None
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int
clear_status_reg(void)
{
    env_mcu_clear_status_reg_t clear_status_reg;
    uchar status_reg[2];

    clear_status_reg.cmd = ENV_MCU_CLEAR_STATUS_REG;

    if (env_mcu_tx(&clear_status_reg.cmd, 1)) {
	printf("%s failed\n", __FUNCTION__);
	return FAILED;
    }

    msleep(5);

    if (read_status_reg(status_reg, ENV_MCU_RX_DATA_TIMEOUT)) {
        //	printf("%s read status register failed\n", __FUNCTION__);
	return FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("%s status %#x %#x\n",
		__FUNCTION__, status_reg[0], status_reg[1]);
    }

    return PASSED;
}


/*******************************************************************************
 *
 * Function   : read_status_reg
 * Description:	To read Environmental MCU Status Register.
 * Inputs     : data - status read data (2 bytes)
 *              timeout - timeout value for receiving status
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int read_status_reg (uchar *data, int timeout)
{
    env_mcu_read_status_reg_t read_status_reg;
    uint8_t                   ctr = 0;

    read_status_reg.cmd = ENV_MCU_READ_STATUS_REG;
    env_mcu_rx_buf_p = env_mcu_rx_buf;

    /* transmit command */
    if (env_mcu_tx(&read_status_reg.cmd, 1)) {
	printf("%s transmit command failed\n", __FUNCTION__);fflush(stdout);
	return (FAILED);
    }

    /* Read status data */
    if (env_mcu_read_rx_data(2, timeout)) {
        //	printf("%s failed\n", __FUNCTION__); fflush(stdout);
	return (FAILED);
    }

    /* Save status data */
    for (ctr = 0; ctr < 2; ctr++) {
	*data++ = env_mcu_rx_buf[ctr];
        env_mcu_rx_buf[ctr] = 0;
    }

    return (PASSED);
}
    

/*******************************************************************************
 *
 * Function   : erase_flash_status_check
 * Description:	Erase all unlocked blocks status check
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int erase_flash_status_check (void)
{
    uchar    status_reg[2];
    uint32_t erase_timer = 0;
   
    while (erase_timer < ENV_MCU_DNLD_ERASE_TIMEOUT) {
        printf(".");fflush(stdout);

        /* read status register */
        if (read_status_reg(status_reg, 100) == PASSED) {
            /* Check if any Erase Error occurred */
            if (status_reg[0] & ENV_MCU_SRD_ERASE_ERROR) {
                printf("[%s:%d] Erase Error (status reg. = %#x %#x).\n",
                       __FUNCTION__, __LINE__, status_reg[0], status_reg[1]);
                return (FAILED);
            }

            if (status_reg[0] & ENV_MCU_SRD_SEQUENCE_READY) {
                prpass(testpass, "Erase flash..Done");
                return (PASSED);
            }
        }
    
        msleep(500);
        erase_timer+=500;
    }

            printf("[%s:%d] Erase flash Time out (status reg. = %#x %#x).\n",
                   __FUNCTION__, __LINE__, status_reg[0], status_reg[1]);
            return (FAILED);
}


/**********************************************************************
 *
 * Function:	erase_all_block
 *
 * Description:	Erase all unlocked blocks
 *
 * Inputs:	None
 *
 * Outputs:	PASSED/FAILED.
 *
 **********************************************************************
 */
static int erase_all_block(void)
{
    env_mcu_erase_all_blk_t erase_all_blk;

    /* Use clear status register command to initialize the status 
     * register before eraseing the flash memory.
     */
    if (clear_status_reg() == FAILED) {
        printf("[%s:%d] Failed to clear status register.\n",
               __FUNCTION__, __LINE__);
	return (FAILED);
    }

    erase_all_blk.cmd = ENV_MCU_ERASE_ALL_BLOCK;
    erase_all_blk.confirm_cmd = ENV_MCU_TX_CONFIRM;

    /* send erase all blocks command */
    if (env_mcu_tx((uchar *)&erase_all_blk, 2)) {
	printf("[%s:%d] Failed to send erase all blocks command.\n",
               __FUNCTION__, __LINE__);
	return (FAILED);
    }

    prpass(testpass, "Erase flash..");

    /* Check the erase status by using read status register command */
    if (erase_flash_status_check() != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : program_status_ok
 * Description:	Check program status
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int program_status_ok (void)
{
    uchar    status_reg[2];

    /* read status register */
    if (read_status_reg(status_reg, 10) == PASSED) {
        /* check if program error is occurred */
        if (status_reg[0] & ENV_MCU_SRD_PROGRAM_ERROR) {
            printf("%s error status_reg[0] = %#x %#x.\n", __FUNCTION__, status_reg[0], status_reg[1] );
            return (FALSE);
        }

        /* check if program is completed */
        if (status_reg[0] & ENV_MCU_SRD_SEQUENCE_READY) {
            return (TRUE);
        }
    } else {
        printf("program status read is incoreect\n");
    }

    return (FALSE);
}


/**********************************************************************
 *
 * Function:	env_mcu_id_check
 *
 * Description:	Check ID
 *
 * Inputs:	None
 *
 * Outputs:	TRUE/FALSE
 *
 **********************************************************************
 */
static int
env_mcu_id_check(void)
{
    env_mcu_id_check_t id_check;
    uchar status_reg[2];
    int i;

#ifdef DEBUG
    uchar *id_check_p;
#endif
    //    msleep(200); /* ZZZ */
    rx_size = 2;
    //    show = 0;
    if (!init_mcu_done && init_env_mcu()) {
	printf("Init MCU failed\n");
	return FAILED;
    }

    //    printf("MCU ID check: ");

    id_check.cmd = ENV_MCU_ID_CHECK;
    id_check.low_addr = 0xDF;
    id_check.mid_addr = 0xFF;
    id_check.high_addr = 0x00;
    id_check.id_size = 0x07;

    for (i = 0; i < 7; i++) {
	id_check.id[i] = 0;
    }

#ifdef DEBUG
    printf("id check cmd size: %d\n", sizeof(id_check));
    id_check_p = (uchar *)&id_check;
    printf("ID check command: ");
    for (i = 0; i < 12; i++) {
	printf("0x%x ", *id_check_p++);
    }
    printf("\n");
#endif

    //    printf("rx_size = %d.\n", rx_size);

    /* send ID check command */
    if (env_mcu_tx((uchar *)&id_check, sizeof(id_check))) {
	printf("\n%s failed\n", __FUNCTION__);
	return FALSE;
    }
    msleep(10);
    //    printf("environmeching reading status\n");fflush(stdout);
    if (read_status_reg(status_reg, ENV_MCU_RX_DATA_TIMEOUT)) {
        printf("\n%s read status register failed\n", __FUNCTION__);
	return FALSE;
    }
    //    printf("environmeching reading status\n");fflush(stdout);
    if ((status_reg[1] & ENV_MCU_SRD1_ID_CHECK_MATCH) !=
        ENV_MCU_SRD1_ID_CHECK_MATCH) {
	printf("\n%s error, status: 0x%x\n", __FUNCTION__, status_reg[1]);
        fflush(stdout);
	return FALSE;
    }
    //    printf("Passed\n");

    if ((NVRAM)->diagflag & D_VERBOSE) {
	printf("status %#x %#x. ID = ", status_reg[0], status_reg[1]);
	for (i = 0; i < 7; i++) {
	    printf("%#x ", id_check.id[i]);
	}
 	printf("\n");
    }

    prpass(testpass, "ID check OK");
    return TRUE;
}


/**********************************************************************
 *
 * Function   : init_env_mcu
 * Description:	To initilize Environmental MCU.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 **********************************************************************
 */
static int init_env_mcu (void)
{
    int i;

    env_mcu_rx_buf_p = env_mcu_rx_buf;

    /*
    if (init_mcu_done)
	return PASSED;
	*/

    //    printf("Initializing MCU... ");

    /* Make sure interrupts are disabled */
    IOFPGA_MCU_REGS->mcu_dnld_intr_enable = 0;
    //    disable_platform_mcu_intr(FPGA_ENV_INTR);
    disable_intr();

    /* Enable MCU reset */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= (ENV_MCU_CTRL_RESET_EN | 
				       ENV_MCU_CTRL_DISABLE_RX);
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= ENV_MCU_CTRL_RESET_EN;

    /* Put MCU in reset */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= ENV_MCU_CTRL_RESET;
    msleep(25);
    /* Force mode low */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= ENV_MCU_CTRL_FORCE_MODE_LOW;
    msleep(100);
    /* Deassert MCU reset */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl &= ~ENV_MCU_CTRL_RESET;
    msleep(50);
    /* Force mode high */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl &= ~ENV_MCU_CTRL_FORCE_MODE_LOW;
    msleep(300);

#ifndef SKIP_BRSET
    /* Make sure FPGA is using 9600 baud */
    if ((IOFPGA_MCU_REGS->mcu_dnld_ctrl & ENV_MCU_CTRL_BAUD_RATE_MASK) !=
	ENV_MCU_CTRL_9600_BAUD) {
	printf("FPGA baud rate is not default to 9600. Set it to 9600\n");
	/* set FPGA baud rate to 9600 */
	IOFPGA_MCU_REGS->mcu_dnld_ctrl &= ~ENV_MCU_CTRL_BAUD_RATE_MASK;
	IOFPGA_MCU_REGS->mcu_dnld_ctrl |= ENV_MCU_CTRL_9600_BAUD;
	/* verify if FPGA is set to baud rate 9600 */
	if ((IOFPGA_MCU_REGS->mcu_dnld_ctrl & ENV_MCU_CTRL_BAUD_RATE_MASK) !=
	    ENV_MCU_CTRL_9600_BAUD) {
	    printf("Cannot set FPGA baud rate to 9600\n");
	    return FAILED;
	}
    }
#endif /* SKIP_BRSET */

    /* Adjust baud rate; send 16 standard time command */
    for (i = 0; i < 16; i++) {
	IOFPGA_MCU_REGS->mcu_dnld_data = 0;
	msleep(25);
    }
    msleep(25);

    /* Clear no stop bit received error */
    if (IOFPGA_MCU_REGS->mcu_dnld_status & ENV_MCU_NO_STOP_BIT_RX) {
	IOFPGA_MCU_REGS->mcu_dnld_status = ENV_MCU_NO_STOP_BIT_RX;
    }

    /* Enable Receive data interrupt */
    IOFPGA_MCU_REGS->mcu_dnld_intr_enable |= ENV_MCU_RX_INTR_EN;

    enable_intr();

    init_mcu_done = TRUE;
    printf("FPGA baud rate is set to %d\n", MCU_BAUDRATE);

    /* Fixed me: seems need some delay between init and check ID, but how long ?? and why ??? */
    //msleep(500);

    return (PASSED);
}


#ifdef MCU_DEBUG
/*******************************************************************************
 *
 * Function   : mcu_dispay_reg
 * Description:	To display MCU registers.
 * Inputs     : None
 * Output     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int mcu_display_reg (void)
{
    printf("\nEnvironmental MCU registers: \n");
    printf("MCU Download Control Reg. @%p: 0x%.8x\n", 
	   &IOFPGA_MCU_REGS->mcu_dnld_ctrl, 
	   IOFPGA_MCU_REGS->mcu_dnld_ctrl);
    printf("MCU Download Status Reg.  @%p: 0x%.8x\n", 
	   &IOFPGA_MCU_REGS->mcu_dnld_status, 
	   IOFPGA_MCU_REGS->mcu_dnld_status);
    printf("MCU Interrupt Enable Reg. @%p: 0x%.8x\n", 
	   &IOFPGA_MCU_REGS->mcu_dnld_intr_enable, 
	   IOFPGA_MCU_REGS->mcu_dnld_intr_enable);
    printf("MCU Data Reg.             @%p: 0x%.8x\n", 
	   &IOFPGA_MCU_REGS->mcu_dnld_data,
	   IOFPGA_MCU_REGS->mcu_dnld_data);

    return (PASSED);
}


/*********************************************************************
 *
 * Function:	mcu_show_ver
 *
 * Description:	Show MCU version number
 *
 * Inputs:	None
 *
 * Output:	PASSED/FAILED
 *
 *********************************************************************
 */
static int
mcu_show_ver(void)
{
    env_mcu_ver_info_t ver;
    int i, size;

    if (init_env_mcu()) {
	printf("Init MCU failed\n");
	return FAILED;
    }
    
    ver.cmd = ENV_MCU_VER_INFO;
    size = sizeof(ver.buf);
    env_mcu_rx_buf_p = env_mcu_rx_buf;

    /* transmit command */
    if (env_mcu_tx(&ver.cmd, 1)) {
	printf("%s transmit command failed\n", __FUNCTION__);
	return FAILED;
    }

    if (env_mcu_read_rx_data(size, ENV_MCU_RX_DATA_TIMEOUT)) {
	printf("%s failed\n", __FUNCTION__);
	return FAILED;
    }

    printf("\nMCU boot loader version: ");
    for (i = 0; i < size; i++) {
	printf("%c", env_mcu_rx_buf[i]);
    }
    printf("\n");
    
    return PASSED;
}


/*******************************************************************************
 *
 * Function:	display_status_reg
 * Description:	To display Environmental MCU Status Register.
 * Inputs:	None
 * Outputs:	PASSED/FAILED
 *
 *******************************************************************************
 */
static int display_status_reg (void)
{
    uchar status_reg[2];

    if (read_status_reg(status_reg, ENV_MCU_RX_DATA_TIMEOUT)) {
        //	printf("%s read status register failed\n", __FUNCTION__);
	return (FAILED);
    }

    printf("Status register value: 0x%x 0x%x\n", status_reg[0], status_reg[1]);

    return (PASSED);
}


/**********************************************************************
 *
 * Function   : mcu_show_fw_ver
 *
 * Description: Show MCU firmware version
 *
 * Inputs     : None
 *
 * Outputs    : PASSED/FAILED.
 *
 **********************************************************************
 */
static int mcu_show_fw_ver (void)
{
    uint16_t version = 0;

    if (!select_firmware_type()) {
        return (PASSED);
    }

    (void)reset_start_mcu_flash();

    if (get_version(&version)==FAILED) {
        cterr('f', 0, "%s:%d Failed to read env firmware version.",
              __FUNCTION__, __LINE__);
        return (FAILED);
        
    }
        
    printf("\nCurrent environmental firmware revision is 0x%.4X.\n", version);

    return (PASSED);
}
#endif

/*********************************************************************
 *
 * Function:	select_firmware_type
 *
 * Description:	Select Environmental MCU firmware type
 *
 * Inputs:	None
 *
 * Output:	TRUE/FALSE
 *
 *********************************************************************
 */
static int
select_firmware_type(void)
{
    if (irq == INTR_ENV_MCU) {
        printf("\n\n++++++++Using ENV MCU Firmware++++++++\n");
        program_data_p = (uchar *)eprom;
        program_data_length = (int)eprom_length;
        program_data_revision = (uchar *)overlord_env_mcu_revision;
        program_eprom_start_addr = (ulong)eprom_start;
    }

    if (irq == INTR_VM_MCU) {
        printf("\n\n+++++++++Using Voltage Monitor Firmware+++++++\n");
        program_data_p = (uchar *)vm_eprom;
        program_data_length = (int)vm_eprom_length;
        program_data_revision = (uchar *)overlord_vmon_mcu_revision;
        program_eprom_start_addr = (ulong)vm_eprom_start;
    }

    return TRUE;
}

/*********************************************************************
 *
 * Function:	read_check_mcu_flash
 *
 * Description:	Read and check Environmental MCU flash data
 *
 * Inputs:	None
 *
 * Output:	PASSED/FAILED
 *
 *********************************************************************
 */
static int
read_check_mcu_flash(void)
{
    env_mcu_page_read_t page_read;
    int i, read_size, size;
    uchar read_flash_data[ENV_MCU_FLASH_LENGTH];
    volatile uchar *read_flash_p = read_flash_data;
    uchar *cmp_flash_p, *dump_start, *dump_end;
    uint addr, cmp_addr;
    int blk_no = 0;
    volatile uchar *data_p;

    select_firmware_type();

    data_p = (volatile uchar *)program_data_p;
    size = program_data_length;
    addr = program_eprom_start_addr;

    if (!size) {
        assert(!"program data length is 0!!!!");
    }

    //    printf("read and check MCU flash...\n");

    //    intr_cnt  = 0;
    msleep(FLASH_RD_WR_DELAY);
    show = 0;
    cmp_addr = program_eprom_start_addr;
    cmp_ptr = (uchar *)data_p;
    isr_data_chk = TRUE;
    isr_err_found = FALSE;
    blk_no = 0;
    /* Start read flash data */
    while (size) {
	prpass(testpass, "verifying block # %d", blk_no++);fflush(stdout);
        
	read_size = (size > ENV_MCU_MAX_DATA_SIZE) ? ENV_MCU_MAX_DATA_SIZE :
						     size;

	page_read.cmd = ENV_MCU_PAGE_READ;
	page_read.mid_addr = (addr & ENV_MCU_MID_ORDER_ADDR) >> ENV_MCU_MID_ADDR_SHIFT;
	page_read.high_addr = (addr & ENV_MCU_HIGH_ORDER_ADDR) >> ENV_MCU_HIGH_ADDR_SHIFT;

	env_mcu_rx_buf_p = env_mcu_rx_buf;
	/* Transmit read page with command and 2 byte of address */
	if (env_mcu_tx((uchar *)&page_read, 3)) {
	    printf("%s data trasmit failed\n", __FUNCTION__);
	    return FAILED;
	}
        //	msleep(00);
        msleep(FLASH_RD_WR_DELAY*2);

	/* Read the data back */
	if (env_mcu_read_rx_data(ENV_MCU_MAX_DATA_SIZE,
				 ENV_MCU_RX_DATA_TIMEOUT)) {
	    printf("%s  block %d failed\n", __FUNCTION__, blk_no);
	    return FAILED;
	}

        msleep(FLASH_RD_WR_DELAY*2);

	cmp_flash_p = (uchar *)read_flash_p; /* set compare starting point */
	/* Save read data to data storage area */
	for (i = 0; i < read_size; i++) {
	    *read_flash_p++ = env_mcu_rx_buf[i];
	}

	/* Compare the data */
	for (i = 0; i < read_size; i++, data_p++, cmp_flash_p++, cmp_addr++) {
	    if (*data_p != *cmp_flash_p) {
		printf("\nCompare flash data failed blk%d offset %d;  @0x%lx, "
                       "expect: 0x%x got: 0x%x.\n", blk_no, i, 
                       (ulong)cmp_addr - (ulong)eprom, *data_p, 
		       *cmp_flash_p);
		dump_start = (uchar *)cmp_flash_p;
		for (dump_end = (uchar *)cmp_flash_p + 100;
		     dump_start < dump_end; dump_start++) {
		    printf("0x%02x, ", *dump_start);
		}
		printf("\n");
		return FAILED;
	    }
	}

	size -= read_size;
	addr += read_size;
    }

    isr_data_chk = FALSE;

    printf("\n\nprogram successful.\n\n");

    (void)reset_start_mcu_flash();

    return PASSED;
}    


/*******************************************************************************
 *
 * Function   : program_mcu_flash
 * Description:	To program Environmental MCU flash.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int program_mcu_flash (void)
{
    env_mcu_page_prog_t page_prog;
    unsigned int i, program_size, size, check_program_status = 0;
    uint addr;
    uint blk_no = 1;
    volatile uchar *data_p;

    (void)reset_start_mcu_flash();

    select_firmware_type();

    data_p = (volatile uchar *)program_data_p;
    size = program_data_length;
    if (!size) {
        assert(!"program data length is 0");
    }
    addr = program_eprom_start_addr;

    /*
    program_data_p = (uchar *)eprom;
    program_data_length = (int)eprom_length;
    program_data_revision = (uchar *)overlord_env_mcu_revision;
    program_eprom_start_addr = (uint)eprom_start;
    */

#if 0
    if (get_version(&version)) {
        cterr('w', 0, "%s:%d Failed to read env firmware version.",
                      __FUNCTION__, __LINE__);
    }
    printf("\nCurrent firmware revision is 0x%X.  "
	   "\nYou are about to download revision 0x%s.  Proceed? (y/n) ",
	   version, program_data_revision);
    ans = getchar();
    if (ans == 'N' || ans == 'n')
	return PASSED;
#endif
    
    if (init_env_mcu()) {
	printf("Init MCU failed\n");
	return FAILED;
    }

    msleep(100); /* probably don't need */
    if (!env_mcu_id_check()) {
	printf("%s failed\n", __FUNCTION__);
	return FAILED;
    }

    msleep(100);/* probably don't need */
    if (erase_all_block()) {
	printf("Erase all blocks failed\n");
	return FAILED;
    }

    msleep(700);/* probably don't need */

    rx_size = 2;

    blk_no = 0;
    while (size) {
	prpass(testpass, "program block # %d ", blk_no);
        fflush(stdout);
	program_size = (size > ENV_MCU_MAX_DATA_SIZE) ? ENV_MCU_MAX_DATA_SIZE : size;

	page_prog.cmd = ENV_MCU_PAGE_PROG;
	page_prog.mid_addr = (addr & ENV_MCU_MID_ORDER_ADDR) >> ENV_MCU_MID_ADDR_SHIFT;
	page_prog.high_addr = (addr & ENV_MCU_HIGH_ORDER_ADDR) >> ENV_MCU_HIGH_ADDR_SHIFT;

	/* filling the data buffer. max. 256 bytes */
	for (i = 0; i < program_size; i++) {
	    page_prog.data[i] = *data_p++;
	    addr++;
	}

	/* If data size is less than 256 then filling the remainding data
	   with 0xff */
	if (program_size < ENV_MCU_MAX_DATA_SIZE) {
	    for ( ; i < ENV_MCU_MAX_DATA_SIZE; i++) {
		page_prog.data[i] = 0xff;
		addr++;
	    }
	}

        msleep(FLASH_RD_WR_DELAY);

        if (env_mcu_tx((uchar *)&page_prog, program_size+3)) {
            printf("%s data trasmit failed\n", __FUNCTION__);
            return FAILED;
        }

        /* if device is blanked, program_status_ok() will always fails so we should not call
           it */
        if (check_program_status) {
            if (program_status_ok() != TRUE) {
                printf("%s prgoram status faied block %d  intr_cnt = %d\n", __FUNCTION__, blk_no, intr_cnt);
                return FAILED;
            }
            msleep(10);
        } else {
            msleep(20);
        }
        
	size -= program_size;
        blk_no++;
    }
    //    fflush(stdout);

    if (read_check_mcu_flash()) {
	printf("%s failed\n", __FUNCTION__);
	return FAILED;
    }

    return PASSED;
}    


/*******************************************************************************
 *
 * Function   : reset_start_mcu_flash
 * Description:	To reset and start Environmental MCU flash program.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int reset_start_mcu_flash (void)
{
    //    printf("\nReset and start MCU flash program ");

    /* Make sure interrupts are disabled */
    IOFPGA_MCU_REGS->mcu_dnld_intr_enable = 0;
    //    disable_platform_mcu_intr(FPGA_ENV_INTR);
    disable_intr();

    /* Enable MCU reset */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= (ENV_MCU_CTRL_RESET_EN | 
				       ENV_MCU_CTRL_DISABLE_RX);
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= ENV_MCU_CTRL_RESET_EN;

    /* Put MCU in reset */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl |= ENV_MCU_CTRL_RESET;
    msleep(25);

    /* Deassert MCU reset */
    IOFPGA_MCU_REGS->mcu_dnld_ctrl &= ~ENV_MCU_CTRL_RESET;
    sleep(1); //should be 5 secs for voltage enviromnet?
    
#if 0
    if (irq == INTR_ENV_MCU) {
        printf("resetting ENV MCU..please wait\r");
        sleep(1); //shouelbe 5 secs for voltage enviromnet
    } else {
        printf("resetting VM..please wait\r");
        sleep(5); //shouelbe 5 secs for voltage enviromnet
    }
#endif   

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Done !!\n", __FUNCTION__);
    }

    return (PASSED);
}

void env_mcu_intr_hndlr (int irq, void *arg)
{
    unsigned char *buf_p = (unsigned char *)arg;
    uint32_t reg_data = 0, ctr = 0;
    volatile uint *ptr = (volatile uint *)&intr_cnt;

    if (IOFPGA_MCU_REGS->mcu_dnld_status & 0x2) {
        reg_data = IOFPGA_MCU_REGS->mcu_dnld_data;

        /* clear the receive data bit */
        IOFPGA_MCU_REGS->mcu_dnld_status |= (0x2);

        buf_p[intr_cnt] = reg_data;
        *ptr = *ptr + 1;

        if (show) {
            for (ctr = 0; ctr < (rx_size); ctr++) {
                printf("buf_p[%d] = %#x.\n", ctr, buf_p[ctr]);
            }
        }

    }
}


/*------------------------------------------------------------------
$Log: platform_mcu.c,v $
Revision 1.11  2017/07/10 02:27:50  leschen
Remove unused variable

Revision 1.10  2013/11/26 08:40:38  hroni
fix compiler warning

Revision 1.9  2013/05/16 00:49:16  mcharon
increase delay during env firmware download

Revision 1.8  2012/08/22 20:06:50  mcharon
when programming failed, dump out content of firmware stored on device

Revision 1.7  2012/08/22 19:31:06  mcharon
remove cs_key_buffer.h

Revision 1.6  2012/08/06 22:54:11  mcharon
during VM fw download, don't check fw statushe-- will fail if device is blank

Revision 1.5  2012/06/22 01:20:06  mcharon
support volate monitor firmware download

Revision 1.4  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.3  2012/06/05 11:44:37  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/

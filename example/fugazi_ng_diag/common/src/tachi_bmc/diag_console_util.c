/* $Id: diag_console_util.c,v 1.3 2016/09/02 06:43:15 jimmyya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_console_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_console_util.c - Console Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <unistd.h>
#include "common.h"
#include "queryflags.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "common_utils.h"
#include "diag_fpga_lib.h"
#include "diag_console_util.h"
#include "ngio.h"

int diag_console_util(void);

static int diag_uart_to_intel(void);
static int diag_uart_to_nim(int);
static int diag_uart_to_switch(void);
static int diag_uart_to_isp(void);
static void diag_console_switch(char *);
static int diag_connect_uart1_to_dev(int);
#ifdef FOXCONN_FPGA
static int diag_connect_dev_to_uart1(int, int);
static void diag_clean_up_mux(int);
#endif 

/* Sub Menu used for Console utility.
 */
static submenu_xtable_t console_util_submenu_table[] = {
    {"Switch to Intel UART", (type_t(*)())diag_uart_to_intel,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Switch to NIM1 UART", (type_t(*)())diag_uart_to_nim,   NIM1,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
#ifdef TACHI_H
    {"Switch to NIM2 UART", (type_t(*)())diag_uart_to_nim,   NIM2,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Switch to NIM3 UART", (type_t(*)())diag_uart_to_nim,   NIM3,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},    
#endif
    {"Switch to Lewis UART", (type_t(*)())diag_uart_to_switch,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Switch to ISP test card UART", (type_t(*)())diag_uart_to_isp,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define CONSOLE_UTIL_SUBMENU_TABLE_SIZE (sizeof(console_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t console_util_primary_items[CONSOLE_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t console_util_secondary_items[CONSOLE_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t console_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    console_util_primary_items,
};
menuinfo_t *console_util_submenup = &console_util_subtest_menu;

int diag_console_util (void)
{
    build_primary_submenu(console_util_submenu_table,
			              CONSOLE_UTIL_SUBMENU_TABLE_SIZE,
                          "Console Switch Utility", &console_util_submenup);
    build_secondary_submenu(console_util_submenu_table,
                            CONSOLE_UTIL_SUBMENU_TABLE_SIZE,
                            console_util_secondary_items);    
                            
    menu(console_util_submenup, console_util_secondary_items, '\0');
    return (PASSED);
}

int diag_uart_to_nim_cnnt (int slot_no)
{
    unsigned int val; 

    switch(slot_no) {
    case NIM1:
	val = FPGA_BMC_UART1_NIM1; 
	break;
    case NIM2:
	val = FPGA_BMC_UART1_NIM2; 
	break;
    case NIM3:
	val = FPGA_BMC_UART1_NIM3; 
	break;
    default:
	printf("\nSlot %d not supported\n", slot_no);
	return FAILED;
    }

#ifdef FOXCONN_FPGA

    if (diag_connect_dev_to_uart1(FPGA_MUX_SEL_CTRL_REG2, FPGA_UART_NIM1_SEL)
        == FAILED) {
        printf("%s: Fails in connecting device to UART1\n", __FUNCTION__);
        return (FAILED);
    }

    if (diag_connect_uart1_to_dev(FPGA_UART_MUX_SEL_NIM1) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
#else 
    if (diag_connect_uart1_to_dev(val) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
#endif 

    return (PASSED);
}

int diag_uart_to_isp_cnnt (void)
{
    unsigned int val; 

    val = FPGA_BMC_UART1_ISP_CARD; 
    
    if (diag_connect_uart1_to_dev(val) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


static int diag_uart_to_intel (void)
{
    char buf[64]; 
    
#ifdef FOXCONN_FPGA
    if (diag_connect_dev_to_uart1(FPGA_MUX_SEL_CTRL_REG1, FPGA_UART_INTEL_SEL)
        == FAILED) {
        printf("%s: Fails in connecting device to UART1\n", __FUNCTION__);
        return (FAILED);
    }

    if (diag_connect_uart1_to_dev(FPGA_UART_MUX_SEL_INTEL) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }

    diag_console_switch(UART_TTYS2_DEV);
#else 

    /* control bmc uart mux before console switch */
    sprintf(buf, "echo \"%d\" > /proc/nuova/enable_sol_com1", ENABLE); 
    system(buf);

    diag_console_switch(UART_TTYS3_DEV);

    /* revert status before exit */
    sprintf(buf, "echo \"%d\" > /proc/nuova/enable_sol_com1", DISABLE); 
    system(buf);
#endif 

    return (PASSED);
}

static int diag_uart_to_nim (int slot)
{
    unsigned int val;

    switch(slot) {
    case NIM1:
	val = FPGA_BMC_UART1_NIM1; 
	break;
    case NIM2:
	val = FPGA_BMC_UART1_NIM2; 
	break;
    case NIM3:
	val = FPGA_BMC_UART1_NIM3; 
	break;
    default:
	printf("\nSlot %d not supported\n", slot);
	return FAILED;
    }
	
#ifdef FOXCONN_FPGA

    if (diag_connect_dev_to_uart1(FPGA_MUX_SEL_CTRL_REG2, FPGA_UART_NIM1_SEL)
        == FAILED) {
        printf("%s: Fails in connecting device to UART1\n", __FUNCTION__);
        return (FAILED);
    }

    if (diag_connect_uart1_to_dev(FPGA_UART_MUX_SEL_NIM1) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
#else
    if (diag_connect_uart1_to_dev(val) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
#endif
   
    diag_console_switch(UART_TTYS2_DEV);
    return (PASSED);
}

static int diag_uart_to_switch (void)
{
#ifdef FOXCONN_FPGA
    if (diag_connect_dev_to_uart1(FPGA_MUX_SEL_CTRL_REG2, FPGA_UART_CETUS_SEL)
        == FAILED) {
        printf("%s: Fails in connecting device to UART1\n", __FUNCTION__);
        return (FAILED);
    }

    if (diag_connect_uart1_to_dev(FPGA_UART_MUX_SEL_CETUS) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
#else 
    if (diag_connect_uart1_to_dev(FPGA_BMC_UART1_CETUS) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
#endif 
    diag_console_switch(UART_TTYS2_DEV);
    return (PASSED);
}

static int diag_uart_to_isp (void)
{

    unsigned int val; 

    val = FPGA_BMC_UART1_ISP_CARD; 
    
    if (diag_connect_uart1_to_dev(val) == FAILED) {
        printf("%s: Fails in connecting UART1 to device\n", __FUNCTION__);
        return (FAILED);
    }
    diag_console_switch(UART_TTYS2_DEV);
    return (PASSED);
}


#ifdef FOXCONN_FPGA
static void diag_clean_up_mux (int mux_addr)
{
    unsigned int rd_val, mask, dummy, ia;

    /* Clear the Mux selection for both 2 mux */
    if (diag_fpga_reg_read(mux_addr, &rd_val) == FAILED) {
        printf("%s: Fails in clean up mux 0x%x to device\n",
        __FUNCTION__, mux_addr);
        return;
    }

    /* clean up the other device which is connected to uart1 */
    mask = FPGA_UART_MUX_SEL_BMC_UART1;
    dummy = FPGA_UART_MUX_SEL_NULL;

    /* each unit is 4bits, 32 bits has 8 units */
    for (ia = 0; ia < 8; ia++) {
       if ((rd_val & dummy) == mask) {
           rd_val |= dummy;
       }
       mask = mask << 4;
       dummy = dummy << 4;
    }

    diag_fpga_reg_write(mux_addr, rd_val);
}
#endif 

#ifdef FOXCONN_FPGA
static int diag_connect_dev_to_uart1 (int mux_addr, int bit_pos)
{ 
    unsigned int rd_val;

    /* clean up mux which is connected to UART1 */
    diag_clean_up_mux(FPGA_MUX_SEL_CTRL_REG1); 
    diag_clean_up_mux(FPGA_MUX_SEL_CTRL_REG2); 

    /* Now, connect the target to UART 1 */
    if (diag_fpga_reg_read(mux_addr, &rd_val) == FAILED) {
        return (FAILED);
    }

    rd_val &= ~(0xF << bit_pos);
    rd_val |= (FPGA_UART_MUX_SEL_BMC_UART1 << bit_pos);

    return (diag_fpga_reg_write(mux_addr, rd_val));
}
#endif 

static int diag_connect_uart1_to_dev (int val)
{
    unsigned int rd_val, reg, bit_off;

#ifdef FOXCONN_FPGA
    reg = FPGA_MUX_SEL_CTRL_REG1; 
    bit_off = FPGA_UART_BMC_UART1_SEL;
#else     
    reg = FPGA_UART_MOD_MUX_REG; 
    bit_off = FPGA_BMC_UART1_SEL; 
#endif 

    /* Clear Uart1 Mux Selection */
    if (diag_fpga_reg_read(reg, &rd_val) == FAILED) {
        return (FAILED);
    }
    rd_val &= ~(0xF << bit_off);
    rd_val |= (val << bit_off);

    /* Now connect UART1 to target device */
    return (diag_fpga_reg_write(reg, rd_val));
}

static void diag_console_switch (char *tty)
{
    char cmd[PICOCOM_CMD_LENGTH];

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
           "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    usleep(100*1000);

    snprintf(cmd, PICOCOM_CMD_LENGTH - 1, "picocom %s -d8 -pn -fn %s",
             BAUD9600, tty); 

    printf("cmd=%s\n", cmd);
    system(cmd);
    return;
}


/*---------------------------------------------------------------
$Log: diag_console_util.c,v $
Revision 1.3  2016/09/02 06:43:15  jimmyya
change bluetooth conswitch utility to ISP utility

Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.8  2016/03/08 03:07:07  jimmyya
Add ISP testcard uart test

Revision 1.1.2.7  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.6  2015/10/25 10:58:58  tirawan
Change Cetus to Lewis

Revision 1.1.2.5  2015/10/05 05:43:00  alpeng
update baud rate t0 9600

Revision 1.1.2.4  2015/09/26 05:20:42  alpeng
update console switch utils for intel

Revision 1.1.2.3  2015/09/23 09:06:17  alpeng
update console switch util to support cetus and nim; intel not yet

Revision 1.1.2.2  2015/09/04 01:45:44  alpeng
update console swtich to use ttyS2(BMC UART1)

Revision 1.1.2.1  2015/07/12 06:52:45  tirawan
Add Console Switch Utility, SPI driver and FPGA programming


$Endlog$
*/


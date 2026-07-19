/* $Id: diag_geswitch_util.c,v 1.3 2019/09/10 01:03:39 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_geswitch_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_geswitch_util.c - GE PHY 88E1512 Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "nvmonvars.h"
#include "diag_geswitch_util.h"
#include "diag_smi_lib.h"
#include "patriot_linux/apps/common_utils.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_geswitch_test.h"

int diag_geswitch_util(void);
void diag_6320_eth1_para_input(void);

static int diag_geswitch_reg_alter(void);
static int diag_geswitch_reg_display(void);

static int diag_geswitch_port_pkt_count_display(void);
static int diag_geswitch_get_pkt_count(int, int);
static int diag_geswitch_flush_pkt_count(int);

extern int mrvl6320_alter(void);
extern int mrvl6320_dump(void);


int input_payload_size = PKT_PAYLOAD_SIZE;
int input_pattern; 
int input_packet_cnt = PACKET_COUNT;
int pattern_fix = 0;
/* Sub Menu used for MCU utility.
 */
static submenu_xtable_t geswitch_util_submenu_table[] = {
    {"Register Alter Utility", (type_t(*)())diag_geswitch_reg_alter,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Register Display Utility", (type_t(*)())diag_geswitch_reg_display,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Dump Port 2 Ingress Counters (X710)", (type_t(*)())diag_geswitch_port_pkt_count_display,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Alter BMC <-> 6320 loopback parameters", (type_t(*)())diag_6320_eth1_para_input,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GESWITCH_UTIL_SUBMENU_TABLE_SIZE (sizeof(geswitch_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t geswitch_util_primary_items[GESWITCH_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t geswitch_util_secondary_items[GESWITCH_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t geswitch_util_subtest_menu = {
    "%s Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    geswitch_util_primary_items,
};
menuinfo_t *geswitch_util_submenup = &geswitch_util_subtest_menu;

int diag_geswitch_util (void)
{
    build_primary_submenu(geswitch_util_submenu_table,
			              GESWITCH_UTIL_SUBMENU_TABLE_SIZE,
                          "GE Switch", &geswitch_util_submenup);
    build_secondary_submenu(geswitch_util_submenu_table,
                            GESWITCH_UTIL_SUBMENU_TABLE_SIZE,
                            geswitch_util_secondary_items);    
                            
    menu(geswitch_util_submenup, geswitch_util_secondary_items, '\0');
    return (PASSED);
}

/**********************************************************************
 *
 * Function:	diag_geswitch_reg_alter
 *
 * This function: alter value of marvell 6320 register
 *
 * Input: void
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int diag_geswitch_reg_alter (void)
{
	int port_num, reg_num, reg_ori_val, reg_val;
    int retval = PASSED;
	
    printf("\n\nGE SWITCH Register Read\n\n");
    
    /* Add Port range to write Global2 Register (0x1C) */
	port_num = getdec_answer("Port offset to alter", 0, 0, 30);
	reg_num = getdec_answer("Reg offset to alter", 0, 0, 31);
	reg_val = gethex_answer("Enter the new data (hex): ", reg_val, 0, 0xFFFF);

	/*  Before write, we have to read the original value first*/
    if ( diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                port_num, reg_num, &reg_ori_val) == FAILED ) {
        retval = FAILED;
    }

    /*  Do write process*/
    if ( diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                 port_num, reg_num, reg_val) == FAILED ) {
        retval = FAILED;
    }

    /*read again*/
    if ( diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                port_num, reg_num, &reg_val) == FAILED ) {
        retval = FAILED;
    }

    if(retval == FAILED){
        cterr('f', 0, "geswitch Alter failed");
    }

    return (retval);
}


/**********************************************************************
 *
 * Function:	mrvl6320_dump
 *
 * This function: dump value of marvell register
 *
 * Input: void
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int diag_geswitch_reg_display (void)
{
	int port_num, reg_num, reg_val;
    int retval = PASSED;
	printf("\n\nGE SWITCH Register Read\n\n");
    /* Add Port range to write Global2 Register (0x1C) */
	port_num = getdec_answer("Port offset to read", 0, 0, 30);
	reg_num = getdec_answer("Reg offset to read", 0, 0, 31);

    if ( diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID, port_num,
                                reg_num, &reg_val) == FAILED ) {
        retval = FAILED;
    }
 
    
    printf("reg_val 0X%x\n", reg_val);
    if(retval == FAILED){
        cterr('f', 0, "geswitch Display failed.");
    } 
    return (retval);
}

static int diag_geswitch_port_pkt_count_display(void)
{
    int pkt_count, port_num;
    int retval = PASSED;
	
    port_num = getdec_answer("Port offset to read", 0, 0, 6);

    port_num+=1;

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("port_num is %d\n", port_num);
    }

    pkt_count = diag_geswitch_get_pkt_count(MARVL6320_SO_STATS_PORT(port_num),
                                            MRVL6320_SO_HISTOGRAM_MODE_RT);

    if (pkt_count == MRVL6320_RW_ERROR) {
        retval = FAILED;
    } else {
        printf("Port %d Ingress packet count %d\n", port_num, pkt_count);
    }

    if(diag_geswitch_flush_pkt_count(MARVL6320_SO_STATS_PORT(port_num))
                                     == FAILED) {
        retval = FAILED;
    }

    if(retval == FAILED){
        cterr('f', 0, "geswitch pkt count Display failed.");
    }
    return (retval);
}

static int diag_geswitch_get_pkt_count(int port_num, int direction)
{
	int cmd, reg_S0_val, reg_S1_val, pkt_count;

    /*op: Capture */
    cmd = (MRVL6320_SO_STATS_BUSY | MRVL6320_SO_STATS_OP_CAPTURE_ALL_COUNTER |
           direction | MRVL6320_SO_STATS_BANK_0 |port_num |
           MRVL6320_SO_STATS_PTR_IN_GOOD_OCTET);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Capture cmd is is 0X%x\n", cmd);
    }

    if ( diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                            MRVL6320_GL_REG_1, MRVL6320_SO, cmd) == FAILED ) {
        return (MRVL6320_RW_ERROR);
    }

    /*op: Read*/
    cmd = (MRVL6320_SO_STATS_BUSY | MRVL6320_SO_STATS_OP_READ | direction |
           MRVL6320_SO_STATS_BANK_0 |port_num | MRVL6320_SO_STATS_PTR_IN_BROADCASTS);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Read cmd is is 0X%x\n", cmd);
    }

    if ( diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                 MRVL6320_GL_REG_1, MRVL6320_SO, cmd) == FAILED ) {
        return (MRVL6320_RW_ERROR);
    }

    if ( diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                MRVL6320_GL_REG_1, MRVL6320_S0, &reg_S0_val) == FAILED ) {
        return (MRVL6320_RW_ERROR);
    }

    if ( diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                MRVL6320_GL_REG_1, MRVL6320_S1, &reg_S1_val) == FAILED ) {
        return (MRVL6320_RW_ERROR);
    }

    pkt_count = reg_S0_val;
    return (pkt_count);
}

static int diag_geswitch_flush_pkt_count(int port_num)
{
	int cmd;
    int retval = PASSED;

	cmd = (MRVL6320_SO_STATS_BUSY | MRVL6320_SO_STATS_OP_FLUSH_A_PORT |
           MRVL6320_SO_HISTOGRAM_MODE_RT | MRVL6320_SO_STATS_BANK_0 | port_num |
           MRVL6320_SO_STATS_PTR_IN_BROADCASTS);
    if ( diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
                                 MRVL6320_GL_REG_1, MRVL6320_SO, cmd) == FAILED ) {
        retval = FAILED;
    }
    if(retval == FAILED){
        cterr('f', 0, "geswitch Flush pkt count failed.");
    }
    return (retval);
}

void diag_6320_eth1_para_input(void)
{

    input_packet_cnt = getdec_answer("Enter packet count:", 1, 1, 3000);
    input_payload_size = getdec_answer("Enter packet payload size:", 42, 42, 1496);
    input_pattern = gethex_answer("enter pattern[0x00-0xff]", 0, 0, 0xff);
    pattern_fix = 1;

}
/*---------------------------------------------------------------
$Log: diag_geswitch_util.c,v $
Revision 1.3  2019/09/10 01:03:39  haohsu
[CSCvr07313]-Marvell 6320 to BMC eth1 frame error issue

Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/09/15 06:46:54  benchen2
add i350 lpbk func

Revision 1.1.2.3  2015/08/14 05:53:22  benchen2
add diag_geswitch_port_pkt_count_display

Revision 1.1.2.2  2015/07/31 07:34:05  hondwang
geswitch util

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/


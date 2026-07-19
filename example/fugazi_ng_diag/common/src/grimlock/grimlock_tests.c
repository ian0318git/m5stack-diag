/* $Id: grimlock_tests.c,v 1.2 2020/03/13 12:06:53 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/grimlock/grimlock_tests.c,v $
 *------------------------------------------------------------------
 * grimlock_tests.c
 *
 * Wilbur Huang -- Jan. 2020
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "grimlock.h"
#include "grimlock_fpga.h"
#include "dev_object.h"
#include "dev_4359.h"

extern dev_4359_object_t dev_4359_object;
extern int init_framer_for_lpbk (int port_num, int op_mode, 
				 frmr_lpbk_mode lpbk_mode);
extern int set_tdmsw_lpbk_test(int port, int mode, int lpbk);
extern int npu_loopback_test (int bp_enet, int dc_enet, int tdi_num, 
			      npu_lpbk_mode loop_mode, int op_mode);
extern int  npu_bp_sgmii_lpbk_test();
extern int  npu_dc_sgmii_lpbk_test();
extern int  npu_tdi_lpbk_test();
extern void npu_release_driver();
extern int  tdm_cleanup();
extern void fpga_reset_framer();
extern void fpga_unreset_framer();
extern int  is_board_t1_mode();
extern int  is_port_clk_master_mode(int port_num);
extern void fpga_reset_tdm_pll();
extern void fpga_unreset_tdm_pll();
extern void fpga_config_tdm_pll(frmr_clk_mode clk_mode);
extern int  fpga_check_tdm_pll();
extern void npu_print_dev_statistic (int bp_sgmii, int dc_sgmii, 
				     int tdi_num, int loop_mode);

static int  tdm_lpbk_test();

/* submenu for Grimlock loopback test */
submenu_xtable_t lpbk_tests_submenu_table[] = {
    {"NPU backplane SGMII loopback test",  
     (PFT)npu_bp_sgmii_lpbk_test,     0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"NPU daughter card SGMII loopback test",  
     (PFT)npu_dc_sgmii_lpbk_test,     0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"NPU TDI loopback test",
     (PFT)npu_tdi_lpbk_test,          0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"TDM loopback test",    
     (PFT)tdm_lpbk_test,              0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
};

#define LPBK_TESTS_SUBMENU_TABLE_SIZE (sizeof(lpbk_tests_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lpbk_tests_primary_items[LPBK_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t lpbk_tests_secondary_items[LPBK_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t lpbk_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    lpbk_tests_primary_items,
};
menuinfo_t *lpbk_submenup = &lpbk_subtest_menu;

/*********************************************************************
 *
 * Function: grimlock_lpbk_tests()
 *
 * Description: First build the primary & secondary submenus for the
 * Grimlock loopback tests.  If the given arg is TRUE, execute all the 
 * tests in the menu flagged with MF_DOALL, and return the result.  
 * Otherwise, present the menu to the user for interaction.
 *
 * Inputs: TRUE/FALSE       
 * Outputs: PASSED
 *
 *********************************************************************
 */
int grimlock_lpbk_tests (boolean lpbk_test_items_executed)
{
    build_primary_submenu(lpbk_tests_submenu_table, 
			  LPBK_TESTS_SUBMENU_TABLE_SIZE,
			  "Grimlock Loopback", &lpbk_submenup);
    build_secondary_submenu(lpbk_tests_submenu_table,
			    LPBK_TESTS_SUBMENU_TABLE_SIZE,
			    lpbk_tests_secondary_items);

    if (lpbk_test_items_executed) {
        do_all_menu_items(&lpbk_subtest_menu);
    } else {
        menu(&lpbk_subtest_menu, lpbk_tests_secondary_items, '\0');
    }
    return PASSED;
}

/*********************************************************************
 *
 * Function: system_cleanup()
 *
 * Description: This function will cleanup TDMSW, Framer and WDDI driver.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int system_cleanup() 
{
    npu_release_driver();

    if (tdm_cleanup() == FAILED) {
	cterr('f',0,"Failed to do TDM cleanup");
	return (FAILED);
    }
    usleep(10000);
}


/*********************************************************************
 *
 * Function: bp_sgmii_lpbk_test()
 *
 * Description: This function is to test backplane SGMII interface.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
int bp_sgmii_lpbk_test ()
{
    int ret_val = PASSED;

    /* assume the loopback is setup in the backplane GE switch. */
    prpass(testpass, "NPU backplane lpbk through SGMII port %d test ", 
	   SGMII_INTERFACE_1);
    ret_val = npu_loopback_test(SGMII_INTERFACE_1, 0, 0, 
				NPU_BP_ENET_PASSTHRU, 0); 
    if (ret_val == FAILED) {
	npu_print_dev_statistic(SGMII_INTERFACE_1, 0, 0, NPU_BP_ENET_PASSTHRU);
	npu_release_driver();
	cterr('f', 0, "Failed host backplane SGMII lpbk test");
	return (FAILED);
    } else {
	npu_release_driver();	
	return (PASSED);
    }
}

/*********************************************************************
 *
 * Function: tdm_lpbk_test()
 *
 * Description: This function enables lpbk within FPGA TDMSW and 
 * send/receive data in NPU.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int tdm_lpbk_test ()
{
    int i, port_max;
    int ret_val;

    port_max = get_num_ports();

    for (i = 0; i < port_max; i++) {
	prpass(testpass, "FPGA TDM port %d lpbk test ", i);

	/* first reset framer */
	fpga_reset_framer();
	usleep(1000);
	fpga_unreset_framer();
	usleep(1000);

	/* set CTCLK_SRC to be 8KHz */
	fpga_set_ctclk_src(CTCLK_SRC_8K);

	/* Configure TDM PLL */
	fpga_config_tdm_pll(CLK_SLAVE);

	if (set_tdmsw_lpbk_test(i, CMQ_MODE_E1, TRUE) == FAILED) {
	    tdm_cleanup();
	    return (FAILED);
	}

	ret_val = npu_loopback_test(SGMII_INTERFACE_1, 0, i, 
				    NPU_TDI_PASSTHRU_SLAVE, CMQ_MODE_E1);
	if (ret_val == FAILED) {
	    npu_print_dev_statistic(SGMII_INTERFACE_1, 0, i, 
				    NPU_TDI_PASSTHRU_SLAVE);
	    npu_release_driver();
	    cterr('f', 0, "Failed TDMSW lpbk test for port %d", i);
	    return (FAILED);
	} else {
	    ret_val = system_cleanup();
	    if (ret_val == FAILED)
		return (FAILED);
	}
    }

    return (PASSED);
}

/******** History ********
$Log: grimlock_tests.c,v $
Revision 1.2  2020/03/13 12:06:53  letsai
Merge Grimlock NIM to maintrunk

Revision 1.1.4.3  2020/01/15 09:28:49  wilbhuan
Removed T1/E1 framer function.

Revision 1.1.4.2  2020/01/15 03:30:11  wilbhuan
1. Initial code of Grimlock NIM application.
2. Leveraged from Fortitude Grimlock NIM.
3. Only replace all Fortitude related word as Grimlock.
4. Fortitude's T1/E1 function doesn't remove.

$Endlog$
*/

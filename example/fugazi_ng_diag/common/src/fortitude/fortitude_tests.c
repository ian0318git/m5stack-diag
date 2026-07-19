/* $Id: fortitude_tests.c,v 1.15 2013/05/07 16:40:28 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/fortitude_tests.c,v $
 *------------------------------------------------------------------
 *
 * fortitude_tests.c - Include all the Fortitude loopback tests.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
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
#include "fortitude.h"
#include "fortitude_fpga.h"
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
static int  ext_lpbk_test_master();
static int  ext_lpbk_test_slave();
static int  framer_dig_lpbk_test_master();
static int  framer_dig_lpbk_test_slave();

/* submenu for Fortitude loopback test */
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
    {"Framer digital loopback test(clock master mode)",  
     (PFT)framer_dig_lpbk_test_master,0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Framer digital loopback test(clock slave mode)",  
     (PFT)framer_dig_lpbk_test_slave, 0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"External loopback test(clock master mode)",
     (PFT)ext_lpbk_test_master,       0, MM_2, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"External loopback test(clock slave mode)",
     (PFT)ext_lpbk_test_slave,        0, MM_2, (type_t(*)())0, 0, 
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
 * Function: fortitude_lpbk_tests()
 *
 * Description: First build the primary & secondary submenus for the
 * Fortitude loopback tests.  If the given arg is TRUE, execute all the 
 * tests in the menu flagged with MF_DOALL, and return the result.  
 * Otherwise, present the menu to the user for interaction.
 *
 * Inputs: TRUE/FALSE       
 * Outputs: PASSED
 *
 *********************************************************************
 */
int 
fortitude_lpbk_tests (boolean lpbk_test_items_executed)
{
    build_primary_submenu(lpbk_tests_submenu_table, 
			  LPBK_TESTS_SUBMENU_TABLE_SIZE,
			  "Fortitude Loopback", &lpbk_submenup);
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
static int
system_cleanup() 
{
    npu_release_driver();

    if (tdm_cleanup() == FAILED) {
	cterr('f',0,"Failed to do TDM cleanup");
	return (FAILED);
    }
#ifdef YWEN
    fpga_reset_framer();
    usleep(1000);
    fpga_unreset_framer();
#endif
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
int  
bp_sgmii_lpbk_test ()
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
static int  
tdm_lpbk_test ()
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

/*********************************************************************
 *
 * Function: do_lpbk_test_slave()
 *
 * Description: This function will do loopback test for one port based on
 * loopback mode and T1/E1 type. The framer is configured in clock slave mode.
 *
 * Inputs: port - port number
 *         frm_port - framer port number
 *         lpbk_mode - FRMR_DIG_LPBK, FRMR_EXT_LPBK_SLAVE, FRMR_ALOOP
 *         op_mode - CMQ_MODE_T1, CMQ_MODE_E1
 *
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int  
do_lpbk_test_slave (int port, int frm_port, frmr_lpbk_mode lpbk_mode, int op_mode)
{
    int mode;
    int retval = PASSED;
    fpga_reg_t *fpga_reg;

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    if (op_mode == CMQ_MODE_T1) {
	mode = TRUE;
    } else {
	mode = FALSE;
    }

    /* set CTCLK_SRC to be 8KHz */
    fpga_set_ctclk_src(CTCLK_SRC_8K);
    
    /* Configure TDM PLL */
    fpga_config_tdm_pll(CLK_SLAVE);

    if (set_tdmsw_lpbk_test(port, op_mode, FALSE) == FAILED) {
	tdm_cleanup();
	return (FAILED);
    }
	
    if (init_framer_for_lpbk(frm_port, op_mode, lpbk_mode) == FAILED) {
	tdm_cleanup();
	return (FAILED);
    }

    /* add this delay to fix CSCua79599 */
    sleep(1);

    /* check to see if FPGA detects the T1 mode and clock slave mode */
    if (is_board_t1_mode() != mode) {
	tdm_cleanup();
	cterr('f',0,"The framer is not in the %s mode.", mode==TRUE?"T1":"E1");
	return (FAILED);
    }

    if (is_port_clk_master_mode(frm_port) == TRUE) {
	tdm_cleanup();
	cterr('f',0,"The framer port %d is not in the clock slave mode.", 
	      frm_port);
	return (FAILED);
    }

    retval = npu_loopback_test(SGMII_INTERFACE_1, 0, port, 
			       NPU_TDI_PASSTHRU_SLAVE, op_mode);
    if (retval == FAILED) {
	printf("\npll_ctrl_status[0] = %#x, pll_ctrl_status[1] = %#x\n", 
	       fpga_reg->pll_ctrl_status[0], fpga_reg->pll_ctrl_status[1]);
	
	npu_print_dev_statistic(SGMII_INTERFACE_1, 0, port, 
				NPU_TDI_PASSTHRU_SLAVE);
	npu_release_driver();	
	cterr('f', 0, "Failed port %d %s slave mode %s lpbk test", port, 
	      mode==TRUE?"T1":"E1", 
	      lpbk_mode==FRMR_DIG_LPBK_SLAVE?"framer digital":"external");
	return (FAILED);
    } else {
	retval = system_cleanup(); 
	return (retval);
    }
}

/*********************************************************************
 *
 * Function: do_lpbk_test_master()
 *
 * Description: This function will do loopback test for one port based on
 * loopback mode and T1/E1 type. The framer is configured in clock master mode.
 *
 * Inputs: port - port number
 *         frm_port - framer port number
 *         lpbk_mode - FRMR_DIG_LPBK, FRMR_EXT_LPBK_SLAVE, FRMR_ALOOP
 *         op_mode - CMQ_MODE_T1, CMQ_MODE_E1
 *
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int  
do_lpbk_test_master (int port, int frm_port, frmr_lpbk_mode lpbk_mode, int op_mode)
{
    int mode;
    int retval = PASSED;
    dev_4359_object_t *pmc4359_p;
    unsigned long frmr_bar;
    uchar rdval = 0;

    /* initialize framer for requested loopback mode */
    if (op_mode == CMQ_MODE_T1) {
	mode = TRUE;
    } else {
	mode = FALSE;
    }

    /* set CTCLK_SRC to be 2MHz */
    fpga_set_ctclk_src(CTCLK_SRC_2M);

    /* Configure TDM PLL */
    fpga_config_tdm_pll(CLK_MASTER);

    if (init_framer_for_lpbk(frm_port, op_mode, lpbk_mode) == FAILED) {
	return (FAILED);
    }

    /* fix CSCtz62238 */
    msleep(500);

    /* check to see if FPGA detects the T1 mode and clock master mode */
    if (is_board_t1_mode() != mode) {
	cterr('f',0,"The framer is not in the %s mode.", mode==TRUE?"T1":"E1");
	return (FAILED);
    }

    if (is_port_clk_master_mode(frm_port) == FALSE) {
	cterr('f',0,"The framer port %d is not in the clock master mode.", 
	      frm_port);
	return (FAILED);
    }

    retval = npu_loopback_test(SGMII_INTERFACE_1, 0, port, 
			       NPU_TDI_PASSTHRU_MASTER, op_mode);

    if (retval == FAILED) {
	npu_print_dev_statistic(SGMII_INTERFACE_1, 0, port, 
				NPU_TDI_PASSTHRU_MASTER);
	npu_release_driver();	
	cterr('f', 0, "Failed port %d %s master mode %s lpbk test", port, 
	      mode==TRUE?"T1":"E1", 
	      lpbk_mode==FRMR_DIG_LPBK_MASTER?"framer digital":"external");
	return (FAILED);
    } else {
	npu_release_driver();
	return (PASSED);
    }
}

/*********************************************************************
 *
 * Function: framer_dig_lpbk_test_slave()
 *
 * Description: This function enables digital lpbk in the Framer and 
 * send/receive data in NPU. The framer is configured in clock slave mode.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int  
framer_dig_lpbk_test_slave ()
{
    int i, port_max, frm_port;

    port_max = get_num_ports();
    frm_port = 0;

    for (i = 0; i < port_max; i++) {
	prpass(testpass, "Port %d E1 mode Framer digital lpbk test(slave mode) ", i);
	if (do_lpbk_test_slave(i, frm_port, FRMR_DIG_LPBK_SLAVE, CMQ_MODE_E1)
	    == FAILED) {
	    return (FAILED);
	}
	if (port_max == 2) {
	    /* 2 port SKU uses framers 0 and 3 */
	    frm_port = 3;
	} else {
	    frm_port++;
	}
    }
}

/*********************************************************************
 *
 * Function: framer_dig_lpbk_test_master()
 *
 * Description: This function enables digital lpbk in the Framer and 
 * send/receive data in NPU. The framer is configured in clock master mode.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int  
framer_dig_lpbk_test_master ()
{
    int i, port_max;
    int frm_port = 0;

    port_max = get_num_ports();

    for (i = 0; i < port_max; i++) {
	prpass(testpass, "Port %d T1 mode Framer digital lpbk test(master mode) ", i);
	if (do_lpbk_test_master(i, frm_port, FRMR_DIG_LPBK_MASTER, CMQ_MODE_T1) 
	    == FAILED) {
	    return (FAILED);
	}
	if (port_max == 2) {
	    /* 2 port SKU uses framers 0 and 3 */
	    frm_port = 3;
	} else {
	    frm_port++;
	}
    }

    frm_port = 0;
    for (i = 0; i < port_max; i++) {
	prpass(testpass, "Port %d E1 mode Framer digital lpbk test(master mode) ", i);
	if (do_lpbk_test_master(i, frm_port, FRMR_DIG_LPBK_MASTER, CMQ_MODE_E1) == FAILED) {
	    return (FAILED);
	}
	if (port_max == 2) {
	    /* 2 port SKU uses framers 0 and 3 */
	    frm_port = 3;
	} else {
	    frm_port++;
	}
    }
}

/*********************************************************************
 *
 * Function: ext_lpbk_test_slave()
 *
 * Description: This function needs external lpbk connector plugged in.
 * Data is sent from NPU, through NPU internal GE port and TDI port, goes
 * to FPGA and framer, and then loops back at external lpbk connector.
 * The framer is configured in clock slave mode.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int  
ext_lpbk_test_slave ()
{
    int i, port_max;
    int frm_port = 0;

    /*
     * if D_EXT_LOOPBACK is OFF, then just return
     */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (PASSED);
    }

    port_max = get_num_ports();

    for (i = 0; i < port_max; i++) {
	prpass(testpass, "Port %d E1 mode external lpbk test(slave mode) ", i);
	if (do_lpbk_test_slave(i, frm_port, FRMR_EXT_LPBK_SLAVE, CMQ_MODE_E1) 
	    == FAILED) {
	    return (FAILED);
	}
	if (port_max == 2) {
	    /* 2 port SKU uses framers 0 and 3 */
	    frm_port = 3;
	} else {
	    frm_port++;
	}
    }
}

/*********************************************************************
 *
 * Function: ext_lpbk_test_master()
 *
 * Description: This function needs external lpbk connector plugged in.
 * Data is sent from NPU, through NPU internal GE port and TDI port, goes
 * to framer(bypass FPGA), and then loops back at external lpbk connector.
 * The framer is configured in clock master mode.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int  
ext_lpbk_test_master ()
{
    int i, port_max;
    int frm_port;

    /*
     * if D_EXT_LOOPBACK is OFF, then just return
     */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (PASSED);
    }

    port_max = get_num_ports();
    frm_port = 0;
    for (i = 0; i < port_max; i++) {
	prpass(testpass, "Port %d T1 mode external lpbk test(master mode) frm_port = %d ", i, frm_port);
	if (do_lpbk_test_master(i, frm_port, FRMR_EXT_LPBK_MASTER, CMQ_MODE_T1)
	    == FAILED) {
	    return (FAILED);
	}
	if (port_max == 2) {
	    /* 2 port SKU uses framers 0 and 3 */
	    frm_port = 3;
	} else {
	    frm_port++;
	}
    }

    frm_port = 0;
    for (i = 0; i < port_max; i++) {
	prpass(testpass, "Port %d E1 mode external lpbk test(master mode) ", i);
	if (do_lpbk_test_master(i, frm_port, FRMR_EXT_LPBK_MASTER, CMQ_MODE_E1)
	    == FAILED) {
	    return (FAILED);
	}
	if (port_max == 2) {
	    /* 2 port SKU uses framers 0 and 3 */
	    frm_port = 3;
	} else {
	    frm_port++;
	}
    }
}

/**********************************************************************
 *
 * Function: fortitude_aloop_util()
 *
 * This function will test the path from the NPU through PMC Framer all
 * the way to the external RJ45 port located on the Fortitude NGWIC.
 * In this test, the NPU and Fortitude1 FPGA as well as the PMC framer will
 * not loopback the data. Fortitude TDM engine will be configured to switch
 * NPU TDI ports to each of the Framer ports.
 * Data will be transmitted from the NPU through Fortitude FPGA, PMC framer
 * and the data will be expected to be looped back but will fail
 * because the framer will be setup for Y-cable mode.  
 * A Y-cable is not needed for this test.  First, insure that the
 * external loopback test passes, then execute this test; it should
 * fail.  This indicates that the data path has been switched inside
 * the framer due to enabling of the y-cable option.
 * Tests results from this test are not accurate if the external
 * loopback test fails.

 *
 * Framer registers:
 *	register	name			value	addr offset
 *	--------	----			-----	-----------
 *	0xA		CMQ_MST_DIAG		0x40	0x28
 * 	0xBE		CMQ_TERM_CNTRL		0x0	0x2f8
 *	0xF0		CMQ_XLPG_LINE_DRV_CFG	0x40	0x3c0
 *
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
fortitude_aloop_util ()
{
    int port_max, port, frm_port;

    printf("\n Insure that the external loopback test passes before"
        " executing this test.\n");
    printf("\n Failure during this test indicates that the framer"
	" Y-cable setting is correct.\n\n");

    port_max = get_num_ports();

    if (port_max > 1) {
        port = gethex_answer("\nEnter port number to test:",
                             0, 0, port_max - 1);
    } else {
        port = 0;
    }

    if ((port_max == 2) & (port == 1)) {
	frm_port = 3;
    } else {
	frm_port = port;
    }

    prpass(testpass, "Port %d T1 mode Framer aloop test", port);
    if (do_lpbk_test_master(port, frm_port, FRMR_ALOOP, CMQ_MODE_T1) 
	== FAILED) {
	printf("\nY-cable option functions as expected!\n");
	return (PASSED);
    }
    printf("\nY-cable option does not function properly!\n");
    return (FAILED);
}


/*********************************************************************
 *
 * Function: fortitude_lpbk_util()
 *
 * Description: This function will provide the utility for the user to test 
 * individual framer port. 
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
int  
fortitude_lpbk_util ()
{
    int port_max, port, op_mode, lpbk, frm_port;

    port_max = get_num_ports();

    if (port_max > 1) {
        port = gethex_answer("\nEnter port number to test:",
                             0, 0, port_max - 1);
    } else {
        port = 0;
    }

    if ((port_max == 2) & (port == 1)) {
	frm_port = 3;
    } else {
	frm_port = port;
    }

    lpbk = gethex_answer("\nSelect loopback type \n0 - framer digital lpbk "
			 "in slave mode, \n1 - framer digital lpbk in master "
			 "mode, \n2 - external lpbk in slave mode, "
			 "\n3 - external lpbk in master mode "
			 "\n4 - framer analog lpbk in master mode:",
			 0, 0 , 4);

    if ((lpbk == FRMR_EXT_LPBK_MASTER) || (lpbk == FRMR_DIG_LPBK_MASTER)
	|| (lpbk == FRMR_ALOOP_LPBK_MASTER)) {
	op_mode = gethex_answer("\nSelect operation mode (0 - T1, 1 - E1):",
				0, 0, 1);

	if (do_lpbk_test_master(port, frm_port, lpbk, op_mode) == FAILED) {
	    return (FAILED);
	}
    } else {
	if (do_lpbk_test_slave(port, frm_port, lpbk, CMQ_MODE_E1) == FAILED) {
	    return (FAILED);
	}
    }
    return (PASSED);
}

/*********************************************************************
 *
 * Function: set_bp_reference_clock()
 *
 * Description: This function will configure framer in clock master mode
 * and set FPGA to output NGWIC SYNC_IN to the backplane. 
 * For Nightster use only.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
int  
set_bp_reference_clock ()
{
    fpga_reg_t *fpga_reg;
    dev_4359_object_t *pmc4359 = &dev_4359_object;
    unsigned long frmr_bar;
    uchar rdval = 0;

    frmr_bar = get_framer_base();

    fpga_reg = (fpga_reg_t *)(get_fpga_base() + FPGA_GENERAL_REG_BASE);

    /* set CTCLK_SRC to be 8KHz */
    fpga_set_ctclk_src(CTCLK_SRC_8K);

    /* Configure TDM PLL */
    fpga_config_tdm_pll(CLK_SLAVE);

    if (set_tdmsw_lpbk_test(0, CMQ_MODE_E1, FALSE) == FAILED) {
	tdm_cleanup();
	return (FAILED);
    }

    if (init_framer_for_lpbk(0, CMQ_MODE_E1, FRMR_EXT_LPBK_SLAVE) == FAILED) {
	tdm_cleanup();
	return (FAILED);
    }

    /* configure framer slice 0 to output 8KHz on the RSYNC */
    rdval = pmc4359->callout_fvt->rd_frm_reg(frmr_bar, 0x02, 
					     pmc4359->bus_width);
    pmc4359->callout_fvt->wr_frm_reg(frmr_bar, 0x02, (rdval | 0x08),
				     pmc4359->bus_width);

    /* add this delay to fix CSCua79599 */
    sleep(1);

    /* check to see if FPGA detects the T1 mode and clock master mode */
    if (is_board_t1_mode() != FALSE) {
	tdm_cleanup();
	cterr('f',0,"The framer is not in the E1 mode.");
	return (FAILED);
    }

    if (is_port_clk_master_mode(0) == TRUE) {
	tdm_cleanup();
	cterr('f',0,"The framer port 0 is not in the clock slave mode.");
	return (FAILED);
    }

    /* set input from bp. */
    fpga_reg->pll_ctrl_status[0] |= 0x15;
    sleep(1);

    return (PASSED);
}


/******** History ********
$Log: fortitude_tests.c,v $
Revision 1.15  2013/05/07 16:40:28  ywen
Update set_bp_reference_clock() for Nightster platform.

Revision 1.14  2013/03/19 18:23:49  ywen
Add utility to set backplane reference clock for Nightster testing.

Revision 1.13  2012/09/10 23:17:18  ywen
code cleanup.

Revision 1.12  2012/08/29 22:45:45  ywen
Add framer analog loopback test utility.

Revision 1.11  2012/08/14 22:27:55  ywen
Improve fortitude loopback utility.

Revision 1.10  2012/07/20 18:27:06  ywen
fix CSCtz62238

Revision 1.9  2012/07/09 18:18:58  ywen
Fix for intermittent loopback test failure in clock slave mode -- CSCua79599

Revision 1.8  2012/06/13 17:54:34  ywen
Add support for TDMSW16 and 2 port SKU.

Revision 1.7  2012/06/05 22:21:17  ywen
Fix port number for loopback test.

Revision 1.6  2012/05/15 23:30:56  ywen
Add code to get PID from kernel file and parse the port number from that.

Revision 1.5  2012/05/14 23:21:09  ywen
Code cleanup and add debug information if test fails.

Revision 1.4  2012/04/12 23:15:38  ywen
Move backplane SGMII loopback to utility menu and code cleanup.

Revision 1.3  2012/04/02 21:04:32  ywen
Make framer y-cable utility work.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: ngio_testcard.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/ngio_testcard.c,v $
 *------------------------------------------------------------------
 * Filename   : ngio_testcard.c
 *
 * Description: Common function for TestCard.
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "byteswap.h"
#include "common.h"
#include "cookie_4.h"
#include "common_utils.h"
#include "error.h"
#include "menu.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "ngio.h"
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "testcard_eth.h"
#include "testcard_xaui.h"
#include "testcard_tlk_10232.h"
#include "testcard_plx_pcie_sw.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "proto.h"  /* for msleep */
#include "dash_fpga.h" /* is_platform() */
#include "cross_platform.h" /* SM_DAUGHTER_CARD */
#include "diag_nc_common.h"
#include "diag_lewis_gesw_test.h"
#include "intel_tests.h"
#include "diag_margin_util.h" 
#include "diag_plat_cookie.h" 
#include "platform_fru.h"

/**************************************************************
                      Function Prototypes             
 **************************************************************
 */
static int testcard_init(ngio_if *, int, int);
static int init_tc_i2c_struct(n2g_i2c_if_t *, int, int);
static int do_all_ngwic_testcard_tests(void);
static int do_all_nim_10gkr_testcard_tests(void);
static int is_support(void);
static void cterr_setup(int);
int for_10gkr_testcard(int);

/***************************************************************
                            Externs                                 
 ***************************************************************
 */
extern void build_tc_fpga_menu(int);
extern void build_tc_eth_menu(int);
extern void build_tc_xaui_menu(int);
extern void build_tc_pcie_menu(int);
extern void build_tc_uart_menu(int);
extern void build_tc_sig_menu(int);
extern void build_tc_pll_menu(int);
extern void build_tc_tlk10232_menu(int);
extern void build_tc_plx_menu(int);

extern int tc_fpga_reg_test(void);
extern int tc_sgmii_1g_lpbk_test(int);
extern int tc_xaui_lpbk_test(int);
extern int tc_pcie_8prbs_lpbk_test(void);
extern int tc_uart_lpbk_test(int);
extern int tc_sync_sig_test(void);
extern int tc_pll_locked_test(void);

/*****************************************************************
                       Global Variables              
 *****************************************************************
 */
static ngio_if        tc_ngio;
static n2g_i2c_if_t   tc_i2c_if;
static testcard_if_t  testcard_if;
struct ngio_intf_t *tc_ngwic_iface;

ngio_if        *tc_ngio_p;
testcard_if_t  *testcard_if_p;
uint32_t       tc_real_pcie_port = 0;

/******************************************************************
                             Menus                        
 ******************************************************************
 */
/*
 * WIC Testcard Menu
 */
static submenu_xtable_t wic_tc_table[] = {
    {"FPGA tests",          
     (PFT)build_tc_fpga_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, 
     (PFT)build_tc_fpga_menu, TRUE},
    {"Ethernet tests",           
     (PFT)build_tc_eth_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, FALSE,
     (PFT)build_tc_eth_menu,  TRUE},
    {"TLK10232 loopback tests",           
     (PFT)build_tc_tlk10232_menu,        FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)build_tc_tlk10232_menu,  TRUE},
    {"PCIe test (nc)",           
     (PFT)diag_nc_nim_testcard_test,     FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     0, 0},
#if 0  /* put PCIe test on Intel, skipped XAUI test  */
    {"XAUI tests",   
     (PFT)build_tc_xaui_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)build_tc_xaui_menu, TRUE},
    {"PLX PCIe sw tests",
     (PFT)build_tc_plx_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)build_tc_plx_menu,  TRUE},
    {"PCIe tests",
     (PFT)build_tc_pcie_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, FALSE,
     (PFT)build_tc_pcie_menu, TRUE},
#endif 
    {"UART tests",
     (PFT)build_tc_uart_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0,          0, 
     (PFT)build_tc_uart_menu, TRUE},
    {"Signal tests",
     (PFT)build_tc_sig_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)is_support, 0,
     (PFT)build_tc_sig_menu,  TRUE},
    {"PLL Lock tests",
     (PFT)build_tc_pll_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)is_support, 0, 
     (PFT)build_tc_pll_menu,  TRUE},
};

#define WIC_TC_TABLE_SIZE (sizeof(wic_tc_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t wic_tc_primary_items[WIC_TC_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t wic_tc_secondary_items[WIC_TC_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo wic_tc_diag = {
    "WIC TestCard Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    0,				/* do not show major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    wic_tc_primary_items,
};
static struct menuinfo *wic_tc_diagp = &wic_tc_diag;

static int is_support (void) 
{
    /* not support so far */
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : for_10gkr_testcad
 * Description: is the menu item for new testcard?
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
int for_10gkr_testcard (int inverse)
{
    assert(tc_ngwic_iface);

    if (tc_ngwic_iface->id == NIM_10GKR_TESTCARD) {
        if (inverse) {
            return (TRUE);
        } else {
            return (FALSE);
        }
    } else {
        if (inverse) {
            return (FALSE);
        } else {
            return (TRUE);
        }
    }
}

/*******************************************************************************
 *
 * Function   : init_tc_i2c_struct
 * Description: To init Test Card I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : None
 *
 *******************************************************************************
 */
static int init_tc_i2c_struct (n2g_i2c_if_t *i2c_if, int if_type, int slot)
{
    if (if_type == TC_NGWIC) {
        i2c_if->dev_name = "NIM TestCard";

        /* A fix value with PCIe Switch Ports Mapping */
        switch (slot) {
        case NIM1:
	case NIM2:
        case NIM3:     
            i2c_if->i2c_ctrl = get_wic_i2c_ctrl(slot);
            /* tc_real_pcie_port = TACHI_ENTRY_NGWIC1_PLX_PCIE_P; */
            tc_real_pcie_port = 0; 
#if 0
            printf("fix me tc_real_pcie_port=%d  func:%s \n", 
                    tc_real_pcie_port, __FUNCTION__ );
#endif 
        break;
        default:
            printf("%s:%d Invalid %s slot number (%d).",
                   __FUNCTION__, __LINE__, i2c_if->dev_name, slot);
            return (FAILED);
        break;
        }
    } else {
        printf("%s:%d Invalid interface type of TestCard (%#x).",
               __FUNCTION__, __LINE__, if_type);
        return (FAILED);
    }

    i2c_if->offset = 0;
    i2c_if->i2c_bus_type = IOFPGA_I2C;
#ifdef FOXCONN_FPGA
    i2c_if->sub_addr_len = 1; 
#else
    i2c_if->sub_addr_len = 0;  /* O2/USD */
#endif 
    i2c_if->size = sizeof(uint16_t);
    i2c_if->mux = I2C_MUX_ZERO;
    i2c_if->buf = NULL;
    i2c_if->i2c_dev = 0;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : testcard_init
 * Description:	To do TestCard related initialization.
 * Inputs     : ngwic - related info
 * Outputs    : None
 *
 *******************************************************************************
 */
static int testcard_init (ngio_if *ngio, int if_type, int slot)
{
    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
    sprintf(ngio->name, "NGWIC");
  
    ngio->uart_on(ngio);
#if 0  /* using early unreset flag on platform_slot.c */ 
    ngio->unreset(ngio);
#endif 

    /* Init TestCard I2C interface structure */
    init_tc_i2c_struct(&tc_i2c_if, if_type, slot);

    /* Set-up related TestCard interface info */
    testcard_if_p = &testcard_if;
    tc_ngio_p = ngio;

    testcard_if_p->type = if_type;
    testcard_if_p->type_name = ngio->name;
    testcard_if_p->slot = slot;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ngwic_testcard
 * Description:	Build menu for NGWIC TestCard diag tests.
 * Inputs     : ngwic - related info
 * Outputs    : None
 *
 *******************************************************************************
 */
uint32_t ngwic_testcard (void *ngwic)
{
    uint32_t rc = FAILED;
    int      slot = 0;

    tc_ngwic_iface = (struct ngio_intf_t *)ngwic;

    build_primary_submenu(wic_tc_table, WIC_TC_TABLE_SIZE,
			  "NGWIC TestCard Menu", &wic_tc_diagp);
    build_secondary_submenu(wic_tc_table, WIC_TC_TABLE_SIZE,
			    wic_tc_secondary_items);

    slot = tc_ngwic_iface->slot;

#if 0  /* FIX ME: wait for cetus  */
    /* Configure the NGIO GE ports for 1G or 10G-KR.
     */
    ngio_ge_cfg(tc_ngwic_iface);
    msleep(250);
#endif 

    if (check_intel_linux_ready()) {
        printf("Could not auto check INTEL linux ready.\n");
        return (FAILED);
    } else {
        /* intel and lewis is ready, prepare to */
        /* init vlan before loopback test */
        diag_lewis_gesw_set_nim_vlan_util(TRUE);
    }

    /* Do TestCard related initialization */
    rc = testcard_init(&tc_ngio, TC_NGWIC, slot);
    cterr_setup(slot); 

    if (rc == PASSED) {
        if (tc_ngwic_iface->menu_display == TRUE) {
            menu(&wic_tc_diag, wic_tc_secondary_items, 0);
        } else {
            assert(tc_ngwic_iface);
            if (tc_ngwic_iface->id != NIM_10GKR_TESTCARD) {
                rc = do_all_ngwic_testcard_tests();
            } else {
                rc = do_all_nim_10gkr_testcard_tests();
            }
        }
    } else {
        cterr('f', 0, "%s:%d Failed to init TestCard on NGWIC%d.",
              __FUNCTION__, __LINE__, slot);
    }

    /* clean up vlan before exit diag */
    diag_lewis_gesw_set_nim_vlan_util(FALSE);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : get_tc_i2c_struct
 * Description:	Function to get TestCard common I2C structure.
 * Inputs     : *des_i2c_if - Pointor to get the copy structure of TestCard I2C
 * Outputs    : None
 *
 *******************************************************************************
 */
void get_tc_i2c_struct (n2g_i2c_if_t *des_i2c_if)
{
    memcpy(des_i2c_if, &tc_i2c_if, sizeof(n2g_i2c_if_t));
}

/*******************************************************************************
 *
 * Function   : do_all_nim_10gkr_testcard_tests
 * Description: Do all NIM 10G-KR TestCard diag tests.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int do_all_nim_10gkr_testcard_tests (void)
{
    int dummy = 0; 

    if (tc_fpga_reg_test() != PASSED) {
        return (FAILED);
    } else if (tc_tlk10232_internal_loopback_test(dummy) != PASSED) {
        return (FAILED);
    } else if (diag_nc_nim_testcard_test() != PASSED) {
        return (FAILED);
    } else if (tc_uart_lpbk_test(TC_INT_LPBK) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : do_all_ngwic_testcard_tests
 * Description:	Do all NGWIC TestCard diag tests.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int do_all_ngwic_testcard_tests (void)
{

    if (tc_fpga_reg_test() != PASSED) {
        return (FAILED);
    } else if (tc_sgmii_1g_lpbk_test(TC_ETH_INT_LPBK) != PASSED) {
        return (FAILED);
#if 0  /* put pcie test on intel */
    } else if (tc_pcie_8prbs_lpbk_test() != PASSED) {
        return (FAILED);
#endif 
    } else if (tc_uart_lpbk_test(TC_INT_LPBK) != PASSED) {
        return (FAILED);
#if 0
    } else if (tc_sync_sig_test() != PASSED) {
        return (FAILED);
    } else if (tc_pll_locked_test() != PASSED) {
        return (FAILED);
#endif
    }

    return (PASSED);
}


/*
 **********************************************************************
 *
 *  Function: cterr_setup
 *
 *  Description: Setup 10G-KR testcard specific cterr parameters.
 *
 *  Input: slot
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void
cterr_setup (int slot)
{

    /* Setup the PID */
    memset(&testcard_10gkr_pid[0], 0, sizeof(testcard_10gkr_pid)); /* Set the null string */

    get_cookie_pid(slot, WIC_MODULE, tc_ngwic_iface->cookie, &testcard_10gkr_pid[0]); 

    tc_fru_table_offset = WIC0 + slot -1;
    return; 
}


/*------------------------------------------------------------------
$Log: ngio_testcard.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.12  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.11  2015/12/30 08:37:23  alpeng
 support nc test, check intel and lewis ready before testing

Revision 1.1.2.10  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.9  2015/12/01 02:04:35  alpeng
update nc infra structures and support testcard pcie test with nc

Revision 1.1.2.8  2015/11/23 11:02:20  alpeng
update error msg

Revision 1.1.2.7  2015/10/14 09:12:00  alpeng
support new testcard for p1b

Revision 1.1.2.6  2015/10/02 03:13:36  alpeng
not support sync signal and pll lock test yet

Revision 1.1.2.5  2015/09/18 06:58:54  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.4  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.3  2015/09/04 06:07:28  alpeng
fix testcatd i2c r/w; supporting uart test

Revision 1.1.2.2  2015/08/21 06:46:28  alpeng
support ge/xaui test for testcard; clean up repo;

Revision 1.1.2.1  2015/07/31 10:39:59  alpeng
first check in for testcard


$Endlog$
*/


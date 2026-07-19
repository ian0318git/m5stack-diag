/* $Id: ngio_testcard.c,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/ngio_testcard.c,v $
 *------------------------------------------------------------------
 * Filename   : ngio_testcard.c
 *
 * Description: Common function for TestCard.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
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
#include "nvmonvars.h"
#include "error.h"
#include "menu.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "ngio.h"
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "testcard_xaui.h"
#include "testcard_tlk_10232.h"
#include "testcard_plx_pcie_sw.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "proto.h"  /* for msleep) */
#include "dash_fpga.h" /* is_platform() */
#include "cross_platform.h" /* SM_DAUGHTER_CARD */
#include "diag_eth_pkt_txrx_utils.h"
#include "diag_fpga_i2c_lib.h"
#include "dnv_eth_lib.h"
#include "diag_common.h"

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
static int testcard_init(ngio_if *, int, int);
static int init_tc_pci_i2c_struct(n2g_i2c_if_t *, int, int);
static int do_all_10gkr_testcard_tests(int);
int for_10gkr_testcard(int);
static void is_testcard_10gkr(unsigned int, unsigned int);

#define OVLD_NGWIC1_PCIE_P        8


/******************************************************************************* 
 *                                  Externs                                    *
 *******************************************************************************
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
extern int tc_minus_54v_detect_test(void);
extern int tc_sgmii_1g_lpbk_test(int);
extern int tc_xaui_lpbk_test(int);
extern int tc_pcie_8prbs_lpbk_test(void);
extern int tc_uart_lpbk_test(int);
extern int tc_sync_sig_test(void);
extern int tc_pll_locked_test(void);
extern void remove_pcie_device(void);

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static ngio_if        tc_ngio;
static n2g_i2c_if_t   tc_i2c_if;
static testcard_if_t  testcard_if;
struct ngio_intf_t *tc_ngsm_iface = NULL;
struct ngio_intf_t *tc_ngwic_iface = NULL;

ngio_if        *tc_ngio_p;
testcard_if_t  *testcard_if_p;
uint32_t       tc_real_pcie_port = 0;

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */ 


/*
 * WIC Testcard Menu
 */
static submenu_xtable_t wic_tc_table[] = {
    {"FPGA tests",          
     (PFT)build_tc_fpga_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, 
     (PFT)build_tc_fpga_menu, TRUE},
    {"TLK10232 loopback tests",           
     (PFT)build_tc_tlk10232_menu,        FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)build_tc_tlk10232_menu,  TRUE},
    {"XAUI tests",   
     (PFT)build_tc_xaui_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)build_tc_xaui_menu, TRUE},
    {"PLX PCIe sw tests",
     (PFT)build_tc_plx_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)build_tc_plx_menu,  TRUE},
    {"UART tests",
     (PFT)build_tc_uart_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, 
     (PFT)build_tc_uart_menu, TRUE},
    {"Signal tests",
     (PFT)build_tc_sig_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0,
     (PFT)build_tc_sig_menu,  TRUE},
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
    (PFT)menu_show_dflags,	/* show major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    wic_tc_primary_items,
};
static struct menuinfo *wic_tc_diagp = &wic_tc_diag;


/*******************************************************************************
 *
 * Function   : is_testcard_10gkr
 * Description: use ngio id to distingusih 10G-KR TC or legacy TC.  
 * Inputs     : card_id - testcard id , mod_type - module type
 * Outputs    : None
 *
 *******************************************************************************
 */
void is_testcard_10gkr (unsigned int card_id, unsigned int mod_type)
{
    if (card_id == NIM_10GKR_TESTCARD) {
        /* SM ptr is not init, it is a NIM */ 
        printf("\n10G-KR NIM testcard on NIM slot.  \n"); 
        testcard_if_p->is_10gkr = TC_10GKR_NIM_ON_NIM; /* 0x01 */
    } else {
        printf("\nLegacy NIM or SM testcard (no 10G-KR support).  \n"); 
        testcard_if_p->is_10gkr = TC_LEGACY_NIM_OR_SM; /* 0x0 */
    }
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
    if (testcard_if_p->is_10gkr) { 
        if (inverse) {
            return (TRUE);
        } else {
            return (FALSE);
        }   
    } else { /* non-10GKR tescard case */
        if (inverse) {
            return (FALSE);
        } else {
            return (TRUE);
        }
    }
}

/*******************************************************************************
 *
 * Function   : init_tc_pci_i2c_struct
 * Description: To init Test Card I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : None
 *
 *******************************************************************************
 */
static int init_tc_pci_i2c_struct (n2g_i2c_if_t *i2c_if, int if_type, int slot)
{
    i2c_if->dev_name = "NGWIC TestCard";
    i2c_if->i2c_ctrl = get_wic_i2c_ctrl(slot); 

    tc_real_pcie_port = OVLD_NGWIC1_PCIE_P;

    i2c_if->offset = 0;
    i2c_if->i2c_bus_type = IOFPGA_I2C;
    i2c_if->sub_addr_len = 0;  /* dont' use sub address slave register */
    i2c_if->size = sizeof(uint16_t);
    i2c_if->mux = I2C_MUX_ZERO;
    i2c_if->buf = NULL;
    i2c_if->i2c_dev = 0;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : testcard_init
 * Description:	Do TestCard related initialization.
 * Inputs     : ngwic - related info
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
static int testcard_init (ngio_if *ngio, int if_type, int slot)
{
    /* Power up related NGSM/NGWIC port */
    ngio = (struct ngio_intf_t *)slot_get_ngiowic(slot);
    sprintf(ngio->name, "NGWIC");

    ngio->uart_on(ngio);

    /* Init TestCard PCI & I2C interface structure */
    init_tc_pci_i2c_struct(&tc_i2c_if, if_type, slot);

    /* Set-up related TestCard interface info */
    tc_ngio_p = ngio;

    testcard_if_p->type = if_type;
    testcard_if_p->type_name = ngio->name;
    testcard_if_p->slot = slot;
    testcard_if_p->uart_ctrl = ngio->uart_ctrl; 

    ntc_gesw_p0_type = 0; 
    ntc_gesw_p1_type = 0; 


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
    int slot = 0;

    system(TESTCARD_ETH_UP);

    tc_ngwic_iface = (struct ngio_intf_t *)ngwic;

    build_primary_submenu(wic_tc_table, WIC_TC_TABLE_SIZE,
			  "NGWIC TestCard Menu", &wic_tc_diagp);
    build_secondary_submenu(wic_tc_table, WIC_TC_TABLE_SIZE,
			    wic_tc_secondary_items);

    slot = tc_ngwic_iface->slot;
    testcard_if_p = &testcard_if;
    is_testcard_10gkr(tc_ngwic_iface->id, tc_ngwic_iface->mod_type); 

    /* Configure the NGIO GE ports for 1G or 10G-KR.
     */
    ngio_ge_cfg(tc_ngwic_iface); 

    msleep(SLEEP_250);

    rc = testcard_init(&tc_ngio, TC_NGWIC, slot);

    if (rc == PASSED) {
        if (tc_ngwic_iface->menu_display == TRUE) {
            menu(&wic_tc_diag, wic_tc_secondary_items, 0);
        } else {
            assert(tc_ngwic_iface);
            rc = do_all_10gkr_testcard_tests(slot);
        }
    } else {
        cterr('f', 0, "%s:%d Failed to init TestCard on NGWIC%d.",
              __FUNCTION__, __LINE__, slot);
    }

    system(TESTCARD_ETH_DOWN);

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
 * Function   : do_all_10gkr_testcard_tests
 * Description: Do all NIM/SM 10G-KR TestCard diag tests.
 * Inputs     : slot
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int do_all_10gkr_testcard_tests (int slot)
{
    int dummy = 0; 

    if (tc_fpga_reg_test() != PASSED) {
        return (FAILED);
    } else if (tc_minus_54v_detect_test() != PASSED) {
        return (FAILED);
    } else if (tc_tlk10232_internal_loopback_test(dummy) != PASSED) {
        return (FAILED);
    } else if (new_tc_xaui_lpbk_test() != PASSED) {
        return (FAILED);
    } else if (testcard_pcie_linkup_test_wrapper(slot) != PASSED) {
        return (FAILED);
    } else if (tc_uart_lpbk_test(TC_INT_LPBK) != PASSED) {
        return (FAILED);
    } else if (tc_sync_sig_test() != PASSED) {
        return (FAILED);
    } 

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: eth_is_linkup
 *
 * Description : Check eth port link up status from Linux information.
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
boolean eth_is_linkup (int port)
{
    char file_name[32];
    FILE *stream_p;
    char linkstate[] ="down";

    sprintf(file_name, "/sys/class/net/%s/operstate", TABEI_ETH_BP);

    stream_p = fopen(file_name, "r");
    if (stream_p == NULL) {
        cterr('f', 0," The file `/sys/class/net/%s/operstate' can't be opened.\n ", TABEI_ETH_BP);
    } else {
        fscanf(stream_p, "%s", linkstate);
        fclose(stream_p);
    }

    if (strcmp(linkstate, "up") == 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s link is %s\n", TABEI_ETH_BP, linkstate);
        }
        return (TRUE);
    }
    else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
             printf("%s link is %s\n", TABEI_ETH_BP, linkstate);
        }
        return (FALSE);
    }
}


/***************************************************************************
 *
 * Function   : ntc_chk_eth_linkup
 * Description:	check host eth port link status 
 * Inputs     : host eth port number 
 * Outputs    : TRUE/FALSE
 *
 ***************************************************************************
 */
int ntc_chk_eth_linkup (int host_eth_port)
{  
    int cnt = NTC_RETRY_CNTER;  
    int delay = NTC_DELAY_TIME;  

    while (cnt) {
       if (eth_is_linkup(host_eth_port)) {
           return (TRUE); /* link up */
       } else {
           cnt--;
           msleep(delay);
       }
    }
    return (FALSE);
}

/*------------------------------------------------------------------
$Log: ngio_testcard.c,v $
Revision 1.2  2019/10/17 02:16:25  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.11  2019/08/05 08:00:44  olin2
Clean up code

Revision 1.1.2.10  2019/07/31 08:00:48  olin2
Clean up code

Revision 1.1.2.9  2019/07/30 08:44:31  kodko
Clean up code based on code review

Revision 1.1.2.8  2019/06/04 03:04:08  olin2
Add -54v detect test to default test

Revision 1.1.2.7  2019/03/14 08:18:20  kehuang2
Support Tabei-L Promethium sku

Revision 1.1.2.6  2019/03/13 09:14:14  olin2
Update eth setting

Revision 1.1.2.5  2018/11/06 07:02:11  olin2
Update eth port

Revision 1.1.2.4  2018/11/02 10:08:35  olin2
Support testcard xaui test

Revision 1.1.2.3  2018/10/23 11:34:26  olin2
Support Testcard test

Revision 1.1.2.2  2018/10/18 08:32:33  olin2
Support NIM testcard FPGA reg test

Revision 1.1.2.1  2018/10/09 09:22:05  olin2
Initial commit for NIM test

Revision 1.19.2.2  2018/08/28 16:31:18  alpeng
stablize testcard loobpack test on curie

Revision 1.19.2.1  2018/08/09 10:18:31  alpeng
support NTC on Curie NIM1

Revision 1.19  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.18  2016/10/16 12:28:17  iachang
Supported Goldbeach Platform.

Revision 1.17.20.16  2018/05/04 07:03:58  alpeng
show testcard type and status before menu pump for user

Revision 1.17.20.15  2018/02/02 09:44:16  alpeng
add main flags on SM TC menu; consider x2 case for SM TC

Revision 1.17.20.14  2018/01/16 06:46:31  alpeng
first check in for 10G-KR SM testcard; we need to apply correct id once hw ready for it

Revision 1.17.20.13  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.17.20.12  2017/08/11 03:46:57  leschen
Support Neptune SM4 test. No Sync signals, no GE signals and PCIe from BDW instead of PCIe switch.

Revision 1.17.20.11  2017/07/21 10:02:56  alpeng
fixed typo

Revision 1.17.20.10  2017/07/21 09:18:57  alpeng
add pci_rdy and downstream port reset for ntpn

Revision 1.17.20.9  2017/04/18 01:32:28  alpeng
init ptype as 0 for GB

Revision 1.17.20.8  2017/04/17 10:13:24  alpeng
add gesw ptype check

Revision 1.17.20.7  2017/04/06 02:09:45  leschen
Fix testcard xaui issue.

Revision 1.17.20.6  2017/04/05 06:41:58  leschen
Sync with <ng_diag-tag-032917>

Revision 1.17.20.5  2017/01/03 08:09:15  alpeng
add uart test into new tc

Revision 1.17.20.4  2016/12/23 03:42:24  alpeng
update ext. device reset for nep; clean up msg for NTC

Revision 1.17.20.3  2016/12/15 03:42:04  alpeng
 fix old testcard uart issue, using api to get uart_ctrl num

Revision 1.17.20.2  2016/11/18 06:44:30  alpeng
update menuinfo for testcard

Revision 1.17.20.1  2016/10/20 22:15:46  alpeng
update thule get i2c bus num for carrier card; update tc get i2c bus mechanism; dash_fpga for bypass plx conflict

Revision 1.18  2016/10/16 12:28:17  iachang
Supported Goldbeach Platform.

Revision 1.17  2015/03/20 10:32:29  danchung
add function to recover the new nim test card pcie linkup when the linkup
is down

Revision 1.16  2015/01/14 02:42:51  alpeng
using early unreset to resolve testcard link training error issue

Revision 1.15  2014/09/06 00:53:21  ptong
Remove ngio_ge_cfg from slot.c and add to ngio_testcard.c and sm_woodlawn.c

Revision 1.14  2014/08/04 07:36:30  alpeng
remove useless function

Revision 1.13  2014/07/29 09:22:18  danchung
avoid segmentation falut when running interface test on Overlord

Revision 1.12  2014/07/25 01:36:57  alpeng
support xaui loopback and sort out the test item for new testcard

Revision 1.11  2014/07/22 09:40:13  alpeng
support 10g-kr on ge0 and 1g-kx on ge1 for new testcard

Revision 1.10  2014/07/08 08:09:18  danchung
Fix sm testcard xaui test failure

Revision 1.9  2014/07/02 08:09:43  alpeng
add new testcard id for en/disable menu item and select smi addr

Revision 1.8  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.7  2014/06/06 08:14:52  alpeng
xaui test could be leveraged for new testcard

Revision 1.6  2014/06/06 07:04:45  alpeng
put plx and tlk10232 test into menu

Revision 1.5  2013/11/19 13:31:03  danchung
Add pcie port mapping for Sword NGSM slot 1

Revision 1.4  2013/11/18 07:27:43  alpeng
support get_pci_bus_num()

Revision 1.3  2013/11/01 13:22:16  danchung
Fix testcard failure on Utah.

Revision 1.2  2013/10/22 14:32:34  danchung
Add support for PLX PCIe switch

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.11  2013/01/15 01:45:47  ptong
Remove a debug statement

Revision 1.10  2012/11/21 19:50:08  palin2
1. Add TestCard PLL Lock test support.
2. Update FPGA registers map to v1.05 with backward compatible.

Revision 1.9  2012/11/14 19:15:48  palin2
To support Sync Signals loopback test on TestCards.

Revision 1.8  2012/10/26 16:14:58  palin2
Temporarily Skip PCIe loopback test on NGWIC slot3 for further debug.

Revision 1.7  2012/10/25 18:43:32  mcharon
make sure enough time elapse before accessing fpga register

Revision 1.6  2012/10/10 16:57:01  palin2
Fixed NGWIC TestCard PCIe loopback test based on IDT FAE's suggestion.

Revision 1.5  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

Revision 1.4  2012/09/12 10:26:08  palin2
Add NGWIC TestCard support from Host side(Overlord) DiagMenu.

Revision 1.3  2012/08/22 16:39:55  palin2
Put XAUI into Reset state when exits XAUI related tests to avoid
affecting other interface.

Revision 1.2  2012/08/20 13:22:58  palin2
Add NGSM TestCard support from Host side(Overlord) DiagMenu.

Revision 1.1  2012/08/14 11:30:55  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.5  2012/08/08 22:19:41  palin2
1. Move TestCard UART external loopback test to "testcard_uart.c".
2. Add support TestCard UART internal loopback test and related utilities.

Revision 1.4  2012/07/31 17:08:20  palin2
Initial check-in for TestCard PCIe tests.

Revision 1.3  2012/07/30 15:47:09  palin2
Add support TestCard UART loopback test.

Revision 1.2  2012/07/25 06:39:42  palin2
Help Danny(danchung) to check-in his changes for TestCard UART loopback test.

Revision 1.1  2012/07/23 17:33:54  palin2
Initial check-in for Overlord Test Card diag tests.


$Endlog$
*/


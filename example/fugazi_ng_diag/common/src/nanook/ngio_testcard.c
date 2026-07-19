/* $Id: ngio_testcard.c,v 1.2 2019/12/11 10:10:33 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/ngio_testcard.c,v $
 *------------------------------------------------------------------
 * Filename   : ngio_testcard.c
 *
 * Description: Common function for TestCard.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "testcard_eth.h"
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
#include "nanook_comm.h"


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
    {"Ethernet tests",           
     (PFT)build_tc_eth_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, FALSE,
     (PFT)build_tc_eth_menu,  TRUE},
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
    {"PCIe tests",
     (PFT)build_tc_pcie_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, FALSE,
     (PFT)build_tc_pcie_menu, TRUE},
    {"UART tests",
     (PFT)build_tc_uart_menu,            FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, 
     (PFT)build_tc_uart_menu, TRUE},
    {"Signal tests",
     (PFT)build_tc_sig_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)0, 0,
     (PFT)build_tc_sig_menu,  TRUE},
    {"PLL Lock tests",
     (PFT)build_tc_pll_menu,             FALSE,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, FALSE,
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
 * Description:	To do TestCard related initialization.
 * Inputs     : ngwic - related info
 * Outputs    : None
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

    /* distinguish testcard is 10gkr or legacy
     * it will also fille testcard_if_p->is_10gkr
     * which is used for "for_10gkr_testcard()" */
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
    char cmd[256];

    rc = enable_ether_interface(ETHER_INTERFACE_NIM);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to insmod ixgbe.");
        return (FAILED);
    }
    msleep(SLEEP_1000);
    
    sprintf(cmd, "ifconfig %s up > /dev/null", inface_lan0p0);
    system(cmd);

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
    /* TBD ngio_ge_cfg????????? */
    ngio_ge_cfg(tc_ngwic_iface); 

    msleep(250);

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

    sprintf(cmd, "ifconfig %s down > /dev/null", inface_lan0p0);
    system(cmd);

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
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int do_all_10gkr_testcard_tests (int slot)
{
    int dummy = 0; 

    if (tc_fpga_reg_test() != PASSED) {
        return (FAILED);
    } else if (tc_tlk10232_internal_loopback_test(dummy) != PASSED) {
        return (FAILED);
    } else if (new_tc_xaui_lpbk_test() != PASSED) {
        return (FAILED);
    } else if (testcard_pcie_linkup_test_wrapper(slot) != PASSED) {
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
    char file_name[] = "/sys/class/net/enp36s0f0/operstate";
    FILE *stream_p;
    char linkstate[] ="down";

    sprintf(file_name, "/sys/class/net/%s/operstate", inface_lan0p0);

    stream_p = fopen(file_name, "r");
    if (stream_p == NULL) {
        cterr('f', 0," The file `/sys/class/net/%s/operstate' can't be opened.\n ", inface_lan0p0);
    } else {
        fscanf(stream_p, "%s", linkstate);
        fclose(stream_p);
    }

    if (strcmp(linkstate, "up") == 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s link is %s\n", inface_lan0p0, linkstate);
        }
        return (TRUE);
    }
    else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
             printf("%s link is %s\n", inface_lan0p0, linkstate);
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
Revision 1.2  2019/12/11 10:10:33  lucywang
Merged Nanook to main trunk


$Endlog$
*/


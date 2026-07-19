/* $Id: ppb_test.c,v 1.8 2012/09/10 06:46:04 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/diag_menu/ppb_test.c,v $
 *------------------------------------------------------------------
 * ppb_test.c
 *      Graffham menu 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c)2012by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "menu.h"
#include "diag_ppb.h"
#include "common.h"
#include "error.h"
#include "debug_console.h"
#include "uart.h"

static int eth_int_lpbk(int);
static int eth_ext_lpbk(int);
static int dss_core0_sanity(int);
static int dss_core1_sanity(int);
static int dss_core2_sanity(int);
static int dss_core3_sanity(int);

extern uint32_t test_mem(void);
extern int ecc_mem_test(void);
extern uint32_t ppb_con(void);
extern uint32_t ethernet_test(uint32_t, int, int);
extern uint32_t eth_reg_disp(void);
extern void reload(void);
extern int tdm_lpbk(void);
extern int dss_core_sanity(int);
extern int tdm_int_lpbk(int);
extern int tdm_ext_lpbk(int);
extern int arm11_cpu1_boot_test(void);

static struct menuinfo *maindiagp;
static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    0,
    0,
};
struct menuinfo *utilmenup = &utilmenu;

static int zero  = 0;
static int one  = 1;

static struct mitem graffham_mainmenu[] = {
    {adiagfstr,                   0, 0, (PFI)menu, 
                                             (type_t *)&menu_diagflagp, 0,0, 0},
    {basutilstr,  0, 0, (PFI)menu, (int *)&utilmenup,                 0,  0, 0},
    {doalldgstr,                  0, 0, (PFI)do_menu_all_diags,
                (type_t *)&maindiagp, MF_CONTINUOUS, 0, 0}, {dogrpdgstr,       
                                                   0, 0, (PFI)do_menu_grp_diags, 
                                        (int *)&maindiagp, MF_CONTINUOUS, 0, 0},
    {"PPB Console Utility", 0,0, (PFT)ppb_con,                         0,0,0,0},
    {"DDR3 Memory test", 0,0, (PFT)test_mem, 0,MF_CONTINUOUS | MF_DOALL | 
                                                          MF_SHOW_ERRCOUNT,0,0},
    {"EMAC0 Internal loopback test", 0,0, (PFT)eth_int_lpbk, (type_t *)&zero,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"EMAC1 Internal loopback test", 0,0, (PFT)eth_int_lpbk, (type_t *)&one,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"EMAC0 External loopback test", 0,0, (PFT)eth_ext_lpbk, (type_t *)&zero,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"EMAC1 External loopback test", 0,0, (PFT)eth_ext_lpbk, (type_t *)&one,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"ARM11 CPU1 Boot test ", 0,0, (PFT)arm11_cpu1_boot_test, 0,MF_CONTINUOUS |MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core0 Sanity test ", 0,0, (PFT)dss_core0_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core1 Sanity test ", 0,0, (PFT)dss_core1_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core2 Sanity test ", 0,0, (PFT)dss_core2_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"DSS Core3 Sanity test ", 0,0, (PFT)dss_core3_sanity , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"TDM External Loopback test ", 0,0, (PFT)tdm_ext_lpbk , 0,MF_CONTINUOUS | 
                                               MF_DOALL | MF_SHOW_ERRCOUNT,0,0},
    {"ECC Memory test ", 0,0, (PFT)ecc_mem_test , 0,MF_CONTINUOUS | MF_DOALL | 
                                                          MF_SHOW_ERRCOUNT,0,0},
};

static struct menuinfo  graffhammainmenu = {
    "LSI SP2704 Submenu",                             /* title */
    (int)0,                                           /* title param */
    (PFI)menu_show_dflags,                            /* show diag flags */
    0,                                                /* generic prompt */
    sizeof(graffham_mainmenu)/sizeof(struct mitem),   /* size of menu */
    graffham_mainmenu,
};

int graff_main_test (void)
{
    int  retval = PASSED; 

    if (retval == PASSED) {
        retval = test_mem();
    }
    if (retval == PASSED) {
        retval = eth_int_lpbk(0);
    }
    if (retval == PASSED) {
        retval = eth_int_lpbk(1);
    }
    if (retval == PASSED) {
        retval = eth_ext_lpbk(0);
    }
    if (retval == PASSED) {
        retval = eth_ext_lpbk(1);
    }
    if (retval == PASSED) {
        retval = dss_core0_sanity(DSS_CORE0);
    }
    if (retval == PASSED) {
        retval = dss_core1_sanity(DSS_CORE1);
    }
    if (retval == PASSED) {
        retval = dss_core2_sanity(DSS_CORE2);
    }
    if (retval == PASSED) {
        retval = dss_core3_sanity(DSS_CORE3);
    }
    if (retval == PASSED) {
        retval = tdm_ext_lpbk(EXTERNAL);
    }
    if (retval == PASSED) {
        retval = ecc_mem_test();
    }
    return (retval);
}

void diag_menu (void)
{
#ifdef MENU_DEBUG
    uart_puts("\r\nIn diag_menu\n");
#endif

    maindiagp = &graffhammainmenu;

    if (menu_display == 1)
        menu(&graffhammainmenu, 0, 0);
    else {
        graff_main_test();
        prcomplete(testpass, errcount, 0);
        uart_puts("\r\n Enter <ctrl-a> <ctrl-x> to return to Host menu");
    }
}

static int eth_ext_lpbk (int port)
{
    uart_puts("\r\n Make sure loopback is enabled on host\n");
    uart_puts("\r\n ETH %d external loopback \n");
    return (ethernet_test(0, EXTERNAL, port));
}

static int eth_int_lpbk (int port)
{
    return (ethernet_test(0, INTERNAL, port));
}

int dss_core0_sanity (int core)
{
    return(dss_core_sanity(DSS_CORE0));
}

int dss_core1_sanity (int core)
{
    return(dss_core_sanity(DSS_CORE1));
}

int dss_core2_sanity (int core)
{
    return(dss_core_sanity(DSS_CORE2));
}

int dss_core3_sanity (int core)
{
    return(dss_core_sanity(DSS_CORE3));
}

/******** History ********
$Log: ppb_test.c,v $
Revision 1.8  2012/09/10 06:46:04  srane
Add ARM11 CPU1 test to dsp menu.

Revision 1.7  2012/08/15 15:03:00  srane
Add support for EMAC1 loopback test.

Revision 1.6  2012/07/17 20:34:43  srane
cleanup

Revision 1.5  2012/06/28 21:31:37  srane
add support routines for menu display.

Revision 1.4  2012/06/07 22:51:10  srane
TDM external loopback, ECC memory test

Revision 1.3  2012/05/24 23:25:47  srane
Add GPIO code to set ready bit, uart test, support both
uart mode and ethernet mode, other cleanup

Revision 1.2  2012/05/10 22:58:10  srane
Add TDM support.

Revision 1.1  2012/04/18 09:44:12  srane
Initial checkin


$Endlog$
*/


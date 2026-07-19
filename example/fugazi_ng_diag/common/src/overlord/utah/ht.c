/* $Id: ht.c,v 1.23 2018/08/30 06:59:43 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/ht.c,v $
 *------------------------------------------------------------------
 * ht.c - Ht main routine/menu.
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 * Original Author : mcharon
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* for sleep */
#include "common.h"
#include "types.h"
#include "menu.h"
#include "queryflags.h"
#include "error.h"
#include "nvmonvars.h"
#include "linux_api.h" /* printhexdump */
#include "platform_eth_pkt_txrx.h"  /* host_packet_send */
#include "ht.h"
#include "platform_fru.h"
#include "cli_cmd.h"  /* show maringin */
#include "proto.h" /*msleep*/
#include "plat_defs.h"
#include "dash_fpga.h"

static int hts_send_pkt(int);
static int hts_sweep_send_pkt(int dir);
static int hts_load(char *, char *);
static int hts_menu_load(int);
static int menu_max_pkt_len, menu_pkt_len;
static int shutdown(int);
extern int invoke_bcm_shell(void);

#define LOGFILE "xauilog.txt"
/*
  -d : destination portid. should be 2
  -m : mtu for ht interface, mtu for eth interface
  -a : ip address of ht, and ip address of etqh interface
       if empty, used system default
  -n : ngio_destination
  -o : means running test in local network. script and program runs from 'option' directory
  -p : prio
  -v : verbose mode (v1 to print out echo statements in ifht script.
*/
#ifdef LOCAL_DIR
#define KLM_SCRIPT_OPTION "-omcharon -d2 -m10240,9216 -a14.6.48.21,14.6.48.20 -n0 -p0"
#define DIR_PATH  "/linux_diag/mcharon/"
#else
/* klem/production use default eth3 ip, so don't need option -a */
#define KLM_SCRIPT_OPTION "-d2 -m10240,9216 -n0 -p0"
#define DIR_PATH  ""
#endif

static char ctrl_ifname[10];

#define FLAGS MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT | MF_DOGRP
/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
/* menu for 'double character' entry */
static struct mitem  subdiag[] = {
    {adiagfstr,           0, 0, (PFT)menu,       
     (type_t *)&menu_diagflagp, 0,(type_t(*)())0, 0},
    {"basic utilities",   0, 0, (PFT)menu,       
     (type_t *)&utilmenup, 0,(type_t(*)())0, 0},
    {doalldgstr,          0, 0, (PFT)do_menu_all_diags, 
     (type_t *)&maindiagp_hts, FLAGS,(type_t(*)())0, 0},
    {dogrpdgstr,          0, 0, (PFT)do_menu_grp_diags, 
     (type_t *)&maindiagp_hts, FLAGS,(type_t(*)())0, 0},
    {"Shutdonw ht interface",    0,0, (PFT)0,
     (type_t *)0, 0,(type_t(*)())0, 0},
    {"Invoke BCM Shell",    0,0, (PFT)0,
     (type_t *)0, 0,(type_t(*)())0, 0},
    {"Load Module (skip reload if already loaded)",    0,0, (PFT)0,
     (type_t *)0, 0,(type_t(*)())0, 0},
    {"Load Module (force reload)",    0,0, (PFT)0,
     (type_t *)0, 0,(type_t(*)())0, 0},
    {"Send pkt (ht to eth3)",    0,0, (PFT)0,
     (type_t *)0, FLAGS | MF_DOGRP,(type_t(*)())0, 0},
    {"Send pkt (eth3 to ht)",    0,0, (PFT)0,
     (type_t *)0, FLAGS | MF_DOGRP,(type_t(*)())0, 0},
    {"Sweep Send pkt (ht to eth3)",    0,0, (PFT)0,
     (type_t *)0, FLAGS | MF_DOGRP,(type_t(*)())0, 0},
    {"Sweep Send pkt (eth3 to ht)",    0,0, (PFT)0,
     (type_t *)0, FLAGS | MF_DOGRP,(type_t(*)())0, 0},
};

static struct mitem  maindiag[] = {
    {adiagfstr,           0, 0, (PFT)menu,       
     (type_t *)&menu_diagflagp, 0,(type_t(*)())0, 0},
    {"basic utilities",   0, 0, (PFT)menu,       
     (type_t *)&utilmenup, 0,(type_t(*)())0, 0},
    {doalldgstr,          0, 0, (PFT)do_menu_all_diags, 
     (type_t *)&maindiagp_hts, MF_CONTINUOUS,(type_t(*)())0, 0},
    {dogrpdgstr,          0, 0, (PFT)do_menu_grp_diags, 
     (type_t *)&maindiagp_hts, MF_CONTINUOUS,(type_t(*)())0, 0},
    {"Shutdown ht interface",    0,0, (PFT)shutdown,
     (type_t *)&zero, 0,(type_t(*)())0,0},
    {"Invoke BCM shell",    0,0, (PFT)invoke_bcm_shell,
     (type_t *)&zero, 0,(type_t(*)())0,0},
    {"Load Module (skip reload if already loaded)",    0,0, (PFT)hts_menu_load,
     (type_t *)&zero, 0,(type_t(*)())0,0},
    {"Load Module (force reload)",    0,0, (PFT)hts_menu_load,
     (type_t *)&one, 0,(type_t(*)())0,0},
    {"Send pkt (ht to eth3)",    0,0, (PFT)hts_send_pkt,
     (type_t *)&zero, FLAGS | MF_DOGRP,(type_t(*)())0,0},
    {"Send pkt (eth3 to ht)",    0,0, (PFT)hts_send_pkt,
     (type_t *)&one, FLAGS | MF_DOGRP,(type_t(*)())0,0},
    {"Sweep send pkt (ht to eth3)",    0,0, (PFT)hts_sweep_send_pkt,
     (type_t *)&zero, FLAGS | MF_DOGRP,(type_t(*)())0,0},
    {"Sweep send pkt (eth3 to ht)",    0,0, (PFT)hts_sweep_send_pkt,
     (type_t *)&one, FLAGS | MF_DOGRP,(type_t(*)())0,0},
    
};

static struct menuinfo htsmaindiag = {
    "HT Submenu",                        /* title */
    (int)0 ,                                     /* title param */
    (PFT)menu_show_dflags,                       /* show diag flags */
    0,                                           /* generic prompt */
    sizeof( maindiag)/sizeof(struct mitem),/* size of menu */
     maindiag,
};
struct menuinfo *maindiagp_hts = &htsmaindiag;

static void
display_reg (void)
{
    cterr_db_print("at linux prompt, type 'cat /proc/ht/regs' to dump out "
                   "XAUI registers.");
    return;                        
}

void
display_env (void)
{
    show_margins_x(0, CLI_MODE);
}

static void
add_err_report (void)
{
 
    fru_table_offset = MB_XAUI;
    
    platform_fru_table[MB_XAUI].pid_string = mb_pid;
    platform_fru_table[MB_XAUI].location_string = mb_xaui_loc;

    cterr_add_component("i211", "Brodcom switch", "XAUI FPGA");
    cterr_add_reg_dump((PFV)display_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Run MB->BCM GESW test to make sure switch is working");
    
}

/********************************************************************
 *
 * Function:  hts_tests
 *
 * Description: main entry. test multple packets. because we are sending raw packet, kernel will not
 * fragment the packet, so the packet size is limited by mtu. ETH_NAME supports up to size of 9216, so
 * the larget packet size software can produce is 9214. hardware will add 4 bytes of crc so a total
 * of max 9216 can be sent out.
 * xaui fpga supports 2K buffer each descriptor. if raw packet is less then 2K, single descriptor transfer
 * is performed. if raw packet is greater than 2K, multiple descriptor transfer is performed.
 * Inputs: iface - Pointer to interface data structure.
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *

 ********************************************************************
 */
int 
hts_tests (int show)
{
    int status = PASS;
    int ix;
    char ifdest[10];
    char ifsrc[10];

    int pkt_len[] = {64, 1033, 2001, 3000, 4000, 5051, 7903, 8013, 9212, 0};
    if (is_goldbeach() || is_vg400()) {
        printf("\nGoldbeach didn't have XAUI interface\n");
        return (PASS);
    }

    maindiagp_hts = &htsmaindiag;

    sprintf(ctrl_ifname, "eth3");

    /* need to force reload. this is one way to flush out
       any arp packets that maybe caused by some other tests */
    hts_menu_load(1);
    
    if (show)
        menu(&htsmaindiag,  subdiag, '\0');
    else {

        if (get_enhance_err_flag()) {
            add_err_report();
        }
        
        testname("xaui");

        prpass(testpass, "xaui");
        
        /* when module is first loaded, driver can wait up to 700msecs
           for clock sync,
           so before we can send packet, we'll need to wait. if we dont'
           wait long enough, first packet may not get through. */
        msleep(600);
        if (testpass < 2)
            msleep(1000);
        for (ix = 0; pkt_len[ix]; ix++) {
            sprintf(ifdest, ctrl_ifname);
            sprintf(ifsrc, "ht");
            if ((status = test_txrx("xaui to eth pkt len = %d ", ifdest, ifsrc,
                                    pkt_len[ix], pkt_len[ix], LOGFILE))==FAIL) {
                cterr('f', 0, "xaui to eth pkt len = %d", pkt_len[ix]);
                break;
            }
            /* reverse direction */
            sprintf(ifdest, "ht");
            sprintf(ifsrc, ctrl_ifname);
            if ((status = test_txrx("eth to xaui pkt len %d ", ifdest, ifsrc, 
                                    pkt_len[ix], pkt_len[ix], LOGFILE))==FAIL) {
                cterr('f', 0, "fail: eth to xaui pkt len = %d", pkt_len[ix]);
                break;
                
            }
        }
    }
    /* NOTE: must shut down interface, or modules may have problem
       downloading.
       because arp packets maybe going to this interface instead of
       eth3 interface used for module firmware download */
    shutdown(0);
    prcomplete(testpass, errcount, (char *)0);
    return(status);

}

/********************************************************************
 *
 * Function:  hts_load
 *
 * Description: call script to load our module. we can't load our module until
 * broadcom switch is up which doens't happen until we lauch the program, so loading module
 * needs to be done inside the program.
 * the script can be called outside the proram as well. then this function can be called to
 * load module only if module has not been loaded.  the script allows user to load module
 * with certain features, allowing more flexibility for testing.

 * Inputs: force_optin : string: "-fht" to force loading of ht module,
 *                               "-fhtm" to force loading of htm module
 *                       script:  name of script to run. this script normally runs from
 *                                /overlord/bin directory.
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *

 ********************************************************************
 */
static int
hts_load (char *force_option, char *script)
{
    char *buf = (char *)malloc(sizeof(KLM_SCRIPT_OPTION) + 60);
    
    if (!buf) {
        printf("ht: not enough memory to create buffer need to load script");
        return(FAIL);
    }

    memset((void *)buf, 0, sizeof(KLM_SCRIPT_OPTION) + 60);
    /* ie, ifht  -d2 -m10240,9216 -a14.6.48.18,14.6.48.19 -n0 -p0 -v0 -iht,eth3 
       option '-i' is required by script.
       now ht = 14.6.48.18 with mtu = 10240 and
       eth3   = 14.6.48.19 with mtu = 9216
       force_option = ""
     */

    /* script will check wheather or not to load the module. */
    /* if we want ifht script to load module form mcharon-lnx:/linux_diag, use -omcharon option.
       this will load ht.ko driver located at mcharon-lnx:/mcharon.
       ie, sprintf(buf, "%s %s -iht,%s %s -omcharon", script, KLM_SCRIPT_OPTION,
       ctrl_ifname, force_option);
    */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        sprintf(buf, "%s %s -iht,%s %s -v1", script, KLM_SCRIPT_OPTION, ctrl_ifname, force_option);
        printf("ht: command %s\n", buf);
    } else {
        sprintf(buf, "%s %s -iht,%s %s ", script, KLM_SCRIPT_OPTION, ctrl_ifname, force_option);
    }
    
    system(buf);
    free(buf);
    
    /* check if driver was loaded succeffully or not */
    if (driver_loaded("ht")) {
        /* IMPORTANT: insmod ht.ko with this argument 'destportid=2' desportid must be= 2 for
	   hig2 to work!!!!!!*/
        if (is_plat_10gkr_capable()) {
	    system("echo 8 > /proc/hg/destportid");
	}
	else {
	    system("echo 2 > /proc/hg/destportid");
	}
	return(PASS);
    }
    return(FAIL);
}

static int
hts_menu_load (int force)
{
    int  ret;
    char path[80];

    sprintf(path, "%s%s", DIR_PATH, "ifht");
    
    if (force) {
        ret=hts_load("-fht", path);
    } else {
        ret=hts_load("", path);
    }
    if (ret == FAIL)
        cterr('f', 0, "xaui driver was not loaded. please type "
              "'tail -f /var/log/messages' for more info.");
    return(ret);
}

/********************************************************************
 *
 * Function:  hts_send_pkt
 *
 * Description: send out packets, receive packets, and compare packets.
 *
 * Inputs: dir - 1 if we want to go from eth to ht (xaui) interface
 *               0 if we want to go from ht to eth
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 ********************************************************************
 */
static int
hts_send_pkt (int dir)
{
    int status, tc;
    char ifdest[10];
    char ifsrc[10];
    char buf[80];

    if (testpass < 2) {
        menu_pkt_len = getdec_answer("enter starting packet len", 1000, 1, 10240);
        menu_max_pkt_len = getdec_answer("enter ending packet len", menu_pkt_len,
                                         menu_pkt_len, 10240);
    }
    if (dir) {
        sprintf(ifdest, "ht");
        sprintf(ifsrc, ctrl_ifname);
    } else {
        sprintf(ifsrc, "ht");
        sprintf(ifdest, ctrl_ifname);
    }
    /* test different traffic class */
    tc = testpass%6;
    sprintf(buf, "echo %d > /proc/hg/tc", tc);
    system(buf);
    testname("xaui [tc=%d] [src=%s]", tc, ifsrc);
    status = test_txrx("len = %d ", ifdest, ifsrc, menu_pkt_len,
                       menu_max_pkt_len, LOGFILE);
    if (status == FAIL)
        cterr('f', 0, "failed xaui tx/rx");
    prcomplete(testpass, errcount, (char *)0);
    return(status);
}

/********************************************************************
 *
 * Function:  hts_send_pkt
 *
 * Description: send out packets, sweeping from 64 to 9212 bytes,
 * receive packets, and compare packets.
 *
 * Inputs: dir - 1 if we want to go from eth to ht (xaui) interface
 *               0 if we want to go from ht to eth
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 ********************************************************************
 */
static int
hts_sweep_send_pkt (int dir)
{
    int max_pkt_len, pkt_len, status;
    char ifdest[10];
    char ifsrc[10];

    testname("xaui sweep");
    pkt_len = 64;
    max_pkt_len = 9212;
    if (dir) {
        sprintf(ifdest, "ht");
        sprintf(ifsrc, ctrl_ifname);
    } else {
        sprintf(ifsrc, "ht");
        sprintf(ifdest, ctrl_ifname);
    }
    status = test_txrx("packet len = %d ", ifdest, ifsrc,  pkt_len, max_pkt_len, LOGFILE);
    if (status == FAIL)
        cterr('f', 0, "failed xaui tx/rx");
    prcomplete(testpass, errcount, (char *)0);
    return(status);
}

static int
shutdown (int dummy)
{
    system("ifconfig ht down");
    return(PASSED);
}

/*---------------------------------------------------------------
$Log: ht.c,v $
Revision 1.23  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.22  2016/10/16 15:58:34  iachang
Supported Goldbeach Platform.

Revision 1.21  2016/10/16 15:42:57  iachang
Supported Goldbeach Platform.

Revision 1.20  2014/06/19 22:00:51  ptong
Incorporated BCM sdk-6.4.1-EA2 for 10G-KR interface. New code works on O2, June, and non Greyhound USD regression

Revision 1.19  2014/05/16 22:17:36  mcharon
fix missing argument to cterr

Revision 1.18  2014/05/16 20:13:33  mcharon
for grehound switch, init time is longer so add delay upto 600 secs before tx first packet

Revision 1.17  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.16  2014/04/07 18:21:07  mcharon
reload driver before tests to clear arp packets. shutdown interface after test

Revision 1.15  2014/03/28 03:48:50  mcharon
add sumenu entry to invoke bcm shell

Revision 1.14  2014/03/26 19:23:26  siyen
Added Dynamo supports at the platform (CSCun82755).

Revision 1.13  2014/02/19 04:05:47  hroni
use show_margins_x in CLI_MODE to implement display_env()

Revision 1.12  2014/02/13 00:30:31  mcharon
remove #define LOCAL_DIR

Revision 1.11  2014/02/06 05:55:22  mcharon
make verbose mode even more verbose

Revision 1.10  2014/02/06 05:23:38  mcharon
don't reconfig eth3 ip when running xaui test

Revision 1.9  2014/02/04 21:17:45  mcharon
add header file proto.h needed by msleep

Revision 1.8  2014/02/04 21:14:58  mcharon
xaui can take up to 700ms to come up, so double the wait time to 1500msec

Revision 1.7  2014/02/04 18:53:32  mcharon
need to check if driver is loaded beforre sending packet

Revision 1.6  2014/01/29 20:38:26  mcharon
add some verbose option

$Endlog$
*/

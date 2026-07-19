/* $Id: mgmt_port.c,v 1.2 2017/08/02 14:21:47 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/mgmt_port.c,v $
 *------------------------------------------------------------------
 * Filename:    mgmt_port.c
 *
 * Description:
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getpid */
#include <strings.h>  /* for bzero*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> /* getpid */
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h> /*struct ethtool */
#include <linux/sockios.h> /* SIOCETHTOOL */
#include <pthread.h>
#include <arpa/inet.h> /* htons */

#include "mgmt_port.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "plat_defs.h"
#include "nvsysvars.h"
#include "menu.h"
#include "queryflags.h"

// unit is miniseconds
#define DELAY_SYSCMD 1000
#define DELAY_IF_UP 1000

#if defined (BOARD_P0C)
//P0C didn't install mgmt phy
int mgmtphy_rgmii_port = -1;
#elif defined (BOARD_P1A)
//mgmt phy occupy eth3 if it is present on P1A
int mgmtphy_rgmii_port = 3;
#else
//Mainly for P0A and P1A
int mgmtphy_rgmii_port = DEFAULT_MGMTPHY_RGMII_ID;
#endif

int ignore_mgmtphy_prcomplete = 0;

/***********************************************************************
 *  External functions Declaration
 ************************************************************************/
extern int do_all_menu_items(struct menuinfo *);
extern int check_dsl_sku(uint *);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int mgmtphy_dump_all_reg(void);
void mgmtphy_internal_lpbk_util(boolean);
void mgmtphy_external_lpbk_util(boolean);

/*******************************************************************************
 *                                   Globals
 *******************************************************************************
 */
static mgmtphy_reg_t m88e1510_reg_table[] =
{
    { 0,  0, 10},
    { 0, 13, 23},
    { 0, 26, 26},
    { 1,  0,  8},
    { 1, 15, 19},
    { 1, 21, 21},
    { 1, 23, 26},
    { 2, 16, 16},
    { 2, 18, 22},
    { 2, 24, 25},
    { 3, 16, 19},
    { 4, 20, 20},
    { 4, 22, 22},
    { 5, 16, 21},
    { 5, 23, 28},
    { 6, 16, 20},
    { 6, 23, 27},
    { 7, 16, 21},
    { 7, 25, 28},
    { 8,  0,  3},
    { 8,  8, 15},
    { 8, 22, 22},
    { 9,  0,  3},
    { 9,  5,  5},
    {12,  0, 15},
    {14,  0,  3},
    {14,  8,  8},
    {14, 14, 15},
    {17, 16, 21},
    {17, 23, 28},
    {18,  0,  2},
    {18, 16, 20},
    {18, 25, 26},
};

/******************************************************************************* 
 *                                   Submenu in Motherboard Subtest Menu
 *******************************************************************************/
 
/* 
 * submenu for mgmt PHY tests 
 */ 
static submenu_xtable_t mgmtphy_test_table[] = { 

    {"MAC loopback test", (type_t(*)())mgmtphy_mac_lpbk_test,      0,  
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())mgmtphy_is_present, 0,    (type_t(*)())0,       0}, 
    {"SMI/MDIO PHY register test", (type_t(*)())mgmtphy_register_access_test,      0,
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())mgmtphy_is_present, 0,    (type_t(*)())0,       0},      
    {"Internal loopback test", (type_t(*)())mgmtphy_internal_lpbk_util,      1,  
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())mgmtphy_is_present, 0,    (type_t(*)())mgmtphy_internal_lpbk_util,       0}, 
    {"External loopback test", (type_t(*)())mgmtphy_external_lpbk_util,      1,  
     MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())mgmtphy_is_present, 0,    (type_t(*)())mgmtphy_external_lpbk_util,       0}, 
    {"PHY traffic test", (type_t(*)())mgmtphy_traffic_test,      0,  
     MF_CONTINUOUS,
     (type_t(*)())mgmtphy_is_present, 0,    (type_t(*)())0,       0}, 
}; 

#define mgmtphy_TEST_TABLE_SIZE  \
    (sizeof(mgmtphy_test_table) / sizeof(submenu_xtable_t)) 
 
/* 
 * Primary & secondary submenu items (filled in from xtable) 
 */ 
static mitem_t mgmtphy_tests_primary_items[mgmtphy_TEST_TABLE_SIZE + 
                                            MAX_BASE_ITEMS]; 
static mitem_t mgmtphy_tests_secondary_items[mgmtphy_TEST_TABLE_SIZE + 
                                              MAX_BASE_ITEMS]; 
 
menuinfo_t mgmtphy_testmenu = { 
    "Management PHY Subtest Menu",             /* title */ 
    0,                            /* mtparam added by init_empty_menu */ 
    (PFT)menu_show_dflags,        /* shows major flags */ 
    0,                            /* use generic prompt */ 
    0,                            /* size (bumped by add_menu_item() */ 
    mgmtphy_tests_primary_items, 
}; 
 
menuinfo_t *mgmtphy_testmenup = &mgmtphy_testmenu; 


/*
 * mgmt port_external_lpbk Menu
 */

static submenu_xtable_t mgmt_port_external_test_table[] = {
    {"External loopback test(1000M)", 
        (type_t(*)())mgmtphy_external_lpbk_test, 1, 
        MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"External loopback test(100M)", 
        (type_t(*)())mgmtphy_external_lpbk_test, 0,
        MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define MGMT_PHY_EXTR_TEST_TABLE_SZ \
        (sizeof(mgmt_port_external_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mgmt_phy_extr_pri_test_items[MGMT_PHY_EXTR_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t mgmt_phy_extr_sec_test_items[MGMT_PHY_EXTR_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t mgmt_phy_extr_lpbk_menu = {
    "Mgmt phy external lpbk Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    mgmt_phy_extr_pri_test_items,
};
static menuinfo_t *mgmt_phy_extr_lpbk_menup = &mgmt_phy_extr_lpbk_menu;


/*
 * mgmt port_internal_lpbk Menu
 */

static submenu_xtable_t mgmt_port_internal_test_table[] = {
    {"Internal loopback test(1000M)", 
        (type_t(*)())mgmtphy_internal_lpbk_test, 1, 
        MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Internal loopback test(100M)", 
        (type_t(*)())mgmtphy_internal_lpbk_test, 0,
        MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define MGMT_PHY_INTR_TEST_TABLE_SZ \
        (sizeof(mgmt_port_internal_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mgmt_phy_intr_pri_test_items[MGMT_PHY_INTR_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t mgmt_phy_intr_sec_test_items[MGMT_PHY_INTR_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t mgmt_phy_intr_lpbk_menu = {
    "Mgmt phy internal lpbk Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    mgmt_phy_intr_pri_test_items,
};
static menuinfo_t *mgmt_phy_intr_lpbk_menup = &mgmt_phy_intr_lpbk_menu;


/* Following functions are defined in the switch.c */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int chk_macaddr(uchar *macaddr1, uchar *macaddr2);
extern void display_pkt(unsigned char *b_ptr, int pktlen);
extern unsigned int swap32(unsigned int i);
extern unsigned int crc32(unsigned int crc, unsigned char *data, int len);
extern int create_raw_socket(int protocol);
extern int bind_socket(char *device, int rawsock, int protocol);
extern int clear_promisc (char *device, int sock);
extern int setup_eth_dev(char *eth_name, int *socket);
extern int cleanup_eth_dev(char *if_name, int socket);
extern int tx_a_pkt(int socket, uchar *pkt, int pkt_len);
extern int eth_pkt_tx(eth_tx_pkt_t *tx_pkt_p);
extern int rx_a_pkt(int socket, uchar *buf_p, int buf_size);
extern int eth_pkt_rx(eth_rx_pkt_t *rxpkt_p);
extern void *pkt_rx_double(eth_rx_pkt_t *rxpkt_s_ptr);
extern void gen_eth_pkt(mac_addr_t macda, mac_addr_t macsa, ushort pkt_type,
         uchar seed, char inc_dec, int payload_len,
         uchar *buf_p);
extern int sgmii_lpbk_util(int port, int pkt_cnt);
extern void control_console_message( int enable );
extern int interface_up( char* ethname );

#define rgmii_lpbk_util(port, pkt_cnt) sgmii_lpbk_util(port, pkt_cnt)

/*
    get the file name of CPU MII mac's control file 
    
*/
int getMgmtMacControlPath(char* filename, int bufsize, char* searchpath)
{
    char cmd[MAX_FILENAME_LENGTH];

    sprintf(cmd, "find %s -name '%s' | tr -d '\n'", searchpath, MGMTPHY_PROC_FILE_NAME);

    if ((ExecuteCmdbyPopen (cmd, filename, bufsize)) == 0)
        return FAILED;

    return PASSED;
}

/*
    get the file name of mgmt phy's control file 
    
*/
int getMgmtPhyControlPath(char* filename, int bufsize, char* searchpath)
{
    char cmd[MAX_FILENAME_LENGTH];

    sprintf(cmd, "find %s -name '%s' | tr -d '\n'", searchpath, MGMTPHY_SYS_FILE_NAME);

    if ((ExecuteCmdbyPopen (cmd, filename, bufsize)) == 0)
        return FAILED;

    return PASSED;
}

/*
      get the mgmt phy interface enum number of Marvell 88E1510
*/
int get_mgmtphy_rgmii_port_enum(void)
{

    if (mgmtphy_rgmii_port >= 0)
        return mgmtphy_rgmii_port;
    else
        return DEFAULT_MGMTPHY_RGMII_ID;
}

/*
      get the mgmt phy interface enum name of Marvell 88E1510
*/
int get_mgmtphy_rgmii_port_name(char *ethname)
{
    if (ethname != NULL) {
        if (mgmtphy_rgmii_port >= 0) {
            sprintf(ethname, "eth%d", mgmtphy_rgmii_port);
            return PASSED;
        } else {
            sprintf(ethname, "eth%d", DEFAULT_MGMTPHY_RGMII_ID);
            return PASSED;
        }
    }
    return FAILED;
}

/******************************************************************************
 *
 * Function   :    mgmtphy_is_present
 * Description:    Check if mgmt phy is present.
 * Inputs     :   None
 * Outputs    : TRUE/FALSE
 *
 ******************************************************************************
 */
int
mgmtphy_is_present (void)
{
    char retbuf[500]={0}, cmd[MAX_COMMAND_LENGTH] = {0};
    char controlpath[MAX_PATH_LENGTH]={0};

    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath (controlpath, MAX_PATH_LENGTH, "/sys"))== FAILED)
        return (FALSE);

    sprintf(cmd, "cat '%s' | grep 'Phy is present' | cut -d ':' -f 2 2>/dev/null", controlpath);
    if(((ExecuteCmdbyPopen (cmd, retbuf, 500)) == 0 )||(atoi(retbuf) != (TRUE))) {
        return (FALSE);
    }
    
    return (TRUE);
}
    
/******************************************************************************
 *
 * Function   :    mgmtphy_mac_lpbk_test_main
 * Description:    main function to perform CPU mac loopback test on MII.
 *              host->RGMII 0->host
 * Inputs     :   ethname : ethernet device name
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
mgmtphy_mac_lpbk_test_main (char* ethname)
{
    int ctrl_plane_rgmii_port;
    int num_pkt = TSN_LOOPBACK_MGMTPHY_PACKET_NO;
    int rc = PASSED;
    char cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};

    // get CPU mac's control entry Ex: /proc/xgene_enet
    if ((getMgmtMacControlPath (controlpath, MAX_PATH_LENGTH, "/proc"))== FAILED) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to find CPU mac's control entry\n");
        return (FAILED);
    }  

    sprintf(cmd, "echo \"%s %d %d %d\" > \"%s\"", ethname, MGMTPHY_PROC_COMMAND_WRITE, MGMTPHY_PROC_MACLOOPBACK, MGMTPHY_PROC_MODE_ENABLE, controlpath);

    if ( system(cmd)!=0 ) 
    {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to enable cpu mac-loopback mode for mgmt phy");
        // Recover setting to keep PHY working.
        sprintf(cmd, "echo \"%s %d %d %d\" > \"%s\"", ethname, MGMTPHY_PROC_COMMAND_WRITE, MGMTPHY_PROC_MACLOOPBACK, MGMTPHY_PROC_MODE_DISABLE, controlpath);
        return (FAILED);
    }
    msleep(DELAY_SYSCMD);

    /* Do RGMII loopback test. */
    ctrl_plane_rgmii_port = get_mgmtphy_rgmii_port_enum();

    if (rgmii_lpbk_util(ctrl_plane_rgmii_port, num_pkt)) {
        rc = FAILED;
    } else {
        rc = PASSED;
    }   

    sprintf(cmd, "echo \"%s %d %d %d\" > \"%s\"", ethname, MGMTPHY_PROC_COMMAND_WRITE, MGMTPHY_PROC_MACLOOPBACK, MGMTPHY_PROC_MODE_DISABLE, controlpath);

    if (system(cmd)!=0) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to disable cpu mac-loopback mode for mgmt phy");
        return (FAILED);
    }
    
    return (rc);
}

/******************************************************************************
 *
 * Function   :    mgmtphy_mac_lpbk_test
 * Description:    perform CPU mac loopback test.
 *              host->RGMII 0->host
 * Inputs     :    None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_mac_lpbk_test (void)
{
    int errorFlag = 0;
    char ethname[10]={0};
    char cmd[MAX_COMMAND_LENGTH]={0};
    char *tname = "MAC loopback";

    testname(tname);
    
    cterr_add_component("Backplane RGMII interface from the router");
    cterr_add_debug("Check Backplane RGMII interface from the router");

    // Disable to print kernel messages on console (but to syslog) while testing
    sprintf(cmd, DISABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    // Disable ipv6 to prevent from sending redundant ipv6 packets
    sprintf(cmd, DISABLE_IPV6_CMD);
    system(cmd);

    prpass(testpass, "Mgmt mac loopback test, ");
    printf("...");
    fflush(stdout);
        
    // Get RGMII name. ex: eth0
    if (get_mgmtphy_rgmii_port_name(ethname) == (FAILED)) {
        cterr('f', 0, "Mac loopback test for mgmt phy is FAILED. Cannot get RGMII name.");
        errorFlag = 1;
        goto error;
    }

    // Make RGMII interface up
    if (interface_up (ethname) == (FAILED)) {
        cterr('f', 0, "Mac loopback test for mgmt phy is FAILED. Cannot make interface %s up.", ethname);
        errorFlag = 1;
        goto error;
    }
        
    if (mgmtphy_mac_lpbk_test_main(ethname) == (FAILED)) {
        errorFlag = 1;
        cterr('f', 0, "Mac loopback test for mgmt phy is FAILED,");
      } else {
        printf("Test is PASS\n");
    }

error:
    // Enable print kernel messages on console
    msleep(DELAY_SYSCMD);
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);

    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);
    
    if(errorFlag)
        return (FAILED);
    
    return PASSED;
}

/******************************************************************************
 *
 * Function   :    mgmtphy_port_internal_lpbk_test
 * Description:    perform Mgmt PHY internal loopback test on a port.
 *                 host->RGMII 0->Mgmt phy->RGMII 0->host
 * Inputs     :    is1G - 100M or 1000M
 * Outputs    :    PASSED/FAILED
 *
 ******************************************************************************
 */
static int
mgmtphy_port_internal_lpbk_test (int is1G)
{
    int ctrl_plane_rgmii_port;
    int num_pkt = TSN_LOOPBACK_MGMTPHY_PACKET_NO;
    int rc = PASSED;
    char cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};

    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath (controlpath, MAX_PATH_LENGTH, "/sys"))== FAILED) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to find mgmt PHY's control entry\n");
        return (FAILED);
    }  

    sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_INTLOOPBACK, MGMTPHY_SYS_MODE_ENABLE, is1G, controlpath);
    
    if ( system(cmd)!=0 ) 
    {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to enable mgmt PHY int-loopback mode @ mgmt phy");
        // Recover setting to keep PHY working.
        sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_INTLOOPBACK, MGMTPHY_SYS_MODE_DISABLE, is1G, controlpath);
        return (FAILED);
    }
    msleep(DELAY_SYSCMD);

    /* Do RGMII loopback test. */
    ctrl_plane_rgmii_port = get_mgmtphy_rgmii_port_enum();

    if (rgmii_lpbk_util(ctrl_plane_rgmii_port, num_pkt)) {
        rc = FAILED;
    } else {
        rc = PASSED;
    }   

    sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_INTLOOPBACK, MGMTPHY_SYS_MODE_DISABLE, is1G, controlpath);

    if (system(cmd)!=0) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to disable mgmt PHY int-loopback mode @ mgmt phy");
        return (FAILED);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function   :    mgmtphy_internal_lpbk_util
 * Description:    perform managment PHY internal loopback menu.
 * Inputs     :    do all test item or not
 * Outputs    :    None
 *
 ******************************************************************************
 */
void mgmtphy_internal_lpbk_util(boolean test_items_executed)
{
    char *tname = "Internal loopback";

    testname(tname);

    build_primary_submenu(mgmt_port_internal_test_table,
                          MGMT_PHY_INTR_TEST_TABLE_SZ, "PHY internal loopback",
                          &mgmt_phy_intr_lpbk_menup);

    build_secondary_submenu(mgmt_port_internal_test_table,
                            MGMT_PHY_INTR_TEST_TABLE_SZ,
                            mgmt_phy_intr_sec_test_items);

    if (test_items_executed) {
        do_all_menu_items(&mgmt_phy_intr_lpbk_menu);
    } else {
        menu(&mgmt_phy_intr_lpbk_menu, mgmt_phy_intr_sec_test_items, '\0');
    }
}

/******************************************************************************
 *
 * Function   :    mgmtphy_internal_lpbk_test
 * Description:    perform MGMT PHY internal loopback test.
 *                 host->RGMII 0->Mgmt phy->RGMII 0->host
 * Inputs     :    is1G - 100M or 1000M
 * Outputs    :    PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_internal_lpbk_test (int is1G)
{
    int errorFlag = 0;
    char ethname[10]={0};
    char cmd[MAX_COMMAND_LENGTH]={0};
    char *tname = "Internal loopback";

    testname(tname);
    if(is1G == 0) {        
        printf("TBD - To be Developed\n");
        return PASSED;
    }
    cterr_add_component("Marvell Mgmt PHY",
            "Backplane RGMII interface from the router");
    cterr_add_debug("Check Marvell Mgmt PHY",
            "Check Backplane RGMII interface from the router");

    // Disable to print kernel messages on console (but to syslog) while testing
    sprintf(cmd, DISABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    // Disable ipv6 to prevent from sending redundant ipv6 packets
    sprintf(cmd, DISABLE_IPV6_CMD);
    system(cmd);

    if (is1G) {
        prpass(testpass, "Mgmt PHY internal loopback test(1000M), ");
    }
    else {
        prpass(testpass, "Mgmt PHY internal loopback test(100M), ");        
    }
    printf("...");
    fflush(stdout);
        
    // Get RGMII name. ex: eth0
    if (get_mgmtphy_rgmii_port_name(ethname) == (FAILED)) {
        cterr('f', 0, "Internal loopback test for mgmt phy is FAILED. Cannot get RGMII name.");
        errorFlag = 1;
        goto error;
    }

    // Make RGMII interface up
    if (interface_up (ethname) == (FAILED)) {
        cterr('f', 0, "Internal loopback test for mgmt phy is FAILED. Cannot make interface %s up.", ethname);
        errorFlag = 1;
        goto error;
    }
        
    if (mgmtphy_port_internal_lpbk_test(is1G) == (FAILED)) {
        errorFlag = 1;
        if (is1G) {
            cterr('f', 0, "Internal loopback test(1000M) for mgmt phy is FAILED,");
        }
        else {
            cterr('f', 0, "Internal loopback test(100M) for mgmt phy is FAILED,");
        }
      } else {
        printf("Test is PASS\n");
    }

error:
    // Enable print kernel messages on console
    msleep(DELAY_SYSCMD);
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);

    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);
    
    if(errorFlag)
        return (FAILED);
    
    return PASSED;
}


/******************************************************************************
 *
 * Function   :    mgmtphy_port_external_lpbk_test
 * Description:    perform mgmtphy external loopback test.
 *                 host->RGMII 0->phy->loopback connecter->phy->RGMII 0>host
 * Inputs     :    is1G - 100M or 1000M
 * Outputs    :    PASSED/FAILED
 *
 ******************************************************************************
 */
static int
mgmtphy_port_external_lpbk_test (int is1G)
{
    int ctrl_plane_rgmii_port;
    int num_pkt = TSN_LOOPBACK_MGMTPHY_PACKET_NO;
    int rc = PASSED;
    char cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};

    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath (controlpath, MAX_PATH_LENGTH, "/sys"))== FAILED) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to find mgmt PHY's control entry\n");
        return (FAILED);
    }        

    sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_EXTLOOPBACK, MGMTPHY_SYS_MODE_ENABLE, is1G, controlpath);
    
    if (system(cmd)!=0) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to enable mgmt PHY ext-loopback mode @ mgmt phy");
        // Recover setting to keep PHY working.
        sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_EXTLOOPBACK, MGMTPHY_SYS_MODE_DISABLE, is1G, controlpath);
        return (FAILED);
    }
    msleep(DELAY_SYSCMD);

    /* Do RGMII loopback test. */
    ctrl_plane_rgmii_port = get_mgmtphy_rgmii_port_enum();

    if (rgmii_lpbk_util(ctrl_plane_rgmii_port, num_pkt)) {
        rc = FAILED;
    } else {
        rc = PASSED;
    }   

    sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_EXTLOOPBACK, MGMTPHY_SYS_MODE_DISABLE, is1G, controlpath);

    if (system(cmd)!=0) {
        if (check_menu_flag(D_VERBOSE))
            printf("Failed to disable mgmt PHY ext-loopback mode @ mgmt phy");
        return (FAILED);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function   :    mgmtphy_external_lpbk_util
 * Description:    perform managment PHY external loopback menu.
 * Inputs     :    do all test item or not
 * Outputs    :    None
 *
 ******************************************************************************
 */
void mgmtphy_external_lpbk_util(boolean test_items_executed)
{
    char *tname = "External loopback";

    testname(tname);

    build_primary_submenu(mgmt_port_external_test_table,
                          MGMT_PHY_EXTR_TEST_TABLE_SZ, "PHY external loopback",
                          &mgmt_phy_extr_lpbk_menup);

    build_secondary_submenu(mgmt_port_external_test_table,
                            MGMT_PHY_EXTR_TEST_TABLE_SZ,
                            mgmt_phy_extr_sec_test_items);

    if (test_items_executed) {
        do_all_menu_items(&mgmt_phy_extr_lpbk_menu);
    } else {
        menu(&mgmt_phy_extr_lpbk_menu, mgmt_phy_extr_sec_test_items, '\0');
    }
}

/******************************************************************************
 *
 * Function   :    mgmtphy_external_lpbk_test
 * Description:    perform external loopback test for mgmt phy.
 *              host->RGMII 0 ->Mgmt phy->loopback connecter->Mgmt phy->RGMII 0->host
 * Inputs     :    None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_external_lpbk_test (int is1G)
{
    int errorFlag=0;
    char ethname[10]={0};
    char cmd[MAX_COMMAND_LENGTH]={0};
    char *tname = "External loopback";

    testname(tname);    

    /*
         * if D_EXT_LOOPBACK is OFF, then just return 'PASSED'
         */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass,
            "\n External loopback flag is off, skip '%s' external loopback test \
            and then switch to '%s' internal loopback test. ", tname, tname);
        return (PASSED);
    }

    cterr_add_component("Marvell Mgmt PHY", 
            "External GE loopback connector",
            "Backplane RGMII interface from the router");
    cterr_add_debug("Check Marvell Mgmt PHY",
            "Check External GE loopback connector",
            "Check Backplane RGMII interface from the router");


    // Disable to print kernel messages on console (but to syslog) while testing
    sprintf(cmd, DISABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    // Disable ipv6 to prevent from sending redundant ipv6 packets
    sprintf(cmd, DISABLE_IPV6_CMD);
    system(cmd);
    
    if (is1G) {
        prpass(testpass, "Mgmt PHY external loopback test(1000M), ");
    }
    else {
        prpass(testpass, "Mgmt PHY external loopback test(100M), ");
    }
    printf("...");
    fflush(stdout);
        

    // Get RGMII name. ex: eth1
    if (get_mgmtphy_rgmii_port_name(ethname) == FAILED) {
        cterr('f', 0, "External loopback test for mgmt phy is FAILED. Cannot get RGMII name.");
        errorFlag = 1;
        goto error;
    }
        
    // Make RGMII interface up
    if (interface_up(ethname) == FAILED) {
        cterr('f', 0, "External loopback test for mgmt phy is FAILED. Cannot make interface %s up.", ethname);
        errorFlag = 1;
        goto error;
    }

    if (mgmtphy_port_external_lpbk_test(is1G)){
        errorFlag = 1;
        if (is1G) {
            cterr('f', 0, "External loopback test(1000M) for mgmt phy is FAILED,");
        }
        else {
            cterr('f', 0, "External loopback test(100M) for mgmt phy is FAILED,");
        }
    } else {
        printf("Test is PASS\n");
    }

error:  
    // Enable print kernel messages on console
    msleep(DELAY_SYSCMD);
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);

    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);
        
    if( errorFlag )
        return (FAILED);

    return PASSED;
}


/******************************************************************************
 *
 * Function   :    mgmtphy_traffic_test
 * Description:    perform mgmt phy traffic test.
 *              host->RGMII 0->Mgmt PHY->another host
 * Inputs     :     None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_traffic_test (void)
{
    char cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};
    char ethname[10]={0};
    char *tname = "PHY traffic";

    testname(tname);

    prpass(testpass, "Setting Switch for Traffic test, ");
    printf("...");
    fflush(stdout);

    // Disable to print kernel messages on console (but to syslog) while testing
    sprintf(cmd, DISABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    // Disable ipv6
    sprintf(cmd, DISABLE_IPV6_CMD);
    system(cmd);

    // Get RGMII name. ex: eth1
    if (get_mgmtphy_rgmii_port_name(ethname) == FAILED) {
        printf("Failed\n");
        cterr('f', 0, "Failed to set traffic test for mgmt phy. Cannot get RGMII name.");
        goto error;
    }
    
    // Make RGMII interface up
    if (interface_up (ethname) == FAILED) {
        printf("Failed\n");
        cterr('f', 0, "Failed to set traffic test for mgmt phy. Cannot make interface %s up.", ethname);
        goto error;
    }
    
    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath(controlpath, MAX_PATH_LENGTH, "/sys")) == FAILED) {
        printf("Failed\n");
        cterr('f',0,"Failed to find mgmt phy's control entry");
        goto error;
    }        


    sprintf(cmd, "echo \"%d\" > \"%s\"", MGMTPHY_SYS_TRAFFIC_TEST, controlpath);
    
    if (system(cmd)!=0) {
        printf("Failed\n");
        cterr('f',0,"Failed to enable mgmt PHY traffic mode");
        goto error;
    }

    msleep(DELAY_SYSCMD);

    printf("Done\nYou can start to do real traffic test now.\n");

    // Enable print kernel messages on console
    msleep(DELAY_SYSCMD);
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    
    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

error:

    // Enable print kernel messages on console
    msleep(DELAY_SYSCMD);
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    
    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
}

/******************************************************************************
 *
 * Function   :    mgmtphy_register_access_test
 * Description:    perform register r/w test.
 *              host->RGMII->write PHY
 *              host->RGMII->read PHY
 * Inputs     :     N/A
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_register_access_test (void)
{
    char retbuf[500]={0}, cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};
    char *tname = "SMI/MDIO PHY register";

    testname(tname);
    prpass(testpass, "Register access test, ");
    printf("...");
    fflush(stdout);
  
    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath(controlpath, MAX_PATH_LENGTH, "/sys")) == FAILED) {
        printf("Failed\n");
        cterr('f',0,"Failed to find giga phy's control entry");
        goto error;
    }

    sprintf(cmd, "echo \"%d\" > \"%s\"", MGMTPHY_SYS_REGISTER_TEST, controlpath);

    if (system(cmd)!=0) {
        printf("Failed\n");
        cterr('f',0,"Failed to enable mgmt PHY traffic mode");
        goto error;
    }

    msleep(DELAY_SYSCMD);
    
    sprintf(cmd, "cat '%s' | grep 'Item' | cut -d ':' -f 2", controlpath);
    if (((ExecuteCmdbyPopen (cmd, retbuf, 500)) == 0 )||(atoi(retbuf) != MGMTPHY_SYS_REGISTER_TEST)) {
        printf("Failed\n");
        cterr('f', 0, "Register access test is failed.");
        goto error;
    }

    sprintf(cmd, "cat '%s' | grep 'Status' | cut -d ':' -f 2", controlpath);
    if(((ExecuteCmdbyPopen (cmd, retbuf, 500)) == 0 )||(atoi(retbuf) != (PASSED))) {
        printf("Failed\n");
        cterr('f', 0, "Register access test is failed.");
        goto error;
    }

    printf("Test is PASS\n");

    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

error:

    if(ignore_mgmtphy_prcomplete == 0)
        prcomplete(testpass, errcount, (char *)0);

    return (FAILED);
    
}


/******************************************************************************
 *
 * Function   :    mgmtphy_subsystem_test
 * Description:    This function is only for [Motherboard test -> Mgmt Phy test]:
 *                        if user choose to show all test items, prompt mgmt phy subtest menu
 *                        if user just do auto test:
 *                             perform external loopback test  if D_EXT_LOOPBACK is enabled.
 *                             perform internal loopback test if D_EXT_LOOPBACK is disabled.
 * Inputs     :      do_autotest_only = 0 show submenu, !=0 perform auto test item
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
 
int
mgmtphy_subsystem_test(int do_autotest_only)
{
    printf("TSN use I210, please use ethtool\n");
    return (PASSED);

    int errorFlag = 0;
    uint dsl_sku, force_link;
    char *tname = "Mgmt PHY";

    testname(tname);
    force_link = TRUE; /* force 1G */
    
    /* Check if DSL SKUs */
    if (check_dsl_sku(&dsl_sku) == FAILED) {
        cterr('f', 0, "Read SKU information fails!");
        return (FAILED);
    }

    /* Reset the Management GE PHY 
	 * 1-Reset to the Marvell 88E1510 Management PHY.
	 */
    if (dsl_sku == TRUE) {
#ifdef NO_SUPPORT_YET
        /* DSL SKU reset the Management GE PHY */
        fpga_reset_api(DSL_SKU_FPGA_1510_RST_L, TRUE, RESET_20_MILLISECONDS);
        /* DSL SKU un-reset the Management GE PHY */
        fpga_reset_api(DSL_SKU_FPGA_1510_RST_L, FALSE, UNRESET_20_MILLISECONDS);
#endif
    } else {
#ifdef NO_SUPPORT_YET    
        /* Reset the Management GE PHY */
        fpga_reset_32_api(FPGA_LPC_DEV_RST_REG, LPC_GE_MGNT_PHY_RESET, TRUE, RESET_20_MILLISECONDS);
        /* Un-reset the Management GE PHY */
        fpga_reset_32_api(FPGA_LPC_DEV_RST_REG, LPC_GE_MGNT_PHY_RESET, FALSE, RESET_20_MILLISECONDS);
#endif		
    }
    
    build_primary_submenu(mgmtphy_test_table, mgmtphy_TEST_TABLE_SIZE, 
                          "Management PHY Subtest Menu", &mgmtphy_testmenup); 
    build_secondary_submenu(mgmtphy_test_table, mgmtphy_TEST_TABLE_SIZE, 
                            mgmtphy_tests_secondary_items); 

    if (do_autotest_only) {
        ignore_mgmtphy_prcomplete = 1;

        // 'SMI/MDIO PHY register test' is one of default test items in auto test
        if(mgmtphy_register_access_test() == FAILED)
            errorFlag = 1;
        
        // If following is TURE, means don't do external loopback
        if (check_menu_flag(D_EXT_LOOPBACK)) {
            // If we don't do external loopback test, do internal loopback test instead.
            // This rule refers to Dreamliner project.
            prpass(testpass,
                "\n External loopback flag is off, skip '%s' external loopback test \
                and then switch to '%s' internal loopback test. ", tname, tname);
            if (mgmtphy_internal_lpbk_test(force_link) == FAILED) {
                errorFlag = 1;
            }
        } else {
            /*
             * External loopback off: run PHY loopback;
             * External loopback on: run PHY + external loopback.
             */
            if (mgmtphy_internal_lpbk_test(force_link) == FAILED) {
                errorFlag = 1;
            }

            if(mgmtphy_external_lpbk_test(force_link) == FAILED) {
                errorFlag = 1;
            }
        }

        ignore_mgmtphy_prcomplete = 0;

        if(errorFlag == 1)
            return (FAILED);
        else
            return (PASSED);
    } else {
        menu(&mgmtphy_testmenu, mgmtphy_tests_secondary_items, '\0' );
    }

    return(PASSED);
}

/******************************************************************************
 *
 * Function   :    read_mgmtphy_register
 * Description:    Read the mgmtphy register
 * Inputs     :    N/A
 * Outputs    :    PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_read_register (void)
{
    char cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};
    int reg_page, reg_addr;

    // Enable to print kernel messages on console while testing
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    
    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath(controlpath, MAX_PATH_LENGTH, "/sys")) == FAILED) {
        printf("Failed\n");
        cterr('f',0,"Failed to find mgmt phy's control entry");
        return (FAILED);
    }

    /* Get the register address */
    reg_addr = getdec_answer("Enter the register address:", 0, 0, 31);

    /* Get the register page */
    reg_page = getdec_answer("Enter the register page:", 0, 0, 255);

    sprintf(cmd, "echo \"%d %d %d\" > \"%s\"", MGMTPHY_SYS_REGISTER_READ, reg_addr, reg_page, controlpath);
    
    if (system(cmd)!=0) {
        printf("Failed\n");
        cterr('f',0,"Failed to read mgmt PHY register\n");
        return (FAILED);
    }

    return(PASSED);
}

/******************************************************************************
 *
 * Function   :    mgmtphy_write_register
 * Description:    Alter the mgmtphy register
 * Inputs     :    N/A
 * Outputs    :    PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_write_register (void)
{
    char cmd[MAX_COMMAND_LENGTH] = {0}, controlpath[MAX_PATH_LENGTH]={0};
    int reg_page, reg_addr;
    unsigned reg_value;

    // Enable to print kernel messages on console while testing
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    
    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath(controlpath, MAX_PATH_LENGTH, "/sys")) == FAILED) {
        printf("Failed\n");
        cterr('f',0,"Failed to find mgmt phy's control entry");
        return (FAILED);
    }

    /* Get the register address */
    reg_addr = getdec_answer("Enter the register address:", 0, 0, 31);

    /* Get the register page */
    reg_page = getdec_answer("Enter the register page:", 0, 0, 255);

    /* Get the register value would like to set */
    reg_value = (unsigned) gethex_answer("Enter the register value:", 0x0, 0x0, 0xFFFF);


    sprintf(cmd, "echo \"%d %d %x %d\" > \"%s\"", MGMTPHY_SYS_REGISTER_WRITE, reg_addr, reg_value, reg_page, controlpath);
    
    if (system(cmd)!=0) {
        printf("Failed\n");
        cterr('f',0,"Failed to write mgmt PHY register\n");
        return (FAILED);
    }

    return(PASSED);
}

/******************************************************************************
 *
 * Function    : mgmtphy_dump_all_reg
 * Description : Utility to dump all of MGMT PHY registers.
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************
 */
int
mgmtphy_dump_all_reg (void)
{
    char          cmd[MAX_COMMAND_LENGTH] = {0};
    char          controlpath[MAX_PATH_LENGTH]={0};
    int           ctr = 0, t_size = 0, curr = 0;
    mgmtphy_reg_t *reg_p;

    reg_p = &m88e1510_reg_table[0];
    t_size = (int)(sizeof(m88e1510_reg_table) / sizeof(mgmtphy_reg_t));

    /* Enable to print kernel messages on console while testing */
    sprintf(cmd, ENABLE_CONSOLE_MSG);
    system(cmd);
    msleep(DELAY_SYSCMD);
    
    // get mgmt phy's control entry Ex: /sys/bus/mdio_bus/drivers/Marvell\ 88E1510/mv88e1510
    if ((getMgmtPhyControlPath(controlpath, MAX_PATH_LENGTH, "/sys")) == FAILED) {
        cterr('f', 0, "Failed to find mgmt phy's control entry");
        return (FAILED);
    }

    for (ctr = 0; ctr < t_size; ctr++, reg_p++) {
        for (curr = reg_p->s_offset; curr <= reg_p->e_offset; curr++) {
            sprintf(cmd, "echo \"%d %d %d\" > \"%s\"",
                         MGMTPHY_SYS_REGISTER_READ,
                         curr,
                         reg_p->page,
                         controlpath);
    
            if (system(cmd) != 0) {
                cterr('f', 0, "Failed to read mgmt PHY register"
                              "(Page %d, Register %d).",
                              reg_p->page, curr);
                return (FAILED);
            }

            msleep(100);
        }
    }
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: mgmt_port.c,v $
Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:11  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/21 02:56:06  steja
Add debug card test items


$Endlog$
*/

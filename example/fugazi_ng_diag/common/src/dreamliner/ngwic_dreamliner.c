/* $Id: ngwic_dreamliner.c,v 1.13 2020/05/22 02:28:25 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/ngwic_dreamliner.c,v $
 *------------------------------------------------------------------
 *
 * ngwic_dreamliner.c - This file contains functions for Dreamliner NGWIC.
 *
 * Christine Wen -- Nov. 2013
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/stat.h>
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
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "platform_slot.h"
#include "dreamliner.h"
#include "nim_dm_cpss_extserv.h"
#include "linux_pciutils.h"
#include "dash_fpga.h" 
#include "cookie_4.h"
#include "platform_fru.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "dreamliner_ge_switch.h"

#include <stdlib.h>
#include <sys/wait.h> /* for wait() */
#include <string.h>

extern int tftp_get (char *, char *, char *, char *, int);
extern int do_all_menu_items(struct menuinfo *);
extern void display_env(void);

extern uint32_t get_ngio_pcie_bus_num(void);
extern uint32_t pci_config_read(uint32_t, uint16_t, uint32_t, int);
extern uint32_t pci_config_write(uint32_t, uint16_t, uint32_t, int, uint32_t);

extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);

extern int phy_utils(void);
extern int phy_start_driver (int smi_addr);
extern int phy_unload_driver();
extern int phy_reg_test(void);
extern int phy_internal_lpbk_test_ge0(void);
extern int phy_internal_lpbk_test_ge1(void);
extern int external_lpbk_test(void);
extern int phy_intr_test(void);

extern int xcat2_pci_reg_test(void);
extern int xcat2_all_reg_test(void);
extern int xcat2_temperature_test(void);
extern int xcat2_mac_lpbk_test_ge0(void);
extern int xcat2_mac_lpbk_test_ge1(void);

extern int set_module_ready();
extern int xcat2_utils(void);
extern int sw_init();
extern int fpga_utils(void);
extern int poe_utils(void);
extern void build_volt_margin_menu(int);
extern int poe_reg_test(void);
extern int fpga_reg_test(void);
extern int spi_flash_test(void);
extern int led_test(void);
extern int led_utils(void);
extern int poe_intr_test(void);
extern int poe_power_ports_util();
extern int slot_get_info(struct ngio_intf_t *ngio, char*);

static int ltc4215_register_test (void);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int dreamliner_power_off (void);
static int dreamliner_pwr_on (void);
static int dreamliner_pwr_cycle (void);
static int dreamliner_utils(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int dreamliner_o2_shell(void);
static int dreamliner_o2_command(void);
static void set_gen_speed(int slot);
static void pci_rdy();
static int dreamliner_console_switch(void);
#ifdef DEBUG
static int  show_dreamliner_pwr(void); 
static uint32_t get_dreamliner_current(uint8_t);
#endif
static void (*dreamliner_saved_diag_exec)(void) = NULL;
static void *oir_if;
static n2g_i2c_if_t *pca_i2c;
#ifdef DEBUG
static n2g_i2c_if_t *oir;
#endif
static struct ngio_intf_t *dreamliner_wic_iface;
static boolean skip_setting = FALSE; 
static int dreamliner_pwr_off();
#define DL_PID_COMMON "NIM-"
static void flag_write(char *);
static boolean flag_read(void);
void dl_pcie_config_read(int offset, uint32_t *reg_ptr);
ushort board_id = 0;
char dl_pid[80];	/* Dreamliner PID is much less than this */
static int dreamliner_slot;
static uchar mb_get_loc[FRU_SIZE] = {0};
static void remove_pcie_device(void);
/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t dreamliner_utils_submenu_table[] = {
    {"Marvell XCAT2 switch Utilities", (PFT)xcat2_utils,            0,   0,
     (type_t(*)())0, 0,    (type_t(*)())xcat2_utils,0},
    {"Marvell PHY Utilities",          (PFT)phy_utils,              0,   0,
     (type_t(*)())0, 0,    (type_t(*)())phy_utils,  0},
    {"FPGA Utilities",                 (PFT)fpga_utils,             0,   0,
     (type_t(*)())is_daughter_card_present, 0,    (type_t(*)())fpga_utils,  0},    
    {"POE Utilities",                  (PFT)poe_utils,              0,   0,
     (type_t(*)())is_daughter_card_present, 0,    (type_t(*)())poe_utils,  0},    
    {"Voltage Margin Utilities",         (PFT)build_volt_margin_menu, 0,   0,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"LED Utilities",                  (PFT)led_utils,              0,   0,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"Power off Dreamliner NGWIC",     (PFT)dreamliner_power_off,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Dreamliner NGWIC",      (PFT)dreamliner_pwr_on,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Dreamliner NGWIC",   (PFT)dreamliner_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Read",         (PFT)ltc4215_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Write",        (PFT)ltc4215_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
#ifdef DEBUG
    {"Display Dreamliner Power",      (PFT)show_dreamliner_pwr,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
#endif
    {"PCA9557 Register Read",         (PFT)pca9557_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 Register Write",        (PFT)pca9557_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Escape to Shell (debugging only)", (PFT)dreamliner_o2_shell,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Execute a Shell command (debugging only)",(PFT)dreamliner_o2_command,0,0,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},   
    {"Console Switch",                (PFT)dreamliner_console_switch,0,0,
     (type_t(*)())0, 0,   (type_t(*)())0,          0},   
};

#define DL_UTILS_SUBMENU_TABLE_SZ (sizeof(dreamliner_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t dl_utils_primary_items[DL_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t dl_utils_secondary_items[DL_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

char dreamlinerutiltitle[50];

menuinfo_t dreamliner_util_submenu = {
    dreamlinerutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,            /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    dl_utils_primary_items,
};

menuinfo_t *dreamliner_util_submenup = &dreamliner_util_submenu;

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Dreamliner Utilities",           (PFT)dreamliner_utils,       
     0,   0,
     (type_t(*)())0, 0,    (type_t(*)())dreamliner_utils, 0},
    {"LTC4215 Register Test",          (PFT)ltc4215_register_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"xCat2 PCI Register Test",        (PFT)xcat2_pci_reg_test,     
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),  
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"xCat2 All Register Test",        (PFT)xcat2_all_reg_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),  
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"xCat2 Temperature Test",          (PFT)xcat2_temperature_test,
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),  
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
    {"Marvell PHY Register Test",      (PFT)phy_reg_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Marvell PHY interrupt Test",     (PFT)phy_intr_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Marvell PHY internal loopback Test through GE0", (PFT)phy_internal_lpbk_test_ge0, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Marvell PHY internal loopback Test through GE1", (PFT)phy_internal_lpbk_test_ge1, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"External loopback Test",         (PFT)external_lpbk_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LED Test",                       (PFT)led_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"FPGA Register Test",             (PFT)fpga_reg_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())is_daughter_card_present, 0,	   (type_t(*)())0,          0},
    {"POE Controller Register Test",   (PFT)poe_reg_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())is_daughter_card_present, 0,	   (type_t(*)())0,          0},
    {"POE Controller interrupt Test",  (PFT)poe_intr_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())is_daughter_card_present, 0,	   (type_t(*)())0,          0},
    {"SPI flash Test",                 (PFT)spi_flash_test, 
     0,   (MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL),
     (type_t(*)())is_daughter_card_present, 0,	   (type_t(*)())0,          0},
    {"Power on/off ports",             (PFT)poe_power_ports_util,   
     0,   0, 
     (type_t(*)())is_daughter_card_present, 0,	   (type_t(*)())0,          0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Dreamliner Main Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 **********************************************************************
 *
 *  Function: cterr_init
 *
 *  Description: Initial Dreamliner specific cterr parameters.
 *
 *  Input: slot
 *
 *  Returns: None
 *
 **********************************************************************
 */
static void
cterr_pid(int slot)
{
    char buf[50];

    /* Setup the PID */
    assert(dreamliner_wic_iface);

    /* get the platform PID table */
    memset(&dl_pid[0], 0, sizeof(dl_pid)); /* Set the null string */
    if (get_pid(dreamliner_wic_iface->cookie , &dl_pid[0]) == FAILED) {
        cterr('f', 0, "%s: Unable to get PID from cookie", __FUNCTION__);
    }
    if (dreamliner_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        sprintf(buf, " MB/SM Carrier Card Slot%x",dreamliner_slot);
        strcpy((char *)mb_get_loc, buf);
    } else {
        sprintf(buf, " MB/WIC%x",dreamliner_slot);
        strcpy((char *)mb_get_loc, buf);
    }
}
/*
 **********************************************************************
 *
 *  Function: cterr_setup
 *
 *  Description: Setup Dreamliner specific cterr parameters.
 *
 *  Input: None
 *
 *  Returns: None
 *
 **********************************************************************
 */
void
cterr_setup(void)
{
    int pid_size;
    char *pid_ptr;

    fru_table_offset = WIC0 + dreamliner_slot - 1;
    pid_size = strlen(dl_pid);
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;
    if (strncmp((char *)platform_fru_table[fru_table_offset].pid_string,
		&dl_pid[0], pid_size)) {
        /* Not the correct PID */
        if (strncmp((char *)platform_fru_table[fru_table_offset].pid_string,
            DL_PID_COMMON, sizeof(DL_PID_COMMON)) == 0) {
            free(platform_fru_table[fru_table_offset].pid_string);
        }
        pid_ptr = malloc(pid_size);
        strcpy(pid_ptr, &dl_pid[0]);
        platform_fru_table[fru_table_offset].pid_string = 
                          (unsigned char *)pid_ptr;
    }
    /* Environment information */
    cterr_add_env_dump((PFV)display_env);
}
/*
 **********************************************************************
 *
 *  Function: dreamliner_utils
 *
 *  Description: Dreamliner Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int dreamliner_utils (void)
{
    assert(dreamliner_wic_iface);

    sprintf(dreamlinerutiltitle, "Dreamliner Slot %d Utilities Menu", 
            dreamliner_wic_iface->slot);
    build_primary_submenu(dreamliner_utils_submenu_table,
                          DL_UTILS_SUBMENU_TABLE_SZ,
                          dreamlinerutiltitle, &dreamliner_util_submenup);

    build_secondary_submenu(dreamliner_utils_submenu_table,
                            DL_UTILS_SUBMENU_TABLE_SZ,
                            dl_utils_secondary_items);

    menu(dreamliner_util_submenup, dl_utils_secondary_items, '\0');

    return (PASSED);
}
/*******************************************************************************
 *
 * Function   : remove_pcie_device
 * Description: remove the pcie device
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void remove_pcie_device (void)
{
    char buf1[BUFFER_LENGTH];
    char buf2[BUFFER_LENGTH];
    char buf3[BUFFER_LENGTH];
    char buf4[BUFFER_LENGTH];
    char lspci_cmd[COMMAND_LENGTH];
    char find_cmd[COMMAND_LENGTH] = FIND_DEVICE_CMD; 
    char remove_cmd[COMMAND_LENGTH] = ECHO_ONE_CMD; 
    char *fname1 = LSPCI_FILE; 
    char *fname2 = PCI_BUS_FILE; 
    FILE  *fp;
    /* Get PCIe bus number NIM-ES2-8 =11AB:E61E;NIM-ES2-4 =11AB:E75A */
    sprintf(lspci_cmd, LSPCI_CMD, fname1);
    system(lspci_cmd);
    fp = fopen(fname1, "r");
    if (fp == NULL) {
        printf("Failed to open %s", LSPCI_FILE);
        return;
    }
    fscanf(fp, "%s", buf1);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG: buf1 = %s\n",buf1);
        fflush(stdout);
    }
    fclose(fp);
    /* Search by PCI address */
    strcat(find_cmd,buf1);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG:find_cmd =  %s\n",find_cmd);
        fflush(stdout);
    }
    sprintf(buf2,"%s > %s",find_cmd,fname2);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG: buf2 = %s\n",buf2);
        fflush(stdout);
    }
    system(buf2);
    fp = fopen(fname2, "r");
    if (fp == NULL) {
        printf("Failed to open %s\n",fname2);
        fflush(stdout);
        return;
    }
    fscanf(fp, "%s", buf3);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG: buf3 = %s\n",buf3);
        fflush(stdout);
    }
    /* Remove the device */
    if (strcmp(buf3, PCI_DEV_CMP) == 0) {
        printf("Failed to get /sys/devices/pci0000:00/");
        fflush(stdout);
        return;
    }
    fscanf(fp, "%s", buf4);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG: buf4 =  %s\n",buf4);
        fflush(stdout);
   }
    fclose(fp);
    strcat(remove_cmd,buf4);
    strcat(remove_cmd, REMOVE_STR);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG: %s\n",remove_cmd);
        fflush(stdout);
    }
    system(remove_cmd);
}

/**********************************************************************
 * Function: dreamliner_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void
dreamliner_cleanup ()
{
    char buf[50], dname[16];

    assert(dreamliner_wic_iface);

    if (dreamliner_saved_diag_exec) {
        pre_diag_exec = dreamliner_saved_diag_exec;
        dreamliner_saved_diag_exec = NULL;
    }

    phy_unload_driver();

    nim_dm_cpss_extserv_cleanup();

    /* rmmod nmm_dm, neptune is change the file naming, 
     * the driver insert to OS still using nim_dm ,
     * no need to change nim_dm to nim_dm_nep in here. 
     */
    sprintf(dname, "nim_dm");

    sprintf(buf, "rmmod %s", dname);

    system(buf);

    if (is_goldbeach()) {
        /*CSCuy95342 : Fixed Goldbeach PCIe BAR memory mapping issue*/
        remove_pcie_device();
    }

}

/*************************************************************************
 * Function: dreamliner_iface_test
 *
 * Test entry for Dreamliner interface test.
 *      covered interfaces: I2C, GE, PCIe.
 *
 * Input : iface - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
static int 
dreamliner_iface_test (struct ngio_intf_t *iface)
{

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
	return (FAILED);
    }

    /* Testing the PCIe interface */
    if (xcat2_pci_reg_test()) {
	return (FAILED);
    }

    /* Testing the backplane GE interface */
    if (xcat2_mac_lpbk_test_ge0()) {
	return (FAILED);
    }

#ifdef TABEIL
    printf("\nTabei-L didn't support GE1");
#else
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        printf("\nGoldbeach and Curie 1RU/2RU didn't support GE1");
    } else if (xcat2_mac_lpbk_test_ge1()) {
        return (FAILED);
    }
#endif

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}
/* ******************************************************************
 *
 * Function: flag_write
 *
 * Description: Write the flag value to retry_flag.txt
 * 
 * Input: value
 * Outputs:  None
 *
 * ******************************************************************
 */
void 
flag_write (char *buf)
{
    FILE *fp;

    /* remove file anyway */
    remove("retry_flag.txt");

    fp = fopen("retry_flag.txt", "w");
    if (fp == NULL) {
        cterr('f', 0, "Failed to open /retry_flag.txt file.");
        return;
    }
    fprintf(fp,"%s\n",buf);
    fclose(fp);
}
/* ******************************************************************
 *
 * Function: poe_si_flag_read
 *
 * Description: Read the flag value from retry_flag.txt
 * 
 * Input: None
 * Outputs:  TRUE / FALSE
 *
 * ******************************************************************
 */
boolean  
flag_read (void)
{
    FILE *fp;
    char buf[10];
    boolean value;
    fp = fopen("retry_flag.txt", "r");
    if (fp == NULL) {
        cterr('f', 0, "Failed to open retry_flag.txt.");
        return (FAILED);
    }
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);

		if (strstr(buf, "TRUE") != NULL) {
		    value = TRUE;
		    break;
		}
		if (strstr(buf, "FALSE") != NULL) {
		    value = FALSE;
		    break;
		}
    }
    fclose(fp);
    return (value);
}

/*------------------------------------------------------------------------------
 *
 * Function: dreamliner_test().
 *
 * Description: This function is the entry point for Dreamliner NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int 
dreamliner_test (void *wic)
{ 
    int slot;
    char buf[50];
    pid_t pid; 
    int child_exit_rtn, rx_rtn_stat = 0;
    int retry = 0;
    assert(wic);
    struct stat sts;
    char cmd[32], dname[32], dpath[32];
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;

    dreamliner_wic_iface = (struct ngio_intf_t *)wic;

    slot = dreamliner_slot = dreamliner_wic_iface->slot;
    board_id = dreamliner_wic_iface->id;

    /* Setup common parameters for new error message */
    cterr_pid(slot);
    cterr_setup();

    printf("\ndreamliner_test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Dreamliner NGWIC", slot);

    oir_if = (void *)(dreamliner_wic_iface->oir);

    pca_i2c = dreamliner_wic_iface->pca;

    /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
     * not the ability of auto-negotiation between 1G and 10G */
    module_type = dreamliner_wic_iface->mod_type;
    new_speed = NGIO_ETH_SPEED_1G;
    ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);

#ifdef TABEIL
    skip_setting = TRUE;
#else
    if (is_juno_plx() || is_utah_plx() || is_sword() || is_dagger() || 
        is_goldbeach() || is_ntpn_machines() || is_vg450() ||
        is_curie_1ru() || is_curie_2ru()) {
        skip_setting = TRUE;
    } else {
        skip_setting = FALSE;
    }
#endif
    flag_write("FALSE");
#ifdef TABEIL
    sprintf(dname, "nim_dm_tabeil.ko"); 
    sprintf(dpath, "/firmware/nim_dm_tabeil.ko"); 
#else
    if (is_ntpn_machines() || is_vg450()) {
        sprintf(dname, "nim_dm_nep.ko"); 
        sprintf(dpath, "/firmware/nim_dm_nep.ko"); 
    } else if (is_curie_1ru() || is_curie_2ru()) {
        sprintf(dname, "nim_dm_curie.ko"); 
        sprintf(dpath, "/firmware/nim_dm_curie.ko"); 
    } else {
        sprintf(dname, "nim_dm.ko"); 
        sprintf(dpath, "/firmware/nim_dm.ko"); 
    }
#endif
    /* Re-try one time */
    for (retry = 0; retry < 4 ; retry++) { 

        if (flag_read() == TRUE) {
	        printf("\n========== Dreamliner : Retry %d ==========\n",retry);
            fflush(0);
            dreamliner_wic_iface->off(wic);
            sleep(1); /* for all component power off */

            if (slot_get_info(dreamliner_wic_iface, "WIC") == FAILED) {
                prcomplete(testpass, errcount, 0);
                return (FAILED);
            }

            msleep(500);
        }

        set_gen_speed(slot);
        sleep(5);
        /* NOTE: on Juno PLX, we need to have unreset right after 
         * power on Dreamliner (less than 1 sec).
         * so we do unreset via early_unreset flag on platform_slot.c and slot.c */
        
        pci_rdy();
        msleep(100);

#ifdef ALWAYS_TFTP
        /* Download image from the network for the first time 
           Always Get the latest image -- Debug purpose */
        sprintf(cmd, "rm -f %s", dpath); 
        system(cmd);
        fflush(stdout);
#endif    
        if (stat(dpath, &sts) == -1) {
            if (tftp_get(0, dname, 0, dpath, 0) < 0) {
                sprintf(cmd, "rm -f %s", dpath);
                system(cmd);
                fflush(stdout);
                cterr('f', 0, "Failed to tftp download firmware to local host");
                return (FAILED);
            }
        } else {
            printf("\nFile image exist...ready to boot !!!\n");
        }
        /* insmod nim_dm.ko */
        sprintf(buf, "insmod %s", dpath);
        system(buf);
        if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
            /*CSCuy95342 : Fixed Goldbeach PCIe BAR memory mapping issue*/
            system(PCI_RESCAN);
            sleep(1);
        }

        pid = fork();
        if (pid == -1) {
		    printf("%s() fork() failed", __func__);
		    dreamliner_cleanup();
		    return (FAILED);
        }
        if (pid > 0) {    /* Parent process */
		    /* Wait for the child to exit */
		    wait(&child_exit_rtn);
		    rx_rtn_stat = WEXITSTATUS(child_exit_rtn);
            if (flag_read() == TRUE) {
                continue;
            }
            break;
        } else {          /* Child process  */
            /* Jessica request When there is a pcie link training error, 
             * diag will issue NIM_RESET_L, wait for 10ms, then release the 
             * NIM_RESET_L then start the process (checking the link training 
             * error, load PonCat2 driver, etc..) again.
             */
            
			/* init nim_dm klm */
			if (dreamliner_wic_iface->mod_type == SM_DAUGHTER_CARD) {
			    /* through SM carrier card */
			    printf("\nDreamliner on SM carrier card, slot = %d\n", slot);
			    if (nim_dm_cpss_extserv_init(slot, 0) != PASSED) {
                            dreamliner_cleanup();
                    flag_write("TRUE");
			        exit(FAILED);
			    }
			} else if (dreamliner_wic_iface->mod_type == DAUGHTER_CARD) {
                /* through SM adapter card */
                printf("\nDreamliner on SM adpater card, slot = %d\n", slot);
			    if (nim_dm_cpss_extserv_init(1, slot) != PASSED) {
                            dreamliner_cleanup();
                    flag_write("TRUE");
			        exit(FAILED);
			    }
            } else {
			    printf("\nDreamliner WIC, slot = %d\n", slot);
			    if (nim_dm_cpss_extserv_init(0,slot) != PASSED) {
                            dreamliner_cleanup();
                    flag_write("TRUE");
			        exit(FAILED);
			    }
			}	
			/* init GE switch */
			if (sw_init() == FAILED) {
                printf("\n sw_init failed. \n");
                fflush(0);
			    dreamliner_cleanup();
                flag_write("TRUE");
			    exit(FAILED);
			}
    
			/* load the PHY driver */
			if (phy_start_driver(PHY_ADDR) == FAILED) {
                printf("\n phy_start_driver failed. \n");
                fflush(0);
			    dreamliner_cleanup();
                flag_write("TRUE");
			    exit(FAILED);
			}
            
			/* turn on the green light */
			if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
                printf("\n util_oir_ltc4215_led failed. \n");
                fflush(0);
			    dreamliner_cleanup();
                flag_write("TRUE");
			    exit(FAILED);
			}
            
			/* set module ready pin */
			if (set_module_ready() == FAILED) {
                printf("\n set_module_ready failed. \n");
                fflush(0);
			    dreamliner_cleanup();
                flag_write("TRUE");
			    exit(FAILED);
			}
    
		    if (diagflag_xram & D_DEBUG_OPTIONS) {
                flag_write("TRUE");
                printf("\n ===== D_DEBUG_OPTIONS ,retry %x\n",retry);
                fflush(0);
                if (retry == 1) {
                    flag_write("FALSE");
                    printf("\n ========== DBG : Retry End ========== \n");
                    fflush(0);
                } else {	
			        dreamliner_cleanup();
		            exit(PASSED);
                }
		    }
	
		/*
		 * pm_subtest_menu now built.  Display and interact with user until
		 * <ESC><RET> back to main menu.
		 *
		 * To prevent freeing up allocated memory prematurely,
		 * save the pre_diag_exec function and set it to NULL.
		 * This will prevent menu() marking the needed memory freed.
		 */
		dreamliner_saved_diag_exec = pre_diag_exec;
		pre_diag_exec = NULL;
    
		build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
				      &maindiagp);
		build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
					main_menu_secondary_items);
    
		if (dreamliner_wic_iface->test_type == IFACE_TEST) {
		    sleep(2);
		    dreamliner_iface_test(dreamliner_wic_iface);
		} else if (dreamliner_wic_iface->menu_display == FALSE) {
		    sleep(2);
		    do_all_menu_items(maindiagp);
		} else {
		    menu(maindiagp, main_menu_secondary_items, '\0');
		}
        prcomplete(testpass, errcount, 0);
        flag_write("FALSE");
		dreamliner_cleanup();
		exit(PASSED);
        } //end of Child process 
	}//end of for()
    if (retry == 4) {
	    cterr('f',0,"Failed to re-initial the Dreamliner.");
    }

    /* Curie 2RU: restore eth configuration */
    ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);

    return(rx_rtn_stat);
} 

/**********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_register_test (void)
{
    int ret;

    prpass(testpass, "LTC4215 OIR Register test ");
    cterr_setup();
    /* Setup the components and debug for cterr */
    cterr_add_component("LTC4215 on Dreamliner",
			            "I2C controller at router");
    cterr_add_debug("Check LTC4215 on Dreamliner",
			        "Check the I2C controller at the router");

    ret = oir_ltc4215_register_test(oir_if);
    if (ret == FAILED) {
        cterr('f',0,"LTC4215 register test failed.");
    }

    return (ret);
}


/**********************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_reg_write(void)
{
    return(util_oir_ltc4215_reg_write(oir_if));
}

/**********************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_reg_read(void)
{
    return(util_oir_ltc4215_reg_read(oir_if));
}


/**********************************************************************
 *
 * Function: dreamliner_pwr_off
 *
 * Description: This function power off Dreamliner NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dreamliner_pwr_off (void)
{
    uint8_t data = 0;

    assert(oir_if);

    printf("\nPower Off the Dreamliner NGWIC.\n");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dreamliner_power_off
 *
 * Description: This function is a wrapper to power off Dreamliner NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dreamliner_power_off (void)
{
    uint8_t ans;

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Dreamliner NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    return (dreamliner_pwr_off());
}


/**********************************************************************
 *
 * Function: dreamliner_pwr_on
 *
 * Description: This function power on Dreamliner NGWIC.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dreamliner_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Dreamliner NGWIC.\n");

    assert(dreamliner_wic_iface);
    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(dreamliner_wic_iface, dreamliner_wic_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power on NGWIC module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return(FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return(FAILED);
    }

    printf("Waiting for Dreamliner NGWIC to Power-Up.\n");
    msleep(2000);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    dreamliner_wic_iface->uart_on(dreamliner_wic_iface);    

    /* take Dreamliner NGWIC out of reset */
    dreamliner_wic_iface->unreset(dreamliner_wic_iface);

    printf("Dreamliner NGWIC is powered up.\n");

    return (PASSED);
}
#ifdef DEBUG
/*******************************************************************************
 *
 * Function   : get_dreamliner_current
 * Description: convert sense register value into current.
 * Inputs     : Sense Register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t
get_dreamliner_current (uint8_t data)
{
    uint32_t       current = 0;
    if (data) {
        current = (data - 1) * SINGLE_SM_CURRENT;
    } else {
        current = 0;
    }
    return (current);
}
/*******************************************************************************
 *
 * Function   : show_skye_sm_pwr
 * Description: Display power of Skye.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
show_dreamliner_pwr (void)
{
    uint32_t voltage, current, power;
    uint8_t data = 0;

    printf("\n\nDreamliner Power Measure:\n\n");
    
    /* turn on board power and take I2C out of reset */
    slot_i2c_unreset(dreamliner_wic_iface, dreamliner_wic_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data)) {
        return (FAILED);
    }
    voltage = (data * SINGLE_SM_VOL) / 100;

    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data)) {
        return (FAILED);
    }
    current = get_dreamliner_current(data) / 100;

    power = voltage * current;

    printf("Voltage = %d.%02d V\n", (voltage / 100), (voltage % 100));
    printf("Current = %d.%02d A\n", (current / 100), (current % 100));
    printf("Power = %d.%02d W\n", (power / 10000), ((power % 10000) / 100));

    return (PASSED);
}
#endif
/**********************************************************************
 *
 * Function: dreamliner_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dreamliner_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the Dreamliner NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Dreamliner is not Power Cycled.\n\n");
        return (PASSED);
    }

    cterr_add_component("Platform Power controller");
    cterr_add_debug("Check the platform power circuitry");

    if (dreamliner_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Dreamliner NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (dreamliner_pwr_on()) {
	cterr('f', 0, "Failed to Power On the Dreamliner NGWIC");
        return(FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: pca9557_reg_read
 *
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9557_reg_read (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
        /* Setup the components and debug for cterr */
        cterr_add_component("PCA9557 on Dreamliner", 
                            "I2C controller at router");
        cterr_add_debug("Check PCA9557 on Dreamliner",
                        "Check the I2C controller at the router");
	cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    printf("\nRegister @ %#x = %#x\n", offset, data);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: pca9557_reg_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9557_reg_write (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        /* Setup the components and debug for cterr */
        cterr_add_component("PCA9557 on Dreamliner", 
                            "I2C controller at router");
        cterr_add_debug("Check PCA9557 on Dreamliner",
                        "Check the I2C controller at the router");
        cterr('f', 0, "Unable to write PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: dreamliner_o2_shell
 *
 * This function provides Linux Shell CLI mode
 * Input:  None
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int 
dreamliner_o2_shell ()
{
    int slot;

    assert(dreamliner_wic_iface);
    slot = dreamliner_wic_iface->slot;
    assert((slot == 1) || (slot == 2) || (slot == 3));

    printf("\nEscaping to Shell from NGWIC Slot %d Menu,\n", slot);
    printf("To back to Menu, please type exit from Shell.\n\n");

    system("/bin/bash");
    return(PASSED);    
}

/*------------------------------------------------------------------------------
 *
 * Function: dreamliner_o2_command
 *
 * This function provides send Linux command to Shell.
 * Input:  None
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int 
dreamliner_o2_command ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\nPlease enter command: ");
    fgets(cmd, maxlen-1, stdin);
    system(cmd);

    return(PASSED);
}
/*------------------------------------------------------------------------------
 *
 * Function: dreamliner_console_switch().
 *
 * This function provides console redirect for Dreamliner NIM *
 * Input:    show menu option
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int
dreamliner_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(dreamliner_wic_iface);
    assert(oir_if);

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d", 
             dreamliner_wic_iface->uart_ctrl);

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif

    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}
/******************************************************************************
 *
 * Function   :	dl_pcie_config_read
 * Description:	wrapper to read PCIe configuration space register.
 * Inputs     :	offset - register offset
 *              reg_ptr - pointer to hold register data
 * Outputs    : None
 *
 *******************************************************************************
 */
void 
dl_pcie_config_read (int offset, uint32_t *reg_ptr)
{
    uint32_t bus;
    int device;

    bus = get_ngio_pcie_dev_bus_num(dreamliner_wic_iface->mod_type, dreamliner_wic_iface->slot);
    device = 0;

    *reg_ptr = pci_config_read(bus, device, 0, offset);
}


/******************************************************************************
 *
 * Function   :	dl_pcie_config_write
 * Description:	wrapper to write PCIe configuration space register.
 * Inputs     :	offset - register offset
 *              reg_data - register data to write
 * Outputs    : None
 *
 ******************************************************************************
 */
void 
dl_pcie_config_write (int offset, uint32_t reg_data)
{
    uint32_t bus;
    int device;

    bus = get_ngio_pcie_dev_bus_num(dreamliner_wic_iface->mod_type, dreamliner_wic_iface->slot);
    device = 0;

    pci_config_write(bus, device, 0, offset, reg_data);
}

/******************************************************************************
 *
 * Function   :	get_bus_num
 * Description:	return the PCIe bus number from slot.
 * Inputs     :	slot 
 * Outputs    : bus_num
 *
 ******************************************************************************
 */
int 
get_bus_num (uint mod_type, uint slot)
{
    return (get_ngio_pcie_dev_bus_num(mod_type, slot));
}


/******************************************************************************
 *
 * Function   :	get_port_num
 * Description:	return the SKU port number.
 * Inputs     :	None 
 * Outputs    : port_num
 *
 ******************************************************************************
 */
int 
get_port_num (void)
{
    assert(dreamliner_wic_iface);
    if (dreamliner_wic_iface->id == NIM_ES2_4)
	return 4;
    else
	return 8;
}

/******************************************************************************
 *
 * Function   :	is_poe_sku
 * Description:	check the SKU to be POE sku or not.
 * Inputs     :	None 
 * Outputs    : TRUE/FALSE
 *
 ******************************************************************************
 */
boolean
is_poe_sku (void)
{
    assert(dreamliner_wic_iface);
    if (dreamliner_wic_iface->id == NIM_ES2_8P)
	return TRUE;
    else
	return FALSE;
}

/******************************************************************************
 *
 * Function   :	get_slot_num
 * Description:	return the module slot number.
 * Inputs     :	None 
 * Outputs    : slot_num
 *
 ******************************************************************************
 */
int 
get_slot_num (void)
{    
    int slot;

    assert(dreamliner_wic_iface);
    slot = dreamliner_wic_iface->slot;

    return (slot);
}

static void
set_gen_speed (int slot)
{
    uint pcie_switch_bus_no = 0;
    uint device_id, vendor_id; 
    int data = 0, device_no;

    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        printf("\nGoldbeach and Curie 1RU/2RU Didn't Support PCIe SW \n");
        return; /* Goldbeach didn't have PLX */
    }

    assert(dreamliner_wic_iface);

    /* only idt pcie swtich need to config pcie bus 
     * so juno-plx, utah-plx, sword, and dagger will not need to use bus num. 
     */
    if (is_utah_plx() || is_juno_plx()) {
        device_id = PLX_PCIE_SW_DID_8618;
        vendor_id = PLX_PCIE_SW_VID;
    } else if (is_sword()) {
        device_id = PLX_PCIE_SW_DID_8617;
        vendor_id = PLX_PCIE_SW_VID;
    } else if (is_dagger()) {
        device_id = PLX_PCIE_SW_DID_8604;
        vendor_id = PLX_PCIE_SW_VID;
    } else {  /* idt pcie switch - o2, juno, utah */
        device_id = IDT_PCIE_SW_DID;
        vendor_id = IDT_PCIE_SW_VID;
    }

    pcie_switch_bus_no = get_pcie_bus_num(vendor_id, device_id);

    /* access ngio pcie swtich bus instead pcie swtich itself */
    /* so the bus number should be plus 1 */
    pcie_switch_bus_no += 1; 

    if (dreamliner_wic_iface->mod_type == SM_DAUGHTER_CARD) {
	device_no = get_sm_device_no(slot);
    } else {
	device_no = get_wic_device_no(slot);
    }

    if (skip_setting) {
        return;
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
            printf("pci offset 0x70 is %#x\n", data);
        }
        /* setpci -s 3:device 70.l=1 */
        pci_config_write(pcie_switch_bus_no, device_no, 0, 0x70, 1);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
            printf("pci offset 0x70 is %#x\n", data);
        }
    }
    return;
}

static void
pci_rdy ()
{
    if (is_ntpn_machines() || is_vg450()) {
        dreamliner_wic_iface->pci_rdy(dreamliner_wic_iface, 1);
    }

    if (skip_setting) 
        return;
    
    dreamliner_wic_iface->pci_rdy(dreamliner_wic_iface, 1);
}

/*
 *------------------------------------------------------------------
 * $Log: ngwic_dreamliner.c,v $
 * Revision 1.13  2020/05/22 02:28:25  qingcwan
 * Merge switzer-carrier code into main chunk.
 *
 * Revision 1.12  2020/01/13 09:09:04  jiajliu
 * fixed a merge issue - missing "else" found by Regression and Alan
 *
 * Revision 1.11  2020/01/09 01:02:10  jiajliu
 * Merge Curie 2RU to main trunk
 *
 * Revision 1.10  2019/10/17 02:16:15  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.9  2019/08/20 06:54:08  alpeng
 * fixed dreamliner bug on neptune, missing 'else' statement
 *
 * Revision 1.8  2019/08/06 06:56:07  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.7.2.2  2018/09/20 07:57:44  leschen
 * Support curie_1ru platform.
 *
 * Revision 1.7.2.1  2018/07/16 09:28:04  alpeng
 * skip ge switch portions for prince, reva, arkenstone and dreamliner
 *
 * Revision 1.7  2018/05/22 02:31:11  alpeng
 * fixed compiler warning, CSCvj57934
 *
 * Revision 1.6  2018/05/18 09:24:49  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.5  2017/08/10 10:10:37  iachang
 * CSCvf44161: Merge Goldbeach into USD platform as one image
 *
 * Revision 1.4  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.3  2016/10/16 12:28:15  iachang
 * Supported Goldbeach Platform.
 *
 * Revision 1.2.20.10  2018/05/17 10:50:21  alpeng
 *  sync with trunk <trunk-051618>
 *
 * Revision 1.2.20.9  2017/11/27 05:59:42  leschen
 * Initial check in to support VG450.
 *
 * Revision 1.2.20.8  2017/07/28 09:05:40  alpeng
 * Neptune ROMMON need to use pci_rdy to trigger hotplug intr
 *
 * Revision 1.2.20.7  2017/04/17 10:10:26  alpeng
 * change is_nep to is_nptn
 *
 * Revision 1.2.20.6  2017/04/05 06:32:47  leschen
 * Sync with <ng_diag-tag-032917>
 *
 * Revision 1.2.20.5  2016/12/21 02:51:33  alpeng
 * recover to previous version since the module naming on kernel is keep nim_dm
 *
 * Revision 1.2.20.4  2016/12/15 08:14:26  alpeng
 * take care remove driver during clean up for neptune
 *
 * Revision 1.2.20.3  2016/12/07 05:59:00  alpeng
 *  update dreamliner setting for supporting neptune
 *
 * Revision 1.2.20.2  2016/12/05 06:37:00  alpeng
 * fixed the uart ctrl num for ngio; change is approved on prrq
 *
 * Revision 1.2.20.1  2016/10/28 08:27:48  alpeng
 * update file permission for kernel restriction, add is_neptune
 *
 * Revision 1.5  2017/08/10 10:10:37  iachang
 * CSCvf44161: Merge Goldbeach into USD platform as one image
 *
 * Revision 1.4  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.3  2016/10/16 12:28:15  iachang
 * Supported Goldbeach Platform.
 *
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 * Revision 1.1.6.2  2015/02/14 07:13:54  iachang
 * Dreamliner Diag sync with main trunk.
 *
 * Revision 1.1.4.4  2015/02/06 10:34:31  iachang
 * Moved PoE init function from board init avoid user can't into module menu.
 * 
 * Revision 1.1.4.3  2015/02/05 14:19:01  iachang
 * HW request re-try 3 times.
 * 
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 * 
 * Revision 1.1.2.4  2015/01/28 20:33:40  iachang
 * Add initial retry and diable PCIe downstream port before power down ngwic
 * 
 * Revision 1.1.2.3  2014/12/16 05:36:33  iachang
 * Supported console switch to FPGA.
 * 
 * Revision 1.1.2.2  2014/12/04 12:17:58  iachang
 * Fixed 4 port SKU PCI Register Test failed.
 * 
 * Revision 1.1.2.1  2014/12/02 08:04:11  iachang
 * Dreamliner Diag initial check-in.
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */


/* $Id: diag.c,v 1.2 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/diag.c,v $
 *********************************************************************
 *
 * diag.c - diag entry
 *
 * Copyright (c) 2020-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "error.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "platform_cpu.h"
#include "hr_commn_util.h"

static int display_sys_info (int dummy);
/* Extern functions or variable */
extern int alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();       /* memory utility */
extern int memtest(), memloop(), addrloop(), find_mem(), memdebug();    /* memory utility */
extern int highrise_show_cpuinfo();
extern int build_i2c_menu(int);
extern int highrise_display_temp(void);
extern int hr_cpld_show_sys_info(void);
extern int phy_show_temperature(const char *tag);

extern int build_mb_test_menu (int db_test_items_executed);
extern int emmc_pslc_fully_enable(int);
extern int show_emmc_info(void);
extern void fload_start(void);
extern int emmc_full_test(int option);
extern int smartchip(int);
extern int alter_mb_cookie(int);
extern char *version_str;

extern int ht_modem_diag_menu(int);
extern void modem_power_cycle (int is_modem_pwr_on);
extern int modem_power_dnd (int dummy);
extern int modem_switch_usbport_to_external(int on_off);
/* Declare local functions */
static int display_sys_info (int dummy);



/*  Globals  */

/* FRU PID and Location Strings */
/* Pls keep identical with platform_fru.h */
uchar mb_pid[] = "MB-PID";
uchar io_pid[] = "IO-PID";
uchar dimm_pid[] = "DIMM-PID";

uchar mb_loc[] = "MB";
uchar io_loc[] = "IO";
uchar dimm0_loc[] = "IO/DIMM0";

fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { io_pid,        io_loc },
    { dimm_pid,      dimm0_loc },
};

/*
 * Main menu -> Basic utility -> Memory Utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory", 0, 0,
     (PFT) alt_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"compare memory block", 0, 0,
     (PFT) cmp_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"display memory", 0, 0,
     (PFT) dis_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"move memory block", 0, 0,
     (PFT) mov_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"fill memory", 0, 0,
     (PFT) fil_mem, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"find memory", 0, 0,
     (PFT) memtest, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"memory read or write loop", 0, 0,
     (PFT) memloop, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"memory debug loop", 0, 0,
     (PFT) memdebug, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
    {"address loop", 0, 0,
     (PFT) addrloop, &one,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0},
};

static struct menuinfo mem_debug_menu = {
    "Memory Utility Menu",
    0,
    0,
    0,
    sizeof(mem_debug_items) / sizeof(struct mitem),
    mem_debug_items,
};

static struct menuinfo *mem_debug_menup = &mem_debug_menu;

/*
 * Cookie menu utility
 */
static struct mitem cookie_items[] = {
    {"alter MB cookie", 0, 0,
     (type_t(*)())alter_mb_cookie, &zero,  0, (type_t(*)())0, 0},
    {"dump MB cookie", 0, 0,
     (type_t(*)())alter_mb_cookie, &one, 0, (type_t(*)())0, 0},
};

static struct menuinfo cookie_menu = {
    "Cookie utility Menu",
    0,
    0,
    0,
    sizeof(cookie_items) / sizeof(struct mitem),
    cookie_items,
};

static struct menuinfo *cookie_menup = &cookie_menu;

/* =========================================
 *   Basic utilities
 * ========================================= */

static struct mitem utilmenuitems[] = {
    {"System Information", 0, 0,
     (PFT) display_sys_info, (type_t *) &one, 0,
     (type_t(*)())0, 0},

    {"Memory debug utilities", 0, 0,
    (PFT) menu, (type_t *) &mem_debug_menup, 0,
    (type_t(*)())0, 0},

    {"I2C utilities", 0, 0,
    (PFT) build_i2c_menu, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"Cookie utility",
     0, 0,
     (PFT) menu, (type_t *) &cookie_menup, 0,
     (type_t(*)())0, 0},

    {"Enable eMMC pSLC mode", 0, 0,
    (PFT) emmc_pslc_fully_enable, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"Show eMMC info", 0, 0,
    (PFT) show_emmc_info, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"Full Load Utility", 0, 0,
     (PFT) fload_start, (type_t *) &zero, 0,
     (type_t(*)())0, 0},

    {"eMMC full test", 0, 0,
    (PFT) emmc_full_test, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"Modem power on", 0, 0,
    (PFT) modem_power_cycle, (type_t *) &one, 0,
    (type_t(*)())0, 0},

    {"Modem power off", 0, 0,
    (PFT) modem_power_cycle, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"Modem power control", 0, 0,
    (PFT) modem_power_dnd, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"switch modem to PCIe mode and power off", 0, 0,
    (PFT) modem_switch_usbport_to_external, (type_t *) &zero, 0,
    (type_t(*)())0, 0},

    {"switch modem usb port to external connector", 0, 0,
    (PFT) modem_switch_usbport_to_external, (type_t *) &one, 0,
    (type_t(*)())0, 0},
};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems)/sizeof(struct mitem),
    utilmenuitems,
 };
struct menuinfo *utilmenup = &utilmenu;

/* =========================================
 *  Main menu items
 * ========================================= */
static submenu_xtable_t main_menu_table[] = {
    {"ACT-2 utilities and programming",
     (PFT) smartchip, FALSE,
     MF_CONTINUOUS,
     (type_t(*)())0, 0, (PFT) smartchip, TRUE}
    ,
    {"Motherboard tests",
     (PFT) build_mb_test_menu, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,
     (PFT) build_mb_test_menu, FALSE},

    {"5G Modem Test",
    (PFT) ht_modem_diag_menu, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) ht_modem_diag_menu, FALSE},

};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Hightower Platform Main %s",  /* title */
    0,              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,  /* shows major flags */
    0,              /* generic prompt */
    0,              /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;


/*********************************************************************
 * Function: diag_menu
 * Description: This is the main entry to diag menu interface.
 * Inputs: argc
 *         argv
 * Outputs: None
 *********************************************************************
 */
void diag_menu(int argc, char *argv[])
{
    char arg = 0;

    if(argc > 1) {
        arg = *argv[1];
    } else {
        arg = 0;
    }

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
        &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
        main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

/*********************************************************************
 * Function: has_hidden_item
 *
 * Description: Use this function to hidden menu item
 *
 * Inputs: dummy
 * Outputs: Return FALSE if item is hidden
 *********************************************************************/
boolean has_hidden_item(int dummy)
{
    return (FALSE);
}

static int show_mem_info(void)
{
    FILE *fp = NULL;
    char  buf[128] = {[0 ... sizeof(buf) - 1] = 0};
    char *p = NULL;
    int   i = 0;

    ERR_RET_COND(NULL == (fp = fopen("/proc/meminfo", "r")), FAILED, "Open /proc/meminfo failed.\n");
    for(i = 0; i < 3; i++) { //show first 3 lines
        //MemTotal:         947828 kB
        //MemFree:          829064 kB
        //MemAvailable:     851732 kB
        ERR_RET_COND(0 >= fgets(&buf[0], sizeof(buf) - 1, fp) && (1 | fclose(fp)),
                    FAILED, "Read /proc/meminfo failed.\n");
        ERR_RET_COND(NULL == (p = strstr(&buf[0], ":")) && (1 | fclose(fp)), FAILED, "Failed.\n");
       *p++ = 0;
        while(*p && isspace(*p)) p++;
        printf("%-16s: %s", buf, p);
        memset(buf, 0, sizeof(buf));
    }
    fclose(fp);
    return PASSED;
}

/**********************************************************************
 *
 * Function: display_sys_info
 *
 * Description: display system info, ex. current time, Diag ver
 *
 * Input : dummy
 *
 * Output: None
 *
 **********************************************************************
 */

static int display_sys_info (int dummy)
{
    ERR_RET_COND(PASSED != highrise_show_cpuinfo(), FAILED, "Failed show CPU info.\n");
    
    ERR_RET_COND(PASSED != show_mem_info(), FAILED, "Failed show memory info.\n");

    ERR_RET_COND(PASSED != hr_cpld_show_sys_info(), FAILED, "Failed show CPLD info.\n");

    ERR_RET_COND(PASSED != highrise_display_temp(), FAILED, "Failed show temperature.\n");

    ERR_RET_COND(PASSED != phy_show_temperature("Phy Temp"), FAILED, "Failed show phy temperature.\n");

    ERR_RET_COND(PASSED != show_emmc_info(), FAILED, "Failed show EMMC info.\n");

	printf ("\n%-16s: %s\n", "Diag version", version_str);
	return (PASSED);
}


/*********************************************************************
 * $Log: diag.c,v $
 * Revision 1.2  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.4  2021/01/22 07:01:21  tshanmug
 * chrysler modem power OFF ON sequence and modem access through external usb access
 *
 * Revision 1.1.4.3  2020/10/12 15:48:35  tshanmug
 * Chrysler menu change, mmwave ant test added and Empire modem code cleanup
 *
 * Revision 1.1.4.2  2020/09/24 03:24:24  alpeng
 * support dump cookie
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */


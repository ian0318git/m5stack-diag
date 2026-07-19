/* $Id: diag_async_test.c,v 1.3 2019/12/20 03:05:10 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_async_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_async_test.c - This file is for ASYNC port test
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_common.h"
#include "diag_fpga.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "diag_async_test.h"
#include "nanook_crocus_def.h"

/*
 * Global variables
 */
int as_num_chan = MB_ASYNC_CHAN_NUM;
ulong scc_base;
ulong mb_scc_base;
ulong db_scc_base;
#ifdef XDMA
char *cur_xdma_path;
char mb_xdma_path[32] = MB_XDMA_PATH;
char db_xdma_path[32] = DB_XDMA_PATH;
#endif

/*
 * Static definition
 */
static mitem_t      as_scc_menu_items[MAX_SUBTEST_ITEMS];
static mitem_t      as_util_menu_items[MAX_SUBTEST_ITEMS];
static title_buf_t  as_scc_menu_title[MAX_SUBTEST_ITEMS];
static int          as_scc_menu_index[MAX_SUBTEST_ITEMS];
static title_buf_t  as_scc_menu_header;
static title_buf_t  as_util_menu_header;

/* Local functions */
int diag_async_register_test(void);
int diag_async_intr_test(void);
int async_serial_channel_test(int);
int async_serial_channel_dummy(int);
int build_async_util_menu(int);
int async_serial_channel_test2(int);
extern int crocus_async_chan_lpbk_all_start (int);
extern int crocus_async_chan_lpbk_all_stop (int);
extern int program_reggio_spi_prom_async(int);
extern int program_reggio_spi_prom_async_erase(int);
extern void crocus_spi_read_util(void);
extern void crocus_spi_write_util(void);
extern void crocus_spi_erase_util(void);
extern int crocus_show_fpga_ver (int opt);
/*
 * Sub Menu used for "ASYNC port test"
 */

submenu_xtable_t async_submenu_table[] = {

    {"Register Test",
     (PFT) diag_async_register_test, FALSE,
     MF_SHOW_ERRCOUNT, /* tbd */
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Interrupt Test",
     (PFT) diag_async_intr_test, FALSE,
     MF_SHOW_ERRCOUNT, /* tbd */
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Async Serial channel test (UART/PPP)",
     (PFT) async_serial_channel_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (PFT) async_serial_channel_test, TRUE},
	 
	{"Async Utilities",
     (PFT) build_async_util_menu, FALSE, 0,
     (type_t(*)())0, 0, (PFT) build_async_util_menu, TRUE},
};

#define ASYNC_SUBMENU_TABLE_SIZE (sizeof(async_submenu_table) / \
                               sizeof(submenu_xtable_t))

static mitem_t async_primary_items[ASYNC_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t async_secondary_items[ASYNC_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t async_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    async_primary_items,
};

menuinfo_t *async_submenup = &async_subtest_menu;

static struct menuinfo as_scc_menu = {
    "Async Serial Channel %s ",       /* title */
    (int)0 ,                    /* title param */
    (PFT)menu_show_dflags,      /* show diag flags */
    0,                          /* generic prompt */
    0,                          /* size of menu */
    as_scc_menu_items,
};

static struct menuinfo *as_scc_menup = &as_scc_menu;

static struct menuinfo as_util_menu = {
    "Async FPGA Sub menu",       /* title */
    (int)0 ,                    /* title param */
    (PFT)menu_show_dflags,      /* show diag flags */
    0,                          /* generic prompt */
    0,                          /* size of menu */
    as_util_menu_items,
};

static struct menuinfo *as_util_menup = &as_util_menu;

/* common item menu */
static mitem_t as_scc_common_menu_items[] = {
/*    {"Register test", 0, 0,
    (PFT)async_ch_all_reg_test, (type_t *)0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT, (PFT)async_ch_all_reg_test, 0},*/
};
#define AS_NUM_COMMON_TESTS sizeof(as_scc_common_menu_items)/sizeof(mitem_t)
static title_buf_t  as_common_menu_title[AS_NUM_COMMON_TESTS];
static volatile void *mb_regs = NULL, *db_regs = NULL;

static int get_uio_number(int device_id)
{
	int fd = -1, i, parse_id = 0;
	char path[256], device_id_str[16];
	
	for (i = 0; i < 10; i++) {
		sprintf(path, "/sys/class/uio/uio%d/device/device", i);
		if ((fd = open(path, O_RDONLY)) < 0) {
			return -1;
		}
		
		if (read(fd, device_id_str, sizeof(device_id_str)) < 0) {
			perror("read failed: ");
			return -1;
		}
		close(fd);
		
		sscanf(device_id_str, "0x%x", &parse_id);
		if (device_id == parse_id) {
			return i;
		}
	}
	
	return -1;
}

static int get_xdma_number(int device_id)
{
	int fd = -1, i, parse_id = 0;
	char path[256], device_id_str[16];
	
	for (i = 0; i < 10; i++) {
		sprintf(path, "/sys/class/xdma/xdma%d_c2h_0/device/device", i);
		if ((fd = open(path, O_RDONLY)) < 0) {
			return -1;
		}
		
		if (read(fd, device_id_str, sizeof(device_id_str)) < 0) {
			perror("read failed: ");
			return -1;
		}
		close(fd);
		
		sscanf(device_id_str, "0x%x", &parse_id);
		if (device_id == parse_id) {
			return i;
		}
	}
	
	return -1;
}

int get_scc_base_addr(int mb)
{
	static int mb_resourcefd = -1, db_resourcefd = -1, uio_num, xdma_num;
	char path[256];
		
	if (mb == 1) {
		if (mb_regs == NULL) {
			uio_num = get_uio_number(MB_PCIE_DEVICE_ID);
			if (uio_num < 0) {
				perror("get uio number for MB failed:");
				return errno;
			}
			sprintf(path, "/sys/class/uio/uio%d/device/resource0", uio_num);
			mb_resourcefd = open(path, O_RDWR);
			if (mb_resourcefd < 0) {
				perror("config space resource0: open failed:");
				return errno;
			}
			
			mb_regs = mmap(0, 1024 * 256, PROT_READ|PROT_WRITE, MAP_SHARED, mb_resourcefd, 0);
			if (MAP_FAILED == mb_regs) {
				perror("mmap failed");
				return errno;
			}
			mb_scc_base = (ulong)mb_regs;
			
			xdma_num = get_xdma_number(MB_PCIE_DEVICE_ID);
			if (xdma_num < 0) {
				perror("get xdma number for MB failed:");
				return errno;
			}
			sprintf(mb_xdma_path, "/dev/xdma%d_c2h_0", xdma_num);
		}
	} else {
		if (db_regs == NULL) {
			uio_num = get_uio_number(DB_PCIE_DEVICE_ID);
			if (uio_num < 0) {
				perror("get uio number for DB failed:");
				return errno;
			}
			sprintf(path, "/sys/class/uio/uio%d/device/resource0", uio_num);
			db_resourcefd = open(path, O_RDWR);
			if (db_resourcefd < 0) {
				perror("config space resource0: open failed:");
				return errno;
			}
			
			db_regs = mmap(0, 1024 * 256, PROT_READ|PROT_WRITE, MAP_SHARED, db_resourcefd, 0);
			if (MAP_FAILED == db_regs) {
				perror("mmap failed");
				return errno;
			}
			db_scc_base = (ulong)db_regs;
			
			xdma_num = get_xdma_number(DB_PCIE_DEVICE_ID);
			if (xdma_num < 0) {
				perror("get xdma number for DB failed:");
				return errno;
			}
			sprintf(db_xdma_path, "/dev/xdma%d_c2h_0", xdma_num);
		}
	}
	
	return 0;
}
/*******************************************************************************
 *
 * Function   : build_mb_async_test_menu
 * Description: ASYNC Test Menu
 * Inputs     : Test/Menu
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_mb_async_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "ASYNC Device Test";

    build_primary_submenu(async_submenu_table, ASYNC_SUBMENU_TABLE_SIZE,
                          menu_title, &async_submenup);
    build_secondary_submenu(async_submenu_table, ASYNC_SUBMENU_TABLE_SIZE,
                            async_secondary_items);

    as_num_chan = MB_ASYNC_CHAN_NUM;
	if (get_scc_base_addr(1) == 0) {
		scc_base = (ulong)mb_regs;
#ifdef XDMA
		cur_xdma_path = mb_xdma_path;
#endif
	} else {
		printf("Cannot find Crocus FPGA!\n");
		return (FAILED);
	}
	if (mb_temp_test_items_executed) {
        menu(&async_subtest_menu, async_secondary_items, 0);
    } else {
        do_all_menu_items(async_submenup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : build_db_async_test_menu
 * Description: ASYNC Test Menu
 * Inputs     : Test/Menu
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_db_async_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "ASYNC Device Test";

    build_primary_submenu(async_submenu_table, ASYNC_SUBMENU_TABLE_SIZE,
                          menu_title, &async_submenup);
    build_secondary_submenu(async_submenu_table, ASYNC_SUBMENU_TABLE_SIZE,
                            async_secondary_items);

    as_num_chan = DB_ASYNC_CHAN_NUM;
	if (get_scc_base_addr(0) == 0) {
		scc_base = (ulong)db_regs;
#ifdef XDMA
		cur_xdma_path = db_xdma_path;
#endif
	} else {
		printf("Cannot find Crocus FPGA!\n");
		return (FAILED);
	}
    if (mb_temp_test_items_executed) {
        menu(&async_subtest_menu, async_secondary_items, 0);
    } else {
        do_all_menu_items(async_submenup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_async_register_test
 * Description: ASYNC register test
 * Inputs     : void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_async_register_test (void)
{
    char *tname = "ASYNC Register";
    int  ix, rc = PASSED;

    testname("%", tname);

    prpass(testpass, "%s, ", tname);

    /* Run reg tests */
    for (ix = 0; ix < as_num_chan; ix++) {
        prpass(testpass, "test channel %d,", ix);
        if (crocus_async_reg_test(ix)) {
            rc = FAILED;
            break;
        }
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*******************************************************************************
 *
 * Function   : diag_async_intr_test
 * Description: ASYNC Interrupt test
 * Inputs     : void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_async_intr_test (void)
{

    char *tname = "ASYNC Interrupt";
    int  ix, rc = PASSED;

    testname("%", tname);

    prpass(testpass, "%s, ", tname);

    /* Run reg tests */
    for (ix = 0; ix < as_num_chan; ix++) {
        prpass(testpass, "test channel %d,", ix);
        if (crocus_async_int_test(ix)) {
            rc = FAILED;
            break;
        }
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/**********************************************************************
 * Function: build_asyn_scc_sub_menu
 *
 * Description: Build Async Serial Channel Test menu.
 *
 * Input :  Number of Channels
 * Outputs: None.
 ***********************************************************************/
static void build_async_scc_sub_menu ()
{
    int ix;

    /* Init base menu */
    init_base_submenu(&as_scc_menup, (type_t)as_scc_menu_header.title);

    /* Add common menu items */
    for (ix = 0; ix < AS_NUM_COMMON_TESTS; ix++) {
        sprintf(as_common_menu_title[ix].title, "%s", as_scc_common_menu_items[ix].mline);
        add_menu_item(&as_scc_menu, as_common_menu_title[ix].title,
                      as_scc_common_menu_items[ix].mfunc,
                      (type_t *)&ix,
                      as_scc_common_menu_items[ix].mflag);
    }

    /* Add channel test menu items */
    for (ix = 0; ix < as_num_chan; ix++) {
        sprintf(as_scc_menu_title[ix].title, "%s %d", "test channel", ix);
        as_scc_menu_index[ix] = ix;
        add_menu_item(&as_scc_menu, as_scc_menu_title[ix].title,
                      (PFT)crocus_async_chan_lpbk_test, (type_t *)&as_scc_menu_index[ix],
                      MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT | MF_MULTI);
    }
}

/**********************************************************************
 * Function: async_scc_sub_test
 *
 * This function invokes the sub diagnostics tests for SCC
 *
 * Input : Number of Channels
 * Output: PASSED or FAILED
 ***********************************************************************/
static int async_scc_sub_test ()
{
    int  ix;

    /* Run common tests - skip util from table */
    for (ix = 0; ix < AS_NUM_COMMON_TESTS; ix++) {
        if (as_scc_common_menu_items[ix].mflag & MF_DOALL) {
            prpass(testpass, "%s,", as_scc_menu_title[ix].title);
            if ((as_scc_common_menu_items[ix].mfunc)(as_scc_common_menu_items[ix].mfparam)) {
                return (FAILED);
            }
        }
    }

    /* Run channel tests */
	return crocus_async_chan_lpbk_test_all();
}

/*******************************************************************************
 *
 * Function   : async_serial_channel_test
 * Description: ASYNC Serial Channel Test.
 * Inputs     : show_menu - FALSE for tests. TRUE for submenu.
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int async_serial_channel_test (int show_menu)
{

    int rc = FAILED;

    testname("Async SCC");

    build_async_scc_sub_menu();

    if (show_menu) {
        /* Entered submenu */
        menu(&as_scc_menu, (mitem_t *)0, 0);
        rc = PASSED;
    } else {
        /* Invoked the test */
        rc = async_scc_sub_test();
    }

    return (rc);
}

/**********************************************************************
 * Function: build_async_util_sub_menu
 *
 * Description: Build Async FPGA utility sub menu.
 *
 * Input :  Number of Channels
 * Outputs: None.
 ***********************************************************************/
static void build_async_util_sub_menu ()
{
    /* Init base menu */
    init_base_submenu(&as_util_menup, (type_t)as_util_menu_header.title);

	add_menu_item(&as_util_menu, "Crocus FPGA Version",
				  (PFT)crocus_show_fpga_ver, (type_t *)&zero,
				  0);
	add_menu_item(&as_util_menu, "Upgrade image",
				  (PFT)program_reggio_spi_prom_async, (type_t *)&zero,
				  0);

	if (diagflag_xram & D_DEBUG_OPTIONS) {
		add_menu_item(&as_util_menu, "Erase image",
					  (PFT)program_reggio_spi_prom_async_erase, (type_t *)&zero,
					  0);
		add_menu_item(&as_util_menu, "Async Serial channel test by port/baudrate",
					  (PFT)async_serial_channel_test2, (type_t *)&zero,
					  MF_SHOW_ERRCOUNT);
		add_menu_item(&as_util_menu, "Async loopback all start",
					  (PFT)crocus_async_chan_lpbk_all_start, (type_t *)&zero,
					  MF_SHOW_ERRCOUNT);
		add_menu_item(&as_util_menu, "Async loopback all stop",
					  (PFT)crocus_async_chan_lpbk_all_stop, (type_t *)&zero,
					  MF_SHOW_ERRCOUNT);
		add_menu_item(&as_util_menu, "Flash read utility",
					  (PFT)crocus_spi_read_util, (type_t *)&zero,
					  MF_SHOW_ERRCOUNT);
		add_menu_item(&as_util_menu, "Flash write utility",
					  (PFT)crocus_spi_write_util, (type_t *)&zero,
					  MF_SHOW_ERRCOUNT);
		add_menu_item(&as_util_menu, "Flash erase utility",
					  (PFT)crocus_spi_erase_util, (type_t *)&zero,
					  MF_SHOW_ERRCOUNT);
	}
}

/*******************************************************************************
 *
 * Function   : build_async_util_menu
 * Description: ASYNC FPGA Utilities.
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_async_util_menu (int show_menu)
{
    build_async_util_sub_menu();

    menu(&as_util_menu, (mitem_t *)0, 0);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : async_serial_channel_test2
 * Description: ASYNC Serial Channel Test by port and baud rate.
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int async_serial_channel_test2 (int dummy)
{
	int chan, baud;

    testname("Async SCC");

    chan = getdec_answer("Enter channel value: ", 0, 0, 31);
	baud = getdec_answer("Enter baud rate: ", 300, 300, 256000);

    if (crocus_async_chan_lpbk_test2(chan, baud)) {
		return (FAILED);
	}

    return (PASSED);
}


/**********************************************************************
 * Function: get_as_scc_channel_num
 *
 * Description: Get async scc Channel number
 *
 * Inputs:  None.
 * Outputs: as_num_chan
 ***********************************************************************/
int get_as_scc_channel_num ()
{
    return as_num_chan;
}

/*-------------------------------------------------
 * $Log: diag_async_test.c,v $
 * Revision 1.3  2019/12/20 03:05:10  lucywang
 * CSCvs52759 - System hang when testing ASYNC loopback with Crocus FW 1205, Dynamically get XDMA driver number for Crocus-32/16
 *
 * Revision 1.2  2019/12/11 10:10:27  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */


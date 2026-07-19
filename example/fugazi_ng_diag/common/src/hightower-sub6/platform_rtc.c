/* $Id: platform_rtc.c,v 1.2 2021/06/02 02:56:24 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_rtc.c,v $
 *------------------------------------------------------------------
 * platform_rtc.c - Highrise RTC main function/menu.
 *
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "proto.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_1337.h"
#include "platform_rtc.h"
#include <string.h>
#include "platform_fru.h"

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)
#define ENHANCE_ERROR_MSG_RDY 1

/*
 * static functions prototypes 
 */
static int utility_set_rtc(int);
static int utility_display_rtc(int);
static int rtc_show_regs(int);
static int alter_reg(void);
static int ds1337_register_test_wrapper(int);
static int ds1337_time_validity_test_wrapper(int);
static int ds1337_osc_stop_wrapper(int);
static void dev_obj_create(dev_ds1337_object_t *, n2g_i2c_if_t *);
static int clear_rtc_status_reg(n2g_i2c_if_t *);



/*
 * functions prototypes 
 */
int build_rtc_menu(boolean);
int build_rtc_utils_menu(boolean);
int rtc_init(int);
void display_rtc_wrapper(void);
void time_validity_test_wrapper (void);
int rtc_register_test_wrapper (void);

/*
 * extern functions prototypes
 */
extern int mb_board_type(void);
extern uint32 err_report(dev_object_t *, char *, uint32);
extern int do_all_menu_items(struct menuinfo *);
extern int highrise_display_temp(void);

/*
 * Global variables
 */
#define	MS_PER_SECOND_DELAY 1000



/* RTC Menu */

/*
 * RTC main menu
 */
static submenu_xtable_t rtc_main_menu_table[] = {
    {"RTC chip utilities",  (PFT)build_rtc_utils_menu,    0,
        MF_SHOW_ERRCOUNT,   (type_t(*)())0,     0,
        (PFT)build_rtc_utils_menu,  0},
    { "RTC Init",           (PFT)rtc_init,                0,
        MM_3,               (type_t(*)())0,     0,
        (type_t(*)())0, 0 },
    { "Register Test",      (PFT)ds1337_register_test_wrapper,  0,
        MM_3,               (type_t(*)())0,     0,
        (type_t(*)())0, 0 },
    { "Time Validity Test", (PFT)ds1337_time_validity_test_wrapper, 0,
        MM_3,               (type_t(*)())0,     0,
        (type_t(*)())0, 0 },
    { "Oscillator Stop Check",  (PFT)ds1337_osc_stop_wrapper, 0,
        MM_3,               (type_t(*)())0,     0,
        (type_t(*)())0, 0 },
};

#define RTC_MAIN_MENU_TABLE_SIZE (sizeof(rtc_main_menu_table) / \
                                        sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t rtc_main_menu_primary_items[RTC_MAIN_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];
static mitem_t rtc_main_menu_secondary_items[RTC_MAIN_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];


static struct menuinfo rtcmaindiag = {
  "RTC Test Main Menu",      /* title */
  0,                            /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,        /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  rtc_main_menu_primary_items,
};

static struct menuinfo *rtcmaindiagp = &rtcmaindiag;


/*
 * RTC utils menu
 */
static submenu_xtable_t rtc_util_menu_table[] = {
    { "Display DS1337 RTC", (PFT)utility_display_rtc,  0,
        MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
    { "Set DS1337 RTC",     (PFT)utility_set_rtc,      0,
        MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
    { "Dump DS1337 RTC Registers", (PFT)rtc_show_regs, 0,
        MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
    { "Alter DS1337 RTC Register", (PFT)alter_reg,     0,
        MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 }
};

#define RTC_UTIL_MENU_TABLE_SIZE (sizeof(rtc_util_menu_table) / \
                                        sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t rtc_util_menu_primary_items[RTC_UTIL_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];
static mitem_t rtc_util_menu_secondary_items[RTC_UTIL_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];


static struct menuinfo rtcutildiag = {
  "RTC Utility Menu",      /* title */
  0,                            /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,        /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  rtc_util_menu_primary_items,
};

static struct menuinfo *rtcutildiagp = &rtcutildiag;




/**********************************************************************
 *
 * Function: build_rtc_menu
 *
 * Description: Build RTC chip tests/utils menu.
 *
 * Inputs:      show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
int build_rtc_menu (boolean rtc_test_items_executed)
{
    char *tname = "RTC";

    testname(tname);

    build_primary_submenu(rtc_main_menu_table, RTC_MAIN_MENU_TABLE_SIZE,
                          "RTC Tests Main Menu", &rtcmaindiagp);
    build_secondary_submenu(rtc_main_menu_table, RTC_MAIN_MENU_TABLE_SIZE,
                            rtc_main_menu_secondary_items);

    if (rtc_test_items_executed) {
        do_all_menu_items(rtcmaindiagp);
    } else {
        menu(rtcmaindiagp, rtc_main_menu_secondary_items, '\0' );
    }

    return PASSED;
}


/*************************************************************************
 * Function:    build_rtc_utils_menu
 *
 * Description: Entry to RTC chip utilities menu.
 *
 * Inputs:      None.
 *
 * Outputs:     PASSED.
 *
 *************************************************************************
 */
int build_rtc_utils_menu (boolean rtc_util_items_executed)
{

    build_primary_submenu(rtc_util_menu_table, RTC_UTIL_MENU_TABLE_SIZE,
                          "RTC Utility Main Menu", &rtcutildiagp);
    build_secondary_submenu(rtc_util_menu_table, RTC_UTIL_MENU_TABLE_SIZE,
                            rtc_util_menu_secondary_items);

    if (rtc_util_items_executed) {
        do_all_menu_items(rtcutildiagp);
    } else {
        menu(rtcutildiagp, rtc_util_menu_secondary_items, '\0' );
    }

    return PASSED;
}



/**********************************************************************
 *
 * Function:    rtc_init
 *
 * Description: Initilize Maxim DS1337
 *
 * Inputs:      err_log - cterr if TRUE. printf if FALSE.
 *
 * Outputs:     PASSED/FAILED.
 *
 **********************************************************************
 */
int rtc_init(int err_log)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    rtc_t data;
    int rc;
    char *tname = "RTC init";
    
    testname(tname);

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "rtc_init() Device attach failed");
        return(FAILED);
    }

    /* There are four cases will trigger the OSF bit.
     * 1) The first time power is applied.
     * 2) The voltage present on VCC is insufficient to support oscillation.
     * 3) The EOSC bit is turned off.
     * 4) External influences on the crystal (e.g., noise, leakage, etc.).
     * RTC chip has OSF bit is 1 in first power on issue,
     * need to clear the RTC status register before test.
     * For the other three cases, the OSF bit can not be cleared and will
     * be set to 1 immediately if it's cleared.
     */
    if (clear_rtc_status_reg(&i2c_if) == FAILED) {
        return (FAILED);
    }
    
    /* Call the common device driver init */
    rc = ds1337.base.dev_object_fvt->dev_init(dev);

    /* Calls common device driver destroy instead of detach to free memory */
    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
        if (err_log) {
            cterr('f', 0, "rtc_init() RTC initialization failed");
        } else {
            printf("\n*** rtc_init() RTC initialization failed\n");
        }
    } else {
        utility_display_rtc(TRUE);
        menu_display_real_time = (void *)&display_rtc_wrapper;
    }

    return(rc);

}


/*******************************************************************************
 * Function:	get_rtc
 *
 * Description:	Get DS1337 RTC date/time struct
 *
 * Input:	*rtc - pointer to the date/time struct.
 *
 * Output:	PASSED/FAILED
 *
 *******************************************************************************
 */
int
get_rtc(ds1337_time_t *rtc)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int  rc;
    rtc_t data;

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;
    ds1337.dt = rtc;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
	ds1337.base.dev_object_fvt->dev_destroy(&dev);
	cterr('f', 0, "get_rtc() Device attach failed");
	return(FAILED);
    }

    rc = ds1337.callin_fvt->display_rtc(dev);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
	cterr('f', 0, "get_rtc() Unable to read RTC");
    }

    return (rc);
} /* end of get_rtc */


/*******************************************************************************
 * Function:	set_rtc
 *
 * Description:	Set DS1337 RTC with given date and time.
 *
 * Input:	*rtc - pointer to the date/time struct.
 *
 * Output:	PASSED/FAILED
 *
 *******************************************************************************
 */
int
set_rtc(ds1337_time_t *rtc)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int  rc;
    rtc_t data;

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;
    ds1337.dt = rtc;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
	ds1337.base.dev_object_fvt->dev_destroy(&dev);
	cterr('f', 0, "set_rtc() Device attach failed");
	return(FAILED);
    }

    rc = ds1337.callin_fvt->set_rtc(dev);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
	cterr('f', 0, "set_rtc() Unable to set RTC");
    }

    return (rc);
} /* end of set_rtc */


/**************************************************************
 * Enhance Error Function
 * 1. Subtests of the test function will reuse all variables
 * 2. All variables will be cleared automatically when
 *    entering and leaving each menu item.
 * Segment 1: PID | Unique_string : slot_info 
 *      fru_table_offset should be set, otherwise, it will not 
 *      go to enhanced error message format in cterr() 
 *      set fru_table_offset to get the predefine value 
 *      or change mb_pid & mb_loc 
 * Segment 2: Test step captured from prpass 
 * Segment 3: Failure message captured from cterr 
 * Segment 4: Components used 
 * Segment 5: register and memory dump 
 * Segment 6: Platform Environment initialized here
 * Segment 7: Top 3 Debugging Steps 
 **************************************************************/
static void add_ds1337_err_report(void)
{
    fru_table_offset = MB;
    platform_fru_table[fru_table_offset].pid_string = mb_pid;
    platform_fru_table[fru_table_offset].location_string = mb_loc;

    cterr_add_component("Marvell Armada 7040", "I2C", "RTC");
    cterr_add_env_dump((PFV)highrise_display_temp);
    cterr_add_debug("Do the \"I2C scan test\" to see if "
                    "we can recognize the RTC device.",
                    "If step a is OK, consult with HW to "
                    "verify the I2C interface functionality.");

}
/*******************************************************************************
 *
 * Function:    ds1337_reg_test_wrapper
 *
 * This test writes and reads the registers of the DS1337 and then
 * verifies the read value against the written value.
 *
 * Input:       menu_item - not used
 *
 * Output:      PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ds1337_register_test_wrapper (int menu_item)
{
    if (get_enhance_err_flag()) {
        add_ds1337_err_report();
    }

    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int  rc;
    rtc_t data;
    char *tname = "RTC Register";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "ds1337_register_test_wrapper()Device attach failed");
        return(FAILED);
    }

    rc = ds1337.callin_fvt->register_test(dev);
    if (rc != PASSED) {
        cterr('f', 0, "ds1337_register_test_wrapper() Registers test failed");
    } else {    
        if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
            printf("passed.\n");
        }
    }
    prcomplete(testpass, errcount, (char *)0);
    return (rc);

}

/*******************************************************************************
 * Function:    rtc_register_test_wrapper
 *
 * Description: W/R the registers of the RTC chip (DS1337)
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *******************************************************************************
 */
int rtc_register_test_wrapper (void)
{
    if(ds1337_register_test_wrapper(0) == FAILED) {
        return (FAILED);
    }
    
    return (PASSED);
}


/*******************************************************************************
 *
 * Function:    ds1337_time_validity_test_wrapper
 *
 * This function is a debug function to help debug RTC
 *
 * Input:       menu_item - not used
 *
 * Output:      PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ds1337_time_validity_test_wrapper (int menu_item)
{
    if (get_enhance_err_flag()) {
        add_ds1337_err_report();
    }

    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int rc;
    rtc_t data;
    char *tname = "RTC Validity";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "ds1337_time_validity_test_wrapper Device attach failed");
        return(FAILED);
    }

    rc = ds1337.callin_fvt->time_validity_test(dev);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
        cterr('f', 0, "ds1337_time_validity_test_wrapper() RTC test failed");
    } else {
        prpass(testpass, "%s test passed, ", tname);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/*******************************************************************************
 * Function:    time_validity_test_wrapper
 *
 * Description: DS1337 RTC time validity wrapper
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *******************************************************************************
 */
void
time_validity_test_wrapper (void)
{
    (void)ds1337_time_validity_test_wrapper(TRUE);
}

/*******************************************************************************
 *
 * Function:    ds1337_osc_stop_wrapper
 *
 * This function checks the OSC bit in the status register.
 *
 * Input:       menu_item - not used
 *
 * Output:      PASSED/FAILED
 *
 *******************************************************************************
 */
static int
ds1337_osc_stop_wrapper (int menu_item)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int rc;
    rtc_t data;
    char *tname = "Oscillator Stop Check";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "%s Device attach failed. rc = %#x", __FUNCTION__, rc);
        return(FAILED);
    }

    /* There are four cases will trigger the OSF bit.
     * 1) The first time power is applied.
     * 2) The voltage present on VCC is insufficient to support oscillation.
     * 3) The EOSC bit is turned off.
     * 4) External influences on the crystal (e.g., noise, leakage, etc.).
     * RTC chip has OSF bit is 1 in first power on issue,
     * need to clear the RTC status register before test.
     * For the other three cases, the OSF bit can not be cleared and will
     * be set to 1 immediately if it's cleared.
     */
    if (clear_rtc_status_reg(&i2c_if) == FAILED) {
        return (FAILED);
    }

    rc = ds1337.base.dev_object_fvt->dev_init(dev);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
	    printf("\n *** %s() RTC Oscillator Check failed. rc = %#x",
                      __FUNCTION__, rc);                      
    }

    return (rc);
}


/*******************************************************************************
 * Function:    utility_display_rtc
 *
 * Description: Maxim DS1337 RTC read utility
 *
 * Input:       check - Validate the century field of the RTC.
 *
 * Output:      PASSED/FAILED
 *
 *******************************************************************************
 */
static int utility_display_rtc (int check)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int  rc;
    rtc_t data;

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "utility_display_rtc() Device attach failed");
        return(FAILED);
    }

    rc = ds1337.callin_fvt->display_rtc(dev);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
        cterr('f', 0, "utility_display_rtc() RTC display failed");
    }

    return (rc);
} /* end of utility_display_rtc */


/*******************************************************************************
 * Function:    utility_set_rtc
 *
 * Description: Maxim DS1337 RTC write utility
 *
 * Input:       check - Check the century field of the RTC.
 *
 * Output:      PASSED/FAILED
 *
 *******************************************************************************
 */
static int
utility_set_rtc (int check)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int rc;
    rtc_t data;
    char *tname = "Set DS1337 RTC";

    testname(tname);
    prpass(testpass, "%s, ", tname); 	

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "utility_set_rtc() Device attach failed");
        return(FAILED);
    }

    rc = ds1337.callin_fvt->set_rtc(dev);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
        cterr('f', 0, "utility_set_rtc() Set RTC failed");
    }

    return (rc);
} /* end of utility_set_rtc */


/*******************************************************************************
 * Function:    display_rtc_wrapper
 *
 * Description: DS1337 RTC display wrapper
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *******************************************************************************
 */
void
display_rtc_wrapper (void)
{
    (void)utility_display_rtc(TRUE);
}


/*******************************************************************************
 * Function:    rtc_show_regs
 *
 * Description: Show RTC register contents
 *
 * Inputs:      check - Check the century field of the RTC.
 *
 * Output:      PASSED/FAILED
 *
 *******************************************************************************
 */
static int
rtc_show_regs (int check)
{
    dev_ds1337_object_t ds1337;
    n2g_i2c_if_t i2c_if;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    int  rc;
    rtc_t data;
    char *tname = "Dump DS1337 RTC Registers";

    testname(tname);
    prpass(testpass, "%s, ", tname); 	

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);
    i2c_if.buf = (char *)&data;

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "rtc_show_regs() Device attach failed");
        return(FAILED);
    }

    rc = ds1337.base.dev_object_fvt->dev_show(dev, (print_fn_t)&printf,
                                              DEV_SHOW_ALL);

    ds1337.base.dev_object_fvt->dev_destroy(&dev);

    if (rc != PASSED) {
        cterr('f', 0, "rtc_show_regs Device show failed");
    }

    return (rc);

}/* end of rtc_show_regs */


/*********************************************************************
 *
 * Function:    alter_reg
 *
 * Description: Alter Maxim DS1337 Register.
 *
 * Inputs:      None.
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
alter_reg(void)
{
    dev_ds1337_object_t ds1337;
    dev_object_t *dev = (dev_object_t *)&ds1337;
    n2g_i2c_if_t i2c_if;
    uint32_t rc;
    char *tname = "Alter DS1337 RTC Register";

    testname(tname);
    prpass(testpass, "%s, ", tname); 	

    /* Create the device object */
    dev_obj_create(&ds1337, &i2c_if);

    /* Attach the device object */
    rc = ds1337.base.dev_object_fvt->dev_attach(dev);

    if (rc != PASSED) {
        /* Error occured */
        ds1337.base.dev_object_fvt->dev_destroy(&dev);
        cterr('f', 0, "alter_reg() Device attach failed");
        return(FAILED);
    }

    /* Call common device driver peek-n-poke to alter register */
    rc = ds1337.callin_fvt->peek_n_poke(dev, (print_fn_t)&printf);

    /* detach reset dev_destroy to dev_destroy_default */
    ds1337.base.dev_object_fvt->dev_destroy(&dev);      /* destroy */

    if (rc != PASSED) {
        cterr('f', 0, "alter_reg() Alter Register failed");
    }

    return(rc);

}


/*********************************************************************
 *
 * Function:    dev_obj_create
 *
 * Description: Create Maxim 1337 RTC device object for Common Device Driver.
 *
 * Inputs:      pds1337 - Points to Maxim DS1337 device object
 *              i2c_if - Points to I2C API interface struct
 *
 * Outputs:     None
 *
 *********************************************************************
 */
static void
dev_obj_create(dev_ds1337_object_t *pds1337, n2g_i2c_if_t *i2c_if)
{
    dev_object_t *dev = (dev_object_t *)pds1337;

    /* Setup device struct */

    /* Create common device object */
    dev_1337_create(dev, (dev_error_report_t) err_report);

    /* Setup call-out function vectors */
    pds1337->callout_fvt->open = n2g_i2c_open;
    pds1337->callout_fvt->close = n2g_i2c_close;
    pds1337->callout_fvt->rd = n2g_i2c_read;
    pds1337->callout_fvt->wr = n2g_i2c_write;

    /* Setup other struct fields */
    pds1337->i2c_p = i2c_if;
    pds1337->dt = 0;

    /* Setup I2C API parameter struct */
    i2c_if->offset = 0;
    i2c_if->i2c_bus_type = CPU_I2C2;      /* I2C bus number */
    i2c_if->i2c_dev = MB_I2C_ADDR_RTC;       /* I2C device enum */
    i2c_if->i2c_speed = N2G_I2C_100KHZ; /* I2C bus speed */

    i2c_if->size = sizeof(rtc_t);       /* Buffer size */	

}

/*********************************************************************
 *
 * Function:    clear_rtc_status_reg
 *
 * Description: Clear RTC status register
 *
 * Inputs:      i2c_if - Points to I2C API interface struct
 *
 * Outputs:     None
 *
 *********************************************************************
 */
static int
clear_rtc_status_reg (n2g_i2c_if_t *rtc_i2c_if)
{
    uint32_t rc;

    *rtc_i2c_if->buf = 0;
    rtc_i2c_if->offset = DS1337_STATUS_REG;

    rc = n2g_i2c_write(rtc_i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s(): Clear RTC status failed rc = %#x",
              __FUNCTION__, rc);
        return (FAILED);
    }

    /* Add one second delay after clear the RTC status register */
    msleep(MS_PER_SECOND_DELAY);

    return (PASSED);
}

/*********************************************************************
 * $Log: platform_rtc.c,v $
 * Revision 1.2  2021/06/02 02:56:24  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.2  2020/12/09 06:24:22  alpeng
 * add history
 *
 *
 */

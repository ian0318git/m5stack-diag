/* $Id: platform_30w_poe.c,v 1.8 2017/07/10 02:51:58 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_30w_poe.c,v $
 *------------------------------------------------------------------
 *
 * File name: platform_30w_poe.c
 *
 * Description: PoE Diagnostic Test (Porting from Sunridges)
 *
 * April 2010 by tirawan 
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *------------------------------------------------------------------
 */  
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "defs.h"
#include "proto.h"
#include "menu.h"
#include "queryflags.h"
#include "nvmonvars.h"
#include "pcmap.h"
#include "platform_30w_poe.h"
#include "n2g_api_rc.h"
#include "i2c_api.h"
#include "uio_utils.h"
#include "platform_i2c.h"  
#include "platform_led.h" /*supprt led register */
#include "dev_print.h"
#include "dev_object.h"
#include "dev_ilp.h"
#include "cross_platform.h"
#include "byteswap.h"
#include "i2c_address.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "plat_defs.h"  
#include <pthread.h>

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

static int ovld_poe_init(void);
static int poe_30w_read(n2g_i2c_if_t *);
static int poe_30w_write(n2g_i2c_if_t *);
static int build_poe_led_test(int);
static int build_poe_utility_menu(int);
static int ilp_power_detect(void);
static int read_ilp_util (void);
static int ilp_power_port(void);
static int dc_disc(void);
static int ac_disc(void);
static int show_ilp_regs(void);
static int alter_ilp_regs(void);
static int ilp_poe_2x_mode(void);
static void ilp_device_attach(dev_ilp_object_t * const);
static void ilp_err_report(dev_object_t *, char *, uint32);
static uint32 ilp_i2c_read_callout(uchar, uchar *, int, boolean);
static uint32 ilp_i2c_write_callout(uchar, uchar *, int);
static uint32 ilp_phone_detect(uchar);
static uint32 ilp_get_max_port(void);
static void ilp_led(int, int);
static int ilp_reg_test(void);
static int ilp_int_test(void);
static int ovld_reset_ilp(void);
static inline int ilp_dte_power_setup(int);
static void toggle_ilp_led_off(int);
static void toggle_ilp_green(int, int);
static void toggle_ilp_yellow(int, int);
static int ilp_led_green(int);
static int ilp_led_yellow(int);
static int ilp_led_off(int);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/
extern unsigned long get_fpga_addr(void);
extern int smartchip_authenticate_retest(uchar, uchar);
extern int do_all_menu_items(struct menuinfo *);


/***********************************************************************
 *  Global Variable
 ************************************************************************/

static dev_ilp_object_t ilp_dev;
static dev_ilp_object_t * const pilp_dev = &ilp_dev;

static char ilp_err_msg[POE_ERR_BUF_SIZE];

/******************************************************************
 * PoE LED submenu items    
 *****************************************************************/   
static submenu_xtable_t poe_led_submenu_table[] = {
    { "In Line Power LEDs Green", (PFT)ilp_led_green, 0,
        0, (type_t(*)())0, 0,
      (PFT)ilp_led_green, 0 },
    { "In Line Power LEDs Yellow", (PFT)ilp_led_yellow, 0,
        0, (type_t(*)())0, 0,
      (PFT)ilp_led_yellow, 0 },
    { "In Line Power LEDs off", (PFT)ilp_led_off, 0,
        0, (type_t(*)())0, 0,
      (PFT)ilp_led_off, 0 }
};

#define POE_LED_SUBMENU_TABLE_SZ \
        (sizeof(poe_led_submenu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t poe_led_pri_items[POE_LED_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];
static mitem_t poe_led_sec_items[POE_LED_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

static menuinfo_t poe_led_subtest_menu = {
    "ILP LED Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    poe_led_pri_items,
};
static menuinfo_t *poe_led_submenup = &poe_led_subtest_menu;

/*****************************************************************
 * POE Utilities Menu
 ****************************************************************/

static mitem_t poe_util_submenu_table[] = {
    { "read register util ",                      0, 0, (PFT)read_ilp_util, 
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Ports power detection (Cisco PD)",         0, 0, (PFT)ilp_power_detect, 
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Power On/Off ports",                       0, 0, (PFT)ilp_power_port,  
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Change DC disconnect flags",               0, 0, (PFT)dc_disc,  
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "AC Disconnect",                            0, 0, (PFT)ac_disc, 
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Show all In Line Power ILP registers",     0, 0, (PFT)show_ilp_regs,
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "Alter ILP register",                       0, 0, (PFT)alter_ilp_regs, 
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
    { "ILP 2X Mode",                              0, 0, (PFT)ilp_poe_2x_mode, 
     (type_t *)&zero, 0, (type_t(*)())0, 0 },
};

#define POE_UTIL_SUBMENU_TABLE_SZ    \
                    (sizeof(poe_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t poe_util_subtest_menu = {
    "ILP Utilities Menu",
    (int)0 ,                                       /* title param */
    (PFT)menu_show_dflags,                         /* show diag flags */
    0,
    POE_UTIL_SUBMENU_TABLE_SZ,
    poe_util_submenu_table,
};

static menuinfo_t *poe_util_submenup = &poe_util_subtest_menu;

/******************************************************************
 * POEMain Test Menu
 *****************************************************************/
static submenu_xtable_t poe_submenu_table[] = {
    { "ILP Utilities",            (PFT)build_poe_utility_menu, 1,
      0,                          (type_t(*)())0, 0, (PFT)0, 0},
    { "ILP Register Test",        (PFT)ilp_reg_test,           0,
      (MF_CONTINUOUS | MF_DOALL), (type_t(*)())0, 0, (PFT)0, 0},
    { "ILP Interrupt Test",       (PFT)ilp_int_test,           0,
      (MF_CONTINUOUS | MF_DOALL), (type_t(*)())0, 0, (PFT)0, 0},
    { "ILP LED Test",             (PFT)build_poe_led_test,     0,   
      0,                          (type_t(*)())0, 0, (PFT)0, 0},
};

#define POE_SUBMENU_TABLE_SZ \
        (sizeof(poe_submenu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static  mitem_t poe_pri_items[POE_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static  mitem_t poe_sec_items[POE_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];

static  struct menuinfo    poe_subtest_menu = {
    "In Line Power Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    poe_pri_items,
};
static struct menuinfo *poe_submenup = &poe_subtest_menu;



/***********************************************************************
 *  Functions
 ************************************************************************/

/*******************************************************************************
 *
 * Function   : get_30w_poe_i2c_struct
 * Description: To get 30W POE I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_30w_poe_i2c_struct (n2g_i2c_if_t *i2c_if)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_EIGHT, I2C_MUX_ZERO, 
                                         MB_I2C_ADDR_POE_30W_CTRLER);

    if (tmp == NULL) {
        printf("%s: Failed to get 30w POE I2C interface structure.\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(i2c_if, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_led_green
 *
 * Description: Turn on ILP's LEDs to green.
 *
 * Input:   Show submenu option 
 *
 * Outputs:  PASSED - No errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ilp_led_green (int poeg_submenu)
{
    unsigned int max_port;
    int ix, port_num;
    char msg_buf[64];
    char prpass_buf[64];

    testname("In Line Power LEDs Green");

    prpass(testpass, "ILP Green LED");

    if (poeg_submenu) {
        /* Submenu */
        max_port = ilp_get_max_port() - 1;
        sprintf(msg_buf, "Enter ILP (port) number (0~%d) or others to exit:\n",
                max_port);

        do {
            port_num = getdec_answer(msg_buf, 0, 0, max_port);

            toggle_ilp_green(port_num, TRUE);

            sprintf(prpass_buf, "ILP%d Green LED on", port_num);
            prpass(testpass, prpass_buf);
        } while (1);
    } else {
        /* Not submenu */
        /* Turn on all ILP Green LED */
        for (ix = 0; ix < ilp_get_max_port(); ix++) {
            toggle_ilp_green(ix, TRUE);
        }
        
    }

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_led_yellow
 *
 * Description: Turn on ILP's LEDs to yellow.
 *
 * Input:   Show submenu option 
 *
 * Outputs:  PASSED - No errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ilp_led_yellow (int poeg_submenu)
{

    unsigned int max_port;
    int ix, port_num;
    char msg_buf[64];
    char prpass_buf[64];

    testname("In Line Power LEDs Yellow");

    prpass(testpass, "ILP Yellow LED");

    if (poeg_submenu) {
        /* Submenu */
        max_port = ilp_get_max_port() - 1;
        sprintf(msg_buf, "Enter ILP (port) number (0~%d) or others to exit:\n",
                max_port);

        do {
            port_num = getdec_answer(msg_buf, 0, 0, max_port);

            toggle_ilp_yellow(port_num, TRUE);

            sprintf(prpass_buf, "ILP%d Yellow LED on", port_num);
            prpass(testpass, prpass_buf);
        } while (1);
    } else {
        /* Not submenu */
        /* Turn on all ILP Yellow LED */
        for (ix = 0; ix < ilp_get_max_port(); ix++) {
            toggle_ilp_yellow(ix, TRUE);
        }
        
    }

    return (PASSED);

}


/* ******************************************************
 *
 * Function: ilp_led_off
 *
 * Description: Turn on ILP's LEDs to off.
 *
 * Input:   Show submenu option 
 *
 * Outputs:  PASSED - No errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ilp_led_off (int poeg_submenu)
{

    unsigned int max_port;
    int ix, port_num;
    char msg_buf[64];
    char prpass_buf[64];

    testname("In Line Power LEDs Off");

    prpass(testpass, "ILP LED Off");

    if (poeg_submenu) {
        /* Submenu */
        max_port = ilp_get_max_port() - 1;
        sprintf(msg_buf, "Enter ILP (port) number (0~%d) or others to exit:\n",
                max_port);

        do {
            port_num = getdec_answer(msg_buf, 0, 0, max_port);

            toggle_ilp_led_off(port_num);

            sprintf(prpass_buf, "ILP%d LED off", port_num);
            prpass(testpass, prpass_buf);
        } while (1);
    } else {
        /* Not submenu */
        /* Turn off all ILP LED */
        for (ix = 0; ix < ilp_get_max_port(); ix++) {
            toggle_ilp_led_off(ix);
        }
        
    }

    return (PASSED);

}

/* ******************************************************
 *
 * Function: ilp_poe_2x_mode
 *
 * Description: Enable or Diable PoE 2x mode (support 802.3at, 
 *              30w power)
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ilp_poe_2x_mode (void)
{
    int test_status;

    test_status = pilp_dev->callin_fvt->poe_2x_mode((dev_object_t *)
                                                      pilp_dev);

    return (test_status);
}

/* ******************************************************
 *
 * Function: read_ilp_util
 *
 * Description: Change ILP's register
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 * ONLY FOR DEBUG USING
 * ******************************************************
 */
static int read_ilp_util (void)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc;
    uint32_t data, ix, len;
    uchar offset;
    
    rc = get_30w_poe_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get 30w POE I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }
    while (1) {
        if (((NVRAM)->diagflag & D_CONTINUOUS)) {
            offset = 0;
            len = ILP_MAX_REG;
        } else {

            offset = gethex_answer("Enter in hex the register offset",
                                   0, 0, ILP_MAX_REG);
            
            len = gethex_answer("Enter length of bytes to read",
                            0, 0, ILP_MAX_REG-offset);
        }
        /* Fill the interface struct passed to I2C APIs */
        for (ix = offset; ix < offset + len; ix++) {
            if (ix == ILP_MAX_REG)
                break;
            data = 0;
            i2c_if.size           = sizeof(uchar);
            i2c_if.buf              = (char *)&data;
            i2c_if.offset           = ix;

            /* Perform I2C Read */
            rc = poe_30w_read(&i2c_if);
            if (rc != PASSED) {
                cterr('f', 0, "%s: Unable to read 30W PoE DC register by I2C, rc = %x",
                      __FUNCTION__, rc);
                return (FAILED);
            }
            printf("@0x%02x=0x%02x\n", ix, data & 0xFF);
            
        }
        if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
            break;
        }
    } /* end while */
    return(rc);
}


/* ******************************************************
 *
 * Function: alter_ilp_regs
 *
 * Description: Change ILP's register
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int alter_ilp_regs (void)
{
    int test_status;

    test_status = pilp_dev->callin_fvt->alter_ilp_reg((dev_object_t *)
                                                      pilp_dev);

    return (test_status);
}

/* ******************************************************
 *
 * Function: show_ilp_regs
 *
 * Description: Display ILP's all registers
 *
 * Input:    None 
 *
 * Outputs:  PASSED - No errors encountered.
 *             FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int show_ilp_regs (void)
{
    int test_status;

    test_status = pilp_dev->callin_fvt->show_ilp_regs((dev_object_t *)
                                                      pilp_dev);

    return (test_status);
}

/* ******************************************************
 *
 * Function: ac_disc
 *
 * Description: Test ILP AC Disconnect of a port on ILP. Need IEEE 802.3af
 *              compliant device. ILP interrupt generated by LTC is tested.
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ac_disc (void)
{
    int test_status;

    test_status = pilp_dev->callin_fvt->ac_disc((dev_object_t *)
                                                pilp_dev);

    return (test_status);
}

/* ******************************************************
 *
 * Function: dc_disc
 *
 * Description: Enable or disable ILP DC Disconnect one port at a time,
 *                or all ports at a time.
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int dc_disc (void)
{
    int test_status;

    test_status = pilp_dev->callin_fvt->dc_disc((dev_object_t *)
                                                pilp_dev);

    return (test_status);
}

/* ******************************************************
 *
 * Function: ilp_power_port
 *
 * Description: Power on or off all ports in POE 
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ilp_power_port (void)
{
    int test_status;

    test_status = pilp_dev->callin_fvt->ilp_power_port((dev_object_t *)
                                                        pilp_dev);

    return (test_status);
} 

/* ******************************************************
 *
 * Function: power_detect
 *
 * Description: Check for Power Devices (PD's) at all ports in POE
 *
 * Input:    None
 *
 * Outputs:  PASSED.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static int ilp_power_detect (void)
{
    printf("\n %s: [Fix me!!] need to think about how to do communication"
           " between Intel and Cavium.\n", __FUNCTION__);
#if 0	
    uint port;
    int retval;

    for (port = 0; port < ilp_get_max_port(); port++ ) {
        /* Refer to "DTE Power Over Ethernet" Appnote from Marvell. */
        if ((retval = ilp_phone_detect(port)) == TRUE) {
            printf("\rPort %d: DTE needs power\n", port);
        } else {
            printf("\rPort %d: DTE does not need power\n", port);
        }
    }

#endif	
    return (PASSED);
}

/* ******************************************************
 *
 * Function: rx_intr
 *
 * Description: receive interrput for ilp intr test. 
 *
 * Input:    argument - useless.
 *
 * Outputs:  None.
 *
 * ******************************************************
 */
static
void *rx_intr(void *argument)
{
    unsigned int icount = 0;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while (1) {
        uio_read(&icount);
        printf("\n****interrupt foudn %d****\n", icount);

    }  
    return (void*)NULL;
}


/* ******************************************************
 *
 * Function: ilp_int_test
 *
 * Description: Perform ILP Interrupt Test
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * ******************************************************
 */
static int ilp_int_test (void)
{
    int test_status = FAILED;
    int ilp_int_mask;
    uchar data;
    pthread_t threads;
    void *err;


    /* send speed and port into receive */
    if(pthread_create(&threads, NULL, rx_intr, (void *)NULL)) {
        printf("pthread_create failed \n");
        return 0;
    }

    testname("ILP Interrupt");
    prpass(testpass, "30W PoE DC ");

    /* Reset and init PoE */
    if (ovld_poe_init() == FAILED) {
        return (FAILED);
    }
    
    /* Unmask the PoE Interrupt in FPGA */
    /* ensure other register is zero on 
     * POE Power Supply Unit Interrupt Enable Register (0x28), 
     * and enable the PoE daughter card interrupt */
    ilp_int_mask = get_poe_psu_intr();
    disable_poe_psu_intr(~MB_IOFPGA_POE_PSU_INT_MASK_ALL);
    enable_poe_psu_intr(MB_IOFPGA_POE_PSU_INT_MASK_POE);

    /* Force ILP device to generate interrupt to host */
    if (pilp_dev->callin_fvt->force_interrupt((dev_object_t *)pilp_dev)
        == FAILED) {
        cterr('f', 0, "Failed to force ILP to generate interrupt to host.");
        return (FAILED);
    }
    
    return PASSED;
    sleep(10);
    
     
     if (pthread_cancel(threads)!=0) {
         printf("pthread_cancel error");
     }
     if (pthread_join(threads, &err)!=0) {
         printf("pthread_cancel error");
     }
     if (err == PTHREAD_CANCELED) {
         printf("rx_intr() thread was canceled\n");
     } else {
         printf("rx_intr() thread wasn't canceled (shouldn't happen!)\n");
     }

    if (test_status == FAILED) {
        cterr('f', 0, "Timeout to receive interrupt from ILP device");
    }

    /* Restore the register after the test */
    /* Set operation mode back to auto */
    data = 0xFF;
    if (ilp_i2c_write_callout(ILP_OPERATING_MODE, &data, 1) == FAILED) {
        test_status = FAILED;
    }

    /* Restore intr mask to 0xE4 (default) */
    data = SUPPLY_FAULT | TSTART_FAULT | IMAX_FAULT | DISCONNECT;
    if (ilp_i2c_write_callout(ILP_INT_MASK, &data, 1) == FAILED) {
        test_status = FAILED;
    }

    disable_poe_psu_intr(~MB_IOFPGA_POE_PSU_INT_MASK_ALL);
    enable_poe_psu_intr(ilp_int_mask);

    return (test_status);
}


/* ******************************************************
 *
 * Function: ilp_reg_test
 *
 * Description: Tests ILP's registers.
 *
 * Input:    None
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * ******************************************************
 */
static int ilp_reg_test (void)
{
    int test_status = FAILED;

    testname("ILP Register");

    /* Reset the PoE */
    if (ovld_reset_ilp() == FAILED) {
        return (FAILED);
    }

    test_status = pilp_dev->callin_fvt->register_test((dev_object_t *)
                                                      pilp_dev);

    if (test_status != PASSED) {
        cterr('f', 0, "%s: 30W PoE DC Failed to do register test",
                      __FUNCTION__);
    } else {
        if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
            printf("passed.\n");
        }
    }

    return (test_status);
}


/***************************************************************************
 *
 * Name: build_poe_led_test
 *
 * Description: creating menu for LED tests
 *
 * Input: Show menu option
 *
 * Output: PASSED/FAILED
 *
 ***************************************************************************/
static int build_poe_led_test (int show_menu)
{

    build_primary_submenu(poe_led_submenu_table,
                            POE_LED_SUBMENU_TABLE_SZ,
                            "POE LED",
                            &poe_led_submenup);
    build_secondary_submenu(poe_led_submenu_table,
                            POE_LED_SUBMENU_TABLE_SZ,
                            poe_led_sec_items);
                            

    menu(&poe_led_subtest_menu, poe_led_sec_items, '\0');
    
    return (PASSED);

}


/***************************************************************************
 * Name: build_poe_utility_menu
 *
 * Description: Creating Utils menu for PoE
 *
 * Input: show menu option
 *
 * Output: PASSED/FAILED
 ***************************************************************************
 */
static int build_poe_utility_menu (int show_menu)
{
    menu(poe_util_submenup, poe_util_submenu_table, '\0');

    return (PASSED);
}

/**************************************************************************
 *
 * Name: ilp_err_report
 *
 * Description: Prints and logs error messages
 *
 * Input: none
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
static void ilp_err_report (dev_object_t *dev, char *err_msg, uint32 err_id)
{
    char *ilp_msg;

    ilp_msg = ilp_err_msg;

    /*  First,   dev_print to log err message    */
    /*mask the db_print, unmask when it is needed in the future */

    /* Get the message pass in from the device */
    sprintf(ilp_msg, "%s %s", err_msg,
                 (char *)pilp_dev->base.dev_object_fvt->dev_name);

    /*
     * Second, Call dev_show to get more info from device at the failed state.
     * Depend on err_id to determine what info to get from the device.
     * dev_print() function is used to log all debug info which do not cluster
     * the screen and be able to display in "showdebug" CLI command later
     */
    switch (err_id & ~FATAL) {
    case DEV_ILP_DEV_STATE:
    case DEV_ILP_REGISTER_TEST:
        break;
    /* Error Messages */
    case DEV_ILP_INVALID_MODE:
    case DEV_ILP_UNABLE_READ_CLASS:
    case DEV_ILP_UNABLE_DETECT:
    case DEV_ILP_POWER_STATUS:
    case DEV_ILP_UNABLE_POWER_ON:
    /* Warning Messages */
    case DEV_ILP_NOT_PRESENT:
    case DEV_ILP_MANUAL_MODE:
    case DEV_ILP_SKIPPED:
        break;
    default:
        cterr('f', 0, "%s", ilp_msg);
        break;
    }

    /* Third, log and send the message out in one shot */
    switch (err_id & FATAL) {
    case WARNING:
        cterr('w', 0, "%s", err_msg);
        break;
    case FATAL:
        cterr('f', 0, "%s", err_msg);
        break;
    case RETRY: /* Let Host handle when to print out */
        break;
    default:
        cterr('f', 0, "%s", err_msg);
        break;
    }
}

/**************************************************************************
 *
 * Name: poe_30w_write
 *
 * Description: perform i2c write for 30w poe. 
 *
 * Inputs: i2c_if - structure of i2c.
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
int poe_30w_write(n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"poe_30w_write: buf is null");
    }

    rc = n2g_i2c_write(i2c_if);

    if (rc != RC_I2C_OP_OK) {
        msleep(100);    /* Env MCU I2C cycle time */
        return (FAILED);
    }
    msleep(100);        /* Env MCU I2C cycle time */
    return(PASSED);
}


/**************************************************************************
 *
 * Name: poe_30w_read
 *
 * Description: perform i2c read for 30w poe. 
 *
 * Inputs: i2c_if - structure of i2c.
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
int poe_30w_read(n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"30W PoE read: buf is null");
    }

    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        /* Unable to read data */
        cterr('f', 0, "%s: 30W PoE DC unable to read data via I2C. rc = 0x%08x",
                      __FUNCTION__, rc);
        return(FAILED);
    }

    msleep(100);        /* I2C cycle time */
    return(PASSED);
}


/**************************************************************************
 *
 * Name: ilp_i2c_read_callout
 *
 * Description: This is a wrapper for the i2c read routine
 *
 * Inputs: 
 *         offset - i2c device offset
 *         dest - points to where the read data to be stored
 *         byte_count - number of data bytes to read
 *         verbose - print error messages if true
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
static uint32 ilp_i2c_read_callout (uchar offset, uchar *dest, int byte_count, 
                                    boolean verbose)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc;
    uint32_t data;
    
    rc = get_30w_poe_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get 30w POE I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    /* Fill the interface struct passed to I2C APIs */
    i2c_if.size             = byte_count;
    i2c_if.buf              = (char *)&data;
    i2c_if.offset           = offset;


    /* Perform I2C Read */
    rc = poe_30w_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: 30W PoE DC unable to read I2C, rc = %#x",
                      __FUNCTION__, rc);
        return (FAILED);
    }

    *dest = (uchar)data;

    return(rc);
}

/**************************************************************************
 *
 * Name: ilp_i2c_write_callout
 *
 * Description: This is a wrapper for the i2c write routine
 *
 * Inputs: 
 *         offset - i2c device offset
 *         dest - points to where the write data to be stored
 *         byte_count - number of data bytes to read
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
static uint32 ilp_i2c_write_callout (uchar offset, uchar *dest, int byte_count)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc;
    uint32_t data;
    
    rc = get_30w_poe_i2c_struct(&i2c_if);
    if (rc != PASSED) {
        printf("%s: Failed to get 30w POE I2C structure.\n",
              __FUNCTION__);
        return (rc);
    }

    data = (char)*dest;
    
    /* Fill the interface struct passed to I2C APIs */
    i2c_if.size             = byte_count;
    i2c_if.buf              = (char *)&data;
    i2c_if.offset           = offset;

    /* Perform I2C Write */
    rc = poe_30w_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to write I2C, status=%x", __FUNCTION__, rc);
        return (FAILED);
    }

    return (rc);

}


/* ******************************************************
 *
 * Function: ilp_dte_power_setup
 *
 * Description: Marvell switch firmware download for IP phone detection
 *  Note: This function assumes that Switch device has already been
 *        initialized.
 *
 * Input:    port  - Port number.
 *
 * Outputs:  PASSED/FAILED.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static inline int ilp_dte_power_setup (int port)
{
    printf("%s:%d Sorry, not support now.\n", __FUNCTION__, __LINE__);

    return (FAILED);
#if 0	
	/***** this is used for power detect ******/
    int ix;
    ushort marvell_reg;
    
    if (p88e6046->callout_fvt->mii_write_to_switch(
                               (dev_object_t *)p88e6046,
                               MRVL_PHY_DEV_ADDR + port, 
                               MRVL_PHY_OFF(phy_control), 
                               PHY_SPFC_CTRL1_ENABLE_DTE_DETECT
                               | PHY_SPFC_CTRL1_RESERVED |
                               PHY_SPFC_CTRL1_DISNLPGEN) == FAILED) {
        return (FAILED);
    }

    /* Make sure that the chip is not in reset state */
    for (ix = 0; ix < PHY_SOFT_RESET_TIMEOUT; ix++) {
        if (p88e6046->callout_fvt->mii_read_from_switch(
                                  (dev_object_t *)p88e6046, 
                                  (MRVL_PHY_DEV_ADDR + port),
                                   MRVL_PHY_OFF(phy_control), 
                                   &marvell_reg) == FAILED) {
            return (FAILED);
        }

        /* Check if the soft reset is done */
        if ((marvell_reg & PHY_SW_RESET) == 0) {
            /* Done */
            break;
        }

        msleep(5);   /* Poll every 5 msecs */
    }

    if (ix == PHY_SOFT_RESET_TIMEOUT) {
        cterr('f', 0, "PHY soft reset timedout on port %d", port);
        return (FAILED);
    }

    /* Refer to Marvell DTE Power Over Ethernet Appnote */
    if (p88e6046->callout_fvt->mii_write_to_switch(
                               (dev_object_t *)p88e6046,
                               MRVL_PHY_DEV_ADDR + port, 
                               MRVL_PHY_OFF(phy_spfc_ctrl1), 
                               PHY_SPFC_CTRL1_ENABLE_DTE_DETECT
                               | PHY_SPFC_CTRL1_RESERVED) == FAILED) {
        return (FAILED);
    }


    return (PASSED);
    
#endif
}


/* ******************************************************
 *
 * Function: ilp_phone_detect
 *
 * Description: Check if a Power Device (PD) is detected at a
 *              given port.
 *
 * Input:    port - Ports to check for the PD.
 *
 * Outputs:  TRUE - Found a PD.
 *           FALSE - PD not detected.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_phone_detect (uchar port)
{

    printf("\n %s: [Fix me!!] need to think how to do communction"
         " between Intel and Cavium\n", __FUNCTION__);
    return (FAILED);
#if 0
    int ix;
    ushort marvell_reg;
    char data;

    /* Bring out the Marvell switch from reset state */
    if (cpld_read_reg(CPLD_INTF_RESET_REG_LOW_OFFSET, &data) == FAILED) {
        return (FAILED);
    }
    data &= ~(CPLD_INTF_RESET_LOW_ETH_SW_RESET);
    if (cpld_write_reg(CPLD_INTF_RESET_REG_LOW_OFFSET, data) == FAILED) {
        return (FAILED);
    }   

    /* Bring up and attach switch driver first */
    mrv88e6046_device_attach();

    /* Disable PPU to read register */
    if (p88e6046->callin_fvt->disable_ppu((dev_object_t *)p88e6046) == FAILED) {
        return (FAILED);
    }

    msleep(200);

    /* Setup IP Phone Power detect */
    if (ilp_dte_power_setup(port) == FAILED) {
        return (FAILED);
    }

    /* Wait for 5 secs */
    ix = 5;
    do {
        printf("\rPort %d: %d seconds left", port, ix);
        msleep(ONE_SECOND);
    } while (ix--);

    /* Read the status now */
    if (p88e6046->callout_fvt->mii_read_from_switch(
                              (dev_object_t *)p88e6046, 
                              (MRVL_PHY_DEV_ADDR + port),
                               MRVL_PHY_OFF(phy_spfc_status), 
                               &marvell_reg) == FAILED) {
        return (FAILED);
    }

    /* Enable PPU */
    if (p88e6046->callin_fvt->enable_ppu((dev_object_t *)p88e6046) == FAILED) {
        return (FAILED);
    }

    /* Check if the phone is detected */
    if (marvell_reg & DTE_DETECTED) {
        return (TRUE);  /* Found Cisco PD */
    }

    /* No PD is detected */
    return (FALSE);
 
#endif
}


/* ******************************************************
 *
 * Function: toggle_ilp_green
 *
 * Description: Turn on/off ILP Green LED
 *
 * Input:    port - port number of the LED.
 *           on - TRUE to turn on, FALSE to turn off
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static void toggle_ilp_green (int port, int on)
{
    uint16_t  port_led, led_poe_crd_reg;

    if (port == 0) {
        port_led = POE_PWR_SUPPLY_UNIT1_LED_GREEN;
    } else {
        port_led = POE_PWR_SUPPLY_UNIT2_LED_GREEN;
    }

    led_poe_crd_reg = get_led_status(LED_CTRL_POE_PWR);

    led_poe_crd_reg |= port_led;

    
    if (on) {
        /* Turn on LED */
        set_led_reg(LED_CTRL_POE_PWR, led_poe_crd_reg);
    } else {
        /* Turn off LED */
        set_led_off(LED_CTRL_POE_PWR, port_led);
    }
}


/* ******************************************************
 *
 * Function: toggle_ilp_yellow
 *
 * Description: Turn on/off ILP Yellow LED
 *
 * Input:    port - port number of the LED.
 *           on - TRUE to turn on, FALSE to turn off
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static void toggle_ilp_yellow (int port, int on)
{
    char    port_led, led_poe_crd_reg;

    if (port == 0) {
        port_led = POE_PWR_SUPPLY_UNIT1_LED_YELLOW;
    } else {
        port_led = POE_PWR_SUPPLY_UNIT2_LED_YELLOW;
    }
    
    led_poe_crd_reg = get_led_status(LED_CTRL_POE_PWR);

    led_poe_crd_reg |= port_led;
    
    if (on) {
        /* Turn on LED */
        set_led_reg(LED_CTRL_POE_PWR, led_poe_crd_reg);
    } else {
        /* Turn off LED */
        set_led_off(LED_CTRL_POE_PWR, port_led);
    }

}


/* ******************************************************
 *
 * Function: toggle_ilp_led_off
 *
 * Description: Turn off ILP LED
 *
 * Input:    port - port number of the LED.
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static void toggle_ilp_led_off (int port)
{
    char    port_led;

    if (port == 0) {
        port_led = MSK_POE_PWR_SUPPLY_UNIT1_LED;
    } else {
        port_led = MSK_POE_PWR_SUPPLY_UNIT2_LED;
    }
    
    set_led_off(LED_CTRL_POE_PWR, port_led);
}


/* ******************************************************
 *
 * Function: ilp_led
 *
 * Description: POE ILP LED I/O.
 *
 * Input:    port - port number of the LED.
 *           led_color - ENUM of  LED as per defs.h
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static void ilp_led (int port, int led_color) 
{

    switch (led_color) {
    case ILP_LED_OFF:
        toggle_ilp_led_off(port);
        break;
    case ILP_YELLOW_LED:
        toggle_ilp_yellow(port, TRUE);
        break;
    case ILP_GREEN_LED:
        toggle_ilp_green(port, TRUE);
        break;
    default:
        cterr('f',0, "No such LED  color = %d for port %d", 
                     led_color, port);
        break;
    }
}



/* ******************************************************
 *
 * Function: ilp_get_max_port
 *
 * Description: Get number of ports on POE.
 *
 * Input:    None
 *
 * Outputs:  Returns number of ports.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_get_max_port (void)
{
    return (MAX_PORTS_PER_ILP);
    
}


/**************************************************************************
 *
 * Name: ilp_device_attach
 *
 * Description: Inits
 *
 * Input: dev - pointer to object structure
 *
 * Output: none
 *
 *************************************************************************/
static void ilp_device_attach (dev_ilp_object_t * const dev)
{
    dev_ilp_create((dev_object_t *)dev, ilp_err_report);

    dev->base.dev_object_fvt->dev_attach((dev_object_t *)dev);

    dev->callout_fvt->i2c_read = ilp_i2c_read_callout;
    dev->callout_fvt->i2c_write = ilp_i2c_write_callout;
    dev->callout_fvt->phone_detect = ilp_phone_detect;
    dev->callout_fvt->get_max_port = ilp_get_max_port;
    dev->callout_fvt->ilp_led = ilp_led;
}


/*******************************************************************************
 *
 * Function   : ovld_reset_ilp
 * Description: To reset 30W PoE DC(ILP) via Overlord FPGA.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int ovld_reset_ilp (void)
{
    /* Bring the PoE into reset state */
    reset_platform_ext_dev(FPGA_EXT_POE_RST);

    /* Sleep for awhile */
    msleep(100);

    /* Release it from reset state */
    unreset_platform_ext_dev(FPGA_EXT_POE_RST);
    
    msleep(100);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: ovld_poe_init
 *
 * Description: Initialize PoE related data structure
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int ovld_poe_init (void)
{   
    /* Reset the ILP */
    if (ovld_reset_ilp() != PASSED) {
        return (FAILED);
    }

    /* Attach ILP Device Driver */
    ilp_device_attach(pilp_dev);

    /* Check if ILP is present */
    if (pilp_dev->callin_fvt->is_ilp_present((dev_object_t *)pilp_dev)
                              == FALSE) {
        /* Inline Power Card is not present. Turn off LEDs */
        return (FAILED);
    }

    /* Init ILP */
    if (pilp_dev->callin_fvt->ilp_init((dev_object_t *)pilp_dev) == FAILED) {
        return (FAILED);
    }

    /*  ILP is detected. Turn on all LEDs */

    return (PASSED);
}


/**********************************************************************
 *
 * Function: ilp_int_handler
 *
 * Description: Interrupt Handler for ILP
 *
 * Inputs: None
 *
 * Outputs: None
 *
 **********************************************************************
 */
#if 0
void ilp_int_handler (void)
{
    ilp_interrupt_received = TRUE;
}
#endif 


/*******************************************************************************
 *
 * Function   : is_poe_present
 * Description: Check if Powerball is detected or not.
 *              There is no presence pin to determine whether the module
 *              is present or not, so I2C dummy read will be applied
 *              instead.
 * Inputs     : None
 * Outputs    : TRUE - present, FALSE - not present
 *
 *******************************************************************************
 */
int is_poe_present (char *buf)
{
    n2g_i2c_if_t i2c_if;
    uint32_t data, ret_val = FAILED;

    ret_val = get_30w_poe_i2c_struct(&i2c_if);
    if (ret_val != PASSED) {
        sprintf(buf, "can't to get 30w POE I2C structure (rc = %#x)",
                ret_val);
        return (FALSE);
    }

    if (ovld_reset_ilp() != PASSED) {
        sprintf(buf, "can't reset 30w POE DC.");
        return (FALSE);
    }

    /* Fill the interface struct passed to I2C APIs */
    i2c_if.buf              = (char *)&data;
    i2c_if.offset           = 0;

    /* Perform I2C Read */
    ret_val = n2g_i2c_read(&i2c_if);
    if (ret_val != PASSED) {
        sprintf(buf, "can't read data from 30W PoE DC (rc = %#x)",
                ret_val);
        return (FALSE);
    }

    return (TRUE);
}


/**********************************************************************
 *
 * Function: build_30w_poe_menu
 *
 * Description: Build PoE menu.
 *
 * Inputs: show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int build_30w_poe_menu (int show_menu)
{

    /* unreset the i2c bit 8, for accessing ilp. */
    unreset_platform_in_dev(FPGA_IN_I2C_8_RST);

    build_primary_submenu(poe_submenu_table, POE_SUBMENU_TABLE_SZ,
              "PoE Menu", &poe_submenup);
    build_secondary_submenu(poe_submenu_table, POE_SUBMENU_TABLE_SZ,
                poe_sec_items);

    testname("PoE");

    if (ovld_poe_init() == FAILED) {
        return (FAILED);
    }
   
    if (show_menu) {
        /* Entered with submenu */
        menu(&poe_subtest_menu, poe_sec_items, 0);
    } else {
        do_all_menu_items(poe_submenup);

        if (diagflag_xram & D_XEC_AUTH) { /* For MFG */
            smartchip_authenticate_retest(DAUGHTER_CARD, 0);
        }
    }

    return (PASSED);
} 


/******** History ******** 
$Log: platform_30w_poe.c,v $
Revision 1.8  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.7  2014/01/31 23:57:38  mcharon
support conitnue mode when reading from device

Revision 1.6  2014/01/29 20:35:58  mcharon
allow user to show more than one byte

Revision 1.5  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.4  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.3  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.2  2013/05/23 01:09:25  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.10  2013/03/17 02:04:14  mcharon
support command line for testing 30w poe

Revision 1.9  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.8  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.7  2012/11/07 10:58:15  alpeng
remove useless file and clean up code

Revision 1.6  2012/09/26 18:36:09  palin2
Update the print out format of 30W PoE DC defult tests.

Revision 1.5  2012/08/07 09:44:29  palin2
Clean up function "is_poe_present".

Revision 1.4  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/

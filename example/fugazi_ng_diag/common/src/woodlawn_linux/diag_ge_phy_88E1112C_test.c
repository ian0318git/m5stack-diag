/* $Id: diag_ge_phy_88E1112C_test.c,v 1.2 2013/10/08 08:48:28 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_ge_phy_88E1112C_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_ge_phy_88E1112C_test.c - Menu for Woodlawn PHY 88E1112C
 *
 * January 2013, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "ethernet.h"
#include "common_utils.h"
#include "diag_ge_phy_88E1112C_lib.h"
#include "platform_eth.h"
#include "diag_common_drv.h"
#include "diag_fpga_lib.h"
#include "smi_api.h"
#include "dev_mrvl_ge.h"
#include "platform_ext_lpbk.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

extern uint32 err_report(dev_object_t *, char *, uint32);
extern int mrvl_ge_dev_create(dev_object_t *, dev_error_report_t);
extern int switch_fiber(char *, int, boolean);
extern void msleep(unsigned long);

static int ge_phy_88E1112C_utility(int);
static int ge_phy_88E1112C_register_test(void);
int ge_phy_88E1112C_loopback_test(int);
int ge_88E1112C_do_all_wrapper(void);
static int dump_phy_88E1112C_registers(void);
static int alter_phy_88E1112C_register(void);
void dev_88e1112c_create(dev_mrvl_ge_object_t *, smi_if_t *);

int ge_phy_init(void);
int ge_phy_88E1112C_test(int);
int set_88e1112c_phy_int_lpbk(char *, int, int);
int set_88e1112c_mac_speed(int, int);

static int mrvl_1112_eth_qlm0_list[] = {SGMII0 , SGMII1, SGMII2, SGMII3};
static int sfp_speed_list[] = { SPD_1000MBPS }; /* fiber only test 1000Mpbs*/

/* Sub Menu used for GE phy 88E1112C tests.*/
static submenu_xtable_t ge_phy_88E1112C_tests_submenu_table[] = {
    {"PHY Utilities", (type_t(*)())ge_phy_88E1112C_utility,   FALSE,
        0, NULL, 0, (type_t(*)())ge_phy_88E1112C_utility,   TRUE}, 
    {"PHY Register Test", (type_t(*)())ge_phy_88E1112C_register_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Internal Loopback Test", (type_t(*)())ge_phy_88E1112C_loopback_test,  LOOPBACK_PHY,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GE_PHY_88E1112C_TESTS_SUBMENU_TABLE_SIZE (sizeof(ge_phy_88E1112C_tests_submenu_table) / \
            sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gy_phy_88E1112C_tests_primary_items[GE_PHY_88E1112C_TESTS_SUBMENU_TABLE_SIZE +
                     MAX_BASE_ITEMS];
static mitem_t ge_phy_88E1112C_tests_secondary_items[GE_PHY_88E1112C_TESTS_SUBMENU_TABLE_SIZE +
                     MAX_BASE_ITEMS];

menuinfo_t ge_phy_88E1112C_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    gy_phy_88E1112C_tests_primary_items,
};
menuinfo_t *ge_phy_88E1112C_submenup = &ge_phy_88E1112C_subtest_menu;

/* List of GE phy 88E1112C Utilities */
static submenu_xtable_t ge_phy_88E1112C_util_items[] = {
    {"Dump PHY Registers", (type_t(*)())dump_phy_88E1112C_registers, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Alter PHY Registers", (type_t(*)())alter_phy_88E1112C_register, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define GE_PHY_88E1112C_TESTS_UTIL_SIZE (sizeof(ge_phy_88E1112C_util_items) / \
                                     sizeof(submenu_xtable_t))

/*
 * ge phy 88E1112C util items (filled in from xtable)
 */
static mitem_t ge_phy_88E1112C_tests_primary_util_items[GE_PHY_88E1112C_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t ge_phy_88E1112C_tests_secondary_util_items[GE_PHY_88E1112C_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];

/*
 * GE phy 88E1112C Utils submenu
 */
menuinfo_t ge_phy_88E1112C_util_menu = {
    "GE PHY 88E1112 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    ge_phy_88E1112C_tests_primary_util_items,
};

menuinfo_t *ge_phy_88E1112C_util_menup = &ge_phy_88E1112C_util_menu;


/******************************************************************************
 *
 * Function: ge_phy_88E1112C_test
 *
 * Description: Main entrance for 88E1112C menu
 *
 * Inputs      : port - port number
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************/
int ge_phy_88E1112C_test (int show_menu)
{
    build_primary_submenu(ge_phy_88E1112C_tests_submenu_table,
                        GE_PHY_88E1112C_TESTS_SUBMENU_TABLE_SIZE,
                        "GE PHY 88E1112C", &ge_phy_88E1112C_submenup);
    build_secondary_submenu(ge_phy_88E1112C_tests_submenu_table,
                        GE_PHY_88E1112C_TESTS_SUBMENU_TABLE_SIZE,
                        ge_phy_88E1112C_tests_secondary_items);

    if (show_menu) {
        menu(ge_phy_88E1112C_submenup, ge_phy_88E1112C_tests_secondary_items, '\0' );

    } else {
        menu_exec_doall_diags(ge_phy_88E1112C_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : ge_88E1112C_do_all_wrapper
 * Description : Wrapper for GE PHY 88E1340 do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int ge_88E1112C_do_all_wrapper (void)
{
    int rc = PASSED;

    if (ge_phy_88E1112C_register_test() == FAILED) {
        rc = FAILED;
    }

    if (ge_phy_88E1112C_loopback_test(LOOPBACK_PHY) == FAILED) {
        rc = FAILED;
    }

    return (rc);
}

/*********************************************************************
 * 
 * Function: dev_obj_create
 *
 * Description: Create GE PHY device object for Common Device Driver
 *
 * Inputs: mrvl_p - Points to GE PHY device object
 *
 * Outputs: None
 *
 *********************************************************************
 */
void dev_88e1112c_create (dev_mrvl_ge_object_t *mrvl_p, smi_if_t *smi_p)
{
    dev_object_t *dev = (dev_object_t *)mrvl_p;
    int rc;

    /* Setup device struct */
    mrvl_p->reg_info_p = 0;    /* Use default table */

    /* Create common device object */
    rc = mrvl_ge_dev_create(dev, (dev_error_report_t) err_report);

    /* Setup call-out function vectors */
    mrvl_p->callout_fvt->rd = woodlawn_88e1112c_smi_read;
    mrvl_p->callout_fvt->wr = woodlawn_88e1112c_smi_write;
    mrvl_p->callout_fvt->open  = woodlawn_88e1112c_smi_open;
    mrvl_p->callout_fvt->close = woodlawn_88e1112c_smi_close;
    mrvl_p->callout_fvt->sfp_op = woodlawn_88e1112c_sfp_operation;
    mrvl_p->callout_fvt->sfp_setup = 0; /* Default to no special SFP setup */

    mrvl_p->smi_p = smi_p;

    smi_p->offset = 0;
    mrvl_p->type  = MRVL_GE_PHY_1112;
}

/******************************************************************************
 *  
 * Function: ge_phy_88E1112C_register_test
 *    
 * Description: This function performs the 88E1112C register test.
 *      
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *         
 ******************************************************************************/
static int ge_phy_88E1112C_register_test (void)
{
    dev_mrvl_ge_object_t *mrvl_obj;
    dev_object_t *dev;
    int rc;

    testname("88E1112C PHY Register");

    /* Create the device object */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112c_obj(WOODLAWN_88E1112C_PHY_ID);
    if (mrvl_obj == NULL) {
        cterr('f', 0, "%s: Mrvl 88e1112c Null Object", __FUNCTION__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* Test the registers */
    rc = mrvl_obj->callin_fvt->register_test(dev);

    if (rc != PASSED) {
        cterr('f', 0, "Register test failed");
        return (rc);
    }

#ifdef DEBUG_DETACH
    /* detach calls init_default_dev_object which resets dev_destroy to
     * dev_destroy_default. Since we need destroy to free malloc'ed memory,
     * bypass the detach.
     */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */

    /* Registers test passed. Check for detach return code */
    if (rc != PASSED) {
            cterr('f', 0, "Device detach failed");
            return (rc);
        }
#endif /* DEBUG_DETACH */

    rc = ge_phy_init();
    if (rc != PASSED) {
        cterr('f', 0, "GE PHY init failed.");
        return (rc);
    }

    return (PASSED);
}

/*********************************************************************
 *   
 * Function: ge_phy88E1112C_loopback_test
 *    
 * Description: Perform 88E1112C loopback test
 *      
 * Inputs: lpbk_flag - set loopback mode
 *        
 * Outputs: PASSED/FAILED
 *          
 **********************************************************************/
int ge_phy_88E1112C_loopback_test (int lpbk_flag)
{
    int rc = PASSED, phy_id_1112c, lpbk_mode, bus_id;
    int speed, port, val;
    dev_mrvl_ge_object_t *mrvl_obj;
    dev_object_t *dev;
    char cmd_str[32];

    port = mrvl_1112_eth_qlm0_list[3];    /* port - eth number */
    phy_id_1112c = WOODLAWN_88E1112C_PHY_ID;
    bus_id = SMI_BUS_3;
    
    /* set loopback mode */
    lpbk_mode = lpbk_flag;

    sprintf(cmd_str, "ifconfig eth%d down", port);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig eth%d up", port);
    system(cmd_str);

    if (lpbk_mode == LOOPBACK_PHY) {
        testname("88E1112C Internal loopback");
    } else {
        testname("Backplane loopback");
    }

    /* Do GE PHY unreset */
    ge_phy_reset(ENABLE);    /* reset */
    msleep(MRV88E111N_RESET_TIMEOUT);
    ge_phy_reset(DISABLE);    /* unrest */
    msleep(MRV88E111N_RESET_TIMEOUT);
    
    /* Ensure the cavium is not in loopback mode(follow 1340 internal loopback) */
    set_sgmii_int_lpbk(port, FALSE);

    sleep(1);

    /* Create the device object */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112c_obj(WOODLAWN_88E1112C_PHY_ID);
    if (mrvl_obj == NULL) {
        cterr('f', 0, "%s: Mrvl 88e1112c Null Object", __FUNCTION__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;
    /* Set SGMII MAC Interface Output Amplitude
     * to 1.00V per Programmers Reference Manual
     */
    /* set page */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111N_PAGE_ADDRESS_REG,
                                MRV88E111N_REG_PAGE_2);
    if (rc != PASSED) {
        cterr('f', 0, "Set page-%d SGMII Output Amplitude failed", 
            MRV88E111N_REG_PAGE_2);
        return (rc);
    }
        
    /* write register */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111M_SPECIFIC_CONTROL2_REG,
                                MAC_SP_CTL2_AMP_1);
    if (rc != PASSED) {
        cterr('f', 0, "Set reg-%d SGMII Output Amplitude failed", 
            MRV88E111M_SPECIFIC_CONTROL2_REG);
        return (rc);
    }

    rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111M_SPECIFIC_CONTROL2_REG,
                                MAC_SP_CTL2_AMP_1);
    if (rc != PASSED) {
        cterr('f', 0, "Set reg-%d SGMII Output Amplitude failed",
            MRV88E111M_SPECIFIC_CONTROL2_REG);
        return (rc);
    }

    /* Configure media into Fiber mode */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111M_SPECIFIC_CONTROL1_REG,
                             0x188);
    if (rc != PASSED) {
        cterr('f', 0, "Set reg-%d SGMII Output Amplitude failed",
            MRV88E111M_SPECIFIC_CONTROL2_REG);
        return (rc);
    }

    /* Turn on loopback bit if it is internal loopback */
    if (lpbk_mode == LOOPBACK_PHY) {
        /* Change page to fiber mode */
        rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111N_PAGE_ADDRESS_REG, 1);
        if (rc != PASSED) {
            cterr('f', 0, "Write %#x to MAC Control failed", val);
            return (rc);
        }
        /* Read MAC Control Register for Fiber Mode */
        rc = woodlawn_phy_reg_rd(bus_id, phy_id_1112c, MRV88E111F_CONTROL_REG, &val);
        if (rc != PASSED) {
            cterr('f', 0, "Read MAC Control Failed");
            return (rc);
        }

        val |= MRV88E111N_CONTROL_LOOPBACK;

        rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111F_CONTROL_REG, val);
        if (rc != PASSED) {
            cterr('f', 0, "Write %#x to MAC Control failed", val);
            return (rc);
        }
    }

    speed = sfp_speed_list[0];

    /* Set packet from the packet table and call tx_rx_diag to transfer it */
    rc = sgmii_set_packet(port, speed);
    if (rc == FAILED) {
        cterr('f', 0, "port-%d, speed-%d set packet failed", port, speed);
        return (rc);
    }

    /* Turn off loopback bit if it is internal loopback */
    if (lpbk_mode == LOOPBACK_PHY) {
        /* Change page to fiber mode */
        rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111N_PAGE_ADDRESS_REG, 1);
        if (rc != PASSED) {
            cterr('f', 0, "Write %#x to MAC Control failed", val);
            return (rc);
        }
        /* Read MAC Control Register for Fiber Mode */
        rc = woodlawn_phy_reg_rd(bus_id, phy_id_1112c, MRV88E111F_CONTROL_REG, &val);
        if (rc != PASSED) {
            cterr('f', 0, "Read MAC Control Failed");
            return (rc);
        }

        val &= ~MRV88E111N_CONTROL_LOOPBACK;

        rc = woodlawn_phy_reg_wr(bus_id, phy_id_1112c, MRV88E111F_CONTROL_REG, val);
        if (rc != PASSED) {
            cterr('f', 0, "Write %#x to MAC Control failed", val);
            return (rc);
        }
    }

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: set_88e1112c_phy_int_lpbk
 *  we setup media PHY first which will let cavium know the current 
 *  setting of diag. Then to initial and setup loopback type on 
 *  bridge PHY.
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 * 
 *------------------------------------------------------------------
 */
int set_88e1112c_phy_int_lpbk (char *type, int port, int speed)
{
    char pname[10];
    int rc = PASSED;
    int phy_id_1112c;
    phy_id_1112c = WOODLAWN_88E1112C_PHY_ID;
    
    sprintf(pname,"%s%d", type, port);   

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, INT_LPBK)) != PASSED) {
        cterr('f', 0, "%s : init_sgmii_env failed", pname);
        return(FAILED);
    }

    /* turn off stub loopback to prevent the packet is loop lpbk stub*/
    if ((rc = set_phy_stub(pname, INT_LPBK, SIG_COPPER)) != PASSED) {
        cterr('f', 0, "%s :  set_phy_stub failed", pname); 
        return(FAILED);
    }
    
    /* without loopback stub, the PHY should forced link up here
     * to let setting of init_sgmii_env is working properly.
     */
    if (force_linkup(ENABLE, phy_id_1112c)) {   
        cterr('f', 0, "%s : force_linkup failed.", pname);
        return (FAILED);
    }

    /* turn off half duplex bits, turn on full duplex bit and do soft reset */
    /* ensure the advertisement register is not turn on half duplex */
    if (sgmii_adv_full_duplex(ENABLE, phy_id_1112c)) {
        cterr('f', 0, "%s : sgmii_adv_full_duplex failed", pname);
        return FAILED;
    } 

    /* 20130122-this function "set_bridge_phy_mode" seems just for 88E1340 */
    /* set advertisment reg and PHY mode  for bridge PHY */
    /*if ((rc = set_bridge_int_lpbk(ENABLE_SIG, phy_id_1340)) != PASSED) {
        cterr('f', 0, "%s set_bridge_int_lpbk failed", pname); 
        return (FAILED);
    }*/   

    /* 20130122-88E1112C use different register to set mac speed */
    if (set_88e1112c_mac_speed(phy_id_1112c, speed)) {
        cterr('f', 0, "%s set mac speed bridge PHY failed.", pname);
        return (FAILED);
    }
    
    /* soft reset bridge PHY makes setting of mac speed is work */
    if (direct_phy_soft_reset(phy_id_1112c)) {
        cterr('f', 0, "%s reset bridge PHY failed.", pname);
        return (FAILED);
    }
    
    /* set birdge PHY and also turn on the loopback bit. */
    if (set_bridge_phy_speed(phy_id_1112c, speed)) {
        cterr('f', 0, "%s set bridge PHY speed failed.", pname);
        return (FAILED);
    }   
    
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_88e1112c_mac_speed
 * set mac speed. PHY 1112C use 
 * Reg 0 on Page 2. need follow by soft reset. 
 *  
 * Input:  phy_id - phy addr for setup port
 *         speed - setup speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_88e1112c_mac_speed (int phy_id, int speed)
{
    int rdval, wrval;
   
    woodlawn_phy_reg_wr(WOODLAWN_88E1112C_SMI_BUS, phy_id, MRVL_1112C_PHY_PAGE22,
                        MRVL_1112C_PHY_PAGE2);
    woodlawn_phy_reg_rd(WOODLAWN_88E1112C_SMI_BUS, phy_id, MAC_CTRL_REG0, &rdval);

    rdval = rdval & ~0x2040; /* clean up the speed bit 6, 13 */

    switch(speed) {
        case SPD_10MBPS:
            wrval = rdval | 0x0000;
            break;
        case SPD_100MBPS:
            wrval = rdval | 0x2000;
            break;
        case SPD_1000MBPS:
            wrval = rdval | 0x0040;
            break;
        default:  
            cterr('f', 0, "phy_id %d not support speed %d on MAC", phy_id, speed);
            break;
    }

    woodlawn_phy_reg_wr(WOODLAWN_88E1112C_SMI_BUS, phy_id, MAC_CTRL_REG0, wrval);
    sleep(ETH_DRIVER_DELAY);       //can not be mask bridge PHY will failed
    woodlawn_phy_reg_rd(WOODLAWN_88E1112C_SMI_BUS, phy_id, MAC_CTRL_REG0, &rdval);

    if(rdval < 0) {
        return (FAILED);
    } else { 
        return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: ge_phy_init
 *
 * Description: Initialize the GE PHY.
 *
 * Inputs: void.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int ge_phy_init (void)
{
    dev_mrvl_ge_object_t *mrvl_obj;
    dev_object_t *dev;
    int rc;
#ifdef DEBUG_DETACH
    int rc_det;
#endif /* DEBUG_DETACH */

    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112c_obj(WOODLAWN_88E1112C_PHY_ID);

    if (mrvl_obj == NULL) {
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    rc = mrvl_obj->base.dev_object_fvt->dev_init(dev);

    /* detach calls init_default_dev_object, which reset dev_destroy */
#ifdef DEBUG_DETACH
    rc_det = mrvl_obj.base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
#endif /* DEBUG_DETACH */
    if (rc != PASSED) {
        cterr('f', 0, "ge_phy_init() GE PHY initialization failed");
#ifdef DEBUG_DETACH
    } else {
        /* Enable/Disable successful. Check detach return code */
        if (rc_det != PASSED) {
            cterr('f', 0, "ge_phy_init() Device detach failed");
            return (rc_det);
        }
#endif /* DEBUG_DETACH */
    }

    return (rc);
}

/***********************************************************************
 *
 * Function: dump_phy_88E1112C_registers
 *
 * Description: Display GE PHY registers
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 ***********************************************************************
 */
static int dump_phy_88E1112C_registers (void)
{
    dev_mrvl_ge_object_t *mrvl_obj;
    dev_object_t *dev;
    int rc;

    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112c_obj(WOODLAWN_88E1112C_PHY_ID);

    if (mrvl_obj == NULL) {
        printf("%s: Mrvl 88e1112c Null Object", __FUNCTION__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* Display the registers */
    rc = mrvl_obj->base.dev_object_fvt->dev_show(dev, (print_fn_t)&printf, 
                                                DEV_SHOW_REGISTERS);
    
    if (rc != PASSED) {
        printf("Dump registers failed\n");
        return (rc);
    }
    return (PASSED);
}

/***********************************************************************
 *  
 * Function: alter_phy_88E1112C_register
 *    
 * Description: Alter GE PHY registers
 *      
 * Inputs: None
 *        
 * Outputs: PASSED/FAILED
 *          
 ************************************************************************/
static int alter_phy_88E1112C_register (void)
{
    dev_mrvl_ge_object_t *mrvl_obj;
    dev_object_t *dev;
    int rc;

    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112c_obj(WOODLAWN_88E1112C_PHY_ID);

    if (mrvl_obj == NULL) {
        cterr('f', 0, "%s: Mrvl 88e1112c Null Object", __FUNCTION__);
        return (FAILED);
    }
    
    dev = (dev_object_t *)mrvl_obj;

    /* Callin to alter the registers */
    rc = mrvl_obj->callin_fvt->alter_reg(dev);

    if (rc != PASSED) {
        printf("Alter register failed\n");
        return(rc);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : ge_phy_88E1112C_utility
 * Description :
 * Inputs      :  menu_option - display menu instead of running all temp. sensor tests.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */

static int ge_phy_88E1112C_utility (int show_menu)
{
    build_primary_submenu(ge_phy_88E1112C_util_items, GE_PHY_88E1112C_TESTS_UTIL_SIZE,
                          "GE PHY 88E1112C Utilities Menu", &ge_phy_88E1112C_util_menup);
    build_secondary_submenu(ge_phy_88E1112C_util_items, GE_PHY_88E1112C_TESTS_UTIL_SIZE,
                            ge_phy_88E1112C_tests_secondary_util_items);

    if (show_menu) {
        menu(ge_phy_88E1112C_util_menup, ge_phy_88E1112C_tests_secondary_util_items, '\0' );
    } else {
        menu_exec_doall_diags(ge_phy_88E1112C_util_menup);
    }

    return (PASSED);
}
/*-------------------------------------------------
 * $Log: diag_ge_phy_88E1112C_test.c,v $
 * Revision 1.2  2013/10/08 08:48:28  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:16  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.8  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.7  2013/03/29 08:50:12  leslie
 * Add commetn to each function
 *
 * Revision 1.6  2013/03/27 08:45:05  kuangik
 * Code cleanup
 *
 * Revision 1.4  2013/03/21 00:47:31  kuangik
 * Bring down and up interface in the beginning of test
 *
 * Revision 1.5  2013/03/12 11:24:56  leslie
 * Fix submenu flag for show error number message
 *
 * Revision 1.4  2013/03/07 02:24:04  kuangik
 * Add Show error count
 *
 * Revision 1.2  2013/02/19 00:44:27  leslie
 * Add and fix 88e1112c code
 *
 * Revision 1.1  2013/01/16 02:33:52  leslie
 * Add Woodlawn PHY 88E1112C test.
 *
 * $Endlog$
 *-------------------------------------------------
 */

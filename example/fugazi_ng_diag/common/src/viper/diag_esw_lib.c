 /* $Id: diag_esw_lib.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_esw_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_esw_lib.c - ESW functions library
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proto.h"
#include "common.h"
#include "types.h"
#include "ethernet.h"
#include "diag_esw_lib.h"
#include "diag_esw_test.h"
#include "dnv_eth_lib.h"
#include "dev_88e6176.h"
#include "diag_fpga.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_gephy_test.h"
#include "nvmonvars.h"

int diag_esw_init(void);
int diag_reset_esw_to_default(int);
static int diag_esw_smi_rd(int, ushort *); 
static int diag_esw_smi_wr(int, ushort); 
static int diag_esw_ext_lpbk_test(void); 

extern uint32 err_report(dev_object_t *, char *, uint32);

int diag_esw_dev_create(dev_88e6176_object_t *);

/*******************************************************************************
 *
 * Function    : diag_esw_dev_create
 * Description : Function to create 88E6176 Device Object
 * Inputs      : esw_obj - Pointer of 88E6176 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_esw_dev_create (dev_88e6176_object_t *esw_obj)
{
    dev_object_t *dev = (dev_object_t *)esw_obj;

    /* Create common device object */
    mrv88e6176_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }   

    /* Attach the device */
    esw_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    esw_obj->callout_fvt->rd = diag_esw_smi_rd;
    esw_obj->callout_fvt->wr = diag_esw_smi_wr;
    esw_obj->callout_fvt->sgmii_lpbk_test = diag_esw_ext_lpbk_test;

    return (PASSED);
}



/*******************************************************************************
 *
 * Function    : diag_esw_smi_rd 
 * Description : Function to read Ethernet Switch Register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static int diag_esw_smi_rd (int regnum, ushort *buf) 
{
    return (dnv_read_phy_reg(DNV_LAN1_PORT0, VIPER_6176_PHY_ADDR,
                            regnum, buf));
}


/*******************************************************************************
 *
 * Function    : diag_esw_smi_wr
 * Description : Function to write ethernet switch register through SMI
 * Inputs      : regnum - Register Address
 *               regval - Data to be written to ESW register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static int diag_esw_smi_wr (int regnum, ushort regval) 
{
    return (dnv_write_phy_reg(DNV_LAN1_PORT0, VIPER_6176_PHY_ADDR,
                              regnum, regval));
}


/*******************************************************************************
 *
 * Function    : diag_esw_ext_lpbk_test
 * Description : Function to do external loopback
 * Inputs      : regnum - Register Address
 *               regval - Data to be written to ESW register
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static int diag_esw_ext_lpbk_test (void) 
{
    if (this_is_viper_j()) {
        return (eth_pkt_txrx(VIPERJ_88E6176_IFACE_NAME, LPBKTEST_PKT_CNT, FALSE));
    } else {
        return (eth_pkt_txrx(VIPER_88E6176_IFACE_NAME, LPBKTEST_PKT_CNT, FALSE));
    }
}

/*******************************************************************************
 *
 * Function   : diag_esw_init
 * Description: Function to init Viper switch(Marvell 88E6176).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_esw_init (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int    ctr = 0, rc;
    int    cpu_port = 0;
    int    start_port = 0, end_port = 0;

    /* Create 88e6176 device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Release ESW from reset if needed */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Power Up all GE ports */
    /* Viper ESW has 4 GE ports: port0 ~ 3. */
    start_port = ESW_PORT0;
    end_port = ESW_PORT3;

    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (esw_obj_p->callin_fvt->esw_pwr_up_ge_port((dev_object_t *)esw_obj_p, 
            ctr) != PASSED) {
            printf("%s:%d Failed to power up ESW port%d.",
                   __FUNCTION__, __LINE__, ctr);
            goto _exit;
        }
    } 

    /* Power up ESW to CPU SERDES port:
     * Viper: ESW port5(but SERDES register addr. 0xF)
     */
    cpu_port = (int)(ESW_CPU_PORT_ADDR);

    if (esw_obj_p->callin_fvt->esw_pwr_up_cpu_port((dev_object_t *)esw_obj_p, 
        cpu_port) != PASSED) {
        printf("%s:%d Failed to power up ESW serdes port%d.",
                __FUNCTION__, __LINE__, ctr);
        goto _exit;
    }
    msleep(20);

    /* Set ESW all ports forwarding */
    /* Set parameters based on board SKU type */
    start_port = (int)(ESW_PORT_REG_BASE + ESW_PORT0);
    end_port = (int)(ESW_PORT_REG_BASE + ESW_PORT6);
    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (esw_obj_p->callin_fvt->esw_set_port_forward((dev_object_t *)esw_obj_p, 
            ctr) != PASSED) {
            printf("%s:%d Failed to set forward ESW port%d.",
                   __FUNCTION__, __LINE__, ctr);
            goto _exit;
        }
    }

    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (PASSED);

_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (FAILED);
}


/*******************************************************************************
 *
 * Function   : diag_reset_esw_to_default
 * Description: Function to reset Viper switch and re-init it.
 * Inputs     : quiet_opt - To print message(opt = FALSE) or not(opt = TRUE)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_reset_esw_to_default (int quiet_opt) {
    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("This function will reset Viper switch and re-init it.\n");
    }

    /* 1. Reset Viper swtich. */
    /* 1-1. Put switch in Reset. */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, TRUE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to put switch in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    msleep(ESW_RESET_ONE_SEC);

    /* 1-2. Release switch from Reset. */
    if (fpga_reset_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
        WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("Viper switch is reset successfully.\n");
    }

    /* 2. Re-init Viper switch. */
    if (diag_esw_init() != PASSED) {
        printf("%s: Failed to init Viper ethernet switch.\n", __FUNCTION__);
        return (FAILED);
    }

    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("Viper switch is reset and re-inited successfully.\n");
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_esw_lib.c,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.2  2018/03/28 07:03:51  lucywang
 * Added API to check SKU ViperJ and changed interface name for ViperJ
 *
 * Revision 1.1.2.1  2018/02/27 08:06:34  harrchan
 * Initial viper application code base
 *
 * $Endlog$
 *-------------------------------------------------
 */

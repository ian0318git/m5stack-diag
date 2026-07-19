/* $Id: diag_esw_util.c,v 1.2 2019/12/11 10:10:28 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_esw_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_esw_util.c - This file is for ethernet switch utility
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "types.h"
#include "queryflags.h"
#include "ethernet.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_esw_test.h"
#include "diag_esw_util.h"
#include "diag_esw_lib.h"
#include "diag_esw_test.h"
#include "diag_fpga.h"
#include "platform_cpu.h"
#include "dev_88e1680.h"
#include "dev_98dxc323.h"
#include "madApi.h"
#include "madHwCntl.h"


/*
 * Global variables
 */
extern int marvell_cpssPpInit_xcat3;
extern MAD_DEV phy_dev_88e1680[NANOOK_1680_GROUP_NUM];
extern int marvell_ac3_cpss_dev_num_nanook;

/* Local functions */
int diag_esw_pcie_config_rd_util (void);
int diag_esw_pcie_config_wr_util (void);
int diag_esw_xcat3_reg_rd_util (void);
int diag_esw_xcat3_reg_wr_util (void);
int diag_esw_xcat3_internal_reg_rd_util (void);
int diag_esw_xcat3_internal_reg_wr_util (void);
int diag_esw_xcat3_serdes_tx_config_read_util(void);
int diag_esw_xcat3_serdes_tx_config_write_util (void);
int diag_esw_xcat3_phy_tx_config_read_util (void);
int diag_esw_xcat3_phy_tx_config_write_util (void);
int diag_esw_phy_reg_rd_util (void);
int diag_esw_phy_reg_wr_util (void);
int diag_esw_phy_led_util (void);
int diag_esw_phy_test_mode_util (void);
int diag_esw_set_ixia_snake_config_util (uint);



static int     esw_snake_ixia_map_tbl[] = 
	{XCAT3_PORT_00, XCAT3_PORT_02, XCAT3_PORT_04, XCAT3_PORT_06, XCAT3_PORT_08, 
	XCAT3_PORT_10, XCAT3_PORT_12, XCAT3_PORT_14, XCAT3_PORT_16, XCAT3_PORT_18,
	XCAT3_PORT_20, XCAT3_PORT_22};

static int     esw_snake_ixia_pair_map_tbl[] = 
	{XCAT3_PORT_01, XCAT3_PORT_03, XCAT3_PORT_05, XCAT3_PORT_07, XCAT3_PORT_09, 
	XCAT3_PORT_11, XCAT3_PORT_13, XCAT3_PORT_15, XCAT3_PORT_17, XCAT3_PORT_19,
	XCAT3_PORT_21, XCAT3_PORT_23};


/******************************************************************************
 *
 * Function: diag_esw_pcie_config_rd_util
 *
 * Description: Ethernet switch PCIE config read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_pcie_config_rd_util (void)
{
    int rc = PASSED;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 pcie config read utility function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_pcie_config_read_util((dev_object_t *)esw_98dxc323_obj_p) != PASSED) {
        cterr('f',0,"Failed to read xCat3 pcie config");
        goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_pcie_config_wr_util
 *
 * Description: Ethernet switch PCIE config write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_pcie_config_wr_util (void)
{
    int rc = PASSED;

    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 pcie config write utility function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_pcie_config_write_util((dev_object_t *)esw_98dxc323_obj_p) != PASSED) {
        cterr('f',0,"Failed to read xCat3 pcie config");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_reg_rd_util
 *
 * Description: Ethernet switch xCat3 register read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_reg_rd_util (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 register read utility function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_reg_read_util((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to read xCat3 register");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_reg_wr_util
 *
 * Description: Ethernet switch xCat3 register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_reg_wr_util (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 register write function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_reg_write_util((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to read xCat3 register");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_internal_reg_rd_util
 *
 * Description: Ethernet switch xCat3 internal register read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_internal_reg_rd_util (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 internal register read utility function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_internal_reg_read_util((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to read xCat3 internal register");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_internal_reg_wr_util
 *
 * Description: Ethernet switch xCat3 internal register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_internal_reg_wr_util (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 internal register write function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_internal_reg_write_util((dev_object_t *)esw_98dxc323_obj_p, cpss_dev) != PASSED) {
        cterr('f',0,"Failed to read xCat3 internal register");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_10g_kr_test_mode_util
 *
 * Description: Ethernet switch xCat3 internal register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_10g_kr_test_mode_util (void)
{
    int rc = PASSED;
    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint32_t test_mode = 0;
    uint32_t test_pattern = 0;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

     while (1) {
        printf("Ethernet xCat3 10G-KR Test Mode:\n");
        printf("[0] Normal mode.\n");
	 printf("[1] Test Mode 1 - PRBS09.\n");
	 printf("[2] Test Mode 2 - PRBS15.\n");
	 printf("[3] Test Mode 3 - PRBS31.\n");
	 printf("[4] Test Mode 4 - 8180.\n");
	 printf("[5] Test Mode 5 - Customize Pattern.\n");
        printf("[f] Leave the utility.\n");
	 test_mode = gethex_answer("Enter the test mode: ", 0, 0, 0xF);

	 if (test_mode == XCAT3_10GKR_TEST_MODE_CUSTOMIZE) {
	     test_pattern = gethex_answer("Enter the customize test pattern, between 0x00 and 0xFF: ", 0, 0, 0xFF);
	 }

	 if ((test_mode >= 0) && (test_mode < 6)) {
            /* Call 98dxc323 xcat3 10g kr test mode function */
            if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_10g_kr_test_mode((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, GE0_XCAT3_PORT, test_mode, test_pattern) != PASSED) {
                cterr('f',0,"Failed to read xCat3 internal register");
	         goto _exit;
            }
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_serdes_tx_config_read_util
 *
 * Description: Ethernet switch xCat3 serdes tx config read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_serdes_tx_config_read_util (void)
{
    int rc = PASSED;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 serdes tx config read function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_serdes_tx_config_read((dev_object_t *)esw_98dxc323_obj_p) != PASSED) {
        cterr('f',0,"Failed to xcat3 serdes tx config read function");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_serdes_tx_config_write_util
 *
 * Description: Ethernet switch xCat3 serdes tx config write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_serdes_tx_config_write_util (void)
{
    int rc = PASSED;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 serdes tx config write function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_serdes_tx_config_write((dev_object_t *)esw_98dxc323_obj_p) != PASSED) {
        cterr('f',0,"Failed to xcat3 serdes tx config write function");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_phy_tx_config_read_util
 *
 * Description: Ethernet switch xCat3 phy tx config read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_phy_tx_config_read_util (void)
{
    int rc = PASSED;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 phy tx config read function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_phy_tx_config_read((dev_object_t *)esw_98dxc323_obj_p) != PASSED) {
        cterr('f',0,"Failed to xcat3 phy tx config read function");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_xcat3_phy_tx_config_write_util
 *
 * Description: Ethernet switch xCat3 phy tx config write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat3_phy_tx_config_write_util (void)
{
    int rc = PASSED;
	
    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Call 98dxc323 xcat3 phy tx config write function */
    if (esw_98dxc323_obj_p->callin_fvt->esw_xcat3_phy_tx_config_write((dev_object_t *)esw_98dxc323_obj_p) != PASSED) {
        cterr('f',0,"Failed to xcat3 phy tx config write function");
	 goto _exit;
    }

    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return(PASSED);
	
 _exit:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_phy_reg_rd_util
 *
 * Description: Ethernet switch PHY register read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_reg_rd_util (void)
{

    int rc = 0;
    MAD_DEV * mad_dev;
    uint port_num, page_num, reg_num, data;
    uint port_group, port_group_phy_num;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, NANOOK_ESW_PORT_NUM);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);

    port_group = (port_num / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    port_group_phy_num = (port_num % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    mad_dev = &phy_dev_88e1680[port_group];


    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    /* Call 88e1680 PHY register read utility function */
    if (phy_88e1680_obj_p->callin_fvt->read_phy_reg_util((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, page_num, reg_num, &data)!= PASSED) {
        cterr('f',0,"Failed to read phy reg.", port_num);
        goto _exit;
    }

    printf("PHY port %d register value @ offset %d = %#x\n", port_num, reg_num, data&0xffff);

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);
}


/******************************************************************************
 *
 * Function: diag_esw_phy_reg_wr_util
 *
 * Description: Ethernet switch PHY register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_reg_wr_util (void)
{

    int rc = 0;
    MAD_DEV * mad_dev;
    uint port_num, page_num, reg_num, data;
    uint port_group, port_group_phy_num;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, NANOOK_ESW_PORT_NUM);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);
	
    port_group = (port_num / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    port_group_phy_num = (port_num % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
    mad_dev = &phy_dev_88e1680[port_group];


    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    }

    /* Call 88e1680 PHY register write utility function */
    if (phy_88e1680_obj_p->callin_fvt->write_phy_reg_util((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, page_num, reg_num, data)!= PASSED) {
        cterr('f',0,"Failed to read phy reg.", port_num + 1);
        goto _exit;
    }

    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);
}


/******************************************************************************
 *
 * Function: diag_esw_phy_led_util
 *
 * Description: Ethernet switch PHY led utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_led_util (void)
{

    int ix, rc = 0;
    MAD_DEV * mad_dev;
    uint port_group, port_group_phy_num;

    uint32_t test_mode = 0, test_target =0, test_port = 0;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    } 

     while (1) {
        printf("Ethernet Switch LED Supported Mode:\n");
        printf("[0] Select Specify 88E1680 Port LED.\n");
	 printf("[1] Select All 88E1680 Port LED.\n");
        printf("[f] Leave the utility.\n");

        test_target = gethex_answer("Enter target LED: ", 0, 0, 0xf);
	 if (test_target == 0xf) {
            break;
        }

	 if (test_target == 0x0) {

            printf("[0] Turn Green on.\n");
            printf("[1] Turn off.\n");
            printf("[f] Leave the utility.\n");
            test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);
            if (test_mode == 0xf) {
                break;
            }
            test_port = gethex_answer("Enter Port number: ", 0, 0, (NANOOK_ESW_PORT_NUM - 1));	

            port_group = (test_port / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
            port_group_phy_num = (test_port % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
            mad_dev = &phy_dev_88e1680[port_group];

	     if (test_mode == 0) {
                rc = phy_88e1680_obj_p->callin_fvt->led_on((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num);
                if ( rc == FAILED) {
                    goto _exit;
                } 
            } else if (test_mode == 1) {
                rc = phy_88e1680_obj_p->callin_fvt->led_off((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num);
                if (rc == FAILED) {
                    goto _exit;
                } 
            } else if (test_mode == 0xf) {
                for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	             port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    mad_dev = &phy_dev_88e1680[port_group];
		      rc = phy_88e1680_obj_p->callin_fvt->led_default((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num);
                    if (rc == FAILED) {
                        goto _exit;
                    } 
                }
                break;
            } else {
                printf("Wrong test mode!");
            }
        }else if (test_target == 0x1) {

            printf("[0] Turn Green on.\n");
            printf("[1] Turn off.\n");
            printf("[f] Leave the utility.\n");
            test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);
            if (test_mode == 0xf) {
                break;
            }

	     if (test_mode == 0) {
                for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	             port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    mad_dev = &phy_dev_88e1680[port_group];
		      rc = phy_88e1680_obj_p->callin_fvt->led_on((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num);
                    if (rc == FAILED) {	
                        goto _exit;
                    } 
                } 
            } else if (test_mode == 1) {
                for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	             port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    mad_dev = &phy_dev_88e1680[port_group];
		      rc = phy_88e1680_obj_p->callin_fvt->led_off((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num);
                    if (rc == FAILED) {	
                        goto _exit;
                    } 
                } 
            } else if (test_mode == 0xf) {
                for (ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
	             port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                    mad_dev = &phy_dev_88e1680[port_group];
		      rc = phy_88e1680_obj_p->callin_fvt->led_default((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num);
                    if (rc == FAILED) {
                        goto _exit;
                    } 
                }
                break;
            } else {
                printf("Wrong test mode!");
            }
        } else {
            printf("Wrong test target!");
        }
    }
	
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function: diag_esw_phy_test_mode_util
 *
 * Description: Ethernet switch PHY test mode utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_test_mode_util (void)
{

    int ix, rc = 0;
    MAD_DEV * mad_dev;
    uint port_group, port_group_phy_num;

    uint32_t test_mode = 0, test_port;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit;
    } 

     while (1) {
        printf("Ethernet Switch PHY Test Mode:\n");
        printf("[0] Normal mode.\n");
        printf("[1] Test Mode 1 - Transmit Waveform Test.\n");
        printf("[2] Test Mode 2 - Transmit Jitter Test (Master mode).\n");
        printf("[3] Test Mode 3 - Transmit Jitter Test (Slave mode).\n");
        printf("[4] Test Mode 4 - Transmit Distortion Test.\n\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter the test mode: ", 0, 0, 0xF);
        if (test_mode == 0xf) {
            for (ix = 0; ix <= ( NANOOK_ESW_PORT_NUM - 1); ix++) {
                port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
                mad_dev = &phy_dev_88e1680[port_group];
                rc = phy_88e1680_obj_p->callin_fvt->set_test_mode((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, 0);
                if (rc == FAILED) {
                    goto _exit;
                } 
            }
            break;
        }
        test_port = gethex_answer("Enter Port number: ", 0, 0, NANOOK_ESW_PORT_NUM);

        port_group = (test_port / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (test_port % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];

        if ((test_mode >= 0) && (test_mode < 5)) {
            rc = phy_88e1680_obj_p->callin_fvt->set_test_mode((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, test_mode);
            if ( rc == FAILED) {
                goto _exit;
            } 
        } else {
            printf("Wrong test mode!");
        }
     }
    
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return(PASSED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    return (FAILED);
	
}


/******************************************************************************
 *
 * Function   :	diag_esw_set_ixia_snake_config_util
 * Description: Set ixia snake config utility
 *              
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************
 */
int
diag_esw_set_ixia_snake_config_util (uint speed)
{
    int rc = PASSED;
    int ix, src_port, dst_port;

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
    uint target_speed;

    MAD_SPEED_MODE mad_target_speed;

    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    rc = diag_reset_esw_to_default(0);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Reset ESW to default failed.",__func__);
        return (FAILED);
    }

    if (speed == 0) {
        target_speed = SPD_1000MBPS;
	 mad_target_speed = MAD_SPEED_1000M;
    } else if (speed == 1) {
        target_speed = SPD_100MBPS;
	 mad_target_speed = MAD_SPEED_100M;
    } else if (speed == 2) {
        target_speed = SPD_10MBPS;
	 mad_target_speed = MAD_SPEED_10M;
    } else {
    
        printf("Unsupported speed mode!\n");
	 return (FAILED);
    }

    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++)
    {
        /* Config port speed */
        if (diag_config_port_speed(cpss_dev, ix, target_speed)!= PASSED) {
            cterr('f',0,"Failed to config MAC port speed for port %d.", ix);
            goto _exit;
        }
    }


    for (ix = 0; ix < NANOOK_SNAKE_IXIA_PAIR_NUM; ix++) {

            src_port = esw_snake_ixia_map_tbl[ix];
            dst_port = esw_snake_ixia_pair_map_tbl[ix];;

            /* Call 98dxc323 xcat3 config port pve function */
	     //printf("Config port pve for src_port:%d  dst_port:%d\n", src_port, dst_port);
            if (esw_98dxc323_obj_p->callin_fvt->esw_config_port_pve((dev_object_t *)esw_98dxc323_obj_p, cpss_dev, src_port, dst_port) != PASSED) {
                cterr('f',0,"Failed to configure PVE for src_port:%d and dst_port:%d\n", src_port, dst_port);
                goto _exit;
            }
	  
    }


    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
        port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];

        /* Call 88e1680 force speed function */
        if (phy_88e1680_obj_p->callin_fvt->phy_force_speed((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, mad_target_speed)!= PASSED) {
            cterr('f',0,"Failed to force speed for port %d", ix);
            goto _exit;
        }
    }

    msleep(ESW_WAIT_10000MS); 

    if (speed == 0) {
        printf("Set IXIA Snake Cconfiguration 1000 done.");
    } else if (speed == 1) {
       printf("Set IXIA Snake Cconfiguration 100 done.");
    } else if (speed == 2) {
        printf("Set IXIA Snake Cconfiguration 10 done.");
    } else {
        printf("Unsupported speed mode!\n");
	 return (FAILED);
    }
 

    /* Destroy object */
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
}



/******************************************************************************
 *
 * Function   :	diag_esw_set_ixia_speed_config_util
 * Description: Set for ixia speed config utility
 *              
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************
 */
int
diag_esw_set_ixia_speed_config_util (uint speed)
{
    int rc = PASSED;
    int ix;

    int cpss_dev = marvell_ac3_cpss_dev_num_nanook;
    uint port_group, port_group_phy_num;
    MAD_DEV * mad_dev;
    uint target_speed;

    MAD_SPEED_MODE mad_target_speed;

    dev_98dxc323_object_t esw_98dxc323_obj;
    dev_98dxc323_object_t *esw_98dxc323_obj_p  = &esw_98dxc323_obj;
	
    dev_88e1680_object_t phy_88e1680_obj;
    dev_88e1680_object_t *phy_88e1680_obj_p  = &phy_88e1680_obj;

    /* Create 98dxc323 device driver */
    rc = diag_esw_98dxc323_dev_create(esw_98dxc323_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    /* Create 88e1680 device driver */
    rc = diag_phy_88e1680_dev_create(phy_88e1680_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        goto _exit_98dxc323;
    }

    rc = diag_reset_esw_to_default(0);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Reset ESW to default failed.",__func__);
        return (FAILED);
    }

    if (speed == 0) {
        target_speed = SPD_1000MBPS;
	 mad_target_speed = MAD_SPEED_1000M;
    } else if (speed == 1) {
        target_speed = SPD_100MBPS;
	 mad_target_speed = MAD_SPEED_100M;
    } else if (speed == 2) {
        target_speed = SPD_10MBPS;
	 mad_target_speed = MAD_SPEED_10M;
    } else {
    
        printf("Unsupported speed mode!\n");
	 return (FAILED);
    }

    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++)
    {
        /* Config port speed */
        if (diag_config_port_speed(cpss_dev, ix, target_speed)!= PASSED) {
            cterr('f',0,"Failed to config MAC port speed for port %d.", ix);
            goto _exit;
        }
    }


    for(ix = 0; ix < NANOOK_ESW_PORT_NUM; ix++) {
        port_group = (ix / NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        port_group_phy_num = (ix % NANOOK_1680_NUM_PHY_IN_ONE_GRUOP);
        mad_dev = &phy_dev_88e1680[port_group];

        /* Call 88e1680 force speed function */
        if (phy_88e1680_obj_p->callin_fvt->phy_force_speed((dev_object_t *)phy_88e1680_obj_p, mad_dev, port_group_phy_num, mad_target_speed)!= PASSED) {
            cterr('f',0,"Failed to force speed for port %d", ix);
            goto _exit;
        }
    }

    msleep(ESW_WAIT_10000MS); 

    if (speed == 0) {
        printf("Set for IXIA Speed 1000 Cconfiguration done.");
    } else if (speed == 1) {
       printf("Set for IXIA Speed 100 Cconfiguration done.");
    } else if (speed == 2) {
        printf("Set for IXIA Speed 10 Cconfiguration done.");
    } else {
        printf("Unsupported speed mode!\n");
	 return (FAILED);
    }
 

    /* Destroy object */
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (PASSED);

 _exit_98dxc323:
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
	
 _exit:
    phy_88e1680_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&phy_88e1680_obj_p);
    esw_98dxc323_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_98dxc323_obj_p);
    return (FAILED);
}


/*-------------------------------------------------
 * $Log: diag_esw_util.c,v $
 * Revision 1.2  2019/12/11 10:10:28  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */


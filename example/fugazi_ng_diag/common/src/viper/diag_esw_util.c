 /* $Id: diag_esw_util.c,v 1.3 2018/10/11 06:02:59 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_esw_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_esw_util.c - This file is for ethernet switch utility
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
#include "diag_temp_snsr_test.h"
#include "diag_gephy_test.h"
#include "diag_esw_test.h"
#include "diag_fpga.h"
#include "platform_cpu.h"
#include "dev_88e6176.h"



/* Local functions */
int diag_esw_reg_rd_util(void);
int diag_esw_reg_wr_util(void);
int diag_esw_phy_reg_rd_util(void);
int diag_esw_phy_reg_wr_util(void);
int diag_esw_set_allport_forward_util(void);
int diag_esw_adjust_port_vod_util(void);
int diag_smi_c45_rd_util(void);
int viper_esw_smi_c45_rd(int, int, int, ushort *);
int diag_smi_c45_wr_util(void);
int viper_esw_smi_c45_wr(int, int, int, ushort);
int diag_esw_config_vlan_profile(void);
int esw_set_1k_testmode_util(void);
static int esw_port_vod_adjust(int, int, int);

/******************************************************************************
 *
 * Function: diag_esw_reg_rd_util
 *
 * Description: Utility to read Viper ethernet switch(Marvell 88E6176) register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_reg_rd_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;
    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_obj_p->callin_fvt->esw_reg_read_util((dev_object_t *)esw_obj_p) != PASSED) {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (FAILED);
    } else {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: diag_esw_reg_wr_util
 * Description: Utility to write Viper ethernet switch(Marvell 88E6390) register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_reg_wr_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_obj_p->callin_fvt->esw_reg_write_util((dev_object_t *)esw_obj_p) != PASSED) {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (FAILED);
    } else {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: diag_esw_phy_reg_rd_util
 * Description: Utility to read Viper switch(Marvell 88E6176)'s PHY register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_reg_rd_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_obj_p->callin_fvt->esw_phy_reg_read_util((dev_object_t *)esw_obj_p) != PASSED) {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (FAILED);
    } else {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: diag_esw_phy_reg_wr_util
 * Description: Utility to write Viper switch(Marvell 88E6390)'s PHY register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_reg_wr_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_obj_p->callin_fvt->esw_phy_reg_write_util((dev_object_t *)esw_obj_p) != PASSED) {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (FAILED);
    } else {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (PASSED);
    }

}

/******************************************************************************
 *
 * Function: diag_esw_set_allport_forward_util
 * Description: Utility to set Viper switch all ports forwarding.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_set_allport_forward_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int start_port = 0, end_port = 0, ctr;

    /* Create 88e6176 device driver */
    if (diag_esw_dev_create(esw_obj_p) == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

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
    return (PASSED);
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (FAILED);
}

/******************************************************************************
 *
 * Function:diag_esw_adjust_port_vod_util
 * Description: Utility to adjust Viper LAN Switch port VOD.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_adjust_port_vod_util (void)
{
    int def_port = 0, s_port = 0, e_port = 0;
    int esw_port = 0, eth_mode = 0, vod_val = 0;
    int ctr = 0;

    def_port = ESW_PORT0;
    s_port = ESW_PORT0;
    e_port = (ESW_PORT3 + 1);

    printf("\nViper Switch port mapping - \n");
    printf("-------------------------\n");
    printf("|  0  |  1  |  2  |  3  |\n");
    printf("-------------------------\n");
    esw_port = (int)gethex_answer("Enter port num(0 ~ 3, 4 for all): ",
                                def_port, s_port, e_port);

    if (esw_port != e_port) {
        s_port = esw_port;
        e_port = (s_port + 1);
    }

    printf("\nEthernet modes -\n");
    printf("    1 - 10 Mbps\n");
    printf("    2 - 100 Mbps\n");
    printf("    3 - 1000 Mbps\n");
    eth_mode = (int)gethex_answer("Enter mode: ", 0x2, 0x1, 0x3);

    printf("\nVOD modes -\n");
    printf(" 0:  0%%,   1:  -2%%,   2:  -4%%,   3:  -6%%\n");
    printf(" 4: -8%%,   5: -10%%,   6: -12%%,   7: -14%%\n");
    printf(" 8:  0%%,   9:   2%%,   a:   4%%,   b:   6%%\n");
    printf(" c:  8%%,   d:  10%%,   e:  12%%,   f:  14%%\n");
    vod_val = (int)gethex_answer("Enter VOD value: ", 0x0, 0x0, 0xf);

    for (ctr = s_port; ctr < e_port; ctr++) {
        if (esw_port_vod_adjust(esw_port, eth_mode, vod_val) != PASSED) {
            printf("Failed to adjust port%d VOD.\n", ctr);
            return (FAILED);
        }
        printf("Done adjust LAN Switch port%d VOD.\n", ctr);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_port_vod_adjust
 * Description: Function to adjust Viper LAN Switch port VOD.
 * Inputs     : esw_port - port number of switch
 *              eth_mode - 10/100/1000Mbps
 *              vod_val  - % that want to adjust 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_port_vod_adjust (int esw_port, int eth_mode, int vod_val)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;
    int    reg_page = (int)ESW_REG_PAGE_252;
    int    reg_addr = (int)REG_ADDR(17);
    ushort reg_val = 0;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (eth_mode == 3) {
        reg_addr = (int)REG_ADDR(18);
    }

    /* Adjust VOD */
    if (esw_obj_p->callin_fvt->esw_phy_reg_read((dev_object_t *)esw_obj_p, esw_port
                                                , reg_page, reg_addr, &reg_val) 
                                                != PASSED) {
        printf("%s: Failed to read Reg. %d_%d.\n",
               __FUNCTION__, reg_addr, reg_page);
        goto _exit;
    }

    if (eth_mode == 3) {
        reg_val = (ushort)((vod_val << 12) |
                           (vod_val << 8) |
                           (vod_val << 4) |
                           (vod_val));
    } else if (eth_mode == 1) {
        reg_val &= (ushort)(~0x00ff);
        reg_val |= (ushort)((vod_val << 4) | vod_val);
    } else {
        reg_val &= (ushort)(~0xff00);
        reg_val |= (ushort)((vod_val << 12) | (vod_val << 8));
    }

    msleep(10);

    if (esw_obj_p->callin_fvt->esw_phy_reg_write((dev_object_t *)esw_obj_p, esw_port
                                                 , reg_page, reg_addr, reg_val) 
                                                 != PASSED) {
        printf("%s: Failed to set Reg. %d_%d to 0x%04X.\n",
               __FUNCTION__, reg_addr, reg_page, reg_val);
        goto _exit;
    }

    msleep(10);

    /* Recover Page Addr to 0x0 to trigger adjust process */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)REG_ADDR(22);
    reg_val = 0;
    if (esw_obj_p->callin_fvt->esw_phy_reg_write((dev_object_t *)esw_obj_p, esw_port
                                        , reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to trigger adjust process by set page addr to %d.\n",
	       __FUNCTION__, reg_val);
        goto _exit;
    }
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (PASSED);
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (FAILED);
}

/******************************************************************************
 *
 * Function:diag_smi_c45_rd_util
 * Description: Utility to do SMI Clause45 read on Viper switch(Marvell 88E6176).
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_smi_c45_rd_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_obj_p->callin_fvt->esw_c45_phy_reg_read_util((dev_object_t *)esw_obj_p) != PASSED) {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (FAILED);
    } else {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (PASSED);
    }
}


/*******************************************************************************
 *
 * Function   : viper_esw_smi_c45_wr_util
 * Description: Utility to write Viper switch(Marvell 88E6176)'s PHY register.
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_smi_c45_wr_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc = FAILED;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    if (esw_obj_p->callin_fvt->esw_c45_phy_reg_write_util((dev_object_t *)esw_obj_p) != PASSED) {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (FAILED);
    } else {
        esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
        return (PASSED);
    }
}

/*******************************************************************************
 *
 * Function   : esw_set_1k_testmode_util
 * Description: Utility to set Viper LAN Switch PHY 1000BaseT test mode.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int esw_set_1k_testmode_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    rc = esw_obj_p->callin_fvt->esw_set_testmode((dev_object_t *)esw_obj_p);

    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);

    return (rc);
}


/******************************************************************************
 *
 * Function: diag_esw_config_vlan_profile
 *
 * Description: Utility to configure Port-based VLAN
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_config_vlan_profile (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc;

    /* Create device driver */
    rc = diag_esw_dev_create(esw_obj_p);

    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    printf("Configure VLAN Profile as followings:\n");
    printf("  P0~P2, P1~P3~CPU Port (P5)\n\n");

    return (esw_obj_p->callin_fvt->esw_config_pvlan((dev_object_t *)esw_obj_p,
                                                     VLAN_PROFILE_1));
}
/*-------------------------------------------------
 * $Log: diag_esw_util.c,v $
 * Revision 1.3  2018/10/11 06:02:59  harrchan
 * Add FPGA function test (CSCvm72986)
 *
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/03/16 06:51:37  harrchan
 * Support ESW PHY 1000base-T Test Mode
 *
 * Revision 1.1.2.2  2018/03/05 08:54:21  harrchan
 * Initial hydra application code base
 *
 * Revision 1.1.2.1  2018/02/27 08:06:39  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */




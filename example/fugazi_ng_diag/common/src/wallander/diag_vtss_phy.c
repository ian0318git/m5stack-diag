/* $Id: diag_vtss_phy.c,v 1.2 2015/07/14 06:35:03 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_vtss_phy.c,v $
 *------------------------------------------------------------------
 *
 * diag_vtss_phy.c - Wallander Vitesse PHY functions.
 *
 * Xiaoying Zhang -- Mar. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "common_utils.h"
// #include "pcmap.h"
#include "nvsysvars.h"
// #include "nvmonvars.h"
#include "router_if.h"
#include "diag_vtss_phy.h"
#include "diag_ge_phy.h"
#include "diag_common_drv.h"
#include "vtss_misc_api.h"
#include "vtss_phy_api.h"
#include "vtss_init_api.h"
#include "vtss_port_api.h"
#include "vtss_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

int vsc_phy_access_print = 0;
#define PHY_ACCESS_PRINT_1G   0x1
#define PHY_ACCESS_PRINT_10G  0x2
#define PHY_ACCESS_API        0x4

int wallander_num_ports = 0;
/*
 * Returns Phy port number for SFP
 */
static int wallander_phy_mappingtable[4] = {0,1,2,3};

static int wallander_port_to_phyport_mapping (int port) {
    return (wallander_phy_mappingtable[port]);
}

dev_object_t *phy_dev[VTSS_NUM_PHY_MAX_PORTS];

dev_object_fvt_t vts_obj_fvt;

static vtss_rc
phy_port_link_status (dev_vsc8x_object_t *vsc_phy,
                      const vtss_port_no_t port_num)
{
    return(VTSS_RC_OK);

}

static dev_vts8x_phy_callout_t vsc_phy_callout = {
    phy_port_link_status,   /* phy_port_link_status_notify */
};


/*
 * Default init paramaters for 1G phy . Platform can change it by 
 * passing this values in create_into_t
 */

static vtss_phy_reset_conf_t vsc_phy_init_default_params =
    /* MAC Interface, Media Interface, RGMII Setup, TBI Setup, I-CPU enable */
    {VTSS_PORT_INTERFACE_QSGMII, VTSS_PHY_MEDIA_IF_FI_1000BX, {1,2},
        {FALSE}, FALSE};

static vtss_phy_reset_conf_t vsc_1g_serdes_phy_reset_init_conf =
    /* MAC Interface, Media Interface, RGMII Setup, TBI Setup, I-CPU enable */
    {VTSS_PORT_INTERFACE_QSGMII, VTSS_PHY_MEDIA_IF_FI_1000BX/*VTSS_PHY_MEDIA_IF_SFP_PASSTHRU*/, {1,2},
        {FALSE}, FALSE};

static vtss_phy_reset_conf_t vsc_1g_cu_phy_reset_init_conf =
    /* MAC Interface, Media Interface, RGMII Setup, TBI Setup, I-CPU enable */
    {VTSS_PORT_INTERFACE_QSGMII, VTSS_PHY_MEDIA_IF_CU, {1,2},
        {TRUE}, FALSE};

static vtss_phy_reset_conf_t vsc_1g_sfp_phy_reset_init_conf =
    /* MAC Interface, Media Interface, RGMII Setup, TBI Setup, I-CPU enable */
    {VTSS_PORT_INTERFACE_QSGMII, VTSS_PHY_MEDIA_IF_FI_1000BX, {1,2},
        {TRUE}, FALSE};

/*
 * Default phy config paramaters for 1G phy.
 */
vtss_phy_conf_t vsc_1g_cu_phy_conf = 
{
    /* PHY mode,  */
    /* Forced mode configuration */
    /* Auto-negotiation mode configuration */
    /* Cu cable MDI (Crossed cable / normal cable) */
    VTSS_PHY_MODE_ANEG/*VTSS_PHY_MODE_FORCED*/,
    {VTSS_SPEED_1G, TRUE},
    {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE},
    VTSS_PHY_MDIX_AUTO
};
vtss_phy_conf_t vsc_1g_sfp_phy_conf = 
{
    /* PHY mode,  */
    /* Forced mode configuration */
    /* Auto-negotiation mode configuration */
    /* Cu cable MDI (Crossed cable / normal cable) */
    VTSS_PHY_MODE_ANEG/*VTSS_PHY_MODE_FORCED*/,
    {VTSS_SPEED_1G, TRUE},
    {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE},
    VTSS_PHY_MDIX_AUTO
};
vtss_phy_conf_t vsc_1g_serdes_phy_conf = 
{
    /* PHY mode,  */
    /* Forced mode configuration */
    /* Auto-negotiation mode configuration */
    /* Cu cable MDI (Crossed cable / normal cable) */
    VTSS_PHY_MODE_FORCED,
    {VTSS_SPEED_1G, TRUE},
    {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE},
    VTSS_PHY_MDIX_AUTO
};

static dev_object_t         *dev_gptr = NULL;
static dev_vsc8x_object_t   *phy_gptr = NULL;
static vsc_api_controller_t *api_gptr = NULL;

/* stores malloc'ed addr for dev_vsc & api_ctlr structures */
dev_vsc8x_object_t   *dev_vsc_addrs[VTSS_NUM_PHY_MAX_PORTS];
vsc_api_controller_t *api_ctlr_addrs[VTSS_NUM_PHY_MAX_PORTS];

/* stores malloc'ed addr for vtss_inst_t structures */
vtss_inst_t vtss_inst_addr[VTSS_NUM_PHY_MAX_PORTS];

static int phyPortInitialized[VTSS_NUM_PHY_MAX_PORTS] = {0, 0, 0, 0};

BOOL vsc_phy_ts_fifo_read_cb_installed = FALSE;
BOOL ts_init_reqd = FALSE;

vtss_phy_clock_conf_t vts_phy_1g_clock_default_conf =
    {VTSS_PHY_LOCAL_XTAL, VTSS_PHY_FREQ_125M, VTSS_PHY_CLK_SQUELCH_MIN};

vtss_phy_recov_clk_t vts_phy_1g_recov_clk_default_conf = VTSS_PHY_RECOV_CLK1;

// prototypes for cisco device API
//void dev_base_attach (dev_object_t *dev);
//void dev_base_detach (dev_object_t *dev);
//void dev_base_init (dev_object_t *dev);
//void dev_base_deinit (dev_object_t *dev);
//void dev_destroy_default (dev_object_t **dev);
extern uint32 dev_do_nothing (dev_object_t *dev);

static BOOL
vts_enable_disable_link (dev_vsc8x_object_t *phy, 
                         u16 port_num, BOOL enable)
{
    vsc_api_controller_t *api_ctlr;
    BOOL rc;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                    "phy object is not ready yet");
       return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        if (!(api_ctlr->phy_config_set)) {
            cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                        "1G phy init configs are not set yet");
            return (FALSE);
        }
        if (enable) {
            api_ctlr->phy_config_set->mode =
                            VTSS_PHY_MODE_FORCED;
        } else {
            api_ctlr->phy_config_set->mode =
                        VTSS_PHY_MODE_POWER_DOWN;
        }
        rc = vtss_phy_conf_set(api_ctlr->vsc_instance,
                                (vtss_port_no_t)port_num,
                            api_ctlr->phy_config_set);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                        "phy_conf_set failed rc = %d", rc);
            return (FALSE);
        }
        return (TRUE);
    }
/*    if (api_ctlr->platform_target == VTSS_TARGET_10G_PHY) {
       vtss_phy_10g_power_t power;
//       dev_vts_phy_loopback_type_t *loopback;

       if (enable) {
          power = VTSS_PHY_10G_POWER_ENABLE;
       } else {
          power = VTSS_PHY_10G_POWER_DISABLE;
       }
       rc = vtss_phy_10g_power_set(api_ctlr->vsc_instance,
                                 (vtss_port_no_t)port_num,
                                                  &power);
       if (rc != VTSS_RC_OK) {
          cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                       "phy_conf_set failed rc = %d", rc);
          return (FALSE);
       }

       return (TRUE);
    }*/
    /* Should never come here */   
    return (TRUE);
}

static void
compare_and_set_autoneg_mode (vtss_phy_conf_t *phy_oper_conf,
                              dev_vsc8x_port_config_t *phy_config)
{
    if ((phy_config->aneg_enable == DEV_VTSS_PHY_MODE_ANEG) &&
            (phy_oper_conf->mode == VTSS_PHY_MODE_FORCED)) {
        phy_oper_conf->mode = VTSS_PHY_MODE_ANEG;
    }
    if ((phy_config->aneg_enable == DEV_VTSS_PHY_MODE_FORCED) &&
            (phy_oper_conf->mode == VTSS_PHY_MODE_ANEG)) {
        phy_oper_conf->mode = VTSS_PHY_MODE_FORCED;
    }
}

static BOOL
vts_set_speed_duplex (dev_vsc8x_object_t *phy, u16 port_num,
                      dev_vsc8x_port_config_t *phy_config)
{
    vsc_api_controller_t *api_ctlr;
    vtss_phy_conf_t phy_oper_conf;
    BOOL rc;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: SET Speed/Duplex"
                    "phy object is not ready yet");
       return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    rc = vtss_phy_conf_get(api_ctlr->vsc_instance,
                         (vtss_port_no_t)port_num,
                                 &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: SET Speed/Duplex"
                    "phy_conf_get failed rc = %d", rc);
       return (FALSE);
    }
    /*
     * Program Phy if there is mismatch between autoneg mode
     * on phy and autoneg mode passed to this API. 
     * As this is critical to configure  speed and duplex on phy
     */
    compare_and_set_autoneg_mode(&phy_oper_conf, phy_config);  
    rc = vtss_phy_conf_set(api_ctlr->vsc_instance,
             (vtss_port_no_t)port_num, &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: SET Speed/Duplex"
                    "phy_conf_set for autoneg mode failed rc = %d", rc);
       return (FALSE);
    }
    /*
     * Right now we are trusting application to pass 
     * correct speed and duplex based on media type
     * TODO: Check valiadity of speed/duplex if
     * supported on media type.
     */
    if ((&phy_oper_conf)->mode == VTSS_PHY_MODE_ANEG) {
       /*
        * If autoneg was on then we need to set allowed
        * negotiated speed and duplex.
        */
       switch (phy_config->aneg_speed) {
       case DEV_VTSS_SPEED_10MB:
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_AUTO) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = TRUE; 
           }
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_FULL) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = TRUE; 
           }
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_HALF) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = FALSE; 
           }
           break;
       case DEV_VTSS_SPEED_100MB:
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_AUTO) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = TRUE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = FALSE; 
           }
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_FULL) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = TRUE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = FALSE; 
           }
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_HALF) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = FALSE; 
           }
           break;
       case DEV_VTSS_SPEED_1GB:
           /* Only full duplex is supported in 1G mode */
           (&phy_oper_conf)->aneg.speed_1g_fdx = TRUE;
           (&phy_oper_conf)->aneg.speed_100m_fdx = FALSE;
           (&phy_oper_conf)->aneg.speed_100m_hdx = FALSE;
           (&phy_oper_conf)->aneg.speed_10m_hdx = FALSE;
           (&phy_oper_conf)->aneg.speed_10m_fdx = FALSE; 
           break;   
       case DEV_VTSS_SPEED_AUTO:
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_AUTO) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = TRUE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = TRUE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = TRUE; 
           }
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_FULL) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = TRUE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = TRUE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = FALSE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = TRUE; 
           }
           if (phy_config->aneg_duplex == DEV_VTSS_DUPLEX_TYPE_HALF) {
              (&phy_oper_conf)->aneg.speed_1g_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_fdx = FALSE;
              (&phy_oper_conf)->aneg.speed_100m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_hdx = TRUE;
              (&phy_oper_conf)->aneg.speed_10m_fdx = FALSE; 
           }
           break;
        default:
           cterr('f', 0, "VTS: SET_SPEED_DUPLEX: Err: unknown speed %d",
                        phy_config->aneg_speed);
           return (FALSE);
       } /* End Speed handling */
    } /* End autoneg mode handling */

    if ((&phy_oper_conf)->mode == VTSS_PHY_MODE_FORCED) {
       switch (phy_config->speed) {
       case DEV_VTSS_SPEED_10MB:
           if (phy_config->duplex == DEV_VTSS_DUPLEX_TYPE_FULL) {
              (&phy_oper_conf)->forced.speed = VTSS_SPEED_10M;
              (&phy_oper_conf)->forced.fdx = TRUE;
           }
           if (phy_config->duplex == DEV_VTSS_DUPLEX_TYPE_HALF) {
              (&phy_oper_conf)->forced.speed = VTSS_SPEED_10M;
              (&phy_oper_conf)->forced.fdx = FALSE;
           }
           break;
       case DEV_VTSS_SPEED_100MB:
           if (phy_config->duplex == DEV_VTSS_DUPLEX_TYPE_FULL) {
              (&phy_oper_conf)->forced.speed = VTSS_SPEED_100M;
              (&phy_oper_conf)->forced.fdx = TRUE;
           }
           if (phy_config->duplex == DEV_VTSS_DUPLEX_TYPE_HALF) {
              (&phy_oper_conf)->forced.speed = VTSS_SPEED_100M;
              (&phy_oper_conf)->forced.fdx = FALSE;
           }
           break;
       case DEV_VTSS_SPEED_1GB:
           (&phy_oper_conf)->forced.speed = VTSS_SPEED_1G;
           (&phy_oper_conf)->forced.fdx = TRUE;
           break;   
       default:
           cterr('f', 0, "VTS: SET_SPEED_DUPLEX: Err: unknown speed %d",
                        phy_config->speed);
           return (FALSE);
       }
    }
    /* Program the Phy with new parameteres  */
    rc = vtss_phy_conf_set(api_ctlr->vsc_instance,
         (vtss_port_no_t)port_num, &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: SET Speed/Duplex"
                    "phy_conf_set for autoneg mode failed rc = %d", rc);
       return (FALSE);
    }
    return (TRUE);
}


vtss_phy_conf_t vtss_phy_default_conf;

static vtss_phy_conf_t * 
init_cu_default_phy_param (void)
{
    vtss_phy_default_conf.aneg.speed_10m_hdx = TRUE;
    vtss_phy_default_conf.aneg.speed_10m_fdx = TRUE;
    vtss_phy_default_conf.aneg.speed_100m_hdx = TRUE;
    vtss_phy_default_conf.aneg.speed_100m_fdx = TRUE;
    vtss_phy_default_conf.aneg.speed_1g_fdx = TRUE;
    vtss_phy_default_conf.aneg.speed_1g_hdx = TRUE;
//     vtss_phy_default_conf.aneg.symmetric_pause = TRUE;
//     vtss_phy_default_conf.aneg.asymmetric_pause = TRUE;

    vtss_phy_default_conf.forced.speed = VTSS_SPEED_1G;
    vtss_phy_default_conf.forced.fdx = TRUE;
    vtss_phy_default_conf.mdi = VTSS_PHY_MDIX_AUTO;

    vtss_phy_default_conf.mode = VTSS_PHY_MODE_FORCED;
    return (&vtss_phy_default_conf);
}

static vtss_phy_conf_t *
init_sfp_default_phy_param (void)
{
    vtss_phy_default_conf.aneg.speed_10m_hdx = TRUE;
    vtss_phy_default_conf.aneg.speed_10m_fdx = TRUE;
    vtss_phy_default_conf.aneg.speed_100m_hdx = TRUE;
    vtss_phy_default_conf.aneg.speed_100m_fdx = TRUE;
    vtss_phy_default_conf.aneg.speed_1g_fdx = TRUE;
    vtss_phy_default_conf.aneg.speed_1g_hdx = TRUE;
//     vtss_phy_default_conf.aneg.symmetric_pause = TRUE;
//     vtss_phy_default_conf.aneg.asymmetric_pause = TRUE;

    vtss_phy_default_conf.forced.speed = VTSS_SPEED_1G;
    vtss_phy_default_conf.forced.fdx = TRUE;
    vtss_phy_default_conf.mdi = VTSS_PHY_MDIX_AUTO;

    vtss_phy_default_conf.mode = VTSS_PHY_MODE_FORCED;
    return (&vtss_phy_default_conf);
}

static BOOL
vts_set_autoneg_mode (dev_vsc8x_object_t *phy, u16 port_num,
                      dev_vsc8x_port_config_t *phy_config)
{
    vsc_api_controller_t *api_ctlr;
    vtss_phy_conf_t phy_oper_conf;
    BOOL rc;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: SET Autoneg"
                    "phy object is not ready yet");
       return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    rc = vtss_phy_conf_get(api_ctlr->vsc_instance,
                         (vtss_port_no_t)port_num,
                                 &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: SET Autonegx"
                    "phy_conf_get failed rc = %d", rc);
       return (FALSE);
    }
    compare_and_set_autoneg_mode(&phy_oper_conf, phy_config);
    rc = vtss_phy_conf_set(api_ctlr->vsc_instance,
                   (vtss_port_no_t)port_num, &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "VSC API: SET Speed/Duplex"
                     "phy_conf_set for autoneg mode failed rc = %d", rc);
        return (FALSE);
    }
    return (TRUE);
}

static BOOL 
vts_set_media_type (dev_vsc8x_object_t *phy, u16 port_num,
                    dev_vsc8x_port_config_t *phy_config)
{

    vsc_api_controller_t *api_ctlr;
    uint16 reg23, reg31, reg18g;
    uint16 data, wait;
    uint16 media_type;
    BOOL reset = TRUE;

    if (!(phy->vsc_api_control)) {
        cterr('f', 0, "VSC:API control failed, port = %d", port_num);
        return (FALSE);
    }

    /*
     * To configure PHY in 100FX or 1000X mode
     * ----------------------------------------
     * 1. write to register 31 = 0x10               // GPIO page 0x10
     * 2. write to register 19G bits 15:14 = 00     // enable SGMII
     * 3. write to register 18G = 0x80F0            // enable SGMII
     *    read 18G till 0x00F0
     * 4. write to register 18G = 0x8FD1            // enable 100FX (100M SFP)
     *    read 18G till 0x0FD1
     * 5. write to register 31 = 0x0                // main page 0
     * 6. write to register 23 bits 10:8 = 011      // enable 100FX mode
     *    23.10:8 will have prev value till reset
     * 7. write to register 0 bit 15 = 1            // sw reset to change mode
     */

    api_ctlr = phy->vsc_api_control;
    api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                              VSC85XX_EXT_PHY_CONTROL_1, &reg23);
    media_type = reg23 & VSC_MEDIA_TYPE_MASK;

    if (phy_config->media == DEV_VSCG_MODE_100BASE_FX_ONLY) {
        if (media_type == VSC_MEDIA_TYPE_100BASE_FX) {
            /* media_type already set to 100FX */
            reset = FALSE;
        } else {
            // register 31
            api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_PHY_EXT_REG_PAGE, &reg31);
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_PHY_EXT_REG_PAGE,
                                      VSC85XX_PHY_EXT_REG_PAGE_16);

            // Register 19G
            api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_GPIO_CONTROL_3, &data);
            printf("VSC:API media_type 100FX, port=%d, r13=0x%x",
                 port_num, data);

            // Register 18G
            api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_COMMAND_REGISTER, &reg18g);
            printf("VSC:API media_type 100FX, port=%d, r12=0x%x",
                 port_num, reg18g);

            /*
             * Bit 15 tells internal processor to execute the command.
             * Bit 15 is cleared when the command has completed.
             * Software needs to wait until bit 15 = 0.
             */
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_COMMAND_REGISTER,
                                      VSC85XX_ENABLE_100FX);

            wait = 0;
            do {
                if (wait > 100) {
                    printf("VSC:API CMD_REG timeout, 100FX port=%d, reg18g=0x%x",
                        port_num, reg18g);
                    break;
                }
                wait++;
                usleep(VSC85XX_MICRO_CMD_DELAY);
                api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                          VSC85XX_COMMAND_REGISTER, &reg18g);
            } while (reg18g & VSC85XX_MICRO_CMD_BEGIN);
            // Register 31
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_PHY_EXT_REG_PAGE, reg31);

            // Register 23
            reg23 &= ~VSC_MEDIA_TYPE_MASK;
            reg23 |= VSC_MEDIA_TYPE_100BASE_FX;
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_EXT_PHY_CONTROL_1, reg23);
        }
    } else if (phy_config->media == DEV_VSCG_MODE_COPPER_ONLY) {
        if (media_type == VSC_MEDIA_TYPE_PASS_THRU) {
            reset = FALSE;
        } else {
            printf("VSC:API media_type pass-thru, port=%d", port_num);
            data &= ~VSC_MEDIA_TYPE_MASK;
            data |= VSC_MEDIA_TYPE_PASS_THRU;
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_EXT_PHY_CONTROL_1, data);
        }
    } else {
        if (media_type == VSC_MEDIA_TYPE_1000BASE_X) {
            /* media_type already set to 1000X */
            reset = FALSE;
        } else {
            // register 31
            api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_PHY_EXT_REG_PAGE, &reg31);
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_PHY_EXT_REG_PAGE,
                                      VSC85XX_PHY_EXT_REG_PAGE_16);

            // Register 19G
            api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_GPIO_CONTROL_3, &data);
            printf("VSC:API media_type 1000X, port=%d, r13=0x%x",
                 port_num, data);

            // Register 18G
            api_ctlr->vsc_init_config.miim_read(api_ctlr->vsc_instance,  port_num,
                                      VSC85XX_COMMAND_REGISTER, &reg18g);
            printf("VSC:API media_type 1000X, port=%d, r12=0x%x",
                 port_num, reg18g);

            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_COMMAND_REGISTER,
                                      VSC85XX_ENABLE_1000X);
            wait = 0;
            do {
                if (wait > 100) {
                    printf("VSC:API CMD_REG timeout 1000X, port=%d, reg18g=0x%x",
                        port_num, reg18g);
                    break;
                }
                wait++;
                usleep(VSC85XX_MICRO_CMD_DELAY);
                api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num,
                                          VSC85XX_COMMAND_REGISTER, &reg18g);
            } while (reg18g & VSC85XX_MICRO_CMD_BEGIN);

            // Register 31
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_PHY_EXT_REG_PAGE, reg31);

            // Register 23
            reg23 &= ~VSC_MEDIA_TYPE_MASK;
            reg23 |= VSC_MEDIA_TYPE_1000BASE_X;
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                      VSC85XX_EXT_PHY_CONTROL_1, reg23);
            api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                                VSC85XX_PHY_EXT_REG_PAGE,
                                                VSC85XX_PHY_EXT_REG_PAGE_0);
        }
    }

    if (reset) {
        printf("VSC:API SW reset for media_type, port=%d", port_num);
        /*
         * Reset the PHY, write (register 0, bit 15)
         * Must wait for atleast 4microsec before another register access
         */
        api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num,
                                  VSC85XX_MODE_CONTROL, VSC85XX_SW_RESET);
        usleep(VSC85XX_SW_RESET_DELAY*1000);
    }
    return (TRUE);
}

static BOOL
vts_get_autoneg_mode (dev_vsc8x_object_t *phy, u16 port_num,
                      dev_vsc8x_port_config_t *phy_config)
{
    vsc_api_controller_t *api_ctlr;
    vtss_phy_conf_t phy_oper_conf;
    BOOL rc;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: GET Autoneg"
                    "phy object is not ready yet");
       return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    rc = vtss_phy_conf_get(api_ctlr->vsc_instance,
                         (vtss_port_no_t)port_num,
                                 &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: GET Autonegx"
                    "phy_conf_get failed rc = %d", rc);
        return (FALSE);
    }

    if (phy_oper_conf.mode == VTSS_PHY_MODE_ANEG) {
        phy_config->aneg_enable = DEV_VTSS_PHY_MODE_ANEG;
    } else if (phy_oper_conf.mode == VTSS_PHY_MODE_FORCED) {
        phy_config->aneg_enable = DEV_VTSS_PHY_MODE_FORCED;
    } else {
        phy_config->aneg_enable = DEV_VTSS_PHY_MODE_UNKNOWN;
    }

   return (TRUE);
}

static BOOL
vts_get_speed_duplex (dev_vsc8x_object_t *phy, 
                      u16 port_num, 
                      dev_vsc8x_port_config_t *phy_config)
{
    vsc_api_controller_t *api_ctlr;
    vtss_phy_conf_t phy_oper_conf;
    BOOL rc;
    vtss_port_status_t phy_status;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: GET speed duplex"
                    "phy object is not ready yet");
       return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    rc = vtss_phy_conf_get(api_ctlr->vsc_instance,
                         (vtss_port_no_t)port_num,
                                 &phy_oper_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: GET speed duplex"
                    "phy_conf_get failed rc = %d", rc);
       return (FALSE);
    }

    rc = vtss_phy_status_get(api_ctlr->vsc_instance,
            (vtss_port_no_t)port_num,
            &phy_status);

    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VSC API: GET speed duplex"
           "phy_status_get failed rc = %d", rc);
       return (FALSE);
    }

    if ((&phy_oper_conf)->mode == VTSS_PHY_MODE_ANEG) {
       /*
        * If autoneg was on then we need to get allowed
        * negotiated speed and duplex.
        */
        phy_config->aneg_enable = DEV_VTSS_PHY_MODE_ANEG;

        if (phy_status.speed == VTSS_SPEED_10M) {
            phy_config->aneg_speed = DEV_VTSS_SPEED_10MB;
        } else if (phy_status.speed == VTSS_SPEED_100M) {
            phy_config->aneg_speed = DEV_VTSS_SPEED_100MB;
        } else if (phy_status.speed == VTSS_SPEED_1G) {
            phy_config->aneg_speed = DEV_VTSS_SPEED_1GB;
        } else if (phy_status.speed == VTSS_SPEED_10G) {
            phy_config->aneg_speed = DEV_VTSS_SPEED_10GB;
        } else {
            phy_config->aneg_speed = DEV_VTSS_SPEED_AUTO;
        }

        phy_config->aneg_duplex = phy_status.fdx;

        printf("\n Port = %x Speed=%x DUPLEX=%x ANEG", port_num,
            phy_status.speed, phy_status.fdx);

        return (TRUE);
    } /* End autoneg mode handling */

    if ((&phy_oper_conf)->mode == VTSS_PHY_MODE_FORCED) {
        phy_config->aneg_enable = DEV_VTSS_PHY_MODE_FORCED;

        if (phy_status.speed == VTSS_SPEED_10M) {
            phy_config->speed = DEV_VTSS_SPEED_10MB;
        } else if (phy_status.speed == VTSS_SPEED_100M) {
            phy_config->speed = DEV_VTSS_SPEED_100MB;
        } else if (phy_status.speed == VTSS_SPEED_1G) {
            phy_config->speed = DEV_VTSS_SPEED_1GB;
        } else if (phy_status.speed == VTSS_SPEED_10G) {
            phy_config->speed = DEV_VTSS_SPEED_10GB;
        } else {
            phy_config->speed = DEV_VTSS_SPEED_AUTO;
        }

       phy_config->duplex = phy_status.fdx;

       printf("\n Port = %x Speed=%x DUPLEX=%x FORCE", port_num,
            phy_status.speed, phy_status.fdx);

       return (TRUE);
    }
    cterr('f', 0, "Err: Unknow speed returning port(%d)", port_num);
    return (FALSE);
}

static BOOL 
vts_set_one_feature (dev_vsc8x_object_t *phy, u16 port_num,
                     vts_phy_port_feature_t feature,
                     dev_vsc8x_port_config_t phy_config)
{
    BOOL result = TRUE;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: Set one feature "
                    "phy object is not ready yet");
       return (FALSE);
    }
    if (phy->vsc_api_control->platform_target 
                     != VTSS_TARGET_CU_PHY) {
        cterr('f', 0, "VSC API: Set one feature "
                    "Invalid feature for 10G phy");
       return (FALSE);
    }

    switch (feature) {
    case vts_phy_set_speed_type:
    case vts_phy_set_duplex_type:
        result = vts_set_speed_duplex(phy,port_num, &phy_config);
        break;
    case vts_phy_set_autoneg_enable_type:
        result = vts_set_autoneg_mode(phy,port_num, &phy_config);
        break; 
    case vts_phy_set_media_type:
        result = vts_set_media_type(phy,port_num, &phy_config);
        break;
    case vts_phy_set_phy_sfp_mode:
        /*
         * phy_sfp_mode should be controlled through init params
         * as it causes phy reset and there is no need to change
         * it dynamically as for one type of HW it will be fixed
         */
    default:
        cterr('f', 0, "VSC API: Set one feature "
                     "Invalid feature %d", feature);
        result = FALSE;
    }
    return (result);
}

static BOOL 
vts_get_one_feature (dev_vsc8x_object_t *phy, u16 port_num,
                     vts_phy_port_feature_t feature,
                     dev_vsc8x_port_config_t *phy_config)
{
    BOOL result=TRUE;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: get one feature "
                    "phy object is not ready yet");
       return (FALSE);
    }
    if (phy->vsc_api_control->platform_target 
                     != VTSS_TARGET_CU_PHY) {
       cterr('f', 0, "VSC API: Get one feature "
                    "Invalid feature for 10G phy");
       return (FALSE);
    }

    switch (feature) {
    case vts_phy_get_speed_type:
    case vts_phy_get_duplex_type:
        result = vts_get_speed_duplex(phy, port_num, phy_config);
        break;
    case vts_phy_ang_status:
        result = vts_get_autoneg_mode(phy, port_num, phy_config);
        break; 
    default:
        cterr('f', 0, "VSC API: Get one feature "
                     "Invalid feature %d", feature);
        result = FALSE;
    }
    return (result);
}

static BOOL 
vts_get_oper_state (dev_vsc8x_object_t *phy, u16 port_num,
                    dev_vsc8x_port_config_t *phy_config)
{
   return (TRUE);
}

static BOOL 
vts_set_loopback (dev_vsc8x_object_t *phy, u16 port_num,
                  dev_vts_phy_loopback_type_t *loopback_type)
{
/*    vsc_api_controller_t *api_ctlr;
    vtss_rc rc = EOK;
    vtss_phy_10g_loopback_t loop_10g = loopback_type->loopback_10g;

    api_ctlr = phy->vsc_api_control;
    if (phy->vsc_api_control->platform_target == 
                                VTSS_TARGET_CU_PHY) {
        cterr('f', 0, "\nloopback not supported for 1G");        
        return (FALSE);
    } else if (phy->vsc_api_control->platform_target ==
                                VTSS_TARGET_10G_PHY) {
    rc = vtss_phy_10g_loopback_set(api_ctlr->vsc_instance, port_num, &loop_10g);
        if (rc != EOK) {
            return (FALSE);
        }
        return (TRUE);
    }
    return (FALSE);*/
    return (TRUE);
}

static BOOL 
vts_get_loopback (dev_vsc8x_object_t *phy, u16 port_num,
                  dev_vts_phy_loopback_type_t *loopback_type)
{
/*    vsc_api_controller_t *api_ctlr;
    vtss_rc rc = EOK;
    vtss_phy_10g_loopback_t loop_10g;

    api_ctlr = phy->vsc_api_control;
    if (phy->vsc_api_control->platform_target == 
                                VTSS_TARGET_CU_PHY) {
        cterr('f', 0, "\nloopback not supported for 1G");        
        return (FALSE);
    } else if (phy->vsc_api_control->platform_target ==
                                VTSS_TARGET_10G_PHY) {
        rc = vtss_phy_10g_loopback_get(api_ctlr->vsc_instance, port_num, &loop_10g);
        if (rc != EOK) {
            return (FALSE);
        }
        loopback_type->loopback_10g = loop_10g;
        return (TRUE);
    }
    return (FALSE);*/
    return (TRUE);
}

static BOOL
vts_1g_link_led (dev_vsc8x_object_t *phy, u16 port_num,
                 dev_vscg_led_signals *led_signals)
{
    vsc_api_controller_t *api_ctlr;
    u16 data;

    if (!(phy->vsc_api_control)) {
        cterr('f', 0, "VSC:API control failed, port = %d", port_num);
        return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    api_ctlr->vsc_init_config.miim_read( api_ctlr->vsc_instance, port_num, VSC_LED_MOD_SEL, &data);

    if (api_ctlr->phy_reset_config->media_if == VTSS_PHY_MEDIA_IF_FI_1000BX) {
        switch (led_signals->led_dest_signals[0]) {
        case VSC_LED_OFF:
            data = VSC_ALL_LED_OFF;
            break;

        case VSC_LED_GREEN: /* LED Green ON : pin0 low, pin1 high. */
            data = ((data & ~VSC_LED0_MASK) | VSC_LED0_ON);     /* pin 0 low */
            data = ((data & ~VSC_LED1_MASK) | VSC_LED1_OFF);    /* pin 1 high */
            break;

        case VSC_LED_AMBER: /* LED Amber ON : pin0 high, pin1 low. */
            data = ((data & ~VSC_LED0_MASK) | VSC_LED0_OFF);    /* pin 0 high */
            data = ((data & ~VSC_LED1_MASK) | VSC_LED1_ON);     /* pin 1 low */
            break;

        default:
            cterr('f', 0, "VSC:LED invalid SFP mode, port = %d, mode = %d",
                         port_num, led_signals->led_dest_signals[0]);
            return (FALSE);
            break;
        }
    } else if (api_ctlr->phy_reset_config->media_if == VTSS_PHY_MEDIA_IF_CU) {
        switch (led_signals->led_dest_signals[0]) {
        case VSC_LED_OFF:
            data = VSC_ALL_LED_OFF;
            break;

        case VSC_LED_GREEN: /* LED Green ON : pin0 high, pin1 low. */
            data = ((data & ~VSC_LED0_MASK) | VSC_LED0_OFF);    /* pin 0 high */
            data = ((data & ~VSC_LED1_MASK) | VSC_LED1_ON);     /* pin 1 low */
            break;

        case VSC_LED_AMBER: /* LED Amber ON : pin0 low, pin1 high. */
            data = ((data & ~VSC_LED0_MASK) | VSC_LED0_ON);     /* pin 0 low */
            data = ((data & ~VSC_LED1_MASK) | VSC_LED1_OFF);    /* pin 1 high */
            break;

        default:
            cterr('f', 0, "VSC:LED invalid CU mode, port = %d, mode = %d",
                         port_num, led_signals->led_dest_signals[0]);
            return (FALSE);
            break;
        }
    }

    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num, VSC_LED_MOD_SEL, data);
    return (TRUE);
}

static BOOL
vts_1g_speed_led (dev_vsc8x_object_t *phy, u16 port_num,
                  dev_vscg_led_signals *led_signals)
{
    vsc_api_controller_t *api_ctlr;
    u16 data;

    if (!(phy->vsc_api_control)) {
        cterr('f', 0, "VSC:API control failed, port = %d", port_num);
        return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    api_ctlr->vsc_init_config.miim_read(api_ctlr->vsc_instance,  port_num, VSC_LED_MOD_SEL, &data);

    if (led_signals->led_dest_signals[2] == VSC_LED_GREEN) {
        /* LED ON : pin2 low. (Green) */
        data = ((data & ~VSC_LED2_MASK) | VSC_LED2_ON);
    } else {
        /* LED OFF : pin2 high. */
        data = ((data & ~VSC_LED2_MASK) | VSC_LED2_OFF);
    }

    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_num, VSC_LED_MOD_SEL, data);
    return (TRUE);
}

static BOOL
vts_set_led_color (dev_vsc8x_object_t *phy, u16 port_num,
                   dev_vscg_led_signals *led_signals)
{
    vsc_api_controller_t *api_ctlr;

    if (!(phy->vsc_api_control)) {
        cterr('f', 0, "VSC:API control failed, port = %d", port_num);
        return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;

    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        /* CU & SFP IMs */
        vts_1g_link_led(phy, port_num, led_signals);
        vts_1g_speed_led(phy, port_num, led_signals);
    }

    return (TRUE);
}

static BOOL
vts_is_workaround_reqd (dev_vsc8x_object_t *phy, u16 port_num,
                        dev_vsc8x_workaround_t workaround)
{
    return (TRUE);
}

static BOOL
vts_workaround_apply (dev_vsc8x_object_t *phy, u16 port_num,
                      dev_vsc8x_workaround_t workaround)
{
   return (TRUE);
}

static vsc8x_link_status_t
vts_get_link_status (dev_vsc8x_object_t *phy, u16 port_num)
{
    vsc_api_controller_t *api_ctlr;
    vtss_rc rc;

    if (!phy || !(phy->vsc_api_control)) {
       cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                    "phy object is not ready yet");
       return (DEV_VSC_LINK_STATUS_UNAVAIL);
    }
    api_ctlr = phy->vsc_api_control;
    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        vtss_port_status_t      phy_status;
        rc = vtss_phy_status_get(api_ctlr->vsc_instance,
                              (vtss_port_no_t)port_num,
                                     &phy_status);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "VSC:GET_LINK_STATUS: phy_status_get"
                       "failed rc = %d", rc);
            return (DEV_VSC_LINK_STATUS_UNAVAIL);
        }
        if ((&phy_status)->link == TRUE) {
            return (DEV_VSC_LINK_STATUS_UP);
        }
        return (DEV_VSC_LINK_STATUS_DN);
    }
#if 0
    if (api_ctlr->platform_target == VTSS_TARGET_10G_PHY) {
        vtss_phy_10g_status_t   phy_status;
        vtss_phy_10g_mode_t  phy_mode;

        /* First get the operating mode of the PHY, WAN or LAN */
        rc = vtss_phy_10g_mode_get(api_ctlr->vsc_instance,
                              (vtss_port_no_t)port_num,
                                            &phy_mode);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "VSC:GET_LINK_STATUS: phy 10g mode get"
                      "failed rc = %d", rc);
            return (DEV_VSC_LINK_STATUS_UNAVAIL);
        }
        rc = vtss_phy_10g_status_get(api_ctlr->vsc_instance,
                              (vtss_port_no_t)port_num,
                                            &phy_status);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "VSC:GET_LINK_STATUS: phy 10g status get"
                      "failed rc = %d", rc);
            return (DEV_VSC_LINK_STATUS_UNAVAIL);
        }
        switch (phy_mode.oper_mode) {
        case VTSS_PHY_LAN_MODE:
            if (phy_status.pcs.rx_link) {
                return (DEV_VSC_LINK_STATUS_UP);
            } else {
                return (DEV_VSC_LINK_STATUS_DN);
            }
            break;
        case VTSS_PHY_WAN_MODE:
            if (phy_status.wis.rx_link) {
                return (DEV_VSC_LINK_STATUS_UP);
            } else {
                return (DEV_VSC_LINK_STATUS_DN);
            }
            break;
        default:
            return (DEV_VSC_LINK_STATUS_UNAVAIL);
        }
    } /* END: 10G link stauts handling */
#endif
    /* Should never come here */
    return (DEV_VSC_LINK_STATUS_UNAVAIL);
}

static BOOL
vts_set_one_feature_raw (dev_vsc8x_object_t *phy, u16 port_num,
                         vts_phy_port_feature_t feature, void *param)
{
    return (TRUE);
}

static BOOL
vts_set_recover_clk (dev_vsc8x_object_t *phy, u16 port_num,
                     BOOL primary, BOOL enable)
{
    vsc_api_controller_t *api_ctlr;
    vtss_phy_clock_conf_t clk_cfg;
    u16 port;

    vtss_phy_conf_1g_t conf_1g;
    vtss_rc rc;

    if (!phy || !(phy->vsc_api_control)) {
        cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                      "phy object is not ready yet");
        return (FALSE);
    }

    api_ctlr = phy->vsc_api_control;
    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        printf("Set up SYNCE on 1G PHY for port = %d", port_num);
        if (api_ctlr->phy_reset_config->media_if ==
                                            VTSS_PHY_MEDIA_IF_CU) {
            if (enable == TRUE) {
                /*
                 * For Cu Phy force to Auto Neg Slave
                 */
                conf_1g.master.cfg = 1;
                conf_1g.master.val = 0;
            } else {
                /*
                 * For Cu Phy force to Auto Neg Master
                 */
                conf_1g.master.cfg = 1;
                conf_1g.master.val = 1;
            }
            rc =  vtss_phy_conf_1g_set(api_ctlr->vsc_instance, port_num,
                                       &conf_1g);
            if (rc != VTSS_RC_OK) {
                cterr('f', 0, "Failed to force 1G PHY as SLAVE = %d", port_num);
                return (FALSE);
            }
            clk_cfg.src = VTSS_PHY_COPPER_MEDIA;
        } else {
            clk_cfg.src = VTSS_PHY_SERDES_MEDIA;
        }
        if (enable == TRUE) {
            clk_cfg.freq = VTSS_PHY_FREQ_25M;
            clk_cfg.squelch = VTSS_PHY_CLK_SQUELCH_NONE;
        } else {
            clk_cfg.src = VTSS_PHY_CLK_DISABLED;
        }
        port = wallander_port_to_phyport_mapping(port_num);
        if (primary) {
            rc = vtss_phy_clock_conf_set(api_ctlr->vsc_instance, port,
                                         VTSS_PHY_RECOV_CLK1, &clk_cfg);
        } else {
            rc = vtss_phy_clock_conf_set(api_ctlr->vsc_instance, port,
                                         VTSS_PHY_RECOV_CLK2, &clk_cfg);
        }

        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "Failed to set up SYNCE on 1G PHY for "
                         "port = %d phy =%d", port_num, port);
            return (FALSE);
        } else {
            printf("Set up SYNCE on 1G PHY for "
                         "port = %d phy =%d", port_num, port);
        }
    } else {
        cterr('f', 0, "Unsupported");
        return (FALSE);
    }
    return (TRUE);
}

static BOOL
vts_set_sync_mode (dev_vsc8x_object_t *phy, u16 port_num,
                   BOOL enable)
{
    vsc_api_controller_t *api_ctlr;
    vtss_phy_conf_1g_t conf_1g;
    vtss_rc rc;

    if (!phy || !(phy->vsc_api_control)) {
        cterr('f', 0, "VSC API: ENABLE/DISABLE linki phy object "
                     "is not ready yet");
        return (FALSE);
    }
    api_ctlr = phy->vsc_api_control;
    if ((api_ctlr->platform_target == VTSS_TARGET_CU_PHY) &&
        (api_ctlr->phy_reset_config->media_if ==
                                         VTSS_PHY_MEDIA_IF_CU)) {
        /*
         * For Cu Phy force to Auto Neg Master
         */
        printf("Configure Synchronous %d mode for port %d",
                     enable, port_num);
        if (enable) {
            conf_1g.master.cfg = 1;
            conf_1g.master.val = 1;
        } else {
            conf_1g.master.cfg = 0;
            conf_1g.master.val = 0;
        }
        rc =  vtss_phy_conf_1g_set(api_ctlr->vsc_instance, port_num, &conf_1g);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "Failed to force 1G PHY as SLAVE = %d", port_num);
            return (FALSE);
        }

    } else {
        cterr('f', 0, "Unsupported SPA");
        return (FALSE);
    }
    return (TRUE);
}

static dev_vts8x_phy_callin_t vts_fvt = {
    .enable_disable_link       = vts_enable_disable_link,
    .set_one_feature           = vts_set_one_feature,
    .get_one_feature           = vts_get_one_feature,
    .get_oper_state            = vts_get_oper_state,
    .set_loopback              = vts_set_loopback,
    .get_loopback              = vts_get_loopback,
    .set_led_color             = vts_set_led_color,
    .is_workaround_reqd        = vts_is_workaround_reqd,
    .workaround_apply          = vts_workaround_apply,
    .get_link_status           = vts_get_link_status,
    .set_one_feature_raw       = vts_set_one_feature_raw,
    .set_recover_clk           = vts_set_recover_clk,
    .set_sync_mode             = vts_set_sync_mode,
};

static dev_vts8x_phy_callin_t vscphy_do_nothing_fvt = {
    .enable_disable_link       =
        (dev_vsc8x_callin_enable_disable_link_fn)dev_do_nothing,
    .set_one_feature           =
        (dev_vsc8x_callin_set_one_feature_fn)dev_do_nothing,
    .get_one_feature           =
        (dev_vsc8x_callin_get_one_feature_fn)dev_do_nothing,
    .get_oper_state            =
        (dev_vsc8x_callin_get_oper_state_fn)dev_do_nothing,
    .set_loopback              =
        (dev_vsc8x_callin_loopback_fn)dev_do_nothing,
    .get_loopback              =
        (dev_vsc8x_callin_loopback_fn)dev_do_nothing,
    .set_led_color             =
        (dev_vsc8x_callin_set_led_color_fn)dev_do_nothing,
    .is_workaround_reqd        =
        (dev_vsc8x_is_workaround_reqd_fn)dev_do_nothing,
    .workaround_apply          =
        (dev_vsc8x_workaround_apply_fn)dev_do_nothing,
    .get_link_status           =
        (dev_vsc8x_callin_get_link_status_fn)dev_do_nothing,
    .set_one_feature_raw       =
        (dev_vsc8x_callin_set_one_feature_raw_fn)dev_do_nothing,
    .set_recover_clk           =
    (dev_vsc8x_callin_set_recover_clk)dev_do_nothing,
    .set_sync_mode           =
        (dev_vsc8x_callin_set_sync_mode)dev_do_nothing,

};

static dev_status 
vscphy_dev_attach (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
//     dev_base_attach(dev);
    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc) {
        cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
        return (DEV_STATUS_INVALID_ARG);
    }
    dev_vsc->callins = &vts_fvt;
    dev_vsc->callin_10g = NULL;

    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_detach (dev_object_t *dev)
{
   dev_vsc8x_object_t *dev_vsc;

//   dev_base_detach(dev);
   dev_vsc = (dev_vsc8x_object_t *)dev;
   if (!dev_vsc) {
      cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
      return (DEV_STATUS_INVALID_ARG);
   }   
   dev_vsc->callins = &vscphy_do_nothing_fvt;
   dev_vsc->callin_10g = NULL;

   return (DEV_STATUS_SUCCESS);
}

#define WALLANDER_PHY_PORT0   0
#define WALLANDER_PHY_PORT1   1
#define WALLANDER_PHY_PORT2   2
#define WALLANDER_PHY_PORT3   3

/*
 * vsc_1gphy_ts_upper_shared_port
 *
 * There are 2 timstamping blocks in the quad PHY VSC 8574.
 * Each block is shared by 2 ports. When engine_init is done,
 * either of the 2 ports can be passed to Vitesse API
 * This function takes a IM front panel port number and returns
 * true if it shares an engine with another port which port number
 * is less than it. Assumption is that port init is done in a loop
 * for all the ports and we skip the engine_init for the 2nd port
 * which has a higher numerical value
 */
BOOL
vsc_1gphy_ts_upper_shared_port (u32 port_num)
{
    switch (port_num) {
    case WALLANDER_PHY_PORT2: /* shared with 0 */
    case WALLANDER_PHY_PORT3: /* shared with 1 */
        /* intentional fall throughs */
        return (TRUE);
    default:
        return (FALSE);
    }
}

static vtss_phy_ts_init_conf_t vtss_phy_default_ts_init_conf;

static vtss_phy_ts_init_conf_t *
init_default_1g_ts_init_conf (void)
{
    vtss_phy_default_ts_init_conf.clk_freq =  VTSS_PHY_TS_CLOCK_FREQ_125M;
    vtss_phy_default_ts_init_conf.clk_src  = VTSS_PHY_TS_CLOCK_SRC_EXTERNAL;
    vtss_phy_default_ts_init_conf.rx_ts_pos = 
            VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP;
    vtss_phy_default_ts_init_conf.rx_ts_len = 
            VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT;
    vtss_phy_default_ts_init_conf.tx_fifo_mode = VTSS_PHY_TS_FIFO_MODE_SPI;
    vtss_phy_default_ts_init_conf.tx_ts_len = 
            VTSS_PHY_TS_FIFO_TIMESTAMP_LEN_10BYTE;
    return (&vtss_phy_default_ts_init_conf);
}

dev_status 
vsc_1gphy_ts_ptp_init (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;
    int rc;
    vtss_port_no_t port_no;
    vtss_phy_ts_init_conf_t *phy_ts_init_conf;
    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc) {
       cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    /* Check for VSC instance before calling vitesse API */
    if (!(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "%s : VSC instance is not initialized"
                    "call dev_create first" , __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    api_ctlr = dev_vsc->vsc_api_control;

    port_no = (vtss_port_no_t)dev_vsc->port;

    phy_ts_init_conf = init_default_1g_ts_init_conf();

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_init port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_init(api_ctlr->vsc_instance, port_no,
                          phy_ts_init_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "TS-Eng: VTS PHY TS INIT failed, rc=%d", rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_mode_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_mode_set(api_ctlr->vsc_instance, port_no,
                              TRUE);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "TS-Eng: VTS PHY TS MODE INIT failed, rc=%d", rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    return (DEV_STATUS_SUCCESS);
}

static u32
get_ip_addr_from_env (const char* env)
{
    u32 ip_add = 1;

    return (ip_add);
}

static vtss_phy_ts_engine_flow_conf_t vtss_phy_default_in_eng_flow_conf;

static vtss_phy_ts_engine_flow_conf_t *
init_1g_default_ts_in_eng_flow_conf (void)
{
//     mac_addr_t mac_addr = {0x01, 0x00, 0x5e, 0x0, 0x1, 0x81};
//     mac_addr_t mac_addr = {0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc};
    mac_addr_t mac_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
    memset(&vtss_phy_default_in_eng_flow_conf, 0, 
           sizeof(vtss_phy_ts_engine_flow_conf_t));

    vtss_phy_default_in_eng_flow_conf.eng_mode = TRUE;

    vtss_phy_default_in_eng_flow_conf.channel_map[0] = 
        (VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1 | VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0);
    vtss_phy_default_in_eng_flow_conf.channel_map[1] = 
        (VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1 | VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0);
    /* 
     * Eth1 comparator settings 
     */
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.comm_opt.etype 
                                                    = 0x800; /* IPv4 */
    /* pbb_en, tpid not required */
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].flow_en = TRUE;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].addr_match_mode
        = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST/*VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST*/;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].addr_match_select
        = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].vlan_check 
        = FALSE; /* parse VLAN tag if any, but don't check */
/*    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].tag_range_mode 
        =  VTSS_PHY_TS_TAG_RANGE_NONE;*/
    /* Configure the MAC address of the flow, needs to be time stamped */
/*    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].mac_addr =
           {0x01, 0x00, 0x5e, 0x0, 0x1, 0x81};*/
    memcpy((char *)vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].mac_addr,
           (char *)mac_addr, sizeof(mac_addr_t));

    /* Eth2 comparator is not used */

    /* 
     * IP1 comparator settings 
     */
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.ip_mode 
        = VTSS_PHY_TS_IP_VER_4;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.sport_mask
        = 0x0000; /* Allow all src ports */
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.dport_val
        = PTP_EVENT_MSG_PORT;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.dport_mask
        = /*0xFFFF*/0; /* Strict match dst port */

    /* Flow 0 */
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].flow_en
        = TRUE;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].match_mode
        = VTSS_PHY_TS_IP_MATCH_DEST;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].ip_addr.ipv4.addr
        = get_ip_addr_from_env("PTP_IP_ADDRESS1");
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].ip_addr.ipv4.mask
        = 0xFFFFFF00;

    /* Flow 1 */
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].flow_en
        = TRUE;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].match_mode
        = VTSS_PHY_TS_IP_MATCH_DEST;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].ip_addr.ipv4.addr
        = 0;
    vtss_phy_default_in_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].ip_addr.ipv4.mask
        = 0;

    /* IP2 comparator not used */
    /* MPLS comparator not used */
    /* PTP comparator set using action */
    return (&vtss_phy_default_in_eng_flow_conf);
}

static vtss_phy_ts_engine_flow_conf_t vtss_phy_default_eg_eng_flow_conf;

static vtss_phy_ts_engine_flow_conf_t *
init_1g_default_ts_eg_eng_flow_conf (void)
{
//     mac_addr_t mac_addr = {0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc};
    mac_addr_t mac_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
    memset(&vtss_phy_default_eg_eng_flow_conf, 0, 
           sizeof(vtss_phy_ts_engine_flow_conf_t));

    vtss_phy_default_eg_eng_flow_conf.eng_mode = TRUE;
//     vtss_phy_default_eg_eng_flow_conf.eng_mode = FALSE;

    vtss_phy_default_eg_eng_flow_conf.channel_map[0] = 
         (VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1 | VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0);
    vtss_phy_default_eg_eng_flow_conf.channel_map[1] = 
         (VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1 | VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0);
    /*
     * Eth1 comparator settings 
     */
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.comm_opt.etype 
                                                    = 0x800; /* IPv4 */
    /* pbb_en, tpid not required */
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].flow_en = TRUE;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].addr_match_mode
        = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].vlan_check 
        = FALSE; /* parse VLAN tag if any, but don't check */
/*    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].tag_range_mode 
        =  VTSS_PHY_TS_TAG_RANGE_NONE;*/
/*    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].mac_addr =  
        {0x01, 0x00, 0x5e, 0x0, 0x1, 0x81};*/
    memcpy((char *)vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.eth1_opt.flow_opt[0].mac_addr,
           (char *)mac_addr, sizeof(mac_addr_t));
    /* Eth2 comparator is not used */

    /* 
     * IP1 comparator settings 
     */
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.ip_mode 
        = VTSS_PHY_TS_IP_VER_4;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.sport_mask
        = 0x0000; /* Allow all src ports */
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.dport_val
        = PTP_EVENT_MSG_PORT;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.comm_opt.dport_mask
        = 0xFFFF; /* Strict match dst port */

    /* Flow 0 */
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].flow_en
        = TRUE;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].match_mode
        = VTSS_PHY_TS_IP_MATCH_SRC;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].ip_addr.ipv4.addr
        = get_ip_addr_from_env("PTP_IP_ADDRESS1");
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[0].ip_addr.ipv4.mask
        = 0xFFFFFF00;

    /* Flow 1 */
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].flow_en
        = TRUE;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].match_mode
        = VTSS_PHY_TS_IP_MATCH_SRC;
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].ip_addr.ipv4.addr
        = get_ip_addr_from_env("PTP_IP_ADDRESS1");
    vtss_phy_default_eg_eng_flow_conf.flow_conf.ptp.ip1_opt.flow_opt[1].ip_addr.ipv4.mask
        = 0xFFFFFF00;

    /* IP2 comparator not used */
    /* MPLS comparator not used */
    /* PTP comparator set using action */
    return (&vtss_phy_default_eg_eng_flow_conf);
}

static vtss_phy_ts_engine_action_t vtss_phy_default_in_eng_action_conf;

static vtss_phy_ts_engine_action_t *
init_1g_default_ts_in_eng_action_conf (void) 
{
    memset(&vtss_phy_default_in_eng_action_conf, 0,
           sizeof(vtss_phy_ts_engine_action_t));

    vtss_phy_default_in_eng_action_conf.action_ptp = TRUE;

    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].enable = TRUE;
    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].channel_map = 
        (VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1 | VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0);
    /* ether type is IP for ETH/IP/PTP encapsulation */ 
    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].ptp_conf.range_en = TRUE;
    /* ether type is IP for ETH/IP/PTP encapsulation */
    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].ptp_conf.domain.range.upper = 3; 
    /* ether type is IP for ETH/IP/PTP encapsulation */
    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].ptp_conf.domain.range.lower = 0; 

    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].clk_mode
        = VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP;

    vtss_phy_default_in_eng_action_conf.action.ptp_conf[0].delaym_type 
        = VTSS_PHY_TS_PTP_DELAYM_E2E;

    return (&vtss_phy_default_in_eng_action_conf);
}

static vtss_phy_ts_engine_action_t vtss_phy_default_eg_eng_action_conf;

static vtss_phy_ts_engine_action_t *
init_1g_default_ts_eg_eng_action_conf (void) 
{
    memset(&vtss_phy_default_eg_eng_action_conf, 0,
           sizeof(vtss_phy_ts_engine_action_t));

    vtss_phy_default_eg_eng_action_conf.action_ptp = TRUE; /* FIX_ME */

    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].enable = TRUE;
    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].channel_map = 
        (VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1 | VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0);

    /* ether type is IP for ETH/IP/PTP encapsulation */ 
    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].ptp_conf.range_en = TRUE;
    /* ether type is IP for ETH/IP/PTP encapsulation */
    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].ptp_conf.domain.range.upper = 3; 
    /* ether type is IP for ETH/IP/PTP encapsulation */
    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].ptp_conf.domain.range.lower = 0; 

    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].clk_mode
        = VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP;

    vtss_phy_default_eg_eng_action_conf.action.ptp_conf[0].delaym_type 
        = VTSS_PHY_TS_PTP_DELAYM_E2E;

    return (&vtss_phy_default_eg_eng_action_conf);
}

dev_status 
vsc_1gphy_ts_ptp_engine_conf (dev_object_t *dev,
                               vtss_phy_ts_engine_t in_eng_id,
                               vtss_phy_ts_engine_t eg_eng_id)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;
    int rc;
    vtss_port_no_t port_no;
    vtss_phy_ts_engine_flow_conf_t *in_flow_conf = NULL;
    vtss_phy_ts_engine_flow_conf_t *eg_flow_conf = NULL;
    vtss_phy_ts_engine_action_t *in_action_conf = NULL;
    vtss_phy_ts_engine_action_t *eg_action_conf = NULL;

    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc) {
       cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    /* Check for VSC instance before calling vitesse API */
    if (!(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "%s : VSC instance is not initialized"
                    "call dev_create first" , __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    api_ctlr = dev_vsc->vsc_api_control;

    port_no = (vtss_port_no_t)dev_vsc->port;

    in_flow_conf = init_1g_default_ts_in_eng_flow_conf();

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ingress_engine_conf_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_ingress_engine_conf_set(api_ctlr->vsc_instance, 
                        port_no,
                        in_eng_id,
                        in_flow_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY TS INGRESS FLOW CONF failed for port %d, rc=%d",
                  port_no, rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    eg_flow_conf = init_1g_default_ts_eg_eng_flow_conf();

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_egress_engine_conf_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_egress_engine_conf_set(api_ctlr->vsc_instance, 
                        port_no,
                        eg_eng_id,
                        eg_flow_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY TS EGRESS FLOW CONF failed for port %d, rc=%d",
                  port_no, rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    in_action_conf = init_1g_default_ts_in_eng_action_conf();

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ingress_engine_action_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_ingress_engine_action_set(api_ctlr->vsc_instance, 
                        port_no,
                        in_eng_id,
                        in_action_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY TS INGRESS ACTION CONF failed for port %d, rc=%d",
                  port_no, rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    eg_action_conf = init_1g_default_ts_eg_eng_action_conf();

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_egress_engine_action_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_egress_engine_action_set(api_ctlr->vsc_instance, 
                        port_no,
                        eg_eng_id,
                        eg_action_conf);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY TS EGRESS ACTION CONF failed for port %d, rc=%d",
                  port_no, rc);
       return (DEV_STATUS_INVALID_ARG);
    }

  return (DEV_STATUS_SUCCESS);
}

static vtss_phy_ts_fifo_sig_mask_t vtss_phy_default_ts_sig_conf;
vtss_phy_ts_fifo_sig_mask_t
init_1g_default_sig_mask_conf (void)
{
    vtss_phy_default_ts_sig_conf = (VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID | 
                                   VTSS_PHY_TS_FIFO_SIG_SEQ_ID | 
                                   VTSS_PHY_TS_FIFO_SIG_DEST_IP);

    return (vtss_phy_default_ts_sig_conf);
}

dev_status
vsc_1gphy_ts_ptp_engine_alloc (dev_object_t *dev,
                                vtss_phy_ts_engine_t in_eng_id,
                                vtss_phy_ts_engine_t eg_eng_id)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;
    int rc;
    vtss_port_no_t port_no;

    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc) {
       cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    /* Check for VSC instance before calling vitesse API */
    if (!(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "%s : VSC instance is not initialized"
                    "call dev_create first" , __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    api_ctlr = dev_vsc->vsc_api_control;

    port_no = (vtss_port_no_t)dev_vsc->port;

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ingress_engine_init port=%d in_eng_id=%d eg_eng_id=%d\n\r", 
                port_no, in_eng_id, eg_eng_id);
    }
    rc = vtss_phy_ts_ingress_engine_init(api_ctlr->vsc_instance,
                        port_no,
                        in_eng_id,
                        VTSS_PHY_TS_ENCAP_ETH_IP_PTP,
                        0, /* flow start index */
                        /*DEFAULT_PTP_MAX_FLOWS_10G - 1*/3, /* flow end index */
                        VTSS_PHY_TS_ENG_FLOW_MATCH_ANY);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY TS INGRESS ENG INIT failed, rc=%d", rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_egress_engine_init port=%d \n\r", port_no );
    }
    rc = vtss_phy_ts_egress_engine_init(api_ctlr->vsc_instance,
                        port_no,
                        eg_eng_id,
                        VTSS_PHY_TS_ENCAP_ETH_IP_PTP,
                        0, /* flow start index */
                        /*DEFAULT_PTP_MAX_FLOWS_10G - 1*/3,  /* flow end index */
                        VTSS_PHY_TS_ENG_FLOW_MATCH_ANY);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY TS EGRESS ENG INIT failed, rc=%d", rc);
       return (DEV_STATUS_INVALID_ARG);
    }
    return (DEV_STATUS_SUCCESS);
}

/*
 * vscphy_dev_init : INITIALIZATION ROUTINES
 */

static dev_status 
vsc_1gphy_dev_init (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;
//     vtss_phy_reset_conf_t *phy_reset_conf;
    vtss_phy_conf_t *phy_oper_conf;
    vtss_phy_clock_conf_t *phy_clock_conf;
    vtss_phy_recov_clk_t phy_recov_clk;
    vtss_phy_ts_engine_t in_eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
    vtss_phy_ts_engine_t eg_eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
    vtss_phy_ts_fifo_sig_mask_t phy_ts_init_sig_conf;
    int rc;
//     uchar hw_bld, hw_rev;
    vtss_port_no_t port_no;
//     u16 data;

    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc) {
       cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }

    /* Check for VSC instance before calling viteese API */
    if (!(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "%s : VSC instance is not initialized"
                    "call dev_create first" , __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    api_ctlr = dev_vsc->vsc_api_control;

    port_no = (vtss_port_no_t)dev_vsc->port;


    /* PHY has been reset, configure the PHY */
    phy_clock_conf = api_ctlr->phy_clock_conf;
    phy_recov_clk = *(api_ctlr->phy_recov_clk);

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("API: vtss_phy_clock_conf_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_clock_conf_set(api_ctlr->vsc_instance,port_no,
                                 phy_recov_clk,
                                 phy_clock_conf);
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "VTS PHY CLOCK CONF SETTING failed, rc=%d", rc);
        return (DEV_STATUS_INVALID_ARG);
    }

    /* 
     * Disable recovered clock squelch work-around 
     *
     * Note:This work around code can be removed once VTSS
     * remove the enabled work around from their API
     * API4.04 from vtss still have this work around enabled
     * For details on why we added vtss_squelch_workaround() 
     * call here refer to CDET CSCud70335
     */
    /*
     * Per Vitess, this workaround is included in 
       current API 4.49d, therefore we can remove this 
       function call here.
     */
#if 0
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("API: vtss_squelch_workaround port=%d \n\r", port_no );
    }
    rc = vtss_squelch_workaround(api_ctlr->vsc_instance,port_no, FALSE);
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "VTS PHY RECVRD CLOCK SQUELCH DID NOT DISABLE, rc=%d", rc);
        return (DEV_STATUS_INVALID_ARG);
    }
#endif

    if (!(api_ctlr->phy_config_set)) {
        cterr('f', 0, "1G phy init configs are not set yet");
        return (DEV_STATUS_INVALID_ARG);
    }
    phy_oper_conf = api_ctlr->phy_config_set;

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("API: vtss_phy_conf_set port=%d \n\r", port_no );
    }
    rc = vtss_phy_conf_set(api_ctlr->vsc_instance, port_no,
                        phy_oper_conf);

    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "VTS PHY CONFIG failed, rc=%d", rc);
       return (DEV_STATUS_INVALID_ARG);
    }

    if ((port_no & 1) == 0 && 
        (api_ctlr->phy_reset_config->media_if == VTSS_PHY_MEDIA_IF_FI_1000BX)) {
//         printf("Set the SigDet pin high 19E1.0  \n\r");
        /*
        *  Set the SigDet pin high 19E1.0 for SFP
        *  Enable Fast Link Failure by setting 19E1.4 
        */
        api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no,
                                            VSC85XX_PHY_EXT_REG_PAGE,
                                            VSC85XX_PHY_EXT_REG_PAGE_1);
        api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no, 0x13, 0x11/*0x1*/);
        api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no,
                                            VSC85XX_PHY_EXT_REG_PAGE,
                                            VSC85XX_PHY_EXT_REG_PAGE_0);
    }

   /*  
    * set  23G   0x8130   Enable RecClk0 125MHz & set it to PHY0 
    * set  24G   0x9130   Enable RecClk1 125MHz & set it to PHY1
    */
    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no,
                                          VSC85XX_PHY_EXT_REG_PAGE,
                                          VSC85XX_PHY_EXT_REG_PAGE_16);
    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no,
                                          VSC85XX_PHY_RCVD_CLK0_CTRL,
                                          0x8130/*0x8123*/);
    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no,
                                          VSC85XX_PHY_RCVD_CLK1_CTRL,
                                          0x9130/*0*/);
    /* Set COMA_MODE pin as input */
    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no,
                                          VSC85XX_PHY_GPIO_CTRL2,
                                          0x2600);
    api_ctlr->vsc_init_config.miim_write( api_ctlr->vsc_instance, port_no, 
                                          VSC85XX_PHY_EXT_REG_PAGE,
                                          VSC85XX_PHY_EXT_REG_PAGE_0);

    if (ts_init_reqd) {
        /*
         * Initialize Timestamping
         */
        if (( port_no % 4 ) == 0 )
            printf("Initializing Timestamp Engine\n\r");

        rc = vsc_1gphy_ts_ptp_init(dev);
        if (rc != DEV_STATUS_SUCCESS) {
            cterr('f', 0, "TS-Eng: Init failed for port %d, rc=%d", port_no, rc);
            /*
             * Always return success since this will fail on
             * older PHY
             */
//           return (DEV_STATUS_SUCCESS);
        }

        if (!(vsc_1gphy_ts_upper_shared_port(port_no))) {
            printf("vsc_1gphy_ts_ptp_engine_alloc port=%d \n\r", port_no );
            rc = vsc_1gphy_ts_ptp_engine_alloc(dev,
                                               in_eng_id,
                                               eg_eng_id);
            if (rc != DEV_STATUS_SUCCESS) {
                cterr('f', 0, "PTP engine alloc failed for port %d IGNORING, "
                           "rc=%d", port_no, rc);
                /*
                 * Always return success since this will fail on
                 * older PHY
                 */
//              return (DEV_STATUS_SUCCESS);
            }
        }

        rc = vsc_1gphy_ts_ptp_engine_conf(dev,
                                           in_eng_id,
                                           eg_eng_id);
        if (rc != DEV_STATUS_SUCCESS) {
            cterr('f', 0, "PTP flow/action conf failed for port %d, rc=%d",
                       port_no, rc);
            /*
             * Always return success since this will fail on
             * older PHY
             */
//          return (DEV_STATUS_SUCCESS);
        }

        phy_ts_init_sig_conf = init_1g_default_sig_mask_conf();

        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("vtss_phy_ts_fifo_sig_set port=%d \n\r", port_no );
        }
        rc = vtss_phy_ts_fifo_sig_set(api_ctlr->vsc_instance, port_no,
                                      phy_ts_init_sig_conf);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "VTS PHY TS SIG INIT failed for port %d, rc=%d",
                       port_no, rc);
            /*
             * Always return success since this will fail on
             * older PHY
             */
//           return (DEV_STATUS_SUCCESS);
        }
    }

    return (DEV_STATUS_SUCCESS);
}

/*
 * vscphy_dev_init : Initializing Phy related to port belonging to 
 *           this dev_object_t .
 *  In application code init should be called per port.
 *  port_num used here is same as passed during dev_xxx_create()
 */

static dev_status
vscphy_dev_init (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;

    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc) {
       cterr('f', 0, "Invalid Arguments %s", __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    /* Check for VSC instance before calling Vitesse API */
    if (!(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "%s : VSC instance is not initialized"
                    " call dev_create first" , __FUNCTION__);
       return (DEV_STATUS_INVALID_ARG);
    }
    api_ctlr = dev_vsc->vsc_api_control;
    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
       return (vsc_1gphy_dev_init(dev));
    }
    
    /* Should never come here */
    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_enable_op (dev_object_t *dev)
{
    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_disable_op (dev_object_t *dev)
{
    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_enable_intr (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;

//     u16 data;
    vtss_phy_event_t ev_mask=0;
    vtss_phy_event_t ev_status=0;
/*    vtss_phy_10g_event_t ev_10g_mask=0;
    vtss_phy_10g_event_t ev_10g_status=0;*/
    dev_vsc = (dev_vsc8x_object_t *)dev;

    if (!dev_vsc || !(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "VSC API: ENABLE/DISABLE link"
                 "phy object is not ready yet for port=%d",dev_vsc->port);
       return (FALSE);
    }
    api_ctlr = dev_vsc->vsc_api_control;

    if(api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        if (!(api_ctlr->phy_config_set)) {
            cterr('f', 0, "VSC API: ENABLE/DISABLE link"
             "1G phy init configs are not set yet for port = %d",dev_vsc->port);
            return (FALSE);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("vtss_phy_event_enable_set port=%d \n\r", dev_vsc->port );
        }
        vtss_phy_event_enable_get(api_ctlr->vsc_instance,dev_vsc->port, &ev_status);
        printf(" VTSS Interrupt event status %x ", (u16)ev_status);
        ev_mask |= VTSS_PHY_LINK_FFAIL_EV | 
                    VTSS_PHY_LINK_LOS_EV   |
                    VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV |
                    VTSS_PHY_LINK_FDX_STATE_CHANGE_EV   |
                    VTSS_PHY_LINK_AUTO_NEG_ERROR_EV     |
                    VTSS_PHY_LINK_AUTO_NEG_COMPLETE_EV  |
                    VTSS_PHY_LINK_TX_FIFO_OVERFLOW_INT_EV  |
                    VTSS_PHY_LINK_RX_FIFO_OVERFLOW_INT_EV  ;
        vtss_phy_event_enable_set(api_ctlr->vsc_instance,dev_vsc->port,ev_mask,TRUE);
        printf(" VTSS Interrupt MASK set to   %x ", (u16)ev_mask); 
    }
    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_disable_intr (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;
//    vtss_rc rc;
    vtss_phy_event_t ev_mask=0;
    vtss_phy_event_t ev_status=0;
/*    vtss_phy_10g_event_t ev_10g_mask=0;
    vtss_phy_10g_event_t ev_10g_status=0;*/
    dev_vsc = (dev_vsc8x_object_t *)dev;

    if (!dev_vsc || !(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "VSC API: "
                 "phy object is not ready yet for port=%d",dev_vsc->port);
       return (FALSE);
    }
    api_ctlr = dev_vsc->vsc_api_control;
    
    if(api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        if (!(api_ctlr->phy_config_set)) {
            cterr('f', 0, "VSC API: "
             "1G phy init configs are not set yet for port = %d",dev_vsc->port);
            return (FALSE);
        }
 
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("vtss_phy_event_enable_get port=%d \n\r", dev_vsc->port );
        }
        vtss_phy_event_enable_get(api_ctlr->vsc_instance,dev_vsc->port, &ev_status);
        printf(" VTSS Interrupt event status %x ", (u16)ev_status); 
        ev_mask |= VTSS_PHY_LINK_FFAIL_EV | 
                   VTSS_PHY_LINK_LOS_EV   |
                   VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV |
                   VTSS_PHY_LINK_FDX_STATE_CHANGE_EV   |
                   VTSS_PHY_LINK_AUTO_NEG_ERROR_EV     |
                   VTSS_PHY_LINK_AUTO_NEG_COMPLETE_EV  |
                   VTSS_PHY_LINK_TX_FIFO_OVERFLOW_INT_EV  |
                   VTSS_PHY_LINK_RX_FIFO_OVERFLOW_INT_EV  ;
        /* Passing FALSE to vtss_phy_event_enable will disable the interrupts 
        * on the MDINT pin 
        */
 
        printf("vtss_phy_event_enable_set port=%d \n\r", dev_vsc->port );
        vtss_phy_event_enable_set(api_ctlr->vsc_instance,
                              dev_vsc->port,ev_mask,FALSE);
        printf(" VTSS Interrupt MASK set to   %x ", (u16)ev_mask); 

    }/* else if(api_ctlr->platform_target == VTSS_TARGET_10G_PHY ) {
        printf("vtss_phy_10g_event_poll port=%d \n\r", dev_vsc->port );
        vtss_phy_10g_event_poll(api_ctlr->vsc_instance,dev_vsc->port, &ev_10g_status);
        printf(" VTSS Interrupt 10G event status %x ", ev_10g_status);
        ev_10g_mask |= VTSS_PHY_10G_LOPC_EV |
        VTSS_PHY_10G_RX_LOL_EV |
        VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV;

        vtss_phy_10g_event_enable_set(api_ctlr->vsc_instance,dev_vsc->port,
                                   ev_10g_mask,FALSE); 
        printf(" VTSS Interrupt MASK set to   %x ", (u16)ev_10g_mask); 
    }*/
    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_isr (dev_object_t *dev)
{
    dev_vsc8x_object_t *dev_vsc;
    vsc_api_controller_t *api_ctlr;
//    u16 data;
//    BOOL rc;
    vtss_phy_event_t ev_status=0;
    vtss_phy_event_t * const ev_ptr=&ev_status;
    dev_vsc = (dev_vsc8x_object_t *)dev;
    if (!dev_vsc || !(dev_vsc->vsc_api_control)) {
       cterr('f', 0, "VSC API: phy object is not ready yet");
       return (FALSE);
    }
    api_ctlr = dev_vsc->vsc_api_control;
    if(api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        if (!(api_ctlr->phy_config_set)) {
            cterr('f', 0, "VSC API: ENABLE/DISABLE link"
             "1G phy init configs are not set yet");
             return (FALSE);
        }
        *ev_ptr = 0;

        printf("vtss_phy_event_poll port=%d \n\r", dev_vsc->port );
        vtss_phy_event_poll(api_ctlr->vsc_instance, dev_vsc->port,
                            ev_ptr);
        printf(" Interrupts  set are %x \n", ev_status);
        if(ev_status & VTSS_PHY_LINK_FFAIL_EV)  {
            cterr('f', 0, " VTSS_PHY_LINK_FFAIL_EV \n");
            dev_vsc->callout->phy_fast_link_fail_notify(dev_vsc, dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_LINK_LOS_EV)    { 
            cterr('f', 0, "VTSS_PHY_LINK_LOS_EV");
            dev_vsc->callout->phy_link_notify(dev_vsc, dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV )    {
            cterr('f', 0, "VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV");
            dev_vsc->callout->phy_speed_state_change_notify(dev_vsc,
                                                            dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_LINK_FDX_STATE_CHANGE_EV )  { 
            cterr('f', 0, "VTSS_PHY_LINK_FDX_STATE_CHANGE_EV");
            dev_vsc->callout->
                phy_full_duplex_state_change_notify(dev_vsc,dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_LINK_AUTO_NEG_ERROR_EV )    {
            cterr('f', 0, "VTSS_PHY_LINK_AUTO_NEG_ERROR_EV");
            dev_vsc->callout->phy_auto_neg_err_notify(dev_vsc,dev_vsc->port);
     
        }
        if(ev_status & VTSS_PHY_LINK_AUTO_NEG_COMPLETE_EV )     {
            cterr('f', 0, " VTSS_PHY_LINK_AUTO_NEG_COMPLETE_EV");
            dev_vsc->callout->phy_auto_neg_comp_notify(dev_vsc,
                                                       dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_LINK_TX_FIFO_OVERFLOW_INT_EV )  {
            cterr('f', 0, "VTSS_PHY_LINK_TX_FIFO_OVERFLOW_INT_EV");
            dev_vsc->callout->phy_tx_fifo_overflw_notify(dev_vsc,
                                                         dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_LINK_RX_FIFO_OVERFLOW_INT_EV )  {
            cterr('f', 0, "VTSS_PHY_LINK_RX_FIFO_OVERFLOW_INT_EV");
            dev_vsc->callout->phy_rx_fifo_overflw_notify(dev_vsc,
                                                         dev_vsc->port);
        }
    }/* else if(api_ctlr->platform_target == VTSS_TARGET_10G_PHY) {

        printf("vtss_phy_10g_event_poll port=%d \n\r", dev_vsc->port );
        vtss_phy_10g_event_poll(api_ctlr->vsc_instance,dev_vsc->port,
                                (vtss_phy_10g_event_t *)&ev_status);
        printf(" VTSS Interrupt event status %x for porti %x ", 
                ev_status, dev_vsc->port);

        if(ev_status & VTSS_PHY_10G_LOPC_EV )  {
            cterr('f', 0, "VTSS_PHY_10G_LOPC_EV");
            dev_vsc->callout->phy_10g_lopc_status_notify(dev_vsc,
                                                          dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_10G_RX_LOL_EV ) {
            cterr('f', 0, "VTSS_PHY_10G_RX_LOL_EV");
            dev_vsc->callout->phy_10g_rxlol_status_notify(dev_vsc,
                                                           dev_vsc->port);
        }
        if(ev_status & VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV ) {
            cterr('f', 0, "VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV");
            dev_vsc->callout->phy_10g_pcs_rcv_fault_notify(dev_vsc,
                                                            dev_vsc->port);
        }
    }*/
    return (DEV_STATUS_SUCCESS);
}

static dev_status 
vscphy_dev_show (dev_object_t *dev, print_fn_t print_fn, u32 cmd) 
{
    return (DEV_STATUS_SUCCESS);
}

static void 
vscphy_dev_destroy (dev_object_t **dev)
{
    if (dev != NULL) {
//         dev_vsc8x_object_t *dev_obj = (dev_vsc8x_object_t*)*dev;
//        dev_base_deinit(&dev_obj->base);
        free(*dev);
        *dev = NULL;
    }
}
vtss_rc
phy_1g_fpga_read (const vtss_inst_t *inst, const vtss_port_no_t port_num, const unsigned char reg,
          unsigned short * val)
{
    int status = 0;
    uint16 phy_reg = (uint16)reg;
    uint bus_no = SMI_BUS_0;
    uint phy_adr = port_num;
    int mii_value;

    status = wallander_phy_reg_rd(bus_no, phy_adr, phy_reg, &mii_value);
    if (status) {
        cterr('f', 0, "\n failed to read data from reg %x for port %d", reg,
                   port_num);
        return (VTSS_RC_ERROR);
    }
    *val = (unsigned short)mii_value;

    if ( vsc_phy_access_print & PHY_ACCESS_PRINT_1G) {
        printf(" 1G#%d [0x%04x] -> 0x%08x \n", port_num, reg, *val );
    }

    return (VTSS_RC_OK);
}

vtss_rc
phy_1g_fpga_write (const vtss_inst_t *inst, const vtss_port_no_t port_num, const uint8 reg,
                   const uint16 val)
{
    int status = 0;
    uint16 phy_reg = (uint16)reg;
    uint bus_no = SMI_BUS_0;
    uint phy_adr = port_num;

    status = wallander_phy_reg_wr(bus_no, phy_adr, phy_reg, val);
    if (status) {
        cterr('f', 0, "\n failed to write data to reg %x for port %d", reg,
                   port_num);
        return (VTSS_RC_ERROR);
    }

    if ( vsc_phy_access_print & PHY_ACCESS_PRINT_1G ) {
        printf(" 1G#%d [0x%04x] <- 0x%08x \n", port_num, reg, val );
    }

    return (VTSS_RC_OK);
}

int vsc_1588_reg_access( u32 port, u32 action, u32 block, u32 addr, u32 *value ) 
{
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    if ( action == 0 ) {
        if (vtss_phy_1588_csr_reg_read( api_gptr->vsc_instance, port, block, addr, value)) {
            cterr('f', 0, " Failed to read 1588 CSR register of port %d ", port);
            return( FAILED );
        }
    }
    else if ( action == 1 ) {
        if (vtss_phy_1588_csr_reg_write( api_gptr->vsc_instance, port, block, addr, value)) {
            cterr('f', 0, " Failed to write 1588 CSR register of port %d\n", port);
            return( FAILED );
        }
    }
    else {
        cterr('f', 0, "Code bug, fix me!" );
        return( FAILED );
    }

    return PASSED;
}

int vsc8574_1588_reg_dump_core( u32 port, uint32 *reg_addr, uint32 block, char *msg1 ) 
{

    int rv = PASSED;
    uint32 data;
    uint32 i, k, inst, addr;
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    printf("  addr     %s block=%d \n\r", msg1, block );

    i = 0;
    while ( 1 ) {
        inst = reg_addr[i] >> 28;

        if ( inst == 0xF )
            return PASSED;

        for ( k=0; k<=inst; k++ ) {
            addr  = reg_addr[i] & 0xFFFF;
            addr += ( k * 0x10 );

            if ( vtss_phy_1588_csr_reg_read( api_gptr->vsc_instance, port, block, addr, &data ))
            return( FAILED );

            printf(" [0x%04X]  %08X \n\r",  addr, data );
        }
        i++;
    }

    return rv;
}


int vsc8574_1588_reg_dump_proc( u32 port ) {

    int rv = PASSED;
    uint32 reg_addr[] = { 0x00,0x01,0x02,
              0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x1A,0x1B,
              0x20,0x21,0x22,0x26,0x2C,0x2D,0x2E,0x2F,
              0x35,0x36,0x37,0x38,0x39,0x3A,
              0x44,0x45,0x46,0x47,0x4D,0x4E,0x4F,
              0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
              0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
              0xF0000000 };

    rv |= vsc8574_1588_reg_dump_core( port, &reg_addr[0], 6, "PROC_0");
    rv |= vsc8574_1588_reg_dump_core( port, &reg_addr[0], 7, "PROC_1");

    return rv;
}


int vsc8574_1588_reg_dump_ingress0( u32 port ) {

    int rv = PASSED;
    uint32 reg_addr[] = { 0x00,0x01,0x02,0x3,

              0x80000010,
              0x80000011,
              0x80000012,
              0x80000013,
              0x80000014,
              0x80000015,
              0x80000016,

              0x90,0x91,0x92,

              0x800000A0,
              0x800000A1,
              0x800000A2,
              0x800000A3,
              0x800000A4,
              0x800000A5,
              0x800000A6,

              0x120,

              0x80000130,
              0x80000132,
              0x80000133,
              0x80000134,
              0x80000135,
              0x80000136,
              0x80000137,
              0x80000138,
              0x80000139,

              0x1B0,0x1B1,0x1B2,0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1B8,

              0x800001C0,
              0x800001C1,
              0x800001C2,
              0x800001C3,
              0x800001C4,
              0x800001C5,
              0x800001C6,
              0x800001C7,
              0x800001C8,

              0x240,0x241,0x242,0x243,0x244,0x245,0x246,0x247,0x248,

              0x80000250,
              0x80000251,
              0x80000252,
              0x80000253,
              0x80000254,
              0x80000255,
              0x80000256,
              0x80000257,
              0x80000258,

              0x600002D0,
              0x600002D1,
              0x600002D2,
              0x600002D3,
              0x600002D4,
              0x600002D5,
              0x600002D6,
              0x600002D7,
              0x600002D8,

              0x330,
              0xF0000000 };

    rv |= vsc8574_1588_reg_dump_core( port, &reg_addr[0], 0, "INGRESS_0" );
    rv |= vsc8574_1588_reg_dump_core( port, &reg_addr[0], 2, "INGRESS_1" );

    return rv;
}



int vsc8574_1588_reg_dump_ingress2( u32 port ) {

    uint32 reg_addr[] = { 0x00,0x01,0x02,0x03,
              0x10,0x11,0x12,0x13,

              0x80000020,
              0x80000021,
              0x80000022,
              0x80000023,
              0x80000024,
              0x80000025,
              0x80000026,
              
              0xA0,0xA1,0xA2,

              0x800000C0,
              0x800000C1,
              0x800000C2,
              0x800000C3,
              0x800000C4,
              0x800000C5,
              0x800000C6,

              0x140,

              0x80000160,
              0x80000161,
              0x80000162,
              0x80000163,
              0x80000164,
              0x80000165,
              0x80000166,
              0x80000167,
              0x80000168,

              0x6000016E,
              0x600001E1,
              0x600001E2,
              0x600001E3,
              0x600001E4,
              0x600001E5,
              0x600001E6,
              0x600001E7,
              0x600001E8,

              0x330,
              0xF0000000  };

    return vsc8574_1588_reg_dump_core( port, &reg_addr[0], 4, "INGRESS_2" );
}



int vsc8574_1588_reg_dump_egress0( u32 port ) {

    int rv = PASSED;
    uint32 reg_addr[] = { 0x00,0x01,0x02,0x3,

              0x80000010,
              0x80000011,
              0x80000012,
              0x80000013,
              0x80000014,
              0x80000015,
              0x80000016,

              0x90,0x91,0x92,

              0x800000A0,
              0x800000A1,
              0x800000A2,
              0x800000A3,
              0x800000A4,
              0x800000A5,
              0x800000A6,

              0x120,

              0x80000130,
              0x80000132,
              0x80000133,
              0x80000134,
              0x80000135,
              0x80000136,
              0x80000137,
              0x80000138,
              0x80000139,

              0x1B0,0x1B1,0x1B2,0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,

              0x800001C0,
              0x800001C1,
              0x800001C2,
              0x800001C3,
              0x800001C4,
              0x800001C5,
              0x800001C6,
              0x800001C7,
              0x800001C8,

              0x240,0x241,0x242,0x243,0x244,0x245,0x246,0x247,0x248,0x249,

              0x80000250,
              0x80000251,
              0x80000252,
              0x80000253,
              0x80000254,
              0x80000255,
              0x80000256,
              0x80000257,
              0x80000258,

              0x600002D0,
              0x600002D1,
              0x600002D2,
              0x600002D3,
              0x600002D4,
              0x600002D5,
              0x600002D6,
              0x600002D7,
              0x600002D8,

              0x330,0x331,0x332,0x333,0x334,0x335,
              0xF0000000};

    rv |= vsc8574_1588_reg_dump_core( port, &reg_addr[0], 1, "EGRESS_0" );
    rv |= vsc8574_1588_reg_dump_core( port, &reg_addr[0], 3, "EGRESS_1" );

    return rv;
}


int vsc8574_1588_reg_dump_egress2( u32 port ) {

    uint32 reg_addr[] = { 0x00,0x01,0x02,0x03,
              0x10,0x11,0x12,0x13,

              0x80000020,
              0x80000021,
              0x80000022,
              0x80000023,
              0x80000024,
              0x80000025,
              0x80000026,

              0xA0,0xA1,0xA2,

              0x800000C0,
              0x800000C1,
              0x800000C2,
              0x800000C3,
              0x800000C4,
              0x800000C5,
              0x800000C6,

              0x140,

              0x80000160,
              0x80000161,
              0x80000162,
              0x80000163,
              0x80000164,
              0x80000165,
              0x80000166,
              0x80000167,
              0x80000168,

              0x600001E0,
              0x600001E1,
              0x600001E2,
              0x600001E3,
              0x600001E4,
              0x600001E5,
              0x600001E6,
              0x600001E7,
              0x600001E8,

              0xF0000000  };

    return vsc8574_1588_reg_dump_core( port, &reg_addr[0], 5, "EGRESS_2" );
}


int vsc_1588_reg_dump( u32 port ) {

    int rv = PASSED;

    rv |= vsc8574_1588_reg_dump_proc( port );
    rv |= vsc8574_1588_reg_dump_ingress0( port );
    rv |= vsc8574_1588_reg_dump_ingress2( port );
    rv |= vsc8574_1588_reg_dump_egress0( port );
    rv |= vsc8574_1588_reg_dump_egress2( port );

    return rv;
}


int vsc_phy_ts_read( uint32 port, uint16 *sec_hi16, uint32 *sec_lo, uint32 *nsecs ) 
{
    int rc;
    vtss_phy_timestamp_t tod;
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ptptime_get port=%d \n\r", port );
    }
    rc = vtss_phy_ts_ptptime_get( api_gptr->vsc_instance, port, &tod );
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "vtss_phy_ts_ptptime_get failed with rc = %d", rc);
        return( FAILED );
    }

    *sec_hi16 = tod.seconds.high;
    *sec_lo   = tod.seconds.low;
    *nsecs    = tod.nanoseconds;
    printf("Read ToD from PHY sec_hi %04x, sec_lo %08x, nsec %08x,",
         *sec_hi16, *sec_lo, *nsecs);

    return PASSED;
}


int vsc_phy_ts_load( uint32 port, uint16 sec_hi, uint32 sec_lo, uint32 nsecs ) 
{
    int rc;
    vtss_phy_timestamp_t tod;
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    tod.seconds.high = sec_hi;
    tod.seconds.low  = sec_lo;
    tod.nanoseconds  = nsecs;

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ptptime_set port=%d \n\r", port );
    }
    rc = vtss_phy_ts_ptptime_set( api_gptr->vsc_instance, port, &tod);
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "vtss_phy_ts_ptptime_set failed with rc = %d", rc);
        return( FAILED );
    }

    return PASSED;
}

int vsc_phy_ts_arm( uint32 port ) 
{
    int rc;
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ptptime_arm port=%d \n\r", port );
    }
    rc = vtss_phy_ts_ptptime_arm( api_gptr->vsc_instance, port );
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "vtss_phy_ts_ptptime_arm failed with rc = %d", rc);
        return( FAILED );
    }

    return PASSED;
}

int vsc_phy_ts_set_done( uint32 port ) 
{
    int rc;
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_ptptime_set_done port=%d \n\r", port );
    }
    rc = vtss_phy_ts_ptptime_set_done( api_gptr->vsc_instance, port );
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "vtss_phy_ts_ptptime_set_done failed with rc = %d", rc);
        return( FAILED );
    }

    return PASSED;
}

int vsc_1588_tod_access (int port, int mode, u16 tod_hi, u32 tod_lo, u32 tod_ns)
{
    u16 sec_hi;
    u32 sec_lo, sec_ns;

    sec_hi = tod_hi;
    sec_lo = tod_lo;
    sec_ns = tod_ns;

    if ( mode == 0 ) {
        if (vsc_phy_ts_read(port, &sec_hi, &sec_lo, &sec_ns)) {
            cterr('f', 0, "#### TOD Read failed for port %d", port);
            return( FAILED );
        }
        printf("TOD in port %d sec_hi=0x%04x sec_lo=0x%08x nsecs=0x%08x \n",
            port, sec_hi, sec_lo, sec_ns);
    } else if ( mode == 1 ) {
        if (vsc_phy_ts_load(port, sec_hi, sec_lo, sec_ns)) {
            cterr('f', 0, "#### TOD Load failed for port %d", port);
            return( FAILED );
        }
    } else if ( mode == 2 ) {
        if (vsc_phy_ts_arm(port)) {
            cterr('f', 0, "#### TOD Arm failed for port %d", port);
            return( FAILED );
        }
    } else if ( mode == 3 ) {
        if (vsc_phy_ts_set_done(port)) {
            cterr('f', 0, "#### TOD Set done failed for port %d", port);
            return( FAILED );
        }
    } else {
        cterr('f', 0, "Code bug, fix me!" );
        return( FAILED );
    }

    return PASSED;
}


int vsc_phy_ts_stats_get ( int port, vtss_phy_ts_stats_t *ts_stats )
{
    int rc = PASSED;
    vtss_target_type_t type;

    if ( vsc_phy_ready( port, &type )) {
        return( FAILED );
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("vtss_phy_ts_stats_get port=%d \n\r", port );
    }
    rc = vtss_phy_ts_stats_get( api_gptr->vsc_instance, port, ts_stats);
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "VTS PHY TS stats get failed, rc=%d", rc);
        return( FAILED );
    }

    return rc;
}

int vsc_phy_1588_stats(int port)
{
    vtss_phy_ts_stats_t ts_buff[4];
    uint32 ts_start;

    ts_start = 0x5B;

    memset( &ts_buff[0], 0, sizeof(ts_buff));

    assert(port >= 0 && port < 4);

    if ( vsc_phy_ts_stats_get( port, &ts_buff[port] )) {
        cterr('f', 0, "VSC: Failed to get TimeStamp Stats, port = %d", port);
        return (FAILED);
    }

    printf(" %08X  %08X  %08X  %08X  %08X  %08X  %08X  %08X - port %d\n\r",
        ts_buff[port].ingr_frm_mod_cnt,
        ts_buff[port].egr_frm_mod_cnt,
        ts_buff[port].ts_fifo_tx_cnt,
        ts_buff[port].ts_fifo_drop_cnt,
        ts_buff[port].ingr_pream_shrink_err,
        ts_buff[port].egr_pream_shrink_err,
        ts_buff[port].ingr_fcs_err,
        ts_buff[port].egr_fcs_err, port );

    return (PASSED);
}

int vsc_load_timestamp( void ) 
{
    int rv = PASSED;
    unsigned int j;
    uint16 sec_hi, data_hi;
    uint32 sec_lo, nsecs, data_lo;

    printf("Generating timestamp... \n");

    int max_phy_ports = get_num_ports();

//     cp_cpld_clr_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );

    for (j = 0; j < max_phy_ports; j++) {
        data_hi = 0x1110 + j;
        data_lo = 0x02222222 | ( j << 28 );

        sec_hi = data_hi;
        sec_lo = data_lo;
        nsecs  = 0;

        if (vsc_phy_ts_load( j, sec_hi, sec_lo, nsecs)) {
            return (FAILED);
        }
    }

//     cp_cpld_set_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );
//     cp_cpld_clr_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );
//     delay_ms( 1500 ); 
    msleep(1500);

    for (j = 0; j < max_phy_ports; j++) {
        if (vsc_phy_ts_set_done( j )) {
            return (FAILED);
        }
    }

//     cp_cpld_set_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );
//     cp_cpld_clr_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );
//     delay_ms( 1500 ); 
    msleep(1500);

    for (j = 0; j < max_phy_ports; j++) {
        if (vsc_phy_ts_arm( j )) {
            return (FAILED);
        }
    }

//     cp_cpld_set_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );
//     cp_cpld_clr_bit( RP1RUVE_FPGA_REG_PHY_LOAD_SAVE, RP1RUVE_FPGA_BIT_PHY_LOAD_SAVE_STROBE );
//     delay_ms( 1500 ); 
    msleep(1500);

    for (j = 0; j < max_phy_ports; j++) {
        data_hi = 0x1110 + j;
        data_lo = 0x02222222 | ( j << 28 );

        if (vsc_phy_ts_read( j, &sec_hi, &sec_lo, &nsecs)) {
            return (FAILED);
        }

        if ( sec_hi != data_hi ) {
            printf("TOD read=0x%04X/0x%08X/0x%08X  write=0x%04X/0x%08X/0x00000000  port=%d ", 
                sec_hi, sec_lo, nsecs, data_hi, data_lo, j );
            rv = FAILED;
        } else {
            printf("TOD read=0x%04X/0x%08X/0x%08X  write=0x%04X/0x%08X/0x00000000  port=%d \n\r", 
                sec_hi, sec_lo, nsecs, data_hi, data_lo, j );
        }
    }

    return rv;
}

/******************************************************************************
 * NAME         : dev_vsc8x_phy_create_active
 *
 * DESCRIPTION  : This function facilitates function vectors on active for
 *                PHY device.
 *
 * INPUT(S)     : dev
 *                  Pointer to dev object for this PHY
 *
 * OUTPUTS(S)   : void
 *
 * NOTES        : None
 *****************************************************************************/

void
dev_vsc8x_phy_create_active (dev_object_t *dev)
{
    dev->dev_object_fvt->dev_attach       = vscphy_dev_attach;
    dev->dev_object_fvt->dev_oper_enable  = vscphy_dev_enable_op;
    dev->dev_object_fvt->dev_oper_disable = vscphy_dev_disable_op;
    dev->dev_object_fvt->dev_intr_enable  = vscphy_dev_enable_intr;
    dev->dev_object_fvt->dev_intr_disable = vscphy_dev_disable_intr;
    dev->dev_object_fvt->dev_isr          = vscphy_dev_isr;
    dev->dev_object_fvt->dev_init         = vscphy_dev_init;
}

/*
 * dev_vsc8x_phy_create()
 *      Create the PHY dev object. 
 * Returns :
 *      pointer to the dev object.
 */
dev_object_t *
dev_vsc8x_phy_create (dev_vts_gphy_create_info_t *params)
{
    dev_vsc8x_object_t *dev_vsc;
    dev_object_t *dev;
    vtss_target_type_t target_type;
    static vsc_api_controller_t *api_ctlr=NULL;
    vtss_rc rc;
    int chip_no;

    if (dev_vsc_addrs[params->port]) {
        /* malloc is previously done, so use stored malloc'ed addr */
        dev_vsc = dev_vsc_addrs[params->port];
    } else {
        /* first time here, so malloc now */
        dev_vsc = (dev_vsc8x_object_t *)malloc(sizeof(dev_vsc8x_object_t));
        if (!dev_vsc) {
            cterr('f', 0, "\n %s malloc FAILED", __FUNCTION__);
            return (NULL);
        }
        dev_vsc_addrs[params->port] = dev_vsc;
    }

    memset(dev_vsc, 0, sizeof(dev_vsc8x_object_t));

    /* Perform base dev_object Init */
    dev = (dev_object_t *)&dev_vsc->base;
    dev->dev_object_fvt = &vts_obj_fvt;
//    dev_base_init(dev);

    /*Get phy type from platfrom params */
    target_type = params->phy_target; /* VTSS_TARGET_CU_PHY - 1G 
                                         VTSS_TARGET_10G_PHY - 10G */

    if (api_ctlr_addrs[params->port]) {
        /* malloc is previously done, so use stored malloc'ed addr */
        api_ctlr = api_ctlr_addrs[params->port];
    } else {
        api_ctlr = (vsc_api_controller_t *)malloc(sizeof(vsc_api_controller_t));
        if (!api_ctlr) {
            cterr('f', 0, "\n %s malloc for api_ctlr FAILED", __FUNCTION__);
            return (NULL);
        }
        api_ctlr_addrs[params->port] = api_ctlr;
    }

    memset(api_ctlr, 0, sizeof(vsc_api_controller_t));
    api_ctlr->platform_target = target_type;
    /*
     * Phy init params can also be passed as per platform 
     * Requirement .
     */
    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        if (params->port_1g_phy_reset_init_conf) {
            api_ctlr->phy_reset_config = params->port_1g_phy_reset_init_conf;
        } else {
            api_ctlr->phy_reset_config = &vsc_phy_init_default_params;
        }

        if (params->port_1g_phy_clock_conf) {
            api_ctlr->phy_clock_conf = params->port_1g_phy_clock_conf;
        } else {
            api_ctlr->phy_clock_conf = &vts_phy_1g_clock_default_conf;
        }
        if (params->port_1g_phy_recov_clk_conf) {
            api_ctlr->phy_recov_clk = params->port_1g_phy_recov_clk_conf;
        } else {
            api_ctlr->phy_recov_clk = &vts_phy_1g_recov_clk_default_conf;
        }
        /*
         * Below default parameters will be changed by callin vector
         * to change speed/auto etc using api set_one_feature
         */
        if (params->port & 1) {
//             printf("port%d: &vsc_1g_serdes_phy_conf\n", params->port);
            api_ctlr->phy_config_set =  &vsc_1g_serdes_phy_conf/*vsc_1g_cu_phy_conf*/;
        } else {
            if ((api_ctlr->phy_reset_config->media_if == 
                                                VTSS_PHY_MEDIA_IF_CU) ||
                (api_ctlr->phy_reset_config->media_if == 
                                                VTSS_PHY_MEDIA_IF_AMS_CU_1000BX) ) {
//                 printf("port%d: &vsc_1g_cu_phy_conf\n", params->port);
                api_ctlr->phy_config_set =  /*init_cu_default_phy_param()*/&vsc_1g_cu_phy_conf;
            } else if ((api_ctlr->phy_reset_config->media_if ==
                                                VTSS_PHY_MEDIA_IF_FI_1000BX) ||
                (api_ctlr->phy_reset_config->media_if == 
                                                VTSS_PHY_MEDIA_IF_AMS_FI_1000BX) ) {
//                 printf("port%d: &vsc_1g_sfp_phy_conf\n", params->port);
                api_ctlr->phy_config_set = &vsc_1g_sfp_phy_conf;
            }
        }
    }

    /* 
     * Before attaching dev_object vectors create phy instances 
     * We need only one VSC instance per hardware 
     * Assuming one process iomd/iosd control one miii bus 
     * we create one single instance per process
     */
    if (params->vsc_inst_created) {
//         printf("INSTANCE ALREADY CREATED.\n");
        goto VSC_INSTANCE_CREATED;
    }
    chip_no = 0;

    if ( vtss_inst_addr[chip_no] ) {
        api_ctlr->vsc_instance = vtss_inst_addr[chip_no];

        rc = vtss_inst_destroy( api_ctlr->vsc_instance );
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "%s: vtss_inst_destroy fails with %d \n",__FUNCTION__, rc);
        }
    }

    /* Create the instance */
    vtss_inst_get(api_ctlr->platform_target, &api_ctlr->vsc_create_inst);

    rc = vtss_inst_create( &api_ctlr->vsc_create_inst, &api_ctlr->vsc_instance );
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "%s: vtss_inst_create fails with %d \n",
               __FUNCTION__, rc);
        free(api_ctlr);
        free(dev_vsc);
        api_ctlr_addrs[params->port] = NULL;
        dev_vsc_addrs[params->port] = NULL;
        api_ctlr = NULL;
        dev_vsc = NULL;
        return (NULL);
    }

    vtss_inst_addr[chip_no] = api_ctlr->vsc_instance;

    /* Get the initialization configuration */
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("API: vtss_init_conf_get \n\r" );
    }
    rc = vtss_init_conf_get(api_ctlr->vsc_instance, &api_ctlr->vsc_init_config);
    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "%s: vtss_init_conf_get fails with %d \n",
                      __FUNCTION__, rc);
       free(vtss_inst_addr[chip_no]);
       free(api_ctlr);
       free(dev_vsc);
       vtss_inst_addr[chip_no] = NULL;
       api_ctlr_addrs[params->port] = NULL;
       dev_vsc_addrs[params->port] = NULL;
       api_ctlr = NULL;
       dev_vsc = NULL;
       return (NULL);
    }

    if (api_ctlr->platform_target == VTSS_TARGET_CU_PHY) {
        /* Provide Platform's specific PHY read and write APIs to Vittesse */
        api_ctlr->vsc_init_config.miim_read  = (vtss_miim_read_t)phy_1g_fpga_read;
        api_ctlr->vsc_init_config.miim_write = (vtss_miim_write_t)phy_1g_fpga_write;
    }

    /* Set the initialization configuration */
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("API: vtss_init_conf_set \n\r" );
    }
    rc = vtss_init_conf_set(api_ctlr->vsc_instance, &api_ctlr->vsc_init_config);

    if (rc != VTSS_RC_OK) {
       cterr('f', 0, "%s: vtss_init_conf_set fails with %d \n",
                    __FUNCTION__, rc);
       free(vtss_inst_addr[chip_no]);
       free(api_ctlr);
       free(dev_vsc);
       vtss_inst_addr[chip_no] = NULL;
       api_ctlr_addrs[params->port] = NULL;
       dev_vsc_addrs[params->port] = NULL;
       api_ctlr = NULL;
       dev_vsc = NULL;
       return (NULL);
    }
    params->vsc_inst_created = TRUE;

VSC_INSTANCE_CREATED:
    dev_vsc->vsc_api_control = api_ctlr;
    if (params->port % 4) {
        memcpy((void *)(&api_ctlr->vsc_init_config), 
               (void *)(&api_ctlr_addrs[0]->vsc_init_config), 
               sizeof(vtss_init_conf_t));
    }

    dev_vsc8x_phy_create_active(dev);
    dev->dev_object_fvt->dev_detach       = vscphy_dev_detach;

    dev->dev_object_fvt->dev_show         = vscphy_dev_show;
    dev->dev_object_fvt->dev_name         = "VSC 8x TS PHY";
//    dev->dev_fvt_detached.dev_destroy      = vscphy_dev_destroy;

    dev_vsc->callins = &vscphy_do_nothing_fvt;
    dev_vsc->callin_10g = /*&vts_10g_do_nothing_fvt*/NULL;
    dev_vsc->slot = params->slot;
    dev_vsc->port = params->port;

    return (dev);
}


int phy_dev_create (uint port, int mode)
{
    dev_vts_gphy_create_info_t vsc_phy_info;
    dev_object_t *phy_obj;
    int rc = 0;

    memset(&vsc_phy_info, 0, sizeof(dev_vts_gphy_create_info_t));

    vsc_phy_info.slot = 0;
    vsc_phy_info.callout = &vsc_phy_callout;
    vsc_phy_info.port = port;
    vsc_phy_info.vsc_inst_created = FALSE;
    vsc_phy_info.phy_target = VTSS_TARGET_CU_PHY;

    if (port & 1) {
        /* Backplane eth */
        vsc_phy_info.port_1g_phy_reset_init_conf = &vsc_1g_serdes_phy_reset_init_conf/*vsc_1g_cu_phy_reset_init_conf*/;
    } else if (mode == 0) {
        vsc_phy_info.port_1g_phy_reset_init_conf = &vsc_1g_sfp_phy_reset_init_conf;
    } else {
        vsc_phy_info.port_1g_phy_reset_init_conf = &vsc_1g_cu_phy_reset_init_conf;
    }

    /* If port 1, 2, 3, (4 ports per device) not to create inst */
    if (port % 4)
        vsc_phy_info.vsc_inst_created = TRUE;

    /*
     * fill the port number and create dev_obj instance for the port
     * If we get this far, the port number is valid
     */
    phy_dev[port] = dev_vsc8x_phy_create(&vsc_phy_info);
    if (phy_dev[port] == NULL) {
        cterr('f', 0, "\n The VSC phy instance not created for the port %d", port);
        return (VTSS_RC_ERROR);
    }
    phy_obj = phy_dev[port];

    rc = phy_obj->dev_object_fvt->dev_attach(phy_dev[port]);
    if (rc != DEV_STATUS_SUCCESS){
        cterr('f', 0, "An unsuccessful rc %d to attach PHY ", rc);
        return (VTSS_RC_ERROR);
    }

    return(VTSS_RC_OK);
}

int vsc_phy_ready( uint32 port, vtss_target_type_t *type )
{
    if (port >= get_num_ports()) {
        cterr('f', 0, "Port can't be greater than %d for this board, input=%d ", 
              (get_num_ports() - 1), port);
        return ( FAILED );
    }

    dev_gptr = phy_dev[port];
    if ( !dev_gptr ) {
        cterr('f', 0, "dev+gptr == NULL, PHY driver not yet initialized, port = %d", port);
        return ( FAILED );
    }

    phy_gptr = (dev_vsc8x_object_t *)dev_gptr;

    api_gptr = phy_gptr->vsc_api_control;
    if ( ! api_gptr ) {
        cterr('f', 0, "VSC:API control failed, port = %d", port);
        return ( FAILED );
    }

    if (( api_gptr->platform_target == VTSS_TARGET_CU_PHY  ) || 
        ( api_gptr->platform_target == VTSS_TARGET_10G_PHY )) {
        *type = api_gptr->platform_target;
    } else {
        cterr('f', 0, "VSC:API platform_target failed, port = %d", port);
        return ( FAILED );
    }

    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, port,
                                        VSC85XX_PHY_EXT_REG_PAGE,
                                        VSC85XX_PHY_EXT_REG_PAGE_0);
    return (PASSED);
}

int phy_dev_pre_reset( int port ) 
{
    vtss_target_type_t type;
    int rc = VTSS_RC_OK;

    if ( vsc_phy_ready( port, &type )) {
        return ( FAILED );
    }

    if (( port % 4 ) == 0 ){
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("API: vtss_phy_pre_reset port=%d \n\r", port );
        }
        rc = vtss_phy_reset_lcpll( api_gptr->vsc_instance, port );
        if (rc != VTSS_RC_OK) {
            printf("vtss_phy_reset_lcpll failed! port=%d rc=%d\n", port, rc);
            return ( FAILED );
        }

        rc = vtss_phy_pre_reset( api_gptr->vsc_instance, port );
        if (rc != VTSS_RC_OK) {
            printf("vtss_phy_pre_reset failed! port=%d rc=%d\n", port, rc);
            return ( FAILED );
        }

//         api_gptr->ucode_done |= BIT_0;

        /* Sleep for 10msecs for reset to take action */
        usleep(10000);
    }

    return rc;
}


int phy_dev_reset( int port ) {
    vtss_target_type_t type;
    vtss_phy_conf_t *phy_oper_conf;
    int rc = VTSS_RC_OK;

    if ( vsc_phy_ready( port, &type )) {
        return ( FAILED );
    }
    /* Do Phy Reset */
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("API: vtss_phy_reset port=%d \n\r", port );
    }
    rc = vtss_phy_reset(api_gptr->vsc_instance, port, api_gptr->phy_reset_config);
    if (rc != VTSS_RC_OK) {
        cterr('f', 0, "vtss_phy_reset failed! port=%d rc=%d", port, rc);
        return ( FAILED );
    }

/*    if (!(api_gptr->phy_config_set)) {
        cterr('f', 0, "1G phy init configs are not set yet");
        return (DEV_STATUS_INVALID_ARG);
    }
    phy_oper_conf = api_gptr->phy_config_set;

    rc = vtss_phy_conf_set(api_gptr->vsc_instance, port,
                           phy_oper_conf);*/
    return rc;
}

int phy_dev_post_reset( int port ) {

    vtss_target_type_t type;
    int rc = VTSS_RC_OK;

    if ( vsc_phy_ready( port, &type )) {
        return ( FAILED );
    }

    if (( port % 4 ) == 0 ) {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("API: vtss_phy_post_reset port=%d \n\r", port );
        }
        rc = vtss_phy_post_reset( api_gptr->vsc_instance, port );
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "vtss_phy_post_reset failed! port=%d rc=%d", port, rc);
            return ( FAILED );
        }
        rc = vtss_phy_sd6g_ob_lev_wr( api_gptr->vsc_instance, 0, 2);
        if (rc != VTSS_RC_OK) {
            cterr('f', 0, "vtss_phy_post_reset failed! port=%d rc=%d", port, rc);
            return ( FAILED );
        }

    }

    return rc;
}

static int vsc8552_adjust_amplitude ()
{
    vtss_target_type_t type;
    uint16_t reg18g;
    int timeout;

    if ( vsc_phy_ready( 0, &type )) {
        return ( FAILED );
    }

    // Switch to micro/GPIO register-page
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 31, 0x10);
    // Read MCB for 6G macro 0 into PRAM
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0x8013);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }

    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0xd7cc);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }

    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0x8007);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }

    // ob-level ha total of 6 bits, 
    // the lower 3 bits are configured on the following write, bit[11:9]
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0x9806);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }

    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0x8007);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }
    // The upper 3 bits are configured on the following write, bit[6:4]. 
    // In this example, ob_level = 010000
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0x8006);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }

    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 18, 0x9c40);
    timeout = 100;
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
    while ((reg18g & 0x8000) && timeout > 0) {
        api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, 0, 18, &reg18g);
        timeout--; // Make sure that we don't run forever
        usleep(1000);
    }

    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, 0, 31, 0x0);

    return (PASSED);
 }

int phy_dev_init (uint port, BOOL ts_init)
{
    dev_object_t *phy_obj;
    vtss_target_type_t type;
    int rc = VTSS_RC_OK;

    if ( vsc_phy_ready( port, &type )) {
        return ( FAILED );
    }

    // This flag is used to control TS block init in the phy
    ts_init_reqd = ts_init;

    phy_obj = phy_dev[port];

    rc = phy_obj->dev_object_fvt->dev_init(phy_dev[port]);
    if (rc != DEV_STATUS_SUCCESS){
        cterr('f', 0, "An unsuccessful rc %d to init PHY for port %d", rc, port);
        return (VTSS_RC_ERROR);
    }

    rc = phy_obj->dev_object_fvt->dev_oper_enable(phy_dev[port]);
    if (rc != DEV_STATUS_SUCCESS){
        cterr('f', 0, "An unsuccessful rc %d to enable PHY for port %d", rc, port);
        return (VTSS_RC_ERROR);
    }

    return(VTSS_RC_OK);
}

int vscphy_enable_pcs_autoneg(int port)
{
    uint16_t data;
    vtss_target_type_t type;
    int rc = VTSS_RC_OK;

    if ( vsc_phy_ready( port, &type )) {
        return ( FAILED );
    }

    /*
        * Enable pcs autoneg ??
        */
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, port,
                                        VSC85XX_PHY_EXT_REG_PAGE,
                                        VSC85XX_PHY_EXT_REG_PAGE_3);
    api_gptr->vsc_init_config.miim_read(api_gptr->vsc_instance, port,
        VSC85XX_PHY_PCS_CONTROL, &data);
    data |= VSC85XX_PHY_PCS_CONTROL_ANEG_ENABLE;
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, port,
        VSC85XX_PHY_PCS_CONTROL, data);
    api_gptr->vsc_init_config.miim_write(api_gptr->vsc_instance, port,
                                        VSC85XX_PHY_EXT_REG_PAGE,
                                        VSC85XX_PHY_EXT_REG_PAGE_0);

    return (rc);
}


int wallander_init_all_phy_ports (boolean time_stamp_input, boolean forceInit, int mode)
{
    int j;
    int rv = PASSED;
    int max_phy_ports = get_num_ports();

    for (j = 0; j < max_phy_ports; j++) {
        if (!forceInit) {
            if ( phyPortInitialized[j] ) {
                return (VTSS_RC_OK);
            }
        }
    }

    printf("Create PHY device...\n");
    for (j = 0; j < max_phy_ports; j++) {
        if ( phy_dev_create( j, mode )) {
            return (FAILED);
        }
    }

    printf("Device Pre-reset...\n");
    for (j = 0; j < max_phy_ports; j++) {
        if ( phy_dev_pre_reset(j)) {
            return (FAILED);
        }
    }

    printf("Device Reset...\n");
    for (j = 0; j < max_phy_ports; j++) {
        if ( phy_dev_reset(j)) {
            return (FAILED);
        }
    }

    printf("Device Post-reset...\n");
    for (j = 0; j < max_phy_ports; j++) {
        if ( phy_dev_post_reset(j)) {
            return (FAILED);
        }
    }

    printf("Enable PCS Autoneg...\n");
    for (j = 0; j < max_phy_ports; j++) {
        if ( vscphy_enable_pcs_autoneg(j)) {
            return (FAILED);
        }
    }

    printf("Device Init...\n");
    for (j = 0; j < max_phy_ports; j++) {
        if ( phy_dev_init( j, time_stamp_input )) {
            return (FAILED);
        }
    }

    for (j = 0; j < max_phy_ports; j++) {
        phyPortInitialized[j] = 1;  // static variable to indicate port state
    }

    sleep(3);

    return rv;
}

int wallander_set_phy_loopback (int port_no, BOOL sys_lpbk, BOOL line_lpbk)
{
    dev_object_t *phy_obj;
    vtss_target_type_t type;
    vtss_phy_loopback_t loopback;

    int rc = VTSS_RC_OK;

    if ( vsc_phy_ready( port_no, &type )) {
        return ( FAILED );
    }

    vtss_phy_loopback_get(api_gptr->vsc_instance, port_no, &loopback);

    loopback.near_end_enable = sys_lpbk;  // Set to TRUE for enabling Near End loopback 
    loopback.far_end_enable  = line_lpbk;  // Set to TRUE for enabling Far End loopback 

    // Setup the loopback
    vtss_phy_loopback_set(api_gptr->vsc_instance, port_no, loopback);

    // Example of getting the current loopback settings
    vtss_phy_loopback_get(api_gptr->vsc_instance, port_no, &loopback);
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        printf("loopback for port:%d near_end_loopback=%d, far_end_loopback = %d \n",
            port_no,
            loopback.near_end_enable,
            loopback.far_end_enable);
    }
    return (rc);
}


/* Trace callout function */
void vtss_callout_trace_printf(
    const vtss_trace_layer_t layer,
    const vtss_trace_group_t group,
    const vtss_trace_level_t level,
    const char *file,
    const int line,
    const char *function,
    const char *format,
    ...)
{
    va_list va;
    time_t  t;
    int     h, m, s;

    /* Get rid of warnings */

    t = time(NULL);
    h = (t / 3600 % 24);
    m = (t / 60 % 60);
    s = (t % 60);
    printf("%u:%02u:%02u %s %s: ",
           h, m, s,
           level == VTSS_TRACE_LEVEL_ERROR ? "Error" :
           level == VTSS_TRACE_LEVEL_INFO ?  "Info " :
           level == VTSS_TRACE_LEVEL_DEBUG ? "Debug" :
           level == VTSS_TRACE_LEVEL_NOISE ? "Noise" : "?????",
           function);

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    printf("\n");
}

BOOL api_locked = FALSE;

void vtss_callout_lock(const vtss_api_lock_t *function)
{
    // For testing we don't get a deadlock. The API must be unlocked before "locking"
    if (api_locked) {
        printf("(%s) API lock problem\n",__FUNCTION__);
    }
    api_locked = TRUE;
}

void vtss_callout_unlock(const vtss_api_lock_t *function)
{
    // For testing we don't get a deadlock. vtss_callout_lock must have been called before vtss_callout_unlock is called.
    if (!api_locked) {
        printf("(%s) API unlock problem\n",__FUNCTION__);
    }
    api_locked = FALSE;
}

unsigned int vtss_os_ctz( unsigned int val32 ) {

    int i;

    if ( val32 == 0 )
    return 0;

    for ( i=0; i<32; i++ ) {
    if ( val32 & ( 1 << i ))
        return i;
    }

    return 0;
}


void *vtss_os_malloc( unsigned int size, unsigned int flags ) {
    unsigned int *ptr;
    ptr = malloc( size );
    printf("vtss_os_malloc size=0x%x flags=0x%x ptr=0x%p \n\r", size, flags, ptr );
    return ptr;
}


void vtss_os_free( unsigned int *ptr, unsigned int flags ) {
    printf("vtss_os_free flags=0x%x ptr=0x%p \n\r", flags, ptr  );
    free( ptr );
}


/******** History ********
$Log: diag_vtss_phy.c,v $
Revision 1.2  2015/07/14 06:35:03  xiaoyizh
Remove vtss_squelch_workaround since it is already included in Vitesse API.
Remove some cterr to avoid unwanted error counter increase.

Revision 1.1  2015/02/26 07:18:29  xiaoyizh
Initial check in for Wallander.


$Endlog$
*/

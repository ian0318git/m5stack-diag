/* $Id: diag_esw_util.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_esw_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "ethernet.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "diag_smi_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_pkt_txrx_lib.h"
#include "diag_cpu_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "dev_88e6176.h"
#include "dev_88e6390.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"

static dev_88e6390_object_t dev_88e6390_obj;
static dev_88e6176_object_t dev_88e6176_obj;

/*******************************************************************************
 * Function   : esw_send_packet_util
 *
 * Description: Utility to run actual loopback test 
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int esw_send_packet_util (int opt)
{
    if (plat_sgmii_lpbk_test(ETH1, opt) != PASSED) {
        printf("ESW  %dmbps ext. loopback test failed.\n", opt);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : esw_set_allports_forward_util
 *
 * Description: Function to set switch all ports forwarding.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_set_allports_forward_util (void)
{
    int ctr = 0;
    int start_port = 0, end_port = 0;
    int result=FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
        start_port = ESW_PORT0;
        end_port = ESW_PORT10;
    } else {
        esw_obj_p = &dev_88e6176_obj;
        start_port = (int)(PLAT_M_ESW_PORT_REG_BASE + ESW_PORT0);
        end_port = (int)(PLAT_M_ESW_PORT_REG_BASE + ESW_PORT6);
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    for (ctr = start_port; ctr <= end_port; ctr++) {
        /* setting forward on 88E6390 */
        if (platform_esw_type() == ESW_MRVL88E6390) {
            if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_set_port_forward
                ((dev_object_t *)esw_obj_p, ctr) != PASSED) {
                printf("%s:%d:Failed for set forward on 88E6390\n", __FUNCTION__, __LINE__);
                goto _exit;
            }
            result = PASSED;

        /* setting forward on 88E6176 */
        } else {
            if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_set_port_forward
                ((dev_object_t *)esw_obj_p, ctr) != PASSED) {
                printf("%s:%d:Failed for set forward on 88E6176\n", __FUNCTION__, __LINE__);
                goto _exit;
            }
            result = PASSED;
        }
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function: diag_esw_reg_rd_util
 *
 * Description: Utility to read ESW register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_reg_rd_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_reg_read_util
           ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW register read utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_reg_read_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW register read utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function: diag_esw_reg_wr_util
 *
 * Description: Utility to write ESW register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_reg_wr_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_reg_write_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW register write utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_reg_write_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW register write utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function: diag_esw_phy_reg_rd_util
 *
 * Description: Utility to read ESW PHY register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_phy_reg_rd_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_read_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW PHY register read utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_read_util
           ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW PHY register read utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function: diag_esw_phy_reg_wr_util
 *
 * Description: Utility to write ESW PHY register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_phy_reg_wr_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_write_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW PHY register write utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_write_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW PHY register write utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/******************************************************************************
 * Function:diag_smi_c45_rd_util
 *
 * Description: Utility to read ESW PHY register (Clause 45).
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_smi_c45_rd_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility(Clause 45) */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_read_util
           ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW PHY register read utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_read_util
           ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW PHY register read utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/*******************************************************************************
 * Function   : viper_esw_smi_c45_wr_util
 *
 * Description: Utility to write ESW PHY register (Clause 45).
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_smi_c45_wr_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register write utility(Clause 45) */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_write_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW C45 PHY register write utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_write_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW C45 PHY register write utility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy
        ((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy
        ((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/*******************************************************************************
 * Function   : diag_esw_set_1k_testmode_util
 *
 * Description: Utility to set ESW PHY 1000BaseT test mode.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_set_1k_testmode_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 utility to set testmode */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_set_testmode_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW PHY set test modeutility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_set_testmode
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW PHY set test modeutility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/*******************************************************************************
 * Function   : diag_esw_force_led_onoff_util
 *
 * Description: Utility to force ESW port LEDs ON/OFF.
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_force_led_onoff_util(void)
{
    int     port_opt = 1;
    boolean onoff = 0;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        port_opt = gethex_answer("Enter port number(1~8, 0xf for all): ", 0x1, 0x1, 0xf);
        onoff = (boolean)getdec_answer("Turn it ON/OFF.(0: OFF, 1: ON): ", 0, 0, 1);
    } else if (platform_esw_type() == ESW_MRVL88E6176) {
        port_opt = gethex_answer("Enter port number(0~3, 0xf for all): ", 0x0, 0x0, 0xf);
        onoff = (boolean)getdec_answer("Turn it ON/OFF.(0: OFF, 1: ON): ", 0, 0, 1);
    }

    if (diag_esw_force_led_onoff(port_opt, onoff) != PASSED) {
        printf("Failed to Turn %s ESW LED.\n",
               (onoff == 1) ? "ON" : "OFF");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_esw_port_vod_adjust_util
 *
 * Description: Utility to adjust ESW port VOD.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_port_vod_adjust_util (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &dev_88e6390_obj;
    } else {
        esw_obj_p = &dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 utility to set testmode */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_set_vod_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6390 ESW PHY set test modeutility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_set_vod_util
            ((dev_object_t *)esw_obj_p) != PASSED) {
            printf("%s:%d:Failed in 88E6176 ESW PHY set test modeutility\n", __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    }

    /* use to destroy ESW device object */
_exit:
    if (platform_esw_type() == ESW_MRVL88E6390) {
        ((dev_88e6390_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    } else {
        ((dev_88e6176_object_t *)esw_obj_p)->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    }

    return (result);
}

/*-------------------------------------------------
 * $Log: diag_esw_util.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

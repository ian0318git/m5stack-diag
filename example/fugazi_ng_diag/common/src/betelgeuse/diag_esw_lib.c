/* $Id: diag_esw_lib.c,v 1.2 2019/01/10 06:36:22 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_esw_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_lib.c
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
#include "diag_cpu_lib.h"

extern uint32 err_report(dev_object_t *, char *, uint32);
static dev_88e6390_object_t lib_dev_88e6390_obj;
static dev_88e6176_object_t lib_dev_88e6176_obj;
static int diag_esw_phy_tx_rx_test(void);
static int diag_esw_phy_pwr_up_all_ge_port(void);
static int diag_esw_phy_pwr_up_serdes_port (void);
static int diag_esw_rgmii_config_for_wifi_comm(void);

/*******************************************************************************
 * Function    : diag_esw_force_cpu_linkup 
 *
 * Description : Function to force link down CUP side MAC, 
 *               then force link up it
 * Inputs      :
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
int diag_esw_force_cpu_linkup (void)
{
    uint cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(PLAT_ESW_CPU_MACNUM);
    uint cpu_reg = 0;

    /* force link down CPU side MAC */
    cpu_reg |= (uint)PANCR_FORCE_LINK_DOWN;
    if (plat_mem_write32(cpu_reg_addr, cpu_reg) != PASSED) {
        printf("%s(%d): Failed to force link down CPU side MAC\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    msleep(ESW_ACCESS_WAITTIME);

    /* force link up CPU side MAC */
    cpu_reg = 0;
    cpu_reg =(uint) (PANCR_RESERVED | 
                     PANCR_SET_FULL_DUPLEX |
                     PANCR_SUPPORT_FC |
                     PANCR_SET_SGMII_1000 |
                     PANCR_FORCE_LINK_UP);
    if (plat_mem_write32(cpu_reg_addr, cpu_reg) != PASSED) {
        printf("%s(%d): Failed to force link up CPU side MAC\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    msleep(ESW_ACCESS_WAITTIME);
    return (PASSED);
}

/*******************************************************************************
 * Function    : diag_esw_smi_rd 
 *
 * Description : Function to read Ethernet Switch Register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
int diag_esw_smi_rd (int cmd, ushort *buf) 
{
    switch (platform_esw_type()) {
        case ESW_MRVL88E6390:
            return (plat_smi_read(PHY_88E6390_SMIADDR, cmd, buf));
            break;
        case ESW_MRVL88E6176:
            return (plat_smi_read(PHY_88E6176_SMIADDR, cmd, buf));
            break;
        default:
            printf("%s:%d:Can't get the correct ESW PHY chip type. Please check the PID\n", __FUNCTION__, __LINE__);
            return (FAILED);
    }
}

/*******************************************************************************
 * Function    : diag_esw_smi_wr 
 *
 * Description : Function to write Ethernet Switch Register through SMI
 * Inputs      : addr - Register Address
 *               buf - pointer to the buffer
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
int diag_esw_smi_wr (int cmd, ushort buf) 
{
    switch (platform_esw_type()) {
        case ESW_MRVL88E6390:
            return (plat_smi_write(PHY_88E6390_SMIADDR, cmd, buf));
            break;
        case ESW_MRVL88E6176:
            return (plat_smi_write(PHY_88E6176_SMIADDR, cmd, buf));
            break;
        default:
            printf("%s:%d:Can't get the correct ESW PHY chip type. Please check the PID\n", __FUNCTION__, __LINE__);
            return (FAILED);
    }
}

/*******************************************************************************
 * Function    : diag_esw_phy_tx_rx_test 
 *
 * Description : run actual loopback test
 * Inputs      : 
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
int diag_esw_phy_tx_rx_test (void) 
{
    return (plat_sgmii_lpbk_test(PLAT_ESW_ETHNUM, RSV_SPD_FIELD));
}

/*******************************************************************************
 * Function   : diag_esw_init
 *
 * Description: Function to init switch(Marvell 88E6390).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_init (void)
{
    uint   fpga_reg_offset = (uint)FPGA_EXTER_DEV_RST_REG;
    uint   fpga_reg_val = 0;

    /* Release ESW from reset if needed */
    if (fpga_read_32_reg(fpga_reg_offset, &fpga_reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.0x%04X.\n",
               __FUNCTION__, fpga_reg_offset);
        return (FAILED);
    }

    if ((fpga_reg_val & (uint)EXT_ESW_RESET) == EXT_ESW_RESET) {
        fpga_reg_val &= (uint)(~EXT_ESW_RESET);

        if (fpga_write_32_reg(fpga_reg_offset, fpga_reg_val) != PASSED) {
            printf("%s: Failed to release ESW from reset.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(50);
    }

    /* Power Up all GE ports */
    if (diag_esw_phy_pwr_up_all_ge_port() != PASSED) {
        printf("%s:%d: Failed to power up all ESW PHY GE port\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Power up ESW to CPU SERDES port: */
    if (diag_esw_phy_pwr_up_serdes_port() != PASSED) {
        printf("%s:%d: Failed to power up SERDES port\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    msleep(20);

    /* Set ESW all ports forwarding */
    if (diag_esw_set_allports_forward_util() != PASSED) {
        printf("%s: Failed to set ESW all ports forwarding.", __FUNCTION__);
        return (FAILED);
    }

    /* RGMII configuration for WiFI SKUs due to WiFI module
     * connect to 88E6390 ESW Port0. 
     * This configuration makes WiFI module can download 
     * WiFi kernel from platform by TFTP */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (diag_esw_rgmii_config_for_wifi_comm() != PASSED) {
            printf("%s:%d: Failed to config RGMII for WiFi communication\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_reset_esw_to_default
 *
 * Description: Function to reset ESW and re-init it.
 * Inputs     : quiet_opt - To print message(opt = FALSE) or not(opt = TRUE)
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_reset_esw_to_default (int quiet_opt) {
    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("This function will reset switch and re-init it.\n");
    }

    /* 1. Reset ESW. */
    /* 1-1. Put switch in Reset. */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, TRUE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to put switch in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    msleep(ESW_RESET_ONE_SEC);

    /* 1-2. Release switch from Reset. */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("switch is reset successfully.\n");
    }

    /* 2. Re-init ESW. */
    if (diag_esw_init() != PASSED) {
        printf("%s: Failed to init ethernet switch.\n", __FUNCTION__);
        return (FAILED);
    }

    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("switch is reset and re-inited successfully.\n");
    }

    return (PASSED);
}

/*******************************************************************************
 * Function    : diag_esw_reset 
 *
 * Description : Function to reset ESW PHY
 * Inputs      : 
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
int diag_esw_reset (void) 
{
    return (diag_reset_esw_to_default(TRUE));
}

/*******************************************************************************
 * Function    : diag_esw_dev_create
 *
 * Description : Function to create ESW Device Object
 * Inputs      : esw_obj - Pointer of 88E6176 device driver object
 * Outputs     : PASSED / FAILED
 *******************************************************************************/
int diag_esw_dev_create (void *input_esw_obj)
{
    dev_88e6390_object_t *dev_88e6390_esw_obj = (platform_esw_type() == ESW_MRVL88E6390)? input_esw_obj:NULL;
    dev_88e6176_object_t *dev_88e6176_esw_obj = (platform_esw_type() == ESW_MRVL88E6176)? input_esw_obj:NULL;

    dev_object_t *dev = NULL;

    switch (platform_esw_type()) {
        case ESW_MRVL88E6390:
            dev = (dev_object_t *)dev_88e6390_esw_obj;

            /* Create common device object */
            mrv88e6390_dev_create(dev, (dev_error_report_t)err_report);

            /* Attach the device */
            dev_88e6390_esw_obj->base.dev_object_fvt->dev_attach(dev);

            /* Setup call-out function vectors */
            dev_88e6390_esw_obj->callout_fvt->rd = diag_esw_smi_rd;
            dev_88e6390_esw_obj->callout_fvt->wr = diag_esw_smi_wr;
            dev_88e6390_esw_obj->callout_fvt->esw_phy_tx_rx_test = diag_esw_phy_tx_rx_test;
            dev_88e6390_esw_obj->callout_fvt->reset = diag_esw_reset;
            dev_88e6390_esw_obj->callout_fvt->chk_intr_assert = diag_check_esw_ext_intr_pending;
            dev_88e6390_esw_obj->callout_fvt->chk_intr_deassert = diag_check_esw_ext_no_intr_pending;
        break;
    
        case ESW_MRVL88E6176:
            dev = (dev_object_t *)dev_88e6176_esw_obj;

            /* Create common device object */
            mrv88e6176_dev_create(dev, (dev_error_report_t)err_report);

            /* Attach the device */
            dev_88e6176_esw_obj->base.dev_object_fvt->dev_attach(dev);

            /* Setup call-out function vectors */
            dev_88e6176_esw_obj->callout_fvt->rd = diag_esw_smi_rd;
            dev_88e6176_esw_obj->callout_fvt->wr = diag_esw_smi_wr;
            dev_88e6176_esw_obj->callout_fvt->sgmii_lpbk_test = diag_esw_phy_tx_rx_test;
            dev_88e6176_esw_obj->callout_fvt->chk_intr_assert = diag_check_esw_ext_intr_pending;
            dev_88e6176_esw_obj->callout_fvt->chk_intr_deassert = diag_check_esw_ext_no_intr_pending;
        break;
    
        default:
            dev = NULL;
        break;
    }

    if (dev == NULL) {
        printf("ESW device create fail\n");
        return (FAILED);
    } else {
        return (PASSED);
    } 
}

/*******************************************************************************
 * Function   : diag_esw_reg_rd_if
 *
 * Description: Function to read ESW register.
 * Inputs     : dev_addr - SMI device addr.
 *              reg_addr - SMI register addr.
 *              wr_data  - buffer to put the value that wanted to write in
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
int diag_esw_reg_rd_if (int dev_addr, int reg_addr, ushort *buf)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_reg_read(
             (dev_object_t *)esw_obj_p, dev_addr, reg_addr, buf) != PASSED) {
            printf("%s:%d:Failed to read data into 88E6390 phy register\n", 
                   __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_reg_read(
             (dev_object_t *)esw_obj_p, dev_addr, reg_addr, buf) != PASSED) {
            printf("%s:%d:Failed to read data into 88E6176 phy register\n", 
                   __FUNCTION__, __LINE__);
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
 * Function   : diag_esw_reg_wr_if
 *
 * Description: Function to write ESW register.
 * Inputs     : dev_addr - SMI device addr.
 *              reg_addr - SMI register addr.
 *              wr_data  - buffer to put the value that wanted to write in
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
int diag_esw_reg_wr_if (int dev_addr, int reg_addr, ushort wr_data)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_reg_write(
             (dev_object_t *)esw_obj_p, dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s:%d:Failed to read data into 88E6390 phy register\n", 
                   __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_reg_write(
             (dev_object_t *)esw_obj_p, dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s:%d:Failed to write data into 88E6176 phy register\n", 
                   __FUNCTION__, __LINE__);
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
 * Function   : diag_esw_phy_reg_rd_if
 *
 * Description: Function to read ESW PHY register.
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_phy_reg_rd_if (int phy_port, int reg_page, int reg_addr, ushort *buf)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_read(
             (dev_object_t *)esw_obj_p, phy_port, reg_page, reg_addr, buf) != PASSED) {
            printf("%s:%d:Failed to read data from 88E6390 phy register\n", 
                   __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_read(
             (dev_object_t *)esw_obj_p, phy_port, reg_page, reg_addr, buf) != PASSED) {
            printf("%s:%d:Failed to read data from 88E6176 phy register\n", 
                   __FUNCTION__, __LINE__);
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
 * Function   : diag_esw_phy_reg_wr_if
 *
 * Description: Function to write ESW PHY register.
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              wr_in    - write value
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_phy_reg_wr_if (int phy_port, int reg_page, int reg_addr, ushort wr_in)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_write(
             (dev_object_t *)esw_obj_p, phy_port, reg_page, reg_addr, wr_in) != PASSED) {
            printf("%s:%d:Failed to write data into 88E6390 phy register\n", 
                   __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_phy_reg_write(
             (dev_object_t *)esw_obj_p, phy_port, reg_page, reg_addr, wr_in) != PASSED) {
            printf("%s:%d:Failed to write data into 88E6176 phy register\n", 
                   __FUNCTION__, __LINE__);
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
 * Function   : diag_esw_smi_c45_rd_if
 *
 * Description: Function to read ESW registers (Clause 45)
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_smi_c45_rd_if (int phy_port, int dev_num, int reg_addr, ushort *buf)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_read(
             (dev_object_t *)esw_obj_p, phy_port, dev_num, reg_addr, buf) != PASSED) {
            printf("%s:%d:Failed to read data into 88E6390 phy register\n", 
                   __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_read(
             (dev_object_t *)esw_obj_p, phy_port, dev_num, reg_addr, buf) != PASSED) {
            printf("%s:%d:Failed to read data into 88E6176 phy register\n", 
                   __FUNCTION__, __LINE__);
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
 * Function   : diag_esw_smi_c45_wr_if
 *
 * Description: Function to write ESW registers (Clause 45)
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_esw_smi_c45_wr_if (int phy_port, int dev_num, int reg_addr, ushort wr_in)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* 88E6390 or 88E6176 ESW PHY register read utility */
    if (platform_esw_type() == ESW_MRVL88E6390) {
        if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_write(
             (dev_object_t *)esw_obj_p, phy_port, dev_num, reg_addr, wr_in) != PASSED) {
            printf("%s:%d:Failed to write data into 88E6390 phy register\n", 
                   __FUNCTION__, __LINE__);
            goto _exit;
        }
        result = PASSED;
    } else {
        if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_c45_phy_reg_write(
             (dev_object_t *)esw_obj_p, phy_port, dev_num, reg_addr, wr_in) != PASSED) {
            printf("%s:%d:Failed to write data into 88E6176 phy register\n", 
                   __FUNCTION__, __LINE__);
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
 * Function   : diag_esw_force_led_onoff
 *
 * Description: Function to force ESW LEDs ON/OFF.
 * Inputs     : port_opt - port number
 *              onoff    - turn LED(s) ON/OFF
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_esw_force_led_onoff (int port_num, boolean onoff)
{
    int result = FAILED, port_ctr;
    int start_port = port_num, end_port = port_num;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
        /* ALL port */
        if (port_num == ALL_ESW_LEDS) {
            start_port = ESW_PORT1;
            end_port   = ESW_PORT8;     
        }
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
        /* ALL port */
        if (port_num == ALL_ESW_LEDS) {
            start_port = ESW_PORT0;
            end_port   = ESW_PORT3;     
        }
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    for (port_ctr = start_port; port_ctr <= end_port; port_ctr++)
    {
        /* 88E6390 or 88E6176 ESW SERDES port power up function  */
        if (platform_esw_type() == ESW_MRVL88E6390) {
            if (onoff == ESW_LED_F_ON) {
                /* turn on LED */
                if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_set_led_on(
                     (dev_object_t *)esw_obj_p, port_ctr) != PASSED) {
                    printf("%s:%d:Failed to turn ON LED on port:%d\n", 
                           __FUNCTION__, __LINE__, port_ctr);
                    goto _exit;
                }
            } else {
                /* turn on LED */
                if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_set_led_off(
                     (dev_object_t *)esw_obj_p, port_ctr) != PASSED) {
                    printf("%s:%d:Failed to turn OFF LED on port:%d\n", 
                           __FUNCTION__, __LINE__, port_ctr);
                    goto _exit;
                }
            }
            result = PASSED;
        } else {
            if (onoff == ESW_LED_F_ON) {
                /* turn on LED */
                if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_set_led_on(
                     (dev_object_t *)esw_obj_p, port_ctr) != PASSED) {
                    printf("%s:%d:Failed to turn ON LED on port:%d\n", 
                           __FUNCTION__, __LINE__, port_ctr);
                    goto _exit;
                }
            } else {
                /* turn on LED */
                if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_set_led_off(
                     (dev_object_t *)esw_obj_p, port_ctr) != PASSED) {
                    printf("%s:%d:Failed to turn OFF LED on port:%d\n", 
                           __FUNCTION__, __LINE__, port_ctr);
                    goto _exit;
                }
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

/*******************************************************************************
 * Function   : diag_esw_phy_pwr_up_all_ge_port
 *
 * Description: Function to power up all GE PHY port which is inside ESW device.
 *              For 88E6390, Betelgeuse use port1 ~ port 8 for 8P E2E SKUs.
 *              For 88E6176, Betelgeuse use port0 ~ port 3 for 4P E2E SKUs.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
static int diag_esw_phy_pwr_up_all_ge_port (void)
{
    int result = FAILED;
    int port_num = 0, start_port = 0, end_port = 0;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

    /* set start port and end port */ 
    if (platform_esw_type() == ESW_MRVL88E6390) {
        start_port = ESW_PORT1;
        end_port = ESW_PORT8;
    } else if (platform_esw_type() == ESW_MRVL88E6176){
        start_port = ESW_PORT0;
        end_port = ESW_PORT3;
    } else {
        printf("%s:%d:Can't get the correct ESW PHY chip type. Please read cookie first\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }


    for (port_num = start_port; port_num <= end_port; port_num++) 
    {
        /* 88E6390 or 88E6176 ESW PHY power up function  */
        if (platform_esw_type() == ESW_MRVL88E6390) {
            if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_pwr_up_ge_port(
                 (dev_object_t *)esw_obj_p, port_num) != PASSED) {
                printf("%s:%d:Failed to power up Port:%d\n", 
                       __FUNCTION__, __LINE__, port_num);
                goto _exit;
            }
            result = PASSED;
        } else {
            if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_pwr_up_ge_port(
                 (dev_object_t *)esw_obj_p, port_num) != PASSED) {
                printf("%s:%d:Failed to power up Port:%d\n", 
                       __FUNCTION__, __LINE__, port_num);
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

/*******************************************************************************
 * Function   : diag_esw_phy_pwr_up_serdes_port
 *
 * Description: Function to power up ESW SERDES port.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
static int diag_esw_phy_pwr_up_serdes_port (void)
{
    int result = FAILED;
    void *esw_obj_p = NULL;

    if (platform_esw_type() == ESW_MRVL88E6390) {
        esw_obj_p = &lib_dev_88e6390_obj;
    } else {
        esw_obj_p = &lib_dev_88e6176_obj;
    }

    /* create ESW device object */
    if (diag_esw_dev_create(esw_obj_p) !=PASSED) {
        return (FAILED);
    }

     /* 88E6390 or 88E6176 ESW SERDES port power up function  */
     if (platform_esw_type() == ESW_MRVL88E6390) {
         if (((dev_88e6390_object_t *)esw_obj_p)->callin_fvt->esw_pwr_up_serdes_port(
              (dev_object_t *)esw_obj_p) != PASSED) {
             printf("%s:%d:Failed to power up SERDES Port\n", 
                    __FUNCTION__, __LINE__);
             goto _exit;
         }
         result = PASSED;
     } else {
         if (((dev_88e6176_object_t *)esw_obj_p)->callin_fvt->esw_pwr_up_cpu_port(
              (dev_object_t *)esw_obj_p, ESW_CPU_PORT_ADDR) != PASSED) {
             printf("%s:%d:Failed to power up SERDES Port:%d\n", 
                    __FUNCTION__, __LINE__, ESW_CPU_PORT_ADDR);
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
 * Function   : diag_esw_rgmii_config_for_wifi_comm
 *
 * Description: Function to config 88E6390 RGMII for WiFI SKUs
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
static int diag_esw_rgmii_config_for_wifi_comm (void)
{
    ushort reg_val = 0;
    int port_num = ESW_PORT0;
    int reg_addr = 0;

    if (platform_esw_type() != ESW_MRVL88E6390) {
        printf("%s:%d:This is not WiFi SKU\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    reg_addr = (int)ESW_PHYCTR_REG;

    reg_val = (ushort)(ESW_PCR_RGMII_RX_DELAY |
                       ESW_PCR_RGMII_TX_DELAY |
                       ESW_PCR_FORCE_SPEED |
                       ESW_PCR_FORCE_LINK |
                       ESW_PCR_F_FULLDPX |
                       ESW_PCR_FORCE_DPX |
                       ESW_PCR_1000MBPS);

    if (diag_esw_reg_wr_if(port_num, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to let ESW port%d force link down.",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    reg_val |= (ushort)(ESW_PCR_F_LINKUP);
    if (diag_esw_reg_wr_if(port_num, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to let ESW port%d link down.",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_esw_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

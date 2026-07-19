/* $Id: diag_ge_phy_lib.c,v 1.3 2019/06/24 07:21:37 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_ge_phy_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_ge_phy_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "nvmonvars.h"
#include "ethernet.h"
#include "common_utils.h"
#include "diag_cpu_lib.h"
#include "diag_pkt_txrx_lib.h"
#include "platform_cookie.h"
#include "diag_moka_fpga_lib.h"
#include "diag_smi_lib.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_test.h"
#include "diag_ge_phy_lib.h"
#include "diag_cpu_lib.h"

smi_t sfp_mac_media_mode = (uint)MSCR1_MOD_1000BASEX_ONLY; /* 111 = 1000Base-X only */
uint sfp_type = SFP_DEFAULT;
uint sfp_encode = SFP_ENCODE_UNKNOWN;

extern uint32 err_report(dev_object_t *, char *, uint32);
static dev_mrvl_ge_object_t mrvl_88e1112_obj; /* 88E1112 device object */
static smi_if_t mrvl_88e1112_smi;
static smi_t mrvl_88e1112_smi_buf;
static uint32_t diag_ge0_smi_read(smi_if_t *);
static uint32_t diag_ge0_smi_write(smi_if_t *);
static uint32_t diag_ge1_smi_read(smi_if_t *);
static uint32_t diag_ge1_smi_write(smi_if_t *);
static uint32_t diag_88e1112_smi_open(smi_if_t *);
static uint32_t diag_88e1112_smi_close(smi_if_t *);

int diag_ge_phy_setting_reg(int, smi_t, uchar, smi_t);
static int diag_set_sfp_glc_ge_100fx(int);
static int diag_set_sfp_I2C_op(uint8_t, uint8_t, n2g_i2c_if_t *);
static int diag_set_sfp_glc_t_1000(int);
static int diag_get_sfp_reg(uint);
static int diag_read_sfp_ext_id(int);

/* for MAC loopback test setup */
static int diag_88e1112_cpu_phy_mac_setup(int, int);
static int diag_88e1112_cpu_to_ge0_mac_config (int);
static int diag_88e1112_cpu_to_ge1_mac_config (int);
static int diag_88e1112_cpu_to_ge0_mac_check_linkup (void);

/* for external loopback test setup */
static int diag_88e1112_cpu_ext_lpbk_init(int);
static int diag_88e1112_cpu_to_ge0_ext_lpbk_init(void);
static int diag_88e1112_cpu_to_ge1_ext_lpbk_init(void);

/* for SFP external loopback test setup */
static int diag_88e1112_sfp_setup(int);
static int diag_88e1112_sfp_get_media_mode(uint16_t *);
static int diag_88e1112_sfp_tx_disable(void);
static int diag_88e1112_sfp_tx_enable(void);

/* for interrupt test */
static int diag_ge0_chk_intr_assert(void);
static int diag_ge1_chk_intr_assert(void);
static int diag_ge0_chk_intr_deassert(void);
static int diag_ge1_chk_intr_deassert(void);

/* for actual loopback test */
static int diag_88e1112_ge0_phy_tx_rx_test(void);
static int diag_88e1112_ge1_phy_tx_rx_test(void);

static char *connector[16] = {
	"Unknown",
	"SC",
	"Fibre Channel Style 1 copper connector",
	"Fibre Channel Style 2 copper connector",
	"BNC/TNC",
	"Fibre Channel coaxial headers",
	"FiberJack",
	"LC",
	"MT-RJ",
	"MU",
	"SG",
	"Optical pigtail"};

/**********************************************************************
 * Function: diag_get_88e11112_obj
 * Description: (1) Get the 88E1112c device driver object
 *              (2) Attach configurations to device object
 * Inputs     : GE0/GE1 
 * Outputs    : address of device object
 **********************************************************************
 */
dev_object_t *diag_get_88e11112_obj (int ge_num)
{
    /* [Pointer Config] Point to actual structure of device object */
    dev_mrvl_ge_object_t *mrvl_obj_p = &mrvl_88e1112_obj;
    int rc;
    
    /* Setup device struct */
    mrvl_obj_p->reg_info_p = 0;    /* Use default table */

    /* Setup device object base */
    rc = mrvl_n88e111x_dev_create((dev_object_t *)mrvl_obj_p, (dev_error_report_t) err_report);
    if (rc != PASSED) {
        printf("%s:%d:Failed to create n88e111x object\n", __FUNCTION__, __LINE__);
        return (NULL);
    }

    /* Setup call-out function vectors */
    mrvl_obj_p->callout_fvt->rd                        = (ge_num == GE0)? diag_ge0_smi_read:diag_ge1_smi_read;
    mrvl_obj_p->callout_fvt->wr                        = (ge_num == GE0)? diag_ge0_smi_write:diag_ge1_smi_write;
    mrvl_obj_p->callout_fvt->open                      = diag_88e1112_smi_open;
    mrvl_obj_p->callout_fvt->close                     = diag_88e1112_smi_close;
    mrvl_obj_p->callout_fvt->sfp_setup                 = diag_88e1112_sfp_setup;
    mrvl_obj_p->callout_fvt->sfp_get_media_mode        = diag_88e1112_sfp_get_media_mode;
    mrvl_obj_p->callout_fvt->sfp_tx_disable            = diag_88e1112_sfp_tx_disable;
    mrvl_obj_p->callout_fvt->sfp_tx_enable             = diag_88e1112_sfp_tx_enable;
    mrvl_obj_p->callout_fvt->cpu_phy_mac_setup         = (ge_num == GE0)? diag_88e1112_cpu_to_ge0_mac_config:diag_88e1112_cpu_to_ge1_mac_config;
    mrvl_obj_p->callout_fvt->cpu_phy_mac_check_linkup  = (ge_num == GE0)? diag_88e1112_cpu_to_ge0_mac_check_linkup:NULL;
    mrvl_obj_p->callout_fvt->cpu_phy_mac_autoneg_setup = (ge_num == GE0)? diag_88e1112_cpu_to_ge0_ext_lpbk_init:diag_88e1112_cpu_to_ge1_ext_lpbk_init; 
    mrvl_obj_p->callout_fvt->ge_phy_tx_rx_test         = (ge_num == GE0)? diag_88e1112_ge0_phy_tx_rx_test:diag_88e1112_ge1_phy_tx_rx_test;
    mrvl_obj_p->callout_fvt->chk_intr_assert           = (ge_num == GE0)? diag_ge0_chk_intr_assert:diag_ge1_chk_intr_assert;
    mrvl_obj_p->callout_fvt->chk_intr_deassert         = (ge_num == GE0)? diag_ge0_chk_intr_deassert:diag_ge1_chk_intr_deassert;

    /* [Pointer Config] Point to actual structure of SMI interface*/ 
    mrvl_obj_p->smi_p = &mrvl_88e1112_smi;
    mrvl_obj_p->smi_p->offset = 0; 
    mrvl_obj_p->smi_p->buf = &mrvl_88e1112_smi_buf;
    
    mrvl_obj_p->type = MRVL_GE_PHY_1112;

    /* Attach the device object */
    rc = mrvl_obj_p->base.dev_object_fvt->dev_attach((dev_object_t *)mrvl_obj_p);
    if (rc != PASSED) {
        printf("%s:%d:Failed to attach n88e111x object\n", __FUNCTION__, __LINE__);
        return (NULL);
    }

    return ((dev_object_t *)mrvl_obj_p);
}

/**********************************************************************
 * Function: diag_88e1112_cpu_phy_mac_setup
 * Description: setpu CPU side MAC for MAC loopback test 
 * Inputs     : ge_num   - GE0/GE1
 *              test_spd - 10Mbps/100Mbps/1000Mbps
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_88e1112_cpu_phy_mac_setup (int ge_num, int test_spd)
{
    uint cpu_reg_addr = 0, cpu_reg_val = 0;

    /* Config CPU register offset with input GE number */
    switch (ge_num) {
        case GE0:
            cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(0);
        break;

        case GE1:
            cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);
        break;

        default:
            printf("Unrecognized GE number: GE%d\n",ge_num);
            return (FAILED);
        break;
    }

    /* Force link down CPU GE PHY */
    cpu_reg_val = (uint)PANCR_FORCE_LINK_DOWN; 
    if (plat_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s:%d:Failed to config CPU GE%d reg. 0x%08X\n", 
               __FUNCTION__, __LINE__, ge_num, cpu_reg_addr);
        return (FAILED);
    }

    /* Config CPU GE MAC and force link up */
    cpu_reg_val = (uint)(PANCR_RESERVED |
                         PANCR_SET_FULL_DUPLEX |
                         PANCR_AN_FC_EN |
                         PANCR_INBAND_BYPASS_EN |
                         PANCR_INBAND_AN_EN |
                         PANCR_FORCE_LINK_UP);
    switch (test_spd) {
        case SPD_10MBPS:
            cpu_reg_val |= (uint)PANCR_SET_MII_10;
        break;

        case SPD_100MBPS:
            cpu_reg_val |= (uint)PANCR_SET_MII_100;
        break;

        case SPD_1000MBPS:
            cpu_reg_val |= (uint)PANCR_SET_SGMII_1000;
        break;

        default:
            printf("Unsupported test speed: %dMbps\n",test_spd);
        break;
    }
    if (plat_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s:%d:Failed to config CPU GE%d reg. 0x%08X\n", 
               __FUNCTION__, __LINE__, ge_num, cpu_reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 * Function: diag_88e1112_cpu_to_ge0_mac_config
 * Description: config CPU side MAC with GE0 port
 * Inputs     : test_spd - SPD_10MBPS, SPD_100MBPS and SPD_1000MBPS
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_88e1112_cpu_to_ge0_mac_config (int test_spd)
{
    return (diag_88e1112_cpu_phy_mac_setup(GE0, test_spd));
}

/**********************************************************************
 * Function: diag_88e1112_cpu_to_ge1_mac_config
 * Description: config CPU side MAC with GE0 port
 * Inputs     : test_spd - SPD_10MBPS, SPD_100MBPS and SPD_1000MBPS
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_88e1112_cpu_to_ge1_mac_config (int test_spd)
{
    return (diag_88e1112_cpu_phy_mac_setup(GE1, test_spd));
}

/**********************************************************************
 * Function: diag_88e1112_cpu_to_ge0_mac_check_linkup
 * Description: Confirm CPU MAC link to GE PHY MAC
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_88e1112_cpu_to_ge0_mac_check_linkup (void)
{
    return (plat_cpu_mac_check_linkstat(GE0, CPUMAC_LINKUP));
}

/**********************************************************************
 * Function: diag_88e1112_cpu_ext_lpbk_init
 * Description: setup CPU side MAC for external loopback test 
 * Inputs     : ge_num - GE0/GE1
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_cpu_ext_lpbk_init (int ge_num)
{
    uint cpu_reg_offset;
    uint rd_data;
    uint wr_data;
    int ctr = 0, rc = FAILED;

    cpu_reg_offset = (ge_num == GE0)? (uint)CPU_PORT_AN_CONF_REG(0):
                                          (uint)CPU_PORT_AN_CONF_REG(3);
    wr_data = (uint)PANCR_FORCE_LINK_DOWN;
    /* Force CPU GEMAC port link down for configure.
     * CPU offset: 0xF2130E0C + (m*0x1000) 
     * write data: 0x0001 */
    if (plat_mem_write32(cpu_reg_offset, wr_data) != PASSED) {
        printf("%s:%d: Write data:0x%04x to CPU reg:0x%04x, Fail to force link down CPU MAC\n", 
               __FUNCTION__, __LINE__, wr_data, cpu_reg_offset);
        return (FAILED);
    }

    /* Confirm CPU GEMAC is link down */
    /* check "Port Status Register0" */
    /* read CPU offset: 0xF2130E10 + (m*0x1000), Bit[0] */
    cpu_reg_offset = (ge_num == GE0)? (uint)CPU_PORT_STATUS_REG0(0):
                                      (uint)CPU_PORT_STATUS_REG0(3);
    for (ctr = 0; ctr < LINK_STATUS_POLLING_ROUND; ctr++) {
        rd_data = 0;
        if (plat_mem_read32(cpu_reg_offset, &rd_data) != PASSED) {
            printf("%s:%d: Read CPU reg:0x%04x, Fail to check link status\n", 
                   __FUNCTION__, __LINE__, cpu_reg_offset);
            return (FAILED);
        }
        /* if link down */
        if ((rd_data & CPU_PSR_LINK_UP) == CPU_PSR_LINK_DOWN) {
            rc = PASSED;
            break;
        }
        msleep(LINK_STATUS_POLLING_PERIOD);
    } 

    if (rc == FAILED) {
        printf("%s:%d:CPU MAC still link up\n", __FUNCTION__, __LINE__);
        printf("%s:%d:Polling CPU reg:0x%04x fail, pattern:0x%04x, read data:0x%04x\n",
               __FUNCTION__, __LINE__, cpu_reg_offset, CPU_PSR_LINK_DOWN, rd_data); 
        return (FAILED);
    }
    msleep(BUFFER_TIME);

    cpu_reg_offset = (ge_num == GE0)? (uint)CPU_PORT_AN_CONF_REG(0):
                                          (uint)CPU_PORT_AN_CONF_REG(3);
    wr_data = (uint)(PANCR_RESERVED |  /* Bit[15]=1, reserved */
              PANCR_AN_DUPLEX_EN |     /* Bit[13]=1, enable Auto-Nego in Duplex */
              PANCR_AN_FC_EN |         /* Bit[11]=1, enable Auto-Nego for flow control */
              PANCR_AN_SPEED_EN |      /* Bit[7] =1, enable Auto-Nego for interface speed */
              PANCR_INBAND_BYPASS_EN | /* Bit[3] =1, enable Auto-Nego bypass feature */
              PANCR_INBAND_AN_EN);     /* Bit[2] =1, enable In-Band Auto-Nego */
    /* Force CPU GEMAC port link down for configure.
     * CPU offset: 0xF2130E0C + (m*0x1000)
     * write data: 0x0001 */
    if (plat_mem_write32(cpu_reg_offset, wr_data) != PASSED) {
        printf("%s:%d: Write data:0x%04x to CPU reg:0x%04x, Fail to force link down CPU MAC\n", 
               __FUNCTION__, __LINE__, wr_data, cpu_reg_offset);
        return (FAILED);
    }

    msleep(BUFFER_TIME);
    return (PASSED);
}

/**********************************************************************
 * Function: diag_88e1112_cpu_to_ge0_ext_lpbk_init
 * Description: setup CPU side MAC for GE0 external loopback test
 * Inputs     :  
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_cpu_to_ge0_ext_lpbk_init (void)
{
    return (diag_88e1112_cpu_ext_lpbk_init(GE0));
}

/**********************************************************************
 * Function: diag_88e1112_cpu_to_ge1_ext_lpbk_init
 * Description: setup CPU side MAC for GE1 external loopback test
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_cpu_to_ge1_ext_lpbk_init (void)
{
    return (diag_88e1112_cpu_ext_lpbk_init(GE1));
}

/**********************************************************************
 * Function: diag_88e1112_sfp_setup 
 * Description: do all configuration and setup for SFP module
 *              before run SFP loopback test 
 * Inputs     : rsv - reserved
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_sfp_setup (int rsv) 
{
    int rc = 0;

    /* Check if SFP is present */
    if (is_sfp_present() != TRUE) {
        printf("SFP module is not detected.");
        return (rc);
    } else {
        printf("SFP module is detected");
    } 

    /* Set flag according to SFP encoding type 
     * 1. Get_sfp_encoding 
     *        sfp_cookie_read
     *    sfp_encode = 
     *        SFP_ENCODE_8B10B
     *        SFP_ENCODE_SONET
     *        SFP_ENCODE_4B5B 
     *            sfp_ge_100fx or sfp_fe_100fx.
     *   */
    switch (sfp_encode = diag_get_sfp_reg(SFP_COO_ENC)) {
        case SFP_ENCODE_8B10B:
            printf("\n8B10B\n");
            /* Configure the external loopback setting for SFP Copper Module 
             * GLC-TE */
            if (diag_get_sfp_reg(SFP_ETH_COMP_CODES) == SFP_1000BASE_T) {
                printf("Found GLC-TE\n");
                sfp_type = SFP_GLC_TE;
            }
            printf("Speed = 1000BASEX\n");
            sfp_mac_media_mode = (uint)MSCR1_MOD_1000BASEX_ONLY; /* media mode = 111, 1000Base-X only */
            break;
        case SFP_ENCODE_SONET:
            printf("\nSONET\n");
            printf("Speed = 1000BASEX\n");
            sfp_mac_media_mode = (uint)MSCR1_MOD_1000BASEX_ONLY; /* media mode = 111, 1000Base-X only */
            break;
        case SFP_ENCODE_4B5B:
            printf("\n4B5B\n");
            if (diag_read_sfp_ext_id(CPU_I2C1) == SFP_XID_GE_100FX) {
                printf("Found GLC-GE-100FX\n");
                printf("Speed = SGMII ONLY\n");
                sfp_type = SFP_GE_100FX;
                sfp_mac_media_mode = (uint)MSCR1_MOD_SGMII_ONLY; /* media mode = 110, SGMII only */
            } else 
            if (diag_read_sfp_ext_id(CPU_I2C1) == SFP_XID_FE_100FX) {
                printf("Speed = 100FX\n");
                sfp_type = SFP_FE_100FX;
                sfp_mac_media_mode = (uint)MSCR1_MOD_100BASE_FX; /* media mode = 000, 100BASE-FX */
            } else {
                printf("Unsupported SFP Ext. ID\n");
                goto restore_default;
            } 
            break;
        default:
            printf("%s:%d:Unsupported SFP encoding %#x\n", 
                   __FUNCTION__, __LINE__, sfp_encode);
            goto restore_default;
    };  

    /* Check sfp_encode and sfp_type to do bellow item:
     * Config SFP module mode (diag_set_sfp_glc_ge_100fx(CPU_I2C1) or diag_set_sfp_glc_t_1000(CPU_I2C1))
     */
    if ((sfp_encode == SFP_ENCODE_4B5B) && (sfp_type == SFP_GE_100FX)) {
        if (diag_set_sfp_glc_ge_100fx(CPU_I2C1) != PASSED) {
            printf("%s:%d:Failed to set sfp glc ge 100fx\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
    } else if ((sfp_encode == SFP_ENCODE_4B5B) && (sfp_type == SFP_FE_100FX)) {
        /* For GLC-FE-100FX using this default speed */
    } else if ((sfp_encode == SFP_ENCODE_8B10B) && (sfp_type == SFP_GLC_TE)) {
        /* For GLC-FE-100FX using this default speed and config SFP Internal Reg */
        if (diag_set_sfp_glc_t_1000(CPU_I2C1) != PASSED) {
            printf("%s:%d:Failed to set sfp glc ge 100fx\n", __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }


    /* Configure the external loopback setting for SFP Copper Module (sfp_type, sfp_mac_media_mode) */
    return (PASSED);
restore_default:
    sfp_encode = SFP_ENCODE_UNKNOWN;
    sfp_type = SFP_DEFAULT;
    sfp_mac_media_mode = (uint)MSCR1_MOD_1000BASEX_ONLY; /* 111 = 1000Base-X only */
    return (FAILED);
}

/**********************************************************************
 * Function: diag_88e1112_sfp_get_media_mode 
 * Description: 
 * Inputs     : media_mode - point to the specific MAC media mode
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_sfp_get_media_mode (smi_t *media_mode) 
{
    printf("sfp_mac_media_mode = %d\n", sfp_mac_media_mode);
    *media_mode  = (smi_t)(sfp_mac_media_mode << MSCR1_MOD_OFFSET); 
    return (PASSED);
}

/**********************************************************************
 * Function: diag_88e1112_sfp_tx_disable
 * Description: Disable SFP TX by FPGA
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_sfp_tx_disable (void)
{
    return (sfp_tx_enable_switch(DISABLE));
}

/**********************************************************************
 * Function: diag_88e1112_sfp_tx_enable
 * Description: Disable SFP TX by FPGA
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_88e1112_sfp_tx_enable (void)
{
    return (sfp_tx_enable_switch(ENABLE));
}

/**********************************************************************
 * Function: diag_88e1112_ge0_phy_tx_rx_test
 * Description: run loopback test for GE0 on specific media speed
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_88e1112_ge0_phy_tx_rx_test (void)
{
    return (plat_sgmii_lpbk_test(PLAT_GE0_ETHNUM, RSV_SPD_FIELD));
}

/**********************************************************************
 * Function: diag_88e1112_ge1_phy_tx_rx_test
 * Description: run loopback test for GE0 on specific media speed
 * Inputs     : 
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_88e1112_ge1_phy_tx_rx_test (void)
{
    return (plat_sgmii_lpbk_test(PLAT_GE1_ETHNUM, RSV_SPD_FIELD));
}

/**********************************************************************
 * Function: diag_ge0_smi_read
 * Description: Function to read GE Register through SMI 
 * Inputs     : pointer of type - smi_if_t
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static uint32_t diag_ge0_smi_read(smi_if_t *smi_p) 
{
    return (plat_smi_read(PHY_88E1112_GE0_SMIADDR,  /* GE0 PHY SMI ADDRESS */ 
                          smi_p->offset,            /* target register of 88E1112 */
                          smi_p->buf));             /* read/write buffer pointer */
}

/**********************************************************************
 * Function: diag_ge0_smi_write
 * Description: Function to read GE Register through SMI 
 * Inputs     : pointer of type - smi_if_t
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static uint32_t diag_ge0_smi_write(smi_if_t *smi_p) 
{
    return (plat_smi_write(PHY_88E1112_GE0_SMIADDR,  /* GE0 PHY SMI ADDRESS */ 
                           smi_p->offset,            /* target register of 88E1112 */
                           (ushort)*(smi_p->buf)));  /* read/write buffer pointer */
}

/**********************************************************************
 * Function: diag_ge1_smi_read
 * Description: Function to read GE Register through SMI 
 * Inputs     : pointer of type - smi_if_t
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static uint32_t diag_ge1_smi_read(smi_if_t *smi_p) 
{
    return (plat_smi_read(PHY_88E1112_GE1_SMIADDR,  /* GE1 PHY SMI ADDRESS */ 
                          smi_p->offset,            /* target register of 88E1112 */
                          smi_p->buf));             /* read/write buffer pointer */
}

/**********************************************************************
 * Function: diag_ge1_smi_write
 * Description: Function to read GE Register through SMI 
 * Inputs     : pointer of type - smi_if_t
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static uint32_t diag_ge1_smi_write(smi_if_t *smi_p) 
{
    return (plat_smi_write(PHY_88E1112_GE1_SMIADDR,  /* GE1 PHY SMI ADDRESS */ 
                           smi_p->offset,            /* target register of 88E1112 */
                           (ushort)*(smi_p->buf)));  /* read/write buffer pointer */
}

/**********************************************************************
 * Function: diag_88e1112_smi_open
 * Description: Reserved function
 * Inputs     : pointer of type - smi_if_t
 * Outputs    : PASSED
 **********************************************************************
 */
static uint32_t diag_88e1112_smi_open (smi_if_t *smi_p)
{
    return (PASSED);
}

/**********************************************************************
 * Function: diag_88e1112_smi_close
 * Description: Reserved function
 * Inputs     : pointer of type - smi_if_t
 * Outputs    : PASSED
 **********************************************************************
 */
static uint32_t diag_88e1112_smi_close (smi_if_t *smi_p)
{
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_88e1112_ge_set_txtype
 * Description: Function to set GE Transmitter Type.
 * Inputs     : eth_num - ethernet number
 *              tx_type - transmitter type (0: Class B; 1: Class A)
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_88e1112_ge_set_txtype (int ge_num, ushort tx_type)
{

    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->if_ge_set_tx_type(dev, tx_type);
    if (rc != PASSED) {
        printf("%s:%d:Failed to set GE%d Tx type as Class A\n", 
               __FUNCTION__, __LINE__, ge_num);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
        printf("%s:%d:Device detach failed\n", __FUNCTION__, __LINE__);
        return (rc);
    }

    return (rc);
}

/**********************************************************************
 * Function:    diag_ge_phy_reg_rd_if
 * Description: an interface to read data from GE PHY register
 * Inputs     : ge_num   - GE0/GE1
 *              reg_page - page number in GE PHY
 *              reg_addr - register offset in GE PHY
 *              buf      - read buffer
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_ge_phy_reg_rd_if (int ge_num, smi_t reg_page, uchar reg_addr, smi_t *buf)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* read GE PHY register */
    rc = mrvl_obj->callin_fvt->if_ge_rd_reg(dev,
                                            (smi_t)reg_page,
                                            (uchar)reg_addr,
                                            (smi_t*)buf); 

    if (rc != PASSED) {
        printf("%s:%d:Failed to read GE PHY Page:0x%x Reg:0x%x\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
            printf("%s:%d:Device detach failed\n", __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/**********************************************************************
 * Function:    diag_ge_phy_reg_wr_if
 * Description: an interface to write data to GE PHY register
 * Inputs     : ge_num   - GE0/GE1
 *              reg_page - page number in GE PHY
 *              reg_addr - register offset in GE PHY
 *              w_data   - write buffer
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int diag_ge_phy_reg_wr_if (int ge_num, smi_t reg_page, uchar reg_addr, smi_t *w_data)
{
    dev_mrvl_ge_object_t *mrvl_obj; 
    dev_object_t *dev;
    int rc;

    /* create 88E1112 device */
    mrvl_obj = (dev_mrvl_ge_object_t *)diag_get_88e11112_obj(ge_num); 
    if (mrvl_obj == NULL) {
        printf("%s:%d:Mrvl 88e1112 Null Object\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    dev = (dev_object_t *)mrvl_obj;

    /* run test */
    rc = mrvl_obj->callin_fvt->if_ge_wr_reg(dev,
                                            (smi_t)reg_page,
                                            (uchar)reg_addr,
                                            (smi_t*)w_data); 

    if (rc != PASSED) {
        printf("%s:%d:Failed to write GE PHY Page:0x%x Reg:0x%x Data:0x%x\n", 
               __FUNCTION__, __LINE__, reg_page, reg_addr, *w_data);
        return (rc);
    }

    /* detach device */
    rc = mrvl_obj->base.dev_object_fvt->dev_detach(dev); /* detach dev obj */
    if (rc != PASSED) {
            printf("%s:%d:Device detach failed\n", __FUNCTION__, __LINE__);
            return (rc);
    }
    
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_ge_phy_all_led_off
 * Description: Function to turn all GE LEDs OFF.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_ge_phy_all_led_off (int eth_num)
{
    smi_t reg_page = 0;
    uchar reg_addr = 0;
    smi_t reg_val = 0;

    reg_page = (int)REG_PAGE(3);
    reg_addr = (int)REG_ADDR(16);
    reg_val = GE_LED_OFF;

    if (diag_ge_phy_reg_wr_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to write eth%d, page%d reg%d.\n",
               __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_ge_phy_all_led_on
 * Description: Function to turn all GE LEDs ON.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_ge_phy_all_led_on (int eth_num)
{
    smi_t reg_page = 0;
    uchar reg_addr = 0;
    smi_t reg_val = 0;

    if (eth_num == GE1) {
        reg_page = (int)REG_PAGE(3);
        reg_addr = (int)REG_ADDR(17);
        if (diag_ge_phy_reg_rd_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read eth%d, page%d reg%d.\n",
                   __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
            return (FAILED);
        }
        reg_val &= (uint16_t)(~(REG_BIT(6) | REG_BIT(7)));

        if (diag_ge_phy_reg_wr_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to write eth%d, page%d reg%d.\n",
                   __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
            return (FAILED);
        }
    }

    reg_page = (int)REG_PAGE(3);
    reg_addr = (int)REG_ADDR(16);
    reg_val = GE_LED_ON;

    if (diag_ge_phy_reg_wr_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to write eth%d, page%d reg%d.\n",
               __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_ge_phy_setting_reg
 * Description: Wrapped function to set GE PHY register.
 * Inputs     : eth_num  - ethernet number(eth0, eth1, eth2,...)
 *              reg_page - page number of register
 *              reg_addr - offset of wanted register
 *              set_data - Data that wanted to set to register
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_ge_phy_setting_reg (int eth_num, smi_t reg_page, uchar reg_addr, smi_t set_data)
{
    smi_t reg_val = 0;

    /* First, read current value of wanted register. */
    if (diag_ge_phy_reg_rd_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read GE PHY(%d_%d).\n",
               __FUNCTION__, reg_addr, reg_page);
        return (FAILED);
    }

    /* If value already same as user wants to set, then just return. */
    if ((reg_val & set_data) == set_data) {
        return (PASSED);
    }

    /* Else, set new value to corresponed register. */
    reg_val |= set_data;
    if (diag_ge_phy_reg_wr_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to write GE PHY(%d_%d).\n",
               __FUNCTION__, reg_addr, reg_page);
        return (FAILED);
    }

    reg_val = 0;
    if (diag_ge_phy_reg_rd_if(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read GE PHY(%d_%d) for confirm.\n",
               __FUNCTION__, reg_addr, reg_page);
        return (FAILED);
    }

    if ((reg_val & set_data) != set_data) {
        printf("%s: Failed to set 0x%04X to GE PHY(%d_%d).\n",
               __FUNCTION__, set_data, reg_addr, reg_page);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_ge_phy_config_gephy_fiber
 * Description: Function to default config GE PHY Fiber.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************/
int diag_ge_phy_config_gephy_fiber (void)
{
    smi_t set_data = 0;

    /* Set SIGDET Polarity(88E1112, 16_1.9) to 1 based on platform HW design.*/
    if (diag_ge_phy_setting_reg(ETH0, 
                                (smi_t)REG_PAGE(1), 
                                (uchar)REG_ADDR(16),
                                (smi_t)REG_BIT(9)) != PASSED) {
        printf("%s: Failed to set SIGDET Polarity(16_1.9).\n", __FUNCTION__);
        return (FAILED);
    }

    /* To config Function control Reg(16_3) for LED behavior. */
    /* config value are:
     * 16_3.15:12 = 0111 (On-Fiber Link, Off-Else)
     * 16_3.11:8  = 0001 (On-Link, Blink-Activity, Off-No Link)
     * 16_3.7:4   = 1000 (Force Off)
     * 16_3.3:0   = 0001 (On-Link, Blink-Activity, Off-No Link)
     */
    set_data = LED_MAGIC_NUMBER;
    if (diag_ge_phy_setting_reg(ETH0, 
                                (smi_t)REG_PAGE(3), 
                                (uchar)REG_ADDR(16),
                                (smi_t)set_data) != PASSED) {
        printf("%s: Failed to config. GE PHY LED behavior(16_3).\n",
               __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 * Function:    set_sfp_glc_ge_100fx
 * Description:	Configure GLC-GE-100FX to FX mode. 
 *              Refer to vendor data sheet.
 * Input:	i2c_bus - I2C bus enum.
 * Output:	PASSED/FAILED
 **********************************************************************/
static int diag_set_sfp_glc_ge_100fx (int i2c_bus)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    uint8_t reg_hi, reg_lo;
    char err_buf[ERR_BUF_SIZE * 2];
    
    /* Setup theeset_sfp_glc_ge_100fxI2C struct */
    /* I2C bus number, and device enum */
    i2c_if.i2c_dev = SFP_I2C_DEV_ADDR;
    i2c_if.i2c_bus_type = i2c_bus;

    i2c_if.i2c_speed = N2G_I2C_100KHZ;	/* I2C bus speed */

    /* Open the device */
    if ((rc = n2g_i2c_open(&i2c_if)) != PASSED) {
        printf("%s:%d:Unable to open. rc = %#x\n", 
               __FUNCTION__, __LINE__, rc);
    	return (FAILED);
    }
    
    /* Write FX100 Enable register to set FX mode */
    i2c_if.offset = SFP_GE_100FX_REG; /* offset */
    reg_lo = SFP_GE_100FX_REG_FX_L;   /* Set the FX mode */
    reg_hi = SFP_GE_100FX_REG_FX_H;

    /* Ready to write the high byte */
    i2c_if.size = sizeof(reg_hi);
    i2c_if.buf = (char *)&reg_hi;

    msleep(SFP_I_INIT_TIME);    /* wait for t_init */

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s %s () Unable to write FX Enable Register "
                         "High byte in GLC-GE-100FX SFP.\nrc = %#x",
			 __FILE__, __FUNCTION__, rc);
        rc = FAILED;
    } else {
        /* Ready to write the low byte */
        i2c_if.size = sizeof(reg_lo);
        i2c_if.buf = (char *)&reg_lo;

        rc = n2g_i2c_write(&i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "%s %s() Unable to write FX Enable Register "
                             "Low byte in GLC-GE-100FX SFP.\nrc = %#x",
			        __FILE__, __FUNCTION__, rc);
            rc = FAILED;
        } else {
            /* Write the Edge Control register for the loopback plug */
            i2c_if.offset = SFP_GE_100FX_REG18; /* Offset */
            reg_lo = SFP_GE_100FX_REG_EC_L;     /* New Edge Control */
            reg_hi = SFP_GE_100FX_REG_EC_H;

            /* Ready to write the high byte */
            i2c_if.size = sizeof(reg_hi);
            i2c_if.buf = (char *)&reg_hi;

            msleep(SFP_I_INIT_TIME);	/* wait for t_init */

            rc = n2g_i2c_write(&i2c_if);
            if (rc != PASSED) {
            	sprintf(err_buf, "%s() Unable to write GLC-GE-FX100 "
                                     "Edge Control Register High bytes.\n"
                                     "rc = %#x", __FUNCTION__, rc);
            	rc = FAILED;
            } else {
            	/* Ready to write the low byte */
            	i2c_if.size = sizeof(reg_lo);
            	i2c_if.buf = (char *)&reg_lo;
            	rc = n2g_i2c_write(&i2c_if);
            	if (rc != PASSED) {
                	    sprintf(err_buf, "%s() Unable to write GLC-GE-FX100 "
                                             "Edge Control Register Low byte.\n"
                                             "rc = %#x", __FUNCTION__, rc);
                 	    rc = FAILED;
                    } /* endof if write Edge Control low byte */
            } /* endof if write Edge Control high byte */
	    } /* endof if write FX Enable low byte */
    } /* endof if write FX Enable high bytes */

    return (rc);

}


/**********************************************************************
 * Function:    sfp_I2C_oper
 * Description: This function porting from FinMcMissile.
 *              For configure SFP internal register	
 *              write operation.
 * Input:	reg_high - for register endian
 *              reg_low - for register endian
 *              i2c structure.
 * Output:	PASSED/FAILED
 **********************************************************************/
static int diag_set_sfp_I2C_op (uint8_t reg_high, uint8_t reg_low, n2g_i2c_if_t *i2c_if)
{
    uint32_t rc;
    uint8_t reg_hi, reg_lo;
    char err_buf[ERR_BUF_SIZE * 2];
    
    reg_hi = reg_high;
    reg_lo = reg_low;

    /* Ready to write the high byte */
    i2c_if->size = sizeof(reg_hi);
    i2c_if->buf = (char *)&reg_hi;

    /* Open the device */
    if ((rc = n2g_i2c_open(i2c_if)) != PASSED) {
        printf("%s:%d:Unable to open. rc = %#x\n", 
               __FUNCTION__, __LINE__, rc);
    	return (FAILED);
    }

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s %s () Unable to write GLC-T SFP.\nrc = %#x",
			 __FILE__, __FUNCTION__, rc);
        rc = FAILED;
    } else {
        /* Ready to write the low byte */
        i2c_if->size = sizeof(reg_lo);
        i2c_if->buf = (char *)&reg_lo;

        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "%s %s() Unable to write GLC-T SFP.\nrc = %#x",
			        __FILE__, __FUNCTION__, rc);
            rc = FAILED;
        }
    }
    return (rc);
}

/**********************************************************************
 * Function:    set_sfp_glc_t_1000
 * Description:	Configure SFP Internal Register for SFP GLC-t 1000 
 *              to support loopback test. 
 *              Refer to Avago vendor data sheet 
 *              "Frequently Asked Questins - Question 14".
 * Input:	i2c_bus - I2C bus enum.
 * Output:	PASSED/FAILED
 **********************************************************************/
static int diag_set_sfp_glc_t_1000 (int i2c_bus)
{
    n2g_i2c_if_t i2c_if;

    /* Setup the I2C struct */
    /* I2C bus number, and device enum */
    switch(i2c_if.i2c_bus_type = i2c_bus) {
        case CPU_I2C1:
        case CPU_I2C0:
            /* SFP 1 */
            i2c_if.i2c_dev = SFP_I2C_DEV_ADDR;
            break;
        default:
            /* Invalid SFP */
            printf("%s:%d:Invalid SFP I2C bus %#x\n", __FUNCTION__, __LINE__, i2c_bus);
            return (FAILED);
    } /* endof switch */

    i2c_if.i2c_speed = N2G_I2C_100KHZ;	/* I2C bus speed */

    /* Clear all interrupts */
    i2c_if.offset = SFP_COPPER_INT_REG;
    if (diag_set_sfp_I2C_op(SFP_CLR_INT_H, SFP_CLR_INT_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Force Master mode */
    i2c_if.offset = SFP_COPPER_MA_SL_CR;
    if (diag_set_sfp_I2C_op(SFP_FRC_MASTER_H, SFP_FRC_MASTER_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Apply soft reset and enable auto-negotiation */
    i2c_if.offset = SFP_COPPER_CONTROL;
    if (diag_set_sfp_I2C_op(SFP_RES_EN_AUTO_NEG_H, SFP_RES_EN_AUTO_NEG_L,
                &i2c_if) == FAILED) {
        return (FAILED);
    }

    msleep(SFP_PHY_RESET_DELAY);

    /* Select page 7 of reg 30 */
    i2c_if.offset = 0x1D;
    if (diag_set_sfp_I2C_op(SFP_SEL_P7_REG30_H, SFP_SEL_P7_REG30_L,
                &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Force Gigabit mode */
    i2c_if.offset = 0x1E;
    if (diag_set_sfp_I2C_op(SFP_FRC_GBPS_MODE_H, SFP_FRC_GBPS_MODE_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Select page 16 of reg 30 */
    i2c_if.offset = 0x1D;
    if (diag_set_sfp_I2C_op(SFP_SEL_P16_REG30_H, SFP_SEL_P16_REG30_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Enable Stub loopback */
    i2c_if.offset = 0x1E;
    if (diag_set_sfp_I2C_op(SFP_EN_LBPK_STUB_H, SFP_EN_LBPK_STUB_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Select page 18 of reg 30 */
    i2c_if.offset = 0x1D;
    if (diag_set_sfp_I2C_op(SFP_SEL_P18_REG30_H, SFP_SEL_P18_REG30_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Disable Near End Crosstalk (Next) canceller. */
    i2c_if.offset = 0x1E;
    if (diag_set_sfp_I2C_op(SFP_DIS_NEXT_H, SFP_DIS_NEXT_L, &i2c_if) == FAILED) {
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 * Function: read_sfp_cookie
 * Description: This function is the function to ready 
 *              SFP eeprom contents
 * Inputs     : opt - dummy 
 * Outputs    : PASSED/FAILED
 **********************************************************************/
int diag_read_sfp_cookie (int opt)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = PASSED;
    unsigned char A50[SFP_BUFFER_256]; /* only interested in the first 128 bytes */
    int jx;
    unsigned char index_start = 0;
    unsigned char index_end = 0;
    int kx = 0, ix =0;
    unsigned int bus;
    unsigned char vendor[SFP_EEPROM_16_LENGTH]; /* 16 bytes - address 20 to 35 */
    unsigned char partnumber[SFP_EEPROM_16_LENGTH]; /* 16 bytes - address 40 to 55 */
    unsigned char serial[SFP_EEPROM_16_LENGTH]; /* 16 bytes - address 68 to 83 */
    unsigned char date[SFP_EEPROM_8_LENGTH];/* 8 bytes - address 84 to 91  */
    int cwdm_wave;
    
    memset(&i2c_if, 0, sizeof(i2c_if));
    bus = 1;
    i2c_if.i2c_bus_type = bus;
    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;
    i2c_if.mux = 0; 
    i2c_if.i2c_dev = 0x50; /* 0xA0 > 1 = 0x50 */
    i2c_if.offset = 0;  
    i2c_if.size = 1;  
    memset(A50, 0, sizeof(A50)); 
    i2c_if.buf = (char *)A50;

    /* Check if SFP is available. if yes then continue */
    if (is_sfp_present() != TRUE) {
        printf("SFP module is not detected.");
        return (rc);
    } else {
        printf("SFP module is detected");
    } 

    printf("\n");
    printf ("Dump eeprom contents to: \n") ;
    printf("     0   1   2   3   4   5   6   7   8   9   a   b  "
            " c   d   e   f   0123456789abcdef");
    for (ix = 0; ix < SFP_EEPROM_SIZE; ix++) {
        i2c_if.offset = ix;
        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            /*
             * Unable to read data
            */
            printf("%s: Failed to I2C read (rc = 0x%08x).", __FUNCTION__, rc);
            return (rc);
        }
        A50[ix] = *i2c_if.buf & 0xFF; 
        if ((ix % 0x10) == 0) {
            index_end = ix;
            printf("  ");
            for(jx = index_start; jx <index_end; jx++) {
                if (A50[index_start] == 0x0 ||
                    A50[index_start] == 0xff)
                    printf(".");
                 else
                 if (A50[index_start] < 32 ||
                     A50[index_start] >=127){
                     printf("?");
                 }
                 else
                     printf("%c",A50[index_start]);
                     index_start++;
            }
            index_start = index_end;
            printf("\n%02x:",kx);
	    kx += 0x10;
        } 
        printf("  %02x", A50[ix]);  
    }

    /* print the connector type */
    printf("\nConnector Type = %s",connector[A50[2]]);

    /* print the transceiver typei */
    if (A50[6] & 16) {
	    printf("\nTransceiver is 100Base FX");
    } else if (A50[6] & 8) {
	    printf("\nTransceiver is 1000Base TX");
    } else if (A50[6] & 4) {
        printf("\nTransceiver is 1000Base CX");
    } else if (A50[6] & 2) {
        printf("\nTransceiver is 1000Base LX");
    } else if (A50[6] & 1) {
        printf("\nTransceiver is 1000Base SX");
    } else if (A50[3] & 64) {
        printf("\nTransceiver is 10GBase-ER");
    } else if (A50[3] & 32) {
        printf("\nTransceiver is 10GBase-LRM");
    } else if (A50[3] & 16) {
        printf("\nTransceiver is 10GBase-LR");
    } else if (A50[4] & 12) {
        printf("\nTransceiver is 10GBase-ZR");
    } else if (A50[3] & 8) {
        printf("\nTransceiver is 10GBase-SR");
    } else {
        printf("\nTransceiver is unknown");
    }

    /* 3 bytes.60 high order, 61 low order. Byte 62 is the mantissa */
    /* print sfp wavelength */
    cwdm_wave = ((int) A50[60]<<8) | ((int) A50[61]);
    printf("\nWavelength = %d.%d",cwdm_wave,A50[62]);

    /* print vendor id bytes 20 to 35 */
    memcpy(&vendor, &A50[20],16);
    vendor[16] = '\0';
    printf("\nVendor = %s",vendor);

    /* Print partnumber values address 40 to 55 */ 
    memcpy(&partnumber, &A50[40], 16);
    partnumber[16] = '\0';
    printf("\nPartnumber = %s", partnumber);

    /* Print serial values address 68 to 83 */
    memcpy(&serial, &A50[68], 16);
    serial[16] = '\0';
    printf("\nSerial = %s", serial);

    /* Print date values address 84 to 91 */ 
    memcpy(&date, &A50[84], 8);
    date[8] = '\0';
    printf("\ndate = %s", date);
	
    return (rc);
}

/**********************************************************************
 * Function:    diag_get_sfp_reg
 * Description: This function to get SFP Reg 
 * Inputs     : Offset - i2c offset register 
 * Outputs    : Register value / Failed
 **********************************************************************/
int diag_get_sfp_reg (uint offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int rc = PASSED;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = CPU_I2C1;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    i2c_if.i2c_dev = MB_I2C_ADDR_SFP0;

    i2c_if.offset = offset;
    
    i2c_if.size = 1;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32; 

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (d32[0]);
}

/**********************************************************************
 * Function:    diag_read_sfp_ext_id
 * Description: Read SFP Extended ID.(Check if the SFP isGLC-GE-100FX.
 *              Refer to ENG-107393. 
 * Inputs:      i2c_bus - I2C bus number
 * Outputs:     GLC-GE-100FX or other SFPs Ext. ID
 **********************************************************************/
int diag_read_sfp_ext_id (int i2c_bus)
{
    n2g_i2c_if_t i2c_if;
    unsigned int rc = PASSED;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    i2c_if.i2c_bus_type = i2c_bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    i2c_if.i2c_dev = MB_I2C_ADDR_SFP0;

    i2c_if.offset = SFP_COO_GECC;
    
    i2c_if.size = SFP_COO_GECC_L;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32; 
    
    /* Read Gigabit Ethernet compliance code */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    if (d32[0]) {
        /* Standard GBIC. Not GLC-GE-100FX */
        return (FALSE); 
    }

    /* Read Extended GBIC ID */
    i2c_if.offset = SFP_COO_XID;
    
    i2c_if.size = SFP_COO_XID_L;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32; 

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        printf("%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
    }
    
    return (d32[0]);  
}

/**********************************************************************
 * Function: diag_ge0_chk_intr_assert
 * Description: checking the GE0 interrupt is asserted
 * Inputs     : rsv - reserved field
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_ge0_chk_intr_assert (void)
{
    return (diag_check_ext_intr_pending(PENDING_BIT_GE0));
}

/**********************************************************************
 * Function: diag_ge1_chk_intr_assert
 * Description: checking the GE1 interrupt is asserted
 * Inputs     : rsv - reserved field
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_ge1_chk_intr_assert (void)
{
    return (diag_check_ext_intr_pending(PENDING_BIT_GE1));
}

/**********************************************************************
 * Function: diag_ge0_chk_intr_deassert
 * Description: checking the GE0 interrupt is de-asserted
 * Inputs     : rsv - reserved field
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_ge0_chk_intr_deassert (void)
{
    return (diag_check_ext_intr_no_pending(PENDING_BIT_GE0));
}

/**********************************************************************
 * Function: diag_ge1_chk_intr_deassert
 * Description: checking the GE1 interrupt is de-asserted
 * Inputs     : rsv - reserved field
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
static int diag_ge1_chk_intr_deassert (void)
{
    return (diag_check_ext_intr_no_pending(PENDING_BIT_GE1));
}

/**********************************************************************
 * Function: gephy_set_1000basex_mode
 * Description: Function to set GE PHY to 1000Base-X mode.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int gephy_set_1000basex_mode(void)
{
    int      eth_num = 0;
    uint     cpu_reg_addr = 0, cpu_reg_val = 0;

    eth_num = PLAT_GE1_ETHNUM;
    cpu_reg_addr = (uint)CPU_PORT_MAC_CTRL_REG(3);

    /* Set CPU MAC 1000BASE-X Mode */
    if (plat_mem_read32(cpu_reg_addr, &cpu_reg_val) != PASSED) {
        cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n",
         __FUNCTION__, cpu_reg_addr);
        return (FAILED);
    }
    cpu_reg_val |= CPU_MAC_1000BASEX_MODE_REG; 
    if (plat_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X",
                          GE1, cpu_reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 * Function: gephy_set_loopback_mode
 * Description: Function to set loopback mode.
 * Inputs     : enable - enable/disable loopback
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int gephy_set_loopback_mode(int enable)
{
    uint cpu_phy0_test_control_reg_addr = 0;
    uint cpu_digital_loopback_enable_reg_addr = 0, cpu_digital_loopback_enable_reg_val = 0;

    cpu_phy0_test_control_reg_addr = (uint)CPU_PHY_0_CONTORL_REG(1);
    cpu_digital_loopback_enable_reg_addr = (uint)CPU_DIGITAL_LOOPBACK_ENABLE_REG(1);

    /* Set loopback mode */
    if (enable) {
        if (plat_mem_write32(cpu_phy0_test_control_reg_addr, CPU_PHY_0_CTRL_TEST_ENABLE_1) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", GE1, cpu_phy0_test_control_reg_addr);
            return (FAILED);
        }
		
		if (plat_mem_read32(cpu_digital_loopback_enable_reg_addr, &cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n", __FUNCTION__, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
		
		cpu_digital_loopback_enable_reg_val |= CPU_LOCAL_DIG_RX2TX_LPBK_EN;
		
		if (plat_mem_write32(cpu_digital_loopback_enable_reg_addr, cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", GE1, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
		
		if (plat_mem_write32(cpu_phy0_test_control_reg_addr, CPU_PHY_0_CTRL_TEST_ENABLE_2) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", GE1, cpu_phy0_test_control_reg_addr);
            return (FAILED);
        }
    } else {
		if (plat_mem_write32(cpu_phy0_test_control_reg_addr, CPU_PHY_0_CTRL_TEST_ENABLE_0) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", GE1, cpu_phy0_test_control_reg_addr);
            return (FAILED);
        }
		
		if (plat_mem_read32(cpu_digital_loopback_enable_reg_addr, &cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n", __FUNCTION__, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
		
		cpu_digital_loopback_enable_reg_val &= ~CPU_LOCAL_DIG_RX2TX_LPBK_EN;
		
		if (plat_mem_write32(cpu_digital_loopback_enable_reg_addr, cpu_digital_loopback_enable_reg_val) != PASSED) {
            cterr('f', 0, "Failed to config CPU GE%d reg. 0x%08X", GE1, cpu_digital_loopback_enable_reg_addr);
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 * Function: gephy_get_loopback_mode
 * Description: Function to get loopback mode.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 **********************************************************************
 */
int gephy_get_loopback_mode(void)
{
    uint cpu_digital_loopback_enable_reg_addr = 0, cpu_digital_loopback_enable_reg_val = 0;

    cpu_digital_loopback_enable_reg_addr = (uint)CPU_DIGITAL_LOOPBACK_ENABLE_REG(1);

    /* Get loopback mode */
    if (plat_mem_read32(cpu_digital_loopback_enable_reg_addr, &cpu_digital_loopback_enable_reg_val) != PASSED) {
      cterr('f', 0, "%s: Failed to read CPU register 0x%08X.\n",
         __FUNCTION__, cpu_digital_loopback_enable_reg_addr);
        return (FAILED);
    }
    
    if (cpu_digital_loopback_enable_reg_val & CPU_LOCAL_DIG_RX2TX_LPBK_EN) {
        return (FAILED);
    }
    
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_ge_phy_lib.c,v $
 * Revision 1.3  2019/06/24 07:21:37  wilbhuan
 * Supported Pluggable Serial Module.
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

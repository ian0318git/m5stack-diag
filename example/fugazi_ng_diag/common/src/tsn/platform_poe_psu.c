/* $Id: platform_poe_psu.c,v 1.3 2017/10/19 13:41:11 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_poe_psu.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : platform_poe_psu.c
 * Description: TSN PoE PSU(TI, TPS2386) Library.
 *
 * Copyright (c) 2016 ~ 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "common_utils.h"
#include "nvmonvars.h"
#include "i2c_address.h"
#include "i2c_api.h"
#include "proto.h"
#include "platform_fpga.h"
#include "platform_i2c.h"
#include "plat_defs.h"
#include "platform_poe_psu.h"
#include "tsn_comm.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int        plat_psu_utils(int);
int        tsn_psu_reg_rd(uint32_t, char *);
int        tsn_psu_reg_wr(uint32_t, uchar);
static int psu_reg_rd_util(int);
static int psu_reg_wr_util(int);
static int psu_regs_dump(int);
static int pse_p_status_util(int);
static int pse_show_p_status(int);
static int pse_pwr_detect(int);
static int pse_reset_all_ports(void);
static int pse_set_all_ports_semiauto(void);
static int pse_all_ports_det_cla_en(void);
static int pse_set_port_poe_plus(int, int, uchar);
static int pse_enable_port_ieee_pwr(int, int);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
/*
 * PoE PSU Utilities
 */
static submenu_xtable_t psu_utils_tbl[] = {
    {"PoE PSE port power detect",   (type_t(*)())pse_pwr_detect,     0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"PoE PSU register Read",       (type_t(*)())psu_reg_rd_util,    0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"PoE PSU register Write",      (type_t(*)())psu_reg_wr_util,    0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"Dump PoE PSU all registers",  (type_t(*)())psu_regs_dump,      0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
    {"Show PoE PSU status info",    (type_t(*)())pse_p_status_util,  0,
     0,                             (type_t(*)())0,                  0,
     (type_t(*)())0,                0},
};

#define PSU_UTILS_TBL_SIZE (sizeof(psu_utils_tbl) / sizeof(submenu_xtable_t))

/* PoE PSU Utilities items (filled in from xtable) */
static mitem_t psu_utils_pri_items[PSU_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t psu_utils_sec_items[PSU_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* PoE PSU Utils submenu */
menuinfo_t psu_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    psu_utils_pri_items,
};
menuinfo_t *psu_utils_menup = &psu_utils_menu;

/* TSN PSE(TPS2386B) Register Map */
static pse_cmd_info_t pse_reg_tbl[] = {
    /* Format: name, cmd_code, type, data_byte. */
    {"Interrupt",            TPS2386B_INTR_REG,           READ_ONLY,  ONE_B},
    {"Interrupt Mask",       TPS2386B_INTR_MASK_REG,      READ_WRITE, ONE_B},
    {"Power Event",          TPS2386B_PWREVENT_REG,       READ_ONLY,  ONE_B},
    {"Detect Event",         TPS2386B_DET_REG,            READ_ONLY,  ONE_B},
    {"Fault Event",          TPS2386B_FAULTEVENT_REG,     READ_ONLY,  ONE_B},
    {"Start Event",          TPS2386B_STARTEVENT_REG,     READ_ONLY,  ONE_B},
    {"Supply Event",         TPS2386B_SUPPLYEVENT_REG,    READ_ONLY,  ONE_B},
    {"Port1 Status",         TPS2386B_P1_STATUS_REG,      READ_ONLY,  ONE_B},
    {"Port2 Status",         TPS2386B_P2_STATUS_REG,      READ_ONLY,  ONE_B},
    {"Port3 Status",         TPS2386B_P3_STATUS_REG,      READ_ONLY,  ONE_B},
    {"Port4 Status",         TPS2386B_P4_STATUS_REG,      READ_ONLY,  ONE_B},
    {"Power Status",         TPS2386B_PWR_STAT_REG,       READ_ONLY,  ONE_B},
    {"I2C Slave Addr.",      TPS2386B_I2C_SLVADDR_REG,    READ_ONLY,  ONE_B},
    {"Operating mode",       TPS2386B_OP_MODE_REG,        READ_WRITE, ONE_B},
    {"Disconnect Enable",    TPS2386B_DISCONN_EN_REG,     READ_WRITE, ONE_B},
    {"Detect/Class Enable",  TPS2386B_DETCLA_EN_REG,      READ_WRITE, ONE_B},
    {"PwrRR/ICUT Disable",   TPS2386B_PWRRR_ICUT_DIS_REG, READ_WRITE, ONE_B},
    {"Timing Config",        TPS2386B_TIMING_CONF_REG,    READ_WRITE, ONE_B},
    {"General Mask",         TPS2386B_GENERAL_MASK_REG,   READ_WRITE, ONE_B},
    {"Detect/Class Restart", TPS2386B_DETCLA_RESTART_REG, WRITE_ONLY, ONE_B},
    {"Power Enable",         TPS2386B_PWR_EN_REG,         WRITE_ONLY, ONE_B},
    {"RESET",                TPS2386B_RESET_REG,          WRITE_ONLY, ONE_B},
    {"ID",                   TPS2386B_ID_REG,             READ_WRITE, ONE_B},
    {"Police 21 Config",     TPS2386B_POLICE21_CONF_REG,  READ_WRITE, ONE_B},
    {"Police 43 Config",     TPS2386B_POLICE43_CONF_REG,  READ_WRITE, ONE_B},
    {"IEEE Power Enable",    TPS2386B_IEEE_PWR_EN_REG,    WRITE_ONLY, ONE_B},
    {"Power-on Fault",       TPS2386B_IEEE_PWRFAULT_REG,  READ_ONLY,  ONE_B},
    {"Temperature",          TPS2386B_TEMP_REG,           READ_ONLY,  ONE_B},
    {"Input Voltage",        TPS2386B_IN_VOLT_REG,        READ_ONLY,  TWO_B},
    {"Port 1 Current",       TPS2386B_P1_CURR_REG,        READ_ONLY,  TWO_B},
    {"Port 1 Voltage",       TPS2386B_P1_VOLT_REG,        READ_ONLY,  TWO_B},
    {"Port 2 Current",       TPS2386B_P2_CURR_REG,        READ_ONLY,  TWO_B},
    {"Port 2 Voltage",       TPS2386B_P2_VOLT_REG,        READ_ONLY,  TWO_B},
    {"Port 3 Current",       TPS2386B_P3_CURR_REG,        READ_ONLY,  TWO_B},
    {"Port 3 Voltage",       TPS2386B_P3_VOLT_REG,        READ_ONLY,  TWO_B},
    {"Port 4 Current",       TPS2386B_P4_CURR_REG,        READ_ONLY,  TWO_B},
    {"Port 4 Voltage",       TPS2386B_P4_VOLT_REG,        READ_ONLY,  TWO_B},
    {"PoE Plus",             TPS2386B_POEPLUS_REG,        READ_WRITE, ONE_B},
    {"Firmware Revision",    TPS2386B_FW_REV_REG,         READ_ONLY,  ONE_B},
    {"I2C Watchdog",         TPS2386B_I2C_WD_REG,         READ_WRITE, ONE_B},
    {"Device ID",            TPS2386B_DEV_ID_REG,         READ_WRITE, ONE_B},
    {"Cool Down",            TPS2386B_COOL_DOWN_REG,      READ_WRITE, ONE_B},
};


/*******************************************************************************
 *
 * Function    : plat_psu_utils
 * Description : Function to show TSN PoE PSU utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_psu_utils (int opt)
{
    build_primary_submenu(psu_utils_tbl, PSU_UTILS_TBL_SIZE,
                          "PoE PSU Utilities", &psu_utils_menup);
    build_secondary_submenu(psu_utils_tbl, PSU_UTILS_TBL_SIZE,
                            psu_utils_sec_items);

    menu(psu_utils_menup, psu_utils_sec_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : psu_reg_rd_util
 * Description : Utility to read TSN PoE controller register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_reg_rd_util (int opt)
{
    uint32_t reg_offset = 0;
    uchar    reg_val = 0;
    
    reg_offset = (uint32_t)gethex_answer("Enter Reg. address(0 ~ 0x45) ",
                                         TPS2386B_ID_REG, 0, 0x45);

    if (tsn_psu_reg_rd(reg_offset, (char *)&reg_val) != PASSED) {
        return (FAILED);
    }
    printf("PoE controller reg.(0x%02X) = 0x%02X\n", reg_offset, reg_val);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : psu_reg_wr_util
 * Description : Utility to write TSN PoE controller register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_reg_wr_util (int opt)
{
    uint32_t reg_offset = 0;
    uchar    orig_val = 0, wr_val = 0;
    
    reg_offset = (uint32_t)gethex_answer("Enter Reg. address(0 ~ 0x45) ",
                                         TPS2386B_PWR_EN_REG, 0, 0x45);

    if (tsn_psu_reg_rd(reg_offset, (char *)&orig_val) != PASSED) {
        return (FAILED);
    }

    wr_val = (uchar)gethex_answer("Enter write-in data(hex) ",
                                  orig_val, 0, 0xff);

    if (tsn_psu_reg_wr(reg_offset, wr_val) != PASSED) {
        return (FAILED);
    }
    printf("Done writing 0x%02X to PoE reg.(0x%02X).\n", wr_val, reg_offset);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_psu_reg_rd
 * Description : Function to read TSN PoE PSU register.
 * Inputs      : reg_off - register offset
 *               *rd_buf - read buffer
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_psu_reg_rd (uint32_t reg_off, char *rd_buf)
{
    n2g_i2c_if_t *i2c_if;
    int          ret_val = 0;

    /* Init i2c_if for PoE controller */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_POE_CONTR);
    if (i2c_if == NULL) {
        printf("%s: Failed to get PoE controller I2C info.\n", __FUNCTION__);
        return (FAILED);
    }
    i2c_if->offset = reg_off;
    i2c_if->buf = rd_buf;

    ret_val = n2g_i2c_read(i2c_if);
    if (ret_val != PASSED) {
        printf("%s: Failed to read PoE controller Reg. %#x.(ret_code = %#x)",
               __FUNCTION__, reg_off, ret_val);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_psu_reg_wr
 * Description : Function to write TSN PoE PSU register.
 * Inputs      : reg_off - register offset
 *               wr_data - write data
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_psu_reg_wr (uint32_t reg_off, uchar wr_data)
{
    n2g_i2c_if_t *i2c_if;
    int           ret_val = 0;

    /* Init i2c_if for PoE controller */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO,
                                            MB_I2C2_POE_CONTR);
    if (i2c_if == NULL) {
        printf("%s: Failed to get PoE controller I2C info.\n", __FUNCTION__);
        return (FAILED);
    }
    i2c_if->offset = reg_off;
    i2c_if->buf = (char *)&wr_data;

    ret_val = n2g_i2c_write(i2c_if);
    if (ret_val != PASSED) {
        printf("%s: Failed to write PoE controller Reg. %#x.(ret_code = %#x)",
               __FUNCTION__, reg_off, ret_val);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : psu_regs_dump
 * Description : Utility to dump TSN PoE PSU all registers.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_regs_dump (int opt)
{
    uchar          rd_val = 0;
    uint16_t       reg_val = 0;
    uint32_t       reg_addr = 0;
    pse_cmd_info_t *reg_p = 0;
    int            ctr = 0, total_reg_num = 0;
    int            r_ctr = 0;

    reg_p = &pse_reg_tbl[0];
    total_reg_num = (sizeof(pse_reg_tbl) / sizeof(pse_cmd_info_t));

    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        reg_val = 0;
        reg_addr = reg_p->cmd_code;
        for (r_ctr = 0; r_ctr < reg_p->data_byte; r_ctr++, reg_addr++) {
            rd_val = 0;
            if (tsn_psu_reg_rd(reg_addr, (char *)&rd_val) != PASSED) {
                printf("%s: Failed to read PoE PSE %s reg.(0x%02X).\n",
                       __FUNCTION__, reg_p->name, reg_addr);
                return (FAILED);
            }
            reg_val |= (uint16_t)(rd_val << (r_ctr * 8));
        }

        if (reg_p->data_byte == ONE_B) {
            printf("%-20s reg(0x%02X): 0x%02X\n",
                   reg_p->name, reg_p->cmd_code, (uchar)reg_val);
        } else {
            printf("%-20s reg(0x%02X): 0x%04X\n",
                   reg_p->name, reg_p->cmd_code, reg_val);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : pse_p_status_util
 * Description : Utility to show TSN PoE PSU current status.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_p_status_util (int opt)
{
    /* Get and show PSE port status */
    if (pse_show_p_status(PSE_ALL_PORTS) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_reset_all_ports
 * Description:	Function to reset PoE PSE all ports.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_reset_all_ports (void)
{
    uint32_t reg_addr = 0;
    uchar    config_val = 0, reg_val = 0;
    int      t_ctr = 0, p_ctr = 0;
    int      ret_val = FAILED;

    /* Reset PSE port */
    config_val = (uchar)PSE_RESET_ALL_PORTS;
    reg_addr = (uint32_t)TPS2386B_RESET_REG;

    if (tsn_psu_reg_wr(reg_addr, config_val) != PASSED) {
        printf("%s: Failed to set PSE reg.0x%02X reset port%d bit.\n",
               __func__, reg_addr, p_ctr);
        return (FAILED);
    }

    /* Based on datasheet, it takes at least 5ms
     * from Reset to Start condition. So wait 10ms here.
     */
    msleep(TSN_PSE_RST_WAIT_TIME);

    for (p_ctr = PSE_PORT_ONE; p_ctr <= PSE_PORT_FOUR; p_ctr++) {
        /* Polling PSE port status register to confirm port resetl */
        reg_addr = (uint32_t)PSE_PSTAT_REG(p_ctr);
        ret_val = FAILED;

        for (t_ctr = 0; t_ctr < PSE_CHKTIME_MAX; t_ctr += PSE_CHK_INTVAL) {
            reg_val = 0;

            if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
                printf("%s: Failed to read PSE port%d status reg.0x%02X.\n",
                       __func__, p_ctr, reg_addr);
                return (FAILED);
            }

            if (reg_val == (uchar)PSE_PSTAT_RST_VALUE) {
                ret_val = PASSED;
                break;
            }
            msleep(PSE_CHK_INTVAL);
        }

        if (ret_val != PASSED) {
            printf("%s: Failed to RESET PSE port%d.\n", __func__, p_ctr);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_set_all_ports_semiauto
 * Description:	Function to set Semi-auto mode to PoE PSE all ports.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_set_all_ports_semiauto (void)
{
    uint32_t reg_addr = (uint32_t)TPS2386B_OP_MODE_REG;
    uchar curr_msk = (uchar)PSE_ALL_PORTS_SEMIAUTO;
    uchar reg_val = 0;
    int   t_ctr = 0;
    int   ret_val = FAILED;

    /* Read out current PSE port operating mode. */
    if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
        printf("%s: Failed to read PSE reg.0x%02X.\n", __func__, reg_addr);
        return (FAILED);
    }

    /* Set PSE port to Semi-auto if needed. */
    if ((reg_val & curr_msk) != curr_msk) {
        reg_val = curr_msk;

        if (tsn_psu_reg_wr(reg_addr, reg_val) != PASSED) {
            printf("%s: Failed to write 0x%02X to PSE reg.0x%02X.\n",
                   __func__, reg_val, reg_addr);
            return (FAILED);
        }

        /* Polling to confirm all ports are set to semiauto mode correctly. */
        for (t_ctr = 0; t_ctr < PSE_CHKTIME_MAX; t_ctr += PSE_CHK_INTVAL) {
            reg_val = 0;
            ret_val = FAILED;

            /* Read back PSE port operating setup for confirmation. */
            if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
                printf("%s: Failed to read PSE reg.0x%02X for confirmation.\n",
                       __func__, reg_addr);
                return (FAILED);
            }

            /* Check PSE port is set to Semi-auto. */
            if ((reg_val & curr_msk) == curr_msk) {
                ret_val = PASSED;
                break;
            }

            msleep(PSE_CHK_INTVAL);
        }

        if (ret_val != PASSED) {
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_all_ports_det_cla_en
 * Description:	Function to enable PSE all ports detection and classification.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_all_ports_det_cla_en (void)
{
    uint32_t reg_addr = (uint32_t)TPS2386B_DETCLA_EN_REG;
    uchar curr_msk = (uchar)PSE_ALL_PORTS_DETCLA;
    uchar reg_val = 0;
    int   t_ctr = 0;
    int   ret_val = FAILED;

    /* Read PSE port detection and classification register */
    if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
        printf("%s(%d): Failed to read PSE Detect/Class enable reg.(0x%02X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    /* Enable PSE port detection if needed. */
    if ((reg_val & curr_msk) != curr_msk) {
        reg_val = curr_msk;

        if (tsn_psu_reg_wr(reg_addr, reg_val) != PASSED) {
            printf("%s: Failed to write 0x%02X to PSE "
                   "Detect/Class enable reg.(0x%02X).\n",
                   __func__, reg_val, reg_addr);
            return (FAILED);
        }

        for (t_ctr = 0; t_ctr < PSE_CHKTIME_MAX; t_ctr += PSE_CHK_INTVAL) {
            reg_val = 0;
            ret_val = FAILED;

            /* Read back PSE port operating setup for confirmation. */
            if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
                printf("%s(%d): Failed to read PSE Detect/Class enable"
                       " reg.(0x%02X).\n",
                       __func__, __LINE__, reg_addr);
                return (FAILED);
            }

            /* Check PSE port detection and classification is enabled. */
            if ((reg_val & curr_msk) == curr_msk) {
                ret_val = PASSED;
                break;
            }
            msleep(PSE_CHK_INTVAL);
        }

        if (ret_val != PASSED) {
            printf("%s: Failed to enable PSE all ports Detect and Classify.\n",
                   __func__);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_set_port_poe_plus
 * Description:	Function to config PSE port PoE Plus.
 * Inputs     : p_num - port number
 *              pwr_type - port power type
 *              icut_val - Icut current setup
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_set_port_poe_plus (int p_num, int pwr_type, uchar icut_val)
{
    uint32_t reg_addr = 0;
    uchar reg_val = 0;

    /* Read out current PSE PoE Plus setups. */
    reg_addr = (uint32_t)TPS2386B_POEPLUS_REG;
    if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
        printf("%s(%d): Failed to read PSE PoE Plus reg.(0x%02X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    if (pwr_type == PSE_PWR_IEEE_TP1) {
        reg_val &= (uchar)(~(PSE_PORT_POEPLUS(p_num)));
    } else if (pwr_type == PSE_PWR_IEEE_TP2) {
        reg_val |= (uchar)(PSE_PORT_POEPLUS(p_num));
    } else {
        printf("Unsupported power type(%#x).\n", pwr_type);
        return (FAILED);
    }
    reg_val |= (uchar)PSE_TPON;

    if (tsn_psu_reg_wr(reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%02X to PSE PoE Plus reg.(0x%02X).\n",
               __func__, reg_val, reg_addr);
        return (FAILED);
    }

    /* Configure Icut (0x1Eh/0x1Fh) */
    reg_val = 0;

    if (icut_val > PSE_ICUT_MAX) {
        printf("%s: Icut value is out of range.(%#x)\n", __func__, icut_val);
        return (FAILED);
    }

    if (p_num <= (int)PSE_PORT_TWO) {
        reg_addr = (uint32_t)TPS2386B_POLICE21_CONF_REG;
    } else {
        reg_addr = (uint32_t)TPS2386B_POLICE43_CONF_REG;
    }

    if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
        printf("%s(%d): Failed to read PSE reg.0x%02X.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    if ((p_num % 2) == 0) {
        reg_val &= (uchar)(~PSE_POL_ODD_MSK);
        reg_val |= (uchar)(icut_val & PSE_POL_ODD_MSK);
    } else {
        reg_val &= (uchar)(~PSE_POL_EVEN_MSK);
        reg_val |= (uchar)((icut_val << PSE_POL_EVEN_SHIFT) & PSE_POL_EVEN_MSK);
    }

    /* Set PSE port Icut if needed. */
    if (tsn_psu_reg_wr(reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%02X to PSE reg.0x%02X.\n",
               __func__, reg_val, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_enable_port_ieee_pwr
 * Description:	Function to enable PSE port IEEE power.
 * Inputs     : p_num - port number
 *              pwr_type - port power type
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_enable_port_ieee_pwr (int p_num, int pwr_type)
{
    uint32_t reg_addr = 0; 
    uchar reg_val = 0;
    uchar val_msk = 0;
    struct timeval f_start, f_end;
    double timer = 0;
    int ret_val = FAILED;

    /* Based on datasheet, IEEE Power Enable register(0x23h) is write-only. */
    reg_addr = (uint32_t)TPS2386B_IEEE_PWR_EN_REG;

    if (pwr_type == PSE_PWR_IEEE_TP1) {
        reg_val = (uchar)(PSE_IEEE_TP1_EN(p_num));
    } else if (pwr_type == PSE_PWR_IEEE_TP2) {
        reg_val = (uchar)(PSE_IEEE_TP2_EN(p_num));
    } else {
        printf("Unsupported power type(%#x).\n", pwr_type);
        return (FAILED);
    }
    if (tsn_psu_reg_wr(reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%02X to PSE reg.0x%02X.\n",
               __func__, reg_val, reg_addr);
        return (FAILED);
    }

    /* Check if port power is enabled successfully. */
    reg_addr = (uint32_t)TPS2386B_PWR_STAT_REG;
    val_msk = (uchar)(PSE_PWRSTAT_PORT_PGPE(p_num));

    do {
        gettimeofday(&f_start, NULL);
        reg_val = 0;

        if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
            printf("%s: Failed to read PSE port%d status reg.(0x%02X).\n",
                   __func__, p_num, reg_addr);
            return (FAILED);
        }

        if ((reg_val & val_msk) == val_msk) {
            ret_val = PASSED;
            break;
        }
        msleep(PSE_I2C_CHK_INTERVAL);

        gettimeofday(&f_end, NULL);
        timer += (double)(f_end.tv_usec - f_start.tv_usec);
    } while (timer <= PSE_PWRON_MAX_MICROSEC);

    if (ret_val != PASSED) {
        printf("Failed to enable port%d power.\n", p_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_enable_port_pwr
 * Description:	Function to enable PSE port power.
 * Inputs     : p_num - port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_enable_port_pwr (int p_num)
{
    uint32_t reg_addr = 0;
    uchar reg_val = 0, curr_msk = 0, p_det = 0, p_class = 0;
    int ret_val = FAILED;
    struct timeval f_start, f_end;
    double timer = 0;

    /* 1. Confirm current detection and classification are valid. */
    reg_addr = (uint32_t)PSE_PSTAT_REG(p_num);
    curr_msk = (uchar)PSE_PSTAT_DET_MSK;

    /* 1.1 Read PSE port status register(P1:0x0C ~ P4:0x0F) */
    do {
        gettimeofday(&f_start, NULL);
        reg_val = 0;

        if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
            printf("%s: Failed to read PSE port%d status reg.(0x%02X).\n",
                   __func__, p_num, reg_addr);
            return (FAILED);
        }

        /* 1.2 Confirm port detection is valid. */
        p_det = (reg_val & curr_msk);
        if (p_det == (uchar)PSE_PSTAT_DET_VALID) {
            /* 1.3 Get class */
            p_class = (uchar)((reg_val & (uchar)PSE_PSTAT_CLA_MSK)
                               >> PSE_PSTAT_CLA_SHIFT);

            if ((p_class == PSE_PSTAT_CLA0) ||
                ((p_class >= PSE_PSTAT_CLA1) && (p_class <= PSE_PSTAT_CLA4))) {
                ret_val = PASSED;
                break;
            }
        }

        if ((p_det == (uchar)PSE_PSTAT_DET_TOO_LOW) ||
            (p_det == (uchar)PSE_PSTAT_DET_TOO_HIGH) ||
            (p_det == (uchar)PSE_PSTAT_DET_OPEN_CIRC) ||
            (p_det == (uchar)PSE_PSTAT_DET_MOS_FAULT)) {
            ret_val = FAILED;
            break;
        }

        msleep(PSE_I2C_CHK_INTERVAL);

        gettimeofday(&f_end, NULL);
        timer += (double)(f_end.tv_usec - f_start.tv_usec);
    } while (timer <= PSE_DETCLA_MAX_MICROSEC);

    if (ret_val != PASSED) {
        printf("Port%d: Failed to get valid Detect/Class\n", p_num);
        return (FAILED);
    }

    /* 2. Enable port power based on detected class */
    switch (p_class) {
    case PSE_PSTAT_UNKNOWN_CLA:
        printf("Port%d: Detected, Class: Unknown(%#x).\n", p_num, p_class);
        ret_val = FAILED;
        break;
    case PSE_PSTAT_CLA1:
    case PSE_PSTAT_CLA2:
    case PSE_PSTAT_CLA3:
        printf("Port%d: Detected, Class: Class %d.\n", p_num, p_class);

        /* Set PSE port PoE Plus mode */
        if (pse_set_port_poe_plus(p_num, PSE_PWR_IEEE_TP1,
                                  PSE_ICUT_320MA) != PASSED) {
            return (FAILED);
        }

        /* Enable port IEEE Type1 power(0x23h) */
        if (pse_enable_port_ieee_pwr(p_num, PSE_PWR_IEEE_TP1) != PASSED) {
            return (FAILED);
        }
        ret_val = PASSED;
        break;
    case PSE_PSTAT_CLA4:
        printf("Port%d: Detected, Class: Class 4.\n", p_num);

        /* Set PSE port PoE Plus mode */
        if (pse_set_port_poe_plus(p_num, PSE_PWR_IEEE_TP2,
                                  PSE_ICUT_640MA) != PASSED) {
            return (FAILED);
        }

        /* Enable port IEEE Type2 power(0x23h) */
        if (pse_enable_port_ieee_pwr(p_num, PSE_PWR_IEEE_TP2) != PASSED) {
            return (FAILED);
        }
        ret_val = PASSED;
        break;
    case PSE_PSTAT_CLA0:
        printf("Port%d: Detected, Class: Class 0.\n", p_num);

        /* Set PSE port PoE Plus mode */
        if (pse_set_port_poe_plus(p_num, PSE_PWR_IEEE_TP1,
                                  PSE_ICUT_320MA) != PASSED) {
            return (FAILED);
        }

        /* Enable port IEEE Type1 power(0x23h) */
        if (pse_enable_port_ieee_pwr(p_num, PSE_PWR_IEEE_TP1) != PASSED) {
            return (FAILED);
        }
        ret_val = PASSED;
        break;
    case PSE_PSTAT_CLA_OC:
        printf("Port%d: Detected, Class: Over current.\n", p_num);
        ret_val = FAILED;
        break;
    default:
        printf("Port%d: Detected, Class: Unknown(%#x).\n", p_num, p_class);
        ret_val = FAILED;
        break;
    }

    if (ret_val != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : pse_pwr_detect
 * Description:	Function performs PoE PSU port power detection test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_pwr_detect (int opt)
{
    int      p_ctr = 0, end_port = (int)PSE_PORT_FOUR;

    testname("PoE PSE Power Detect");

    /* For TSN: TSN-H supports 4-ports PoE; and TSN-M supports 2-ports PoE. */
    if (this_is_tsn_h_sku() == TRUE) {
        end_port = (int)PSE_PORT_FOUR;
    } else {
        end_port = (int)PSE_PORT_TWO;
    }

    /* Reset PSE all ports(0x1Ah) */
    prpass(testpass, "Reset PSE all ports");

    if (pse_reset_all_ports() != PASSED) {
        printf("%s: Failed to reset PSE all ports.\n", __func__);
        return (FAILED);
    }

    /* Config PSE all ports to Semi-auto mode(0x12h) */
    prpass(testpass, "Configure PSE all ports to Semi-Auto");

    if (pse_set_all_ports_semiauto() != PASSED) {
        printf("%s: Failed to set PSE all ports to Semi-auto.\n", __func__);
        return (FAILED);
    }

    /* Enable port detection and classification(0x14h) */
    prpass(testpass, "Enable PSE all ports Detect and Classify");

    if (pse_all_ports_det_cla_en() != PASSED) {
        printf("%s: Failed to do PSE all ports detect/classify.\n", __func__);
        return (FAILED);
    }

    /* Enable port power */
    prpass(testpass, "Enable PSE port power\n");
    for (p_ctr = (int)PSE_PORT_ONE; p_ctr <= end_port; p_ctr++) { 
        if (pse_enable_port_pwr(p_ctr) != PASSED) {
            printf("%s: Failed to enable PSE port%d power.\n",
                   __func__, p_ctr);
            continue;
        }
    }

    /* Get and show PSE port status */
    prpass(testpass, "Show PSE all ports status");

    if (pse_show_p_status(PSE_ALL_PORTS) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : pse_show_p_status
 * Description : Function to show PoE PSE port status.
 * Inputs      : port_num - number of PSE port
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int pse_show_p_status (int port_num)
{
    int           p_ctr = 1, s_port = 1, e_port = 1;
    int           ctr = 0, sub_ctr = 0;
    uint32_t      reg_addr = 0;
    uchar         reg_val = 0;
    pse_pstat_t   p_stat;
    uint16_t      curr_reg = 0, volt_reg = 0;
    double        c_pwr = 0, c_curr = 0, c_volt = 0;

    if ((port_num < (int)PSE_PORT_ONE) || (port_num > (int)PSE_ALL_PORTS)) {
        printf("%s: Failed ! Wanted port num(%d) is out of range.\n",
               __FUNCTION__, port_num);
        return (FAILED);
    } else if (port_num == (int)PSE_ALL_PORTS) {
        s_port = (int)PSE_PORT_ONE;

        if (this_is_tsn_h_sku() == TRUE) {
            e_port = (int)TSN_H_POE_PORTS;
        } else {
            e_port = (int)TSN_M_POE_PORTS;
        }
    } else {
        s_port = port_num;
        e_port = port_num;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: s_port = %d; and e_port = %d.\n",
               __FUNCTION__, s_port, e_port);
    }

    printf("\n\n\n");
    printf("Port   Mode       ON/OFF Detect/Class  Power(W)"
               "  Volt(V)   Current(mA)\n");
    for (ctr = 0; ctr < 70; ctr++) {
        printf("=");
    }
    printf("\n");

    /* Get port status */
    for (p_ctr = s_port; p_ctr <= e_port; p_ctr++) {
        p_stat.p_num = p_ctr;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: p_stat.p_num = %d.\n", __FUNCTION__, p_stat.p_num);
        }

        /* 1 Get port operation mode(0x12h) */
        reg_val = 0;
        reg_addr = (uint32_t)TPS2386B_OP_MODE_REG;
        if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
            printf("%s: Failed to read PoE PSE Power Status reg.(%#x).\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }
        p_stat.p_opmod = ((uchar)(reg_val >> ((p_ctr - 1) * 2)) &
                          (uchar)PSE_OPMODE_MSK);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: p_stat.p_opmod = %d.\n", __FUNCTION__, p_stat.p_opmod);
        }

        /* 2 Get port status(0x0Ch ~ 0x0Fh) */
        reg_val = 0;
        reg_addr = (uint32_t)(TPS2386B_PSTAT_REG_BASE + (p_ctr - 1));
        if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
            printf("%s: Failed to read PoE PSE Power Status reg.(%#x).\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }
        p_stat.p_det = (reg_val & PSE_DETECT_MSK);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: p_stat.p_det = %d.\n", __FUNCTION__, p_stat.p_det);
        }

        p_stat.p_class = ((reg_val >> PSE_CLASS_SHIFT) & PSE_CLASS_MSK);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: p_stat.p_class = %d.\n", __FUNCTION__, p_stat.p_class);
        }

        /* 3 Get port power status(0x10h) */
        reg_val = 0;
        reg_addr = (uint32_t)TPS2386B_PWR_STAT_REG;
        if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
            printf("%s: Failed to read PoE PSE Power Status reg.(%#x).\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }
        p_stat.p_onoff = ((reg_val >> (p_ctr - 1)) & PSE_PORT_ONOFF_MSK);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: p_stat.p_onoff = %d.\n", __FUNCTION__, p_stat.p_onoff);
        }

        p_stat.p_pwr_stat = ((reg_val >> ((p_ctr - 1) + PSE_PWR_STAT_SHIFT)) &
                             PSE_PWR_STAT_MSK);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: p_stat.p_pwr_stat = %d.\n",
                   __FUNCTION__, p_stat.p_pwr_stat);
        }

        /* Check if PD starts to pull power. */
        reg_addr = (uint32_t)(PSE_CURR_REG_BASE(p_ctr));

        for (ctr = 0; ctr < PSE_PWR_POLL_MAX; ctr++) {
            printf("\rDetecting power...");
            fflush(stdout);

            reg_val = 0;
            if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
                printf("%s: Failed to read PoE PSE port%d current reg.(%#x).\n",
                       __func__, p_ctr, reg_addr);
                return (FAILED);
            }

            if (reg_val != 0) {
                /* PD starts polling power, wait PD to get expected power.
                 * Confirmed with PSE vendor, no way for PSE to know PD
                 * polls enough power.
                 * So here wait 5 seconds from PD starts to poll.
                 */
                for (sub_ctr = 0; sub_ctr < WAIT_PD_READY_MAX; sub_ctr++) {
                    printf(".");
                    fflush(stdout);
                    msleep(WAIT_PD_READY_INTVAL);
                }
                break;
            }
            msleep(PSE_I2C_CHK_INTERVAL);
        }

        /* Get current, voltage and power if port is ON */
        if (p_stat.p_onoff == 1) {
            /* Current */
            for (ctr = 0; ctr < 2; ctr++) {
                reg_val = 0;
                reg_addr = (uint32_t)(PSE_P_CURR_REG_BASE + ((p_ctr - 1) * 4) + ctr);
                if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
                    printf("%s: Failed to read PoE PSE port%d current reg.(%#x).\n",
                           __FUNCTION__, p_ctr, reg_addr);
                    return (FAILED);
                }
                curr_reg |= (uint16_t)(reg_val << (ctr * 8));
            }
            c_curr = (double)((curr_reg * PSE_CURR_BASE) / 1000);
            p_stat.p_curr = c_curr;
            curr_reg = 0;

            /* Voltage */
            for (ctr = 0; ctr < 2; ctr++) {
                reg_val = 0;
                reg_addr = (uint32_t)(PSE_P_VOLT_REG_BASE + ((p_ctr - 1) * 4) + ctr);
                if (tsn_psu_reg_rd(reg_addr, (char *)&reg_val) != PASSED) {
                    printf("%s: Failed to read PoE PSE port%d voltage reg.(%#x).\n",
                           __FUNCTION__, p_ctr, reg_addr);
                    return (FAILED);
                }
                volt_reg |= (uint16_t)(reg_val << (ctr * 8));
            }
            c_volt = (double)((volt_reg * PSE_VOLT_BASE) / 1000);
            p_stat.p_volt = c_volt;
            volt_reg = 0;

            c_pwr = (double)((c_volt * c_curr) / 1000);
            p_stat.p_pwr = c_pwr;
        }

        /* Show port status */
        printf("\rPort%d  ", p_stat.p_num);

        if (p_stat.p_opmod == 0) {
            printf("OFF        ");
        } else if (p_stat.p_opmod == 1) {
            printf("Manual     ");
        } else {
            printf("Semi-Auto  ");
        }

        if (p_stat.p_onoff == 1) {
            printf("ON   ");
        } else {
            printf("OFF  ");
        }

        if (p_stat.p_det == 0) {
            printf("Unknown         ");
        } else if (p_stat.p_det == 1) {
            printf("Short-circuit   ");
        } else if (p_stat.p_det == 2) {
            printf("Reserved        ");
        } else if (p_stat.p_det == 3) {
            printf("Too Low         ");
        } else if (p_stat.p_det == 4) {
            if (p_stat.p_class == 0) {
                printf("Unknown         ");
            } else if (p_stat.p_class == 1) {
                printf("Class 1         ");
            } else if (p_stat.p_class == 2) {
                printf("Class 2         ");
            } else if (p_stat.p_class == 3) {
                printf("Class 3         ");
            } else if (p_stat.p_class == 4) {
                printf("Class 4         ");
            } else if (p_stat.p_class == 5) {
                printf("Reserved        ");
            } else if (p_stat.p_class == 6) {
                printf("Class 0         ");
            } else if (p_stat.p_class == 7) {
                printf("OverCurrent     ");
            } else {
                printf("Undefined(0x%02X) ", p_stat.p_class);
            }
        } else if (p_stat.p_det == 5) {
            printf("Too High        ");
        } else if (p_stat.p_det == 6) {
            printf("Open-circuit    ");
        } else if (p_stat.p_det == 7) {
            printf("Reserved        ");
        } else if (p_stat.p_det == 14) {
            printf("MOSFET fault    ");
        } else {
            printf("Undefined(0x%02X) ", p_stat.p_det);
        } 

        if (p_stat.p_onoff == 1) {
            printf("%.2lf(W)   %.2lf(V)   %.2lf(mA)",
                   p_stat.p_pwr, p_stat.p_volt, p_stat.p_curr);
        } else {
            printf("---");
        }
        printf("\n");
    }
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: platform_poe_psu.c,v $
 * Revision 1.3  2017/10/19 13:41:11  palin2
 * Fixed CSCvg23616: TSN PoE link down intermittently when connect to iPorter PoE tester.
 *
 * Revision 1.2  2017/08/02 14:21:49  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/24 14:14:11  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.7.2.1  2017/05/17 02:19:42  palin2
 * Updated function of triggering PoE PSE side interrupt.
 *
 * Revision 1.1.4.7  2016/10/03 00:38:49  palin2
 * Added utility to dump PSE registers.
 *
 * Revision 1.1.4.6  2016/08/01 12:43:51  palin2
 * Fixed potential issue of PoE PSE display port(s) voltage and current.
 *
 * Revision 1.1.4.5  2016/07/22 13:02:41  palin2
 * Extended waiting PSE access time to 1 sec.
 *
 * Revision 1.1.4.4  2016/07/17 11:15:16  palin2
 * Added function to distinguish bwteen TSN-H and TSN-M.
 *
 * Revision 1.1.4.3  2016/06/30 14:06:32  steja
 * Pick up the latest from tsn-branch1
 *
 * Revision 1.1.4.2  2016/06/30 06:22:51  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.4  2016/06/29 14:14:51  palin2
 * 1. Updated code to support TSN-M.
 * 2. Added utility to set LAN PHY 1000Base-T Test mode.
 *
 * Revision 1.1.2.3  2016/06/16 07:51:11  palin2
 * Updated PoE related utilities and code.
 *
 * Revision 1.1.2.2  2016/05/10 06:17:33  palin2
 * Updated PoE PSE related diag code after bring up.
 *
 * Revision 1.1.2.1  2016/03/22 22:19:12  palin2
 * Added PoE PSU Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */


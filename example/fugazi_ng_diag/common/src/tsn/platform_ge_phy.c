/* $Id: platform_ge_phy.c,v 1.7 2018/11/23 08:49:52 hondwang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_ge_phy.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_ge_phy.c
 * Description: TSN GE PHY(Marvell 88E1112) Library.
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "dev_mrvl_ge.h"
#include "types.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "ethernet.h"
#include "queryflags.h"
#include "platform_ext_lpbk.h"
#include "platform_ge_phy.h"
#include "diag_ge_phy.h"
#include "platform_fpga.h"
#include "platform_smi.h"
#include "platform_i2c.h"
#include "proto.h"
#include "platform_sfp_cookie.h"
#include "byteswap.h"
#include "i2c_address.h"
#include "i2c_api.h"


/*******************************************************************************
 *                                 Externs                                      
 *******************************************************************************
 */
int tsn_gephy_reg_rd(int, int, int, ushort *);
int tsn_gephy_reg_wr(int, int, int, ushort);
int tsn_set_gephy_reg(int, int, int, ushort);
int tsn_config_gephy_fiber(void);
int tsn_gephy_reg_wr_util(int);
int tsn_gephy_reg_rd_util(int);
int gephy_vod_adj_util(int);
extern uint sfp_type;

/*******************************************************************************
 *                                  Global                                      
 *******************************************************************************
 */
#define TSN_GEPHY_CONFIG_TIME   1000
#define SEC_TO_MICROSEC         1000000.0
#define MAX_CHECKTIME_USEC      1000000   /* 1sec */
#define MAX_POLLING_COUNTS      100
#define POLLING_INTRVL          100 /* 100ms */
#define MAX_TRY                 5


/*******************************************************************************
 *                                  Function                                      
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function   : tsn_all_ge_leds_off
 * Description: Function to turn TSN all GE LEDs OFF.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_all_ge_leds_off (int eth_num)
{
    int      reg_page = 0, reg_addr = 0;
    uint16_t reg_val = 0;

    reg_page = (int)REG_PAGE(3);
    reg_addr = (int)REG_ADDR(16);
    reg_val = 0x9898;

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to write eth%d, page%d reg%d.\n",
               __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_all_ge_leds_on
 * Description: Function to turn TSN all GE LEDs ON.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_all_ge_leds_on (int eth_num)
{
    int      reg_page = 0, reg_addr = 0;
    uint16_t reg_val = 0;

    if (eth_num == TSN_GE1) {
        reg_page = (int)REG_PAGE(3);
        reg_addr = (int)REG_ADDR(17);
        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read eth%d, page%d reg%d.\n",
                   __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
            return (FAILED);
        }
        reg_val &= (uint16_t)(~(REG_BIT(6) | REG_BIT(7)));

        if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to write eth%d, page%d reg%d.\n",
                   __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
            return (FAILED);
        }
    }

    reg_page = (int)REG_PAGE(3);
    reg_addr = (int)REG_ADDR(16);
    reg_val = 0x8989;

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to write eth%d, page%d reg%d.\n",
               __FUNCTION__, __LINE__, eth_num, reg_page, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_reg_rd_util
 * Description: Utility to read TSN GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_reg_rd_util (int eth_num)
{
    int      reg_page = 0, reg_addr = 0;
    uint16_t reg_val = 0;

    reg_page = getdec_answer("Enter page offset(0 ~ 255): ", 0, 0, 255);
    reg_addr = getdec_answer("Enter register offset(0 ~ 31): ", 0, 0, 31);

    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("eth%d page%d reg%d: 0x%04X.\n",
               eth_num, reg_page, reg_addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_reg_wr_util
 * Description: Utility to write TSN GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_reg_wr_util (int eth_num)
{
    int      reg_page = 0, reg_addr = 0;
    uint16_t reg_val = 0, w_data = 0;

    reg_page = getdec_answer("Enter page offset(0 ~ 255): ", 0, 0, 255);
    reg_addr = getdec_answer("Enter register offset(0 ~ 31): ", 0, 0, 31);

    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }

    w_data = gethex_answer("Enter write in data(0x0 ~ 0xfffff): ",
                           reg_val, 0, 0xffff);

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, w_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%04X to eth%d page%d reg%d.\n",
               w_data, eth_num, reg_page, reg_addr);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_config_gephy_fiber
 * Description: Function to default config TSN GE PHY Fiber.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_config_gephy_fiber (void)
{
    ushort set_data = 0;

    /* Set SIGDET Polarity(88E1112, 16_1.9) to 1 based on TSN HW design.*/
    if (tsn_set_gephy_reg(ETH0, (int)REG_PAGE(1), (int)REG_ADDR(16),
                          (ushort)REG_BIT(9)) != PASSED) {
        printf("%s: Failed to set SIGDET Polarity(16_1.9).\n", __FUNCTION__);
        return (FAILED);
    }

    /* To config Function control Reg(16_3) for LED behavior. */
    /* For TSN, config value is:
     * 16_3.15:12 = 0111 (On-Fiber Link, Off-Else)
     * 16_3.11:8  = 0001 (On-Link, Blink-Activity, Off-No Link)
     * 16_3.7:4   = 1000 (Force Off)... TSN P0 board
     * 16_3.3:0   = 0001 (On-Link, Blink-Activity, Off-No Link)
     */
    set_data = LED_MAGIC_NUMBER;
    if (tsn_set_gephy_reg(ETH0, (int)REG_PAGE(3), (int)REG_ADDR(16),
                          set_data) != PASSED) {
        printf("%s: Failed to config. GE PHY LED behavior(16_3).\n",
               __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_set_gephy_reg
 * Description: Wrapped function to set TSN GE PHY(Marvell 88E1112) register.
 * Inputs     : eth_num  - ethernet number(eth0, eth1, eth2,...)
 *              reg_page - page number of register
 *              reg_addr - offset of wanted register
 *              set_data - Data that wanted to set to register
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_set_gephy_reg (int eth_num, int reg_page, int reg_addr, ushort set_data)
{
    ushort reg_val = 0;

    /* First, read current value of wanted register. */
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
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
    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write GE PHY(%d_%d).\n",
               __FUNCTION__, reg_addr, reg_page);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
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
 *
 * Function   : tsn_gephy_reg_rd
 * Description: Function to read TSN GE PHY(Marvell 88E1112) register.
 *              Star at GE0 will link with pluggabel I2C
 * Inputs     : eth_num  - ethernet number(eth0, eth1, eth2,...)
 *              reg_page - page number of register
 *              reg_addr - offset of wanted register
 *              *buf     - buffer to put read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_reg_rd (int eth_num, int reg_page, int reg_addr, ushort *buf)
{
    ushort reg_val = 0;
    int    page_reg = (int)REG_PAGE(22);
    int    phy_smiaddr = 0;
    int    ctr = 0, polling_result = FAILED;

    if (eth_num == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (eth_num == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, eth_num);
        return (FAILED);
    }

    /* Read out current value of page address register(0x16) */
    if (tsn_smi_read(phy_smiaddr, page_reg, &reg_val) != PASSED) {
        printf("%s(%d): Failed to SMI read eth%d reg.0x%08X.\n",
               __func__, __LINE__, eth_num, page_reg);
        return (FAILED);
    }

    if (reg_val != (ushort)reg_page) {
        for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
            /* Set page to user requested */
            if (tsn_smi_write(phy_smiaddr, page_reg,
                              (ushort)reg_page) != PASSED) {
                printf("%s(%d): Failed to SMI write eth%d reg.0x%08X.\n",
                       __func__, __LINE__, eth_num, page_reg);
                return (FAILED);
            }

            reg_val = 0;
            if (tsn_smi_read(phy_smiaddr, page_reg, &reg_val) != PASSED) {
                printf("%s(%d): Failed to SMI read eth%d reg.0x%08X.\n",
                       __func__, __LINE__, eth_num, page_reg);
                return (FAILED);
            }

            if (reg_val == (ushort)reg_page) {
                polling_result = PASSED;
                break;
            }
        }

        if (polling_result != PASSED) {
            printf("%s(%d): Failed to set eth%d to page 0x%02X(%d).\n",
                   __func__, __LINE__, eth_num, reg_page, reg_page);
            return (FAILED);
        }
    }

    /* Read Data for user */
    if (tsn_smi_read(phy_smiaddr, reg_addr, buf) != PASSED) {
        printf("%s(%d): Failed to SMI read eth%d reg.0x%08X.\n",
               __func__, __LINE__, eth_num, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_reg_wr
 * Description: Function to write TSN GE PHY(Marvell 88E1112) register.
 *              Star GE0 will link with pluggable I2C
 * Inputs     : eth_num  - ethernet number(eth0, eth1, eth2,...)
 *              reg_page - page number of register
 *              reg_addr - offset of wanted register
 *              wr_data  - new value thatwanted to write into register
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_reg_wr (int eth_num, int reg_page, int reg_addr, ushort w_data)
{
    ushort reg_val = 0;
    int    page_reg = (int)REG_PAGE(22);
    int    phy_smiaddr = 0;

    if (eth_num == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (eth_num == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, eth_num);
        return (FAILED);
    }

    /* Read out current value of page address register(0x16) */
    if (tsn_smi_read(phy_smiaddr, page_reg, &reg_val) != PASSED) {
        printf("%s(%d): Failed to SMI read eth%d reg.0x%08X.\n",
               __func__, __LINE__, eth_num, page_reg);
        return (FAILED);
    }

    if (reg_val != (ushort)reg_page) {
        /* Set page to user requested */
        if (tsn_smi_write(phy_smiaddr, page_reg, (ushort)reg_page) != PASSED) {
            printf("%s(%d): Failed to SMI write eth%d reg.0x%08X.\n",
                   __func__, __LINE__, eth_num, page_reg);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_smi_read(phy_smiaddr, page_reg, &reg_val) != PASSED) {
            printf("%s(%d): Failed to SMI read eth%d reg.0x%08X.\n",
                   __func__, __LINE__, eth_num, page_reg);
            return (FAILED);
        }

        if (reg_val != (ushort)reg_page) {
            printf("%s(%d): Failed to set eth%d to page 0x%02X(%d).\n",
                   __func__, __LINE__, eth_num, reg_page, reg_page);
            return (FAILED);
        }
    }

    if (tsn_smi_write(phy_smiaddr, reg_addr, w_data) != PASSED) {
        printf("%s(%d): Failed to SMI write eth%d reg.0x%08X.\n",
               __func__, __LINE__, eth_num, reg_addr);
        return (FAILED);
    }
    msleep(100);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_config_w_rst
 * Description: Function to config TSN GE PHY(Marvell 88E1112) with PHY reset.
 * Inputs     : eth_num - ethernet number
 *              r_page  - page number of register
 *              r_addr  - offset of wanted register
 *              w_data  - buffer to put value that will be written in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_config_w_rst (int eth_num, int r_page, int r_addr, ushort w_data)
{
    ushort reg_val = 0;
    int    t_ctr_ms = 0;

    w_data |= (ushort)GEPHY_REG_RESET;
    if (tsn_gephy_reg_wr(eth_num, r_page, r_addr, w_data) != PASSED) {
        printf("%s:%d Failed to write eth%d PHY reg. %d_%d.\n",
               __FUNCTION__, __LINE__, eth_num, r_addr, r_page);
        return (FAILED);
    }

    do {
        reg_val = (ushort)GEPHY_REG_RESET;
        if (tsn_gephy_reg_rd(eth_num, r_page, r_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read eth%d PHY reg. %d_%d.\n",
                   __FUNCTION__, __LINE__, eth_num, r_addr, r_page);
            return (FAILED);
        }

        if ((reg_val & (ushort)GEPHY_REG_RESET) == 0) {
            break;
        }
        t_ctr_ms += 10;
    } while(t_ctr_ms <= TSN_GEPHY_CONFIG_TIME);

    if (t_ctr_ms > TSN_GEPHY_CONFIG_TIME) {
        return (FAILED);
    }
    msleep(100);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : gephy_vod_adj_util
 * Description: Utility to do GE PHY VOD Adjustments.
 * Inputs     : ge_num - GE port number(for TSN: GE0/GE1)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int gephy_vod_adj_util (int ge_num)
{
    ushort vod_adj = VOD_ADJ_DEFAULT;
    int    reg_page = (int)PHY_PAGE(0);
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    int    eth_num = 0;

    if (ge_num == TSN_GE0) {
        eth_num = TSN_GE0_ETHNUM;
    } else if (ge_num == TSN_GE1) {
        eth_num = TSN_GE1_ETHNUM;
    } else {
        printf("%s(%d): TSN doesn't have GE%d.", __func__, __LINE__, ge_num);
        return (FAILED);
    }

    /* Based on Marvell 88E1112 release note, MV-S300751.
     * For gigabit and 100Mbps mode, the MDI VOD output may be adjusted by
     * 1. Set reg.29 = 0x0004
     * 2. Then bit[2:0] of reg.30 represents
     *    "000" = +8%
     *    "001" = +6%
     *    "010" = +4%
     *    "011" = +2%
     *    "100" = default
     *    "101" = -2%
     *    "110" = -4%
     *    "111" = -6%
     *
     * Note: These reigster settings have no effect for 10Mbps mode.
     */

    /* 1. Set reg.29 to 0x0004. */
    reg_addr = (int)GEPHY_MFG_R29_REG;
    wr_data = (ushort)GEPHY_MFG_R29_VOD;
    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to set GE%d(eth%d) PHY reg. %d_%d to 0x%04X.\n",
               __func__, __LINE__, ge_num, eth_num, reg_addr, reg_page, wr_data);
        return (FAILED);
    }
    
    /* 2. Get current VOD Adjust value from reg.30 bit[2:0] */
    reg_addr = (int)GEPHY_VOD_ADJ_REG;
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read GE%d(eth%d) PHY reg. %d_%d.\n",
               __func__, __LINE__, ge_num, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    vod_adj = (ushort)(reg_val & (ushort)GEPHY_VOD_ADJ_MASK);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): Power on GE%d(eth%d) vod_adj = %#x.\n",
               __func__, __LINE__, ge_num, eth_num, vod_adj);
    }

    printf("\nVOD Adjustments -\n");
    printf("    0: +8%%\n");
    printf("    1: +6%%\n");
    printf("    2: +4%%\n");
    printf("    3: +2%%\n");
    printf("    4: default\n");
    printf("    5: -2%%\n");
    printf("    6: -4%%\n");
    printf("    7: -6%%\n");
    vod_adj = (ushort)gethex_answer("Enter VOD adjust value you want: ",
                                    vod_adj, VOD_ADJ_PLUS8, VOD_ADJ_MINUS6);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): User input vod_adj = %#x.\n",
               __func__, __LINE__, vod_adj);
    }

    /* 3. Adjust VOD based on user request. */
    wr_data = (ushort)((reg_val & (ushort)(~GEPHY_VOD_ADJ_MASK)) |
                       (vod_adj & (ushort)GEPHY_VOD_ADJ_MASK));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): wr_data = %#x.\n", __func__, __LINE__, wr_data);
    }

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to set GE%d(eth%d) PHY reg. %d_%d to 0x%04X.\n",
               __func__, __LINE__, ge_num, eth_num, reg_addr, reg_page, wr_data);
        return (FAILED);
    }

    printf("Done adjust GE%d(eth%d) PHY VOD.\n", ge_num, eth_num);    

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_get_media_mode
 * Description: Function to get TSN GE PHY(Marvell 88E1112) MAC media mode.
 * Inputs     : eth_num - ethernet number
 *              *buf - buffer to put get back media mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_get_media_mode (int eth_num, uint *buf)
{
    int      reg_page = (int)MRVL1112_MSCR1_REGPAGE;
    int      reg_addr = (int)MRVL1112_MSCR1_REGADDR;
    uint16_t reg_val = 0;

    /* Get current mode by reading PHY register 16_2 */
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get eth%d PHY media mode by read reg.%d_%d.\n",
               __func__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    *buf = (uint)((reg_val & (uint16_t)MSCR1_MOD_MSK) >> MSCR1_MOD_OFFSET);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: current media mode of GEWAN PHY(eth%d) is %d.\n",
               __func__, eth_num, *buf);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_set_media_mode
 * Description: Function to set TSN GE PHY(Marvell 88E1112) MAC media mode.
 * Inputs     : eth_num - ethernet number
 *              sel_mod - selected MAC mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_set_media_mode (int eth_num, uint sel_mod)
{
    struct timeval t_start, t_end;
    double   t_exec = 0;
    int      chk_result = FAILED;
    int      reg_page = (int)MRVL1112_MSCR1_REGPAGE;
    int      reg_addr = (int)MRVL1112_MSCR1_REGADDR;
    uint16_t reg_val = 0, exp_mode = 0;
    uint16_t mode_msk = (uint16_t)(MSCR1_MOD_MSK << MSCR1_MOD_OFFSET);

    if (sel_mod > (uint)MSCR1_MOD_MSK) {
        printf("%s: Unsupported media mode: %d.\n", __func__, sel_mod);
        return (FAILED);
    }

    exp_mode = (uint16_t)((sel_mod & MSCR1_MOD_MSK) << MSCR1_MOD_OFFSET);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d) sel_mod = %d, exp_mode = 0x%04X.\n",
               __func__, __LINE__, sel_mod, exp_mode);
    }

    /* Check if current mode is needed to reconfigure. */
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read eth%d register %d_%d.\n",
               __func__, __LINE__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    if ((reg_val & mode_msk) == exp_mode) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(%d) Current mode(%d) is same as expected(%d).\n",
                   __func__, __LINE__,
                   ((reg_val & mode_msk) >> MSCR1_MOD_OFFSET), sel_mod);
        }
        return (PASSED);
    }

    reg_val &= (uint16_t)(~mode_msk);
    reg_val |= exp_mode;

    /* Set the wanted mode */
    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write eth%d register %d_%d.\n",
               __func__, __LINE__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    /* Confirm the mode is set correctly */
    /* Record function start time */
    gettimeofday(&t_start, NULL);

    do {
        reg_val = 0;

        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read eth%d register %d_%d.\n",
                   __func__, __LINE__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }

        /* Record function end time and count current execution time */
        gettimeofday(&t_end, NULL);
        t_exec = (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                          (t_end.tv_usec - t_start.tv_usec));

        if ((reg_val & mode_msk) == exp_mode) {
            chk_result = PASSED;

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("[DBG]%s(%d) execute time: %f seconds.\n",
                       __func__, __LINE__, (float)(t_exec / SEC_TO_MICROSEC));
            }

            break;
        }
    } while (t_exec <= MAX_CHECKTIME_USEC); /* Polling duration: 1sec. */

    if (chk_result != PASSED) {
        printf("%s: Failed to set mode to %d.\n", __func__, sel_mod);
        return (FAILED);
    }

    msleep(100);
    /* Config set speed for GLC-GE-100FX to 100MBPS */
    if (sfp_type == SFP_GE_100FX) {
        reg_page = (int)MRVL1112_MSCR1_REGPAGE;
        reg_addr = (int)MRVL1112_MAC_CTRL_REG;
        reg_val = 0;
        /* Check if current mode is needed to reconfigure. */
        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read eth%d register %d_%d.\n",
                   __func__, __LINE__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }
        
        reg_val &= (int)~MCR_SPD_MASK;
        reg_val |= (int)MCR_SPD_100Mbps; 
        
        if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
            printf("%s(%d) Failed to write eth%d register %d_%d.\n",
                   __func__, __LINE__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }
    } /* End of Config set speed for GLC-GE-100FX */
    
    /* Based on Marvell 88E1112, need a soft-reset to make 1112 update
     * the selected MAC mode.
     */
    reg_page = (int)MRVL1112_CCR_REGPAGE;
    reg_addr = (int)MRVL1112_CCR_REGADDR;
    reg_val = 0;
    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read eth%d PHY reg. %d_%d.\n",
               __func__, __LINE__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    reg_val |= (uint16_t)COP_CTRL_RESET; 
    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write eth%d register %d_%d.\n",
               __func__, __LINE__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    t_exec = 0;
    chk_result = FAILED;
 
    /* Record function start time */
    gettimeofday(&t_start, NULL);

    do {
        reg_val = (uint16_t)COP_CTRL_RESET;

        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read eth%d register %d_%d.\n",
                   __func__, __LINE__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }

        /* Record function end time and count current execution time */
        gettimeofday(&t_end, NULL);
        t_exec = (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                          (t_end.tv_usec - t_start.tv_usec));

        if ((reg_val & (uint16_t)COP_CTRL_RESET) == 0) {
            chk_result = PASSED;

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("[DBG]%s(%d) execute time: %f seconds.\n",
                       __func__, __LINE__, (float)(t_exec / SEC_TO_MICROSEC));
            }
            break;
        }
    } while (t_exec <= MAX_CHECKTIME_USEC); /* Polling duration: 1sec. */

    if (chk_result != PASSED) {
        printf("%s: Failed to soft-reset eth%d.\n", __func__, eth_num);
        return (FAILED);
    }

    msleep(100);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_check_linkstat
 * Description: Function to confirm GEWAN PHY Copper/Fiber Link state.
 *              By confirm Copper(17_0.10) / Fiber(17_1.10) is up(1) / down(0).
 * Inputs     : eth_num - ethernet number(eth0/1/2)
 *              intf_opt - Copper(0) / Fiber(1)
 *              link_opt - to confirm link up(1) / down(0)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_check_linkstat (int eth_num, boolean intf_opt, boolean link_opt)
{
    uint16_t reg_val = 0, chk_val = CSSR1_COP_LINKUP;
    int reg_page = MRVL1112_CSSR1_REGPAGE;
    int reg_addr = MRVL1112_CSSR1_REGADDR;
    int ctr = 0;
    int polling_result = FAILED;

    if (intf_opt == GEPHY_FIB) {
        reg_page = MRVL1112_FSSR1_REGPAGE;
    }

    if (link_opt == GEPHY_LINKDOWN) {
        chk_val = 0;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Check GEWAN PHY(eth%d) reg.%d_%d, link_opt = %d.\n",
               __func__, eth_num, reg_addr, reg_page, link_opt);
    }

    for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
        reg_val = 0;
        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read GEWAN PHY reg. %d_%d.\n",
                   __func__, reg_addr, reg_page);
            return (FAILED);
        }

        if ((reg_val & CSSR1_COP_LINKUP) == chk_val) {
            polling_result = PASSED;
            break;
        }

        msleep(POLLING_INTRVL);
    }

    if (polling_result != PASSED) {
        printf("%s: TIMEMOUT! But PHY Copper link is still %s.\n",
               __func__, ((link_opt == GEPHY_LINKUP) ? "down" : "up"));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: GEWAN PHY(eth%d) Copper link is %s.\n",
               __func__, eth_num, ((link_opt == GEPHY_LINKUP) ? "up" : "down"));
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_force_linkgood
 * Description: Function to config. GEWAN PHY Copper/Fiber force Link good.
 *              By confirm Copper(16_0.10) / Fiber(16_1.10) is up(1) / down(0).
 * Inputs     : eth_num - ethernet number(eth0/1/2)
 *              intf_opt - Copper(0) / Fiber(1)
 *              link_opt - to confirm link up(1) / down(0)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_force_linkgood (int eth_num, boolean intf_opt, boolean link_opt)
{
    int reg_page = MRVL1112_CSCR1_REGPAGE;
    int reg_addr = MRVL1112_CSCR1_REGADDR;
    uint16_t reg_val = 0, chk_val = CSCR1_FORCE_COP_LINK;
    uint16_t msk_val = CSCR1_FORCE_COP_LINK;
    int ctr = 0;
    int polling_result = FAILED;

    if (intf_opt == GEPHY_FIB) {
        reg_page = MRVL1112_FSCR1_REGPAGE;
    }

    if (link_opt == GEPHY_LINKDOWN) {
        chk_val = 0;
    }

    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read eth%d PHY reg. %d_%d.\n",
               __func__, __LINE__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    if ((reg_val & msk_val) != chk_val) { 
        if (link_opt == GEPHY_LINKUP) {
            reg_val |= CSCR1_FORCE_COP_LINK;
        } else {
            reg_val &= (uint16_t)(~CSCR1_FORCE_COP_LINK);
        }

        if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
            printf("%s: Failed to write eth%d PHY reg. %d_%d.\n",
                   __func__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }

        msleep(100);

        for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
            reg_val = 0;
            if (tsn_gephy_reg_rd(eth_num, reg_page,
                                 reg_addr, &reg_val) != PASSED) {
                printf("%s(%d): Failed to read eth%d PHY reg. %d_%d.\n",
                       __func__, __LINE__, eth_num, reg_addr, reg_page);
                return (FAILED);
            }

            if ((reg_val & msk_val) == chk_val) {
                polling_result = PASSED;
                break;
            }
            msleep(POLLING_INTRVL);
        }

        if (polling_result != PASSED) {
            printf("%s: Failed to %s GEWAN PHY(eth%d) %s force Link Good.\n",
                   __func__, ((link_opt == GEPHY_LINKUP) ? "enable" : "disable"),
                   eth_num, ((intf_opt == GEPHY_FIB) ? "fiber" : "copper"));
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_set_macspeed
 * Description: Function to config. GEWAN PHY MAC speed,
 *              this is by configure MAC control register(0_2).
 * Inputs     : eth_num - ethernet number(eth0/1/2)
 *              conf_val - configure value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_set_macspeed (int eth_num, ushort conf_val)
{
    int reg_page = MRVL1112_MCR_REGPAGE; 
    int reg_addr = MRVL1112_MCR_REGADDR;
    ushort reg_val = 0;
    int ctr = 0, polling_result = FAILED;

    reg_val = (conf_val | GEPHY_REG_RESET);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: write-in data = 0x%04X.\n", __func__, reg_val);
    }

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write eth%d PHY reg. %d_%d.\n",
               __func__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    sleep(ETH_DRIVER_DELAY);

    for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
        reg_val = 0;
        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read eth%d PHY reg. %d_%d.\n",
                   __func__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }

        if (reg_val == conf_val) {
            polling_result = PASSED;
            break;
        }
        msleep(POLLING_INTRVL);
    }

    if (polling_result != PASSED) {
        printf("%s: Failed to configure eth%d PHY MAC speed to 0x%04X.\n",
               __func__, eth_num, conf_val);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_set_macloopback
 * Description: Function to enable/disable GEWAN PHY MAC loopback
 *              by configure Copper(0_0.14) / Fiber(0_1.14) Contorl reg.
 * Inputs     : eth_num - ethernet number(eth0/1/2)
 *              intf_opt - Copper(0) / Fiber(1)
 *              onoff - enable(1) / disable(0) loopback
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_set_macloopback (int eth_num, boolean intf_opt, boolean onoff)
{
    int reg_page = MRVL1112_CCR_REGPAGE;
    int reg_addr = MRVL1112_CCR_REGADDR;
    uint16_t reg_val = 0, chk_val = COP_CTRL_LPBK;
    uint16_t msk_val = COP_CTRL_LPBK;
    int ctr = 0;
    int polling_result = FAILED;

    if (intf_opt == GEPHY_FIB) {
        reg_page = MRVL1112_FCR_REGPAGE;
    }

    if (onoff == GEPHY_DIS) {
        chk_val = 0;
    }

    if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read eth%d PHY reg. %d_%d.\n",
               __func__, __LINE__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    if ((reg_val & msk_val) == chk_val) {
        return (PASSED);
    }

    if (onoff == GEPHY_EN) {
        reg_val |= COP_CTRL_LPBK;
    } else {
        reg_val &= (uint16_t)(~COP_CTRL_LPBK);
    }

    if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write eth%d PHY reg. %d_%d.\n",
               __func__, eth_num, reg_addr, reg_page);
        return (FAILED);
    }

    msleep(100);

    for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
        reg_val = 0;
        if (tsn_gephy_reg_rd(eth_num, reg_page, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read eth%d PHY reg. %d_%d.\n",
                   __func__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }

        if ((reg_val & msk_val) == chk_val) {
            polling_result = PASSED;
            break;
        }
        msleep(POLLING_INTRVL);
    }

    if (polling_result != PASSED) {
        printf("%s: Failed to %s eth%d PHY MAC loopback.\n",
               __func__, ((onoff == GEPHY_EN) ? "enable" : "disable"), eth_num);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_gephy_config_media
 * Description: Function to set GEWAN PHY Copper(0_0) / Fiber(0_1) Contorl reg.
 * Inputs     : eth_num - ethernet number(eth0/1/2)
 *              intf_opt - Copper(0) / Fiber(1)
 *              conf_val - configure value
 *              sw_reset - enable(1) / disable(0)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_gephy_config_media (int eth_num, boolean intf_opt, 
                            uint16_t conf_val, boolean sw_reset)
{
    int reg_page = MRVL1112_CCR_REGPAGE;
    int reg_addr = MRVL1112_CCR_REGADDR;
    uint16_t reg_val = conf_val;
    int ctr = 0, r_ctr = 0;
    int polling_result = FAILED;

    if (intf_opt == GEPHY_FIB) {
        reg_page = MRVL1112_FCR_REGPAGE;
    }

    for (r_ctr = 0; r_ctr < MAX_TRY; r_ctr++) {
        if (sw_reset == GEPHY_EN) {
            reg_val |= COP_CTRL_RESET;
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: Set eth%d reg. %d_%d to 0x%04X.\n",
                   __func__, eth_num, reg_addr, reg_page, reg_val);
        }

        if (tsn_gephy_reg_wr(eth_num, reg_page, reg_addr, reg_val) != PASSED) {
            printf("%s: Failed to write eth%d PHY reg. %d_%d.\n",
                   __func__, eth_num, reg_addr, reg_page);
            return (FAILED);
        }

        sleep(ETH_DRIVER_DELAY);

        for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
            reg_val = 0;
            if (tsn_gephy_reg_rd(eth_num, reg_page,
                                 reg_addr, &reg_val) != PASSED) {
                printf("%s: Failed to read eth%d PHY reg. %d_%d.\n",
                       __func__, eth_num, reg_addr, reg_page);
                return (FAILED);
            }

            if (reg_val == conf_val) {
                polling_result = PASSED;
                break;
            }
            msleep(POLLING_INTRVL);
        }

        if (polling_result == PASSED) {
            break;
        }
    }

    if (polling_result != PASSED) {
        printf("%s: Failed to config eth%d PHY %s.\n",
               __func__, eth_num,
               ((intf_opt == GEPHY_COP) ? "Copper" : "Fiber"));
        return (FAILED);
    }
    return (PASSED);
}


 /*-------------------------------------------------
 * $Log: platform_ge_phy.c,v $
 * Revision 1.7  2018/11/23 08:49:52  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.6.34.1  2018/10/15 06:53:07  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.6  2018/05/24 09:47:10  steja
 * CSCvj57981-Enhance SFP GLC-GE-100FX Support
 *
 * Revision 1.5  2018/04/15 22:03:31  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.4  2018/04/13 08:52:58  palin2
 * To fix CSCvi96469: Potential issue on GEWAN0(Copper + SFP) loopback test.
 *
 * Revision 1.3.10.2  2018/04/09 21:35:56  palin2
 * Added soft-reset after set Marvell 88e1112  media mode based on datasheet.
 *
 * Revision 1.3.10.1  2018/04/09 20:57:00  palin2
 * Enhanced GEWAN PHY Diag tests by config testing media accordingly.
 *
 * Revision 1.3  2018/02/09 09:56:55  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.2.20.1  2018/01/24 07:59:43  hondwang
 * fix miss plug gephy merge
 *
 * Revision 1.2.4.3  2017/11/20 07:54:32  lucywang
 * Changed PID to C1101/C1109-2P/C1109-4P
 *
 * Revision 1.2.4.2  2017/08/28 03:34:13  lucywang
 * modified for C949-2P
 *
 * Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:48  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.3  2017/07/31 16:33:17  palin2
 * Added utiltiy to support GE WAN PHY VOD adjustments.
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/24 14:14:10  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.4.6.2  2017/06/18 07:52:27  hondwang
 * Modify all phy SMI bus function to support I2C bus
 *
 * Revision 1.1.4.4.6.1  2017/06/17 12:13:07  hondwang
 * Add test card phy testing function
 *
 * Revision 1.1.4.4.2.2  2017/07/17 13:54:44  palin2
 * Code cleanup.
 *
 * Revision 1.1.4.4.2.1  2017/07/05 14:05:24  steja
 * Enhance code readability
 *
 * Revision 1.1.4.4  2016/09/13 08:14:23  palin2
 * Added CPU to GE PHY MAC loopback test.
 *
 * Revision 1.1.4.3  2016/07/17 10:52:56  palin2
 * 1. Added function and utility to set GE WAN PHY Transmitter Type.
 * 2. Clean up code.
 *
 * Revision 1.1.4.2  2016/06/30 06:22:50  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.5  2016/05/26 11:53:03  palin2
 * Added utilities to turn TSN all Green/Yellow LEDs ON.
 *
 * Revision 1.1.2.4  2016/05/24 01:20:11  palin2
 * Updated GE Switch and PHY utilities.
 *
 * Revision 1.1.2.3  2016/04/26 20:48:49  palin2
 * Updated code after bring up SFP external loopback test.
 *
 * Revision 1.1.2.2  2016/04/22 12:28:36  palin2
 * Updated code after bring up GE PHY external loopback test.
 *
 * Revision 1.1.2.1  2016/03/29 02:50:02  palin2
 * Added GE PHY Diag.
 *
 * $Endlog$
 *-------------------------------------------------
 */


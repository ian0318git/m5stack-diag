/* $Id: diag_i2c_test.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_i2c_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_dimm_util.h"
#include "diag_mcu_util.h"
#include "diag_temp_sensor_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_poe_psu_lib.h"
#include "diag_ge_phy_lib.h"

static n2g_i2c_if_t plat_cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Aikido Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    /*
     * I2C 1
     */
    {
     .dev_name = "Temp. Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "SFP Module",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_SFP0,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C 2
     */
    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC Controller",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_EEPROM,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
     {
     .dev_name = "WiFi Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_PLAT_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

/*****************************************************************************
 *
 * Function   : diag_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on platform
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int diag_i2c_scan_test (int option)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "I2C", "ACT2/TAM, Boot Strap I2C EEPROM, RTC, SFP, or POE");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the interface between the Host SoC "
                    "and the failed I2C devices.",
                    "If there is no problem for these interfaces, "
                    "replace one I2C device and redo the test.");
#endif

    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED;
    uint32_t status;
    uint8_t now_test = 0, test_end = 0, test_num = 1;
    uint8_t plat_cpu_i2c_test_end = 0;
    uchar *tname = (uchar *) "I2C scan";
    int err_code = 0;

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /*
     * Setup end of test by calculate all I2C device number
     */
    plat_cpu_i2c_test_end = (sizeof(plat_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));

    test_end = plat_cpu_i2c_test_end;

    for (now_test = 0; now_test < test_end; now_test++) {
        /*
         * Get I2C device structure
         */
        memcpy(&i2c_if, &plat_cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        i2c_if.buf = (char *) &reg_val;

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf
                ("Now testing %2d: I2C bus %2d, Mux %d, %-29s(0x%.2X)\n",
                 now_test, i2c_if.i2c_bus_type, i2c_if.mux, i2c_if.dev_name,
                 (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "[%2d] I2C_%d: %s, ",
                test_num, i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /* Skipped to check SFP if no SFP module present */
        /* Check if SFP is available. if not, return failed */
        if ((i2c_if.i2c_dev == MB_I2C_ADDR_SFP0)) {
            if ((is_sfp_present(GE0) != PASSED) && (is_sfp_present(GE1) != PASSED)) {
                printf("Skipped SFP Scan because SFP module is not present.\n");
                continue;
            } else if (is_sfp_present(GE0) == PASSED) {
                if (sfp_mux_select(GE0) != PASSED) {
                    printf("%s:%d:Failed to switch SFP mux\n", __FUNCTION__, __LINE__);
                    return (FAILED);
                }
            } else if (is_sfp_present(GE1) == PASSED) {
                if (sfp_mux_select(GE1) != PASSED) {
                    printf("%s:%d:Failed to switch SFP mux\n", __FUNCTION__, __LINE__);
                    return (FAILED);
                }
            }
        }

        /* Skipped to check PoE if no PoE module present,
         * due to the because PoE DC is optional. */
        if ((i2c_if.i2c_dev == MB_I2C_ADDR_POE_30W_CTRLER) &&
            (platform_has_poe(0) != TRUE)) {
            printf("Skipped PoE because PoE DC(optional) is not present.\n");
            continue;
        }

        /* Skipped to check PoE if no PoE EEPROM module present,
         * due to the PoE DC is optional. */
        if ((i2c_if.i2c_dev == MB_I2C_ADDR_POE_EEPROM) &&
            (platform_has_poe(0) != TRUE)) {
            printf("Skipped PoE because PoE EEPROM is not present.\n");
            continue;
        }

        /*
         * Read I2C device Register 0
         */
        ret_val = n2g_i2c_read(&i2c_if);
        if (ret_val != PASSED) {
            err_code = i2c_err_no(&status);
            cterr('f', 0, "%s failed %s [i2c_status=%#x]",
                  i2c_if.dev_name, i2c_err_str(err_code), status);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("...Done\n");
        }

        test_num++;
    }

    prpass(testpass, "%s, ", tname);
    prcomplete(testpass, errcount, (char *)0);
    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_i2c_test.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.5  2021/07/01 02:36:27  harrchan
 * 1.Due to aikido v6 implementation. Need to  modify I2C scan test read size from 2 bytes to 1 bytes.
 * 2.Due to aikido v6 implementation. Need to remove some items in register test table.
 *
 * Revision 1.1.2.4  2020/11/06 06:23:25  harrchan
 * Add wifi I2C address into I2C scan test
 *
 * Revision 1.1.2.3  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.2  2020/09/16 02:25:35  harrchan
 * Support GE1 SFP test
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

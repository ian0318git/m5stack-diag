 /* $Id: diag_i2c_test.c,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_i2c_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_i2c_test.c - For I2C test
 *
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "diag_i2c_test.h"
#include "diag_i2c_addr.h"
#include "diag_i2c_lib.h"

/*
 * Functional prototype
 */
int viper_i2c_scan_test(int);

static n2g_i2c_if_t cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM SPD",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_DIMM0,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Temperature Sensor(LM75BD)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP_LM75,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C FPGA
     */
    {
     .dev_name = "ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
};

/*****************************************************************************
 *
 * Function   : viper_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on viper
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int viper_i2c_scan_test (int option)
{
    //uchar mb_get_loc[FRU_SIZE] = {0};
    //uchar mb_get_pid[FRU_SIZE] = {0};
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
    //platform_get_pid((char *)mb_get_pid);
    //strcpy((char *)mb_get_loc, "MB");
    //platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    //platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    //cterr_add_component("Marvell Armada 7040", "I2C", "ACT2/TAM, Boot Strap I2C EEPROM, RTC, SFP, or POE");

    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/

    /* Segment 7: Top 3 Debugging Steps */
    //cterr_add_debug("Check the interface between the Host SoC "
                    //"and the failed I2C devices.",
                    //"If there is no problem for these interfaces, "
                    //"replace one I2C device and redo the test.");
#endif

    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED;
    uint32_t ix, max_retry;
    uint8_t now_test = 0, test_end = 0, test_num = 1;
    uchar *tname = (uchar *) "I2C scan";
    int err_code = 0;
    max_retry = MAX_RETRY;

    testname("%s", tname);

    /*
     * Setup end of test by calculate all I2C device number
     */
    test_end = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));

    for (now_test = 0; now_test < test_end; now_test++) {
        /*
         * Get I2C device structure
         */
        memcpy(&i2c_if, &cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        i2c_if.buf = (char *) &reg_val;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Now testing %2d: I2C bus %2d, Mux %d, %-29s(0x%.2X)... ",
                   now_test, i2c_if.i2c_bus_type, i2c_if.mux, i2c_if.dev_name,
                   (i2c_if.i2c_dev << 1));
        } else {
            printf("[%2d] I2C_%d: %s\n ",
                   test_num, i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /*
         * Read I2C device Register 0
         */
        for (ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val != PASSED) {
                printf("I2C scan retry %d\n", ix);
            } else {
                break;
            }
            msleep(DELAY_I2C_SCAN_RETRY);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done\n");
        }

        test_num++;
    }
    /* Enable cterr */
    if (ret_val != PASSED) {
        cterr('f', 0, "%s failed %s", i2c_if.dev_name, i2c_err_str(err_code));
    } else {
        prpass(testpass, "%s, ", tname);
        prcomplete(testpass, errcount, (char *)0);
    }
    return (ret_val);
}

/*-------------------------------------------------
 * $Log: diag_i2c_test.c,v $
 * Revision 1.2  2018/08/06 02:31:50  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.4  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.3  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.2  2018/03/28 07:55:52  lucywang
 * Changed Thermal sersor to LM75B, TBD : bug fix
 *
 * Revision 1.1.2.1  2018/02/27 08:06:44  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

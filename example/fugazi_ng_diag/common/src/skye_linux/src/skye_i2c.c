/* $Id: skye_i2c.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_i2c.c,v $
 *------------------------------------------------------------------------------
 * skye_i2c.c - Main file of Skye I2C related tests and utilities.
 *
 * July 07 2014, Paul Lin(palin2) created for ShrinkRay.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"
#include "error.h"
#include "nvmonvars.h"
#include "types.h"
#include "skye_i2c.h"
#include "proto.h"

#ifdef SKYE_ENHANCED_ERR_MSG
#include "platform_fru.h"
#endif   /* SKYE_ENHANCED_ERR_MSG */


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
int skye_i2c_scan_test(int);


/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int skye_i2c_mux_ctrl_reg_wr(uchar *);
extern boolean cpu_id;


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
static skye_i2c_dev_t skye_cpu0_i2c_dev[] = {
    /* I2CM0: BIB ROM(0xA8), addr. size = 2(bytes) */
    {
        .dev_name = "BIB ROM",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_BIB_I2C_ADDR,
        .addr_sz  = SR_BIB_I2C_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM0,
        .size     = SR_BIB_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH0 DIMM SPD(0xA0), addr. size = 1(byte) */
    {
        .dev_name = "CH0 DIMM SPD",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM0_SPD_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH0 DIMM Thermal Sensor(0x30), addr. size = 1(byte) */
    {
        .dev_name = "CH0 DIMM Thermal Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM0_TS_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_TS_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH1 DIMM SPD(0xA4), addr. size = 1(byte) */
    {
        .dev_name = "CH1 DIMM SPD",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM1_SPD_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH1 DIMM Thermal Sensor(0x34), addr. size = 1(byte) */
    {
        .dev_name = "CH0 DIMM Thermal Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM1_TS_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_TS_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH0: Power sequencer(0x82), addr. size = 1(byte) */
    {
        .dev_name = "Power Sequencer",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_PWR_SEQ_I2C_ADDR,
        .addr_sz  = SR_PWR_SEQ_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_PWR_SEQ_REG_SZ,
        .mux_ch   = SR_PWR_SEQ_CH,
        .mux      = PCA9546A_I2C_CH0,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH0: Clock Buffer(0xD8), addr. size = 1(byte) */
    {
        .dev_name = "Clock Buffer",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_CLK_BUF_I2C_ADDR,
        .addr_sz  = SR_CLK_BUF_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_CLK_BUF_REG_SZ,
        .mux_ch   = SR_CLK_BUF_CH,
        .mux      = PCA9546A_I2C_CH0,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH1: Thermal Sensor(0x98), addr. size = 1(byte) */
    {
        .dev_name = "Thermal Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_THERMAL_I2C_ADDR,
        .addr_sz  = SR_THERMAL_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_THERMAL_REG_SZ,
        .mux_ch   = SR_THERMAL_CH,
        .mux      = PCA9546A_I2C_CH1,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH2: Szalinski FPGA(0x54), addr. size = 1(byte) */
    {
        .dev_name = "Szalinski FPGA",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_CPU0_FPGA_I2C_ADDR,
        .addr_sz  = SR_FPGA_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_FPGA_REG_SZ,
        .mux_ch   = SR_FPGA_CH,
        .mux      = PCA9546A_I2C_CH2,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH3: Current Sensor(0x80), addr. size = 1(byte) */
    {
        .dev_name = "Current Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_CUR_SENSOR_I2C_ADDR,
        .addr_sz  = SR_CURRENT_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_CURRENT_REG_SZ,
        .mux_ch   = SR_CUR_SENSOR_CH,
        .mux      = PCA9546A_I2C_CH3,
        .buf      = NULL,
    },
};

static skye_i2c_dev_t skye_cpu1_i2c_dev[] = {
    /* I2CM0: BIB ROM(0xA8), addr. size = 2(bytes) */
    {
        .dev_name = "BIB ROM",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_BIB_I2C_ADDR,
        .addr_sz  = SR_BIB_I2C_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM0,
        .size     = SR_BIB_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH0 DIMM SPD(0xA0), addr. size = 1(byte) */
    {
        .dev_name = "CH0 DIMM SPD",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM0_SPD_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH0 DIMM Thermal Sensor(0x30), addr. size = 1(byte) */
    {
        .dev_name = "CH0 DIMM Thermal Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM0_TS_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_TS_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH1 DIMM SPD(0xA4), addr. size = 1(byte) */
    {
        .dev_name = "CH1 DIMM SPD",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM1_SPD_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM1: CH1 DIMM Thermal Sensor(0x34), addr. size = 1(byte) */
    {
        .dev_name = "CH0 DIMM Thermal Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_DIMM1_TS_I2C_ADDR,
        .addr_sz  = SR_DIMM_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM1,
        .size     = SR_DIMM_TS_REG_SZ,
        .mux_ch   = 0,
        .mux      = 0,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH1: Thermal Sensor(0x98), addr. size = 1(byte) */
    {
        .dev_name = "Thermal Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_THERMAL_I2C_ADDR,
        .addr_sz  = SR_THERMAL_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_THERMAL_REG_SZ,
        .mux_ch   = SR_THERMAL_CH,
        .mux      = PCA9546A_I2C_CH1,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH2: Szalinski FPGA(0x58), addr. size = 1(byte) */
    {
        .dev_name = "Szalinski FPGA",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_CPU1_FPGA_I2C_ADDR,
        .addr_sz  = SR_FPGA_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_FPGA_REG_SZ,
        .mux_ch   = SR_FPGA_CH,
        .mux      = PCA9546A_I2C_CH2,
        .buf      = NULL,
    },
    /* I2CM2, I2C Mux CH3: Current Sensor(0x80), addr. size = 1(byte) */
    {
        .dev_name = "Current Sensor",
        .offset   = -1,  /* need to be -1 to tell driver not to use offset !!! */
        .dev_addr = SR_CUR_SENSOR_I2C_ADDR,
        .addr_sz  = SR_CURRENT_ADDR_SZ,
        .i2c_ctrl = SR_CPU_I2CM2,
        .size     = SR_CURRENT_REG_SZ,
        .mux_ch   = SR_CUR_SENSOR_CH,
        .mux      = PCA9546A_I2C_CH3,
        .buf      = NULL,
    },
};

/*******************************************************************************
 *
 * Function   : skye_i2c_scan_test
 * Description:	Test to quick check all Skye I2C devices.
 * Inputs     : opt - Reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_scan_test (int opt)
{
    skye_i2c_dev_t  *test_dev;
    uint32_t        curr_test = 0, test_size = 0, rd_val = 0;
    char            devname[32];
    int             fd = -1, ret_val = FAILED, ctr = 1;
    uint8_t         test_ctrl = 3;
    uchar           mux_data = 0;
    uint16_t        test_reg = 0;

    testname("I2C Device Scan");
    prpass(testpass, "Skye CPU%d ", cpu_id);

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_I2C;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "I2C device");
	
    /* Segment 5: register and memory dump */

    /* Segment 6: Platform Environment initialized here*/
    if (cpu_id == MASTER_CPU) {
        cterr_add_env_dump((PFV)skye_dump_volt_margins,
                           (PFV)skye_dump_temps);
    } else {
        /* Based on Skye HW design, CPU1(2nd CPU) can't access power sequencer.
         * So CPU1 won't know Skye board current voltage margins. */
        cterr_add_env_dump((PFV)skye_dump_temps);
    }

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check I2C device is mounted in module.",
                    "Measure I2C bus signal.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
	
    if (cpu_id == MASTER_CPU) {
        test_dev = &skye_cpu0_i2c_dev[0];
        test_size = (uint32_t)(sizeof(skye_cpu0_i2c_dev) /
                               sizeof(skye_i2c_dev_t));
    } else {
        test_dev = &skye_cpu1_i2c_dev[0];
        test_size = (uint32_t)(sizeof(skye_cpu1_i2c_dev) /
                               sizeof(skye_i2c_dev_t));
    }

    /* Enable I2C Mux all channels for testing purpose. */
    mux_data = PCA9546A_I2C_ALL_CH;

    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        cterr('f', 0, "%s: Failed to Enable CPU%d I2CM%d "
                      "I2C Mux channel%d to %s.\n",
                      __FUNCTION__, cpu_id, test_dev->i2c_ctrl,
                      test_dev->mux_ch, test_dev->dev_name);

        return (FAILED);
    }

    for (curr_test = 0; curr_test < test_size; curr_test++, test_dev++) {
        if (DIAGFLAG & D_VERBOSE) {
            if (curr_test == 0) {
                printf("\n");
            }

            if ((test_dev->i2c_ctrl == SR_CPU_I2CM2) &&
                (test_dev->dev_addr != SR_I2C_MUX_ADDR)) {
                printf("[%2d]: CPU%d I2CM%d, Mux CH%d, %-15s(0x%.2X)... ",
                       (curr_test + 1), cpu_id, test_dev->i2c_ctrl,
                       test_dev->mux_ch, test_dev->dev_name,
                       (test_dev->dev_addr << 1));
            } else {
                printf("[%2d]: CPU%d I2CM%d, %-24s(0x%.2X)... ",
                       (curr_test + 1), cpu_id, test_dev->i2c_ctrl,
                       test_dev->dev_name, (test_dev->dev_addr << 1));
            }
        } else {
            if ((test_dev->i2c_ctrl == SR_CPU_I2CM2) &&
                (test_dev->dev_addr != SR_I2C_MUX_ADDR)) {
                prpass(testpass, "[%2d]: CPU%d I2CM%d, Mux CH%d, %-15s(0x%.2X), ",
                       (curr_test + 1), cpu_id, test_dev->i2c_ctrl,
                       test_dev->mux_ch, test_dev->dev_name,
                       (test_dev->dev_addr << 1));
            } else {
                prpass(testpass, "[%2d]: CPU%d I2CM%d, %-24s(0x%.2X), ",
                       (curr_test + 1), cpu_id, test_dev->i2c_ctrl,
                       test_dev->dev_name, (test_dev->dev_addr << 1));
            }
        } 
   
        /* Read I2C device Register 0 */
        if (test_dev->i2c_ctrl != test_ctrl) {
            if (fd != -1) {
                close(fd);
            }

            fd = -1;
            memset(devname, 0, sizeof(devname));
            snprintf(devname, sizeof(devname), "/dev/i2c-%d", test_dev->i2c_ctrl);

            fd = open(devname, O_RDWR);
            if (fd < 0) {
                cterr('f', 0, "%s: Failed to open /dev/i2c-%d.\n",
                              __FUNCTION__, test_dev->i2c_ctrl);
                return (FAILED);
            }

            test_ctrl = test_dev->i2c_ctrl;
        }

        rd_val = 0;
        test_dev->buf = (uchar *)&rd_val;

        for (ctr = 1; ctr <= SKYE_I2C_RETRY_MAX; ctr++) {
            ret_val = FAILED;
            ret_val = skye_i2c_read(fd, test_dev->dev_addr, test_dev->addr_sz,
                                    test_reg, test_dev->size, test_dev->buf);

            if (ret_val == PASSED) {
                break;
            } else if ((cpu_id == SLAVE_CPU) && (ctr < SKYE_I2C_RETRY_MAX)) {
                /* This is workaround code:
                 * We use temporarily to pass Skye P1B 2nd CPU BST I2C intermittent issue.
                 * Added 2 times re-try here w/o power cycle module,
                 * will keep debug this issue with HW team later.
                 */
                printf("\n\n===[Retry %d] CPU%d I2C scan Thermal sensor !!===\n\n",
                       ctr, cpu_id);
                msleep(500);
                continue;
            } else {
                if ((test_dev->i2c_ctrl == SR_CPU_I2CM2) &&
                    (test_dev->dev_addr != SR_I2C_MUX_ADDR)) {
                    cterr('f', 0, "Failed CPU%d I2CM%d, Mux CH%d, %s(0x%.2X)",
                                  cpu_id, test_dev->i2c_ctrl, test_dev->mux_ch,
                                  test_dev->dev_name, (test_dev->dev_addr << 1));
                } else {
                    cterr('f', 0, "Failed CPU%d I2CM%d, %s(0x%.2X)",
                                  cpu_id, test_dev->i2c_ctrl, test_dev->dev_name,
                                  (test_dev->dev_addr << 1));
                }

                close(fd);
                return (FAILED);
            }
        }

        if (DIAGFLAG & D_VERBOSE) {
            printf("Done\n");
        }
    }
    close(fd);

    return (PASSED);
}


/*-------------------------------------------------
$Log: skye_i2c.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:35  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.5  2015/01/20 00:49:23  palin2
Temporarily added retry when CPU1 of 2-CPUs I2C access thermal sensor.

Revision 1.1.2.4  2014/11/27 09:14:14  palin2
Updated enhanced error message dump out info for CPU1 of 2-CPUs Skye.

Revision 1.1.2.3  2014/09/18 07:22:26  palin2
Updated enhanced error message - debugging steps.

Revision 1.1.2.2  2014/09/17 04:35:07  palin2
Updated Skye enhanced error message.

Revision 1.1.2.1  2014/07/21 01:56:55  palin2
Initial check-in Skye module side Diag code.

---------------------------------------------------
skye_i2c.c:
Revision 1.1.2.1  2014/07/09 02:21:09  palin2
Support I2C scan test for Shrinkray.

---------------------------------------------------
$Endlog$
*/


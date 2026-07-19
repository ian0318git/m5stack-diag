/* $Id: vm_timingcard_cpld_lib.c,v 1.3 2017/07/14 02:51:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_cpld_lib.c,v $
 *******************************************************************************
 * File Name: vm_timingcard_cpld_lib.c
 *
 * Description: NGVM Timing Card CPLD library source file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "string.h"
#include "sm_slot.h"
#include "menu.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "platform_slot.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "plat_defs.h"
#include "i2c_api.h"
#include "jbiexprt.h"
#include "vm_timingcard_zl3036x_lib.h"
#include "vm_timingcard_cpld_lib.h"
#include "vm_timingcard_zl3036x_diag.h"
#include "vm_timingcard.h"
#include "vm_timingcard_pca9557_lib.h"
#include "vm_timingcard_cpld_diag.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/
#define CPLD_RONLY    (READ_ONLY | REG_ACCESS)
#define CPLD_RW       (READ_WRITE | REG_ACCESS)

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static int timingcard_cpld_read_fn(unsigned long, int, unsigned long *, void *);
static int timingcard_cpld_write_fn(unsigned long, int, unsigned long, void *);
static void cpld_reset_2_run(void);
static unsigned int cpld_shift(unsigned int, unsigned char, unsigned char,
                               boolean);
#ifdef ORIGINAL_SPEEDUP_PROGRAM
static void cpld_reset_jtag(void);
static void cpld_run_2_ir(void);
static void cpld_ir_2_run(void);
static void cpld_dr_2_run(void);
static void cpld_ir_2_dr(void);
static void cpld_run_2_dr(void);
#endif
static void cpld_run_test_delay(int);
static long speed_up_cpld_jtag_io(uchar, uchar, boolean);
static void speedup_cpld_reset_jtag(void);
static void speedup_cpld_run_2_ir(void);
static void speedup_cpld_ir_2_run(void);
static void speedup_cpld_dr_2_run(void);
static void speedup_cpld_ir_2_dr(void);
static void speedup_cpld_run_2_dr(void);
static void erase_all(int);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long timingcard_cpld_reg_test_lib(void);
long util_oir_cpld_reg_read(void);
long util_oir_cpld_reg_write(void);
long timingcard_cpld_drive_gpio(uint);
long timingcard_cpld_check_gpio_val(uint, uint, uint);
long timingcard_cpld_reg_read_lib(uchar, uchar *);
long timingcard_cpld_reg_write_lib(uchar, uchar);
long timingcard_cpld_jtag_ctl(boolean);
long display_embedded_cpld_fw_version(uchar *);
long cpld_jtag_io(int, int, int);
long max2_cpld_program(void);
long timingcard_cpld_power_ctl(void);
long clock_verification_path_lib(uint);
long clock_direction_lib(uint);
long init_timingcard(void);
long timingcard_simply_program_cpld(unsigned char *);

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern unsigned char cpld_fw_array[];
extern unsigned long cpld_fw_size;
extern int upgrade_interface;
extern int getdec_answer(char *msgstr, uint, uint, uint);
extern int get_timingcard_sku_id(void);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

char *error_text[] =
{
/* JBIC_SUCCESS            0 */ "success",
/* JBIC_OUT_OF_MEMORY      1 */ "out of memory",
/* JBIC_IO_ERROR           2 */ "file access error",
/* JAMC_SYNTAX_ERROR       3 */ "syntax error",
/* JBIC_UNEXPECTED_END     4 */ "unexpected end of file",
/* JBIC_UNDEFINED_SYMBOL   5 */ "undefined symbol",
/* JAMC_REDEFINED_SYMBOL   6 */ "redefined symbol",
/* JBIC_INTEGER_OVERFLOW   7 */ "integer overflow",
/* JBIC_DIVIDE_BY_ZERO     8 */ "divide by zero",
/* JBIC_CRC_ERROR          9 */ "CRC mismatch",
/* JBIC_INTERNAL_ERROR    10 */ "internal error",
/* JBIC_BOUNDS_ERROR      11 */ "bounds error",
/* JAMC_TYPE_MISMATCH     12 */ "type mismatch",
/* JAMC_ASSIGN_TO_CONST   13 */ "assignment to constant",
/* JAMC_NEXT_UNEXPECTED   14 */ "NEXT unexpected",
/* JAMC_POP_UNEXPECTED    15 */ "POP unexpected",
/* JAMC_RETURN_UNEXPECTED 16 */ "RETURN unexpected",
/* JAMC_ILLEGAL_SYMBOL    17 */ "illegal symbol name",
/* JBIC_VECTOR_MAP_FAILED 18 */ "vector signal name not found",
/* JBIC_USER_ABORT        19 */ "execution cancelled",
/* JBIC_STACK_OVERFLOW    20 */ "stack overflow",
/* JBIC_ILLEGAL_OPCODE    21 */ "illegal instruction code",
/* JAMC_PHASE_ERROR       22 */ "phase error",
/* JAMC_SCOPE_ERROR       23 */ "scope error",
/* JBIC_ACTION_NOT_FOUND  24 */ "action not found",
};

#define MAX_ERROR_CODE (int)((sizeof(error_text)/sizeof(error_text[0]))+1)

static n2g_i2c_if_t cpld =
{
    .dev_name = "Timing Card CPLD",
    .offset = 0,
    .i2c_bus_type = IOFPGA_I2C,
    .size = 0x1,
    .mux = I2C_MUX_ZERO,
    .buf = NULL,
    /* Dash FPGA I2C Controller 15 is used for NGVM, refer to O2 HFS. */
    .i2c_ctrl = I2C_CTRL_FIFTEEN,
    /* Timing Card CPLD I2C 7-bits address */
    .i2c_dev = TIMING_CARD_CPLD_I2C_ADDR,
};

static reg_info_t_ext cpld_reg_ext = {1, timingcard_cpld_read_fn,
                                      timingcard_cpld_write_fn, 0};

static reg_info_t cpld_test_regs[] = {
    {"NGIO Expander Input Register", CPLD_NGIO_EXPANDER_INPUT,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x20, 0x23},
    {"NGIO Expander Output Register", CPLD_NGIO_EXPANDER_OUTPUT,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x20, 0x23},
    {"NGIO Expander Dir Register", CPLD_NGIO_EXPANDER_DIR,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x20, 0xff},
    {"ZL30363 and Control Status Register", CPLD_ZL30363_CONT_STS,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x00},
    {"Power Sequence Status Register", CPLD_POW_SEQ_STS,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0x8f, 0x80},
    {"Jtag Control Register", CPLD_JTAG_CTL,
     CPLD_RW, {(unsigned long)&cpld_reg_ext}, 0xe, 0x0},
    {"CPLD Version Register", CPLD_VERSION,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xf, 0x0},
    {"CPLD GPIO Direction Register", CPLD_GPIO_DIRECTION,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Speed Up Register", CPLD_SPEED_UP,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Ten Register", CPLD_TEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Eleven Register", CPLD_ELEVEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Twelve Register", CPLD_TWELVE,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Thirteen Register", CPLD_THIRTEEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Fourteen Register", CPLD_FOURTEEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"CPLD Fifteen Register", CPLD_FIFTEEN,
     CPLD_RONLY, {(unsigned long)&cpld_reg_ext}, 0xff, 0x0},
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},
};

static int count = 0;

/**********************************************************************
 *
 * Function: util_oir_cpld_reg_read
 *
 * Wrapper for CPLD Register Display utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_cpld_reg_read (void)
{
    if (register_display(0, &cpld_test_regs[0]) == FAILED) {
        cterr('f', 0, "CPLD Register Display Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_oir_cpld_reg_write
 *
 * Wrapper for CPLD Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_cpld_reg_write (void)
{
    int rc = PASSED;
    uint8_t cpld_write_buf;
    uint write_addr;
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&cpld;

    printf("\n\nCPLD Register Write\n\n");

    write_addr = gethex_answer("Reg offset to write", 0, 0, 0xf);
    i2c_p->offset = write_addr;
    
    cpld_write_buf = gethex_answer("Enter new reg value", 0x0, 0, 0xff);
    
    i2c_p->buf = (char *)&cpld_write_buf;

    /* alter reg with new value */
    rc = n2g_i2c_write(i2c_p);
    if (rc != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_drive_gpio
 *
 * This function drive the CPLD gpio.
 *
 * Input : drive_val - gpio drive value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_drive_gpio (uint drive_val)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&cpld;

    /* Drive the GPIO status by setting the Control Status register */
    i2c_p->offset = CPLD_ZL30363_CONT_STS;
    i2c_p->buf = (char *)&drive_val;

    /* Alter reg with new value */
    i2c_p->buf = (char *)&drive_val;
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_check_gpio_val
 *
 * This function check the CPLD gpio value.
 *
 * Input : check_mask - check mask value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_check_gpio_val (uint gpio_no, uint check_mask, uint check_sts)
{
    uchar buffer;
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&cpld;

    /* Drive the GPIO status by setting the Control Status register */
    i2c_p->offset = CPLD_ZL30363_CONT_STS;
    i2c_p->buf = (char *)&buffer;

    if (n2g_i2c_read(i2c_p) != PASSED) {
        printf("unable to read i2c.\n");
        return (FAILED);
    }

    /* Check the GPIO status */
    if (!check_sts) {
        /* Check the GPIO low status */
        if ((~check_mask) & buffer) {
            cterr('f', 0, "Check GPIO %d offset %#.2x value failed, buffer is %#.2x "
                  "check mask is %#.2x", gpio_no, CPLD_ZL30363_CONT_STS, buffer, check_mask);
            return (FAILED);
        }
    } else {
        /* Check the GPIO high status */
        if (!(check_mask & buffer)) {
            cterr('f', 0, "Check GPIO %d offset %#.2x value failed, buffer is %#.2x "
                  "check mask is %#.2x", gpio_no, CPLD_ZL30363_CONT_STS, buffer, check_mask);
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_reg_read_lib
 *
 * This function provides the library for cpld register reads.
 *
 * Input : offset - register offset
 *         *buffer - pointer of the buffer address
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_reg_read_lib (uchar offset, uchar *buffer)
{
    int rc;
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&cpld;

    /* Before perform the register test, need to keep the
     * JTAG Control register JTAG_ON bit as 0 */
    i2c_p->offset = offset;
    i2c_p->buf = (char *)buffer;

    /* Read the CPLD_JTAG_CTL offset value */
    rc = n2g_i2c_read(i2c_p);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to read i2c offset %#.2x, ret_code = %#x",
              offset, rc);
        return (rc);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_reg_write_lib
 *
 * This function provides the library for cpld register writes.
 *
 * Input : offset - register offset
 *         buffer - write buffer value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_reg_write_lib (uchar offset, uchar buffer)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&cpld;

    i2c_p->offset = offset;
    i2c_p->buf = (char *)&buffer;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_reg_test_lib
 *
 * This function perform the CPLD register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_reg_test_lib (void)
{
    uchar cpld_buf;

    /* Before perform the register test, need to keep the
     * JTAG Control register JTAG_ON bit as 0 */
    if (timingcard_cpld_reg_read_lib(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
        return (FAILED);
    }

    /* Make JTAG_ON bit as 0 */
    cpld_buf &= ~(CPLD_JTAG_ON);

    /* Alter the register */
    if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
        return (FAILED);
    }

    /* Perform the CPLD register test. */
    if (register_tests(0, &cpld_test_regs[0]) == FAILED) {
        cterr('f', 0, "CPLD Register Test Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_power_ctl
 *
 * Function to control cpld power on/off
 *
 * Input : cpld_power - power on/off the cpld
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_power_ctl (void)
{
    uchar cpld_buf;

    /* Power Sequence Status register bit 7 as 1 */
    if (timingcard_cpld_reg_read_lib(CPLD_POW_SEQ_STS, &cpld_buf) == FAILED) {
        return (FAILED);
    }

    /* Make Power Sequence Status bit 7 as 0 */
    cpld_buf &= ~(CPLD_POWER_RESTART);

    /* Alter the register */
    if (timingcard_cpld_reg_write_lib(CPLD_POW_SEQ_STS, cpld_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_jtag_ctl
 *
 * Function to turn on jtag
 *
 * Input : jtag_on_off - turn on/off the jtag on
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_cpld_jtag_ctl (boolean jtag_on_off)
{
    uchar cpld_buf;

    if (upgrade_interface == UPGRADE_FROM_CPLD) {
        /* JTAG Control register JTAG_ON bit as 1 */
        if (timingcard_cpld_reg_read_lib(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
            return (FAILED);
        }

        if (jtag_on_off == TRUE) {
            /* Make JTAG_ON bit as 1 */
            cpld_buf |= (CPLD_JTAG_ON);
        } else {
            /* Make JTAG_ON bit as 0 */
            cpld_buf &= ~(CPLD_JTAG_ON);
        }

        /* Make JTAG_TCK bit as 0 */
        cpld_buf &= ~(CPLD_JTAG_TCK_ST);

        /* Alter the register */
        if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : tms - jtag tms control
 *         tdi - jtag tdi control
 *         read_tdo - jtag read_tdo control
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long cpld_jtag_io (int tms, int tdi, int read_tdo)
{
    int tdo = 0;
    uchar cpld_buf;
    uchar io_expander_buf;

    /* Print out the status. */
    count++;
    if ((count % 10) == 0) {
        printf(".");
        count = 0;
    }

    if (upgrade_interface == UPGRADE_FROM_CPLD) {
        /* Upgrade the firmware from CPLD
         * Write data (TMS, TDI, TCK written low)
         * Read TDO
         * Write data (TCK written high)
         * Write data (TCK written low)
         */
        cpld_buf = CPLD_JTAG_ON;

        /* Prepare: Write data (TMS) */
        if (tms) {
            cpld_buf |= CPLD_JTAG_TMS_ST;
        } else {
            cpld_buf &= ~CPLD_JTAG_TMS_ST;
        }

        /* Prepare: Write data (TDI) */
        if (tdi) {
            cpld_buf |= CPLD_TDI_ST;
        } else {
            cpld_buf &= ~CPLD_TDI_ST;
        }

        /* Prepare: TCK written low */
        cpld_buf &= ~CPLD_JTAG_TCK_ST;

        /* step 1: Write data (TMS, TDI, TCK written low) */
        if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }

        /* step 2: Read TDO */
        if (read_tdo) {
            if (timingcard_cpld_reg_read_lib(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
                return (FAILED);
            }

            tdo = (cpld_buf & CPLD_TDO_ST) ? 1 : 0;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%d ", tdo);
            }
        }

        /* Step 3: Write data (TCK written high) */
        cpld_buf |= CPLD_JTAG_TCK_ST;
        if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }

        /* Step 4: Write data (TCK written low) */
        cpld_buf &= ~CPLD_JTAG_TCK_ST;
        if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }
    } else {
        /* Upgrade the firmware from IO Expander interface.
         * Write data (TMS, TDI, TCK written low)
         * Read TDO
         * Write data (TCK written high)
         * Write data (TCK written low)
         */
        io_expander_buf = 0;

        /* Prepare: Write data (TMS) */
        if (tms) {
            io_expander_buf |= PCA9557_TMS_GPIO7_OUTPUT;
        } else {
            io_expander_buf &= ~PCA9557_TMS_GPIO7_OUTPUT;
        }

        /* Prepare: Write data (TDI) */
        if (tdi) {
            io_expander_buf |= PCA9557_TDI_GPIO4_OUTPUT;
        } else {
            io_expander_buf &= ~PCA9557_TDI_GPIO4_OUTPUT;
        }

        /* Prepare: TCK written low */
        io_expander_buf &= ~PCA9557_TCK_GPIO6_OUTPUT;

        /* step 1: Write data (TMS, TDI, TCK written low) */
        if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, io_expander_buf)
            == FAILED) {
            return (FAILED);
        }

        /* step 2: Read TDO */
        if (read_tdo) {
            if (timingcard_pca9557_i2c_r(PCA9557_NGIO_EXPANDER_INPUT, &io_expander_buf)
                == FAILED) {
                return (FAILED);
            }

            tdo = (io_expander_buf & PCA9557_TDO_GPIO5_INPUT) ? 1 : 0;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%d ", tdo);
            }
        }

        /* Step 3: Write data (TCK written high) */
        io_expander_buf |= PCA9557_TCK_GPIO6_OUTPUT;
        if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, io_expander_buf)
            == FAILED) {
            return (FAILED);
        }

        /* Step 4: Write data (TCK written low) */
        io_expander_buf &= ~PCA9557_TCK_GPIO6_OUTPUT;
        if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, io_expander_buf)
            == FAILED) {
            return (FAILED);
        }
    }

    return (tdo);
}

/**********************************************************************
 *
 * Function: speed_up_cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : reg_offset - cpld register offset
 *         cpld_w_buf - cpld write data value
 *         tdo_read - cpld read data value flag
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long speed_up_cpld_jtag_io (uchar reg_offset, uchar cpld_w_buf,
                                   boolean tdo_read)
{
    uchar cpld_r_buf = 0, rd_data;

    if (tdo_read == FALSE) {
        /* Write data value */
        if (timingcard_cpld_reg_write_lib(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }
    }

    /* Read data value */
    if (tdo_read == TRUE) {
        if (timingcard_cpld_reg_read_lib(reg_offset, &rd_data) == FAILED) {
            return (FAILED);
        }

        cpld_r_buf = rd_data;
    }

    return (cpld_r_buf);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_jtag_ctl
 *
 * Function to program MAXII CPLD firmware library.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long max2_cpld_program (void)
{
    uint retval = FAILED;
    char *init_list[10];
    int offset = 0L;
    char key[33] = {0};
    int index = 0;
    int exec_result = JBIC_SUCCESS;
    char *action_name = NULL;
    char *description = NULL;
    JBI_PROCINFO *procedure_list = NULL;
    JBI_PROCINFO *procptr = NULL;
    char value[MAX_NOTE_LEN] = {0};
    char *workspace = NULL;
    int workspace_size = 0;
    char *action = "PROGRAM\0";
    int reset_jtag = 1;
    int error_address = 0L;
    int exit_code = 0;
    char *exit_string = NULL;
    int format_version = 0, action_count = 0, procedure_count = 0;

    /* Initialize the programming status count */
    count = 0;

    /* Do real-time ISP so the CPLD is still operation during programming
     * Without this, the CPLD pins will go tri-state and HRESET to CPU will
     * go low since it has external pull low resistor
     */
    init_list[0] = "do_real_time_isp=1";
    init_list[1] = NULL;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        /*
         *    Display file format version
         */
        jbi_get_file_info(cpld_fw_array, cpld_fw_size, (int *)&format_version,
                          (int *)&action_count, (int *)&procedure_count);

        printf("%s: File format is %s ByteCode format\n", __FUNCTION__,
               (format_version == 2) ? "Jam STAPL" : "pre-standardized Jam 1.1");

        /*
         *    Dump out NOTE fields
         */
        while (jbi_get_note(cpld_fw_array, cpld_fw_size, (int *)&offset, (char *)key,
                            (char *)value, 256) == 0) {
            printf("NOTE \"%s\" = \"%s\"\n", key, value);
        }

        /*
         *    Dump the action table
         */
        if ((format_version == 2) && (action_count > 0)) {
            printf("\nActions available in this file:\n");

            for (index = 0; index < action_count; ++index) {
                jbi_get_action_info(cpld_fw_array, cpld_fw_size, index, &action_name,
                                    &description, &procedure_list);

                if (description == NULL) {
                    printf("%s\n", action_name);
                } else {
                    printf("%s \"%s\"\n", action_name, description);
                }

                procptr = procedure_list;
                while (procptr != NULL) {
                    if (procptr->attributes != 0) {
                        printf("    %s (%s)\n", procptr->name,
                                (procptr->attributes == 1) ?
                                 "optional" : "recommended");
                    }

                    procedure_list = procptr->next;
                    free(procptr);
                    procptr = procedure_list;
                }
            }

            /* add a blank line before execution messages */
            printf("\n");
        }
    } /* diag_global_flag & DIAG_FLAG_VERBOSE_MODE */

    exec_result = jbi_execute(cpld_fw_array, cpld_fw_size, workspace,
                              workspace_size, action, init_list, reset_jtag,
                              &error_address, &exit_code, &format_version);

    if (exec_result == JBIC_SUCCESS) {
        if (format_version == 2) {
            switch (exit_code) {
                case  0:
                     exit_string = "Success";
                     retval = PASSED;
                     break;
                case  1: exit_string = "Checking chain failure"; break;
                case  2: exit_string = "Reading IDCODE failure"; break;
                case  3: exit_string = "Reading USERCODE failure"; break;
                case  4: exit_string = "Reading UESCODE failure"; break;
                case  5: exit_string = "Entering ISP failure"; break;
                case  6: exit_string = "Unrecognized device"; break;
                case  7: exit_string = "Device revision is not supported"; break;
                case  8: exit_string = "Erase failure"; break;
                case  9: exit_string = "Device is not blank"; break;
                case 10: exit_string = "Device programming failure"; break;
                case 11: exit_string = "Device verify failure"; break;
                case 12: exit_string = "Read failure"; break;
                case 13: exit_string = "Calculating checksum failure"; break;
                case 14: exit_string = "Setting security bit failure"; break;
                case 15: exit_string = "Querying security bit failure"; break;
                case 16: exit_string = "Exiting ISP failure"; break;
                case 17: exit_string = "Performing system test failure"; break;
                default: exit_string = "Unknown exit code"; break;
            }
        } else {
            switch (exit_code) {
                case 0:
                    exit_string = "Success";
                    retval = PASSED;
                    break;
                case 1: exit_string = "Illegal initialization values"; break;
                case 2: exit_string = "Unrecognized device"; break;
                case 3: exit_string = "Device revision is not supported"; break;
                case 4: exit_string = "Device programming failure"; break;
                case 5: exit_string = "Device is not blank"; break;
                case 6: exit_string = "Device verify failure"; break;
                case 7: exit_string = "SRAM configuration failure"; break;
                default: exit_string = "Unknown exit code"; break;
            }
        }

        printf("\nExit code = %d... %s\n", exit_code, exit_string);
        return (retval);
    } else if ((format_version == 2) &&
               (exec_result == JBIC_ACTION_NOT_FOUND)) {
        if ((action == NULL) || (*action == '\0'))
        {
            printf("%s: Error: no action specified for Jam STAPL file.",
                   __FUNCTION__);
        } else {
            printf("%s: Error: action \"%s\" is not supported for this "
                   "Jam STAPL file.", __FUNCTION__, action);
        }
    } else if (exec_result < MAX_ERROR_CODE) {
        printf("%s: Error at address %d: %s.", __FUNCTION__,
               error_address, error_text[exec_result]);
    } else {
        printf("%s: Unknown error code %#.8x", __FUNCTION__, exec_result);
    }

    if (workspace != NULL) {
        free(workspace);
    }

    return (FAILED);
}

/**********************************************************************
 *
 * Function: clock_verification_path_lib
 *
 * Description: The clock verification path library mainly set up the
 *              clock path from Overlord/Victory to the timing card.
 *              It requires Woodlawn NGSM installed in the platform
 *              to verify the clock or verify through O2 Cavium PHY.
 *
 * Input : clk_dest - ngsm slot or O2 cavium PHY
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long clock_verification_path_lib (uint clk_dest)
{
    int sel_freq;

    printf("\nBeaware not to power off NGVM after exit the timing card menu.\n");

    printf("Input 1: 8KHz, 2:25MHz, 3:125MHz\n");
    sel_freq  = getdec_answer("Select the output frequency:", 1, 1, 3);

    /* Configures the O2 dash fpga SYNC/TRIG Control Register
     * to select the clock frequency that outputs to timing card. */
    timingcard_o2_set_sync_trig_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL, FALSE);

    /* Perform the ZL3036X reference 0 clock input test to HPOUTCLK0 */
    if (timingcard_zl3036x_conf_ref_path(ZL3036X_REF_0, ZL3036X_HP_COMS_EN_0,
                                         sel_freq, TRUE, FALSE)
        == FAILED) {
        return (FAILED);
    }

    if (get_timingcard_sku_id() != SKU_30361) {
        /* 30361 SKU only has DPLL0 */
        /* Perform the ZL3036X reference 1 clock input test to HPOUTCLK2 */
        if (timingcard_zl3036x_conf_ref_path(ZL3036X_REF_1, ZL3036X_HP_COMS_EN_2,
                                             sel_freq, TRUE, FALSE)
            == FAILED) {
            return (FAILED);
        }
    }

    /* Configures the O2 dash fpga SYNC/TRIG Control Register
     * to select the clock frequency that outputs to ngsm card. */
    timingcard_o2_set_sm_sync_trig_out(clk_dest, sel_freq);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: clock_direction_lib
 *
 * Description: The clock verification path library mainly set up the
 *              clock path from Overlord/Victory to the timing card.
 *              It requires Woodlawn NGSM installed in the platform
 *              to verify the clock or verify through O2 Cavium PHY.
 *
 * Input : clk_dest - ngsm slot or O2 cavium PHY
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long clock_direction_lib (uint clk_dest)
{
    int sel_freq;

    printf("\nBeaware not to power off NGVM after exit the timing card menu.\n");

    sel_freq  = ZL3036X_8K_HZ;
    /* Configures the O2 dash fpga SYNC/TRIG Control Register
     * to select the clock frequency that outputs to timing card. */
    timingcard_o2_set_sync_trig_out(sel_freq, DASH_FPGA_SYNC_INTER_PLL, FALSE);

    /* Perform the ZL3036X reference 0 clock input test to HPOUTCLK0 */
    if (timingcard_zl3036x_conf_ref_path(ZL3036X_REF_0, ZL3036X_HP_COMS_EN_0,
                                         sel_freq, TRUE, FALSE)
        == FAILED) {
        return (FAILED);
    }

    /* Perform the ZL3036X reference 1 clock input test to HPOUTCLK2 */
    if (timingcard_zl3036x_conf_ref_path(ZL3036X_REF_1, ZL3036X_HP_COMS_EN_2,
                                         sel_freq, TRUE, FALSE)
        == FAILED) {
        return (FAILED);
    }

    /* Configures the O2 dash fpga SYNC/TRIG Control Register
     * to select the clock frequency that outputs to ngsm card. */
    timingcard_o2_set_sm_sync_trig_out(clk_dest, sel_freq);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_read_fn
 *
 * Read Timing Card CPLD Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         *buf - pointer to read buffer
 *         *param - pointer to param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int timingcard_cpld_read_fn (unsigned long addr, int size,
                                    unsigned long *buf, void *param)
{
    n2g_i2c_if_t *i2c_if = &cpld;
    int rc = PASSED;

    i2c_if->offset = (unsigned int)addr;
    i2c_if->buf = (char *)buf;

    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to read i2c, ret_code = %#x", rc);
        rc = FAILED;
        return (rc);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_write_fn
 *
 * Write Timing Card CPLD Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         data - data for write
 *         *param - pointer to param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int timingcard_cpld_write_fn (unsigned long addr, int size,
                                     unsigned long data, void *param)
{
    n2g_i2c_if_t *i2c_if = &cpld;
    int rc = PASSED;
    i2c_if->buf = (char *)&data;
    i2c_if->offset = (unsigned int)addr;
   
    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("unable to write i2c.\n");
        rc = FAILED;
        return (rc);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: display_embedded_cpld_fw_version
 *
 * Display the embedded CPLD firmware version
 *
 * Input : *cpld_rev - pointer to cpld revision buffer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long display_embedded_cpld_fw_version (uchar *cpld_rev)
{
    int ret_val = FAILED;
    int index = 0;
    long offset = 0L;
    char key[33] = {0};
    char *action_name = NULL;
    char *description = NULL;
    char usercode[MAX_NOTE_LEN];
    char value[MAX_NOTE_LEN] = {0};
    JBI_PROCINFO *procptr = NULL;
    JBI_PROCINFO *procedure_list = NULL;
    int format_version = 0, action_count = 0, procedure_count = 0;

    memset((char *)usercode, 0, sizeof(usercode));

    /* Display file format version */
    jbi_get_file_info(cpld_fw_array, cpld_fw_size, (int *)&format_version,
                      (int *)&action_count, (int *)&procedure_count);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: File format is %s ByteCode format\n", __FUNCTION__,
               (format_version == 2) ? "Jam STAPL" : "pre-standardized Jam 1.1");
    }

    /* Dump out NOTE fields */
    while (jbi_get_note(cpld_fw_array, cpld_fw_size, (int *)&offset, (char *)key,
                        (char *)value, 256) == 0)
    {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("NOTE \"%s\" = \"%s\"\n", key, value);
        }
        if (strcmp(key, "USERCODE") == 0) {
            strcpy(usercode, value);
            ret_val = PASSED;
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        /* Dump the action table */
        if ((format_version == 2) && (action_count > 0))
        {
            printf("\nActions available in this file:\n");

            for (index = 0; index < action_count; ++index) {
                jbi_get_action_info(cpld_fw_array, cpld_fw_size, index, &action_name,
                                    &description, &procedure_list);

                if (description == NULL) {
                    printf("%s\n", action_name);
                } else {
                    printf("%s \"%s\"\n", action_name, description);
                }

                procptr = procedure_list;
                while (procptr != NULL) {
                    if (procptr->attributes != 0) {
                        printf("    %s (%s)\n", procptr->name,
                                (procptr->attributes == 1) ?
                                 "optional" : "recommended");
                    }

                    procedure_list = procptr->next;
                    free(procptr);
                    procptr = procedure_list;
                }
            }

            /* add a blank line before execution messages */
            printf("\n");
        }
    }

    if (cpld_rev != NULL) {
        memcpy((char *)cpld_rev, usercode, IOCPLD_VER_LEN);
    }

    return (ret_val);
}

/**********************************************************************
 *
 * Function: init_timingcard
 *
 * This function initializes the timing card
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long init_timingcard (void)
{
    int set_val;
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&cpld;

    /* Set the NGIO GPIO Dir to output */
    set_val = 0;
    i2c_p->offset = CPLD_NGIO_EXPANDER_DIR;
    i2c_p->buf = (char *)&set_val;

    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("%s() unable to write i2c.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Set the ZL30363 Reset to active low */
    set_val = 0;
    i2c_p->offset = CPLD_NGIO_EXPANDER_OUTPUT;
    i2c_p->buf = (char *)&set_val;

    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("%s() unable to write i2c.\n", __FUNCTION__);
        return (FAILED);
    }

    /* ZL30363 set LOS0 = 0 (GPIO 0), LOS1 = 1 (GPIO 1) */
    if (timingcard_cpld_drive_gpio(CPLD_GPIO_1) == FAILED) {
        cterr('f', 0, "CPLD drive GPIO 0/1 fails");
        return (FAILED);
    }

    /* Set the ZL30363 Reset to active high */
    set_val = ZL30363_RESET_ACTIVE_HIGH;
    i2c_p->offset = CPLD_NGIO_EXPANDER_OUTPUT;
    i2c_p->buf = (char *)&set_val;

    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("%s() unable to write i2c.\n", __FUNCTION__);
        return (FAILED);
    }

    /* At least delay 125 ms */
    msleep(150);

    /* ZL30363 set LOS0 = 1 (GPIO 0), LOS1 = 1 (GPIO 1) */
    if (timingcard_cpld_drive_gpio(CPLD_GPIO_0 | CPLD_GPIO_1) == FAILED) {
        cterr('f', 0, "CPLD drive GPIO 0/1 fails");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: speedup_cpld_reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_reset_jtag(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0xFF, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_reset_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_reset_2_run(void)
{
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: erase_all
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void erase_all(int erase_addr)
{
    unsigned short w_data;

    printf("\nErase start address %#x.\n", erase_addr);
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    cpld_shift(erase_addr, 13, 1, 0);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_ERASE */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x013d here. */
    w_data = CPLD_ERASE_COMMAND;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    msleep(600);
}

/**********************************************************************
 *
 * Function: speedup_cpld_run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_run_2_ir(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0xC, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_shift
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static unsigned int cpld_shift(unsigned int data, unsigned char cnt,
                               unsigned char msb, boolean read_tdo)
{
    unsigned char ix;
    unsigned char tms;
    unsigned char tdi;
    unsigned char tdo;
    unsigned int rddata;


    rddata = 0;
    for(ix = 0; ix < cnt; ix++) {
        if(ix == (cnt - 1)) {
            tms = 1;
        } else {
            tms = 0;
        }

        if(msb == 1) {
            tdi = (data & (1 << (cnt - ix - 1))) > 0 ? 1:0;
        } else {
            tdi = (data & (1 << ix))> 0 ? 1:0;
        }

        /* Write TMS, TDI and read TDO */
        tdo = cpld_jtag_io(tms, tdi, read_tdo);

        /* read tdo */
        if(msb == 1) {
            rddata = (tdo != 0) ? (rddata | (1 << (cnt - ix - 1))) : rddata;
        } else {
            rddata = (tdo != 0) ? (rddata | ( 1 << ix)) : rddata;
        }
    }

    return (rddata);
}

/**********************************************************************
 *
 * Function: speedup_cpld_ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_ir_2_run(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x80, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_run_test_delay
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_test_delay(int delay_cycles)
{
    int ix;

    for (ix = 0; ix < delay_cycles; ix++) {
        cpld_jtag_io(0, 0, 0);
    }
}

/**********************************************************************
 *
 * Function: speedup_cpld_dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_dr_2_run(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x80, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_ir_2_dr(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x1C, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_run_2_dr(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x4, FALSE);
}

/**********************************************************************
 *
 * Function: timingcard_simply_program_cpld
 *
 * This function programs the cpld firmware via pca9557
 *
 * Input : *pof_start_address - pointer to the program firmware.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_simply_program_cpld (unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;

    memset((void *)silicon_id, 0, sizeof(silicon_id));

    /* reset */
    speedup_cpld_reset_jtag();

    /* reset to ir */
    cpld_reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("\nSample Preload.\n");

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0280 here. */
    w_data = CPLD_SAMPLE_PRELOAD;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 8 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 8 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("\nRT ISP Enable.\n");

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0266 here. */
    w_data = CPLD_ISP_ENABLE;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /**************************** read  silicon_id ****************************/
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();

    /* Speed up write data. */
    speed_up_cpld_jtag_io(CPLD_TEN, 0x12, FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, 0x20, FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        speedup_cpld_run_2_dr();
        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        silicon_id[ix] = r_data;
        speedup_cpld_dr_2_run();
    }

#ifdef DEBUG
    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");
#endif

    /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0000 for 5M40Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0100 for 5M80Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0200 for 5M160Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0300 for 5M240Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0400 for 5M570Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0500 for 5M1270Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0600 for 5M2210Z */

    /* Erase 0 */
    erase_all(0x0);
    /* Erase 1 */
    erase_all(0x1000);
    /* Erase 2 */
    erase_all(0x1100);

    /******************************** Program *********************************/
    printf("\nProgram.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_PROG */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x00bd here. */
    w_data = CPLD_ISC_PROG;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Start to program from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    /* 3328 */
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* write */
        speedup_cpld_run_2_dr();

        /* Speed up write data. */
        speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
        speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

        speedup_cpld_dr_2_run();

        /* Delay clock cycle which refers from LA waves of website sample code. */
        speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, FALSE);
    }

    /******************************** Verify *********************************/
    printf("\nVerify.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* w_data=0;
         * w_data=((*file_pt)<<8);
         * file_pt++;
         * w_data=w_data|(*file_pt);
         * file_pt++;
         */

        /* write */
        speedup_cpld_run_2_dr();;

        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        speedup_cpld_dr_2_run();

        if(r_data != w_data) {
            cpld_jtag_io(0, 0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
            return (FAILED);
        }

    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /***************************** Program Done ******************************/
    printf("\nSending Program Done.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_dr_2_run();

    /* LOAD ISC_PROG */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x00bd here. */
    w_data = CPLD_ISC_PROG;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();
    speedup_cpld_run_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0xffdf here. */
    w_data = CPLD_ISC_PROG_DONE_1;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    usleep(100);

    /* set add 0 */
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_dr_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    speedup_cpld_run_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0xFFFF here. */
    w_data = CPLD_ISC_PROG_DONE_2;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    speedup_cpld_dr_2_run();

    /**************************** RT ISP Disable *****************************/
    printf("\nRT ISP Disable.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_DISABLE */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x019a here. */
    w_data = CPLD_ISP_DISABLE;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* BYPASS */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x03ff here. */
    w_data = CPLD_BYPASS;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    cpld_run_test_delay(7);

    speedup_cpld_reset_jtag();

    printf("\nProgram Done.\n");

    return (PASSED);
}

/* Following codes is the original source code from Altera. */
#ifdef ORIGINAL_CPLD_PROGRAM
/**********************************************************************
 *
 * Function: cpld_reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_reset_jtag(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_2_ir(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_ir_2_run(void)
{
    /* Enter Update-IR from Exit2-IR */
    cpld_jtag_io(1, 0, 0);
    /* Enter Run from Update-IR */
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_ir_2_dr(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_dr_2_run(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_2_dr(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: speed_up_cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : reg_offset - cpld register offset
 *         cpld_w_buf - cpld write data value
 *         tdo_read - cpld read data value flag
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long speed_up_cpld_jtag_io (uchar reg_offset, uchar cpld_w_buf,
                                   boolean tdo_read)
{
    int ix;
    uchar cpld_r_buf = 0, rd_data;

    if (tdo_read == FALSE) {
        /* Write data value */
        if (timingcard_cpld_reg_write_lib(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }
    }

    /* Read data value */
    if (tdo_read == TRUE) {
        /* Write data value */
        if (timingcard_cpld_reg_write_lib(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }

        if (timingcard_cpld_reg_read_lib(CPLD_SPEED_UP, &rd_data) == FAILED) {
            return (FAILED);
        }

        /* Do MSB LSB bit swap. */
        for (ix = 0; ix < 8; ix++) {
            cpld_r_buf = ((rd_data & (0x1 << ix)) != 0) ? (cpld_r_buf | (1 << (7 - ix))) : cpld_r_buf;
        }
    }

    return (cpld_r_buf);
}

/**********************************************************************
 *
 * Function: timingcard_simply_program_cpld
 *
 * This function programs the cpld firmware via pca9557
 *
 * Input : *pof_start_address - pointer to the program firmware.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_simply_program_cpld (unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;

    /* reset */
    cpld_reset_jtag();

    /* reset to ir */
    cpld_reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("\nSample Preload.\n");

    /* run to ir */
    cpld_run_2_ir();

    /* ISC_DISABLE */
    cpld_shift(0x5, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x0, 1, 1, 0);

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x0, 1, 1, 0);
    cpld_dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("\nRT ISP Enable.\n");

    /* run to ir */
    cpld_run_2_ir();

    /* ISC_DISABLE */
    cpld_shift(0x199, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /**************************** read  silicon_id ****************************/
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();

    /* Speed up write data. */
    speed_up_cpld_jtag_io(CPLD_TEN, 0x12, FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, 0x20, FALSE);

    /* run */
    cpld_ir_2_run();

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x205, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        cpld_run_2_dr();
        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        silicon_id[ix] = r_data;
        cpld_dr_2_run();
    }

#ifdef DEBUG
    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");
#endif

    /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0000 for 5M40Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0100 for 5M80Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0200 for 5M160Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0300 for 5M240Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0400 for 5M570Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0500 for 5M1270Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0600 for 5M2210Z */

    /* Erase 0 */
    erase_all(0x0);
    /* Erase 1 */
    erase_all(0x1000);
    /* Erase 2 */
    erase_all(0x1100);

    /******************************** Program *********************************/
    printf("\nProgram.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_ir_2_run();

    /* LOAD ISC_PROG */
    cpld_run_2_ir();
    cpld_shift(0x2F4, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Start to program from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    /* 3328 */
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* write */
        cpld_run_2_dr();

        /* Speed up write data. */
        speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
        speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

        cpld_dr_2_run();

        /* Delay clock cycle which refers from LA waves of website sample code. */
        speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, FALSE);
    }

    /******************************** Verify *********************************/
    printf("\nVerify.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_ir_2_run();

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x205, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* w_data=0;
         * w_data=((*file_pt)<<8);
         * file_pt++;
         * w_data=w_data|(*file_pt);
         * file_pt++;
         */

        /* write */
        cpld_run_2_dr();

        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        cpld_dr_2_run();

        if(r_data != w_data) {
            cpld_jtag_io(0, 0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
            return (FAILED);
        }

    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /***************************** Program Done ******************************/
    printf("\nSending Program Done.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10 ,0 , 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_dr_2_run();

    /* LOAD ISC_PROG */
    cpld_run_2_ir();
    cpld_shift(0x2F4, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    cpld_run_2_dr();
    w_data = 0xFFDF;
    r_data = cpld_shift(w_data, 16, 1, 0);            //0xFBFF,0x7BFF

    cpld_dr_2_run();

    usleep(100);

    /* set add 0 */
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_dr_2_run();

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x205, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    cpld_run_2_dr();
    r_data = cpld_shift(0xFFFF, 16, 1, 0);

    cpld_dr_2_run();

    /**************************** RT ISP Disable *****************************/
    printf("\nRT ISP Disable.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_DISABLE */
    cpld_shift(0x166, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /* run to ir */
    cpld_run_2_ir();

    /* BYPASS */
    cpld_shift(0x3FF, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    cpld_run_test_delay(7);

    cpld_reset_jtag();

    printf("\nProgram Done.\n");

    return (PASSED);
}
#endif

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_cpld_lib.c,v $
 * Revision 1.3  2017/07/14 02:51:39  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.9  2014/04/22 06:06:03  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.8  2014/03/19 07:13:50  kodko
 * Speed up the CPLD firmware upgrade time under 2 minutes.
 *
 * Revision 1.1.2.7  2014/03/11 03:56:11  leschen
 * Miss argument for setting sync trig out function.
 *
 * Revision 1.1.2.6  2014/03/11 03:14:37  leschen
 * Fix setting timingcard clk/trig function.
 *
 * Revision 1.1.2.5  2014/03/10 08:00:10  kodko
 * Remove redundant code.
 *
 * Revision 1.1.2.4  2014/03/07 07:39:58  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.3  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */


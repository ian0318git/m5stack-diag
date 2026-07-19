/* $Id: vm_timingcard_pca9557_lib.c,v 1.3 2017/07/14 02:51:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_pca9557_lib.c,v $
 *******************************************************************************
 * File Name: vm_timingcard_pca9557_lib.c
 *
 * Description: NGVM Timing Card PCA9558 library source file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2014 - 2017 by Cisco Systems, Inc.
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
#include "vm_timingcard_pca9557_lib.h"
#include "vm_timingcard_cpld_lib.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/
#define PCA9557_RONLY    (READ_ONLY | REG_ACCESS)
#define PCA9557_RW       (READ_WRITE | REG_ACCESS)

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static int timingcard_pca9557_read_fn(unsigned long, int, unsigned long *, void *);
static int timingcard_pca9557_write_fn(unsigned long, int, unsigned long, void *);
static unsigned char pca9557_jtag_io(char,char);
static void reset_jtag(void);
static void reset_2_run(void);
static void run_2_ir(void);
static unsigned int shift(unsigned int,unsigned char,unsigned char);
static void ir_2_dr(void);
static void ir_2_run(void);
static void run_2_dr(void);
static void dr_2_run(void);
static void run_test_delay(int);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long timingcard_pca9557_init(void);
long timingcard_pca9557_program_cpld(unsigned char *);
long util_oir_pca9557_reg_write(void);
long util_oir_pca9557_reg_read(void);
long timingcard_pca9557_reg_test_lib(void);
long timingcard_pca9557_i2c_w(uint, uchar);
long timingcard_pca9557_i2c_r(uint, uchar *);
long timingcard_pca9557_power_cycle_cpld(void);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

static n2g_i2c_if_t pca9557 =
{
    .dev_name = "Timing Card PCA9557",
    .offset = 0,
    .i2c_bus_type = IOFPGA_I2C,
    .size = 0x1,
    .mux = I2C_MUX_ZERO,
    .buf = NULL,
    /* Dash FPGA I2C Controller 15 is used for NGVM, refer to O2 HFS. */
    .i2c_ctrl = I2C_CTRL_FIFTEEN,
    /* Timing Card CPLD I2C 7-bits address */
    .i2c_dev = TIMING_CARD_PCA9557_I2C_ADDR,
};

static reg_info_t_ext pca9557_reg_ext = {1, timingcard_pca9557_read_fn,
                                         timingcard_pca9557_write_fn, 0};

static reg_info_t pca9557_test_regs[] = {
    {"Input Port Register", PCA9557_NGIO_EXPANDER_INPUT,
     PCA9557_RONLY, {(unsigned long)&pca9557_reg_ext}, 0x0, 0x0},
    {"Output Port Register", PCA9557_NGIO_EXPANDER_OUTPUT,
     PCA9557_RONLY, {(unsigned long)&pca9557_reg_ext}, 0x0, 0x0},
    {"Polarity Inversion Register", PCA9557_POLARITY_INVERSION,
     PCA9557_RONLY, {(unsigned long)&pca9557_reg_ext}, 0x0, 0xf0},
    {"Configuration Register", PCA9557_CONFIGURATION,
     PCA9557_RW, {(unsigned long)&pca9557_reg_ext}, 0xff, 0xff},
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},
};

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: timingcard_pca9557_read_fn
 *
 * Read Timing Card PCA9557 Register.
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
static int timingcard_pca9557_read_fn (unsigned long addr, int size,
                                       unsigned long *buf, void *param)
{
    n2g_i2c_if_t *i2c_if = &pca9557;
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
 * Function: timingcard_pca9557_write_fn
 *
 * Write Timing Card PCA9557 Register.
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
static int timingcard_pca9557_write_fn (unsigned long addr, int size,
                                     unsigned long data, void *param)
{
    n2g_i2c_if_t *i2c_if = &pca9557;
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
 * Function: timingcard_pca9557_i2c_w
 *
 * This common function for the I2C write
 *
 * Input : reg_offset - register offset
 *         pca9557_wbuf - write buffer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_pca9557_i2c_w (uint reg_offset, uchar pca9557_wbuf)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&pca9557;

    i2c_p->offset = reg_offset;
    i2c_p->buf = (char *)&pca9557_wbuf;

    /* Alter reg with new value */
    i2c_p->buf = (char *)&pca9557_wbuf;
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("Unable to write i2c offset %#x value is %#x.\n",
                reg_offset, pca9557_wbuf);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_i2c_r
 *
 * This common function for the I2C read
 *
 * Input : reg_offset - register offset
 *         *pca9557_rbuf - pointer of read buffer
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_pca9557_i2c_r (uint reg_offset, uchar *pca9557_rbuf)
{
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&pca9557;

    /* Read reg with new value */
    i2c_p->offset = reg_offset;
    i2c_p->buf = (char *)pca9557_rbuf;

    if (n2g_i2c_read(i2c_p) != PASSED) {
        printf("Unable to read i2c offset %#x value is %#x.\n",
                reg_offset, *pca9557_rbuf);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_init
 *
 * This function initialized the PCA9557.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_pca9557_init (void)
{
    uchar cpld_write_buf;

    /* Configure the Polarity Inversion Register as 0. */
    cpld_write_buf = 0;
    if (timingcard_pca9557_i2c_w(PCA9557_POLARITY_INVERSION, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    if (timingcard_pca9557_i2c_r(PCA9557_POLARITY_INVERSION, &cpld_write_buf)
        == FAILED) {
        return (FAILED);
    }

    /* Configure the GPIO 5 as INPUT, others as OUTPUT. */
    cpld_write_buf = (PCA9557_TDO_GPIO5_INPUT | PCA9557_TDI_GPIO2_OUTPUT);
    if (timingcard_pca9557_i2c_w(PCA9557_CONFIGURATION, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: pac9557_jtag_io
 *
 * This function provide the jtag io function via pca9557
 *
 * Input : tms
 *         tdi
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static unsigned char pca9557_jtag_io (char tms,char tdi)
{
    uchar out_data=0;
    uchar in_data;

    if(tms != 0) {
        /* tms */
        out_data |= PCA9557_TMS_GPIO7_OUTPUT;
    }

    if(tdi != 0) {
        /* tdi */
        out_data |= PCA9557_TDI_GPIO4_OUTPUT;
    }

    /* tck = 0 */
    if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, out_data)
        == FAILED) {
        return (FAILED);
    }

    /* read tdo */
    if (timingcard_pca9557_i2c_r(PCA9557_NGIO_EXPANDER_INPUT, &in_data)
        == FAILED) {
        return (FAILED);
    }

    /* tdo = 1? write 1 to uart */
    if((in_data & PCA9557_TDO_GPIO5_INPUT) >= 1) {
        in_data = 0x01;
    } else {
        in_data = 0x00;
    }


    /* tck = 1 */
    out_data |= PCA9557_TCK_GPIO6_OUTPUT;
    if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, out_data)
        == FAILED) {
        return (FAILED);
    }

    /* tck = 0 */
    out_data &= ~PCA9557_TCK_GPIO6_OUTPUT;
    if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, out_data)
        == FAILED) {
        return (FAILED);
    }

    return (in_data);
}

/**********************************************************************
 *
 * Function: reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void reset_jtag(void)
{
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
}

/**********************************************************************
 *
 * Function: reset_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void reset_2_run(void)
{
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void ir_2_run(void)
{
    /* Enter Pause-IR from Exit1-IR */
    pca9557_jtag_io(0, 0);
    /* Enter Exit2-IR from Pause-IR */
    pca9557_jtag_io(1, 0);
    /* Enter Update-IR from Exit2-IR */
    pca9557_jtag_io(1, 0);
    /* Enter Run from Update-IR */
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void run_2_ir(void)
{
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void ir_2_dr(void)
{
    /* Enter Pause-IR from Exit1-IR */
    pca9557_jtag_io(0, 0);
    /* Enter Exit2-IR from Pause-IR */
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void run_2_dr(void)
{
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void dr_2_run(void)
{
    /* Enter Pause-IR from Exit1-IR */
    pca9557_jtag_io(0, 0);
    /* Enter Exit2-IR from Pause-IR */
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(1, 0);
    pca9557_jtag_io(0, 0);
}

/**********************************************************************
 *
 * Function: run_test_delay
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void run_test_delay(int delay_times)
{
    int ix;

    for (ix = 0; ix < delay_times; ix++) {
        pca9557_jtag_io(0, 0);
    }
}

/**********************************************************************
 *
 * Function: shift
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
unsigned int shift(unsigned int data, unsigned char cnt, unsigned char msb)
{
    unsigned char i;
    unsigned char tms;
    unsigned char tdi;
    unsigned char tdo;
    unsigned int rddata;


    rddata = 0;
    for(i = 0; i < cnt; i++) {
        if(i == (cnt-1)) {
            tms = 1;
        } else {
            tms = 0;
        }

        if(msb == 1) {
            tdi = (data & (1 << (cnt - i - 1))) > 0 ? 1:0;
        } else {
            tdi = (data & (1 << i))> 0 ? 1:0;
        }

        /* write tdi,tms */
        tdo = pca9557_jtag_io(tms, tdi);

        /* read tdo */
        if(msb == 1) {
            rddata = (tdo != 0) ? (rddata | (1 << (cnt - i - 1))) : rddata;
        } else {
            rddata = (tdo != 0) ? (rddata | ( 1 << i)) : rddata;
        }
    }

    return (rddata);

}

/**********************************************************************
 *
 * Function: timingcard_pca9557_program_cpld
 *
 * This function programs the cpld firmware via pca9557
 *
 * Input : *pof_start_address - pointer to the program firmware.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_pca9557_program_cpld (unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;

    memset((void *)silicon_id, 0, sizeof(silicon_id));

    /* reset */
    reset_jtag();

    /* reset to ir */
    reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("Sample Preload.\n");

    /* run to ir */
    run_2_ir();

    /* ISC_DISABLE */
    shift(0x5,10,0);

    /* run */
    ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(4);

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x0, 1, 1);
    dr_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x0, 1, 1);
    dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("RT ISP Enable.\n");

    /* run to ir */
    run_2_ir();

    /* ISC_DISABLE */
    shift(0x199,10,0);

    /* run */
    ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(4);

    /**************************** read  silicon_id ****************************/
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203, 10, 0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x1220, 13, 1);

    /* run */
    ir_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x205, 10, 0);

    /* run */
    ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        run_2_dr();
        r_data = shift(0xFFFF, 16, 1);
        silicon_id[ix] = r_data;
        dr_2_run();
    }

#ifdef DEBUG
    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");
#endif

    return (PASSED);

    /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0000 for 5M40Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0100 for 5M80Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0200 for 5M160Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0300 for 5M240Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0400 for 5M570Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0500 for 5M1270Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0600 for 5M2210Z */

    /******************************** Erase 0 *********************************/
    printf("Erase 0.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x0,13,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_ERASE */
    run_2_ir();
    shift(0x2F2,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    /******************************** Erase 1 *********************************/
    printf("Erase 1.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x1000,13,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_ERASE */
    run_2_ir();
    shift(0x2F2,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    /******************************** Erase 2 *********************************/
    printf("Erase 2.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x1100,13,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_ERASE */
    run_2_ir();
    shift(0x2F2,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    /******************************** Program *********************************/
    printf("Program.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x0,13,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_PROG */
    run_2_ir();
    shift(0x2F4,10,0);

    /* run */
    ir_2_run();

    /* Start to program from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    /* 3328 */
    for(ix = 0; ix < 3328; ix++) {
        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* write */
        run_2_dr();

        r_data = shift(w_data, 16, 1);

        dr_2_run();

        /* Delay 3 clock cycle which refers from LA waves of website sample code. */
        run_test_delay(3);
    }

    /******************************** Verify *********************************/
    printf("Verify.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x0,13,1);

    /* run */
    ir_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x205,10,0);

    /* run */
    ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    for(ix = 0; ix < 3328; ix++) {
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
        run_2_dr();
        r_data = shift(0xFFFF, 16, 1);

        dr_2_run();

        if(r_data != w_data) {
            pca9557_jtag_io(0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
        }

    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    /***************************** Program Done ******************************/
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x0,13,1);

    /* run */
    dr_2_run();

    /* LOAD ISC_PROG */
    run_2_ir();
    shift(0x2F4,10,0);

    /* run */
    ir_2_run();

    run_2_dr();
    w_data = 0xFFDF;
    r_data = shift(w_data, 16, 1);            //0xFBFF,0x7BFF
    dr_2_run();

    usleep(100);

    /* set add 0 */
    /* run to ir */
    run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    shift(0x203,10,0);

    /* LOAD Address in DR */
    ir_2_dr();
    shift(0x0,13,1);

    /* run */
    dr_2_run();

    /* LOAD ISC_READ */
    run_2_ir();
    shift(0x205,10,0);

    /* run */
    ir_2_run();

    run_2_dr();
    r_data = shift(0xFFFF,16,1);

    dr_2_run();

    /**************************** RT ISP Disable *****************************/
    printf("RT ISP Disable.\n");
    /* run to ir */
    run_2_ir();

    /* ISC_DISABLE */
    shift(0x166,10,0);

    /* run */
    ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    run_test_delay(7);

    /* run to ir */
    run_2_ir();

    /* BYPASS */
    shift(0x3FF,10,0);

    /* run */
    ir_2_run();

    run_test_delay(7);

    reset_jtag();

    printf("Program Done.\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_reg_test_lib
 *
 * This function perform the PCA9557 register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_pca9557_reg_test_lib (void)
{
    /* Perform the CPLD register test. */
    if (register_tests(0, &pca9557_test_regs[0]) == FAILED) {
        cterr('f', 0, "PCA9557 Register Test Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_oir_pca9557_reg_write
 *
 * Wrapper for PCA9557 Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_pca9557_reg_write (void)
{
    int rc = PASSED;
    uint8_t cpld_write_buf;
    uint write_addr;
    n2g_i2c_if_t *i2c_p = (n2g_i2c_if_t *)&pca9557;

    printf("\n\nCPLD Register Write\n\n");

    write_addr = gethex_answer("Reg offset to write", 0, 0, 0x3);
    i2c_p->offset = write_addr;

    cpld_write_buf = gethex_answer("Enter new reg value", 0x0, 0, 0xff);

    i2c_p->buf = (char *)&cpld_write_buf;

    /* alter reg with new value */
    rc = n2g_i2c_write(i2c_p);
    if (rc != PASSED) {
        printf("Unable to write i2c.\n");
        return (FAILED);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: util_oir_pca9557_reg_read
 *
 * Wrapper for PCA9557 Register Display utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_pca9557_reg_read (void)
{
    if (register_display(0, &pca9557_test_regs[0]) == FAILED) {
        cterr('f', 0, "PCA9557 Register Display Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_pca9557_power_cycle_cpld
 *
 * This function supports the PCA9557 to power cycle CPLD.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_pca9557_power_cycle_cpld (void)
{
    uchar cpld_write_buf;

    printf("\nPOWER-CYCLE the CPLD to get new CPLD image running...\n");

    /* Step 1: Set output high on IO2 */
    cpld_write_buf = 0x4;
    if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 2: Set IO2 as output */
    cpld_write_buf = 0xFB;
    if (timingcard_pca9557_i2c_w(PCA9557_CONFIGURATION, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 3: Power cycle the CPLD. */
    if (timingcard_cpld_power_ctl() == FAILED) {
        printf("Current CPLD firmware is not responding.\n");
    }

    /* Need to delay at least 10 ms after step 3. */
    msleep(15);

    /* Step 4: Turn off 1.8V CPLD */
    cpld_write_buf = 0x0;
    if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    /* Step 5: Turn on 1.8V CPLD */
    cpld_write_buf = 0x4;
    if (timingcard_pca9557_i2c_w(PCA9557_NGIO_EXPANDER_OUTPUT, cpld_write_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_pca9557_lib.c,v $
 * Revision 1.3  2017/07/14 02:51:39  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2  2015/02/14 12:48:43  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.3  2014/03/07 10:28:57  kodko
 * Remove manually Power-Cycle message.
 *
 * Revision 1.1.2.2  2014/03/07 07:39:59  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.1  2014/02/24 09:02:44  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

/* $Id: vm_timingcard_zl3036x_lib.c,v 1.3 2015/02/18 06:08:26 bowang3 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_zl3036x_lib.c,v $
 *******************************************************************************
 * File Name: vm_timingcard_zl3036x_lib.c
 *
 * Description: NGVM Timing Card ZL3036X library source file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
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
#include "vm_timingcard.h"
#include "dash_fpga.h"
#include "vm_timingcard_zl3036x_lib.h"
#include "vm_timingcard_zl3036x_diag.h"
#include "vm_timingcard_cpld_diag.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/
#define ZL3036X_RONLY    (READ_ONLY | REG_ACCESS)
#define ZL3036X_RW       (READ_WRITE | REG_ACCESS)

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern int get_timingcard_sku_id(void);

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static int timingcard_zl3036x_read_fn(unsigned long, int, unsigned long *, void *);
static int timingcard_zl3036x_write_fn(unsigned long, int, unsigned long, void *);
static int show_debug(n2g_i2c_if_t *, int);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long timingcard_zl3036x_reg_test_lib(void);
long timingcard_zl3036x_ref_x_lib(uint, uint);
long util_oir_zl3036x_reg_read(void);
long util_oir_zl3036x_reg_write(void);
long zl3036x_sel_page(n2g_i2c_if_t *, uint);
void timingcard_o2_set_sync_trig_out(int, int, boolean);
void timingcard_o2_disable_sync_trig_out(void);
void timingcard_o2_sync_trig_out_ctrl(boolean);
void timingcard_o2_set_sync1_out(int, int);
void timingcard_o2_disable_sync1_out(void);
long zl3036x_configure_ref_clock(n2g_i2c_if_t *, uint, uint, boolean, boolean);
long timingcard_zl3036x_ref_x_lpbk_lib(uint, uint, boolean, boolean);
long select_page_no(n2g_i2c_if_t *, uint);
long sel_zl3036x_ref_clock_lib(void);
long zl3036x_set_gpio_dir(uint, uint);
long zl3036x_check_gpio_value(uint, uint, uchar, uint);
long zl3036x_set_gpio_value(uint, uchar);
void timingcard_o2_set_sm_sync_trig_out(uint, uint);
long timingcard_zl3036x_conf_ref_path(uint, uint, uint, boolean, boolean);
long timingcard_zl3036x_sticky_read(uint32, uchar *, int);
long timingcard_zl3036x_outx_lpbk_dpll_check(uint, uint, boolean);
long timingzard_zl30361_ref2_check(uchar *);

static reg_info_t_ext zl3036x_reg_ext = {1, timingcard_zl3036x_read_fn,
                                         timingcard_zl3036x_write_fn, 0};

static reg_info_t zl3036x_test_regs[] = {
    {"ref_fail_isr_mask_10_8", ZL3036X_REF_FAIL_ISR_MASK_10_8, ZL3036X_RW,
        {(unsigned long)&zl3036x_reg_ext}, 0xf8, 0x0},
    {"dpll_isr_mask", ZL3036X_DPLL_ISR_MASK, ZL3036X_RW,
        {(unsigned long)&zl3036x_reg_ext}, 0xf0, 0x0},
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},
};

/**********************************************************************
 *
 * Function: zl3036x_sel_page
 *
 * Wrapper for ZL3036X select the current page.
 *
 * Input : pointer of i2c device structure
 *         page_value - select page value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long zl3036x_sel_page (n2g_i2c_if_t *zl_i2c_p, uint page_value)
{
    zl_i2c_p->offset = ZL3036X_PAGE_SEL_REGISTER;
    zl_i2c_p->buf = (char *)&page_value;

    /* Select the page number */
    if (n2g_i2c_write(zl_i2c_p) != PASSED) {
        cterr('f', 0, "Unable to write i2c while select the page.");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: zl3036x_sel_page
 *
 * Wrapper for ZL3036X select the reference clock source.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long sel_zl3036x_ref_clock_lib (void)
{
    uchar zl3036x_buf;
    uint ref_number;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Call the select page number function. */
    if (zl3036x_sel_page(i2c_p, ZL3036X_REG_PAGE_1) == FAILED) {
        return (FAILED);
    }

    ref_number = gethex_answer("Select reference clock 0/1", 0, 0, 0x1);

    /* Select DPLL reference clock */
    if (ref_number == ZL3036X_REF_0) {
        /* Configures the DPLL0 mode for the reference 0 clock input */
        zl3036x_buf = (ZL3036X_DPLL_FORCE_LOCK_MODE | ZL3036X_DPLL_SEL_REF0);
    } else {
        /* Configures the DPLL0 mode for the reference 1 clock input */
        zl3036x_buf = (ZL3036X_DPLL_FORCE_LOCK_MODE | ZL3036X_DPLL_SEL_REF1);
    }
    i2c_p->offset = ZL3036X_DPLL0_MODE_REFSEL;
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: zl3036x_set_gpio_dir
 *
 * Wrapper for ZL3036X sets the gpio direction.
 *
 * Input : reg_offset - register offset
 *         assign_vale - assigned value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long zl3036x_set_gpio_dir (uint reg_offset, uint assign_val)
{
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Call the select page number function. */
    if (zl3036x_sel_page(i2c_p, ZL3036X_REG_PAGE_4) == FAILED) {
        return (FAILED);
    }

    /* Configures the DPLL0 mode for the reference 0 clock input */
    i2c_p->offset = reg_offset;
    i2c_p->buf = (char *)&assign_val;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: zl3036x_check_gpio_value
 *
 * Wrapper for ZL3036X check the gpio value
 *
 * Input : gpio_no    - gpio number
 *         check_mask - check gpio value mask
 *         reg_offset - register offset
 *         check_sts  - the gpio status that should be
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long zl3036x_check_gpio_value (uint gpio_no, uint reg_offset, uchar check_mask,
                               uint check_sts)
{
    int rc = PASSED;
    uchar buffer;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Call the select page number function. */
    if (zl3036x_sel_page(i2c_p, ZL3036X_REG_PAGE_4) == FAILED) {
        return (FAILED);
    }

    /* Configures register offset and the value */
    i2c_p->offset = reg_offset;
    i2c_p->buf = (char *)&buffer;

    /* Alter reg with new value */
    if (n2g_i2c_read(i2c_p) != PASSED) {
        printf("unable to read i2c.\n");
        return (FAILED);
    }

    /* Check the GPIO status */
    if (!check_sts) {
        /* Check the GPIO low status */
        if ((~check_mask) & buffer) {
            rc = FAILED;
        }
    } else {
        /* Check the GPIO high status */
        if (!(check_mask & buffer)) {
            rc = FAILED;
        }
    }

    if (rc == FAILED) {
        cterr('f', 0, "Check GPIO %d offset %#.2x value failed, buffer is %#.2x "
              "check mask is %#.2x", gpio_no, reg_offset, buffer, check_mask);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: zl3036x_set_gpio_value
 *
 * Wrapper for ZL3036X sets the gpio value
 *
 * Input : reg_offset - register offset
 *         set_val - set value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long zl3036x_set_gpio_value (uint reg_offset, uchar set_val)
{
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Call the select page number function. */
    if (zl3036x_sel_page(i2c_p, ZL3036X_REG_PAGE_4) == FAILED) {
        return (FAILED);
    }

    /* Configures register offset and the value */
    i2c_p->offset = reg_offset;
    i2c_p->buf = (char *)&set_val;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_oir_zl3036x_reg_read
 * Note that:
 * 1. User should wait at least 25ms between two write accesses to the
 *    same register
 *
 * 2. For the page selection register (at addresses 0x07F, 0x0FF, 0x17F,
 *    0x1FF, 0x27F and 0x2FF), there is no waiting time required between
 *    write accesses.
 *
 * Wrapper for ZL3036X Register Display utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_zl3036x_reg_read (void)
{
    int rc = PASSED;
    uchar zl3036x_buf;
    uint reg_addr, page_value;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    printf("\n\nZL3036X Register Read\n\n");

    page_value = gethex_answer("Select page number (0-4) to read register",
                               0, 0, 0x4);

    reg_addr = gethex_answer("Reg offset to read", 0, 0, 0x2ff);

    /* Need to call sticky read to read register. */
    if (getc_answer("Sticky read?", "yn", 'y') == 'y') {
        rc = timingcard_zl3036x_sticky_read(reg_addr, &zl3036x_buf, page_value);
    } else {
        /* Call the select page number function. */
        if (zl3036x_sel_page(i2c_p, page_value) == FAILED) {
            return (FAILED);
        }

        i2c_p->offset = reg_addr;
        i2c_p->buf = (char *)&zl3036x_buf;
        rc = n2g_i2c_read(i2c_p);
    }

    if (rc != PASSED) {
        printf("Unable to read i2c, ret_code = %#x\n", rc);
        rc = FAILED;
    } else {
        printf("\nRegister @ %#x = %#x\n", reg_addr, zl3036x_buf);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: util_oir_zl3036x_reg_write
 * Note that:
 * 1. User should wait at least 25ms between two write accesses to the
 *    same register
 *    - The dplln_df_offset registers can be written with a minimum wait
 *      time of 300 microseconds between write accesses to the same register.
 *
 * 2. For the page selection register (at addresses 0x07F, 0x0FF, 0x17F,
 *    0x1FF, 0x27F and 0x2FF), there is no waiting time required between
 *    write accesses.
 *
 * Wrapper for ZL3036X Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_zl3036x_reg_write (void)
{
    int rc = PASSED;
    uint8_t zl3036x_write_buf;
    uint write_addr, page_value;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    printf("\n\nZL3036X Register Write\n\n");
    page_value = gethex_answer("Select page number (0-4) to read register",
                               0, 0, 0x4);
    /* Call the select page number function. */
    if (zl3036x_sel_page(i2c_p, page_value) == FAILED) {
        return (FAILED);
    }

    write_addr = gethex_answer("Reg offset to write", 0, 0, 0x2ff);
    i2c_p->offset = write_addr;

    zl3036x_write_buf = gethex_answer("Enter new reg value", 0x0, 0, 0xff);

    i2c_p->buf = (char *)&zl3036x_write_buf;

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
 * Function: timingcard_zl3036x_reg_test_lib
 *
 * This function perform the ZL3036X register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_zl3036x_reg_test_lib (void)
{
    uchar zl3036x_buf;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Before perform the register test, need to make the
     * page_sel_register (0x07F) register page value as 0 */
    zl3036x_buf = ZL3036X_REG_PAGE_0;
    i2c_p->offset = ZL3036X_PAGE_SEL_REGISTER;
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    /* Perform the ZL3036X register test. */
    if (register_tests(0, zl3036x_test_regs) == FAILED) {
        cterr('f', 0, "ZL3036X Register Test Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_o2_set_sync_trig_out
 *
 * This function configures the O2 dash fpga SYNC/TRIG Control Register
 * to select the clock frequency that outputs to timing card.
 *
 * Input : freq - clock frequency
 *         clock_source - clock source
 *         sel_1pps - 1PPS select flag.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void timingcard_o2_set_sync_trig_out (int freq, int clock_source,
                                      boolean sel_1pps)
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out_trig_out;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    sync_out_trig_out = (volatile uint32_t *)(fpga_addr + NGVM_SYNC_OUT_SYNC_TRIG_OUT);

    if (freq == ZL3036X_8K_HZ) {
        if (sel_1pps == TRUE) {
            /* Set Dash FPGA SYNC/TRIG Control Register as 8kHz output.
             * Enable SYNC_TRGI_OUT and set SYNC_TRIG_OUT as NGVM,
             * Enable PreScaler (div by 3125), Enable the SYNC_OUT,
             * Internal (FPGA) PLL */
            *sync_out_trig_out = (DASH_FPGA_TRIG_OUTPUT_EN | DASH_FPGA_TRIG_NGVM |
                                  DASH_FPGA_DIV_BY_3125 | DASH_FPGA_SYNC_OUTPUT_EN |
                                  clock_source);
        } else {
            /* Set Dash FPGA SYNC/TRIG Control Register as 8kHz output.
             * Enable PreScaler (div by 3125), Enable the SYNC_OUT,
             * Internal (FPGA) PLL */
            *sync_out_trig_out = (DASH_FPGA_DIV_BY_3125 | DASH_FPGA_SYNC_OUTPUT_EN |
                                  clock_source);
        }
    } else {
        /* Set Dash FPGA SYNC/TRIG Control Register as 25MHz output.
         * Enable PreScaler (div by 5), Enable the SYNC_OUT,
         * Internal (FPGA) PLL */
        *sync_out_trig_out = (DASH_FPGA_SYNC_OUTPUT_EN | clock_source);
    }
}

/**********************************************************************
 *
 * Function: timingcard_o2_set_sm_sync_trig_out
 *
 * This function configures the O2 dash fpga SYNC/TRIG Control Register
 * to select the clock frequency that outputs to NGSM card.
 *
 * Input : clk_dest - ngsm slot number or O2 Cavium PHY
 *         uint sel_freq - select frequency
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void timingcard_o2_set_sm_sync_trig_out (uint clk_dest, uint sel_freq)
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out_trig_out;
    uint32_t value;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    if (clk_dest == NGSM_SLOT_ONE) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGSM1_SYNC_OUT_SYNC_TRIG_OUT);
    } else if (clk_dest == NGSM_SLOT_TWO) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGSM2_SYNC_OUT_SYNC_TRIG_OUT);
    } else if (clk_dest == NGWIC_SLOT_ONE) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGWIC1_SYNC_OUT_SYNC_TRIG_OUT);
    } else if (clk_dest == NGWIC_SLOT_TWO) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGWIC2_SYNC_OUT_SYNC_TRIG_OUT);
    } else if (clk_dest == NGWIC_SLOT_THREE) {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             NGWIC3_SYNC_OUT_SYNC_TRIG_OUT);
    } else {
        sync_out_trig_out = (volatile uint32_t *)(fpga_addr +
                             QUAD_PHY_SYNC_TRIG_CTRL_REG);
    }

    if (clk_dest == OVERLORD_CAVIUM) {
        /* Set Dash FPGA Quad PHY PTP/TRIG control register */
        value = (DASH_FPGA_TRIG_OUTPUT_EN | (DASH_FPGA_SEL_NGVM << 16) |
            DASH_FPGA_PTP_CLK_OUT_EN | DASH_FPGA_SEL_NGVM);
    } else {
        /* Set Dash FPGA SYNC/TRIG Control Register as NGVM output.
        * Enable the SYNC_OUT, NGSM1/NGSM2 */
        value = (DASH_FPGA_TRIG_OUTPUT_EN | (DASH_FPGA_SEL_NGVM << 16) |
            DASH_FPGA_SYNC_OUTPUT_EN | DASH_FPGA_SEL_NGVM);
    }

    *sync_out_trig_out = value;
}

/**********************************************************************
 *
 * Function: timingcard_o2_disable_sync_trig_out
 *
 * This function disable the O2 dash fpga SYNC/TRIG Control Register
 * to select the clock frequency that outputs to timing card.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void timingcard_o2_disable_sync_trig_out (void)
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out_trig_out;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    sync_out_trig_out = (volatile uint32_t *)(fpga_addr + NGVM_SYNC_OUT_SYNC_TRIG_OUT);

    /* Restore the sync_out control register as original value */
     *sync_out_trig_out &= ~(DASH_FPGA_SYNC_OUTPUT_EN);
}

/**********************************************************************
 *
 * Function: timingcard_o2_sync_trig_out_ctrl
 *
 * This function enable/disable the trigger the O2 dash fpga SYNC/TRIG Control
 * Register.
 *
 * Input : on_off - turn on or off
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void timingcard_o2_sync_trig_out_ctrl (boolean on_off)
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out_trig_out;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    sync_out_trig_out = (volatile uint32_t *)(fpga_addr + NGVM_SYNC_OUT_SYNC_TRIG_OUT);

    if (on_off == TRUE) {
        /* Enable the sync_out control register as original value */
         *sync_out_trig_out |= (DASH_FPGA_TRIG_OUTPUT_EN | DASH_FPGA_TRIG_SELECT);
    } else {
        /* Disable the sync_out control register as original value */
         *sync_out_trig_out &= ~(DASH_FPGA_TRIG_OUTPUT_EN | DASH_FPGA_TRIG_SELECT);
    }
}

/**********************************************************************
 *
 * Function: timingcard_o2_set_sync1_out
 *
 * This function configures the O2 dash fpga SYNC1 Control Register
 * to select the clock frequency that outputs to timing card.
 *
 * Input : freq - clock frequency
 *         clock_source - the clock frequency of clock source
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void timingcard_o2_set_sync1_out (int freq, int clock_source)
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out1;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    sync_out1 = (volatile uint32_t *)(fpga_addr + NGVM_SYNC_OUT1_CTRL);

    if (clock_source == DASH_FPGA_SEL_NGVM) {
        /* Set Dash FPGA SYNC/TRIG Control Register from clock source */
        *sync_out1 = (DASH_FPGA_SYNC_OUTPUT_EN | clock_source);
    } else {
        if (freq == ZL3036X_8K_HZ) {
            /* Set Dash FPGA SYNC/TRIG Control Register as 8kHz output.
             * Enable PreScaler (div by 3125), Enable the SYNC_OUT,
             * Internal (FPGA) PLL */
            *sync_out1 = (DASH_FPGA_SYNC_OUTPUT_EN | DASH_FPGA_DIV_BY_3125 |
                          clock_source);
        } else {
            /* Set Dash FPGA SYNC/TRIG Control Register from clock source */
            *sync_out1 = (DASH_FPGA_SYNC_OUTPUT_EN | clock_source);
        }
    }
}

/**********************************************************************
 *
 * Function: timingcard_o2_restore_sync1_out
 *
 * This function restore the O2 dash fpga SYNC/TRIG Control Register
 * to select the clock frequency that outputs to timing card.
 *
 * Input : *bak_out - pointer to bak_out
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
void timingcard_o2_disable_sync1_out ()
{
    unsigned long fpga_addr;
    volatile uint32_t *sync_out1;

    /* Get the pointer address of dash fpga sync_out register
     * offset. */
    fpga_addr = get_platform_net_clk_ptp_conf_base();
    sync_out1 = (volatile uint32_t *)(fpga_addr + NGVM_SYNC_OUT1_CTRL);

    /* Restore the sync_out control register as original value */
     *sync_out1 &= ~DASH_FPGA_SYNC_OUTPUT_EN;
}

/**********************************************************************
 *
 * Function: select_page_no
 *
 * This function selects the ZL3036X page number.
 *
 * Input : *conf_i2c_p - pointer of i2c device structure
 *         page_no - page number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long select_page_no (n2g_i2c_if_t *conf_i2c_p, uint page_no)
{
    uchar zl3036x_buf;

    /* Before perform the register test, need to make the
     * page_sel_register (0x07F) register page value as 0 */
    zl3036x_buf = page_no;
    conf_i2c_p->offset = ZL3036X_PAGE_SEL_REGISTER;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: zl3036x_configure_ref_clock
 *
 * This function configure the ZL3036X reference clock frequency.
 *
* Input : *conf_i2c_p - pointer of i2c device structure
*         ref_num - reference clock number
*         clock_freq - source clock frequency
*         ref_path - flag of ref_path
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long zl3036x_configure_ref_clock (n2g_i2c_if_t *conf_i2c_p, uint ref_num,
                                  uint clock_freq, boolean ref_path,
                                  boolean lpbk_flag)
{
    uchar zl3036x_buf;
    uint clock_freq_br, clock_freq_kr;

    /* Select page 3 */
    if (select_page_no(conf_i2c_p, ZL3036X_REG_PAGE_3) == FAILED) {
        return (FAILED);
    }

    /* Enable the synth 0, 1, 2, and 3 */
    conf_i2c_p->offset = ZL3036X_SYNTH_ENABLE;
    zl3036x_buf = ENABLE_SYNTH0_1_2_3;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select page 0 */
    if (select_page_no(conf_i2c_p, ZL3036X_REG_PAGE_0) == FAILED) {
        return (FAILED);
    }

    /* Set the phase memory limit */
    if (ref_num == ZL3036X_REF_0) {
        conf_i2c_p->offset = ZL3036X_PHASEMEM_LIMIT_REF0;
    } else {
        conf_i2c_p->offset = ZL3036X_PHASEMEM_LIMIT_REF1;
    }
    zl3036x_buf = PHASE_MEM_LIMIT_1MS;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select page 1 */
    if (select_page_no(conf_i2c_p, ZL3036X_REG_PAGE_1) == FAILED) {
        return (FAILED);
    }

    /* Set the clock frequency */
    if (clock_freq == ZL3036X_1_HZ) {
        /* Set ZL3036X base frequency as 1Hz (Br 1Hz X Kr 1). */
        clock_freq_br = ZL3036X_REF_BR0_1_HZ;
        clock_freq_kr = ZL3036X_REF_KR0_1;
    } else if (clock_freq == ZL3036X_8K_HZ) {
        /* Set ZL3036X base frequency as 8kHz (Br 1Hz X Kr 8). */
        clock_freq_br = ZL3036X_REF_BR0_1K_HZ;
        clock_freq_kr = ZL3036X_REF_KR0_8;
    } else {
        /* Set ZL3036X base frequency as 25MHz (Br 25Hz X Kr 1K). */
        clock_freq_br = ZL3036X_REF_BR0_25K_HZ;
        clock_freq_kr = ZL3036X_REF_KR0_1K;
    }

    /* Due to ZL3036X register value is Big Endian mode, MSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_br >> 8);
    if (ref_num == ZL3036X_REF_0) {
        /* Reference 0 clock */
        conf_i2c_p->offset = ZL3036X_REF0_BASE_FREQ_LOW;
    } else if (ref_num == ZL3036X_REF_1) {
        /* Reference 1 clock */
        conf_i2c_p->offset = ZL3036X_REF1_BASE_FREQ_LOW;
    } else {
        /* Reference 2 clock */
        conf_i2c_p->offset = ZL3036X_REF2_BASE_FREQ_LOW;
    }
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to ZL3036X register value is Big Endian mode, LSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_br & 0xFF);
    if (ref_num == ZL3036X_REF_0) {
        /* Reference 0 clock */
        conf_i2c_p->offset = ZL3036X_REF0_BASE_FREQ_HIGH;
    } else if (ref_num == ZL3036X_REF_1) {
        /* Reference 1 clock */
        conf_i2c_p->offset = ZL3036X_REF1_BASE_FREQ_HIGH;
    } else {
        /* Reference 2 clock */
        conf_i2c_p->offset = ZL3036X_REF2_BASE_FREQ_HIGH;
    }
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to ZL3036X register value is Big Endian mode, MSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_kr >> 8);
    if (ref_num == ZL3036X_REF_0) {
        /* Reference 0 clock */
        conf_i2c_p->offset = ZL3036X_REF0_BASE_FREQ_MULTIPLE_LOW;
    } else if (ref_num == ZL3036X_REF_1) {
        /* Reference 1 clock */
        conf_i2c_p->offset = ZL3036X_REF1_BASE_FREQ_MULTIPLE_LOW;
    } else {
        /* Reference 2 clock */
        conf_i2c_p->offset = ZL3036X_REF2_BASE_FREQ_MULTIPLE_LOW;
    }
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to ZL3036X register value is Big Endian mode, LSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_kr & 0xFF);
    if (ref_num == ZL3036X_REF_0) {
        /* Reference 0 clock */
        conf_i2c_p->offset = ZL3036X_REF0_BASE_FREQ_MULTIPLE_HIGH;
    } else if (ref_num == ZL3036X_REF_1) {
        /* Reference 1 clock */
        conf_i2c_p->offset = ZL3036X_REF1_BASE_FREQ_MULTIPLE_HIGH;
    } else {
        /* Reference 2 clock */
        conf_i2c_p->offset = ZL3036X_REF2_BASE_FREQ_MULTIPLE_HIGH;
    }
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Selects DPLL0 and DPLL1 enabled */
    /* Select page 3 */
    if (select_page_no(conf_i2c_p, ZL3036X_REG_PAGE_3) == FAILED) {
        return (FAILED);
    }

    if (get_timingcard_sku_id() == SKU_30361) {
        /* 30361 SKU only has DPLL0 */
        zl3036x_buf = DPLL_0_ENABLE;
    } else {
        /* 30363 SKU has DPLL0 and DPLL1 */
        zl3036x_buf = DPLL_0_AND_1_ENABLE;
    }
    conf_i2c_p->offset = ZL3036X_DPLL_CONFIG;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Selects central frequency */
    /* Select page 0 */
    if (select_page_no(conf_i2c_p, ZL3036X_REG_PAGE_0) == FAILED) {
        return (FAILED);
    }

    zl3036x_buf = 0x0;
    conf_i2c_p->offset = ZL3036X_CENTRAL_FREQ_OFFSET_B;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    zl3036x_buf = 0x0;
    conf_i2c_p->offset = ZL3036X_CENTRAL_FREQ_OFFSET_C;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    zl3036x_buf = 0x0;
    conf_i2c_p->offset = ZL3036X_CENTRAL_FREQ_OFFSET_D;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    zl3036x_buf = 0x0;
    conf_i2c_p->offset = ZL3036X_CENTRAL_FREQ_OFFSET_E;
    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select page 2 */
    if (select_page_no(conf_i2c_p, ZL3036X_REG_PAGE_2) == FAILED) {
        return (FAILED);
    }

    /* Select DPLL reference clock */
    if (ref_path == FALSE) {
        if (get_timingcard_sku_id() == SKU_30361) {
            /* 30361 SKU only has DPLL0 */
            if (ref_num == ZL3036X_REF_0) {
                /* Configures the DPLL0 mode for the reference 0 clock input
                 * Bit 7:4 is REF1, Bit 3:0 is REF 0 */
                zl3036x_buf = ((DPLL_PRIORITY_1) << 0x4 | DPLL_PRIORITY_0);
                conf_i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
            } else {
                /* Configures the DPLL0 mode for the reference 1 clock input
                 * Bit 7:4 is REF1, Bit 3:0 is REF 0 */
                zl3036x_buf = ((DPLL_PRIORITY_0) << 0x4 | DPLL_PRIORITY_1);
                conf_i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
            }
        } else {
            if (ref_num == ZL3036X_REF_0) {
                /* Configures the DPLL1 mode for the reference 0 clock input
                 * Bit 7:4 is REF1, Bit 3:0 is REF 0 */
                zl3036x_buf = ((DPLL_PRIORITY_1) << 0x4 | DPLL_PRIORITY_0);
                conf_i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY1_0;
            } else {
                /* Configures the DPLL0 mode for the reference 1 clock input
                 * Bit 7:4 is REF1, Bit 3:0 is REF 0 */
                zl3036x_buf = ((DPLL_PRIORITY_0) << 0x4 | DPLL_PRIORITY_1);
                conf_i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
            }
        }
    } else  {
        if (ref_num == ZL3036X_REF_0) {
            /* Configures the DPLL0 mode for the reference 0 clock input
             * Bit 7:4 is REF1, Bit 3:0 is REF 0 */
            zl3036x_buf = ((DPLL_PRIORITY_0) << 0x4 | DPLL_PRIORITY_1);
            conf_i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
        } else {
            /* Configures the DPLL1 mode for the reference 1 clock input
             * Bit 7:4 is REF1, Bit 3:0 is REF 0 */
            zl3036x_buf = ((DPLL_PRIORITY_1) << 0x4 | DPLL_PRIORITY_0);
            conf_i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY1_0;
        }
    }

    conf_i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(conf_i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Set the phase memory limit to 1000ms */
    if (clock_freq == ZL3036X_1_HZ) {
        zl3036x_buf = PHASE_MEMORY_LIMIT;
        conf_i2c_p->offset = ZL3036X_PHASEMEM_LIMIT_REF2;
        conf_i2c_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(conf_i2c_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(conf_i2c_p, conf_i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* User should wait at least 25ms between two write accesses to the same
     * register
     *  - The dplln_df_offset registers can be written with a minimum wait
     *  time of 300 microseconds between write accesses to the same register.
     */
    msleep(350);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_sticky_read
 *
 * This function perform the ZL3036X sticky read register.
 *
 * Input : offset - register offset
 *         *buff - buffer pointer
 *         page_no - page number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_zl3036x_sticky_read (uint32 offset, uchar *buff, int page_no)
{
    int rc = PASSED;
    char w_buf;
    n2g_i2c_if_t *i2c_r_p = get_timingcard_i2c_device();

    if (get_timingcard_sku_id() == SKU_30361) {
        /* 30361 I2C slave address is 0x58 */
        i2c_r_p->i2c_dev = TIMING_CARD_ZL30361_I2C_ADDR;
    }

    /* Select page 0 */
    if (select_page_no(i2c_r_p, ZL3036X_REG_PAGE_0) == FAILED) {
        return (FAILED);
    }

    /* write 0x01 to Sticky Lock Register at address 0x011 */
    w_buf = 0x1;
    i2c_r_p->offset = ZL3036X_STICKY_LOCK_REG;
    i2c_r_p->buf = (char *)&w_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_r_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    /* Select page number */
    if (select_page_no(i2c_r_p, page_no) == FAILED) {
        return (FAILED);
    }

    /* Clear status register(s) by writing 0x00 to it */
    w_buf = 0x0;
    i2c_r_p->offset = offset;
    i2c_r_p->buf = (char *)&w_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_r_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    /* Select page 0 */
    if (select_page_no(i2c_r_p, ZL3036X_REG_PAGE_0) == FAILED) {
        return (FAILED);
    }

    /* Write 0x00 to StickyR Lock Register at address 0x011 */
    w_buf = 0x0;
    i2c_r_p->offset = ZL3036X_STICKY_LOCK_REG;
    i2c_r_p->buf = (char *)&w_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_r_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    /* Wait at least 25 ms */
    msleep(30);

    /* Select page number */
    if (select_page_no(i2c_r_p, page_no) == FAILED) {
        return (FAILED);
    }

    /* Read the status register */
    i2c_r_p->offset = offset;
    i2c_r_p->buf = (char *)buff;

    if (n2g_i2c_read(i2c_r_p) == FAILED) {
        printf("Unable to read i2c");
        return (FAILED);
    }

    return (rc);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_ref_x_lib
 *
 * This function perform the ZL3036X reference clock test.
 *
 * Input : ref_no - reference clock number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_zl3036x_ref_x_lib (uint ref_no, uint clock_freq)
{
    int ix = 0;
    uchar zl3036x_buf;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Configure the ZL3036X reference clock and DPLL. */
    if (zl3036x_configure_ref_clock(i2c_p, ref_no, clock_freq, FALSE, FALSE)
                                    == FAILED) {
        return (FAILED);
    }

    /* Reference 2 clock test does not need to check dpll status. */
    if (clock_freq == ZL3036X_1_HZ) {
        return (PASSED);
    }

    /* Wait no more than 50 second to check if the clock is locked. */
    while (ix < CHECK_LOCK_STS_TIMES) {
        /* Need to call sticky read to read DPLL lock register. */
        if (timingcard_zl3036x_sticky_read(ZL3036X_DPLL_HOLD_LOCK_STATUS,
                                           &zl3036x_buf, ZL3036X_REG_PAGE_3) ==
                                           FAILED) {
            return (FAILED);
        }

        if (ref_no == ZL3036X_REF_0) {
            if (get_timingcard_sku_id() == SKU_30361) {
                /* 30361 SKU only has DPLL0 */
                /* Check DPLL 0 reference clock hold lock status */
                if (zl3036x_buf & ZL3036X_DPLL0_LOCK) {
                    break;
                }
            } else {
                /* Check DPLL 1 reference clock hold lock status */
                if (zl3036x_buf & ZL3036X_DPLL1_LOCK) {
                    break;
                }
            }
        } else {
            /* Check DPLL 0 reference clock hold lock status */
            if (zl3036x_buf & ZL3036X_DPLL0_LOCK) {
                break;
            }
        }

        msleep(1000);
        ix++;
    }

    if (ref_no == ZL3036X_REF_0) {
        if (get_timingcard_sku_id() == SKU_30361) {
            /* 30361 SKU only has DPLL0 */
            /* Check DPLL 0 reference clock hold lock status */
            if (!(zl3036x_buf & ZL3036X_DPLL0_LOCK)) {
                cterr('f', 0, "DPLL0 lock failed, read dpll_hold_lock_status is %#.2x"
                      , zl3036x_buf);
                return (FAILED);
            }
        } else {
            /* Check DPLL 1 reference clock hold lock status */
            if (!(zl3036x_buf & ZL3036X_DPLL1_LOCK)) {
                cterr('f', 0, "DPLL1 lock failed, read dpll_hold_lock_status is %#.2x"
                      , zl3036x_buf);
                return (FAILED);
            }
        }
    } else {
        /* Check DPLL 0 reference clock hold lock status */
        if (!(zl3036x_buf & ZL3036X_DPLL0_LOCK)) {
            cterr('f', 0, "DPLL0 lock failed, read dpll_hold_lock_status is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    }

    /* Check DPLL lock reference pin status */
    for (ix = 0; ix < CHECK_LOCK_STS_TIMES; ix++) {
        /* Select page 2 */
        if (select_page_no(i2c_p, ZL3036X_REG_PAGE_2) == FAILED) {
            return (FAILED);
        }

        /* Check if the DPLL lock correct reference pin. */
        if (ref_no == ZL3036X_REF_0) {
            if (get_timingcard_sku_id() == SKU_30361) {
                /* 30361 SKU only has DPLL0 */
                i2c_p->offset = ZL3036X_DPLL0_REFSEL_STAT;
            } else {
                i2c_p->offset = ZL3036X_DPLL1_REFSEL_STAT;
            }
        } else {
            i2c_p->offset = ZL3036X_DPLL0_REFSEL_STAT;
        }
        i2c_p->buf = (char *)&zl3036x_buf;

        if (n2g_i2c_read(i2c_p) == FAILED) {
            printf("Unable to read i2c");
            return (FAILED);
        }

        if (ref_no == ZL3036X_REF_0) {
            /* Check DPLL 1 lock reference 0 status on 30363 sku,
             * check DPLL 0 lock reference 0 status on 30361 sku. */
            if (zl3036x_buf == ZL3036X_DPLL_LOCK_REF0) {
                break;
            }
        } else {
            /* Check DPLL 0 lock reference 1 status */
            if (zl3036x_buf == ZL3036X_DPLL_LOCK_REF1) {
                break;
            }
        }

        msleep(1000);
    }

    if (ref_no == ZL3036X_REF_0) {
        /* Check DPLL 1 lock reference 0 status on 30363 sku,
         * check DPLL 0 lock reference 0 status on 30361 sku. */
        if (zl3036x_buf != ZL3036X_DPLL_LOCK_REF0) {
            if (get_timingcard_sku_id() == SKU_30361) {
                /* 30361 SKU only has DPLL0 */
                cterr('f', 0, "DPLL0 lock reference 0 failed, read dpll0_refsel_stat is %#.2x"
                      , zl3036x_buf);
            } else {
                cterr('f', 0, "DPLL1 lock reference 0 failed, read dpll1_refsel_stat is %#.2x"
                      , zl3036x_buf);
            }
            return (FAILED);
        }
    } else {
        /* Check DPLL 0 lock reference 1 status */
        if (zl3036x_buf != ZL3036X_DPLL_LOCK_REF1) {
            cterr('f', 0, "DPLL0 lock reference 1 failed, read dpll0_refsel_stat is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingzard_zl30361_ref2_check
 *
 * This function check the reference monitor register
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingzard_zl30361_ref2_check (uchar *data_buf)
{
    /* Need to call sticky read to read reference monitor fail register. */
    if (timingcard_zl3036x_sticky_read(ZL3036X_REF_MON_FAIL_2,
                                       data_buf, ZL3036X_REG_PAGE_0) ==
                                       FAILED) {
        return (FAILED);
    }

    if ((*data_buf & REF2_FAIL_GST) || (*data_buf & REF2_FAIL_CFM) ||
            (*data_buf & REF2_FAIL_SCM)) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_conf_ref_path
 *
 * This function perform the ZL3036X reference clock loopback test.
 *
 * Input : ref_no - reference number
 *         hpoutclk_no - hpoutclock number
 *         clock_freq - clock frequency
 *         ref_path - boolean flag of reference clock path
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_zl3036x_conf_ref_path (uint ref_num, uint hpoutclk_no,
                                       uint clock_freq, boolean ref_path,
                                       boolean lpbk_flag)
{
    uchar zl3036x_buf;
    uint clock_freq_bs, clock_freq_ks;
    n2g_i2c_if_t *i2c_ref_p = get_timingcard_i2c_device();

    /* Configure the ZL3036X reference clock and DPLL. */
    if (zl3036x_configure_ref_clock(i2c_ref_p, ref_num, clock_freq, ref_path,
                                    lpbk_flag)
        == FAILED) {
        return (FAILED);
    }

    /* Select page 3 */
    if (select_page_no(i2c_ref_p, ZL3036X_REG_PAGE_3) == FAILED) {
        return (FAILED);
    }

    /* Select DPLL that drive synthesizer register */
    /* DPLL0 to synth 0/2, DPLL1 to synth 1/3 */
    zl3036x_buf = ZL3036X_DPLL_SYNTH1_2;
    i2c_ref_p->offset = ZL3036X_SYNTH_DRIVE_PLL;
    i2c_ref_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_ref_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Set the clock frequency */
    if (clock_freq == ZL3036X_8K_HZ) {
        /* Set ZL3036X base frequency as 8kHz */
        clock_freq_bs = ZL3036X_REF_BS0_8K_HZ;
        clock_freq_ks = ZL3036X_REF_8K_KS0;
    } else if (clock_freq == ZL3036X_25M_HZ) {
        /* Set ZL3036X base frequency as 25MHz */
        clock_freq_bs = ZL3036X_REF_BS0_25M_HZ;
        clock_freq_ks = ZL3036X_REF_25M_KS0;
    } else {
        clock_freq_bs = ZL3036X_REF_BS0_125M_HZ;
        clock_freq_ks = ZL3036X_REF_125M_KS0;
    }

    /* Select DPLL that drive synthesizer register */
    if (ref_path == FALSE) {
        /* Set ZL3036X Synth 1 and 2 frequency as 8kHz (Bs 1Hz X Ks 8). */
        /* Select 1Hz for Bs 1Hz and due to big Endian, MSB needs to be written
         * to the lower address */
        zl3036x_buf = (clock_freq_bs >> 8);
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_BASE_FREQ_LOW;
        } else {
            /* Synth 2 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH2_BASE_FREQ_LOW;
        }
    } else {
        /* Set ZL3036X Synth 0 and 1 frequency as 8kHz (Bs 1Hz X Ks 8). */
        /* Select 1Hz for Bs 1Hz and due to big Endian, MSB needs to be written
         * to the lower address */
        zl3036x_buf = (clock_freq_bs >> 8);
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 0 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH0_BASE_FREQ_LOW;
        } else {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_BASE_FREQ_LOW;
        }
    }
    i2c_ref_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_ref_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to big Endian, LSB needs to be written to the higher address */
    zl3036x_buf = (clock_freq_bs & 0xFF);
    if (ref_path == FALSE) {
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_BASE_FREQ_HIGH;
        } else {
            /* Synth 2 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH2_BASE_FREQ_HIGH;
        }
    } else {
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 0 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH0_BASE_FREQ_HIGH;
        } else {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_BASE_FREQ_HIGH;
        }
    }

    i2c_ref_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_ref_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to big Endian, MSB needs to be written to the lower address */
    zl3036x_buf = (clock_freq_ks >> 8);
    if (ref_path == FALSE) {
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_FREQ_MULTIPLE_LOW;
        } else {
            /* Synth 2 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH2_FREQ_MULTIPLE_LOW;
        }
    } else {
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 0 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH0_FREQ_MULTIPLE_LOW;
        } else {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_FREQ_MULTIPLE_LOW;
        }
    }
    i2c_ref_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_ref_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to big Endian, LSB needs to be written to the higher address */
    zl3036x_buf = (clock_freq_ks & 0xFF);
    if (ref_path == FALSE) {
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_FREQ_MULTIPLE_HIGH;
        } else {
            /* Synth 2 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH2_FREQ_MULTIPLE_HIGH;
        }
    } else {
        if (ref_num == ZL3036X_REF_0) {
            /* Synth 0 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH0_FREQ_MULTIPLE_HIGH;
        } else {
            /* Synth 1 frequency */
            i2c_ref_p->offset = ZL3036X_SYNTH1_FREQ_MULTIPLE_HIGH;
        }
    }
    i2c_ref_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_ref_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Configure synth 0 and 1 output clock to hpoutclk 0 and hpoutclk 2 */
    if (ref_path == TRUE) {
        /* Select page 4 */
        if (select_page_no(i2c_ref_p, ZL3036X_REG_PAGE_4) == FAILED) {
            return (FAILED);
        }

        /* Configure the synth clock */
        if (clock_freq == ZL3036X_8K_HZ) {
            zl3036x_buf = SYNTH_8K_CLOCK;
        } else if (clock_freq == ZL3036X_25M_HZ) {
            zl3036x_buf = SYNTH_25M_CLOCK;
        } else if (clock_freq == ZL3036X_125M_HZ) {
            zl3036x_buf = SYNTH_125M_CLOCK;
        }

        if (ref_num == ZL3036X_REF_0) {
            /* Configure the synth 0 clock C, bit 23:16 */
            i2c_ref_p->offset = ZL3036X_SYNTH0_POST_DIV_C1;
        } else {
            /* Configure the synth 1 clock C, bit 23:16 */
            i2c_ref_p->offset = ZL3036X_SYNTH1_POST_DIV_C1;
        }
        i2c_ref_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(i2c_ref_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
                return (FAILED);
            }
        }

        /* Configure the synth clock */
        if (clock_freq == ZL3036X_8K_HZ) {
            zl3036x_buf = DIVISION_8K_1;
        } else if (clock_freq == ZL3036X_25M_HZ) {
            zl3036x_buf = DIVISION_25M_1;
        } else if (clock_freq == ZL3036X_125M_HZ) {
            zl3036x_buf = DIVISION_125M_1;
        }

        if (ref_num == ZL3036X_REF_0) {
            /* Configure the synth 0 clock C, bit 15:8 */
            i2c_ref_p->offset = ZL3036X_SYNTH0_POST_DIV_C2;
        } else {
            /* Configure the synth 1 clock C, bit 15:8 */
            i2c_ref_p->offset = ZL3036X_SYNTH1_POST_DIV_C2;
        }
        i2c_ref_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(i2c_ref_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
                return (FAILED);
            }
        }

        /* Configure the synth clock */
        if (clock_freq == ZL3036X_8K_HZ) {
            zl3036x_buf = DIVISION_8K_2;
        } else if (clock_freq == ZL3036X_25M_HZ) {
            zl3036x_buf = DIVISION_25M_2;
        } else if (clock_freq == ZL3036X_125M_HZ) {
            zl3036x_buf = DIVISION_125M_2;
        }

        if (ref_num == ZL3036X_REF_0) {
            /* Configure the synth 0 clock C, bit 7:0 */
            i2c_ref_p->offset = ZL3036X_SYNTH0_POST_DIV_C3;
        } else {
            /* Configure the synth 1 clock C, bit 7:0 */
            i2c_ref_p->offset = ZL3036X_SYNTH1_POST_DIV_C3;
        }
        i2c_ref_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(i2c_ref_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
                return (FAILED);
            }
        }

        /* Configure 1 PPS clock */
        /* Select page 4 */
        if (select_page_no(i2c_ref_p, ZL3036X_REG_PAGE_4) == FAILED) {
            return (FAILED);
        }

        /* Configure the 1PPS clock */
        zl3036x_buf = PTP_1PPS_1HZ_CLOCK;
        /* Configure the synth 0 clock D, bit 23:16 */
        i2c_ref_p->offset = ZL3036X_SYNTH0_POST_DIV_D1;
        i2c_ref_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(i2c_ref_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
                return (FAILED);
            }
        }

        /* Configure the 1PPS clock divider */
        zl3036x_buf = PTP_1PPS_1HZ_DIVIDER1;
        /* Configure the synth 0 clock D, bit 23:16 */
        i2c_ref_p->offset = ZL3036X_SYNTH0_POST_DIV_D2;
        i2c_ref_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(i2c_ref_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
                return (FAILED);
            }
        }

        /* Configure the 1PPS clock divider */
        zl3036x_buf = PTP_1PPS_1HZ_DIVIDER2;
        /* Configure the synth 0 clock D, bit 23:16 */
        i2c_ref_p->offset = ZL3036X_SYNTH0_POST_DIV_D3;
        i2c_ref_p->buf = (char *)&zl3036x_buf;

        /* Alter reg with new value */
        if (n2g_i2c_write(i2c_ref_p) != PASSED) {
            printf("unable to write i2c.\n");
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
                return (FAILED);
            }
        }
    }

    /* Select page 4 */
    if (select_page_no(i2c_ref_p, ZL3036X_REG_PAGE_4) == FAILED) {
        return (FAILED);
    }

    /* Configure the hp_coms_en to select the out clock. */
    if (ref_path == FALSE) {
        zl3036x_buf = (ZL3036X_HP_COMS_EN_3 | ZL3036X_HP_COMS_EN_4);
    } else {
        if (ref_num == ZL3036X_REF_0) {
            zl3036x_buf = (ZL3036X_HP_COMS_EN_0 | ZL3036X_HP_COMS_EN_1);
        } else {
            zl3036x_buf = (ZL3036X_HP_COMS_EN_0 | ZL3036X_HP_COMS_EN_1 |
                           ZL3036X_HP_COMS_EN_2);
        }
    }
    i2c_ref_p->offset = ZL3036X_HP_COMS_EN;
    i2c_ref_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_ref_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_ref_p, i2c_ref_p->offset) == FAILED) {
            return (FAILED);
        }
    }

   return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_ref_x_lpbk_lib
 *
 * This function perform the ZL3036X reference clock loopback test.
 *
 * Input : ref_no - reference number
 *         clock_freq - clock frequency
 *         lpbk_flag - clock loopback flag
 *         pps_flag - 1pps flag
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_zl3036x_ref_x_lpbk_lib (uint ref_no, uint clock_freq,
                                        boolean lpbk_flag, boolean pps_flag)
{
    int counter = 0, ix;
    uchar zl3036x_buf;
    uint clock_freq_br, clock_freq_kr;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Select page 0 */
    if (select_page_no(i2c_p, ZL3036X_REG_PAGE_0) == FAILED) {
        return (FAILED);
    }

    /* Set the phase memory limit */
    if (ref_no == ZL3036X_REF_0) {
        if (pps_flag == TRUE) {
            i2c_p->offset = ZL3036X_PHASEMEM_LIMIT_REF0;
        } else {
            i2c_p->offset = ZL3036X_PHASEMEM_LIMIT_REF3;
        }
    } else {
        i2c_p->offset = ZL3036X_PHASEMEM_LIMIT_REF4;
    }
    /* Configures the DPLL0 mode for the reference 0 clock input */
    zl3036x_buf = PHASE_MEM_LIMIT_1MS;
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Configure the hp_coms_en to select the out clock. */
    if (pps_flag == TRUE) {
        /* HPOUTCLK1 */
        if (timingcard_zl3036x_conf_ref_path(ref_no, ZL3036X_HP_COMS_EN_1,
                                             clock_freq, TRUE, lpbk_flag)
            == FAILED) {
            return (FAILED);
        }
    } else {
        if (ref_no == ZL3036X_REF_0) {
            /* HPOUTCLK3 */
            if (timingcard_zl3036x_conf_ref_path(ref_no, ZL3036X_HP_COMS_EN_3,
                                                 clock_freq, FALSE, lpbk_flag)
                == FAILED) {
                return (FAILED);
            }
        } else {
            /* HPOUTCLK4 */
            if (timingcard_zl3036x_conf_ref_path(ref_no, ZL3036X_HP_COMS_EN_4,
                                                 clock_freq, FALSE, lpbk_flag)
                == FAILED) {
                return (FAILED);
            }
        }
    }

    /* Select page 2 */
    if (select_page_no(i2c_p, ZL3036X_REG_PAGE_2) == FAILED) {
        return (FAILED);
    }

    /* Select DPLL 0 for reference 3 clock, DPLL 1 for reference 4 */
    if (ref_no == ZL3036X_REF_0) {
        if (pps_flag == TRUE) {
            /* Configures the DPLL0 mode reference 0 and 1 priority */
            i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
            zl3036x_buf = ((DPLL_PRIORITY_1 << 4) | DPLL_PRIORITY_0);
        } else {
            /* Configures the DPLL0 mode reference 0 and 1 priority */
            i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
            zl3036x_buf = ((DPLL_PRIORITY_1 << 4) | DPLL_PRIORITY_3);
        }

    } else {
        /* Configures the DPLL1 mode reference 0 and 1 priority */
        i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY1_0;
        zl3036x_buf = ((DPLL_PRIORITY_4 << 4) | DPLL_PRIORITY_1);
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select DPLL 0 for reference 3 clock, DPLL 1 for reference 4 */
    if (ref_no == ZL3036X_REF_0) {
        if (pps_flag == TRUE) {
            /* Configures the DPLL0 mode reference 2 and 3 priority */
            i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY3_2;
            zl3036x_buf = ((DPLL_PRIORITY_3 << 4) | DPLL_PRIORITY_2);
        } else {
            /* Configures the DPLL0 mode reference 2 and 3 priority */
            i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY3_2;
            zl3036x_buf = ((DPLL_PRIORITY_0 << 4) | DPLL_PRIORITY_2);
        }
    } else {
        /* Configures the DPLL1 mode reference 4 and 5 priority */
        i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY5_4;
        zl3036x_buf = ((DPLL_PRIORITY_5 << 4) | DPLL_PRIORITY_0);
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select page 1 */
    if (select_page_no(i2c_p, ZL3036X_REG_PAGE_1) == FAILED) {
        return (FAILED);
    }

    /* Set the clock frequency */
    if (clock_freq == ZL3036X_8K_HZ) {
        if (ref_no == ZL3036X_REF_0) {
            /* Set ZL3036X base Br3 KR3 frequency as 8kHz (Br3 1Hz X Kr3 8). */
            clock_freq_br = ZL3036X_REF_BR3_1K_HZ;
            clock_freq_kr = ZL3036X_REF_KR3_8;
        } else {
            /* Set ZL3036X base Br4 KR4 frequency as 8kHz (Br4 1Hz X Kr4 8). */
            clock_freq_br = ZL3036X_REF_BR4_1K_HZ;
            clock_freq_kr = ZL3036X_REF_KR4_8;
        }
    } else {
        if (ref_no == ZL3036X_REF_0) {
            /* Set ZL3036X base BR3 KR3 frequency as 25MHz (Br3 25Hz X Kr3 1K). */
            clock_freq_br = ZL3036X_REF_BR3_25K_HZ;
            clock_freq_kr = ZL3036X_REF_KR3_1K;
        } else {
            /* Set ZL3036X base BR4 KR 4 frequency as 25MHz (Br4 25Hz X Kr4 1K). */
            clock_freq_br = ZL3036X_REF_BR4_25K_HZ;
            clock_freq_kr = ZL3036X_REF_KR4_1K;
        }
    }

    /* Due to big Endian, MSB needs to be written to the lower address */
    if (ref_no == ZL3036X_REF_0) {
        zl3036x_buf = (clock_freq_br >> 8);
        if (pps_flag == TRUE) {
            /* Reference 0 clock */
            i2c_p->offset = ZL3036X_REF0_BASE_FREQ_LOW;
        } else {
            /* Reference 3 clock */
            i2c_p->offset = ZL3036X_REF3_BASE_FREQ_LOW;
        }
    } else {
        /* Reference 4 clock */
        zl3036x_buf = (clock_freq_br >> 8);
        i2c_p->offset = ZL3036X_REF4_BASE_FREQ_LOW;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to big Endian, LSB needs to be written to the higher address */
    if (ref_no == ZL3036X_REF_0) {
        zl3036x_buf = (clock_freq_br & 0xFF);
        if (pps_flag == TRUE) {
            /* Reference 0 clock */
            i2c_p->offset = ZL3036X_REF0_BASE_FREQ_HIGH;
        } else {
            /* Reference 3 clock */
            i2c_p->offset = ZL3036X_REF3_BASE_FREQ_HIGH;
        }
    } else {
        /* Reference 4 clock */
        zl3036x_buf = (clock_freq_br & 0xFF);
        i2c_p->offset = ZL3036X_REF4_BASE_FREQ_HIGH;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to big Endian, MSB needs to be written to the lower address */
    if (ref_no == ZL3036X_REF_0) {
        zl3036x_buf = (clock_freq_kr >> 8);
        if (pps_flag == TRUE) {
            /* Reference 0 clock */
            i2c_p->offset = ZL3036X_REF0_BASE_FREQ_MULTIPLE_LOW;
        } else {
            /* Reference 3 clock */
            i2c_p->offset = ZL3036X_REF3_BASE_FREQ_MULTIPLE_LOW;
        }
    } else {
        /* Reference 4 clock */
        zl3036x_buf = (clock_freq_kr >> 8);
        i2c_p->offset = ZL3036X_REF4_BASE_FREQ_MULTIPLE_LOW;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to big Endian, LSB needs to be written to the higher address */
    if (ref_no == ZL3036X_REF_0) {
        zl3036x_buf = (clock_freq_kr & 0xFF);
        if (pps_flag == TRUE) {
            /* Reference 0 clock */
            i2c_p->offset = ZL3036X_REF0_BASE_FREQ_MULTIPLE_HIGH;
        } else {
            /* Reference 3 clock */
            i2c_p->offset = ZL3036X_REF3_BASE_FREQ_MULTIPLE_HIGH;
        }
    } else {
        /* Reference 4 clock */
        zl3036x_buf = (clock_freq_kr & 0xFF);
        i2c_p->offset = ZL3036X_REF4_BASE_FREQ_MULTIPLE_HIGH;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select page 4 */
    if (select_page_no(i2c_p, ZL3036X_REG_PAGE_4) == FAILED) {
        return (FAILED);
    }

    /* Configure the synth clock */
    if (clock_freq == ZL3036X_8K_HZ) {
        if (pps_flag == TRUE) {
            zl3036x_buf = SYNTH_TRIGOUT_8K_CLOCK;
        } else {
            zl3036x_buf = SYNTH_8K_CLOCK;
        }
    } else {
        zl3036x_buf = SYNTH_25M_CLOCK;
    }

    if (ref_no == ZL3036X_REF_0) {
        if (pps_flag == TRUE) {
            /* Configure the synth 0 clock D, bit 23:16 */
            i2c_p->offset = ZL3036X_SYNTH0_POST_DIV_D1;
        } else {
            /* Configure the synth 1 clock D, bit 23:16 */
            i2c_p->offset = ZL3036X_SYNTH1_POST_DIV_D1;
        }
    } else {
        /* Configure the synth 2 clock D, bit 23:16 */
        i2c_p->offset = ZL3036X_SYNTH2_POST_DIV_C1;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Configure the synth clock */
    if (clock_freq == ZL3036X_8K_HZ) {
        if (pps_flag == TRUE) {
            zl3036x_buf = SYNTH_TRIGOUT_DIVISION_8K_1;
        } else {
            zl3036x_buf = DIVISION_8K_1;
        }
    } else {
        zl3036x_buf = DIVISION_25M_1;
    }

    if (ref_no == ZL3036X_REF_0) {
        if (pps_flag == TRUE) {
            /* Configure the synth 0 clock D, bit 15:8 */
            i2c_p->offset = ZL3036X_SYNTH0_POST_DIV_D2;
        } else {
            /* Configure the synth 1 clock D, bit 15:8 */
            i2c_p->offset = ZL3036X_SYNTH1_POST_DIV_D2;
        }
    } else {
        /* Configure the synth 2 clock D, bit 15:8 */
        i2c_p->offset = ZL3036X_SYNTH2_POST_DIV_C2;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Configure the synth clock */
    if (clock_freq == ZL3036X_8K_HZ) {
        if (pps_flag == TRUE) {
            zl3036x_buf = SYNTH_TRIGOUT_DIVISION_8K_2;
        } else {
            zl3036x_buf = DIVISION_8K_2;
        }
    } else {
        zl3036x_buf = DIVISION_25M_2;
    }
    if (ref_no == ZL3036X_REF_0) {
        if (pps_flag == TRUE) {
            /* Configure the synth 0 clock D, bit 7:0 */
            i2c_p->offset = ZL3036X_SYNTH0_POST_DIV_D3;
        } else {
            /* Configure the synth 1 clock D, bit 7:0 */
            i2c_p->offset = ZL3036X_SYNTH1_POST_DIV_D3;
        }
    } else {
        /* Configure the synth 2 clock D, bit 7:0 */
        i2c_p->offset = ZL3036X_SYNTH2_POST_DIV_C3;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* User should wait at least 25ms between two write accesses to the same
     * register
     *  - The dplln_df_offset registers can be written with a minimum wait
     *  time of 300 microseconds between write accesses to the same register.
     */
    msleep(350);

    /* Wait no more than 50 second to check if the clock is locked. */
    while (counter < 50) {
        /* Need to call sticky read to read DPLL lock register. */
        if (timingcard_zl3036x_sticky_read(ZL3036X_DPLL_HOLD_LOCK_STATUS,
                                           &zl3036x_buf, ZL3036X_REG_PAGE_3) ==
                                           FAILED) {
            return (FAILED);
        }

        if (ref_no == ZL3036X_REF_0) {
            /* Check DPLL reference clock hold lock status */
            if (zl3036x_buf & ZL3036X_DPLL0_LOCK) {
                break;
            }
        } else {
            /* Check DPLL reference clock hold lock status */
            if (zl3036x_buf & ZL3036X_DPLL1_LOCK) {
                break;
            }
        }

        msleep(1000);
        counter++;
    }

    if (ref_no == ZL3036X_REF_0) {
        /* Check DPLL 0 reference clock hold lock status */
        if (!(zl3036x_buf & ZL3036X_DPLL0_LOCK)) {
            cterr('f', 0, "DPLL0 lock failed, read dpll_hold_lock_status is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    } else {
        /* Check DPLL 1 reference clock hold lock status */
        if (!(zl3036x_buf & ZL3036X_DPLL1_LOCK)) {
            cterr('f', 0, "DPLL0 lock failed, read dpll_hold_lock_status is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    }

    /* Check DPLL lock reference pin status */
    for (ix = 0; ix < 50; ix++) {
        /* Select page 2 */
        if (select_page_no(i2c_p, ZL3036X_REG_PAGE_2) == FAILED) {
            return (FAILED);
        }

        /* Check if the DPLL lock correct reference pin. */
        if (ref_no == ZL3036X_REF_0) {
            i2c_p->offset = ZL3036X_DPLL0_REFSEL_STAT;
        } else {
            i2c_p->offset = ZL3036X_DPLL1_REFSEL_STAT;
        }
        i2c_p->buf = (char *)&zl3036x_buf;

        if (n2g_i2c_read(i2c_p) == FAILED) {
            printf("Unable to read i2c");
            return (FAILED);
        }

        if (ref_no == ZL3036X_REF_0) {
            /* Check DPLL 0 lock reference 0 status */
            if (pps_flag == TRUE) {
                if (zl3036x_buf == ZL3036X_DPLL_REF_0) {
                    break;
                }
            } else {
                if (zl3036x_buf == ZL3036X_DPLL_REF_3) {
                    break;
                }
            }
        } else {
            /* Check DPLL 1 lock reference 1 status */
            if (zl3036x_buf == ZL3036X_DPLL_REF_4) {
                break;
            }
        }

        msleep(1000);
    }

    if (ref_no == ZL3036X_REF_0) {
        /* Check DPLL 0 lock reference 0 status */
        if (pps_flag == TRUE) {
            if (zl3036x_buf != ZL3036X_DPLL_REF_0) {
                cterr('f', 0, "DPLL0 lock reference 0 failed, read dpll0_refsel_stat is %#.2x"
                      , zl3036x_buf);
                return (FAILED);
            }
        } else {
            if (zl3036x_buf != ZL3036X_DPLL_REF_3) {
                cterr('f', 0, "DPLL0 lock reference 0 failed, read dpll0_refsel_stat is %#.2x"
                      , zl3036x_buf);
                return (FAILED);
            }
        }
    } else {
        /* Check DPLL 1 lock reference 1 status */
        if (zl3036x_buf != ZL3036X_DPLL_REF_4) {
            cterr('f', 0, "DPLL1 lock reference 1 failed, read dpll1_refsel_stat is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_outx_lpbk_dpll_check
 *
 * This function checks the ZL3036X reference clock is locks from O2 FPGA
 * sync1_out.
 *
 * Input : hpoutclk_no - hpoutclk number
 *         clock_freq - clock frequency
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long timingcard_zl3036x_outx_lpbk_dpll_check (uint hpoutclk_no, uint clock_freq,
                                              boolean pps_flag)
{
    int ix = 0;
    uchar zl3036x_buf;
    uint clock_freq_br, clock_freq_kr;
    n2g_i2c_if_t *i2c_p = get_timingcard_i2c_device();

    /* Select page 2 */
    if (select_page_no(i2c_p, ZL3036X_REG_PAGE_2) == FAILED) {
        return (FAILED);
    }

    /* Selects DPLL 0 for reference 0 clock */
        /* Configures the DPLL0 mode reference 0 as first priority */
    i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
    zl3036x_buf = ((DPLL_PRIORITY_1 << 4) | DPLL_PRIORITY_0);
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* HPOUTCLK0 selects DPLL 1 for reference 1 clock,
     * HPOUTCLK1 selects DPLL 1 for reference 2 clock,
     * HPOUTCLK2 selects DPLL 0 for reference 1 clock */
    if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
        /* Configures the DPLL1 mode reference 0 and 1 priority */
        i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY1_0;
        zl3036x_buf = ((DPLL_PRIORITY_0 << 4) | DPLL_PRIORITY_1);
    } else if (hpoutclk_no == ZL3036X_HP_COMS_EN_1) {
        /* Configures the DPLL1 mode reference 0 and 1 priority */
        i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY1_0;
        zl3036x_buf = ((DPLL_PRIORITY_1 << 4) | DPLL_PRIORITY_2);
    } else if (hpoutclk_no == ZL3036X_HP_COMS_EN_2) {
        /* Configures the DPLL0 mode reference 0 and 1 priority */
        i2c_p->offset = ZL3036X_DPLL0_REF_PRIORITY1_0;
        zl3036x_buf = ((DPLL_PRIORITY_1 << 4) | DPLL_PRIORITY_0);
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* HPOUTCLK0 selects DPLL 1 for reference 1 clock */
    if (hpoutclk_no == ZL3036X_HP_COMS_EN_1) {
        /* Configures the DPLL1 mode reference 0 and 1 priority */
        i2c_p->offset = ZL3036X_DPLL1_REF_PRIORITY3_2;
        zl3036x_buf = ((DPLL_PRIORITY_3 << 4) | DPLL_PRIORITY_0);
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Select page 1 */
    if (select_page_no(i2c_p, ZL3036X_REG_PAGE_1) == FAILED) {
        return (FAILED);
    }

    /* Set the clock frequency */
    if (clock_freq == ZL3036X_8K_HZ) {
        if (pps_flag == TRUE) {
            /* Set ZL3036X base frequency as 8kHz (Br 1Hz X Kr 8). */
            clock_freq_br = ZL3036X_REF_BR0_1PPS;
            clock_freq_kr = ZL3036X_REF_KR0_1PPS;
        } else {
            /* Set ZL3036X base frequency as 8kHz (Br 1Hz X Kr 8). */
            clock_freq_br = ZL3036X_REF_BR0_1K_HZ;
            clock_freq_kr = ZL3036X_REF_KR0_8;
        }
    } else if (clock_freq == ZL3036X_25M_HZ) {
        /* Set ZL3036X base frequency as 25MHz (Br 25 KHz X Kr 1K). */
        clock_freq_br = ZL3036X_REF_BR0_25K_HZ;
        clock_freq_kr = ZL3036X_REF_KR0_1K;
    } else {
        /* Set ZL3036X base frequency as 125MHz (Br 25 KHz X Kr 5K). */
        clock_freq_br = ZL3036X_REF_BR0_25K_HZ;
        clock_freq_kr = ZL3036X_REF_KR0_5K;
    }

    /* Due to ZL3036X register value is Big Endian mode, MSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_br >> 8);
    if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
        /* Reference 1 clock */
        i2c_p->offset = ZL3036X_REF1_BASE_FREQ_LOW;
    } else {
        /* Reference 2 clock */
        i2c_p->offset = ZL3036X_REF2_BASE_FREQ_LOW;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to ZL3036X register value is Big Endian mode, LSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_br & 0xFF);
    if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
        /* Reference 1 clock */
        i2c_p->offset = ZL3036X_REF1_BASE_FREQ_HIGH;
    } else {
        /* Reference 2 clock */
        i2c_p->offset = ZL3036X_REF2_BASE_FREQ_HIGH;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to ZL3036X register value is Big Endian mode, MSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_kr >> 8);
    if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
        /* Reference 1 clock */
        i2c_p->offset = ZL3036X_REF1_BASE_FREQ_MULTIPLE_LOW;
    } else {
        /* Reference 2 clock */
        i2c_p->offset = ZL3036X_REF2_BASE_FREQ_MULTIPLE_LOW;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* Due to ZL3036X register value is Big Endian mode, LSB needs to be written
     * to the lower address */
    zl3036x_buf = (clock_freq_kr & 0xFF);
    if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
        /* Reference 1 clock */
        i2c_p->offset = ZL3036X_REF1_BASE_FREQ_MULTIPLE_HIGH;
    } else {
        /* Reference 2 clock */
        i2c_p->offset = ZL3036X_REF2_BASE_FREQ_MULTIPLE_HIGH;
    }
    i2c_p->buf = (char *)&zl3036x_buf;

    /* Alter reg with new value */
    if (n2g_i2c_write(i2c_p) != PASSED) {
        printf("unable to write i2c.\n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_debug(i2c_p, i2c_p->offset) == FAILED) {
            return (FAILED);
        }
    }

    /* User should wait at least 25ms between two write accesses to the same
     * register
     *  - The dplln_df_offset registers can be written with a minimum wait
     *  time of 300 microseconds between write accesses to the same register.
     */
    msleep(350);

    /* Wait no more than 50 second to check if the clock is locked. */
    while (ix < 50) {
        /* Need to call sticky read to read DPLL lock register. */
        if (timingcard_zl3036x_sticky_read(ZL3036X_DPLL_HOLD_LOCK_STATUS,
                                           &zl3036x_buf, ZL3036X_REG_PAGE_3) ==
                                           FAILED) {
            return (FAILED);
        }

        /* Check DPLL 1 reference clock hold lock status */
        if (zl3036x_buf & ZL3036X_DPLL1_LOCK) {
            break;
        }

        msleep(1000);
        ix++;
    }

    /* Check DPLL 1 reference clock hold lock status */
    if (!(zl3036x_buf & ZL3036X_DPLL1_LOCK)) {
        cterr('f', 0, "DPLL1 lock failed, read dpll_hold_lock_status is %#.2x"
              , zl3036x_buf);
        return (FAILED);
    }

    for (ix = 0; ix < 50; ix++) {
        /* Select page 2 */
        if (select_page_no(i2c_p, ZL3036X_REG_PAGE_2) == FAILED) {
            return (FAILED);
        }

        /* Check if the DPLL lock correct reference pin. */
        i2c_p->offset = ZL3036X_DPLL1_REFSEL_STAT;
        i2c_p->buf = (char *)&zl3036x_buf;

        if (n2g_i2c_read(i2c_p) == FAILED) {
            printf("Unable to read i2c");
            return (FAILED);
        }

        if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
            if (zl3036x_buf == ZL3036X_DPLL_REF_1) {
                break;
            }
        } else {
            if (zl3036x_buf == ZL3036X_DPLL_REF_2) {
                break;
            }
        }

        msleep(1000);
    }

    if (hpoutclk_no == ZL3036X_HP_COMS_EN_0) {
        if (zl3036x_buf != ZL3036X_DPLL_REF_1) {
            cterr('f', 0, "DPLL1 lock reference 1 failed, read dpll1_refsel_stat is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    } else {
        if (zl3036x_buf != ZL3036X_DPLL_REF_2) {
            cterr('f', 0, "DPLL1 lock reference 2 failed, read dpll1_refsel_stat is %#.2x"
                  , zl3036x_buf);
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_zl3036x_read_fn
 *
 * Read Timing Card Zl3036X Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         buf - Read buffer
 *         param -param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int timingcard_zl3036x_read_fn (unsigned long addr, int size,
                                       unsigned long *buf, void *param)
{
    int rc = PASSED;
    n2g_i2c_if_t *i2c_if = get_timingcard_i2c_device();

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
 * Function: timingcard_zl3036x_write_fn
 *
 * Write Timing Card ZL3036X Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         data - data for write
 *         param -param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int timingcard_zl3036x_write_fn (unsigned long addr, int size,
                                        unsigned long data, void *param)
{
    int rc = PASSED;
    n2g_i2c_if_t *i2c_if = get_timingcard_i2c_device();
    i2c_if->buf = (char *)&data;
    i2c_if->offset = (unsigned int)addr;

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("unable to write i2c.\n");
        rc = FAILED;
        return (rc);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: show_debug
 *
 * Show debug message
 *
 * Input : *s_i2c_if - pointer to i2c interface
 *         reg_offset - register offset
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int show_debug (n2g_i2c_if_t *s_i2c_if, int reg_offset)
{
    uchar zl3036x_buf;

    s_i2c_if->offset = reg_offset;
    s_i2c_if->buf = (char *)&zl3036x_buf;

    if (n2g_i2c_read(s_i2c_if) == FAILED) {
        printf("Unable to read i2c");
        return (FAILED);
    }

    printf("conf_i2c_p->offset is %#x\n", s_i2c_if->offset);
    printf("zl3036x_buf is %#x\n", zl3036x_buf);

    return (PASSED);
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_zl3036x_lib.c,v $
 * Revision 1.3  2015/02/18 06:08:26  bowang3
 * Support Wallander NIM 1588 test with timing card
 *
 * Revision 1.2  2015/02/14 12:48:43  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.8  2014/04/25 06:56:34  kodko
 * Support ZL30361 reference 2 clock input test.
 *
 * Revision 1.1.2.7  2014/04/22 06:06:03  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.6  2014/03/11 03:44:09  leschen
 * Fix setting dash fpga clk/trig function.
 *
 * Revision 1.1.2.5  2014/03/07 07:44:55  kodko
 * Add check if the DPLL locks the correct reference pin.
 *
 * Revision 1.1.2.4  2014/02/24 09:02:44  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.3  2014/01/14 01:28:57  kodko
 * Fixs ZL3036X register is at page 4 instead page 2 issue.
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

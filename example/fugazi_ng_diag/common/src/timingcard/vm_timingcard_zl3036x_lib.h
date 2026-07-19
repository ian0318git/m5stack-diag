/* $Id: vm_timingcard_zl3036x_lib.h,v 1.3 2015/02/18 06:08:26 bowang3 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_zl3036x_lib.h,v $
 *******************************************************************************
 * File Name: vm_timingcard_zl3036x_lib.h
 *
 * Description: Timing Card ZL3036X header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_TIMINGCARD_ZL3036X_LIB_H_
#define VM_TIMINGCARD_ZL3036X_LIB_H_

/* ZL3036X I2C slave address */
#define TIMING_CARD_ZL3036X_I2C_ADDR            0x60
#define TIMING_CARD_ZL30361_I2C_ADDR            0x58

/* ZL3036X register offset page 0 */
#define ZL3036X_CENTRAL_FREQ_OFFSET_B           0x0B
#define ZL3036X_CENTRAL_FREQ_OFFSET_C           0x0C
#define ZL3036X_CENTRAL_FREQ_OFFSET_D           0x0D
#define ZL3036X_CENTRAL_FREQ_OFFSET_E           0x0E
#define ZL3036X_STICKY_LOCK_REG                 0x011

#define ZL3036X_REF_FAIL_ISR_MASK_10_8          0x024
#define ZL3036X_DPLL_ISR_MASK                   0x025
#define ZL3036X_REF_MON_FAIL_2                  0x028
#define ZL3036X_PHASEMEM_LIMIT_REF0             0x06A
#define ZL3036X_PHASEMEM_LIMIT_REF1             0x06B
#define ZL3036X_PHASEMEM_LIMIT_REF2             0x06C
#define ZL3036X_PHASEMEM_LIMIT_REF3             0x06D
#define ZL3036X_PHASEMEM_LIMIT_REF4             0x06E
#define ZL3036X_PAGE_SEL_REGISTER               0x07F

/* ZL3036X register offset page 1 */
#define ZL3036X_REF0_BASE_FREQ_LOW              0x080
#define ZL3036X_REF0_BASE_FREQ_HIGH             0x081
#define ZL3036X_REF0_BASE_FREQ_MULTIPLE_LOW     0x082
#define ZL3036X_REF0_BASE_FREQ_MULTIPLE_HIGH    0x083

#define ZL3036X_REF1_BASE_FREQ_LOW              0x088
#define ZL3036X_REF1_BASE_FREQ_HIGH             0x089
#define ZL3036X_REF1_BASE_FREQ_MULTIPLE_LOW     0x08A
#define ZL3036X_REF1_BASE_FREQ_MULTIPLE_HIGH    0x08B

#define ZL3036X_REF2_BASE_FREQ_LOW              0x090
#define ZL3036X_REF2_BASE_FREQ_HIGH             0x091
#define ZL3036X_REF2_BASE_FREQ_MULTIPLE_LOW     0x092
#define ZL3036X_REF2_BASE_FREQ_MULTIPLE_HIGH    0x093

#define ZL3036X_REF3_BASE_FREQ_LOW              0x098
#define ZL3036X_REF3_BASE_FREQ_HIGH             0x099
#define ZL3036X_REF3_BASE_FREQ_MULTIPLE_LOW     0x09A
#define ZL3036X_REF3_BASE_FREQ_MULTIPLE_HIGH    0x09B

#define ZL3036X_REF4_BASE_FREQ_LOW              0x0A0
#define ZL3036X_REF4_BASE_FREQ_HIGH             0x0A1
#define ZL3036X_REF4_BASE_FREQ_MULTIPLE_LOW     0x0A2
#define ZL3036X_REF4_BASE_FREQ_MULTIPLE_HIGH    0x0A3

/* ZL3036X register offset page 2 */
#define ZL3036X_DPLL0_MODE_REFSEL               0x103
#define ZL3036X_DPLL0_REFSEL_STAT               0x104
#define ZL3036X_DPLL0_REF_PRIORITY1_0           0x105
#define ZL3036X_DPLL0_REF_PRIORITY3_2           0x106
#define ZL3036X_DPLL0_REF_PRIORITY5_4           0x107
#define ZL3036X_DPLL1_MODE_REFSEL               0x123
#define ZL3036X_DPLL1_REFSEL_STAT               0x124
#define ZL3036X_DPLL1_REF_PRIORITY1_0           0x125
#define ZL3036X_DPLL1_REF_PRIORITY3_2           0x126
#define ZL3036X_DPLL1_REF_PRIORITY5_4           0x127

/* ZL3036X register offset page 3 */
#define ZL3036X_DPLL_HOLD_LOCK_STATUS           0x180
#define ZL3036X_DPLL_CONFIG                     0x182
#define ZL3036X_SYNTH_DRIVE_PLL                 0x1B0
#define ZL3036X_SYNTH_ENABLE                    0x1B1
#define ZL3036X_SYNTH0_BASE_FREQ_LOW            0x1B8
#define ZL3036X_SYNTH0_BASE_FREQ_HIGH           0x1B9
#define ZL3036X_SYNTH0_FREQ_MULTIPLE_LOW        0x1BA
#define ZL3036X_SYNTH0_FREQ_MULTIPLE_HIGH       0x1BB
#define ZL3036X_SYNTH1_BASE_FREQ_LOW            0x1C0
#define ZL3036X_SYNTH1_BASE_FREQ_HIGH           0x1C1
#define ZL3036X_SYNTH1_FREQ_MULTIPLE_LOW        0x1C2
#define ZL3036X_SYNTH1_FREQ_MULTIPLE_HIGH       0x1C3
#define ZL3036X_SYNTH2_BASE_FREQ_LOW            0x1C8
#define ZL3036X_SYNTH2_BASE_FREQ_HIGH           0x1C9
#define ZL3036X_SYNTH2_FREQ_MULTIPLE_LOW        0x1CA
#define ZL3036X_SYNTH2_FREQ_MULTIPLE_HIGH       0x1CB

/* ZL3036X register offset page 4 */
#define ZL3036X_SYNTH0_POST_DIV_C1              0x206
#define ZL3036X_SYNTH0_POST_DIV_C2              0x207
#define ZL3036X_SYNTH0_POST_DIV_C3              0x208
#define ZL3036X_SYNTH0_POST_DIV_D1              0x209
#define ZL3036X_SYNTH0_POST_DIV_D2              0x20A
#define ZL3036X_SYNTH0_POST_DIV_D3              0x20B
#define ZL3036X_SYNTH1_POST_DIV_C1              0x212
#define ZL3036X_SYNTH1_POST_DIV_C2              0x213
#define ZL3036X_SYNTH1_POST_DIV_C3              0x214
#define ZL3036X_SYNTH1_POST_DIV_D1              0x215
#define ZL3036X_SYNTH1_POST_DIV_D2              0x216
#define ZL3036X_SYNTH1_POST_DIV_D3              0x217
#define ZL3036X_SYNTH2_POST_DIV_C1              0x21E
#define ZL3036X_SYNTH2_POST_DIV_C2              0x21F
#define ZL3036X_SYNTH2_POST_DIV_C3              0x220
#define ZL3036X_GPIO_FUNCTION_PIN0              0x266
#define ZL3036X_GPIO_FUNCTION_PIN1              0x267
#define ZL3036X_GPIO_FUNCTION_PIN2              0x268
#define ZL3036X_GPIO_FUNCTION_PIN3              0x269
#define ZL3036X_GPIO_FUNCTION_PIN4              0x26A
#define ZL3036X_GPIO_FUNCTION_PIN5              0x26B
#define ZL3036X_GPIO_FUNCTION_PIN6              0x26C
#define ZL3036X_GPIO_PIN_IN_6_0                 0x276
#define ZL3036X_GPIO_PIN_OUT_6_0                0x278
#define ZL3036X_GPIO_OUT_EN_6_0                 0x27A

/* Priority 0 is highest, Priority is lowest */
#define DPLL_PRIORITY_0                         (0x0)
#define DPLL_PRIORITY_1                         (0x1)
#define DPLL_PRIORITY_2                         (0x2)
#define DPLL_PRIORITY_3                         (0x3)
#define DPLL_PRIORITY_4                         (0x4)
#define DPLL_PRIORITY_5                         (0x5)
#define DPLL_PRIORITY_6                         (0x6)
#define DPLL_PRIORITY_7                         (0x7)

#define SYNTH_8K_CLOCK                          0x1
#define DIVISION_8K_1                           0xE8
#define DIVISION_8K_2                           0x48

#define SYNTH_TRIGOUT_8K_CLOCK                  0x0
#define SYNTH_TRIGOUT_DIVISION_8K_1             0x0
#define SYNTH_TRIGOUT_DIVISION_8K_2             0x28

#define SYNTH_25M_CLOCK                         0x0
#define DIVISION_25M_1                          0x0
#define DIVISION_25M_2                          0x28

#define SYNTH_125M_CLOCK                        0x0
#define DIVISION_125M_1                         0x0
#define DIVISION_125M_2                         0x8

#define PTP_1PPS_1HZ_CLOCK                      0xF3
#define PTP_1PPS_1HZ_DIVIDER1                   0xC3
#define PTP_1PPS_1HZ_DIVIDER2                   0x50

/* ZL3036X GPIO input/output */
#define ZL3036X_GPIO_STATUS_MODE_FUNC_0         (0x0)

/* ZL3036X register offset page 2 */
#define ZL3036X_HP_COMS_EN                      0x262

#define ZL3036X_1_HZ                            0x0
#define ZL3036X_8K_HZ                           0x1
#define ZL3036X_25M_HZ                          0x2
#define ZL3036X_125M_HZ                         0x3

/* ZL3036X Base 1Hz frequency Br 0 */
#define ZL3036X_REF_BR0_1_HZ                    0x0001
/* ZL3036X Base 1 frequency multiple Kr 0 */
#define ZL3036X_REF_KR0_1                       0x0001

/* ZL3036X Base 1PPS frequency Br 0 */
#define ZL3036X_REF_BR0_1PPS                    0x9C40
/* ZL3036X Base 1PPS frequency multiple Kr 0 */
#define ZL3036X_REF_KR0_1PPS                    0x0271

/* ZL3036X Base 1kHz frequency Br 0 */
#define ZL3036X_REF_BR0_1K_HZ                   0x03E8
/* ZL3036X Base 8 frequency multiple Kr 0 */
#define ZL3036X_REF_KR0_8                       0x0008

/* ZL3036X Base 25kHz frequency Br 0 */
#define ZL3036X_REF_BR0_25K_HZ                  0x61A8
/* ZL3036X Base 1K frequency multiple Kr 0 */
#define ZL3036X_REF_KR0_1K                      0x03E8

/* ZL3036X Base 125MHz frequency Br 0 */
#define ZL3036X_REF_BR0_25K_HZ                  0x61A8
/* ZL3036X Base 1K frequency multiple Kr 0 */
#define ZL3036X_REF_KR0_5K                      0x1388

/* ZL3036X Base 1kHz frequency Br 3 */
#define ZL3036X_REF_BR3_1K_HZ                   0x03E8
/* ZL3036X Base 8 frequency multiple Kr 3 */
#define ZL3036X_REF_KR3_8                       0x0008

/* ZL3036X Base 25kHz frequency Br 3 */
#define ZL3036X_REF_BR3_25K_HZ                  0x61A8
/* ZL3036X Base 1K frequency multiple Kr 3 */
#define ZL3036X_REF_KR3_1K                      0x03E8

/* ZL3036X Base 1kHz frequency Br 4 */
#define ZL3036X_REF_BR4_1K_HZ                   0x03E8
/* ZL3036X Base 8 frequency multiple Kr 4 */
#define ZL3036X_REF_KR4_8                       0x0008

/* ZL3036X Base 25kHz frequency Br 4 */
#define ZL3036X_REF_BR4_25K_HZ                  0x61A8
/* ZL3036X Base 1K frequency multiple Kr 4 */
#define ZL3036X_REF_KR4_1K                      0x03E8

/* ZL3036X Base 8kHz frequency Bs */
#define ZL3036X_REF_BS0_8K_HZ                   0x61A8
/* ZL3036X Base frequency multiple Ks */
#define ZL3036X_REF_8K_KS0                      0x09C4

/* ZL3036X Base 25kHz frequency Bs */
#define ZL3036X_REF_BS0_25M_HZ                  0x61A8
/* ZL3036X Base 1K frequency multiple Ks */
#define ZL3036X_REF_25M_KS0                     0x09C4

/* ZL3036X Base 25kHz frequency Bs */
#define ZL3036X_REF_BS0_125M_HZ                 0x61A8
/* ZL3036X Base 1K frequency multiple Ks */
#define ZL3036X_REF_125M_KS0                    0x09C4

/* ZL3036X DPLL REF 0 */
#define ZL3036X_DPLL_REF_0                      0x0000
/* ZL3036X DPLL REF 1 */
#define ZL3036X_DPLL_REF_1                      0x0001
/* ZL3036X DPLL REF 2 */
#define ZL3036X_DPLL_REF_2                      0x0002
/* ZL3036X DPLL REF 3 */
#define ZL3036X_DPLL_REF_3                      0x0003
/* ZL3036X DPLL REF 4 */
#define ZL3036X_DPLL_REF_4                      0x0004

/* DPLL LOCK status */
#define ZL3036X_DPLL_LOCK_REF0                  0x0
#define ZL3036X_DPLL_LOCK_REF1                  0x1

/* ZL3036X REF Mode Selection */
#define ZL3036X_DPLL_FORCE_LOCK_MODE            0x2
#define ZL3036X_DPLL_SEL_REF0                   (0x0 << 4)
#define ZL3036X_DPLL_SEL_REF1                   (0x1 << 4)
#define ZL3036X_DPLL_SEL_REF2                   (0x2 << 4)
#define ZL3036X_DPLL_SEL_REF3                   (0x3 << 4)
#define ZL3036X_DPLL_SEL_REF4                   (0x4 << 4)

/* ZL3036X DPLL lock */
#define ZL3036X_DPLL0_LOCK                      (0x1 << 1)
#define ZL3036X_DPLL1_LOCK                      (0x1 << 3)

/* ZL3036X DPLL Synthesizer */
#define ZL3036X_DPLL_SYNTH1                     0xF3
#define ZL3036X_DPLL_SYNTH2                     0xCF
#define ZL3036X_DPLL_SYNTH1_2                   0x44
#define ZL3036X_DPLL_SYNTH0_2                   0xDC

/* ZL3036X Synth Enable */
#define ZL3036X_SYNTH1_EN                       (0x1 << 1)
#define ZL3036X_SYNTH2_EN                       (0x1 << 2)

/* ZL3036X HPOUT CLOCK selection */
#define ZL3036X_HP_COMS_EN_0                    (0x1 << 0)
#define ZL3036X_HP_COMS_EN_1                    (0x1 << 1)
#define ZL3036X_HP_COMS_EN_2                    (0x1 << 2)
#define ZL3036X_HP_COMS_EN_3                    (0x1 << 3)
#define ZL3036X_HP_COMS_EN_4                    (0x1 << 4)

/* O2 dash fpga SYNC/TRIG Control Register */
#define DASH_FPGA_TRIG_OUTPUT_EN                (0x1 << 24)
#define DASH_FPGA_TRIG_SELECT                   (0xFF << 16)
#define DASH_FPGA_TRIG_INTER_PLL                (0x18 << 16)
#define DASH_FPGA_TRIG_NGSM2                    (0x1 << 16)
#define DASH_FPGA_TRIG_NGVM                     (0x8 << 16)
#define DASH_FPGA_DIV_BY_5                      (0x1 << 10)
#define DASH_FPGA_DIV_BY_3125                   (0x1 << 9)
#define DASH_FPGA_SYNC_OUTPUT_EN                (0x1 << 8)
#define DASH_FPGA_SYNC_INTER_PLL                (0x19)
#define DASH_FPGA_SEL_NGVM                      (0x8)
#define DASH_FPGA_SYNC_OUT_NGSM1                (0x0)
#define DASH_FPGA_SYNC_OUT_NGSM2                (0x1)
#define DASH_FPGA_PTP_CLK_OUT_EN                (0x1 << 8)

#define NGSM_SLOT_ONE                           (0x1)
#define NGSM_SLOT_TWO                           (0x2)
#define OVERLORD_CAVIUM                         (0x3)
#define NGWIC_SLOT_ONE                          (0x4)
#define NGWIC_SLOT_TWO                          (0x5)
#define NGWIC_SLOT_THREE                        (0x6)

#define NGSM1_SYNC_OUT_SYNC_TRIG_OUT            (0x10)
#define NGSM2_SYNC_OUT_SYNC_TRIG_OUT            (0x14)
#define NGWIC1_SYNC_OUT_SYNC_TRIG_OUT           (0x20)
#define NGWIC2_SYNC_OUT_SYNC_TRIG_OUT           (0x24)
#define NGWIC3_SYNC_OUT_SYNC_TRIG_OUT           (0x28)
#define QUAD_PHY_SYNC_TRIG_CTRL_REG             (0x40)

#define PHASE_MEM_LIMIT_1MS                     (0x40)

#define ENABLE_SYNTH0_1_2_3                     (0xF)

#define DPLL_0_ENABLE                           (0x1)
#define DPLL_0_AND_1_ENABLE                     (0x2)

#define PHASE_MEMORY_LIMIT                      (0xA0)

#define REF2_FAIL_GST                           (0x1 << 3)
#define REF2_FAIL_CFM                           (0x1 << 2)
#define REF2_FAIL_SCM                           (0x1 << 1)

#define CHECK_LOCK_STS_TIMES                    50

/* Define ZL3036X page selection value */
typedef enum {
    ZL3036X_REG_PAGE_0 = 0,
    ZL3036X_REG_PAGE_1,
    ZL3036X_REG_PAGE_2,
    ZL3036X_REG_PAGE_3,
    ZL3036X_REG_PAGE_4,
} zl3036x_page_t;

extern long util_oir_zl3036x_reg_read(void);
extern long util_oir_zl3036x_reg_write(void);
extern long timingcard_zl3036x_reg_test_lib(void);
extern long timingcard_zl3036x_ref_x_lib(uint, uint);
extern void timingcard_o2_set_sync_trig_out(int, int, boolean);
extern void timingcard_o2_disable_sync_trig_out(void);
extern void timingcard_o2_set_sync1_out(int, int);
extern void timingcard_o2_disable_sync1_out(void);
extern void timingcard_o2_sync_trig_out_ctrl(boolean);
extern long timingcard_zl3036x_ref_x_lpbk_lib(uint, uint, boolean, boolean);
extern long sel_zl3036x_ref_clock_lib(void);
extern long zl3036x_set_gpio_dir(uint, uint);
extern long zl3036x_check_gpio_value(uint, uint, uchar, uint);
extern long zl3036x_set_gpio_value(uint, uchar);
extern void timingcard_o2_set_sm_sync_trig_out(uint, uint);
extern long timingcard_zl3036x_conf_ref_path(uint, uint, uint, boolean, boolean);
extern long timingcard_zl3036x_outx_lpbk_dpll_check(uint, uint, boolean);
extern long timingzard_zl30361_ref2_check(uchar *);
extern long timingcard_zl3036x_sticky_read(uint32, uchar *, int);
extern long clock_direction_lib(uint);

#endif /* VM_TIMINGCARD_ZL3036X_LIB_H_ */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_zl3036x_lib.h,v $
 * Revision 1.3  2015/02/18 06:08:26  bowang3
 * Support Wallander NIM 1588 test with timing card
 *
 * Revision 1.2  2015/02/14 12:48:43  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.8  2014/04/30 13:47:20  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.1.2.7  2014/04/25 06:56:34  kodko
 * Support ZL30361 reference 2 clock input test.
 *
 * Revision 1.1.2.6  2014/04/22 06:06:03  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.5  2014/03/11 03:50:20  leschen
 * Add macros to support dash FPGA setting.
 *
 * Revision 1.1.2.4  2014/03/07 07:44:55  kodko
 * Add check if the DPLL locks the correct reference pin.
 *
 * Revision 1.1.2.3  2014/02/24 09:02:44  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */


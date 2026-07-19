/* $Id: vm_timingcard_cpld_lib.h,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_cpld_lib.h,v $
 *******************************************************************************
 * File Name: vm_timingcard_cpld_lib.h
 *
 * Description: Timing Card CPLD header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_TIMINGCARD_CPLD_LIB_H
#define VM_TIMINGCARD_CPLD_LIB_H

#define TIMING_CARD_CPLD_I2C_ADDR       0x1F

/* Define CPLD register offset */
typedef enum {
    CPLD_NGIO_EXPANDER_INPUT = 0,
    CPLD_NGIO_EXPANDER_OUTPUT,
    CPLD_RESERVED,
    CPLD_NGIO_EXPANDER_DIR,
    CPLD_ZL30363_CONT_STS,
    CPLD_POW_SEQ_STS,
    CPLD_JTAG_CTL,
    CPLD_VERSION,
    CPLD_GPIO_DIRECTION,
    CPLD_SPEED_UP,
    CPLD_TEN,
    CPLD_ELEVEN,
    CPLD_TWELVE,
    CPLD_THIRTEEN,
    CPLD_FOURTEEN,
    CPLD_FIFTEEN,
} cpld_offset_t;

/* Power Sequence Status (Offset 5) */
#define CPLD_POWER_RESTART          (0x1 << 7)
#define CPLD_VP3P3_LDO_GOOD         (0x1 << 3)
#define CPLD_VP1P8_LDO_GOOD_L       (0x1 << 2)
#define CPLD_VP3P3_LDO_EN           (0x1 << 1)
#define CPLD_VP1P8_LDO_EN           (0x1 << 0)

/* JTAG Control (Offset 6) */
#define CPLD_JTAG_ON                (0x1 << 4)
#define CPLD_JTAG_TCK_ST            (0x1 << 3)
#define CPLD_JTAG_TMS_ST            (0x1 << 2)
#define CPLD_TDI_ST                 (0x1 << 1)
#define CPLD_TDO_ST                 (0x1 << 0)

/* ZL30363 and Control Status (Offset 4) */
#define CPLD_GPIO_0                 (0x1)
#define CPLD_GPIO_1                 (0x1 << 1)
#define CPLD_GPIO_2                 (0x1 << 2)
#define CPLD_GPIO_3                 (0x1 << 3)
#define CPLD_GPIO_4                 (0x1 << 4)
#define CPLD_GPIO_5                 (0x1 << 5)
#define CPLD_GPIO_6                 (0x1 << 6)

/* ZL30363 Expander Output (Offset 1) */
#define ZL30363_RESET_ACTIVE_HIGH   (0x1 << 5)

#define IOCPLD_VER_LEN              (8)
#define MAX_NOTE_LEN                (257)

/* CPLD Program */
#define CPLD_ERASE_COMMAND          0x013d
#define CPLD_SAMPLE_PRELOAD         0x0280
#define CPLD_ISP_ENABLE             0x0266
#define CPLD_ADDRESS_SHIFT          0x0301
#define CPLD_ISP_READ               0x0281
#define CPLD_ISC_PROG               0x00bd
#define CPLD_ISC_PROG_DONE_1        0xFFDF
#define CPLD_ISC_PROG_DONE_2        0xFFFF
#define CPLD_ISP_DISABLE            0x019a
#define CPLD_BYPASS                 0x03ff

extern long util_oir_cpld_reg_read(void);
extern long util_oir_cpld_reg_write(void);
extern long timingcard_cpld_reg_test_lib(void);
extern long timingcard_cpld_drive_gpio(uint);
extern long timingcard_cpld_check_gpio_val(uint, uint, uint);
extern long timingcard_cpld_reg_read_lib(uchar, uchar *);
extern long timingcard_cpld_reg_write_lib(uchar, uchar);
extern long timingcard_cpld_jtag_ctl(boolean);
extern long display_embedded_cpld_fw_version(uchar *);
extern long cpld_jtag_io(int, int, int);
extern long max2_cpld_program(void);
extern long timingcard_cpld_power_ctl(void);
extern long clock_verification_path_lib(uint);
extern long clock_direction_lib(uint);
extern long init_timingcard(void);
extern long timingcard_simply_program_cpld(unsigned char *);

#endif /* VM_TIMINGCARD_CPLD_LIB_H */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_cpld_lib.h,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.5  2014/03/11 03:36:37  leschen
 * Support 1588 clk/trig verification.
 *
 * Revision 1.1.2.4  2014/03/07 07:39:59  kodko
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
 * $Endlog $
 *------------------------------------------------------------------------------
 */

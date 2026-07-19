/* $Id: diag_mcu_test.h,v 1.3 2016/11/01 01:57:41 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mcu_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_mcu_test.h - Header file for MCU Test
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_MCU_TEST__
#define __DIAG_MCU_TEST__

extern int diag_mcu_test(int);
/* Registers offset defines */
#define ENV_MCU_FIRMWARE_REVISION               0x00
#define ENV_MCU_BOARD_TYPE                      0x01
#define ENV_MCU_STATUS_STICKY                   0x02
#define ENV_MCU_STATUS_CLEARABLE                0x03
#define ENV_MCU_VOLTAGE_FAULT_REG0_STICKY       0x04
#define ENV_MCU_VOLTAGE_FAULT_REG1_STICKY       0x05
#define ENV_MCU_VOLTAGE_FAULT_REG0_CLEARABLE    0x06
#define ENV_MCU_VOLTAGE_FAULT_REG1_CLEARABLE    0x07
#define ENV_MCU_SCRATCH_PAD_0                   0x08
#define ENV_MCU_SCRATCH_PAD_1                   0x09
#define ENV_MCU_SCRATCH_PAD_2                   0x0A
#define ENV_MCU_SCRATCH_PAD_3                   0x0B
#define ENV_MCU_SPARE                           0x0C


typedef uint8_t ren_o;      /* Renesas registers offset size */
typedef uint16_t ren_t;     /* Renesas registers size */

/* Simple Register Read/Write test table struct */
typedef struct mcu_reg_test_t_ {
    uint	option;
    ren_o	offset;
} mcu_reg_test_t;


/* Simple Register Read/Write test options */
typedef enum {
    MCU_REG_TEST_OP1 = 0,	/* Read only register */
    MCU_REG_TEST_OP2,		/* Write -25 and 120. same result */
    MCU_REG_TEST_OP3,		/* Write 0 and 30000. same result */
    MCU_REG_TEST_OP4,		/* Write 0 and 30000. 0 to 1 */
    MCU_REG_TEST_INVALID,	/* End of table */
} mcu_reg_test_opt_t;

/* Special Environmental Control Unit diag return code */
typedef enum {
    MCU_WR_RD_PASSED = 0,   /* Read/write successful */
    MCU_WR_FAILED,      /* Write failed */
    MCU_RD_FAILED,      /* Read failed */
} mcu_reg_ret_t;


#define EN_NIOS_MODE_DELAY       2000
#define ERR_BUF_SIZE             80
#define ENV_TEST_VER	         0x0300	/* Fully Env tests version supports */
#define ENV_MCU_AST	         0x2B
#define ENV_REG_TEST_OPTION1_PATTERN	0	/* test option 1 pattern */
#define ENV_REG_TEST_OPTION2_PATTERN1	-25	/* test option 2 low pattern */
#define ENV_REG_TEST_OPTION2_PATTERN2	120	/* test option 2 high pattern */

#define MCU_RO  (READ_ONLY | SAVE_RESTORE | REG_ACCESS)
#define MCU_RW  (READ_WRITE | SAVE_RESTORE | REG_ACCESS)

#endif /* __DIAG_MCU_TEST__ */

/*---------------------------------------------------------------
$Log: diag_mcu_test.h,v $
Revision 1.3  2016/11/01 01:57:41  iachang
Add dealy time for en/disable nios mode for MCU test failure

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/08/31 08:06:42  meho
Fixed MCU register test bug.

Revision 1.1.2.2  2015/07/31 07:36:43  hondwang
mcu test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/


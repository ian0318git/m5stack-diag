/* $Id: diag_cpld_lib.h,v 1.2 2021/04/15 00:52:23 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_cpld_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_cpld_lib.h
 * Description: CPLD library header file.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define PHOENIX_CPLD_KLM             "cpld"
#define CPLD_SIZE_BAR0               0x1000000
#define CPLD_SIZE_BAR1               0x1000

/* CPLD DEVICE ADDRESS */
#define CPLD_PCIE_BAR0_ADDRESS       0x85000000
#define CPLD_PCIE_BAR1_ADDRESS       0x86000000

/* CPLD */
#define CPLD_PCIE_ENABLE_BAR1_REG    0x10
#define CPLD_PCIE_ENABLE             0xCA80

/* CPLD LED Reg */
#define CPLD_STATUS_LED_REG          0x14
#define PHOENIX_LED_OFF               (0 << 0)
#define STATUS_LED_YELLOW            0x2
#define STATUS_LED_GREEN             0x3

#define CPLD_IRQ0                    0x040000
#define CPLD_IRQ6                    0x100000
#define CPLD_MAGIC_NUM_CA            0xCA

#define CPLD_TIMEOUT                 10

extern int cpld_read_reg(uint, uint32_t *);
extern int cpld_write_reg(uint, uint);
extern int cpld_register_operation(uint, uint, uint);
extern int open_cpld(void);
extern int cpld_enable_pcie_bar1(void);
extern int cpld_reset_api(uint, uint, uint, uint);
extern void cpld_set_irq(int);
extern int cpld_check_irq(int);


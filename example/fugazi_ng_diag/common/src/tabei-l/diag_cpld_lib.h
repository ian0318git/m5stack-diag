/* $Id: diag_cpld_lib.h,v 1.2 2019/10/17 02:16:20 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_cpld_lib.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_cpld_lib.h
 * Description: CPLD library header file.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */

#define TABEI_L_CPLD_KLM             "cpld"
#define CPLD_SIZE_BAR0               0x1000000
#define CPLD_SIZE_BAR1               0x1000

/* CPLD DEVICE ADDRESS */
#define CPLD_PCIE_BAR0_ADDRESS       0xDD000000
#define CPLD_PCIE_BAR1_ADDRESS       0xDE000000
#define CPLD_PROMETHIUM_ADDRESS      0xFED40000

/* CPLD */
#define CPLD_PCIE_ENABLE_BAR1_REG    0x10
#define CPLD_PCIE_ENABLE             0xCA80

/* CPLD LED Reg */
#define CPLD_STATUS_LED_REG          0x14
#define TABEI_LED_OFF               (0 << 0)
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

/*-------------------------------------------------
 * $Log: diag_cpld_lib.h,v $
 * Revision 1.2  2019/10/17 02:16:20  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.8  2019/09/27 07:57:23  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.7  2019/09/05 09:30:40  kehuang2
 * Support Promethium Init CPLD
 *
 * Revision 1.1.2.6  2019/09/05 08:50:35  olin2
 * Support FPGA serial IRQ interrupt util
 *
 * Revision 1.1.2.5  2019/07/08 01:49:20  kehuang2
 * Rename variable to avoid redefined with PIM module
 *
 * Revision 1.1.2.4  2019/06/17 03:23:45  olin2
 * Support CPLD reset api
 *
 * Revision 1.1.2.3  2019/04/24 07:59:20  kehuang2
 * Update CPLD access
 *
 * Revision 1.1.2.2  2019/04/19 03:25:40  kehuang2
 * Support CPLD access
 *
 * Revision 1.1.2.1  2018/10/15 12:30:13  kodko
 * Add CPLD register read/write function.
 *
 * $Endlog$
 *-------------------------------------------------
 */

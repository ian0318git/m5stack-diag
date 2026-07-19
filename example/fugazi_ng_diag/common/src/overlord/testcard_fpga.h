/* $Id: testcard_fpga.h,v 1.2 2014/08/05 12:08:25 danchung Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_fpga.h,v $
 *--------------------------------------------------------------------
 * Filename   : testcard_fpga.h
 *
 * Description: Specific header file of TestCard FPGA.
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *--------------------------------------------------------------------
 */

#ifndef __TESTCARD_FPGA_H__
#define __TESTCARD_FPGA_H__

/* Common definition */
#define FPGA_REG_TEST_PATTERN  0xA56C
#define TC_FPGA_REV_MSK        0x7FFF

/* FPGA Registers */
/* FPGA Reg. offset */
#define FPGA_ID_REG_OFFSET     0x00
#define DEV_RESET_REG_OFFSET   0x02
#define SMI0_STAT_REG_OFFSET   0x04
#define SMI0_CTRL_REG_OFFSET   0x06
#define SMI0_ADDR_REG_OFFSET   0x08
#define SMI1_STAT_REG_OFFSET   0x0A
#define SMI1_CTRL_REG_OFFSET   0x0C
#define SMI1_ADDR_REG_OFFSET   0x0E
#define UART_REG_OFFSET        0x10
#define PCIE_LPBK_REG_OFFSET   0x12
#define GPIO_STAT_REG_OFFSET   0x14
#define EXT_MOD_REG_OFFSET     0x16
#define GPIO_INTR_REG_OFFSET   0x18
#define PHY_INTR_REG_OFFSET    0x1A
#define SYNC_LPBK_REG_OFFSET   0x1C
#define HOT_SWAP_REG_OFFSET    0x1E
#define PLL_LOCK_REG_OFFSET    0x20
#define MINUS_54V_REG_OFFSET   0x22

/* FPGA Reg. definition */
/* FPGA ID Reg. (+0x00h) */
#define FPGA_DEBUG_SHIFT     15
#define FPGA_DEBUG_MSK       0x8000
#define FPGA_MAJ_REV_SHIFT   8
#define FPGA_MAJ_REV_MSK     0x7F00
#define FPGA_MIN_REG_MSK     0x00FF

/* Individual Device Reset Reg. (+0x02h) */
#define EXT_MOD_RESET_SHIFT      6
#define EXT_MOD_RESET            0x40
#define PCIE_REDRV_RESET_SHIFT   5
#define PCIE_REDRV_RESET         0x20
#define SMI1_RESET_SHIFT         4
#define SMI1_RESET               0x10
#define SMI0_RESET_SHIFT         3
#define SMI0_RESET               0x08
#define PCIE_RESET_SHIFT         2
#define PCIE_RESET               0x04
#define XAUI_RESET_SHIFT         1
#define XAUI_RESET               0x02
#define GE_RESET                 0x01

/* SMI0 Speed/Status Reg. (+0x04h)
 * SMI1 Speed/Status Reg. (+0x0Ah)
 */
#define SMI_SPEED_DIV_SHIFT  8
#define SMI_SPEED_DIV_MSK    0x3F00
#define SIM_PREAM_EN_SHIFT   7
#define SMI_PREAM_EN         0x0080
#define SMI_CHAN_BUSY        0x0001

/* SMI0 Control Reg. (+0x06h)
 * SMI1 Control Reg. (+0x0Ch)
 */
#define SMI_SOF_SHIFT        14
#define SMI_SOF_MSK          0xC000
#define SMI_OP_CODE_SHIFT    12
#define SMI_OP_CODE_MSK      0x3000
#define SMI_PHY_ADDR_SHIFT   7
#define SMI_PHY_ADDR_MSK     0x0F80
#define SMI_DEV_ADDR_SHIFT   2
#define SMI_DEV_ADDR_MSK     0x007C

/* Start of Frame */
#define CLAUSE_45_FRAME      0x0
#define CLAUSE_22_FRAME      0x1

/* OPT code */
#define CLAUSE_45_ADDR       0x0
#define WRITE_FRAME          0x1
#define READ_22_FRAME        0x2
#define READ_45_P_INCR_ADDR  0x2   /* Read frame plus auto increment register address for Clause 45 */
#define READ_45_FRAME        0x3

/* SMI0 Data/Address Reg. (+0x08h)
 * SMI1 Data/Address Reg. (+0x0Eh)
 */
#define SMI_ADDR_MSK         0xFFFF

/* UART Control Reg. (+0x10h) */
#define UART_LINE_LPBK       0x0002
#define UART_HOST_LPBK       0x0001

/* PCIe Loopback Reg. (+0x12h) */
#define PCIE_HOST_LPBK       0x0001

/* GPIO-16 Port Expander Status Reg. (+0x14h) */
#define NGIO_GPIO15          0x8000
#define NGIO_GPIO14          0x4000
#define NGIO_GPIO13          0x2000
#define NGIO_GPIO12          0x1000
#define NGIO_GPIO11          0x0800
#define NGIO_GPIO10          0x0400
#define NGIO_GPIO9           0x0200
#define NGIO_GPIO8           0x0100
#define NGIO_GPIO7           0x0080
#define NGIO_GPIO6           0x0040
#define NGIO_GPIO5           0x0020
#define NGIO_GPIO4           0x0010
#define NGIO_GPIO3           0x0008
#define NGIO_GPIO2           0x0004
#define NGIO_GPIO1           0x0002
#define NGIO_GPIO0           0x0001

/* External Module Control Reg. (+0x16h) */
#define EXT_DB_GPIO9         0x0100
#define EXT_DB_GPIO8         0x0080
#define EXT_DB_GPIO7         0x0040
#define EXT_DB_GPIO6         0x0020
#define EXT_DB_GPIO5         0x0010
#define EXT_DB_GPIO4         0x0008
#define EXT_DB_GPIO3         0x0004
#define EXT_DB_GPIO2         0x0002
#define EXT_DB_GPIO1         0x0001

/* GPIO-16 Expander Interrupt Reg. (+0x18h) */
#define GPIO_EXPANDER_INTR   0x0001

/* GE and XAUI PHY Interrupt Reg. (+0x1Ah) */
#define XAUI_PHY_INTR        0x0002
#define GE_PHY_INTR          0x0001

/* SyncE Loopback Control Reg. (+0x1Ch) */
#define SYNC_TRIG_LPBK       0x0002
#define SYNC_LPBK            0x0001

/* Hot Swap Controller GPIO3 Reg. (+0x1Eh) */
#define HOT_SWAP_GPIO3_STAT  0x0001

/* PLL Lock Reg. (+0x20h) */
#define PLL_LOCK_PCIE_REF    0x0001

/* -54V Presetn Reg. (+0x22h) */
#define MINUS_54V_AUX        0x0002
#define MINUS_54V_MAIN       0x0001

/* Externs */
extern int tc_fpga_reg_rd(uint32_t, uint16_t *);
extern int tc_fpga_reg_wr(uint32_t, uint16_t);
extern int tc_alter_fpga_reg_wrap(void);
extern int tc_read_fpga_reg_wrap(void);
extern int tc_set_fpga_reg(uint32_t, uint16_t, uint8_t);
extern int tc_fpga_reset_device(uint16_t);
extern int tc_fpga_unreset_device(uint16_t);
extern int tc_fpga_set_sync_lpbk(boolean);
extern int tc_sync_sig_test(void);


#endif /* __TESTCARD_FPGA_H__ */

/* ------- End of file ------- */

/******** History ******** 
$Log: testcard_fpga.h,v $
Revision 1.2  2014/08/05 12:08:25  danchung
Support NIM test card POE -54V detection test

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.4  2012/11/21 19:50:08  palin2
1. Add TestCard PLL Lock test support.
2. Update FPGA registers map to v1.05 with backward compatible.

Revision 1.3  2012/11/14 19:15:48  palin2
To support Sync Signals loopback test on TestCards.

Revision 1.2  2012/08/22 16:39:55  palin2
Put XAUI into Reset state when exits XAUI related tests to avoid
affecting other interface.

Revision 1.1  2012/08/14 11:30:55  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.4  2012/08/08 22:19:41  palin2
1. Move TestCard UART external loopback test to "testcard_uart.c".
2. Add support TestCard UART internal loopback test and related utilities.

Revision 1.3  2012/08/03 08:34:49  palin2
Update TestCard FPGA registers map to rev 1.02.

Revision 1.2  2012/07/31 17:08:20  palin2
Initial check-in for TestCard PCIe tests.

Revision 1.1  2012/07/23 17:33:54  palin2
Initial check-in for Overlord Test Card diag tests.


$Endlog$
*/

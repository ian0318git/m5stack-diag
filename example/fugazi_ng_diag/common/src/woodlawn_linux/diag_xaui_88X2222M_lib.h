/* $Id: diag_xaui_88X2222M_lib.h,v 1.6 2015/02/14 12:48:41 kodko Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_xaui_88X2222M_lib.h,v $
 *-----------------------------------------------------------------------------
 * diag_xaui_88X2222M_lib.h
 *
 * February 2012, Kody Ko
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __DIAG_XAUI_88X2222M_LIB_H__
#define __DIAG_XAUI_88X2222M_LIB_H__

#define SFP_EEPROM_ADDR         (0x50)
#define SFP_EEPROM_SIZE         (256)

typedef enum mrvl_88x2222m_phy_t_ {
    MRVL_88X2222M_XAUI,
} mrvl_88x2222m_xaui_t;

#define MRVL_88X2222M_PORT_0_ADDR       (0x4)
#define MRVL_88X2222M_PORT_2_ADDR       (0x6)
#define MRVL_88X2222M_SMI2_ADDR         (0x2)
#define MRVL_88X2222M_PORTS             (2)

#define MRVL_88X2222M_SMI2_PORT0_ADDR   (MRVL_88X2222M_PORT_0_ADDR | \
                                        (MRVL_88X2222M_SMI2_ADDR << 4))
#define MRVL_88X2222M_SMI2_PORT2_ADDR   (MRVL_88X2222M_PORT_2_ADDR | \
                                        (MRVL_88X2222M_SMI2_ADDR << 4))

#define MRVL_88X2222M_PHY_REG_LEN       (2)

/* 88X2222M Device address and Register */
/* Device Address 1 */
#define MRV88X2222M_REG_DEVICE_1        (0x1)

/* Device Address 2 */
#define MRV88X2222M_REG_DEVICE_2        (0x2)

/* Device Address 3 */
#define MRV88X2222M_REG_DEVICE_3        (0x3)
/* Register Offset 0xF030 of Device Address 3 */
#define PRBS_CONTROL_LANE_0             (0xF030)
#define PRBS_MODE_MASK                  (0xF)
#define PRBS31_PATTERN_MODE             (0x0)
#define PRBS9_PATTERN_MODE              (0x2)
#define SQUARE_WAVE_PATTERN_MODE        (0xF)
#define ENABLE_CHECKER                     (0x1 << 4)
#define ENABLE_GENERATOR                (0x1 << 5)

/* Device Address 4 */
#define MRV88X2222M_REG_DEVICE_4        (0x4)
/* Register Offset 0xF003 of Device Address 4 */
#define SERDES_CTRL_REG                 (0xF003)
#define EN_HOST_LPBK                       (0x1 << 12)
#define HOST_LPBK_MASK                  (0xF7FF)
#define XFI_PCS_CTRL1_REG             (0x1000)
#define XFI_PCS_STATUS_1_REG      (0x1001)
#define XFI_PCS_LINK_STATUS         (0x1 << 2)
#define SFI_PCS_STATUS_1_REG      (0x1001)
#define SFI_LINE_SIDE_PCS_STATUS_REG (0x0001)
#define XFI_PCS_CTRL1_VALUE         (0xa040)
#define SERDES_CTRL_VALUE            (0x1010)

/* Device Address 30 */
#define MRV88X2222M_REG_DEVICE_30       (0x1E)
#define INIT_REPLUG_MODULE_REG          (0xb049)

/* Device Address 31 */
#define MRV88X2222M_REG_DEVICE_31       (0x1F)
#define MRV88X2222M_F2R_CFG_REG         (0xF000)
#define MRV88X2222M_F2R_MAC_PWR_STS     (0x0C00)

/* Register Offset 0xF012 of Device Address 31 */
#define GPIO_DATA                                       (0xF012)
#define DETECT_SFP_PLUS_MODULE                (0xFFFE)
/* Register Offset 0xF400 of Device Address 31 */
#define TRANS_SOUR_N_REG                (0xF400)
#define LINE_PORT_POWERDOWN             (0x0)
#define LINE_PORT_IDLE                  (0x1)
#define SELECT_XFI_PORT_0               (0x8)
#define SELECT_XFI_PORT_1               (0x9)
#define SELECT_XFI_PORT_2               (0xa)
#define SELECT_XFI_PORT_3               (0xb)
/* Register Offset 0xF401 of Device Address 31 */
#define TRANS_SOUR_M_REG                (0xF401)
#define HOST_PORT_POWERDOWN             (0x0)
#define HOST_PORT_IDLE                  (0x1)
#define SELECT_SFI_PORT_0               (0x8)
#define SELECT_SFI_PORT_2               (0xa)

#define EEPROM_OP_TIMEOUT               (500) /* 5 secs */
/* EEPROM Register */
#define EEPROM_ADDRESS_REGISTER         (0x8001)
#define EEPROM_READ_DATA_REGISTER       (0x8002)
#define EEPROM_WRITE_DATA_REGISTER      (0x8003)

/* EEPROM Address Register bit */
#define EEPROM_SLAVE_ADDRESS_SHIFT      (9)
#define EEPROM_SLAVE_COMMAND_WRITE      (0x0000)
#define EEPROM_SLAVE_COMMAND_READ       (0x0100)

/* EEPROM Read Data bit */
#define EEPROM_TWSI_STATUS_SHIFT        (8)
#define EEPROM_TWSI_STATUS_READY        (0x0)
#define EEPROM_TWSI_STATUS_DONE         (0x1)
#define EEPROM_TWSI_STATUS_IN_PROG      (0x2)
#define EEPROM_TWSI_STATUS_READ_FAILED  (0x3)
#define EEPROM_TWSI_STATUS_COM_FAILED   (0x5)
#define EEPROM_TWSI_STATUS_IFACE_BUSY   (0x7)

#define GPIO_TRISTATE_CTRL                          (0xF013)
#define GPIO_INTERRUPT_TYPE                       (0xF016)
#define MRVL2222M_SCL_OUTPUT_MASK          (0x0800)
#define MRVL2222M_SDA_OUTPUT_MASK         (0x0400)
#define MRVL2222M_SDA_FUNC_MASK             (0x0800)
#define MRVL2222M_SCL_FUNC_MASK              (0x8000)
/* Need to read status register more than once to get real time link value */
#define RETRY_STICKY_BIT               (10)
#define RETRY_LIMIT                    (9)

#define MRVL2222M_Z1       (0xdb1)
#define MRVL2222M_A0_REV1  (0xf12)
#define MRVL2222M_A0_REV2  (0xf16)
#define MRVL2222P          (0xd98)
#define MRVL2222P_A0       (0xd99)

#define LED1_CONTROL_REG (0xf021);
#define SFP_PLUS_SPEED_LED_ON (0x764)
#define SFP_PLUS_SPEED_LED_OFF (0x60)
#define PTP_DATA_LO        (0x2)
#define PTP_DATA_HI        (0x3)
#define PTP_VERIFY_NUM     (3) 
#define WAIT_PHY_READY     (100)
#define PTP_CNT_DELAY      (1000)
#define PTP_CONFIG_DELAY   (1000)
#define PTP_INIT_PULSE_IN_CNT        (0xff) 
#define PTP_PULSE_IN_CNT_FULL        (0xff)
#define PTP_INIT_CLOCK_IN_CNT        (0xff)
#define PTP_CLOCK_IN_CNT_FULL        (0xffff)
#define MRVL2222P_TRIG_VERIFY_TIME   (10)
#define MRVL2222P_CLK_VERIFY_TIME    (10)
#define MRVL2222P_TRIG_VERIFY_NUM    (3)
#define MRVL2222P_CLK_VERIFY_NUM     (3)
#define MRVL2222P_PTP_READ_DELAY     (500)

#define MRV2222P_PULSE_IN_COUNT_REG  (0xbc44)
#define MRV2222P_CLOCK_IN_COUNT_REG  (0xbc5c)
#define MRV2222P_INDIRECT_RD_ADDR    (0x97fd)
#define MRV2222P_INDIRECT_RD_DATA_LO (0x97fe)
#define MRV2222P_INDIRECT_RD_DATA_HI (0x97ff)

/* TOD Alignment Clock Cycle */
#define MRV2222P_CLOCK_CYC           (0xbc58)

/* TAI Clock Configurations */
#define MRV2222P_TOD_CFG_GEN         (0xbc16)

/* TOD/Pulse-Trigger Functions */
#define MRV2222P_TOD_FUNC_CFG        (0xbc46)

/* Pulse-Trigger Generation Mask */
#define MRV2222P_TRIG_GEN_MASK0      (0xbc22)
#define MRV2222P_TRIG_GEN_MASK1      (0xbc24)
#define MRV2222P_TRIG_GEN_MASK2      (0xbc26)
#define MRV2222P_TRIG_GEN_MASK3      (0xbc28)

/* Pulse-Trigger Generation */
#define MRV2222P_TRIG_GEN_TOD0       (0xbc1a)
#define MRV2222P_TRIG_GEN_TOD1       (0xbc1c)
#define MRV2222P_TRIG_GEN_TOD2       (0xbc1e)
#define MRV2222P_TRIG_GEN_TOD3       (0xbc20)

extern int mrvl_88X2222M_en_ext_lpbk(void);;
extern void marvell_2222m_init(void);
extern void marvell_2222m_sfi_compliance_testing(void);
extern int mrvl_88X2222M_sel_port2(void);
extern int mrvl_88X2222M_is_sfp_plus_present(void);
extern int mrvl_88X2222M_is_sfp_plus_present(void);
extern int mrvl_88X2222M_read_i2c(int, int, char *);
extern int mrvl_88X2222M_disp_sfp_eeprom(void);
extern int turn_off_mrvl_88X2222M_i2c(void);
extern int switch_sfp_plus_led(int);
extern int dump_phy_88X2222M_registers(void);
extern int alter_phy_88X2222M_register(void);
extern int dump_phy_88X2222M_ptp_register(void);
extern int alter_phy_88X2222M_ptp_register(void);
extern int verify_2222_clk_trig_in(void);
extern int config_2222_gen_clk_out(void);
extern int config_2222_gen_trig_out(void);
extern int enable_mrvl2222m_macsec_power(void);

#endif
/*-------------------------------------------------
 * $Log: diag_xaui_88X2222M_lib.h,v $
 * Revision 1.6  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.5  2015/02/04 07:23:12  leschen
 * Fix for sfp+ speed led control.
 *
 * Revision 1.4  2014/03/17 06:51:28  leschen
 * Add 2222P A0 id.
 *
 * Revision 1.3.2.2  2014/05/02 02:43:21  kodko
 * Modify the verify time from 10000 to 10.
 *
 * Revision 1.3.2.1  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.3  2013/12/12 09:19:07  leschen
 * Defines macros for init script
 *
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/06/10 13:25:21  leschen
 * Add check sfi line side link status macro
 *
 * Revision 1.1.2.2  2013/05/17 05:56:26  leschen
 * Add 88X2222 register macro
 *
 * Revision 1.1.2.1  2013/04/24 10:37:19  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/04/10 11:17:44  leslie
 * Add turn off 88X2222M I2C bus macros
 *
 * Revision 1.2  2013/03/20 10:34:55  kuangik
 * Implement I2C Read Utility and SFP+ eeprom display utility
 *
 * Revision 1.10  2012/12/11 00:58:59  leslie
 * Add 88X2222M register macro.
 *
 * Revision 1.9  2012/11/19 02:38:06  leslie
 * Add extern declaratoin of function 2222m sfi compliance testing.
 *
 * Revision 1.8  2012/09/21 11:46:58  kody
 * Add 88X2222M definition.
 *
 * Revision 1.7  2012/09/05 22:27:33  leslie
 * Update phy address.
 *
 * Revision 1.6  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/07/09 08:49:32  kody
 * Add Phy 2222M PRBS test mode in utilities.
 *
 * Revision 1.3  2012/05/18 10:21:29  kody
 * Add 88X2222M register Macro
 *
 * Revision 1.2  2012/05/15 01:33:14  leslie
 * Update for 88X2222M test item
 *
 * Revision 1.1  2012/04/16 02:39:42  kody
 * Add Marvell XAUI 88X2222M test.
 *
 * $Endlog$
 *-------------------------------------------------
 */

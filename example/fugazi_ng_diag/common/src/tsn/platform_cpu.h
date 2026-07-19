/* $Id: platform_cpu.h,v 1.8 2018/11/23 08:49:51 hondwang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_cpu.h,v $ 
 *------------------------------------------------------------------
 *
 * Filename   : platform_cpu.h
 * Description: Header file of TSN CPU Library.
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

/* Common defines */
typedef enum {
    CPUMAC_LINKDOWN = 0,
    CPUMAC_LINKUP,
} cpu_mac_linkopt;


/* Register map */
#define TSN_MCU_VERSION_REG     0x00
#define TSN_MCU_PS_MARGIN_REG   0x17

/* Port Auto-Negotiation Config Reg(m = 0-3) */
#define CPU_PORT_AN_CONF_REG(m)        (uint)(0xF2130E0C + (m*0x1000))
#define CPU_PACR_F_LINKUP         (uint)(1 << 1)
#define CPU_PACR_F_LINKDOWN       (uint)(1)

/* Port MAC Control Config Reg(m = 0-3) */
#define CPU_PORT_MAC_CTRL_REG(m)   (0xF2130E00 + (m*0x1000))

/* Port Status Reg0(m = 0-3) */
#define CPU_PORT_STATUS_REG0(m)   (uint)(0xF2130E10 + (m*0x1000))
#define CPU_PSR0_LINKUP           (uint)(1)   /* bit0 */

/* PHY Test Control 0 Register(n = 0-5) */
#define CPU_PHY_0_CONTORL_REG(n)			(0xF2120854 + (n*0x1000))
#define CPU_PHY_0_CONTORL_TEST_ENABLE_0		0x0000
#define CPU_PHY_0_CONTORL_TEST_ENABLE_1		0x00d0
#define CPU_PHY_0_CONTORL_TEST_ENABLE_2		0x80d0

/* Digital Loopback Enable Register(n = 0-5) */
#define CPU_DIGITAL_LOOPBACK_ENABLE_REG(n)	(0xF212088C + (n*0x1000))
#define CPU_LOCAL_DIG_RX2TX_LPBK_EN			0x8000	/* (1 << 15) */
#define CPU_RXPHER_TO_TX_EN					0x1000	/* (1 << 12) */

#define PANCR_RESERVED            0x8000   /* 1 << 15 */
#define PANCR_AUTOMEDIA_SEL_EN    0x4000   /* 1 << 14 */
#define PANCR_AN_DUPLEX_EN        0x2000   /* 1 << 13 */
#define PANCR_SET_FULL_DUPLEX     0x1000   /* 1 << 12 */
#define PANCR_AN_FC_EN            0x0800   /* 1 << 11 */
#define PANCR_SUPPORT_FC          0x0200   /* 1 <<  9 */
#define PANCR_AN_SPEED_EN         0x0080   /* 1 <<  7 */
#define PANCR_SET_SGMII_1000      0x0040   /* 1 <<  6 */
#define PANCR_SET_MII_100         0x0020   /* 1 <<  5 */
#define PANCR_INBAND_RESTART_AN   0x0010   /* 1 <<  4 */
#define PANCR_INBAND_BYPASS_EN    0x0008   /* 1 <<  3 */
#define PANCR_INBAND_AN_EN        0x0004   /* 1 <<  2 */
#define PANCR_FORCE_LINK_UP       0x0002   /* 1 <<  1 */
#define PANCR_FORCE_LINK_DOWN     0x0001   /* 1 <<  0 */

#define PANCR_FORCE_LINK_MSK      0x0003   /* bit 0 and bit 1 */

/*
 * Device Bus
 */
/* Common */
#define TSN_DEVBUS_CONF_TIME      (20)   /* 20ms */
#define TSN_DEVBUS_0              (0)
#define TSN_DEVBUS_1              (1)

/* Type struct of device bus configure registers */
typedef struct devbus_conf_t_ {
    char *desc;
    int  bus_num;
    int  rd_param;
    int  wr_param;
    int  polarity;
    int  ignore;
} devbus_conf_t;

/* Definition of device bus number */
#define TSN_DEVBUS_0   (0)
#define TSN_DEVBUS_1   (1)

#define TSN_FPGA_DEVBUS_NUM     (TSN_DEVBUS_0)
#define TSN_AIKIDO_DEVBUS_NUM   (TSN_DEVBUS_1)

/* Bridge Window Control Reg. */
#define BRIDGE_WIN_CTRL_REG_ADDR(x)   (0xf270ff00 + (0x8 * (x)))
/* Definition of device bus window size */
#define TSN_DEVBUS_WINSIZE_16MB       ((0xff) << 16)   /* [31:16] window size */
/* Definition of device bus window attribute */
#define TSN_DEVBUS0_ATTR              ((0x3e) << 8)    /* [15:8] window attributes */
#define TSN_DEVBUS1_ATTR              ((0x3d) << 8)    /* [15:8] window attributes */
/* Definition of device bus window enable */
#define TSN_DEVBUS_WIN_EN             (1)              /* bit 0: window enable */

/* Bridge Window Base Reg. */
#define BRIDGE_WIN_BASE_REG_ADDR(x)   (0xf270ff04 + (0x8 * (x)))
/* Definition of device bus window base */
#define DEVBUS_WINBASE_MSK            (0xffff0000)     /* [31:16] base addr */

/* Dev_CS[x] Read Parameters Reg. */
#define DEVBUS_RD_PARAMS_REG_ADDR(x)  (0xf2700408 + (0x8 * (x)))
/* Definition of device bus read parameters */
#define TSN_DEVBUS0_RD_PARAM          (0x003e07cf)
#define TSN_DEVBUS1_RD_PARAM          (0x001e0c0f)

/* Definition of Star device bus timing parameters */
#define STAR_DEVBUS0_RD_PARAM          (0x003e09cf)

/* Dev_CS[x] Write Parameters Reg. */
#define DEVBUS_WR_PARAMS_REG_ADDR(x)  (0xf270040c + (0x8 * (x)))
/* Definition of device bus write parameters */
#define TSN_DEVBUS0_WR_PARAM          (0x000f0f0f)
#define TSN_DEVBUS1_WR_PARAM          (0x000f1c0f)

/* Device Bus Synchronous Control Reg. */
#define DEVBUS_SYNC_CTRL_REG_ADDR     (0xf27004c8)
/* Definition of device bus ready polarity */
#define TSN_DEVBUS_ACT_LOW            (0)
#define TSN_DEVBUS_ACT_HIGH           (1)
/* Definition of device bus ready ignore */
#define TSN_DEVBUS_READY_IGNORED      (1)
/* Definition of device bus CS ready polarity and ignore */
#define DBSCR_POLAR_SHIFT(x)          (10 + (5 * x))
#define DBSCR_POLAR_MSK               (0x1)
#define DBSCR_IGNORE_SHIFT(x)         (9 + (5 * x))
#define DBSCR_IGNORE_MSK              (0x1)

/* Externs */
extern uint tsn_fpga_reg_baseaddr;
extern uint tsn_aikido_reg_baseaddr;
extern int tsn_devbus_init(int);
extern int tsn_get_devbus_baseaddr(int, uint *);
extern void tsn_show_devbus_info(void);
extern int tsn_cpu_ondie_temp(int);
extern int tsn_cpu_mac_check_linkstat(int, boolean);
extern int tsn_cpu_mac_config(int, uint);

/*
 * Define ECC relative commands
 */
#define ECC_ERR_LOG_CONFIG            "devmem 0xf0020360 32 0x1"
#define ECC_1BIT_ERR_COUNTER          "devmem 0xf0020364 32 0x0"
#define ECC_ERR_INFO_0           	  "devmem 0xf0020368 32 0x0"
#define ECC_ERR_INFO_1           	  "devmem 0xf002036c 32 0x0"
#define INTERRUPT_STATUS_REG          "devmem 0xf0020140 32 0x1000"
#define INTERRUPT_ENABLE_REG          "devmem 0xf0020144 32 0x1000"
#define PHY_REG_FILE_ACCESS_0         "devmem 0xf00116a0 32 0xc0030003"
#define PHY_REG_FILE_ACCESS_1         "devmem 0xf00116a0 32 0xd010001f"

/*******************************************************************************
 *                                   Global
 *******************************************************************************
 */
#define CPU_AP_REG_BASE         0xF0000000
#define CPU_ONDIE_TEMP_REG      0x6F808C
#define CPU_SAR_REG             0x6F4400
#define CPU_SAR_RST2_FREQ_MASK  0x1F
#define CPU_600_DDR_800_RCLK_800 0x1B
#define CPU_800_DDR_800_RCLK_800 0x1C

#define CPU_THERM_TEMP_OFFSET   0
#define CPU_THERM_TEMP_MASK     (0x3FF << CPU_THERM_TEMP_OFFSET)

#define CPU_THERM_OUTPUT_MSB    512
#define CPU_THERM_OUTPUT_COMP   1024

#define CPU_THERM_GAIN          425
#define CPU_THERM_OFFSET        153400
#define CPU_THERM_DIV           1000

#define TSN_CPU_NUM 4

#endif /* __PLATFORM_CPU_H__ */

/*------------------------------------------------------------------
$Log: platform_cpu.h,v $
Revision 1.8  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.7.16.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.7  2018/07/10 00:28:08  lucywang
Enhanced CPU core test to  make sure each CPU core is activated

Revision 1.6  2018/04/15 22:03:30  palin2
Merged Vulcan back to maintrunk.

Revision 1.5  2018/02/22 04:14:49  hondwang
Add diff CPU freq check for C1101 and C1109

Revision 1.4  2018/02/12 09:13:46  hondwang
merge Star CPU frequency check into main trunk

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.6  2018/02/12 08:39:01  hondwang
Add CPU frequency check

Revision 1.2.4.5  2017/09/22 03:22:05  lucywang
Set CPU registers to enable motherboard line loopback for Pluggable Serial GE port

Revision 1.2.4.4  2017/09/15 04:30:38  lucywang
added utility to enable/diable motherboard line loopback for Pluggable Serial GE port, not work yet

Revision 1.2.4.3  2017/08/23 05:46:33  lucywang
enable/disable Receiver to Tansmitter in local PHY for pluggable serial module

Revision 1.2.4.2  2017/08/22 03:29:59  lucywang
set 1000Base-X for pluggable serial and set sgmii for pluggable test card

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.6.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.4.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.4.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.2.3.6.1  2017/06/23 02:20:18  tirawan
Upload Star Second FPGA read parameter if this platform is Star with Pluggable module and correct LTE reset initialization

Revision 1.1.2.3  2016/11/29 02:54:39  palin2
Dynamically getting device bus window base from CPU register.

Revision 1.1.2.2  2016/09/28 04:36:15  palin2
Added CPU to ESW PHY MAC loopback test.

Revision 1.1.2.1  2016/09/13 08:14:23  palin2
Added CPU to GE PHY MAC loopback test.

$Endlog$
*/


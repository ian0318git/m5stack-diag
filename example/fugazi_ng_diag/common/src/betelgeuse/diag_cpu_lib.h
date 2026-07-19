/* $Id: diag_cpu_lib.h,v 1.3 2019/06/24 07:21:37 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_cpu_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_cpu_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_CPU_LIB_H__
#define __DIAG_CPU_LIB_H__


extern uint plat_fpga_reg_baseaddr;
extern uint plat_aikido_reg_baseaddr;

/* Common defines */
typedef enum {
    CPU_CORE_ZERO = 0,
    CPU_CORE_ONE,
    CPU_CORE_TWO,
    CPU_CORE_THREE,
    CPU_ALL_CORES = 4,   /* Betelgeuse CPU(Marvell 88e7040) is a 4-core CPU */
} cpu_core_num;

typedef enum {
    CPUMAC_LINKDOWN = 0,
    CPUMAC_LINKUP,
} cpu_mac_linkopt;

#define CPU_STRESS_LOG   "/tmp/cpu_core_log"
#define CPU_STRESS_RLT   "/tmp/cpu_core_rlt"

#define POLLING_INTRVL 100 
#define MAX_POLLING_COUNTS 100

/* Register map */
#define PLAT_MCU_VERSION_REG     0x00
#define PLAT_MCU_PS_MARGIN_REG   0x17

/* Port Auto-Negotiation Config Reg(m = 0-3) */
#define CPU_PORT_AN_CONF_REG(m)   (uint)(0xF2130E0C + (m*0x1000))
#define CPU_PACR_F_LINKUP         (uint)(1 << 1)
#define CPU_PACR_F_LINKDOWN       (uint)(1)

/* Port MAC Control Config Reg(m = 0-3) */
#define CPU_PORT_MAC_CTRL_REG(m)   (0xF2130E00 + (m*0x1000))

/* Port Status Reg0(m = 0-3) */
#define CPU_PORT_STATUS_REG0(m)   (uint)(0xF2130E10 + (m*0x1000))
#define CPU_PSR0_LINKUP           (uint)(1)   /* bit0 */
#define CPU_PSR_LINK_UP   0x1
#define CPU_PSR_LINK_DOWN 0x0

/* PHY Test Control 0 Register(n = 0-5) */
#define CPU_PHY_0_CONTORL_REG(n)			(0xF2120854 + (n*0x1000))
#define CPU_PHY_0_CTRL_TEST_ENABLE_0		0x0000
#define CPU_PHY_0_CTRL_TEST_ENABLE_1		0x00d0
#define CPU_PHY_0_CTRL_TEST_ENABLE_2		0x80d0

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
#define PANCR_SET_SGMII_1000      0x0040   /* 1 <<  6, 1000Mbps, Bit[6:5] = 10 */ 
#define PANCR_SET_MII_100         0x0020   /* 1 <<  5, 100Mbps,  Bit[6:5] = 01 */
#define PANCR_SET_MII_10          0x0000   /* 0 <<  5, 10Mbps,   Bit[6:5] = 00 */
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
#define PLAT_DEVBUS_CONF_TIME      (20)   /* 20ms */
#define PLAT_DEVBUS_0              (0)
#define PLAT_DEVBUS_1              (1)

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
#define PLAT_DEVBUS_0   (0)
#define PLAT_DEVBUS_1   (1)

#define PLAT_FPGA_DEVBUS_NUM     (PLAT_DEVBUS_0)
#define PLAT_AIKIDO_DEVBUS_NUM   (PLAT_DEVBUS_1)

/* Bridge Window Control Reg. */
#define BRIDGE_WIN_CTRL_REG_ADDR(x)   (0xf270ff00 + (0x8 * (x)))
/* Definition of device bus window size */
#define PLAT_DEVBUS_WINSIZE_16MB       ((0xff) << 16)   /* [31:16] window size */
/* Definition of device bus window attribute */
#define PLAT_DEVBUS0_ATTR              ((0x3e) << 8)    /* [15:8] window attributes */
#define PLAT_DEVBUS1_ATTR              ((0x3d) << 8)    /* [15:8] window attributes */
/* Definition of device bus window enable */
#define PLAT_DEVBUS_WIN_EN             (1)              /* bit 0: window enable */

/* Bridge Window Base Reg. */
#define BRIDGE_WIN_BASE_REG_ADDR(x)   (0xf270ff04 + (0x8 * (x)))
/* Definition of device bus window base */
#define DEVBUS_WINBASE_MSK            (0xffff0000)     /* [31:16] base addr */

/* Dev_CS[x] Read Parameters Reg. */
#define DEVBUS_RD_PARAMS_REG_ADDR(x)  (0xf2700408 + (0x8 * (x)))
/* Definition of device bus read parameters */
#define PLAT_DEVBUS0_RD_PARAM          (0x003e09cf)
#define PLAT_DEVBUS1_RD_PARAM          (0x001e0c0f)

/* Dev_CS[x] Write Parameters Reg. */
#define DEVBUS_WR_PARAMS_REG_ADDR(x)  (0xf270040c + (0x8 * (x)))
/* Definition of device bus write parameters */
#define PLAT_DEVBUS0_WR_PARAM          (0x000f0f0f)
#define PLAT_DEVBUS1_WR_PARAM          (0x000f1c0f)

/* Device Bus Synchronous Control Reg. */
#define DEVBUS_SYNC_CTRL_REG_ADDR     (0xf27004c8)
/* Definition of device bus ready polarity */
#define PLAT_DEVBUS_ACT_LOW            (0)
#define PLAT_DEVBUS_ACT_HIGH           (1)
/* Definition of device bus ready ignore */
#define PLAT_DEVBUS_READY_IGNORED      (1)
/* Definition of device bus CS ready polarity and ignore */
#define DBSCR_POLAR_SHIFT(x)          (10 + (5 * x))
#define DBSCR_POLAR_MSK               (0x1)
#define DBSCR_IGNORE_SHIFT(x)         (9 + (5 * x))
#define DBSCR_IGNORE_MSK              (0x1)

/* SMI Management Reg. (0xF212A200) */
#define SMIMR_BUSY           (uint32_t)(1 << 28)    /* bit28 */
#define SMIMR_READVALID      (uint32_t)(1 << 27)    /* bit27 */
#define SMIMR_OPCODE         (uint32_t)(1 << 26)    /* bit26 */
#define SMIMR_OPCODE_RD      (uint32_t)(1 << 26)
#define SMIMR_REGAD_MSK      0x1F /* bit25:21 */
#define SMIMR_REGAD_OFFSET   (21) /* bit25:21 */
#define SMIMR_REGAD          (uint32_t)(SMIMR_REGAD_MSK << SMIMR_REGAD_OFFSET)
#define SMIMR_PHYAD_MSK      0x1F /* bit20:16 */
#define SMIMR_PHYAD_OFFSET   (16) /* bit20:16 */
#define SMIMR_PHYAD          (uint32_t)(SMIMR_PHYAD_MSK << SMIMR_PHYAD_OFFSET)
#define SMIMR_DATA           (uint32_t)(0xFFFF)     /* bit15:0 */

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

/* Marvell ARMADA CPU 88F7040 */
/* South Bridge: CP110N */
#define CP_MPP_CTRL_REG(n)           (0xF2440000 + (0x4 * n))
#define CP_GPIO_DATA_OUT_EN_REG(n)   (0xF2440104 + (0x40 * n))
#define CP_GPIO_DATA_IN_POLAR_REG(n) (0xF244010C + (0x40 * n))
#define CP_GPIO_DATA_IN_REG(n)       (0xF2440110 + (0x40 * n))

#define CP0_GPIO8                    0x100
#define PLAT_FPGA_CPU_INTR            CP0_GPIO8

#define CP1_GPIO50                   0x40000    /* 1 << 18 star from bit32*/
#define CP0_GPIO62                   (1 << 30)
#define BETELGEUSE_P1A_PLUG_INTR_MPP CP0_GPIO62
#define PLAT_PLUG_FPGA_CPU_INTR       CP0_GPIO50

#define CPU_FREQ_1600_RAM_FREQ_800 0x04
#define CPU_FREQ_1400_RAM_FREQ_800 0x1a
#define CPU_FREQ_1200_RAM_FREQ_800 0x19
#define CPU_FREQ_1000_RAM_FREQ_800 0x1d
#define CPU_FREQ_800_RAM_FREQ_800  0x1c
#define CPU_FREQ_600_RAM_FREQ_800  0x1b


/* Externs */
extern int plat_cpu_mac_check_linkstat(int, boolean);
extern int show_cpu_ddr_freq(void);
extern int plat_devbus_init(int);

extern int plat_mem_read32(uint, uint *);
extern int plat_mem_write32(uint, uint);
extern int plat_get_cpu_ondie_temp(int *);


#endif /* __DIAG_CPU_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_cpu_lib.h,v $
 * Revision 1.3  2019/06/24 07:21:37  wilbhuan
 * Supported Pluggable Serial Module.
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

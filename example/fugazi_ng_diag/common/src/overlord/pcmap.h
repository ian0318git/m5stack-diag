/* $Id: pcmap.h,v 1.1 2013/05/09 05:42:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/pcmap.h,v $
 *------------------------------------------------------------------
 *
 * pcmap.h - Memory map defines for Informers.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PCMAP_H_
#define _PCMAP_H_


/*
 * Macro to map DRAM virtual space to physical space and vice versa. 
 */
#define PHYSICAL_ADDR_START     0

/*
 * Physical DRAM definitions.
 */
#define ADRSPC_RAM		PHYSICAL_ADDR_START     /* start of RAM */
#define PHY_ADRSPC_RAM		PHYSICAL_ADDR_START	/* Start of RAM */
#define ADRSPC_RAM_END		(PHYSICAL_ADDR_START + 0x80000000) 
#define ADRSPC_RAM_SIZE		(ADRSPC_RAM_END - ADRSPC_RAM)

/*
 * Top memory reserved by BIOS+ROMMON
 */
#define BIOS_TOP_MEM_RSV_SZ	0x4000000	/* Top mem reserved by BIOS */

/*
 * Definition for ROMMON used for bus error testing. On this platform
 * there is no real KSEG space but these definitions
 * help facilitate porting the existing code from MARs which is
 * a MIPs based platform.
 */ 
#define ADRSPC_BAD_ADDR		0xbad0add0	/* for stack.c */
#define KSEG1_ADRSPC_BAD_ADDR	0xbad0add0	/* for bus error test */
#define ADRSPC_K1BASE		0x0		/* for bus error test */

/*
 * PCIe interface memory map
 */

#define PCI_ADRSPC_GOOFY	get_pcie_bar0(GOOFY_ASIC)

#define PCI_ADRSPC_HWIC_SZ	0x00200000		/* 2M */
#define PCI_ADRSPC_HWIC0_SZ	PCI_ADRSPC_HWIC_SZ
#define PCI_ADRSPC_HWIC1_SZ	PCI_ADRSPC_HWIC_SZ
#define PCI_ADRSPC_HWIC2_SZ	PCI_ADRSPC_HWIC_SZ
#define PCI_ADRSPC_HWIC3_SZ	PCI_ADRSPC_HWIC_SZ
#define PCI_ADRSPC_LU0_SZ	0x00800000		/* 8M */
#define PCI_ADRSPC_LU1_SZ	0x00100000		/* 1M */
#define PCI_ADRSPC_LU2_SZ	0x00100000		/* 1M */
#define PCI_ADRSPC_LU3_SZ	0x00010000		/* 64K */
#define PCI_ADRSPC_GOOFY_HWIC_SZ PCI_ADRSPC_LU3_SZ
#define PCI_ADRSPC_GOOFY_HSIB_SZ 0x00010000		/* 64K */
#define PCI_ADRSPC_GOOFY_SZ	0x01000000		/* 16M */
#define PCI_ADRSPC_SM_SZ	0x02000000      	/* 32M */
#define PCI_ADRSPC_NITROX_SZ	PCI_ADRSPC_SM_SZ
#define PCI_ADRSPC_WLAN_ISM_SZ  PCI_ADRSPC_SM_SZ
#define PCI_ADRSPC_SM1_SZ	PCI_ADRSPC_SM_SZ
#define PCI_ADRSPC_SM2_SZ	PCI_ADRSPC_SM_SZ
#define PCI_ADRSPC_SM3_SZ	PCI_ADRSPC_SM_SZ
#define PCI_ADRSPC_SM4_SZ	PCI_ADRSPC_SM_SZ

#define VIR_ADDRESS_SM1		PCI_ADRSPC_SM1
#define VIR_ADDRESS_SM2		PCI_ADRSPC_SM2
#define VIR_ADDRESS_SM3		PCI_ADRSPC_SM3
#define VIR_ADDRESS_SM4		PCI_ADRSPC_SM4

#define PCI_DEV_IO_SEPARATION   0x00100000	/* 1 MB for each device */

/*------------------------------------------------------------------------------
 * Marvell Device addresses.
 */
#define GT_BASE_ADDRESS		0xB4000000
#define NETGX_REG_OFFSET	0xE000

/*
 * Goofy ASIC address. Refer to Xformer HW programmers reg manual
 */

/* Goofy master device node address mapping */
#define ADRSPC_GOOFY_MASTER		PCI_ADRSPC_GOOFY
#define GOOFY_MASTER_BAR0		PCI_ADRSPC_GOOFY
#define GOOFY_MASTER_QHWIC_RM_OFST	0x00000000
#define GOOFY_MASTER_WAN_OFST		0x00800000
#define GOOFY_MASTER_GLBREG_OFST	0x00900000
#define GOOFY_MASTER_QHWIC_LC_OFST	0x00FE0000
#define GOOFY_MASTER_HSIB_CONF_OFST	0x00FF0000
#define GOOFY_MASTER_DS0_OFST		0x01000000
#define GOOFY_MASTER_DS1_OFST		0x03000000
#define GOOFY_MASTER_DS2_OFST		0x05000000
#define GOOFY_MASTER_DS3_OFST		0x07000000
#define GOOFY_MASTER_DS_BLK_SZ		0x02000000

/* Should not use */
// Scrooge address mapping
#define ADRSPC_SCROOGE			0x97000000
#define SCROOGE_BAR0			0x97000000
#define SCROOGE_QHWIC_RM_OFST		0x00000000
#define SCROOGE_WAN_OFST		0x00600000
#define SCROOGE_GLBREG_OFST		0x00700000
#define SCROOGE_QHWIC_LC_OFST		0x007E0000
#define SCROOGE_HSIB_CONF_OFST		0x007F0000

/* added for Detox compilation */
#define ADRSPC_ENM              	0x35800000 
#define PHY_ADRSPC_ENM_PCI_MEM		0x45800000
#define PHY_ADRSPC_ENM_PCI_MEM_END	0x45FFFFFF
#define ADRSPC_FIO0_TDM         	0x30400000
#define ADRSPC_TDM              	(get_pcie_bar0(GOOFY_ASIC) + \
                                	 GOOFY_MASTER_WAN_OFST + \
                                	 GOOFY_TDM_OFFSET)

#define ADRSPC_FIO0_PKTPUMP		0x30410000
#define ADRSPC_FIO0_HDLC		0x30420000

/* added for hwic_utils.c compile */
#define ADRSPC_FIO0_HWIC0		0x30430000
#define FIO_LOCAL_HWIC_OFFSET		0x00000400
#define FIO_HWIC_PCI_OFFSET		0x00200000
#define FIO_OFFSET			0x04000000

/*
 * Macro for endianess is platform dependant.
 * Here is the location for Little Endian NMs/SMs
 *
 * PCIe side:
 * PCIe memory 0x2000_0000 - 0x3FFF_FFFF (512MB) for Little-Endian modules
 *----------------------------------------------------------------------------
 * CPU side:
 * DDR local memory 0x0000_0000 - 0x1FFF_FFFF (512MB) for Little-Endian memory
 */
#define LITTLE_ENDIAN_MEM_MAP(x)	(x)

/*
 * Informers Bootflash layout from RomMon
 * 	SPI flash 0: 0xff800000 to 0xffffffff
 * 	SPI flash 1: 0xff000000 to 0xff7fffff
 * 	sector size: 64KB
 */

#define ADRSPC_BFLASH_DIAG_TEST			ADRSPC_BFLASH_DIAG_TEST_0

/* Boot flash #0 base addresses */
#define ADRSPC_BFLASH_ROMMON_READONLY		0xffc00000	/* 64 sectors */
#define ADRSPC_BFLASH_SPARE_0			0xffbf0000	/*  1 sectors */
#define ADRSPC_BFLASH_UBOOT_CFG_2ND_CORE	0xff950000	/* 42 sectors */
#define ADRSPC_BFLASH_LICENSING_2ND		0xff8f0000	/*  6 sectors */
#define ADRSPC_BFLASH_LICENSING_PRI		0xff890000	/*  6 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_RSV		0xff870000	/*  2 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_2ND		0xff850000	/*  2 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_PRI		0xff830000	/*  2 sectors */
#define ADRSPC_BFLASH_DIAG_TEST_0		0xff820000	/*  1 sectors */
#define ADRSPC_BFLASH_ERR_LOG_0			0xff810000	/*  1 sectors */
#define ADRSPC_BFLASH_DESC_0			0xff800000	/*  1 sectors */
/* Boot flash #1 base addresses */
#define ADRSPC_BFLASH_ROMMON_UPGRADE		0xff400000	/* 64 sectors */
#define ADRSPC_BFLASH_SPARE_1			0xff3e0000	/*  2 sectors */
#define ADRSPC_BFLASH_IOS_WEAR_LEVELING		0xff3a0000	/*  4 sectors */
#define ADRSPC_BFLASH_IOS_NVRAM_2ND		0xff220000	/* 24 sectors */
#define ADRSPC_BFLASH_IOS_NVRAM_PRI		0xff0a0000	/* 24 sectors */
#define ADRSPC_BFLASH_ROMMON_COOKIE_2ND		0xff080000	/*  2 sectors */
#define ADRSPC_BFLASH_ROMMON_COOKIE_PRI		0xff060000	/*  2 sectors */
#define ADRSPC_BFLASH_SPARE_1_2			0xff030000	/*  3 sectors */
#define ADRSPC_BFLASH_DIAG_TEST_1		0xff020000	/*  1 sectors */
#define ADRSPC_BFLASH_ERR_LOG_1			0xff010000	/*  1 sectors */
#define ADRSPC_BFLASH_DESC_1			0xff000000	/*  1 sectors */

#define ADRSPC_BFLASH_BASE			0xff000000
#define ADRSPC_BFLASH_SECTOR_LAST		0xffff0000
#define ADRSPC_BFLASH_NVRAM			ADRSPC_BFLASH_IOS_NVRAM_PRI
#define ADRSPC_MAINBOARD_FLASH			ADRSPC_BFLASH_BASE

/* for ../epromtest.c - not used */
#define ADRSPC_PROM				ADRSPC_BFLASH_BASE /* bootrom */

/* Boot flash size */
#define ADRSPC_BFLASH_SECTOR_SZ			0x00010000	/* 64 KB */
#define ADRSPC_BFLASH_PAGE_SZ			0x00000100	/* 256B */
#define ADRSPC_BFLASH_ROMMON_READONLY_SZ	0x00400000	/* 64 sectors */
#define ADRSPC_BFLASH_UBOOT_CFG_2ND_CORE_SZ	0x002a0000	/* 42 sectors */
#define ADRSPC_BFLASH_LICENSING_2ND_SZ		0x00060000	/*  6 sectors */
#define ADRSPC_BFLASH_LICENSING_PRI_SZ		0x00060000	/*  6 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_SZ		0x00020000	/*  2 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_RSV_SZ	0x00020000	/*  2 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_2ND_SZ	0x00020000	/*  2 sectors */
#define ADRSPC_BFLASH_CODE_SIGNING_PRI_SZ	0x00020000	/*  2 sectors */
#define ADRSPC_BFLASH_DIAG_TEST_SZ		0x00010000	/*  1 sectors */
#define ADRSPC_BFLASH_DIAG_TEST_0_SZ		ADRSPC_BFLASH_DIAG_TEST_SZ
#define ADRSPC_BFLASH_DIAG_TEST_1_SZ		ADRSPC_BFLASH_DIAG_TEST_SZ
#define ADRSPC_BFLASH_ERR_LOG_0_SZ		0x00010000	/*  1 sectors */
#define ADRSPC_BFLASH_DESC_0_SZ			0x00010000	/*  1 sectors */

#define ADRSPC_BFLASH_ROMMON_UPGRADE_SZ		0x00400000	/* 64 sectors */
#define ADRSPC_BFLASH_IOS_WEAR_LEVELING_SZ	0x00040000	/*  2 sectors */
#define ADRSPC_BFLASH_IOS_NVRAM_PRI_SZ		0x00180000	/* 24 sector */
#define ADRSPC_BFLASH_IOS_NVRAM_2ND_SZ		0x00180000	/* 24 sector */
#define ADRSPC_BFLASH_ROMMON_COOKIE_2ND_SZ	0x00020000	/*  2 sectors */
#define ADRSPC_BFLASH_ROMMON_COOKIE_PRI_SZ	0x00020000	/*  2 sectors */
#define ADRSPC_BFLASH_ERR_LOG_1_SZ		0x00010000	/*  1 sectors */
#define ADRSPC_BFLASH_DESC_1_SZ			0x00010000	/*  1 sectors */

#if 1 // Stuff from Xformers

/*
 * LED Register
 */
#define	ADRSPC_LED		ADRSPC_HW_REGISTERS
#define ILP_ENABLED		4		/* bit 2: inline power enable */
#define LED_SYSTEM_OK		0x8000		/* bit 15: System OK */

#define PCI_FUNC_0		0		/* Function 0 */
#define PCI_FUNC_1		1		/* Function 1 */
#define PCI_FUNC_2		2		/* Function 2 */

/* PCI VID/DIDs */
#define USB_FN0_PCI_DEV_ID	0x15611131	/* USB Function 0 DID/VID */
#define USB_FN1_PCI_DEV_ID	0x15611131	/* USB Function 1 DID/VID */
#define USB_FN2_PCI_DEV_ID	0x15621131	/* USB Function 2 DID/VID */
#define TALITOS_PCI_DEV_ID	0x64051057	/* Talitos DID/VID */
#define ATHEROS_PCI_DEV_ID	0x0013168c	/* Aircard DID/VID */

/* PCI IDSEL */
#define USB_IDSEL		16	/* USB IDSEL device 16 */
#define AC_PCI0_IDSEL		17	/* Mini PCI0 IDSEL device 17 */
#ifdef EVAL_8560
#define AC_PCI1_IDSEL		21	/*Mini PCI2 IDSEL device 21 */
#else
#define AC_PCI1_IDSEL		18	/* Mini PCI1 IDSEL device 18 */
#endif /* EVAL_8560 */
#define TALITOS_IDSEL		19	/* Talitos IDSEL device 19 */
#ifdef EVAL_8560
#define AC_PCI2_IDSEL		18	/* Mini PCI2 IDSEL device 18 */
#endif /* EVAL_8560 */

/*
 * SMC Transmit and Receive Enable bit definitions
 */
#define SMC_COUNT              	2
#define SMC_REN                	0x01
#define SMC_TEN                	0x02

/*
 * I2C Addresses. Needs to be left shifted by one for the I2C command register
 */
#define I2C_LTC_ALERT		0x0C	/* LTC4259 Alert Response */
#define I2C_LTC_PORTS0_3	0x20	/* LTC4259 P50 Ports 0-3 */
#define I2C_LTC_PORTS4_7	0x21	/* LTC4259 P50 Ports 4-7 */
#define I2C_LTC_BROADCAST	0x30	/* LTC4259 Broadcast	*/
#define I2C_ILP_COOKIE		0x50	/* ILP Cookie EEPROM	*/
#define I2C_DDR_DIMM_SPD	0x51	/* DDR Memory DIMM SPD	*/
#define RTC_I2C_ADDR		0x68	/* Maxim DS1337C RTC	*/
#ifdef ILP_LTC0_ADDR
#define ILP_LTC0_ADDR		I2C_LTC_PORTS0_3
#define ILP_LTC1_ADDR		I2C_LTC_PORTS4_7
#define ILP_EEPROM_ADDR		I2C_ILP_COOKIE
#endif /* ILP_LTC0_ADDR */

/*
 * MII Management Interface Address.
 */
#define MII_8PORT_PHY_ADDR	0	/* Marvell 6083 PHY Control */
#define MII_8PORT_PORT_ADDR	0x10	/* Marvell 6083 Port Status */
#define MII_8PORT_GLOBAL_ADDR	0x1B	/* Marvell 6083 Global Status */
#define MII_FE0_PHY_ADDR	0x08	/* Micrel FE0 PHY	*/
#define MII_FE1_PHY_ADDR	0x09	/* Micrel FE1 PHY	*/

#define MII_ADDR_MAX		0x1F	/* Max MII address */
#define MII_REG_MAX		0x1F	/* Max MII register offset */
#define MII_DATA_MAX		0xFFFF	/* Max MII data */
#endif

#define VIR_ADRSPC_GOOFY_QUAD_HWIC	VIR_ADDR(ADRSPC_GOOFY_MASTER)
#define HWIC_OFFSET			0x00000400

#define DSP_OFFSET			0x00000040
#define PVDM_OFFSET			0x04000000
#define PVDM0_DSP0_BASE_ADDRESS		0x30410000
#define PVDM0_DSP0_EHPI_ADDRESS		0x30411000
#define PVDM0_DSP0_CPLD_ADDRESS		0x30411400
#define PVDM0_INT_READ			0x30410009 /* ADRSPC_IIR_REG */
#define PVDM0_INT_MASK			0x3041000D /* ADRSPC_IMR2_REG */
#define PVDM_EHPI_OFFSET		0x00001000
#define PVDM0_DSP0_CPLD_ADDRESS		0x30411400
#define ADRSPC_FIO1_PKTPUMP		0x34410000

/*
 * Informers IOFPGA Registers (02/10/09: IOFPGA change)
 */
//#define MB_IOFPGA_MAIN_REGS		0xFED40000
#define MB_IOFPGA_PROM_REGS             0xFED400C0
#define MB_IOFPGA_CF0_REGS		0xFED400E0
#define MB_IOFPGA_CF1_REGS		0xFED400E6
#define MB_IOFPGA_SMI_REGS		0xFED400F0 /* GESW MDIO */
#define MB_IOFPGA_SMI_STAT_REGS		0xFED40110 /* SMI Local Stat Buffer */
#define MB_IOFPGA_SMI2_REGS		0xFED40940 /* PVDM MDIO */
#define MB_IOFPGA_CTS_REGS		0xFED40924
#define MB_IOFPGA_LED_REGS		0xFED40400

#define MB_IOFPGA_SYS_LOW_LEVEL_REGS		0xFED40000
#define MB_IOFPGA_EXT_DEV_RST_OFFSET		0x4
#define MB_IOFPGA_EXT_DEV_RST_REG_ADDR \
        (MB_IOFPGA_SYS_LOW_LEVEL_REGS + MB_IOFPGA_EXT_DEV_RST_OFFSET) /* 0xFED40004 */

#define MB_IOFPGA_PSU_ENV_REGS		0xFED32100
#define MB_IOFPGA_POE_PSU_INT_OFFSET		0x24
#define MB_IOFPGA_POE_PSU_INT_REG_ADDR \
        (MB_IOFPGA_PSU_ENV_REGS + MB_IOFPGA_POE_PSU_INT_OFFSET)  /* 0xFED32124 */
#define MB_IOFPGA_POE_PSU_INT_REG_PTR \
          (volatile uint *)(MB_IOFPGA_POE_PSU_INT_REG_ADDR)
/*
 * Overlord IOFPGA Registers Offset
 */
#define OVLD_FPGA_QUACK_REG_OFF         0x00000930

#define CS6_MB_IOFPGA_MAIN_REGS		MB_IOFPGA_MAIN_REGS


#endif /* _PCMAP_H_ */


/******** History ******** 
$Log: pcmap.h,v $
Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: platform_pci.h,v 1.2 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_pci.h,v $
 *------------------------------------------------------------------
 *
 * platform_pci.h - pci related definitions for Informers.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_PCI_H_
#define _PLATFORM_PCI_H_

#include "types.h"
#include "pci.h"

#if !defined(ASMINLUDE)
typedef enum {
    ONE_BYTE  = 1,
    TWO_BYTE  = 2,
    FOUR_BYTE = 4,
} pci_len;
#endif /* !defined(ASMINLUDE) */

#define PRTHDR(_f, _v) printf("%#.8x  %-32s  %#x\n", &_v, _f,  _v)
#define PRTRCR(_m, _v) printf("%#.8x  %-32s  %#.8x\n", &_v, _m,  _v)
#define PRTR64(_m, _v) printf("%#.8x  %-32s  %#.8x:%#.8x\n", &_v, _m,  _v>>32, (uint)_v)
#define PRTR09(_m, _v, _i) printf("%#.8x  %s %d           %#.8x\n", &_v, _m, _i, _v)
#define PRTR10(_m, _v, _i) printf("%#.8x  %s %d          %#.8x\n", &_v, _m, _i, _v)

/* Max number of bridges/switches traversed from root port to the
   PCIE end point */
/* ??? clean up move ids together */
#define MAX_BRIDGE_COUNT                4
#define PCI_CLASS_BRIDGE                0x06
#define PCIE_SWITCH                     10
#define PCIE_NITROX_ID                  0x0010177D
#define PCIE_ID_8509_SWITCH             0x850910B5
#define PCIE_DUAL_GE_MACPHY_ID          0x10AA8086

#define PLX_8509_UPSTRM_PORT            0
#define PLX_8509_DNSTRM_PORT1           1
#define PLX_8509_DNSTRM_PORT2           2

#define PCIE_BRIDGE                     2

#define PCI_ERR                         0xFFFF 

typedef enum {
    BUS_NO = 0,
    DEV_NO,
    FUNC_NO,
    DEV_DESC,
    BAR0,
    MEMIO_LIMIT,
    SEC_BUS,
} pcie_field_enum;

/* PCIE Platform devices like SMs, GE MACs, Nitrox, Goofy mapping to the 
   x86 PCIE Root Ports */
/*??? May not need this */
typedef struct pcie_dev_to_rtprt_t {
    uchar *rtprt_desc;
    uchar *pcie_dev_desc;
    uchar bus;				/* Primary bus */
    uchar dev;
    uchar func;
    uchar plat_slot;	/* platform slot number associated with the PCIE Root Port */
} pcie_dev_to_rtprt_t;

typedef enum {
    GEMACPHY = 0,
    ASIC,
    SMMODULE,
    SECNITROX,
} pcie_dev_type;

typedef enum {
    GEMACPHY0 = 0,
    GEMACPHY1,
    GEMACPHY2,
    GEMACPHY3,
    GE0,
    GE1,
    GOOFY_ASIC,
    SM_SLOT_1,
    SM_SLOT_2,
    SM_SLOT_3,
    SM_SLOT_4,
    NITROX,
    MAX_PCIE_TYPE0_DEV,
    T1_MCH_PORT_2 = MAX_PCIE_TYPE0_DEV,
    T1_MCH_PORT_3,
    T1_MCH_PORT_4,
    T1_MCH_PORT_5,
    T1_MCH_PORT_6,
    T1_MCH_PORT_7,
    T1_ICH_PORT_1,
    T1_ICH_PORT_5,
    T1_SM2NM_1,
    T1_SM2NM_2,
    T1_SM2NM_3,
    T1_SM2NM_4,
    T1_PLX8509_P0,
    T1_PLX8509_P1,
    T1_PLX8509_P2,
    MAX_PCIE_DEVICES
} plat_pcie_dev_enum;

//#define MAX_PCIE_DEV (MAX_PCIE_DEV - MAX_PCIE_TYPE0_DEV)
#define PCI_SM_SLOT_BASE_OFFSET         SM_SLOT_1 + 1
#define PCI_GE_SLOT_BASE_OFFSET         GEMACPHY0
#define NUM_PLX_8509_DNSTREAM_PORTS     4

typedef enum {
    T0_GEMACPHY1 = 0,
    T0_GEMACPHY2,
    T0_SM_SLOT_1,
    T0_SM_SLOT_2,
    T0_SM_SLOT_3,
    T0_SM_SLOT_4,
    T0_GOOFY_ASIC,
    T0_NITROX,
    T0_GEMACSW
} plat_pcie_type0_dev_enum;

typedef struct pcie_info_t_ {
    uint bus;
    uint dev;
    uint func;
    ulong vendor_id;
    uchar *desc;
} pcie_info_t;

typedef struct rt_prt_t_ {
    uchar *rtprt_desc;
    uint num;
    uint bus;
    uint dev;
    uint func;
} rt_prt_t;

/* Number of PCIE end devices (not including MCH/ICH) */
#define MAX_PCI_BUS		256
#define MAX_PCI_DEV             32
#define MAX_PCI_FUNC            8
#define MAX_PCI_REG		0x100
#define MAX_PCIE_REG		0x1000

/* PCI configuration mechanism #1 */
#define	PCI_CFGADDR		0xCF8		/* CONFIG_ADDRESS */
#define	PCI_CFGDATA		0xCFC		/* CONFIG_DATA */
#define PCI_ID_NONE		0xffffffff
#define PCI_ID_UNKNOWN		PCI_ID_NONE
#define PCI_DID_UNKNOWN		0xffff
#define PCI_VID_UNKNOWN		0xffff

#define PCICFG_ENA		0x80000000
#define PCICFG_BUS_MASK		0x00FF0000
#define PCICFG_DEV_MASK		0x0000F800
#define PCICFG_FUN_MASK		0x00000700
#define PCICFG_REG_MASK		0x000000FC
#define PCICFG_BUS_SHFT		16
#define PCICFG_DEV_SHFT		11
#define PCICFG_FUN_SHFT		8
#define PCICFG_REG_SHFT		2

#define PIC_ID_GOOFY		0x001e1137
#define PIC_ID_82576		0x10C98086
#define NPIC_ID_82576		0x10E78086
#define PIC_ID_XIO2000		0x8231104c
#define PCI_ID_82571_ETH_CTRL	0x105e8086

/* definitions for PCI device map from Bryce */
#define PCI_MCH_BUS             0
#define PCI_MCH_DEV             0
#define PCI_MCH_FUNC            0

#define PCI_MCHER_BUS           0
#define PCI_MCHER_DEV           0
#define PCI_MCHER_FUNC          1

#define PCI_PCIE0_BUS           0
#define PCI_PCIE0_DEV           2
#define PCI_PCIE0_FUNC          0

#define PCI_PCIE1_BUS           0
#define PCI_PCIE1_DEV           3
#define PCI_PCIE1_FUNC          0

/* MCH Memory Branch Map, Control, Errors */
#define PCI_MCH_MCE_BUS		0
#define PCI_MCH_MCE_DEV		16
#define PCI_MCH_MCE_FUNC	1
#define PCI_MCH_MCE_FUNC0	0
#define PCI_MCH_MCE_FUNC1	1
#define PCI_MCH_MCE_FUNC2	2

#define PCI_LPC_BUS             0
#define PCI_LPC_DEV             31
#define PCI_LPC_FUNC            0

#define PCI_IDE_BUS             0
#define PCI_IDE_DEV             31
#define PCI_IDE_FUNC            1

#define PCI_SATA_BUS            0
#define PCI_SATA_DEV            31
#define PCI_SATA_FUNC           2

#define PCI_SMBUS_BUS           0
#define PCI_SMBUS_DEV           31
#ifdef INTEL_CPU_SKYLAKE
#define PCI_SMBUS_FUNC          4
#else
#define PCI_SMBUS_FUNC          3
#endif

#define PCI_PCI_BUS             0
#define PCI_PCI_DEV             30
#define PCI_PCI_FUNC            0

#define PCI_PCIX_BUS            0
#define PCI_PCIX_DEV            28
#define PCI_PCIX_FUNC           0

#define PCI_USB0_BUS            0
#define PCI_USB0_DEV            29
#define PCI_USB0_FUNC           0

#define PCI_USB1_BUS            0
#define PCI_USB1_DEV            29
#define PCI_USB1_FUNC           1

#define PCI_USBe_BUS            0
#define PCI_USBe_DEV            29
#define PCI_USBe_FUNC           7

#define PCI_WDT_BUS             0
#define PCI_WDT_DEV             29
#define PCI_WDT_FUNC            4

#define PCI_APIC1_BUS           0
#define PCI_APIC1_DEV           29
#define PCI_APIC1_FUNC          5

#ifdef EVAL_BOARD  /* for MCH 7520 */
#define PCI_DID_MCH             0x3590
#else  /* for MCH 7320 */
#define PCI_DID_MCH             0x3592
#endif
#define PCI_DID_MCHER           0x3593
#define PCI_DID_PCIE0           0x3595
#define PCI_DID_PCIE1           0x3596
#define PCI_DID_PCI             0x244e
#define	PCI_DID_LPC		0x25a1
#define PCI_DID_IDE             0x25a2
#define PCI_DID_SATA            0x25a3
#define PCI_DID_SMBUS           0x25a4
#define PCI_DID_USB0            0x25a9
#define PCI_DID_USB1            0x25aa 
#define PCI_DID_WDT             0x25ab
#define PCI_DID_APIC1           0x25ac
#define PCI_DID_USBe            0x25ad
#define PCI_DID_PCIX            0x25ae

#define INTEL_VENDOR_ID         0x8086

/* general definiton for all the PCI devices */
#define PCI_VID_OFFSET          0x0
#define PCI_DID_OFFSET          0x02
#define PCI_CMD_OFFSET          0x04
#define PCI_STATUS_OFFSET       0x06

/*
 * bit definition for pci Command register.
 * Only bits 0:9 are defined.  Bits 10:15 are reserved.
 */
#define	PCI_CMD_ENA_IO		0x0001	/* respond to PCI I/O accesses */
#define	PCI_CMD_ENA_MEM		0x0002	/* respond to PCI memory accesses */
#define	PCI_CMD_ENA_BUS_MASTER	0x0004	/* allow device to be a bus master */
#define	PCI_CMD_ENA_SPECIALS	0x0008	/* special cycle recognition */
#define	PCI_CMD_ENA_MWINV	0x0010	/* memory write and invalidate enable */
#define	PCI_CMD_ENA_VGA_SNOOP	0x0020	/* VGA palette snoop enable */
#define	PCI_CMD_RESP_PAR_ERROR	0x0040	/* respond to parity errors */
#define	PCI_CMD_ENA_WAIT_CYCLE	0x0080	/* does device to addr/data stepping */
#define	PCI_CMD_ENA_SYS_ERROR	0x0100	/* device can drive the SERR# line */
#define	PCI_CMD_ENA_FAST_B2B	0x0200	/* can to fast back-to-back xactions */

/* definitions for MCH */

#define PCI_MCH_MCHSCRB_OFFSET		0x52

/* bit definitions for MCH memory scrub and init config register */
#define PCI_MCH_MCHSCRB_SCRUB_ON	(1 << 1)

#define PCI_MCH_DRAM_CTRL_OFFSET	0x7c
/* bit definitions for DRAM controller mode register */
#define PCI_MCH_DRAM_INIT_DONE		0x20000000
#define PCI_MCH_DRAM_ECC		0x00100000
#define PCI_MCH_DRAM_CHIPFAIL_ECC	0x00200000
#define PCI_MCH_FSB_FREQ_133		(1 << 2)
#define PCI_MCH_FSB_FREQ_166		(1 << 3)
#define PCI_MCH_FSB_FREQ_200		((1 << 3) | (1 << 2))
#define PCI_MCH_DRAM_DDR333		(1 << 0)
#define PCI_MCH_DRAM_DDR400		(1 << 1)

#define PCI_MCH_DDRCSR_OFFSET		0x9a
/* bit definitions for DDR channel config and status register */
#define PCI_MCH_DDRCSR_DUAL		((1 << 2) | (1 << 3))
#define PCI_MCH_DDRCSR_SINGLE_B		(1 << 3)
#define PCI_MCH_DDRCSR_SINGLE_A		(1 << 2)

#define PCI_MCH_DEVPRES_OFFSET		0x9c
/* bit definition for Device Present register */
#define PCI_MCH_DEV2_ENA		(1 << 2)
#define PCI_MCH_DEV3_ENA		(1 << 3)

#define PCI_MCH_TOLM_OFFSET		0xc4
/* bit definition for Top of Low Memory register */
#define PCI_MCH_TOLM_MASK		0xf800

#define PCI_MCH_TOM_OFFSET		0xcc
/* bit definition for Top of Memory register */
#define PCI_MCH_TOM_MASK		0x01ff
#define PCI_MCH_TOM_SHIFT		27        /* 128 MB granularity */

#define PCI_MCH_PCIE_BASE_OFFSET	0xce
/* bit definition for PCIE base address register */
#define PCI_MCH_PCIE_BASE_MASK		0xf000
#define PCI_MCH_PCIE_BASE_SHIFT		28        /* 256 MB range */

#define PCI_MCH_CACHECTL_OFFSET		0xd8
/* bit definition for write cache control register */
#define PCI_MCH_CACHECTL_WCFLUSH	0x01

#define PCI_MCH_DEVPRES1_OFFSET		0xf4
/* bit definition for Device Present 1 register */
#define PCI_MCH_DEV0_FUNC1_ENA		(1 << 5)
#define PCI_MCH_DEV8_FUNC0_ENA		(1 << 1)

/* definitions for MCH error reporting block */

#define PCI_MCH_DRAM_FERR_OFFSET	0x80
#define PCI_MCH_DRAM_NERR_OFFSET	0x82
#define PCI_MCH_DRAM_SEC1_ADD_OFFSET	0xa0
#define PCI_MCH_DRAM_DED_ADD_OFFSET	0xa4

/* bit definitions for FERR and NERR */
#define PCI_MCH_DRAM_CE_A		(1 << 0)
#define PCI_MCH_DRAM_CE_B		(1 << 8)
#define PCI_MCH_DRAM_WUE_B		(1 << 14)
#define PCI_MCH_DRAM_SUE_B		(1 << 10)
#define PCI_MCH_DRAM_RUE_B		(1 << 9)
#define PCI_MCH_DRAM_WUE_A		(1 << 6)
#define PCI_MCH_DRAM_SUE_A		(1 << 2)
#define PCI_MCH_DRAM_RUE_A		(1 << 1)
#define PCI_MCH_DRAM_UE_B		((1 << 14) | (1 << 10) | (1 << 9))
#define PCI_MCH_DRAM_UE_A		((1 << 6) | (1 << 2) | (1 << 1))

/* definitions for PCI express ports */
#define PCI_PCIE_SEC_BUSNUM_OFFSET	0x19

/* definitions for SMBUS */

#define PCI_SMBUS_BAR_OFFSET		0x20
#define PCI_SMBUS_HSTCFG_OFFSET		0x40

/* bit definitions for SMBUS*/
#define PCI_SMBUS_IO_SPACE_EN		0x01
#define PCI_SMBUS_I2C_ENABLE		0x04
#define PCI_SMBUS_ENABLE		0x01
#define PCI_SMBUS_BAR_MASK		0x0000ffe0

/* definitions for LPC I/F bridge */

#define PCI_LPC_GPIO_BAR_OFFSET		0x58
#define PCI_LPC_GPIO_CNTL_OFFSET	0x5c
#define PCI_LPC_COM_DEC_OFFSET		0xe0
#define PCI_LPC_IF_ENA_OFFSET		0xe6
#define PCI_LPC_FUNC_DIS_OFFSET		0xf2
#define PCI_LPC_BIOS_CNTL_OFFSET	0x4e
#define PCI_LPC_PMBASE_OFFSET		0x40
#define PCI_LPC_ACPI_CNTL_OFFSET	0x44
#define PCI_LPC_PIRQA_RCR_OFFSET	0x60
#define PCI_LPC_PIRQB_RCR_OFFSET	0x61
#define PCI_LPC_PIRQC_RCR_OFFSET	0x62
#define PCI_LPC_PIRQD_RCR_OFFSET	0x63
#define PCI_LPC_SIRQ_CNTL_OFFSET	0x64
#define PCI_LPC_PIRQE_RCR_OFFSET	0x68
#define PCI_LPC_PIRQF_RCR_OFFSET	0x69
#define PCI_LPC_PIRQG_RCR_OFFSET	0x6a
#define PCI_LPC_PIRQH_RCR_OFFSET	0x6b

/* bit definitions for LPC */
#define PCI_LPC_GPIO_BAR_MASK 		0x0000ffc0
#define PCI_LPC_GPIO_ENA		0x10
#define PCI_LPC_COM_DEC_COMA_MASK	0xf8
#define PCI_LPC_COM_DEC_COMA_SET	0x00	/* 0x3f8 - 0x3ff */
#define PCI_LPC_COM_DEC_COMB_MASK	0x8f
#define PCI_LPC_COM_DEC_COMB_SET	0x10	/* 0x2f8 - 0x2ff */
#define PCI_LPC_IF_ENA_MASK		0x3f0f
#define PCI_LPC_IF_ENA_SET		0x3f0f
#define PCI_LPC_FUNC_DIS_MASK		0x0000
#define PCI_LPC_FUNC_DIS_SET		0x0060
#define PCI_LPC_PMBASE_MASK		0x0000ff80
#define PCI_LPC_ACPI_EN			0x10
#define PCI_LPC_PIRQ_IRQEN		0x80
#define PCI_LPC_SIRQEN			0x80

#define PCI_LPC_BIOS_WE			0x1

/* LPC - D31/F0 Reset Control Register define: 1 byte */
#define LPC_RST_CNT_IOPORT		0xcf9	/* io port */
#define LPC_RST_CNT_FULL_RESET		8	/* full reset */
#define LPC_RST_CNT_RESET_CPU		4	/* reset cpu if from 0 to 1 */
#define LPC_RST_CNT_SYS_RST		2	/* 0=soft reset; 1=hard reset */

/* definitions for IDE controller */
#define PCI_IDE_PI_OFFSET		0x09

/* bit definitions for IDE controller */
#define PCI_IDE_PI_POP_MODE_SEL		1
#define PCI_IDE_PI_SOP_MODE_SEL		(1 << 2)

/* definitions for USB EHCI controller */
#define PCI_USBe_BAR_OFFSET		0x10

/* bit definition for USB EHCI */
#define PCI_USBe_BAR_MASK		0xfffffc00
 
/* definitions for USB UHCI controller */
#define PCI_USB_BAR_OFFSET		0x20

/* bit definition for USB UHCI */
#define PCI_USB_BAR_MASK		0x0000ffe0

/* RCBA defines */
#define PCI_RCBA_OFFSET			0xf0		/* Root Complex Base */
#define PCI_RCBA_MASK			0xffffc000	/* Base Mask */
#define PCI_RCBA_ENA			1		/* Enable RCBA */

/* RCRB OIC register defines */
#define OIC_ASEL_MASK			0xf0            /* APIC range select */
#define OIC_CEN				0x02            /* Co error enable */
#define OIC_AEN				0x01            /* APIC enable */

/* D16:F0 EXSMRTOP - Extended System Management RAM Top Register */
#define PCI_EXSMRTOP_OFFSET		0x63
#define PCI_EXSMRTOP_ESMMTOP_MASK	0xf		/* bit 31:28 bits */	
#define PCI_ESMMTOP(x)			((x) << 28)

/* D16:F0 EXSMRC - Extended System Management RAM Control Register */
#define PCI_EXSMRC_OFFSET		0x62
#define PCI_EXSMRC_H_SMRAME		0x80		/* Enable High SMRAM */
#define PCI_EXSMRC_MDAP			0x40		/* MDA Present */
#define PCI_EXSMRC_G_SMRAME		0x08		/* Global SMRAM Enable*/
#define PCI_EXSMRC_TSEG_SZ_MASK		0x06		/* TSEG Size */
#define PCI_EXSMRC_T_EN			0x01		/* TSEG Enable */
#define PCI_EXSMRTOP_TSEG_SZ(x)		(0x80000 << (x))/* TSEG size */

#define PCI_VENDOR_ID_OFFSET		0x00
#define PCI_DEVICE_ID_OFFSET		0x02
#define PCI_COMMAND_REG_OFFSET		0x04
#define PCI_STATUS_REG_OFFSET		0x06
#define PCI_REVISION_ID_OFFSET		0x08
#define PCI_PROG_IF_OFFSET		0x09
#define PCI_SUB_CLASS_OFFSET		0x0A
#define PCI_BASE_CLASS_OFFSET		0x0B
#define PCI_LATENCY_OFFSET		0x0C	/* Add in to support MARS */
#define PCI_LAT_TIMER_OFFSET		0x0C	/* Actual byte offset = 0x0D */
#define PCI_IO_BASE_OFFSET		0x10
#define PCI_MMIO_BASE_OFFSET		0x14
#define PCI_MEM_BAR0_OFFSET		0x10
#define PCI_MEM_BAR1_OFFSET		0x14
#define PCI_MEM_BAR2_OFFSET		0x18
#define PCI_MEM_BAR3_OFFSET		0x1C
#define PCI_MEM_BAR4_OFFSET		0x20
#define PCI_MEM_BAR5_OFFSET		0x24
#define PCI_LOCAL_BASE_OFFSET		0x18
#define PCI_BASE_ADDR0_OFFSET		0x10
#define PCI_BASE_ADDR1_OFFSET		0x14
#define PCI_BASE_ADDR2_OFFSET		0x18
#define PCI_BRIDGE_CONTROL_OFFSET	0x3E
#define PCI_DEV_CNTRL    		0x74

#define BIOS_FW_RESERVED_SPACE		0x00200000 /* reserved for FirmBase */
#define PSE2_PCI_LINKUP                 0x2000 /* The PCI link status bit */

//extern void show_plx_8509(uint);
extern uint get_esmmtop(void);
extern uint get_tseg_sz(void);
extern void init_rcrb(void);
extern void init_mmcfg(void);
extern uint get_dev_mmcfg_base(uint bus, ushort dev, uint func);
extern void pci_mmcfg_write(uint b, ushort d, uint f, uint r, uint dt, uint sz);
extern uint pci_mmcfg_read(uint b, ushort d, uint f, uint r, uint sz);
extern uint pci_cfg_read(uint bus, ushort dev, uint fn, uint reg);
extern void pci_cfg_write(uint bus, ushort dev, uint fn, uint reg, uint v);
extern ulong pci_config_read(uchar bus, uchar dev, uchar fn, uint reg);
extern void pci_config_write(uchar b, uchar dev, uchar fn, uint reg, ulong v);
extern ulong pci_config_read_byte(uchar bus, uchar dev, uchar fn, uint reg);
extern void pci_config_write_byte(uchar b, uchar d, uchar f, uchar r, ulong v);
extern uint pcie_config_read(uint p, uint bus, ushort dev, uint func, uint reg);
extern void pcie_config_write(uint p, uint b, ushort d, uint f, uint r, uint v);
#if 0
extern pci_cap_t *get_1st_pci_cap(uint bus, ushort dev, uint func);
extern pci_cap_t *get_next_pci_cap(pci_cap_t *cap);
extern pci_cap_t *find_pci_cap(uint bus, ushort dev, uint func, uchar capid);
extern pci_xcap_t *get_1st_pci_xcap(uint bus, ushort dev, uint func);
extern pci_xcap_t *get_next_pci_xcap(pci_xcap_t *cap);
extern pci_xcap_t *find_pci_xcap(uint bus, ushort dev, uint func, ushort capid);
#endif
extern void display_pcie_bar_reg(uint pcie_type0_dev);
extern uint get_pcie_type0_device_info(uint pcie_device, pcie_info_t *info);
extern uint get_pcie_type1_device_info(uint pcie_device, pcie_info_t *info);
extern uint get_pcie_type0_device_bus(uint pcie_device);
extern uint get_pcie_type1_device_bus(uint pcie_device);
extern uint get_pcie_type0_device_no(uint pcie_device);
extern uint get_pcie_type1_device_no(uint pcie_device);
extern uint get_pcie_type0_device_func_no(uint pcie_device);
extern uint get_pcie_type1_device_func_no(uint pcie_device);
extern void *find_pcie_type1_device(uint pcie_device);
extern void *find_pcie_device_rtport(uint pcie_device);
extern uint get_pcie_dev_field(uint, uchar);
extern ulong get_pcie_bar0(uint);
extern void print_parent_bridges(uint pcie_device);
extern uint get_pcie_dev_info(uint pcie_device, pcie_info_t *info);
extern uint get_index_pcie_dev_info(uint pcie_device);
extern void print_plat_pcie_end_dev_info(void);
extern void reconfig_pcie_type0_regs(void);
extern void print_dnstream_ports (uint pcie_dev, uint num_ports);
extern void show_plx_8509(uint);
extern int usr_set_mch_pcie_err(uint);
extern int usr_show_mch_pcie_errlog_report(uint);
extern uint32_t platform_get_pci_link_status(uint32_t);
extern boolean  platform_is_pci_linkup(uint32_t);

#endif	/* _PLATFORM_PCI_H_ */

/******** History ******** 
$Log: platform_pci.h,v $
Revision 1.2  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

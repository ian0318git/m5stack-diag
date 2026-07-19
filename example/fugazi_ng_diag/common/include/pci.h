/* $Id: pci.h,v 1.5 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/pci.h,v $
 *------------------------------------------------------------------
 * pci.h -- definitions for PCI bus specific devices
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Anh Dang
 *------------------------------------------------------------------
 */

#ifndef _PCI_H_
#define _PCI_H_

#define CAVIUM_56XX_ID                     0x0050177D
#define CAVIUM_58XX_ID                     0x0040177D
#define MPC8572E_PCIE_ID         	   0x00401957
#define GOOFY_PCIE_CORE_ID                 0x001e1137
#define PEX8111_PCE_ID                     0x811110b5
#define XIO2000_PCIE_ID                    0x8231104c

/* some PCI bus constants */
#define PCI_BUS_0      0
#define PCI_BUS_1      1
#define PCI_BUS_2      2
#define PCI_BUS_3      3
#define PCI_BUS_4      4
#define PCI_BUS_5      5
#define PCI_BUS_6      6
#define PCI_BUS_7      7
#define PCI_BUS_8      8
#define PCI_BUS_9      9
#define PCI_BUS_10     10
#define PCI_BUS_11     11
#define PCI_BUS_12     12
#define PCI_BUS_18     18
#define PCI_BUS_19     19
#define PCI_BUS_20     20
#define PCI_BUS_21     21
#define PCI_BUS_26     26
#define PCI_BUS_27     27
#define PCI_BUS_28     28
#define PCI_BUS_29     29
#define PCI_BUS_30     30
#define PCI_BUS_34     34
#define PCI_BUS_35     35
#define PCI_BUS_36     36
#define PCI_BUS_37     37
#define PCI_BUS_38     38
#define PCI_BUS_41     41
#define PCI_BUS_42     42
#define PCI_BUS_43     43
#define PCI_BUS_44     44
#define PCI_BUS_45     45
#define PCI_BUS_50     50
#define PCI_BUS_51     51
#define PCI_BUS_52     52
#define PCI_BUS_53     53
#define PCI_BUS_59     59
#define PCI_BUS_60     60
#define PCI_BUS_61     61
#define PCI_BUS_68     68
#define PCI_BUS_69     69

#define PCI_DEV_0      0
#define PCI_DEV_1      1
#define PCI_DEV_2      2
#define PCI_DEV_3      3
#define PCI_DEV_4      4

/* some PCI bus constants */
#define PCI_0           0
#define PCI_1           1
#define PCI_BUSMAX	255
#define PCI_DEVMAX	31
#define PCI_FUNCMAX	7
#define PCI_REGMAX	255

#define PCI_NETWORK_CTRL       0x02
#define PCI_ETHERNET           0x00
#define PCI_IO_ADDR_SPACE      0x20   /* 32 bytes of IO address space (2^5=32)*/
#define PCI_IOSPACE            0x01   /* I/O space indicator at offset 10h */

/*---------- PCI configuration specific definitions-----------------*/
/*
 * PCI configuration register offsets. Note that a 32 bit access
 * on a longword aligned address is possible and is used in the inlines
 * defined later. Eg:
 * A 32 bit config data read on 0x00 will yield a 32 bit value
 * containing both the vendor id  and device id. 
 */
#define PCI_ADDR_DWORD_ALIGN(x) (x & 0xfffffffc)
#define PCI_VENDOR_ID_OFFSET	0x00
#define PCI_DEVICE_ID_OFFSET	0x02
#define PCI_COMMAND_REG_OFFSET  0x04
#define PCI_STATUS_REG_OFFSET   0x06
#define PCI_REVISION_ID_OFFSET  0x08
#define PCI_PROG_IF_OFFSET      0x09
#define PCI_SUB_CLASS_OFFSET    0x0A
#define PCI_BASE_CLASS_OFFSET   0x0B
#define PCI_LATENCY_OFFSET      0x0C   /* Add in to support MARS */
#define PCI_LAT_TIMER_OFFSET    0x0C   /* Note: Actual byte offset = 0x0D */
#define PCI_IO_BASE_OFFSET      0x10
#define PCI_MMIO_BASE_OFFSET 	0x14
#define PCI_MEM_BAR2_OFFSET     0x18
#define PCI_BUSES_OFFSET        0x18
#define PCI_MEM_BAR3_OFFSET     0x1C
#define PCI_LOCAL_BASE_OFFSET   0x18
#define PCI_BASE_ADDR0_OFFSET   0x10
#define PCI_BASE_ADDR1_OFFSET   0x14
#define PCI_BASE_ADDR2_OFFSET   0x18
#define PCI_INTR_LINE_OFFSET    0x3C

#define PCI_CAP_PTR_OFFSET      0x34
#define PCI_BRIDGE_CTRL_OFFSET 	0x3C
#define PCI_TIMEOUT_OFFSET 	0x40
#define PCI_MSI_CAP_ID_OFFSET   0x60
#define PCI_MSI_MSG_ADDR_OFFSET 0x64
#define PCI_MSI_MSG_UPPER_ADDR_OFFSET  0x68
#define PCI_MSI_MSG_DATA_OFFSET 0x6C
#define PCI_ADAPT_EXT_OFFSET    0x98
#define PCI_SEC_CLK_STS_OFFSET  0x68
#define PCI_ERR_ADDR_OFFSET     0x84
#define PCI_IRQ_MODE_CTL_OFFSET 0xE0
#define PCI_IRQ_MODE_STS_OFFSET 0xE4

#define PCI_CLASS_CODE_MASK     0xFFFFFF00 
#define PCI_BRIDGE_CTRL_RESET 	0x00400000

/*
 * Type 1 header related macros
 */
#define PCI_2ND_BUS_NUM_OFFSET  0x19  /* Type 1 Hdr secondary bus number */
#define PCI_SUB_BUS_NUM_OFFSET  0x1a  /* Type 1 Hdr subordinate bus number */
#define PCI_MEM_BASE_OFFSET     0x20  /* Memory base */
#define PCI_MEM_LIMIT_OFFSET    0x22  /* Memory limit */
#define PCI_PMEM_BASE_OFFSET    0x24  /* Memory limit */
#define PCI_PMEM_LIMIT_OFFSET   0x26  /* Memory limit */

/*
 * PCI Express Capability ID
 */
#define PCI_POWER_MAN_CAP_ID    0x01
#define PCI_MSI_CAP_ID          0x05
#define PCI_EXP_CAP_ID          0x10

/*
 * PCI Express Power Management capability structure data offset
 */
#define PCI_EXP_PM_STAT_CTRL_OFFSET 0x04
#define PCI_EXP_PM_DATA_MASK        0xff000000
#define PCI_EXP_PM_CS_BR_EXT_MASK   0x00ff0000
#define PCI_EXP_PM_STAT_CTRL_MASK   0x0000ffff
#define PCI_EXP_PM_DATA_SHIFT       24
#define PCI_EXP_PM_CS_BR_EXT_SHIFT  16

/*
 * PCI Express Message Signaling Interrupt capability structure data offset
 */
#define PCI_EXP_MSI_MSG_ADR_OFFSET  0x04
#define PCI_EXP_MSI_MSG_UADR_OFFSET 0x08
#define PCI_EXP_MSI_MSG_DATA_OFFSET 0x0c
#define PCI_EXP_MSI_MSG_DATA_MASK   0xffff

/*
 * PCI Express capability structure data byte offset
 */
#define PCI_EXP_CAP_ID_OFFSET      0x00
#define PCI_EXP_CAP_NEXTPTR_OFFSET 0x01
#define PCI_EXP_CAP_OFFSET         0x02
#define PCI_EXP_DEV_CAP_OFFSET     0x04
#define PCI_EXP_DEV_CTRL_OFFSET    0x08
#define PCI_EXP_DEV_STATUS_OFFSET  0x0a
#define PCI_EXP_LINK_CAP_OFFSET    0x0c
#define PCI_EXP_LINK_CTRL_OFFSET   0x10
#define PCI_EXP_LINK_STATUS_OFFSET 0x12
#define PCI_EXP_SLOT_CAP_OFFSET    0x14
#define PCI_EXP_SLOT_CTRL_OFFSET   0x18
#define PCI_EXP_SLOT_STATUS_OFFSET 0x1a
#define PCI_EXP_ROOT_CTRL_OFFSET   0x1c
#define PCI_EXP_ROOT_STATUS_OFFSET 0x20

#define PCI_EXP_CAP_ID_SHIFT       0
#define PCI_EXP_CAP_NEXTPTR_SHIFT  8
#define PCI_EXP_CAP_SHIFT          16
#define PCI_EXP_DEV_CAP_SHIFT      0
#define PCI_EXP_DEV_CTRL_SHIFT     0
#define PCI_EXP_DEV_STATUS_SHIFT   16
#define PCI_EXP_LINK_CAP_SHIFT     0
#define PCI_EXP_LINK_CTRL_SHIFT    0
#define PCI_EXP_LINK_STATUS_SHIFT  16
#define PCI_EXP_SLOT_CAP_SHIFT     0
#define PCI_EXP_SLOT_CTRL_SHIFT    0
#define PCI_EXP_SLOT_STATUS_SHIFT  16
#define PCI_EXP_ROOT_CTRL_SHIFT    0
#define PCI_EXP_ROOT_STATUS_SHIFT  0

/* parse PCI speed */
#define PCI_EXP_LINK_STA_SPD_MASK  0x0000000f
#define PCI_EXP_LINK_STA_SPD_2DOT5 0x00000001
#define PCI_EXP_LINK_STA_SPD_5GT   0x00000002
#define PCI_EXP_LINK_STA_SPD_8GT   0x00000003

/* link capability speed and width mask */
#define PCI_EXP_SPD_WID_MASK      0x000003ff

#define PCI_EXP_CAP_ID_MASK        0x000000ff
#define PCI_EXP_CAP_NEXTPTR_MASK   0x0000ff00
#define PCI_EXP_CAP_MASK           0xffff0000
#define PCI_EXP_DEV_CAP_MASK       0xffffffff
#define PCI_EXP_DEV_CTRL_MASK      0x0000ffff
#define PCI_EXP_DEV_STATUS_MASK    0xffff0000
#define PCI_EXP_LINK_CAP_MASK      0xffffffff
#define PCI_EXP_LINK_CTRL_MASK     0x0000ffff
#define PCI_EXP_LINK_STATUS_MASK   0xffff0000
#define PCI_EXP_SLOT_CAP_MASK      0xffffffff
#define PCI_EXP_SLOT_CTRL_MASK     0x0000ffff
#define PCI_EXP_SLOT_STATUS_MASK   0xffff0000
#define PCI_EXP_ROOT_CTRL_MASK     0x0000ffff
#define PCI_EXP_ROOT_STATUS_MASK   0xffffffff

/* express capability device control register */
#define PCI_EXP_MAX_RD_REQ_MASK    0x00007000
#define PCI_EXP_NO_SNOOP_EN        0x00000800
#define PCI_EXP_AUX_PWR_PM_EN      0x00000400
#define PCI_EXP_PHAN_FUNC_EN       0x00000200
#define PCI_EXP_EXT_TAG_EN         0x00000100
#define PCI_EXP_MAX_PYLD_MASK      0x000000e0
#define PCI_EXP_RLX_ORD_EN         0x00000010
#define PCI_EXP_UNSUP_REQ_EN       0x00000008
#define PCI_EXP_FATAL_ERR_EN       0x00000004
#define PCI_EXP_NONFATAL_ERR_EN    0x00000002
#define PCI_EXP_CORR_ERR_EN        0x00000001
#define PCI_EXP_DEV_CTRL_ERR_MASK  0x0000000F

/* express capability slot control register */
#define PCI_EXP_PWR_CTRLLR_CTRL    0x00000400
#define PCI_EXP_PWR_CTRL_MASK      0x00000300
#define PCI_EXP_ATTN_CTRL_MASK     0x000000c0
#define PCI_EXP_HOT_PLUG_INT_EN    0x00000020
#define PCI_EXP_CMD_CMPL_INT_EN    0x00000010
#define PCI_EXP_PRES_DET_EN        0x00000008
#define PCI_EXP_MLR_SENSOR_EN      0x00000004
#define PCI_EXP_PWR_FLT_DET_EN     0x00000002
#define PCI_EXP_ATTN_BUTTON_EN     0x00000001
#define PCI_EXP_SLOT_CTRL_EN_MASK  0x0000003F

/*
 * PCI Advance Error Reporting capability structure data offset
 */
#define PCI_AER_ENH_CAP_HDR_OFFSET      0x00
#define PCI_AER_UNCORR_ERR_STAT_OFFSET  0x04
#define PCI_AER_UNCORR_ERR_MASK_OFFSET  0x08
#define PCI_AER_UNCORR_ERR_SEV_OFFSET   0x0c
#define PCI_AER_CORR_ERR_STAT_OFFSET    0x10
#define PCI_AER_CORR_ERR_MASK_OFFSET    0x14
#define PCI_AER_CAP_AND_CTRL_OFFSET     0x18
#define PCI_AER_HDR_LOG1_OFFSET         0x1c
#define PCI_AER_HDR_LOG2_OFFSET         0x20
#define PCI_AER_HDR_LOG3_OFFSET         0x24
#define PCI_AER_HDR_LOG4_OFFSET         0x28
#define PCI_AER_ROOT_ERR_CMD_OFFSET     0x2c    /* root port only */
#define PCI_AER_ROOT_ERR_STAT_OFFSET    0x30    /* root port only */
#define PCI_AER_ERR_SRC_ID_OFFSET       0x34    /* root port only */

/*
 * PCI configuration address register mask and shift values.
 */
#define PCI_CONFIG_ADDREG_MASK  0x80000000
#define PCI_CONFIG_BUSNUM       16
#define PCI_CONFIG_DEVNUM       11
#define PCI_CONFIG_FUNCNUM       8
#define PCI_DEVICE_ID_MASK      0xFFFF0000
#define PCI_VENDOR_ID_MASK      0x0000FFFF
#define PCI_COMMAND_MASK        0x0000FFFF
#define PCI_REVISION_MASK       0x000000FF
#define PCI_CACHE_SIZE_MASK     0x000000FF
#define PCI_LATENCY_TIMER_MASK  0x0000FF00
#define PCI_HEADER_TYPE_MASK    0x00FF0000
#define PCI_BIST_MASK           0xFF000000
#define PCI_LATENCY_TIMER_SHIFT 8
#define PCI_INTR_LINE_MASK      0x000000FF

/* 
 * PCI BAR register values
 */
#define	PCI_BAR_PREFETCH	0x8

/*
 * PCI command register MASKS.
 */
#define PCI_COMMAND_INTR_DIS	0x0400
#define PCI_COMMAND_SERR	0x0100
#define PCI_COMMAND_PARITY      0x0040
#define PCI_MEMWR_INVALID       0x0010
#define PCI_COMMAND_BMEN	0x0004
#define PCI_COMMAND_MEMEN       0x0002
#define PCI_COMMAND_IOEN        0x0001
#define PCI_MSI_EN              0x0001
#define PCI_MSI_MM_EN_1         0x0000
#define PCI_MSI_MM_EN_2         0x0010
#define PCI_MSI_MM_EN_4         0x0020
#define PCI_MSI_MM_EN_8         0x0030

/*
 * PCI status register MASKS.
 */
#define PCI_STATUS_FBTBC	0x0080
#define PCI_STATUS_DATAPERR	0x0100
#define PCI_STATUS_DEVSEL   	0x0200
#define PCI_STATUS_STABORT	0x0800
#define PCI_STATUS_RTABORT	0x1000
#define PCI_STATUS_RMBORT	0x2000
#define PCI_STATUS_SERR		0x4000
#define PCI_STATUS_PERR		0x8000

#define SET_MSI_REMAP_BIT       0x400000
#define SET_MSI_MESSAGES_8      0x300000

/*
 * PCI NIM specific defines.
 */
#define PCI_NIM_MAX_DEVICES     8           /* # of pci devices possible */
#define PCI_DEVICE_SCAN         0xffffffff  /* Value to scan PCI IO BASE register */

/*
 * Actual separation between I/O spaces of 2 consecutive PCI devices.
 * NOTE: All PCI designs for C4XXX NIMs should conform to this.
 * This assumes no controller will need more than 255 registers.
 */
#define PCI_NIM_DEV_IO_SEPARATION     	0x0100

#define PCI_NIM_HIGH_ORDER_REG_OFFSET 	0x0240
#define PCI_NIM_CONF_ADDR_REG_OFFSET  	0x0300
#define PCI_NIM_CONF_DATA_REG_OFFSET  	0x20000
#define PCI_NIM_DEVICE_BASE_OFFSET    	0x10000

/* 
 * status register
 */
#define PCI_PERR     0x8000
#define PCI_SERR     0x4000
#define PCI_RMABORT  0x2000
#define PCI_RTABORT  0x1000
#define PCI_STABORT  0x0800
#define PCI_DATAPERR 0x0100

/* 
 * bridge control register
 */
#define PCI_BRDG_PERR     0x0001
#define PCI_BRDG_SERR     0x0002
 

/* advance error reporting base address */
#define AER_BUS0_BASE_ADDR      0x00000100
#define AER_BUS_BASE_ADDR       0x00000800

/* advance error reporting uncorrectable error status/mask register */
#define AER_UNSUP_REQ_ERR       0x00100000
#define AER_ECRC_ERR            0x00080000
#define AER_MALFORMED_TLP       0x00040000
#define AER_RCVR_OVFL           0x00020000
#define AER_UNEX_CMPLTN         0x00010000
#define AER_CMPLTR_ABRT         0x00008000
#define AER_CMPLTN_TO           0x00004000
#define AER_FC_PROTC_ERR        0x00002000
#define AER_POISON_TLP          0x00001000
#define AER_DLP_ERR             0x00000010
#define AER_UNCORR_ERR_MASK     0x001FF010

/* advance error reporting correctable error status/mask register */
#define AER_REPLAY_TMR_TO       0x00001000
#define AER_REPLAY_NUM_RO       0x00000100
#define AER_BAD_TLP             0x00000040
#define AER_RCVR_ERR            0x00000001
#define AER_CORR_ERR_MASK       0x00001141

/*************** Add in to support MARS ***************/

/*
 * Types of PCI Access
 */
#define PCI_TYPE_ZERO		0x00
#define PCI_TYPE_ONE		0x01

/*
 * Typedef for bridge configuration data structure
 */
/*
 * PCI register spec for register access testing
 */
typedef struct pci_reg_spec_t_ {
    char *reg_desc;
    unsigned int mask;       /* 1 => R/W bit position */
    unsigned int offset;     /* from register set base */
} pci_reg_spec_t;

typedef struct msi_msg_t_ {
    unsigned int address_hi;
    unsigned int address_lo;
    unsigned int data;

} msi_msg_t;

/*  
 * MSI  Capability Structure
 */
typedef struct pci_msi_cap_reg_t {
    volatile unsigned char      capid;          /* 00: capability id */
    volatile unsigned char      next;           /* 01: next pointer */
    volatile unsigned short     mcr;            /* 02: msi control reg */
    volatile unsigned int       mar;            /* 04: msi address reg */
    volatile unsigned int       muar;           /* 08: msi upper address reg */
    volatile unsigned short     mdr;            /* 0c: msi data reg */
    volatile unsigned short     _rsv;           /* 0e: resv */
    volatile unsigned int       mmr;            /* 10: msi mask bits */
    volatile unsigned int       mpr;            /* 14: msi pending bits */
} pci_msi_cap_reg_t;

/*
 * Function prototypes
 */

/*=======================================================================*
 *                     	  External declarations                          *
 *=======================================================================*/
extern void display_pci_header(unsigned int, unsigned short, unsigned int);
extern int pci_find_capability(int bus, int dev, int func, int cap);
extern unsigned int pcie_config_read(unsigned int pcie_port, unsigned int bus, unsigned short dev, unsigned int fn, unsigned int reg);
extern void pcie_config_write(unsigned int pcie_port, unsigned int bus, unsigned short dev, unsigned int fn, unsigned int reg, unsigned int val);

extern uchar get_next_pci_exp_cap_ptr(uint32_t, uint16_t, uint32_t, uint32_t);
extern char get_pci_exp_cap_id(uint32_t, uint16_t, uint32_t, uint32_t);

#endif
/******** History ******** 
$Log: pci.h,v $
Revision 1.5  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.4.54.1  2016/12/15 08:50:48  leschen
Define pcie link cap speed and width mask.

Revision 1.4  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.3  2012/11/12 09:59:52  alpeng
support show pcie speed on overdrive

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

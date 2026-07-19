/* $Id: platform_sys_clk.h,v 1.2 2013/05/23 01:09:26 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_sys_clk.h,v $
 *------------------------------------------------------------------------------
 * Filename:	platform_sys_clk.h
 *
 * Description:	IDT ICS9DB403 clock buffer.
 *              This header file defines registers offset, defaults, read &
 *              write bitmasks, & bit locations.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#ifndef __PLATFORM_SYS_CLK_H__
#define __PLATFORM_SYS_CLK_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

#define SYS_CLK_DEF         0
#define SYS_CLK_BUF_SIZE   10

/* Register address offset defines of registers */
#define SQ420D_OUT_EN0          0
#define SQ420D_OUT_EN1          1
#define SQ420D_OUT_EN2          2
#define SQ420D_RESERV1          3
#define SQ420D_RESERV2          4
#define SQ420D_NS_FQ_SEL        5
#define SQ420D_CPU_FQ_SEL       6
#define SQ420D_VEN_ID           7
#define SQ420D_BYTE_CNT         8
#define SQ420D_DEV_ID           9

/* Defines of byte 0: Output enable register 0 */
#define SQ420D_SRC0_EN          0x01
#define SQ420D_SRC1_EN          0x02
#define SQ420D_SRC2_EN          0x04
#define SQ420D_NS_SRC0_EN       0x08
#define SQ420D_NS_SRC1_EN       0x10
#define SQ420D_NS_SAS0_EN       0x20
#define SQ420D_NS_SAS1_EN       0x40
#define SQ420D_DOT96_EN         0x80
#define SQ420D_BYTE0_HW_DFT     0xFF

/* Defines of byte 1: Output enable register 1 */
#define SQ420D_SS_EN            0x01
#define SQ420D_CPU0_EN          0x02
#define SQ420D_CPU1_EN          0x04
#define SQ420D_CPU2_EN          0x08
#define SQ420D_CPU3_EN          0x10
#define SQ420D_REF14_EN         0x80
#define SQ420D_BYTE1_HW_DFT     0x9E

/* Defines of byte 2: Output enable register 2 */
#define SQ420D_48M_EN           0x01
#define SQ420D_PCI0_EN          0x02
#define SQ420D_PCI1_EN          0x04
#define SQ420D_PCI2_EN          0x08
#define SQ420D_PCI3_EN          0x10
#define SQ420D_PCI4_EN          0x20
#define SQ420D_BYTE2_HW_DFT     0x3F

/* byte 3 & 4 are reserved */
#define SQ420D_BYTE3_HW_DFT     0x00
#define SQ420D_BYTE4_HW_DFT     0x00

/* Defines of byte 5:  NS_SAS/NS_SRC Freq. select register */
#define SQ420D_NS_FS_MASK       0x1F
#define SQ420D_BYTE5_HW_DFT     0x0F

/* Defines of byte 6: CPU/SRC/PCI Freq. select register */
#define SQ420D_CPU_FS_MASK      0x0F
#define SQ420D_FS_MASK          0x10
#define SQ420D_T_SEL_MASK       0x40
#define SQ420D_T_MODE_MASK      0x80
#define SQ420D_BYTE6_HW_DFT     0x18

/* Defines of byte 7: Vendor & Rev ID register */
#define SQ420D_VEN_ID_MASK      0x0F
#define SQ420D_REV_ID_MASK      0xF0
#define SQ420D_VEN_ID_DFT       0x01
#define SQ420D_REV_ID_DFT       0x03
#define SQ420D_REV_ID_SHFT        4
#define SQ420D_BYTE7_HW_DFT     0x31

/* Defines of byte 8: Byte Count register */
#define SQ420D_BYTE_CUNT_MASK   0xFF
#define SQ420D_BYTE_CUNT_DFT    0x07
#define SQ420D_BYTE8_HW_DFT     0x0A

/* Defines of byte 9: Device ID register */
#define SQ420D_DEV_ID_MASK      0xFF
#define SQ420D_DEV_ID_DFT       0x17
#define SQ420D_BYTE9_HW_DFT     0x17

/*
 * device callin function - service provided and defined by the device
 */
typedef struct sq420d_callin_fvt_t_ {
    int	(*register_test)(dev_object_t *);
}sq420d_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct sq420d_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
}sq420d_callout_fvt_t;

/* The following struct can be used by the bit definition text descriptor.
 * Each pin has different function in different platforms. This struct
 * provides mechanism allowing the caller to provide special text for
 * each pin.
 */
typedef struct sq420d_bit_t_ {
    char	*name;  /* Text */
    char	*true;  /* Text for mask bit been set. If 0, use default */
    char	*false;	/* Text for mask bit been cleared. If 0, use default */
    uchar	offset;	/* Register offset */
    uchar	mask;   /* bit mask */
} sq420d_bit_t;

/* Registers struct */
typedef struct sq420d_reg_t_ {
    volatile unsigned char count;                    /* Read byte count */
    volatile unsigned char ctl[SQ420D_DEV_ID + 1];
} sq420d_reg_t;

/*
** Define the device object structure.
*/
typedef struct dev_sq420d_object_t_ {
    dev_object_t         base;
    sq420d_callin_fvt_t  *callin_fvt;
    sq420d_callout_fvt_t *callout_fvt;
    n2g_i2c_if_t         *i2c_p;         /* I2C API interface pointer */
    reg_info_t           *reg_table_p;   /* Register test table pointer*/
    sq420d_bit_t         *bit_p;         /* Bit text pointer */
    sq420d_bit_t         enable;         /* Bit enable/disable param */
}dev_sq420d_object_t;


/* Conversion table */
typedef struct sys_clk_conv_t_ {
    uchar    entry_type;     /* Hex, character, end of table */
    uint16_t sys_clk_type;   /* sys_clk_type */
    uchar    offset;         /* Register offset */
    uchar    mask;           /* bit(s) mask */
} sys_clk_conv_t;

/* entry_type defines */
#define CLKBUF_ENT_END	0	/* Last entry */
#define CLKBUF_ENT_HEX	1	/* Hex data */
#define CLKBUF_ENT_CHAR	2	/* Character data */

/* Overlord specific bits defines */
#define OVLD_CAVE_SATA_CLK100  ((SQ420D_OUT_EN0 << 8) | SQ420D_SRC0_EN)
#define OVLD_CAVE_PCIE_CLK100  ((SQ420D_OUT_EN0 << 8) | SQ420D_SRC1_EN)
#define OVLD_CAVE_CRU_CLK100   ((SQ420D_OUT_EN0 << 8) | SQ420D_SRC2_EN)
#define OVLD_GLADDEN_BCLK      ((SQ420D_OUT_EN1 << 8) | SQ420D_CPU0_EN)
#define OVLD_XDP_REF_CLK       ((SQ420D_OUT_EN1 << 8) | SQ420D_CPU1_EN)
#define OVLD_DB1200_DIF_IN     ((SQ420D_OUT_EN1 << 8) | SQ420D_CPU2_EN)
#define OVLD_CAVE_DMI_CLK100   ((SQ420D_OUT_EN1 << 8) | SQ420D_CPU3_EN)
#define OVLD_CAVE_USB_CLK96    ((SQ420D_OUT_EN0 << 8) | SQ420D_DOT96_EN)
#define OVLD_CAVE_PCICLK       ((SQ420D_OUT_EN2 << 8) | SQ420D_PCI0_EN)
#define OVLD_FPGA              ((SQ420D_OUT_EN2 << 8) | SQ420D_PCI1_EN)
#define OVLD_DEBUG             ((SQ420D_OUT_EN2 << 8) | SQ420D_PCI2_EN)
#define OVLD_CAVE_UART_CLK     ((SQ420D_OUT_EN2 << 8) | SQ420D_48M_EN)
#define OVLD_CAVE_CLK14        ((SQ420D_OUT_EN1 << 8) | SQ420D_REF14_EN)

/* Frequency Margin */
#define SQ420D_FREQ_SEL_1003P_M   0x0   /* -10.03% (89.97 if original 100MHz) */
#define SQ420D_FREQ_SEL_872P_M    0x1   /* -8.72%  (91.28 if original 100MHz) */
#define SQ420D_FREQ_SEL_742P_M    0x2   /* -7.42%  (92.58 if original 100MHz) */
#define SQ420D_FREQ_SEL_625P_M    0x3   /* -6.25%  (93.75 if original 100MHz) */
#define SQ420D_FREQ_SEL_495P_M    0x4   /* -4.95%  (95.05 if original 100MHz) */
#define SQ420D_FREQ_SEL_378P_M    0x5   /* -3.78%  (96.22 if original 100MHz) */
#define SQ420D_FREQ_SEL_247P_M    0x6   /* -2.47%  (97.53 if original 100MHz) */
#define SQ420D_FREQ_SEL_117P_M    0x7   /* -1.17%  (98.83 if original 100MHz) */
#define SQ420D_FREQ_SEL_NORMAL    0x8   /* (100.00 if original 100MHz) */
#define SQ420D_FREQ_SEL_130P_P    0x9   /* +1.30%  (101.30 if original 100MHz) */
#define SQ420D_FREQ_SEL_247P_P    0xA   /* +2.47%  (102.47 if original 100MHz) */
#define SQ420D_FREQ_SEL_378P_P    0xB   /* +3.78%  (103.78 if original 100MHz) */
#define SQ420D_FREQ_SEL_508P_P    0xC   /* +5.08%  (105.08 if original 100MHz) */
#define SQ420D_FREQ_SEL_625P_P    0xD   /* +6.25%  (106.25 if original 100MHz) */
#define SQ420D_FREQ_SEL_755P_P    0xE   /* +7.55%  (107.55 if original 100MHz) */
#define SQ420D_FREQ_SEL_1003P_P   0xF   /* +10.03% (110.03 if original 100MHz) */

/* Functions prototype */
extern int sq420d_get_reg(int, char *);
extern int sq420d_set_freq(char ,boolean , boolean);

#endif /* __PLATFORM_SYS_CLK_H__ */

/*------------------------------------------------------------------
$Log: platform_sys_clk.h,v $
Revision 1.2  2013/05/23 01:09:26  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.3  2013/01/31 10:48:46  alpeng
supported CLI cmds for voltage and freq margin

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

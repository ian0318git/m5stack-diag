/* $Id: platform_sys_clk.h,v 1.6 2016/10/16 12:28:22 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_sys_clk.h,v $
 *------------------------------------------------------------------------------
 * Filename:	platform_sys_clk.h
 *
 * Description:	IDT 9VRS442-B clock buffer.
 *              This header file defines registers offset, defaults, read &
 *              write bitmasks, & bit locations.
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
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
#define SYS_CLK_BUF_SIZE_DFT   10 /* Chip default size */
#define SYS_CLK_BUF_SIZE   14
#define SYS_CLK_EXT_BUF_SIZE   24 /* extend buffer size for M/N programming */
#define RS4420B_EXT_BYTE23     23
#define RS4420B_EXT_BYTE17     17
#define RS4420B_EXT_BYTE16     16
#define RS4420B_EXT_BYTE15     15
#define RS4420B_EXT_BYTE14     14

/* Register address offset defines of registers */
#define RS4420B_FR_SE           0 /* Freq select, PD config and SATA src select */
#define RS4420B_DOT96           1 /* DOT96/SRC6 ctrl */
#define RS4420B_OUT_EN1         2 /* output en 1 */
#define RS4420B_OUT_EN2         3 /* output en 2 */
#define RS4420B_OUT_EN3         4 /* output en 3 */
#define RS4420B_CLK_REQ         5 /* output en and SS en */
#define RS4420B_DIF_STP         6 /* CLK REQ A and B */
#define RS4420B_REV_VID         7 /* revision and vender id */
#define RS4420B_INT_PCI         8 /* intentional PCI skew ctrl */
#define RS4420B_BYTE_CNT        9 /* byte count */
#define RS4420B_SIN_END_SLEW    10 /* single-ended slew rate ctrl */
#define RS4420B_DIFF_CTRL       11 /* differential and single-ended slew rate ctrl */
#define RS4420B_MN_PROG         12 /* M/N Programming en */
#define RS4420B_RD_BK           13 /* Readback PCI Stop and WLAN en */

/* Defines of byte 0 */
#define RS4420B_PD_CONFIG       0x01
#define RS4420B_SATA_SEL        0x02
#define RS4420B_FSLA            0x80
#define RS4420B_BYTE0_HW_DFT    0x81

/* Defines of byte 1 */
#define RS4420B_DOT96_SEL       0x80
#define RS4420B_BYTE1_HW_DFT    0x0

/* Defines of byte 2 */
#define RS4420B_PCI_F1_OE       0x02
#define RS4420B_PCI2_OE         0x04
#define RS4420B_PCI3_OE         0x08
#define RS4420B_25M_PCI4_OE     0x10
#define RS4420B_USB_48MHZ       0x40
#define RS4420B_REF0_OE         0x80
#define RS4420B_BYTE2_HW_DFT    0xFF

/* Defines of byte 3 */
#define RS4420B_SATA_OE         0x01
#define RS4420B_SRC4_OE         0x02
#define RS4420B_SRC5_OE         0x04
#define RS4420B_BYTE3_HW_DFT    0xFF

/* Defines of byte 4 */
#define RS4420B_SRC_PLL_EN      0x01
#define RS4420B_DIF0_OE         0x04
#define RS4420B_DIF1_OE         0x08
#define RS4420B_DOT96_OE        0x10
#define RS4420B_SRC1_OE         0x20
#define RS4420B_SRC2_OE         0x40
#define RS4420B_SRC3_OE         0x80
#define RS4420B_BYTE4_HW_DFT    0xFC

/* Defines of byte 5 */
#define RS4420B_CLKREQ_SRC5     0x04
#define RS4420B_CLKREQ_SRC4     0x08
#define RS4420B_CLKREQ_SRC3     0x10
#define RS4420B_CLKREQ_A        0x40
#define RS4420B_CLKREQ_A_EN     0x80
#define RS4420B_BYTE5_HW_DFT    0x10

/* Defines of byte 6 */
#define RS4420B_SRC_STP_EN      0x01
#define RS4420B_DIF0_STP_EN     0x02
#define RS4420B_DIF1_STP_EN     0x04
#define RS4420B_DIF0_STBY_EN    0x08
#define RS4420B_DIF1_STBY_EN    0x10
#define RS4420B_BYTE6_HW_DFT    0x18

/* Defines of byte 7 */
#define RS4420B_VID0            0x01
#define RS4420B_VID1            0x02
#define RS4420B_VID2            0x04
#define RS4420B_VID3            0x08
#define RS4420B_RID0            0x10
#define RS4420B_RID1            0x20
#define RS4420B_RID2            0x40
#define RS4420B_RID3            0x80
#define RS4420B_BYTE7_HW_DFT    0x11

/* Defines of byte 8 Reserverd */

/* Defines of byte 9 */
#define RS4420B_BC0              0x01
#define RS4420B_BC1              0x02
#define RS4420B_BC2              0x04
#define RS4420B_BC3              0x08
#define RS4420B_BC4              0x10
#define RS4420B_BYTE9_HW_DFT     0x0A

/* Defines of byte 10 */
#define RS4420B_25M_PCI0         0x01
#define RS4420B_25M_PCI1         0x02
#define RS4420B_PCI3_PCI0        0x04
#define RS4420B_PCI3_PCI1        0x08
#define RS4420B_REF0             0x10
#define RS4420B_REF1             0x20
#define RS4420B_USB48M0          0x40
#define RS4420B_USB48M1          0x80
#define RS4420B_BYTE10_HW_DFT    0x15

/* Defines of byte 11 */
#define RS4420B_PCI1_PCI0        0x01
#define RS4420B_PCI1_PCI1        0x02
#define RS4420B_PCI2_PCI0        0x04
#define RS4420B_PCI2_PCI1        0x08
#define RS4420B_DOT96_SLEW       0x10
#define RS4420B_SATA             0x20
#define RS4420B_SRC              0x40
#define RS4420B_DIF              0x80
#define RS4420B_BYTE11_HW_DFT    0xF5

/* Defines of byte 12 */
#define RS4420B_SRC_PCI_SS0      0x01
#define RS4420B_SRC_PCI_SS1      0x02
#define RS4420B_DIF_SRC_PCI_MN_EN  0x80
#define RS4420B_BYTE12_HW_DFT   0x0

/* Defines of byte 13 */
#define RS4420B_25M_PCI4         0x02
#define RS4420B_PCI3             0x04
#define RS4420B_PCI2             0x08
#define RS4420B_PCI_F1           0x10
#define RS4420B_WLAN_EN          0x20
#define RS4420B_SEL_PCI          0x40
#define RS4420B_BYTE13_HW_DFT    0x4F

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

typedef struct rs4420b_bit_t_ {
    char	*name;  /* Text */
    char	*true;  /* Text for mask bit been set. If 0, use default */
    char	*false;	/* Text for mask bit been cleared. If 0, use default */
    uchar	offset;	/* Register offset */
    uchar	mask;   /* bit mask */
} rs4420b_bit_t;

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

/* Utah specific bits defines */
#define UTAH_XDP_HOOK_4AND5        ((RS4420B_OUT_EN3 << 8) | RS4420B_SRC1_OE)
#define UTAH_RANGELEY_HPLL         ((RS4420B_OUT_EN3 << 8) | RS4420B_SRC2_OE)
#define UTAH_RENGELEY_DDR3_0_REF   ((RS4420B_OUT_EN3 << 8) | RS4420B_SRC3_OE)
#define UTAH_RANGELEY_DDR3_1_REF   ((RS4420B_OUT_EN2 << 8) | RS4420B_SRC4_OE)
#define UTAH_PCIE_BIF_DIF_IN       ((RS4420B_OUT_EN2 << 8) | RS4420B_SRC5_OE)
#define UTAH_RANGELEY_GBE_REFCLK   ((RS4420B_OUT_EN3 << 8) | RS4420B_DIF0_OE)
#define UTAH_RANGELEY_SATA3_REFCLK ((RS4420B_OUT_EN2 << 8) | RS4420B_SATA_OE)
#define UTAH_RANGELEY_USB_REFCLK   ((RS4420B_OUT_EN3 << 8) | RS4420B_DOT96_OE)
#define UTAH_OPTION_FOR_FPGA       ((RS4420B_OUT_EN1 << 8) | RS4420B_25M_PCI4_OE)
#define UTAH_RANGELEY_CLK14_IN     ((RS4420B_OUT_EN1 << 8) | RS4420B_REF0_OE)

/* Goldbeach specific bits defines */
#define GB_PCIE_BIF_DIF_IN       ((RS4420B_OUT_EN2 << 8) | RS4420B_SRC5_OE)
#define GB_RANGELEY_HPLL         ((RS4420B_OUT_EN3 << 8) | RS4420B_SRC1_OE)
#define GB_RENGELEY_DDR3_0_REF   ((RS4420B_OUT_EN3 << 8) | RS4420B_SRC2_OE)
#define GB_NIM_SLOT_1            ((RS4420B_OUT_EN3 << 8) | RS4420B_SRC3_OE)
#define GB_OPTION_FOR_FPGA       ((RS4420B_OUT_EN2 << 8) | RS4420B_SRC4_OE)
#define GB_RANGELEY_USB_REFCLK   ((RS4420B_OUT_EN3 << 8) | RS4420B_DIF0_OE)
#define GB_RANGELEY_SATA3_REFCLK ((RS4420B_OUT_EN2 << 8) | RS4420B_SATA_OE)
#define GB_RANGELEY_GBE_REFCLK   ((RS4420B_OUT_EN3 << 8) | RS4420B_DOT96_OE)
#define GB_NIM_SLOT_2            ((RS4420B_OUT_EN1 << 8) | RS4420B_25M_PCI4_OE)
#define GB_RANGELEY_CLK14_IN     ((RS4420B_OUT_EN1 << 8) | RS4420B_REF0_OE)

/* Frequency Margin */
/* we are using M/N programming for setting margin, 
 * the data is composed : (reg 23|SS(reg17)|SS(reg16)|VCO N(reg15)|REF M(reg14)) 
 */
#define RS4420B_FREQ_REG23_MSK    0xFF00000000
#define RS4420B_FREQ_REG23_OFS    32
#define RS4420B_FREQ_REG17_MSK    0x00FF000000
#define RS4420B_FREQ_REG17_OFS    24
#define RS4420B_FREQ_REG16_MSK    0x0000FF0000
#define RS4420B_FREQ_REG16_OFS    16 
#define RS4420B_FREQ_REG15_MSK    0x000000FF00
#define RS4420B_FREQ_REG15_OFS    8
#define RS4420B_FREQ_REG14_MSK    0x00000000FF
#define RS4420B_FREQ_REG14_OFS    0

#define RS4420B_FREQ_SEL_M3P0    0x2068CEBD   /* minus 3.0 % */
#define RS4420B_FREQ_SEL_M2P5    0x20AE4795   /* minus 2.5 % */
#define RS4420B_FREQ_SEL_M2P0    0x20F49A2D   /* minus 2.0 % */
#define RS4420B_FREQ_SEL_M1P5    0x213B63DD   /* minus 1.5 % */
#define RS4420B_FREQ_SEL_M1P0    0x21817923   /* minus 1.0 % */
#define RS4420B_FREQ_SEL_M0P5    0x21C7CD3B   /* minus 0.5 % */
#define RS4420B_FREQ_SEL_NORM    0x220EDC3F
#define RS4420B_FREQ_SEL_P0P5    0x2254BA35   /* plus 0.5 % */
#define RS4420B_FREQ_SEL_P1P0    0x229AB3F3   /* plus 1.0 % */
#define RS4420B_FREQ_SEL_P1P5    0x22E19FAD   /* plus 1.5 % */
#define RS4420B_FREQ_SEL_P2P0    0x2327E07F   /* plus 2.0 % */
#define RS4420B_FREQ_SEL_P2P5    0x236DE1BF   /* plus 2.5 % */
#define RS4420B_FREQ_SEL_P3P0    0x23B48FE8   /* plus 3.0 % */

/* for Reversion D chip */
#define RS4420BD_FREQ_SEL_M3P0    0x20AE7093   /* minus 3.0 % */
#define RS4420BD_FREQ_SEL_M2P5    0x20F423c6   /* minus 2.5 % */
#define RS4420BD_FREQ_SEL_M2P0    0x213B41CB   /* minus 2.0 % */
#define RS4420BD_FREQ_SEL_M1P5    0x2181C0A0   /* minus 1.5 % */
#define RS4420BD_FREQ_SEL_M1P0    0x21C71E45   /* minus 1.0 % */
#define RS4420BD_FREQ_SEL_M0P5    0x220E6751   /* minus .5 % */
#define RS4420BD_FREQ_SEL_NORM    0x22546E12   
#define RS4420BD_FREQ_SEL_P0P5    0x229A5C0F   /* plus .5 % */
#define RS4420BD_FREQ_SEL_P1P0    0x22E13148   /* plus 1.0 % */
#define RS4420BD_FREQ_SEL_P1P5    0x232794D8   /* plus 1.5 % */
#define RS4420BD_FREQ_SEL_P2P0    0x236D5D8F   /* plus 2.0 % */
#define RS4420BD_FREQ_SEL_P2P5    0x23B43849   /* plus 2.5 % */
#define RS4420BD_FREQ_SEL_P3P0    0x23FA580E   /* plus 3.0 % */


#define RS4420B_FREQ_MN_BYTE23    0x81   /* value of byte23 for freq. margin */

#define RS4420B_REV_ID_BITS_OFS      4
#define RS4420B_REV_B_ID           0x1
#define RS4420B_REV_D_ID           0x3

/* Functions prototype */
extern int rs4420b_get_reg(int, char *);
extern int rs4420b_set_freq(ulong ,boolean , boolean);

#endif /* __PLATFORM_SYS_CLK_H__ */

/*------------------------------------------------------------------
$Log: platform_sys_clk.h,v $
Revision 1.6  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.5  2014/06/05 08:53:32  danchung
Support clock chip version D

Revision 1.4  2014/01/30 02:03:42  ptong
Call set_byte_count(SYS_CLK_BUF_SIZE_DFT) to set the byte size to vendor default. Utah BIOS expect that default

Revision 1.3  2013/10/14 12:15:34  danchung
Correct the frequency margin programming table to solve the hang issue

Revision 1.2  2013/07/18 17:17:05  mcharon
add -Wal and clean up compile warnings

Revision 1.1  2013/06/17 11:14:50  alpeng
support chip 9VRS4420B and freq margin

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

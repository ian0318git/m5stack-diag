/* $Id: goofy_global.h,v 1.2 2012/03/28 00:38:09 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/include/goofy_global.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's global registers
 *
 * July 2006, Bao Buu
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef GOOFY_GLOBAL_H
#define GOOFY_GLOBAL_H

/*
 * Global register memory map
 */
#define A_OFST_GFY_JTAG_ID                   0x00000000
#define A_OFST_GFY_REV_ID                    0x00000004
#define A_OFST_GFY_DIAG_ECHO                 0x00000008
#define A_OFST_GFY_SW_MEM_BIST               0x0000000c
#define A_OFST_GFY_5MHZ_PRESCALER            0x00000010
#define A_OFST_GFY_RESET_SAMPLED_INPUT       0x00000014
#define A_OFST_GFY_SLEW_RATE_CTRL_GPIO1      0x00000020
#define A_OFST_GFY_SLEW_RATE_CTRL_GPIO2      0x00000024
#define A_OFST_GFY_SLEW_RATE_CTRL_PVDM       0x00000028
#define A_OFST_GFY_SLEW_RATE_CTRL_SGPIO      0x0000002c
#define A_OFST_GFY_SLEW_RATE_CTRL_WIC0       0x00000030
#define A_OFST_GFY_SLEW_RATE_CTRL_WIC1       0x00000034
#define A_OFST_GFY_SLEW_RATE_CTRL_WIC2       0x00000038
#define A_OFST_GFY_SLEW_RATE_CTRL_WIC3       0x0000003c
#define A_OFST_GFY_WICSPI_CTRL0              0x00000040
#define A_OFST_GFY_WICSPI_CTRL1              0x00000044
#define A_OFST_GFY_GLBL_STI_DEBUG_STATE      0x00000050
#define A_OFST_GFY_GLBL_STII_PC_XACTION_CNT  0x00000054
#define A_OFST_GFY_GLBL_STII_NC_XACTION_CNT  0x00000058
#define A_OFST_GFY_GLBL_STIT_PC_XACTION_CNT  0x0000005c
#define A_OFST_GFY_GLBL_STIT_NC_XACTION_CNT  0x00000060
#define A_OFST_GFY_GLBL_STI_ERR              0x00000064
#define A_OFST_GFY_GLBL_STI_ERR_INTR_EN      0x00000068


#define GOOFY_SW_MEM_BIST_TIMEOUT   1000
#define MFGID_RENESAS               0x223
#define GOOFY_WICSPI_TIMEOUT        1000
#define GOOFY_WICSPI_READ_FAILED    0xffff

typedef enum
{
    SLEW_GPIO1 = 0,
    SLEW_GPIO0,
    SLEW_PVDM,
    SLEW_SGPIO,
    SLEW_WIC0,
    SLEW_WIC1,
    SLEW_WIC2,
    SLEW_WIC3,
    MAX_SLEW_CTRL_REGS,    /* this must be the last element */
} goofy_slew_cntrl_t;

enum
{
    WIC_CS_00,
    WIC_CS_01,
    WIC_CS_10_INACT,
    WIC_CS_11_ID_PROM,
};

enum
{
    SPI_100KHZ,
    SPI_200KHZ,
    SPI_1P25MHZ,
    SPI_2P5MHZ
};

typedef struct goofy_global {
    volatile uint32 jtag_id;               /* 0x00 */
    volatile uint32 rev_id;
    volatile uint32 diag_echo;
    volatile uint32 sw_mem_bist;
    volatile uint32 five_mhz_prescaler;    /* 0x10 */
    volatile uint32 reset_sampled_inputs;
    uint32 reserved[2];                    /* 0x18 - 0x1c */
    volatile uint32 slew_rate_ctrl[MAX_SLEW_CTRL_REGS];  /* 0x20 - 0x3c */
    volatile uint32 wic_spi_ctrl0;         /* 0x40 */
    volatile uint32 wic_spi_ctrl1;
    volatile uint32 reserved_1[2];
    volatile uint32 glbdev_sti_dbg_state;  /* 0x50 */
    volatile uint32 glbdev_stii_pc_xact_cnt;
    volatile uint32 glbdev_stii_nc_xact_cnt;
    volatile uint32 glbdev_stit_pc_xact_cnt;
    volatile uint32 glbdev_stit_nc_xact_cnt; /* 0x60 */
    volatile uint32 glbdev_sti_err;
    volatile uint32 glbdev_sti_err_intr_en;
} goofy_global_t;

goofy_global_t *goofy_global_regs;
goofy_global_t goofy_global_tmp_regs;
goofy_global_t *goofy_global_dbg_regs;

int goofy_init_global_regs(int dbgbus);
int goofy_set_5mhz_prescaler(int dbgbus);
int goofy_init_wic_spi(int dbgbus);
ushort goofy_read_wic_spi (int wic, int device, int speed);
int goofy_write_wic_spi (int wic, int device, int speed, ushort data);
int goofy_global_test_diagecho(int dbgbus);
int goofy_global_SWMemBist(void);

/******************************************
 * jtag_id
 *  0: 0 lsb
 * 11: 1 mfg_id
 * 27:12 part_num
 * 31:28 dev_version
 ******************************************/
#define MASK_JTAG_ID_LSB                                    0x00000001
#define MASK_JTAG_ID_MFG_ID                                 0x00000ffe
#define MASK_JTAG_ID_PART_NUM                               0x0ffff000
#define MASK_JTAG_ID_DEV_VERSION                            0xf0000000

/******************************************
 * rev_id
 *  7: 0 minor
 * 15: 8 major
 * 23:16 id_byte1
 * 31:24 id_byte0
 ******************************************/
#define MASK_REV_ID_MINOR                                   0x000000ff
#define MASK_REV_ID_MAJOR                                   0x0000ff00
#define MASK_REV_ID_ID                                      0xffff0000

/******************************************
 * sw_mem_bist
 *  0: 0 run
 *  1: 1 done
 *  2: 2 pass
 * 31: 3 reserved1
 ******************************************/
#define MASK_SW_MEM_BIST_RUN                                0x00000001
#define MASK_SW_MEM_BIST_DONE                               0x00000002
#define MASK_SW_MEM_BIST_PASS                               0x00000004

/******************************************
 * five_mhz_prescaler
 *  5: 0 regs
 * 31: 6 reserved1
 ******************************************/
#define MASK_FIVE_MHZ_PRESCALER_REGS                        0x0000003f
#define MASK_EHWIC_REFCLK_FREQ                              0x00000040

/******************************************
 * reset_sampled_inputs
 *  2:0 I2C address
 * 31:3 reserved1
 ******************************************/
#define MASK_RESET_SAMPLED_INPUTS_I2C_ADDR                  0x00000007

/******************************************
 * slew_rate_ctrl[_i]
 * 31: 0 regs
 ******************************************/
#define MASK_SLEW_RATE_CTRL_GPIO1                           0x0000ffff
#define MASK_SLEW_RATE_CTRL_GPIO0                           0xffffffff
#define MASK_SLEW_RATE_CTRL_PVDM_HD                         0x0000ffff
#define MASK_SLEW_RATE_CTRL_PVDM_HCNTL                      0x000f0000
#define MASK_SLEW_RATE_CTRL_SGPIO_SRCLK_OUT                 0x00000001
#define MASK_SLEW_RATE_CTRL_SGPIO_STCP_OUT                  0x00000002
#define MASK_SLEW_RATE_CTRL_SGPIO_PL_OUT_L                  0x00000004
#define MASK_SLEW_RATE_CTRL_SGPIO_RCLK_OUT                  0x00000008
#define MASK_SLEW_RATE_CTRL_SGPIO_OE_OUT_L                  0x00000010
#define MASK_SLEW_RATE_CTRL_SGPIO_DATA_OUT                  0x00000020
#define MASK_SLEW_RATE_CTRL_WIC_P5_BI                       0x00000001
#define MASK_SLEW_RATE_CTRL_WIC_P7_BI                       0x00000002
#define MASK_SLEW_RATE_CTRL_WIC_P8_OUT                      0x00000004
#define MASK_SLEW_RATE_CTRL_WIC_P26_BI                      0x00000008
#define MASK_SLEW_RATE_CTRL_WIC_P27_OUT                     0x00000010
#define MASK_SLEW_RATE_CTRL_WIC_P28_BI                      0x00000020
#define MASK_SLEW_RATE_CTRL_WIC_P30_BI                      0x00000040
#define MASK_SLEW_RATE_CTRL_WIC_TXCLK_TXCE2_P32             0x00000080
#define MASK_SLEW_RATE_CTRL_WIC_P38_BI                      0x00000100
#define MASK_SLEW_RATE_CTRL_WIC_P40_BI                      0x00000200
#define MASK_SLEW_RATE_CTRL_WIC_P42_BI                      0x00000400
#define MASK_SLEW_RATE_CTRL_WIC_WR_TXCE2_P59                0x00000800
#define MASK_SLEW_RATE_CTRL_WIC_P60_BI                      0x00001000
#define MASK_SLEW_RATE_CTRL_WIC_P61_BI                      0x00002000
#define MASK_SLEW_RATE_CTRL_WIC_P62_BI                      0x00004000
#define MASK_SLEW_RATE_CTRL_WIC_P63_BI                      0x00008000
#define MASK_SLEW_RATE_CTRL_WIC_P65_BI                      0x00010000
#define MASK_SLEW_RATE_CTRL_WIC_DDR_TX_CTRL                 0x00020000
#define MASK_SLEW_RATE_CTRL_WIC_CS_L                        0x00020000
#define MASK_SLEW_RATE_CTRL_WIC_DDR_TX_DATA                 0x00040000
#define MASK_SLEW_RATE_CTRL_WIC_ADDR                        0x00040000
#define MASK_SLEW_RATE_CTRL_WIC_DDR_RX_CTRL                 0x00080000
#define MASK_SLEW_RATE_CTRL_WIC_RD_L                        0x00080000
#define MASK_SLEW_RATE_CTRL_WIC_DDR_RX_DATA                 0x00100000
#define MASK_SLEW_RATE_CTRL_WIC_DATA                        0x00100000

     
/******************************************
 * wic_spi_ctrl0
 * 15: 0 data
 * 17:16 wic0_scs
 * 19:18 wic1_scs
 * 21:20 wic2_scs
 * 23:22 wic3_scs
 * 24:24 reserved1
 * 25:25 start
 * 26:26 done
 * 27:27 reserved2
 * 29:28 speed
 * 30:30 reserved3
 * 31:31 loopback
 ******************************************/
#define OFFSET_WIC_SPI_CTRL0_DATA                           0
#define OFFSET_WIC_SPI_CTRL0_WIC0_SCS                       16
#define OFFSET_WIC_SPI_CTRL0_WIC1_SCS                       18
#define OFFSET_WIC_SPI_CTRL0_WIC2_SCS                       20
#define OFFSET_WIC_SPI_CTRL0_WIC3_SCS                       22
#define OFFSET_WIC_SPI_CTRL0_START                          25
#define OFFSET_WIC_SPI_CTRL0_DONE                           26
#define OFFSET_WIC_SPI_CTRL0_SPEED                          28
#define OFFSET_WIC_SPI_CTRL0_LOOPBACK                       31

#define MASK_WIC_SPI_CTRL0_DATA                             0x0000ffff
#define MASK_WIC_SPI_CTRL0_WIC0_SCS                         0x00030000
#define MASK_WIC_SPI_CTRL0_WIC0_CS_0                        0x00010000
#define MASK_WIC_SPI_CTRL0_WIC0_CS_1                        0x00020000
#define MASK_WIC_SPI_CTRL0_WIC1_SCS                         0x000c0000
#define MASK_WIC_SPI_CTRL0_WIC1_CS_0                        0x00040000
#define MASK_WIC_SPI_CTRL0_WIC1_CS_1                        0x00080000
#define MASK_WIC_SPI_CTRL0_WIC2_SCS                         0x00300000
#define MASK_WIC_SPI_CTRL0_WIC2_CS_0                        0x00100000
#define MASK_WIC_SPI_CTRL0_WIC2_CS_1                        0x00200000
#define MASK_WIC_SPI_CTRL0_WIC3_SCS                         0x00c00000
#define MASK_WIC_SPI_CTRL0_WIC3_CS_0                        0x00400000
#define MASK_WIC_SPI_CTRL0_WIC3_CS_1                        0x00800000
#define MASK_WIC_SPI_CTRL0_START                            0x02000000
#define MASK_WIC_SPI_CTRL0_DONE                             0x04000000
#define MASK_WIC_SPI_CTRL0_SPEED                            0x30000000
#define MASK_WIC_SPI_CTRL0_SPEED_100K                       0x00000000
#define MASK_WIC_SPI_CTRL0_SPEED_250K                       0x10000000
#define MASK_WIC_SPI_CTRL0_SPEED_1_25M                      0x20000000
#define MASK_WIC_SPI_CTRL0_SPEED_2_5M                       0x30000000
#define MASK_WIC_SPI_CTRL0_LOOPBACK                         0x80000000
/******************************************
 * wic_spi_ctrl1
 *  3: 0 invert
 * 31: 4 reserved1
 ******************************************/
#define MASK_WIC_SPI_CTRL1_INVERT                           0x0000000f

/*
 * Global devices STI error register
 * bit defines
 */
#define STII_RESP_CMD_ERR          0x00000001
#define STII_RESP_SIZE_ERR         0x00000002
#define STII_RESP_ERR              0x00000004
#define STII_RESP_NXA_ERR          0x00000008
#define STIT_SIZE_ERR              0x00000010
#define STIT_NPW_ERR               0x00000020
#define STIT_BE_ERR                0x00000040
#define MASK_GLBDEV_STI_ERR_ALL    0x0000007f

/*
 * Global devices STI error interrtupe enable register
 * bit defines
 */
#define STII_RESP_CMD_ERR_INT_EN          0x00000001
#define STII_RESP_SIZE_ERR_INT_EN         0x00000002
#define STII_RESP_ERR_INT_EN              0x00000004
#define STII_RESP_NXA_ERR_INT_EN          0x00000008
#define STIT_SIZE_ERR_INT_EN              0x00000010
#define STIT_NPW_ERR_INT_EN               0x00000020
#define STIT_BE_ERR_INT_EN                0x00000040
#define MASK_GLBDEV_STI_ERR_INTR_EN_ALL   0x0000007f

#endif /* GOOFY_GLOBAL_H */

/******** History ******** 
$Log: goofy_global.h,v $
Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/

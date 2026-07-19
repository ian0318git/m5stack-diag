/* $Id: diag_fpga_i2c_lib.h,v 1.2 2019/06/14 05:24:48 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/diag_fpga_i2c_lib.h,v $
 *------------------------------------------------------------------
 * structures and definitions for goofy's i2c registers
 *
 * 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef DIAG_FPGA_I2C_LIB_H
#define DIAG_FPGA_I2C_LIB_H


/*
 * I2C master register address offset
 */
#define A_OFST_GFY_I2C_CTRL                  0x00000000
#define A_OFST_GFY_I2C_SCRATCH               0x00000004
#define A_OFST_GFY_I2C_STATUS                0x00000008
#define A_OFST_GFY_I2C_STAT_MSK              0x0000000c
#define A_OFST_GFY_I2C_SLV_ADDR              0x00000010
#define A_OFST_GFY_I2C_SLV_SUB_ADDR          0x00000014
#define A_OFST_GFY_I2C_BIT_BANG              0x00000018
#define A_OFST_GFY_I2C_BYTE_COUNT            0x0000001c
#define A_OFST_GFY_I2C_DMA_START_ADDR        0x00000020
#define A_OFST_GFY_I2C_DMA_XFER_SZ           0x00000024
#define A_OFST_GFY_I2C_DMA_NEXT_ADDR         0x00000028
#define A_OFST_GFY_I2C_DMA_REMAIN_XFER_SZ    0x0000002c
#define A_OFST_GFY_I2C_NON_EXIST_STI_ADDR    0x00000030
#define A_OFST_GFY_I2C_STI_STATUS            0x00000034
#define A_OFST_GFY_I2C_STI_STAT_MSK          0x00000038
#define A_OFST_GFY_I2C_DATA_FIFO_ADDR        0x00001800
#define A_OFST_GFY_I2C_DATA_FIFO_RW_PTR      0x00001804

/*
 * Contrl register bit mask
 */
#define MSK_GFY_I2C_CTRL_EN                 0x00000003
#define MSK_GFY_I2C_CTRL_CLK_SEL            0x0000001c
#define MSK_GFY_I2C_CTRL_SLV_EXT_ADDR_MODE  0x00000020
#define MSK_GFY_I2C_CTRL_SPEED              0x00000040
#define MSK_GFY_I2C_CTRL_RW                 0x00000080
#define MSK_GFY_I2C_CTRL_BYTE_LEN           0x0003ff00   //ZZZ 0x0000ff00
#define MSK_GFY_I2C_CTRL_SUB_ADDR_EN        0x03000000   //ZZZ 0x00030000 
#define MSK_GFY_I2C_CTRL_SOFT_RESET         0x04000000   //ZZZ 0x00040000
#define MSK_GFY_I2C_CTRL_SLV_ACK_MSK        0x08000000   //ZZZ 0x00040000
#define MSK_GFY_I2C_CTRL_MUX                0x60000000

/*
 * Contrl register bit position left shift value
 */
#define L_SHFT_GFY_I2C_CTRL_EN                 0
#define L_SHFT_GFY_I2C_CTRL_CLK_SEL            2
#define L_SHFT_GFY_I2C_CTRL_SLV_EXT_ADDR_MODE  5
#define L_SHFT_GFY_I2C_CTRL_SPEED              6
#define L_SHFT_GFY_I2C_CTRL_RW                 7
#define L_SHFT_GFY_I2C_CTRL_BYTE_LEN           8
#define L_SHFT_GFY_I2C_CTRL_SUB_ADDR_EN        24  //16
#define L_SHFT_GFY_I2C_CTRL_SOFT_RESET         26  //18
#define L_SHFT_GFY_I2C_CTRL_SLV_ACK_MSK        27  //19
#define L_SHFT_GFY_I2C_CTRL_MUX                29

/*
 * Contrl register bit field values
 */
#define GFY_I2C_CTRL_DISABLE            0
#define GFY_I2C_CTRL_NORMAL             1
#define GFY_I2C_CTRL_DMA                2
#define GFY_I2C_CTRL_BITBANG            3

#define GFY_I2C_CTRL_CLK_25             0x00000000
#define GFY_I2C_CTRL_CLK_50             0x00000004
#define GFY_I2C_CTRL_SLV_ADDR_7         0x00000000
#define GFY_I2C_CTRL_SLV_ADDR_10        0x00000020
#define GFY_I2C_CTRL_SPEED_NORMAL_100   0x00000000
#define GFY_I2C_CTRL_SPEED_NORMAL_400   0x00000040
#define GFY_I2C_CTRL_SPEED_DMA_400      0x00000000
#define GFY_I2C_CTRL_SPEED_DMA_HI       0x00000040
#define GFY_I2C_CTRL_WR_MODE            0x00000000
#define GFY_I2C_CTRL_RD_MODE            0x00000080
#define GFY_I2C_CTRL_SUB_ADDR_DIS       0x00000000
#define GFY_I2C_CTRL_SUB_ADDR_1BYTE     0x01000000
#define GFY_I2C_CTRL_SUB_ADDR_2BYTE     0x02000000
#define GFY_I2C_CTRL_SUB_ADDR_3BYTE     0x03000000
#define GFY_I2C_CTRL_SOFT_RESET         0x04000000
#define GFY_I2C_CTRL_CHK_SLV_ACK        0x00000000
#define GFY_I2C_CTRL_IGNOR_SLV_ACK      0x08000000

/*
 * Status register bit mask
 */
#define MSK_GFY_I2C_STAT_NOT_ACTIVE      0x00000001
#define MSK_GFY_I2C_STAT_BUS_ERR         0x00000002
#define MSK_GFY_I2C_STAT_NO_SLV          0x00000004
#define MSK_GFY_I2C_STAT_SUB_ADDR_NACK   0x00000008
#define MSK_GFY_I2C_STAT_STD_DONE        0x00000010
#define MSK_GFY_I2C_STAT_DATA_NACK       0x00000020
#define MSK_GFY_I2C_STAT_FIFO_UNDER      0x00000040
#define MSK_GFY_I2C_STAT_FIFO_OVER       0x00000080

/*
 * Slave address register bit mask
 */
#define MSK_GFY_I2C_SLV_ADDR          0x0000007f
#define MSK_GFY_I2C_EXT_SLV_MODE      0x00000380

/*
 * Slave address register bit position left shift value
 */
#define L_SHFT_GFY_I2C_SLV_ADDR          0
#define L_SHFT_GFY_I2C_EXT_SLV_MODE      7

/*
 * I2C timeout defines
 * A typical I2C bus cycle is shown below.
 * Address phase [10 bits]: START, 7-bit addr, 1-bit W/R, 1-bit Ack
 * Data phase [n-bytes * 9 bits + 1-bit]: n*(8-bit data, 1-bit Ack), STOP
 * If P is bit cycle time, the timeout limit is roughly
 *   10*P + n*10*P = P * 10 * (n+1)
 * At 100kHz, P is 10us  (for timeout limit, use 13us for extra time)
 * At 400KHz, P is 2.5us (for timeout limit, use 3us for extra time)
 */
#define I2C_DATA_BIT_COUNT             10
#define I2C_100KHZ_BIT_TIME            13 // 1 bit cycle time in micro sec
#define I2C_400KHZ_BIT_TIME             3 // 1 bit cycle time in micro sec
#define GFY_I2C_XFER_BIT_COUNT(n) (100000)

/*
 * Notes:
 * Do not use goofy_i2c_info_t outside shinkansen files.
 * The xformers use the generic I2C wrapper for the
 * i2c code. We did not have a chance to covert in
 * the shinkansen tree.
 */
typedef struct goofy_i2c_info_
{
    int i2c_addr;
    int mux;
    int i2c_ctrl;
    int     i2c_mode;
    boolean rd_mode;
    uint32_t   speed_mode;
    int     clk_sel;
    boolean slave_addr_7bit;
    uint32_t   slave_addr;
    uint32_t   length;
    uchar   *data;
    uint32_t   sub_slv_addr_size;
    uint32_t   sub_slv_addr;
} goofy_i2c_info_t;

typedef struct fpga_i2c_
{
    volatile uint32_t i2c_control;                /* 0x0  */
    volatile uint32_t i2c_scratch;                /* 0x4  */
    volatile uint32_t i2c_status;                 /* 0x8  */
    volatile uint32_t i2c_status_mask;            /* 0xC  */
    volatile uint32_t i2c_slave_addr;             /* 0x10 */
    volatile uint32_t i2c_slave_sub_addr;         /* 0x14 */
    volatile uint32_t bb;                         /* 0x18 */
    volatile uint32_t byte_cnt;                   /* 0x1C */
    volatile uint32_t pad[8];                     /* 0x20 - 0x3C */
    volatile uint32_t i2c_data_fifo;              /* 0x40 */
} fpga_i2c_t; /* struct goofy_i2c_ */

#define I2C_CMD_TIMEOUT                        100
#define I2C_MAX_DATA_LEN                       0x80

/*
 * Error code returned by the I2C low level write/read operation
 */
/*
enum {
  RC_I2C_OP_OK = 0,
  RC_I2C_BUSY,
  RC_I2C_TIMEOUT, 
  RC_I2C_DMA_ADDR_NOT_64ALIGN,
  RC_I2C_SLV_NACK,
  RC_I2C_SLV_SUB_ADDR_NACK,
  RC_I2C_BUS_ERR,
  RC_I2C_UNKNOWN,  // always last item
};
*/
/*
 * Bit field positions of the I2C master control reg
 */

#define OFFSET_I2C_CNTL_EN                     0
#define OFFSET_I2C_CNTL_CLK_SEL                2
#define OFFSET_I2C_CNTL_SLV_EXT_ADDR           5
#define OFFSET_I2C_CNTL_SPEED_MODE             6
#define OFFSET_I2C_CNTL_RD_MODE                7
#define OFFSET_I2C_CNTL_BYTE_LEN               8
#define OFFSET_I2C_CNTL_SUB_ADDR_EN            16
#define OFFSET_I2C_CNTL_SOFT_RESET             18
#define OFFSET_I2C_CNTL_SLV_ACK_MSK            19
#define OFFSET_I2C_CNTL_HI_DRIVE_SEL           20

#define MASK_I2C_CNTL_EN                       0x00000003
#define MASK_I2C_CNTL_CLK_SEL                  0x0000001C
#define MASK_I2C_CNTL_SLV_EXT_ADDR             0x00000020
#define MASK_I2C_CNTL_SPEED_MODE               0x00000040
#define MASK_I2C_CNTL_RD_MODE                  0x00000080
#define MASK_I2C_CNTL_BYTE_LEN                 0x0000FF00
#define MASK_I2C_CNTL_SUB_ADDR_EN              0x00030000
#define MASK_I2C_CNTL_SOFT_RESET               0x00040000
#define MASK_I2C_CNTL_SLV_ACK_MSK              0x00080000
#define MASK_I2C_CNTL_HI_DRIVE_SEL             0x00100000

#define I2C_CNTL_MODE_WR                       MASK_I2C_CNTL_WR_MODE
#define I2C_CNTL_MODE_RD                       0x00000000
#define I2C_CNTL_SPEED_100                     0x00000000
#define I2C_CNTL_SPEED_400                     MASK_I2C_CNTL_SPEED_MODE
#define I2C_CNTL_SPEED_DMA_400                 0x00000000
#define I2C_CNTL_SPEED_DMA_HIGH                MASK_I2C_CNTL_SPEED_MODE
#define I2C_CNTL_SUB_ADDR_ENABLE               MASK_I2C_CNTL_SUB_ADDR_EN
#define I2C_CNTL_CLK_SEL_25                    0x00000000
#define I2C_CNTL_CLK_SEL_50                    0x00000004
#define I2C_CNTL_CLK_SEL_33                    0x00000008
#define I2C_CNTL_CLK_SEL_66                    0x0000000C
#define I2C_CNTL_EN_DISABLE                    0x00000000
#define I2C_CNTL_EN_NORMAL                     0x00000001
#define I2C_CNTL_EN_DMA                        0x00000002
#define I2C_CNTL_EN_BITBANG                    0x00000003
#define I2C_SLAVE_ADDR_7BIT_MODE               0x00000000
#define I2C_SLAVE_ADDR_EXT_MODE                MASK_I2C_CNTL_SLV_EXT_ADDR
#define I2C_SLV_SUB_ADDR_SIZE_0                0x00000000
#define I2C_SLV_SUB_ADDR_SIZE_1                0x00010000
#define I2C_SLV_SUB_ADDR_SIZE_2                0x00020000
#define I2C_SLV_SUB_ADDR_SIZE_3                0x00030000

#define I2C_STAT_MASK_ALL                      0x000003FF
#define I2C_STAT_DMA_ERR                       0x00000200
#define I2C_STAT_DMA_DONE                      0x00000100
#define I2C_STAT_DFIFO_OV_FLO                  0x00000080
#define I2C_STAT_DFIFO_UN_FLO                  0x00000040
#define I2C_STAT_DAT_NACK                      0x00000020
#define I2C_STAT_STD_DONE                      0x00000010
#define I2C_STAT_SUB_ADDR_NACK                 0x00000008
#define I2C_STAT_DEV_NO_ACK                    0x00000004
#define I2C_STAT_BUS_ERR                       0x00000002
#define I2C_STAT_MASTER_NOT_ACT                0x00000001

#define MASK_I2C_SLAVE_ADDR_ADDRESS            0x000002FF

#define MASK_I2C_SLV_SUB_ADDR_SIZE             0x07000000
#define MASK_I2C_SLV_SUB_ADDR_ADDRESS          0x00FFFFFF

#define I2C_BITBANG_SCL_DRIVER                 0x00000001
#define I2C_BITBANG_SDA_DRIVER                 0x00000002
#define I2C_BITBANG_SCL_IN                     0x00000004
#define I2C_BITBANG_SDA_IN                     0x00000008

#define I2C_DFIFO_WR_PTR_BCOUNT                0x000000FF
#define I2C_DFIFO_WR_PTR_WPTR                  0x000F0000
#define I2C_DFIFO_WR_PTR_RPTR_WRAP             0x00100000
#define I2C_DFIFO_WR_PTR_RPTR                  0x01E00000
#define I2C_DFIFO_WR_PTR_WPTR_WRAP             0x02000000

#define SCL_DRIVE_TIMES   10

/* IO FPGA I2C Master */
typedef enum {
        IOFPGA_I2C_QUACK = 0,       /* Mother Board Quack */
        IOFPGA_I2C_INVALID, /* Invalid I2C */
} IOFPGA_I2C_DEVICE;


extern void gfy_i2c_reset(fpga_i2c_t *i2c);
extern int fpga_i2c_rd(fpga_i2c_t *i2c, uint8_t, uint32_t, int32_t, 
                       uint32_t, uint32_t, uchar *);
extern int fpga_i2c_wr(fpga_i2c_t *, uint8_t, uint32_t, int32_t, 
                      uint32_t, uint32_t, uchar *);
extern int fpga_i2c_scan_addr (int);
extern void fpga_i2c_reset(void);
extern unsigned long get_platform_i2c_addr(int );



#endif      /* DIAG_FPGA_I2C_LIB_H */

/******** History ******** 
$Log: diag_fpga_i2c_lib.h,v $
Revision 1.2  2019/06/14 05:24:48  mikech2
Collapse katar-branch00 to Main Trunk

Revision 1.1.2.1  2018/12/20 09:10:57  peteteng
Add FPGA I2C read/write/scan/reset util

Revision 1.1.2.1  2018/02/27 08:06:41  harrchan
Initial viper application code base



$Endlog$
*/

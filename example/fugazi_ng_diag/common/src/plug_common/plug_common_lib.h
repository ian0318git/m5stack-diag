/* $Id: plug_common_lib.h,v 1.3 2019/08/06 06:56:16 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_common_lib.h,v $
 *------------------------------------------------------------------
 *
 * plug_common_lib.h - Header file for plug common lib
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

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
 *  * Error code returned by the I2C low level write/read operation
 *   */
enum {
    RC_I2C_OP_OK = 0,
    RC_I2C_BUSY,
    RC_I2C_TIMEOUT, 
    RC_I2C_DMA_ADDR_NOT_64ALIGN,
    RC_I2C_SLV_NACK,
    RC_I2C_SLV_SUB_ADDR_NACK,
    RC_I2C_BUS_ERR,
    RC_I2C_UNKNOWN,  /* always last item */
};

extern int plug_slot_module_info(int, uint16_t);
extern uint32_t i2c_dswap4(int);
extern void plug_fpga_wr_i2c_data_fifo(int, uint32_t, uchar *);
extern int plug_common_fpga_i2c_ack_check(int, uint8_t, uint32_t, int32_t, 
                                           uint32_t, uint32_t, uchar *);
extern void plug_fpga_rd_i2c_data_fifo(int, uint32_t, uchar *);
extern int plug_fpga_i2c_normal_op(int, uint8_t, uint32_t, uint32_t,
                                   uint32_t, uint32_t, uint32_t);
extern int plug_fpga_i2c_send_reg_offset (int , uint32_t, uint32_t,
                                          uint32_t, uint32_t);
extern void plug_fpga_i2c_reset(int);
extern boolean plug_fpga_chk_i2c_idle(int i2c_addr);
extern int plug_slot_get_bd_revision(uchar *, unsigned short *);
extern int plug_slot_get_pcb_serial(uchar *, char *);


/*-------------------------------------------------
$Log: plug_common_lib.h,v $
Revision 1.3  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.2  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.2  2018/10/22 09:38:07  hondwang
move plug_slot_module_info to common codeplug_common/plug_common_lib.c

Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
pluggable common code re-instruct add and remove files



$Endlog$
*/

/* $Id: patriot_fpga.h,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_fpga.h
 *
 * Description: Header file for FPGA kernel module
 *
 *      
 * Author: Huan Ngo, port from IOS
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <asm/prom.h>
#include <asm/machdep.h>
#include <linux/interrupt.h>

//#include "../ucc_hdlc/patriot_fpga_fw.h"

#define FPGA_REG_CTRL   0x00
#define FPGA_LED 0x00
#define FPGA_REG_REV    0x0E
#define FPGA_REG_PORT_TYPE_SEL  0x01
#define FPGA_GPIO_REG  0x02
#define FPGA_GPIO_OE_REG  0x02
#define FPGA_TE3_STATUS_REG  0x03
#define FPGA_TE3_LINE_CONFIG_REG  0x04
#define FPGA_SUBRATE_BYPASS 0x40
#define FPGA_CPU_LOOPBACK 0x20
#define FPGA_LED 0x00
#define PATRIOT_XFER_READ 1
#define PATRIOT_XFER_WRITE 0

/* 0x00  TE3 LED's control register */
#define T3E3_TDMFPGA_RX_LOS        0x01
#define T3E3_TDMFPGA_RX_RAI        0x02      
#define T3E3_TDMFPGA_RX_AIS        0x04      
#define T3E3_TDMFPGA_LPBK          0x08     

/* 0x01  Port Type Select Register */
#define T3E3_TDMFPGA_E3MODE        0x01
#define T3E3_TDMFPGA_ENABLE_OSC    0x02
#define T3E3_TDMFPGA_CLOCK_LINE    0x04
#define T3E3_TDMFPGA_ENABLE_FTSOF  0x08
#define T3e3_TDMFPGA_LOOP_PAYLOAD  0x10

/* 0x04  TE Status Register */
#define T3E3_TDMFPGA_PLL_LOCK      0x01
#define T3E3_TDMFPGA_PLL_HOLDOVER  0x02
#define T3E3_TDMFPGA_LIU_LOS       0x04
#define T3E3_TDMFPGA_FRM_LOS       0x08
#define T3E3_TDMFPGA_FRM_LOF       0x10

/* 0x05  TE3 Line Configuration Register */
#define T3E3_TDMFPGA_LIU_ANALOG_LPBK  0x01
#define T3E3_TDMFPGA_LIU_REMOTE_LPBK  0x02
#define T3E3_TDMFPGA_LIU_LPBK_MASK    0x03
#define T3E3_TDMFPGA_LIU_TX_ENABLE    0x04
#define T3E3_TDMFPGA_LIU_TX_TJ_ENABLE 0x08  /* TX jitter attenuation enable */
#define T3E3_TDMFPGA_LIU_TX_RJ_ENABLE 0x14  /* RX jitter attenuation enable */
#define T3E3_TDMFPGA_LIU_TX_MASK      0x0C
#define T3E3_TDMFPGA_LBO_LT_225FT     0x20

/* 0x04  T3 Subrate Mode Selection Register */
#define PKTNM_SUBRATE_CLEAR_MODE      0
#define PKTNM_SUBRATE_DL_MODE         1
#define PKTNM_SUBRATE_KENTROX_MODE    2
#define PKTNM_SUBRATE_LARSCOM_MODE    3
#define PKTNM_SUBRATE_ADTRAN_MODE     4
#define PKTNM_SUBRATE_VERILINK_MODE   5
#define PKTNM_SUBRATE_MODE_MAX        PKTNM_SUBRATE_VERILINK_MODE
#define PKTNM_SUBRATE_T3_MODE_MASK    0x7
#define PKTNM_SUBRATE_E3_MODE_MASK    0x3

/* 0x05, 0x06, 0x07 T3 Subrate Bandwidth Selection Registers */
#define PKTNM_SUBRATE_REG1_MASK  0xFF
#define PKTNM_SUBRATE_REG2_MASK  0xFF00
#define PKTNM_SUBRATE_REG3_MASK  0xFF0000

#define PKTNM_LARSCOM_TIMESLOT_MASK   0xF
#define PKTNM_ADTRAN_TIMESLOT_MASK    0x3
#define PKTNM_VERILINK_TIMESLOT_MASK  0x1F

#define PKTNM_SUBRATE_T3_SCRAMBLE_EN  0x20
#define PKTNM_SUBRATE_E3_SCRAMBLE_EN  0x04

/* 0x10 ICR */
#define PATRIOT_TX_FOFL_INTR_ON 0x01 
#define PATRIOT_TX_FOFL_INTR_OFF 0x00

#define MAX_LEN_OF_DISPLAY_TABLE 40
#define LINE_LEN 40 
#define uchar u8 
typedef volatile struct patriot_fpga_reg_ {
    uchar led;       /* 0x00  LED's Control Register */
    uchar port_type; /* 0x01  Port Type Select Register */
    uchar gpio_reg; /* 0x02 Framer GPIO register */
    uchar gpio_oe_reg; /* 0x03 Framer GPIO OE register */ 
    uchar status;    /* 0x04  TE Status Register */
    uchar t3e3_liu;  /* 0x05  TE3 Line Configuration Register */
    uchar t3_mode;   /* 0x06  T3 Subrate Mode Selection Register */
    uchar t3_bw_1;   /* 0x07  T3 Subrate Bandwidth Selection Register 1 */
    uchar t3_bw_2;   /* 0x08  T3 Subrate Bandwidth Selection Register 2 */
    uchar t3_bw_3;   /* 0x09  T3 Subrate Bandwidth Selection Register 3 */
    uchar e3_mode;   /* 0x0A  E3 Subrate Mode Selection Register */
    uchar e3_bw_1;   /* 0x0B  E3 Subrate Bandwidth Selection Register 1 */
    uchar e3_bw_2;   /* 0x0C  E3 Subrate Bandwidth Selection Register 2 */
    uchar e3_bw_3;   /* 0x0D  E3 Subrate Bandwidth Selection Register 3 */
    uchar fpga_ver;  /* 0x0E   TDM fpga revision register */
    uchar serial_fifo;  /* 0x0F  Nibble to Serial FIFO control register */
    uchar icr;      /* 0x10  Interrupt Cause register */
} __attribute__ ((packed)) patriot_fpga_reg_t; 

struct patriot_fpga {
    struct mutex lock;
    struct device_node *np;
    struct i2c_client *client;
    patriot_fpga_reg_t fpga_reg; 
    struct work_struct work;
    int irq_recvd; 
    u8 reg_ctrl;
    struct patriot_hal_dev_t *hal;
    patriot_intr_dev_t *intr_dev;
};

//prototypes 
int patriot_display_fpga_reg (unsigned long toUser, unsigned char hw);
int patriot_fpga_read_led(void); 
int patriot_fpga_write_led(unsigned char value);
int patriot_fpga_read_porttype(void); 
int patriot_fpga_write_porttype(unsigned char value);
int patriot_fpga_read_frmr_gpio_reg(void);
int patriot_fpga_write_frmr_gpio_reg(unsigned char value);
int patriot_fpga_read_read_frmr_gpio_oe_reg(void);
int patriot_fpga_read_write_frmr_gpio_oe_reg(unsigned char value);
int patriot_fpga_read_te3_status (void);
int patriot_fpga_write_te3_status (unsigned char value);
int patriot_fpga_read_line_config (void);
int patriot_fpga_write_line_config(unsigned char value);
int patriot_fpga_read_T3_subrate_sel (void);
int patriot_fpga_write_T3_subrate_sel (unsigned char value);
int patriot_fpga_read_T3_subrate_bw_sel1 (void);
int patriot_fpga_write_T3_subrate_bw_sel1 (unsigned char value);
int patriot_fpga_read_T3_subrate_bw_sel2 (void);
int patriot_fpga_write_T3_subrate_bw_sel2 (unsigned char value);
int patriot_fpga_read_T3_subrate_bw_sel3 (void);
int patriot_fpga_write_T3_subrate_bw_sel3 (unsigned char value);
int patriot_fpga_read_E3_subrate_sel (void);
int patriot_fpga_write_E3_subrate_sel (unsigned char value);
int patriot_fpga_read_E3_subrate_bw_sel1 (void);
int patriot_fpga_write_E3_subrate_bw_sel1 (unsigned char value);
int patriot_fpga_read_E3_subrate_bw_sel2 (void);
int patriot_fpga_write_E3_subrate_bw_sel2 (unsigned char value);
int patriot_fpga_read_E3_subrate_bw_sel3 (void);
int patriot_fpga_write_E3_subrate_bw_sel3 (unsigned char value);
int patriot_fpga_read_fpga_ver(void); 
int patriot_fpga_read_serial_fifo(void); 
int patriot_fpga_write_serial_fifo (unsigned char value);
int patriot_fpga_read_icr(void); 
int patriot_fpga_write_icr (unsigned char value);

/******** History ********/ 
/*------------------------------------------------------------------------------
 * $Log: patriot_fpga.h,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.2.1  2011/12/21 23:49:22  huanngo
 * Support for FPGA kernel module
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

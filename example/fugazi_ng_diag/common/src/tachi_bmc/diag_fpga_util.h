/* $Id: diag_fpga_util.h,v 1.3 2016/10/27 03:24:46 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_util.h - Header file for FPGA Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FPGA_UTIL__
#define __DIAG_FPGA_UTIL__
extern int lewis_reset_mask_flag ;
extern int diag_fpga_util(void);
extern unsigned int get_diag_fpga_ver(void);
extern void unreset_platform_ext_dev(int);
extern void disable_platform_mcu_intr (int);
extern void disable_platform_vm_mcu_intr(int);
extern int get_platform_plane(void);
extern void enable_platform_mcu_intr();
extern void enable_platform_vm_mcu_intr();
extern int diag_fpga_ver_display(void);
#define FPGA_EXT_BAR_RST               0x40
typedef struct sys_lvl_t_ {
    volatile unsigned int pad1;
    volatile unsigned int ext_rst  /*0x04*/;                   
    volatile unsigned int in_rst /*0x08*/;
    volatile unsigned int pad2; /*0xC; */
#define EXT_PIN_CTRL_MAGIC_VAL  (0xCA << 8)
#define EXT_PIN_CTRL_NEXT_SEL  (1 << 4)
    volatile unsigned int ext_pin_ctrl  /*0x10*/;
    volatile unsigned char pad3[0x6C];  /* 0x80 - 0x10 - 4 */
    volatile unsigned int brd /*0x80*/;
    volatile unsigned int ver /*0x84*/;
    volatile unsigned int slave_ver /*0x88*/;
    volatile unsigned int rev /*0x88*/;
} sys_lvl_t;

extern unsigned long dash_fpga;
#define CP   0
#define FP   1
#define NIOS 2

#define FPGA_CP_INTR_CTRL_REG_OFFSET                      0x100
#define FPGA_FP_INTR_CTRL_REG_OFFSET                      0x200
#define FPGA_NIOS_INTR_CTRL_REG_OFFSET                    0x300
#define FPGA_ENV_MCU_OFFSET     0x33000
#define ENV_MCU_RX_DATA         0x2     /* Received Data */
#define ENV_MCU_TX_DONE         0x1     /* Transmit Done */
#define FPGA_MISC_ENV_MCU                               0x1
#define FPGA_MISC_INTR                                 0x10
#define FPGA_VOL_MON_OFFSET     0x33100
#define FPGA_MISC_VM_MCU   0x2

typedef struct fpga_intr_t_ {
    volatile unsigned int top_sts;                      /* 0x0 */
    volatile unsigned int top_en;                       /* 0x4 */
    volatile unsigned int pad1;                     /* 0x8 */
    volatile unsigned int pad2;                     /* 0xC */
    volatile unsigned int sfp_sts;                  /* 0x10 */
    volatile unsigned int sfp_en;                   /* 0x14 */
    volatile unsigned int pad3[2];                  /* 0x18, 0x1C*/
    volatile unsigned int c2w_sts;                  /* 0x20 */
    volatile unsigned int c2w_en;                   /* 0x24 */
    volatile unsigned int c2w_ovr;             /* 0x28 */
    volatile unsigned int pad3a;                    /* 0x2C */
    volatile unsigned int uart_sts;                 /* 0x30 */
    volatile unsigned int uart_en;                  /* 0x34 */
    volatile unsigned int uart_ovr;                 /* 0x38 */
    volatile unsigned int pad3b;                    /* 0x3C */

#define FPGA_OIR_SATA      0x1000
#define FPGA_OIR_NGWIC3    0x40
#define FPGA_OIR_NGWIC2    0x20
#define FPGA_OIR_NGWIC1    0x10
#define FPGA_OIR_NGSM2     2
#define FPGA_OIR_NGSM1     1
    volatile unsigned int oir_sts;                  /* 0x40 */
    volatile unsigned int oir_en;                   /* 0x44 */
    volatile unsigned int oir_ovr;                  /* 0x48 */
    volatile unsigned int pad4;                     /* 0x4C */

#define FPGA_MISC_SPI                                0x10
    //predefined #define FPGA_MISC_UART                               0x04
    //predefined #define FPGA_MISC_VM_MCU                             0x02
    //predefined #define FPGA_MISC_ENV_MCU                            0x01
    volatile unsigned int misc_sts;                 /* 0x50 */
    volatile unsigned int misc_intr;                /* 0x54 */
    volatile unsigned int misc_ovr;                 /* 0x58 */
} fpga_intr_t;

typedef struct env_dwnld_t_ {
    volatile unsigned int ctrl;
    volatile unsigned int sts;
    volatile unsigned int intr_en;
    volatile unsigned int data;
} env_dwnld_t ;

#endif /* __DIAG_FPGA_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_fpga_util.h,v $
Revision 1.3  2016/10/27 03:24:46  iachang
Fixed Lewis issue with FPGA ver1.4

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/09/21 13:09:16  tirawan
Display temperature sensor and FPGA version during boot up

Revision 1.1.2.3  2015/07/31 10:39:59  alpeng
first check in for testcard

Revision 1.1.2.2  2015/07/31 07:48:43  hondwang
add macro define

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/

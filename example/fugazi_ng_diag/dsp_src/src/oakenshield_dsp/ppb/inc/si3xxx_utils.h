/* $Id: si3xxx_utils.h,v 1.4 2021/04/15 00:52:45 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/si3xxx_utils.h,v $
 *------------------------------------------------------------------------
 * si3xxx_utils.h - Header file for SiLab 32xx chip utilities
 *
 * Oct 2016 - Owen Lin
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------
 */

#ifndef SI3XXX_UTILS_H
#define SI3XXX_UTILS_H


/* Function Prototypes */
extern int send_tdm_packet(uint8_t *, int , uint16_t ,uint16_t);
extern int si3xxx_reg_write(int port, int reg, int data);
extern int si3xxx_reg_read(int port, int reg, int * data);
extern int si3050_reg_write(int port, int reg, int data);
extern int si3050_reg_read(int port, int reg, int * data);
extern ulong verify_si3xxx_lpbk_digital_data (int , int , uint16_t);
extern void TDM_isr(void);
extern int si_write_ram_codec(int , uint, uint16, uint16);
extern int si_read_ram_codec(int , uint, uint16_t *, uint16*);
extern int get_tdm_port(int , int );
extern int get_chan(int, int); 
extern int si3xxx_reg_write_calibr(int *, int, int, int);
extern int si_write_ram_codec_calibr(int *, int, uint, uint16_t, uint16_t);
extern int send_pkt_to_codec_calibr (int *, int, int, int, int*, int, int, int, int);
extern void build_which_port_arr(int *, int);
extern int build_tdm_port_arr(int *, int);
extern int build_channel_num_arr(int *, int);

#define WRITE                    0X1
#define READ                     0X0

#define VOICE                    0x0
#define MONITOR                  0x1
#define FRAME					 0x2    /* send num of channels in a frame */
#define SI3XXX_WRITE             0x0100
#define SI3XXX_READ              0x8100
#define HW_ADDR_CH_0             0x9100
#define HW_ADDR_CH_1             0x8900
#define FXS_CODEC                0x01
#define FXO_CODEC                0x02

/* Vg400 define */
#define REG_DAA_CONTROL          0x5
#define REG_RING_VAL             0x18
#define POS_RING_DETECT          0x24
#define TIME_DETECT_RING         500
#define VG_2FXS_STR_PORT         4

/* Defines */
/* Phoenix define */
#define NUMBER_OF_PHOENIX_SIU    2 
#define NUM_OF_SIU0              0 
#define NUM_OF_SIU1              1 
#define PHOENIX_DB3_START_TDM7_PORT224   224 
#define SEND_TDM_PACKET_DELAY    1000000
#define ONOFF_HOOK_MAP_PORT      2

#endif /* SI3XXX_UTILS_H */

/************* History ************
$Log: si3xxx_utils.h,v $
Revision 1.4  2021/04/15 00:52:45  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.3  2018/08/30 06:40:21  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.2.28.2  2018/05/25 16:27:44  haohsu
Code change for FXO Ring Detection

Revision 1.2.28.1  2018/05/23 17:01:13  haohsu
Add FXO Ring detection

Revision 1.2  2017/07/28 07:58:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:33  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.3  2017/04/26 01:58:29  harrchan
Optimize oakenshield  FXS calibration

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 05:03:50  olin2
Initial commit code for Oakenshield





$Endlog$
*/


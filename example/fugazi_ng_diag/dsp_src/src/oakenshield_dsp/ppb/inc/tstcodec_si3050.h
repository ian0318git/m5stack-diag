/* $Id: tstcodec_si3050.h,v 1.3 2018/08/30 06:40:21 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/tstcodec_si3050.h,v $
 *------------------------------------------------------------------------
 * tstcodec_si3050.h - Header file for SiLab 3050 chip
 *
 * Oct 2016 - Owen Lin
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------
 */

#ifndef TSTCODEC_SI3050_H
#define TSTCODEC_SI3050_H

/* Defines */
typedef struct si3050_init_t_ {
    int reg;	/* Register # */
    int data;	/* Data       */
} si3050_init_t;

typedef struct oak_port_cfg_ {
    int start;
    int size;
} oak_port_cfg;

/* Si3050 Defines */
#define SI3050_CONTROL_1		1
#define SI3050_CONTROL_2		2
#define SI3050_DAA_CONTROL_2		6
#define DAA_CONTROL_3_REG		10
#define SYSTEM_SIDE_CHIP_REV_REG	11
#define LINE_SIDE_CHIP_STATUS_REG	12
#define LINE_SIDE_DEV_REV_REG		13
#define SI3050_RING_V_CTL_1		22
#define SI3050_RING_V_CTL_2		23
#define SI3050_RING_V_CTL_3		24
#define SI3050_DC_TERM_CTRL		26
#define SI3050_GCI_CTL			42

/* DAA Control 3 (Register 10) */
#define DIGITAL_DATA_LOOPBACK 	0x01

/* System Side and Line Side Device Revision (Register 11) */
#define SI3050_LINEID_MASK	0xF0
#define SILAB_ID_3019		0x30	/* Si3019 */

/* Line-Side Device Status (Register 12) */
#define FRAME_DETECT		0x40

/* Function Prototypes */
extern int si3050_codec_digital_loopback(int which_port);
extern int si3050_codec_init(int which_port);
extern void si3050_reset(boolean reset);
extern volatile dspif_info_t *hd_if;
extern void msleep(uint32);
extern int silab_fxo_lpbk_test (void);
extern void reset_si3050(void);

#endif /* TSTCODEC_SI3050_H */

/************* History ************
$Log: tstcodec_si3050.h,v $
Revision 1.3  2018/08/30 06:40:21  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.2.28.1  2018/05/08 23:06:43  haohsu
Add FXS/FXO indivisual LED test

Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:33  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.2  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.1  2016/12/14 05:03:51  olin2
Initial commit code for Oakenshield





$Endlog$
*/


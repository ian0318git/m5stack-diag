 /* $Id: dash_fpga.h,v 1.7 2020/10/07 08:20:48 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/dash_fpga.h,v $
 *------------------------------------------------------------------
 * Filename:    dash_fpga.h
 *
 *
 * Copyright (c) 2018-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *------------------------------------------------------------------
 */
#ifndef __DASH_FPGA__
#define  __DASH_FPGA__

#define FPGA_RST_ACT2   0x4


extern unsigned long dash_fpga;

extern int read_eeprom_block(unsigned int, unsigned int size, unsigned char *);
extern void reset_plat_dev(unsigned int);
extern void unreset_plat_dev(unsigned int);

/* For NIM Dummy function */
extern int is_goldbeach(void);
extern int is_sword(void);
extern int is_dagger(void);
extern int is_vg450(void);
extern int is_juno_plx(void);
extern int is_utah_plx(void);
extern int is_ntpn_machines(void);
extern int is_tabeil(void);
extern int is_juno(void);
extern int is_neptune(void);
extern int is_curie_1ru(void);
extern int is_curie_2ru(void);
extern int is_promethium(void);
extern int is_overlord(void);
extern int is_triton(void);
extern void get_platform_bd_rev(unsigned int *);

#define FAN_PWM_SLOPE_DEFAULT    0x14
#define FAN_PWM_SLOPE_MAX        0x3FF
#define FAN_SPD_30PER_PWM        0x258   /* 30% PWM duty cycle */
#define FAN_SPD_35PER_PWM        0x2BC   /* 35% PWM duty cycle (default) */
#define FAN_SPD_40PER_PWM        0x320   /* 40% PWM duty cycle (default) */
#define FAN_SPD_50PER_PWM        0x3e8   /* 50% PWM duty cycle */
#define FAN_SPD_60PER_PWM        0x4B0   /* 60% PWM duty cycle */
#define FAN_SPD_70PER_PWM        0x578   /* 70% PWM duty cycle */
#define FAN_SPD_100PER_PWM       0x7D0   /* 100% PWM duty cycle */
#define FAN_TEST_DURATION        10*1000 /* 10s */
#define LOWER_BOUNDARY           0.9    /* Take 90% as lower boundary of pass criteria */
#define UPPER_BOUNDARY           1.1    /* Take 110% as upper boundary of pass criteria */
#define RPS_TO_RPM               60      

enum {
TEST_FAN1 = 1,
TEST_FAN2
};

#endif  /* #if __DASH_FPGA */
/*------------------------------------------------------------------
$Log: dash_fpga.h,v $
Revision 1.7  2020/10/07 08:20:48  kehuang2
CSCvv99413: Collapse Promethium-L into main trunk

Revision 1.6  2020/09/21 06:11:19  kehuang2
CSCv74461: Fan speed test may have the concern to sample the speed in non-linear zone

Revision 1.5  2020/08/17 07:26:18  kehuang2
CSCvv34796: Support fan speed test

Revision 1.4  2020/01/09 01:02:42  jiajliu
Merge Curie 2RU to main trunk

Revision 1.3  2019/12/30 05:59:18  kehuang2
CSCvs55860: Support Gaffham

Revision 1.2  2019/10/17 02:16:19  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.6  2019/09/24 09:43:18  kehuang2
Sync NIM with main trunk

Revision 1.1.2.5  2019/07/16 09:33:36  olin2
Support Promethium platform

Revision 1.1.2.4  2019/06/17 06:19:58  olin2
Correct ACT2 reset

Revision 1.1.2.3  2019/03/26 06:09:16  olin2
Support Dreamliner on Tabei-L

Revision 1.1.2.2  2018/11/16 05:42:09  olin2
Clean up code

Revision 1.1.2.1  2018/10/02 01:49:57  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/

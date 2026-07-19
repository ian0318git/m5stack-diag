/* $Id: fxs_test.h,v 1.4 2021/04/15 00:52:44 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/fxs_test.h,v $
 *------------------------------------------------------------------
 * fxs_test.h
 *      FXS header file 
 *
 * Oct 2016 - Owen Lin
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _FXS_TEST_H_
#define _FXS_TEST_H_

#include <stdint.h>

extern int si32261_codec_digital_loopback(int);
extern int si32261_codec_set_ring(int);
extern int si32261_codec_stop_ring(int);
extern int si32261_protected_mode(int);
extern int si_write_reg_32261(int, uint, uint);
extern int si_read_reg_32261(int, uint, uint*);
extern int si_write_ram_32261(int, uint, uint16, uint16);
extern int si_read_ram_32261(int, uint, uint16*, uint16*);
extern void msleep(uint32);
extern int si32261_common_mode_calibration(int, uint*);
extern int si32261_common_mode_calibration_wo_result(int);
extern uchar get_oak_id(void);
extern int si32261_collect_cal_result(void);
extern int si32261_save_cal_data(void);
extern int get_fxs_port_num(void);
extern uint32_t env_set_string(char *, char *);
extern uint32_t env_sync(int);
extern void env_init(void);
extern uint32_t env_get(void);
extern volatile dspif_info_t *hd_if;
extern int silab_fxs_lpbk_test(void);
extern int fxs_fxo_led_utility(void);
extern int get_tdm_bus_num(int, int);
extern void reset_fxs_codec(void);
extern int si32261_codec_set_onoff_hook_map(int);
extern int get_fxs_which_codec(uchar, int);

#define CAL_LB_ALL   0x0F
#define COMP_5V      0x51EB82L
#define SI32261_FORWARD_ACTIVE  0x1
#define SRAM_CLEAR_COMPLETED    0x10

/* FXS FXO port macro */
#define FXS_TYPE 1
#define FXO_TYPE 0

#define FXS_PORT0       0
#define FXS_PORT7       7
#define FXS_PORT12      12
#define FXS_PORT15      15
#define FXS_PORT16      16
#define FXS_PORT23      23
#define FXS_PORT24      24
#define FXS_PORT31      31
#define FXS_PORT32      32
#define FXS_PORT33      33
#define FXS_PORT34      34
#define FXS_PORT35      35
#define FXS_PORT36      36
#define FXS_PORT39      39
#define FXS_PORT40      40
#define FXS_PORT47      47
#define FXS_PORT48      48
#define FXS_PORT49      49
#define FXS_PORT50      50
#define FXS_PORT52      52
#define FXS_PORT55      55
#define FXS_PORT56      56
#define FXS_PORT64      64
#define FXS_PORT65      65
#define FXS_PORT66      66
#define FXS_PORT68      68
#define FXS_PORT71      71
#define FXS_PORT72      72
#define FXS_PORT79      79
#define FXS_PORT80      80
#define FXS_PORT83      83
#define FXS_PORT84      84
#define FXS_PORT95      95
#define FXS_PORT96      96
#define FXS_PORT100     100
#define FXS_PORT107     107
#define FXS_PORT108     108
#define FXS_PORT112     112
#define FXS_PORT116     116
#define FXS_PORT119     119
#define FXS_PORT120     120    
#define FXS_PORT128     128    
#define FXS_PORT131     131
#define FXS_PORT132     132
#define FXS_PORT143     143
#define FXO_PORT0       0
#define FXO_PORT1       1
#define FXO_PORT3       3
#define FXO_PORT5       5
#define FXO_PORT6       6


typedef struct
{
    int          gain;
    unsigned int scale;
} ProSLIC_GainScaleLookup;

#define EXTENDED_GAIN_MAX 9
#define GAIN_MAX          6
#define GAIN_MIN          -30

#define PHOENIX_FXS_CODEC_SHIFT   6

#endif /* _FXS_TEST_H_ */

/******** History ********
$Log: fxs_test.h,v $
Revision 1.4  2021/04/15 00:52:44  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.3  2017/08/09 08:12:25  harrchan
Display TDM bus number when FXS/FXO loopback fail

Revision 1.2  2017/07/28 07:58:37  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.3  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.2  2017/01/05 06:06:33  olin2
Support FXS Ring and Calibration

Revision 1.1.2.1  2016/12/14 05:03:49  olin2
Initial commit code for Oakenshield



$Endlog$
*/


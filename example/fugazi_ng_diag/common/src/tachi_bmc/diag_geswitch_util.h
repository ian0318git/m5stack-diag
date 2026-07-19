/* $Id: diag_geswitch_util.h,v 1.3 2019/09/10 01:03:39 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_geswitch_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_geswitch_util.h - Header file for GE Switch Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_GESWITCH_UTIL__
#define __DIAG_GESWITCH_UTIL__

extern int diag_geswitch_util(void);
extern int input_packet_cnt;
extern int input_payload_size;
extern int input_pattern;
extern int pattern_fix;

#define MRVL6320_GL_REG_1                0x1B

/*SO: STATS_OPERATION_REG*/
#define MRVL6320_SO   0x1D
#define MRVL6320_SO_STATS_BUSY    0x8000

#define MRVL6320_SO_STATS_OP_FLUSH_ALL_PORT   0x1000
#define MRVL6320_SO_STATS_OP_FLUSH_A_PORT   0x2000
#define MRVL6320_SO_STATS_OP_READ   0x4000
#define MRVL6320_SO_STATS_OP_CAPTURE_ALL_COUNTER   0x5000

#define MRVL6320_SO_HISTOGRAM_MODE_RO   0x0400
#define MRVL6320_SO_HISTOGRAM_MODE_TO   0x0800
#define MRVL6320_SO_HISTOGRAM_MODE_RT   0x0C00

#define MRVL6320_SO_STATS_BANK_0   0x0000
#define MRVL6320_SO_STATS_BANK_1   0x0200

#define MRVL6320_SO_STATS_PORT_2  0x0060
#define MRVL6320_SO_STATS_PORT_6  0x00A0
#define MARVL6320_SO_STATS_PORT(x)  x<<5

#define MRVL6320_SO_STATS_PTR_IN_UNICASTS    0x0004
#define MRVL6320_SO_STATS_PTR_IN_BROADCASTS  0x0006
#define MRVL6320_SO_STATS_PTR_IN_GOOD_OCTET  0x0001
#define MRVL6320_RW_ERROR  (-1)


/*S1:STATS_COUNTER_REG_BYTES_3_2*/
#define MRVL6320_S1   0x1E
/*S0:STATS_COUNTER_REG_BYTES_1_0*/
#define MRVL6320_S0   0x1F






#endif /* __DIAG_GESWITCH_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_geswitch_util.h,v $
Revision 1.3  2019/09/10 01:03:39  haohsu
[CSCvr07313]-Marvell 6320 to BMC eth1 frame error issue

Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/14 05:53:25  benchen2
add diag_geswitch_port_pkt_count_display

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/

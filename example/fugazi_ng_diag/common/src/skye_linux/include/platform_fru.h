/* $Id: platform_fru.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/platform_fru.h,v $
 *----------------------------------------------------------------
 * Filename   : platform_fru.h
 *
 * Description: Enhanced error message for Skye SM FRU PID and
 *              Location Strings, and offset define.
 *
 * Aug 2014, Paul Lin(palin2)
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_FRU_H_
#define _PLATFORM_FRU_H_

#define FRU_SIZE   80

/* define fru offset for Skye */
typedef enum {
    SKYE_DIMM = 0,
    SKYE_FPGA,
    SKYE_SPIROM,
    SKYE_TLK10232,
    SKYE_BP_XAUI,
    SKYE_BP_GE0,
    SKYE_BP_GE1,
    SKYE_I2C,
    SKYE_881514,
} fru_offset_t;

/* Externs */
extern uchar skye_pid[];

extern uchar mb_loc[];
extern uchar cpu0_loc[];
extern uchar cpu1_loc[];
extern uchar dimm0_loc[];
extern uchar dimm1_loc[];
extern uchar i2c_loc[];
extern uchar ge0_loc[];
extern uchar ge1_loc[];
extern uchar fpga_loc[];
extern uchar tlk_loc[];
extern uchar pwr_seq_loc[];
extern uchar clk_buf_loc[];
extern uchar backplane_loc[];
extern uchar mv1514_loc[];

extern fru_table_t platform_fru_table[];

extern unsigned int fru_table_offset;

/* Platform/Module dependent Externs */
extern int  skye_dump_volt_margins(void);
extern int  skye_dump_temps(void);

#endif   /* _PLATFORM_FRU_H_ */

/******** History ******** 
$Log: platform_fru.h,v $
Revision 1.2  2015/05/25 03:59:10  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:27  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.6  2014/09/17 11:13:00  palin2
Removed unused function extern.

Revision 1.1.2.5  2014/09/17 04:35:03  palin2
Updated Skye enhanced error message.

Revision 1.1.2.4  2014/09/02 13:09:44  steja
Update Enhance error code for 88E1514

Revision 1.1.2.3  2014/08/31 23:00:17  palin2
Updated Skye enhanced error message FRU table.

Revision 1.1.2.2  2014/08/28 08:03:19  palin2
Update Skye show all temp. and all voltage margin states utilities to
support enhanced error message.

Revision 1.1.2.1  2014/08/22 04:58:49  palin2
First check-in to enhance Skye error message.

$Endlog$
*/


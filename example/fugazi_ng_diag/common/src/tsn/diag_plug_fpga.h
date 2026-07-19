/* $Id: diag_plug_fpga.h,v 1.2 2018/02/09 09:56:56 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_plug_fpga.h,v $
 *  
 * Filename   : diag_plug_fpga.h
 * Description: Header file of Pluggable Header
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_PLUG_FPGA_H__
#define __DIAG_PLUG_FPGA_H__
 
#define PLUG_FPGA_REG_WIDTH                          (4)
/* Check with HW 1ms enough */
#define PLUG_INT_ACT_WAIT                            (1)

extern int diag_plug_fpga(int);

#endif

/*-------------------------------------------------
$Log: diag_plug_fpga.h,v $
Revision 1.2  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 05:57:48  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.3  2017/09/28 21:46:11  hondwang
Add Moka and Sirius FPGA interrupt 1ms(HW suggest) wait

Revision 1.1.4.2  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.1.2.2  2017/06/22 19:27:10  tirawan
Add LTE Test items and add log section at the bottom of the code


*/


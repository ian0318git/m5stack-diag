/* $Id: dash_fpga.h,v 1.1 2020/08/19 09:50:52 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/dash_fpga.h,v $
 *------------------------------------------------------------------
 * 
 * dash_fpga.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DASH_FPGA__
#define  __DASH_FPGA__

#define FPGA_RST_ACT2   0

extern int read_eeprom_block(unsigned int, unsigned int size, unsigned char *);
extern void reset_plat_dev(unsigned int);
extern void unreset_plat_dev(unsigned int);
extern void reset_tam_dev(void);

#endif  /* #if __DASH_FPGA */

/*-------------------------------------------------
 * $Log: dash_fpga.h,v $
 * Revision 1.1  2020/08/19 09:50:52  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

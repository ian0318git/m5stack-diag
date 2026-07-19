/* $Id: dash_fpga.h,v 1.2 2021/04/15 00:52:23 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/dash_fpga.h,v $
 *------------------------------------------------------------------
 * Filename:    dash_fpga.h
 *
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
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
extern int is_juno(void);
extern int is_neptune(void);
extern int is_curie_1ru(void);
extern int is_curie_2ru(void);


#endif  /* #if __DASH_FPGA */

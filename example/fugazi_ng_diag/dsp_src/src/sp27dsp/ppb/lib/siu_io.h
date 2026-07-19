/* $Id: siu_io.h,v 1.3 2012/06/07 22:50:24 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/siu_io.h,v $
 *------------------------------------------------------------------
 * siu_io.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * siu_io.h
 *
 *  Created on: Sep 18, 2009
 *      Author: dokim
 */

#ifndef SIU_IO_H_
#define SIU_IO_H_

typedef union
{
    struct
    {
        unsigned long
            sidv : 1,
            sibv : 1,
            iactive : 1,
            sodv : 1,
            iferr : 1,
            oferr : 1,
            ioflow : 1,
            ouflow : 1,
            oactive : 1,
            ofsdrop : 1,
            sidfull : 1,
            sodfull : 1,
            iparerr : 1,
            imultperr : 1,
            fill1 : 2,
            iparerrch : 8,
            fill0 : 8;
    } fields;
    long reg;
} SIU_STATUS;

/* reset specified SIU */
int __siu_reset ( int port );

/* start the port */
int __siu_start(int port);

/* stop the port */
int __siu_stop( int port );

/* start internal clk */
void __siu_int_clk_fs_on(int port);

/* stop internal clk */
void __siu_int_clk_fs_off(int port);

/* set bits/frame for intput and output */
int __siu_nbits_set(int port, int input, int output);

/* set input clk */
int __siu_in_clk_set(int port, int active, int internally_gen);

/* set output clk */
int __siu_out_clk_set(int port, int active, int internally_gen);

/* set input frame sync set */
int __siu_in_fsync_set(int port, int active, int internally_gen);

/* set output frame sync set */
int __siu_out_fsync_set(int port, int active, int internally_gen);

/* set clk and frame sync divide ratio */
int __siu_ratio_set(int port, int agcklim, int agfslim);

/* enable internal loopback (for test purpose) */
int __siu_set_loopback_for_test(int port);

/* Disable internal loopback (for test purpose) */
int __siu_reset_loopback_for_test(int port);

/* channel mode or frame mode */
int __siu_mode_set(int port, int mode);

/* set number of channels */
int __siu_chnum_set(int port, int num_ch);

/* enabling/disabling input channel(s) */
int __siu_onoff_in_chs(int port, int group, int ch_bmask, int onoff);

/* enabling/disabling output channel(s) */
int __siu_onoff_out_chs(int port, int group, int ch_bmask, int onoff);

/* masking/unmasking output channel(s) */
int __siu_msk_out_chs(int port, int group, int ch_bmask, int onoff);

/* read status */
SIU_STATUS __siu_chk_status(int port);

/* clear status */
int __siu_clr_status(int port);

int __siu_clk_ctrl(int port, int);
int __siu_shift_order(int port, int , int);
int __siu_agsync(int port);
int __siu_clk_ctl(int port);

#endif /* SIU_IO_H_ */

/******** History ********
$Log: siu_io.h,v $
Revision 1.3  2012/06/07 22:50:24  srane
Support TDM external loopback test.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/


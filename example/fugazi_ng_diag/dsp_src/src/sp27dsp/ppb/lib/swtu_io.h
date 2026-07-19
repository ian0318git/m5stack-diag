/* $Id: swtu_io.h,v 1.2 2012/05/10 22:57:02 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/swtu_io.h,v $
 *------------------------------------------------------------------
 * swtu_io.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * swtu_io.h
 *
 *  Created on: Sep 18, 2009
 *      Author: dokim
 */

#ifndef SWTU_IO_H_
#define SWTU_IO_H_

/*
 * SWTU Destination Interrupt Condition (SIGCON)
 * The SWTU generates an interrupt request following completion of a
 * transfer with DCOL equal to:
 * 	111:  DLASTCOL and DROW equal to DLASTROW/n (where n is DINTC + 1.)
 * 	110:  DLASTCOL/2* and DROW equal to DLASTROW.
 * 	101:  DLASTCOL and DROW equal to DLASTROW.
 * 	100:  DLASTROW.
 * 	011:  DLASTCOL and DROW equal to DLASTROW/2.*
 * 	010:  DLASTCOL.
 * 	001:  DLASTROW/2.
 * or:
 * 	000: The SWTU generates an interrupt request after each single word has been transferred.
 */
#define SIGCON_EVERY_WORD		(0)
#define SIGCON_LROW2			(1)
#define SIGCON_LCOL				(2)
#define SIGCON_LCOL_LROW2		(3)
#define SIGCON_LROW				(4)
#define SIGCON_LCOL_LROW		(5)
#define SIGCON_LCOL2_LROW		(3)
#define SIGCON_LCOL2_LROW_INTC	(7)

#define POSTMOD_2DIMENSIONAL	(1)
#define POSTMOD_1DIMENSIONAL	(2)

typedef union
{
    struct
    {
        unsigned long
            derr : 1,
            serr : 1,
            dperr : 1,
            sperr : 1,
            drdy : 1,
            srdy : 1,
            dbsy : 1,
            sbsy : 1,
            fill0 : 24;
    } fields;
    long reg;
} SWTU_STATUS;

/* Universal Timer setting */
#define __UNI_TIMER_SET(t, ivalue /* initial value */, lvalue /* limit value */, on_off, clk_src ) \
				REG32_WRITE(LSI_SP27XX_TDM_ULIM_RA((t)), ((on_off)<<LSI_SP27XX_TDM_ULIM_ULIMEN_BO)\
					| ((clk_src)<<LSI_SP27XX_TDM_ULIM_ULIMSEL_BO) | (lvalue)); \
				REG32_WRITE(LSI_SP27XX_TDM_UCNT_RA((t)), (ivalue));

/* reset specified SWTU */
int __swtu_reset(int port);

/* halt specified SWTU */
int __swtu_halt(int port);

int __swtu_conf(int port, int dimension, int stride, int reindex, int backward_compat, int autoload\
				, int type_intr, int num_sample_intr /* this is valid only if type_int == SIGCON_LCOL2_LROW_INTC */ );

/* setting up registers for ch buffers for 2D standard mode */
int __swtu_set_ch_buf_2D(int port, int ch_num, int num_sample, int* src, int* dst);

/* setting up registers for ch buffers for 1D and 2D backward compatible mode */
int __swtu_set_ch_buf_1D_2D_Backward(int port, int ncolumn, int nrow, int* src, int* dst);

/* connect universal counter to the channel */
int __swtu_conn_unicnt_to_ch(int port, int ch_num, int uni_cnt);

/* connect universal counter to the channel */
int __swtu_disconn_unicnt_to_ch(int port, int ch_num);

/* start SWTU */
int __swtu_start(int port);

/* start SWTU */
int __swtu_stop(int port);

SWTU_STATUS __swtu_chk_status(int port);

/* clear status */
int __swtu_clr_status(int port);

/* setting up registers for ch buffers -- backward compat*/
/* start address of a buf for each ch will be calculated based on row ,col and the base address of first buf */
int __swtu_set_old(int port, int num_row, int num_col, int* src, int* dst);

#endif /* SWTU_IO_H_ */

/******** History ********
$Log: swtu_io.h,v $
Revision 1.2  2012/05/10 22:57:02  srane
Add TDM support.

Revision 1.1  2012/04/18 09:47:32  srane
Initial checkin


$Endlog$
*/


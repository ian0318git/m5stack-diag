/* $Id: libtdm.h,v 1.4 2012/06/07 22:50:24 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libtdm.h,v $
 *------------------------------------------------------------------
 * libtdm.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * libtdm.h
 *
 *  Created on: Sep 18, 2009
 *      Author: dokim
 */

#ifndef LIBPPBTDM_H_
#define LIBPPBTDM_H_

#define TDM(n)		(n)
#define UNI_CNT(n)	(n)

#define MAX_NUM_UNI_CNT 			(32)
#define MAX_NUM_PORT				(6)
#define MAX_CH						(256)

#define SIU_ILB_ON					(1)
#define SIU_ILB_OFF					(0)

#define SIU_XLB_ON					(1)
#define SIU_XLB_OFF					(0)

#define BIT_SIZE_8					(0)
#define BIT_SIZE_16					(1)
#define BIT_SIZE_4					(2)
#define BIT_SIZE_12					(3)
#define BIT_SIZE_24					(4)
#define BIT_SIZE_32					(5)
#define BIT_SIZE_20					(6)
#define BIT_SIZE_28					(7)
#define BIT_SIZE_40					(8)
#define BIT_SIZE_48					(9)
#define BIT_SIZE_36					(10)
#define BIT_SIZE_44					(11)
#define BIT_SIZE_56					(12)
#define BIT_SIZE_64					(13)
#define BIT_SIZE_52					(14)
#define BIT_SIZE_60					(15)

#define CH_MODE						(0)
#define FRAME_MODE					(1)

#define ACTIVE_CLK						(1)
#define PASSIVE_CLK						(0)

#define ACTIVE_FS						(1)
#define PASSIVE_FS						(0)

#define ON_INTERNAL_CLK					(1)
#define OFF_INTERNAL_CLK				(0)

#define ON_INTERNAL_FS					(1)
#define OFF_INTERNAL_FS					(0)

#define CHAN_ON							(1)
#define CHAN_OFF						(0)

#define CHAN_MASK						(1)
#define CHAN_UNMASK						(0)

#define AUTOLOAD_ON						(1)
#define AUTOLOAD_OFF					(2)

#define NO_BACKWARD_COMPAT				(0)
#define BACKWARD_COMPAT					(1)

#define NOT_USING_UNIVERSAL_COUNTER		(UNI_CNT(MAX_NUM_UNI_CNT))

/*  SCTL and DSCTL Register Bits  */
#define POSTMOD_2DIMENSIONAL			(1)
#define POSTMOD_1DIMENSIONAL			(2)

typedef enum {
	ONE_DIMENSIONAL = 0,
	TWO_DIMENSIONAL,
	TWO_DIMENSIONAL_BACKWARD_COMP
}tdm_array_dimension_t;

typedef enum {
	SINGLE_BUFFERING = 0,
	DOUBLE_BUFFERING,
}tdm_buffering_option_t;

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

#define TDM_INTR_SIU_IN		(1<<0)
#define TDM_INTR_SIU_OUT	(1<<1)
#define TDM_INTR_SWTU_SRC	(1<<2)
#define TDM_INTR_SWTU_DST	(1<<3)

#define TDM_INTR_TO_DSS0	(1<<0)
#define TDM_INTR_TO_DSS1	(1<<1)
#define TDM_INTR_TO_DSS2	(1<<2)
#define TDM_INTR_TO_DSS3	(1<<3)
#define TDM_INTR_TO_ARM		(1<<4)

#define USE_SIU0_INFRAME_SYNC	0
#define USE_SIU1_INFRAME_SYNC	1
#define USE_SIU2_INFRAME_SYNC	2
#define USE_SIU3_INFRAME_SYNC	3
#define USE_SIU4_INFRAME_SYNC	4
#define USE_SIU5_INFRAME_SYNC	5
#define USE_SIU0_OUTFRAME_SYNC	8
#define USE_SIU1_OUTFRAME_SYNC	9
#define USE_SIU2_OUTFRAME_SYNC	10
#define USE_SIU3_OUTFRAME_SYNC	11
#define USE_SIU4_OUTFRAME_SYNC	12
#define USE_SIU5_OUTFRAME_SYNC	13

extern unsigned int code_to_nbits[16];

#define	INTR_SIU_IN 		(1<<0)
#define	INTR_SIU_OUT		(1<<1)
#define	INTR_SWTU_SRC		(1<<2)
#define	INTR_SWTU_DST		(1<<3)
#define INTR_PARITY			(1<<4)

/* TDM interrupt init */
int32_t
sp_TdmIntrInit(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t intr_type);	/* in: intr_type, any combination of  */
								/* < TDM_INTR_SIU_IN, TDM_INTR_SIU_OUT, TDM_INTR_SWTU_SRC, TDM_INTR_SWTU_DST > */
/* TDM interrupt clear */
int32_t
sp_TdmIntrClr(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t intr_type);    /* in: intr_type, any combination of  */
								/* < TDM_INTR_SIU_IN, TDM_INTR_SIU_OUT, TDM_INTR_SWTU_SRC, TDM_INTR_SWTU_DST > */

/* TDM interrupt reset */
int32_t
sp_TdmIntrRst(
		uint32_t port,			/* in: port number (0~5) */
		uint32_t intr_type);	/* in: intr_type, any combination of  */
								/* < TDM_INTR_SIU_IN, TDM_INTR_SIU_OUT, TDM_INTR_SWTU_SRC, TDM_INTR_SWTU_DST > */

/* indentify the interrupt */
uint32_t
sp_TdmIdentifyIntr(uint32_t port);

/* Configure all TDM ports to use common clk (in/out) and fs (in/out) */
int32_t
sp_TdmSIUConfCommonFSCLK(
		uint32_t port_input_clk_fs_from, 	/* in: port that provides input clk and fs to other ports */
		uint32_t port_output_clk_fs_from);  /* in: port that provides output clk and fs to other ports */

/* configure SIU for internal loopback mode */
int32_t
sp_TdmSIUConfInternalLB(
		uint32_t lpbk, 			/* Turn on internal loopback */
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t n_ch, 			/* in: number of channels (1~256) */
		uint32_t n_bit_code, 	/* in: bit size , i.e> BIT_SIZE_8, BIT_SIZE_16 and so on */
		uint32_t mode, 			/* in: CH_MODE or FRAME_MODE */
		uint32_t clk_div);		/* in: clk divider, Siu CLK freq will be sysclk freq * clk_div */

int32_t sp_TdmSIUResetInternalLB( uint32_t port);

/* configure SIU for external loopback test using Base board */
int32_t
sp_TdmSIUConfExtLB_usingBB(
		uint32_t port,			/* in: port number (0~5) */
		uint32_t n_ch, 			/* in: number of channels (1~256) */
		uint32_t n_bit_code, 	/* in: bit size , i.e> BIT_SIZE_8, BIT_SIZE_16 and so on */
		uint32_t mode);			/* in: CH_MODE or FRAME_MODE */

/* configure SWTU for 2D (new add) mode */
int32_t
sp_TdmSWTU2Dconfig(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t autoload,		/* in: enabling autoload, when '1', it will repeat opeartion forever, when '0', it will do only 1 time transaction */
		uint32_t when_intr, 	/* in: interrupt mode, one of <SIGCON_EVERY_WORD, SIGCON_LROW2 ... and so on> */
		uint32_t nfrm_intr);	/* in: number of samples before interrupt is generated, This is valid only if when_int == SIGCON_LCOL2_LROW_INTC */

/* configure SWTU for 1D mode */
int32_t
sp_TdmSWTU1Dconfig(
		uint32_t port,			/* in: port number (0~5) */
		uint32_t autoload,		/* in: enabling autoload, when '1', it will repeat opeartion forever, when '0', it will do only 1 time transaction */
		uint32_t when_intr,		/* in: interrupt mode, one of <SIGCON_EVERY_WORD, SIGCON_LROW2 ... and so on> */
		uint32_t n_channel_intr /* how many rows before interrupt is generated? */);

/* configure SWTU for 2D backward compatible mode */
int32_t
sp_TdmSWTU2D_backward_config(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t autoload,		/* in: enabling autoload, when '1', it will repeat opeartion forever, when '0', it will do only 1 time transaction */
		uint32_t stride,		/* in: stride parameter */
		uint32_t reindex,		/* in: reindex parameter */
		uint32_t when_intr,		/* in: interrupt mode, one of <SIGCON_EVERY_WORD, SIGCON_LROW2 ... and so on> */
		uint32_t n_sample_intr /* this is valid only if type_int == 0x7 */);

/* initialize channels -- assign buffers to each channel for 2D standard mode */
int32_t
sp_Tdm2DInitChan(
		uint32_t port,			 /* in: port number (0~5) */
		uint32_t ch_num_from, 	 /* in: src and dst buffer will be assigned channels from <ch_num_from>  */
		uint32_t ch_num_to, 	 /* in: to <ch_num_to> */
		uint32_t nbits_sample, 	 /* in: bits per sample */
		uint32_t nsample, 		 /* in: number of samples */
		uint32_t* src_base, 	 /* in: ptr to start address of src buffer */
		uint32_t* dst_base, 	 /* in: ptr to start address of dst buffer */
		uint32_t uni_cnt, 		 /* in: universal counter's id that is used for those channels if uni_cnt > 32, then universal counter will not be used for this channel */
		uint32_t double_buffering ); /* in: '1' : double buffering mode, '0' : single buffering mode*/

/* initialize channels -- assign buffers to each channel and configure counters for 1D compatible mode */
int32_t
sp_Tdm1D_InitChan(
		uint32_t port,			/* in: port number (0~5) */
		uint32_t nchannel, 		/* in: number of channels defined */
		uint32_t nsample, 		/* in: number of samples */
		uint32_t* src_base, 	/* in: ptr to start address of src buffer */
		uint32_t* dst_base, 	/* in: ptr to start address of dst buffer */
		uint32_t double_buffering ); /* in: '1' : double buffering mode, '0' : single buffering mode*/

/* initialize channels -- assign buffers to each channel and configure counters for 2D backward compatible mode */
int32_t
sp_Tdm2D_Backward_InitChan(
		uint32_t port,			/* in: port number (0~5) */
		uint32_t nchannel, 		/* in: number of channels defined */
		uint32_t nsample, 		/* in: number of samples */
		uint32_t* src_base, 	/* in: ptr to start address of src buffer */
		uint32_t* dst_base, 	/* in: ptr to start address of dst buffer */
		uint32_t double_buffering ); /* in: '1' : double buffering mode, '0' : single buffering mode*/


/* set-up universal counter of TDM block */
void
sp_TdmInitUniversalCnt(
		uint32_t unicnt,		/* in: universal counter's id */
		uint32_t src_clk, 		/* in: type of Universal counter's src clk i.e) USE_SIU0_INFRAME_SYNC */
		uint32_t limit);		/* in: counter's limit */

/* enable input channels in a range */
int32_t
sp_TdmEnableInChan(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t ch_num_from, 	/* in: range <ch_num_from> to <ch_num_to> */
		uint32_t ch_num_to);

/* enable output channels in a range */
int32_t
sp_TdmEnableOutChan(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t ch_num_from, 	/* in: range <ch_num_from> to <ch_num_to> */
		uint32_t ch_num_to);

/* disable input channels in a range */
int32_t
sp_TdmDisableInChan(
		uint32_t port,			/* in: port number (0~5) */
		uint32_t ch_num_from, 	/* in: range <ch_num_from> to <ch_num_to> */
		uint32_t ch_num_to);

/* disable output channels in a range */
int32_t
sp_TdmDisableOutChan(
		uint32_t port, 			/* in: port number (0~5) */
		uint32_t ch_num_from,	/* in: range <ch_num_from> to <ch_num_to> */
		uint32_t ch_num_to);

/* Start internally generated CLK and FS */
int32_t
sp_TdmInternalClkRun(
		uint32_t port);			/* in: port number (0~5) */

/* start TDM */
int32_t
sp_TdmRun(
		uint32_t port);			/* in: port number (0~5) */

/* halt TDM movement */
int32_t
sp_TdmStop(
		uint32_t port);

#endif /* LIBPPBTDM_H_ */

/******** History ********
$Log: libtdm.h,v $
Revision 1.4  2012/06/07 22:50:24  srane
Support TDM external loopback test.

Revision 1.3  2012/05/24 23:23:08  srane
Add wrapper for TDM external loopback test.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/


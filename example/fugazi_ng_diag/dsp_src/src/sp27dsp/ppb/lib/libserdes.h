/* $Id: libserdes.h,v 1.2 2012/05/10 22:48:11 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libserdes.h,v $
 *------------------------------------------------------------------
 * libserdes.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef LIBSERDES_H_
#define LIBSERDES_H_

/*----------------------------------*/
/*   	CONSTANTS    				*/
/*----------------------------------*/

#ifndef sRIO_1x4_mode
#define sRIO_1x4_mode	0
#endif

#ifndef sRIO_2x1_mode
#define sRIO_2x1_mode	1
#endif

#define sRIO0_x1_mode	2
#define sRIO1_x1_mode	3

/*----------------------------------*/
/*   	MACROS    					*/
/*----------------------------------*/

#define CHAN(ch)	(1 << ch)

#define NWORDS_TO_PRBS57_PCWORD15_0_BM(x) (((x) & 0xffff) & LSI_SP27XX_PRBS57_PCWORD15_0_BM)
#define NWORDS_TO_PRBS58_PCWORD31_16_BM(x) ((((x) & 0xffff0000) >> 16) & LSI_SP27XX_PRBS58_PCWORD31_16_BM)
#define NWORDS_TO_PRBS59_PCWORD39_32_BM(x) ((((x) & 0xff00000000LL) >> 32) & LSI_SP27XX_PRBS59_PCWORD39_32_BM)

/* PRBS test function status return masks (it is assumed that there are
 * a maximum of 4 SerDes channels available per SerDes block) */
#define ERR_NO_SYNC_ON_CH(offset)					(0x1<<offset)
#define ERR_PCERROF_ASSERTED_ON_CH(offset)			(0x10<<offset)
#define ERR_PCWORDUF_ASSERTED_ON_CH(offset)			(0x100<<offset)
#define ERR_PCSEL_NOT_ASSERTED_ON_CH(offset)		(0x1000<<offset)
#define ERRORS_FOUND_ON_CH(offset)					(0x10000<<offset)
#define ERR_PRBS_CHKR_CAPTURED_WORDS_ON_CH(offset)	(0x100000<<offset)
#define ERR_PCDONE_NEVER_SET_ON_CH(offset)			(0x1000000<<offset)
#define ERR_RX_FRQINVLD_SET_ON_CH(offset)			(0x10000000LL<<offset)
#define ERR_PCLOS_ASSERTED_ON_CH(offset)			(0x100000000LL<<offset)
#define MISC_ERR									0xF000000000LL

/*----------------------------------*/
/*   	TYPE DEFINITIONS    		*/
/*----------------------------------*/

typedef enum {
	GBAUD_3_125 = 0,
	GBAUD_2_5,
	GBAUD_1_25,
	GBAUD_5_0,
	DISABLE_SERDES,
	ENABLE_SERDES
} serdesRateCfg_t;

typedef enum {
	SERIAL_LOOPBACK = 0,
	PARALLEL_LOOPBACK,
	SYNC_REVERSE_LOOPBACK,
	ASYNC_REVERSE_LOOPBACK,
	DISABLE_LOOPBACK
} serdesLoopbackType_t;

typedef enum {
	SRIO_SERDES = 0,
	GIGE_SERDES,
	PCIE_SERDES
} serdesInterface_t;

typedef enum {
	PRBS7 = 0,
	PRBS15,
	PRBS23,
	PRBS31
} serdesPrbsPoly_t;

typedef enum {
	PRBS_stop = 0,
	PRBS_start
} serdesPrbsCtl_t;

typedef enum {
	run4ever_off = 0,
	run4ever_on
} run4ever_t;

typedef struct {
	uint32_t pcinsync;
	uint32_t pcdone;
	uint32_t pcerrof;
	uint32_t pcworduf;
	uint32_t pcsel;
	uint64_t word_cnt;
	uint32_t err_cnt;
	uint32_t pclos;
} serdesPrbsChkrStat_t;

typedef struct {
	serdesInterface_t	interface;
	serdesRateCfg_t		rate;
	uint8_t				lane;
	serdesLoopbackType_t loopback;
	serdesPrbsPoly_t	poly;
	uint64_t			numwords;
	uint8_t				ckiInMHz;
	run4ever_t			run4ever;
} serdesTestParams_t;

/*----------------------------------*/
/* 		FUNCTION DECLARATIONS 		*/
/*----------------------------------*/

/* Initialize GIGE SerDes */
int32_t /* Return: SUCCESS or ERROR */
sp_GigeSerdesInit(
		serdesRateCfg_t speed, /* in: speed of each SerDes lane */
		uint8_t channel, /* in: GIGE port that will receive the "speed" parameter; bitmap with CHAN(0) and CHAN(1) */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Initialize sRIO SerDes */
int32_t /* Return: SUCCESS or ERROR */
sp_SrioSerdesInit(
		serdesRateCfg_t speed, /* in: speed of each SerDes lane */
		uint8_t lane_mode, /* in: lane mode, one of sRIO_1x4_mode, sRIO_2x1_mode, sRIO0_x1_mode, or sRIO1_x1_mode */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Initialize PCIe SerDes separate from PIPE layer for PRBS testing purposes */
/* Both lanes 0 and 1 are initialized */
int32_t /* Return: SUCCESS or ERROR */
sp_PcieSerdesInit(serdesRateCfg_t speed, /* in: speed of each SerDes lane */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Setup the loopback path in the GIGE SerDes block */
int32_t  /* Return: SUCCESS or ERROR */
sp_GigeSerdesSetupLB(
		uint8_t channel, /* in: GIGE port that will be in loopback; bitmap with CHAN(0) and CHAN(1) */
		serdesLoopbackType_t loopback, /* in: loopback type */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Setup the loopback path on all 4 lanes in the sRIO SerDes block */
int32_t /* Return: SUCCESS or ERROR */
sp_SrioSerdesSetupLB(
		uint8_t lane_mode, /* in: lane mode, one of sRIO_1x4_mode, sRIO_2x1_mode, sRIO0_x1_mode, or sRIO1_x1_mode */
		serdesLoopbackType_t loopback, /* in: loopback type */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Setup the loopback path in the PCIe SerDes block */
int32_t /* Return: SUCCESS or ERROR */
sp_PcieSerdesSetupLB(
		uint8_t channel, /* in: PCIe SerDes channel that will be in loopback; bitmap with CHAN(0) and CHAN(1) */
		serdesLoopbackType_t loopback, /* in: loopback type */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Perform PRBS testing using the GIGE SerDes block */
/* 64K words are generated and checked; (PRBS7 = X7 + X6 + 1) polynomial used */
uint64_t /* Return: 64-bit mask signifying the error on a specific SerDes channel */
sp_GigeSerdesPRBSChk(
		serdesLoopbackType_t loopback, /* in: loopback type */
		serdesRateCfg_t rate, /* in: current SerDes serial rate */
		uint8_t ckiInMHz, /* in: Current CKI input frequency to the SP2704 */
		uint8_t channel); /* in: SerDes channel that will get tested with PRBS; bitmap */

/* Perform PRBS testing using all 4 lanes of the sRIO SerDes block */
/* 64K words are generated and checked; (PRBS7 = X7 + X6 + 1) polynomial used */
uint64_t /* Return: 64-bit mask signifying the error on a specific SerDes channel */
sp_SrioSerdesPRBSChk(
		serdesLoopbackType_t loopback, /* in: loopback type */
		serdesRateCfg_t rate, /* in: current SerDes serial rate */
		uint8_t ckiInMHz, /* in: Current CKI input frequency to the SP2704 */
		uint8_t lane_mode); /* in: lane mode, one of sRIO_1x4_mode, sRIO_2x1_mode, sRIO0_x1_mode, or sRIO1_x1_mode */

/* Perform PRBS testing using the PCIe SerDes block */
/* 64K words are generated and checked; (PRBS7 = X7 + X6 + 1) polynomial used */
uint64_t /* Return: 64-bit mask signifying the error on a specific SerDes channel */
sp_PcieSerdesPRBSChk(
		serdesLoopbackType_t loopback, /* in: loopback type */
		serdesRateCfg_t rate, /* in: current SerDes serial rate */
		uint8_t ckiInMHz, /* in: Current CKI input frequency to the SP2704 */
		uint8_t channel); /* in: SerDes channel that will get tested with PRBS; bitmap */

/* Generic SerDes loopback path setup function for all SerDes blocks in the SP2704 */
int32_t /* Return: SUCCESS or ERROR */
sp_SerdesSetupLB(
		serdesInterface_t interface, /* in: SerDes interface to be in loopback */
		serdesLoopbackType_t loopback, /* in: loopback type */
		uint8_t channel, /* in: SerDes channel that will get tested; bitmap with CHAN(0), CHAN(1), CHAN(2), and CHAN(3) */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Generic SerDes PRBS testing function for all SerDes blocks in the SP2704 */
uint64_t /* Return: 64-bit mask signifying the error on a specific SerDes channel */
sp_SerdesPRBSChk(serdesTestParams_t *testParams);

/* Clear SerDes PRBS checker status */
void
sp_SerdesPrbsClearChkr(
		serdesInterface_t interface, /* in: SerDes interface to be in loopback */
		uint8_t channel); /* in: the checker whose status to clear; bitmap with CHAN(0), CHAN(1), CHAN(2), and CHAN(3) */

/* Get current PRBS checker status for a single channel */
serdesPrbsChkrStat_t /* Return: specific channel checker status snapshot */
sp_SerdesPrbsGetChkrStat(
		serdesInterface_t interface, /* in: SerDes interface whose checker status to retrieve */
		uint8_t single_channel); /* in: the specific channel whose checker status to retrieve */

/* Independently control PRBS generator */
int32_t /* Return: SUCCESS or ERROR */
sp_SerdesPrbsGenCtrl(
		serdesPrbsCtl_t ctrl, /* in: start or stop PRBS generator */
		serdesInterface_t interface, /* in: SerDes interface that will have its PRBS generator controlled */
		uint8_t channel, /* in: the channel whose generator will be controlled */
		serdesPrbsPoly_t polynomial, /* in: generator polynomial to use (if starting generator) */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* Independently control PRBS checker */
uint64_t /* Return: 64-bit mask signifying the error on a specific SerDes channel */
sp_SerdesPrbsChkrCtrl(
		serdesPrbsCtl_t ctrl, /* in: start or stop PRBS checker */
		serdesInterface_t interface, /* in: SerDes interface that will have its PRBS checker controlled */
		uint8_t channel, /* in: the channel whose checker will be controlled */
		serdesPrbsPoly_t polynomial, /* in: checker polynomial to use (if starting checker) */
		run4ever_t run4ever, /* in: 1 = have the checker run forever without any conditional stops; 0 = checker stops when the below numwords counts down to 0 */
		uint64_t numwords, /* in: if run4ever is not set, the checker will count down from numwords to 0 with each valid word received */
		uint8_t ckiInMHz); /* in: frequency of CKI in MHz, typically 25 or 50 */

/* A delay needs to be performed after starting a PRBS checker and before checking PCDONE */
int32_t /* Return: SUCCESS or ERROR */
sp_SerdesPrbsWaitPcDone(
		serdesRateCfg_t rate, /* in: SerDes data rate that is being tested */
		uint64_t numwords, /* in: number of words that will be counted down by the checker before setting PCDONE */
		uint8_t ckiInMHz); /* in: CKI frequency in MHz (ex. 20, 25, 50, etc) */

#endif /* LIBSERDES_H_ */

/******** History ********
$Log: libserdes.h,v $
Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/


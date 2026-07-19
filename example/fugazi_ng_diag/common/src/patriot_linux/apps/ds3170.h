/* $Id: ds3170.h,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: ds3170.c
 *
 * Description: defines for Framer chip Dallas 3170
 *
 *
 * Author: Sofian Teja
 * Copyright (c)2011-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */



#ifndef DS3170_H
#define DS3170_H

/*
 * Driver Header file for the Dallas Semiconductor DS3170 chips
 */

typedef volatile unsigned char VUC;
typedef unsigned char UC;

#define VUC_FIELD(a, b, c) (((a) & (0xff >> (8 - (c)))) << (b))

typedef struct {
    VUC l;
    VUC h;
} VUS;

typedef struct ds3170 { 

	struct _PORT {
/* Per Port register section */

	struct _GLOBAL {
/* Global Registers */
	VUS idr; 	/* 000 - 001 */
#define IDR_ADDR_L                     0x0000
#define IDR_ADDR_H                     0x0001

	VUS gcr1;	/* 002 - 003 */
#define GCR1_ADDR_L                    0x0002
#define GCR1_ADDR_H                    0x0003
#define GCR1_RST (1 << 0)
#define GCR1_RSTDP ( 1 << 1)
#define GCR1_LSBCRE ( 1 << 2)
#define GCR1_PMU ( 1 << 3)
#define GCR1_GPM(x) VUC_FIELD(x, 4, 2)
#define GCR1_MEIMS ( 1 << 6)
#define GCR1_TMEI ( 1 << 7)
#define GCR1_INTM ( 1 << 6)

	VUS gcr2;	/* 004 - 005 */
#define GCR2_ADDR_L                   0x0004
#define GCR2_ADDR_H                   0x0005
#define GCR2_CLAD(x) VUC_FIELD(x, 1, 3)
#define GCR2_G8KIS (1 << 0)
#define GCR2_G8KOS (1 << 1)
#define GCR2_G8KRS(x) VUC_FIELD(x, 2, 2)

	VUS hole1[2];	/* 006 - 009 */

	VUS giocr;	/* 00A - 00B */
#define GIOCR_ADDR_L                 0x000a
#define GIOCR_ADDR_H                 0x000b
#define GIOCR_GPIO1S(x) VUC_FIELD(x, 0, 2)
#define GIOCR_GPIO2S(x) VUC_FIELD(x, 2, 2)
#define GIOCR_GPIO3S(x) VUC_FIELD(x, 4, 2)
#define GIOCR_GPIO4S(x) VUC_FIELD(x, 6, 2)
#define GIOCR_GPIO5S(x) VUC_FIELD(x, 0, 2)
#define GIOCR_GPIO6S(x) VUC_FIELD(x, 2, 2)
#define GIOCR_GPIO7S(x) VUC_FIELD(x, 4, 2)
#define GIOCR_GPIO8S(x) VUC_FIELD(x, 6, 2)

	VUS hole2[2];	/* 00C - 00F */

	VUS isr;	/* 010 - 011 */
#define GL_ISR_ADDR_L                0x0010
#define GL_ISR_ADDR_H                0x0011
#define ISR_GSR	(1 << 0)
#define ISR_PISR (1 << 4)

	VUS isrie;	/* 012 - 013 */
#define ISRIE_ADDR_L                 0x0012
#define ISRIE_ADDR_H                 0x0013
#define ISRIE_GSRIE (1 << 0)
#define ISRIE_PISRIE (1 << 4)

	VUS sr;		/* 014 - 015 */
#define GL_SR_ADDR_L                 0x0014
#define GL_SR_ADDR_H                 0x0015
#define SR_GPMS (1 << 0)
#define SR_CLOL (1 << 1)

	VUS srl;	/* 016 - 017 */
#define GL_SRL_ADDR_L                0x0016
#define GL_SRL_ADDR_H                0x0017
#define SRL_GPMSL (1 << 0)
#define SRL_CLOLL (1 << 1)
#define SRL_ONESL (1 << 2)
#define SRL_CLADL (1 << 3)
#define SRL_8KREFL (1 << 4)

	VUS srie;	/* 018 - 019 */
#define GL_SRIE_ADDR_L               0x0018
#define GL_SRIE_ADDR_H               0x0019
#define SRIE_GPMSIE (1 << 0)
#define SRIE_CLOLIE (1 << 1)
#define SRIE_ONESIE (1 << 2)

	VUS hole3;	/* 01A - 01B */

	VUS giorr;	/* 01C - 01D */
#define GIORR_ADDR_L                 0x001c
#define GIORR_ADDR_H                 0x001d
#define GIORR_GPIO(x) VUC_FIELD(x, 0, 8)

	VUS hole1e;	/* 01E - 01F */

	} gl; /* Global Registers */

/*****************************************************************************/
	
	VUS hole4[8];	/* 020 - 02F */

/*****************************************************************************/

	VUS hole6[8];	/* 030 - 03F */

/*****************************************************************************/

	VUS cr1;	/* 040 - 041 */
#define CR1_ADDR_L                   0x0040
#define CR1_ADDR_H                   0x0041
#define CR1_RST (1 << 0)
#define CR1_RSTDP (1 << 1)
#define CR1_PD (1 << 2)
#define CR1_PMU (1 << 3)
#define CR1_PMUM (1 << 4)
#define CR1_MEIM (1 << 6)
#define CR1_TMEI (1 << 7)
#define CR1_BENA (1 << 1)
#define CR1_LAIS(x) VUC_FIELD(x, 2, 2) 
#define CR1_PAIS(x) VUC_FIELD(x, 4, 3)

	VUS cr2;	/* 042 - 043 */
#define CR2_ADDR_L                   0x0042
#define CR2_ADDR_H                   0x0043
#define CR2_LM(x) VUC_FIELD(x, 0, 3)
#define CR2_FM(x) VUC_FIELD(x, 3, 3)
#define CR2_TLBO (1 << 4)
#define CR2_RMON (1 << 5)
#define CR2_TTS (1 << 6)
#define CR2_TLEN (1 << 7)

	VUS cr3;	/* 044 - 045 */
#define CR3_ADDR_L                   0x0044
#define CR3_ADDR_H                   0x0045
#define CR3_TLTS (1 << 0)
#define CR3_TFTS (1 << 1)
#define CR3_RFTS (1 << 2)
#define CR3_CLADC (1 << 3)
#define CR3_LOOPT (1 << 4)
#define CR3_P8KREF (1 << 5)
#define CR3_P8KRS(x) VUC_FIELD(x, 6, 2)
#define CR3_TSOFOS (1 << 1)
#define CR3_TCLKS (1 << 2)
#define CR3_RSOFOS (1 << 4)
#define CR3_RCLKS (1 << 5)

	VUS cr4;	/* 046 - 047 */
#define CR4_ADDR_L                   0x0046
#define CR4_ADDR_H                   0x0047
#define CR4_GPIOA(x) VUC_FIELD(x, 0, 4)
#define CR4_GPIOB(x) VUC_FIELD(x, 4, 4)
#define CR4_LBM(x) VUC_FIELD(x, 0, 3)
	
	VUS hole7; /* 048 - 049 */

	VUS inv1;	/* 04A - 04B */
#define INV1_ADDR_L                  0x004a
#define INV1_ADDR_H                  0x004b
#define INV1_TCKII (1 << 0)
#define INV1_TCKOI (1 << 1)
#define INV1_TLCKI (1 << 2)
#define INV1_TDATI (1 << 3)
#define INV1_TNEGI (1 << 4)
#define INV1_TSOFII (1 << 5)
#define INV1_TOHCKI (1 << 6)
#define INV1_TOHI (1 << 7)
#define INV1_TOHEI (1 << 0)
#define INV1_TOHSI (1 << 1)
#define INV1_TSERI (1 << 2)
#define INV1_TSOFOI (1 << 4)

	VUS inv2;	/* 04C - 04D */
#define INV2_ADDR_L                  0x004c
#define INV2_ADDR_H                  0x004d
#define INV2_RCKOI (1 << 1)
#define INV2_RLCKI (1 << 2)
#define INV2_RPOSI (1 << 3)
#define INV2_RNEGI (1 << 4)
#define INV2_ROHCKI (1 << 6)
#define INV2_ROHI (1 << 7)
#define INV2_ROHSI (1 << 1)
#define INV2_RSERI ( 1<< 2)
#define INV2_RSOFSI (1 << 4)

	VUS hole8; /* 04E - 04F */

	VUS isr;	/* 050 - 051 */
#define PORT_ISR_ADDR_L              0x0050
#define PORT_ISR_ADDR_H              0x0051
#define ISR_FMSR (1 << 0)
#define ISR_BSR (1 << 4)
#define ISR_HSR (1 << 5)
#define ISR_FSR (1 << 6)
#define ISR_TTSR (1 << 7)
#define ISR_LCSR (1 << 0)
#define ISR_PSR (1 << 1)

	VUS sr;	/* 052 - 053 */
#define PORT_SR_ADDR_L               0x0052
#define PORT_SR_ADDR_H               0x0053
#define SR_PMS (1 << 0)
#define SR_RLOL (1 << 1)
#define SR_TDM (1 << 2)

	VUS srl;	/* 054 - 055 */
#define PORT_SRL_ADDR_L              0x0054
#define PORT_SRL_ADDR_H              0x0055
#define SRL_PMSL (1 << 0)
#define SRL_RLOLL (1 << 1)
#define SRL_TDML (1 << 2)
#define SRL_TCLKIL (1 << 6)
#define SRL_RLCLKL (1 << 7)

	VUS srie;	/* 056 - 057 */
#define PORT_SRIE_ADDR_L             0x0056
#define PORT_SRIE_ADDR_H             0x0057
#define SRIE_PMSIE ( 1 << 0)
#define SRIE_RLOLIE (1 << 1)
#define SRIE_TDMIE (1 << 2)

	VUS hole9[4]; /* 058 - 05F */


/*****************************************************************************/
	
	struct _BERT {
/* BERT Registers */

	VUS cr;	/* 060 - 061 */
#define BERT_CR_ADDR_L              0x0060
#define BERT_CR_ADDR_H              0x0061
#define BERT_CR_TPIC ( 1 << 0)
#define BERT_CR_TNPL (1 << 1)
#define BERT_CR_APRD (1 << 2)
#define BERT_CR_MPR (1 << 3)
#define BERT_CR_RPIC (1 << 4)
#define BERT_CR_RNPL (1 << 5)
#define BERT_CR_LPMU (1 << 6)
#define BERT_CR_PMUM (1 << 7)

	VUS pcr;	/* 062 - 063 */
#define BERT_PCR_ADDR_L              0x0062
#define BERT_PCR_ADDR_H              0x0063
#define BERT_PCR_PLF(x) VUC_FIELD(x, 0, 5)
#define BERT_PCR_PTS (1 << 5)
#define BERT_PCR_QRSS (1 << 6)
#define BERT_PCR_PTF(x) VUC_FIELD(x, 0, 5)

	VUS spr1;	/* 064 - 065 */
#define BERT_SPR1_ADDR_L             0x0064
#define BERT_SPR1_ADDR_H             0x0065

	VUS spr2;	/* 066 - 067 */
#define BERT_SPR2_ADDR_L             0x0066
#define BERT_SPR2_ADDR_H             0x0067

	VUS teicr;	/* 068 - 069 */
#define BERT_TEICR_ADDR_L            0x0068
#define BERT_TEICR_ADDR_H            0x0069
#define BERT_TEICR_MEIMS (1 << 0)
#define BERT_TEICR_TSEI (1 << 1)
#define BERT_TEICR_BEI (1 << 2)
#define BERT_TEICR_TEIR(x) VUC_FIELD(x, 3, 3)

	VUS hole10;	/* 06A - 06B */

	VUS sr;		/* 06C - 06D */
#define BERT_SR_ADDR_L               0x006c
#define BERT_SR_ADDR_H               0x006d
#define BERT_SR_OOS (1 << 0)
#define BERT_SR_BEC (1 << 1)
#define BERT_SR_PMS (1 << 3)

	VUS srl;	/* 06E - 06F */
#define BERT_SRL_ADDR_L              0x006e
#define BERT_SRL_ADDR_H	             0x006f
#define BERT_SRL_OOSL (1 << 0)
#define BERT_SRL_BECL (1 << 1)
#define BERT_SRL_BEL (1 << 2)
#define BERT_SRL_PMSL (1 << 3)

	VUS srie;	/* 070 - 071 */
#define BERT_SRIE_ADDR_L            0x0070
#define BERT_SRIE_ADDR_H            0x0071
#define BERT_SRIE_OOSIE (1 << 0)
#define BERT_SRIE_BECIE (1 << 1)
#define BERT_SRIE_BEIE (1 << 2)
#define BERT_SRIE_PMSIE (1 << 3)

	VUS hole11;	/* 072 - 073 */

	VUS rbecr1;	/* 074 - 075 */
#define BERT_RBECR1_ADDR_L           0x0074
#define BERT_RBECR1_ADDR_H           0x0075

	VUS rbecr2;	/* 076 - 077 */
#define BERT_RBECR2_ADDR_L           0x0076
#define BERT_RBECR2_ADDR_H           0x0077

	VUS rbcr1;	/* 078 - 079 */
#define BERT_RBCR1_ADDR_L            0x0078
#define BERT_RBCR1_ADDR_H            0x0079

	VUS rbcr2;	/* 07A - 07B */
#define BERT_RBCR2_ADDR_L            0x007a
#define BERT_RBCR2_ADDR_H            0x007b

	VUS hole12[2];	/* 07C - 07F */
	} bert;
	
/*****************************************************************************/

	struct _LINE {
/* Line Registers */

	VUS hole12b[6]; /* 080 - 08B */

	VUS tcr;	/* 08C - 08D */
#define LINE_TCR_ADDR_L              0x008c
#define LINE_TCR_ADDR_H              0x008d
#define LINE_TCR_MEIMS (1 << 0)
#define LINE_TCR_TSEI (1 << 1)
#define LINE_TCR_BPVI (1 << 2)
#define LINE_TCR_EXZI (1 << 3)
#define LINE_TCR_TZSD (1 << 4)

	VUS hole13;	/* 08E - 08F */

	VUS rcr;	/* 090 - 091 */
#define LINE_RCR_ADDR_L              0x0090
#define LINE_RCR_ADDR_H              0x0091
#define LINE_RCR_RZSD (1 << 0)
#define LINE_RCR_RZSF (1 << 1)
#define LINE_RCR_E3CVE (1 << 2)

	VUS hole14;	/* 092 - 093 */

	VUS rsr;	/* 094 - 095 */
#define LINE_RSR_ADDR_L              0x0094
#define LINE_RSR_ADDR_H              0x0095
#define LINE_RSR_LOS (1 << 0)
#define LINE_RSR_BPVC (1 << 1)
#define LINE_RSR_EXZC (1 << 3)

	VUS rsrl;	/* 096 - 097 */
#define LINE_RSRL_ADDR_L             0x0096
#define LINE_RSRL_ADDR_H             0x0097
#define LINE_RSRL_LOSL (1 << 0)
#define LINE_RSRL_BPVCL (1 << 1)
#define LINE_RSRL_BPVL (1 << 2)
#define LINE_RSRL_EXZCL (1 << 3)
#define LINE_RSRL_EXZL (1 << 4)
#define LINE_RSRL_ZSCDL (1 << 5)

	VUS rsrie;	/* 098 - 099 */
#define LINE_RSRIE_ADDR_L            0x0098
#define LINE_RSRIE_ADDR_H		     0x0099
#define LINE_RSRIE_LOSIE (1 << 0)
#define LINE_RSRIE_BPVCIE (1 << 1)
#define LINE_RSRIE_BPVIE (1 << 2)
#define LINE_RSRIE_EXZCIE (1 << 3)
#define LINE_RSRIE_EXZIE (1 << 4)
#define LINE_RSRIE_ZSCDIE (1 << 5)

	VUS hole15;	/* 09A - 09B */

	VUS rbpvcr;	/* 09C - 09D */
#define LINE_RBPVCR_ADDR_L           0x009c
#define LINE_RBPVCR_ADDR_H           0x009d

	VUS rexzcr;	/* 09E - 09F */
#define LINE_REXZCR_ADDR_L           0x009e
#define LINE_REXZCR_ADDR_H           0x009f
	} line;
	
	struct _HDLC {
/* HDLC Registers */

	VUS tcr;	/* 0A0 - 0A1 */
#define HDLC_TCR_ADDR_L              0x00a0
#define HDLC_TCR_ADDR_H	             0x00a1
#define HDLC_TCR_TFRST (1 << 0)
#define HDLC_TCR_TFPD (1 << 1)
#define HDLC_TCR_TDIE (1 << 2)
#define HDLC_TCR_TBRE (1 << 3)
#define HDLC_TCR_TIFV (1 << 4)
#define HDLC_TCR_TFEI (1 << 5)
#define HDLC_TCR_TPSD (1 << 6)
#define HDLC_TCR_TDAL(x) VUC_FIELD(x, 0, 5)

	VUS tfdr;	/* 0A2 - 0A3 */
#define HDLC_TFDR_ADDR_L             0x00a2
#define HDLC_TFDR_ADDR_H             0x00a3
#define HDLC_TFDR_TDPE (1 << 0)
#define HDLC_TFDR_TFD(x) VUC_FIELD(x, 0, 8)

	VUS tsr;	/* 0A4 - 0A5 */
#define HDLC_TSR_ADDR_L              0x00a4
#define HDLC_TSR_ADDR_H              0x00a5
#define HDLC_TSR_THDA (1 << 0)
#define HDLC_TSR_TFE (1 << 1)
#define HDLC_TSR_TFF (1 << 2)
#define HDLC_TSR_TFFL(x) VUC_FIELD(x, 0, 6)

	VUS tsrl;	/* 0A6 - 0A7 */
#define HDLC_TSRL_ADDR_L             0x00a6
#define HDLC_TSRL_ADDR_H             0x00a7
#define HDLC_TSRL_THDAL (1 << 0)
#define HDLC_TSRL_TFEL (1 << 1)
#define HDLC_TSRL_TPEL (1 << 3)
#define HDLC_TSRL_TFUL (1 << 4)
#define HDLC_TSRL_TFOL (1 << 5)

	VUS tsrie;	/* 0A8 - 0A9 */
#define HDLC_TSRIE_ADDR_L            0x00a8
#define HDLC_TSRIE_ADDR_H            0x00a9
#define HDLC_TSRIE_THDAIE (1 << 0)
#define HDLC_TSRIE_TFEIE (1 << 1)
#define HDLC_TSRIE_TPEIE (1 << 3)
#define HDLC_TSRIE_TFUIE (1 << 4)
#define HDLC_TSRIE_TFOIE (1 << 5)

	VUS hole16[3];	/* 0AA - 0AF */

	VUS rcr;	/* 0B0 - 0B1 */
#define HDLC_RCR_ADDR_L              0x00b0
#define HDLC_RCR_ADDR_H              0x00b1
#define HDLC_RCR_RFRST (1 << 0)
#define HDLC_RCR_RFPD (1 << 1)
#define HDLC_RCR_RDIE (1 << 2)
#define HDLC_RCR_RBRE (1 << 3)
#define HDLC_RCR_RDAL(x) VUC_FIELD(x, 0, 5)

	VUS hole17;	/* 0B2 - 0B3 */

	VUS rsr;	/* 0B4 0 0B5 */
#define HDLC_RSR_ADDR_L              0x00b4
#define HDLC_RSR_ADDR_H              0x00b5
#define HDLC_RSR_RHDA (1 << 0)
#define HDLC_RSR_RFE (1 << 1)
#define HDLC_RSR_RFF (1 << 2)

	VUS rsrl;	/* 0B6 - 0B7 */
#define HDLC_RSRL_ADDR_L             0x00b6
#define HDLC_RSRL_ADDR_H             0x00b7
#define HDLC_RSRL_RHDAL (1 << 0)
#define HDLC_RSRL_RFFL (1 << 2)
#define HDLC_RSRL_RPSL (1 << 3)
#define HDLC_RSRL_RPEL (1 << 4)
#define HDLC_RSRL_RFOL (1 << 7)

	VUS rsrie;	/* 0B8 - 0B9 */
#define HDLC_RSRIE_ADDR_L            0x00b8
#define HDLC_RSRIE_ADDR_H            0x00b9
#define HDLC_RSRIE_RHDAIE (1 << 0)
#define HDLC_RSRIE_RFFIE (1 << 2)
#define HDLC_RSRIE_RPSIE (1 << 3)
#define HDLC_RSRIE_RPEIE (1 << 4) 
#define HDLC_RSRIE_RFOIE (1 << 7)

	VUS hole18;	/* 0BA - 0BB */

	VUS rfdr;	/* 0BC - 0BD 8 */
#define HDLC_RFDR_ADDR_L             0x00bc
#define HDLC_RFDR_ADDR_H             0x00bd
#define HDLC_RFDR_RFDV (1 << 0)
#define HDLC_RFDR_RPS(x) VUC_FIELD(x, 1, 3)
#define HDLC_RFDR_RFD(x) VUC_FIELD(x, 0, 8)

	VUS hole19; /* 0BE - 0BF */
	} hdlc;
	
/*****************************************************************************/

	
	struct _FEAC {
/* FEAC Registers */

	VUS tcr;	/* 0C0 - 0C1 */
#define FEAC_TCR_ADDR_L              0x00c0
#define FEAC_TCR_ADDR_H              0x00c1
#define FEAC_TCR_TFS(x) VUC_FIELD(x, 0, 2)
#define FEAC_TCR_TFCL (1 << 2)

	VUS tfdr;	/* 0C2 - 0 C3 */
#define FEAC_TFDR_ADDR_L             0x00c2
#define FEAC_TFDR_ADDR_H             0x00c3
#define FEAC_TFDR_TFCA(x) VUC_FIELD (x, 0, 6)
#define FEAC_TFDR_TFCB(x) VUC_FIELD (x, 0, 6)

	VUS tsr;	/* 0C4 - 0C5 */
#define FEAC_TSR_ADDR_L              0x00c4
#define FEAC_TSR_ADDR_H              0x00c5
#define FEAC_TSR_TFI	(1 << 0)

	VUS tsrl;	/* 0C6 - 0C7 */
#define FEAC_TSRL_ADDR_L             0x00c6
#define FEAC_TSRL_ADDR_H             0x00c7
#define FEAC_TSRL_TFIL (1 << 0)

	VUS tsrie;	/* 0C8 - 0C9 */
#define FEAC_TSRIE_ADDR_L            0x00c8
#define FEAC_TSRIE_ADDR_H            0x00c9
#define FEAC_TSRIE_TFIIE (1 << 0)

	VUS hole20[3]; /* 0CA - 0CF */

	VUS rcr;	/* 0DO - 0D1 */
#define FEAC_RCR_ADDR_L              0x00d0
#define FEAC_RCR_ADDR_H              0x00d1
#define FEAC_RCR_RFR (1 << 0)

	VUS hole21;	/* 0D2 - 0D3 */

	VUS rsr;	/* 0D4 - 0D5 */
#define FEAC_RSR_ADDR_L              0x00d4
#define FEAC_RSR_ADDR_H	             0x00d5
#define FEAC_RSR_RFI (1 << 0)
#define FEAC_RSR_RFCD (1 << 1)
#define FEAC_RSR_RFFE (1 << 3)

	VUS rsrl;	/* 0D6 - 0D7 */
#define FEAC_RSRL_ADDR_L             0x00d6
#define FEAC_RSRL_ADDR_H             0x00d7
#define FEAC_RSRL_RFIL (1 << 0)
#define FEAC_RSRL_RFCDL (1 << 1)
#define FEAC_RSRL_RFFOL (1 << 2)

	VUS rsrie;	/* 0D8 - 0D9 */
#define FEAC_RSRIE_ADDR_L            0x00d8
#define FEAC_RSRIE_ADDR_H            0x00d9
#define FEAC_RSRIE_RFIIE (1 << 0)
#define FEAC_RSRIE_RFCDIE (1 << 1)
#define FEAC_RSRIE_RFFOIE (1 << 2)

	VUS hole22;	/* 0DA - 0DB */

	VUS rfdr;	/* 0DC - 0DD */
#define FEAC_RFDR_ADDR_L             0x00dc
#define FEAC_RFDR_ADDR_H             0x00dd
#define FEAC_RFDR_RFF(x) VUC_FIELD(x, 0, 6)
#define FEAC_RFDR_RFFI (1 << 7)

	VUS hole23;	/* 0DE - 0DF */
	} feac;


/*****************************************************************************/

	struct _TRAILTRACE {
/* Trail Trace Registers */

	VUS hole23b[4]; /* 0E0 - 0E7 */
	
	VUS tcr;	/* 0E8 - 0E9 */
#define TT_TCR_ADDR_L                0x00e8
#define TT_TCR_ADDR_H                0x00e9
#define TT_TCR_TBRE (1 << 0)
#define TT_TCR_TDIE (1 << 1)
#define TT_TCR_TIDLE (1 << 2)
#define TT_TCR_TMAD (1 << 3)

	VUS tiar;	/* 0EA - 0EB */
#define TT_TIAR_ADDR_L               0x00ea
#define TT_TIAR_ADDR_H               0x00eb
#define TT_TIAR_TTIA(x) VUC_FIELD(x, 0, 4)

	VUS tir;	/* 0EC - 0ED */
#define TT_TIR_ADDR_L                0x00ec
#define TT_TIR_ADDR_H                0x00ed
#define TT_TIR_TTD(x) VUC_FIELD(x, 0, 8)

	VUS hole24;	/* 0EE - 0EF */

	VUS rcr;	/* 0F0 - 0F1 */
#define TT_RCR_ADDR_L                0x00f0
#define TT_RCR_ADDR_H                0x00f1
#define TT_RCR_RBRE (1 << 0)
#define TT_RCR_RDIE (1 << 1)
#define TT_RCR_RETCD (1 << 2)
#define TT_RCR_RMAD (1 << 3)

	VUS riar;	/* 0F2 - 0F3 */
#define TT_RIAR_ADDR_L               0x00f2
#define TT_RIAR_ADDR_H               0x00f3
#define TT_RIAR_RTIA(x) VUC_FIELD(x, 0, 4)
#define TT_RIAR_ETIA(x) VUC_FIELD(x, 0, 4)

	VUS rsr;	/* 0F4 - 0F5 */
#define TT_RSR_ADDR_L                0x00f4
#define TT_RSR_ADDR_H                0x00f5
#define TT_RSR_RIDL (1 << 0)
#define TT_RSR_RTIU (1 << 1)
#define TT_RSR_RTIM (1 << 2)

	VUS rsrl;	/* OF6 - 0F7 */
#define TT_RSRL_ADDR_L               0x00f6
#define TT_RSRL_ADDR_H               0x00f7
#define TT_RSRL_RIDLL (1 << 0)
#define TT_RSRL_RTIUL (1 << 1)
#define TT_RSRL_RTIML (1 << 2)
#define TT_RSRL_RTICL (1 << 3)

	VUS rsrie;	/* 0F8 - 0F9 */
#define TT_RSRIE_ADDR_L              0x00f8
#define TT_RSRIE_ADDR_H              0x00f9
#define TT_RSRIE_RIDLIE (1 << 0)
#define TT_RSRIE_RTIUIE (1 << 1)
#define TT_RSRIE_RTIMIE (1 << 2)
#define TT_RSRIE_RTICIE (1 << 3)

	VUS hole25;	/* 0FA - 0FB */

	VUS rir;	/* 0FC - 0FD */
#define TT_RIR_ADDR_L                0x00fc
#define TT_RIR_ADDR_H                0x00fd
#define TT_RIR_RTD(x) VUC_FIELD(x, 0, 8)

	VUS eir;	/* 0FE - 0FF */
#define TT_EIR_ADDR_L                0x00fe
#define TT_EIR_ADDR_H                0x00ff
#define TT_EIR_ETD(x) VUC_FIELD(x, 0, 8)

	VUS hole26[12];	/* 100 - 117*/
	} tt;


/*****************************************************************************/
union _frm {
	struct _T3 {

/* T3 Registers */

	VUS tcr;	/* 118 - 119 */
#define T3_TCR_ADDR_L                0x0118
#define T3_TCR_ADDR_H                0x0119
#define T3_TCR_TAIS (1 << 0)
#define T3_TCR_TFGD (1 << 1)
#define T3_TCR_ARDID (1 << 2)
#define T3_TCR_TRDI (1 << 3)
#define T3_TCR_AFEBED (1 << 4)
#define T3_TCR_TFEBE (1 << 5)
#define T3_TCR_CBGE (1 << 2)
#define T3_TCR_TIDLE (1 << 3)
#define T3_TCR_PBGE (1 << 4)

	VUS teir;	/* 11A - 11B */
#define T3_TEIR_ADDR_L               0x011a
#define T3_TEIR_ADDR_H               0x011b
#define T3_TEIR_MEIMS (1 << 0)
#define T3_TEIR_TSEI (1 << 1)
#define T3_TEIR_FEI (1 << 2)
#define T3_TEIR_FEIC(x) VUC_FIELD(x, 3, 2)
#define T3_TEIR_PEI (1 << 5)
#define T3_TEIR_CPEIE (1 << 6)
#define T3_TEIR_FBEI (1 << 0)
#define T3_TEIR_CFBEIE (1 << 1)
#define T3_TEIR_CPEI (1 << 2)
#define T3_TEIR_CCPEIE (1 << 3)

	VUS hole27[2];	/* 11C - 11F */

	VUS rcr;	/* 120 - 121 */
#define T3_RCR_ADDR_L                0x0120
#define T3_RCR_ADDR_H                0x0121
#define T3_RCR_FRSYNC (1 << 0)
#define T3_RCR_LIP(x) VUC_FIELD(x, 1, 2)
#define T3_RCR_ROMD (1 << 3)
#define T3_RCR_RAIAD (1 << 4)
#define T3_RCR_RAIOD (1 << 5)
#define T3_RCR_RAILD (1 << 6)
#define T3_RCR_RAILE (1 << 7)
#define T3_RCR_FECC(x) VUC_FIELD(x, 0, 2)
#define T3_RCR_ECC (1 << 2)
#define T3_RCR_AAISD (1 << 3)
#define T3_RCR_MDAISI (1 << 4)
#define T3_RCR_MAOD (1 << 5)
#define T3_RCR_COVHD (1 << 6)

	VUS hole28;	/* 122 - 123 */

	VUS rsr1;	/* 124 - 125 */
#define T3_RSR1_ADDR_L               0x0124
#define T3_RSR1_ADDR_H               0x0125
#define T3_RSR1_LOS (1 << 0)
#define T3_RSR1_OOF (1 << 1)
#define T3_RSR1_AIS (1 << 2)
#define T3_RSR1_RAI (1 << 3)
#define T3_RSR1_LOF (1 << 4)
#define T3_RSR1_SEF (1 << 6)
#define T3_RSR1_OOMF (1 << 7)
#define T3_RSR1_RUA1 (1 << 0)
#define T3_RSR1_IDLE (1 << 1)
#define T3_RSR1_AIC (1 << 2)
#define T3_RSR1_T3FM (1 << 3)

	VUS rsr2;	/* 126 - 127 */
#define T3_RSR2_ADDR_L               0x0126
#define T3_RSR2_ADDR_H               0x0127
#define T3_RSR2_FEC (1 << 0)
#define T3_RSR2_PEC (1 << 1)
#define T3_RSR2_FBEC (1 << 2)
#define T3_RSR2_CPEC (1 << 3)

	VUS rsrl1;	/* 128 - 129 */
#define T3_RSRL1_ADDR_L              0x0128
#define T3_RSRL1_ADDR_H              0x0129
#define T3_RSRL1_LOSL (1 << 0)
#define T3_RSRL1_OOFL (1 << 1)
#define T3_RSRL1_AISL (1 << 2)
#define T3_RSRL1_RAIL (1 << 3)
#define T3_RSRL1_LOFL (1 << 4)
#define T3_RSRL1_COFAL (1 << 5)
#define T3_RSRL1_SEFL (1 << 6)
#define T3_RSRL1_OOMFL (1 << 7)
#define T3_RSRL1_RUAIL (1 << 0)
#define T3_RSRL1_IDLEL (1 << 1)
#define T3_RSRL1_AICL (1 << 2)
#define T3_RSRL1_T3FML (1 << 3)

	VUS rsrl2;	/* 12A - 12B */
#define T3_RSRL2_ADDR_L              0x012a
#define T3_RSRL2_ADDR_H              0x012b
#define T3_RSRL2_FECL (1 << 0)
#define T3_RSRL2_PECL (1 << 1)
#define T3_RSRL2_FBECL (1 << 2)
#define T3_RSRL2_CPECL (1 << 3)
#define T3_RSRL2_FEL (1 << 0)
#define T3_RSRL2_PEL (1 << 1)
#define T3_RSRL2_FBEL (1 << 2)
#define T3_RSRL2_CPEL (1 << 3)

	VUS rsrie1;	/* 12C - 12D */
#define T3_RSRIE1_ADDR_L             0x012c
#define T3_RSRIE1_ADDR_H             0x012d
#define T3_RSRIE1_LOSIE (1 << 0)
#define T3_RSRIE1_OOFIE (1 << 1)
#define T3_RSRIE1_AISIE (1 << 2)
#define T3_RSRIE1_RAIIE (1 << 3)
#define T3_RSRIE1_LOFIE (1 << 4)
#define T3_RSRIE1_COFAIE (1 << 5)
#define T3_RSRIE1_SEFIE (1 << 6)
#define T3_RSRIE1_OOMFIE (1 << 7)
#define T3_RSRIE1_RUA1IE (1 << 0)
#define T3_RSRIE1_IDLEIE (1 << 1)
#define T3_RSRIE1_AICIE ( 1 << 2)
#define T3_RSRIE1_T3FMIE (1 << 3)

	VUS rsrie2;	/* 12E - 12F */
#define T3_RSRIE2_ADDR_L             0x012e
#define T3_RSRIE2_ADDR_H             0x012f
#define T3_RSRIE2_FECIE (1 << 0)
#define T3_RSRIE2_PECIE (1 << 1)
#define T3_RSRIE2_FBECIE (1 << 2)
#define T3_RSRIE2_CPECIE (1 << 3)
#define T3_RSRIE2_FEIE (1 << 0)
#define T3_RSRIE2_PEIE (1 << 1)
#define T3_RSRIE2_FBEIE (1 << 2)
#define T3_RSRIE2_CPEIE (1 << 3)

	VUS hole29[2];	/* 130 - 133 */

	VUS rfecr;	/* 134 - 135 */
#define T3_RFECR_ADDR_L             0x0134
#define T3_RFECR_ADDR_H             0x0135

	VUS rpecr;	/* 136 - 137 */
#define T3_RPECR_ADDR_L             0x0136
#define T3_RPECR_ADDR_H             0x0137

	VUS rfbecr;	/* 138 - 139 */
#define T3_RFBECR_ADDR_L             0x0138
#define T3_RFBECR_ADDR_H             0x0139

	VUS rcpecr;	/* 13A - 13B */
#define T3_RCPECR_ADDR_L             0x013a
#define T3_RCPECR_ADDR_H             0x013b

	VUS hole29b[2];	/* 13C - 13F */
	} t3;

/*****************************************************************************/

	struct _E3G751 {
/* E3 G.751 Registers */

	VUS tcr;	/* 118 - 119 */
#define G751_TCR_ADDR_L              0x0118
#define G751_TCR_ADDR_H              0x0119
#define G751_TCR_TAIS (1 << 0)
#define G751_TCR_TFGD (1 << 1)
#define G751_TCR_TABC(x) VUC_FIELD(x, 2, 2)
#define G751_TCR_TNBC(x) VUC_FIELD(x, 0, 2)

	VUS teir;	/* 11A - 11B */
#define G751_TEIR_ADDR_L             0x011a
#define G751_TEIR_ADDR_H             0x011b
#define G751_TEIR_MEIMS (1 << 0)
#define G751_TEIR_TSEI (1 << 1)
#define G751_TEIR_FEI (1 << 2)
#define G751_TEIR_FEIC(x) VUC_FIELD(x, 3, 2)

	VUS hole30[2];	/* 11C - 11F */

	VUS rcr;	/* 120 - 121 */
#define G751_RCR_ADDR_L              0x0120
#define G751_RCR_ADDR_H              0x0121
#define G751_RCR_FRSYNC (1 << 0)
#define G751_RCR_LIP(x) VUC_FIELD(x, 1, 2)
#define G751_RCR_ROMD (1 << 3)
#define G751_RCR_RAIAD (1 << 4)
#define G751_RCR_RAIOD (1 << 5)
#define G751_RCR_RAILD (1 << 6)
#define G751_RCR_RAILE (1 << 7)
#define G751_RCR_FECC(x) VUC_FIELD(x, 0, 2)
#define G751_RCR_ECC (1 << 2)
#define G751_RCR_AAISD (1 << 3)
#define G751_RCR_MDAISI (1 << 4)
#define G751_RCR_DLS (1 << 5)

	VUS hole31;	/* 122 - 123 */

	VUS rsr1;	/* 124 - 125 */
#define G751_RSR1_ADDR_L             0x0124
#define G751_RSR1_ADDR_H             0x0125
#define G751_RSR1_LOS (1 << 0)
#define G751_RSR1_OOF (1 << 1)
#define G751_RSR1_AIS (1 << 2)
#define G751_RSR1_RAI (1 << 3)
#define G751_RSR1_LOF (1 << 4)
#define G751_RSR1_RNB (1 << 6)
#define G751_RSR1_RAB (1 << 7)
#define G751_RSR1_RUA1 (1 << 0)

	VUS rsr2;	/* 126 - 127 */
#define G751_RSR2_ADDR_L             0x0126
#define G751_RSR2_ADDR_H             0x0127
#define G751_RSR2_FEC (1 << 0)
	
	VUS rsrl1;	/* 128 - 129 */
#define G751_RSRL1_ADDR_L            0x0128
#define G751_RSRL1_ADDR_H            0x0129
#define G751_RSRL1_LOSL (1 << 0)
#define G751_RSRL1_OOFL (1 << 1)
#define G751_RSRL1_AISL (1 << 2)
#define G751_RSRL1_RAIL (1 << 3)
#define G751_RSRL1_LOFL (1 << 4)
#define G751_RSRL1_COFAL (1 << 5)
#define G751_RSRL1_NCL (1 << 6)
#define G751_RSRL1_ACL (1 << 7)
#define G751_RSRL1_RUA1L (1 << 0)

	VUS rsrl2;	/* 12A - 12B */
#define G751_RSRL2_ADDR_L            0x012a
#define G751_RSRL2_ADDR_H            0x012b
#define G751_RSRL2_FECL (1 << 0)
#define G751_RSRL2_FEL (1 << 0)

	VUS rsrie1;	/* 12C - 12D */
#define G751_RSRIE1_ADDR_L           0x012c
#define G751_RSRIE1_ADDR_H           0x012d
#define G751_RSRIE1_LOSIE (1 << 0)
#define G751_RSRIE1_OOFIE (1 << 1)
#define G751_RSRIE1_AISIE (1 << 2)
#define G751_RSRIE1_RAIIE (1 << 3)
#define G751_RSRIE1_LOFIE (1 << 4)
#define G751_RSRIE1_COFAIE (1 << 5)
#define G751_RSRIE1_NCIE (1 << 6)
#define G751_RSRIE1_ACIE (1 << 7)
#define G751_RSRIE1_RUA1IE (1 << 0)

	VUS rsrie2;	/* 12E - 12F */
#define G751_RSRIE2_ADDR_L           0x012e
#define G751_RSRIE2_ADDR_H           0x012f
#define G751_RSRIE2_FECIE (1 << 0)
#define G751_RSRIE2_FEIE (1 << 0)

	VUS hole32[2];	/* 130 - 133 */

	VUS rfecr;	/* 134 - 135 */
#define G751_RFECR_ADDR_L            0x0134
#define G751_RFECR_ADDR_H            0x0135

	VUS hole33[3];	/* 136 - 138 */

	VUS hole34[2];	/* 13C - 13F */
	} e3g751;

/*****************************************************************************/


	struct _E3G832 {
/* E3 G.832 Registers */
	
	VUS tcr;	/* 118 - 119 */
#define G832_TCR_ADDR_L              0x0118
#define G832_TCR_ADDR_H              0x0119
#define G832_TCR_TAIS (1 << 0)
#define G832_TCR_TFGD (1 << 1)
#define G832_TCR_ARDID (1 << 2)
#define G832_TCR_TRDI (1 << 3)
#define G832_TCR_AFEBED (1 << 4)
#define G832_TCR_TFEBE (1 << 5)
#define G832_TCR_TNRC(x) VUC_FIELD(x, 0, 2)
#define G832_TCR_TGCC (1 << 2)

	VUS teir;	/* 11A - 11B */
#define G832_TEIR_ADDR_L             0x011a
#define G832_TEIR_ADDR_H             0x011b
#define G832_TEIR_MEIMS (1 << 0)
#define G832_TEIR_TSEI (1 << 1)
#define G832_TEIR_FEI (1 << 2)
#define G832_TEIR_FEIC(x) VUC_FIELD(x, 3, 2)
#define G832_TEIR_PEI (1 << 5)
#define G832_TEIR_CPEIE (1 << 6)
#define G832_TEIR_PBEE (1 << 7)

	VUS tmabr;	/* 11C - 11D */
#define G832_TMABR_ADDR_L            0x011c
#define G832_TMABR_ADDR_H            0x011d
#define G832_TMABR_TTI(x) VUC_FIELD(x, 0, 4)
#define G832_TMABR_TTIGD (1 << 4)
#define G832_TMABR_TPT(x) VUC_FIELD(x, 5, 3)

	VUS tngbr;	/* 11E - 11F */
#define G832_TNGBR_ADDR_L            0x011e
#define G832_TNGBR_ADDR_H            0x011f
#define G832_TNGBR_TNR(x) VUC_FIELD(x, 0, 8)
#define G832_TNGBR_TGC(x) VUC_FIELD(x, 0, 8)

	VUS rcr;	/* 120 - 121 */
#define G832_RCR_ADDR_L              0x0120
#define G832_RCR_ADDR_H              0x0121
#define G832_RCR_FRSYNC (1 << 0)
#define G832_RCR_LIP(x) VUC_FIELD(x, 1, 2)
#define G832_RCR_ROMD (1 << 3)
#define G832_RCR_RAIAD (1 << 4)
#define G832_RCR_RAIOD (1 << 5)
#define G832_RCR_RAILD (1 << 6)
#define G832_RCR_RAILE (1 << 7)
#define G832_RCR_FECC(x) VUC_FIELD(x, 0, 2)
#define G832_RCR_ECC (1 << 2)
#define G832_RCR_AAISD (1 << 3)
#define G832_RCR_MDAISI (1 << 4)
#define G832_RCR_DLS (1 << 5)
#define G832_RCR_PEC (1 << 6)

	VUS rmacr;	/* 122 - 123 */
#define G832_RMACR_ADDR_L            0x0122
#define G832_RMACR_ADDR_H            0x0123
#define G832_RMACR_TIED (1 << 0)
#define G832_RMACR_EPT(x) VUC_FIELD(x, 1, 3)

	VUS rsr1;	/* 124 - 125 */
#define G832_RSR1_ADDR_L             0x0124
#define G832_RSR1_ADDR_H             0x0125
#define G832_RSR1_LOS (1 << 0)
#define G832_RSR1_OOF (1 << 1)
#define G832_RSR1_AIS (1 << 2)
#define G832_RSR1_RAI (1 << 3)
#define G832_RSR1_LOF (1 << 4)
#define G832_RSR1_RUA1 (1 << 0)
#define G832_RSR1_RPTM (1 << 3)
#define G832_RSR1_RPTU (1 << 4)

	VUS rsr2;	/* 126 - 127 */
#define G832_RSR2_ADDR_L             0x0126
#define G832_RSR2_ADDR_H             0x0127
#define G832_RSR2_FEC (1 << 0)
#define G832_RSR2_PEC (1 << 1)
#define G832_RSR2_FBEC (1 << 2)

	VUS rsrl1;	/* 128 - 129 */
#define G832_RSRL1_ADDR_L            0x0128
#define G832_RSRL1_ADDR_H            0x0129
#define G832_RSRL1_LOSL (1 << 0)
#define G832_RSRL1_OOFL (1 << 1)
#define G832_RSRL1_AISL (1 << 2)
#define G832_RSRL1_RAIL (1 << 3)
#define G832_RSRL1_LOFL (1 << 4)
#define G832_RSRL1_COFAL (1 << 5)
#define G832_RSRL1_NRL (1 << 6)
#define G832_RSRL1_GCL (1 << 7)
#define G832_RSRL1_RUA1L (1 << 0)
#define G832_RSRL1_RPTL (1 << 2)
#define G832_RSRL1_RPTML (1 << 3)
#define G832_RSRL1_RPTUL (1 << 4)
#define G832_RSRL1_TIL (1 << 5)

	VUS rsrl2;	/* 12A - 12B */
#define G832_RSRL2_ADDR_L            0x012a
#define G832_RSRL2_ADDR_H            0x012b
#define G832_RSRL2_FECL (1 << 0)
#define G832_RSRL2_PECL (1 << 1)
#define G832_RSRL2_FBECL (1 << 2)
#define G832_RSRL2_FEL (1 << 0)
#define G832_RSRL2_PEL (1 << 1)
#define G832_RSRL2_FBEL (1 << 2)

	VUS rsrie1;	/* 12C - 12D */
#define G832_RSRIE1_ADDR_L           0x012c
#define G832_RSRIE1_ADDR_H           0x012d
#define G832_RSRIE1_LOSIE (1 << 0)
#define G832_RSRIE1_OOFIE (1 << 1)
#define G832_RSRIE1_AISIE (1 << 2)
#define G832_RSRIE1_RAIIE (1 << 3)
#define G832_RSRIE1_LOFIE (1 << 4)
#define G832_RSRIE1_COFAIE (1 << 5)
#define G832_RSRIE1_NRIE (1 << 6)
#define G832_RSRIE1_GCIE (1 << 7)
#define G832_RSRIE1_RUA1IIE (1 << 0)
#define G832_RSRIE1_RPTIE ( 1 << 2)
#define G832_RSRIE1_RPTMIE (1 << 3)
#define G832_RSRIE1_RPTUIE (1 << 4)
#define G832_RSRIE1_TIIE (1 << 5)

	VUS rsrie2;	/* 12E - 12F */
#define G832_RSRIE2_ADDR_L           0x012e
#define G832_RSRIE2_ADDR_H           0x012f
#define G832_RSRIE2_FECIE (1 << 0)
#define G832_RSRIE2_PECIE (1 << 1)
#define G832_RSRIE2_FBECIE (1 << 2)
#define G832_RSRIE2_FEIE (1 << 0)
#define G832_RSRIE2_PEIE (1 << 1)
#define G832_RSRIE2_FBEIE (1 << 2)

	VUS rmabr;	/* 130 - 131 */
#define G832_RMABR_ADDR_L            0x0130
#define G832_RMABR_ADDR_H            0x0131
#define G832_RMABR_T1(x) VUC_FIELD(x, 0, 4)
#define G832_RMABR_RPT(x) VUC_FIELD(x, 4, 3)

	VUS rngbr;	/* 132 - 133 */
#define G832_RNGBR_ADDR_L            0x0132
#define G832_RNGBR_ADDR_H            0x0133
#define G832_RNGBR_RNR(x) VUC_FIELD(x, 0, 8)
#define G832_RNGBR_RGC(x) VUC_FIELD(x, 0, 8)

	VUS rfecr;	/* 134 - 135 */
#define G832_RFECR_ADDR_L            0x0134
#define G832_RFECR_ADDR_H            0x0135

	VUS rpecr;	/* 136 - 137 */
#define G832_RPECR_ADDR_L            0x0136
#define G832_RPECR_ADDR_H            0x0137

	VUS rfber;	/* 138 - 139 */
#define G832_RFBER_ADDR_L            0x0138
#define G832_RFBER_ADDR_H            0x0139

	VUS hole35;	/* 13A - 13B */

	VUS hole36[2];	/* 13C - 13F */
	} e3g832;

/*****************************************************************************/
} frm; /* UNION END */

	VUS hole43[8];	/* 140 - 14F */

	VUS hole45[8];	/* 150 - 15F */

	VUS hole46[8];	/* 160 - 16F */

	VUS hole44[8];	/* 170 - 17F */

	VUS hole47[8];	/* 180 - 18F */

	VUS hole49[8];	/* 190 - 19F */

	VUS hole51[8];	/* 1A0 - 1AF */

	VUS hole52[8];	/* 1B0 - 1BF */

	VUS hole53[8];	/* 1C0 - 1CF */

	VUS hole54[8]; 	/* 1D0 - 1DF */

	VUS hole55[8];	/* 1E0 - 1E8 */

	VUS hole61[8];	/* 1F0 - 1FF */

	} port;

} DEVICE;	/* DS3170  structure */

typedef DEVICE        ds3170_t;
typedef struct _PORT  FRAMER;

/*
 *	The Device version is the chip revision that is supported by this
 *	driver.
 *
 *	Device Version:
 *		Rev 0
 *
 *	Revision:
 *		xx/xx/xx - Initial Release
 *
 *	See CUSTOMIZE HERE for areas that may need to be customized for your
 *	platform.
 */

/* -------------------------------------------------------------------------- */
/*
 * Driver for the Dallas ds3170 DS3170 chip, global section
 */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!CUSTOMIZATION NOTE !!!!!!!!!!!!!!!!!!!!!!!!!! */
/*
 * The DS3170 chip contains PACKET and CELL processor capabilities, you may
 * wish to add code to manage these functions along side of what the NComm Inc.
 * driver provides. These instructions are provided to help in the process of
 * integrating your driver with the NComm driver.
 *
 * Some basic work has already been included
 * By defining DS3170DATAP, Indexes into the Interrupt handler and the 
 * Ticker handler are available as DRVR_IDX_DATAP
 *
 * Using this index, calls similar to those below can be made into the 
 * header code to start and stop interrupt processing and timers.
 *
 *	NCID_GHOOKTICKER(sptr->d, DRVR_IDX_DATAP, ticker, onoff,
 *			DATAPRegTicker, sptr, DATAPGlobalTickerHandler);
 *
 * 	NCID_GHOOKISR(sptr->d, DRVR_IDX_DATAP, set_clear,
 *                      Glob2DATAPhookISR, sptr, DATAP_handler);
 *
 * The DATARegTicker and Glob2DATAPhookISR calls will need to be provided to
 * call the OS to enable the actual timer or Interrupt.
 *
 * In this header is also provided a Global Reset routine to initialize the 
 * device. Care must be taken to ensure that if your driver is the first to
 * be run that the Global Reset is called with a call to NCID_GRESET(*device).
 *
 * Examples can be found in the T3 and E3 code as to the proper handling of
 * these features.
 *
 */

/* -------------------------------------------------------------------------- */
/* 
 * The following sections consists of defines for your platform as well
 * as how the DS3170 works in your system.
 * You need to review each of these sections.  All of the defines shown
 * here are what is required to operate this chip on one of NComm's
 * evaluation platforms.  Your platform may require different settings.
 * The places that will be different are dependent upon the hardware
 * design of your platform.  The design team responsible for your
 * hardware platform will likely have the answers to any customizations that
 * are required.
 *
 * NComm recommends that you add a pound include line at the end of these
 * sections which contains an #undef of the items that need to be changed and
 * a subsequent #define that provides a different value.  This pound include 
 * file would then become a record of what changed between the standard NComm
 * driver distribution and your platform.  
 * See the line pound include "customizations_for_NCI_CHIP_NAME.h"
 * below for where this should be done.
 *
 */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* Platform Customizations Section
 * This section defines how the chip and driver interact with other parts
 * of your hardware.
 */

#define EDGE_TRIG_INTRPT	0	/* 0 = level triggered interrupts */
					/* level triggered is recommended */
					/* 1 = edge triggered */

#define USER_DEFINED_HOOKINT_VALUE 0	/* User defined value */
					/* A value that will be passed to the */
					/* application when a HOOKINTERRUPT is*/
					/* made.  This value is not used by */
					/* TMS nor the driver, just passed */

	/* 
	 * MAX_DEVICES defines the number of devices (hardware chips) you have
	 * in your system.  If you have less than the number shown, change
	 * this to the lower number.  If you have more, you want to change this
	 * to the higher value. Since there is an interrupt service routine per 
	 * device, when you get a larger number than NCI_MAX_DEVICES,
	 * you need to add the additional interrupt service routines.  The
	 * code is set up to generate a #warning when this happens.
	 */
#define MAX_DEVICES 4

	/* The following define controls if the standard CLIB functions
	 * memset and memcpy are used by the driver or
	 * the driver uses its own local routine for these functions.
	 * By default, the standard CLIB functions are used.
	 */
#undef USE_OWN_DRV_MEMLIB

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* Chip Level CUSTOMIZATION Section
 * This set of defines control the global part of the driver.
 */

/* Global Control Register 1 */
#define INT_PIN_MODE	0	/* 0 Pin is High impedance when not active */
				/* 1 Pin drives high when not active */

#define DIRECT_STATUS	0	/* 0 Polled status mode UTOPIA and POS-PHY */
				/* 1 Direct Status mode	UTOPIA and POS-PHY */

#define GLOBAL_PERF_MODE 2	/* 0 Update using the PMU bit */
				/* 1 Update using the GPIO8 pin */
				/* 2 Update using the Internal One Second */

#define LATCH_CLEAR	0	/* 0 Latched status cleared on a write */
				/* 1 Latched status cleared on a read */

/* Global Control Register 2 */
#define SOURCE_8KHZ	1	/* 0 = None, 8KHZ disabled */
				/* 1 = CLAD output */
				/* 2 = 8KREF Selected by P8KRS */
				/* 3 = Undefined */
				/* 1XX = GPIO4 */

#define OUTPUT_SEL_8KHZ	0	/* 0 GPIO2 mode selected by GPIO2S */
				/* 1 GPIO2 is 8KREFO */

#define INPUT_SEL_8KHZ 0 	/* 0 GPIO4 mode selected by GPIO4S */
				/* 1 GPIO4 is the 8KREF input signal */

#define CLAD_IO_MODE	0	/* 0 - 15 see data sheet for chart */


/* PORT CR2 Register */

#define LIU_RMON_MODE 0		/* 0 Disable 20dB pre-amp */
				/* 1 Enable 20dB pre-amp */

#define TX_LIU_LBO	0	/* 0 Full amplitude signals */
				/* 1 approximate 225 feet of cable */

#define ATM_CELL_DELIN	0	/* 0 Six cells required */
				/* 1 Eight Cells required */

#define LINE_MODE_SEL	2	/* 0 LIU OFF, JA OFF */
				/* 1 LIU ON , JA OFF */
				/* 2 LIU ON , JA TX */
				/* 3 LIU ON , JA RX */

#define CELL_DELIN_DIS	0	/* 0 delineation is in the ATM processor */
				/* 1 delineation is in the PLCP framer */

#define POS_PHY_MODE	0	/* 0 Packet processing */
				/* 1 Cell processing */
				
/* PORT CR3 Register */

#define RX_CLK_OUTPUT_SEL 0	/* 0 Select RGCLK, RPOHCLK or drive low */
				/* 1 Select RCLKO pin function */

#define RX_SOF_SEL	 0	/* 0 Select RDEN  pin function*/
				/* 1 Select RSOFO pin function */

#define RX_PLCP_PORT_EN	0	/* 0 Disable RX PLCP/FRACTIONAL port pins */
				/* 1 Enable RX PLCP/FRACTIONAL port pins */

#define TX_CLK_OUTPUT_SEL 0	/* 0 Select TGCLK, TPOHCLK pin function */
				/* 1 Select TCLKO pin function */

#define TX_SOF_SEL	 0	/* 0 Select TDEN  pin function*/
				/* 1 Select TSOFO pin function */

#define TX_PLCP_PORT_EN	0	/* 0 Disable TX PLCP/FRACTIONAL port pins */
				/* 1 Enable TX PLCP/FRACTIONAL port pins */

#define SOURCE_SEL_8KHZ	0	/* 0 Port 0 */
				/* 1 Port 1 */
				/* 2 Port 2 */
				/* 3 Port 3 */

#define PORT_8K_REF	0	/* 0 Use Global Source */
				/* 1 Use 8K Ref from Port */

#define CLAD_TX_CLK_CTRL 0	/* 0 Use CLAD clocks as the transmit source */
				/* 1 Do not use CLAD clocks */

#define RX_FRM_IO_SEL	0	/* 0 Use output clocks as the reference */
				/* 1 Use input clocks as the reference */

#define TX_FRM_IO_SEL	0	/* 0 Use output clocks as the reference */
				/* 1 Use input clocks as the reference */

#define TX_LINE_IO_SEL	0	/* 0 Use output clocks as the reference */
				/* 1 Use input clocks as the reference */


/* Port Invert 1 */

#define TPDENO_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TPDAT_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TPOHSOF_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TPOHEN_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TPOH_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TOHSOF_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TOHEN_INVERT	1	/* 0 Don't Invert */
				/* 1 Invert */

#define TOH_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TOHCLK_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TSOFI_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TNEG_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TPOS_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TLCLK_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TCLKO_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define TCLKI_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */


/* Port Invert 2 */

#define RPDAT_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RFOHEN_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RPOHSOF_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RPOH_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define ROHSOF_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define ROH_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define ROHCLK_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RNEG_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RPOS_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RLCLK_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */

#define RLCLKO_INVERT	0	/* 0 Don't Invert */
				/* 1 Invert */



/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* T3 CUSTOMIZATION Section
 * This set of defines control the T3 part of the driver.
 */


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* E3 CUSTOMIZATION Section
 * This set of defines control the E3 part of the driver.
 */

#define GC_SOURCE	1	/* Source for the GC Byte 	*/
				/* 0 = transmit HDLC controller	*/
				/* 1 = GC/NR byte register	*/

#define GC_BYTE_DEFAULT	0xaa	/* Default value to set in GC register */

#define NR_SOURCE	3	/* Source for the NR Byte 	*/
				/* 0 = all ones			*/
				/* 1 = transmit HDLC controller	*/
				/* 2 = transmit FEAC controller	*/
				/* 3 = GC/NR byte register	*/

#define NR_BYTE_DEFAULT	0x55	/* Default value to set in GC register */


/* BERT Status Register */
#define BIT_ERROR_CNT_ZERO   0

/* -------------------------------------------------------------------------- */
/* CUSTOMIZE HERE
 * This is where you should put your pound include line to include your
 * own file to customize the drivers.  The file will want the form of:
 * #undef the name you want to change
 * #define the name to the value you need
 */

/* pound include "customizations_for_NCI_CHIP_NAME.h" */


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*          You should not need to edit anything below this point.            */
/*          You should not need to edit anything below this point.            */
/*          You should not need to edit anything below this point.            */
/*          You should not need to edit anything below this point.            */
/*          You should not need to edit anything below this point.            */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


#define MAX_T3_CHANNELS 4
#define MAX_E3_CHANNELS 4


/* -------------------------------------------------------------------------- */

#define MAX_GLOBAL_TICKERS 0

/*
 * MAX_GLOBAL_TICKERS defines the number of global ticker routines that are 
 * shared between each of the driver technologies.
 */


/* -------------------------------------------------------------------------- */
/* 
 * This section has some misc. defines 
 */

#ifndef NULL
	#define NULL ((void *)(0))
#endif

/* -------------------------------------------------------------------------- */
/*
 * This section contains the defines which are used by the different driver
 * files.
 */

/* Function Entry Points */
#define NCID_GDEVICEMAP _ds3170_GlobalDeviceMap
#define NCID_GDEVICEUNMAP _ds3170_GlobalDeviceUnMap
#define NCID_GRESET _ds3170_GlobalReset
#define NCID_GUNRESET _ds3170_GlobalUnReset
#define NCID_GHOOKISR _ds3170_GlobalHookIsr
#define NCID_GHOOKTICKER _ds3170_GlobalHookTicker
#define NCID_GTICKER _ds3170_GlobalTicker


/* -------------------------------------------------------------------------- */
/* 
 * This section defines the top level device structure for the device
 */

typedef enum _ds3170_tech_index {
#ifdef DS3170TE3	
	DRVR_IDX_TE3,
#endif	
#ifdef DS3170DATAP
	DRVR_IDX_DATAP,
#endif
	DRVR_IDX_END
} NCID_TI; 

struct _ds3170_global {
	int onesec_cnt;
	unsigned long t3map;		/* Controls T3 allocation */
	unsigned long e3map;		/* Controls E3 allocation */
};

typedef struct _ds3170_Device {
	int init;			/* When 0, chip is not Mapped */
					/* When 1, chip is Mapped */
	int id;				/* Chip ID number */
	int mapCnt;			/* Number times chip has been mapped */
	int resetCnt;			/* Number of times chip has Global */
					/* reset called */
	void *BaseAdr;			/* Address of the chip */

					/* define the interrupt entry points */
	void *tech_handlers[DRVR_IDX_END];
	int isr_cnt;
	void *device_isr;		/* The actual ISR */

#if MAX_GLOBAL_TICKERS
	struct _tickers {
		int cnt;		/* number of times ticker allocated */
		void *handle;		/* Ticker handle */
		void *tickerHandler[DRVR_IDX_END];
	} tickers[MAX_GLOBAL_TICKERS];
#endif /* MAX_GLOBAL_TICKERS */

	struct _ds3170_global chip;
} GDEVICE;

/* 
 * define the global level functions.  These functions may be called
 * from each driver component.
 */

extern int NCID_GDEVICEUNMAP(GDEVICE *d);
extern GDEVICE *NCID_GDEVICEMAP(void *baseaddr);
extern void NCID_GRESET(GDEVICE *d);
extern void NCID_GUNRESET(GDEVICE *d);
extern void NCID_GHOOKISR(GDEVICE *d, NCID_TI ti, int onoff,
	void *fptr, void *LnPtr, void *isr);
extern void NCID_GHOOKTICKER(GDEVICE *d, NCID_TI ti, int ticker, int onoff,
	void *fptr, void *LnPtr, void *tech_handler);
extern void NCID_GRESOURCE(int res, int onoff,
	void *LnPtr, int fc, void *isr, int line);




#ifdef USE_OWN_DRV_MEMLIB
/*------------------------------------------------------------------------*/
/* Define the driver specific memset call so that we have a self
 * contained driver.
 */
static void drvMemset(void *p, unsigned char c, unsigned int len)
{

unsigned char *ucp;
unsigned int i;

	for(i = 0, ucp = (unsigned char *)(p) ; i < len ; i++) 
		*ucp++ = c;
}

/*------------------------------------------------------------------------*/
/* Define the driver specific memcpy call so that we have a self
 * contained driver.
 */
static void drvMemcpy(void *dest, void *src, unsigned int len)
{

unsigned char *sp = (unsigned char *)src;
unsigned char *dp = (unsigned char *)dest;
unsigned int i;

	for(i = 0; i < len ; i++) 
		*dp++ = *sp++;

}
#else

	#define drvMemset memset
	#define drvMemcpy memcpy

#endif	/* DRV_MEMLIB */


#if (defined(TE3_DRIVER) && defined(DS3170TE3))



/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* 
 * Define the data structures that are required for the device.
 */

static GDEVICE gdevice[MAX_DEVICES];
static int gdevice_init = 0;



/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* 
 * The Device Specific Routines are defined below.
 */


/* -------------------------------------------------------------------------- */
/* The _device_GlobalReset implements the device specific reset routine.
 * This routine gets call once when the first time any technology issues its
 * reset call.
 *
 * This routine will be called once, the firs time that a device is reset.
 * This routine should issue a soft reset to the device if it 
 * is capable, then initialize all the hardware register to the value it wants
 * to be for the driver.  The setup once and forget about hardware
 * registers should be done here.
 * It IS possible that this routine will be called a 2nd time if a device
 * is not mapped at all, then a new global reset will call this a 2nd time.
 * If the device is completely un-reset, this will get called also.
 */
static int _device_GlobalReset(GDEVICE *d)
{
int retval = 1;
int i;
VUC dummy;
DEVICE *f = (DEVICE *)(d->BaseAdr);

	f->port.gl.gcr1.l = GCR1_RST;
	/* must delay for a min of 100 ms */
	for (i = 0; i <= 50; i++)
		dummy = f->port.gl.idr.l;
	
	f->port.gl.gcr1.l = 0;

	f->port.gl.gcr1.l = (GCR1_GPM(GLOBAL_PERF_MODE)) |
				(LATCH_CLEAR ? GCR1_LSBCRE : 0);

	f->port.gl.gcr1.h = (INT_PIN_MODE ? GCR1_INTM : 0) ;

	f->port.gl.gcr2.h = (GCR2_G8KRS(SOURCE_8KHZ)) |
				(OUTPUT_SEL_8KHZ ? GCR2_G8KOS : 0) |
				(INPUT_SEL_8KHZ ? GCR2_G8KIS : 0);
	
	f->port.gl.gcr2.l = GCR2_CLAD(CLAD_IO_MODE);

	f->port.gl.isrie.l = ISRIE_PISRIE | ISRIE_GSRIE;	

	return(retval);
}


/* -------------------------------------------------------------------------- */
/* The global unreset should put the device in the unreset state.
 * This state is up to your platform.  By default, we will just call the
 * reset routine.
 */
static int _device_GlobalUNReset(GDEVICE *d)
{
int retval = 1;

	_device_GlobalReset(d);


	return(retval);
}

/* -------------------------------------------------------------------------- */
/* This routine hooks and unhooks the ISR for the chip.  It can be called from
 * any technology driver.  The first one here, will actually be the
 * driver that hooks up the ISR via the technology CLBK API call.
 * The last one will cause the ISR to be un-hooked.
 */
void NCID_GHOOKISR(GDEVICE *d, NCID_TI ti, int onoff,
	void *fPtr, void *LnPtr, void *isr)
{
void (*fooptr)(void *, void *, int) = fPtr;

	if(onoff) {
		d->isr_cnt++;
		d->tech_handlers[ti] = isr;

		if(d->isr_cnt == 1) {
			(*fooptr)(LnPtr, d->device_isr, 1);
		} 
	}
	else {
		if(d->isr_cnt == 1) {
			(*fooptr)(LnPtr, d->device_isr, 0);
		} 

		d->isr_cnt--;
	}
}

#if (MAX_GLOBAL_TICKERS	> 0)
/* -------------------------------------------------------------------------- */
/* This routine translates the ticker into a number of ms required
 */
static int get_ms(int ticker)
{
int ms = 0;
	switch(ticker) {
	default:
		break;
	}
	return(ms);
}

/* -------------------------------------------------------------------------- */
/* This routine get called first on each global ticker before the technology
 * handlers get called
 */

static void GlobalTickerHandler(GDEVICE *d, int ticker)
{

	switch(ticker) {
	default:
		break;
	}

	return;
}


/* -------------------------------------------------------------------------- */
/* This is the OSTickerhandler common to all devices of this type.  
 * This routine will call the OSTicker service routines in each of 
 * the driver files 
 */
static void GlobalTickerProc(void *refptr, GDEVICE *d, int ticker)
{
int i;
void (*fooptr)(void *, int);

	refptr = refptr;	/* trash compiler warning */

	GlobalTickerHandler(d, ticker);

	for(i = 0; i < DRVR_IDX_END; i++) {

		fooptr = d->tickers[ticker].tickerHandler[i];

		if(fooptr != NULL)
		   (*fooptr)(d, ticker);
	}
}

/* -------------------------------------------------------------------------- */
/* This routine gets call by each technology driver to request starting 
 * and unstarting of a global OS Ticker.  
 * The variable "ticker" is the id number and has to
 * be in the range [0,MAX_NCI_TICKERS)
 * The first technology to request the global ticker will start the ticker
 * going.  The last technology to free the ticker will cause the ticker to
 * be freed.
 */
void NCID_GHOOKTICKER(GDEVICE *d, NCID_TI ti, int ticker, int onoff,
		void *fPtr, void *LnPtr, void *tech_handler)
{
struct _tickers *p;
int ms;
void (*fooptr)(void *, void *, int, int, int, void *) = fPtr;

	p = &d->tickers[ticker];
	ms = get_ms(ticker);

	if(onoff) {
		p->cnt++;
		p->tickerHandler[ti] = tech_handler;

		if(p->cnt == 1) {
			(*fooptr)(LnPtr, GlobalTickerProc, onoff, ticker, 
				ms, &p->handle);
		} 
	}
	else {
		if(p->cnt == 1) {
			(*fooptr)(LnPtr, GlobalTickerProc, onoff, ticker,
				ms, &p->handle);
			p->handle = NULL;
		} 

		p->cnt--;
	}
}
#endif 	/* MAX_GLOBAL_TICKERS */

/* -------------------------------------------------------------------------- */
/* This routine gets called before any of the technology handlers get called.
 * This is where you would read status registers that are done on a global
 * basis.
 */
static int GlobalISRHandler(GDEVICE *d)
{
int check_again = 0;

	/* !!! PUT any chip level stuff here !!! */
	return(check_again);
}

/* -------------------------------------------------------------------------- */
/* This is the interrupt handler common to all devices of this type.  
 * This routine will call the interrupt services routines in each of 
 * the driver files 
 */
static void _device_handler(int device)
{
GDEVICE *d = &gdevice[device];
int check_again;
int i;
int (*fooptr)(void *);


	do {
		check_again = GlobalISRHandler(d);

		for(i = 0; i < DRVR_IDX_END; i++) {

			fooptr = d->tech_handlers[i];
		
			if(fooptr != NULL)
				check_again |= (*fooptr)(d);
		}

	} while (EDGE_TRIG_INTRPT && check_again);

}


/* -------------------------------------------------------------------------- */
/* This is interrupt services routines per each device.
 * These routines call the main routine with an -1 of the device which
 * is interrupting.
 */

#if MAX_DEVICES > 0
static void _device0_handler(void)
{
        _device_handler(0);
}
#endif

#if MAX_DEVICES > 1
static void _device1_handler(void)
{
        _device_handler(1);
}
#endif

#if MAX_DEVICES > 2
static void _device2_handler(void)
{
        _device_handler(2);
}
#endif

#if MAX_DEVICES > 3
static void _device3_handler(void)
{
        _device_handler(3);
}
#endif

#if MAX_DEVICES > 4
static void _device4_handler(void)
{
        _device_handler(4);
}
#endif

#if MAX_DEVICES > 5
static void _device5_handler(void)
{
        _device_handler(5);
}
#endif

#if MAX_DEVICES > 6
static void _device6_handler(void)
{
        _device_handler(6);
}
#endif

#if MAX_DEVICES > 7
static void _device7_handler(void)
{
        _device_handler(7);
}
#endif

#if MAX_DEVICES > 8
static void _device8_handler(void)
{
        _device_handler(8);
}
#endif

#if MAX_DEVICES > 9
static void _device9_handler(void)
{
        _device_handler(9);
}
#endif

#if MAX_DEVICES > 10
static void _device10_handler(void)
{
        _device_handler(10);
}
#endif

#if MAX_DEVICES > 11
static void _device11_handler(void)
{
        _device_handler(11);
}
#endif

#if MAX_DEVICES > 12
static void _device12_handler(void)
{
        _device_handler(12);
}
#endif

#if MAX_DEVICES > 13
static void _device13_handler(void)
{
        _device_handler(13);
}
#endif

#if MAX_DEVICES > 14
static void _device14_handler(void)
{
        _device_handler(14);
}
#endif

#if MAX_DEVICES > 15
static void _device15_handler(void)
{
        _device_handler(15);
}
#endif

/*------------------------------------------------------------------------*/
/* This routine maps a line to a particular device and channel, and initializes
 * any data records required to manage interrupts for that particular line.
 * Returns NULL if errored, else returns a pointer to the device.
 * This routine gets call from all technology drivers
 */
GDEVICE *NCID_GDEVICEMAP(void *baseadr)
{
int i;
GDEVICE *d = &gdevice[0];

	/* if the first is uninitialized then nothing is initialized
	 */
	if (!gdevice_init) {
		gdevice_init = 1;

		drvMemset(d, 0, sizeof(gdevice));

		/* record info for when we hook interrupts during the INIT call
		 */
		gdevice[0].device_isr = _device0_handler;

#if MAX_DEVICES > 1
		gdevice[1].device_isr = _device1_handler;
#endif
#if MAX_DEVICES > 2
		gdevice[2].device_isr = _device2_handler;
#endif
#if MAX_DEVICES > 3
		gdevice[3].device_isr = _device3_handler;
#endif
#if MAX_DEVICES > 4
		gdevice[4].device_isr = _device4_handler;
#endif
#if MAX_DEVICES > 5
		gdevice[5].device_isr = _device5_handler;
#endif
#if MAX_DEVICES > 6
		gdevice[6].device_isr = _device6_handler;
#endif
#if MAX_DEVICES > 7
		gdevice[7].device_isr = _device7_handler;
#endif
#if MAX_DEVICES > 8
		gdevice[8].device_isr = _device8_handler;
#endif
#if MAX_DEVICES > 9
		gdevice[9].device_isr = _device9_handler;
#endif
#if MAX_DEVICES > 10
		gdevice[10].device_isr = _device10_handler;
#endif
#if MAX_DEVICES > 11
		gdevice[11].device_isr = _device11_handler;
#endif
#if MAX_DEVICES > 12
		gdevice[12].device_isr = _device12_handler;
#endif
#if MAX_DEVICES > 13
		gdevice[13].device_isr = _device13_handler;
#endif
#if MAX_DEVICES > 14
		gdevice[14].device_isr = _device14_handler;
#endif
#if MAX_DEVICES > 15
		gdevice[15].device_isr = _device15_handler;
#endif

		for (d = gdevice, i = 0; i < MAX_DEVICES; i++, d++) {
			d->id = i;
		}
	}


	/* First, See if device is already allocated
	 * This is done by the base address of the chip
	 */
	for (d = gdevice, i = 0; i < MAX_DEVICES; i++, d++) {

		if (d->init && (d->BaseAdr == baseadr)) {
			d->mapCnt++;
			return(d);
		}
	}

	/* OK, this is a new chip, find a location and allocate it 
	 */
	for (d = gdevice, i = 0; i < MAX_DEVICES; i++, d++) {
		/* init the device information
		 */
		if (!d->init) {
			d->init = 1;
			d->BaseAdr = baseadr;
			d->mapCnt++;
			return(d);
		}
	}

	/* No room at the inn, so return null 
	 */
	return(NULL);
}


/*------------------------------------------------------------------------*/
/* This routine performs the unmap function and marks the
 * entire device when all mapping has been cleared 
 * This routine gets call from all technology drivers
 */
int NCID_GDEVICEUNMAP(GDEVICE *d)
{

	if(d->mapCnt > 0)
		d->mapCnt--;

	if(d->mapCnt == 0) {
		d->init = 0;
		d->BaseAdr = NULL;
	}

	return(1);
}

/*------------------------------------------------------------------------*/
/* This routine performs the general reset duties, but nothing chip
 * specific.  It calls the routine _device_GlobalReset to perform
 * the chip specific duties.
 */

void NCID_GRESET(GDEVICE *d)
{

unsigned long lval;

	INTS_OFF(lval);
	if(!d->resetCnt) {

		_device_GlobalReset(d);

	}
	d->resetCnt++;
	INTS_ON(lval);
}

/*------------------------------------------------------------------------*/
/* This routine performs the general UNReset duties, but nothing chip
 * specific.  It calls the routine _device_GlobalUNReset to perform
 * the chip specific duties.
 */

void NCID_GUNRESET(GDEVICE *d)
{

unsigned long lval;

	INTS_OFF(lval);
	if(d->resetCnt == 1) {

		_device_GlobalUNReset(d);

	}
	d->resetCnt--;
	INTS_ON(lval);
}

#endif /* Build for Specific Driver */

#define DS3170_WRSR        0x01
#define DS3170_READ        0x80
#define DS3170_WRITE       0x00
#define DS3170_RD_CMD_LEN  0x02
#define DS3170_WR_CMD_LEN  0x03
#define DS3170_RPLY_LEN    0x03
#define DS3170_RDSR        0x05
#define DS3170_WREN        0x06
#define INVALID_DATA       0xFF
#define MAXIM_RDSR_RDY     0x01
#define MAX_DS3170_SPIN    1000

#define DS3170_GLOBAL  0x01
#define DS3170_PORT    0x02
#define DS3170_BERT    0x03
#define DS3170_LINE    0x04
#define DS3170_HDLC    0x05
#define DS3170_FEAC    0x06
#define DS3170_TT      0x07
#define DS3170_T3      0x08
#define DS3170_E3G751  0x09
#define DS3170_E3G832  0x0a

#define MASK_BITS10    0x000000FF
#define MASK_BITS32    0x0000FF00
#define MASK_BITS54    0x00FF0000
#define MASK_BITS76    0xFF000000


/* Global extern */
extern int ds3170_init_clear_te3(uint);
extern int ds3170_register_test(void);
extern int ds3170_read_if(unsigned char, unsigned char, unsigned char *, int);
extern int ds3170_write_if(unsigned char, unsigned char, unsigned char, int);
extern int ds3170_access_spi(int, unsigned char *, unsigned char *, int, int);
extern int ds3170_read_if_status(int cs);
extern int ds3170_read(unsigned char *, uint);
extern int ds3170_write(unsigned char, uint);


#endif /* DS3170H */


/*------------------------------------------------------------------------------
 * $Log: ds3170.h,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.5  2012/03/13 13:36:38  steja
 * Modified uchar to unsigned char
 *
 * Revision 1.1.4.4  2011/10/11 01:51:29  steja
 * Update DS3170 Register test code
 *
 * Revision 1.1.4.3  2011/09/20 10:10:56  steja
 * Update DS3170 code for AIS and BERT register
 *
 * Revision 1.1.4.2  2011/08/18 19:43:22  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.13  2011/08/16 17:57:59  huanngo
 * Fix bugs for SPI EEPROM
 *
 * Revision 1.1.2.12  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.11  2011/07/08 10:38:18  steja
 * Clean up code
 *
 * Revision 1.1.2.10  2011/07/07 16:21:54  steja
 * 1. Clean up code
 * 2. Add check statur register after loopback test for DS3170.
 *
 * Revision 1.1.2.9  2011/06/29 16:24:55  steja
 * Update DS3170 code.
 *
 * Revision 1.1.2.8  2011/06/27 14:14:06  steja
 * 1. Update FPGA register test function
 * 2. Add FPGA dump register function
 * 3. Add FPGA register read / write utility function
 * 4. Add FPGA initialization function
 *
 * Revision 1.1.2.7  2011/06/13 06:48:56  steja
 * Update code for DS3170 and FPGA
 *
 * Revision 1.1.2.6  2011/06/09 07:03:37  steja
 * Update the code for DS3170 and FPGA's Patriot
 *
 * Revision 1.1.2.5  2011/05/26 03:29:16  steja
 * Fix the checkin log
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

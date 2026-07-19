/* $Id: cookie_4.h,v 1.41 2021/06/02 02:56:20 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/cookie_4.h,v $
 *------------------------------------------------------------------
 * cookie_4.h
 *
 * Jun 1998, Alan Hsu 
 *
 * Copyright (c) 2011 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __COOKIE_4_
#define __COOKIE_4_

#define NEED_NEW_COOKIE_FORMAT  0xfc
#define PARSING_N_SETUP_PASSED  0xfd
#define PARSING_N_SETUP_FAILED  0xfe

#define CURRENT_FORMAT_VERSION  4
#define FORMAT_1_VERSION        1

#define SIX_SLOTS		0x06

#define NON_SPARE               0
#define SPARE                   1

#define TYPE_LENGTH_MASK        0x3f
#define TYPE_SIZE_MASK          0xc0
#define TYPE_LENGTH_FOLLOW      0xc0
#define DYNAMIC_LENGTH_BASE     0x80


/* dynamic display format (can be altered, depending what bits are set in the
   length field */
#define HEX_FORMAT              0x00 /* dynamic dec format */
#define DEC_FORMAT              0x40 /* dynamic hex format */
#define ALP_FORMAT              0x80 /* dynamic alphanumberic format */

#define DLEN                    0x01 /* indicates variable length field*/
#define DFMT                    0x02 /* indicates alterable display format */

#define F_TYPE                  0xf0 /* indicates '0xFx' type field */
#define C_TYPE                  0xc0 /* indicates '0xCx' type field */

#define LAST_BYTE               0xff
#define EXTENSION_BYTE          0x00
#define NONE                    0x00
#define COOKIE_SIZE_512         512    /* bytes */
#define COOKIE_SIZE_256         256    /* bytes */
#define COOKIE_SIZE_128         128    /* bytes */
#define COOKIE_SIZE_32          32     /* bytes */

#define UPPER_4_BITS            0xf0
#define LOWER_4_BITS            0x0f

#define NUMBER_OF_SLOTS		0x01
#define CPU_TYPE		0x09
#define FORMAT_VERSION_TYPE     0x3f
#define COMPATIBILITY_TYPE      0x3e
#define CONTROLLER_TYPE         0x40
#define BOARD_REV               0x42
#define HARDWARE_REV            0x41
#define PART_NUM_73             0x82
#define VERSION_ID              0x89
#define PCB_SERIAL_NUM		0xc1
#define CHASSIS_SERIAL_NUM      0xc2
#define CHASSIS_MAC_TYPE        0xc3
#define DEVICE_VALUES           0xc9
#define PRODUCT_ID              0xcb
#define ASSET_ID_TYPE           0xcc
#define BOARD_MAC_ADDR          0xcf
#define LONG_CALI_TYPE          0xd3

#define WRITE_EEPROM            0
#define SHOW_COOKIE_INGREDIENT  1
#define WRITE_COOKIE_FROM_FILE  2

#define DIG_SIG_LIST            0xD9

#define VERSION_ID_SIZE          15
#define PCB_SERIAL_NUM_SIZE      14
#define PRODUCT_ID_SIZE         128
#define ASSET_ID_FIELD_SIZE       5
#define DEVICE_VALUES_SIZE       64     /* based on EDCS-1020001 */

/* 
 * mother board IDs (in ID ASCENDING order)
 */
#define REDBARON_SOPWITH_ID     0x042A  /* Sopwith Main Board */
#define REDBARON_CURTISS_BP_ID  0x042B  /* Curtiss Backplane  */
#define REDBARON_CURTISS_ID     0x042C  /* Curtiss Main Board */
#define SWOOP_FT_ID		0x06DA  /* Swoop Flying Tiger Board */

/*
 * NMs IDs (in ID ASCENDING order)
 */
#define NM_4E_ID                0x0042
#define NM_1V                   0x0064  /* DSP/Voice NM 1V 2 DSP VPM */
#define NM_2V                   0x0065  /* DSP/Voice NM 2V 4 DSP VPM */
#define NM_1A_OC3MM_1V          0x009B  /* OC3/STM-1 with CES ATM NM */
#define NM_1A_OC3SMI_1V         0x009D  /* OC3/STM-1 with CES ATM NM */
#define NM_1A_OC3SML_1V         0x009F  /* OC3/STM-1 with CES ATM NM */
#define NM_1HSSI                0x00A1
#define NM_HDV                  0x00CC  /* Copland - High Density voice NM */
#define NEZHA_STM1_MM           0x01B8  /* Nezha STM1 multi-mode (same as 7200 PA_HARDWARE_CT3P3M) */
#define NEZHA_STM1_SM           0x01BA  /* Nezha STM1 single-mode (same as 7200 PA_HARDWARE_CT3P3S) */
#define NM_HDA                  0x022C  /* Bigband _ High density Analog 
					   Telephony NM */
#define SKYHAWK_SW_ID           0x02A9  /* Single wide base board */
#define SKYHAWK_DW_ID           0x02B1  /* Double wide base board */
#define VOLANT_1NM_ID           0x0381  /* Volant-1NM base board */
#define VOLANT_2NM_SCSI36G_ID   0x0387  /* Volant-2NM-36G base board */
#define NM_1FE2W_V2             0x0396  /* Mufasa NM with 1FE 2 WICs */
#define NM_2FE2W_V2             0x0397  /* Mufasa NM with 2FE 2 WICs */
#define NM_1FE1R2W_V2           0x0398  /* Mufasa NM with 1FE 1 TR 2 WICs */
#define NM_HDV_1V               0x039A  /* Guido NM with 1 Vic */
#define NM_HDV_2V               0x039B  /* Guido NM with 2 Vics */
#define NM_HDV_2VE              0x039C  /* Guido NM with 2 Vics */
#define NM_1CE1T1_PRI           0x039D
#define NM_2CE1T1_PRI           0x039E
#define AESOP_1NM_ID            0x03AA  /* Aesop-1NM (volant 1NM) base board */
#define NM_HDV2_0T1E1           0x03D9  /* Soprano NM with 0 T1E1 */
#define NM_HDV2_1T1E1           0x03DA  /* Soprano NM with 1 T1E1 */
#define NM_HDV2_2T1E1           0x03DB  /* Soprano NM with 2 T1E1 */
#define BORA_4TE1_ID            0x03DF  /* Borabora NM with 4T1/E1 */
#define VOLANT_2NM_SCSI73G_ID   0x0405  /* Volant-2NM-73G base board */
#define V1_8FXSDID_ID           0x0416  /* Venom base board */
#define EM_3FXS_4FXO            0x0417  /* Venom EM */
#define EM_6FXO                 0x0418  /* Venom EM */
#define EM_4BRI                 0x0419  /* Venom EM */
#define MOHAWK_1NM_ID		0x0425  /* Mohawk-1NM (volant 1NM) base board */
#define BORA_4SER_ID            0x0453  /* Borabora NM with 4 Serial */
#define NM_VSAT_ID              0x0461  /* Bruno Satellite NM */
#define JETFIRE_1NM_ID		0x046A  /* Jetfire-1NM (volant 1NM) base board */
#define AONS_1NM_ID		0x0489  /* AONS-1NM (volant 1NM) base brd */
#define MIRAGE_16_ID            0x04A2  /* Mirage 16 ports */
#define MIRAGE_23_ID            0x04A3  /* Mirage 23 ports */
#define MIRAGE_48_ID            0x04A4  /* Mirage 48 ports */
#define MIRAGE_24_ID            0x04A5  /* Mirage 24 ports */
#define MIRAGE_ILP_24_ID        0x04A6  /* Mirage ILP Daughter card 24-port */
#define MIRAGE_ILP_16_ID        0x04A7  /* Mirage ILP Daughter card 16-port */
#define DETOX_NM_ID		0x04AB  /* Detox NM - added to distinguish FPGA IDs */
#define CUE_EC_1NM_ID		0x04E9  /* Aesop-1NM with larger memory */
#define MIRAGE_16_NON_ILP_ID    0x04EF  /* Mirage 16 ports */
#define MIRAGE_23_NON_ILP_ID    0x04F0  /* Mirage 23 ports */
#define MIRAGE_24_NON_ILP_ID    0x04F1  /* Mirage 24 ports */
#define MIRAGE_48_NON_ILP_ID    0x04F2  /* Mirage 24 ports */
#define BRYCE_NM_ID             0x04F8  /* Bryce NM */
#define BRYCE_NM_DAUGH_ID       0x04F9  /* Bryce NM */
#define BOXER_NM_ID		0x0503  /* Boxer is Volant NM without Hdriver */
#define PANOPTES_NM_ID          0x0521  /* Panoptes NM */
#define PANOPTES_NM_DAUGH_ID    0x0522  /* Panoptes NM */
#define MOHICAN_NM_ID		0x0528  /* Mohican NM-1A-T3/E3 */
#define HWIC_1CE1T1_PRI         0x0529  /* Spidey 1 port T1/E1 */
#define HWIC_2CE1T1_PRI         0x052A  /* Spidey 2 port T1/E1 */
#define HWIC_4T1E1              0x052B  /* Spidey 4 port T1/E1 */
#define VWIC3_1MFT_T1E1         0x06ED  /* Argot 1 port        */
#define VWIC3_1MFT_G703         0x06F0  /* Argot 1 port G703   */
#define VWIC3_2MFT_T1E1         0x06EF  /* Argot 2 port        */
#define VWIC3_2MFT_G703         0x06F1  /* Argot 2 port G703   */
#define VWIC3_4MFT_T1E1         0x06F2  /* Argot 4 port        */
#define HWIC_4CE1T1_PRI         0x06BC  /* Argot 4 port T1/E1  */
#define NM_IRONMAN		0x052D  /* NM carrier */
#define NM_8CE1T1_PRI		0x052E  /* NM carrier w/two 4 port T1/E1 */
#define NM_1CT3			0x052F  /* NM carrier w/one 1 port CT3 */
#define HWIC_1B_U               0x053D  /* Mario HWIC */
#define HWIC_4B_ST              0x053E  /* Luigi HWIC */
#define STOLI_NM_ID             0x055B  /* Stoli NM */
#define JASPER_NM_ID            0x0582  /* Jasper NM */
#define NM_CIPS_ID              0x0555  /* PSE - Crazy Hawk - NM-CIPS */
#define NM_WPO_80G_ID           0x0558  /* PSE - Cache Advance - NM-COMP/RE-80G*/
#define NM_WPO_ID               0x0559  /* PSE - Cache Advance - NM-COMP/RE*/
#define HWIC_1T1E1              0x05D4  /* Modified Spidey 1 port T1E1 */
#define HWIC_1VDSL              0x067A   /* BCM6368 VDSL POTS */
#define NM_CIPS_NAME            "NME-IPS-K9"
#define NM_WPO_80G_NAME         "NME-TPO-80G"
#define NM_WPO_NAME             "NME-TPO"
#define NM_ROCKY_WLC6_NAME      "NME-AIR-WLC6-K9"
#define NM_ROCKY_WLC8_NAME      "NME-AIR-WLC8-K9"
#define NM_ROCKY_WLC12_NAME     "NME-AIR-WLC12-K9"
#define NM_ROCKY_WLC25_NAME     "NME-AIR-WLC25-K9"

/*
 * SM Adaper and SM card IDs (in ID ASCENDING order)
 */
#define SM_ADAPTER_CARD         0x05F4  /* SM Adapter Card */
#define APEXZETA_CARD           0x650 /* Apex-Zeta Single Core Card */
#define APEXZETA_DUAL_CARD      0x652 /* Apex-Zeta Dual Core Card */
#define SM2PA_ADAPTER_CARD      0x6DB  /* SM2PA Adapter Card */
#define SM_1T3E3                0x0775  /* SM Patriot */

#define NGSM_TESTCARD           0x0B71  /* NGSM TestCard */
#define SM_10GKR_TESTCARD       0x1066  /* SM 10GKR TestCard */
#define SM_BCM57412_TESTCARD    0x10ca  /* SM BCM57412 TestCard */

#define NGSM_WOODLAWN_10G4G     0x0BC5  /* NGSM Woodlawn 10G/4G */
#define NGSM_WOODLAWN_6G        0x0BC4  /* NGSM Woodlawn 6G */

#define NGSM_THULE              0x0C85  /* NGSM Thule Carrier Card */
#define SM_REVA_64A             0x0D77  /* Reva SM 64 port Async   */
#define OAKENSHIELD_SM          0x0BEB  /* Oakenshield SM */
#define C_SM_NIM_ADPT           0x0DAD  /* Switzer-Carrier */

#define NGSM_NWK24              0x108D  /* NGSM Nightwatch-24 Card */
#define NGSM_NWK48              0x109A  /* NGSM Dual-width Nightwatch-48 Card */

/*
 * ISM card IDs (in ID ASCENDING order)
 */
#define ISM_AIRCONNECT_ID       0x05A4
#define ISM_TESTCARD_ID         0x0610
#define ISM_APEX_VEGA_ID        0x0631

/*
 * AIM IDs (in ID ASCENDING order)
 */
#define AIM_ID_VPN_HP_KONTROL   0x030B  /* HP Kontrol card */
#define AIM_ID_VPN_EP_KONTROL   0x0310  /* EP Kontrol card */
#define AIM_CHAUCER_ID          0x038E  /* Chaucer(Aesop) AIM */
#define AIM_PSE_ID              0x0552
#define AIM_PSE_NAME            "Generic PSE AIM" /* generic name */
#define AIM_IPS_NAME            "AIM-IPS-K9" /* PSE - Crazy Hawk */
#define AIM_TPO1_NAME           "AIM-TPO-1"  /* PSE - Cache Advance */
#define AIM_TPO2_NAME           "AIM-TPO-2"  /* PSE - Cache Advance */

/*
 * WIC/HWIC/VWIC IDs (in ID ASCENDING order)
 */
#define WIC_MFT_T1              0x20
#define WIC_2MFT_T1             0x22
#define WIC_1B_ST_V2            0x4E
#define HWIC_4E                 0x56    /* Pan 4 Port */
#define HWIC_4E_ILP             0x57    /* Pan 4 Port ILP */
#define HWIC_8E                 0x58    /* Pan 8 Port */
#define HWIC_8E_ILP             0x59    /* Pan 8 Port ILP */
#define HWIC_1GE_SFP            0x5E    /* Cheyenne */
#define VWIC2_1MFT_G703         0x3FA   /* Shamu */
#define VWIC2_2MFT_G703         0x3FB    /* Pan 8 Port */
#define VWIC2_2MFT_T1E1         0x3FC    /* Pan 8 Port ILP */
#define VWIC2_1MFT_T1E1         0x3FD    /* Cheyenne */
#define WIC_CRUSHER             0x0431  /* Crusher */
#define HWIC_4T                 0x0443  /* Simpsons 4 port high speed */
#define HWIC_4AS                0x0444  /* Simpsons 4 port Async/Sync */
#define HWIC_8AS_RS232          0x0445  /* Simpsons 8 port Async/Sync */
#define HWIC_16A                0x0446  /* Simpsons 16 port Sync */
#define HWIC_8A                 0x0447  /* Simpsons 8 port Sync */
#define HWIC_ADSL_BST		0x0471	/* Borghetti ADSL & ISDN S/T - AnnexA */
#define HWIC_ADSLI_BST		0x0472	/* Borghetti ADSL & ISDN S/T - AnnexB */
#define WIC_1B_ST_V3            0x0484  /* Fiddle V3 */
#define WIC_1AM_V2              0x048A  /* Madjack 1-port analog modem */
#define WIC_2AM_V2              0x048B  /* Madjack 2-port analog modem */
#define HWIC_AP_XG_A     	0x0491  /* Airlink */
#define HWIC_1ADSL           	0x04C8  /* Borghetti ADSL only - Annex A */
#define HWIC_1ADSLI           	0x04C9  /* Borghetti ADSL only - Annex B */
#define HWIC_CABLE_D            0x04F6  /* Snowshoe cable modem usa */
#define HWIC_CABLE_EJ           0x04F7  /* Snowshoe cable modem euro/japan */
#define HWIC_ARCHER_2PORT    	0x050E  /* 2 port shdsl */
#define HWIC_ARCHER_4PORT       0x050F  /* 4 port shdsl */
#define HWIC_3G_GSM             0x051E  /* Enzo GSM */
#define HWIC_3G_CDMA            0x051F  /* Enzo CDMA */
#define HWIC_3G_HSPA            0x061D  /* Enzo GSM HSPA */
#define HWIC_3G_HSPA_G          0x067E  /* Enzo GSM HSPA-G */
#define HWIC_3G_HSPA_A          0x0688  /* Enzo GSM HSPA-A */
#define HWIC_3G_HSPA_PLUS       0x06C3  /* Enzo (McLarenF1) GSM HSPA-PLUS */
#define HWIC_3G_GPS_CDMA        0x06C4  /* Enzo (McLarenF1) CDMA GPS */
#define HWIC_3G_GPS_HSPA_U      0x06C5  /* Enzo (McLarenF1) GSM GPS HSPA-U */
#define HWIC_1CE1T1_PRI     	0x0529  /* Spidey 1 port T1/E1 */
#define HWIC_2CE1T1_PRI     	0x052A  /* Spidey 2 port T1/E1 */
#define HWIC_4T1E1	     	0x052B  /* Spidey 4 port T1/E1 */
#define HWIC_4CE1T1_PRI	     	0x06BC  /* Argot 4 port T1/E1  */
#define HWIC_1CT3	     	0x052C  /* Spidey 1 port CT3 */
#define HWIC_1E                 0x053A  /* Jackson Falls 1 Port */
#define HWIC_2E                 0x053B  /* Jackson Falls 2 Port */
#define HWIC_1ADSLM             0x0566  /* Borghetti ADSL only - Annex M */
#define HWIC_1T                 0x0588  /* Mallard HWIC-1T */
#define HWIC_2T                 0x0589  /* Mallard HWIC-2T */
#define HWIC_1DSU_T1            0x058A  /* Mallard HWIC-1DSU-T1 */
#define HWIC_2AS                0x058E  /* Mallard HWIC-2A/S */
#define SP_VIC2_BRI_NT          0x05B3  /* BRI card for IAD2801 only */
#define GRWIC_1CE1T1_PRI        0x0671  /* GRWIC Spidey 1 port T1/E1 */
#define GRWIC_2CE1T1_PRI        0x0699  /* GRWIC Spidey 2 port T1/E1 */
#define GRWIC_8AS_RS232         0x069a  /* GRWIC Simpsons 8 port Async/Sync */
#define EHWIC_1GE_SFP_CU        0x06A3  /* Squier GE SFP or Cu port */
#define EHWIC_1GE_SFPX_CU       0x06EE  /* Precision GE SFP or Cu port */
#define HWIC_4GE                0x06B5  /* Firebee 4 Port */
#define HWIC_8GE                0x06B7  /* Firebee 8 Port */
#define HWIC_2TC                0x06D0  /* Mallard HWIC-2T-C */
#define HWIC_1TC                0x06D3  /* Mallard HWIC-1T-C */
#define HWIC_8EC                0x06D6  /* Pan 8 Port-C */
#define HWIC_4EC                0x06D7  /* Pan 4 Port-C */
#define VWIC2_1MFT_G703_C       0x06D4  /* Shamu 1 port G703 */ 
#define VWIC2_2MFT_G703_C       0x06D1  /* Shamu 2 port G703 */ 
#define HWIC_VACCODE            0xfe    /* port is vacant */
#define HWIC_ILLCODE            0xfd    /* illegal port code */

#define NGWIC_FORTITUDE         0x783   /* Fortitude NGWIC */
#define NGWIC_PRINCE_2T         0x0B73  /* Prince NGWIC: NIM-2T */
#define NGWIC_PRINCE_1T         0x0B74  /* Prince NGWIC: NIM-1T */
#define NGWIC_PRINCE_4T         0x0B72  /* Prince NGWIC: NIM-4T */
#define NGWIC_TESTCARD          0x0B70  /* NGWIC TestCard */
#define NIM_10GKR_TESTCARD      0x0C98  /* NIM 10GKR TestCard */
#define NIM_WALLANDER_1GE       0x0C88  /* Wallander NGWIC : NIM-1GE-CU-SFP */
#define NIM_WALLANDER_2GE       0x0C89  /* Wallander NGWIC : NIM-2GE-CU-SFP */
#define NIM_ES2_8P              0x0C6D  /* Dreamliner NIM: NIM-8-POE */
#define NIM_ES2_8               0x0C6F  /* Dreamliner NIM: NIM-8     */
#define NIM_ES2_4               0x0C72  /* Dreamliner NIM: NIM-4     */
#define NIM_4G_LTE_LA           0x0D08  /* Arkenstone NIM: CAT4 */
#define NIM_LTEA                0x0D13  /* Arkenstone NIM: CAT6 */
#define NIM_REVA_24A            0x0D1E  /* Reva 24 port Async   */
#define NIM_REVA_16A            0x0D1F  /* Reva 16 port Async   */
#define NIM_F2W_ID              0x0D48  /* F2W NIM   */
#define NIM_KAZIRZNGA           0x0D7C  /* Dynamo NIM-2BRI-S/T NIM-4BRI-S/T */
#define NIM_KALAMATA_GSHDSL     0x1039  /* NIM KALAMATA GSHDSL */
#define C_NIM_1X                0x10AD  /* Switzer-10G */
#define C_NIM_2M                0x112D  /* Switzer-Manhattan-2.5G (obseleted) */
#define C_NIM_4T                0x112F  /* Switzer-Manhattan-4G (obseleted) */
#define C_NIM_1M                0x1140  /* Switzer 2.5G NIM Card */
#define C_NIM_2T                0x113F  /* Switzer 4x1G NIM Card */
#define ATREIDES_VIRTUAL_NIM    0x0BEB  /* Virtual-NIM */
#define PHOENIX_VIRTUAL_SM      0x0BEB  /* Phoenix Virtual-SM */

/*
 * Daughter Card IDs (in ID ASCENDING order)
 */
#define PSU_AC_DELTA_ID         0x06BA  /* Delta AC PSU */
#define PSU_DC_DELTA_ID         0x06BB  /* Delta DC PSU */
#define SKYHAWK_GE_CARD_ID      0x02B2  /* Gigabit Daughter card  */ 
#define SKYHAWK_PSW_CARD_ID     0x02B3  /* Single-Wide Power daughter card */
#define SKYHAWK_PDW_CARD_ID     0x02B4  /* Double-Wide Power daughter card */
#define VOLANT_SCSI_DC_ID       0x0382  /* Volant-1NM SCSI daughter card */
#define VOLANT_IDE20G_DC_ID     0x0383  /* Volant-1NM IDE-20G daughter card  */
#define VOLANT_2NM_SCSI36G_ID   0x0387  /* Volant-2NM-36G base board */
#define VOLANT_IDE40G_DC_ID     0x03D8  /* Volant-1NM IDE-40G daughter card  */
#define NM_HDV2_0T1E1           0x03D9  /* Soprano NM with 0 T1E1 */
#define NM_HDV2_1T1E1           0x03DA  /* Soprano NM with 1 T1E1 */
#define NM_HDV2_2T1E1           0x03DB  /* Soprano NM with 2 T1E1 */
#define VOLANT_2NM_SCSI73G_ID   0x0405  /* Volant-2NM-73G base board */
#define HWIC_ADSL_DC		0x047A  /* Borghetti HWIC Annex A */
#define HWIC_ADSLI_DC		0x047B  /* Borghetti HWIC Annex B */
#define HWIC_ADSLM_DC		0x0567  /* Borghetti HWIC Annex M */
#define VOLANT_IDE80G_DC_ID     0x047C  /* Volant-1NM IDE-80G daughter card  */
#define HWIC_F_CABLE_D          0x04EA  /* Snowcrest cable modem usa */
#define HWIC_F_CABLE_EJ         0x04EB  /* Snowcrest cable modem euro/japan */

#define GRAFFHAM_VM             0x0B0B  /* Graffham NGVM */
#define GRAFFHAM_TESTCARD       0x0BC8  /* Graffham NGVM Testcard */
#define TIMINGCARD_VM           0x0B0C  /* TimingCard NGVM */
#define DYNAMO_NIM1             0x0BEB  /* Dynamo NIM-2/4FXS, NIM-2/4FXO,
                                           NIM-2/42EM */
#define DYNAMO_NIM2             0x0BEC  /* Dynamo NIM-2/4 BRI-NT/T/E */
#define NGSM_SKYE_1CPU          0x0BF4  /* SKYE NGSM 1CPU */
#define NGSM_SKYE_2CPU          0x0CAA  /* SKYE NGSM 2CPU*/
#define INVALID_ID              0xffff

/* 
 * Pluggable Module IDs (in ID ASCENDING order)
 */
#define PLUGGABLE_TEST_CARD           0x1234
#define PLUGGABLE_PCIE_TEST_CARD      0x10EE
#define PLUGGABLE_LTE_EM              0x1047
#define PLUGGABLE_LTE_WP7601          0x1048
#define PLUGGABLE_LTE_WP7603          0x104a
#define PLUGGABLE_LTE_WP7605          0x10c9
#define PLUGGABLE_LTE_WP7607          0x1059
#define PLUGGABLE_LTE_WP7608          0x1083
#define PLUGGABLE_LTE_WP7609          0x1084
#define PLUGGABLE_LTE_WP7610          0x10ec
#define PLUGGABLE_SERIAL              0x1051
#define PLUGGABLE_LTE_TELIT_LM9x0     0x10c8    
#define PLUGGABLE_NR_5G_TELIT_FN980   0x1129


/* 
   fixed portion of data structure
   act as info provider from Eng-14099 spec. 
*/
typedef struct cookie_4_table_t {
    char	*p_fs;			/* point to a field string */
    char   *p_sn;          /* point to a short name */
    uint	val_length;		/* data length info */
    uchar       type;			/* individual section entry */
    uint        len_fd;			/* max length field for 0xc0 type */
    char       *input_form;		/* user input format */
    uchar       variable_type;          /* which field is alterable */
} cookie_4_table;

/*
  element of link list buffer
  */
typedef struct cookie_4_t {
    struct cookie_4_t   *f;             /* forward pointer */
    struct cookie_4_t   *b;             /* backward pointer */
    uchar               type;
    uint		        length;	    	/* current length for current type */
    uchar               misc;           /* misc for format/compatibility use */
    uchar               spare_flag;     /* spare = 1; non_spare = 0 */
    uchar               *p_val_byte;    /* pointer to real value byte */
    cookie_4_table      *p_info;        /* pointer to cookie_4_table element */
} COOKIE_4;

/* for Controller type 0x40 only */
typedef struct controller_type {
    uint	ctrl_type;
    char	*ctrl_name;
} controller_type_t;


/* 
 * Globals
 */
extern int daughtercard_tbl_size;
extern controller_type_t daughtercard_controller_type_info[];
extern uchar default_cookie_4_fmt[];
extern uchar default_spmm_vic_wic_cookie_4_fmt[];

/* 
 * Prototypes
 */
extern ushort get_nm_id(int );
extern void  get_cookie_4_plat_x (int, int, int *, uchar *, int);
extern void  read_cookie_content(uchar *, int, int, int);
extern void  read_cookie_content_x (uchar *, int, int, int, int);
extern int   check_cookie_valid(int , int, int);
extern int   check_cookie_valid_x (int , int , int , int);
extern ushort format_eeprom_data(uchar *);
extern int             toss_cookie_4 (int , int );
extern cookie_4_table * search_type_4_table (uchar );



/* PSE - Cache Advance/Crazy Hawk  -
   nm test prototypes are normally in slot.h, aim test prototypes are normally 
   in their own .h. each project is required to modify cookie_4.h platform_slot.h, 
   platform_aim.h, slot.h, and aim_slot.c just to add the cookie id and associate 
   it to test functions.
   Since the implementation lacks consistency, I hope to start defining prototypes
   here to minimize files modified for new projects to cookie_4.h, platform_slot.h
   and platform_aim.h
 */

/* AIM test prototypes */
int pse_aim_test(int slot);

/* NM test prototypes */
int pse_nm_test(int slot);

/* PCI test prototypes */
int pse_sys_pci_test(int slot);


extern void  get_cookie_4_plat(int32_t, int32_t, int32_t *, uchar *);
extern void  display_eeprom(uchar *);
extern void  run_smart_chip_menu(void *);
extern uchar *search_type_ret_addr_of_first_data(uchar *, uchar, uchar *, int);
extern ushort format_eeprom_data(uchar *);

extern void  init_cookie_4_default_x (int32_t , int32_t , uchar *, int32_t);
extern ushort format_eeprom_data_x(uchar *, int32_t);
extern void  display_eeprom_x(uchar *, int32_t);

extern void  swap_eeprom_x (uchar *, uchar *, int32_t );
extern ushort format_eeprom_data(uchar *eeprom_data_ptr );
extern void set_chassis_mac(char *buf);
extern uchar *get_mac_from_blk(uint32_t);
extern uint32_t get_mac_blk_size();
extern int get_mac_from_block(uint32_t, uchar *);


#endif
/* end of file: cookie_4.h */

/******** History ******** 
$Log: cookie_4.h,v $
Revision 1.41  2021/06/02 02:56:20  alpeng
merge sears into trunk

Revision 1.40  2021/04/14 09:10:13  achiu2
[PRRQ:CSCvx56970-2] Phoenix code review for ER

Revision 1.39  2021/04/12 13:36:05  xiaolaya
*** empty log message ***

Revision 1.38  2020/05/22 02:28:15  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.37  2019/10/17 02:16:14  kehuang2
Collapse Tabei-L into main trunk

Revision 1.36  2019/08/15 09:27:52  shjung
Supported WP7610 PIM

Revision 1.35  2019/07/19 08:34:08  alpeng
support sm testcard w/ bcm57412

Revision 1.34  2019/06/14 05:56:25  shjung
Supported WP7605 modules

Revision 1.33  2019/05/14 09:19:47  sherliu2
Support hyperloop

Revision 1.32.22.1  2018/12/13 19:06:01  shjung
Supported Hyperloop PIM

Revision 1.32  2018/08/30 07:03:45  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.31  2018/05/18 09:24:47  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.30  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.29  2018/02/24 07:22:15  letsai
Collapse Kalamata-branch to Main Trunk.

Revision 1.28.2.1  2018/03/09 05:55:35  shjung
Supported WP7608/7609

Revision 1.28  2018/02/09 09:10:26  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.27.22.1  2018/01/20 04:24:37  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.27.6.4  2017/12/12 13:02:56  shjung
Supported LTE-WP7607 module

Revision 1.27.6.3  2017/09/06 01:41:58  shjung
Add pluggable module info to support WP7603

Revision 1.27.6.2  2017/08/31 01:24:19  lucywang
updated control type of pluggable serial

Revision 1.27.6.1  2017/08/15 14:03:14  hondwang
star branch c9xx initial check in

Revision 1.27  2017/07/28 07:49:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.26  2017/03/16 10:55:48  umlin
Reva-SM: Commit Reva-SM platform side code to main trunk. RevaSM controller type is 0x0D77.

Revision 1.25  2017/03/16 02:46:10  haohsu
Add PID for kaziranga

Revision 1.24  2016/07/12 01:55:28  hondwang
Fix F2W bug and add PCAMAP ID

Revision 1.23.8.5  2017/07/31 10:49:57  lucywang
add pluggable serial code of host and module

Revision 1.23.8.3  2017/07/14 00:11:52  shjung
Add pluggable LTE-WP module test

Revision 1.23.8.2  2017/06/23 18:07:13  tirawan
Update Pluggable LTE cookie id

Revision 1.23  2016/06/04 09:22:19  alpeng
initial check in for f2w

Revision 1.22.2.6  2018/05/17 10:50:19  alpeng
 sync with trunk <trunk-051618>

Revision 1.22.2.5  2018/04/20 08:45:40  alpeng
support kalamata on Neptune

Revision 1.22.2.4  2018/01/29 23:15:09  ptong
Set SM 10GKR Testcard cookie controller ID to 0x1066

Revision 1.22.2.3  2018/01/16 06:46:31  alpeng
first check in for 10G-KR SM testcard; we need to apply correct id once hw ready for it

Revision 1.22.2.2  2017/09/19 10:18:50  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.22.2.1  2017/04/05 09:10:30  leschen
Sync with <ng_diag-tag-032917>

Revision 1.30  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.29  2018/02/24 07:22:15  letsai
Collapse Kalamata-branch to Main Trunk.

Revision 1.28.2.1  2018/03/09 05:55:35  shjung
Supported WP7608/7609

Revision 1.28  2018/02/09 09:10:26  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.27.22.1  2018/01/20 04:24:37  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.27.6.4  2017/12/12 13:02:56  shjung
Supported LTE-WP7607 module

Revision 1.27.6.3  2017/09/06 01:41:58  shjung
Add pluggable module info to support WP7603

Revision 1.27.6.2  2017/08/31 01:24:19  lucywang
updated control type of pluggable serial

Revision 1.27.6.1  2017/08/15 14:03:14  hondwang
star branch c9xx initial check in

Revision 1.27  2017/07/28 07:49:38  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.26  2017/03/16 10:55:48  umlin
Reva-SM: Commit Reva-SM platform side code to main trunk. RevaSM controller type is 0x0D77.

Revision 1.25  2017/03/16 02:46:10  haohsu
Add PID for kaziranga

Revision 1.24  2016/07/12 01:55:28  hondwang
Fix F2W bug and add PCAMAP ID

Revision 1.23.8.5  2017/07/31 10:49:57  lucywang
add pluggable serial code of host and module

Revision 1.23.8.3  2017/07/14 00:11:52  shjung
Add pluggable LTE-WP module test

Revision 1.23.8.2  2017/06/23 18:07:13  tirawan
Update Pluggable LTE cookie id

Revision 1.23  2016/06/04 09:22:19  alpeng
initial check in for f2w

Revision 1.22  2016/04/26 02:15:42  umlin
Initial check-in for Reva.
Merge Reva to maintrunk.

Revision 1.21  2016/04/15 10:19:24  xiaoyizh
Initial check-in for Arkenstone.

Revision 1.20  2015/05/25 00:41:19  steja
Add support Skye SM

Revision 1.19.2.1  2015/04/29 11:36:09  steja
Code check-in to skye-branch2 for ER code review

Revision 1.19  2015/02/27 10:02:12  iachang

Add support dreamliner NIM

Revision 1.18  2015/02/14 12:48:40  kodko
Collapse timing card branch code into main trunk.

Revision 1.17  2015/02/12 05:58:13  bowang3
Add support to NIM Wallander

Revision 1.16  2014/07/02 08:09:42  alpeng
add new testcard id for en/disable menu item and select smi addr

Revision 1.15  2014/07/01 09:07:29  bowang3
Add support to NGSM carrier card Thule

Revision 1.14  2014/04/28 11:33:56  danchung
Add related functions for Greyhound 10G-KR bring-up

Revision 1.13  2014/03/26 19:23:09  siyen
Added Dynamo supports at the platform (CSCun82755).

Revision 1.12.4.1  2014/04/25 06:56:34  kodko
Support ZL30361 reference 2 clock input test.

Revision 1.12  2013/11/27 10:36:28  alpeng
support /cookie.txt to store/write cookie

Revision 1.11  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.10  2013/10/08 08:48:25  tirawan
Woodlawn collapsed to main trunk

Revision 1.9  2013/03/05 02:09:40  liwwang
add ngwic prince support

Revision 1.8  2013/02/28 00:35:57  srane
Add support for NGVM testcard.

Revision 1.7  2012/11/21 19:49:25  palin2
Add cookie part number (73) offset definition.

Revision 1.6  2012/08/20 13:22:38  palin2
Add NGSM and NGWIC TestCard ID definition.

Revision 1.5  2012/06/05 11:44:24  palin2
Clean up compiler warnings.

Revision 1.4  2012/05/17 23:20:38  shhuang
Minor change from review feedback.

Revision 1.3  2012/03/28 23:33:33  huanngo
Adding Patriot cookie ID

Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/

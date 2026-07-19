/* $Id: platform_macsec.h,v 1.1 2013/01/25 10:47:02 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_macsec.h,v $
 *------------------------------------------------------------------
 * Header file for macsec ethernet port tests
 * 
 * Jan 2013 Alan Peng
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PLARFORM_MACSEC_H__
#define __PLARFORM_MACSEC_H__

/* We list the registers that macsec will be used for diagnostic
 * these registers are listed on 88E1584P Datasheet section 5.
 * LinkCrypt Register Description */

/* LinkCrypt Memory Map */
#define GENERAL_PORT_OFFSET 0x800
#define CRYPT_IGR_GEN      0xb   /* for disabe drop bad tag */
#define CRYPT_ISC_GEN      0x10  /* for VFL setting */
#define ELU_TBL_OFF4       0x104  /* egress default match, encrypt+auth */
#define ILU_TBL_OFF6       0x206  /* ingress decrypt+auth and VFL setting*/ 
#define ILU_TBL_OFF7       0x207 /* igr default match */ 
#define EGR_CTXT_OFF0     0x300  /* set sci[31:0] */
#define EGR_CTXT_OFF1     0x301  /* set sci[63:32] */
#define EGR_CTXT_OFF3     0x303  /* set tci[7:0] */
#define ENC_KEY_OFF0      0x400  /* set encrypt key */
#define ENC_KEY_OFF1      0x401  
#define ENC_KEY_OFF2      0x402  
#define ENC_KEY_OFF3      0x403  
#define EGR_HKEY_OFF0      0x480  /* set egres hash key */
#define EGR_HKEY_OFF1      0x481  
#define EGR_HKEY_OFF2      0x482  
#define EGR_HKEY_OFF3      0x483  
#define DEC_KEY_OFF0      0x500  /*  set decrypt key */
#define DEC_KEY_OFF1      0x501  
#define DEC_KEY_OFF2      0x502  
#define DEC_KEY_OFF3      0x503  
#define IGR_HKEY_OFF0      0x580  /*  set ingress hash key */
#define IGR_HKEY_OFF1      0x581  
#define IGR_HKEY_OFF2      0x582  
#define IGR_HKEY_OFF3      0x583  

/* MACsec Counters */
#define MACSEC_CNT_OFFSET      0x150
#define IGR_HIT      0x2800
#define IGR_OK       0x2820
#define IGR_UNCHK    0x2840
#define IGR_DELAY    0x2860
#define IGR_LATE     0x2880
#define IGR_INVLD       0x28A0
#define IGR_NOTVLD      0x28C0
#define EGR_PKT_PORT   0x28E0
#define EGR_PKT_ENC    0x2900
#define EGR_HIT        0x2920
#define IGR_OCT_VAL     0x2940
#define IGR_OCT_DEC     0x2941
#define IGR_UNTAG       0x2942
#define IGR_NOTAG       0x2943
#define IGR_BADTAG      0x2944
#define IGR_UNKSCI      0x2945
#define IGR_NOSCI       0x2946
#define IGR_UNUSSA      0x2947
#define IGR_NOUSSA      0x2948
#define IGR_OCT_TOT     0x2949
#define EGR_OCT_PORT   0x294A
#define EGR_OCT_ENC    0x294B
#define EGR_OCT_TOT    0x294C
#define IGR_MISS      0x294D
#define EGR_MISS      0x294E
#define IGR_REDIR      0x294F

int macsec_test_main(int);
static int ovld_macsec_test(void);
static int macsec_test_88e1548p(uint, uint);
static int macsec_test_88e1548p_util(void);
static void init_macsec(uint, uint);
static void init_macsec_util(void);
static void clr_macsec_cnt(uint, uint);
static void clr_macsec_cnt_util(void);
static void macsec_reg_wr(uint, ushort, ushort, uint);
static int macsec_reg_rd(uint, ushort, ushort);
static void write_reg_util(void); 
static void read_reg_util(void); 

static void dump_statistic(void);
static int check_status(uint, uint);
static void cleanup_setting(uint, uint);
static int send_packet_util(void);
static void set_drop_util(void);
static void set_macsec_util(void);
static int set_internal_loopback_util(void);

#endif /* __PLARFORM_MACSEC_H__ */
/* end of module */

/*
$Log: platform_macsec.h,v $
Revision 1.1  2013/01/25 10:47:02  alpeng
support macsec util


$Endlog$
*/

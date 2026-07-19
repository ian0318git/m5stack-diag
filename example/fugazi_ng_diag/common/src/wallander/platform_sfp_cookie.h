/* $Id: platform_sfp_cookie.h,v 1.1 2015/02/26 07:18:30 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/platform_sfp_cookie.h,v $
 *------------------------------------------------------------------
 * Filename: platform_sfp_cookie.h
 *
 * Description: Wallander SFP Cookie structs and definitions.
 *		          This file is ported from Overlord.
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SFP_COOKIE_H__
#define __PLATFORM_SFP_COOKIE_H__

#include "dev_at24c0n.h"

typedef uint8_t sfp_p; /* SFP One Byte Size Register */

#define SFP_EEPROM_SIZE           (256)
#define FPGA_SFP_PATH_TEST_SIZE   (16)

/* Common defines */
#define SFP_PAD         at_t
#define INVALID_PID	 -1

/* Assume SFP enums are contigueous */
#define NUMBER_OF_SFP	  4
#define SFP_I_INIT_TIME	 330	/* t_init - 300 ms in MSA */

/* Number of SFP definitions */
#define SFP_ZERO    0
#define SFP_ONE     1
#define SFP_TWO     2
#define SFP_THREE   3

/*
 * Transceiver type
 */
typedef struct sfp_xv_t_ {
    SFP_PAD pad1;	     /* 0 - Reserved */
    at_t sonet[2];     /* Sonet Compliance Codes */
    at_t ge;			     /* Gigabit Ethernet Compliance Codes */
    at_t fib_ch[2];    /* Fibre Channel link length, xmitter tech */
    at_t fib_ch_med;   /* Fibre Channel transmission media */
    at_t fib_ch_sp;    /* Fibre Channel speed */
} sfp_xv_t;

/*
 * SFP Cookie Contents
 */
typedef struct sfp_cookie_t_ {
    /* Base ID Fields */
    at_t id;			/* 0 - Identifier */
    at_t x_id;			/* Extended Identifier */
    at_t connector;		/* Connector */
    sfp_xv_t xv;		/* 3 - Transceiver */
    at_t code;			/* Encoding */
    at_t br_nom;		/* BR, Nominal */
    SFP_PAD pad1;		/* Reserved */
    at_t len_9m_km;		/* Length (9m) - km */
    at_t len_9m;		/* Length (9m) */
    at_t len_50m;		/* 10 - Length (50m) */
    at_t len_62_5m;		/* Length (62.5m) */
    at_t len_cu;		/* Length (Copper) */
    SFP_PAD pad2;		/* Reserved */
    at_t vendor[16];		/* 14 - Vendor name */
    SFP_PAD pad3;		/* 24 - Reserved */
    at_t vendor_oui[3];		/* Vendor OUI */
    at_t vendor_pn[16];		/* 28 - Vendor PN */
    at_t vendor_rev[4];		/* 38 - Vendor rev */
    SFP_PAD pad4[3];		/* Reserved */
    at_t cc_base;		/* 3F - Check code for Base ID fields (0-62) */

    /* Extended ID Fields */
    at_t options[2];		/* 40 - Options */
    at_t br_max;		/* BR, Max */
    at_t br_min;		/* BR, Min */
    at_t vendor_sn[16];		/* 44 - Vendor SN */
    at_t date[8];		/* 54 - Date code */
    SFP_PAD pad5[3];		/* Reserved */
    at_t cc_ext;		/* 5F - Check code for Extended ID fields */

    /* Vendor Specific ID Fields */
    at_t vendor_sp[32];		/* 60 - Vendor specific data, read only */
    SFP_PAD pad6[384];		/* 80 - Reserved */
} sfp_cookie_t;

/* Fields offset defines */
#define SFP_COO_ID	    0x00
#define SFP_COO_X_ID  	0x01
#define SFP_COO_CNT	    0x02
#define SFP_COO_XVR	    0x03
#define SFP_COO_GECC	  0x06	/* Gigabit Ethernet Compliance Codes */
#define SFP_COO_ENC	    0x0B
#define SFP_COO_BR_N	  0x0C
#define SFP_COO_L_9KM	  0x0E
#define SFP_COO_L_9M	  0x0F
#define SFP_COO_L_50	  0x10
#define SFP_COO_L_62	  0x11
#define SFP_COO_L_CU	  0x12
#define SFP_COO_VEND	  0x14
#define SFP_COO_CH_S	  0x24
#define SFP_COO_VEN_O	  0x25
#define SFP_COO_VEN_PN	0x28
#define SFP_COO_VEN_R	  0x38
#define SFP_COO_LSR_W	  0x3C
#define SFP_COO_DWDM_W	0x3E
#define SFP_COO_CC_B	  0x3F
#define SFP_COO_OPT	    0x40
#define SFP_COO_BR_MAX	0x42
#define SFP_COO_BR_MIN	0x43
#define SFP_COO_VEN_SN	0x44
#define SFP_COO_DATE	  0x54
#define SFP_COO_DIAG	  0x5C
#define SFP_COO_ENH	    0x5D
#define SFP_COO_CC_X	  0x5F
#define SFP_COO_VEND_SP	0x60
#define SFP_COO_XID	    0x60

/* Fields Size */
#define SFP_COO_ID_L	   1
#define SFP_COO_XID_L	   1
#define SFP_COO_CNT_L	   1
#define SFP_COO_XVR_L	   8
#define SFP_COO_GECC_L	 1
#define SFP_COO_ENC_L	   1
#define SFP_COO_BR_N_L	 1
#define SFP_COO_L_9KM_L	 1
#define SFP_COO_L_9M_L	 1
#define SFP_COO_L_50_L	 1
#define SFP_COO_L_62_L	 1
#define SFP_COO_L_CU_L	 1
#define SFP_COO_VEND_L	16
#define SFP_COO_CH_S_L	 1
#define SFP_COO_VEN_O_L	 3
#define SFP_COO_VEN_P_L	16
#define SFP_COO_VEN_R_L	 4
#define SFP_COO_LSR_W_L	 2
#define SFP_COO_DWDM_L	 1
#define SFP_COO_CC_B_L	 1
#define SFP_COO_OPT_L	   2  
#define SFP_COO_BR_MX_L	 1
#define SFP_COO_BR_MN_L	 1
#define SFP_COO_VEN_S_L	16
#define SFP_COO_DATE_L	 8
#define SFP_COO_DIAG_L	 1
#define SFP_COO_ENH_L	   1
#define SFP_COO_CC_X_L	 1
#define SFP_COO_VN_SP_L	32
#define SFP_COO_XID_L	   1

/* Gigabit Ethernet Compliance Codes - 0x06 */
#define SFP_GECC_T		0x08	/* 1000BASE-T */
#define SFP_GECC_CX		0x04	/* 1000BASE-CX */
#define SFP_GECC_LX		0x02	/* 1000BASE-LX */
#define SFP_GECC_SX		0x01	/* 1000BASE-SX */

/* SFP Encoding - 0x0B */
#define SFP_ENCODE_UNKNOWN	    0	   /* Upspecified */
#define SFP_ENCODE_8B10B	      1	   /* 8B10B */
#define SFP_ENCODE_4B5B		      2	   /* 4B5B */
#define SFP_ENCODE_NRZ		      3  	 /* NRZ */
#define SFP_ENCODE_MANCHESTER	  4	   /* Manchester */
#define SFP_ENCODE_SONET	      5	   /* SONET Scrambled */
#define SFP_ENCODE_INVALID	   0xFF	 /* Reserved */

/* SFP Extended ID (GBIC) - 0x60 */
#define SFP_XID_GE_100FX	0x2A
#define SFP_XID_FE_100FX	0x2B
#define SFP_XID_FE_100LX	0x2C

/* SFP GLC-GE-100FX equates */
#define SFP_GE_100FX_REG	      0x1C	  /* Register offset to be updated */
#define SFP_GE_100FX_REG_FX_L	  0x10	  /* FX mode enable bit */
#define SFP_GE_100FX_REG_FX_H	  0x88  	/* Write and shadow page */
#define SFP_GE_100FX_REG18      0x18    /* Edge Control Register offset */
#define SFP_GE_100FX_REG_EC_L   0x30    /* 100Base-T 0ns */
#define SFP_GE_100FX_REG_EC_H   0x04    /* Normal Tx mode */

/* Extern Functions */
extern int sfp_cookie_read(int, int, int, char *, int);
extern void show_sfp_type(int);
extern int set_sfp_glc_ge_100fx(int);
extern int sfp_i2c_test_warp(void);
extern int fpga_sfp_i2c_read_test(void);

#endif /* __PLATFORM_SFP_COOKIE_H__ */


/*------------------------------------------------------------------
 * $Log: platform_sfp_cookie.h,v $
 * Revision 1.1  2015/02/26 07:18:30  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */


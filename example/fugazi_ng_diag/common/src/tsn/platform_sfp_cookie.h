/* $Id: platform_sfp_cookie.h,v 1.2 2018/05/30 10:06:57 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_sfp_cookie.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_sfp_cookie.h
 *
 * Description: Port from Inception SFP Cookie structs and defines.
 *		This file is based on EDCS-275976 and SFP Transceiver MSA.
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SFP_COOKIE_H__
#define __PLATFORM_SFP_COOKIE_H__

#define NUMBER_OF_SFP	        2
#define INVALID_PID	            -1
#define SFP_I_INIT_TIME	        330	    /* t_init - 300 ms in MSA */
#define SFP_I2C_DEV_ADDR        (0xAC >> 1) /* 0xAC > 1 = 0x56  */

/* SFP Encoding - 0x0B */
#define SFP_ENCODE_UNKNOWN	    0	    /* Upspecified */
#define SFP_ENCODE_8B10B	    1	    /* 8B10B */
#define SFP_ENCODE_4B5B		    2	    /* 4B5B */
#define SFP_ENCODE_NRZ		    3	    /* NRZ */
#define SFP_ENCODE_MANCHESTER	4	    /* Manchester */
#define SFP_ENCODE_SONET	    5	    /* SONET Scrambled */
#define SFP_ENCODE_INVALID	    0xFF	/* Reserved */

/* SFP Ethernet Compliance codes */
#define SFP_ETH_COMP_CODES      0x6
#define SFP_1000BASE_SX         0x01
#define SFP_1000BASE_LX         0x02
#define SFP_1000BASE_CX         0x04
#define SFP_1000BASE_T          0x08
#define SFP_100BASE_LX10        0x10
#define SFP_100BASE_FX          0x20
#define SFP_BASE_BX10           0x40
#define SFP_BASE_PX             0x80

/* SFP GLC-GE-100FX equates */
#define SFP_GE_100FX_REG	    0x1C	/* Register offset to be updated */
#define SFP_GE_100FX_REG_FX_L	0x10	/* FX mode enable bit */
#define SFP_GE_100FX_REG_FX_H	0x88	/* Write and shadow page */
#define SFP_GE_100FX_REG18      0x18    /* Edge Control Register offset */
#define SFP_GE_100FX_REG_EC_L   0x30    /* 100Base-T 0ns */
#define SFP_GE_100FX_REG_EC_H   0x04    /* Normal Tx mode */

/* Fields offset defines */
#define SFP_COO_ID	            0x00
#define SFP_COO_X_ID	        0x01
#define SFP_COO_CNT	            0x02
#define SFP_COO_XVR	            0x03
#define SFP_COO_GECC	        0x06	/* Gigabit Ethernet Compliance Codes */
#define SFP_COO_ENC	            0x0B
#define SFP_COO_BR_N	        0x0C
#define SFP_COO_L_9KM	        0x0E
#define SFP_COO_L_9M	        0x0F
#define SFP_COO_L_50	        0x10
#define SFP_COO_L_62	        0x11
#define SFP_COO_L_CU	        0x12
#define SFP_COO_VEND	        0x14
#define SFP_COO_CH_S	        0x24
#define SFP_COO_VEN_O	        0x25
#define SFP_COO_VEN_PN	        0x28
#define SFP_COO_VEN_R	        0x38
#define SFP_COO_LSR_W	        0x3C
#define SFP_COO_DWDM_W	        0x3E
#define SFP_COO_CC_B	        0x3F
#define SFP_COO_OPT	            0x40
#define SFP_COO_BR_MAX	        0x42
#define SFP_COO_BR_MIN	        0x43
#define SFP_COO_VEN_SN	        0x44
#define SFP_COO_DATE	        0x54
#define SFP_COO_DIAG	        0x5C
#define SFP_COO_ENH	            0x5D
#define SFP_COO_CC_X	        0x5F
#define SFP_COO_VEND_SP	        0x60
#define SFP_COO_XID	            0x60

/* Fields Size */
#define SFP_COO_ID_L	        1
#define SFP_COO_XID_L	        1
#define SFP_COO_CNT_L	        1
#define SFP_COO_XVR_L	        8
#define SFP_COO_GECC_L	        1
#define SFP_COO_ENC_L	        1
#define SFP_COO_BR_N_L	        1
#define SFP_COO_L_9KM_L	        1
#define SFP_COO_L_9M_L	        1
#define SFP_COO_L_50_L	        1
#define SFP_COO_L_62_L	        1
#define SFP_COO_L_CU_L	        1
#define SFP_COO_VEND_L	        16
#define SFP_COO_CH_S_L	        1
#define SFP_COO_VEN_O_L	        3
#define SFP_COO_VEN_P_L	        16
#define SFP_COO_VEN_R_L	        4
#define SFP_COO_LSR_W_L	        2
#define SFP_COO_DWDM_L	        1
#define SFP_COO_CC_B_L	        1
#define SFP_COO_OPT_L	        2
#define SFP_COO_BR_MX_L	        1
#define SFP_COO_BR_MN_L	        1
#define SFP_COO_VEN_S_L	        16
#define SFP_COO_DATE_L	        8
#define SFP_COO_DIAG_L	        1
#define SFP_COO_ENH_L	        1
#define SFP_COO_CC_X_L	        1
#define SFP_COO_VN_SP_L	        32
#define SFP_COO_XID_L	        1

/* SFP Extended ID (GBIC) - 0x60 */
#define SFP_XID_GE_100FX	    0x2A
#define SFP_XID_FE_100FX	    0x2B
#define SFP_XID_FE_100LX	    0x2C

/* SFP Type */
#define SFP_DEFAULT             0 
#define SFP_GE_100FX            1
#define SFP_FE_100FX            2
#define SFP_GLC_TE              3

/* SFP Copper ABCU-5710RZ-CS2 register offset */
#define SFP_COPPER_CONTROL      0x0
#define SFP_COPPER_STATUS       0x1
#define SFP_COPPER_AO_NG_AD     0x4
#define SFP_COPPER_AO_NG_LPA    0x5
#define SFP_COPPER_AO_EXP       0x6
#define SFP_COPPER_AO_NPT       0x7
#define SFP_COPPER_AO_LPRNT     0x8
#define SFP_COPPER_MA_SL_CR     0x9
#define SFP_COPPER_MA_SL_SR     0xA
#define SFP_COPPER_EC1          0x10
#define SFP_COPPER_ES1          0x11
#define SFP_COPPER_INT_REG      0x12
#define SFP_COPPER_EC2          0x14
#define SFP_COPPER_REC          0x15
#define SFP_COPPER_CD1          0x16
#define SFP_COPPER_EC3          0x1A
#define SFP_COPPER_ES2          0x1B
#define SFP_COPPER_CD2          0x1C
#define CISCO_AVAGO_SFP         {0x43,0x49,0x53,0x43,0x4f,0x2d,0x41,0x56,0x41, \
                                 0x47,0x4f,0x20,0x20,0x20,0x20,0x20}

#define SFP_CLR_INT_H           0x0
#define SFP_CLR_INT_L           0x0
#define SFP_FRC_MASTER_H        0x1B
#define SFP_FRC_MASTER_L        0x00
#define SFP_RES_EN_AUTO_NEG_H   0x91 
#define SFP_RES_EN_AUTO_NEG_L   0x40
#define SFP_SEL_P7_REG30_H      0x00 
#define SFP_SEL_P7_REG30_L      0x07
#define SFP_FRC_GBPS_MODE_H     0x08 
#define SFP_FRC_GBPS_MODE_L     0x08 
#define SFP_SEL_P16_REG30_H     0x00 
#define SFP_SEL_P16_REG30_L     0x10 
#define SFP_EN_LBPK_STUB_H      0x00 
#define SFP_EN_LBPK_STUB_L      0x02 
#define SFP_SEL_P18_REG30_H     0x00 
#define SFP_SEL_P18_REG30_L     0x12 
#define SFP_DIS_NEXT_H          0x80 
#define SFP_DIS_NEXT_L          0x01 


#define SFP_PHY_RESET_DELAY 3000
/* Functions prototype */
extern int sfp_cookie_read(int sfp, int offset, int size, char *data, int err);
extern int get_sfp_reg(uint);
extern int read_sfp_ext_id(int);

 #endif /* __PLATFORM_SFP_COOKIE_H__ */

/*------------------------------------------------------------------
 * $Log: platform_sfp_cookie.h,v $
 * Revision 1.2  2018/05/30 10:06:57  steja
 * <CSCvj57981>Enhance SFP read Ext.ID functionality to be reuse.
 *
 * Revision 1.1  2018/05/24 09:47:10  steja
 * CSCvj57981-Enhance SFP GLC-GE-100FX Support
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */


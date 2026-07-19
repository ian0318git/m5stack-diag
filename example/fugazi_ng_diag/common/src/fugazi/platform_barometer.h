/* $Id: platform_barometer.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_barometer.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_barometer.h
 *
 * Description: Definitions for Operation Overlord Barometer.
 *
 * Ported from ovld_barometer.h
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_BAROMETER_H__
#define __PLATFORM_BAROMETER_H__

/* MPL115A2 Barometer */
#define FUGAZI_BRMTR_POUTH                      0x00 /*  */ 
#define FUGAZI_BRMTR_POUTL                      0x01 /*  */ 
#define FUGAZI_BRMTR_TOUTH                      0x02 /*  */ 
#define FUGAZI_BRMTR_TOUTL                      0x03 /*  */ 
#define FUGAZI_BRMTR_COEF1                      0x04 /*  */ 
#define FUGAZI_BRMTR_COEF2                      0x05 /*  */ 
#define FUGAZI_BRMTR_COEF3                      0x06 /*  */ 
#define FUGAZI_BRMTR_COEF4                      0x07 /*  */ 
#define FUGAZI_BRMTR_COEF5                      0x08 /*  */ 
#define FUGAZI_BRMTR_COEF6                      0x09 /*  */ 
#define FUGAZI_BRMTR_COEF7                      0x0A /*  */
#define FUGAZI_BRMTR_COEF8                      0x0B /*  */
#define FUGAZI_BRMTR_COEF9                      0x0C /*  */
#define FUGAZI_BRMTR_COEF10                     0x0D /*  */
#define FUGAZI_BRMTR_COEF11                     0x0E /*  */
#define FUGAZI_BRMTR_COEF12                     0x0F /*  */
#define FUGAZI_BRMTR_PRESS                      0x10 /*  */
#define FUGAZI_BRMTR_TEMP                       0x11 /*  */
#define FUGAZI_BRMTR_BOTH                       0x12 /*  */

/* ST LPS25H Barometer */
#define ST_REF_P_XL        0x8
#define ST_REF_P_L         0x9
#define ST_REF_P_H         0xA
#define ST_WHO_AM_I        0xF
#define ST_RES_CONF        0x10
#define ST_STRL_REG1       0x20
#define ST_STRL_REG2       0x21
#define ST_STRL_REG3       0x22
#define ST_STRL_REG4       0x23
#define ST_INT_CFG         0x24
#define ST_INT_SOURCE      0x25
#define ST_STATUS_REG      0x27
#define ST_PRESS_OUT_XL    0x28
#define ST_PRESS_OUT_L     0x29
#define ST_PRESS_OUT_H     0x2A
#define ST_TEMP_OUT_L      0x2B
#define ST_TEMP_OUT_H      0x2C
#define ST_FIFO_CTRL       0x2E
#define ST_FIFO_STATUS     0x2F
#define ST_THS_P_L         0x30
#define ST_THS_P_H         0x31
#define ST_RPDS_L          0x39
#define ST_RPDS_H          0x3A

/* MAX1617 Commands */
#define FUGAZI_MAX1617_RLTS                      0x00 /* Read local temperature: returns latest temperature    */ 
#define FUGAZI_MAX1617_RRTE                      0x01 /* Read remote temperature: returns latest temperature    */ 
#define FUGAZI_MAX1617_RSL                       0x02 /* Read status byte (flags, busy signal)    */ 
#define FUGAZI_MAX1617_RCL                       0x03 /* Read configuration byte    */ 
#define FUGAZI_MAX1617_RCRA                      0x04 /* Read conversion rate byte    */ 
#define FUGAZI_MAX1617_RLHN                      0x05 /* Read local THIGH limit    */ 
#define FUGAZI_MAX1617_RLLI                      0x06 /* Read local TLOW limit    */ 
#define FUGAZI_MAX1617_RRHI                      0x07 /*     */ 
#define FUGAZI_MAX1617_RRLS                      0x08 /*     */ 
#define FUGAZI_MAX1617_WCA                       0x09 /*     */ 
#define FUGAZI_MAX1617_WCRW                      0x0A /*     */
#define FUGAZI_MAX1617_WLHO                      0x0B /*     */
#define FUGAZI_MAX1617_WLLM                      0x0C /*     */
#define FUGAZI_MAX1617_WRLA                      0x0D /*     */
#define FUGAZI_MAX1617_WRLN                      0x0E /*     */
#define FUGAZI_MAX1617_OSHT                      0x0F /*     */
#define FUGAZI_MAX1617_SPOR                      0xFC /*     */
#define FUGAZI_MAX1617_MFGID                     0xFE /*     */
#define FUGAZI_MAX1617_DEVID                     0xFF /*     */

typedef struct barometer_reg_info {
    char * name;
    unsigned int offset;
} barometer_reg_info_t;


#endif   /* __PLATFORM_BAROMETER_H__ */



/*-------------------------------------------------
 * $Log: platform_barometer.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:50  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */


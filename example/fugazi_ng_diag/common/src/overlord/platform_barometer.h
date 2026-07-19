/* $Id: platform_barometer.h,v 1.2 2018/05/18 09:24:51 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_barometer.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_barometer.h
 *
 * Description: Definitions for Operation Overlord Barometer.
 *
 * Ported from ovld_barometer.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_BAROMETER_H__
#define __PLATFORM_BAROMETER_H__

/* MPL115A2 Barometer */
#define OVLD_BRMTR_POUTH                      0x00 /*     */ 
#define OVLD_BRMTR_POUTL                      0x01 /*     */ 
#define OVLD_BRMTR_TOUTH                      0x02 /*     */ 
#define OVLD_BRMTR_TOUTL                      0x03 /*     */ 
#define OVLD_BRMTR_COEF1                      0x04 /*     */ 
#define OVLD_BRMTR_COEF2                      0x05 /*     */ 
#define OVLD_BRMTR_COEF3                      0x06 /*     */ 
#define OVLD_BRMTR_COEF4                      0x07 /*     */ 
#define OVLD_BRMTR_COEF5                      0x08 /*     */ 
#define OVLD_BRMTR_COEF6                      0x09 /*     */ 
#define OVLD_BRMTR_COEF7                      0x0A /*  */
#define OVLD_BRMTR_COEF8                      0x0B /*  */
#define OVLD_BRMTR_COEF9                      0x0C /*  */
#define OVLD_BRMTR_COEF10                     0x0D /*  */
#define OVLD_BRMTR_COEF11                     0x0E /*  */
#define OVLD_BRMTR_COEF12                     0x0F /*  */
#define OVLD_BRMTR_PRESS                      0x10 /*  */
#define OVLD_BRMTR_TEMP                       0x11 /*  */
#define OVLD_BRMTR_BOTH                       0x12 /*  */

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
#define OVLD_MAX1617_RLTS                      0x00 /* Read local temperature: returns latest temperature    */ 
#define OVLD_MAX1617_RRTE                      0x01 /* Read remote temperature: returns latest temperature    */ 
#define OVLD_MAX1617_RSL                       0x02 /* Read status byte (flags, busy signal)    */ 
#define OVLD_MAX1617_RCL                       0x03 /* Read configuration byte    */ 
#define OVLD_MAX1617_RCRA                      0x04 /* Read conversion rate byte    */ 
#define OVLD_MAX1617_RLHN                      0x05 /* Read local THIGH limit    */ 
#define OVLD_MAX1617_RLLI                      0x06 /* Read local TLOW limit    */ 
#define OVLD_MAX1617_RRHI                      0x07 /*     */ 
#define OVLD_MAX1617_RRLS                      0x08 /*     */ 
#define OVLD_MAX1617_WCA                       0x09 /*     */ 
#define OVLD_MAX1617_WCRW                      0x0A /*     */
#define OVLD_MAX1617_WLHO                      0x0B /*     */
#define OVLD_MAX1617_WLLM                      0x0C /*     */
#define OVLD_MAX1617_WRLA                      0x0D /*     */
#define OVLD_MAX1617_WRLN                      0x0E /*     */
#define OVLD_MAX1617_OSHT                      0x0F /*     */
#define OVLD_MAX1617_SPOR                      0xFC /*     */
#define OVLD_MAX1617_MFGID                     0xFE /*     */
#define OVLD_MAX1617_DEVID                     0xFF /*     */

typedef struct barometer_reg_info {
    char * name;
    unsigned int offset;
} barometer_reg_info_t;


#endif   /* __PLATFORM_BAROMETER_H__ */


/*------------------------------------------------------------------
$Log: platform_barometer.h,v $
Revision 1.2  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.66.1  2017/03/23 06:30:38  leschen
Support barometer LPS25H.

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/

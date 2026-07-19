/* $Id: diag_barometer_util.h,v 1.3 2016/06/17 07:08:22 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_barometer_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_barometer_util.h - Utility Function
 *
 * July 2015, iyc
 * 
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef DIAG_BAROMETER_UTIL_H_
#define DIAG_BAROMETER_UTIL_H_

extern int diag_barometer_util(void);
extern int diag_show_barometer (void);

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

#define TACHI_SYS_PRESS_THRE                  70

#endif /* DIAG_BAROMETER_UTIL_H_ */

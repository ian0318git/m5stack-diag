/* $Id: platform_margin_utils.h,v 1.2 2018/05/18 09:25:00 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/platform_margin_utils.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_margin_utils.h
 *
 * Description: Operation-Overlord Margin utilities.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_MARGIN_UTILS_H__
#define __PLATFORM_MARGIN_UTILS_H__

#define MB_IDT286_PLL0_CTRL_REG    (0x0008)

/* Margin type enums */
typedef enum {
    MRGN_1ST = 1,  
    MRGN_2ND,       
    MRGN_3RD,       
    MRGN_4TH,       
    MRGN_5TH,       
} margin_type_t;

/* Margin level enums */
typedef enum {
    MRGN_LV0 = 0,   /* for error report */
    MRGN_LV1,       /* lowest level */
    MRGN_LV2,       /* low level */
    MRGN_LV3,       /* normal */
    MRGN_LV4,       /* high */
    MRGN_LV5,       /* highest */
} margin_level_t;

typedef enum {
	FREQ_GET_MRGN = 0,
    FREQ_MARG_LOW,
    FREQ_MARG_HIGH,
    FREQ_MARG_NORM
} freq_margin_t;

extern int vtg_mrgn_x(int, int);
extern int freq_mrgn_x(int, int, boolean);

#endif /* __PLATFORM_MARGIN_UTILS_H__ */

/*------------------------------------------------------------------
$Log: platform_margin_utils.h,v $
Revision 1.2  2018/05/18 09:25:00  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2017/02/18 03:37:51  meho
Added voltage margin utility.

Revision 1.2  2014/02/11 09:52:46  hroni
enhance voltage margin support for USD

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.1  2013/01/31 10:48:46  alpeng
supported CLI cmds for voltage and freq margin

$Endlog$
*/

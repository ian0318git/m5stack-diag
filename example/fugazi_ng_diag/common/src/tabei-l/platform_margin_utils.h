/* $Id: platform_margin_utils.h,v 1.2 2019/11/25 08:55:52 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_margin_utils.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_margin_utils.h
 *
 * Description: For support NIM Kalamata
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_MARGIN_UTILS_H__
#define __PLATFORM_MARGIN_UTILS_H__

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


extern int vtg_mrgn_x(int, int);
extern int freq_mrgn_x(int, int, boolean);

#endif /* __PLATFORM_MARGIN_UTILS_H__ */

/*------------------------------------------------------------------
$Log: platform_margin_utils.h,v $
Revision 1.2  2019/11/25 08:55:52  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.1  2019/10/31 02:27:39  olin2
Support Kalamata on Tabei-L



$Endlog$
*/

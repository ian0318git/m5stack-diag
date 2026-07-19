/* $Id: platform_margin_utils.h,v 1.1 2020/01/09 01:02:03 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_margin_utils.h,v $
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

/*
 *-----------------------------------------------------------------------------
$Log: platform_margin_utils.h,v $
Revision 1.1  2020/01/09 01:02:03  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

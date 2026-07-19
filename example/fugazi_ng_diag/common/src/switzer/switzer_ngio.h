/* $Id: switzer_ngio.h,v 1.1 2020/05/22 02:28:47 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_ngio.h,v $
 *------------------------------------------------------------------
 *
 * switzer_ngio.h - Switzer WIC ngio interfaces.
 *
 * Mar. 2019, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_NGIO_H__
#define __SWITZER_NGIO_H__

#include "csrs/sm_csrs_top.h"

int switzer_ngiowic_present(struct switzer_ng_t *wic);
void switzer_ngiowic_disable(struct switzer_ng_t *wic);
int switzer_ngiowic_i2c_unreset(struct switzer_ng_t *wic);
int switzer_ngiowic_unreset(struct switzer_ng_t *wic);
int switzer_ngiowic_i2c_reset(struct switzer_ng_t *wic);
int switzer_ngiowic_reset(struct switzer_ng_t *wic);
int switzer_ngiowic_enable_uart(struct switzer_ng_t *wic);
int switzer_ngiowic_disable_uart(struct switzer_ng_t *wic);
int switzer_ngiowic_enable(struct switzer_ng_t *wic);
void switzer_ngiowic_pci_rdy(struct switzer_ng_t *wic, int on);

#endif

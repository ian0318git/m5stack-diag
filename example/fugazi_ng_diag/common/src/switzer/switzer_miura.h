/* $Id: switzer_miura.h,v 1.2 2019/08/06 06:56:17 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_miura.h,v $
 *------------------------------------------------------------------
 *
 * switzer_miura.h - Switzer broadcom miura interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_MIURA_H__
#define __SWITZER_MIURA_H__

#ifndef PLP_MIURA_SUPPORT
#define PLP_MIURA_SUPPORT
#endif
#ifndef PLP_MACSEC_SUPPORT
#define PLP_MACSEC_SUPPORT
#endif

#include <stdint.h>
#include <epdm.h>
#include <epdm_sec.h>

#include "switzer_fpga.h"

#define SWITZER_MIURA_LANE_MAP 0xF
#define SWITZER_MIURA_LANE_MAX 4
#define SWITZER_MIURA_SYS_SIDE 1
#define SWITZER_MIURA_LINE_SIDE 0
#define SWITZER_MIURA_EGRESS    0
#define SWITZER_MIURA_INGRESS   1
#define SWITZER_MIURA_MAX_MACSEC_SIDE_ALLOWED 2

struct switzer_miura_settings {
    const char *type;
    void *ctx;
    int (*read)(void* ctx, uint8_t devad, uint16_t addr, uint16_t *data);
    int (*write)(void* ctx, uint8_t devad, uint16_t addr, uint16_t data);
};

struct switzer_miura {
    const char *type;
    bcm_plp_access_t info;
    bcm_plp_mac_access_t mac_info;
    struct switzer_miura_settings settings;
    int id;
    int downloaded;
    int cfye_inited[SWITZER_MIURA_MAX_MACSEC_SIDE_ALLOWED];
    int secy_inited[SWITZER_MIURA_MAX_MACSEC_SIDE_ALLOWED];
};

int switzer_miura_init(struct switzer_miura *miura,
                       struct switzer_miura_settings *settings);
void switzer_miura_exit(struct switzer_miura *miura);
void switzer_miura_reset(struct switzer_miura *miura);

int switzer_miura_fw_download(struct switzer_miura *miura);
int switzer_miura_macsec_init(struct switzer_miura *miura, int bypass);
void switzer_miura_macsec_exit(struct switzer_miura *miura);

#endif /* __SWITZER_MIURA_H__ */

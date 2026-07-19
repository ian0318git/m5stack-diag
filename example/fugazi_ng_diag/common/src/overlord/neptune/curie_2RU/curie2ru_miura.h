/* $Id: curie2ru_miura.h,v 1.1 2020/01/09 01:01:57 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_miura.h,v $
 *------------------------------------------------------------------
 *
 * curie2ru_miura.h - Curie2ru broadcom miura interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_MIURA_H__
#define __CURIE2RU_MIURA_H__

#ifndef PLP_MIURA_SUPPORT
#define PLP_MIURA_SUPPORT
#endif
#ifndef PLP_MACSEC_SUPPORT
#define PLP_MACSEC_SUPPORT
#endif
#ifndef PLP_QUADRA28_SUPPORT
#define PLP_QUADRA28_SUPPORT
#endif

#include <stdint.h>
#include <epdm.h>
#include <epdm_sec.h>

#define CURIE2RU_MIURA_LANE_MAP 0xF
#define CURIE2RU_MIURA_LANE_MAX 4
#define CURIE2RU_MIURA_SYS_SIDE 1
#define CURIE2RU_MIURA_LINE_SIDE 0
#define CURIE2RU_MIURA_EGRESS    0
#define CURIE2RU_MIURA_INGRESS   1
#define CURIE2RU_MIURA_MAX_MACSEC_SIDE_ALLOWED 2

struct curie2ru_miura_settings {
    const char *type;
    void *ctx;
    int (*read)(void* ctx, uint8_t prtad, uint8_t devad, uint16_t addr, uint16_t *data);
    int (*write)(void* ctx, uint8_t prtad, uint8_t devad, uint16_t addr, uint16_t data);
    unsigned char prtad;
};

struct curie2ru_miura {
    const char *type;
    bcm_plp_access_t info;
    bcm_plp_mac_access_t mac_info;
    struct curie2ru_miura_settings settings;
    int id;
    int downloaded;
    int cfye_inited[CURIE2RU_MIURA_MAX_MACSEC_SIDE_ALLOWED];
    int secy_inited[CURIE2RU_MIURA_MAX_MACSEC_SIDE_ALLOWED];
};

int curie2ru_miura_init(struct curie2ru_miura *miura,
                        struct curie2ru_miura_settings *settings);
void curie2ru_miura_exit(struct curie2ru_miura *miura);
void curie2ru_miura_reset(struct curie2ru_miura *miura);

int curie2ru_miura_fw_download(struct curie2ru_miura *miura);
int curie2ru_miura_macsec_init(struct curie2ru_miura *miura, int bypass);
void curie2ru_miura_macsec_exit(struct curie2ru_miura *miura);

#endif /* __CURIE2RU_MIURA_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_miura.h,v $
Revision 1.1  2020/01/09 01:01:57  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

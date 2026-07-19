/* $Id: curie_quadra28.h,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/curie_quadra28.h,v $
 *------------------------------------------------------------------
 *
 * curie_quadra28.h - Curie broadcom quadra28 interfaces.
 *
 * Jan. 2019, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE_QUADRA28_H__
#define __CURIE_QUADRA28_H__

#ifndef PLP_QUADRA28_SUPPORT
#define PLP_QUADRA28_SUPPORT
#endif

#include <stdint.h>
#include <epdm.h>

#define CURIE_QUADRA28_LANE_MAP 0x1
#define CURIE_QUADRA28_LANE_MAX 4
#define CURIE_QUADRA28_SYS_SIDE 1
#define CURIE_QUADRA28_LINE_SIDE 0
#define CURIE_QUADRA28_EGRESS    0
#define CURIE_QUADRA28_INGRESS   1

struct curie_quadra28_settings {
    const char *type;
    void *ctx;
    int (*read)(void* ctx, uint8_t mdio, uint8_t devad, uint16_t addr, uint16_t *data);
    int (*write)(void* ctx, uint8_t mdio, uint8_t devad, uint16_t addr, uint16_t data);
};

struct curie_quadra28 {
    const char *type;
    bcm_plp_access_t info;
    struct curie_quadra28_settings settings;
    int id;
    int downloaded;
};

int curie_quadra28_init(struct curie_quadra28 *quadra28,
                           struct curie_quadra28_settings *settings);
void curie_quadra28_exit(struct curie_quadra28 *quadra28);
void curie_quadra28_reset(struct curie_quadra28 *quadra28);

int curie_quadra28_fw_download(struct curie_quadra28 *quadra28);

int bcmphy_quadra28_reg_read(struct curie_quadra28 *q28,
							 int if_side, unsigned int phy_id,
                             unsigned int lane, unsigned int dev_id,
                             unsigned int *reg_addr, unsigned int *val, int n);

int bcmphy_quadra28_reg_write(struct curie_quadra28 *q28,
                              int if_side, unsigned int phy_id,
                              unsigned int lane, unsigned int dev_id,
                              unsigned int *reg_addr, unsigned int *val, int n);
#endif /* __CURIE_QUADRA28_H__ */


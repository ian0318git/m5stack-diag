/* $Id: curie2ru_quadra28.h,v 1.1 2020/01/09 01:01:58 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_quadra28.h,v $
 *------------------------------------------------------------------
 *
 * curie2ru_quadra28.h - Curie2ru broadcom quadra28 interfaces.
 *
 * Feb. 2019, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_QUADRA28_H__
#define __CURIE2RU_QUADRA28_H__

#ifndef PLP_QUADRA28_SUPPORT
#define PLP_QUADRA28_SUPPORT
#endif

#include <stdint.h>
#include <epdm.h>

#define CURIE2RU_QUADRA28_LANE_MAP 0x1
#define CURIE2RU_QUADRA28_LANE_MAX 4
#define CURIE2RU_QUADRA28_SYS_SIDE 1
#define CURIE2RU_QUADRA28_LINE_SIDE 0
#define CURIE2RU_QUADRA28_EGRESS    0
#define CURIE2RU_QUADRA28_INGRESS   1

struct curie2ru_quadra28_settings {
    const char *type;
    void *ctx;
    int (*read)(void* ctx, uint8_t mdio, uint8_t devad, uint16_t addr, uint16_t *data);
    int (*write)(void* ctx, uint8_t mdio, uint8_t devad, uint16_t addr, uint16_t data);
};

struct curie2ru_quadra28 {
    const char *type;
    bcm_plp_access_t info;
    struct curie2ru_quadra28_settings settings;
    int id;
    int downloaded;
};

int curie2ru_quadra28_init(struct curie2ru_quadra28 *quadra28,
                           struct curie2ru_quadra28_settings *settings);
void curie2ru_quadra28_exit(struct curie2ru_quadra28 *quadra28);
void curie2ru_quadra28_reset(struct curie2ru_quadra28 *quadra28);

int curie2ru_quadra28_fw_download(struct curie2ru_quadra28 *quadra28);

int bcmphy_quadra28_reg_read(struct curie2ru_quadra28 *q28,
							 int if_side, unsigned int phy_id,
                             unsigned int lane, unsigned int dev_id,
                             unsigned int *reg_addr, unsigned int *val, int n);

int bcmphy_quadra28_reg_write(struct curie2ru_quadra28 *q28,
                              int if_side, unsigned int phy_id,
                              unsigned int lane, unsigned int dev_id,
                              unsigned int *reg_addr, unsigned int *val, int n);
#endif /* __CURIE2RU_QUADRA28_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_quadra28.h,v $
Revision 1.1  2020/01/09 01:01:58  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

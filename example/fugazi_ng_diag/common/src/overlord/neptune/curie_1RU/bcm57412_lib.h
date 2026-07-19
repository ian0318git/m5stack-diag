/* $Id: bcm57412_lib.h,v 1.3 2020/01/21 05:49:01 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm57412_lib.h,v $
 *------------------------------------------------------------------
 *
 * bcm57412_lib.h - Header file of BCM57412, related to MDIO.
 *
 * Feb. 2019, Leslie Chen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE1RU_BNXT_H__
#define __CURIE1RU_BNXT_H__

#include "curie_quadra28.h"

typedef enum {
    CURIE_IF_SIDE_LINE,
    CURIE_IF_SIDE_SYS,

    MAX_NR_CURIE_IF_SIDE
} curie_if_side_t;

#define CURIE_QUADRA28_DEV_PMA_PMD       1
#define CURIE_QUADRA28_DEV_PCS           3
#define CURIE_QUADRA28_DEV_CL73_AN       7

struct curie_bnxt_settings {
    uint16_t pci_domain;
    uint8_t pci_bus;
    uint8_t pci_dev;
    uint8_t pci_func;
};

struct curie_bnxt {
    struct curie_bnxt_settings settings;
#define BNXT_IFNAME_SIZE    32
    char ifname[BNXT_IFNAME_SIZE];
    int skfd;
};

#define NR_CURIE_BNXT    2
#define CURIE_MIURA_DEV_PMA_PMD       1
#define CURIE_MIURA_DEV_PCS           3
#define CURIE_MIURA_DEV_CL73_AN       7
#define UP_ETH_PORT1 "ifconfig eth4 up"
#define UP_ETH_PORT2 "ifconfig eth5 up"

struct curie_bcm82752 {
    struct curie_bnxt bnxt[NR_CURIE_BNXT];
    struct curie_quadra28 quadra28[NR_CURIE_BNXT];
};

int curie_bnxt_init(struct curie_bnxt *bnxt,
                       struct curie_bnxt_settings *settings);
void curie_bnxt_exit(struct curie_bnxt *bnxt);

int curie_bnxt_mdio_read(struct curie_bnxt *bnxt,
                            uint8_t prtad, uint8_t devad,
                            uint16_t addr, uint16_t *data);
int curie_bnxt_mdio_write(struct curie_bnxt *bnxt,
                             uint8_t prtad, uint8_t devad,
                             uint16_t addr, uint16_t data);

int curie1ru_port_is_linkup(int port);
#endif /* __CURIE1RU_BNXT_H__ */


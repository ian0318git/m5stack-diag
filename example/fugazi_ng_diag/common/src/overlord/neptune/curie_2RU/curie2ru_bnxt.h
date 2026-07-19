/* $Id: curie2ru_bnxt.h,v 1.1 2020/01/09 01:01:56 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_bnxt.h,v $
 *------------------------------------------------------------------
 *
 * curie2ru_bnxt.h - Curie2ru broadcom bnxt interfaces.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_BNXT_H__
#define __CURIE2RU_BNXT_H__

struct curie2ru_bnxt_settings {
    uint16_t pci_domain;
    uint8_t pci_bus;
    uint8_t pci_dev;
    uint8_t pci_func;
};

struct curie2ru_bnxt {
    struct curie2ru_bnxt_settings settings;
#define BNXT_IFNAME_SIZE    32
    char ifname[BNXT_IFNAME_SIZE];
    int skfd;
};

int curie2ru_bnxt_init(struct curie2ru_bnxt *bnxt,
                       struct curie2ru_bnxt_settings *settings);
void curie2ru_bnxt_exit(struct curie2ru_bnxt *bnxt);

int curie2ru_bnxt_mdio_read(struct curie2ru_bnxt *bnxt,
                            uint8_t prtad, uint8_t devad,
                            uint16_t addr, uint16_t *data);
int curie2ru_bnxt_mdio_write(struct curie2ru_bnxt *bnxt,
                             uint8_t prtad, uint8_t devad,
                             uint16_t addr, uint16_t data);

#endif /* __CURIE2RU_BNXT_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_bnxt.h,v $
Revision 1.1  2020/01/09 01:01:56  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

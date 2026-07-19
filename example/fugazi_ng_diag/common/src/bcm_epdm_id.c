/* $Id: bcm_epdm_id.c,v 1.2 2021/10/20 06:10:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/bcm_epdm_id.c,v $
 *------------------------------------------------------------------
 *
 * bcm_epdm_id.c - Broadcom EPDM id allocation APIs
 *
 * May 10, 2019, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <string.h>
#include "common.h"
 
#define BITS_PER_BYTE    8
#define MAX_NR_BCM_EPDM_PHY_ID  1024

static unsigned int
bcm_epdm_ids[MAX_NR_BCM_EPDM_PHY_ID / (sizeof(int) * BITS_PER_BYTE)];

/* alloc a global id for Broadcom EPDM SDK */
int bcm_epdm_alloc_id(void)
{
    int idx, bit, size = sizeof(bcm_epdm_ids) / sizeof(bcm_epdm_ids[0]);

    for (idx = 0; idx < size; idx++) {
        if (!(bit = ffs(~bcm_epdm_ids[idx])))
            continue;
        bit -= 1;
        bcm_epdm_ids[idx] |= (1 << bit);
        return idx * sizeof(int) * BITS_PER_BYTE + bit;
    }
    return -1;
}

/* free the global id for Broadcom EPDM SDK */
void bcm_epdm_free_id(int id)
{
    int idx, bit;

    if ((unsigned int)id >= MAX_NR_BCM_EPDM_PHY_ID)
        return;
    idx = id / (sizeof(int) * BITS_PER_BYTE);
    bit = id % (sizeof(int) * BITS_PER_BYTE);
    bcm_epdm_ids[idx] &= ~(1 << bit);
}

/*
 *-----------------------------------------------------------------------------
$Log: bcm_epdm_id.c,v $
Revision 1.2  2021/10/20 06:10:21  iachang
Migrate ISR Platform common code From RHEL7 to RHEL8

Revision 1.1  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

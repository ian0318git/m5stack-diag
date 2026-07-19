/* $Id: curie2ru_xhci.c,v 1.1 2020/01/09 01:01:58 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_xhci.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru_xhci.c - Curie2ru XHCI interfaces.
 *
 * Dec. 2018, Jiajia Liu <Jiajia@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "curie2ru_common.h"

/* Super Speed Port Enable (SSPE) Offset 80B8h */
#define XHCI_SSPE           0x80B8
#define XHCI_SSPE_PORT_MASK 0x3ff

/* disable/enable usb3 port super speed */
int __c2ru_disable_usb3_ss(int disable, unsigned int mask)
{
    struct pci_dev *pci;
    struct curie2ru_mmap map;
    uint32_t sspe;
    uint32_t port_mask = mask & XHCI_SSPE_PORT_MASK;

    pci = curie2ru_pci_dev_get(0, 0, 0x14, 0);
    if (pci == NULL) {
        pci = curie2ru_pci_dev_get(0, 0, 0x14, 0);
        if (pci == NULL) {
            log_err("failed to find XHCI Controller\n");
            return FAILED;
        }
    }

    map.paddr = (void *)pci->bar[0].address;
    map.length = pci->bar[0].size;

    if (curie2ru_file_mmap(NULL, &map,
                           CURIE2RU_MMAP_READ | CURIE2RU_MMAP_WRITE) < 0) {
        log_err("failed to mmap XHCI MMIO space\n");
        pci_dev_put(pci);
        return FAILED;
    }

    sspe = *(uint32_t *)(map.vaddr + XHCI_SSPE);
    if (disable)
        sspe &= ~port_mask;
    else
        sspe |= port_mask;

    *(uint32_t *)(map.vaddr + XHCI_SSPE) = sspe;

    curie2ru_file_munmap(&map);
    pci_dev_put(pci);

    return PASSED;
}

int c2ru_disable_usb3_ss(int disable)
{
    return  __c2ru_disable_usb3_ss(disable, 0x2C0);
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_xhci.c,v $
Revision 1.1  2020/01/09 01:01:58  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

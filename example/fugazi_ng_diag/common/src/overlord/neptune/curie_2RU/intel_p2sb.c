/* $Id: intel_p2sb.c,v 1.1 2020/01/09 01:02:00 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/intel_p2sb.c,v $
 *------------------------------------------------------------------
 *
 * intel_p2sb.c - Intel sideband P2SB interface
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <linux_pciutils.h>

#define SBREG_BAR       0x10
#define SBREG_BARH      0x14
#define SBREG_HIDDEN    0xe1

int intel_p2sb_get_bar(uint64_t *base_addr)
{
    struct pci_access *access;
    struct pci_dev *dev;
    uint64_t addr64;
    uint32_t addr;
    uint8_t hidden;
    int err = 0;

    access = pci_alloc();
    if (!access)
        return -ENOMEM;

    access->method = PCI_ACCESS_I386_TYPE1;
    pci_init(access);
    pci_scan_bus(access);

    /* P2SB: D31:F1 */
    dev = pci_get_dev(access, 0, 0, 31, 1);
    if (dev == NULL) {
        printf("error: Intel P2SB: No P2SB device found\n");
        err = -ENODEV;
        goto out;
    }

    /* The BIOS prevents the P2SB device from being enumerated by the
     * PCI subsystem, so we need to unhide and hide it back to lookup
     * the P2SB BAR. */

    hidden = pci_read_byte(dev, SBREG_HIDDEN);
    if (hidden)
        pci_write_byte(dev, SBREG_HIDDEN, 0x00);

    addr = pci_read_long(dev, SBREG_BAR);
    if (addr & 0x1) {
        printf("error: INTEL P2SB: SBREG_BAR address type is unexpected\n");
        err = -EFAULT;
        goto out;
    }

    addr64 = addr & ~0xF;
    addr64 |= (uint64_t)pci_read_long(dev, SBREG_BARH) << 32;

    if (base_addr)
        *base_addr = addr64;

out:
    pci_free_dev(dev);
    pci_cleanup(access);

    return err;
}

/*
 *-----------------------------------------------------------------------------
$Log: intel_p2sb.c,v $
Revision 1.1  2020/01/09 01:02:00  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

/* $Id: linux_pci.h,v 1.3 2020/01/09 01:01:49 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_pci.h,v $
 *------------------------------------------------------------------
 *
 *   \file pci.h
 *   \brief PCI API, derived from starfleet project
 *
 *  September 2012, Nocken Zou
 *
 *  Copyright (c) 2013-2019 by Cisco Systems, Inc.
 *  All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PCI_LINUX_H_
#define _PCI_LINUX_H_

#include <time.h>
#include <stdint.h>

#ifndef bool
#define bool uint8_t
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true !false
#endif

/* bit mask for flags read from kernel */
#define LINUX_IORESOURCE_BITS           0x000000ff /* Bus-specific bits */
#define LINUX_IORESOURCE_TYPE_BITS      0x00001f00 /* Resource type */
#define LINUX_IORESOURCE_IO             0x00000100
#define LINUX_IORESOURCE_MEM            0x00000200
#define LINUX_IORESOURCE_IRQ            0x00000400
#define LINUX_IORESOURCE_DMA            0x00000800
#define LINUX_IORESOURCE_BUS            0x00001000

#define PCI_CFG_SPACE_SIZE      256
#define PCI_CFG_SPACE_EXP_SIZE  4096

#define MAX_NR_PCI_BARS         6

struct pci_bar {
    unsigned long address;      /* memory range base address */
    unsigned long size;         /* memory range size */
    unsigned long flags;        /* memory range flags(from kernel) */
    unsigned char virtual;      /* memory range virtual flag */
};

struct pci_dev {
    uint16_t vendor;
    uint16_t device;
    uint16_t class;
    uint16_t domain;                 /* PCI domain (host bridge) */
    uint8_t bus;                     /* Bus inside domain */
    uint8_t dev;                     /* Dev inside bus */
    uint8_t func;                    /* Function inside device */
    uint8_t hdr_type;                /* header type */
    uint8_t multifunction;           /* Part of multi-function device */
    struct pci_bar bar[MAX_NR_PCI_BARS];
    /* PCI-to-PCI bridges */
    uint8_t primary;             /* Primary bus number */
    uint8_t secondary;           /* Secondary bus number */
    uint8_t subordinate;         /* Highest bus number behind the bridge */
    /* PCIe */
    uint8_t pcie_cap;                /* PCIe capability offset */
    uint16_t pcie_flags;
    uint8_t pcie_type;
    uint8_t msi_cap;                 /* MSI capability offset */
    uint8_t msix_cap;                /* MSI-X capability offset */
};

struct pci_snapshot;

void pci_dev_put(struct pci_dev *dev);
struct pci_dev *pci_dev_get(struct pci_dev *dev);
struct pci_dev *__pci_dev_get_by_id(uint16_t vendor, uint16_t device, int inst,
                                    struct pci_dev *bus);
struct pci_dev *pci_dev_get_by_id(uint16_t vendor, uint16_t device, int inst);
struct pci_dev *pci_dev_get_by_path(uint16_t domain, uint8_t bus, uint8_t dev, uint8_t func);
struct pci_dev *pci_dev_get_bus(struct pci_dev *dev);
struct pci_dev *pci_dev_find(void *data, int (*match)(struct pci_dev *dev, void *data));

/**
 *  \brief dump the information of a PCI device
 *
 *  \param pci device
 *  \return 0 on success, -1 on failure
 */
void pci_dev_show(struct pci_dev *dev);

/**
 *  \brief pci_dev_enable
 *
 *  enable/disable pci device
 *
 *  \param pci device
 *  \param pci enable
 *  \return 0 on success, -1 on failure
 */
int pci_dev_enable(struct pci_dev *dev, bool enable);

int pci_dev_enable_master(struct pci_dev *dev, bool enable);
int pci_dev_enable_msi(struct pci_dev *dev, bool enable);
int pci_dev_enable_msix(struct pci_dev *dev, bool enable);
int pci_dev_enable_aer(struct pci_dev *dev, bool enable);

/**
 *  \brief pci_dev_remove
 *
 *  remove pci device, it will remove the corresponding PCI device in
 *  the Linux Kernel space.
 *
 *  \param pci device
 *  \return 0 on success, -1 on failure
 */
int pci_dev_remove(struct pci_dev *dev);

/**
 *  \brief wait a pci device linked up in a timeout time
 *
 *  usually used after hard unresetting pci device
 *
 *  \param pci device
 *  \return true for linked up, false for linked down.
 */
bool pci_dev_link_status(struct pci_dev *dev, time_t timeout);

/**
 *  \brief wait a pci bus linked up in a timeout time
 *
 *  usually used after hard unresetting pci device
 *
 *  \param pci bus device
 *  \return true for linked up, false for linked down.
 */
bool pci_bus_link_status(struct pci_dev *dev, time_t timeout);

/**
 *  \brief do pci enumeration(sysfs only)
 *
 *  \return 0 on success, -1 on failure
 */
int __pci_rescan(void);

/**
 *  \brief do pci enumeration on a pci bus
 *
 *  \return 0 on success, -1 on failure
 */
int pci_bus_rescan(struct pci_dev *dev);

/**
 *  \brief do pci enumeration
 *
 *  \return 0 on success, -1 on failure
 */
int pci_rescan(void);

/**
 *  \brief Access to PCI configuration space
 *  \param pci device
 *  \param register in configuration space
 *  \param data
 *
 *  \return 0 on success, -1 for failure.
 */
int pci_read_config_byte(struct pci_dev *dev, int where, uint8_t *val);
int pci_read_config_word(struct pci_dev *dev, int where, uint16_t *val);
int pci_read_config_dword(struct pci_dev *dev, int where, uint32_t *val);
int pci_write_config_byte(struct pci_dev *dev, int where, uint8_t val);
int pci_write_config_word(struct pci_dev *dev, int where, uint16_t val);
int pci_write_config_dword(struct pci_dev *dev, int where, uint32_t val);

/**
 *  \brief create a snapshot for pci device
 *
 *  \param pci device
 *  \return snapshot handle on success, NULL on failure
 */
struct pci_snapshot *pci_snapshot_create(struct pci_dev *dev);

/**
 *  \brief capture a pci snapshot
 *
 *  \param snapshot
 *  \return 0 on success, -1 on failure
 */
int pci_snapshot_capture(struct pci_snapshot *snapshot);

/**
 *  \brief restore the snapshot to a pci device
 *
 *  usually used after hard unresetting pci device
 *
 *  \param snapshot
 *  \return 0 on success, -1 on failure
 */
int pci_snapshot_restore(struct pci_snapshot *snapshot);

/**
 *  \brief destroy the pci snapshot
 *
 *  \param snapshot
 */
void pci_snapshot_destroy(struct pci_snapshot *snapshot);

/**
 *  \brief achieve the pci device via pci snapshot
 *
 *  Note that: the pci device reference count do not increase here,
 *  because the snapshot already got reference of the pci device.
 *
 *  \param snapshot
 *  \return pci device
 */
struct pci_dev *pci_snapshot_dev(struct pci_snapshot *snapshot);

#endif /* _PCI_LINUX_H_ */

/* $Id: linux_pciutils.c,v 1.3 2018/08/06 02:37:59 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/linux_pciutils.c,v $
 *------------------------------------------------------------------
 * Filename:    linux_pciutils.c
 *
 * Description: utilities based on PCI Libraries.
 *
 * Copyright (c) 2016-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "linux_pciutils.h"
#include "nim_test_defs.h"

#define UNKNOWN_VENDOR_ID 0xFFFF
extern int module_num; 

/*
 * Function: diag_pci_get_device
 *
 * Description : using vendor id and device id to get the pci device structure.
 *
 * Inputs: vendor - PCI vendor id to match, or PCI_ANY_ID to match all vendor ids
 *         device - PCI device id to match, or PCI_ANY_ID to match all device ids
 *         from - Previous PCI device found in search, or NULL for new search.
 *
 * Output: pci_dev - return of pci device structure
 *                 - NULL : when device id or vendor id is not matched.
 */
struct pci_dev *diag_pci_get_device (unsigned short vendor,
                                     unsigned short device,
                                     struct pci_dev *from)
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    static struct pci_dev dev_last;
    boolean found_dev = 0; 

    pacc = pci_alloc(); /* Get the pci_access structure */
    pci_init(pacc);     /* Initialize the PCI library */
    pci_scan_bus(pacc); /* We want to get the list of devices */
    for (dev=pacc->devices; dev; dev=dev->next) /* Iterate over all devices */
    {

        /* Fill in header info we need */
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS); 

        /* Read config register directly */
        pci_read_byte(dev, PCI_INTERRUPT_PIN);

        if ((dev->vendor_id == vendor) && (dev->device_id == device)) {
            found_dev = 1;
            memcpy(&dev_last, dev, sizeof(struct pci_dev));
        }
    }

    pci_cleanup(pacc);   /* Close everything */

    if (found_dev == 1) {
        return &dev_last; 
    } else {
        return NULL;
    }
}

/*
 * Function: get_pcie_info
 *
 * Description : using vendor id and device id to get the pci bus number
 *
 * Inputs: vendor - PCI vendor id to match, or PCI_ANY_ID to match all vendor ids
 *         device - PCI device id to match, or PCI_ANY_ID to match all device ids
 *
 * Output: bus number 
 *                 
 */
int get_pcie_info (unsigned short vendor, unsigned short device,
                    unsigned char *bus, unsigned char *dev, 
                    unsigned char *func)
{
    struct pci_dev *pcidev;

    pcidev = diag_pci_get_device(vendor, device, NULL);

    if (pcidev == NULL) {
        return (UNKNOWN_PCI_BUS_NUM); /* return 0xFF to stand for error */
    } 

    *bus = pcidev->bus;
    *dev = pcidev->dev;
    *func = pcidev->func;
    pcidev = NULL;

    return 0; 
}

/*
 * Function: get_pcie_dev_bus_wrapper
 *
 * Description : a wrapper to get ngio pcie device bus number
 *
 * Inputs: module - nim module number 
 *
 * Output: bus number 
 *                 
 * Note: O2/USD using fixed nim pcie bus,dev,fn to get bus number, 
 *       we are using device vendor id and device id to get bus number.
 *       we can follow O2/USD once we are make sure nim pcie bus,dev,fn. 
 */
int get_pcie_dev_bus_wrapper (int module, int slot) 
{

    unsigned int vid, did;
    unsigned char bus, dev, func;

    switch (module) {
    case 0: 
        vid = TESTCARD_PCIE_SW_VID; 
        did = TESTCARD_PCIE_SW_DID; 
    break; 
    case 1:
        vid = 0x11AB;
        did = 0xe61e;  /* cell79 dreamliner vid, did */
    break; 
    default: 
        printf("unknown module , return 0. ..\n");
    break; 
    }
   
    get_pcie_info(vid, did, &bus, &dev, &func); 

    return (bus);
}

/*
 * Function: get_ngio_pcie_dev_bus_num
 *
 * Description : return ngio pcie device bus number. 
 *
 * Inputs: mod_type - nim or sm 
 *         slot - module slot   
 *
 * Output: bus number
 *
 * Note: using module_num to return bus number on tachi-entry
 */
int get_ngio_pcie_dev_bus_num (uint mod_type, uint slot)
{
    return (get_pcie_dev_bus_wrapper(module_num, slot)); 
}

/*------------------------------------------------------------------
$Log: linux_pciutils.c,v $
Revision 1.3  2018/08/06 02:37:59  harrchan
Changing daily build server from sjc-ads-1686 to sjc-ads-9106 (CSCvk60118)

Revision 1.2  2016/04/20 08:53:59  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/12/09 10:35:56  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.3  2015/09/14 08:02:29  alpeng
update to support dreamliner

Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code

Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

$Endlog$
*/


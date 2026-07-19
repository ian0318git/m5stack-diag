/* $Id: linux_pciutils.c,v 1.9 2020/01/09 01:01:51 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_pciutils.c,v $
 *------------------------------------------------------------------
 * Filename:    linux_pciutils.c
 *
 * Description: These utilities are based on PCI Libraries.  
 *
 * Copyright (c) 2013-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>

#include "linux_pciutils.h"
#include "types.h"
#include "common.h"

#define UNKNOWN_VENDOR_ID 0xFFFF


/*
 * Function: get_pci_vendor_id
 *
 * Description : using pci bus to get the vendor id. 
 *
 * Inputs: bus domain, bus, device, and func. 
 *
 * Output: PASSED/FAILED
 *
 * Note: lspci to get pci bus info 
 *       if PCI info is 0000:64:00.4 <=> [domain]:[bus]:[device]:[func]
 */
ushort get_pci_vendor_id (ushort domain, uchar bus, uchar device, uchar func) 
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    ushort vendor_id = UNKNOWN_VENDOR_ID;

    pacc = pci_alloc();	/* Get the pci_access structure */
    pci_init(pacc);     /* Initialize the PCI library */
    pci_scan_bus(pacc);	/* We want to get the list of devices */
    for (dev=pacc->devices; dev; dev=dev->next)	/* Iterate over all devices */
    {
        /* Fill in header info we need */
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS); 

        /* Read config register directly */
        pci_read_byte(dev, PCI_INTERRUPT_PIN);  

        if ((dev->domain == domain) && (dev->bus == bus) 
            && (dev->dev == device) && (dev->func == func)) {
             vendor_id = dev->vendor_id;
             break;
        } 
    }

    if (vendor_id == UNKNOWN_VENDOR_ID) {
        printf("Cannnot get device on %04x:%02x:%02x.%d \n", 
            dev->domain, dev->bus, dev->dev, dev->func);
    }

    pci_cleanup(pacc);   /* Close everything */
    return (vendor_id);
}

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
struct pci_dev *diag_pci_get_device (ushort vendor, ushort device, struct pci_dev *from)
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    static struct pci_dev dev_last;
    unsigned int  bus_tmp = 0xFE;
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
            /* if there are multiple PCIe device with the same vid and did
             * always choose the smaller bus number for PCIe switch case 
             */
            if (bus_tmp > dev->bus) {
                bus_tmp = dev->bus; 
                found_dev = TRUE;
                memcpy(&dev_last, dev, sizeof(struct pci_dev));
            } else {
               /*  printf("Skip a device with bus=%d\n", dev->bus);  */
            }
        }
    }

    pci_cleanup(pacc);   /* Close everything */

    if (found_dev == TRUE) {
        return &dev_last; 
    } else {
        return NULL;
    }
}

/*
 * Function: get_pci_bus_num
 *
 * Description : using vendor id and device id to get the pci bus number
 *
 * Inputs: vendor - PCI vendor id to match, or PCI_ANY_ID to match all vendor ids
 *         device - PCI device id to match, or PCI_ANY_ID to match all device ids
 *
 * Output: bus number 
 *                 
 */
ushort get_pcie_bus_num (ushort vendor, ushort device)
{
    struct pci_dev *dev;
    ushort bus_num;

    dev = diag_pci_get_device(vendor, device, NULL);

    if (dev == NULL) {
        return (UNKNOWN_PCI_BUS_NUM); /* return 0xFF to stand for error */
    } 

    bus_num = dev->bus;
    dev = NULL;

    return (bus_num);
}

/*
 * Function: get_pci_bus_num3
 *
 * Description : using vendor id , device id and instance to get the pci bus number
                 Curie 2RU has 2 BCM57412 10G MACs on different buses.
 *
 * Inputs: vendor - PCI vendor id to match, or PCI_ANY_ID to match all vendor ids
 *         device - PCI device id to match, or PCI_ANY_ID to match all device ids
 *         inst - The inst-th instance that matched, 0 or 1 return the first instance
 *
 * Output: bus number
 *
 */
ushort get_pcie_bus_num3 (ushort vendor, ushort device, ushort inst)
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    ushort bus_num = UNKNOWN_PCI_BUS_NUM;

    pacc = pci_alloc(); /* Get the pci_access structure */
    pci_init(pacc);     /* Initialize the PCI library */
    pci_scan_bus(pacc); /* We want to get the list of devices */

    for (dev=pacc->devices; dev; dev=dev->next) /* Iterate over all devices */
    {
        /* Fill in header info we need */
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

        /* Read config register directly */
        pci_read_byte(dev, PCI_INTERRUPT_PIN);

        if ((dev->vendor_id == vendor) && (dev->device_id == device) &&
            --inst < 1) {
            bus_num = dev->bus;
            break;
        }
    }

    pci_cleanup(pacc);   /* Close everything */

    return bus_num;
}

/*------------------------------------------------------------------
$Log: linux_pciutils.c,v $
Revision 1.9  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

Revision 1.8  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.7  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.6.52.2  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.6.52.1  2017/06/01 00:49:34  alpeng
update pci util to return bus smaller bus number

Revision 1.6  2013/12/18 05:25:26  hroni
fix invalid bus no issue

Revision 1.5  2013/11/20 08:46:18  alpeng
find out the minimal bus number for pci device

Revision 1.4  2013/11/18 07:27:43  alpeng
support get_pci_bus_num()

Revision 1.3  2013/11/05 09:23:54  danchung
Remove debug message

Revision 1.2  2013/11/01 07:02:59  alpeng
support is_juno_plx()

Revision 1.1  2013/10/16 02:18:03  alpeng
support pci_get_device() from pciutils package

$Endlog$
*/


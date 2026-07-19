/* $Id: nim_testcard.c,v 1.3 2016/05/19 05:55:29 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/nim_testcard.c,v $
 *------------------------------------------------------------------
 * Filename:   nim_testcard.c
 *
 * Description: intel nim testcard diag entry
 *
 * Copyright (c) 2015-2016 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linux_pciutils.h"
#include "nim_test_defs.h"

extern char *optarg;
extern unsigned int pci_config_read(unsigned int, unsigned short, unsigned int, int);

/*
 * Function: testcard_pcie_linkup_test
 *
 * Description : testcard pcie linkup test
 *
 * Inputs: slot - slot number
 *
 * Output: test result.
 *
 */
int testcard_pcie_linkup_test (int slot) 
{   
    unsigned int data; 
    unsigned short vendor_id = 0;
    unsigned short device_id = 0;
    unsigned char bus, dev, func;

    /* check testcard pcie sw link up state instead of 
     * platfrom pericom pcie sw, pass pcie vid, did directly
     */
    vendor_id = TESTCARD_PCIE_SW_VID;
    device_id = TESTCARD_PCIE_SW_DID;
    
    data = get_pcie_info(vendor_id, device_id, &bus, &dev, &func);
    printf("get_pcie_info bus = 0x%x, dev = 0x%x, func = 0x%x\n", bus, dev, func);

    if (data & 0xFF) {
        printf("Failed - Cannot detect NIM testcard pcie sw\n");
        return (1);
    }

    /* 0x78 for PLX pcie sw  Link status register */
    /* bit[25:20] for Link width */
    data = pci_config_read(bus, dev, func, 0x78);

#if 0
    printf("Link status, control   = %#.4x, %#.4x\n",
         (data & PCI_EXP_LINK_STATUS_MASK) >> PCI_EXP_LINK_STATUS_SHIFT,
         (data & PCI_EXP_LINK_CTRL_MASK));
#endif 
    
    data = ((data >> 20) & 0x3F); 
    if (data != 0) {
    /* port link up */
        printf("PASSED\n");
        return (0);
    } else {
    /* port link up */
        printf("FAILED\n");
        return (1);
    }
}

/*
 * Function: tc_pcie_test
 *
 * Description : testcard pcie test entry. 
 *
 * Inputs: slot - slot number
 *
 * Output: test result. 
 *
 */
int tc_pcie_test (int slot) {

    return (testcard_pcie_linkup_test(slot));
}

/*
 * Function: tc_pcie_utils
 *
 * Description : function for testcard pcie utils, dummy so far 
 *
 * Inputs: slot - slot number
 *
 * Output: NONE
 *
 */
int tc_pcie_utils (int slot) {

    printf("dummy\n");
    return (0);
}

/*------------------------------------------------------------------
$Log: nim_testcard.c,v $
Revision 1.3  2016/05/19 05:55:29  alpeng
fix error message

Revision 1.2  2016/04/20 08:54:00  benchen2
add tachi fru portion

Revision 1.1.2.1  2016/01/29 02:23:39  alpeng
rename files

Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code

Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

$Endlog$
*/


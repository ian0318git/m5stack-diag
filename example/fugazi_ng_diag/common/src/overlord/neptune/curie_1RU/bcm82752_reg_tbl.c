/* $Id: bcm82752_reg_tbl.c,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm82752_reg_tbl.c,v $
*-----------------------------------------------------------------------------
* bcm82752_reg_tbl.c - Register table for BCM 10G PHY bcm82752.
*          Leverage from KP
*
* Feb 2019, Leschen
*
* Copyright (c) 2019 by Cisco Systems, Inc.
* All rights reserved.
*-----------------------------------------------------------------------------
*/
#include "bcm82752_api.h"
#include "bcm82752_reg_def.h"

phy_reg_tbl_t bcm82752_global_reg_tbl[] = {
    {"BCM82752_PMD_CONTROL_REG", 0x0000, BCM82752_DEV_DEFAULT},
    {"BCM82752_PMD_STATUS_REG",  0x0001, BCM82752_DEV_DEFAULT},
    //...
};

uint32_t bcm82752_global_reg_tbl_size = sizeof(bcm82752_global_reg_tbl)/sizeof(bcm82752_global_reg_tbl[0]);

phy_reg_tbl_t bcm82752_port_line_reg_tbl[] = {
    {"BCM82752_RX_ALARM_CONTROL_REG", 0x9000, BCM82752_DEV_DEFAULT},
    //...
};

uint32_t bcm82752_port_line_reg_tbl_size = sizeof(bcm82752_port_line_reg_tbl)/sizeof(bcm82752_port_line_reg_tbl[0]);

phy_reg_tbl_t bcm82752_port_host_reg_tbl[] = {
    {"BCM82752_PMD_AND_PCS_STATUS_REG", 0xCD04, BCM82752_DEV_DEFAULT},
    //...
};

uint32_t bcm82752_port_host_reg_tbl_size = sizeof(bcm82752_port_host_reg_tbl)/sizeof(bcm82752_port_host_reg_tbl[0]);

/*-------------------------------------------------
$Log: bcm82752_reg_tbl.c,v $
Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/03/12 07:41:51  leschen
Initial check in to support BCM82752


$Endlog$
*/

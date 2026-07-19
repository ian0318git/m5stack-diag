/* $Id: m2_testcard_host_impl.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/m2_testcard_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * m2_testcard_host_impl.c - M.2 testcard host provide.
 *
 * Apr 2021, Ian Chang <iachang@cisco.com>
 *
 * Copyright (c) 2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "common.h"
#include "dash_fpga.h"
#include "platform_i2c.h"
#include "linux_pciutils.h"
#include "m2_testcard_host_impl.h"


/*******************************************************************************
 *
 * Function   :	is_m2_testcard_in
 * Description:	Detect M.2 testcard present bit via FPGA
 * Inputs     :	None
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
boolean is_m2_testcard_in (void)
{
    /* check FPGA M.2 testcard present bit
     * M2_MODULE_PRESENT_BIT  = 1 means M.2 slot insert card
     * M2_PCIE_PRESENT_BIT    = 1 &&
     * M2_USB_2p0_PRESENT_BIT = 1 means this card is test card
     */
    uint rdval = 0;
    dash_fpga_reg_read(M2_MODULE_STS_CTL_REG, &rdval);

    if ((rdval & M2_MODULE_PRESENT_BIT) &&
        (rdval & M2_PCIE_PRESENT_BIT)   &&
        (rdval & M2_USB_2p0_PRESENT_BIT)) {
        return (TRUE);
    } else {
        printf("M.2 test card is not present\n");
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   :	m2_tc_host_get_m2_pcie_config
 * Description:	Provided PCIe config information to M.2 test card
 * Inputs     :	bus   : pcie bus number
 *              dev   : pcie device number
 *              fn    : pcie function number
 *              speed : pcie speed
 *              width : pcie width value
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int m2_tc_host_get_m2_pcie_config(int *bus, int *dev, int *fn,
                                  int *speed, int *width)
{
    *bus   = 0x19;
    *dev   = 0;
    *fn    = 0;
    *speed = PCI_EXP_LINK_STA_SPD_8GT;
    *width = PCI_EXP_LINK_STA_WID_4;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	m2_tc_host_get_nvme_dev
 * Description:	This function provide M.2 testcard nvme device name with Fugazi
 * Inputs     :	dev_name - buffer to put device name
 *              size - buffer size
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int m2_tc_host_get_nvme_dev(char *dev_name, int size)
{
    char *str="/dev/nvme0n1";
    if (size < sizeof(str)) {
        printf("Memory overflow risk!!!Please check the dev_name\n");
        return (FAILED);
    }
    strcpy(dev_name,str);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	m2_tc_host_get_eusb_dev
 * Description:	This function provide M.2 testcard eusb device name with Fugazi
 * Inputs     :	dev_name - buffer to put device name
 *              size - buffer size
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int m2_tc_host_get_eusb_dev(char *dev_name, int size)
{
    char *str="/dev/m2usb";
    if (size < sizeof(str)) {
        printf("Memory overflow risk!!!Please check the dev_name\n");
        return (FAILED);
    }
    strcpy(dev_name, str);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	m2_tc_host_get_i2c_dev
 * Description:	This function provide M.2 testcard i2c interface information
 * Inputs     :	i2c_ctrl - I2C bus control value
 *              i2c_mux  - I2C bus MUX value
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int m2_tc_host_get_i2c_dev(uint8_t *i2c_ctrl, uint8_t *i2c_mux)
{
    *i2c_ctrl = I2C_CTRL_ONE;
    *i2c_mux = I2C_MUX_ONE;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	m2_tc_power_control
 * Description:	Enable & Disable M.2 testcard power
 * Inputs     :	power_enable : enable / disable
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int m2_tc_power_control(int power_enable)
{
    uint32_t rdval = 0;
    dash_fpga_reg_read(M2_MODULE_STS_CTL_REG, &rdval);

    /* if register bit0=1 enable power, bit0=0 disable power */
    if (power_enable) {
        rdval |= M2_POWER_BIT;
        printf("Enable power to module\n");
    } else{
        rdval &= (~M2_POWER_BIT);
        printf("Disable power to module\n ");
    }
    dash_fpga_reg_write(M2_MODULE_STS_CTL_REG, rdval);

    return (PASSED);
}
/*
 *------------------------------------------------------------------
 * $Log: m2_testcard_host_impl.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.2.1  2021/04/26 08:15:25  iachang
 * CSCvy10910:Fugazi Diag supportted M.2 test card
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

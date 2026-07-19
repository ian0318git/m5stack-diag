/* $Id: m2_testcard_host_impl.c,v 1.1 2021/05/13 08:49:58 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/m2_testcard_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * m2_testcard_host_impl.c - M.2 testcard host provide.
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
#include "platform_i2c.h"
#include "linux_pciutils.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"
#include "m2_testcard_host_impl.h"

/**********************************************************************
 *
 * Function: is_m2_testcard_in 
 *
 * Description: Check if M.2 testcard is presented or not
 *
 * Input : None
 *                     
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean is_m2_testcard_in (void)
{
    /* check FPGA M.2 testcard present bit
     * FPGA_M2_MODULE_PRESENT  = 1 means M.2 slot insert card
     * FPGA_M2_PCIE_PRESENT    = 1 &&
     * FPGA_M2_USB_PRESENT = 1 means this card is test card
     */
    uint rdval = 0;
    fpga_read_reg(FPGA_M2_CTLSTS_REG, &rdval);

    if ((rdval & FPGA_M2_MODULE_PRESENT) &&
        (rdval & FPGA_M2_PCIE_PRESENT)   &&
        (rdval & FPGA_M2_USB_PRESENT)) {
        return (TRUE);
    } else {
        printf("M.2 test card is not present\n");
        return (FALSE);
    }
}

/**********************************************************************
 *
 * Function: m2_tc_host_get_m2_pcie_config  
 *
 * Description: Get M.2 testcard platform PCIE bus configuration
 *
 * Input : *bus   - bus number
 *         *dev   - device number
 *         *fn    - function number 
 *         *speed - PCIE speed
 *         *width - PCIE width
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int m2_tc_host_get_m2_pcie_config(int *bus, int *dev, int *fn,
                                  int *speed, int *width)
{
    *bus   = M2_PCIE_BUS2;
    *dev   = 0;
    *fn    = 0;
    *speed = PCI_EXP_LINK_STA_SPD_2DOT5;
    *width = PCI_EXP_LINK_STA_WID_2;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: m2_tc_host_get_nvme_dev
 *
 * Description: Get M.2 testcard NVME device node name
 *
 * Input : *dev_name - device name
 *         size      - size of device name
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int m2_tc_host_get_nvme_dev(char *dev_name, int size)
{
    char *str="/dev/m2nvme1";
    if (size < sizeof(str)) {
        printf("Memory overflow risk!!!Please check the dev_name\n");
        return FAILED;
    }
    strcpy(dev_name,str);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: m2_tc_host_get_eusb_dev 
 *
 * Description: Get M.2 testcard USB device node name
 *
 * Input : *dev_name - device name
 *         size      - size of device name
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int m2_tc_host_get_eusb_dev(char *dev_name, int size)
{
    char *str="/dev/m2usb";
    if (size < sizeof(str)) {
        printf("Memory overflow risk!!!Please check the dev_name\n");
        return FAILED;
    }
    strcpy(dev_name, str);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: m2_tc_host_get_i2c_dev
 *
 * Description: Get M.2 testcard FPGA I2C ctrl and mux number
 *
 * Input : *i2c_ctrl - FPGA ctrl number
 *         *i2c_mux  - FPGA i2c mux number
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int m2_tc_host_get_i2c_dev(uint8_t *i2c_ctrl, uint8_t *i2c_mux)
{
    *i2c_ctrl = I2C_CTRL_ONE;
    *i2c_mux = I2C_MUX_ONE;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: m2_tc_power_control
 *
 * Description: M.2 testcard power control
 *
 * Input : power_enable - enable or disable power
 *                     
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int m2_tc_power_control(int power_enable)
{
    uint32_t rdval = 0;
    fpga_read_reg(FPGA_M2_CTLSTS_REG, &rdval);

    /* if register bit0=1 enable power, bit0=0 disable power */
    if (power_enable) {
        rdval |= M2_POWER_BIT;
        printf("Enable power to module\n");
    } else{
        rdval &= (~M2_POWER_BIT);
        printf("Disable power to module\n ");
    }
    fpga_write_reg(FPGA_M2_CTLSTS_REG, rdval);

    return (PASSED);
}
 
/*-------------------------------------------------
 * $Log: m2_testcard_host_impl.c,v $
 * Revision 1.1  2021/05/13 08:49:58  kodko
 * Support M.2 testcard.
 *
 * $Endlog$
 *-------------------------------------------------
 */

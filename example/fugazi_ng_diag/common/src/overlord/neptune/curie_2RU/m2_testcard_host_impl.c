/*------------------------------------------------------------------
 *
 * m2_testcard_host_impl.c - M.2 testcard host provide.
 *
 * Feb 2021, Xiaolan Yang <xiaolaya@cisco.com>
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

#define M2_POWER_BIT         (0x1 << 0)

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
        return (FALSE);
    }
}

int m2_tc_host_get_m2_pcie_config(int *bus, int *dev, int *fn,
                                  int *speed, int *width)
{
    *bus   = 0x3b;
    *dev   = 0;
    *fn    = 0;
    *speed = PCI_EXP_LINK_STA_SPD_2DOT5;
    *width = PCI_EXP_LINK_STA_WID_2;
    return PASSED;
}

int m2_tc_host_get_nvme_dev(char *dev_name, int size)
{
    char *str="/dev/m2nvme01";
    if (size < sizeof(str)) {
        printf("Memory overflow risk!!!Please check the dev_name\n");
        return FAILED;
    }
    strcpy(dev_name,str);
    return PASSED;
}

int m2_tc_host_get_eusb_dev(char *dev_name, int size)
{
    char *str="/dev/m2eusb";
    if (size < sizeof(str)) {
        printf("Memory overflow risk!!!Please check the dev_name\n");
        return FAILED;
    }
    strcpy(dev_name, str);
    return PASSED;
}

int m2_tc_host_get_i2c_dev(uint8_t *i2c_ctrl, uint8_t *i2c_mux)
{
    *i2c_ctrl = I2C_CTRL_ONE;
    *i2c_mux = I2C_MUX_ONE;
    return PASSED;
}

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

    return PASSED;
}

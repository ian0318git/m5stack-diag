
/*------------------------------------------------------------------
 *
 * highrise.c - specific APIs for highrise platform
 *
 * May 2019, markzha
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <unistd.h>
#include "gpio.h"
#include "highrise.h"
#include "common.h"
#include "highrise_cpld_api.h"
#include "diag_lte_telit_lib.h"

extern int highrise_init_i2c(void);
extern int highrise_init_phy_device(void);
extern int highrise_config_ts_init(void);
extern int hr_cpld_init_default(int flag);
extern int phy_enable_temperature(void);


int highrise_init()
{
    int rc = 0;

    /*GPIO init*/
    rc = gpio_export(CPLD_CPU_INT_L);
    INFRA_ERR_HANDLE("Export CPLD_CPU_INT_L failed", rc, 1);
    rc = gpio_direction(CPLD_CPU_INT_L, IN);
    INFRA_ERR_HANDLE("Set CPLD_CPU_INT_L direction failed", rc, 1);

    rc = gpio_export(USB_MUX_DEBUG_EN);
    INFRA_ERR_HANDLE("Export USB_MUX_DEBUG_EN failed", rc, 1);
    rc = gpio_direction(USB_MUX_DEBUG_EN, OUT);
    INFRA_ERR_HANDLE("Set USB_MUX_DEBUG_EN direction failed", rc, 1);

    rc = gpio_export(THERM_CPU_INT_L);
    INFRA_ERR_HANDLE("Export THERM_CPU_INT_L failed", rc, 1);
    rc = gpio_direction(THERM_CPU_INT_L, IN);
    INFRA_ERR_HANDLE("Set THERM_CPU_INT_L direction failed", rc, 1);

    rc = gpio_export(SIM0_DETECT_L);
    INFRA_ERR_HANDLE("Export SIM0_DETECT_L failed", rc, 1);
    rc = gpio_direction(SIM0_DETECT_L, IN);

    rc = gpio_export(SIM1_DETECT_L);
    INFRA_ERR_HANDLE("Export SIM1_DETECT_L failed", rc, 1);
    rc = gpio_direction(SIM1_DETECT_L, IN);

    rc = gpio_export(DDR4_CPU_ALERT_L);
    INFRA_ERR_HANDLE("Export DDR4_CPU_ALERT_L failed", rc, 1);
    rc = gpio_direction(DDR4_CPU_ALERT_L, IN);

    rc = gpio_export(CPU_TO_CPLD_STATUS_0);
    INFRA_ERR_HANDLE("Export CPU_TO_CPLD_STATUS_0 failed", rc, 1);
    rc = gpio_direction(CPU_TO_CPLD_STATUS_0, OUT);

    rc = gpio_export(CPU_TO_CPLD_STATUS_1);
    INFRA_ERR_HANDLE("Export CPU_TO_CPLD_STATUS_0 failed", rc, 1);
    rc = gpio_direction(CPU_TO_CPLD_STATUS_1, OUT);

    rc = highrise_init_i2c();
    INFRA_ERR_HANDLE("Error: fail to open I2C device", rc, 1);

    rc = hr_cpld_unreset_act2();
    INFRA_ERR_HANDLE("Error: fail to unrest ACT2 chip", rc, 1);

    rc = highrise_config_ts_init();
    INFRA_ERR_HANDLE("Error: fail to config TMP75 resolution", rc, 1);

    rc = highrise_init_phy_device();
    INFRA_ERR_HANDLE("Error: fail to init PHY device", rc, 1);

    rc = phy_enable_temperature();
    INFRA_ERR_HANDLE("Error: fail to enable PHY temp sensor", rc, 1);

    rc = hr_cpld_init_default(0);
    INFRA_ERR_HANDLE("Error: fail to init cpld.", rc, 1);

    return (0);

}


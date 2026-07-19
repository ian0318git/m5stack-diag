/* $Id: hightower_sub6.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/hightower_sub6.c,v $
 *********************************************************************
 *
 * hightower_sub6.c - platform entry
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "menu.h"
#include "error.h"
#include "hightower_sub6.h"
#include "gpio.h"
#include "highrise_cpld_api.h"
#include "hightower_5g_modem_lib.h"
#include "highrise_cpld_lib.h"

extern int highrise_init_i2c(void);
extern int highrise_init_phy_device(void);
extern int highrise_config_ts_init(void);
extern int hr_cpld_init_default(int flag);
extern int phy_enable_temperature(void);
extern int ht_modem_test(boolean); 
extern int ht_cpld_show_poe_info(void); 
extern int hr_cpld_get_boardid(uint8_t *, char *);

/*********************************************************************
 * Function: ht_init 
 * Description: a init function for hightower, 
 *              we seperate with highrise due to the gpio 
 *              base offset is different with new SDK. 
 * Inputs: dummy 
 * Outputs: None
 * Note: before highrise code ready, hightower using this function 
 *       as an entry for diag code development. 
 *********************************************************************
 */
int ht_init()
{
    int rc = 0;
    uint8_t id; 
    char name[32]; 

    /* enable overcommit memory for system; 
     * setup on rootfs makes people forget easily */
   
    /* 5.5.2020 - overcommit ratio is based on full memory size. 
     * we plan to use get_mem_overhead_factor() which is based 
     * on free memory size. This change is more accurrable 
     * since eio project has small memory size. */
    /* system("echo 2 >  /proc/sys/vm/overcommit_memory"); 
       system("echo 35 >  /proc/sys/vm/overcommit_ratio");
     */

    /*GPIO init*/
    rc = gpio_export(CPLD_CPU_INT_L);
    INFRA_ERR_HANDLE("Export CPLD_CPU_INT_L failed", rc, FALSE);
    rc = gpio_direction(CPLD_CPU_INT_L, IN);
    INFRA_ERR_HANDLE("Set CPLD_CPU_INT_L direction failed", rc, FALSE);

    rc = gpio_export(USB_MUX_DEBUG_EN);
    INFRA_ERR_HANDLE("Export USB_MUX_DEBUG_EN failed", rc, FALSE);
    rc = gpio_direction(USB_MUX_DEBUG_EN, OUT);
    INFRA_ERR_HANDLE("Set USB_MUX_DEBUG_EN direction failed", rc, FALSE);

    rc = gpio_export(THERM_CPU_INT_L);
    INFRA_ERR_HANDLE("Export THERM_CPU_INT_L failed", rc, FALSE);
    rc = gpio_direction(THERM_CPU_INT_L, IN);
    INFRA_ERR_HANDLE("Set THERM_CPU_INT_L direction failed", rc, FALSE);

    rc = gpio_export(SIM0_DETECT_L);
    INFRA_ERR_HANDLE("Export SIM0_DETECT_L failed", rc, FALSE);
    rc = gpio_direction(SIM0_DETECT_L, IN);

    rc = gpio_export(SIM1_DETECT_L);
    INFRA_ERR_HANDLE("Export SIM1_DETECT_L failed", rc, FALSE);
    rc = gpio_direction(SIM1_DETECT_L, IN);

    rc = gpio_export(DDR4_CPU_ALERT_L);
    INFRA_ERR_HANDLE("Export DDR4_CPU_ALERT_L failed", rc, FALSE);
    rc = gpio_direction(DDR4_CPU_ALERT_L, IN);

    rc = gpio_export(CPU_TO_CPLD_STATUS_0);
    INFRA_ERR_HANDLE("Export CPU_TO_CPLD_STATUS_0 failed", rc, FALSE);
    rc = gpio_direction(CPU_TO_CPLD_STATUS_0, OUT);

    rc = gpio_export(CPU_TO_CPLD_STATUS_1); 
    INFRA_ERR_HANDLE("Export CPU_TO_CPLD_STATUS_0 failed", rc, FALSE);
    rc = gpio_direction(CPU_TO_CPLD_STATUS_1, OUT);

    rc = gpio_export(SIM_SELECT);
    INFRA_ERR_HANDLE("Export SIM_SELECT failed", rc, FALSE);
    rc = gpio_direction(SIM_SELECT, OUT);
    INFRA_ERR_HANDLE("Set SIM_SELECT direction failed", rc, FALSE);

    rc = highrise_init_i2c();
    INFRA_ERR_HANDLE("Error: fail to open I2C device", rc, FALSE);

    rc = hr_cpld_unreset_act2();
    INFRA_ERR_HANDLE("Error: fail to unrest ACT2 chip", rc, FALSE);

    rc = highrise_config_ts_init();
    INFRA_ERR_HANDLE("Error: fail to config TMP75 resolution", rc, FALSE);

    rc = highrise_init_phy_device();
    INFRA_ERR_HANDLE("Error: fail to init PHY device", rc, FALSE);

    rc = phy_enable_temperature();
    INFRA_ERR_HANDLE("Error: fail to enable PHY temp sensor", rc, FALSE);

    rc = hr_cpld_init_default(0);
    INFRA_ERR_HANDLE("Error: fail to init cpld.", rc, FALSE);

    rc = ht_cpld_show_poe_info();
    INFRA_ERR_HANDLE("Error: fail to display PoE status.", rc, FALSE);

    rc = hr_cpld_get_boardid(&id, name); 
    if (id != HR_CPLD_BOARD_HIGHTOWER_SUB6) { 
        printf("CPLD board id is 0x%d, %s, not HIGHTOWER_SUB6, exit diag\n", 
                id, name); 
        exit(-1); 
    }

    return (0);

}

int highrise_toggle_act2_i2c_reset(void)
{
    return (PASSED);
}

/*********************************************************************
 * $Log: hightower_sub6.c,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.4  2020/12/09 07:29:50  alpeng
 * add function prologue; remove redundant header; adding ifdef for header files;
 *
 * Revision 1.1.4.3  2020/09/10 07:02:08  alpeng
 * fixed typo
 *
 * Revision 1.1.4.2  2020/09/10 06:03:16  alpeng
 * add board id check before launch diag
 *
 * Revision 1.1.4.1  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */


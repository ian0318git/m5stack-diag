/* $Id: hightower_mmwv.c,v 1.2 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/hightower_mmwv.c,v $
 *********************************************************************
 *
 * hightower_mmwv.c - specific APIs for hightower_mmwv platform
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
#include "hightower_mmwv.h"
#include "gpio.h"
#include "highrise_cpld_api.h"
#include "hightower_5g_modem_lib.h"
#include "highrise_cpld_lib.h"

extern int highrise_init_i2c(void);
extern int highrise_init_phy_device(void);
extern int highrise_config_ts_init(void);
extern int hr_cpld_init_default(int flag);
extern int phy_enable_temperature(void);
extern int ht_plug_lte_util(void);
extern int modem_bootup_msg (void);
extern int ht_cpld_show_poe_info(void);
extern int hr_cpld_get_boardid(uint8_t *, char *);
extern int ht_cpld_show_antenna_info(void);
extern swi_5g_modem_usb_config_t diag_5g_swi_usb_cfg;

#if 0 //Thiru moving the modem specific test to modem files.
/* =========================================
 *  Main menu items
 * ========================================= */
static submenu_xtable_t ht_modem_menu_table[] = {
    {"Hightower mmwv modem test",
    (PFT) ht_modem_test, TRUE,
    MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) ht_modem_test, FALSE},

    {"Hightower mmwv modem util",
    (PFT) ht_plug_lte_util, TRUE, 0, 
    (type_t(*)())0, 0,
    (PFT) ht_plug_lte_util, FALSE},

};

#define HT_MODEM_TABLE_SIZE \
        (sizeof(ht_modem_menu_table) / sizeof(submenu_xtable_t))
/*
 *  * Primary & secondary submenu items (filled in from xtable)
 *   */
static mitem_t ht_modem_menu_primary_items[HT_MODEM_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t ht_modem_menu_secondary_items[HT_MODEM_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo htmodemdiag = {
    "Hightower Modem Main %s",  /* title */
    0,              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,  /* shows major flags */
    0,              /* generic prompt */
    0,              /* size -- bumped by add_menu_item() */
    ht_modem_menu_primary_items,
};
static struct menuinfo *htmodemdiagp = &htmodemdiag;

/*********************************************************************
 * Function: ht_modem_diag_menu
 * Description: This is the main entry to modem diag menu.
 * Inputs: dummy 
 * Outputs: None
 * Note: before highrise code ready, hightower using this function 
 *       as an entry for diag code development. 
 *********************************************************************
 */
int ht_modem_diag_menu (int db_test_items_executed) 
{
    int ret;
    int modem_found = FALSE;

    /* Store all USB port info that LTE modem might use from platform*/

    diag_5g_swi_store_usb_devinfo();


#if 0 //Thiru Modem testing
    diag_modem_reset_pin_ctrl(HIGH);

    diag_modem_pwr_ctrl(TRUE);

    diag_swi_5g_insmod(TRUE);

    //Thiru : Check for modem presence here... <PCIe interface>
    fflush(stdout);
#endif //Thiru modem testing

//    modem_found = diag_swi_5g_modem_usb_detect(diag_5g_swi_usb_cfg.usb_devinfo,
//                        MODEM_SWI_USB_VID, USB2P0_SPEED);

    //Thiru : Check for modem presence here... <PCIe interface>
    modem_found = diag_swi_5g_modem_pci_detect(MODEM_SWI_PCI_BUS_NUM,
                        MODEM_SWI_PCI_VID, PCI_GEN2_SPEED);

    if (modem_found == PASSED) {
        printf ("Modem found ");
    } else {
        cterr('f', 0, "SWI Modem is not detected");
        ret = FAILED;
        goto __exit;
    }

    ret = modem_bootup_msg();
    if (ret == FAILED) goto __exit;


    build_primary_submenu(ht_modem_menu_table, HT_MODEM_TABLE_SIZE, dgmenustr,
        &htmodemdiagp);
    build_secondary_submenu(ht_modem_menu_table, HT_MODEM_TABLE_SIZE,
        ht_modem_menu_secondary_items);

    if (db_test_items_executed) {
        do_all_menu_items(&htmodemdiag);
    } else {
        menu(&htmodemdiag, ht_modem_menu_secondary_items, '\0');
    }

__exit:
#if 0 //Thiru modem testing
    diag_modem_pwr_ctrl(FALSE);

    diag_swi_5g_insmod(FALSE);
#endif //Thiru modem testing

    return (PASSED); 
}
#endif //Thiru moving the modem specific test to modem files.

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
    if (id != HR_CPLD_BOARD_HIGHTOWER_MMWV) { 
        printf("CPLD board id is 0x%d, %s, not HIGHTOWER_MMWV, exit diag\n", 
                id, name); 
        exit(-1); 
    }

    rc = ht_cpld_show_antenna_info();
    INFRA_ERR_HANDLE("Error: fail to display Modem status.", rc, FALSE);

    /* enable overcommit memory for system; 
     * setup on rootfs makes people forget easily */
    system("echo 2 >  /proc/sys/vm/overcommit_memory"); 
    system("echo 40 >  /proc/sys/vm/overcommit_ratio");
    return (0);

}

int highrise_toggle_act2_i2c_reset(void)
{
    return (PASSED);
}

/*********************************************************************
 * $Log: hightower_mmwv.c,v $
 * Revision 1.2  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.4  2021/05/12 17:54:23  tshanmug
 * Chrysler infra for SIM1 test
 *
 * Revision 1.1.4.3  2020/10/06 01:56:33  alpeng
 * display modem antenna id before menu prompt
 *
 * Revision 1.1.4.2  2020/09/10 05:53:52  alpeng
 * add board compare before diag menu launch
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */


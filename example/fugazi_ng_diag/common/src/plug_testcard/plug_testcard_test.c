/* $Id: plug_testcard_test.c,v 1.8 2021/09/24 01:27:20 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_test.c,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_test.c - PLUGGABLE Test Card Main Functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "proto.h"
#include "menu.h"
#include "plug_slot.h"
#include "plug_common_host_impl.h"
#include "plug_common_lib.h"
#include "plug_testcard_util.h"
#include "plug_testcard_gpio_exp_lib.h"
#include "plug_temp_sensor_test.h"
#include "plug_testcard_phy.h"
#include "plug_testcard_test.h"
#include "plug_testcard_host.h"
#include "plug_testcard_host_impl.h"
#include "plug_testcard_usb_lib.h"
#include "plug_common_host.h"
#include "plug_host_fpga_lib.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "cookie_4.h"
#include "linux_pciutils.h"

extern int get_pcie_cap_struct_ptr (uint32_t bus, uint16_t dev, int fn, uint reg);
extern int get_pcie_link_cap (uint32_t bus, uint16_t dev, int fn, uint reg);
extern int get_pcie_link_status (uint32_t bus, uint16_t dev, int fn, uint reg);
int plug_testcard_main(void *);

boolean tc_has_pcie = FALSE;

static int plug_testcard_ts_test(int);
static int plug_testcard_gpio_exp_test(int);
static int plug_testcard_uart_test(int);
static int plug_testcard_module_reset_pin_test(int);
static int plug_testcard_i2c_reset_pin_test(int);
static int plug_testcard_gps_test(int);
static int plug_testcard_nvme_test(void);
static boolean plug_tc_sgmii_present(void);
static boolean plug_tc_pcie_present(void);
extern int do_all_menu_items(struct menuinfo *);
int plug_testcard_usb_test(int);
int plug_testcard_usb_hub_test(int);
static int plug_tc_usb_slot_tests(int);
static int plug_tc_access_device_test(char *);
extern int do_all_menu_items(struct menuinfo *);
int plug_testcard_usb_test(int);
int plug_testcard_usb_hub_test(int);
static int plug_tc_usb_slot_tests(int);
static int plug_tc_access_device_test(char *);

extern struct usb_info_t plug_usb[MAX_PLUG_USB_MASS_STORE_ON_SYS];
static int plug_tc_quiet_launch = 0;

static submenu_xtable_t pluggable_testcard_table[] = {
    {"GPIO Expander Test", (type_t(*)())plug_testcard_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Temperature Sensor Test", (type_t(*)())plug_testcard_ts_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_tc_sgmii_present, 0, (type_t(*)())0, 0},
    {"USB Test", (type_t(*)())plug_testcard_usb_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_tc_host_usb_hub_menu_flag, FALSE, (type_t(*)())0, 0},
    {"USB Test with 3P0 HUB", (type_t(*)())plug_testcard_usb_hub_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_tc_host_usb_hub_menu_flag, TRUE, (type_t(*)())0, 0},
    {"UART Test", (type_t(*)())plug_testcard_uart_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SGMII Loopback Test", (type_t(*)())plug_testcard_sgmii_loopback_test, FALSE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_tc_host_sgmii_present, 0, (type_t(*)())plug_testcard_sgmii_loopback_test, TRUE},
    {"GPS pin Test", (type_t(*)())plug_testcard_gps_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"I2C Reset pin Test", (type_t(*)())plug_testcard_i2c_reset_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"NVMe SSD Test", (type_t(*)())plug_testcard_nvme_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_tc_pcie_present, 0, (type_t(*)())0, 0},
    {"Module Reset pin Test", (type_t(*)())plug_testcard_module_reset_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Utilities", (type_t(*)())plug_testcard_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_TESTCARD_TEST_TABLE_SZ \
        (sizeof(pluggable_testcard_table) / sizeof(submenu_xtable_t))


static mitem_t plug_tc_pri_test_items[PLUG_TESTCARD_TEST_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t plug_tc_sec_test_items[PLUG_TESTCARD_TEST_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t plug_tc_test_menu = {
    "Pluggable Test Card Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_tc_pri_test_items,
};
static menuinfo_t *plug_tc_test_menup = &plug_tc_test_menu;

struct plug_intf_t *plug_test_if;

/*******************************************************************************
 * Function   : plug_tc_sgmii_present
 * Description: Check the TestCard which has SGMII to connect to host of not
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : TRUE or FALSE
 *******************************************************************************
 */
static boolean plug_tc_sgmii_present (void)
{
    if (plug_test_if->id == PLUGGABLE_TEST_CARD) {
        return (TRUE);
    }
    return (FALSE);
}

#ifndef PLUGGABLE_PCIE_TEST_CARD_OLD
#define PLUGGABLE_PCIE_TEST_CARD_OLD 0x1235
#endif
/*******************************************************************************
 * Function   : plug_tc_pcie_present
 * Description: Check the TestCard which has PCIe to connect to host of not
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : TRUE or FALSE
 *******************************************************************************
 */
static boolean plug_tc_pcie_present (void)
{
    if (plug_test_if->id == PLUGGABLE_PCIE_TEST_CARD) {
        return (TRUE);
    }

#ifdef PLUGGABLE_PCIE_TEST_CARD_OLD
    if (plug_test_if->id == PLUGGABLE_PCIE_TEST_CARD_OLD) {
        return (TRUE);
    }
#endif
    return (FALSE);
}

/*******************************************************************************
 * Function   : plug_testcard_main
 * Description: Main Entry point for Pluggable Test card
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_testcard_main (void *in)
{
    struct plug_intf_t *plug;

    /* Sanity check */
    if (in == NULL) {
        cterr('f', 0, "Null pointer");
        return (FAILED);
    }

    plug = (struct plug_intf_t *)in;
    plug_test_if = plug;

    if (plug_tc_pcie_present() == TRUE) {
        tc_has_pcie = TRUE;
    } else if (plug_tc_sgmii_present() == TRUE) {
        tc_has_pcie = FALSE;
    }

    /* unreset module and I2C bus */
    plug->unreset((void*)plug);
    plug->i2c_unreset((void*)plug);
    msleep(PLUG_TESTCARD_UNRESET_WAIT);
    
    if (tc_has_pcie == TRUE) {
        if (plug_testcard_pcie_post_pwr_up() == FAILED) {
            cterr('f', 0, "Workaround with PCIE power up fail");
            return (FAILED);
        }
    }

    build_primary_submenu(pluggable_testcard_table, PLUG_TESTCARD_TEST_TABLE_SZ, 
                         "Pluggable Test Card", &plug_tc_test_menup);

    build_secondary_submenu(pluggable_testcard_table, PLUG_TESTCARD_TEST_TABLE_SZ,
                            plug_tc_sec_test_items);

    if (plug->test_type == IFACE_TEST) {
        /* wait for USB stick enumerate */
        msleep(DELAY_USBCMD);
        do_all_menu_items(&plug_tc_test_menu);
        if (tc_has_pcie == TRUE) {
            plug_testcard_pcie_device_remove();
        }
        return (PASSED);
    }

    if (plug->menu_display) {
        menu(&plug_tc_test_menu, plug_tc_sec_test_items, '\0');
    } else {
        /* wait for USB stick enumerate */
        msleep(DELAY_USBCMD);
        do_all_menu_items(&plug_tc_test_menu);
    }
    if (tc_has_pcie == TRUE) {
        plug_testcard_pcie_device_remove();
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_testcard_gpio_exp_test
 * Description: GPIO Expander Test for Pluggable Test Card
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_gpio_exp_test (int input)
{
    dev_pca9557_object_t pca_data;
    dev_pca9557_object_t *pca_obj = &pca_data;
    int ret;

    testname("GPIO Expander");
    prpass(testpass, "Register Test");

    plug_tc_gpio_exp_dev_create(pca_obj);

    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret = pca_obj->callin_fvt->register_test((dev_object_t *)pca_obj);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);
}


/*******************************************************************************
 * Function   : plug_testcard_ts_test
 * Description: Thermal Sensor Test for Pluggable Test Card
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_ts_test (int input)
{
    testname("Thermal Sensor");
    prpass(testpass, "Register Test");

    return (plug_temp_sensor_reg_test());
}

/*******************************************************************************
 * Function   : plug_testcard_module_reset_pin_test
 * Description: Test pluggable test card module reset pin
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_module_reset_pin_test (int input)
{
    struct plug_intf_t *plug;
    int i2c_addr = PLUG_TC_I2C_CTRL_OFFSET; 
    uchar data_buf[32];

    int ret = FAILED;

    testname("Module Reset Pin");
    prpass(testpass, "Pin Test");
    plug = (struct plug_intf_t *)plug_test_if;

    if (plug->slot == PLUG_SLOT_2) {
        i2c_addr = i2c_addr + PLUG_TC_FPGA_I2C_OFFSET;
    }
    
    if (tc_has_pcie == TRUE) {
        plug_testcard_pcie_device_remove();
    }

    /* Have to temporary remove every driver that PIM test-card needed
     * before reset the PIM module. */
    if (plug_slot_reset(plug)) {
        cterr('f', 0, "Reset testcard slot %d fail", plug->slot);
        return (FAILED);
    }

    ret = plug_common_fpga_i2c_ack_check(i2c_addr,PLUG_FPGA_I2C_ACK_MUX,
             PLUG_FPGA_TC_I2C_ADDR_GPIO_EXP, PLUG_FPGA_I2C_ACK_REG_ADD,
             PLUG_FPGA_I2C_ACK_SUB_ADD, PLUG_FPGA_I2C_ACK_DATA_LEN, data_buf);


    /* ACK testing shoud not PASS when module reset */ 
    if (ret == PASSED) {
        ret = FAILED;
    } else {
        ret = PASSED;
    }

    /* Tabei-L HW suggest to check if NVMe is down. To avoid NVMe corrupt */
    if (plug_tc_host_check_nvme_existence(FALSE) == FAILED) {
        return (FAILED);
    }

    if (tc_has_pcie == TRUE) {
        if (plug_testcard_pcie_post_pwr_down() == FAILED) {
            cterr('f', 0, "Workaround with PCIE power down fail");
            return (FAILED);
        }
    }

    if (plug_slot_unreset(plug)) {
        cterr('f', 0, "Unreset testcard slot %d fail", plug->slot);
        return (FAILED);
    }
    msleep(PLUG_TESTCARD_UNRESET_WAIT);

    if (tc_has_pcie == TRUE) {
        if (plug_testcard_pcie_post_pwr_up() == FAILED) {
            cterr('f', 0, "Workaround with PCIE power up fail");
            return (FAILED);
        }
    }

    /* Tabei-L HW suggest to check if NVMe is up. To avoid NVMe corrupt */
    if (plug_tc_host_check_nvme_existence(TRUE) == FAILED) {
        return (FAILED);
    }

    if (ret == PASSED) {
        return (PASSED);
    } else {
        cterr('f', 0, "module reset ping test slot %d fail", plug->slot);
        return (FAILED);
    }
}


/*******************************************************************************
 * Function   : plug_testcard_i2c_reset_pin_test 
 * Description: Test pluggable test card module reset pin
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_i2c_reset_pin_test (int input)
{
    struct plug_intf_t *plug;
    int i2c_addr = PLUG_TC_I2C_CTRL_OFFSET; 
    uchar data_buf[32];

    int ret = FAILED;

    testname("Module Reset Pin");
    prpass(testpass, "Pin Test");
    plug = (struct plug_intf_t *)plug_test_if;
    if (plug->slot == PLUG_SLOT_2) {
        i2c_addr = i2c_addr + PLUG_TC_FPGA_I2C_OFFSET;
    }
    
    if (plug_slot_i2c_reset(plug)) {
        cterr('f', 0, "Reset testcard slot %d fail", plug->slot);
        return (FAILED);
    }
    
    ret = plug_common_fpga_i2c_ack_check(i2c_addr,PLUG_FPGA_I2C_ACK_MUX,
             PLUG_FGPA_TC_I2C_ADDR_ACT2, PLUG_FPGA_I2C_ACK_REG_ADD,
             PLUG_FPGA_I2C_ACK_SUB_ADD, PLUG_FPGA_I2C_ACK_DATA_LEN, data_buf);
    
    /* ACK testing shoud not PASS when module reset */ 
    if (ret == PASSED) {
        ret = FAILED;
    } else {
        ret = PASSED;
    }

    if (plug_slot_i2c_unreset(plug)) {
        cterr('f', 0, "Unreset testcard slot %d fail", plug->slot);
        return (FAILED);
    }
    msleep(PLUG_TESTCARD_UNRESET_WAIT);
    
    if (ret == PASSED) {
        return (PASSED);
    } else {
        cterr('f', 0, "module reset ping test slot %d fail", plug->slot);
        return (FAILED);
    }
}

/*******************************************************************************
 * Function   : plug_testcard_gps_test
 * Description: Test pluggable test card GPS pin
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_gps_test (int input)
{
    dev_pca9557_object_t pca_data;
    dev_pca9557_object_t *pca_obj = &pca_data;
    struct plug_intf_t *plug;
    int slot;
    uint data;

    testname("Module GPS Pin");
    prpass(testpass, "GPS Pin Test");

    plug = (struct plug_intf_t *)plug_test_if;
    slot = plug->slot;

    /* Configure GPIO expander pca9557 IO3 be output */
    plug_tc_gpio_exp_dev_create(pca_obj);
    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }
    pca_obj->callin_fvt->config_port((dev_object_t *)pca_obj,PORT_0, 
                                            PLUG_TESTCARD_GPS_PIN, PORT_DIR_OUTPUT );

    /* Enable GPS then check */
    pca_obj->callin_fvt->drive_port((dev_object_t *)pca_obj,PORT_0, 
                                           PLUG_TESTCARD_GPS_PIN, PORT_VAL_HIGH );

    plug_common_host_plug_fpga_reg_read(PLUG_FPGA_DBG_LED_ADDR_REG, &data);

    if (slot == PLUG_SLOT_1) {
        data = data & PLUG_TESTCARD_GPS_SLOT1;
        if (data != PLUG_TESTCARD_GPS_SLOT1){
            cterr('f', 0, "SLOT1 GPS enable Test Failed");
            pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);
            return (FAILED);
        }
    } else {
        data = data & PLUG_TESTCARD_GPS_SLOT2;
        if (data != PLUG_TESTCARD_GPS_SLOT2){
            cterr('f', 0, "SLOT2 GPS enable Test Failed");
            pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);
            return (FAILED);
        }
    }


    /* Disable GPS then check */
    pca_obj->callin_fvt->drive_port((dev_object_t *)pca_obj,PORT_0, 
                                           PLUG_TESTCARD_GPS_PIN, PORT_VAL_LOW );
    
    plug_common_host_plug_fpga_reg_read(PLUG_FPGA_DBG_LED_ADDR_REG, &data);
    
    if (slot == PLUG_SLOT_1) {
        data = data & PLUG_TESTCARD_GPS_SLOT1;
        if (data == PLUG_TESTCARD_GPS_SLOT1){
            cterr('f', 0, "SLOT1 GPS disable Test Failed");
            pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);
            return (FAILED);
        } 
    } else {
        data = data & PLUG_TESTCARD_GPS_SLOT2;
        if (data == PLUG_TESTCARD_GPS_SLOT2){
            cterr('f', 0, "SLOT2 GPS disable Test Failed");
            pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);
            return (FAILED);
        }
    }
    
    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);
    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_testcard_uart_test
 * Description: UART Test for Pluggable Test Card
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_testcard_uart_test (int input)
{
    int pattern[]= {0x00, 0xFF, 0xA5, 0x5A};
    int size_pattern = sizeof(pattern)/sizeof(int);
    int offset;
    struct plug_intf_t *plug;
    int slot=USB_SLOT1;
    int ix;
    uint data, reg;

    testname("UART");
    prpass(testpass, "Loopback Test");
    
    plug = (struct plug_intf_t *)plug_test_if;
    slot = plug->slot;
    printf("slot=%d\n", slot);

    /* Disable Divisor Latch Access bit to access Transmit/Read Buffer */
    reg = PLUG_UART_CONTROL_OFFSET_BY_SLOT(PLUG_UART_CONTROLLER_OFFSET, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, 
                                          reg + PLUG_UART_LCR_OFFSET,
                                          PLUG_UART_LCR_LAB_BIT);

    offset = reg + PLUG_UART_RBR_THR_DLL_OFFSET;

    /* Flush any data FIFO in read buffer */
    for (ix = 0; ix < UART_FLUSH_FIFO_TIMES; ix++) {
        plug_common_host_plug_fpga_reg_read(offset, &data);
    }

    for (ix = 0; ix < size_pattern; ix++) {
        plug_common_host_plug_fpga_reg_write(offset, pattern[ix]);
        
        msleep(UART_TEST_DELAY);

        plug_common_host_plug_fpga_reg_read(offset, &data);

        if (data != pattern[ix]) {
            cterr('f', 0, "Pattern mismatch, expected=%#x, read=%#x",
                  pattern[ix], data);
            return (FAILED);
        }
                            
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : get_pcie_bus_num
 * Description: get platform pcie bus number 
 * Inputs     : vendor id and device id 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
ushort get_pcie_bus_num (ushort vid, ushort did) 
__attribute__((weak, alias("__get_pcie_bus_num")));
ushort __get_pcie_bus_num (ushort vid, ushort did)
{
    printf("%s : is not implemented \n", __FUNCTION__); 
    return (UNKNOWN_PCI_BUS_NUM); 
}

/*******************************************************************************
 * Function   : plug_testcard_pcie_post_pwr_up
 * Description: The function to implement process after pcie device power up
 * Inputs     : 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int plug_testcard_pcie_post_pwr_up (void) 
__attribute__((weak, alias("__plug_testcard_pcie_post_pwr_up")));
int __plug_testcard_pcie_post_pwr_up (void)
{
    printf("%s : is not implemented \n", __FUNCTION__); 
    return (PASSED); 
}

/*******************************************************************************
 * Function   : plug_testcard_pcie_post_pwr_down
 * Description: The function to implement process after pcie device power down
 * Inputs     :  
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int plug_testcard_pcie_post_pwr_down (void) 
__attribute__((weak, alias("__plug_testcard_pcie_post_pwr_down")));
int __plug_testcard_pcie_post_pwr_down (void)
{
    printf("%s : is not implemented \n", __FUNCTION__); 
    return (PASSED); 
}

/*******************************************************************************
 * Function   : plug_testcard_pcie_device_remove 
 * Description: The function was to remove pcie device 
 * Inputs     : none 
 * Outputs    : none
 *
 *******************************************************************************
 */
void plug_testcard_pcie_device_remove (void) 
__attribute__((weak, alias("__plug_testcard_pcie_device_remove")));
void __plug_testcard_pcie_device_remove (void)
{
    printf("%s : is not implemented \n", __FUNCTION__); 
}

/*******************************************************************************
 * Function   : plug_pcie_lane_scan_test      
 * Description: scan pcie bus lane info from platform 
 * Inputs     : vendor id, device id, pcie speed, pcie lane width. 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
static int plug_pcie_lane_scan_test (uint32_t dev_vid, uint32_t dev_did, uint32_t dev_speed, uint32_t dev_width) 
{
    uint32_t bus, reg_val, cap_val, sta_val;
    uint32_t cap_s, sta_s, sta_w; /*cap_w*/
    char dev_name[] = "NVME";
    /* please note dev width is the same order as dev_vid and dev_did */

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("%s : testing vendor id 0x%x, device id 0x%x, %s\n", __FUNCTION__,
                     dev_vid, dev_did, dev_name);
    }

    bus = get_pcie_bus_num(dev_vid, dev_did);

    if (bus == UNKNOWN_PCI_BUS_NUM) { 
        cterr('f',0, "Unknown PCI bus number for device %04x:%04x",
              dev_vid, dev_did);
        return (FAILED);
    }

    reg_val = get_pcie_cap_struct_ptr(bus, PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
    if (reg_val == FAILED) {
        cterr('f',0, "Can't get PCI cap pointer");
        return (FAILED);
    }

    cap_val = get_pcie_link_cap(bus, PCI_DEV_0, PCI_FUN_0, reg_val);
    sta_val = get_pcie_link_status(bus, PCI_DEV_0, PCI_FUN_0, reg_val);

    /* Speed - bit 0~3 */
    cap_s = cap_val & PCI_EXP_LINK_STA_SPD_MASK;
    sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
    /* Width - bit 4~9 */
    sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

    if (sta_s == dev_speed) {
        if ((cap_s == PCI_EXP_LINK_STA_SPD_2DOT5) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
            printf("Link speed is 2.5G\n");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_5GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
            printf("Link speed is 5G\n");
#ifdef TABEIL
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
            printf("Link speed is 5G\n");
#endif
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_8GT)) {
            printf("Link speed is 8G\n");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
            printf("Link speed is 2.5G\n");
        }
    } else {
        cterr('f',0, "Link speed is not 2.5G, 5G or 8G, device id 0x%x vendor id 0x%x"\
                     "device-%s capability speed is %x status speed is %x", 
                      dev_vid, dev_did, dev_name, cap_s, sta_s);
        return (FAILED);
    }

    if (sta_w == dev_width) {
        if (sta_w == PCI_EXP_LINK_STA_WID_1) {
            printf("Link width is x1\n");
        } else if (sta_w == PCI_EXP_LINK_STA_WID_2) {
            printf("Link width is x2\n");
        } else if (sta_w == PCI_EXP_LINK_STA_WID_4) {
            printf("Link width is x4\n");
        } else if (sta_w == PCI_EXP_LINK_STA_WID_8) {
            printf("Link width is x8\n");
        }
    } else {
        cterr('f',0, "Link width is not correct, device id 0x%x vendor id 0x%x, device-%s"
                     "status width = %x",
                     dev_vid, dev_did, dev_name, sta_w);
        return (FAILED);
    }
	
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_testcard_nvme_test
 * Description: main test for m2 PCIE test
 * Inputs     : void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
static int plug_testcard_nvme_test (void)
{
    int slot, rc = FAILED;
    char *tname = "NVMe PCIE read/write";
    char m2_dev[DEV_NAME_LEN];
    uint32_t dev_vid, dev_did, dev_speed, dev_width;
    struct plug_intf_t *plug;

    plug = (struct plug_intf_t *)plug_test_if;
    if (plug == NULL) {
        cterr('f', 0, "plug_test_if not init");
        return (FAILED);
    }
    slot = plug->slot;

    testname("%s access", tname);

    prpass(testpass, "%s, ", tname);

    if (!plug_tc_host_pcie_present(slot)) {
        printf("Host does NOT has PCIe connect to PIM NVMe SSD.\n");
        printf("Skip the NVMe SSD test...\n");
        return (PASSED);
    }

    printf("\npcie lane scan\n");
    plug_tc_host_get_pcie_dev_info(slot, &dev_vid, &dev_did, &dev_speed, &dev_width);
    rc = plug_pcie_lane_scan_test(dev_vid, dev_did, dev_speed, dev_width);
    if (rc == FAILED) {
        cterr('f', 0, "NVMe PCIE lane scan failed.");
        return (FAILED);
    }

    /* get the platform nvme device node */
    plug_tc_host_get_nvme_info(slot, m2_dev);

    return (rc);
}

/*******************************************************************************
 * Function   : plug_testcard_usb_test
 * Description: USB Test for Pluggable Test Card
 * Inputs     : input - slot
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_testcard_usb_test (int slot)
{
    struct plug_intf_t *plug;
    int usb_speed = 0, usb_idx;
    int rc = FAILED;
    int hub = FALSE;
    char tname[32];

    plug = (struct plug_intf_t *)plug_test_if;
    if (plug == NULL) {
        cterr('f', 0, "plug_test_if not init");
        return (FAILED);
    }
    slot = plug->slot;
    printf("slot=%d\n", slot);
    
    memset(tname, 0, sizeof(tname));
    sprintf(tname, "External USB %d", slot);
    testname(tname);
    prpass(testpass, "%s, ", tname);
 
    /* Tabei-L keep the origin setting */
#ifndef TABEIL    
    /* Temporary disable showing kernel messages because unbind and 
    bind XHCI controller to XHCI driver */
    system(SUPPRESS_MESG);
#endif

    /*
     * testname is printed on plug_tc_usb_slot_tests
     */
    /* Test USB Auto(3.0) mode */
    if (plug_tc_usb_parse_info() == FAILED) {
        cterr('f', 0, "plug_tc_usb_parse_info() failed");
        return (FAILED);
    }
    
    /* Check if the USB device is detected or not */
    if (plug_tc_usb_mass_stor_present_index(slot, &usb_idx, PLUG_TESTCARD_USB3P0_SPEED, hub) == FALSE) {
        cterr('f', 0, "PLUG USB Mass Storage is not detected in Slot %d", slot);
        return (FAILED);
    } 

    usb_speed = plug_tc_usb_get_speed(usb_idx);
    if (usb_speed != PLUG_TESTCARD_USB3P0_SPEED) {
        printf("\nNEED TO USE USB 3.0 TO RUN USB TEST\n");
        cterr('f', 0, "USB 3.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = plug_tc_usb_slot_tests(usb_idx);
        if (rc == PASSED) {
            prpass(testpass, "%s 3.0 read/write test passed, ", tname);
        } else {
            cterr('f', 0, "USB 3.0 read/write test failed");
            return (FAILED);
        }
    }

    /* Change to USB 2.0 */
    plug_common_host_usb_2p0_mode_set(slot);

    /* Run USB 2.0 test */
    if (plug_tc_usb_parse_info() == FAILED) {
        cterr('f', 0, "plug_tc_usb_parse_info() failed");
        return (FAILED);
    }
    
    /* Check if the USB device is detected or not */
    if (plug_tc_usb_mass_stor_present_index(slot, &usb_idx, PLUG_TESTCARD_USB2P0_SPEED, hub) == FALSE) {
        cterr('f', 0, "PLUG USB Mass Storage is not detected in Slot %d", slot);
        return (FAILED);
    } 

    usb_speed = plug_tc_usb_get_speed(usb_idx);
    if (usb_speed != PLUG_TESTCARD_USB2P0_SPEED) {
        cterr('f', 0, "USB 2.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = plug_tc_usb_slot_tests(usb_idx);
        if (rc == PASSED) {
            prpass(testpass, "%s 2.0 read/write test passed, ", tname);
        } else {
            cterr('f', 0, "USB 2.0 read/write test failed");
            return (FAILED);
        }
    }

    /* Change back to USB 3.0 */
    plug_common_host_usb_3p0_mode_set(slot);

    plug->unreset((void*)plug);
    plug->i2c_unreset((void*)plug);
    msleep(DELAY_USBCMD); 
    
    /* Tabei-L keep the origin setting */
#ifndef TABEIL
    /* Enable kernel message */
    system(OPEN_MESG);
#endif

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}


/*******************************************************************************
 * Function   : plug_testcard_usb_hub_test
 * Description: USB Test for Pluggable Test Card with USB 3P0 Hub
 * Inputs     : input - slot
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_testcard_usb_hub_test (int slot)
{
    struct plug_intf_t *plug;
    int usb_speed = 0, usb_idx;
    int rc = FAILED;
    int hub = TRUE;
    char tname[32];

    plug = (struct plug_intf_t *)plug_test_if;
    if (plug == NULL) {
        cterr('f', 0, "plug_test_if not init");
        return (FAILED);
    }
    slot = plug->slot;
    printf("slot=%d\n", slot);
    
    memset(tname, 0, sizeof(tname));
    sprintf(tname, "External USB HUB %d", slot);
    testname(tname);
    prpass(testpass, "%s, ", tname);
    
    /*
     * testname is printed on plug_tc_usb_slot_tests
     */
    /* Test USB Auto(3.0) mode */
    if (plug_tc_usb_parse_info() == FAILED) {
        cterr('f', 0, "plug_tc_usb_parse_info() failed");
        return (FAILED);
    }
    
    /* Check if the 3.0 USB device is detected or not */
    if (plug_tc_usb_mass_stor_present_index(slot, &usb_idx, PLUG_TESTCARD_USB3P0_SPEED, hub) == FALSE) {
        cterr('f', 0, "PLUG USB 3.0 Mass Storage is not detected with Hub in Slot %d", slot);
        return (FAILED);
    } 

    usb_speed = plug_tc_usb_get_speed(usb_idx);
    if (usb_speed != PLUG_TESTCARD_USB3P0_SPEED) {
        printf("\nNEED TO USE USB 3.0 TO RUN USB TEST\n");
        cterr('f', 0, "USB 3.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = plug_tc_usb_slot_tests(usb_idx);
        if (rc == PASSED) {
            prpass(testpass, "%s 3.0 read/write test passed, \n", tname);
        } else {
            cterr('f', 0, "USB 3.0 read/write test failed");
            return (FAILED);
        }
    }

    /* Check if the 2.0 USB device is detected or not */
    if (plug_tc_usb_mass_stor_present_index(slot, &usb_idx, PLUG_TESTCARD_USB2P0_SPEED, hub) == FALSE) {
        cterr('f', 0, "PLUG USB 2.0 Mass Storage is not detected with Hub in Slot %d", slot);
        return (FAILED);
    } 

    usb_speed = plug_tc_usb_get_speed(usb_idx);
    if (usb_speed != PLUG_TESTCARD_USB2P0_SPEED) {
        cterr('f', 0, "USB 2.0 setting failed (%d)", usb_speed);
        return (FAILED);
    } else {
        rc = plug_tc_usb_slot_tests(usb_idx);
        if (rc == PASSED) {
            prpass(testpass, "%s 2.0 read/write test passed, ", tname);
        } else {
            cterr('f', 0, "USB 2.0 read/write test failed");
            return (FAILED);
        }
    }

    plug->unreset((void*)plug);
    plug->i2c_unreset((void*)plug);
    msleep(DELAY_USBCMD); 

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/*******************************************************************************
 *
 * Function   :    plug_tc_usb_slot_tests
 * Description:    entry point to usb device test
 * Inputs     :    usb_idx - Index to USB device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
static int plug_tc_usb_slot_tests (int usb_idx)
{
    char src[32], usb_spd[15];
    int retval;
    char tname[32] = { 0, };

    sprintf(tname, "USB Index %d", usb_idx);

    /* don't find the devname ex./dev/sda */
    if (strcmp(plug_usb[usb_idx].dev_name, "") == 0) {
        cterr('f',0,"Failed to find USB storage.");
        return (FAILED);
    }

    if (plug_usb[usb_idx].spd == PLUG_TESTCARD_USB3P0_SPEED) {
        sprintf(usb_spd, "%s", "[USB 3.0] ");
    } else {
        sprintf(usb_spd, "%s", "[USB 2.0] ");
    }

    sprintf(src, "/dev/%s", plug_usb[usb_idx].dev_name);
    retval = plug_tc_access_device_test(src);
    
    if(!plug_tc_quiet_launch) {
        prpass(testpass, "%s%s, ", usb_spd, tname);
    }
    return (retval);
}

/*******************************************************************************
 *
 * Function   :    plug_tc_access_device_test
 * Description:    main test for usb device test
 * Inputs     :    file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
static int plug_tc_access_device_test (char *src)
{
    char buf[128], buf_bk[512], buf_wr[512], buf_rd[512];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;

    if(!plug_tc_quiet_launch) {
        prpass(testpass, "Access device '%s' , ", src);
    }
    sprintf(buf, "%s", src);

    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    for (ix = 0; ix < PLUG_TESTCARD_USB_DEVFD_COUNT; ix++) {
        devfd = open(buf, O_RDWR);
        if (devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }
    }
    if (devfd < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "there is no device file descriptor available.");
        return (FAILED);
    }

    /*
     * back up data
     */
    if (!plug_tc_quiet_launch) {
        prpass(testpass, "Backup data , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("backup lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }
    if ((num = read(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }

    /*
     * prepare data pattern
     */
    if (!plug_tc_quiet_launch) {
        prpass(testpass, "Prepare data pattern , ");
    }
    for (cnt = 0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }

    /*
     * write data pattern
     */
    if (!plug_tc_quiet_launch) {
        prpass(testpass, "Write data pattern , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("write lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,"Write test pattern failed, can not write to drive.");
        printf("Unable to write data pattern to device.");
        return (FAILED);
    }
    if (num != sizeof(buf_bk)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for data pattern");
        return (FAILED);
    }

    if (fsync(devfd) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "fsync failed.");
        printf("Unable to sync data pattern to device.");
        return (FAILED);
    }

    /*
     * read back data for comparing
     */
    if (!plug_tc_quiet_launch) {
        prpass(testpass, "Read back data for comparing , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd, sizeof(buf_rd))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }

    /*
     * comparing data
     */
    if (!plug_tc_quiet_launch) {
        prpass(testpass, "Comparing data , ");
    }
    cnt = 0;
    for (ib = 0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",
                   (ib + 1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    /*
     * restore data
     */
    if (!plug_tc_quiet_launch) {
        prpass(testpass, "Restore data , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_bk, sizeof(buf_bk))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write restore data failed, can not write to drive.\n");
        return (FAILED);
    }

    if (num != sizeof(buf_bk)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for restore");
        return (FAILED);
    }

    if (fsync(devfd) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "fsync failed.");
        return (FAILED);
    }

    close(devfd);               /* don't need it anymore */
    return (PASSED);

}


/*-------------------------------------------------
$Log: plug_testcard_test.c,v $
Revision 1.8  2021/09/24 01:27:20  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.7  2020/01/09 01:02:33  jiajliu
Merge Curie 2RU to main trunk

Revision 1.6  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.5  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.4  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.3.54.6  2018/11/16 06:40:49  hondwang
modify PRRQ suggest with CSCvn17216 pluggable re-instruct

Revision 1.3.54.5  2018/11/01 08:17:45  hondwang
Add USB hub flag for USB menu test item

Revision 1.3.54.4  2018/11/01 06:24:33  hondwang
Add plug testcard USB HUB testing function

Revision 1.3.54.3  2018/10/22 08:47:59  hondwang
Add IO interface testing for plug testcard

Revision 1.3.54.2  2018/10/16 07:08:45  hondwang
plug_tc_host_sgmii_present should be platform code, modified

Revision 1.3.54.1  2018/10/15 06:50:50  hondwang
pluggable common code re-instruct modify code

Revision 1.3  2018/02/09 09:18:35  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/02/08 07:16:04  lucywang
Merged LTE USB2.0 detect test from trunk

Revision 1.2.2.2  2018/01/20 06:56:38  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:10  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.10  2017/09/23 03:47:00  hondwang
Fix pluggable reset mode not working issue

Revision 1.1.4.9  2017/09/19 21:40:20  hondwang
Change Pluggable USB testing power sequence

Revision 1.1.4.8  2017/09/15 22:53:25  hondwang
Fix GPS testing fail with slot 2 run all

Revision 1.1.4.7  2017/09/15 17:59:18  hondwang
Add plug_testcard_sgmii_present function

Revision 1.1.4.6  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.1.4.5  2017/08/31 05:01:49  hondwang
Add GPS test with pluggable test card

Revision 1.1.4.4  2017/08/31 00:13:59  lucywang
remove the setting to SGMII mode of GE1 on the testcard

Revision 1.1.4.3  2017/08/22 03:29:59  lucywang
set 1000Base-X for pluggable serial and set sgmii for pluggable test card

Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
add pluggable testcard for star-branch-c9xx

Revision 1.1.2.4  2017/07/21 09:00:05  hondwang
fix pluggable fail with USB enumerate time not enought

Revision 1.1.2.3  2017/07/21 03:09:54  hondwang
fix group test wait time not enough issue

Revision 1.1.2.2  2017/07/20 12:52:38  hondwang
Add module reset pin testing and fix test card not unreset issue

Revision 1.1.2.1  2017/07/13 06:32:22  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.9  2017/07/11 11:47:32  hondwang
Fix test card SGMII menu issue

Revision 1.1.2.8  2017/06/26 22:48:41  tirawan
UART test for Cisco pluggable FPGA

Revision 1.1.2.7  2017/06/26 08:11:55  steja
Fixed Pluggable testcard USB test

Revision 1.1.2.6  2017/06/22 18:13:16  tirawan
Update for Pluggable LTE test items



*/


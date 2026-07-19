/*
 * $Id: plug_NR_5G_telit_lib.h,v 1.2 2021/06/02 02:56:20 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_lib.h,v $
 *------------------------------------------------------------------
 *
 * plug_NR_5G_telit_lib.h - Header File for Pluggable LTE Telit
 *                        Library Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "dev_NR_5G_telit.h"

#ifndef __PLUG_NR_5G_TELIT_LIB_H__
#define __PLUG_NR_5G_TELIT_LIB_H__

#define DELAY_1000_MS                            (1000)    //msleep()
#define DELAY_5_SEC                              (5)    //sleep()
#define DELAY_60_SEC                             (60)   //sleep()
#define TELIT_NR_5G_ACTIVATED_DELAY              (5)
#define MODEM_FN980_PWR_ON_DELAY                 (50)
#define MODEM_CHK_PWR_TOUT                       (6000)
#define PROBE_NR_5G_TELIT_TOUT                   (6000)
#define PLUG_NR_5G_TELIT_POLLING_DELAY           (10)
#define PLUG_NR_5G_TELIT_TTY_ATTACH_DELAY        (500)
#define MODEM_MODE_SWITCHING_TOUT                (500)

#define USB_SYS_VID_FILE                         "idVendor"
#define USB_SYS_PID_FILE                         "idProduct"
#define USB_SYS_PRODUCT_FILE                     "product"

#define USB_AT_CMD_PORT                          "1.4"
#define USB_TTY_PATH                             "/dev"
#define TELIT_AT_CMD_DEV_NAME                    "AT_port"
#define PLUG_NR_5G_FN980_STR                     "FN980"
#define USB_SYS_DRV_PATH                         "/sys/bus/usb/drivers/usb"
#define INSMOD_CMD                               "insmod"
#define RMMOD_CMD                                "rmmod"
#define TELIT_USB_SERIAL_DRV                     "GobiSerial.ko"
#define SUPER_SPD_STR                            "USB3.0"
#define HIGH_SPD_STR                             "USB2.0"
#define HOST_CONN_STR                            "Host connector"
#define DPORT_STR                                "Debug port"

#define FN980_RST_ASSERTION                      (1000)

typedef struct plug_NR_5G_usb_config_t {
    char usb_devinfo[128];
    char at_usb_devinfo[128];
} plug_NR_5G_telit_usb_config_t;

enum usb_port_t {
    DEBUG_USB,
    USB2P0,
    USB3P0,
    MAX_USB_PORT
} usb_mode;

enum sim_num {
    SIM0,
    SIM1
};

typedef struct gpio_exp_init {
    int dev;
    int dir;
    int port;
    int bit;
    int dev_fun;
    int def_val;
} NR_5G_gpio_exp;

typedef enum {
    INPUT,
    OUTPUT
} pgio_dirc;

typedef enum {
    LOW,
    HIGH
} gpio_driv;

typedef enum {
    PORT0,
    PORT1
} gpio_port;

typedef enum {
    PLUG_NR_5G_TELIT_FN980,
} plug_NR_5G_telit_msku_t;

typedef enum {
    ENABLE_LED_GREEN = 0,
    ENABLE_LED_YELLOW,
    HOST_SERDES_TYPE_0,
    HOST_SERDES_TYPE_1,
    PRIMARY_INTERFACE_READY,
    USB3_PCIE_SERDES_SEL,
    DYING_GASP_OK, 
    USB_DEBUG_ENABLE, 
    W_DISABLE_1, 
    W_DISABLE_2,
    USB3_SERDES_SELECTOR,
    SAFE_PWR_REMOVAL,
    RESET,
    MODEM_POWER_ON,
    LED_SIM0_OK,      
    LED_SIM0_NOT_OK, 
    LED_SIM1_OK,     
    LED_SIM1_NOT_OK, 
    LED_GPS_OK,      
    LED_GPS_NOT_OK,  
    LED_RSSI0, 
    LED_RSSI1, 
    LED_RSSI2, 
    WWAN_LED_ENABLE, 
    WWAN_LED_SIM_SEL,
    SIM_SELECT,      
    SIM0_DETECT,     
    SIM1_DETECT     
} gpio_func;

typedef struct led_dev {
    int dev_fun;
    int drive_on;
    int drive_off;
} plug_NR_5G_led_dev;

typedef struct plut_NR_5G_led {
    int dev;
    int val;
} plug_NR_5G_led_ctrl;

typedef enum {
    SERDES_PCIE_3P0,
    SERDES_USB_3P0,
} plug_NR_5G_modem_serdes_t;

extern int plug_NR_5g_telit_set_gpio_exp_test_reg(int);
extern int plug_NR_5g_telit_gpio_exp_out_init(void);
extern int plug_NR_5g_telit_gpio_exp_dir_init(void);
extern int plug_NR_5g_telit_gpio_exp_sim_card_detect(int);
extern int plug_NR_5g_telit_modem_pwr_ctrl(int);
extern int plug_NR_5g_telit_insmod(int);

extern int plug_NR_5g_telit_usb_is_found(int, int, int);
extern int plug_NR_5g_telit_usb_deb_enable(int);
extern int plug_NR_5g_telit_w_disable1_ctrl(int);
extern int plug_NR_5g_telit_modem_reset_pin_ctrl(int);
extern int plug_NR_5g_telit_modem_pwron_pin_ctrl(int);
extern int plug_NR_5g_telit_get_tty_devname(char *);
extern int plug_NR_5g_telit_toggle_led(int, int);
extern int plug_NR_5g_telit_hard_reset(void);
extern int plug_NR_5g_telit_soft_reboot(int);
extern int plug_NR_5g_telit_select_modem_serdes(int);
extern int plug_NR_5g_telit_wwan_led_output_enable_ctrl(int);
extern int plug_NR_5g_telit_wwan_led_sim_select(int);
extern int plug_NR_5g_telit_dev_create(dev_NR_5g_telit_object_t *);
extern int plug_NR_5g_telit_dump_modem_temp(void);
extern int plug_NR_5G_telit_set_modem_sku(void);
extern int plug_NR_5G_telit_set_modem_pwron_pin_test(int);
extern int plug_NR_5g_telit_set_modem_default_feature(void);
extern void plug_NR_5g_telit_modem_searching (int *, int *);
extern void plug_NR_5g_telit_get_modem_sku(int *);
extern void plug_NR_5g_telit_store_usb_devinfo(int);
extern void plug_NR_5g_telit_set_current_usb_port(int);
extern void plug_NR_5g_telit_get_current_usb_port(int *);
extern boolean plug_NR_5g_telit_has_temp_sensor(void);
extern boolean plug_NR_5g_telit_has_dedicated_gps_antenna(void);
extern boolean plug_NR_5g_telit_supports_pcie_intf(void);
extern int plug_NR_5G_telit_sim_sel (int sim_sel);

#endif  /* __PLUG_NR_5G_TELIT_LIB_H__ */

/*********************************************************************
 * $Log: plug_NR_5G_telit_lib.h,v $
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.3  2021/02/27 00:43:08  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.2  2020/12/02 03:57:22  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */

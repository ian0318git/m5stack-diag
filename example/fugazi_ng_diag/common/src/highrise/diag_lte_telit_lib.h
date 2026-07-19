 /*------------------------------------------------------------------
 *
 * diag_lte_telit_lib.h - Header File for LTE Telit
 *                        Library Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "dev_lte_telit.h"

#ifndef __LTE_TELIT_LIB_H__
#define __LTE_TELIT_LIB_H__

#define DELAY_5_SEC                              (5)
#define MODEM_LM9X0_PWR_ON_DELAY                 (30)
#define MODEM_CHK_PWR_ON                         (3000)
#define MODEM_CHK_PWR_TOUT                       (6000)
#define PROBE_LTE_TELIT_TOUT                     (6000)
#define LTE_TELIT_POLLING_DELAY                  (10)
#define LTE_TELIT_TTY_ATTACH_DELAY               (500)
#define LTE_TELIT_SERDES_SWITCHING_DELAY         (2000)
#define MODEM_MODE_SWITCHING_TOUT                (500)
#define LTE_TELIT_SHDN_TIMEOUT                   (25)

#define MODEM_TELIT_USB_VID                      (0x1bc7)
#define USB_SYS_VID_FILE                         "idVendor"
#define USB_SYS_PID_FILE                         "idProduct"
#define USB_SYS_PRODUCT_FILE                     "product"

#define USB_AT_CMD_PORT                          "1.4"
#define USB_TTY_PATH                             "/dev"
#define DEFAULT_LTE_USB_TTY_DEV                  "ttyUSB2"
#define LTE_LM940_STR                            "LM940"
#define LTE_LM960_STR                            "LM960"
#define USB_SYS_DRV_PATH                         "/sys/bus/usb/drivers/usb"
#define INSMOD_CMD                               "insmod"
#define RMMOD_CMD                                "rmmod"
#define TELIT_USB_SERIAL_DRV                     "GobiSerial.ko"
#define SUPER_SPD_STR                            "USB3.0"
#define HIGH_SPD_STR                             "USB2.0"
#define HOST_CONN_STR                            "Host connector"
#define DPORT_STR                                "Debug port"

#define LM960_RST_ASSERTION                      (1000)
#define LM940_RST_ASSERTION                      (100)

typedef struct lte_usb_config_t {
    char usb_devinfo[128];
    char at_usb_devinfo[128];
} lte_telit_usb_config_t;

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
} lte_gpio_exp;

typedef enum {
    INPUT,
    OUTPUT
} pgio_dirc;

typedef enum {
    PORT0,
    PORT1
} gpio_port;

typedef enum {
    LTE_TELIT_LM940,
    LTE_TELIT_LM960,
} lte_telit_msku_t;

typedef enum {
    ENABLE_LED_GREEN = 0,
    ENABLE_LED_YELLOW,
    HOST_SERDES_TYPE_0,
    HOST_SERDES_TYPE_1,
    PRIMARY_INTERFACE_READY,
    DYING_GASP_OK, 
    USB_DEBUG_ENABLE, 
    W_DISABLE_1, 
    MODEM_TRIGGER_SEND_SMS,
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
    LED_RSSI3, 
    LED_4G_3G, 
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
} lte_led_dev;

typedef struct lte_led {
    int dev;
    int val;
} lte_led_ctrl;

typedef enum {
    SERDES_PCIE_3P0,
    SERDES_USB_3P0,
} lte_modem_serdes_t;

extern int diag_lte_telit_set_gpio_exp_test_reg(int);
extern int diag_lte_telit_gpio_exp_out_init(void);
extern int diag_lte_telit_gpio_exp_dir_init(void);
extern int diag_lte_telit_gpio_sim_card_detect(int);
extern int diag_lte_telit_insmod(int);
extern int diag_lte_telit_usb_is_found(int, int, int);
extern int diag_lte_telit_usb_deb_enable(int);
extern int diag_lte_telit_w_disable1_ctrl(int);
extern int diag_lte_telit_modem_reset_pin_ctrl(uchar);
extern int diag_lte_telit_modem_pwron_pin_ctrl(int);
extern int diag_lte_telit_get_tty_devname(char *);
extern int diag_lte_telit_toggle_led(int, int);
extern int diag_lte_telit_hard_reset(void);
extern int diag_lte_telit_modem_reset(void);
extern int diag_lte_telit_soft_reboot(int);
extern int diag_lte_telit_select_modem_serdes(int);
extern int diag_lte_telit_wwan_led_output_enable_ctrl(int);
extern int diag_lte_telit_wwan_led_sim_select(int);
extern int diag_lte_telit_dev_create(dev_lte_telit_object_t *);
extern int diag_lte_telit_dump_modem_temp(void);
extern int diag_lte_telit_set_modem_sku(void);
extern int diag_lte_telit_set_modem_pwron_pin_test(int);
extern int diag_lte_telit_set_modem_default_feature(void);
extern void diag_lte_telit_modem_searching (int *, int *);
extern void diag_lte_telit_get_modem_sku(int *);
extern void diag_lte_telit_store_usb_devinfo(void);
extern void diag_lte_telit_set_current_usb_port(int);
extern void diag_lte_telit_get_current_usb_port(int *);
extern int diag_lte_telit_modem_power_down(void);
extern int diag_lte_telit_set_shutdown_indicator(void);
extern boolean diag_lte_telit_has_temp_sensor(void);
extern boolean diag_lte_telit_has_dedicated_gps_antenna(void);
extern boolean diag_lte_telit_has_2_rssi_antenna(void);
extern boolean diag_lte_telit_supports_pcie_intf(void);
extern boolean diag_lte_telit_soft_shutdown_indicator_is_set(void);
extern int diag_lte_telit_set_testmode(int);
extern int diag_lte_telit_config_modem_to_usb3p0(void);
extern int diag_lte_telit_config_modem_to_usb2p0(void);
extern int diag_lte_telit_rescan (int);
extern int diag_lte_telit_modem_pwr_ctrl (int);

#endif  /* __LTE_TELIT_LIB_H__ */

/*------------------------------------------------------------------
$Log: diag_lte_telit_lib.h,v $
Revision 1.1  2020/08/19 09:49:35  markzha
*** empty log message ***



$Endlog$
*/

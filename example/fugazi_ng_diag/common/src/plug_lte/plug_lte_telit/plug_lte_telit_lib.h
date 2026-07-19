/* $Id: plug_lte_telit_lib.h,v 1.5 2019/08/14 02:27:18 shjung Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_lib.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_lib.h - Header File for Pluggable LTE Telit
 *                        Library Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "dev_lte_telit.h"

#ifndef __PLUG_LTE_TELIT_LIB_H__
#define __PLUG_LTE_TELIT_LIB_H__

#define DELAY_5_SEC                              (5)
#define TELIT_LTE_ACTIVATED_DELAY                (5)
#define MODEM_LM9X0_PWR_ON_DELAY                 (30)
#define MODEM_CHK_PWR_TOUT                       (6000)
#define PROBE_LTE_TELIT_TOUT                     (6000)
#define PLUG_LTE_TELIT_POLLING_DELAY             (10)
#define PLUG_LTE_TELIT_TTY_ATTACH_DELAY          (500)
#define PLUG_LTE_TELIT_SERDES_SWITCHING_DELAY    (2000)
#define MODEM_MODE_SWITCHING_TOUT                (500)

#define MODEM_TELIT_USB_VID                      (0x1bc7)
#define USB_SYS_VID_FILE                         "idVendor"
#define USB_SYS_PID_FILE                         "idProduct"
#define USB_SYS_PRODUCT_FILE                     "product"

#define USB_AT_CMD_PORT                          "1.4"
#define USB_TTY_PATH                             "/dev"
#define TELIT_AT_CMD_DEV_NAME                    "AT_port"
#define PLUG_LTE_LM940_STR                       "LM940"
#define PLUG_LTE_LM960_STR                       "LM960"
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

typedef struct plug_lte_usb_config_t {
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
    LOW,
    HIGH
} gpio_driv;

typedef enum {
    PORT0,
    PORT1
} gpio_port;

typedef enum {
    PLUG_LTE_TELIT_LM940,
    PLUG_LTE_TELIT_LM960,
} plug_lte_telit_msku_t;

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
} plug_lte_led_dev;

typedef struct plut_lte_led {
    int dev;
    int val;
} plug_lte_led_ctrl;

typedef enum {
    SERDES_PCIE_3P0,
    SERDES_USB_3P0,
} plug_lte_modem_serdes_t;

extern int plug_lte_telit_set_gpio_exp_test_reg(int);
extern int plug_lte_telit_gpio_exp_out_init(void);
extern int plug_lte_telit_gpio_exp_dir_init(void);
extern int plug_lte_telit_gpio_exp_sim_card_detect(int);
extern int plug_lte_telit_modem_pwr_ctrl(int);
extern int plug_lte_telit_insmod(int);
extern int plug_lte_telit_usb_is_found(int, int, int);
extern int plug_lte_telit_usb_deb_enable(int);
extern int plug_lte_telit_w_disable1_ctrl(int);
extern int plug_lte_telit_modem_reset_pin_ctrl(int);
extern int plug_lte_telit_modem_pwron_pin_ctrl(int);
extern int plug_lte_telit_get_tty_devname(char *);
extern int plug_lte_telit_toggle_led(int, int);
extern int plug_lte_telit_hard_reset(void);
extern int plug_lte_telit_soft_reboot(int);
extern int plug_lte_telit_select_modem_serdes(int);
extern int plug_lte_telit_wwan_led_output_enable_ctrl(int);
extern int plug_lte_telit_wwan_led_sim_select(int);
extern int plug_lte_telit_dev_create(dev_lte_telit_object_t *);
extern int plug_lte_telit_dump_modem_temp(void);
extern int plug_lte_telit_set_modem_sku(void);
extern int plug_lte_telit_set_modem_pwron_pin_test(int);
extern int plug_lte_telit_set_modem_default_feature(void);
extern void plug_lte_telit_modem_searching (int *, int *);
extern void plug_lte_telit_get_modem_sku(int *);
extern void plug_lte_telit_store_usb_devinfo(int);
extern void plug_lte_telit_set_current_usb_port(int);
extern void plug_lte_telit_get_current_usb_port(int *);
extern boolean plug_lte_telit_has_temp_sensor(void);
extern boolean plug_lte_telit_has_dedicated_gps_antenna(void);
extern boolean plug_lte_telit_has_2_rssi_antenna(void);
extern boolean plug_lte_telit_supports_pcie_intf(void);

#endif  /* __PLUG_LTE_TELIT_LIB_H__ */

/*------------------------------------------------------------------
$Log: plug_lte_telit_lib.h,v $
Revision 1.5  2019/08/14 02:27:18  shjung

1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational

Revision 1.4  2019/07/01 10:05:24  sherliu2
Supported mdev for Hyperloop

Revision 1.3  2019/05/20 07:28:06  shjung
1. Replace USB serial driver option.ko/usb_wwan.ko with GobiSerial.ko
2. Changes based on last code review comments.
3. Use poll mechanism to query modem functionality level in W_DISABLE pin test
4. Add 1 second delay after close tty device, which is following Telit's test script process.

Revision 1.2  2019/05/14 08:48:37  sherliu2
Support hyperloop

Revision 1.1.2.13  2019/05/09 07:50:18  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.12  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.11  2019/04/17 10:09:11  sherliu2
remove mdev related

Revision 1.1.2.10  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.9  2019/04/08 09:54:25  sherliu2
Modified tty device name to symbolic name generated by mdev

Revision 1.1.2.8  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.7  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.6  2019/02/15 02:59:49  shjung
Added WWAN_LED control utility

Revision 1.1.2.5  2019/02/13 01:44:30  shjung
Corrected the MODEM_POWER_ON pin value

Revision 1.1.2.4  2019/01/18 06:15:31  shjung

1. Added W_DISABLE pin test
2. Added modem USB mode switching utility
3. Added delay in modem reboot function based on spec
4. Removed USB mode resotre operation when debug port test failed
5. Code clean up

Revision 1.1.2.3  2019/01/15 10:22:19  shjung
Modified the mechanism to get modem USB device info

Revision 1.1.2.2  2019/01/02 02:09:27  shjung
Restore LTE back to USB3.0 mode while debug port test failed

Revision 1.1.2.1  2018/12/14 00:50:16  shjung
Initial check-in for Hyperloop



$Endlog$
*/

/* $Id: plug_lte_lib.h,v 1.12 2020/03/31 01:38:00 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_lib.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_lib.c - PLUGGABLE LTE Library Functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PLUG_LTE_LIB__
#define __PLUG_LTE_LIB__

extern int plug_lte_gpio_exp_dir_init(void);
extern int plug_lte_gpio_exp_out_init(void);
extern int plug_lte_toggle_led(int ,int);
extern int plug_lte_pri_interface_rdy(void);
extern int plug_lte_usb_deb_enable(int);
extern int plug_lte_w_1_disable(int);
extern int plug_lte_w_2_disable(int);
extern int plug_lte_em_hard_reset(void);
extern int plug_lte_modem_soft_reset(int);
extern int plug_lte_modem_pwr_ctrl(int);
extern int plug_lte_wwan_led_enable(int);
extern int plug_lte_wwan_led_sim_sel(int);
extern int plug_lte_sim_sel(int); 
extern int plug_lte_sim_detect(int);
extern int plug_lte_get_tty_devname(char *);
extern int plug_lte_dying_gasp_is_ok(void);
extern int plug_lte_read_man_gpio(int);
extern int plug_lte_check_modem_rdy(int);
extern int plug_lte_get_modem_status(int);
extern int plug_lte_modem_is_online(int);
extern int plug_lte_set_gpio_exp_test_reg(int);
extern int plug_lte_chk_modem_carrier_is_match(void);
extern int plug_lte_set_modem_carrier(void);
extern int plug_lte_en_auto_switch_img(void);
extern int plug_lte_usb_detect(char *, int, int);
extern int plug_lte_insmod(int);
extern int plug_lte_force_gps_pin_val(int, int);
extern int plug_lte_get_gps_pin_status(int, int *);
extern int plug_lte_modem_shutdown(void);
extern void plug_lte_get_ctype(int *);
extern void plug_lte_set_ctype(int);
extern void plug_lte_store_usb_devinfo(char *, char *);
extern void plug_lte_set_at_usb_devinfo(int);
extern boolean is_plug_lte_em(void);
extern boolean is_plug_lte_wp(void);
extern boolean plug_lte_has_2_sim_slot(void);
extern boolean plug_lte_has_temp_sensor(void);

#define PLUG_LTE_RESET_WAIT_IN_MS              (150)
#define PLUG_LTE_TOFF_WAIT_IN_SEC              (8)

#define PLUG_USB2P0_SPEED                      (480)
#define PLUG_USB3P0_SPEED                      (5000)

#define MODEM_SWI_USB_VID                      (0x1199)

#define WP_PWR_ON_DELAY                        (15500)

#define SEC_TO_MICROSEC                        (1000)
#define PLUG_LTE_MIN_ACTIVE_SEC                (45)
#define PROBE_LTE_TOUT                         (1000)
#define PLUG_LTE_POLLING_DELAY                 (10)
#define PLUG_LTE_CHK_TTY_STAT_DELAY            (500)
#define MODEM_PWROFF_SEQ_DELAY                 (25000)
#define MODEM_PWRON_SEQ_DELAY                  (45000)
#define MODEM_USB_RESET_DELAY                  (2000)
#define MODEM_HD_RESET_DELAY                   (25000)
#define EM_HD_RESET_H_DELAY                    (3500)
#define WP_HD_RESET_H_DELAY                    (35)
#define TTY_ACCESS_DELAY                       (500)
#define MAX_RETRY_TIME                         (480)
#define MAX_AT_CHK_TIME                        (3)
#define MAX_RECHECK_LTE_STAT_TIME              (6000)
#define USB_TTY_TOUT                           (6000)
#define WP_CHK_PWR_TOUT                        (6000)
#define WP_PWR_REMOVE_DELAY                    (13)
#define MAX_POLLING_TIME                       (500)
#define MAX_IMG_CARRIER_MATCH_TIME             (6)
#define CARRIER_MATCH_POLLING_DELAY            (10)
#define MAX_GET_TTY_DEV_POLLING_TIME           (10)
#define GET_TTY_DEV_POLLING_DELAY              (500)

#define USB_AT_CMD_PORT                        "1.3"
#define USB_TTY_PATH                           "/dev/"
#define DEFAULT_LTE_USB_TTY_DEV                "ttyUSB2"
#define LTE_TESTMSG_BUFSZ                      (128)

#define HOST_SERDES_TYPE_BIT                   (2)
#define USB_SYS_DRV_PATH                       "/sys/bus/usb/drivers/usb"
#define USB_SYS_SPEED_FILE                     "speed"
#define USB_SYS_VID_FILE                       "idVendor"
#define USB_SYS_DID_FILE                       "idProduct"
#define INSMOD_CMD                             "insmod"
#define RMMOD_CMD                              "rmmod"
#define GOBISERIAL_KO                          "GobiSerial.ko"
#define SIERRA_KO                              "sierra.ko"


enum sim_num {
  SIM0 = 0,
  SIM1,
};

enum sim_stat {
  SIM_NOT_PRESENT = 0,
  SIM_PRESENT,
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
} gpio_dirc;

typedef enum {
    LOW = 0,
    HIGH
} gpio_driv;

typedef enum {
    PCIe,
    SERDES_1000BASEX,
    SERDES_10GKR
} gpio_host_serdes_type;

typedef enum {
    PORT0,
    PORT1
} gpio_port;

typedef enum {
    ENABLE_LED_GREEN = 0,
    ENABLE_LED_YELLOW,
    HOST_SERDES_TYPE_0,
    HOST_SERDES_TYPE_1,
    PRIMARY_INTERFACE_READY,
    DYING_GASP_OK,
    USB_DEBUG_ENABLE,
    WDISABLE_1,
    WDISABLE_2,
    GPS_BIAS_OK,
    SAFE_PWR_REMOVAL,
    RESET,
    MODEM_POWER_OFF, 
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
    int driv_on;
    int driv_off;
    } plug_lte_led_dev;

typedef struct plug_lte_led {
    int dev;
    int val;
    } plug_lte_led_ctrl;

#endif

/*-------------------------------------------------
$Log: plug_lte_lib.h,v $
Revision 1.12  2020/03/31 01:38:00  sherliu2
Fix CSCvs60975: Timing issue - modem disconnect/reconnect sometimes will get wrong tty device

Revision 1.11  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.10  2019/06/14 05:48:11  shjung
Supported WP7605 modules

Revision 1.9  2018/11/23 09:15:07  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.8.14.2  2018/10/15 07:43:26  shjung
Re-struct for pluggable-LTE common codes

Revision 1.8.14.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.8  2018/07/12 09:33:41  shjung
Fixed CSCvk20378:Covered pluggable LTE modem SIM_DETECT pin

Revision 1.7  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.6  2018/05/21 08:11:29  shjung
Merged code from star-branch-c110x

Revision 1.5  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.5  2018/05/17 02:53:41  shjung

1. Added delay for SIM initialization while switching UIM interface
2. Clear SAFE_PWR_REMOVAL signal after powered down WP module
3. Added 15.5 seconds boot-up delay for WP module based on spec.

Revision 1.3.2.4  2018/04/11 09:21:05  shjung

1. Remove modem reset test and add modem soft reset utility
2. Code modified based on pluggable LTE EM7455 ER code review
Revision 1.3.2.3  2018/03/22 11:32:15  shjung
Modified EM module power-off timing

Revision 1.3.2.2  2018/03/16 07:47:28  shjung
1. Correct the default value of WP SAFE_POWER_REMOVAL and implement WP modem power-off function 2. Check modem status after hard reset

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:45  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/02/01 23:41:02  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.2  2018/01/20 06:56:34  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:09  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.7  2018/01/09 06:10:00  shjung
Added test criteria for the hold-up time of super caps, which are used for dying gasp feature

Revision 1.1.4.6  2017/12/13 15:14:51  shjung
Added dying gasp test for pluggable LTE-EM module

Revision 1.1.4.5  2017/12/08 12:28:46  shjung
Check if usb device attaches to tty successfully before capture corresponding ttyUSB number

Revision 1.1.4.4  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.3  2017/10/25 04:40:50  shjung
Modified pluggable module USB interface power-on/off sequence and USB interface mode configuration

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.2  2017/07/20 17:22:50  tirawan
Add USB 2.0 test and Debug port, and host implementation function prototype

Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.5  2017/06/27 22:45:14  shjung
Add pluggable LTE LED utility

Revision 1.1.2.4  2017/06/25 06:41:23  tirawan
Initialize GPIO Expander Output port before configuring its direction

Revision 1.1.2.3  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/


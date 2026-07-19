/* $Id: lte.h,v 1.9 2020/07/10 11:36:50 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/lte.h,v $
 *------------------------------------------------------------------
 *
 * lte.h - This file contains definitions for TSN-High LTE.
 *
 * Sofian Teja 
 *
 * Copyright (c) 2011-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _TSN_HIGH_LTE_H_
#define _TSN_HIGH_LTE_H_

enum sim_num {
  SIM0 = 0,
  SIM1,
};

enum sim_stat {
  SIM_NOT_PRESENT = 0,
  SIM_PRESENT,
};

#define MAX_RETRY_CNT    0

#define DEF_LTE_MODEM_0   "/dev/bus/usb/002/002"
#define DEF_LTE_MODEM_1   "/dev/bus/usb/006/002"
#define MODEM_SWI_USB_VID   0x1199
#define USB_SYS_PATH                        "/sys/bus/usb/drivers/usb"
#define USB_SYS_SPEED_FILE                  "speed"
#define USB_SYS_VID_FILE                    "idVendor"
#define USB_SYS_DID_FILE                    "idProduct"
#define HOST_USB1_20_PATH                   "3-1"
#define HOST_USB1_20_SLOT1_PATH             "3-1.1"
#define HOST_USB1_20_SLOT2_PATH             "3-1.2"
#define USB2P0_SPEED                         (480)
#define RSSI_B8_FREQ                        "944.5"
#define RSSI_B4_FREQ                        "2134.5"
#define RSSI_AMP                            "-70"

#define ENABLE_CONSOLE_MSG "dmesg -E >/dev/null 2>&1"
#define DISABLE_CONSOLE_MSG "dmesg -D >/dev/null 2>&1"
#define MAX_COMMAND_LENGTH 2048
#define AT_CMD_BUFFER_SIZE 1024
#define LTE_DELAY_SYSCMD 1000
#define MAX_SELFTEST_RETRY 1000

#define USB_TTY_TOUT                                (24000)
#define MAX_POLLING_TIME                            (5)
#define MAX_RETRY_TIME                              (6000)
#define LTE_CHK_MAX_RETRY_TIME                      (480)
#define AT_SELFTEST_TOUT_IN_SEC                     (1)
#define AT_SELFTEST_DELAY                           (500)
#define LTE_CHK_TTY_STAT_DELAY                  (500)
#define VTIME_TIMEOUT                           (30)
#define TTY_ACCESS_DELAY                        (500)
#define WP_PWR_ON_DELAY                         (15500)
#define PROBE_LTE_USB_TOUT                      (60000)
#define LTE_POLLING_DELAY                       (10)
#define WP_CHK_PWR_TOUT                         (6000)
#define WP_PWR_REMOVE_DELAY                     (13)
#define WP_HD_RESET_H_DELAY                     (35)
#define LTE_SIM_MUX_SWITCH_DELAY                (500)
#define TIMEOUT_600                             (600)

#define GPS_TEST_FREQ_MAX                       (105000)
#define GPS_TEST_FREQ_MIN                       (95000)

#define GPS_CTON_MAX                            (63)
#define GPS_CTON_MIN                            (53)

#define HIGH	1
#define LOW	0

#define USB2P0_PORT                             "3-1"
#define USB3P0_PORT                             "4-1"
#define USB_AT_CMD_PORT                        "1.3"
#define USB_TTY_PATH                           "/dev/"
#define TTYUSB_INFO_CMD                        "readlink /sys/class/tty/ttyUSB*"
#define TTYUSB_INFO_FILE                       "/diag/tty_num.txt"
#define TTYUSB_BUF_SIZE                        (1024)
#define TTYUSB_INFO_SIZE                       (128)
#define LTE_TESTMSG_BUFSZ                      (128)   /* 128 bytes */

#define IN_TEST1_KO "insmod /diag/sierra.ko"
#define IN_TEST2_KO "insmod /diag/sierra_net.ko"
#define RM_TEST1  "rmmod sierra.ko"
#define RM_TEST2  "rmmod sierra_net.ko"

#define IN_TRAFFIC1_KO "insmod /diag/GobiNet.ko"
#define IN_TRAFFIC2_KO "insmod /diag/GobiSerial.ko"
#define RM_TRAFFIC1  "rmmod GobiNet.ko"
#define RM_TRAFFIC2  "rmmod GobiSerial.ko"

#define MODEN_ERR (-1)

/* AT command Tests for modem */
#define RSSI_3G_MAIN_TEST     1
#define RSSI_3G_DIV_TEST      2
#define LTE_SIM_TEST          3  
#define ATI                   4  
#define AT_FROM_CONSOLE       5  
#define GPS_ANTENNAE_TEST     6
#define SET_GPS_PORT_CONN1    7
#define SET_GPS_PORT_CONN2    8
#define GPSAUTOSTART_OFF      9
#define GPSAUTOSTART_ON       10
#define ENABLE_GPS            11
#define DISABLE_GPS           12
#define RSSI_LTE_MAIN_TEST    13
#define RSSI_LTE_DIV_TEST     14
#define RSSI_RESET_MODEM      15
#define RSSI_NFT_SET_MODE     16
#define LTE_SIM1_TEST         17
#define LTE_GPS_ENABLE        18
#define LTE_GPS_DR_SYNC_TEST  19
#define LTE_GPS_FIXES_STATUS  20
#define LTE_GPS_DR_SYNC_FORCE_HIGH	21
#define LTE_GPS_DR_SYNC_FORCE_LOW	22
#define EM74XX_SIMDETECT_L       23
#define EM74XX_SIMDETECT_H       24
#define EM74XX_SIMDETECT2_L      25
#define EM74XX_SIMDETECT2_H      26
#define EM74XX_SIMDETECT_STAT    27
#define EM74XX_SIMDETECT2_STAT   28
#define WP76XX_UIM1_DET_L        29
#define WP76XX_UIM1_DET_H        30
#define WP76XX_UIM1_DET_STAT     31
#define LTE_SET_IMG_VERZ         32
#define LTE_SET_IMG_ATT          33
#define LTE_SET_IMG_GENC         34
#define LTE_EN_AUTO_SWITCH_IMG   35
#define LTE_PWR_DOWN             36
#define LTE_WP_SIM_PROTECT       37

#define AT_CMD_RESP_TOUT_IN_SEC                 (30)

/* Structure for sending AT commands to modem */
typedef struct at_cmd_str_ {
    char *str;                         /* AT command */
    uint delay;                        /* Wait for resp to above cmd */
} at_cmd_str;

typedef enum {
    USB_2P0_MODE,
    USB_3P0_MODE
} usb_host_mode_t;

/* The commmon AT command sequences for LTE RSSI 7750 value reading */

/* The AT command sequences for LTE RSSI value reading of 7750 modem */
/* SWI 4014 */
static at_cmd_str lte_main_rssi_b8_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 2 },
    { "AT!DAFTMACT\r", 2 },
    { "AT!DASBAND=47\r", 4 },
    { "AT!DALSTXBW=2\r", 4 },
    { "AT!DALSRXBW=2\r", 4 },
    { "AT!DASCHAN=21625\r", 1 },
    { "AT!DALGAVGAGC=21625,0\r", 5 },
};

#define USB_SYSFS_PATH "/sys/bus/usb/drivers/usb/3-1/product"
static const unsigned int lte_main_rssi_b8_no_at_cmd =
    sizeof(lte_main_rssi_b8_at_cmd_str) / sizeof(at_cmd_str);

/* The AT command sequences for LTE RSSI value reading of WP7601 & WP7603 modem */    
static at_cmd_str lte_main_rssi_B4_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 2 },
    { "AT!DAFTMACT\r", 2 },
    { "AT!DASBAND=42\r", 4 },
    { "AT!DALSTXBW=2\r", 4 },
    { "AT!DALSRXBW=2\r", 4 },
    { "AT!DASCHAN=20175\r", 1 },
    { "AT!DALGAVGAGC=20175,0\r", 5 },
};

static const unsigned int lte_main_rssi_B4_no_at_cmd =
    sizeof(lte_main_rssi_B4_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str gps_enable_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!CUSTOM=\"GPSENABLE\",1\r", 1 },
    { "AT!GPSAUTOSTART=1\r", 1 },
    { "AT!RESET\r", 5 },
};

static const char gps_enable_at_cmd_str_size =
    sizeof(gps_enable_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str gps_dis_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!CUSTOM=\"GPSENABLE\",0\r", 10 },
    { "AT!GPSAUTOSTART=0\r", 10 },
    { "AT!RESET\r", 17 },
};

static const char gps_dis_no_at_cmd =
    sizeof(gps_dis_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str gps_autostartdis_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!GPSAUTOSTART=0\r", 1 },
    { "AT!RESET\r", 17 },
};

static const char gps_autostartdis_no_at_cmd =
    sizeof(gps_autostartdis_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str gps_autostarten_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!GPSAUTOSTART=1\r", 1 },
    { "AT!RESET\r", 17 },
};

static const char gps_autostarten_no_at_cmd =
    sizeof(gps_autostarten_at_cmd_str) / sizeof(at_cmd_str);

/* request the current status of a GPS position fixes */
static at_cmd_str lte_gps_fix_at_cmd_str[] = {
    { "AT!GPSSTATUS?\r", 1 },
};

static const unsigned int lte_gps_fix_at_cmd_str_size = 
    sizeof(lte_gps_fix_at_cmd_str) / sizeof(at_cmd_str);

/* GPS DR_SYNC(Dead Reckoning Synchronization) enable */
static at_cmd_str lte_gps_dr_sync_at_cmd_str[] = {
    { "AT!GPSIDREN=1\r", 0 },
};

static const unsigned int lte_gps_dr_sync_at_cmd_str_size = 
    sizeof(lte_gps_dr_sync_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str gps_rssi_7750_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!GPSEND=0\r", 0 },  
    { "AT!DAFTMACT\r", 1 },
    { "AT!DACGPSTESTMODE=1\r", 1 },
    { "AT!DACGPSSTANDALONE=1\r", 1 },
    { "AT!DACGPSMASKON\r", 1 },
    { "AT!DACGPSCTON\r", 5 },
};

static const char gps_rssi_7750_no_at_cmd =
    sizeof(gps_rssi_7750_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str set_gps_port1_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!CUSTOM=\"GPSSEL\",0\r", 5 },
    { "AT!RESET\r", 17 },
};

static const char set_gps_port1_no_at_cmd =
    sizeof(set_gps_port1_str) / sizeof(at_cmd_str);

static at_cmd_str set_gps_port2_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!CUSTOM=\"GPSSEL\",1\r", 5 },
    { "AT!RESET\r", 17 },
};

static const char set_gps_port2_no_at_cmd =
    sizeof(set_gps_port2_str) / sizeof(at_cmd_str);

/* Note that AT!BSGPIO=53,1 is only valid for the WP76xx series hardware.
 * The AT!BSGPIO command with IO number 53 can be used to force the DR_SYNC
 * signal high or low. If the wrong GPIO number is specified, it could 
 * potentially damage the hardware depending on what is attached to the IO pin.
 */
/* GPS DR_SYNC(Dead Reckoning Synchronization) force high */
static at_cmd_str lte_gps_dr_sync_h_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO=53,1\r", 0 },
};

static const unsigned int lte_gps_dr_sync_h_at_cmd_str_size = 
    sizeof(lte_gps_dr_sync_h_at_cmd_str) / sizeof(at_cmd_str);

/* GPS DR_SYNC(Dead Reckoning Synchronization) force low */
static at_cmd_str lte_gps_dr_sync_l_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO=53,0\r", 0 },
};

static const unsigned int lte_gps_dr_sync_l_at_cmd_str_size = 
    sizeof(lte_gps_dr_sync_l_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str rssi_reset_at_cmd_str[] = {
    { "AT!RESET\r", 1 },
};

static const char rssi_reset_no_at_cmd =
    sizeof(rssi_reset_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str rssi_ft_at_cmd_str[] = {
    { "AT!DAFTMDEACT\r", 9 },
};

static const char rssi_ft_no_at_cmd =
    sizeof(rssi_ft_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str rssi_3g_main_b22_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!DAFTMACT\r", 1 },
    { "AT!DASBAND=22\r", 5 },
    { "AT!DASCHAN=4182\r", 1 },
    { "AT!DASLNAGAIN=0\r", 1 },
    { "AT!DAWSSCHAIN=0\r", 1 },
    { "AT!DAWGAVGAGC=4182,0,0\r", 1 },
};

static const char rssi_3g_b22_no_at_cmd =
    sizeof(rssi_3g_main_b22_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str rssi_3g_div_b22_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!DAFTMACT\r", 1 },
    { "AT!DASBAND=22\r", 4 },
    { "AT!DASCHAN=4182\r", 1 },
    { "AT!DASLNAGAIN=0\r", 1 },
    { "AT!DAWSSCHAIN=1\r", 1 },
    { "AT!DAWGAVGAGC=4182,0,1\r", 1 },
};

static const char rssi_div_3g_b22_no_at_cmd =
    sizeof(rssi_3g_div_b22_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str rssi_umts_main_7710_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!DAFTMACT\r", 1 },
    { "AT!DASBAND=29\r", 1 },
    { "AT!DASCHAN=2812\r", 1 },
    { "AT!DASLNAGAIN=0\r", 1 },
    { "AT!DAWGAVGAGC=2812,0\r", 1 },
};
static at_cmd_str rssi_umts_div_7710_at_cmd_str[] = {
    { "AT\r", 1 },
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!DAFTMACT\r", 1 },
    { "AT!DASBAND=29\r", 1 },
    { "AT!DAWSSCHAIN=1\r", 1 },
    { "AT!DASCHAN=2812\r", 1 },
    { "AT!DASLNAGAIN=0\r", 1 },
    { "AT!DAWGAVGAGC=2812,0,1\r", 1 },
};
static const char rssi_umts_main_7710_no_at_cmd =
    sizeof(rssi_umts_main_7710_at_cmd_str) / sizeof(at_cmd_str);
static const char rssi_umts_div_7710_no_at_cmd =
    sizeof(rssi_umts_div_7710_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str lte_sim_at_cmd_str[] = {
    { "at!uims=0\r", 1 }, /* switch to slot 0*/
    { "AT+CPIN?\r", 1 },
};

static const char lte_sim_no_at_cmd =
    sizeof(lte_sim_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str lte_sim1_at_cmd_str[] = {
    { "at!uims=1\r", 1 }, /* switch to slot 1*/
    { "AT+CPIN?\r", 1 },
};

static const char lte_sim1_no_at_cmd =
    sizeof(lte_sim1_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str ati_cmd_str[] = {
    { "ATI\r", 1 },
};

static const char no_ati_at_cmd =
    sizeof(ati_cmd_str) / sizeof(at_cmd_str);

/* AT commands set to check the state of EM74xx Modem SIM_DETECT signal */
/* Based on comment from SWI(Sierra wireless):
 * For EM74xx, AT!BSGPIO?77 can be used to check the state of SIM_DETECT signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str em74xx_simdetect_at_cmd[] = {
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!BSGPIO?77\r", 1 },
};

static const char em74xx_simdetect_at_cmd_size = 
    sizeof(em74xx_simdetect_at_cmd) / sizeof(at_cmd_str);

/* AT commands set to check the state of EM74xx Modem SIM_DETECT_2 signal */
/* Based on comment from SWI(Sierra wireless):
 * For EM74xx, AT!BSGPIO?15 can be used to check the state of SIM_DETECT_2 signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str em74xx_simdetect2_at_cmd[] = {
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!BSGPIO?15\r", 1 },
};

static const char em74xx_simdetect2_at_cmd_size = 
    sizeof(em74xx_simdetect2_at_cmd) / sizeof(at_cmd_str);

/* AT commands set to check the state of WP76xx Modem UIM1_DET signal */
/* Based on comment from SWI(Sierra wireless):
 * For WP76xx, AT!BSGPIO?34 can be used to check the state of UIM1_DET signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str wp76xx_simdetect_at_cmd[] = {
    { "AT!ENTERCND=\"A710\"\r", 1 },
    { "AT!BSGPIO?34\r", 1 },
};

static const char wp76xx_simdetect_at_cmd_size = 
    sizeof(wp76xx_simdetect_at_cmd) / sizeof(at_cmd_str);
    
/* Set modem carrier to Verizon */
static at_cmd_str lte_set_img_verizon_at_cmd_str[] = {
    { "AT!IMPREF=\"VERIZON\"\r", 0 },
};

static const unsigned int lte_set_img_verizon_at_cmd_str_size = 
    sizeof(lte_set_img_verizon_at_cmd_str) / sizeof(at_cmd_str);

/* Set modem carrier to Generic */
static at_cmd_str lte_set_img_generic_at_cmd_str[] = {
    { "AT!IMPREF=\"GENERIC\"\r", 0 },
};

static const unsigned int lte_set_img_generic_at_cmd_str_size = 
    sizeof(lte_set_img_generic_at_cmd_str) / sizeof(at_cmd_str);

/* Set modem carrier to ATT */
static at_cmd_str lte_set_img_att_at_cmd_str[] = {
    { "AT!IMPREF=\"ATT\"\r", 0 },
};

static const unsigned int lte_set_img_att_at_cmd_str_size = 
    sizeof(lte_set_img_att_at_cmd_str) / sizeof(at_cmd_str);

/* Enable modem SIM-based image switching */
static at_cmd_str lte_en_auto_switch_img_at_cmd_str[] = {
    { "AT!IMPREF=\"AUTO-SIM\"\r", 0 },
};

static const unsigned int lte_en_auto_switch_img_at_cmd_str_size = 
    sizeof(lte_en_auto_switch_img_at_cmd_str) / sizeof(at_cmd_str);
    
/* Power down LTE modem */
static at_cmd_str lte_pwr_down_at_cmd_str[] = {
    { "AT!POWERDOWN\r", 0 },
};

static const unsigned int lte_pwr_down_at_cmd_str_size = 
    sizeof(lte_pwr_down_at_cmd_str) / sizeof(at_cmd_str);
    
/* LTE-WP SIM Detect Test */
/* Modem changes to a sim option that is not in use
 * (WP modem only supportd one SIM) before SIM mux switching
 * to avoid power glitch
 */
static at_cmd_str star_lte_wp_sim_protect_str[] = {
    { "at!uims=1\r", 0 }, /* switch to slot 1 */
};

static const unsigned int star_lte_wp_sim_protect_str_size = 
    sizeof(star_lte_wp_sim_protect_str) / sizeof(at_cmd_str);

/* extern */
extern int tsn_get_lte_0_main_rssi(void);
extern int tsn_get_lte_0_div_rssi(void);
extern int tsn_get_lte_0_gps_antennae_test(void);
extern int tsn_get_lte_0_sim_0_card_test(void);
extern int tsn_modem_0_reset_test(void);
extern int lte_utility_main(int);
extern int tsn_modem_0_detect_test(void);
extern int tsn_sim_0_test(void);
extern int tsn_sim_1_test(void);

/* extern */
extern int lte_subsystem_test(int);
extern void insert_test_module(boolean);
extern int lte_reset_init(void);
extern int usb_lte_utility(boolean); 

#endif /*_TSN_HIGH_LTE_H_*/

/*-------------------------------------------------
$Log: lte.h,v $
Revision 1.9  2020/07/10 11:36:50  steja
Enhanced TSN LTE Series
1.CSCvu76591 [TSN-H/TSN-GFAST] Modify SIM_DETECT pin Test item
2.CSCvu72092: [TSN-H/TSN-GFAST] Enhance the LTE USB port number change dynamically
3.CSCvu72089: [TSN-H/TSN-GFAST] Adjust LTE Power Sequence

Revision 1.8.44.1  2020/06/19 08:24:57  steja
1. Adjust LTE Power Sequence based SWI Guideline
2. Dynamically ttyUSB port for AT command
3. Remove LTE init on SIM Detection Test

Revision 1.8  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.7  2018/08/20 01:26:14  lucywang
CSCvm04601 - [Star] LTE SIM card test failed during RDT on C1109-2P

Revision 1.6  2018/07/12 07:24:48  lucywang
CSCvk17720 - [Star] LTE SIM Detect pin(UIM1_DET) issue on C1109-2P

Revision 1.5  2018/06/29 14:13:44  palin2
CSCvk03740: TSN LTE SIM0 SIM_DETECT pin issue.
Enhanced test coverage on EM74xx LTE SIM_DETECT pin.

Revision 1.4  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.3  2018/02/08 07:16:05  lucywang
Merged LTE USB2.0 detect test from trunk

Revision 1.2.20.2  2018/01/22 07:26:37  lucywang
Fixed CSCvh60750, sync from Pluggable-LTE, Set longer polling time for modem reset test

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.7  2017/12/15 06:27:35  lucywang
Sync from Pluggable LTE : Added diagnostic test mode for pluggable LTE-WP76xx GPS pin test

Revision 1.2.4.6  2017/11/17 05:42:34  lucywang
Sync from Pluggable LTE : GPS Antennae test AT command

Revision 1.2.4.5  2017/11/10 08:17:40  lucywang
Sync from Pluggable LTE : Modified the timeout mechanism of GPS pin test

Revision 1.2.4.4  2017/11/06 06:28:16  lucywang
Added GPS pin test for on-board WP module

Revision 1.2.4.3  2017/09/15 03:04:00  lucywang
added timeout for LTE AT commands

Revision 1.2.4.2  2017/08/28 03:34:13  lucywang
modified for C949-2P

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:46  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3.6.2  2017/06/25 06:41:23  tirawan
Initialize GPIO Expander Output port before configuring its direction

Revision 1.1.4.3.6.1  2017/06/22 19:27:10  tirawan
Add LTE Test items and add log section at the bottom of the code

Revision 1.1.4.3.2.3  2017/07/18 03:53:01  steja
Code cleanup

Revision 1.1.4.3.2.2  2017/07/11 10:13:16  steja
1. Remove Debugcard test
2. Add LTE micro usb utility to basic utilities
3. Code clean up

Revision 1.1.4.3.2.1  2017/07/08 07:27:26  steja
Code Clean up

Revision 1.1.4.3  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.3  2016/06/27 09:03:43  steja
Fixed SIM CARD Detect using AT command

Revision 1.1.2.2  2016/06/17 10:37:13  steja
Fix Compiler issue

Revision 1.1.2.1  2016/03/24 03:58:26  steja
Add LTE test



*/



/* $Id: plug_lte_at.c,v 1.15 2020/01/18 07:02:06 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_at.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_at.c - PLUGGABLE LTE AT command functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <proto.h>
#include "nvmonvars.h"
#include "common.h"
#include "types.h"
#include "error.h"
#include "plug_lte_at.h"
#include "plug_lte_lib.h"

static at_cmd_str lte_ati_cmd_str[] = {
    { "ATI\r", 0 },
};

static const unsigned int lte_ati_cmd_str_size = 
    sizeof(lte_ati_cmd_str) / sizeof(at_cmd_str);

/* The AT command sequences for LTE RSSI value reading of 7430/7455 modem */
static at_cmd_str lte_main_rssi_b8_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASBAND=47\r", 0 },
    { "AT!DALSTXBW=2\r", 0 },
    { "AT!DALSRXBW=2\r", 0 },
    { "AT!DASCHAN=21625\r", 0 },
    { "AT!DALGAVGAGC=21625,0\r", 1 },
};

static const unsigned int lte_main_rssi_b8_at_cmd_str_size =
    sizeof(lte_main_rssi_b8_at_cmd_str) / sizeof(at_cmd_str);

/* The AT command sequences for LTE RSSI value reading of WP7601/03 modem
 * Test setting:Band 4(Band ID = 42), Channel = 20175, Frequency = 2134.5 MHz
 */
static at_cmd_str lte_main_rssi_b4_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASBAND=42\r", 0 },
    { "AT!DALSTXBW=2\r", 0 },
    { "AT!DALSRXBW=2\r", 0 },
    { "AT!DASCHAN=20175\r", 0 },
    { "AT!DALGAVGAGC=20175,0\r", 1 },
};

static const unsigned int lte_main_rssi_b4_at_cmd_str_size =
    sizeof(lte_main_rssi_b4_at_cmd_str) / sizeof(at_cmd_str);

/* The AT command sequences for LTE RSSI value reading of WP7607/08/09 modem
 * Test setting:Band 1(Band ID = 34), Channel = 18300, Frequency = 2142 MHz
 */
static at_cmd_str lte_main_rssi_b1_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASBAND=34\r", 0 },
    { "AT!DALSTXBW=2\r", 0 },
    { "AT!DALSRXBW=2\r", 0 },
    { "AT!DASCHAN=18300\r", 0 },
    { "AT!DALGAVGAGC=18300,0\r", 1 },
};

static const unsigned int lte_main_rssi_b1_at_cmd_str_size =
    sizeof(lte_main_rssi_b1_at_cmd_str) / sizeof(at_cmd_str);

/* GPS Antenna Test */
static at_cmd_str gps_rssi_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!GPSEND=0\r", 0 },  
    { "AT!DAFTMACT\r", 0 },
    { "AT!DACGPSTESTMODE=1\r", 0 },
    { "AT!DACGPSSTANDALONE=1\r", 0 },
    { "AT!DACGPSMASKON\r", 0 },
    { "AT!DACGPSCTON\r", 1 },
};

static const unsigned int gps_rssi_at_cmd_str_size = 
    sizeof(gps_rssi_at_cmd_str) / sizeof(at_cmd_str);

/* Enable GPS */
static at_cmd_str gps_enable_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!CUSTOM=\"GPSENABLE\",1\r", 0 },
    { "AT!GPSAUTOSTART=1\r", 0 },
    { "AT!RESET\r", 0 },
};

static const unsigned int gps_enable_at_cmd_str_size = 
    sizeof(gps_enable_at_cmd_str) / sizeof(at_cmd_str);

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
    { "AT!BSGPIO=53,0\r", 0 },
};

static const unsigned int lte_gps_dr_sync_l_at_cmd_str_size = 
    sizeof(lte_gps_dr_sync_l_at_cmd_str) / sizeof(at_cmd_str);

/* Check modem carrier match*/
static at_cmd_str lte_chk_img_carrier_match_at_cmd_str[] = {
    { "AT!IMPREF?\r", 1 },
};

static const unsigned int lte_chk_img_carrier_match_at_cmd_str_size =
    sizeof(lte_chk_img_carrier_match_at_cmd_str) / sizeof(at_cmd_str);

/* Set modem carrier to DOCOMO */
static at_cmd_str lte_set_img_docomo_at_cmd_str[] = {
    { "AT!IMPREF=\"DOCOMO\"\r", 0 },
};

static const unsigned int lte_set_img_docomo_at_cmd_str_size = 
    sizeof(lte_set_img_docomo_at_cmd_str) / sizeof(at_cmd_str);

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

/* Modem Reset */
static at_cmd_str rssi_reset_at_cmd_str[] = {
    { "AT!RESET\r", 1 },
};

static const unsigned int rssi_reset_at_cmd_str_size = 
    sizeof(rssi_reset_at_cmd_str) / sizeof(at_cmd_str);

/* Modem in Boot&Hold mode */
static at_cmd_str lte_boot_mode_at_cmd_str[] = {
    { "AT!BOOTHOLD\r", 0 },
};

static const unsigned int lte_boot_mode_at_cmd_str_size = 
    sizeof(lte_boot_mode_at_cmd_str) / sizeof(at_cmd_str);

/* SIM-0 Detect Test */
static at_cmd_str lte_sim0_detect_str[] = {
    { "at!uims=0\r", 1 }, /* switch to slot 0*/
    { "AT+CPIN?\r", 0 },
};

static const unsigned int lte_sim0_detect_str_size = 
    sizeof(lte_sim0_detect_str) / sizeof(at_cmd_str);

/* SIM-1 Detect Test */
static at_cmd_str lte_sim1_detect_str[] = {
    { "at!uims=1\r", 1 }, /* switch to slot 1*/
    { "AT+CPIN?\r", 0 },
};

static const unsigned int lte_sim1_detect_str_size = 
    sizeof(lte_sim1_detect_str) /sizeof(at_cmd_str);

/* LTE-WP SIM Detect Test */
/* Modem changes to a sim option that is not in use
 * (WP modem only supportd one SIM) before SIM mux switching
 * to avoid power glitch
 */
static at_cmd_str lte_wp_sim_protect_str[] = {
    { "at!uims=1\r", 0 }, /* switch to slot 1 */
};

static const unsigned int lte_wp_sim_protect_str_size = 
    sizeof(lte_wp_sim_protect_str) / sizeof(at_cmd_str);

/* Display LTE modem current temperature */
static at_cmd_str dis_lte_temp_at_cmd_str[] = {
    { "AT!PCTEMP?\r", 1 }, 
};

static const unsigned int dis_lte_temp_at_cmd_str_size = 
    sizeof(dis_lte_temp_at_cmd_str) / sizeof(at_cmd_str);

/* request the current status of LTE modem status */
static at_cmd_str lte_get_stat_at_cmd_str[] = {
    { "AT!GSTATUS?\r", 1 },
};

static const unsigned int lte_get_stat_at_cmd_str_size = 
    sizeof(lte_get_stat_at_cmd_str) / sizeof(at_cmd_str);

/* Power down LTE modem */
static at_cmd_str lte_pwr_down_at_cmd_str[] = {
    { "AT!POWERDOWN\r", 0 },
};

static const unsigned int lte_pwr_down_at_cmd_str_size = 
    sizeof(lte_pwr_down_at_cmd_str) / sizeof(at_cmd_str);

/* AT commands set to check the state of EM74xx Modem SIM_DETECT signal */
/* Based on comment from SWI(Sierra wireless):
 * For EM74xx, AT!BSGPIO?77 can be used to check the state of SIM_DETECT signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str lte_em74xx_simdetect_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO?77\r", 1 },
};

static const unsigned int lte_em74xx_simdetect_at_cmd_str_size = 
    sizeof(lte_em74xx_simdetect_at_cmd_str) / sizeof(at_cmd_str);

/* AT commands set to check the state of EM74xx Modem SIM_DETECT_2 signal */
/* Based on comment from SWI(Sierra wireless):
 * For EM74xx, AT!BSGPIO?15 can be used to check the state of SIM_DETECT_2 signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str lte_em74xx_simdetect2_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO?15\r", 1 },
};

static const unsigned int lte_em74xx_simdetect2_at_cmd_str_size = 
    sizeof(lte_em74xx_simdetect2_at_cmd_str) / sizeof(at_cmd_str);

/* AT commands set to check the state of WP76xx Modem UIM1_DET signal */
/* Based on comment from SWI(Sierra wireless):
 * For WP76xx, AT!BSGPIO?34 can be used to check the state of UIM1_DET signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str lte_wp76xx_uim1det_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO?34\r", 1 },
};

static const unsigned int lte_wp76xx_uim1det_at_cmd_str_size = 
    sizeof(lte_wp76xx_uim1det_at_cmd_str) / sizeof(at_cmd_str);


int plug_lte_at_run_cmd(int);
static int plug_lte_at_open_tty(int *);
static int plug_lte_process_at_cmd(int, char *, int, int);
static int plug_lte_at_selftest(int);

/***************************************************************************
* Name: plug_lte_at_run_cmd
*
* Description: This function sends AT command to modem
* 
* Input: at_test = which command want to execute
*
* Output: PASSED/FAILED
***************************************************************************/
int plug_lte_at_run_cmd (int at_test)
{
    int tty_dev_fd;
    at_cmd_str *at_cmd;
    int at_cmd_length;
    int ix;

    /* Polling modem AT command usb device status in case modem is not ready */
    if (plug_lte_check_modem_rdy(TRUE) != PASSED) {
        printf("Pluggable LTE is not ready\n");
        return (FAILED);
    }
    /* Get TTY Device file descriptor */
    if (plug_lte_at_open_tty(&tty_dev_fd) == FAILED) {
        return (FAILED);
    }

    /* To fix CSCvh79986 and CSCvh79979, ensure communication between host
     * and modem is good */
    if (plug_lte_at_selftest(tty_dev_fd) == FAILED) {
        return (FAILED);
    }

    switch (at_test) {
    case RSSI_LTE_B8_MAIN_TEST:
    case RSSI_LTE_B8_DIV_TEST:
        at_cmd = lte_main_rssi_b8_at_cmd_str;
        at_cmd_length = lte_main_rssi_b8_at_cmd_str_size;
        break;
    case RSSI_LTE_B4_MAIN_TEST:
    case RSSI_LTE_B4_DIV_TEST:
        at_cmd = lte_main_rssi_b4_at_cmd_str;
        at_cmd_length = lte_main_rssi_b4_at_cmd_str_size;
        break;
    case RSSI_LTE_B1_MAIN_TEST:
    case RSSI_LTE_B1_DIV_TEST:
        at_cmd = lte_main_rssi_b1_at_cmd_str;
        at_cmd_length = lte_main_rssi_b1_at_cmd_str_size;
        break;
    case RSSI_LTE_GPS_TEST:
        at_cmd = gps_rssi_at_cmd_str;
        at_cmd_length = gps_rssi_at_cmd_str_size;
        break;
    case RSSI_LTE_RESET_TEST:
        at_cmd = rssi_reset_at_cmd_str;
        at_cmd_length = rssi_reset_at_cmd_str_size;
        break;
    case RSSI_LTE_ATI_TEST:
        at_cmd = lte_ati_cmd_str;
        at_cmd_length = lte_ati_cmd_str_size;
        break;
    case LTE_SIM0_DETECT_TEST:
        at_cmd = lte_sim0_detect_str;
        at_cmd_length = lte_sim0_detect_str_size;
        break;
    case LTE_SIM1_DETECT_TEST:
        at_cmd = lte_sim1_detect_str;
        at_cmd_length = lte_sim1_detect_str_size;
        break;
    case LTE_WP_SIM_PROTECT:
        at_cmd = lte_wp_sim_protect_str;
        at_cmd_length = lte_wp_sim_protect_str_size;
        break;
    case LTE_GPS_ENABLE:
        at_cmd = gps_enable_at_cmd_str;
        at_cmd_length = gps_enable_at_cmd_str_size;
        break;
    case LTE_GPS_FIXES_STATUS:
        at_cmd = lte_gps_fix_at_cmd_str;
        at_cmd_length = lte_gps_fix_at_cmd_str_size;
        break;
    case LTE_GPS_DR_SYNC_TEST:
        at_cmd = lte_gps_dr_sync_at_cmd_str;
        at_cmd_length = lte_gps_dr_sync_at_cmd_str_size;
        break;
    case LTE_GPS_DR_SYNC_FORCE_HIGH:
        at_cmd = lte_gps_dr_sync_h_at_cmd_str;
        at_cmd_length = lte_gps_dr_sync_h_at_cmd_str_size;
        break;
    case LTE_GPS_DR_SYNC_FORCE_LOW:
        at_cmd = lte_gps_dr_sync_l_at_cmd_str;
        at_cmd_length = lte_gps_dr_sync_l_at_cmd_str_size;
        break;
    case PLUG_LTE_BOOT_MODE:
        at_cmd = lte_boot_mode_at_cmd_str;
        at_cmd_length = lte_boot_mode_at_cmd_str_size;
        break;
    case DUMP_LTE_MODEM_TEMP:
        at_cmd = dis_lte_temp_at_cmd_str;
        at_cmd_length = dis_lte_temp_at_cmd_str_size;
        break;
    case LTE_GET_MODEM_STATUS:
        at_cmd = lte_get_stat_at_cmd_str;
        at_cmd_length = lte_get_stat_at_cmd_str_size;
        break;
    case PLUG_LTE_PWR_DOWN:
        at_cmd = lte_pwr_down_at_cmd_str;
        at_cmd_length = lte_pwr_down_at_cmd_str_size;
        break;
    case PLUG_LTE_CHK_IMG_CARRIER_MATCH:
        at_cmd = lte_chk_img_carrier_match_at_cmd_str;
        at_cmd_length = lte_chk_img_carrier_match_at_cmd_str_size;
        break;
    case PLUG_LTE_SET_IMG_GENC:
        at_cmd = lte_set_img_generic_at_cmd_str;
        at_cmd_length = lte_set_img_generic_at_cmd_str_size;
        break;
    case PLUG_LTE_SET_IMG_ATT:
        at_cmd = lte_set_img_att_at_cmd_str;
        at_cmd_length = lte_set_img_att_at_cmd_str_size;
        break;
    case PLUG_LTE_SET_IMG_VERZ:
        at_cmd = lte_set_img_verizon_at_cmd_str;
        at_cmd_length = lte_set_img_verizon_at_cmd_str_size;
        break;
    case PLUG_LTE_SET_IMG_DOCOMO:
        at_cmd = lte_set_img_docomo_at_cmd_str;
        at_cmd_length = lte_set_img_docomo_at_cmd_str_size;
        break;
    case PLUG_LTE_EN_AUTO_SWITCH_IMG:
        at_cmd = lte_en_auto_switch_img_at_cmd_str;
        at_cmd_length = lte_en_auto_switch_img_at_cmd_str_size;
        break;
    case EM74XX_SIMDETECT_L:
    case EM74XX_SIMDETECT_H:
    case EM74XX_SIMDETECT_STAT:
        at_cmd = lte_em74xx_simdetect_at_cmd_str;
        at_cmd_length = lte_em74xx_simdetect_at_cmd_str_size;
        break;
    case EM74XX_SIMDETECT2_L:
    case EM74XX_SIMDETECT2_H:
    case EM74XX_SIMDETECT2_STAT:
        at_cmd = lte_em74xx_simdetect2_at_cmd_str;
        at_cmd_length = lte_em74xx_simdetect2_at_cmd_str_size;
        break;
    case WP76XX_UIM1DET_L:
    case WP76XX_UIM1DET_H:
    case WP76XX_UIM1DET_STAT:
        at_cmd = lte_wp76xx_uim1det_at_cmd_str;
        at_cmd_length = lte_wp76xx_uim1det_at_cmd_str_size;
        break;
    default:
        printf("%s: Not supported AT command ('%d')\n", __func__, at_test);
        return (FAILED);
    }

    /* Process AT commands */
    for (ix = 0; ix < at_cmd_length; ix++) {
        if (plug_lte_process_at_cmd(tty_dev_fd, at_cmd[ix].str, at_test, 
                                    at_cmd[ix].parse_the_result) == FAILED) {
            close(tty_dev_fd);
            return (FAILED);
        }
    }

    close(tty_dev_fd);
    return (PASSED);
}


/***************************************************************************
* Name: plug_lte_process_at_cmd 
*
* Description: This function sends AT command, and expects the result
*              from the modem
* 
* Input: fd - TTY file descriptor
*        atcmd_str - AT Command String
*        at_test - What AT test is being executed
*        parse_result - Need to parse the result of AT command or just
*                       expecting OK from the modem
*
* Output: PASSED/FAILED
***************************************************************************/
static int plug_lte_process_at_cmd (int fd, char *atcmd_str,
                                    int at_test, int parse_result)
{
    char buffer[AT_CMD_BUFFER_SIZE] = {0,};
    char *bufptr;
    int nbytes = 0;
    int atcmd_length = strlen(atcmd_str);
    fd_set tout_set;
    struct timeval timeout;
    int ret;
    int high_pwr, low_pwr;
    int db, cton, freq;
    char *rslt_str, *rslt_str1, *rslt_str2, *rslt_str3;
    ssize_t wl;
    
    /* Read from tty with timeout */
    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_CMD_RESP_TOUT_IN_SEC;
    timeout.tv_usec = 0;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nAT Command is %s\n", atcmd_str);
        fflush(stdout);
    }

    /* Transmit AT command to modem */
    if ((wl = write(fd, atcmd_str, atcmd_length)) < atcmd_length) {
        printf("%s: Fail to write AT command: %zd\n", __func__, wl);
        if (wl < 0) {
            printf("syscall write: %s\n", strerror(errno));
        }
        return (FAILED);
    }

    bufptr = buffer;
    ret = select(fd + 1, &tout_set, NULL, NULL, &timeout);

    if (ret < 1) {
        printf("%s: Modem is not responding to AT command\n", __func__);
        return (FAILED);
    }

    while ((nbytes = read(fd, bufptr, AT_CMD_BUFFER_SIZE)) > 0) {
        bufptr += nbytes;

        if (bufptr[-1] == '\n' || bufptr[-1] == '\r') { 
            break;
        }
    }

    *bufptr = '\0';

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("AT_COMMAND \n%s\n", buffer);
        fflush(stdout);
    }

    /* Now, process the string */
    if (parse_result == 0) {
        /* No need to digest the result, 'OK' from modem is enough */
        if (strstr(buffer, "OK") != 0) {
            return (PASSED);
        } else {
            printf("No 'OK' from the modem, (%s)\n", buffer);
            return (FAILED);
        }
    } else {
        /* Need to parse the result based on what AT command we are sending
         * to modem
         */
        switch (at_test) {
            case RSSI_LTE_B8_MAIN_TEST:
            case RSSI_LTE_B8_DIV_TEST:
            case RSSI_LTE_B4_MAIN_TEST:
            case RSSI_LTE_B4_DIV_TEST:
            case RSSI_LTE_B1_MAIN_TEST:
            case RSSI_LTE_B1_DIV_TEST:
                high_pwr = MAIN_DIV_RSSI_AMP_DBM + MAIN_DIV_RSSI_DELTA;
                low_pwr  = MAIN_DIV_RSSI_AMP_DBM - MAIN_DIV_RSSI_DELTA;
                db = 0;

                /* Find the occurance of -xx.y dBm
                 *
                 * Here is the example of AT command output
                 * AT!DALGAVGAGC=20175,0
                 * Paths: 2
                 * Rx0:  AGC: -94.5 dBm  LNA: 0 Chain: 0
                 * Rx1:  AGC: -100.2 dBm  LNA: 0 Chain: 1
                 */
                rslt_str1 = (char *)strchr(buffer, '-');
                /* Search AGC of Rx1 for Div RSSI */
                if (at_test == RSSI_LTE_B8_DIV_TEST || 
                    at_test == RSSI_LTE_B4_DIV_TEST ||
                    at_test == RSSI_LTE_B1_DIV_TEST) {
                    if (rslt_str1) {
                      rslt_str1++;
                      rslt_str1 = (char *)strchr(rslt_str1, '-');
                    }
                } 
                
                /* Now separate the number and dBm */
                if (rslt_str1) {
                    rslt_str2 = (char *) strstr(rslt_str1, " dBm");
                    rslt_str2 = NULL;
                }

                if (rslt_str1) {
                    printf("Result is %s\n", rslt_str1);
                    db = atoi(rslt_str1);
                }

                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("db is %d\n", db);
                }

                if ((db > high_pwr) || (db < low_pwr)) {
                    printf("\nTest Failed dBm = %d expected dBm between %d and "
                                               "%d dBm\n***If Stop on error ...download kernel\n", db, high_pwr, low_pwr);
                    printf("Reading RSSI = %d dBm Test failed"
                          "\nWarning: Please verify the settings of the signal generator.", db);
                } else {
                    printf("\n Test Passed dbm = %d", db);
                    return (PASSED);
                }

                break;
            case RSSI_LTE_GPS_TEST:
                /* Example output:
                 * CtoN=31.7, Freq=100202
                 */
                rslt_str = strstr(buffer, "=");
                rslt_str1 = strstr(buffer, ".");
                if ((rslt_str == NULL) || (rslt_str1 == NULL)) {  
                    printf("Could not collect GPS RF data for CTON\n");
                    return (FAILED);
                }
                rslt_str++;
                rslt_str2 = strstr(rslt_str, "=");
                if (rslt_str2 == NULL) {
                    printf("Could not collect GPS RF data for Freq\n");
                    return (FAILED);
                }
                rslt_str2++;
                rslt_str1 = NULL;
                rslt_str3 = strstr(rslt_str2, "\n");
                if (rslt_str3 == NULL) {
                    printf("Could not collect GPS RF data for Freq "
                           "(no newline)\n");
                    return (FAILED);
                }
                rslt_str3 = NULL;
                cton = atoi(rslt_str);
                freq = atoi(rslt_str2);

                if (((cton <= GPS_CTON_MAX) && (cton >= GPS_CTON_MIN)) && 
                   ((freq <= GPS_TEST_FREQ_MAX) && (freq >= GPS_TEST_FREQ_MIN))) {
                    printf("\n GPS RF Passed CtoN = %ddBm, Freq = %dHz\n", cton, freq);
                        return (PASSED);
                } else {
                    printf("\n ***GPS RF FAILED CtoN = %ddBm, Freq = %dHz\n"
                           " ***If Stop on error ... download kernel again and "
                           "Set GPSAUTOSTART\n", cton, freq);
                    printf(" GPS RF Failed CtoN = %ddB, Freq = %dHz\n"
                          " CtoN should be within 58 +/- 5dBm and Freq within "
                          "100000 Hz +/- 5000 Hz.\n", cton, freq);
                    return (FAILED);
                }
                break;
            case LTE_GPS_FIXES_STATUS:
                rslt_str = strstr(buffer, "SUCCESS");
                rslt_str1 = strstr(buffer, "ACTIVE");
                if ((rslt_str != NULL) && (rslt_str1 != NULL)) {
                    printf("\n GPS position fix session is active.\n");
                    return (PASSED);
                }
                break;
            case LTE_GET_MODEM_STATUS:
                rslt_str = strstr(buffer, "ONLINE");
                if (rslt_str != NULL) {
                    printf("\n LTE modem is online.\n");
                    return (PASSED);
                }
                break;
            case DUMP_LTE_MODEM_TEMP:
                printf("LTE modem temperature info:\n%s\n", buffer);
                return (PASSED);
                break;
            case LTE_SIM0_DETECT_TEST:
            case LTE_SIM1_DETECT_TEST:
                if (strstr(buffer, "OK") != 0) {
                    msleep(SIM_INIT_DELAY);
                    return (PASSED);
                } else {
                    printf("No 'OK' from the modem, (%s)\n", buffer);
                    return (FAILED);
                }
            case RSSI_LTE_RESET_TEST:
                return (PASSED); 
                break;
            case EM74XX_SIMDETECT_L:
            case EM74XX_SIMDETECT2_L:
            case WP76XX_UIM1DET_L:
                if (strstr(buffer, "State:     0") != 0) {
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case EM74XX_SIMDETECT_H:
            case EM74XX_SIMDETECT2_H:
            case WP76XX_UIM1DET_H:
                if (strstr(buffer, "State:     1") != 0) {
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case EM74XX_SIMDETECT_STAT:
            case EM74XX_SIMDETECT2_STAT:
            case WP76XX_UIM1DET_STAT:
                if (strstr(buffer, "State:     0") != 0) {
                    printf("SIM_DETECT pin current state: 0.\n"); 
                } else if (strstr(buffer, "State:     1") != 0) {
                    printf("SIM_DETECT pin current state: 1.\n"); 
                }

                if (strstr(buffer, "OK") != 0) {
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case PLUG_LTE_CHK_IMG_CARRIER_MATCH:
                if (strstr(buffer, "carrier name mismatch") != 0) {
                    /* Carrier "Mismatch" */
                    printf("Modem Carrier 'Mismatch'\n");
                } else {
                    /* Carrier "Match" */
                    if (strstr(buffer, "OK") != 0) {
                        printf("Modem Carrier 'Match'\n");
                        return (PASSED);
                    } else {
                        return (FAILED);
                    }
                }
                break;
        }
    }

    return (FAILED);
}


/***************************************************************************
* Name: plug_lte_at_open_tty
*
* Description: This function opens tty device for AT command 
* 
* Input: *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int plug_lte_at_open_tty (int *tty_fd)
{
    int fd;
    struct termios options;
    int timeout = VTIME_TIMEOUT;
    char usb_tty_dev[256];
    char usb_tty[15];

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */ 
    if (plug_lte_get_tty_devname(usb_tty_dev) != PASSED) {
        printf("%s: Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }

    sprintf(usb_tty, "%s%s", USB_TTY_PATH, usb_tty_dev);
    fd = open(usb_tty, O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fd == -1) {
        printf("%s: Can't open tty device\n", __func__);
        return (FAILED);
    }

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN] = 0xFF;
    options.c_cc[VTIME] = timeout;
    tcsetattr(fd, TCSANOW, &options);

    *tty_fd = fd;

    return (PASSED);
}


/***************************************************************************
* Name: plug_lte_fd_selftest
*
* Description: This function sends "AT" command to ensure the communication 
*              between host and modem is good 
* 
* Input: *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int plug_lte_at_selftest (int fd)
{
    struct timeval timeout;
    char *test_str = "AT";
    char cr = '\r';
    char buffer[AT_CMD_BUFFER_SIZE] = {0,};
    char *bufptr;
    fd_set tout_set;
    int ix, ret, stat = FAILED;
 
    for (ix = 0; ix < MAX_SELFTEST_RETRY; ix++) {
        bufptr = buffer;
 
        FD_ZERO(&tout_set);
        FD_SET(fd, &tout_set);
        timeout.tv_sec  = AT_SELFTEST_TOUT_IN_SEC;
        timeout.tv_usec = 0;
 
        ret = select(fd + 1, &tout_set, NULL, NULL, &timeout);
 
        if (ret) {
            read(fd, bufptr, AT_CMD_BUFFER_SIZE);
 
            if (strstr(buffer, test_str) != 0) {
                stat = PASSED;
                break;
            }
            msleep(AT_SELFTEST_DELAY);
        }
        write(fd, test_str, strlen(test_str));
        write(fd, &cr, 1);
    }

    if (stat != PASSED) {
        printf("%s: Modem communication selftest failed\n", __func__);
        return (FAILED);
    }

    /* Flush buffer */
    ret = select(fd + 1, &tout_set, NULL, NULL, &timeout);
    if (ret) {
        read(fd, bufptr, AT_CMD_BUFFER_SIZE);
        printf("buf:%s\n", buffer);
    }

    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_lte_at.c,v $
Revision 1.15  2020/01/18 07:02:06  sherliu2
Modify WP7605 test carrier firmware from Generic to DOCOMO.

Revision 1.14  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.13  2020/01/09 01:02:30  jiajliu
Merge Curie 2RU to main trunk

Revision 1.12  2019/06/14 05:48:10  shjung
Supported WP7605 modules

Revision 1.11  2018/11/23 09:15:07  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.10.6.3  2018/11/21 01:02:50  shjung
Added GPIO expander test register table and modified RF test macro name based on test RF band

Revision 1.10.6.2  2018/10/15 07:43:26  shjung
Re-struct for pluggable-LTE common codes

Revision 1.10.6.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.10  2018/08/17 02:22:39  shjung
Correct AT command string length

Revision 1.9  2018/07/12 09:33:41  shjung
Fixed CSCvk20378:Covered pluggable LTE modem SIM_DETECT pin

Revision 1.8  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.7  2018/05/21 08:11:29  shjung
Merged code from star-branch-c110x

Revision 1.6  2018/04/13 09:34:59  shjung

1. Fix CSCvh79986 and CSCvh79979: Added modem tty device file descriptor
   slef test to ensure communication between host and modem is good
2. Modified code based on Pluggable LTE WP7601/03 ER code review
3. Put all cterr functions to the outer file
4. Modified modem USB device enumeration timeout and GPS pin vaule polling
   timeout

Revision 1.5  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4  2018/02/26 09:56:42  shjung
Code clean up

Revision 1.3.2.6  2018/05/17 02:53:41  shjung

1. Added delay for SIM initialization while switching UIM interface
2. Clear SAFE_PWR_REMOVAL signal after powered down WP module
3. Added 15.5 seconds boot-up delay for WP module based on spec.

Revision 1.3.2.4  2018/04/11 09:21:05  shjung

1. Remove modem reset test and add modem soft reset utility
2. Code modified based on pluggable LTE EM7455 ER code review

Revision 1.3.2.3  2018/03/23 06:15:36  shjung
Slow down USB write speed from host to LTE modem

Revision 1.3.2.2  2018/03/16 07:47:28  shjung
1. Correct the default value of WP SAFE_POWER_REMOVAL and implement WP modem power-off function 2. Check modem status after hard reset

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:45  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.4  2018/02/01 23:40:59  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.3  2018/01/22 01:09:01  shjung
Code clean up

Revision 1.1.4.11  2018/01/22 01:07:12  shjung
Code clean up

Revision 1.1.4.10  2017/12/13 08:33:24  shjung
Added diagnostic test mode for pluggable LTE-WP76xx GPS pin test

Revision 1.1.4.9  2017/12/08 12:28:46  shjung
Check if usb device attaches to tty successfully before capture corresponding ttyUSB number

Revision 1.1.4.8  2017/12/06 13:45:49  shjung
Modified SIM detection on LTE-WP modem to avoid power glitch

Revision 1.1.4.7  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.6  2017/11/08 02:56:07  shjung
Modified the timeout mechanism of GPS pin test

Revision 1.1.4.5  2017/10/30 14:15:13  shjung
Added GPS pin test for LTE-WP module

Revision 1.1.4.4  2017/08/31 06:07:36  shjung
Fixed GPS antenna hanging issue.

Revision 1.1.4.3  2017/08/30 02:03:46  shjung
Update AT command for modem reset and ensure modem finish reset test

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.1  2017/07/24 22:51:25  tirawan
Add Pluggable AT command functions


*/


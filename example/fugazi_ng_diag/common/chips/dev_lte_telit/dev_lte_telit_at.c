/* $Id: dev_lte_telit_at.c,v 1.8 2020/08/19 09:48:53 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_telit/dev_lte_telit_at.c,v $
 *------------------------------------------------------------------
 *
 * Filename:	dev_lte_telit_at.c
 *
 * Description:	LTE Telit AT Command Driver.
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "common.h"
#include "proto.h"

#include "dev_lte_telit.h"
#include "dev_lte_telit_at.h"

static int dev_lte_telit_dbg = 0;

int dev_lte_telit_at_run_cmd(dev_lte_telit_object_t *, int);
void dev_lte_telit_store_expected_img(char *);
void dev_lte_set_modem_pwrsav_para(int);

static int dev_lte_telit_at_open_tty(char *, int *);


/* AT command to detect modem */
static at_cmd_str lte_ati_cmd_str[] = {
    {"ATI4\r", 0},
};

static const unsigned int lte_ati_cmd_str_size =
    sizeof(lte_ati_cmd_str) / sizeof(at_cmd_str);

/* AT command to reboot modem */
static at_cmd_str lte_reboot_at_cmd_str[] = {
    {"AT#REBOOT\r", 0},
};

static const unsigned int lte_reboot_at_cmd_str_size =
    sizeof(lte_reboot_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to dump modem temperature */
static at_cmd_str lte_dump_temp_at_cmd_str[] = {
    {"AT#TEMPSENS=2\r", 1},
};

static const unsigned int lte_dump_temp_at_cmd_str_size =
    sizeof(lte_dump_temp_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to dump modem_information */
static at_cmd_str lte_dump_modem_info_at_cmd_str[] = {
    {"AT#ACTIVEFW?\r", 1},
};

static const unsigned int lte_dump_modem_info_at_cmd_str_size =
    sizeof(lte_dump_modem_info_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check the level of functionality of the modem */
static at_cmd_str lte_check_func_level_at_cmd_str[] = {
    {"AT+CFUN?\r", 1},
};

static const unsigned int lte_check_func_level_at_cmd_str_size =
    sizeof(lte_check_func_level_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check shutdown indicator */
static at_cmd_str lte_chk_softshdn_indicator_at_cmd_str[] = {
    {"AT#SHDNIND?\r", 1},
};

static const unsigned int lte_chk_softshdn_indicator_at_cmd_str_size =
    sizeof(lte_chk_softshdn_indicator_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set shutdown indicator */
static at_cmd_str lte_set_shdn_indicator_at_cmd_str[] = {
    {"AT#SHDNIND=3,3\r", 0},
    {"AT#REBOOT\r", 0},
};

static const unsigned int lte_set_shdn_indicator_at_cmd_str_size =
    sizeof(lte_set_shdn_indicator_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to disable shutdown indicator */
static at_cmd_str lte_disable_shdn_indicator_at_cmd_str[] = {
    {"AT#SHDNIND=0\r", 0},
    {"AT#REBOOT\r", 0},
};

static const unsigned int lte_disable_shdn_indicator_at_cmd_str_size =
    sizeof(lte_disable_shdn_indicator_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check dying gasp status */
static at_cmd_str lte_chk_dg_stat_at_cmd_str[] = {
    {"AT#GETCUSTFEAT=\"DGENABLE\"\r", 1},
};

static const unsigned int lte_chk_dg_stat_at_cmd_str_size =
    sizeof(lte_chk_dg_stat_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to disable dying gasp */
static at_cmd_str lte_disable_dg_at_cmd_str[] = {
    {"AT#SETCUSTFEAT=\"DGENABLE\",0\r", 0},
};

static const unsigned int lte_disable_dg_at_cmd_str_size =
    sizeof(lte_disable_dg_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to enable dying gasp */
static at_cmd_str lte_enable_dg_at_cmd_str[] = {
    {"AT#SETCUSTFEAT=\"DGENABLE\",150\r", 0},
};

static const unsigned int lte_enable_dg_at_cmd_str_size =
    sizeof(lte_enable_dg_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check fast shutdown status */
static at_cmd_str lte_chk_fastshdn_stat_at_cmd_str[] = {
    {"AT#FASTSHDN?\r", 1},
};

static const unsigned int lte_chk_fastshdn_stat_at_cmd_str_size =
    sizeof(lte_chk_fastshdn_stat_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to disable fast shutdown */
static at_cmd_str lte_dis_fastshdn_at_cmd_str[] = {
    {"AT#FASTSHDN=0\r", 0},
};

static const unsigned int lte_dis_fastshdn_at_cmd_str_size =
    sizeof(lte_dis_fastshdn_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to enable fast shutdown and monitor shutdown indicator */
static at_cmd_str lte_en_fastshdn_at_cmd_str[] = {
    {"AT#FASTSHDN=1,5\r", 0},
    {"AT#SHDNIND=3,3\r", 0},
    {"AT#REBOOT\r", 0},
};

static const unsigned int lte_en_fastshdn_at_cmd_str_size =
    sizeof(lte_en_fastshdn_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check audio disabling status */
static at_cmd_str lte_chk_audio_dis_at_cmd_str[] = {
    {"AT#AUDIS?\r", 1},
};

static const unsigned int lte_chk_audio_dis_at_cmd_str_size =
    sizeof(lte_chk_audio_dis_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to disable audio */
static at_cmd_str lte_dis_audio_at_cmd_str[] = {
    {"AT#AUDIS=1\r", 0},
};

static const unsigned int lte_dis_audio_at_cmd_str_size =
    sizeof(lte_dis_audio_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check modem GPIO_05 status */
static at_cmd_str lte_chk_gpio5_stat_at_cmd_str[] = {
    {"AT#GPIO=5,2\r", 1},
};

static const unsigned int lte_chk_gpio5_stat_at_cmd_str_size =
    sizeof(lte_chk_gpio5_stat_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to power down modem */
static at_cmd_str lte_pwr_down_at_cmd_str[] = {
    {"AT#SHDN\r", 0},
};

static const unsigned int lte_pwr_down_at_cmd_str_size =
    sizeof(lte_pwr_down_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to check the modem power saving mode */
static at_cmd_str lte_chk_pwrsav_at_cmd_str[] = {
    {"AT#PSMWDISACFG?\r", 1},
};

static const unsigned int lte_chk_pwrsav_at_cmd_str_size =
    sizeof(lte_chk_pwrsav_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set modem power saving mode */
static at_cmd_str lte_set_pwrsav_at_cmd_str[] = {
    {"AT#PSMWDISACFG=", 0},
};

static const unsigned int lte_set_pwrsav_at_cmd_str_size =
    sizeof(lte_set_pwrsav_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to switch modem to USB2.0 mode */
static at_cmd_str lte_switch_modem_usb2p0_at_cmd_str[] = {
    {"AT#USBSWITCH=1\r", 0},
    {"AT#REBOOT\r", 0},
};

static const unsigned int lte_switch_modem_usb2p0_at_cmd_str_size =
    sizeof(lte_switch_modem_usb2p0_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to switch modem to USB3.0 mode */
static at_cmd_str lte_switch_modem_usb3p0_at_cmd_str[] = {
    {"AT#USBSWITCH=0\r", 0},
    {"AT#REBOOT\r", 0},
};

static const unsigned int lte_switch_modem_usb3p0_at_cmd_str_size =
    sizeof(lte_switch_modem_usb3p0_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to detect SIM card in SIM interface 1 */
static at_cmd_str lte_sim1_detect_at_cmd_str[] = {
    {"AT#SIMDET=0\r", 0},
    {"AT+CMEE=2\r", 0},
    {"AT+CPIN?\r", 0},
};

static const unsigned int lte_sim1_detect_at_cmd_str_size =
    sizeof(lte_sim1_detect_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to detect SIM card in SIM interface 2 */
static at_cmd_str lte_sim2_detect_at_cmd_str[] = {
    {"AT#SIMDET=1\r", 0},
    {"AT+CMEE=2\r", 0},
    {"AT+CPIN?\r", 0},
};

static const unsigned int lte_sim2_detect_at_cmd_str_size =
    sizeof(lte_sim2_detect_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to read SIMIN1 pin status */
static at_cmd_str lte_query_simin1_status_at_cmd_str[] = {
    {"AT#SIMDET=0\r", 0},
    {"AT#SIMDET?\r", 1},
};

static const unsigned int lte_query_simin1_status_at_cmd_str_size =
    sizeof(lte_query_simin1_status_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to read SIMIN2 pin status */
static at_cmd_str lte_query_simin2_status_at_cmd_str[] = {
    {"AT#SIMDET=1\r", 0},
    {"AT#SIMDET?\r", 1},
};

static const unsigned int lte_query_simin2_status_at_cmd_str_size =
    sizeof(lte_query_simin2_status_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set gps antenna test*/
/* According to the vendor suggestion, get value after send AT#TESTMODE="GNSS"
 * 5~10 times */
static at_cmd_str lte_gps_antenna_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT$GPSANTPORT=2\r", 0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},  
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,1},
};

static const unsigned int lte_gps_antenna_at_cmd_str_size =
    sizeof(lte_gps_antenna_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to set Rx RSSI test configuration with 4G band 2 carrier */
static at_cmd_str lte_rssi_4g_b2_config_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"SETLTEBAND 2\"\r", 0},
    {"AT#TESTMODE=\"LTXBW\"\r", 0},
    {"AT#TESTMODE=\"LRXBW\"\r", 0},
    {"AT#TESTMODE=\"CH 18900\"\r", 0},
    {"AT#TESTMODE=\"LNA4G\"\r", 0},
};

static const unsigned int lte_rssi_4g_b2_config_at_cmd_str_size =
    sizeof(lte_rssi_4g_b2_config_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set Rx RSSI test configuration with 4G band 30 carrier */
static at_cmd_str lte_rssi_4g_b30_config_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"SETLTEBAND 30\"\r", 0},
    {"AT#TESTMODE=\"LTXBW\"\r", 0},
    {"AT#TESTMODE=\"LRXBW\"\r", 0},
    {"AT#TESTMODE=\"CH 27710\"\r", 0},
    {"AT#TESTMODE=\"LNA4G\"\r", 0},
};

static const unsigned int lte_rssi_4g_b30_config_at_cmd_str_size =
    sizeof(lte_rssi_4g_b30_config_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to return primary RF Rx RSSI test power level */
static at_cmd_str lte_rssi_read_pri_rx_pwr_at_cmd_str[] = {
    {"AT#TESTMODE=\"PRXRL4G\"\r", 1},
};

static const unsigned int lte_rssi_read_pri_rx_pwr_at_cmd_str_size =
    sizeof(lte_rssi_read_pri_rx_pwr_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to return secondary RF Rx RSSI test power level */
static at_cmd_str lte_rssi_read_sec_rx_pwr_at_cmd_str[] = {
    {"AT#TESTMODE=\"DRXRL4G\"\r", 1},
};

static const unsigned int lte_rssi_read_sec_rx_pwr_at_cmd_str_size =
    sizeof(lte_rssi_read_sec_rx_pwr_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to exit test mode */
static at_cmd_str lte_exit_test_mode_at_cmd_str[] = {
    {"AT#TESTMODE=\"ESC\"\r", 0},
};

static const unsigned int lte_exit_test_mode_at_cmd_str_size = 
    sizeof(lte_exit_test_mode_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to force the LTE module in Operation Mode */
static at_cmd_str lte_enable_op_mode_at_cmd_str[] = {
    {"AT#TESTMODE=\"OM\"\r", 0},
};

static const unsigned int lte_enable_op_mode_at_cmd_str_size =
    sizeof(lte_enable_op_mode_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to force the LTE module in Test Mode */
static at_cmd_str lte_enable_test_mode_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
};

static const unsigned int lte_enable_test_mode_at_cmd_str_size =
    sizeof(lte_enable_test_mode_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to query the LTE modem mode */
static at_cmd_str lte_query_testmode_stat_at_cmd_str[] = {
    {"AT#TESTMODE?\r", 1},
};

static const unsigned int lte_query_testmode_stat_at_cmd_str_size =
    sizeof(lte_query_testmode_stat_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to keep WWAN_LED off when modem is in LPM */
static at_cmd_str lte_lpm_wwanled_off_at_cmd_str[] = {
    {"AT#WWANLED=0,1,0,100\r", 0},
};

static const unsigned int lte_lpm_wwanled_off_at_cmd_str_size =
    sizeof(lte_lpm_wwanled_off_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to keep WWAN_LED on when modem is in LPM */
static at_cmd_str lte_lpm_wwanled_on_at_cmd_str[] = {
    {"AT#WWANLED=0,1,100,0\r", 0},
};

static const unsigned int lte_lpm_wwanled_on_at_cmd_str_size =
    sizeof(lte_lpm_wwanled_on_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to restore WWAN_LED blinking pattern to default setting 
 * when modem is in LPM */
static at_cmd_str lte_lpm_wwanled_default_at_cmd_str[] = {
    {"AT#WWANLED=0,0,0,0\r", 0},
};

static const unsigned int lte_lpm_wwanled_default_at_cmd_str_size =
    sizeof(lte_lpm_wwanled_default_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to get the LTE modem current image */
static at_cmd_str lte_get_modem_image_at_cmd_str[] = {
    {"AT#GETFW?\r", 1},
};

static const unsigned int lte_get_modem_image_at_cmd_str_size =
    sizeof(lte_get_modem_image_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to disable modem SIM-based carrier image auto-switching feature */
static at_cmd_str lte_disable_img_switching_at_cmd_str[] = {
    {"AT#ACTIVEFW=0,2\r", 0},
};

static const unsigned int lte_disable_img_switching_at_cmd_str_size =
    sizeof(lte_disable_img_switching_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set LTE modem image based on carrier */
static at_cmd_str lte_set_att_image_at_cmd_str[] = {
    {"AT#ACTIVEFW=2,\"ATT\"\r", 0},
};

static const unsigned int lte_set_att_image_at_cmd_str_size =
    sizeof(lte_set_att_image_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str lte_set_verizon_image_at_cmd_str[] = {
    {"AT#ACTIVEFW=2,\"Verizon\"\r", 0},
};

static const unsigned int lte_set_verizon_image_at_cmd_str_size =
    sizeof(lte_set_verizon_image_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str lte_set_generic_image_at_cmd_str[] = {
    {"AT#ACTIVEFW=2,\"Generic\"\r", 0},
};

static const unsigned int lte_set_generic_image_at_cmd_str_size =
    sizeof(lte_set_generic_image_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str lte_set_sprint_image_at_cmd_str[] = {
    {"AT#ACTIVEFW=2,\"Sprint\"\r", 0},
};

static const unsigned int lte_set_sprint_image_at_cmd_str_size =
    sizeof(lte_set_sprint_image_at_cmd_str) / sizeof(at_cmd_str);

static char telit_lte_expected_img[16] = {0,};
static char telit_lte_cmd_tmp[128] = {0,};
static int telit_lte_para_tmp = -1;

/*******************************************************************************
 * Name:        dev_lte_set_modem_pwrsav_para
 * Description: Function to store the expect carrier
 * Input:       mode - which mode will modem switch to while power 
 *                     saving event is triggered(i.e. W_DISABLE_N pin
 *                     goes to LOW)
 * Returns:     None
 *****************************************************************************/
void dev_lte_set_modem_pwrsav_para (int mode)
{
    sprintf(telit_lte_cmd_tmp, "%s%d\r", LTE_SET_PSAV_CMD_STR, mode);
    lte_set_pwrsav_at_cmd_str[0].str = telit_lte_cmd_tmp;
    telit_lte_para_tmp = mode;
}


/*******************************************************************************
 * Name:        dev_lte_telit_store_expected_img
 * Description: Function to store the expect carrier
 * Input:       carrier_img - pointer to store which carrier image that modem 
 *              expected to
 * Returns:     None
 *****************************************************************************/
void dev_lte_telit_store_expected_img (char *carrier_img)
{
    strncpy(telit_lte_expected_img, carrier_img,
            sizeof(telit_lte_expected_img));
}


/*******************************************************************************
 * Name:        dev_lte_telit_at_open_tty
 * Description: Function opens ttydevice to send AT command
 * Input:       tty_usb_name - TTY USB Device Name.
 *              *tty_fd - Pointer to the TTY file descriptor
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_at_open_tty (char *tty_usb_name, int *tty_fd)
{
    int fd;
    struct termios options;
    int timeout = VTIME_TIMEOUT;
    char tty_dev[64];

    sprintf(tty_dev, "%s/%s", SYS_DEVICE_PATH, tty_usb_name);
    fd = open(tty_dev, O_RDWR | O_NOCTTY);

    if (fd == -1) {
        cterr('f', 0, "Can't open tty device");
        return (FAILED);
    }

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= CRTSCTS;
    options.c_cflag |= (B115200 | CREAD | CLOCAL); 

    options.c_iflag = IGNPAR | ICRNL;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_oflag = 0;
    options.c_oflag &= ~OPOST;

    options.c_cc[VMIN] = 0xFF;
    options.c_cc[VTIME] = timeout;

    tcflush(fd, TCIFLUSH);
    if ((tcsetattr(fd, TCSANOW, &options)) != 0) {
        cterr('f', 0, "Can't set tty attributes");
        return (FAILED);
    }

    *tty_fd = fd;

    return (PASSED);
}


/*******************************************************************************
 * Name:        dev_lte_telit_at_process_cmd
 * Description: Function to send AT command to modem based on test option
 * Input:       fd - file descriptor of tty device
 *              atcmd_str - AT command string
 *              at_test - test option
 *              parse_result - flag to determine whether to parse modem's
 *                             response
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_at_process_cmd (int fd, char *atcmd_str,
                                         int at_test, int parse_result)
{
    char buffer[AT_CMD_BUFFER_SIZE] = {0,};
    char tmp[32];
    char *bufptr;
    int nbytes = 0;
    int atcmd_length = strlen(atcmd_str);
    fd_set tout_set;
    struct timeval timeout;
    int ret;
    int high_pwr, low_pwr, high_frq, low_frq;
    int db, db_amp, hz_frq, ret_val = 0;
    int gps_pwr = 0, gps_frq = 0;
    char *rslt_str, *rslt_str_amp, *rslt_str_frq, *ret_str;
    
    /* Read from tty with timeout */
    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_CMD_RESP_TOUT_IN_SEC;
    timeout.tv_usec = 0;

    if (dev_lte_telit_dbg) {
        printf("\nAT Command is %s\n", atcmd_str);
        fflush(stdout);
    }

    /* Transmit AT command to modem */
    if (write(fd, atcmd_str, atcmd_length) < atcmd_length) {
        printf("%s: Fail to write AT command\n", __func__);
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
    if (dev_lte_telit_dbg) {
        printf("AT_COMMAND \n%s\n", buffer);
        fflush(stdout);
    }

    /* Now, process the string */
    if (parse_result == 0) {
        /* No need to digest the result, 'OK' from modem is enough */
        if (strstr(buffer, "OK") != 0) {
            return (PASSED);
        } else {
            printf("%s: No 'OK' from the modem, (%s)\n", __func__, buffer);
            return (FAILED);
        }
    } else {
        /* Need to parse the result based on what AT command we are sending
         * to modem
         */
        switch (at_test) {
            case LTE_IN_OP_MODE:
                rslt_str = (char *)strchr(buffer, ':');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == OPERATION_MODE) {
                    return (PASSED);
                }
                break;
            case LTE_GPS_ANTENNA_TEST:
                high_pwr = GPS_AMP_DBM + GPS_AMP_TORLENCE;
                low_pwr = GPS_AMP_DBM - GPS_AMP_TORLENCE;
                high_frq = GPS_FRQ_HZ + GPS_FRQ_TORLENCE;
                low_frq = GPS_FRQ_HZ - GPS_FRQ_TORLENCE;
                db_amp = 0;
                hz_frq = 0;
                /* Capture the gps power level and frequency
                 *
                 * Here's the example of AT command output:
                 * #TESTMODE: 57.5dB,-20.1Hz
                 *
                 * OK
                 */

                /* Expected power/frequency info */
                printf("\nExpected Power level: %d dB ~ %d dB",
                       low_pwr, high_pwr);
                printf("\nExpected frequency: %d Hz ~ %d Hz\n",
                       low_frq, high_frq);

                /* power level */
                rslt_str_amp = (char *)strchr(buffer, ':');
                if (rslt_str_amp) {
                    rslt_str_amp ++;
                    db_amp = atoi(rslt_str_amp);
                }

                /* frequency */
                rslt_str_frq = (char *)strchr(buffer, ',');
                if (rslt_str_frq) {
                    rslt_str_frq ++;
                    hz_frq = atoi(rslt_str_frq);
                }          

                /* test(power level) */
                if ((db_amp >= low_pwr) && (db_amp < high_pwr)) {
                    gps_pwr = 1;
                }
                /* test(freq) */
                if ((hz_frq >= low_frq) && (hz_frq <= high_frq)) {
                    gps_frq = 1;
                }

                /* test result */                
                if((gps_pwr && gps_frq) == 1) {
                    printf("\nGPS Test Passed\n"
                           "GPS power level = %d dB, GPS frequency = %d Hz\n",
                           db_amp, hz_frq);
                    return(PASSED);
                } else {
                    printf("\nGPS Test Failed\n"
                           "GPS power level = %d dB, GPS frequency = %d Hz\n",
                            db_amp, hz_frq);   
                    printf("\nWarning: Please verify the settings of the "
                           "signal generator.\n");
                }

                break;
            case LTE_READ_MAIN_RSSI_PWR:
            case LTE_READ_DIV_RSSI_PWR:
                high_pwr = MAIN_DIV_RSSI_AMP_DBM + MAIN_DIV_RSSI_TORLENCE;
                low_pwr = MAIN_DIV_RSSI_AMP_DBM - MAIN_DIV_RSSI_TORLENCE;
                db = 0;
                
                /* Expected power/frequency info */
                printf("\nExpected Power level: -%d dBm ~ -%d dBm\n",
                       low_pwr, high_pwr);

                /* Capture the 4G power level 
                 *
                 * Here's the example of AT command output:
                 * AT#TESTMODE="PRXL4G"
                 * PRXL4G: -60
                 *
                 * OK
                 */
                rslt_str = (char *)strchr(buffer, '-');
                if (rslt_str) {
                    rslt_str++;
                    db = atoi(rslt_str);
                    printf("Result is -%d dBm\n", db);
                }

                if ((db >= high_pwr) || (db < low_pwr)) {
                    printf("Reading RSSI = -%d dBm. Test failed."
                           "\nWarning: Please verify the settings of the"
                           " signal generator.\n", db);
                } else {
                    printf("\n Test Passed dbm = -%d\n", db);
                    return (PASSED);
                }
                break;
            case LTE_DUMP_SIMIN1_STAT:
            case LTE_DUMP_SIMIN2_STAT:
            case LTE_SIMIN1_DETECT_TEST:
            case LTE_SIMIN2_DETECT_TEST:
                /* Capture SININ pin status
                 *
                 * AT#SIMDET? output format:
                 * #SIMDET: <SIM slot>,<SININ status>
                 *
                 * example of AT command output:
                 * AT#SIMDET?
                 * #SIMDET: 0,0
                 */
                rslt_str = (char *)strchr(buffer, ',');
                rslt_str++;
                ret_val = atoi(rslt_str);
                if ((at_test == LTE_DUMP_SIMIN1_STAT) ||
                    (at_test == LTE_DUMP_SIMIN2_STAT)) {
                    printf("SIMIN pin status = %d\n", ret_val);
                    return (PASSED);
                }
                if(ret_val == SIM_PRESENT) {
                    return (PASSED);
                }
                break;
            case LTE_DUMP_TEMP:
                rslt_str = (char *)strchr(buffer, ',');
                rslt_str++;
                ret_val = atoi(rslt_str);
                printf("Modem temperature = %d C\n", ret_val);
                return (PASSED);
                break;
            case LTE_DUMP_MODEM_INFO:
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str++;
                ret_str = rslt_str;
                printf("Modem Info : \nModem FW =%s\n", ret_str);
                return (PASSED);
                break;
            case LTE_IN_LOWPWR_MODE:
            case LTE_FULL_FUNC:
                /* Capture the level of functionality of the modem
                 *
                 * AT+CFUN? output format:
                 * +CFUN: <fun>
                 */
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str += 2;
                ret_val = atoi(rslt_str);
                if ((at_test == LTE_IN_LOWPWR_MODE) &&
                    (ret_val == LOWPWR_MODE)) {
                    return (PASSED);
                } else if ((at_test == LTE_FULL_FUNC) &&
                           (ret_val == OP_MODE)) {
                    return (PASSED);
                }
                break;
            case LTE_GPIO5_IS_HIGH:
            case LTE_GPIO5_IS_LOW:
                /* Capture the modem audio disabling status
                 *
                 * at#gpio=5,2
                 * #GPIO: 0,1 (<dir>, <stat>)
                 */
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str = (char *)strchr(rslt_str, ',');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if ((at_test == LTE_GPIO5_IS_HIGH) &&
                    (ret_val == GPIO_HIGH)) {
                    return (PASSED);
                } else if ((at_test == LTE_GPIO5_IS_LOW) &&
                           (ret_val == GPIO_LOW)) {
                    return (PASSED);
                }
                break;
            case LTE_AUDIO_IS_DISABLE:
                /* Capture the modem audio disabling status
                 *
                 * at#audis?
                 * #AUDIS: 0
                 */
                rslt_str = (char *)strchr(buffer, ':');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == AUDIO_DISABLE) {
                    return (PASSED);
                }
                break;
            case LTE_FASTSHDN_IS_DISABLE:
                /* Capture the modem audio disabling status
                 *
                 * at#tshdn?
                 * #FASTSHDN: 0 (disable)
                 * #FASTSHDN: 1,5 (enable: <en>, <gpio>)
                 */
                rslt_str = (char *)strchr(buffer, ':');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == FASTSHDN_DISABLE) {
                    return (PASSED);
                }
                break;
            case LTE_IMG_IS_MATCHED:
                /* Capture LTE current image
                 *
                 * at#getfw? output format:
                 * #GETFW: <carrier name>, <ensim>
                 */
                rslt_str = (char *)strstr(buffer, telit_lte_expected_img);
                if (rslt_str != NULL) {
                    return (PASSED);
                }
                break;
            case LTE_SOFTSHDN_IND_IS_SET:
                /* Capture the modem shutdown indicator status
                 *
                 * at#shdnind?
                 * #SHDNIND: 0 (disable)
                 * #SHDNIND: 3,3 (enable: <en_mode>, <gpio>)
                 */
                rslt_str = (char *)strchr(buffer, ':');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == ALLSHDN_IND_ENABLE) {
                    return (PASSED);
                }
                break;
            case LTE_DG_IS_DISABLE:
                /* Capture the setting of the dying gasp feature
                 *
                 * AT#GETCUSTFEAT=”DGENABLE” output format:
                 * #GETCUSTFEAT: "DGENABLE",000,1 (disable)
                 */
                rslt_str = (char *)strchr(buffer, ',');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == DYINGGASP_DISABLE) {
                    return (PASSED);
                }
                break;
            case LTE_CHK_PWRSAV_MODE:
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str += 2;
                ret_val = atoi(rslt_str);
                if (ret_val == telit_lte_para_tmp) {
                    return (PASSED);
                }
                break;
        }
    }

    return (FAILED);
}


/*******************************************************************************
 * Name:        dev_lte_telit_at_run_cmd
 * Description: Function to send AT command to modem based on test option
 * Input:       obj_lte_telit - pointer to the Telit LTE device.
 *              at_test - test option
 * Returns:     PASSED/FAILED
 *****************************************************************************/
int dev_lte_telit_at_run_cmd (dev_lte_telit_object_t *obj_lte_telit,
                              int at_test)
{
    char tty_usb_name[64];
    int tty_dev_fd, uport = -1;
    at_cmd_str *at_cmd;
    int ix, at_cmd_length;

    /* Get modem current USB port */
    obj_lte_telit->callout_fvt->get_current_usb_port(&uport);

    /* Get TTY USB device name */
    obj_lte_telit->callout_fvt->get_ttyusb_dev_name(tty_usb_name);

    if (dev_lte_telit_at_open_tty(tty_usb_name, &tty_dev_fd) != PASSED) {
        return (FAILED);
    }


    switch (at_test) {
        case LTE_MODEM_DETECTION:
            at_cmd = lte_ati_cmd_str;
            at_cmd_length = lte_ati_cmd_str_size;
            break;
        case LTE_DUMP_TEMP:
            at_cmd = lte_dump_temp_at_cmd_str;
            at_cmd_length = lte_dump_temp_at_cmd_str_size;
            break;
        case LTE_DUMP_MODEM_INFO:
            at_cmd = lte_dump_modem_info_at_cmd_str;
            at_cmd_length = lte_dump_modem_info_at_cmd_str_size;
            break;
        case LTE_REBOOT:
            at_cmd = lte_reboot_at_cmd_str;
            at_cmd_length = lte_reboot_at_cmd_str_size;
            break;
        case LTE_PWR_DOWN:
            at_cmd = lte_pwr_down_at_cmd_str;
            at_cmd_length = lte_pwr_down_at_cmd_str_size;
            break;
        case LTE_AUDIO_IS_DISABLE:
            at_cmd = lte_chk_audio_dis_at_cmd_str;
            at_cmd_length = lte_chk_audio_dis_at_cmd_str_size;
            break;
        case LTE_FASTSHDN_IS_DISABLE:
            at_cmd = lte_chk_fastshdn_stat_at_cmd_str;
            at_cmd_length = lte_chk_fastshdn_stat_at_cmd_str_size;
            break;
        case LTE_GPIO5_IS_HIGH:
        case LTE_GPIO5_IS_LOW:
            at_cmd = lte_chk_gpio5_stat_at_cmd_str;
            at_cmd_length = lte_chk_gpio5_stat_at_cmd_str_size;
            break;
        case LTE_DISABLE_AUDIO:
            at_cmd = lte_dis_audio_at_cmd_str;
            at_cmd_length = lte_dis_audio_at_cmd_str_size;
            break;
        case LTE_DISABLE_FASTSHDN:
            at_cmd = lte_dis_fastshdn_at_cmd_str;
            at_cmd_length = lte_dis_fastshdn_at_cmd_str_size;
            break;
        case LTE_ENABLE_FASTSHDN:
            at_cmd = lte_en_fastshdn_at_cmd_str;
            at_cmd_length = lte_en_fastshdn_at_cmd_str_size;
            break;
        case LTE_SIM1_DETECT_TEST:
            at_cmd = lte_sim1_detect_at_cmd_str;
            at_cmd_length = lte_sim1_detect_at_cmd_str_size;
            break;
        case LTE_SIM2_DETECT_TEST:
            at_cmd = lte_sim2_detect_at_cmd_str;
            at_cmd_length = lte_sim2_detect_at_cmd_str_size;
            break;
        case LTE_DUMP_SIMIN1_STAT:
        case LTE_SIMIN1_DETECT_TEST:
            at_cmd = lte_query_simin1_status_at_cmd_str;
            at_cmd_length = lte_query_simin1_status_at_cmd_str_size;
            break;
        case LTE_DUMP_SIMIN2_STAT:
        case LTE_SIMIN2_DETECT_TEST:
            at_cmd = lte_query_simin2_status_at_cmd_str;
            at_cmd_length = lte_query_simin2_status_at_cmd_str_size;
            break;
        case LTE_SWITCH_USB2P0:
            at_cmd = lte_switch_modem_usb2p0_at_cmd_str;
            at_cmd_length = lte_switch_modem_usb2p0_at_cmd_str_size;
            break;
        case LTE_SWITCH_USB3P0:
            at_cmd = lte_switch_modem_usb3p0_at_cmd_str;
            at_cmd_length = lte_switch_modem_usb3p0_at_cmd_str_size;
            break;
        case LTE_GPS_ANTENNA_TEST:
            at_cmd = lte_gps_antenna_at_cmd_str;
            at_cmd_length = lte_gps_antenna_at_cmd_str_size;
            break;     
        case LTE_RSSI_CONFIG_B2:
            at_cmd = lte_rssi_4g_b2_config_at_cmd_str;
            at_cmd_length = lte_rssi_4g_b2_config_at_cmd_str_size;
            break;
        case LTE_RSSI_CONFIG_B30:
            at_cmd = lte_rssi_4g_b30_config_at_cmd_str;
            at_cmd_length = lte_rssi_4g_b30_config_at_cmd_str_size;
            break;
        case LTE_READ_MAIN_RSSI_PWR:
            at_cmd = lte_rssi_read_pri_rx_pwr_at_cmd_str;
            at_cmd_length = lte_rssi_read_pri_rx_pwr_at_cmd_str_size;
            break;
        case LTE_READ_DIV_RSSI_PWR:
            at_cmd = lte_rssi_read_sec_rx_pwr_at_cmd_str;
            at_cmd_length = lte_rssi_read_sec_rx_pwr_at_cmd_str_size;
            break;
        case LTE_EXIT_TEST_MODE:
            at_cmd = lte_exit_test_mode_at_cmd_str;
            at_cmd_length = lte_exit_test_mode_at_cmd_str_size;
            break;
        case LTE_ENABLE_OP_MODE:
            at_cmd = lte_enable_op_mode_at_cmd_str;
            at_cmd_length = lte_enable_op_mode_at_cmd_str_size;
            break;
        case LTE_ENABLE_TEST_MODE:
            at_cmd = lte_enable_test_mode_at_cmd_str;
            at_cmd_length = lte_enable_test_mode_at_cmd_str_size;
            break;
        case LTE_IN_OP_MODE:
            at_cmd = lte_query_testmode_stat_at_cmd_str;
            at_cmd_length = lte_query_testmode_stat_at_cmd_str_size;
            break;
        case LTE_IN_LOWPWR_MODE:
        case LTE_FULL_FUNC:
            at_cmd = lte_check_func_level_at_cmd_str;
            at_cmd_length = lte_check_func_level_at_cmd_str_size;
            break;
        case LTE_LPM_WWANLED_OFF:
            at_cmd = lte_lpm_wwanled_off_at_cmd_str;
            at_cmd_length = lte_lpm_wwanled_off_at_cmd_str_size;
            break;
        case LTE_LPM_WWANLED_ON:
            at_cmd = lte_lpm_wwanled_on_at_cmd_str;
            at_cmd_length = lte_lpm_wwanled_on_at_cmd_str_size;
            break;
        case LTE_LPM_WWANLED_DEFAULT:
            at_cmd = lte_lpm_wwanled_default_at_cmd_str;
            at_cmd_length = lte_lpm_wwanled_default_at_cmd_str_size;
            break;
        case LTE_DISABLE_IMG_SWITCHING:
            at_cmd = lte_disable_img_switching_at_cmd_str;
            at_cmd_length = lte_disable_img_switching_at_cmd_str_size;
            break;
        case LTE_SET_ATT_IMG:
            at_cmd = lte_set_att_image_at_cmd_str;
            at_cmd_length = lte_set_att_image_at_cmd_str_size;
            break;
        case LTE_SET_VERIZON_IMG:
            at_cmd = lte_set_verizon_image_at_cmd_str;
            at_cmd_length = lte_set_verizon_image_at_cmd_str_size;
            break;
        case LTE_SET_GENERIC_IMG:
            at_cmd = lte_set_generic_image_at_cmd_str;
            at_cmd_length = lte_set_generic_image_at_cmd_str_size;
            break;
        case LTE_SET_SPRINT_IMG:
            at_cmd = lte_set_sprint_image_at_cmd_str;
            at_cmd_length = lte_set_sprint_image_at_cmd_str_size;
            break;
        case LTE_IMG_IS_MATCHED:
            at_cmd = lte_get_modem_image_at_cmd_str;
            at_cmd_length = lte_get_modem_image_at_cmd_str_size;
            break;
        case LTE_SET_SHDN_IND:
            at_cmd = lte_set_shdn_indicator_at_cmd_str;
            at_cmd_length = lte_set_shdn_indicator_at_cmd_str_size;
            break;
        case LTE_DISABLE_SHDN_IND:
            at_cmd = lte_disable_shdn_indicator_at_cmd_str;
            at_cmd_length = lte_disable_shdn_indicator_at_cmd_str_size;
            break;
        case LTE_SOFTSHDN_IND_IS_SET:
            at_cmd = lte_chk_softshdn_indicator_at_cmd_str;
            at_cmd_length = lte_chk_softshdn_indicator_at_cmd_str_size;
            break;
        case LTE_ENABLE_DYINGGASP:
            at_cmd = lte_enable_dg_at_cmd_str;
            at_cmd_length = lte_enable_dg_at_cmd_str_size;
            break;
        case LTE_DISABLE_DYINGGASP:
            at_cmd = lte_disable_dg_at_cmd_str;
            at_cmd_length = lte_disable_dg_at_cmd_str_size;
            break;
        case LTE_DG_IS_DISABLE:
            at_cmd = lte_chk_dg_stat_at_cmd_str;
            at_cmd_length = lte_chk_dg_stat_at_cmd_str_size;
            break;
        case LTE_SET_PWRSAV:
            at_cmd = lte_set_pwrsav_at_cmd_str;
            at_cmd_length = lte_set_pwrsav_at_cmd_str_size;
            break;
        case LTE_CHK_PWRSAV_MODE:
            at_cmd = lte_chk_pwrsav_at_cmd_str;
            at_cmd_length = lte_chk_pwrsav_at_cmd_str_size;
            break;
        default:
            printf("%s: Not supported AT command ('%d')\n", __func__, at_test);
            return (FAILED);
    }

    /* Process AT commands */
    for (ix = 0; ix < at_cmd_length; ix++) {
        if (dev_lte_telit_at_process_cmd(tty_dev_fd, at_cmd[ix].str, at_test, 
                                         at_cmd[ix].parse_the_result) ==
                                         FAILED) {
            close(tty_dev_fd);
            /* Followed Telit's test script to add 1 second pause time after
             * closed tty device */
            sleep(DELAY_1_SEC);
            return (FAILED);
        }
        msleep(AT_CMD_SEND_DELAY); 
    }   

    close(tty_dev_fd);
    /* Followed Telit's test script to add 1 second pause time after closed
     * tty device */
    sleep(DELAY_1_SEC);

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: dev_lte_telit_at.c,v $
Revision 1.8  2020/08/19 09:48:53  markzha
*** empty log message ***

Revision 1.7  2019/08/14 02:28:06  shjung

1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational
1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational

Revision 1.6  2019/07/01 10:05:25  sherliu2
Supported mdev for Hyperloop

Revision 1.5  2019/06/26 03:52:59  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.4  2019/06/14 05:46:08  shjung
Fixed CSCvq12342, disable modem dying gasp before toggling Modem_Power_ON pin to avoid modem perform fast shutdown and added enabling dying gasp utility

Revision 1.3  2019/05/20 07:28:14  shjung
1. Replace USB serial driver option.ko/usb_wwan.ko with GobiSerial.ko
2. Changes based on last code review comments.
3. Use poll mechanism to query modem functionality level in W_DISABLE pin test
4. Add 1 second delay after close tty device, which is following Telit's test script process.

Revision 1.2  2019/05/14 09:34:12  shjung
Support Hyperloop

Revision 1.1.2.23  2019/05/09 07:50:19  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.22  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.21  2019/04/17 10:09:10  sherliu2
remove mdev related

Revision 1.1.2.20  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.19  2019/04/08 09:54:26  sherliu2
Modified tty device name to symbolic name generated by mdev

Revision 1.1.2.18  2019/04/08 06:41:22  shjung
Check the configuration of modem shutdown indicator while powering down the modem

Revision 1.1.2.17  2019/03/28 10:48:59  shjung
Code clean up

Revision 1.1.2.16  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.15  2019/03/19 09:38:05  shjung
Chaged RSSI test bands based on test carrier

Revision 1.1.2.14  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.13  2019/03/12 02:53:01  shjung

1. Removed OP mode enabling process when RF test failed
2. Added query modem testmode status function
3. Added enable OP mode function
4. Adjusted RF test criteria
5. Code clean up

Revision 1.1.2.12  2019/03/12 02:20:46  shjung
Modified tty device terminal attirbutes to fix tty device hanging issue on CAT18 modems

Revision 1.1.2.11  2019/03/04 08:41:06  shjung
Corrected the user-message for RSSI test criteria

Revision 1.1.2.10  2019/02/23 06:36:27  shjung

1. Added message for RSSI test criteria
2. Followed Telit's suggestion to increase modem reboot pause time

Revision 1.1.2.9  2019/02/15 02:59:49  shjung
Added WWAN_LED control utility

Revision 1.1.2.8  2019/02/12 01:37:51  sherliu2
Modified GPS Antenna Test

Revision 1.1.2.7  2019/02/11 08:06:04  sherliu2
Add GPS Antenna Test

Revision 1.1.2.6  2019/01/18 11:44:12  shjung
CSCvo03379: Temporarily add a workaround for tty device hanging issue on CAT18 modules

Revision 1.1.2.5  2019/01/18 06:17:38  shjung

1. Added W_DISABLE pin test
2. Added modem USB mode switching utility
3. Added delay in modem reboot function based on spec
4. Removed USB mode resotre operation when debug port test failed
5. Code clean up

Revision 1.1.2.4  2019/01/15 10:21:29  shjung
Modified the mechanism to get modem USB device info

Revision 1.1.2.3  2018/12/15 00:28:37  shjung
Disable Rx chain before entering LTE operation mode in RF test

Revision 1.1.2.2  2018/12/14 00:11:11  shjung
Modified RF band configuration

Revision 1.1.2.1  2018/12/12 01:46:12  shjung
Initial check-in for Hyperloop: Added device driver for Telit LTE-LM9x0 modem



$Endlog$
*/

/* 
 * $Id: dev_NR_5G_telit_at.c,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_telit/dev_NR_5G_telit_at.c,v $
 *
 *------------------------------------------------------------------
 *
 * Filename:	dev_NR_5G_telit_at.c
 *
 * Description:	Telit AT Command Driver.
 * Copyright (c) 2020 by cisco Systems, Inc.
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
#include "nvmonvars.h"

#include "dev_NR_5G_telit.h"
#include "dev_NR_5G_telit_at.h"
#include "dev_NR_5G_band_info.h"

static int dev_NR_5G_telit_dbg = 0;
static char swi_NR_5G_setsub6band[100];
static char swi_NR_5G_freq[100];

int ant_test_band_config(nr_sub6_band_struct *, int); 
int dev_NR_5g_telit_at_run_cmd(dev_NR_5g_telit_object_t *, int);
void dev_NR_5g_set_modem_pwrsav_para(int);

static int dev_NR_5G_telit_at_open_tty(char *, int *, int);

//====================AT command list=======================

/* AT command to display modem manufacturer */
static at_cmd_str NR_5g_mfg_name_cmd_str[] = {
    {"AT+GMI\r", 1},
};

static const unsigned int NR_5g_mfg_name_cmd_str_size =
    sizeof(NR_5g_mfg_name_cmd_str) / sizeof(at_cmd_str);


/* AT command to display modem part number */
static at_cmd_str NR_5g_modem_type_cmd_str[] = {
    {"AT+GMM\r", 1},
};

static const unsigned int NR_5g_modem_type_cmd_str_size =
    sizeof(NR_5g_modem_type_cmd_str) / sizeof(at_cmd_str);


/* AT command to display modem serial number */
static at_cmd_str NR_5g_modem_snum_cmd_str[] = {
    {"AT+GSN\r", 1},
};

static const unsigned int NR_5g_modem_snum_cmd_str_size =
    sizeof(NR_5g_modem_snum_cmd_str) / sizeof(at_cmd_str);


/* AT command to display modem host firmware */
static at_cmd_str NR_5g_host_fw_ver_cmd_str[] = {
    {"AT#GETFW\r", 1},
};

static const unsigned int NR_5g_host_fw_ver_cmd_str_size =
    sizeof(NR_5g_host_fw_ver_cmd_str) / sizeof(at_cmd_str);


/* AT command to detect modem */
static at_cmd_str NR_5G_ati_cmd_str[] = {
    {"ATI4\r", 0},
};

static const unsigned int NR_5G_ati_cmd_str_size =
    sizeof(NR_5G_ati_cmd_str) / sizeof(at_cmd_str);


/* AT command to reboot modem */
static at_cmd_str NR_5G_reboot_at_cmd_str[] = {
    {"AT#ENHRST=1,0\r", 0},
};

static const unsigned int NR_5G_reboot_at_cmd_str_size =
    sizeof(NR_5G_reboot_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to dump modem temperature */
static at_cmd_str NR_5G_dump_temp_at_cmd_str[] = {
    {"AT#TEMPSENS=2\r", 1},
};

static const unsigned int NR_5G_dump_temp_at_cmd_str_size =
    sizeof(NR_5G_dump_temp_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to dump modem_information */
static at_cmd_str NR_5G_dump_modem_info_at_cmd_str[] = {
    {"AT#ACTIVEFW?\r", 1},
};

static const unsigned int NR_5G_dump_modem_info_at_cmd_str_size =
    sizeof(NR_5G_dump_modem_info_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to check the level of functionality of the modem */
static at_cmd_str NR_5G_check_func_level_at_cmd_str[] = {
    {"AT+CFUN?\r", 1},
};

static const unsigned int NR_5G_check_func_level_at_cmd_str_size =
    sizeof(NR_5G_check_func_level_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to check shutdown indicator */
static at_cmd_str NR_5G_chk_softshdn_indicator_at_cmd_str[] = {
    {"AT#SHDNIND?\r", 1},
};

static const unsigned int NR_5G_chk_softshdn_indicator_at_cmd_str_size =
    sizeof(NR_5G_chk_softshdn_indicator_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to set shutdown indicator */
static at_cmd_str NR_5G_set_shdn_indicator_at_cmd_str[] = {
    {"AT#SHDNIND=3,8\r", 0},
    {"AT#ENHRST=1,0\r", 0},
};

static const unsigned int NR_5G_set_shdn_indicator_at_cmd_str_size =
    sizeof(NR_5G_set_shdn_indicator_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to disable shutdown indicator */
static at_cmd_str NR_5G__disable_shdn_indicator_at_cmd_str[] = {
    {"AT#SHDNIND=0\r", 0},
    {"AT#ENHRST=1,0\r", 0},
};

static const unsigned int NR_5G__disable_shdn_indicator_at_cmd_str_size =
    sizeof(NR_5G__disable_shdn_indicator_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to check fast shutdown status */
static at_cmd_str NR_5G_chk_fastshdn_stat_at_cmd_str[] = {
    {"AT#FASTSHDN?\r", 1},
};

static const unsigned int NR_5G_chk_fastshdn_stat_at_cmd_str_size =
    sizeof(NR_5G_chk_fastshdn_stat_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to disable fast shutdown */
static at_cmd_str NR_5G_dis_fastshdn_at_cmd_str[] = {
    {"AT#FASTSHDN=0\r", 0},
};

static const unsigned int NR_5G_dis_fastshdn_at_cmd_str_size =
    sizeof(NR_5G_dis_fastshdn_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to power down modem */
static at_cmd_str NR_5G_pwr_down_at_cmd_str[] = {
    {"AT#SHDN\r", 0},
};

static const unsigned int NR_5G_pwr_down_at_cmd_str_size =
    sizeof(NR_5G_pwr_down_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to check the modem power saving mode */
static at_cmd_str NR_5G_chk_pwrsav_at_cmd_str[] = {
    {"AT#PSMWDISACFG?\r", 1},
};

static const unsigned int NR_5G_chk_pwrsav_at_cmd_str_size =
    sizeof(NR_5G_chk_pwrsav_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to set modem power saving mode */
static at_cmd_str NR_5G_set_pwrsav_at_cmd_str[] = {
    {"AT#PSMWDISACFG=", 0},
};

static const unsigned int NR_5G_set_pwrsav_at_cmd_str_size =
    sizeof(NR_5G_set_pwrsav_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to switch modem to USB2.0 mode */
static at_cmd_str NR_5G_switch_modem_usb2p0_at_cmd_str[] = {
    {"AT#USBSWITCH=1\r", 0},
};

static const unsigned int NR_5G_switch_modem_usb2p0_at_cmd_str_size =
    sizeof(NR_5G_switch_modem_usb2p0_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to switch modem to USB3.0 mode */
static at_cmd_str NR_5G_switch_modem_usb3p0_at_cmd_str[] = {
    {"AT#USBSWITCH=0\r", 0},
};

static const unsigned int NR_5G_switch_modem_usb3p0_at_cmd_str_size =
    sizeof(NR_5G_switch_modem_usb3p0_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to detect SIM card in SIM interface 1 */
static at_cmd_str NR_5G_sim1_detect_at_cmd_str[] = {
    {"AT+CMEE=2\r", 0},
    {"AT+CPIN?\r", 0},
};

static const unsigned int NR_5G_sim1_detect_at_cmd_str_size =
    sizeof(NR_5G_sim1_detect_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to read SIMIN1 pin status */
static at_cmd_str NR_5G_query_simin1_status_at_cmd_str[] = {
    {"AT+CFUN=0\r",0},
    {"AT+CFUN=1\r",0},
    {"AT#SIMDET=0\r", 0},
    {"AT#SIMDET?\r", 1},
};

static const unsigned int NR_5G_query_simin1_status_at_cmd_str_size =
    sizeof(NR_5G_query_simin1_status_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set gps antenna test*/
/* According to the vendor suggestion, get value after send AT#TESTMODE="GNSS"
 * 5~10 times */
static at_cmd_str NR_5G_gps_antenna_at_cmd_str[] = {
    {"AT$GPSANTPORT=2\r", 0},
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},  
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,0},
    {"AT#TESTMODE=\"GNSS\"\r" ,1},
    {"AT$GPSANTPORT=3\r", 0},
};

static const unsigned int NR_5G_gps_antenna_at_cmd_str_size =
    sizeof(NR_5G_gps_antenna_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str NR_5G_gpsantport_active_at_cmd_str[] = {
    {"AT$GPSANTPORT=3\r", 0},
};
static const unsigned int NR_5G_gpsantport_active_at_cmd_str_size =
    sizeof(NR_5G_gpsantport_active_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to set Rx RSSI test configuration with 5G band N66 carrier */
static at_cmd_str sub6_rssi_config_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"SETSUB6BAND 41\"\r", 0},
    {"AT#TESTMODE=\"FREQ 2593000\"\r", 0},
};

static const unsigned int sub6_rssi_config_at_cmd_str_size =
    sizeof(sub6_rssi_config_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to return primary RF Rx RSSI test power level */
static at_cmd_str sub6_rssi_read_pri_rx_pwr_at_cmd_str[] = {
    {"AT#TESTMODE=\"PRXRLSUB6\"\r", 1},
};

static const unsigned int sub6_rssi_read_pri_rx_pwr_at_cmd_str_size =
    sizeof(sub6_rssi_read_pri_rx_pwr_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to return secondary RF Rx RSSI test power level */
static at_cmd_str sub6_rssi_read_sec_rx_pwr_at_cmd_str[] = {
    {"AT#TESTMODE=\"DRXRLSUB6\"\r", 1},
};

static const unsigned int sub6_rssi_read_sec_rx_pwr_at_cmd_str_size =
    sizeof(sub6_rssi_read_sec_rx_pwr_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to return mimo1 RF Rx RSSI test power level */
static at_cmd_str sub6_rssi_read_mimo1_rx_pwr_at_cmd_str[] = {
    {"AT#TESTMODE=\"MIMOSETSUB6\"\r", 0},
    {"AT#TESTMODE=\"MIMO1RLSUB6\"\r", 1},
};

static const unsigned int sub6_rssi_read_mimo1_rx_pwr_at_cmd_str_size =
    sizeof(sub6_rssi_read_mimo1_rx_pwr_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to return mimo2 RF Rx RSSI test power level */
static at_cmd_str sub6_rssi_read_mimo2_rx_pwr_at_cmd_str[] = {
    {"AT#TESTMODE=\"MIMOSETSUB6\"\r", 0},
    {"AT#TESTMODE=\"MIMO2RLSUB6\"\r", 1},
};

static const unsigned int sub6_rssi_read_mimo2_rx_pwr_at_cmd_str_size =
    sizeof(sub6_rssi_read_mimo2_rx_pwr_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to exit test mode */
static at_cmd_str NR_5G_exit_test_mode_at_cmd_str[] = {
    {"AT#TESTMODE=\"ESC\"\r", 0},
};

static const unsigned int NR_5G_exit_test_mode_at_cmd_str_size = 
    sizeof(NR_5G_exit_test_mode_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to force the module in Operation Mode */
static at_cmd_str NR_5G_enable_op_mode_at_cmd_str[] = {
    {"AT#TESTMODE=\"OM\"\r", 0},
};

static const unsigned int NR_5G_enable_op_mode_at_cmd_str_size =
    sizeof(NR_5G_enable_op_mode_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to force the module in Test Mode */
static at_cmd_str NR_5G_enable_test_mode_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
};

static const unsigned int NR_5G_enable_test_mode_at_cmd_str_size =
    sizeof(NR_5G_enable_test_mode_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to query the modem mode */
static at_cmd_str NR_5G_query_testmode_stat_at_cmd_str[] = {
    {"AT#TESTMODE?\r", 1},
};

static const unsigned int NR_5G_query_testmode_stat_at_cmd_str_size =
    sizeof(NR_5G_query_testmode_stat_at_cmd_str) / sizeof(at_cmd_str);


/* AT command to set Tx RSSI test configuration with 5G band N1 carrier */
static at_cmd_str sub6_tx_N1_config_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"SETSUB6BAND 1\"\r", 0},
    {"AT#TESTMODE=\"FREQ 2140000\"\r", 0},
    {"AT#TESTMODE=\"TCH\"\r",0},
    {"AT#TESTMODE=\"STXGAIN 67\"\r",0},
};

static const unsigned int sub6_tx_N1_config_at_cmd_str_size =
    sizeof(sub6_tx_N1_config_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set Tx RSSI test configuration with 5G band N25 carrier */
static at_cmd_str sub6_tx_N25_config_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"SETSUB6BAND 25\"\r", 0},
    {"AT#TESTMODE=\"FREQ 1962500\"\r", 0},
    {"AT#TESTMODE=\"TCH\"\r",0},
    {"AT#TESTMODE=\"STXGAIN 67\"\r",0},
};

static const unsigned int sub6_tx_N25_config_at_cmd_str_size =
    sizeof(sub6_tx_N25_config_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to set Tx RSSI test configuration with 5G band N79 carrier */
static at_cmd_str sub6_tx_N79_config_at_cmd_str[] = {
    {"AT#TESTMODE=\"TM\"\r", 0},
    {"AT#TESTMODE=\"SETSUB6BAND 79\"\r", 0},
    {"AT#TESTMODE=\"FREQ 4700000\"\r", 0},
    {"AT#TESTMODE=\"TCH\"\r",0},
    {"AT#TESTMODE=\"STXGAIN 67\"\r",0},
};

static const unsigned int sub6_tx_N79_config_at_cmd_str_size =
    sizeof(sub6_tx_N79_config_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to keep WWAN_LED off when modem is in LPM */
static at_cmd_str NR_5G_lpm_wwanled_off_at_cmd_str[] = {
    {"AT#WWANLED=0,1,0,100\r", 0},
};

static const unsigned int NR_5G_lpm_wwanled_off_at_cmd_str_size =
    sizeof(NR_5G_lpm_wwanled_off_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to keep WWAN_LED on when modem is in LPM */
static at_cmd_str NR_5G_lpm_wwanled_on_at_cmd_str[] = {
    {"AT#WWANLED=0,1,100,0\r", 0},
};

static const unsigned int NR_5G_lpm_wwanled_on_at_cmd_str_size =
    sizeof(NR_5G_lpm_wwanled_on_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to restore WWAN_LED blinking pattern to default setting 
 * when modem is in LPM */
static at_cmd_str NR_5G_lpm_wwanled_default_at_cmd_str[] = {
    {"AT#WWANLED=0,0,0,0\r", 0},
};

static const unsigned int NR_5G_lpm_wwanled_default_at_cmd_str_size =
    sizeof(NR_5G_lpm_wwanled_default_at_cmd_str) / sizeof(at_cmd_str);

/* AT command to get the modem current image */
static at_cmd_str NR_5G_get_modem_image_at_cmd_str[] = {
    {"AT#GETFW?\r", 1},
};

static const unsigned int NR_5G_get_modem_image_at_cmd_str_size =
    sizeof(NR_5G_get_modem_image_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str sim_settings_diag_req[] = {
    {"at#simincfg=1,1\r", 0},
    {"at#hsen=0,0\r", 0},
};

static const unsigned int sim_settings_diag_req_at_cmd_str_size =
    sizeof(sim_settings_diag_req) / sizeof(at_cmd_str);

static at_cmd_str sim_hotswap_status[] = {
    {"AT#HSEN?\r", 1},
};
static const unsigned int sim_hotswap_status_str_size =
    sizeof(sim_hotswap_status) / sizeof(at_cmd_str);

static char telit_cmd_tmp[128] = {0,};
static int telit_para_tmp = -1;

/*******************************************************************************
 * Name:        dev_NR_5g_set_modem_pwrsav_para
 * Description: Function to store the expect carrier
 * Input:       mode - which mode will modem switch to while power 
 *                     saving event is triggered(i.e. W_DISABLE_N pin
 *                     goes to LOW)
 * Returns:     None
 *****************************************************************************/
void dev_NR_5g_set_modem_pwrsav_para (int mode)
{
    sprintf(telit_cmd_tmp, "%s%d\r", NR_5G_SUB6_SET_PSAV_CMD_STR, mode);
    NR_5G_set_pwrsav_at_cmd_str[0].str = telit_cmd_tmp;
    telit_para_tmp = mode;
}


/*******************************************************************************
 * Name:        dev_NR_5G_telit_at_open_tty
 * Description: Function opens ttydevice to send AT command
 * Input:       tty_usb_name - TTY USB Device Name.
 *              *tty_fd - Pointer to the TTY file descriptor
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5G_telit_at_open_tty (char *tty_usb_name, int *tty_fd, int timeout)
{
    int fd;
    struct termios options;
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
 * Name:        dev_NR_5G_telit_at_process_cmd
 * Description: Function to send AT command to modem based on test option
 * Input:       fd - file descriptor of tty device
 *              atcmd_str - AT command string
 *              at_test - test option
 *              parse_result - flag to determine whether to parse modem's
 *                             response
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5G_telit_at_process_cmd (dev_NR_5g_telit_object_t *obj_5g_nr_sub6_telit,
                                         int fd, char *atcmd_str,
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
    int len;
    int high_pwr, low_pwr, high_frq, low_frq;
    int db, db_amp, hz_frq, ret_val = 0;
    int gps_pwr = 0, gps_frq = 0;
    char *rslt_str,*rslt_str1, *rslt_str_amp, *rslt_str_frq, *ret_str;
    
    /* Read from tty with timeout */
    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_CMD_RESP_TOUT_IN_SEC;
    timeout.tv_usec = 0;

    if (dev_NR_5G_telit_dbg) {
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
    if (dev_NR_5G_telit_dbg) {
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
            case NR_5G_IN_OP_MODE:
                rslt_str = (char *)strchr(buffer, ':');
                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == OPERATION_MODE) {
                    return (PASSED);
                }
                break;
            case NR_5G_GPS_ANTENNA_TEST:
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
            case NR_5G_READ_MAIN_RSSI_PWR:
            case NR_5G_READ_DIV_RSSI_PWR:
            case NR_5G_READ_MIMO1_RSSI_PWR:
            case NR_5G_READ_MIMO2_RSSI_PWR:
                high_pwr = MAIN_DIV_RSSI_AMP_DBM + MAIN_DIV_RSSI_TORLENCE;
                low_pwr = MAIN_DIV_RSSI_AMP_DBM - MAIN_DIV_RSSI_TORLENCE;
                db = 0;
                
                /* Expected power/frequency info */
                printf("\nExpected Power level: -%d dBm ~ -%d dBm\n",
                       low_pwr, high_pwr);

                /* Capture the 5G power level 
                 *
                 * Here's the example of AT command output:
                 * AT#TESTMODE="PRXSUB6"
                 * PRXRLSUB6: -60
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
            case NR_5G_DUMP_SIMIN1_STAT:
            case NR_5G_DUMP_SIMIN2_STAT:
            case NR_5G_SIMIN1_DETECT_TEST:
            case NR_5G_SIMIN2_DETECT_TEST:
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
                if ((at_test == NR_5G_DUMP_SIMIN1_STAT) ||
                    (at_test == NR_5G_DUMP_SIMIN2_STAT)) {
                    printf("SIMIN pin status = %d\n", ret_val);
                    return (PASSED);
                }
                if(ret_val == SIM_PRESENT) {
                    return (PASSED);
                }
                break;
            case NR_5G_DUMP_TEMP:
                rslt_str = (char *)strchr(buffer, ',');
                rslt_str++;
                ret_val = atoi(rslt_str);
                printf("Modem temperature = %d C\n", ret_val);
                return (PASSED);
                break;
            case NR_5G_DUMP_MODEM_INFO:
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str++;
                ret_str = rslt_str;
                printf("Modem Info : \nModem FW =%s\n", ret_str);
                return (PASSED);
                break;
            case NR_5G_IN_LOWPWR_MODE:
            case NR_5G_FULL_FUNC:
                /* Capture the level of functionality of the modem
                 *
                 * AT+CFUN? output format:
                 * +CFUN: <fun>
                 */
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str += 2;
                ret_val = atoi(rslt_str);
                if ((at_test == NR_5G_IN_LOWPWR_MODE) &&
                    (ret_val != OP_MODE)) {
                    return (PASSED);
                } else if ((at_test == NR_5G_FULL_FUNC) &&
                           (ret_val == OP_MODE)) {
                    return (PASSED);
                }
                break;
            case NR_5G_FASTSHDN_IS_DISABLE:
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
            case NR_5G_SOFTSHDN_IND_IS_SET:
                /* Capture the modem shutdown indicator status
                 *
                 * at#shdnind?
                 * #SHDNIND: 0 (disable)
                 * #SHDNIND: 3,8 (enable: <en_mode>, <gpio>)
                 */
                rslt_str = (char *)strchr(buffer, ':');
                if (rslt_str == NULL) {
                    printf ("\n\nFailed to get shdnind info!!!!\n\n");
                    return (FAILED);
                }

                sscanf(rslt_str, "%[^0-9]%d\n", tmp, &ret_val);
                if (ret_val == ALLSHDN_IND_ENABLE) {
                    return (PASSED);
                }
                break;
            case NR_5G_DG_IS_DISABLE:
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
            case NR_5G_CHK_PWRSAV_MODE:
                rslt_str = (char *)strchr(buffer, ':');
                rslt_str += 2;
                ret_val = atoi(rslt_str);
                if (ret_val == telit_para_tmp) {
                    return (PASSED);
                }
                break;
            case NR_5G_MODEM_MFG_NAME:
            case NR_5G_MODEM_TYPE:
            case NR_5G_MODEM_SL_NUM:
                rslt_str = strstr(buffer, "\n");
                for (len = 0; len <strlen(rslt_str) ; len++) {
                    if (*(rslt_str+len) != '\n') break;
                }
                rslt_str1 = strstr(rslt_str+len, "\n");
                if (rslt_str1) *rslt_str1 = '\0';
                if (rslt_str) {
                    if (strstr (buffer, "AT+GMI")) {
                        printf ("\nModem Manufacturer : %s", (rslt_str+len));
                    } else if (strstr (buffer, "AT+GMM")) {
                        printf ("\nModem Model number : %s", (rslt_str+len));
                    } else if (strstr (buffer, "AT+GSN")) {
                        printf ("\nModem Serial number: %s", (rslt_str+len));
                    } else if (strstr (buffer, "AT+GMR")) {
                        printf ("\nModem Firmware     : %s", (rslt_str+len));
                    }
                }
                return (PASSED);
            break;
            case NR_5G_MODEM_HOST_FW:
                len = 2;
                rslt_str = strstr(buffer, ": "); //HOST FIRMWARE  : <<< get the version
                rslt_str1 = strstr(rslt_str, "\n");
                if (rslt_str1) *rslt_str1 = '\0';
                printf ("\nModem Firmware     : %s", (rslt_str+len));
                return (PASSED);
            break;
           case NR_5G_SIM_HOTSWAP_STATUS:
               //sim slot 0 hot swap disable
               rslt_str = strstr(buffer, "HSEN: 0,0");
               if (rslt_str) {
                   return (PASSED);
               }
           break;
        }
    }

    return (FAILED);
}


/*******************************************************************************
 * Name:        dev_NR_5g_telit_at_run_cmd
 * Description: Function to send AT command to modem based on test option
 * Input:       obj_5g_nr_sub6_telit - pointer to the Telit device.
 *              at_test - test option
 * Returns:     PASSED/FAILED
 *****************************************************************************/
int dev_NR_5g_telit_at_run_cmd (dev_NR_5g_telit_object_t *obj_5g_nr_sub6_telit,
                              int at_test)
{
    char tty_usb_name[64];
    int tty_dev_fd, uport = -1;
    at_cmd_str *at_cmd;
    int ix, at_cmd_length;
    int timeout = 0;

    switch (at_test) {
        case NR_5G_DUMP_SIMIN1_STAT:
        case NR_5G_SIMIN1_DETECT_TEST:
            timeout = VTIME_TIMEOUT_2;
            break;
        default:
            timeout = VTIME_TIMEOUT;
            break;
    }
           
    /* Get modem current USB port */
    obj_5g_nr_sub6_telit->callout_fvt->get_current_usb_port(&uport);

    /* Get TTY USB device name */
    obj_5g_nr_sub6_telit->callout_fvt->get_ttyusb_dev_name(tty_usb_name);

    if (dev_NR_5G_telit_at_open_tty(tty_usb_name, &tty_dev_fd, timeout) != PASSED) {
        return (FAILED);
    }


    switch (at_test) {
        case NR_5G_MODEM_MFG_NAME:
            at_cmd = NR_5g_mfg_name_cmd_str;
            at_cmd_length = NR_5g_mfg_name_cmd_str_size;
            break;
        case NR_5G_MODEM_TYPE:
            at_cmd = NR_5g_modem_type_cmd_str;
            at_cmd_length = NR_5g_modem_type_cmd_str_size;
            break;
        case NR_5G_MODEM_SL_NUM:
            at_cmd = NR_5g_modem_snum_cmd_str;
            at_cmd_length = NR_5g_modem_snum_cmd_str_size;
            break;
        case NR_5G_MODEM_HOST_FW:
            at_cmd = NR_5g_host_fw_ver_cmd_str;
            at_cmd_length = NR_5g_host_fw_ver_cmd_str_size;
            break;
        case NR_5G_MODEM_DETECTION:
            at_cmd = NR_5G_ati_cmd_str;
            at_cmd_length = NR_5G_ati_cmd_str_size;
            break;
        case NR_5G_DUMP_TEMP:
            at_cmd = NR_5G_dump_temp_at_cmd_str;
            at_cmd_length = NR_5G_dump_temp_at_cmd_str_size;
            break;
        case NR_5G_DUMP_MODEM_INFO:
            at_cmd = NR_5G_dump_modem_info_at_cmd_str;
            at_cmd_length = NR_5G_dump_modem_info_at_cmd_str_size;
            break;
        case NR_5G_REBOOT:
            at_cmd = NR_5G_reboot_at_cmd_str;
            at_cmd_length = NR_5G_reboot_at_cmd_str_size;
            break;
        case NR_5G_PWR_DOWN:
            at_cmd = NR_5G_pwr_down_at_cmd_str;
            at_cmd_length = NR_5G_pwr_down_at_cmd_str_size;
            break;
        case NR_5G_FASTSHDN_IS_DISABLE:
            at_cmd = NR_5G_chk_fastshdn_stat_at_cmd_str;
            at_cmd_length = NR_5G_chk_fastshdn_stat_at_cmd_str_size;
            break;
        case NR_5G_DISABLE_FASTSHDN:
            at_cmd = NR_5G_dis_fastshdn_at_cmd_str;
            at_cmd_length = NR_5G_dis_fastshdn_at_cmd_str_size;
            break;
        case NR_5G_SIM1_DETECT_TEST:
            at_cmd = NR_5G_sim1_detect_at_cmd_str;
            at_cmd_length = NR_5G_sim1_detect_at_cmd_str_size;
            break;
        case NR_5G_DUMP_SIMIN1_STAT:
        case NR_5G_SIMIN1_DETECT_TEST:
            at_cmd = NR_5G_query_simin1_status_at_cmd_str;
            at_cmd_length = NR_5G_query_simin1_status_at_cmd_str_size;
            break;
        case NR_5G_SWITCH_USB2P0:
            at_cmd = NR_5G_switch_modem_usb2p0_at_cmd_str;
            at_cmd_length = NR_5G_switch_modem_usb2p0_at_cmd_str_size;
            break;
        case NR_5G_SWITCH_USB3P0:
            at_cmd = NR_5G_switch_modem_usb3p0_at_cmd_str;
            at_cmd_length = NR_5G_switch_modem_usb3p0_at_cmd_str_size;
            break;
        case NR_5G_GPS_ANTENNA_TEST:
            at_cmd = NR_5G_gps_antenna_at_cmd_str;
            at_cmd_length = NR_5G_gps_antenna_at_cmd_str_size;
            break;     
        case NR_5G_RSSI_TEST_CONFIG:
            at_cmd = sub6_rssi_config_at_cmd_str;
            at_cmd_length = sub6_rssi_config_at_cmd_str_size;
            break;
        case NR_5G_TX_CONFIG_N1:
            at_cmd = sub6_tx_N1_config_at_cmd_str;
            at_cmd_length = sub6_tx_N1_config_at_cmd_str_size;
            break;
        case NR_5G_TX_CONFIG_N25:
            at_cmd = sub6_tx_N25_config_at_cmd_str;
            at_cmd_length = sub6_tx_N25_config_at_cmd_str_size;
            break;
        case NR_5G_TX_CONFIG_N79:
            at_cmd = sub6_tx_N79_config_at_cmd_str;
            at_cmd_length = sub6_tx_N79_config_at_cmd_str_size;
            break;
        case NR_5G_READ_MAIN_RSSI_PWR:
            at_cmd = sub6_rssi_read_pri_rx_pwr_at_cmd_str;
            at_cmd_length = sub6_rssi_read_pri_rx_pwr_at_cmd_str_size;
            break;
        case NR_5G_READ_DIV_RSSI_PWR:
            at_cmd = sub6_rssi_read_sec_rx_pwr_at_cmd_str;
            at_cmd_length = sub6_rssi_read_sec_rx_pwr_at_cmd_str_size;
            break;
        case NR_5G_READ_MIMO1_RSSI_PWR:
            at_cmd = sub6_rssi_read_mimo1_rx_pwr_at_cmd_str;
            at_cmd_length = sub6_rssi_read_mimo1_rx_pwr_at_cmd_str_size;
            break;
        case NR_5G_READ_MIMO2_RSSI_PWR:
            at_cmd = sub6_rssi_read_mimo2_rx_pwr_at_cmd_str;
            at_cmd_length = sub6_rssi_read_mimo2_rx_pwr_at_cmd_str_size;
            break;
        case NR_5G_EXIT_TEST_MODE:
            at_cmd = NR_5G_exit_test_mode_at_cmd_str;
            at_cmd_length = NR_5G_exit_test_mode_at_cmd_str_size;
            break;
        case NR_5G_ENABLE_OP_MODE:
            at_cmd = NR_5G_enable_op_mode_at_cmd_str;
            at_cmd_length = NR_5G_enable_op_mode_at_cmd_str_size;
            break;
        case NR_5G_ENABLE_TEST_MODE:
            at_cmd = NR_5G_enable_test_mode_at_cmd_str;
            at_cmd_length = NR_5G_enable_test_mode_at_cmd_str_size;
            break;
        case NR_5G_IN_OP_MODE:
            at_cmd = NR_5G_query_testmode_stat_at_cmd_str;
            at_cmd_length = NR_5G_query_testmode_stat_at_cmd_str_size;
            break;
        case NR_5G_IN_LOWPWR_MODE:
        case NR_5G_FULL_FUNC:
            at_cmd = NR_5G_check_func_level_at_cmd_str;
            at_cmd_length = NR_5G_check_func_level_at_cmd_str_size;
            break;
        case NR_5G_LPM_WWANLED_OFF:
            at_cmd = NR_5G_lpm_wwanled_off_at_cmd_str;
            at_cmd_length = NR_5G_lpm_wwanled_off_at_cmd_str_size;
            break;
        case NR_5G_LPM_WWANLED_ON:
            at_cmd = NR_5G_lpm_wwanled_on_at_cmd_str;
            at_cmd_length = NR_5G_lpm_wwanled_on_at_cmd_str_size;
            break;
        case NR_5G_LPM_WWANLED_DEFAULT:
            at_cmd = NR_5G_lpm_wwanled_default_at_cmd_str;
            at_cmd_length = NR_5G_lpm_wwanled_default_at_cmd_str_size;
            break;
        case NR_5G_SET_SHDN_IND:
            at_cmd = NR_5G_set_shdn_indicator_at_cmd_str;
            at_cmd_length = NR_5G_set_shdn_indicator_at_cmd_str_size;
            break;
        case NR_5G_DISABLE_SHDN_IND:
            at_cmd = NR_5G__disable_shdn_indicator_at_cmd_str;
            at_cmd_length = NR_5G__disable_shdn_indicator_at_cmd_str_size;
            break;
        case NR_5G_SOFTSHDN_IND_IS_SET:
            at_cmd = NR_5G_chk_softshdn_indicator_at_cmd_str;
            at_cmd_length = NR_5G_chk_softshdn_indicator_at_cmd_str_size;
            break;
        case NR_5G_SET_PWRSAV:
            at_cmd = NR_5G_set_pwrsav_at_cmd_str;
            at_cmd_length = NR_5G_set_pwrsav_at_cmd_str_size;
            break;
        case NR_5G_CHK_PWRSAV_MODE:
            at_cmd = NR_5G_chk_pwrsav_at_cmd_str;
            at_cmd_length = NR_5G_chk_pwrsav_at_cmd_str_size;
            break;
        case NR_5G_SIM_MODE_CHANGE_FOR_DIAG:
            at_cmd = sim_settings_diag_req;
            at_cmd_length = sim_settings_diag_req_at_cmd_str_size;
            break;
        case NR_5G_SIM_HOTSWAP_STATUS:
            at_cmd = sim_hotswap_status;
            at_cmd_length = sim_hotswap_status_str_size;
            break;
        case NR_5G_SET_GPSANTPORT_ACTIVE:
            at_cmd = NR_5G_gpsantport_active_at_cmd_str;
            at_cmd_length = NR_5G_gpsantport_active_at_cmd_str_size;
            break;
        default:
            printf("%s: Not supported AT command ('%d')\n", __func__, at_test);
            return (FAILED);
    }

    /* Process AT commands */
    for (ix = 0; ix < at_cmd_length; ix++) {
        if (dev_NR_5G_telit_at_process_cmd(obj_5g_nr_sub6_telit,tty_dev_fd, 
                                         at_cmd[ix].str, at_test, 
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
/***************************************************************************
* Name: ant_test_band_config
*
* Description: This function configures the AT cmd to respective band
*              for performing RSSI test
* Input: band_tbl - list of bands supported to perform RSSI
*
* Output: PASSED/FAILED
***************************************************************************/
int ant_test_band_config(nr_sub6_band_struct *band_tbl, int band_num) 
{
    int i;
    char *ptr;
    at_cmd_str  *at_cmd_tbl;
    unsigned int at_cmd_tbl_size;

    at_cmd_tbl = sub6_rssi_config_at_cmd_str;
    at_cmd_tbl_size = sub6_rssi_config_at_cmd_str_size;

    for (i = 0; i < at_cmd_tbl_size; i++) {
        if ((ptr = (char *) strstr(at_cmd_tbl[i].str, "SETSUB6BAND"))) {
            break;
        }
    }

    if (i >= at_cmd_tbl_size){
        printf ("\nCould not find the SETSUB6BAND in the table");
        return FAILED;
    }

    sprintf (swi_NR_5G_setsub6band, \
                "AT#TESTMODE=\"SETSUB6BAND %d\"\r",band_tbl->band_num);
    at_cmd_tbl[i].str = swi_NR_5G_setsub6band;

    for (i=0 ; i < at_cmd_tbl_size; i++) {
        if ((ptr = (char *) strstr(at_cmd_tbl[i].str, "FREQ"))) {
            break;
        }
    }
    if (i > at_cmd_tbl_size){
        printf ("\nCould not find the FREQ in the table");
        return FAILED;
    }
    sprintf (swi_NR_5G_freq, \
                "AT#TESTMODE=\"FREQ %ld\"\r",band_tbl->band_freq);
    at_cmd_tbl[i].str = swi_NR_5G_freq;
    

    if ((NVRAM)->diagflag & D_VERBOSE) {
        for (i = 0; i < at_cmd_tbl_size; i++) {
            printf ("\n%s", sub6_rssi_config_at_cmd_str[i].str);
        }
    }
    return PASSED;
}

/*********************************************************************
 * $Log: dev_NR_5G_telit_at.c,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.6  2021/04/15 21:23:48  tshanmug
 * sears check for null pointer while checking shutdown indicator status
 *
 * Revision 1.1.2.5  2021/02/27 00:43:07  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.4  2021/02/12 01:08:18  tshanmug
 * Sears multi band test support
 *
 * Revision 1.1.2.3  2020/12/01 06:38:46  tshanmug
 * Sears antenna test modification to test all antenna in a single menu
 *
 * $Endlog$
 */

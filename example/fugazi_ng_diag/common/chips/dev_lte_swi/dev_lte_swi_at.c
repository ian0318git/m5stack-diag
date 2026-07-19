/* $Id: dev_lte_swi_at.c,v 1.6 2020/02/19 03:11:30 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_swi/dev_lte_swi_at.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_lte_swi_at.c
 *
 * Description:	LTE SWI AT Command Driver.
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "defs.h"
#include "common.h"
#include "dev_lte_swi.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#include "nvmonvars.h"
#ifdef LINUX_APP
#include <assert.h>
#endif

#include "dev_lte_swi.h"
#include "dev_lte_swi_at.h"

static int dev_lte_swi_at_open_tty(char *, int *);
static int dev_lte_swi_at_process_cmd(int, char *, int, int);
static int dev_lte_at_selftest(int);

int dev_lte_swi_at_run_cmd(dev_lte_swi_object_t *, int);

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
static at_cmd_str lte_main_rssi_wp_b4_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASBAND=42\r", 0 },
    { "AT!DALSTXBW=2\r", 0 },
    { "AT!DALSRXBW=2\r", 0 },
    { "AT!DASCHAN=20175\r", 0 },
    { "AT!DALGAVGAGC=20175,0\r", 1 },
};

static const unsigned int lte_main_rssi_wp_b4_at_cmd_str_size =
    sizeof(lte_main_rssi_wp_b4_at_cmd_str) / sizeof(at_cmd_str);

/* The AT command sequences for LTE RSSI value reading of WP7607/08/09 modem
 * Test setting:Band 1(Band ID = 34), Channel = 18300, Frequency = 2142 MHz
 */
static at_cmd_str lte_main_rssi_wp_b1_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASBAND=34\r", 0 },
    { "AT!DALSTXBW=2\r", 0 },
    { "AT!DALSRXBW=2\r", 0 },
    { "AT!DASCHAN=18300\r", 0 },
    { "AT!DALGAVGAGC=18300,0\r", 1 },
};

static const unsigned int lte_main_rssi_wp_b1_at_cmd_str_size =
    sizeof(lte_main_rssi_wp_b1_at_cmd_str) / sizeof(at_cmd_str);

/* GPS Antenna Test */
static at_cmd_str gps_rssi_at_cmd_str[] = {
    { "AT\r", 0 },
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DACGPSTESTMODE=1\r", 0 },
    { "AT!DACGPSSTANDALONE=1\r", 0 },
    { "AT!DACGPSMASKON\r", 0 },
    { "AT!DACGPSCTON\r", 1 },
};

static const unsigned int gps_rssi_at_cmd_str_size =
    sizeof(gps_rssi_at_cmd_str) / sizeof(at_cmd_str);

/* WP76XX GPS Pin */
static at_cmd_str lte_main_gps_pin_high_wp76xx_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },  
    { "AT!BSGPIO=53,1\r", 0 },
};
static const unsigned int gps_pin_high_wp76xx_at_cmd_str_size =
    sizeof(lte_main_gps_pin_high_wp76xx_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str lte_main_gps_pin_low_wp76xx_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },  
    { "AT!BSGPIO=53,0\r", 0 },
};

static const unsigned int gps_pin_low_wp76xx_at_cmd_str_size =
    sizeof(lte_main_gps_pin_low_wp76xx_at_cmd_str) / sizeof(at_cmd_str);

/* Modem Reset */
static at_cmd_str rssi_reset_at_cmd_str[] = {
    { "AT!RESET\r", 0 },
};

static const unsigned int rssi_reset_at_cmd_str_size =
    sizeof(rssi_reset_at_cmd_str) / sizeof(at_cmd_str);

/* SIM-0 Detect Test */
static at_cmd_str lte_sim0_detect_str[] = {
    { "at!uims=0\r", 0 }, /* switch to slot 0*/
    { "AT+CPIN?\r", 0 },
};

static const unsigned int lte_sim0_detect_str_size =
    sizeof(lte_sim0_detect_str) / sizeof(at_cmd_str);

/* SIM-1 Detect Test */
static at_cmd_str lte_sim1_detect_str[] = {
    { "at!uims=1\r", 0 }, /* switch to slot 1*/
    { "AT+CPIN?\r", 0 },
};

static const unsigned int lte_sim1_detect_str_size =
    sizeof(lte_sim1_detect_str) /sizeof(at_cmd_str);

/* Power down LTE modem */
static at_cmd_str lte_pwr_down_at_cmd_str[] = {
    { "AT!POWERDOWN\r", 0 },
};

static const unsigned int lte_pwr_down_at_cmd_str_size =
    sizeof(lte_pwr_down_at_cmd_str) / sizeof(at_cmd_str);

/* SIM detect pin status  */
/* Based on comment from SWI(Sierra wireless):
 * For WP76xx, AT!BSGPIO?34 can be used to check the state of UIM1_DET signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str lte_sim_detect_pin_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO?34\r", 1},
};

static const unsigned int lte_sim_detect_pin_at_cmd_str_size =
    sizeof(lte_sim_detect_pin_at_cmd_str) / sizeof(at_cmd_str);

/* LED turn on */
static at_cmd_str lte_led_turn_on_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!LEDTEST=0,1\r", 0 },
};

static const unsigned int lte_led_turn_on_at_cmd_str_size =
    sizeof(lte_led_turn_on_at_cmd_str) / sizeof(at_cmd_str);

/* LED turn off */
static at_cmd_str lte_led_turn_off_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!LEDTEST=0,0\r", 0 },
};

static const unsigned int lte_led_turn_off_at_cmd_str_size =
    sizeof(lte_led_turn_off_at_cmd_str) / sizeof(at_cmd_str);

/* Check modem carrier match*/
static at_cmd_str lte_chk_img_carrier_match_at_cmd_str[] = {
    { "AT!IMPREF?\r", 1},
};

static const unsigned int lte_chk_img_carrier_match_at_cmd_str_size =
    sizeof(lte_chk_img_carrier_match_at_cmd_str) / sizeof(at_cmd_str);

/***************************************************************************
* Name: dev_lte_fd_selftest
*
* Description: This function sends "AT" command to ensure the communication 
*              between host and modem is good 
* 
* Input: *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int dev_lte_at_selftest (int fd)
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

    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_SELFTEST_TOUT_IN_SEC;
    timeout.tv_usec = 0;

    /* Flush buffer */
    select(fd + 1, &tout_set, NULL, NULL, &timeout);
    if  (tcflush(fd, TCIOFLUSH) < 0) {
        printf("Flush buffer failed : %s\n", strerror(errno));
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Name:	dev_lte_swi_at_run_cmd
 *
 * Description:	Function to send AT command to modem based on test option
 *
 * Input:   obj_lte_swi - pointer to the LTE SWI device.
 *          at_test - Test Option
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
int dev_lte_swi_at_run_cmd (dev_lte_swi_object_t *obj_lte_swi, int at_test) 
{
    char tty_usb_name[64];
    int tty_dev_fd;
    at_cmd_str *at_cmd;
    int at_cmd_length;
    int ix;

    /* Get TTY USB Device Name/Path */
    obj_lte_swi->callout_fvt->get_ttyusb_dev_name(tty_usb_name);

    if (dev_lte_swi_at_open_tty(tty_usb_name, &tty_dev_fd) == FAILED) {
        return (FAILED);
    }

    /* To fix CSCvh79986 and CSCvh79979, ensure communication between host
     * and modem is good */
    if (dev_lte_at_selftest(tty_dev_fd) == FAILED) {
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
        at_cmd = lte_main_rssi_wp_b4_at_cmd_str;
        at_cmd_length = lte_main_rssi_wp_b4_at_cmd_str_size;
        break;
    case RSSI_LTE_B1_MAIN_TEST:
    case RSSI_LTE_B1_DIV_TEST:
        at_cmd = lte_main_rssi_wp_b1_at_cmd_str;
        at_cmd_length = lte_main_rssi_wp_b1_at_cmd_str_size;
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
    case GPS_PIN_HIGH_WP76XX:
        at_cmd = lte_main_gps_pin_high_wp76xx_at_cmd_str;
        at_cmd_length = gps_pin_high_wp76xx_at_cmd_str_size;
        break;
    case GPS_PIN_LOW_WP76XX:
        at_cmd = lte_main_gps_pin_low_wp76xx_at_cmd_str;
        at_cmd_length = gps_pin_low_wp76xx_at_cmd_str_size;
        break;
    case LTE_PWR_DOWN:
        at_cmd = lte_pwr_down_at_cmd_str;
        at_cmd_length = lte_pwr_down_at_cmd_str_size;
        break;
    case SIM_DETECT_PIN_PRESENT:
        at_cmd = lte_sim_detect_pin_at_cmd_str;
        at_cmd_length = lte_sim_detect_pin_at_cmd_str_size;
        break;
    case SIM_DETECT_PIN_NO_PRESENT:
        at_cmd = lte_sim_detect_pin_at_cmd_str;
        at_cmd_length = lte_sim_detect_pin_at_cmd_str_size;
        break;
    case SIM_DETECT_PIN_STATUS:
        at_cmd = lte_sim_detect_pin_at_cmd_str;
        at_cmd_length = lte_sim_detect_pin_at_cmd_str_size;
        break;
    case WWAN_LED_TURN_ON:
        at_cmd = lte_led_turn_on_at_cmd_str;
        at_cmd_length = lte_led_turn_on_at_cmd_str_size;
        break;
    case WWAN_LED_TURN_OFF:
        at_cmd = lte_led_turn_off_at_cmd_str;
        at_cmd_length = lte_led_turn_off_at_cmd_str_size;
        break;
    case LTE_CHK_IMG_CARRIER_MATCH:
        at_cmd = lte_chk_img_carrier_match_at_cmd_str;
        at_cmd_length =lte_chk_img_carrier_match_at_cmd_str_size;
        break;
    default:
        printf("%s: Not supported AT command ('%d')\n", __func__, at_test);
        return (FAILED);
    }   

    /* Process AT commands */
    for (ix = 0; ix < at_cmd_length; ix++) {
        if (dev_lte_swi_at_process_cmd(tty_dev_fd, at_cmd[ix].str, at_test, 
                                       at_cmd[ix].parse_the_result) == FAILED) {
            close(tty_dev_fd);
            return (FAILED);
        }   
    }   

    close(tty_dev_fd);

    return (PASSED);
}



/***************************************************************************
* Name: dev_lte_swi_at_process_cmd 
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
static int dev_lte_swi_at_process_cmd (int fd, char *atcmd_str,
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
    if (write(fd, atcmd_str, atcmd_length) < atcmd_length) {
        cterr('f', 0, "Fail to write AT command");
        return (FAILED);
    }

    bufptr = buffer;
    ret = select(fd + 1, &tout_set, NULL, NULL, &timeout);

    if (ret < 1) {
        cterr('f', 0, "Modem is not responding to AT command");
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
            cterr('f', 0, "No 'OK' from the modem, (%s)", buffer);
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
                    cterr('f', 0, "Reading RSSI = %d dBm Test failed"
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
                    printf("\n Could not collect GPS RF data for CTON \n");
                    return (FAILED);
                }
                rslt_str++;
                rslt_str2 = strstr(rslt_str, "=");
                if (rslt_str2 == NULL) {
                    printf("\n Could not collect GPS RF data for Freq \n");
                    return (FAILED);
                }
                rslt_str2++;
                rslt_str1 = NULL;
                rslt_str3 = strstr(rslt_str2, "\n");
                if (rslt_str3 == NULL) {
                    printf("\n Could not collect GPS RF data for Freq (no newline) \n");
                    return (FAILED);
                }
                rslt_str3 = NULL;
                cton = atoi(rslt_str);
                freq = atoi(rslt_str2);

                if ((cton <= GPS_CTON_MAX) && (cton >= GPS_CTON_MIN)) {
                    if ((freq <= GPS_TEST_FREQ_MAX) && (freq >= GPS_TEST_FREQ_MIN)) {
                        printf("\n GPS RF Passed CtoN = %ddBm, Freq = %dHz\n",
                               cton, freq);
                        return (PASSED);
                    }
                } else {
                    printf("\n ***GPS RF FAILED CtoN = %ddBm, Freq = %dHz\n"
                           " ***If Stop on error ... download kernel again and "
                           "Set GPSAUTOSTART\n", cton, freq);
                    cterr('f', 0, " GPS RF Failed CtoN = %ddB, Freq = %dHz\n"
                          " CtoN should be within 58 +/- 5dBm and Freq within "
                          "100000 Hz +/- 5000 Hz.\n", cton, freq);
                    return (FAILED);
                }
                break;
            case SIM_DETECT_PIN_PRESENT:
                printf("SIM Detect PIN PRESENT\n");
                if (strstr(buffer, "State:     1") != 0) {
                    printf("PASSED\n");
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case SIM_DETECT_PIN_NO_PRESENT:
                printf("SIM Detect PIN NO PRESENT\n");
                if (strstr(buffer, "State:     0") != 0) {
                    printf("PASSED\n");
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
            case LTE_CHK_IMG_CARRIER_MATCH:
                if (strstr(buffer, "carrier name mismatch") != 0) {
                    /* Carrier "Mismatch" */
                    printf("Modem Carrier 'Mismatch'\n");
                } else {
                    /* Carrier "Match"*/
                    if (strstr(buffer, "OK") != 0) {
                        printf("Modem Carrier 'Match'\n");
                        return (PASSED);
                    } else {
                        printf("An exception message print!\n");
                        return (FAILED);
                    }
                }
                break;
            case SIM_DETECT_PIN_STATUS:
                printf("\n%s", buffer);
        }
    }

    return (FAILED);
}


/***************************************************************************
* Name: dev_lte_swi_at_open_tty
*
* Description: This function opens tty device for AT command 
* 
* Input: tty_usb_name - TTY USB Device Name
*        *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int dev_lte_swi_at_open_tty (char *tty_usb_name, int *tty_fd)
{
    int fd;
    struct termios options;
    int timeout = VTIME_TIMEOUT;

    fd = open(tty_usb_name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        cterr('f', 0, "Can't open tty device");
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

/*------------------------------------------------------------------
$Log: dev_lte_swi_at.c,v $
Revision 1.6  2020/02/19 03:11:30  harrchan
Add LTE patch for matching modem carrier (CSCvt07550)

Revision 1.5  2019/06/14 09:59:00  steja
Supported Cooper usb dongle LTE

Revision 1.4  2018/11/09 07:33:22  yungchen
Merge viper branch4 to the main trunk (CSCvn11857)

Revision 1.3  2018/08/31 03:59:29  chieyang
Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2

Revision 1.2  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/07/09 08:28:30  olin2
CSCvk17781: Support util to verify SIM Detect pin

Revision 1.1.2.3  2018/05/29 03:03:57  harrchan
Support powerdown AT command and add selftest

Revision 1.1.2.2  2018/02/27 09:10:33  harrchan
Initial viper application code base


$Endlog$
*/

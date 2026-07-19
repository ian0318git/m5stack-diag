/*
 * $Id: dev_NR_5G_swi_at.c,v 1.3 2021/06/30 20:04:55 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_swi/dev_NR_5G_swi_at.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_NR_5G_swi_at.c
 *
 * Description:	SWI 5G sub6 AT Command Driver.
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
#define TTY_DEV_PATH                             "/dev/"

#include "dev_NR_5G_swi.h"
#include "dev_NR_5G_swi_at.h"

static int dev_swi_5g_at_open_tty(char *, int *);
static int dev_swi_5g_at_process_cmd(dev_5g_swi_object_t *,int, char *, int, int);
static int dev_swi_5g_at_selftest(int);
static int check_mmwave_ant_present_status (char *);

int dev_swi_5g_at_run_cmd(dev_5g_swi_object_t *, int);

static char swi_NR_5G_DARCONFIG[AT_CMD_STR_SIZE];
static char swi_NR_5G_DARCONFIG_1[AT_CMD_STR_SIZE];
static char swi_NR_5G_DAUPDATEPARAM[AT_CMD_STR_SIZE];
static char swi_NR_5G_DAGFTMRXAGC[AT_CMD_STR_SIZE];
static char swi_NR_5G_DAGFTMRXAGC_0[AT_CMD_STR_SIZE];
static char swi_NR_5G_DAGFTMRXAGC_1[AT_CMD_STR_SIZE];
static char swi_NR_5G_DAGFTMRXAGC_2[AT_CMD_STR_SIZE];
static char swi_NR_5G_DAGFTMRXAGC_3[AT_CMD_STR_SIZE];
static char swi_NR_5G_DATXCONTROL_start[AT_CMD_STR_SIZE];
static char swi_NR_5G_DATXCONTROL_stop[AT_CMD_STR_SIZE];

int nr_mmwave_beamid[8] = {
    QTM0V_IFV4,
    QTM0H_IFH1,
    QTM1V_IFV3,
    QTM1H_IFH2,
    QTM2V_IFV2,
    QTM2H_IFH3,
    QTM3V_IFV1,
    QTM3H_IFH4
};

int mmwave_ant_instance_map[][INST_PER_ANT]={
    {1,2, 9,10},    //QTM0
    {3,4,11,12},    //QTM1
    {5,6,13,14},    //QTM2
    {7,8,15,16},    //QTM3
};

//Expected mmwave antenna status for the AT!RFDEVSTATUS?
char mmwave_antenna_status[][RFDEVSTAT_STR_SIZE]={
    "0  ,0xff  ,0x526 ,TRUE",    
    "1  ,0x217 ,0xe2  ,TRUE",    //QTM0
    "2  ,0x217 ,0xe2  ,TRUE",    //QTM0
    "3  ,0x217 ,0xe2  ,TRUE",    //QTM1
    "4  ,0x217 ,0xe2  ,TRUE",    //QTM1
    "5  ,0x217 ,0xe2  ,TRUE",    //QTM2
    "6  ,0x217 ,0xe2  ,TRUE",    //QTM2
    "7  ,0x217 ,0xe2  ,TRUE",    //QTM3
    "8  ,0x217 ,0xe2  ,TRUE",    //QTM3
    "9  ,0x217 ,0x3a  ,TRUE",    //QTM0
    "10 ,0x217 ,0x3a  ,TRUE",    //QTM0
    "11 ,0x217 ,0x3a  ,TRUE",    //QTM1
    "12 ,0x217 ,0x3a  ,TRUE",    //QTM1
    "13 ,0x217 ,0x3a  ,TRUE",    //QTM2
    "14 ,0x217 ,0x3a  ,TRUE",    //QTM2
    "15 ,0x217 ,0x3a  ,TRUE",    //QTM3
    "16 ,0x217 ,0x3a  ,TRUE",    //QTM3
};

static at_cmd_str swi_nr_5g_ati_cmd_str[] = {
    { "ATI\r", 0 },
};
static const unsigned int swi_nr_5g_ati_cmd_str_size =
    sizeof(swi_nr_5g_ati_cmd_str) / sizeof(at_cmd_str);   

/* Display  modem current temperature */
static at_cmd_str swi_nr_5g_temp_at_cmd_str[] = {
    { "AT!PCTEMP?\r", 1 },
};
static const unsigned int swi_nr_5g_temp_at_cmd_str_size =
    sizeof(swi_nr_5g_temp_at_cmd_str) / sizeof(at_cmd_str);   

static at_cmd_str NR5G_drop_radio_cfg_at_cmd_str[] = {
    { "AT!DARCONFIGDROP=6\r", 0 },
};
static const unsigned int NR5G_drop_radio_cfg_at_cmd_str_size =
    sizeof(NR5G_drop_radio_cfg_at_cmd_str) / sizeof(at_cmd_str);


/* put the modem online from factory test mode */
static at_cmd_str swi_nr_5g_disable_tm_mode_at_cmd_str[] = {
    { "AT!DAFTMDEACT\r", 0 },
};
static const unsigned int swi_nr_5g_disable_tm_mode_at_cmd_str_size =
    sizeof(swi_nr_5g_disable_tm_mode_at_cmd_str) / sizeof(at_cmd_str);   

/*FR1 Sub6 RX RSSI test AT command template*/
static at_cmd_str NR5G_FR1_rx_rssi_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASUB6TECHACT=1\r", 0 },
    { "AT!DAUPDATEPARAM=18,1\r", 0 },
    { "AT!DARCONFIG=0,6,1,390000,5,428000\r", 0 },
    { "AT!DAGFTMRXAGC=0,6,-800,0\r", 1 },
    { "AT!DARCONFIGDROP=6\r", 0 },
    { "AT!DASUB6TECHACT=0\r", 0 },
    { "AT!DAFTMDEACT\r", 0 },
};
static const unsigned int NR5G_FR1_rx_rssi_at_cmd_str_size =
    sizeof(NR5G_FR1_rx_rssi_at_cmd_str) / sizeof(at_cmd_str);

/*FR1 Sub6 OTA RX RSSI test AT command template*/
static at_cmd_str NR5G_FR1_ota_rx_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASUB6TECHACT=1\r", 0 },
    { "AT!DAUPDATEPARAM=18,79\r", 0 },
    { "AT!DARCONFIG=0,6,79,713333,9,713333\r", 0 },
    { "AT!DAGFTMRXAGC=0,6,-600,0\r", 1 },
    { "AT!DAGFTMRXAGC=0,6,-600,3\r", 1 },
    { "AT!DARCONFIG=0,6,79,713333,9,713333,1\r", 0 },
    { "AT!DAGFTMRXAGC=0,6,-600,1\r", 1 },
    { "AT!DAGFTMRXAGC=0,6,-600,2\r", 1 },
    { "AT!DARCONFIGDROP=6\r", 0 },
    { "AT!DASUB6TECHACT=0\r", 0 },
    { "AT!DAFTMDEACT\r", 0 },
};
static const unsigned int NR5G_FR1_ota_rx_at_cmd_str_size =
    sizeof(NR5G_FR1_ota_rx_at_cmd_str) / sizeof(at_cmd_str);


/*Legacy Tx path test for main*/
static at_cmd_str NR5G_FR1_tx_rssi_N79_legacy_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DAUPDATEPARAM=18,79\r", 0 },
    { "AT!DARCONFIG=0,6,79,713333,50000,713333\r", 0 },
    { "AT!DATXCONTROL=0,6,1,230,10,0,1,0,10\r", 0 },
};
static const unsigned int NR5G_FR1_tx_rssi_N79_legacy_at_cmd_str_size =
    sizeof(NR5G_FR1_tx_rssi_N79_legacy_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str NR5G_FR1_main_tx_stop_legacy_at_cmd_str[] = {
    { "AT!DATXCONTROL=0,6,0,230,10,0,1,0,10\r", 0 },
    { "AT!DARCONFIGDROP=6\r", 0 },
    { "AT!DAFTMDEACT\r", 0 },
};
static const unsigned int NR5G_FR1_main_tx_stop_legacy_at_cmd_str_size =
    sizeof(NR5G_FR1_main_tx_stop_legacy_at_cmd_str) / sizeof(at_cmd_str);

/*Tx path test for main*/

static at_cmd_str NR5G_FR1_tx_rssi_N79_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DASUB6TECHACT=1\r", 0 },
    { "AT!DAUPDATEPARAM=18,79\r", 0 },
    { "AT!DARCONFIG=0,6,79,713333,9,713333\r", 0 },
    { "AT!DATXCONTROL=0,6,1,230,10,0,1,0,10\r", 0 },
};
static const unsigned int NR5G_FR1_tx_rssi_N79_at_cmd_str_size =
    sizeof(NR5G_FR1_tx_rssi_N79_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str NR5G_FR1_main_tx_stop_at_cmd_str[] = {
    { "AT!DATXCONTROL=0,6,0,230,10,0,1,0,10\r", 0 },
    { "AT!DARCONFIGDROP=6\r", 0 },
    { "AT!DASUB6TECHACT=0\r", 0 },
    { "AT!DAFTMDEACT\r", 0 },
};
static const unsigned int NR5G_FR1_main_tx_stop_at_cmd_str_size =
    sizeof(NR5G_FR1_main_tx_stop_at_cmd_str) / sizeof(at_cmd_str);

/* GPS Antenna Test */
//GNSS test for L1
static at_cmd_str gps_l1_rssi_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DACGPSTESTMODE=1\r", 0 },
    { "AT!DACGPSCTON=1\r", 0 },
    { "AT!DACGPSCTON=1\r", 0 },
    { "AT!DACGPSCTON=1\r", 0 },
    { "AT!DACGPSCTON=1\r", 0 },
    { "AT!DACGPSCTON=1\r", 1 },
    { "AT!DAFTMDEACT\r", 0 },

};
static const unsigned int gps_l1_rssi_at_cmd_str_size =
    sizeof(gps_l1_rssi_at_cmd_str) / sizeof(at_cmd_str);

//GNSS test for L5
static at_cmd_str gps_l5_rssi_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!DAFTMACT\r", 0 },
    { "AT!DACGPSTESTMODE=1\r", 0 },
    { "AT!DACGPSCTON=5\r", 0 },
    { "AT!DACGPSCTON=5\r", 0 },
    { "AT!DACGPSCTON=5\r", 0 },
    { "AT!DACGPSCTON=5\r", 0 },
    { "AT!DACGPSCTON=5\r", 1 },
    { "AT!DAFTMDEACT\r", 0 },

};
static const unsigned int gps_l5_rssi_at_cmd_str_size =
    sizeof(gps_l5_rssi_at_cmd_str) / sizeof(at_cmd_str);

/*
 * MMWAVE RSSI & Tx tests
 */

/*FR2 MMWAVE RX RSSI test AT command template*/
static at_cmd_str NR5G_FR2_rx_rssi_at_cmd_str[] = {
    /* To unlock the extended AT command set */
    { "AT!ENTERCND=\"A710\"\r", 0 },

    /* To enter factory test mode */
    { "AT!DAFTMACT\r", 0 },

    /* To turn on PON for mmW IF testing */
    { "AT!DAMMWACT\r", 0 },

    /* To set radio config */
    { "AT!DARCONFIG=0,6,261,2077949,100000,2077949,0,0\r", 0   },
    /* To get the power dbm and check the receiving power
       if it is close to instrument setting value  */
    { "AT!DAGFTMRXAGC=0,6,-500,4,0\r", 1     },
    /* To drop all configurations */
    { "AT!DARCONFIGDROP=6\r", 0 },

    /* To turn off PON */
    { "AT!DAMMWDEACT\r", 0 },
};
static const unsigned int NR5G_FR2_rx_rssi_at_cmd_str_size =
    sizeof(NR5G_FR2_rx_rssi_at_cmd_str) / sizeof(at_cmd_str);

/* FR2 MMWAVE TX path test for QTM Antennas */
static at_cmd_str NR5G_FR2_tx_at_cmd_str[] = {
    /* To unlock the extended AT command set */
    { "AT!ENTERCND=\"A710\"\r", 0 },

    /* To enter factory test mode */
    { "AT!DAFTMACT\r", 0 },

    /* To turn on PON for mmW IF testing */
    { "AT!DAMMWACT\r", 0 },

    /* To set radio config */
    { "AT!DARCONFIG=0,6,261,2077949,100000,2077949,0,0\r", 0   },
    /* To enable transmitter */
    { "AT!DATXCONTROL=0,6,1,-500,10,0,1,0,66,0\r", 0   },
    /*** Take the measurement on CMP200 ***/

    /* To disable transmitter */
    // Use NR5G_FR2_tx_stop_at_cmd_str

    /* To drop all configurations */
    // Use NR5G_drop_radio_cfg_at_cmd_str

    /* To turn off PON */
    // Use NR_5G_mmwv_pon_disable_at_cmd_str
};
static const unsigned int NR5G_FR2_tx_at_cmd_str_size =
    sizeof(NR5G_FR2_tx_at_cmd_str) / sizeof(at_cmd_str);


static at_cmd_str NR5G_FR2_tx_stop_at_cmd_str[] = {
        /* To disable transmitter */
        { "AT!DATXCONTROL=0,6,0,-500,10,0,1,0,66,0\r", 0   },
};

static const unsigned int NR5G_FR2_tx_stop_at_cmd_str_size =
    sizeof(NR5G_FR2_tx_stop_at_cmd_str) / sizeof(at_cmd_str);



/* Modem Reset */
static at_cmd_str swi_nr_5g_reset_at_cmd_str[] = {
    { "AT!RESET\r", 0 },
};
static const unsigned int swi_nr_5g_reset_at_cmd_str_size =
    sizeof(swi_nr_5g_reset_at_cmd_str) / sizeof(at_cmd_str);

/* SIM-0 Detect Test */
static at_cmd_str swi_nr_5g_sim0_detect_str[] = {
    { "at!uims=0\r", 0 }, 
    { "at+cfun=0\r", 0 }, 
    { "at+cfun=1\r", 0 }, 
    { "AT+CPIN?\r", 0 },
};
static const unsigned int swi_nr_5g_sim0_detect_str_size =
    sizeof(swi_nr_5g_sim0_detect_str) / sizeof(at_cmd_str);

/* SIM detect pin status  */
/* Based on comment from SWI(Sierra wireless):
 * AT!BSGPIO?68 can be used to check the state of UIM1_DET signal.
 * And AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
 */
static at_cmd_str lte_sim_detect_pin_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!BSGPIO?68\r", 1},
};
static const unsigned int lte_sim_detect_pin_at_cmd_str_size =
    sizeof(lte_sim_detect_pin_at_cmd_str) / sizeof(at_cmd_str);

//AT!CUSTOM output
static at_cmd_str swi_nr_5g_custom_chk_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!CUSTOM?\r", 1},
};
static const unsigned int swi_nr_5g_custom_chk_cmd_str_size =
    sizeof(swi_nr_5g_custom_chk_cmd_str) / sizeof(at_cmd_str);

//Set custom configuration require for diag
static at_cmd_str swi_nr_5g_custom_cfg_for_diag[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!CUSTOM=\"GPIOSARENABLE\",1\r", 0}, //Required for DPR test
    { "AT!CUSTOM=\"SIMLPM\",2\r", 0}, //SIM ON/OFF when CFUN=0/1
};
static const unsigned int swi_nr_5g_custom_cfg_for_diag_size =
    sizeof(swi_nr_5g_custom_cfg_for_diag) / sizeof(at_cmd_str);

//disable custom cmd - set default config
static at_cmd_str swi_nr_5g_custom_cfg_default[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!CUSTOM=\"GPIOSARENABLE\",0\r", 0},
    { "AT!CUSTOM=\"SIMLPM\",1\r", 0},
};
static const unsigned int swi_nr_5g_custom_cfg_default_size =
    sizeof(swi_nr_5g_custom_cfg_default) / sizeof(at_cmd_str);

//check sar state - DPR
static at_cmd_str swi_nr_5g_chk_sar_status_cmd_str[] = {
    { "AT!SARSTATE?\r", 1},
};
static const unsigned int swi_nr_5g_chk_sar_status_cmd_str_size =
    sizeof(swi_nr_5g_chk_sar_status_cmd_str) / sizeof(at_cmd_str);


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

/*bootup message */
static at_cmd_str swi_nr_5g_modem_info_at_cmd_str[] = {
    { "AT+CGMI\r", 1 },
    { "AT+CGMM\r", 1 },
    { "AT+CGSN\r", 1 },
    { "AT+CGMR\r", 1 },
    { "AT!PCINFO?\r", 1 },
    { "AT!PCTEMP?\r", 1 },
};
static const unsigned int swi_nr_5g_modem_info_at_cmd_str_size =
    sizeof(swi_nr_5g_modem_info_at_cmd_str) / sizeof(at_cmd_str);

/*For EDVT test to excercise the interface between the modem and host*/
static at_cmd_str NR_5G_trial_at_cmd_str[] = {

//Read and display
    { "AT+CIMI\r", 1 },
    { "AT+CGSN\r", 1 },
    { "AT+CGMR\r", 1 },
    { "AT!GSTATUS?\r", 1 },
    { "AT!PCTEMP?\r", 1 },
//Read and cmp
    { "AT+CGMI\r", 1 },
    { "AT+CGMM\r", 1 },
//    { "AT+CSQ\r", 1 },
    { "AT!GPSSATINFO?\r", 1 },


//Read write test
    { "AT+CMEE?\r",  1 },
    { "AT+CMEE=x\r", 0 },
    { "AT+CMEE?\r",  1 },
    { "AT+CMEE=x\r", 0 },

};
static const unsigned int NR_5G_trial_at_cmd_str_size =
    sizeof(NR_5G_trial_at_cmd_str) / sizeof(at_cmd_str);


/* NR 5G MMWAVE PON Enable AT commands  */
static at_cmd_str NR_5G_mmwv_pon_enable_at_cmd_str[] = {
    { "AT!DAMMWACT\r", 0 },
};
static const unsigned int NR_5G_mmwv_pon_enable_at_cmd_str_size =
    sizeof(NR_5G_mmwv_pon_enable_at_cmd_str) / sizeof(at_cmd_str);

/* NR 5G MMWAVE PON Disable AT commands  */
static at_cmd_str NR_5G_mmwv_pon_disable_at_cmd_str[] = {
    { " AT!DAMMWDEACT\r", 0 },
};
static const unsigned int NR_5G_mmwv_pon_disable_at_cmd_str_size =
    sizeof(NR_5G_mmwv_pon_disable_at_cmd_str) / sizeof(at_cmd_str);


static at_cmd_str swi_nr_5g_fsn_cmd_str[] = {
    { "ATI\r", 1 },
};
static const unsigned int swi_nr_5g_fsn_cmd_str_size =
    sizeof(swi_nr_5g_fsn_cmd_str) / sizeof(at_cmd_str);


static at_cmd_str swi_nr_5g_modem_hwid_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!HWID?\r", 1 },
};
static const unsigned int swi_nr_5g_modem_hwid_at_cmd_str_size =
    sizeof(swi_nr_5g_modem_hwid_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str swi_nr_5g_modem_sku_at_cmd_str[] = {
    { "AT!ENTERCND=\"A710\"\r", 0 },
    { "AT!SKU?\r", 1 },
};
static const unsigned int swi_nr_5g_modem_sku_at_cmd_str_size =
    sizeof(swi_nr_5g_modem_sku_at_cmd_str) / sizeof(at_cmd_str);

static at_cmd_str swi_nr_5g_mmwave_ant_status_cmd_str[] = {
    { "AT!RFDEVSTATUS?\r", 1},
};
static const unsigned int swi_nr_5g_mmwave_ant_status_cmd_str_size =
    sizeof(swi_nr_5g_mmwave_ant_status_cmd_str) / sizeof(at_cmd_str);

/***************************************************************************
* Name: dev_swi_5g_at_selftest
*
* Description: This function sends "AT" command to ensure the communication 
*              between host and modem is good 
* 
* Input: *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int dev_swi_5g_at_selftest (int fd)
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
 * Name:	dev_swi_5g_at_run_cmd
 *
 * Description:	Function to send AT command to modem based on test option
 *
 * Input:   dev_5g_swi_object_t - pointer to the SWI device.
 *          at_test - Test Option
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
int dev_swi_5g_at_run_cmd (dev_5g_swi_object_t *obj_swi_modem, int at_test) 
{
    char tty_dev_name[64];
    int tty_dev_fd;
    at_cmd_str *at_cmd;
    int at_cmd_length;
    int ix;

    /* Get TTY USB Device Name/Path */
    obj_swi_modem->callout_fvt->get_tty_dev_name(tty_dev_name);

    if (dev_swi_5g_at_open_tty(tty_dev_name, &tty_dev_fd) == FAILED) {
        printf ("\n%s[%d]failed to open tty : %s", __FUNCTION__, __LINE__, tty_dev_name);
        return (FAILED);
    }

    /* To fix CSCvh79986 and CSCvh79979, ensure communication between host
     * and modem is good */
    if (dev_swi_5g_at_selftest(tty_dev_fd) == FAILED) {
        return (FAILED);
    }

    switch (at_test) {
    case RSSI_FR1_RX_TEST:
    case RSSI_FR1_OTA_IND_ANT_RX_TEST:
        at_cmd = NR5G_FR1_rx_rssi_at_cmd_str;
        at_cmd_length = NR5G_FR1_rx_rssi_at_cmd_str_size;
        break;
    case RSSI_FR1_OTA_ALL_ANT_RX_TEST:
        at_cmd = NR5G_FR1_ota_rx_at_cmd_str;
        at_cmd_length = NR5G_FR1_ota_rx_at_cmd_str_size;
        break;
    case RSSI_DROP_RADIO_CFG:
        at_cmd = NR5G_drop_radio_cfg_at_cmd_str;
        at_cmd_length = NR5G_drop_radio_cfg_at_cmd_str_size;
        break;
    case RSSI_FR1_N79_LEGACY_TX_TEST:
        at_cmd = NR5G_FR1_tx_rssi_N79_legacy_at_cmd_str;
        at_cmd_length = NR5G_FR1_tx_rssi_N79_legacy_at_cmd_str_size;
        break;
    case RSSI_FR1_N79_TX_TEST:
        at_cmd = NR5G_FR1_tx_rssi_N79_at_cmd_str;
        at_cmd_length = NR5G_FR1_tx_rssi_N79_at_cmd_str_size;
        break;
    case RSSI_FR1_N79_STOP_LEGACY_TX_TEST:
        at_cmd = NR5G_FR1_main_tx_stop_legacy_at_cmd_str;
        at_cmd_length = NR5G_FR1_main_tx_stop_legacy_at_cmd_str_size;
        break;
    case RSSI_FR1_N79_STOP_TX_TEST:
        at_cmd = NR5G_FR1_main_tx_stop_at_cmd_str;
        at_cmd_length = NR5G_FR1_main_tx_stop_at_cmd_str_size;
        break;
    case RSSI_GPS_L1_TEST:
        at_cmd = gps_l1_rssi_at_cmd_str;
        at_cmd_length = gps_l1_rssi_at_cmd_str_size;
        break;
    case RSSI_GPS_L5_TEST:
        at_cmd = gps_l5_rssi_at_cmd_str;
        at_cmd_length = gps_l5_rssi_at_cmd_str_size;
        break;
   case NR_5G_MODEM_EXIT_TM:
        at_cmd = swi_nr_5g_disable_tm_mode_at_cmd_str;
        at_cmd_length = swi_nr_5g_disable_tm_mode_at_cmd_str_size;
        break;
    case MODEM_RESET_TEST:
        at_cmd = swi_nr_5g_reset_at_cmd_str;
        at_cmd_length = swi_nr_5g_reset_at_cmd_str_size;
        break;
    case MODEM_ATI_TEST:
        at_cmd = swi_nr_5g_ati_cmd_str;
        at_cmd_length = swi_nr_5g_ati_cmd_str_size;
        break;
    case RSSI_NR_5G_FSN_TEST:
        at_cmd = swi_nr_5g_fsn_cmd_str;
        at_cmd_length = swi_nr_5g_fsn_cmd_str_size;
        break;
    case NR_5G_SIM0_DETECT_TEST:
        at_cmd = swi_nr_5g_sim0_detect_str;
        at_cmd_length = swi_nr_5g_sim0_detect_str_size;
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
   case MODEM_TEMP_DETECT_TEST:
        at_cmd = swi_nr_5g_temp_at_cmd_str;
        at_cmd_length = swi_nr_5g_temp_at_cmd_str_size; 
        break;
   case SHOW_MODEM_INFO:
        at_cmd = swi_nr_5g_modem_info_at_cmd_str;
        at_cmd_length = swi_nr_5g_modem_info_at_cmd_str_size;
        break;
   case SHOW_MODEM_HWID:
        at_cmd = swi_nr_5g_modem_hwid_at_cmd_str;
        at_cmd_length = swi_nr_5g_modem_hwid_at_cmd_str_size;
        break;
   case SHOW_MODEM_SKU:
        at_cmd = swi_nr_5g_modem_sku_at_cmd_str;
        at_cmd_length = swi_nr_5g_modem_sku_at_cmd_str_size;
        break;
   case MODEM_HOST_IF_EXERCISE:
        at_cmd = NR_5G_trial_at_cmd_str;
        at_cmd_length = NR_5G_trial_at_cmd_str_size;
        break;
   case MMWAVE_ANTENNA_PON_ENABLE:
       at_cmd = NR_5G_mmwv_pon_enable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_enable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA_PON_DISABLE:
       at_cmd = NR_5G_mmwv_pon_disable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_disable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA0_PON_ENABLE:
       at_cmd = NR_5G_mmwv_pon_enable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_enable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA0_PON_DISABLE:
       at_cmd = NR_5G_mmwv_pon_disable_at_cmd_str;      // FIXME
       at_cmd_length = NR_5G_mmwv_pon_disable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA1_PON_ENABLE:
       at_cmd = NR_5G_mmwv_pon_enable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_enable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA1_PON_DISABLE:
       at_cmd = NR_5G_mmwv_pon_disable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_disable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA2_PON_ENABLE:
       at_cmd = NR_5G_mmwv_pon_enable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_enable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA2_PON_DISABLE:
       at_cmd = NR_5G_mmwv_pon_disable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_disable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA3_PON_ENABLE:
       at_cmd = NR_5G_mmwv_pon_enable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_enable_at_cmd_str_size;
       break;
   case MMWAVE_ANTENNA3_PON_DISABLE:
       at_cmd = NR_5G_mmwv_pon_disable_at_cmd_str;
       at_cmd_length = NR_5G_mmwv_pon_disable_at_cmd_str_size;
       break;
   case MMWAVE_FR2_RSSI_RX_TEST:
       at_cmd = NR5G_FR2_rx_rssi_at_cmd_str;
       at_cmd_length = NR5G_FR2_rx_rssi_at_cmd_str_size;
       break;
   case MMWAVE_FR2_TRANSMIT_TEST:
       at_cmd = NR5G_FR2_tx_at_cmd_str;
       at_cmd_length = NR5G_FR2_tx_at_cmd_str_size;
       break;
   case MMWAVE_FR2_TRANSMIT_STOP:
       at_cmd = NR5G_FR2_tx_stop_at_cmd_str;
       at_cmd_length = NR5G_FR2_tx_stop_at_cmd_str_size;
       break;
   case MODEM_SET_DIAG_CUSTOM_CONFIG:
       at_cmd = swi_nr_5g_custom_cfg_for_diag;
       at_cmd_length = swi_nr_5g_custom_cfg_for_diag_size;
       break;
   case MODEM_SET_DEFAULT_CUSTOM_CONFIG:
       at_cmd = swi_nr_5g_custom_cfg_default;
       at_cmd_length = swi_nr_5g_custom_cfg_default_size;
       break;
   case MODEM_IS_SARSTATE_0:
   case MODEM_IS_SARSTATE_1:
       at_cmd = swi_nr_5g_chk_sar_status_cmd_str;
       at_cmd_length = swi_nr_5g_chk_sar_status_cmd_str_size;
       break;
   case MMWAVE_ANT_PRESENT_STATUS:
       at_cmd = swi_nr_5g_mmwave_ant_status_cmd_str;
       at_cmd_length = swi_nr_5g_mmwave_ant_status_cmd_str_size;
       break;
   default:
        printf("%s: Not supported AT command ('%d')\n", __func__, at_test);
        return (FAILED);
    }   

    /* Process AT commands */
    for (ix = 0; ix < at_cmd_length; ix++) {
        if (dev_swi_5g_at_process_cmd(obj_swi_modem, 
                                      tty_dev_fd, at_cmd[ix].str, at_test, 
                                       at_cmd[ix].parse_the_result) == FAILED) {
            close(tty_dev_fd);
            return (FAILED);
        }   
    }   

    close(tty_dev_fd);
    return (PASSED);
}


/***************************************************************************
* Name: dev_swi_5g_at_process_cmd 
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
static int dev_swi_5g_at_process_cmd (dev_5g_swi_object_t *dev, 
                                       int fd, char *atcmd_str,
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
    int db, cton, freq, sar_back_off;
    unsigned int freq_min, freq_max;
    char *rslt_str, *rslt_str1, *rslt_str2, *rslt_str3;
    int len;
    
    /* Read from tty with timeout */
    FD_ZERO(&tout_set);
    FD_SET(fd, &tout_set);
    timeout.tv_sec  = AT_CMD_RESP_TOUT_IN_SEC;
    timeout.tv_usec = 0;

    static int read_val = 0;
    static int write_val = 0;
    static int occurred_cnt = 0;
    char at_buf[100];

    snprintf (at_buf, sizeof(at_buf) - 1, "%s",atcmd_str);
    if ((strcmp (at_buf, "AT+CMEE=x\r") == 0)  || \
        (strcmp (at_buf, "AT+CREG=x\r") == 0)) {
        rslt_str1 = strstr (at_buf, "x");
        if (occurred_cnt == 1) {
            write_val = ((read_val == 0) ? 1 : 0);
            *rslt_str1 = '0' + write_val;
            occurred_cnt++;
        } else {
            *rslt_str1 = '0' + read_val;
            occurred_cnt = 0;
        }
        atcmd_str = at_buf;
    }


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
            case MMWAVE_FR2_RSSI_RX_TEST:
                /* extract power from AT!DAGFTMRXAGC command */
                {
                    int exp_power;
                    printf ("atcmdstr : %s\n", atcmd_str);
                    rslt_str1 = (char *) strchr(atcmd_str, '-');
                    rslt_str1++;
                    rslt_str2 = (char *)strchr(rslt_str1, ',');
                    rslt_str2 = NULL;
                    exp_power = atoi(rslt_str1) /10;
                    printf("Expected power level is %d\n", exp_power);
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("Expected power level is %d\n", exp_power);
                    }

                    high_pwr = exp_power + MAIN_AUX_RSSI_DELTA;
                    low_pwr  = exp_power - MAIN_AUX_RSSI_DELTA;
                }
                db = 0;

                /* Find the occurance of -xx.y dBm
                 *
                 * Here is the example of AT command output
                 * AT!DAGFTMRXAGC=0,6,-800,0
                 * -81.8
                 *
                 * OK
                 */
                printf ("\nbuffer : %s", buffer);
                rslt_str1 = (char *)strchr(buffer, '-');
                rslt_str1++;
                rslt_str1 = (char *)strchr(rslt_str1, '-');

                /* Now separate the number and dBm */
                if (rslt_str1) {
                    rslt_str2 = (char *) strstr(rslt_str1, "OK");
                    rslt_str2 = NULL;
                }

                if (rslt_str1) {
                    db = atoi(rslt_str1);
                }
                    printf("db is %d\n", db);

                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("db is %d\n", db);
                }

                if ((db > high_pwr) || (db < low_pwr)) {
                    printf("\nTest Failed dBm = %d expected dBm between %d and "
                                               "%d dBm\n", db, high_pwr, low_pwr);
                    cterr('f', 0, "Reading RSSI = %d dBm Test failed"
                          "\nWarning: Please verify the settings of the signal generator.", db);
                } else {
                    printf("\nTest Passed dbm = %d\n", db);
                    return (PASSED);
                }
               
                break;
          //  case RSSI_FR1_LEGACY_RX_TEST:  //Legacy RSSI ATcommad test
            case RSSI_FR1_RX_TEST:
                high_pwr = MAIN_AUX_RSSI_AMP_DBM + MAIN_AUX_RSSI_DELTA;
                low_pwr  = MAIN_AUX_RSSI_AMP_DBM - MAIN_AUX_RSSI_DELTA;
                db = 0;

                /* Find the occurance of -xx.y dBm
                 *
                 * Here is the example of AT command output
                 * AT!DAGFTMRXAGC=0,6,-800,0
                 * -81.8
                 * 
                 * OK 
                 */
                rslt_str1 = (char *)strchr(buffer, '-');              
                rslt_str1++;
                rslt_str1 = (char *)strchr(rslt_str1, '-');
                
                /* Now separate the number and dBm */
                if (rslt_str1) {
                    rslt_str2 = (char *) strstr(rslt_str1, "OK");
                    rslt_str2 = NULL;
                }

                if (rslt_str1) {
                    db = atoi(rslt_str1);
                }

                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("db is %d\n", db);
                }
                
                if ((db > high_pwr) || (db < low_pwr)) {
                    printf("\nTest Failed dBm = %d expected dBm between %d and "
                                               "%d dBm\n", db, high_pwr, low_pwr);
                    cterr('f', 0, "Reading RSSI = %d dBm Test failed"
                          "\nWarning: Please verify the settings of the signal generator.", db);
                } else {
                    printf("\nTest Passed dbm = %d\n", db);
                    return (PASSED);
                }

                break;
            case RSSI_FR1_OTA_ALL_ANT_RX_TEST:
            case RSSI_FR1_OTA_IND_ANT_RX_TEST:
                db = 0;

                /* Find the occurance of -xx.y dBm
                 *
                 * Here is the example of AT command output
                 * AT!DAGFTMRXAGC=0,6,-600,0
                 * -61.8
                 *
                 * OK
                 */
                rslt_str1 = (char *)strchr(buffer, '-');
                rslt_str1++;
                rslt_str1 = (char *)strchr(rslt_str1, '-');

                /* Now separate the number and dBm */
                if (rslt_str1) {
                    rslt_str2 = (char *) strstr(rslt_str1, "OK");
                    rslt_str2 = NULL;
                }
                if (rslt_str1) {
                    db = atoi(rslt_str1);
                }

                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("db is %d\n", db);
                }
                if ((char *)strstr(buffer, MODEM_ATCMD_MAIN_PORT)){
                    dev->ant_rx_value[MODEM_ANT_MAIN_PORT] = db;
                } else if ((char *)strstr(buffer, MODEM_ATCMD_AUX_PORT)){
                    dev->ant_rx_value[MODEM_ANT_AUX_PORT] = db;
                } else if ((char *)strstr(buffer, MODEM_ATCMD_M1_PORT)){
                    dev->ant_rx_value[MODEM_ANT_M1_PORT] = db;
                } else if ((char *)strstr(buffer, MODEM_ATCMD_M2_PORT)){
                    dev->ant_rx_value[MODEM_ANT_M2_PORT] = db;
                }
                return (PASSED);
                break;
            case RSSI_GPS_L1_TEST:
            case RSSI_GPS_L5_TEST:
                freq_min = GPS_TEST_FREQ_MIN;
                freq_max = GPS_TEST_FREQ_MAX;
                //L5 offset is 1000Khz and L1 is 100Khz so multply with 10    
                if (at_test == RSSI_GPS_L5_TEST){
                    freq_min = GPS_TEST_FREQ_MIN*10;
                    freq_max = GPS_TEST_FREQ_MAX*10;

                }
                /* Example output:
                 * CtoN=31.7, Freq=100202
                 */
                rslt_str = strstr(buffer, "=");
                rslt_str++;
                rslt_str = strstr(rslt_str, "=");
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
                    if ((freq <= freq_max) && (freq >= freq_min)) {
                        printf("\n GPS RF Passed CtoN = %ddBm, Freq = %dHz\n",
                               cton, freq);
                        return (PASSED);
                    }
                } else {
                    cterr('f', 0, " GPS RF Failed CtoN = %ddB, Freq = %dHz\n"
                          " CtoN should be within 60 +/- 5dBm and Freq within "
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
           case MODEM_IS_SARSTATE_0:
                rslt_str1 = strstr(buffer, "?"); 
                if (rslt_str1) {
                    rslt_str2 = (char *) strstr(rslt_str1, "OK");
                    rslt_str2 = NULL;
                }
                rslt_str1++;
                if (rslt_str1) {
                    sar_back_off = atoi(rslt_str1);
                }
                if (sar_back_off == 0 ) return (PASSED);
                break;
           case MODEM_IS_SARSTATE_1:
                rslt_str1 = strstr(buffer, "?");
                if (rslt_str1) {
                    rslt_str2 = (char *) strstr(rslt_str1, "OK");
                    rslt_str2 = NULL;
                }
                rslt_str1++;
                if (rslt_str1) {
                    sar_back_off = atoi(rslt_str1);
                }
                if (sar_back_off == 1 ) return (PASSED);
                break;
            case SHOW_MODEM_INFO:
                if (strstr(buffer, "AT!PCTEMP") == NULL) {
                    rslt_str = strstr(buffer, "\n");
                    for (len = 0; len <strlen(rslt_str) ; len++) {
                        if (*(rslt_str+len) != '\n') break;
                    }
                    rslt_str1 = strstr(rslt_str+len, "\n");
                    if (rslt_str1) *rslt_str1 = '\0';
                    if (rslt_str) {
                        if (strstr (buffer, "AT+CGMI")) {
                            printf ("\nModem Manufacturer : %s", (rslt_str+len));
                        } else if (strstr (buffer, "AT+CGMM")) {
                            printf ("\nModem Model number : %s", (rslt_str+len));
                            snprintf (dev->model,  sizeof (dev->model),
                                                            "%s",(rslt_str+len));
                        } else if (strstr (buffer, "AT+CGSN")) {
                            printf ("\nModem Serial number: %s", (rslt_str+len));
                        } else if (strstr (buffer, "AT+CGMR")) {
                            printf ("\nModem Firmware     : %s", (rslt_str+len));
                            snprintf (dev->modem_firmware,  sizeof (dev->model),
                                                            "%s",(rslt_str+len));
                        } else if (strstr (buffer, "AT!PCINFO?")) {
                            rslt_str = strstr (buffer, "State:");
                            rslt_str1 = strstr (rslt_str, "\n");
                            if (rslt_str1) *rslt_str1 = '\0';
                            printf ("\nModem %s", rslt_str);
                        }
                    }
                } else if (strstr(buffer, "AT!PCTEMP")) {
                    rslt_str = strstr (buffer, "Temperature:");
                    rslt_str1 = strstr (rslt_str, "\n");
                    if (rslt_str1) *rslt_str1 = '\0';
                    printf ("\nModem %s\n\n", rslt_str);
                }
                return (PASSED);
                break;
           case SHOW_MODEM_HWID:
               if (strstr (buffer, "AT!HWID")) {
                   rslt_str = strstr (buffer, "Revision:");
                   rslt_str1 = strstr (rslt_str, "\n");
                   if (rslt_str1) *rslt_str1 = '\0';
                   printf ("\nModem %s\n", rslt_str);
               } else {
                   cterr('f', 0, "AT!HWID command failed");
                   return (FAILED);
               }
               return (PASSED);
               break;
           case SHOW_MODEM_SKU:
               if (strstr (buffer, "AT!SKU")) {
                   rslt_str = strstr (buffer, ":");
                   rslt_str+=2;
                   rslt_str1 = strstr (rslt_str, "\n");
                   if (rslt_str1) *rslt_str1 = '\0';
                   printf ("\nModem SKU = %s\n", rslt_str);
                   snprintf (dev->sku,  sizeof (dev->sku), "%s",(rslt_str));
               } else {
                   cterr('f', 0, "AT!SKU command failed");
                   return (FAILED);
               }
               return (PASSED);
               break;
           case MODEM_HOST_IF_EXERCISE:
                if (strcmp(atcmd_str, "AT+CGMI\r") == 0){
                    if (strstr(buffer, MFG_NAME) != 0) {
                        printf("mfg name PASSED\n");
                        return (PASSED);
                    }
                    else {
                        printf("mfg name not found\n");
                        return (FAILED);
                    }
                } else if (strcmp(atcmd_str, "AT+CGMM\r") == 0){
                    if (strstr(buffer, DEV_TYPE) != 0) {
                        printf("Dev type PASSED\n");
                        return (PASSED);
                    }
                    else {
                        printf("dev name not found\n");
                        return (FAILED);
                    }
                } else if (strcmp(atcmd_str, "AT+CSQ\r") == 0){
                    if (strstr(buffer, SGL_LVL) != 0) {
                        printf("sgl lvl PASSED\n");
                        return (PASSED);
                    }
                    else {
                        printf("sgl lvl not found\n");
                        return (FAILED);
                    }
                } else if (strcmp(atcmd_str, "AT!GPSSATINFO?\r") == 0){
                    if (strstr(buffer, GPS_INFO) != 0) {
                        printf("GPS_INFO PASSED\n");
                        return (PASSED);
                    }
                    else {
                        printf("sgl lvl not found\n");
                        return (FAILED);
                    }
                } else if (strcmp(atcmd_str, "AT+CMEE?\r") == 0){
                    rslt_str1 = strstr(buffer, ":");
                    rslt_str1++;
                    if (occurred_cnt == 0) {
                        read_val = atoi(rslt_str1);
                    } else {
                        if (write_val != atoi(rslt_str1)) {
                            occurred_cnt = 0;
                            return (FAILED);
                        }
                    }
                    occurred_cnt++;
                    return (PASSED);
                } else if (strcmp(atcmd_str, "AT+CREG?\r") == 0){
                    rslt_str1 = strstr(buffer, ":");
                    rslt_str1++;
                    rslt_str2 = strstr (rslt_str1, ",");
                    *rslt_str2 = '\0';
                    if (occurred_cnt == 0) {
                        read_val = atoi(rslt_str1);
                    } else {
                        if (write_val != atoi(rslt_str1)) {
                            occurred_cnt = 0;
                            return (FAILED);
                        }
                    }
                    occurred_cnt++;
                    return (PASSED);

                } else {
                    printf("\n%s", buffer);
                    return (PASSED);
                }
                break;
            case SIM_DETECT_PIN_STATUS:
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
            case RSSI_NR_5G_FSN_TEST:
                rslt_str = strstr(buffer, "FSN:");
                rslt_str1 = strstr(buffer, "+GCAP:");
                if (rslt_str1 != NULL) {
                    rslt_str1[0]= '\0';
                }
                snprintf (dev->fsn, sizeof (dev->fsn), "%s", rslt_str);
                return PASSED;
            case MODEM_TEMP_DETECT_TEST:
                printf("\n%s", buffer);
                return (PASSED);
                break;
            case MMWAVE_ANT_PRESENT_STATUS:
                if (check_mmwave_ant_present_status(buffer) == 0 ){
                    return (PASSED);
                } else {
                    return (FAILED);
                }
                break;
        }
    }
    
    return (FAILED);
}
/***************************************************************************
* Name: dev_swi_5g_at_open_tty
*
* Description: This function opens tty device for AT command 
* 
* Input: tty_dev_name - TTY USB Device Name
*        *tty_fd - Pointer to the TTY file descriptor
*
* Output: PASSED/FAILED
***************************************************************************/
static int dev_swi_5g_at_open_tty (char *tty_dev_name, int *tty_fd)
{
    int fd;
    struct termios options;
    int timeout = VTIME_TIMEOUT;
    char usb_tty[15];

    sprintf(usb_tty, "%s%s", TTY_DEV_PATH, tty_dev_name);
    fd = open(usb_tty, O_RDWR | O_NOCTTY | O_NDELAY);

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

/***************************************************************************
* Name: sub6_ant_test_band_config
*
* Description: This function configures the AT cmd to respective band
*              for performing RSSI test   
* Input: band_tbl - list of bands supported to perform RSSI
*        ant_type - which antenna to test 
*
* Output: PASSED/FAILED
***************************************************************************/


int sub6_ant_test_band_config(nr_sub6_band_struct *band_tbl, 
                         int ant_type, int exp_pwr) {
    int i;
    char *ptr;
    at_cmd_str  *at_cmd_tbl;
    unsigned int at_cmd_tbl_size;
    int all_ant_test = 0;

    exp_pwr = exp_pwr*10;
    if (ant_type == (MAIN_RSSI | AUX_RSSI | MIMO1_RSSI | MIMO2_RSSI)) {
        all_ant_test = MODEM_ALL_ANT;
    }

    if (all_ant_test != MODEM_ALL_ANT){
        at_cmd_tbl = NR5G_FR1_rx_rssi_at_cmd_str;
        at_cmd_tbl_size = NR5G_FR1_rx_rssi_at_cmd_str_size;	    
    } else {
        at_cmd_tbl = NR5G_FR1_ota_rx_at_cmd_str;
        at_cmd_tbl_size = NR5G_FR1_ota_rx_at_cmd_str_size;	    
    }

    for (i = 0; i < at_cmd_tbl_size; i++, at_cmd_tbl++) {
        if ((ptr = (char *) strstr(at_cmd_tbl->str, "AT!DAUPDATEPARAM"))) {
            break;
        }
    }
    if (i >= at_cmd_tbl_size){
        printf ("\nCould not find the AT!DAUPDATEPARAM in the table");
        return FAILED;
    }

    sprintf (swi_NR_5G_DAUPDATEPARAM, \
                "AT!DAUPDATEPARAM=%d,%d\r",MODEM_TECH_FAMILY, band_tbl->band_num);
    at_cmd_tbl->str = swi_NR_5G_DAUPDATEPARAM;

    for (; i < at_cmd_tbl_size; i++, at_cmd_tbl++) {
        if ((ptr = (char *) strstr(at_cmd_tbl->str, "AT!DARCONFIG"))) {
            break;
        }
    }
    if (i >= at_cmd_tbl_size){
        printf ("\nCould not find the AT!DARCONFIG in the table");
        return FAILED;
    }

    if (all_ant_test == 0) {
        if ((ant_type == MAIN_RSSI) || (ant_type == AUX_RSSI)) {
            sprintf (swi_NR_5G_DARCONFIG, \
             "AT!DARCONFIG=%d,%d,%d,%d,%d,%d\r",
                 MODEM_CARRIER, MODEM_TECH,
                 band_tbl->band_num,band_tbl->tx_channel, 
                 band_tbl->band_width, band_tbl->rx_channel);
        } else {
            sprintf (swi_NR_5G_DARCONFIG, \
                 "AT!DARCONFIG=%d,%d,%d,%d,%d,%d,%d\r",
                 MODEM_CARRIER, MODEM_TECH,
                 band_tbl->band_num,band_tbl->tx_channel, 
                 band_tbl->band_width, band_tbl->rx_channel, MODEM_ANT_MIMO_CFG);
        }
        at_cmd_tbl->str = swi_NR_5G_DARCONFIG;

        for (; i < at_cmd_tbl_size; i++, at_cmd_tbl++) {
            if ((ptr = (char *) strstr(at_cmd_tbl->str, "AT!DAGFTMRXAGC"))) {
                break;
            }
        }
        if (i >= at_cmd_tbl_size){
            printf ("\nCould not find the AT!DAGFTMRXAGC in the table");
            return FAILED;
        }

        sprintf (swi_NR_5G_DAGFTMRXAGC, \
             "AT!DAGFTMRXAGC=%d,%d,%d,%d\r", 
                 MODEM_CARRIER, MODEM_TECH, exp_pwr,
                 (ant_type == MAIN_RSSI) ? MODEM_ANT_MAIN_PORT : 
                 (ant_type == AUX_RSSI)  ? MODEM_ANT_AUX_PORT :
                 (ant_type == MIMO1_RSSI)? MODEM_ANT_M1_PORT : 
                                           MODEM_ANT_M2_PORT);

       at_cmd_tbl->str = swi_NR_5G_DAGFTMRXAGC;
    } else {

        sprintf (swi_NR_5G_DARCONFIG, \
         "AT!DARCONFIG=%d,%d,%d,%d,%d,%d\r",
             MODEM_CARRIER, MODEM_TECH,
             band_tbl->band_num,band_tbl->tx_channel, 
             band_tbl->band_width, band_tbl->rx_channel);

        at_cmd_tbl->str = swi_NR_5G_DARCONFIG;

        for (; i < at_cmd_tbl_size; i++, at_cmd_tbl++) {
            if ((ptr = (char *) strstr(at_cmd_tbl->str, "AT!DAGFTMRXAGC"))) {
                break;
            }
        }
        if (i >= at_cmd_tbl_size){
            printf ("\nCould not find the AT!DAGFTMRXAGC in the table");
            return FAILED;
        }

        sprintf (swi_NR_5G_DAGFTMRXAGC_0, "AT!DAGFTMRXAGC=%d,%d,%d,%d\r",
                       MODEM_CARRIER, MODEM_TECH, exp_pwr,MODEM_ANT_MAIN_PORT);


        at_cmd_tbl->str = swi_NR_5G_DAGFTMRXAGC_0;

        at_cmd_tbl++;
        sprintf (swi_NR_5G_DAGFTMRXAGC_3, "AT!DAGFTMRXAGC=%d,%d,%d,%d\r",
                       MODEM_CARRIER, MODEM_TECH, exp_pwr,MODEM_ANT_AUX_PORT);
        at_cmd_tbl->str = swi_NR_5G_DAGFTMRXAGC_3;

        at_cmd_tbl++;
        sprintf (swi_NR_5G_DARCONFIG_1, \
             "AT!DARCONFIG=%d,%d,%d,%d,%d,%d,%d\r",
             MODEM_CARRIER, MODEM_TECH,
             band_tbl->band_num,band_tbl->tx_channel, 
             band_tbl->band_width, band_tbl->rx_channel, MODEM_ANT_MIMO_CFG);
        at_cmd_tbl->str = swi_NR_5G_DARCONFIG_1;

        at_cmd_tbl++;
        sprintf (swi_NR_5G_DAGFTMRXAGC_1, "AT!DAGFTMRXAGC=%d,%d,%d,%d\r",
                                           MODEM_CARRIER, MODEM_TECH, exp_pwr,MODEM_ANT_M1_PORT);
        at_cmd_tbl->str = swi_NR_5G_DAGFTMRXAGC_1;

        at_cmd_tbl++;
        sprintf (swi_NR_5G_DAGFTMRXAGC_2, "AT!DAGFTMRXAGC=%d,%d,%d,%d\r",
                                           MODEM_CARRIER, MODEM_TECH, exp_pwr,MODEM_ANT_M2_PORT);
        at_cmd_tbl->str = swi_NR_5G_DAGFTMRXAGC_2;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        for (i = 0; i < at_cmd_tbl_size; i++) {
            printf ("\n%s", (all_ant_test) ? 
                  NR5G_FR1_ota_rx_at_cmd_str[i].str : 
                  NR5G_FR1_rx_rssi_at_cmd_str[i].str);
        }
    }

    return 0;
}

/***************************************************************************
* Name: ant_mmwave_rx_test_radio_config
*
* Description: This function configures the AT cmd to respective band
*              for performing RSSI test for mmwave testing
* Input: band_tbl - list of bands supported to perform RSSI
*        antenna_mask - which antenna to turn on
*        power - power level to measure
*
* Output: PASSED/FAILED
***************************************************************************/


int ant_mmwave_rx_test_radio_config(nr_mmwave_band_struct *band_tbl, int antenna_mask,
                                    int power)
{
    int i;
    char *ptr;

    /* AT!DARCONFIG */
    for (i = 0; i < NR5G_FR2_rx_rssi_at_cmd_str_size; i++) {
        if ((ptr = (char *) strstr(NR5G_FR2_rx_rssi_at_cmd_str[i].str, "AT!DARCONFIG"))) {
            break;
        }
    }
    if (i >= NR5G_FR2_rx_rssi_at_cmd_str_size){
        printf ("\nCould not find the AT!DARCONFIG in the table");
        return FAILED;
    }

    sprintf (swi_NR_5G_DARCONFIG, \
            "AT!DARCONFIG=%d,%d,%d,%d,%d,%d,%d,%d\r",
             MODEM_CARRIER, MODEM_TECH,
            band_tbl->band_num, band_tbl->tx_channel,
            band_tbl->band_width, band_tbl->rx_channel,
            MODEM_MIMO_MODE,
            antenna_mask);

    NR5G_FR2_rx_rssi_at_cmd_str[i].str = swi_NR_5G_DARCONFIG;

    /* AT!DAGFTMRXAGC */
    for (i = 0; i < NR5G_FR2_rx_rssi_at_cmd_str_size; i++) {
        if ((ptr = (char *) strstr(NR5G_FR2_rx_rssi_at_cmd_str[i].str, "AT!DAGFTMRXAGC"))) {
            break;
        }
    }
    if (i >= NR5G_FR2_rx_rssi_at_cmd_str_size){
        printf ("\nCould not find the AT!DAGFTMRXAGC in the table");
        return FAILED;
    }
    sprintf (swi_NR_5G_DAGFTMRXAGC, \
            "AT!DAGFTMRXAGC=%d,%d,%d,%d,%d\r", 
            MODEM_CARRIER, MODEM_TECH, power*10, MODEM_PATH, antenna_mask);

    NR5G_FR2_rx_rssi_at_cmd_str[i].str = swi_NR_5G_DAGFTMRXAGC;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf ("\nDBEUG: print modified 'NR5G_FR2_rx_rssi_at_cmd_str' structure:\n");
        for (i = 0; i < NR5G_FR2_rx_rssi_at_cmd_str_size; i++) {
            printf ("\n%s", NR5G_FR2_rx_rssi_at_cmd_str[i].str);
       }
    }
    return 0;
}

/***************************************************************************
* Name: ant_mmwave_tx_test_radio_config
*
* Description: This function configures the AT cmd to respective band
*              for performing transmit test for mmwave testing
* Input: band_tbl - list of bands supported to perform RSSI
*        antenna_mask - which antenna to turn on
*        power - power level to transmit
*
* Output: PASSED/FAILED
***************************************************************************/


int ant_mmwave_tx_test_radio_config(nr_mmwave_band_struct *band_tbl, int antenna_mask,
                                    int power)
{

    int i;
    char *ptr;

    /* AT!DARCONFIG */
    for (i = 0; i < NR5G_FR2_tx_at_cmd_str_size; i++) {
        if ((ptr = (char *) strstr(NR5G_FR2_tx_at_cmd_str[i].str, "AT!DARCONFIG"))) {
            break;
        }
    }
    if (i >= NR5G_FR2_tx_at_cmd_str_size){
        printf ("\nCould not find the AT!DARCONFIG in the table");
        return FAILED;
    }

    sprintf (swi_NR_5G_DARCONFIG, \
                "AT!DARCONFIG=%d,%d,%d,%d,%d,%d,%d,%d\r",
                MODEM_CARRIER, MODEM_TECH,
                band_tbl->band_num, band_tbl->tx_channel,
                band_tbl->band_width, band_tbl->rx_channel,
                MODEM_MIMO_MODE,
                antenna_mask);

    NR5G_FR2_tx_at_cmd_str[i].str = swi_NR_5G_DARCONFIG;  

    /* AT!DATXCONTROL to enable Tx */
    for (i = 0; i < NR5G_FR2_tx_at_cmd_str_size; i++) {
        if ((ptr = (char *) strstr(NR5G_FR2_tx_at_cmd_str[i].str, "AT!DATXCONTROL"))) {
            break;
        }
    }
    if (i >= NR5G_FR2_tx_at_cmd_str_size){
        printf ("\nCould not find the AT!DATXCONTROL in the table");
        return FAILED;
    }

    sprintf (swi_NR_5G_DATXCONTROL_start, \
            "AT!DATXCONTROL=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r",
            MODEM_CARRIER,MODEM_TECH,MODEM_ENABLE_TX, power*10,
            MODEM_MMWAVE_WAVEFORM, MODEM_MMWAVE_MOD,MODEM_MMWAVE_NW_SGL_VAL,
            MODEM_MMWAVE_START_RB, MODEM_MMWAVE_NUM_RB, antenna_mask);

    NR5G_FR2_tx_at_cmd_str[i].str = swi_NR_5G_DATXCONTROL_start;


    /* AT!DATXCONTROL to disable Tx */
    for (i = 0; i < NR5G_FR2_tx_stop_at_cmd_str_size; i++) {
        if ((ptr = (char *) strstr(NR5G_FR2_tx_stop_at_cmd_str[i].str, "AT!DATXCONTROL"))) {
            break;
        }
    }
    if (i >= NR5G_FR2_tx_stop_at_cmd_str_size){
        printf ("\nCould not find the AT!DATXCONTROL in the table");
        return FAILED;
    }


    sprintf (swi_NR_5G_DATXCONTROL_stop, \
            "AT!DATXCONTROL=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r",
            MODEM_CARRIER,MODEM_TECH,MODEM_DISABLE_TX, power*10,
            MODEM_MMWAVE_WAVEFORM, MODEM_MMWAVE_MOD,MODEM_MMWAVE_NW_SGL_VAL, 
            MODEM_MMWAVE_START_RB, MODEM_MMWAVE_NUM_RB, antenna_mask);


    NR5G_FR2_tx_stop_at_cmd_str[i].str = swi_NR_5G_DATXCONTROL_stop;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf ("\nDBEUG: print modified 'NR5G_FR2_tx_at_cmd_str' structure:\n");
        for (i = 0; i < NR5G_FR2_tx_at_cmd_str_size; i++) {
            printf ("\n%s", NR5G_FR2_tx_at_cmd_str[i].str);
        }
    }
    return 0;
}

/***************************************************************************
* Name: check_mmwave_ant_present_status
*
* Description: This function check the mmwave antenna present status 
* Input: mmwave_ant_status_buf - AT command buffer
*
* Output: PASSED/FAILED
***************************************************************************/
static int check_mmwave_ant_present_status (char *mmwave_ant_status_buf) {
    int ant = 0;
    int travers_ant_inst = 0;
    int ant_instance = 0;
    int res = 0;
    char *result;
    for (ant = 0; ant < MAX_QTM_ANTS; ant++){
        for (travers_ant_inst = 0; travers_ant_inst < INST_PER_ANT; travers_ant_inst++){
            ant_instance = mmwave_ant_instance_map[ant][travers_ant_inst];
            result = strstr (mmwave_ant_status_buf, &mmwave_antenna_status[ant_instance][0]);
            if (result == NULL) {
                printf ("\nAnt : %d instance : %d Failed to detect", ant, ant_instance);
                res |= 1 << ant;
            }
        }
    }
    return (res);
}

/*********************************************************************
 * $Log: dev_NR_5G_swi_at.c,v $
 * Revision 1.3  2021/06/30 20:04:55  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.7  2021/03/09 01:23:07  tshanmug
 * Chrysler and Empire modem state display on bootup message
 *
 * Revision 1.1.4.6  2020/12/31 07:21:51  tshanmug
 * chrysler mmwave antenna detetection test added
 *
 * Revision 1.1.4.5  2020/12/22 22:49:28  tshanmug
 * Empire prrq review comment fix
 *
 * $Endlog$
 */

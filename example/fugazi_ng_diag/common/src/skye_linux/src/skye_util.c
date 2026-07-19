/* $Id: skye_util.c,v 1.2 2015/05/25 03:59:17 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_util.c,v $
 *******************************************************************************
 * File Name: skye_util.c
 *
 * Description: This file is for test functions
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "defs.h"
#include "types.h"
#include "common.h"
#include "error.h"
#include "proto.h"
#include "queryflags.h" 
#include "skye_main.h"
#include "skye_i2c.h"
#include "common_utils.h"
#include "router_if.h"
#include "nvmonvars.h"
#include "skye_comm_lib.h"

/*******************************************************************************
 *                           Function Prototypes
 *******************************************************************************/
static long diag_alive_check_wrap(char *);
int         lnx_get_dev_maj_num(char *, int *);


/*******************************************************************************
 *                                Externs
 *******************************************************************************/
extern boolean cpu_id;
extern int     szalinski_spirom_read(unsigned int, unsigned int,unsigned int);
extern int     szalinski_spirom_write(unsigned int, unsigned int, unsigned int,
                                      unsigned int);
extern int     skye_dump_volt_margins(void);
extern int     skye_dump_temps(void);
extern long    skye_diag_do_all(char *);
extern int     skye_set_volt_margin_by_nc(char *);
extern long    skye_diag_mem_test_all(char *);
extern long    skye_diag_fpga_test_all(char *);
extern long    skye_diag_spirom_test_all(char *);
extern long    skye_diag_i2c_dev_test_all(char *);
extern long    skye_diag_tlk_test_all(char *);
extern long    skye_diag_pcie_test_all(char *);
extern int     skye_set_freq_margin_by_nc(char *);

/*******************************************************************************
 *                            Global Variables
 *******************************************************************************/
#define REG_DUMP_BOUND    8
#define SKYE_BP_IP_ADDR   "192.123.123.1"

static unsigned short t_flag = 0;
static unsigned long t_xflag = 0;

static struct nc_command nc_cmd_items[] = {
    {DIAG_CMD_ALIVE_CHECK,   diag_alive_check_wrap},
    {DIAG_DO_ALL_TEST, skye_diag_do_all},
    {DIAG_DO_MEM_TEST, skye_diag_mem_test_all},
    {DIAG_DO_I2CDEV_TEST, skye_diag_i2c_dev_test_all},
    {DIAG_DO_FPGA_TEST, skye_diag_fpga_test_all},
    {DIAG_DO_SPIROM_TEST, skye_diag_spirom_test_all},
    {DIAG_DO_TLK_TEST, skye_diag_tlk_test_all},
    {DIAG_DO_PCIE_TEST, skye_diag_pcie_test_all},
};

#define NC_CMDS_SIZE   (sizeof(nc_cmd_items)/sizeof(struct nc_command))


/**********************************************************************
 *
 * Function: skye_switch_console
 *
 * This function provides the menu
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int
skye_switch_console (void)
{
    uchar ch;

    while (1) {
	printf("\nHit any key to start, '#' to quit");
	ch = getchar();
	if (ch == '#') {
	    break;
	}
	skye_menu();

    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_cpu_speed
 * Description: Entry function for Set scaling cpu speed.
 * Inputs     : menu_opt - parameter determines to show menu or not
 * Outputs    : None
 *
 *******************************************************************************
 */
int
set_cpu_speed (void)
{
    int target = 0;
    /* Read current speed */
    printf("Current Frequency Margins: ");fflush(0);
    system("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    /* Choose option which speed that currently want to change */
    printf("\n\nList of cpu frequency margins:\n");
    printf("1. 1.2Ghz\n");
    printf("2. 1.1Ghz\n");
    printf("3. 1.0Ghz\n");
    printf("4. 900Mhz\n");
    printf("5. Exit\n");
    target = getdec_answer("Please enter the frequency you want: ", 5, 1, 5);

    switch (target) {
    case 1:
        system("echo 1200000 >/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed");
    break;
    case 2:
        system("echo 1100000 >/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed");
    break;
    case 3:
        system("echo 1000000 >/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed");
    break;
    case 4:
        system("echo 900000 >/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed");
    break;
    case 5:
        printf("\nCancelled by User.\n");
        return (PASSED);
    default:
    printf("\n%s: Invalid target (%d).\n", __FUNCTION__, target);
    return (FAILED);
    }
    /* Read back current speed to confirm is changed as requested  */
    printf("Latest changed on Frequency Margins: ");fflush(0);
    system("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: sys_init
 *
 * Description: Initialize the system environment
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int sys_init (void)
{
    if (cpu_id == MASTER_CPU) {
        int  s_maj = 0;
        char cmd_s[64];

        lnx_get_dev_maj_num("srom", &s_maj);
        sprintf(cmd_s, "mknod /dev/srom/3 c %d 3", s_maj);
        system(cmd_s);

#ifdef SKYE_P1A
        /* for USB */
        system("mknod /dev/sdb b 8 0");
        system("mknod /dev/sdb1 b 8 1");
#endif  /* SKYE_P1A */

        /* system("ifconfig xgbe1 up"); */
        system("ifconfig xgbe1 192.168.1.101 netmask 255.255.255.0");
        /* system("ifconfig xgbe2 up"); */
        system("ifconfig xgbe2 192.123.123.201 netmask 255.255.255.0");

#ifdef SKYE_P1A
        /* system("ifconfig gbe3 up"); */
        system("ifconfig gbe3 192.168.1.103 netmask 255.255.255.0");
        /* system("ifconfig gbe4 up"); */
        system("ifconfig gbe4 192.123.123.101 netmask 255.255.255.0");
        /* system("ifconfig gbe5 up"); */
        system("ifconfig gbe5 192.168.1.105 netmask 255.255.255.0");
#endif  /* SKYE_P1A */

        /* For CPU 0 setup IPtable_NAT mode */
        system("iptables -A INPUT -i gbe4 -s 192.123.123.1/24 -j ACCEPT");
        system("echo \"1\" > /proc/sys/net/ipv4/ip_forward");
        system("iptables -A INPUT -i xgbe1 -j ACCEPT");
        system("iptables -t nat -A POSTROUTING -s 192.168.1.0/16 -o gbe4 -j MASQUERADE");
    } else {
#ifdef SKYE_P1A
        /* for USB */
        system("mknod /dev/sdb b 8 0");
        system("mknod /dev/sdb1 b 8 1");

        /* system("ifconfig xgbe1 up"); */
        system("ifconfig xgbe1 192.168.1.102 netmask 255.255.255.0");
        /* system("ifconfig gbe2 up"); */
        system("ifconfig gbe2 192.168.1.106 netmask 255.255.255.0");
#endif  /* SKYE_P1A */
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : dump_readback_reg
 * Description: Function to dump read back register(s).
 * Inputs     : NONE
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int
dump_readback_reg (char *dev_name, uint16_t s_off,
                   uint16_t len, uchar *data)
{
    uint16_t ctr = 0, t_len = 0, s_row = 0, r_ctr = 0; 

    if (s_off % REG_DUMP_BOUND) {
        s_row = (uint16_t)(s_off - (s_off % REG_DUMP_BOUND));
    } else {
        s_row = s_off;
    }

    t_len = (len + (uint16_t)(s_off % REG_DUMP_BOUND)
                 + ((uint16_t)REG_DUMP_BOUND -
                    (uint16_t)((s_off + len) % REG_DUMP_BOUND)));

    printf("\n\n%s reg. 0x%02X to 0x%02X:\n", dev_name, s_off, (s_off + len));

    printf("\n         ");
    for (ctr = 0; ctr < REG_DUMP_BOUND; ctr++) {
        printf("0x%02X ", ctr);
    }

    for (ctr = 0; ctr < t_len; ctr++) {
        if ((ctr % REG_DUMP_BOUND) == 0) {
            printf("\n0x%04X:  ", (s_row + (uint16_t)(r_ctr * REG_DUMP_BOUND)));
            r_ctr++;
        }

        if (((s_row + ctr) < s_off) || ((s_row + ctr) >= (s_off + len))) {
            printf(" --  ");
        } else {
            printf("0x%02X ", data[ctr]);
        }
    }
    printf("\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_dump_env_prm
 * Description: Function to dump Skye testing enivronmental parameters.
 * Inputs     : NONE
 * Outputs    : NONE
 *
 *******************************************************************************
 */
void
skye_dump_env_prm (void)
{
    /* Dump current Voltage Margins */
    if (skye_dump_volt_margins() != PASSED) {
        printf("%s: Failed to dump current Voltage Margins.\n", __FUNCTION__);
    } else {
        printf("\n");
    }

    /* Dump current Temperatures */
    if (skye_dump_temps() != PASSED) {
        printf("%s: Failed to dump current Temperatures.\n", __FUNCTION__);
    } else {
        printf("\n");
    }
}

/*******************************************************************************
 *
 * Function   : diag_report_status_host
 * Description: This function reports the pass/fail status to host through nc
 * Inputs     : str - status string
 * Outputs    : None
 *
 *******************************************************************************
 */
void
diag_report_status_host (char *str)
{
    char cmd[1024];

    /* Sanity check */
    if (str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return;
    }
    /* send dblog files when it fail */
    if (strncmp(str, DIAG_RTN_FAIL_STR, sizeof(str)) == 0) {
        sprintf(cmd, "nc %s %d < %s &",
                     SKYE_BP_IP_ADDR,
                     DIAG_RTN_DBLOG_PORT_BASE, SKYE_NC_DBLOG_FILE);
        system(cmd);
        msleep(1000);  /* wait for 1 second to make sure the file is sent out */
    }

    sprintf(cmd, "echo %s | nc %s %d &",
            str, SKYE_BP_IP_ADDR,
            DIAG_RTN_STS_OUT_PORT_BASE);
    system(cmd);
}

/*******************************************************************************
 *
 * Function   : skye_nc_parser
 * Description: Function to parser the real NC command and set-up environments.
 * Inputs     : *nc_set  - content of gotten NC command set
 *              *cmd_buf - buffer to put the parser out command
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_nc_parser (char *nc_set, char *cmd_buf)
{
    char *nc_type = NULL, *nc_val = NULL;

    printf("%s: nc_set = %s, cmd_buf = %s.\n",
           __FUNCTION__, nc_set, cmd_buf);

    /* Start to parser NC command set */
    /* 1. Get NC command type */
    nc_type = strtok(nc_set, ":");

    /* 2. Get NC command value */
    nc_val = strtok(NULL, ":");

    printf("%s: NC command type = %s, value = %s.\n",
           __FUNCTION__, nc_type, nc_val);

    if (strcmp("FM", nc_type) == 0) {
        if (skye_set_freq_margin_by_nc(nc_val) != PASSED) {
            printf("%s: Failed to set Skye frequency margin.\n", __FUNCTION__);
            return (FAILED);
        }
    } else if (strcmp("VM", nc_type) == 0) {
        if (skye_set_volt_margin_by_nc(nc_val) != PASSED) {
            printf("%s: Failed to set Skye voltage margin.\n", __FUNCTION__);
            return (FAILED);
        }
    } else if (strcmp("CMD", nc_type) == 0) {
        strcpy(cmd_buf, nc_val);
    } else if (strcmp("FLAG", nc_type) == 0) {
        t_flag = atoi(nc_val);
        (NVRAM)->diagflag = t_flag;
    } else if (strcmp("xFLAG", nc_type) == 0) {
        t_xflag = atol(nc_val);
        diagflag_xram = t_xflag;
    } else {
        printf("%s: Got unknown NC command - %s.\n",
               __FUNCTION__, nc_type);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_nc_dispatch_comm
 * Description: This function reads the command transferred via nc command
 *              and execute the function accordingly.
 *              Format: opcode,option (comma as delimiter)
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void
diag_nc_dispatch_comm (void)
{
    FILE *fp;
    char buff[256], cmd_set[10][64], current_cmd[64];
    char *token = NULL;
    char cmd[32], d_cmd[64];
    int  ix = 0, ctr = 0, total_cmd = 0;

    memset(current_cmd, 0, sizeof(current_cmd));

    /* Delete command temp file */
    sprintf(cmd, "rm -f %s", DIAG_CMD_DISPATCH_FILE);
    system(cmd);

    /* Retrieve the command from host side */
    sprintf(cmd, "nc -l -l -p %d > %s",
            DIAG_EXEC_CMD_TRANS_PORT_BASE, DIAG_CMD_DISPATCH_FILE);
    system(cmd);

    fp = fopen(DIAG_CMD_DISPATCH_FILE, "r");
    if (fp == NULL) {
        printf("%s: Open %s fails\n", __FUNCTION__, DIAG_CMD_DISPATCH_FILE);
        return;
    }

    if (fgets(buff, sizeof(buff), fp) == NULL) {
        printf("Nothing in buffer\n");
        goto __exit;
    }

    /* Get the NC command set(s) */
    token = strtok(buff, ";");

    while (token != NULL) {
        strcpy(cmd_set[total_cmd], token);
        total_cmd++;

        /* Get next command flag set */
        token = strtok(NULL, ";");
    }

    for (ctr = 0; ctr < total_cmd; ctr++) {
        if (skye_nc_parser(cmd_set[ctr], current_cmd) != PASSED) {
            printf("%s: Failed to parse NC command.\n", __FUNCTION__);
            goto __exit;
        }
    }

    for (ix = 0; ix < NC_CMDS_SIZE; ix++) {
        if (!strcmp(nc_cmd_items[ix].cmd_str, current_cmd)) {
            nc_cmd_items[ix].func(current_cmd);
            break;
        }
    }

__exit:
    /* Send this to Host side when NC command execute Done.*/
    sprintf(d_cmd, "echo Okay, done !!! | nc -w %d %s %d",
            SKYE_NC_CONN_ITVL, SKYE_BP_IP_ADDR,
            SKYE_NC_DONE_PORT);
    system(d_cmd);

    fclose(fp);
}

/*******************************************************************************
 *
 * Function   : diag_alive_check_wrap
 * Description: This function to check CPU alive through NC.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
static long diag_alive_check_wrap (char *opt)
{
    char r_string[64];

    if (DIAGFLAG & D_VERBOSE) {
        printf("Hello !! This is Skye CPU%d.\n", cpu_id);
    }
    sprintf(r_string, "PASS !! Skye CPU%d is alive.", cpu_id);
    diag_report_status_host(r_string);

    return (PASSED);
}


/**************************************************************************
 *
 * Function: is_host_xgbe2_up
 *
 * Check if Skye xgbe2 interface is up by sending ping packet via the GESW
 *
 * Input: verbose - flag to control message printing
 *
 * Return: TRUE/FALSE
 *
 * *************************************************************************
 */
int is_host_xgbe2_up (boolean verbose)
{
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline, pktsize;
    char *result_file = SKYE_PING_XGBE2_RESULT_FILES;
    static boolean result = FALSE;
    FILE *fp;
    int tx_cnt, rx_cnt;
    int rv = FAILED;
    char sm_ip[16];

    pktcnt = 2;
    deadline = 5;
    pktsize = 1500;

    sprintf(cmdbuf, "rm %s", result_file);
    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp != NULL) {
        fclose(fp);
        sprintf(cmdbuf, "rm %s", result_file);
        system(cmdbuf);
    }

    sprintf(sm_ip, "%s", "192.123.123.1");
    sprintf(cmdbuf, "ping -c %d -w %d -s %d %s > %s",
                pktcnt, deadline, pktsize, sm_ip, result_file);
    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        if (verbose) {
            printf("SKYE: Ping DP %s was not created\n", result_file);
        }
        goto is_skye_up_exit;
    }

    /* Check the result
     */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);

        if (strstr(buf, "received") != NULL) {
#if DEBUG
            printf("SKYE: Ping DP result: %s", buf);
#endif
            result = TRUE;
            break;
        }
    }
    fclose(fp);

    if (result == FALSE) {
        goto  is_skye_up_exit;
    }
    /* Read the string
     */
    sscanf(buf, "%d %s %s %d", &tx_cnt, dum_char, dum_char, &rx_cnt);

    if (rx_cnt < pktcnt) {
        if (verbose) {
            printf("SKYE: Ping DP packet count mismatch. "
                   "Expected= %d, Actual: tx= %d rx= %d\n",
                   pktcnt, tx_cnt, rx_cnt);
        }
        goto  is_skye_up_exit;
    } else {
        rv = PASSED;
    }

is_skye_up_exit:

    if (rv == PASSED) {
        if (verbose) {
            printf("SKYE: Ping DP via GESW passed.\n");
        }
        return (PASSED);
    } else {
        if (verbose) {
            printf("SKYE: Ping DP via GESW failed.\n");
        }
        return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function   : lnx_get_dev_maj_num
 * Description: Function to get device major number from Linux /proc/devices.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int lnx_get_dev_maj_num (char *dev_name, int *dev_maj)
{
    char        cmd_buf[128], maj_p[32];
    FILE        *fp;
    struct stat f_st;
    int         f_sz = 0;

    memset(cmd_buf, 0, sizeof(cmd_buf));
    memset(maj_p, 0, sizeof(maj_p));

    sprintf(cmd_buf,
            "cat /proc/devices | grep \"%s\" | awk '{print $1}' > /tmp/%s.maj",
            dev_name, dev_name);
    system(cmd_buf);

    sprintf(maj_p, "/tmp/%s.maj", dev_name);
    fp = fopen(maj_p, "r");
    if (fp == NULL) {
        cterr('f', 0, "Failed to open %s", maj_p);
        return (FAILED);
    }

    stat(maj_p, &f_st);
    f_sz = f_st.st_size;
    if (f_sz == 0) {
        cterr('f', 0, "%s is empty, is device %s exist ?", maj_p, dev_name);
        fclose(fp);
        return (FAILED);
    }

    fscanf(fp, "%d", dev_maj);

    fclose(fp);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 * $Log: skye_util.c,v $
 * Revision 1.2  2015/05/25 03:59:17  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/04/30 08:33:54  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:37  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.18  2015/03/26 08:33:40  steja
 * Debug edvt found issue on 2CPU skye Dual CPU Xaui Test
 *
 * Revision 1.1.2.17  2015/01/26 01:14:24  steja
 * Add function for frequency margin to host side menu utilities through NC
 *
 * Revision 1.1.2.16  2015/01/22 09:07:47  palin2
 * Moved up I2C scan test ordering in Skye Diag main tests.
 *
 * Revision 1.1.2.15  2015/01/08 07:26:01  palin2
 * 1. Updated Szalinski FW to 14/12/24/18,
 *    this update is to fix the UART hang issue on Skye P1B 2-CPUs version.
 * 2. Dynamically get device major number from Linux /proc/devices.
 *
 * Revision 1.1.2.14  2014/11/27 07:25:20  palin2
 * 1. Fixed PCIe lanes Scan test.
 * 2. Added PCIe lanes Scan test to 2-CPUs Skye default tests.
 * 3. Added SKYE_P1A compile flag to tell difference between P1A and P1B.
 *
 * Revision 1.1.2.13  2014/11/27 02:32:50  steja
 * 1.Fix the intermittent failure to run do all test(CSCur27613)
 * 2.Update TLK full data path by ping test.
 *
 * Revision 1.1.2.12  2014/11/21 09:37:34  steja
 * Support Full data path loopback for 10G-KR by ping test
 *
 * Revision 1.1.2.11  2014/11/10 09:42:45  steja
 * Update TLK10232 10G KR loopback setup
 *
 * Revision 1.1.2.10  2014/10/07 06:05:03  palin2
 * Moved NC command module side related function from skye_main.c to skye_util.c
 *
 * Revision 1.1.2.9  2014/10/02 06:06:03  steja
 * Remove init default ip on module instead using host setup ip config
 *
 * Revision 1.1.2.8  2014/09/18 07:03:06  palin2
 * Added to show current enivronmental parameters in memory test with VERBOSE flag.
 *
 * Revision 1.1.2.7  2014/09/17 11:13:15  palin2
 * Removed unused code.
 *
 * Revision 1.1.2.6  2014/09/17 04:35:08  palin2
 * Updated Skye enhanced error message.
 *
 * Revision 1.1.2.5  2014/09/12 14:38:43  steja
 * Update code for CPU do all test
 *
 * Revision 1.1.2.4  2014/08/28 08:03:24  palin2
 * Update Skye show all temp. and all voltage margin states utilities to
 * support enhanced error message.
 *
 * Revision 1.1.2.3  2014/08/28 02:54:26  steja
 * Support Do all test for NC command
 *
 * Revision 1.1.2.2  2014/08/22 04:58:54  palin2
 * First check-in to enhance Skye error message.
 *
 * Revision 1.1.2.1  2014/07/21 01:56:56  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------------------
 * skye_util.c:
 * Revision 1.2.8.4  2014/07/11 11:00:43  steja
 * Update IP configuration
 *
 * Revision 1.2.8.3  2014/06/27 13:31:22  steja
 * Add init for iptable configuration on CPU0
 *
 * Revision 1.2.8.2  2014/05/20 17:56:26  palin2
 * 1. Move voltage margin utility to "pwr_seq_diag.c".
 * 2. Add function "dump_readback_reg" as common function to handle register(s) dump.
 *
 * Revision 1.2.8.1  2014/05/09 03:09:37  palin2
 * Add device nodes for USB.
 *
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.7  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.4.6  2014/01/27 16:52:47  iachang
 * Support SPI ROM read/write Test
 *
 * Revision 1.1.4.5  2013/11/22 09:16:52  iachang
 * Support Shrinkray SPIROM utility.
 *
 * Revision 1.1.4.4  2013/11/20 00:28:18  iachang
 * Initialize the system environment
 *
 * Revision 1.1.4.3  2013/10/10 00:36:22  steja
 * 1. Add TLK Utility PLL and Polarity TX RX switch
 * 2. Code update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:10  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.3  2013/09/05 08:07:44  steja
 * Support Set CPU Frequency margin utilities
 *
 * Revision 1.1.2.2  2013/08/19 07:11:52  palin2
 * Add Voltage Margin utility.
 *
 * Revision 1.1.2.1  2013/08/15 11:30:33  steja
 * Add code command and respond ( Host <->GE <-> TILE CPU#0) for G2 (PPC & MIPS) platform
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 */

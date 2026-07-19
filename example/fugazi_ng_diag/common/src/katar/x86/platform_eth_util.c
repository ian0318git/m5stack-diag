/* $Id: platform_eth_util.c,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_eth_util.c,v $
 *------------------------------------------------------------------
 * 
 * Filename   : 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <features.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h> /*struct ethtool */
#include <linux/sockios.h> /* SIOCETHTOOL */
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "endians.h"
#include "byteswap.h"
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "menu.h"
#include "queryflags.h"
#include "platform_fpga.h"
#include "mb_tests.h"
#include "ethernet.h"
#include "platform_eth.h"
#include "AQ_API.h"

//#define MB_DEBUG_LOG

#define ETH_DRIVER_POLL_TIMEOUT     (20000) /* 20 secs */

/* Port List */
int eth_ge_phy_list[] = {ETH_GE_PORT0, ETH_GE_PORT1};
int eth_ge2p5_phy_list[] = {ETH_2P5GE_PORT0, ETH_2P5GE_PORT1, ETH_2P5GE_PORT2, ETH_2P5GE_PORT3};
int eth_ge10_phy_list[] = {ETH_10GE_PORT0, ETH_10GE_PORT1};

int eth_ge_phy_nic_list[] = {5, 6};

/* Port Speed List */
int eth_ge_phy_speed_list[] = {SPD_1000MBPS, SPD_100MBPS, SPD_10MBPS};
int eth_ge2p5_phy_speed_list[] = {SPD_2500MBPS, SPD_1000MBPS};
int eth_ge10_phy_speed_list[] = {SPD_10000MBPS, SPD_5000MBPS, SPD_2500MBPS, SPD_1000MBPS, SPD_100MBPS};
int eth_sfp_phy_speed_list[] = {SPD_1000MBPS};
int eth_sfp_plus_phy_speed_list[] = {SPD_10000MBPS};

char eth_rj45_port_name[ETH_PORT_MAX][20] = {"I211-1","I211-2","AQC107-1","AQC107-2","AQR412C-1","AQR412C-2","AQR412C-3","AQR412C-4"};
char eth_sfp_port_name[ETH_PORT_MAX][20] =  {"I211-1","I211-2","AQC100-1","AQC100-2","AQR412C-1","AQR412C-2","AQR412C-3","AQR412C-4"};

char eth_rj45_fw_port_name[ETH_PORT_MAX][20] = {"I211-1","I211-2","AQC107-1","AQC107-2","ETH4_LEK","ETH5_LEK","ETH6_LEK","ETH7_LEK"};
char eth_sfp_fw_port_name[ETH_PORT_MAX][20] =  {"I211-1","I211-2","AQC100-1","AQC100-2","ETH4_LEK","ETH5_LEK","ETH6_LEK","ETH7_LEK"};

#define ETH_GE_PORT_COUNT   	(sizeof(eth_ge_phy_list) / sizeof(int))
#define ETH_GE2P5_PORT_COUNT   	(sizeof(eth_ge2p5_phy_list) / sizeof(int))
#define ETH_GE10_PORT_COUNT   	(sizeof(eth_ge10_phy_list) / sizeof(int))

#define ETH_GE_SPEED_COUNT      (sizeof(eth_ge_phy_speed_list) / sizeof(int))
#define ETH_GE2P5_SPEED_COUNT   (sizeof(eth_ge2p5_phy_speed_list) / sizeof(int))
#define ETH_GE10_SPEED_COUNT    (sizeof(eth_ge10_phy_speed_list) / sizeof(int))
#define ETH_SFP_SPEED_COUNT     (sizeof(eth_sfp_phy_speed_list) / sizeof(int))
#define ETH_SFP_PLUS_SPEED_COUNT     (sizeof(eth_sfp_plus_phy_speed_list) / sizeof(int))

extern unsigned long diagflag_xram;
#define D_EXT_LOOPBACK  0x80

extern void show_eth_counter (char *type, int port);
extern int ge_phy_speed_test(int eth_num, int port2, int speed);
extern int katar_2p5_phy_cross_test(void);
extern unsigned int AQ_reg_read(unsigned int port, unsigned int reg_mmd, unsigned int reg_num);
extern void AQ_reg_write(unsigned int port, unsigned int reg_mmd, unsigned int reg_num, unsigned int reg_val);
extern int AQ_reg_test(void);
extern int is_SFP_plus_module(int port_num);

static int katar_cross_port_util (int dummy);
static int katar_internal_lpbk_util (int dummy);
static int katar_eth_fw_update_util (int dummy);
static int katar_eth_reg_util (int bWrite);
static int aqr_firmware_upgrade_util(int dummy);
static int katar_internal_reg_read(int dummy);
static int katar_internal_reg_write(int dummy);
static void katar_AQR_firmware_version(void);
static int run_intel_internal_lpbk_test(int lpbk_num,int port_num);
static int run_AQC_internal_lpbk_test(int lpbk_num,int port_num);
static int run_AQR_internal_lpbk_test(int lpbk_num,int port_num);

static struct mitem eth_items[] = {
    {"PHY cross-port test util",      0,0, (type_t(*)())katar_cross_port_util,
                             (type_t *)&one, 0, (type_t(*)())0,       0},
    {"PHY internal loopback util",      0,0, (type_t(*)())katar_internal_lpbk_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"PHY firmware upgrade",      0,0, (type_t(*)())katar_eth_fw_update_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"PHY register read",      0,0, (type_t(*)())katar_eth_reg_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"PHY register write",      0,0, (type_t(*)())katar_eth_reg_util,
                             (type_t *)&one, 0, (type_t(*)())0,       0},
};

static struct menuinfo eth_menu = {
    "Ethernet utility Menu",
    0,
    0,
    0,
    sizeof(eth_items)/sizeof(struct mitem),
    eth_items,
};
struct menuinfo *eth_menup = &eth_menu;

static uint8 bIsMB_test = FALSE;

/*------------------------------------------------------------------
 *
 * Function: wait_iface_link_stats
 *           Wait and check until link status is down/up
 *
 * Input:  iface : Interface name
 *         exp_val : IFSTATUS_UP for link up , IFSTATUS_DOWN for link down
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int wait_iface_link_stats (int port_num, int exp_val)
{
    int timeout = ETH_DRIVER_POLL_TIMEOUT;
    struct ifreq ifr;
    int sock;
    struct ethtool_value ecmd;
	char iface[IFNAMSIZ];

	if (diagflag_xram & D_DEBUG_OPTIONS)
		printf("wait_iface_link_stats(%d) Start\n",port_num);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "%s: Failed to create ioctl socket\n", __FUNCTION__);
        return (FAILED);
    }
	sprintf(iface, "eth%d",port_num);

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);

    do {
        ecmd.cmd = ETHTOOL_GLINK;
        ifr.ifr_data = (caddr_t)&ecmd;

        if (ioctl(sock, SIOCETHTOOL, &ifr) == -1) {
            close(sock);
            cterr('f', 0, "%s: Run Eth Tool fails", __FUNCTION__);
            return (FAILED);
        }

        if (ecmd.data == exp_val) {
			if (diagflag_xram & D_DEBUG_OPTIONS)
		        printf("wait_iface_link_stats(%d) End\n",port_num);
            close(sock);
            return (PASSED);
        }
        msleep(1);
    } while (timeout--);

    close(sock);

    if (diagflag_xram & D_DEBUG_OPTIONS)
        printf("wait_iface_link_stats(%d) Fail\n",port_num);
    return (FAILED);
}

int get_link_speed(int port_num)
{
	int sock;
    struct ifreq ifr;
    struct ethtool_cmd edata;
	char iface[IFNAMSIZ];
	int speed=0;
    int rc;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "%s: Failed to create ioctl socket\n", __FUNCTION__);
        return speed;
    }
	sprintf(iface, "eth%d",port_num);

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t)&edata;

    edata.cmd = ETHTOOL_GSET;

    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    if (rc < 0) {
		printf("ioctl Fail\n");
		return speed;
    }
	speed = ethtool_cmd_speed(&edata);

	close(sock);
	return speed;
}

void initial_eth_port(int port_num)
{
	char cmd[1024];

	sprintf(cmd,"ifconfig %s%d up", SEL_PORT_ETH, port_num);

	if (diagflag_xram & D_DEBUG_OPTIONS)
		printf("%s run %s\n",__FUNCTION__,cmd);
	system(cmd);
}

void disable_eth_port(int port_num)
{
    char cmd[1024];

    sprintf(cmd,"ifconfig %s%d down", SEL_PORT_ETH, port_num);

    if (diagflag_xram & D_DEBUG_OPTIONS)
        printf("%s run %s\n",__FUNCTION__,cmd);
    system(cmd);
}

void set_eth_sutoneg(int port_num,int bEnable)
{
	char cmd[1024];

	if(bEnable == TRUE)
	{
	    sprintf(cmd,"ethtool -s %s%d autoneg on > /dev/null 2>&1", SEL_PORT_ETH, port_num);
		if (diagflag_xram & D_DEBUG_OPTIONS)
		    printf("%s run %s\n",__FUNCTION__,cmd);
		system(cmd);
		// reset advertise for I211
		if(port_num ==0 || port_num==1)
		{
			sprintf(cmd,"ethtool -s %s%d advertise 0x3F > /dev/null 2>&1", SEL_PORT_ETH, port_num);
			if (diagflag_xram & D_DEBUG_OPTIONS)
	            printf("%s run %s\n",__FUNCTION__,cmd);
    	    system(cmd);
		}
	}
	else
	{
		sprintf(cmd,"ethtool -s %s%d autoneg on > /dev/null 2>&1", SEL_PORT_ETH, port_num);
		if (diagflag_xram & D_DEBUG_OPTIONS)
		    printf("%s run %s\n",__FUNCTION__,cmd);
        system(cmd);
		sleep(1);
	    sprintf(cmd,"ethtool -s %s%d autoneg off > /dev/null 2>&1", SEL_PORT_ETH, port_num);
		if (diagflag_xram & D_DEBUG_OPTIONS)
		    printf("%s run %s\n",__FUNCTION__,cmd);
		system(cmd);
	}
	sleep(1);
}

int set_eth_speed(int port_num, int speed, int retry)
{
	char cmd[1024];
	int cur_speed=-1;
	int count=0;	

	//I211 need to set advertise
	if(port_num==0 || port_num==1)
	{
		int capability=0;
		switch(speed)
		{
			case 10:
				capability=ADVERTISED_10baseT_Full;
			break;
			case 100:
                capability=ADVERTISED_100baseT_Full;
            break;
			case 1000:
                capability=ADVERTISED_1000baseT_Full;
            break;
			default:
				return -1;
			break;
		}
		sprintf(cmd,"ethtool -s %s%d advertise 0x%x > /dev/null 2>&1", SEL_PORT_ETH, port_num, capability);
	}
	else
		sprintf(cmd,"ethtool -s %s%d speed %d > /dev/null 2>&1", SEL_PORT_ETH, port_num, speed);

	if(retry == FALSE)
	{
		system(cmd);
		sleep(10);
		return -1;
	}
	do
	{
		if(count>5)
        {
           	printf("set_eth_speed port %d to speed %d fail(%d)\n",port_num,speed,cur_speed);
            return (-1);
        }
		system(cmd);
		sleep(5);
        wait_iface_link_stats(port_num,IFSTATUS_UP);
		cur_speed = get_link_speed(port_num);
		if (diagflag_xram & D_DEBUG_OPTIONS)
	        printf("%s run %s(%d)\n",__FUNCTION__,cmd,cur_speed);
		count++;
	}while(cur_speed != speed);
	return cur_speed;
}

void print_eth_fw_ver(void)
{
	char (*port_name)[20];
	char cmd[1024];
	int port_num = 0;

	switch(katar_get_plat_sku())
    {
		case KATAR_RJ45_SKU:
			port_name = eth_rj45_fw_port_name;
			break;
		case KATAR_SFP_SKU:
		case KATAR_SFP1_SKU:
			port_name = eth_sfp_fw_port_name;
			break;
	}
	for(port_num=0;port_num<ETH_PORT_MAX;port_num++)
	{
		sprintf(cmd,"ethtool -i %s%d |grep firmware", SEL_PORT_ETH, port_num);
		printf("%s ",port_name[port_num]);
		fflush(stdout);
		system(cmd);
	}
        katar_AQR_firmware_version();
}

int katar_phy_pkt_test(int txport, int rxport, int test_speed)
{
	int rc = PASSED;
	int cur_speed = 0;

	if(txport != rxport)
	{
		set_eth_speed(rxport,test_speed,FALSE);
		cur_speed = set_eth_speed(txport,test_speed,TRUE);

		if(cur_speed != test_speed)
		{
			cterr('f', 0, "set port %d speed %d failed.\n",txport,test_speed);
			return FAILED;
		}

		sleep(5);
		printf("Cross-port test eth%d -> eth%d at %d M\n",txport, rxport, cur_speed);
		rc = ge_phy_speed_test(txport, rxport, test_speed);

		if(rc == PASSED)
		{
			sleep(5);
			cur_speed = get_link_speed(rxport);
			printf("Cross-port test eth%d -> eth%d at %d M\n",rxport, txport, cur_speed);
			rc = ge_phy_speed_test(rxport, txport, test_speed);
		}
	}else
	{
            if ( test_speed != 0 )
            {
                cur_speed = set_eth_speed(txport,test_speed,TRUE);

                if(cur_speed != test_speed)
                    return FAILED;
            
                printf("Internal lpbk test for eth%d at %d M\n",txport,cur_speed);
            }
	    rc = ge_phy_speed_test(txport, txport, test_speed);
	}
	return rc;
}

int katar_mb_phy_cross_port_test (int lpbk_num)
{
	int rc = PASSED;
	int i,j;
    char *tname[] = {"GE phy ext. lpbk", "10G phy ext. lpbk", "SFP+ ext. lpbk", "SFP ext. lpbk", "2.5G phy ext. lpbk"};
	int port_count=0,speed_count=0;
	int txport,rxport,test_speed;
	int *eth_port_list;
	int *eth_speed_list;

	switch(lpbk_num)
	{
		case GE_PHY_EXT_LPBK:
			port_count = ETH_GE_PORT_COUNT;
			speed_count = ETH_GE_SPEED_COUNT;
			eth_port_list = eth_ge_phy_list;
			eth_speed_list = eth_ge_phy_speed_list;
		    testname("%s test", tname[0]);
			printf("Start %s \n", tname[0]);
			break;

		case GE10_PHY_EXT_LPBK:
			port_count = ETH_GE10_PORT_COUNT;
			speed_count = ETH_GE10_SPEED_COUNT;
			eth_port_list = eth_ge10_phy_list;
			eth_speed_list = eth_ge10_phy_speed_list;
		    testname("%s test", tname[1]);
			printf("Start %s \n", tname[1]);
			break;

		case GE10_PHY_SFP_EXT_LPBK:
			port_count = ETH_GE10_PORT_COUNT;
			eth_port_list = eth_ge10_phy_list;
			if(is_SFP_plus_module(0))
			{
				speed_count = ETH_SFP_PLUS_SPEED_COUNT;
				eth_speed_list = eth_sfp_plus_phy_speed_list;
				testname("%s test", tname[2]);
	            printf("Start %s \n", tname[2]);
			}else
			{
				speed_count = ETH_SFP_SPEED_COUNT;
				eth_speed_list = eth_sfp_phy_speed_list;
				testname("%s test", tname[3]);
	            printf("Start %s \n", tname[3]);
			}
			break;

		case GE2P5_PHY_EXT_LPBK:
            port_count = ETH_GE2P5_PORT_COUNT;
            speed_count = ETH_GE2P5_SPEED_COUNT;
            eth_port_list = eth_ge2p5_phy_list;
            eth_speed_list = eth_ge2p5_phy_speed_list;
            testname("%s test", tname[4]);
			printf("Start %s \n", tname[4]);
            break;

		default:
			printf("Wrong parameter , return\n");
			return FAILED;
			break;
	}

	/* Temporary disable showing kernel messages because unbind and bind XHCI controller to XHCI driver */
	system(SUPPRESS_MESG);

	for(i=0;i<port_count;i+=2)
	{	
		txport = eth_port_list[i];
        rxport = eth_port_list[i+1];

		initial_eth_port(txport);
		initial_eth_port(rxport);
        if(wait_iface_link_stats(txport,IFSTATUS_UP)!=PASSED)
        {
			cterr('f', 0, "check port %d link failed. Is cross-port loopback connector installed?\n",txport);
            rc=FAILED;
			goto phy_link_fail_exit;
        }
        if(wait_iface_link_stats(rxport,IFSTATUS_UP)!=PASSED)
        {
            cterr('f', 0, "check port %d link failed. Is cross-port loopback connector installed?\n",rxport);
            rc=FAILED;
			goto phy_link_fail_exit;
        }

#ifdef MB_DEBUG_LOG
		printf("Status before test:\n");
		show_eth_counter(SEL_PORT_ETH, txport);
        show_eth_counter(SEL_PORT_ETH, rxport);
#endif

		if(lpbk_num != GE_PHY_EXT_LPBK)
		{
			set_eth_sutoneg(txport,FALSE);
			set_eth_sutoneg(rxport,FALSE);
		}

		for(j=0;j<speed_count;j++)
		{
			test_speed = eth_speed_list[j];
			rc = katar_phy_pkt_test(txport, rxport, test_speed);
			if(rc != PASSED)
			{
				set_eth_sutoneg(txport,TRUE);
				set_eth_sutoneg(rxport,TRUE);
				goto phy_cross_test_exit;
			}
		}
		set_eth_sutoneg(txport,TRUE);
		set_eth_sutoneg(rxport,TRUE);
	}
	
phy_cross_test_exit:
	wait_iface_link_stats(txport,IFSTATUS_UP);
	wait_iface_link_stats(rxport,IFSTATUS_UP);
phy_link_fail_exit:

#ifdef MB_DEBUG_LOG
	printf("Status after test:\n");
    show_eth_counter(SEL_PORT_ETH, txport);
    show_eth_counter(SEL_PORT_ETH, rxport);
#endif

	disable_eth_port(txport);
	disable_eth_port(rxport);
	sleep(3);
	system(OPEN_MESG);
	if(rc==PASSED)
		prpass(testpass, NULL);
    return rc;
}

int
katar_mb_ge_phy_cross_port_test (int dummy)
{
	int rc = FAILED;

	bIsMB_test = TRUE;
	if (check_menu_flag(D_EXT_LOOPBACK))
		rc = run_intel_internal_lpbk_test(GE_PHY_INT_LPBK,ETH_GE_PORT_COUNT);
	else
		rc = katar_mb_phy_cross_port_test(GE_PHY_EXT_LPBK);

	bIsMB_test = FALSE;
	return rc;
}

int
katar_mb_ge10_phy_cross_port_test (int dummy)
{
	int rc = FAILED;

    bIsMB_test = TRUE;
	if (check_menu_flag(D_EXT_LOOPBACK))
	{
		if(katar_get_plat_sku()== KATAR_RJ45_SKU)
			rc = run_AQC_internal_lpbk_test(GE10_PHY_INT_LPBK,ETH_GE10_PORT_COUNT);
		else
			rc = run_AQC_internal_lpbk_test(GE10_PHY_SFP_INT_LPBK,ETH_GE10_PORT_COUNT);
	}
	else
	{
		if(katar_get_plat_sku()== KATAR_RJ45_SKU)
			rc = katar_mb_phy_cross_port_test(GE10_PHY_EXT_LPBK);
		else
			rc = katar_mb_phy_cross_port_test(GE10_PHY_SFP_EXT_LPBK);
	}

	bIsMB_test = FALSE;
	return rc;
}

int
katar_mb_ge2p5_phy_cross_port_test (int dummy)
{
	int rc = FAILED;

    bIsMB_test = TRUE;

	if (check_menu_flag(D_EXT_LOOPBACK))
		rc = run_AQR_internal_lpbk_test(GE2P5_PHY_INT_LPBK,ETH_GE2P5_PORT_COUNT);
	else
	    rc = katar_2p5_phy_cross_test();

	bIsMB_test = FALSE;
    return rc;
}

static  int
katar_eth_fw_update_util (int dummy)
{
	int choice=0;

	printf("\n Firmware upgrade utility:\n");

    printf("0) quit\n");
    printf("1) GE PHY firmware upgrade\n");
    printf("2) 10G PHY firmware upgrade\n");
    printf("3) 2.5G PHY firmware upgrade\n");
    choice = gethex_answer("Enter selection:", 0, 0, 3);

	switch(choice)
	{
		default:
			printf("PCie base Ethernet adapter, use vendor tool from kernel prompt\n");
			break;
		case 0:
			printf("Exit by user\n");	
			break;
		case 3:
			aqr_firmware_upgrade_util(0);
			break;
	}
	return (PASSED);
}

static  int
katar_eth_reg_util (int bWrite)
{
    int choice=0;
	char str[256];

	if(bWrite)
		sprintf(str,"register write");
	else
		sprintf(str,"register read");

	printf("\n %s utility:\n",str);

    printf("0) quit\n");
    printf("1) GE PHY %s\n",str);
    printf("2) 10G PHY %s\n",str);
    printf("3) 2.5G PHY %s\n",str);
    choice = gethex_answer("Enter selection:", 0, 0, 3);

    switch(choice)
    {
        default:
            printf("PCie base Ethernet adapter, use vendor tool from kernel prompt\n");
            break;
        case 0:
            printf("Exit by user\n");
            break;
        case 3:
			if(bWrite)
				katar_internal_reg_write(0);
			else
				katar_internal_reg_read(0);
            break;
    }
    return (PASSED);
}

static int 
katar_cross_port_util (int dummy)
{
	int i,speed_choice=0,choice=0;
    boolean exit_flag = FALSE;
	int port_count=0,speed_count=0;
	int txport,rxport,test_speed;
	int *eth_port_list;
	int *eth_speed_list;

    while (!exit_flag) {
        printf("\n Cross-port test utility:\n");

	    printf("0) quit\n");
	    printf("1) GE PHY cross-port test\n");
	    printf("2) 10G PHY cross-port test\n");
	    printf("3) 2.5G PHY cross-port test\n");
		choice = gethex_answer("Enter selection:", 0, 0, 3);

		switch(choice)
		{
			case 1:
				port_count = ETH_GE_PORT_COUNT;
				speed_count = ETH_GE_SPEED_COUNT;
				eth_port_list = eth_ge_phy_list;
				eth_speed_list = eth_ge_phy_speed_list;
				break;

			case 2:
				if(katar_get_plat_sku()== KATAR_RJ45_SKU)
				{
					port_count = ETH_GE10_PORT_COUNT;
					speed_count = ETH_GE10_SPEED_COUNT;
					eth_port_list = eth_ge10_phy_list;
					eth_speed_list = eth_ge10_phy_speed_list;
				}else
				{
					port_count = ETH_GE10_PORT_COUNT;
					eth_port_list = eth_ge10_phy_list;
					if(is_SFP_plus_module(0))
		            {
		                speed_count = ETH_SFP_PLUS_SPEED_COUNT;
        		        eth_speed_list = eth_sfp_plus_phy_speed_list;
		            }else
		            {
                		speed_count = ETH_SFP_SPEED_COUNT;
        		        eth_speed_list = eth_sfp_phy_speed_list;
		            }	
				}
				break;

			case 3:
	            port_count = ETH_GE2P5_PORT_COUNT;
	            speed_count = ETH_GE2P5_SPEED_COUNT;
	            eth_port_list = eth_ge2p5_phy_list;
	            eth_speed_list = eth_ge2p5_phy_speed_list;
	            break;

			default:
				printf("Quit by user selection\n");
				return FAILED;
				break;
		}
		
		if(port_count>2)
		{
			for(i=0;i<port_count;i++)
				printf("Test port %d\n",eth_port_list[i]);
			txport = gethex_answer("Select port 1:", eth_port_list[0], eth_port_list[0], eth_port_list[port_count-1]);
			do
			{
				rxport = gethex_answer("Select port 2:", txport+1, eth_port_list[0], eth_port_list[port_count-1]);
			}while(txport == rxport);
			
		}else
		{
			txport = eth_port_list[0];
			rxport = eth_port_list[1];
		}

		for(i=0;i<speed_count;i++)
			printf("%d) run cross-port test for %dM\n",i,eth_speed_list[i]);
        printf("%d) run cross-port test for all speed\n",speed_count);		
        printf("%d) exit\n",speed_count+1);

        speed_choice = gethex_answer("Enter selection:", speed_count+1, 0, speed_count+1);

		if(speed_choice == speed_count+1)
			exit_flag = TRUE;

		if(exit_flag != TRUE)
		{
			initial_eth_port(txport);
			initial_eth_port(rxport);
	        if(wait_iface_link_stats(txport,IFSTATUS_UP)!=PASSED)
    	    {
        	    printf("check port %d link fail",txport);
				return (FAILED);
	        }
    	    if(wait_iface_link_stats(rxport,IFSTATUS_UP)!=PASSED)
        	{
	            printf("check port %d link fail",rxport);
				return (FAILED);
	        }

			printf("Status before test:\n");
			show_eth_counter(SEL_PORT_ETH, txport);
            show_eth_counter(SEL_PORT_ETH, rxport);
			//Test for all speed
			if(speed_choice == speed_count)
			{
				if(choice != 1)
		        {
        		    set_eth_sutoneg(txport,FALSE);
		            set_eth_sutoneg(rxport,FALSE);
		        }
				for(i=0;i<speed_count;i++)
				{
					test_speed = eth_speed_list[i];
					katar_phy_pkt_test(txport, rxport, test_speed);
					show_eth_counter(SEL_PORT_ETH, txport);
					show_eth_counter(SEL_PORT_ETH, rxport);
				}
                set_eth_sutoneg(txport,TRUE);
                set_eth_sutoneg(rxport,TRUE);
			}else
			{
				test_speed = eth_speed_list[speed_choice];
				if(choice != 1)
				{	
					set_eth_sutoneg(txport,FALSE);
					set_eth_sutoneg(rxport,FALSE);
				}
				katar_phy_pkt_test(txport, rxport, test_speed);
				set_eth_sutoneg(txport,TRUE);
				set_eth_sutoneg(rxport,TRUE);
				show_eth_counter(SEL_PORT_ETH, txport);
				show_eth_counter(SEL_PORT_ETH, rxport);
			}
			wait_iface_link_stats(txport,IFSTATUS_UP);
		    wait_iface_link_stats(rxport,IFSTATUS_UP);

		    disable_eth_port(txport);
		    disable_eth_port(rxport);
		    sleep(3);
		}
    }
    return(PASSED);
}

int set_eth_internal_lpbk_25G(int port_num,int speed)
{
    unsigned int phy_id=0;
    
    if ( port_num == ETH_2P5GE_PORT0)
        phy_id = 2;
    else if ( port_num == ETH_2P5GE_PORT1)
        phy_id = 3;
    else if ( port_num == ETH_2P5GE_PORT2)
        phy_id = 0;
    else if ( port_num == ETH_2P5GE_PORT3)
        phy_id = 1;

    if ( speed == 0 )
        AQ_reg_write(phy_id, 4, 0xC444, 0);
    else if ( speed == 1000 )
        AQ_reg_write(phy_id, 4, 0xC444, 0x4802);
    else if ( speed == 2500 )
        AQ_reg_write(phy_id, 4, 0xC444, 0x4804);

    sleep(2);
    return(PASSED);
}

int set_eth_internal_lpbk(int lpbk_num,int port_num,int bEnable)
{
	char cmd[1024];
	
	switch(lpbk_num)
	{
		default:
		case GE_PHY_INT_LPBK:
		case GE2P5_PHY_INT_LPBK:
		case GE10_PHY_SFP_INT_LPBK:
			//Not support yet
			return(FAILED);
			break;
			
		case GE10_PHY_INT_LPBK:
			if(bEnable == TRUE)
				sprintf(cmd,"ethtool --set-priv-flags %s%d PHYInternalLoopback on", SEL_PORT_ETH, port_num);
			else
				sprintf(cmd,"ethtool --set-priv-flags %s%d PHYInternalLoopback off", SEL_PORT_ETH, port_num);

			if (diagflag_xram & D_DEBUG_OPTIONS)
			    printf("%s run %s\n",__FUNCTION__,cmd);
			system(cmd);
			sleep(5);
		break;

	}
	return(PASSED);
}

static int aqr_firmware_upgrade_util(int dummy)
{
    AQ_API_Port* port=NULL;
    uint32_t imageSizePointer;
    AQ_Retcode ret;
    unsigned char buffer[512*1024];
    uint32_t num;
    int fd;

    port = malloc(sizeof(AQ_API_Port));
    memset(port,0x0,sizeof(port));
    port->device = AQ_DEVICE_CAL;
    port->PHY_ID = 0;
    memset(buffer,0x0, sizeof(buffer));
    if((fd = open("/mnt/bin/AQR412_firmware.cld",O_RDONLY)) < 0) {
      printf("cannot open AQR412 firmware\n");
      return(1);
    } 
    if((num = read(fd,buffer,sizeof(buffer))) == 0)
       printf("num = %u, 11, 22\n",num); 
    if(num < 0) {
        printf("read error...aborting\n");
    }
    printf("num = %u, CS1=%x, CS2=%x\n",num, buffer[num-2], buffer[num-1]);
    imageSizePointer=num;
    ret = AQ_API_WriteAndVerifyFlashImage(port, &imageSizePointer, buffer);
    printf("AQ_API_WriteAndVerifyFlashImage ret = %d\n",ret);   
    return ret;
}

int aqr_register_read_write_util(void)
{
    testname("GE 2.5G PHY MDIO register test");
    if(AQ_reg_test())
    {
        cterr('f', 0, "%s(): MDC/MDIO register test failed",__FUNCTION__);
        return (FAILED);
    }
    else
    {
		prpass(testpass, NULL);
        return (PASSED);
    }
}

static void 
katar_AQR_firmware_version(void)
{
    unsigned int firmware_version=0, major_version=0, minor_version, VER_number, ID_number;
    
    firmware_version = AQ_reg_read(0, 0x1E, 0x0020);
    VER_number = AQ_reg_read(0, 0x1, 0xC41E);
    ID_number = AQ_reg_read(0, 0x1, 0xC41D);
    minor_version = firmware_version & 0x00FF;
    major_version = firmware_version >> 8;
    printf("AQR412c Firmware Major Revision Number = 0x%x\n", major_version);
    printf("AQR412c Firmware Minor Revision Number = 0x%x\n", minor_version);
    printf("AQR412c Firmware ID = %d, VER = %d\n", ID_number, VER_number);
}

static int
katar_internal_reg_read(int dummy)
{
    unsigned int reg_mmd=0;
    unsigned int reg_num=0;
    unsigned int port=0;
    unsigned int ret_val=0;

    port = gethex_answer("Enter port:", 0, 0, 0x3);
    reg_mmd = gethex_answer("Enter mmd:", 0, 0, 0xFFFF);
    reg_num = gethex_answer("Enter reg:", 0, 0, 0xFFFF);
    ret_val = AQ_reg_read(port, reg_mmd, reg_num);
    printf("port%d , %x.%x = %x\n", port, reg_mmd, reg_num, ret_val);
    return(PASSED);
}

static int
katar_internal_reg_write(int dummy)
{
    unsigned int reg_mmd=0;
    unsigned int reg_num=0;
    unsigned int port=0;
    unsigned int reg_val=0, ret_val=0;
    

    port = gethex_answer("Enter port:", 0, 0, 0x3);
    reg_mmd = gethex_answer("Enter mmd:", 0, 0, 0xFFFF);
    reg_num = gethex_answer("Enter reg:", 0, 0, 0xFFFF);
    reg_val = gethex_answer("Enter value:", 0, 0, 0xFFFF);
    AQ_reg_write(port, reg_mmd, reg_num, reg_val);
    printf("Set port%d , %x.%x = %x\n", port, reg_mmd, reg_num, reg_val);
    sleep(1);
    ret_val = AQ_reg_read(port, reg_mmd, reg_num);
    printf("Read back port%d , %x.%x = %x\n", port, reg_mmd, reg_num, ret_val);
    return(PASSED);
}

void katar_AQR_set_force_intr(uint enable)
{
	if(enable)
    {
    	AQ_reg_write(0, 0x1E, 0xD401, 0x1);
    	AQ_reg_write(0, 0x1E, 0xFF01, 0x2);
    	AQ_reg_write(0, 0x1E, 0xFF00, 0x1);
    	
    	AQ_reg_write(0, 0x1E, 0xC470, 0x80);
    }
    else
    {
    	AQ_reg_write(0, 0x1E, 0xD401, 0x0);
    	AQ_reg_write(0, 0x1E, 0xFF01, 0x0);
    	AQ_reg_write(0, 0x1E, 0xFF00, 0x0);
    	
    	AQ_reg_write(0, 0x1E, 0xC470, 0x0);
    }
}

static int exec_cmd_with_fail_check(char* cmd,char* proc_str,int bPrintdebug)
{
	char buf[1024] = "NULL";
	FILE *fp;
	int rc = PASSED;

	fp = popen(cmd,"r");
	fflush(stdout);
	while ((fgets(buf, sizeof(buf), fp))!=NULL) 
	{
		if(bPrintdebug || strstr(buf, proc_str))
		{
			printf("\r%s",buf);
			fflush(stdout);
		}
		if(strstr(buf, "Fail")|| strstr(buf, "FAIL"))
			rc = FAILED;
	}
	pclose(fp);
	return rc;
}

static int run_intel_internal_lpbk_test(int lpbk_num,int port_num)
{
	char cmd[1024];
	int i;
	int rc = FAILED;
	int printlog = TRUE;

	if(bIsMB_test && !(diagflag_xram & D_DEBUG_OPTIONS))
    {
        system(SUPPRESS_MESG);
        printlog = FALSE;
    }

	switch(lpbk_num)
	{
		case GE_PHY_INT_LPBK:
			testname("GE phy int. lpbk test");
			if(port_num == ETH_GE_PORT_COUNT)
			{
				for(i=0;i<ETH_GE_PORT_COUNT;i++)
				{
					printf("\nStart %s Internal lpbk test\n",eth_rj45_port_name[i+ETH_GE_PORT0]);
					sprintf(cmd,"celo64e /nic=%d /lb", eth_ge_phy_nic_list[i]);
    	    	    if (diagflag_xram & D_DEBUG_OPTIONS)
					{
	    	            printf("%s run %s\n",__FUNCTION__,cmd);
						printlog = TRUE;
					}
					rc = exec_cmd_with_fail_check(cmd,"Internal (Extended) Loopback Test",printlog);
					if(rc != PASSED)
                    {
                        cterr('f', 0, "GE Port %d internal loopback test Fail\n",i);
                    }
				}				
			}else
			{
				printf("\nStart %s Internal lpbk test\n",eth_rj45_port_name[port_num+ETH_GE_PORT0]);
				sprintf(cmd,"celo64e /nic=%d /lb", eth_ge_phy_nic_list[port_num]);
				if (diagflag_xram & D_DEBUG_OPTIONS)
				{
				    printf("%s run %s\n",__FUNCTION__,cmd);
					printlog = TRUE;
				}
				rc = exec_cmd_with_fail_check(cmd,"Internal (Extended) Loopback Test",printlog);
				if(rc != PASSED)
                {
                    cterr('f', 0, "GE Port %d internal loopback test Fail\n",port_num);
                }
			}
			break;
		default:
			break;
	}

	if(rc == PASSED)
        prpass(testpass, NULL);

	if(bIsMB_test && !(diagflag_xram & D_DEBUG_OPTIONS))
		system(OPEN_MESG);

	return rc;
}

static int run_AQC_internal_lpbk_test(int lpbk_num,int port_num)
{
	char cmd[1024];
	int i;
	int rc = FAILED;
	int printlog = TRUE;

	if(bIsMB_test && !(diagflag_xram & D_DEBUG_OPTIONS))
	{
		system(SUPPRESS_MESG);
		printlog = FALSE;
	}

	switch(lpbk_num)
    {
        case GE10_PHY_INT_LPBK:
			testname("10G phy int. lpbk test");
			// load aqdiag.ko module
			sprintf(cmd,"insmod /diag_utils/aqdiag/aqdiag.ko inttype=2 vectors=4");
			if (diagflag_xram & D_DEBUG_OPTIONS)
            	printf("%s run %s\n",__FUNCTION__,cmd);
			system(cmd);
			sleep(5);

            if(port_num == ETH_GE10_PORT_COUNT)
            {
                for(i=0;i<ETH_GE10_PORT_COUNT;i++)
                {
					printf("\nStart %s Internal lpbk test\n",eth_rj45_port_name[i+ETH_10GE_PORT0]);
					if (diagflag_xram & D_DEBUG_OPTIONS)
					{
	                    sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -v 2 -a /diag_utils/aqdiag/config/10G_phy_lpbk.cfg", i);
						printf("%s run %s\n",__FUNCTION__,cmd);
						printlog = TRUE;
					}else
						sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -a /diag_utils/aqdiag/config/10G_phy_lpbk.cfg", i);

					rc = exec_cmd_with_fail_check(cmd,"Starting Phy",printlog);
					if(rc != PASSED)
					{
						cterr('f', 0, "10G Port %d internal loopback test Fail\n",i);
						goto AQC_int__lpbktest_exit;
					}
                }
            }else
            {
				printf("\nStart %s Internal lpbk test\n",eth_rj45_port_name[port_num+ETH_10GE_PORT0]);
				if (diagflag_xram & D_DEBUG_OPTIONS)
				{
					sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -v 2 -a /diag_utils/aqdiag/config/10G_phy_lpbk.cfg", port_num);
					printf("%s run %s\n",__FUNCTION__,cmd);
					printlog = TRUE;
				}else
					sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -a /diag_utils/aqdiag/config/10G_phy_lpbk.cfg", port_num);

				rc = exec_cmd_with_fail_check(cmd,"Starting Phy",printlog);
                if(rc != PASSED)
				{
					cterr('f', 0, "10G Port %d internal loopback test Fail\n",port_num);
					goto AQC_int__lpbktest_exit;
				}
            }
            break;

		case GE10_PHY_SFP_INT_LPBK:
			testname("SFP int. lpbk test");
			// load aqdiag.ko module
            sprintf(cmd,"insmod /diag_utils/aqdiag/aqdiag.ko inttype=2 vectors=4");
            if (diagflag_xram & D_DEBUG_OPTIONS)
                printf("%s run %s\n",__FUNCTION__,cmd);
            system(cmd);
            sleep(5);

            if(port_num == ETH_GE10_PORT_COUNT)
            {
                for(i=0;i<ETH_GE10_PORT_COUNT;i++)
                {
					printf("\nStart %s Internal lpbk test\n",eth_sfp_port_name[i+ETH_10GE_PORT0]);
					if (diagflag_xram & D_DEBUG_OPTIONS)
					{
	                    sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -v 2 -t fast_datapath:Mac", i);
						printf("%s run %s\n",__FUNCTION__,cmd);
						printlog = TRUE;
					}else
						sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -t fast_datapath:Mac", i);

					rc = exec_cmd_with_fail_check(cmd,"Starting Mac",printlog);
                    if(rc != PASSED)
					{
						cterr('f', 0, "10G Port %d internal loopback test Fail\n",i);
                        goto AQC_int__lpbktest_exit;
					}
                }
            }else
            {
				printf("\nStart %s Internal lpbk test\n",eth_sfp_port_name[port_num+ETH_10GE_PORT0]);
                if (diagflag_xram & D_DEBUG_OPTIONS)
				{
					sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -v 2 -t fast_datapath:Mac", port_num);
                    printf("%s run %s\n",__FUNCTION__,cmd);
					printlog = TRUE;
				}else
					sprintf(cmd,"/diag_utils/aqdiag/DIAG -d %d -s -t fast_datapath:Mac", port_num);

				rc = exec_cmd_with_fail_check(cmd,"Starting Mac",printlog);
                if(rc != PASSED)
				{
					cterr('f', 0, "10G Port %d internal loopback test Fail\n",port_num);
                    goto AQC_int__lpbktest_exit;
				}
            }
			break;
        default:
            return rc;
            break;
    }

AQC_int__lpbktest_exit:
    // unload aqdiag.ko module
    sprintf(cmd,"rmmod aqdiag.ko");
    if (diagflag_xram & D_DEBUG_OPTIONS)
    	printf("%s run %s\n",__FUNCTION__,cmd);
    system(cmd);
    sleep(5);

	if(bIsMB_test && !(diagflag_xram & D_DEBUG_OPTIONS))
		system(OPEN_MESG);

	if(rc == PASSED)
		prpass(testpass, NULL);

	return rc;
}

//FIXME Didn't support 1F internal loopback yet
#define ONLY_TEST_INTERNEL_LPBK_2P5
static int run_AQR_internal_lpbk_test(int lpbk_num,int port_num)
{
#ifdef ONLY_TEST_INTERNEL_LPBK_2P5
	int i;
#else
	int i,j;
#endif
    int port,test_speed;
	int failcount = 0;
	int rc = FAILED;
	
	if(lpbk_num != GE2P5_PHY_INT_LPBK)
		return rc;

	testname("2.5G phy int. lpbk test");

	if(bIsMB_test && !(diagflag_xram & D_DEBUG_OPTIONS))
        system(SUPPRESS_MESG);

    if(port_num == ETH_GE2P5_PORT_COUNT)
    {
        for(i=0;i<ETH_GE2P5_PORT_COUNT;i++)
        {
			printf("\nStart %s Internal lpbk test\n",eth_rj45_port_name[i+ETH_2P5GE_PORT0]);
            port = eth_ge2p5_phy_list[i];
            initial_eth_port(port);
            msleep(500);

		#ifndef ONLY_TEST_INTERNEL_LPBK_2P5
            for(j=0;j<ETH_GE2P5_SPEED_COUNT;j++)
		#endif
            {
			#ifdef ONLY_TEST_INTERNEL_LPBK_2P5
				test_speed = SPD_2500MBPS;	
			#else
				test_speed = eth_ge2p5_phy_speed_list[j];
			#endif
                if(set_eth_internal_lpbk_25G(port,test_speed)!=PASSED)
                {
					cterr('f', 0, "Internal lpbk not support yet !\n");
					rc = FAILED;
					goto AQR_int_lpbktest_exit;
                }
                if(wait_iface_link_stats(port,IFSTATUS_UP)!=PASSED)
                {
                    set_eth_internal_lpbk_25G(port,0);
					cterr('f', 0, "check port %d link fail\n",port);
					rc = FAILED;
                    goto AQR_int_lpbktest_exit;
                }
                printf("2.5G PHY Internal lpbk test for eth%d at %d M\n",port,test_speed);
                rc = katar_phy_pkt_test(port, port, 0);
				if(rc != PASSED)
					failcount++;

				if(!bIsMB_test)
	                show_eth_counter(SEL_PORT_ETH, port);
                set_eth_internal_lpbk_25G(port,0);
             }
        }
    }
    else
    {
		printf("\nStart %s Internal lpbk test\n",eth_rj45_port_name[port_num+ETH_2P5GE_PORT0]);
        port = eth_ge2p5_phy_list[port_num];
        initial_eth_port(port);
        msleep(500);
	#ifndef ONLY_TEST_INTERNEL_LPBK_2P5
	    for(i=0;i<ETH_GE2P5_SPEED_COUNT;i++)
	#endif
	    {
		#ifdef ONLY_TEST_INTERNEL_LPBK_2P5
            test_speed = SPD_2500MBPS;
        #else
            test_speed = eth_ge2p5_phy_speed_list[i];
        #endif
	        if(set_eth_internal_lpbk_25G(port,test_speed)!=PASSED)
	        {
				cterr('f', 0, "Internal lpbk not support yet !\n");
				rc = FAILED;
                goto AQR_int_lpbktest_exit;
	        }

	        if(wait_iface_link_stats(port,IFSTATUS_UP)!=PASSED)
	        {
	            set_eth_internal_lpbk_25G(port,0);
				cterr('f', 0, "check port %d link fail\n",port);
				rc = FAILED;
				goto AQR_int_lpbktest_exit;
	        }
	        printf("2.5G PHY Internal lpbk test for eth%d at %d M\n",port,test_speed);
	        rc = katar_phy_pkt_test(port, port, 0);
			if(rc != PASSED)
            	failcount++;
			if(!bIsMB_test)
		        show_eth_counter(SEL_PORT_ETH, port);
	        set_eth_internal_lpbk_25G(port,0);
	     }
    }

AQR_int_lpbktest_exit:
	if(bIsMB_test && !(diagflag_xram & D_DEBUG_OPTIONS))
        system(OPEN_MESG);

	if(failcount != 0)
	{
		printf("2.5G PHY Internal lpbk test got %d fail\n",failcount);
		rc = FAILED;
	}

	if(rc == PASSED)
		prpass(testpass, NULL);

	return rc;
}

static int 
katar_internal_lpbk_util (int dummy)
{
    int i,port_choice=0,choice=0;
    boolean exit_flag = FALSE;
    int port_count=0;
    int lpbk_num=0;
    int *eth_port_list;
	int rc = FAILED;

    while (!exit_flag) {
        printf("\n Internal lpbk utility:\n");

	    printf("0) quit\n");
	    printf("1) GE PHY Internal lpbk test\n");
	    printf("2) 10G PHY Internal lpbk test\n");
	    printf("3) 2.5G PHY Internal lpbk test\n");
		choice = gethex_answer("Enter selection:", 0, 0, 3);

	    switch(choice)
	    {
 	        case 1:
			    port_count = ETH_GE_PORT_COUNT;
			    eth_port_list = eth_ge_phy_list;
		    	lpbk_num = GE_PHY_INT_LPBK;
		    break;

		 	case 2:
			    if(katar_get_plat_sku()== KATAR_RJ45_SKU)
			    {
				port_count = ETH_GE10_PORT_COUNT;
				eth_port_list = eth_ge10_phy_list;
				lpbk_num = GE10_PHY_INT_LPBK;
			    }else
			    {
				port_count = ETH_GE10_PORT_COUNT;
				eth_port_list = eth_ge10_phy_list;
				lpbk_num = GE10_PHY_SFP_INT_LPBK;
			    }
		    break;

			case 3:
	            port_count = ETH_GE2P5_PORT_COUNT;
	            eth_port_list = eth_ge2p5_phy_list;
			    lpbk_num = GE2P5_PHY_INT_LPBK;
			break;

			default:
				printf("Quit by user selection\n");
				return FAILED;
			break;
		}
		
		for(i=0;i<port_count;i++)
    	   	    printf("%d) Test port %d\n",i,eth_port_list[i]);
        printf("%d) run test for all port\n",port_count);
		port_choice = gethex_answer("Enter selection:", port_count, 0, port_count);

		switch(choice)
   	 	{
	    	case 1:
				rc = run_intel_internal_lpbk_test(lpbk_num,port_choice);
				if(rc == PASSED)
					printf("Test result : PASSED\n");
				else
					printf("Test result : FAILED\n");
				return rc;
			break;

		    case 2:
				rc =run_AQC_internal_lpbk_test(lpbk_num,port_choice);
				if(rc == PASSED)
    	            printf("Test result : PASSED\n");
	            else
    	            printf("Test result : FAILED\n");
        	    return rc;
			break;

			case 3:
				rc = run_AQR_internal_lpbk_test(lpbk_num,port_choice);
				if(rc == PASSED)
    	            printf("Test result : PASSED\n");
        	    else
	                printf("Test result : FAILED\n");
    	        return rc;

       		default:
				exit_flag = TRUE;
			break;
		}
	}
    return rc;
}
/*
 *------------------------------------------------------------------
 * $Log: platform_eth_util.c,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.13  2019/06/13 06:47:32  benlu
 * Add AQR412c FW ID and VER number in system information display
 *
 * Revision 1.1.2.12  2019/05/15 10:09:27  mikech2
 * Fix typo & missing error for set speed fail
 *
 * Revision 1.1.2.11  2019/04/26 02:55:17  mikech2
 * Remove 1G speed test for SFP+ module
 *
 * Revision 1.1.2.10  2019/04/26 01:17:34  mikech2
 * clean up Makefile
 *
 * Revision 1.1.2.9  2019/04/22 06:58:51  mikech2
 * Fix 10G internal lpbk print issue
 *
 * Revision 1.1.2.8  2019/03/26 06:28:11  mikech2
 * Add SFP/SFP+ module check in AQC100 cross-port test
 *
 * Revision 1.1.2.7  2019/03/25 08:07:22  mikech2
 * Add 1G speed test for AQC100
 *
 * Revision 1.1.2.6  2019/03/05 07:29:37  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.5  2019/02/26 03:51:38  mikech2
 * Add internal loopback support for mb test
 *
 * Revision 1.1.2.4  2019/02/19 02:23:00  mikech2
 * Modify I211 cross-port test setting
 *
 * Revision 1.1.2.3  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/02/01 08:25:25  benlu
 * add 412c firmware version
 *
 * Revision 1.1.2.1  2019/01/29 01:54:20  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.13  2019/01/17 07:14:19  mikech2
 * Modify according to Kwok's review comments
 *
 * Revision 1.1.2.12  2019/01/08 05:50:10  mikech2
 * chnage cross-port lpbk test order and add debug message
 *
 * Revision 1.1.2.11  2018/12/27 03:48:26  mikech2
 * Fix cross-port test issue
 *
 * Revision 1.1.2.10  2018/12/04 12:46:41  benlu
 * Add AQR412/413 PHY internal lpbk test
 *
 * Revision 1.1.2.9  2018/12/04 08:39:09  mikech2
 * Add AQC100/107 internal lpbk test
 *
 * Revision 1.1.2.8  2018/11/30 06:20:05  mikech2
 * Fix cross-port lpbk test link down issue
 *
 * Revision 1.1.2.7  2018/11/28 01:35:42  benlu
 * AQR412c config restore after corss test, link down retry, modify message
 *
 * Revision 1.1.2.6  2018/11/14 08:59:50  benlu
 * Move 2.5G PHY register test from util to mbtest
 *
 * Revision 1.1.2.5  2018/11/12 06:20:44  mikech2
 * Fix internal loopback link down issue
 *
 * Revision 1.1.2.4  2018/11/09 15:08:20  benlu
 * add AQR PHY interrupt auto test
 *
 * Revision 1.1.2.3  2018/10/30 07:19:43  mikech2
 * Change PHY fw name in system info
 *
 * Revision 1.1.2.2  2018/10/26 09:22:09  benlu
 * Modify ethernet utility wording
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.15  2018/10/19 05:59:16  benlu
 * AQR412c reg rd/wr utility and led control function
 *
 * Revision 1.1.2.14  2018/10/18 06:38:49  mikech2
 * Add error count in GE/10G PHY mb test
 *
 * Revision 1.1.2.13  2018/10/16 08:49:14  benlu
 * Add AQR412c register test and firmware upgrade
 *
 * Revision 1.1.2.12  2018/10/12 07:47:18  mikech2
 * Add I211 internal lpbk test
 *
 * Revision 1.1.2.11  2018/10/12 00:59:22  mikech2
 * Fix ethtool_cmd_speed link error issue
 *
 * Revision 1.1.2.10  2018/10/09 03:51:04  mikech2
 * Fix GE & 10G phy mb test issue
 *
 * Revision 1.1.2.9  2018/10/08 03:36:17  mikech2
 * Modify pcie scan for different AQC100 FW
 *
 * Revision 1.1.2.8  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.7  2018/09/21 08:52:12  mikech2
 * Add cross-port & internal lpbk test util
 *
 * Revision 1.1.2.6  2018/09/04 13:34:15  benlu
 * add aqc107 internal loopback test
 *
 * Revision 1.1.2.5  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.4  2018/07/10 00:47:05  mikech2
 * Add wait_iface_link_stats to check link status
 *
 * Revision 1.1.2.3  2018/07/05 02:25:59  mikech2
 * fix false positive for GE phy lpbk test
 *
 * Revision 1.1.2.2  2018/07/04 03:28:47  mikech2
 * Add GE phy mb test function
 *
 * Revision 1.1.2.1  2018/07/02 09:12:50  mikech2
 * Add Ethernet utility Menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

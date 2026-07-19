/* $Id: cavecreek_sgmii.c,v 1.13 2020/01/09 01:02:19 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavecreek_sgmii.c,v $
 *------------------------------------------------------------------
 *
 * cavecreek_sgmii.c - Intel CaveCreek PCH SGMII port tests
 *
 * Dec 2011, Paul Tong
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>  /* getpid */
#include<strings.h>  /* for bzero*/
#include<string.h>
#include<errno.h>
#include <sys/types.h> /* getpid */
#include<sys/socket.h>
#include<features.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<linux/ethtool.h> /*struct ethtool */
#include<linux/sockios.h> /* SIOCETHTOOL */
#include<pthread.h>

#include "types.h"
#include "proto.h"
#include "common.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "bcm_gesw_defs.h"
#include "plat_defs.h"
#include "eth_pkt_utils.h"
#include "platform_eth_pkt_txrx.h"
#include "dash_fpga.h"
#include "nvmonvars.h"

/* for using getifaddrs */
#include<ifaddrs.h>

#undef DEBUG
//#define DEBUG 1

extern int exec_bcm_shell_cmd (int unit, char *cmd, int print_cmd);

/*----------------------------------------------------------------------------*/
/*----   Firmware download example  ------------------------------------------*/

#define PKT_PAYLOAD_SIZE   60
static unsigned char firmware_rx[ETH_PKT_BUF_LEN];
static unsigned char firmware_tx[] = {
     /* For the simplicity of the example, please keep the size of
      * this simple firmware array in multiple of PKT_PAYLOAD_SIZE
      */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
      0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 
      0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  
 
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
      0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 
      0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,  
      0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 
      0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 

      0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,  
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,  
      0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 
      0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 
};
static int firmware_pkt_num = (sizeof(firmware_tx) / PKT_PAYLOAD_SIZE);

static volatile int rx_ready = 0;

/*
 * Function: firmware_download
 * Down load the example firmware code from the Cavecreek sgmii port
 * by useing the mac_da and mac_sa in the frame of packets.
 *
 * Input:
 * sgmii_port - The cavecreek sgmii port number
 * mac_da - MAC DA used in the packet
 * mac_sa - MAC SA used in the packet
 *
 * Return: PASS/FAIL
 */
int firmware_download (int sgmii_port, mac_addr_t mac_da, mac_addr_t mac_sa)
{
    unsigned short pkt_type = 0x0800;

    int ii, f_idx;
    int payload_size = PKT_PAYLOAD_SIZE;
    int tx_skt;
    eth_tx_pkt_t txpkt_s;
    int rv;
    char if_name[10];

    sprintf(if_name, "eth%d", sgmii_port);
    if (setup_eth_dev(if_name, &tx_skt) == FAIL) {
        return(FAIL);
    }

    memcpy(txpkt_s.dest_addr, mac_da, 6);
    memcpy(txpkt_s.src_addr, mac_sa, 6);
    txpkt_s.pkt_type = pkt_type;
    txpkt_s.tx_status = 0; 
    txpkt_s.socket = tx_skt;

    f_idx = 0;
    for (ii=0; ii < firmware_pkt_num; ii++) {
        txpkt_s.bufr_st_addr = &firmware_tx[f_idx];
	txpkt_s.payload_size = payload_size;
	f_idx += payload_size;

	rv = eth_pkt_tx(&txpkt_s);
	if (rv != ETH_PKT_TX_OK) {
	    printf("Error: firmware download TX failed\n");
	    break;
	}
	msleep(1); /* Give some time for the rx to take the pkt */
    }

    cleanup_eth_dev(if_name, tx_skt);

    if (rv != ETH_PKT_TX_OK) {
      return(FAIL);
    }

    return(PASS);
}

/*
 * Function: chk_macaddr
 * Compare if 2 mac addresses matches
 *
 * Input:
 * macaddr1 and macaddr2 - 2 mac addresses to be compared
 *
 * Return: 0 when matched
 */
int chk_macaddr(uchar *macaddr1, uchar *macaddr2)
{
    return(memcmp(macaddr1, macaddr2, 6));
}

/*
 * Function: firmware_receive
 * Receive packets at this port and store the contains into
 * the receive firmware array for comaprison later
 *
 * Input:
 * sgmii_port - The cavecreek sgmii port that receives the packet
 *
 * Return: PASS/FAIL
 */
int firmware_receive (int sgmii_port)
{
    int rx_skt;
    eth_rx_pkt_t rxpkt_s;
    unsigned char pkt[ETH_PKT_BUF_LEN];
    unsigned char *cptr, *bufp;    
    int byte_cnt, firmware_sz, rv;
    int tgt_pkt_cnt;
    mac_addr_t tgt_mac_da;
    char if_name[10];
#if DEBUG 
    int ii;
#endif 
    get_local_mac_addr(sgmii_port, &tgt_mac_da);

    sprintf(if_name, "eth%d", sgmii_port);
    if (setup_eth_dev(if_name, &rx_skt) == FAIL) {
        return(FAIL);
    }

    rxpkt_s.socket = rx_skt;
    rxpkt_s.wait_time = 3 * 1000000; /* in usec, 3 sec */

    rxpkt_s.bufr_st_addr = pkt;
    rxpkt_s.rx_bufr_size = ETH_PKT_BUF_LEN;
    rxpkt_s.pkt_num = 0;
    rxpkt_s.rx_chk = 1;
    /* Receive the download
     */
    bufp = firmware_rx;
    memset(bufp, 0, sizeof(firmware_rx));
    firmware_sz = 0;
    tgt_pkt_cnt = 0;
    rx_ready = 1;
    do {
        rv = eth_pkt_rx (&rxpkt_s);

	if (rv == ETH_PKT_RX_OK) {
#if DEBUG
	    printf("\n\n>>>> %s eth%d %d bytes received: !!! pkt_cnt= %d\n",
		   __FUNCTION__, sgmii_port, rxpkt_s.pkt_size,
		   rxpkt_s.pkt_num);
        
	    printf("Destination MAC address: "
		   "%02x:%02x:%02x:%02x:%02x:%02x\n",
		   pkt[0],pkt[1],pkt[2],
		   pkt[3],pkt[4],pkt[5]);
	    printf("Source MAC address: "
		   "%02x:%02x:%02x:%02x:%02x:%02x\n",
		   pkt[6],pkt[7],pkt[8],
		   pkt[9],pkt[10],pkt[11]);

	    for (ii=0; ii < rxpkt_s.pkt_size; ii++) {
	      if ((ii > 0) && (ii % 16) == 0) {
		printf("\n");
	      }
	      printf("%02x ", pkt[ii]);
	    }
	    printf("end of pkt print. pkt cnt = %d\n", rxpkt_s.pkt_num);
#endif //DEBUG

	    if (chk_macaddr(&pkt[0], (unsigned char*)tgt_mac_da) != 0) {
#if DEBUG
	      printf("\n detected non firmward download packet. Ignore.\n");
#endif
	        continue; /* not firmware download packet */
	    }
 
	    cptr = &pkt[ETH_HDR_LEN]; /* skip the mac da, sa, and type fields */
	    byte_cnt = rxpkt_s.pkt_size - ETH_HDR_LEN;
	    memcpy(bufp, cptr, byte_cnt);
	    bufp += byte_cnt;
	    firmware_sz += byte_cnt;
	    tgt_pkt_cnt++;
	}
    } while ((tgt_pkt_cnt < firmware_pkt_num) && (rv == ETH_PKT_RX_OK));

    cleanup_eth_dev(if_name, rx_skt);

    if (rxpkt_s.pkt_num == 0) {
        printf("Error: firmware receive RX failed\n");
	return(FAIL);
    }

#if DEBUG
    printf("\n>>>> Port %d Received firmware:\n", sgmii_port);
    for (ii=0; ii < firmware_sz; ii++) {
        if ((ii > 0) && (ii % 10) == 0) {
	    printf("\n");
	}
	printf("%02x ", firmware_rx[ii]);
    }
    printf("\nEnd of firmware\n");
#endif

    return(PASS);
}

/*
 * Function: fw_rx
 * Wrapper function of the firmware_receive function to be passed in
 * the pthread_create function.
 *
 * Input:
 * port - pointer to a buffer that contain the cavecreek port number.
 *
 * Return: none
 */
static void *fw_rx(void *port)
{   
    unsigned long long rc = FAILED;
    rc =  firmware_receive(* (int *)port);
    pthread_exit((void *)rc);
}

/*
 * Function: firmware_dnld_util
 * Perform firmware download operation from txport to rxport as a test
 * to check the cavecreek SGMII ports with the GESW, and also as an
 * example for NGIO to do firmware download.
 *
 * Input:
 * txport - cavecreek sgmii port which tx the packets
 * rxport - cavecreek sgmii port which rx the packets
 *
 * Return: PASS/FAIL
 */
int firmware_dnld_util(int txport, int rxport)
{
    pthread_t threads;
    int rc;
    int f_rtn = PASS;
    mac_addr_t macda, macsa;
    void  *pthr_rv = NULL;

    rx_ready =0;

    rc = pthread_create(&threads, NULL, fw_rx, (void *) &rxport);
    if (rc != 0) {
        printf("pthread_create failed \n");
        f_rtn = FAIL;
	goto fexit_1;
    }

    /* Notes;
     * The linux driver always send out a few ipv6 multicase 
     * packets when the port just start up. If the test packets
     * are sent during this time. the port will stop.
     */
    while (rx_ready == 0) {
      msleep(100);
    }
    sleep(2);

    get_local_mac_addr(rxport, &macda);
    get_local_mac_addr(txport, &macsa);
    firmware_download(txport, macda, macsa);

    pthread_join(threads, (void **)&pthr_rv);

    if ((ulong)pthr_rv == FAIL) {
        printf("Packet data receive failed\n");
	f_rtn = FAIL;
	goto fexit_1;
    }

    if (memcmp(firmware_tx, firmware_rx, sizeof(firmware_tx)) == 0) {
        printf("passed");
        f_rtn = PASS;
	goto fexit_1;
    }
    else {
        printf("Packet data TX and RX did not match.\n");
        f_rtn = FAIL;
	goto fexit_1;
    }

 fexit_1:
    return(f_rtn);
}

/*
 * Function: cavecreek_sgmii_port_test
 * This test checks the operation of the cavecreek sgmii port 1 to 3
 * with the GESW. It uses the firmware_dnld_util to perform a firmware
 * download operation from one of the Cavecreek sgmii port to another 
 * through the basic gesw L2 forwarding.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int cavecreek_sgmii_port_test(void)
{
    int rv = PASS;
    char *tname = "Cavecreek SGMII 2-port loopback at GESW";

    cavecreek_sgmii_macsa_declare();

    /* Port2 and 3 are kept in down state normally
     */
    do_ifconfig(CPU_SGMII_PORT2, "up");
    do_ifconfig(CPU_SGMII_PORT3, "up");

    testname("%s", tname);

    prpass(testpass, "SGMII-1 TX to SGMII-2, ");
    rv = firmware_dnld_util(CPU_SGMII_PORT1, CPU_SGMII_PORT2);
    if (rv == FAIL) {
        printf("\n%s port 1 to port 2 failed\n", tname);
	goto fexit;
    }

    prpass(testpass, "SGMII-2 TX to SGMII-3, ");
    rv = firmware_dnld_util(CPU_SGMII_PORT2, CPU_SGMII_PORT3);
    if (rv == FAIL) {
        printf("\n%s port 2 to port 3 failed\n", tname);
	goto fexit;
    }

    prpass(testpass, "SGMII-3 TX to SGMII-1, ");
    rv = firmware_dnld_util(CPU_SGMII_PORT3, CPU_SGMII_PORT1);
    if (rv == FAIL) {
        printf("\n%s port 3 to port 1 failed\n", tname);
	goto fexit;
    }

 fexit:

    cavecreek_sgmii_macsa_declare();
    if (rv == PASS) {
        printf("\n");
    }
    else {
        cterr('f',0,"%s failed\n", tname);
    }
    return(rv);
}

/*
 * Function:  neptune_x86_ge_port_test
 * This test checks the operation of the broadwell port 1 to 2
 * with the GESW. It uses the firmware_dnld_util to perform a firmware
 * download operation from one of the broadwell port to another 
 * through the basic gesw L2 forwarding.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int neptune_x86_ge_port_test(void)
{
    int unit = bcm_uid;
    char cmd[64];
    int rv = PASS;
    char *tname = "Broadwell GE ports 2-port loopback at GESW";

    cavecreek_sgmii_macsa_declare();

    /* Port2 is kept in down state normally
     */
    do_ifconfig(CPU_SGMII_PORT2, "up");

    testname("%s", tname);

    /* Only enable the 2 GESW ports connected to CPU for this test.
     * If other ports are enabled and in any unknown state, they will
     * interfere this test.
     */
    sprintf(cmd, "port all Enable=false;");
    exec_bcm_shell_cmd(unit, cmd, FALSE);
    sprintf(cmd, "port ge0 Enable=true;");    
    exec_bcm_shell_cmd(unit, cmd, FALSE);
    if (is_neptune() || is_vg450()) {
        sprintf(cmd, "port xe5 Enable=true;");    
    }
    else {
        sprintf(cmd, "port xe4 Enable=true;");    
    }
    exec_bcm_shell_cmd(unit, cmd, FALSE);

    prpass(testpass, "Port-1 TX to port-2, ");
    rv = firmware_dnld_util(CPU_SGMII_PORT1, CPU_SGMII_PORT2);
    if (rv == FAIL) {
        printf("\n%s port 1 to port 2 failed\n", tname);
	goto fexit;
    }

    prpass(testpass, "Port-2 TX to port-1, ");
    rv = firmware_dnld_util(CPU_SGMII_PORT2, CPU_SGMII_PORT1);
    if (rv == FAIL) {
        printf("\n%s port 2 to port 1 failed\n", tname);
	goto fexit;
    }

 fexit:
    /* Re-enable all GESW ports
     */
    sprintf(cmd, "port all Enable=true;");
    exec_bcm_shell_cmd(unit, cmd, FALSE);

    cavecreek_sgmii_macsa_declare();
    if (rv == PASS) {
        printf("\n");
    }
    else {
        cterr('f',0,"%s failed\n", tname);
    }
    return(rv);
}

/*-----------------------------------------------------------------------*/
/*----  External Loopback test for Cavecreek SGMII ports  ---------------*/

static mac_addr_t mac_da = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static mac_addr_t mac_sa = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

/*
 * Function: pkt_rx_double
 * This function is the packet receive and compare function for the
 * cavecreek sgmii same port loopback test. It is the called back
 * function passed in the pthread_create function in the test.
 *
 * Input:
 * rxpkt_s_ptr - pointer to the rx data structure
 *
 * Return: return specail code via pthread_exit
 */
static void *pkt_rx_double (eth_rx_pkt_t *rxpkt_s_ptr)
{
    unsigned char pkt[2][ETH_PKT_BUF_LEN];
    uint plen[2], pkt_len;
    unsigned char *bufp;    
    int rv, cnt;
#if DEBUG
    int ii;
#endif
    /* Do packet receive
     */
    bufp = rxpkt_s_ptr->bufr_st_addr;
    cnt = 0;
    rx_ready = 1;
    rxpkt_s_ptr->rx_chk = 1;
    do {
        rv = eth_pkt_rx (rxpkt_s_ptr);

	if (rv == ETH_PKT_RX_OK) {
	    if ((chk_macaddr(&bufp[0], (uchar *)mac_da) != 0) ||
		(chk_macaddr(&bufp[6], (uchar *)mac_sa) != 0)) {
#if DEBUG
		printf(" detected packet not sent by the test. Ignore.\n");
#endif
	        continue; /* not targeted packet */
	    }

	    memcpy(pkt[cnt], bufp, rxpkt_s_ptr->pkt_size);
	    plen[cnt] = rxpkt_s_ptr->pkt_size;

#if DEBUG
	    printf("\n\n>>>> %s %d bytes received into pkt[%d]: pkt_num= %d\n",
		   __FUNCTION__, rxpkt_s_ptr->pkt_size, cnt, rxpkt_s_ptr->pkt_num);
        
	    printf("Destination MAC address: "
		   "%02x:%02x:%02x:%02x:%02x:%02x\n",
		   pkt[cnt][0],pkt[cnt][1],pkt[cnt][2],
		   pkt[cnt][3],pkt[cnt][4],pkt[cnt][5]);
	    printf("Source MAC address: "
		   "%02x:%02x:%02x:%02x:%02x:%02x\n",
		   pkt[cnt][6],pkt[cnt][7],pkt[cnt][8],
		   pkt[cnt][9],pkt[cnt][10],pkt[cnt][11]);

	    for (ii=0; ii < rxpkt_s_ptr->pkt_size; ii++) {
	      if ((ii > 0) && (ii % 16) == 0) {
		printf("\n");
	      }
	      printf("%02x ", pkt[cnt][ii]);
	    }
	    printf("\n");
#endif //DEBUG

	    cnt++;
	}
    } while ((cnt < 2) && (rv == ETH_PKT_RX_OK));

    if ((cnt == 2) && (rv == ETH_PKT_RX_OK)) {
        pkt_len = (plen[0] > plen[1]) ? plen[0] : plen[1];
        if (memcmp(pkt[0], pkt[1], pkt_len)) {
	    printf("Error: %s Two different packet received.\n", __FUNCTION__);
	    printf("pkt[0]:\n");
	    display_pkt(pkt[0], plen[0]);
	    printf("pkt[1]:\n");
	    display_pkt(pkt[1], plen[1]);

	    pthread_exit((void *)ETH_PKT_RX_ERR);
	}
	else {
	  pthread_exit((void *)ETH_PKT_RX_OK);
	}
    }
    else {
        pthread_exit((void *)ETH_NO_PKT_RX);
    }
}

/*
 * Function: sgmii_lpbk_util
 * This fucntion performs same port packet loopback on a cavecreek
 * sgmii port.
 *
 * Input:
 * port - cavecreek sgmii port number
 * pkt_cnt - number of packets used in the test
 *
 * Return: PASS/FAIL
 */
int sgmii_lpbk_util(int port, int pkt_cnt)
{
    pthread_t threads;
    int rc, rtn_val;
    int ii;
    char if_name[10];
    void  *pthr_rv = NULL;

    /* var for rx
     */
    int rx_skt;
    eth_rx_pkt_t rxpkt_s;
    unsigned char rxpkt_buf[ETH_PKT_BUF_LEN];

    /* var for tx
     */
    unsigned short pkt_type = 0x0800;
    int payload_size = PKT_PAYLOAD_SIZE;
    int tx_skt;
    eth_tx_pkt_t txpkt_s;
    unsigned char txpkt_buf[ETH_PKT_BUF_LEN];

    /* 1. prepare the rx data structure
     */
    sprintf(if_name, "eth%d", port);
    if (setup_eth_dev(if_name, &rx_skt) == FAIL) {
        return(FAIL);
    }

    memset(rxpkt_buf, 0, ETH_PKT_BUF_LEN);
    rxpkt_s.socket = rx_skt;
    rxpkt_s.wait_time = 3 * 1000000; /* in usec, 3 sec */

    rxpkt_s.pkt_size = 0;
    rxpkt_s.bufr_st_addr = rxpkt_buf;
    rxpkt_s.rx_bufr_size = ETH_PKT_BUF_LEN;
    rxpkt_s.pkt_num = 0;

    /* 2. prepare the tx data structure
     */
    sprintf(if_name, "eth%d", port);
    if (setup_eth_dev(if_name, &tx_skt) == FAIL) {
        return(FAIL);
    }
    memset(txpkt_buf, 0, ETH_PKT_BUF_LEN);

    /* We are using raw socket and this is a loopback test, so
     * using the same mac addr for both da and sa is okay.
     */
    memcpy(txpkt_s.dest_addr, mac_da, 6);
    memcpy(txpkt_s.src_addr, mac_sa, 6);
    txpkt_s.pkt_type = pkt_type;
    txpkt_s.tx_status = 0; 
    txpkt_s.socket = tx_skt;
    txpkt_s.bufr_st_addr = &txpkt_buf[ETH_HDR_LEN]; /* point to payload */

    /* 3. Do the test
     */
    rtn_val = PASS;
    rx_ready = 0;
    
    for (ii=0; ii < pkt_cnt; ii++) {
        /* Prepare the tx packet
	 */
        txpkt_s.payload_size = payload_size;
        gen_eth_pkt(txpkt_s.dest_addr, txpkt_s.src_addr, txpkt_s.pkt_type,
		    (0x00 + ii), 1, txpkt_s.payload_size, txpkt_buf);

        /* 4. Start a rx thread for each packet
	 */
	rc = pthread_create(&threads, NULL, (void *)pkt_rx_double, (eth_rx_pkt_t *) &rxpkt_s);
	if (rc != 0) {
	    printf("%s pthread_create failed \n", __FUNCTION__);
	    return(FAIL);
	}

	while (rx_ready == 0) {
	    msleep(10);
	}

	/* Notes;
	 * For the first 3 packets, wait a little longer after the rx
	 * is socket enabled.
	 * The linux driver always send out a few ipv6 multicase 
	 * packets when the port just start up. If the test packets
	 * are sent during this time. the port will stop.
	 */
	if(ii < 3) {
	  sleep(1);
	}
	else {
	  msleep(10);
	}

	/* 5. Tx the packet
	 */
	rc = eth_pkt_tx(&txpkt_s);
	if (rc != ETH_PKT_TX_OK) {
	    printf("Error: packet tx failed\n");
	    rtn_val = FAIL;
	    break;
	}
	
	pthread_join(threads, (void **)&pthr_rv);

	/* 6. Compare the rx packet with the tx packet
	 */
	if ((ulong)pthr_rv == ETH_PKT_RX_OK) {
	    if (rxpkt_s.pkt_size == (txpkt_s.payload_size + ETH_HDR_LEN)) {
	        rc = memcmp((rxpkt_s.bufr_st_addr + ETH_HDR_LEN), txpkt_s.bufr_st_addr,
			    txpkt_s.payload_size);
		if (rc == 0) {
		  // printf("TX and RX packet match.\n");
		}
		else {
		    printf("Error: TX and RX packet mismatch.\n");
		    rtn_val = FAIL;
		    break;
		}		  

	    }
	    else {
	        printf("Error: RX packet size %d not equal to the TX packet size %d\n",
		       rxpkt_s.pkt_size, (txpkt_s.payload_size + ETH_HDR_LEN));
		rtn_val = FAIL;
		break;
	    }
	}
	else {
	    printf("Error: RX packet receive error\n");
	    rtn_val = FAIL;
	    break;
	}
    }

    if (rtn_val == FAIL) {
        char tmpcmd[128];
        printf("Found error at %dth packet test\n", ii);
        printf("-----TX packet is : -----\n");
        display_pkt(txpkt_buf, (txpkt_s.payload_size + ETH_HDR_LEN));
        printf("-----RX packet is : -----\n");
        display_pkt(rxpkt_s.bufr_st_addr, rxpkt_s.pkt_size);

        snprintf(tmpcmd, sizeof(tmpcmd), "ifconfig eth%d", port);
        system(tmpcmd);
        snprintf(tmpcmd, sizeof(tmpcmd), "ethtool eth%d", port);
        system(tmpcmd);
        snprintf(tmpcmd, sizeof(tmpcmd), "ethtool -S eth%d", port);
        system(tmpcmd);
    }

    cleanup_eth_dev(if_name, tx_skt);
    cleanup_eth_dev(if_name, rx_skt);

    return(rtn_val);
}

/*------------------------------------------------------------------
 *
 * Function: eth_is_linkup
 *
 * Description : Check eth port link up status from Linux information.
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
boolean eth_is_linkup (int port)
{
    char file_name[] = "/sys/class/net/ethxx/operstate";
    FILE *stream_p;
    char linkstate[] ="down";

    sprintf(file_name, "/sys/class/net/eth%d/operstate", port);

    stream_p = fopen(file_name, "r");
    if (stream_p == NULL) {
        cterr('f', 0," The file `/sys/class/net/eth%d/operstate' can't be opened.\n ", port);
    } else {
        fscanf(stream_p, "%s", linkstate);
        fclose(stream_p);
    }

    if (strcmp(linkstate, "up") == 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("eth%d link is %s\n", port, linkstate);
        }
	return (TRUE);
    }
    else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
             printf("eth%d link is %s\n", port, linkstate);
        }
	return (FALSE);
    }
}

/*------------------------------------------------------------------
 *
 * Function: ovld_is_linkup
 *
 * Description : Check link up status from Linux information.
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ovld_is_linkup(int port){

    int tt, timeout_count = 20, wait_time = 200;

    for (tt=0; tt < timeout_count; tt++) {
        msleep(wait_time);
	if (eth_is_linkup(port) == TRUE) {
	    return(PASS);
	}
    }
    printf("eth%d failed to link up in 4 sec\n", port);
    return(FAIL);
}

/*
 * Function: cavecreek_sgmii_ext_lpbk_test
 * This test calls the sgmii_lpbk_util function to perform same port
 * loopback on each of the cavecreek sgmii ports. The loopback path
 * is achieved by using the line loopback feature provided by the GESW.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int cavecreek_sgmii_ext_lpbk_test(void)
{
    int gesw_port;
    int rv = PASS;
    char *tname = "Cavecreek SGMII same port loopback at GESW";
    int num_pkt = 300;

    /* Port2 and 3 are kept in down state normally after cavecreek_sgmii_macsa_declare
     */
    testname("%s", tname);

    prpass(testpass, "SGMII-1 loopback, ");
    gesw_port = ovld_get_ge_sw_port_num(0, TGT_DEV_CPU, CPU_SGMII_PORT1);
    set_gesw_line_loopback(gesw_port, 1);
    msleep(100);
    do_ifconfig(CPU_SGMII_PORT1, "up");
    do_ifconfig(CPU_SGMII_PORT2, "down");
    do_ifconfig(CPU_SGMII_PORT3, "down");
    /* wait for ovld link up stable */
    rv = ovld_is_linkup(CPU_SGMII_PORT1);
    if (rv == PASS) {
        rv = sgmii_lpbk_util(CPU_SGMII_PORT1, num_pkt);
    } else {
        printf("[%s]PORT%d sgmii link up fail! \n",__FUNCTION__,CPU_SGMII_PORT1);
    }
    set_gesw_line_loopback(gesw_port, 0);
    if (rv == FAIL) {
        printf("%s port 1 failed\n", tname);
	goto lpbk_exit;
    }
    else {
        printf("passed");
    }

    prpass(testpass, "SGMII-2 loopback, ");
    gesw_port = ovld_get_ge_sw_port_num(0, TGT_DEV_CPU, CPU_SGMII_PORT2);
    set_gesw_line_loopback(gesw_port, 1);
    msleep(100);
    do_ifconfig(CPU_SGMII_PORT2, "up");
    do_ifconfig(CPU_SGMII_PORT1, "down");
    do_ifconfig(CPU_SGMII_PORT3, "down");
    /* wait for ovld link up stable */
    rv = ovld_is_linkup(CPU_SGMII_PORT2);
    if (rv == PASS) {
        rv = sgmii_lpbk_util(CPU_SGMII_PORT2, num_pkt);
    } else {
        printf("[%s]PORT%d sgmii link up fail! \n",__FUNCTION__,CPU_SGMII_PORT2);
    }
    set_gesw_line_loopback(gesw_port, 0);
    if (rv == FAIL) {
        printf("%s port 2 failed\n", tname);
	goto lpbk_exit;
    }
    else {
        printf("passed");
    }

    prpass(testpass, "SGMII-3 loopback, ");
    gesw_port = ovld_get_ge_sw_port_num(0, TGT_DEV_CPU, CPU_SGMII_PORT3);
    set_gesw_line_loopback(gesw_port, 1);
    msleep(100);
    do_ifconfig(CPU_SGMII_PORT3, "up");
    do_ifconfig(CPU_SGMII_PORT1, "down");
    do_ifconfig(CPU_SGMII_PORT2, "down");
    /* wait for ovld link up stable */
    rv = ovld_is_linkup(CPU_SGMII_PORT3);
    if (rv == PASS) {
        rv = sgmii_lpbk_util(CPU_SGMII_PORT3, num_pkt);
    } else {
        printf("[%s]PORT%d sgmii link up fail! \n",__FUNCTION__,CPU_SGMII_PORT3);
    }
    set_gesw_line_loopback(gesw_port, 0);
    if (rv == FAIL) {
        printf("%s port 3 failed\n", tname);
	goto lpbk_exit;
    }
    else {
        printf("passed");
    }

 lpbk_exit:

    /* Set the 3 SGMII port for normal operation */
    do_ifconfig(CPU_SGMII_PORT1, "up");
    do_ifconfig(CPU_SGMII_PORT2, "down");
    do_ifconfig(CPU_SGMII_PORT3, "down");
    if (rv == PASS) {
        printf("\n");
    }
    else {
        cterr('f',0, "%s failed\n", tname);
    }
    return(rv);
}

/*
 * Function: ctrl_plane_sgmii_ext_lpbk_test
 * This test calls the sgmii_lpbk_util function to perform same port
 * loopback on each of the control plane cpu sgmii ports. The loopback path
 * is achieved by using the line loopback feature provided by the GESW.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 */
int ctrl_plane_sgmii_ext_lpbk_test(void)
{
    int gesw_port;
    int rv = PASS;
    char *tname = "Control plane SGMII same port loopback at GESW";
    int num_pkt = 300;
    int ctrl_plane_sgmii_port = CPU_SGMII_PORT3;

    ctrl_plane_sgmii_macsa_declare();

    utah_do_ifconfig(ctrl_plane_sgmii_port, "up");

    testname("%s", tname);

    prpass(testpass, "SGMII-%d loopback, ", ctrl_plane_sgmii_port); 
    gesw_port = ovld_get_ge_sw_port_num(0, TGT_DEV_CPU, ctrl_plane_sgmii_port);
    set_gesw_line_loopback(gesw_port, 1);
    /* wait for ovld link up stable */
    rv = ovld_is_linkup(ctrl_plane_sgmii_port);
    if (rv == PASS) {
        rv = sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt);
    } else {
        printf("[%s]PORT%d sgmii link up fail! \n",__FUNCTION__
        ,ctrl_plane_sgmii_port);
    }
    set_gesw_line_loopback(gesw_port, 0);
    if (rv == FAIL) {
        printf("%s port %d failed\n", tname, ctrl_plane_sgmii_port);
	goto lpbk_exit;
    }
    else {
        printf("passed");
    }

lpbk_exit:
    ctrl_plane_sgmii_macsa_declare();
    if (rv == PASS) {
        printf("\n");
    }
    else {
        cterr('f',0, "%s failed\n", tname);
    }
    return(rv);
}

/******** History ******** 
$Log: cavecreek_sgmii.c,v $
Revision 1.13  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.12  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.11.2.1  2018/08/28 00:04:01  alpeng
extern eth_is_linkup() for user to check eth port link status easily

Revision 1.11  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.10.6.3  2017/11/27 06:08:40  leschen
Initial check in to support VG450.

Revision 1.10.6.2  2017/08/11 22:52:23  ptong
Improve neptune_x86_ge_port_test

Revision 1.10.6.1  2016/12/13 00:23:40  ptong
Added GESW port list util, host port send pkt to GESW test support for Neptune

Revision 1.10  2016/01/20 18:32:53  ptong
Fix kernel driver up-down interference to the cavecreek_sgmii_ext_lpbk test

Revision 1.9  2015/02/18 22:46:39  ptong
Improve cavecreek_sgmii_ext_lpbk_test

Revision 1.8  2014/12/04 10:01:54  erwu2
fix cavecreek SGMII same port loopback at GESW failed

Revision 1.7  2014/09/19 02:47:47  alpeng
add debug msg, fix statement

Revision 1.6  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.5  2014/01/28 02:40:35  ptong
Host SGMII port to GE switch use a fix IP address of 192.123.123.1 to support NGIO module code

Revision 1.4  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.3  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.2  2013/09/09 05:52:34  ptong
Support both Utah P1A and P1B in ctrl_plane_sgmii_ext_lpbk_test

Revision 1.1  2013/09/06 22:51:02  ptong
Make cavecreek_sgmii.c sharable among Overlod and Utah

Revision 1.2  2013/08/13 00:07:06  hroni
support Rangeley control plane SGMII to GE same port loopback test

Revision 1.1  2013/05/09 05:52:59  alpeng
add utah tree

Revision 1.10  2012/11/27 01:42:46  ptong
Use pthread_exit() and display mismatch packets in pkt_rx-double()

Revision 1.9  2012/09/15 01:22:46  ptong
Set diag menu items with MF_SHOW_ERRCOUNT flag

Revision 1.8  2012/09/13 20:42:34  ptong
Code clean up and add comments

Revision 1.7  2012/07/24 16:05:09  palin2
Add TestCard SGMII external loopback test support.

Revision 1.6  2012/07/23 17:33:54  palin2
Initial check-in for Overlord Test Card diag tests.

Revision 1.5  2012/06/05 11:44:36  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/04 02:11:26  ptong
Only keep x86 SGMII-1 for DHCP and TFTP operations

Revision 1.3  2012/05/04 23:16:11  ptong
Improve test message printing

Revision 1.2  2012/03/28 00:38:20  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/

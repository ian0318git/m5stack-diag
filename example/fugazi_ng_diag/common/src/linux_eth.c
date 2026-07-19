/* $Id: linux_eth.c,v 1.7 2014/05/04 03:54:46 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_eth.c,v $
 *------------------------------------------------------------------
 * Linux base ethernet port test code.
 * No platform or HW specific code should come in this file.
 * 
 * Sept 2010 ptong
 *
 * Copyright (c) 2011-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <sys/ioctl.h> /* for ioctl */
#include <arpa/inet.h> /* for inet_aton */
#include <sys/wait.h> /* for wait() */
 
#include "defs.h"
#include "types.h"
#include "error.h"
#include "common.h"
#include "proto.h"
#include "monitor.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"

#undef DEBUG
//#define DEBUG 1    // debug flag for this file


/**********************************************************************
 *
 * Function: skio_cfg
 *
 * Description:
 * configure the ethernet connection via socket ioctl calls to set up
 * parameters such as ip addrss and netmask.
 *      
 * Input:
 * sock - socket id
 * flag - socket config flags from sockios.h
 * inet_param - ptr to the ethernet parameter for config
 * ethreq_p - ptr to the associated ifreq for the config param
 *
 * Return: pass/fail
 */
int
skio_cfg(int sock, ushort flag, void *inet_param,
	 struct ifreq *ethreq_p)
{
    struct sockaddr_in *saddr_in_p;

    switch (flag) {
    case SIOCSIFADDR: // set ip address
        /* Get the data structure first
	 */
        if (ioctl(sock, SIOCGIFADDR, ethreq_p) == -1) {
	  printf("%s() ioctl SIOCGIFADDR failed\n", __FUNCTION__);
	  return(FAIL);
	}
	/* Set the new value and write back
	 */
	saddr_in_p = (struct sockaddr_in *)&ethreq_p->ifr_addr;
	inet_aton((char *)inet_param, &(saddr_in_p->sin_addr));
	if (ioctl(sock, SIOCSIFADDR, ethreq_p) == -1) {
	  printf("%s() ioctl SIOCSIFADDR failed\n", __FUNCTION__);
	  return(FAIL);
	}	   
	break;
    case SIOCSIFNETMASK: // set netmask
        /* Get the data structure
	 */
        if (ioctl(sock, SIOCGIFNETMASK, ethreq_p) == -1) {
	    printf("%s() ioctl SIOCGIFNETMASK failed\n", __FUNCTION__);
	    return(FAIL);
	}
	/* Set the new value and write back
	 */
	 saddr_in_p = (struct sockaddr_in *)&ethreq_p->ifr_netmask;
	 inet_aton((char *)inet_param, &(saddr_in_p->sin_addr));
	 if (ioctl(sock, SIOCSIFNETMASK, ethreq_p) == -1) {
	    printf("%s() ioctl SIOCSIFNETMASK failed\n", __FUNCTION__);
	    return(FAIL);
	}
	 break;
      break;
    case SIOCGIFFLAGS: // get socket flags
        if (ioctl(sock, SIOCGIFFLAGS, ethreq_p) == -1) {
	  printf("%s() ioctl SIOCGIFFLAGS failed\n", __FUNCTION__);
	  return(FAIL);
	}
	break;
    case SIOCSIFFLAGS: // set socket flags
        if (ioctl(sock, SIOCGIFFLAGS, ethreq_p) == -1) {
	  printf("%s() ioctl SIOCIFFLAGS failed\n", __FUNCTION__);
	  return(FAIL);
	}
	/* Set the new value and write back
	 */
	ethreq_p->ifr_flags = (ulong)inet_param;
	if (ioctl(sock, SIOCSIFFLAGS, ethreq_p) == -1) {
	  printf("%s() ioctl SIOCSIFFLAGS failed\n", __FUNCTION__);
	  return(FAIL);
	}	   
	break;
    default:
        printf("%s %s() unknown flag %#.8x used\n", __FILE__, __FUNCTION__, flag);
	return(FAIL);
    }
    return(PASS);
}


/**********************************************************************
 *
 * Function: init_ip_socket
 *
 * Description: Initialize an IP socket
 * 
 * Input: skinfo_p - ptr to the structure that contains the
 *        interface name, ip address, etc information.
 *
 * Return: pass/fail
 */
int
init_ip_socket(ethport_info_t *ethport_info_p)
{
    ethport_info_t *ethp = ethport_info_p;
    skinfo_t *skp = &ethp->skt;
    char portname[IFNAMSIZ];
    int rc = PASS;

    sprintf(portname, "%s%d", ethp->ifname, ethp->portid);

    /* Get the IP addresses from string
     */
    inet_aton(ethp->ipstr, &(ethp->skaddr_in.sin_addr));
    ethp->ipaddr = (uint) ethp->skaddr_in.sin_addr.s_addr;

    //    printf("ipaddr= %s %#.8x\n",ethp->ipstr, ethp->ipaddr);

    skp->skid = socket(skp->skfamily, skp->sktype, skp->skprotocol);
    if (-1 == skp->skid) {
      printf("%s() Error Creating Socket", __FUNCTION__);
      return(FAIL);
    }
    
    /* Set up the sockets
     */
    memset(&skp->ethreq, 0, sizeof(struct ifreq));
    sprintf(skp->ethreq.ifr_name, portname);

    /* Do ifconfig to set the IP address and the netmask of the port
     */
    if (skio_cfg(skp->skid, SIOCSIFADDR, ethp->ipstr, &skp->ethreq) == FAIL) {
      printf("%s() ioctl SIOCSIFADDR failed ", __FUNCTION__);
      rc = FAIL;
    }

#ifdef DEBUG
    printf("\nIP addr is %s\n",
	   inet_ntoa(((struct sockaddr_in *)&skp->ethreq.ifr_addr)->sin_addr));
#endif
    
    memset(&skp->ethreq.ifr_netmask, 0, sizeof(skp->ethreq.ifr_netmask));
    if (skio_cfg(skp->skid, SIOCSIFNETMASK, ethp->netmask, &skp->ethreq) == FAIL) {
      printf("%s() ioctl SIOCSIFNETMASK failed ", __FUNCTION__);
      rc = FAIL;
    }

#ifdef DEBUG
    printf("\nnet mask is %s\n",
	   inet_ntoa(((struct sockaddr_in *)&skp->ethreq.ifr_netmask)->sin_addr));
#endif

    if (rc == FAIL)
      close(skp->skid);

    return(rc);
}


#ifdef DEBUG

/**********************************************************************
 *
 * Function: display_eth_pkt
 *
 * Description: Display Ethernet IP packet
 * 
 * Input: ethhead - ptr to the packet
 *        len - the number of bytes shown
 *
 * Return: void
 */
static void
display_eth_pkt(uchar *ethhead, int len)
{
    int eth_hdr_len = sizeof(struct ethhdr);  // 14 bytes
    int ip_hdr_len = sizeof(struct iphdr);   // 20 bytes
    int udp_hdr_len = sizeof(struct udphdr); // 8 bytes
    int eth_ip_udp_hdr_len = (eth_hdr_len +
			      ip_hdr_len +
			      udp_hdr_len);
    uchar *iphead, *data_p; 
    int jj;

    iphead = ethhead + eth_hdr_len;
    data_p = ethhead + eth_ip_udp_hdr_len;

    printf("\n------------------------------------\n");
    printf("MAC DA: "
	   "%02x:%02x:%02x:%02x:%02x:%02x,    "
	   "MAC SA: "
	   "%02x:%02x:%02x:%02x:%02x:%02x\n",
	   ethhead[0],ethhead[1],ethhead[2],
	   ethhead[3],ethhead[4],ethhead[5],
	   ethhead[6],ethhead[7],ethhead[8],
	   ethhead[9],ethhead[10],ethhead[11]);
	      
    printf("Source IP: %d.%d.%d.%d,    "
	   "Dest IP: %d.%d.%d.%d\n",
	   iphead[12],iphead[13],
	   iphead[14],iphead[15],
	   iphead[16],iphead[17],
	   iphead[18],iphead[19]);
    printf("Source, Dest ports: %d, %d\n",
	   (iphead[20]<<8)+iphead[21],
	   (iphead[22]<<8)+iphead[23]);
    printf("Layer-4 protocol %d\n",iphead[9]);
    printf("Data:\n");
    for (jj=0; jj < (len - eth_ip_udp_hdr_len); jj++) {
        if ((jj > 0) && ((jj % 20) == 0)) {
	    printf("\n");
	}
	printf("%02x ", data_p[jj]);
    }
    printf("\n");
}
#endif


/**********************************************************************
 *
 * Function: eth_rx
 *
 * Description: Receive packet via the rx socket. On a linux system,
 * the ethernet port always has traffics, this function need to
 * parse the ip src and dst ddr to detect the packet it is watching.
 * 
 * Input:
 * rx_sk - socket id for the receive
 * ip_watch_src - the ip src addr to check for
 * ip_watch_dst - the ip dst addr to check for
 * rx_buf - the buffer to store the rx packet
 * buflen - the len of the packet
 *
 * Return: pass/fail
 */
int
eth_rx(int rx_sk, uint32_t ip_watch_src, uint32_t ip_watch_dst,
	   uchar *rx_buf, int buflen)
{
    int eth_hdr_len = sizeof(struct ethhdr);  // 14 bytes
    int ip_hdr_len = sizeof(struct iphdr);   // 20 bytes
    int udp_hdr_len = sizeof(struct udphdr); // 8 bytes
    int eth_ip_udp_hdr_len = (eth_hdr_len +
			      ip_hdr_len +
			      udp_hdr_len);
    uchar *iphead, *ethhead;
    uint32_t ip_src, ip_dst;
    uint32_t catch_pkt, rx_size, term_cnt, term_limit;
    struct timeval tv;

    /* Set up socket timeout to handle rx time out
     */
    memset(&tv, 0, sizeof(struct timeval));
    tv.tv_sec = 5;   /* set receive time out for 5 secs */

    if (setsockopt(rx_sk, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv, sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
        exit(-1);
    }

    ethhead = rx_buf;
    iphead = rx_buf + eth_hdr_len; /* Skip Ethernet header */

    catch_pkt = 0;
    term_cnt = 0;
    term_limit = 2;

    while (!catch_pkt && (term_cnt < term_limit)) {
        rx_size = recvfrom(rx_sk, rx_buf, buflen, 0, NULL, NULL);
	term_cnt++;

	/* Check to see if the packet contains at least
	 * complete Ethernet (14), IP (20) and TCP/UDP
	 * (8) headers.
	 */
	if (rx_size < eth_ip_udp_hdr_len) {
	    printf("Incomplete packet (errno is %d)\n", errno);
	    continue;
	}

	if (*iphead == IPV4_VER_HLEN) { // IPV4 pkt
	    ip_src = ((iphead[12] << 24) | (iphead[13] << 16) |
		      (iphead[14] << 8) | iphead[15]);
	    ip_dst = ((iphead[16] << 24) | (iphead[17] << 16) |
		      (iphead[18] << 8) | iphead[19]);

	    /* Check the IP src and des address match the loopback pkt
	     */
	    if ((ip_src == ip_watch_src) && (ip_dst == ip_watch_dst)) {
		catch_pkt = 1;
#ifdef DEBUG
		printf("packet catched. rx_size= %d bytes\n", rx_size);
		display_eth_pkt(ethhead, rx_size);
#endif
	    }
	    else {
#ifdef DEBUG
		printf("%s() RX pkt is IPV4 but not the target\n", __func__);
#endif
	    }	    
	} // end of if ipv4
	else {
#ifdef DEBUG
	    printf("%s() RX pkt is not IPV4\n", __func__);
#endif
	}
    } // end of while

    if (catch_pkt > 0) {
	return(PASS);
    }
    else {
	return(FAIL);
    }
}

/**********************************************************************
 *
 * Function: eth_tx
 *
 * Description: Xmit packet via the tx socket
 * 
 * Input:
 * tx_sk - socket id for the tx
 * dst_sina - the ip dst addr to send to
 * tx_data - the buffer to store the tx packet
 * len - the len of the packet
 *
 * Return: void
 */
int
eth_tx(int tx_sk, struct sockaddr_in *dst_sina, uchar *tx_data, int len)
{
    int tx_size;

#ifdef DEBUG
    printf("send to %s (%#.8x) %d byte \n", inet_ntoa(dst_sina->sin_addr),
	   dst_sina->sin_addr.s_addr, len);
#endif
	 
    tx_size = sendto(tx_sk, tx_data, len, 0, dst_sina, sizeof(struct sockaddr_in)); 

    if (tx_size < 1) {
        printf("unable to send %s\n", strerror(errno));
	return FAIL;
    }
    return(PASS);
}

/**********************************************************************
 *
 * Function: eth_port_lpbk
 *
 * Description:
 * Perform packet loopback on an ethernet port. The internal or external
 * loopback setup at the HW component level is expected to be done
 * before calling this function. The packet being used is UDP
 * packet via Linux socket. The Linux fork() function is used to
 * create a tx and a rx process for the tx socket and rx socket.
 *
 * Input: ethp - pointer to the struct which provdies ip addresses
 *               and netmask for the ifconfig set up
 *
 * Return: pass/fail
 */
int
eth_port_lpbk(eth_lpbk_info_t *ethp)
{
    pktdata_info_t *pktb_p;
    struct sockaddr_in sina;	 
    pid_t pid, wait_rtn_pid;
    int child_exit_rtn, rx_rtn_stat;
    int pp, pktb_size;
    int tt, tx_cnt, tx_total, rx_total;
    uchar tx_data[ETH_PKT_MAX_LEN];
    uchar rx_buf[ETH_PKT_MAX_LEN], *rx_data;
    uchar orig_hkpflag = hkeepflags;
    int tx_sk, rx_sk;
    ulong rxsk_flag;
    int child_rc = FAIL;
    int parent_rc = FAIL;

    ethport_info_t eptx = {
      .portid = ethp->eth_port,
      .ifname = ethp->eth_name,
      .ipstr = ethp->ip_src_str,
      .netmask = ethp->ip_netmask_str,
      .skt = {
	.skid = 0,
	.skfamily = AF_INET,
	.sktype = SOCK_DGRAM,
	.skprotocol = IPPROTO_UDP,
      },
    };

    ethport_info_t eprx = {
      .portid = ethp->eth_port,
      .ifname = ethp->eth_name,
      .ipstr = ethp->ip_src_str, // the tx and rx are the same port
      .netmask = ethp->ip_netmask_str,
      .skt = { // The rx socket is differnet from tx
	.skid = 0,
	.skfamily = PF_PACKET,
	.sktype = SOCK_RAW,
	.skprotocol = htons(ETH_P_IP),
      },
    };

    /* Get the IP source and destination addresses from string
     */
    inet_aton(ethp->ip_src_str, &(sina.sin_addr));
    ethp->ip_src = (uint32_t) sina.sin_addr.s_addr;
    inet_aton(ethp->ip_dst_str, &(sina.sin_addr));
    ethp->ip_dst = (uint32_t) sina.sin_addr.s_addr;

    /* Point to the packet table
     */
    pktb_p = ethp->pkt_tb_p;
    pktb_size = ethp->pktb_sz;
    tx_total = rx_total = 0;
    
    /* filbyte use this flag. Restore to saved value, and 
     * then set to new value for this packet
     */
    hkeepflags = orig_hkpflag;

    /* Set up the TX socket */
    tx_sk = -1;
    if (init_ip_socket(&eptx) == FAIL) {
        printf("%s() Initialize tx socket failed.", __FUNCTION__);
	goto test_exit;
    }
    tx_sk = eptx.skt.skid;
       
    /* Set up the RX sockets
     */
    rx_sk = -1;
    if (init_ip_socket(&eprx) == FAIL) {
        printf("%s() Initialize rx socket failed.", __FUNCTION__);
	goto test_exit;
    }
    rx_sk = eprx.skt.skid;

    /* Set the ethernet port with no arp protocol
     */
    if (skio_cfg(rx_sk, SIOCGIFFLAGS, 0, &eprx.skt.ethreq) == FAIL) {
        printf("%s() get socket flags failed ", __FUNCTION__);
	goto test_exit;
    }
    rxsk_flag = eprx.skt.ethreq.ifr_flags;
    rxsk_flag |= (IFF_NOARP);
    if (skio_cfg(rx_sk, SIOCSIFFLAGS, (void *)rxsk_flag, &eprx.skt.ethreq) == FAIL) {
        printf("%s() set socket flag failed ", __FUNCTION__);
	goto test_exit;
    }

    /* Do the packet TX and RX test
     */
    for (pp=0; pp < pktb_size; pp++) {

        hkeepflags |= pktb_p[pp].hkpflags;

	tx_cnt = pktb_p[pp].send_count;
	for (tt=0; tt < tx_cnt; tt++) {

	    /* Fill tx_data buffer with test data */
	    memset(tx_data, 0, ETH_PKT_MAX_LEN);
	    filbyte(tx_data, pktb_p[pp].len, pktb_p[pp].val);

	    /* Fork out a child process which will run first and 
	     * is used to setup the packet RX. The parent process
	     * is used for packet TX.
	     */
	    pid = fork();
	    if (pid == -1) {
	        printf("%s() fork() failed", __func__);
		goto test_exit;
	    }

	    if (pid > 0) {    /* Parent process: packet tx */
	        /* Make sure the tx process start after rx is setup */
	        msleep(100);
		parent_rc = FAIL;

#ifdef DEBUG
		printf("TX process ready to xmit. child pid = %d\n", pid);
#endif // DEBUG

		/* Set up the destination address and L4 port */
		memset(&sina, 0, sizeof(sina));
		sina.sin_family = AF_INET;
		sina.sin_port = htons(L4_DEST_PORT);
		sina.sin_addr.s_addr = ethp->ip_dst;

		if (eth_tx(tx_sk, &sina, tx_data, pktb_p[pp].len) == FAIL) {
		    printf("%s() Packet %d TX failed\n", __func__, pp);
		}
		tx_total++;

		/* Wait for the child to exit */
		wait_rtn_pid = wait(&child_exit_rtn);
		rx_rtn_stat = WEXITSTATUS(child_exit_rtn);

#ifdef DEBUG
		printf("wait_rtn_pid = %d child_exit_rtn =%d\n"
		       "rx_rtn_stat=%d\n",
		       wait_rtn_pid, child_exit_rtn, 
		       rx_rtn_stat);
#endif

		if (rx_rtn_stat == PASS) {
		    // printf("%s() Packet %d loopback passed\n", __func__, pp);
		  rx_total++;
		  parent_rc = PASS;
		}
		else {
		    printf("%s() Packet %d loopback failed\n", __func__, pp);
		    goto test_exit;
		}
	    }
	    else {     /* Child process: packet rx */
	        /* Note:
		 * Must use exit() to terminate the child process.
		 * Using return() will keep the child process
		 * alive.
		 */
	        child_rc = FAIL;

#ifdef DEBUG
		printf("RX process. pid = %d waiting for rx pkt\n", pid);
#endif // DEBUG

		/* Watch the in coming packets
		 */
		if (eth_rx(rx_sk, ethp->ip_src, ethp->ip_dst, rx_buf, sizeof(rx_buf)) == PASS) {
		    /* Packet detected. Check the payload
		     */
		    rx_data = rx_buf + ETH_IP_UDP_HDR_LEN;
		    if ((cmpbyte(tx_data, rx_data, pktb_p[pp].len)) == PASS) {
		        // printf("%s() Rx packet %d matched\n", __func__, pp);
		      child_rc = PASS;
		    }
		    else {
		      printf("%s() Rx packet %d mismatched\n", __func__, pp);
		    }
		}
		else {
		    printf("Packet %d was not received.\n", pp);
		}

		exit(child_rc);

	    } // end of fork()
	} //end of tt for loop
    } // end of pp for loop

 test_exit:

    if (tx_sk != -1) {
        close(tx_sk);
    }

    /* Restore the eth port to normal mode
     */
    if (rx_sk != -1) {
        if (skio_cfg(rx_sk, SIOCGIFFLAGS, 0, &eprx.skt.ethreq) == FAIL) {
	    printf("%s() get socket flags failed ", __FUNCTION__);
	}
	rxsk_flag = eprx.skt.ethreq.ifr_flags;
	rxsk_flag &= ~(IFF_NOARP);
	if (skio_cfg(rx_sk, SIOCSIFFLAGS, (void *)rxsk_flag, &eprx.skt.ethreq) == FAIL) {
	    printf("%s() set socket flag failed ", __FUNCTION__);
	}

	close(rx_sk);
    }

    hkeepflags = orig_hkpflag;

#ifdef DEBUG
    printf("Packet TX total = %d RX total = %d\n", tx_total, rx_total);
#endif
    if (parent_rc == PASS) {
        return(PASS);
    }
    else {
        return(FAIL);
    }
}

/******** History ******** 
$Log: linux_eth.c,v $
Revision 1.7  2014/05/04 03:54:46  mcharon
add back linux_eth.c needed by cavium code


$Endlog$
*/

/* $Id: platform_sfp_ext_lpbk.c,v 1.22 2013/08/19 01:54:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_sfp_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_sfp_ext_lpbk.c
 * SFP external loopback test.
 *
 * Oct 2011-2012 Alan Peng
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>  /* for bzero*/
#include<string.h>
#include<sys/socket.h>
#include<features.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<linux/ethtool.h> /*struct ethtool */
#include<linux/sockios.h> /* SIOCETHTOOL */
#include <sys/types.h> /* getpid */
#include <unistd.h>  /* getpid */
#include <netinet/in.h>  /* for including the linux_eth.h */

#include<errno.h>
#include<sys/ioctl.h>
#include<net/if.h>

#include<pthread.h>
#include "defs.h"
#include "types.h"
#include "common.h"
#include "error.h"
#include "monitor.h"

#include "dash_fpga.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"


/* global */
static int sfp_port_list[] = { SFP0, SFP1, SFP2, SFP3 };
static int sfp_speed_list[] = { SPD_1000MBPS }; /* fiber only test 1000Mpbs*/


static pktdata_info_t pktdata[] = {
  {0xb1, ETH_PKT_MIN_LEN, H_INCFILL, 100},
  {0xb3, (ETH_PKT_MIN_LEN + 1), H_INCFILL, 100},
  {0xb5, ((ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN - 1)), H_INCFILL, 100},
  {0xb7, (ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN), H_INCFILL, 100},
};


#if DEBUG
/*------------------------------------------------------------------
 *
 * Function: adv_full_duplex
 *   make sure the test is on full duplex.
 *   we have encounter the condition that fiber is not in full duplex 
 *   mode after sgmii tests. Thus, we add this step to turn on full
 *   duplex.
 *
 * Input:  onoff - turn on/off advertise reg.
 *         port - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int adv_full_duplex(boolean onoff, int port)
{
#define ETH_DRIVER_DELAY    1
   ushort rdval, wrval;
	
   ovld_phy_reg_wr(port, 22, 1); /* go to page 0 */
   rdval = ovld_phy_reg_rd(port, 4);
   rdval = rdval & ~0x0040;  /* turn off half duplex */

   if(onoff)
   	wrval = rdval | 0x0020;  /* enable full duplex */
   else
   	wrval = rdval & ~0x0020;  /* restore force link up*/
   	
   ovld_phy_reg_wr(port, 4, wrval);
   rdval = ovld_phy_reg_rd(port, 4);
   sleep(ETH_DRIVER_DELAY);
   
   if(rdval < 0)
       return FAILED;
   else 
       return PASSED;	
}
#endif /* DEBUG */

/*------------------------------------------------------------------
 *
 * Function: switch_fiber
 *   ensure the copper is truned off before fiber test.
 *   turn the copper after the test is finished.
 *
 * Input:  type - port type
 *         onoff - turn on/off advertise reg.
 *         port - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int switch_fiber(char *type, int port, boolean onoff){
	
    char pname[10];
    int rc = 0;
    
    sprintf(pname,"%s%d", type, port);   

    if(onoff) {
        /*enable fiber, disable copper */
        if ((rc = sig_pwr_ctrl(pname, DISABLE_SIG, SIG_COPPER)) != PASSED){  
            printf("sig_pwr_ctrl failed \n"); 
    	      return(FAILED);
        }        
        if ((rc = sig_pwr_ctrl(pname, ENABLE_SIG, SIG_FIBER)) != PASSED){  
    	      printf("sig_pwr_ctrl failed \n"); 
    	      return(FAILED);
        }
#if DEBUG
        /* turn on advertise full duplex reg */
        if ((rc = adv_full_duplex(ENABLE_SIG, port+ADDR_MEDIA_PHY)) != PASSED){  
          	printf("adv_full_duplex failed \n");
          	return(FAILED);
        }   
#endif 
    }
    else {
        /*enable copper, disable fiber */
        if ((rc = sig_pwr_ctrl(pname, DISABLE_SIG, SIG_FIBER)) != PASSED){  
    	      printf("sig_pwr_ctrl failed \n"); 
    	      return(FAILED);
        }
        if ((rc = sig_pwr_ctrl(pname, ENABLE_SIG, SIG_COPPER)) != PASSED){  
    	      printf("sig_pwr_ctrl failed \n"); 
    	      return(FAILED);
        }	
#if DEBUG
        /* turn on advertise full duplex reg */
        if ((rc = adv_full_duplex(DISABLE_SIG, port+ADDR_MEDIA_PHY)) != PASSED){
    	      printf("adv_full_duplex failed \n");
    	      return(FAILED);
        }	
#endif
    }

   return (PASSED);
} 

/*------------------------------------------------------------------
 *
 * Function: sfp_set_phy_lpbk
 *	initial and setup loopback type on sgmii, both internal lpbk and 
 *  external lpbk can use this function. The initial function is based 
 *  on ethtool, because we can not let port link up via set the PHY
 *  directly.
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_set_phy_lpbk(char *type, int port, boolean lpbk_typ, int speed)
{
    char pname[10];
    int rc = 0;
   
    sprintf(pname,"%s%d", type, port);   

    /* init as 100Mbps, duplex full, autoneg off */
    if ((rc = set_port_speed(pname, SPD_10MBPS)) != PASSED){  
    	printf("set_port_speed failed \n");
    	return(FAILED);
    }
    
    if ((rc = ovld_cavium_is_linkup(port)) != PASSED){
        printf("after init port, sfp link up time out after 1 second \n");
    }

    /* turn on fiber and off the copper */
    switch_fiber(type, port, ENABLE_SIG);
   
    /* to ensure the test stay on full duplex and setup speed */
    if ((rc = cfg_phy_setting(pname, speed, FULL_DUPLEX, AUTONEG_OFF, SIG_FIBER)) != PASSED){  
    	printf("cfg_phy_setting fail failed \n"); 
    	return(FAILED);
    }	

    if ((rc = ovld_cavium_is_linkup(port)) != PASSED){
        printf("after set speed, sfp link up time out after 1 second \n");
    }

    /* 1GMbps external loopback need to setup*/
    if ((rc = set_phy_stub(pname, lpbk_typ, SIG_FIBER)) != PASSED){
    	printf("set_phy_stub failed \n"); 
    	return(FAILED);
    }

    /* ovld_phy_soft_reset will turn off Enable loopback reg */
    if ((rc = ovld_phy_soft_reset(pname, SIG_FIBER)) != PASSED){
    	printf("ovld_phy_soft_reset failed \n");
    	return(FAILED);
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY*3);	

    return PASSED;
}

/*------------------------------------------------------------------
 *
 * Function: sfp_set_packet
 *	Set up packet info for tx and rx using.
 *
 *
 * Input:  port: current test port   
 *         speed: current test speed   
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_set_packet(int port, int speed) {
	
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = FAILED;
    uchar orig_hkpflag = hkeepflags;
    
    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    printf("testing.");
    fflush(stdout);

    hkeepflags = orig_hkpflag;
    
    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {            	
    printf(".");  
    fflush(stdout);
         /* set packet */
         pkt_cnt = pktdata[typ_curr].send_count;  
         pkt_len = pktdata[typ_curr].len;
         pkt_val = pktdata[typ_curr].val;
         hkeepflags |= pktdata[typ_curr].hkpflags;
                  
         /* prepare to send packet */
         rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
         if (rc != PASSED) 
             break;
       
    } /* typ_curr */

    if (rc != PASSED) {
        printf("tx_rx_diag failed Port: %d Speed: %d rc = %d\n",port, speed, rc);
	show_status_info(port + ADDR_MEDIA_PHY);
    } 
    else {
        printf("Pass\n");
    }

    fflush(stdout);
    hkeepflags = orig_hkpflag;
    return rc;
}


/*------------------------------------------------------------------
 *
 * Function: sfp_phy_ext_lpbk_test
 *	This is the entry point for SFP external loopback test only.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_phy_ext_lpbk_test(void) {
	  
    int rc = 0, retry_diag = 0; 
    int port_cnt, port_curr, port;
    int speed_cnt, speed_curr, speed;
    char pname[10];
	  
    testname("SFP PHY external loopback");  

    if (!check_ext_lpbk_flag()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
	return PASS; /* external loopback is not set, skipped. */
    }

    /* get test envrionment variable */
    port_cnt = sizeof(sfp_port_list) / sizeof(int);
    speed_cnt = sizeof(sfp_speed_list) / sizeof(int);
	  
    /* select port */
    for(port_curr = 0; port_curr < port_cnt; port_curr++) {
      port = sfp_port_list[port_curr];
      
      /* check SFP is available. if not, skipped */
      if(!is_sfp_present(port)) {
          /* since we already check the ext_lpbk flag before,
           * if the sfp is not present, then report failed.
           */
          cterr('f',0,"cannot detect SFP on port: %d \n", port);
          continue; 
      }
 
     /* select speed*/
      for(speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
      speed = sfp_speed_list[speed_curr];

retry_again:
            sprintf(pname, "eth%d", port);

            prpass(testpass, "Test SFP-%d, ", port);
            /* setup loopback information */          
            rc = sfp_set_phy_lpbk(SEL_PORT_ETH, port, EXT_LPBK, speed);
                   
            if (rc != PASS) {
              /* ensure fiber is off before leaving test.*/
              switch_fiber(SEL_PORT_ETH, port, DISABLE_SIG);
              /* prevent keep "trying speed..." message */ 
              ovld_err_clean_up(port);
              cterr('f',0,"sfp_set_phy_lpbk failed, port: %d \n", port);
              break; /* skip to next port */
            }
            
            /* set packet */
            rc = sfp_set_packet(port , speed);
            
            if(is_sfp_tx_fault(port)) {
                cterr('f',0,"sfp tx fault detect, port: %d \n", port);
            }

            if (rc != PASS) {
              ovld_err_clean_up(port);
              if (retry_diag == 0) {
                printf("\n ++++ retry the test ++++ \n");
		reset_quad_phy();
                retry_diag = 1;
                goto retry_again;
              }

              /* ensure fiber is off before leaving test.*/
              switch_fiber(SEL_PORT_ETH, port, DISABLE_SIG);
	      printf("Possible causes of problem:\n"
		     "1. SFP module or external loopback plug missing/bad.\n"
		     "2. Media PHY fiber mode external path bad. Try to run SGMII internal loopback test.\n");
              cterr('f',0,"sfp_set_packet failed, port: %d \n", port);
              break; /* skip to next port */ 
            }
           
            retry_diag = 0;
        } /* speed_curr */
    
    /* ensure fiber is off before leaving test.*/
    switch_fiber(SEL_PORT_ETH, port, DISABLE_SIG); 
    } /* port_curr */
    
    return PASS;
}

/*------------------------------------------------------------------
 *
 * Function: sfp_ext_lpbk_test_util
 *
 * Utility to execute SFP external loopback
 *
 * Input:  port - port numner 0-3
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_ext_lpbk_test_util(int port_curr) {
	  
    int rc = 0; 
    int speed_curr;

    /* get test envrionment variable */
    speed_curr = sizeof(sfp_speed_list) / sizeof(int);

    /* check SFP is available */
    if(!is_sfp_present(port_curr)) {
        printf("cannot detect SFP on port: %d \n", port_curr);
        return rc;  
    }
 
    /* setup loopback information */          
    rc = sfp_set_phy_lpbk(SEL_PORT_ETH, port_curr, 
         EXT_LPBK, sfp_speed_list[speed_curr]);
                   
    if (rc == FAIL) {
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, port_curr, DISABLE_SIG);
        ovld_err_clean_up(port_curr);
       	cterr('f',0,"sfp_set_phy_lpbk failed, port: %d \n", port_curr);
       	return FAILED;
    }
            
    /* set packet */
    rc = sfp_set_packet(port_curr, sfp_speed_list[speed_curr]);
            
    if (rc == FAIL) {
       	/* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, port_curr, DISABLE_SIG);
        ovld_err_clean_up(port_curr);
	printf("Possible causes of problem:\n"
	       "1. SFP module or external loopback plug missing/bad.\n"
	       "2. Media PHY fiber mode external path bad. Try to run SGMII internal loopback test.\n");
       	cterr('f',0,"sfp_set_packet failed\n");
       	return FAILED;
    }
    
    /* ensure fiber is off before leaving test.*/
    switch_fiber(SEL_PORT_ETH, port_curr, DISABLE_SIG); 
   
    return rc;
}



/*-------------------------------------------------
$Log: platform_sfp_ext_lpbk.c,v $
Revision 1.22  2013/08/19 01:54:12  alpeng
checking sfp tx fault right after spf loopback test

Revision 1.21  2013/02/19 19:01:25  ptong
Add more info in error message

Revision 1.20  2012/11/06 23:00:51  ptong
Add header and clean up

Revision 1.19  2012/10/26 02:00:08  ptong
Add delay to allow PHY to be stable before sending packet

Revision 1.18  2012/10/20 01:27:00  ptong
Bug fix: CSCuc79132 SGMII and SFP ext loopback failing randomly on different ports

Revision 1.17  2012/09/17 15:55:31  alpeng
1. add is_linkup for sfp
2. combine soft reset on set_automedia and bridge_phy_mode for speed up
3. fixed definition order of SGMII_INT_EXT_LPBK for util.
4. clean up code.

Revision 1.16  2012/08/11 00:00:18  ptong
Remove complile flag RELEASE_CVMX_DIAG

Revision 1.15  2012/08/01 14:26:33  alpeng
adding check link up status for SFP and internal loopback

Revision 1.14  2012/07/24 06:09:39  alpeng
support 2nd retry on loopback test

Revision 1.13  2012/07/19 20:10:31  ptong
Improve test progress message

Revision 1.12  2012/07/18 22:59:29  ptong
Fix a problem so that (NVRAM)->diagflag is used correctly on Cavium data plane menu

Revision 1.11  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.10  2012/05/24 01:06:14  alpeng
add error report when external loopback is set, but SFP is not present

Revision 1.9  2012/05/23 09:20:09  alpeng
support is_sfp_present function for SFP test

Revision 1.8  2012/05/08 00:05:15  ptong
Improve test printing

Revision 1.7  2012/05/02 09:44:50  alpeng
skip next speed and switch to next port when test failed.

Revision 1.6  2012/05/02 00:15:36  ptong
Change sfp_set_packet to work with nc command

Revision 1.5  2012/04/27 10:42:43  alpeng
fixed minor bugs and support set external loopback flag for controlling test flow

Revision 1.4  2012/04/27 01:02:15  ptong
Set SFP ext loopback test with the MF_DOALL flag

Revision 1.3  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/

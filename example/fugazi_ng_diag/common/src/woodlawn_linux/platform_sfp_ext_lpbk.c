/* $Id: platform_sfp_ext_lpbk.c,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_sfp_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_sfp_ext_lpbk.c
 * SFP external loopback test.
 *
 * Oct 2011-201 Alan Peng
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
#include <stdint.h>

#include<errno.h>
#include<sys/ioctl.h>
#include<net/if.h>

#include<pthread.h>
#include "defs.h"
#include "types.h"
#include "common.h"
#include "error.h"
#include "monitor.h"

#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"


#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "diag_ge_phy_88E1340_lib.h"
#include "diag_ge_phy_88E1548L_lib.h"
#include "diag_fpga_lib.h"
#include "diag_common_drv.h"

static pktdata_info_t pktdata[] = {
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 5},
  {0xa1, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 5},
  {0xa5, ((ETH_UDP_DATA_MAX_LEN - 1 ) -12), H_INCFILL, 5},
  {0xa6, (ETH_UDP_DATA_MAX_LEN - 12), H_INCFILL, 5},
};

/* Fiber only test 1000Mpbs*/
int sfp_speed_list[] = {SPD_1000MBPS };

/* Mapping of SFP to FPGA port for 6GE SKU
 * SFP0 (PHY1) -> GE1_G0
 * SFP1 (PHY1) -> GE1_G1
 * SFP2 (PHY0) -> GE0_G0
 * SFP3 (PHY0) -> GE0_G1
 * SFP4 (PHY0) -> GE0_G2
 * SFP5 (PHY0) -> GE0_G3
 */
static int sku_6ge_sfp_fpga_map[] = {0, 1, 0, 1, 2, 3};

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
    ushort wrval;
    int bus_id, rc, val;

    bus_id = get_smi_bus_id(port);

    /*go to page 1*/
    rc = woodlawn_phy_reg_wr(bus_id, port, PHY_REG(22), PHY_REG(1));
    if (rc == FAILED) {
        cterr('f', 0, "Write to page %d failed", PHY_REG(1));
        return (rc);
    }

    rc = woodlawn_phy_reg_rd(bus_id, port, 4, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read original reg %d val failed", 4);    
        return (rc);
    }
    
    val = val & ~0x0040;  /* turn off half duplex */

    if (onoff) {
        wrval = val | 0x0020;  /* enable full duplex */
    } else {
        wrval = val & ~0x0020;  /* restore force link up*/
   }

    rc = woodlawn_phy_reg_wr(bus_id, port, 4, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", 4, wrval);    
        return (rc);
    }
    
    sleep(ETH_DRIVER_DELAY);

    return (PASSED);
}



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
int switch_fiber(char *type, int port, boolean onoff) {
    char pname[10];
    int rc = PASSED, sku_id;
    int *sku_eth_fiber_mapping;
    
    sprintf(pname,"%s%d", type, port);   

    sku_id = get_sku_id();

    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        sku_eth_fiber_mapping = old_eth_fiber_mapping;
    } else {
        /* Official SKUs */
        if (sku_id == WOODLAWN_6GE) {
            sku_eth_fiber_mapping = two_phy_eth_fiber_mapping;
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            sku_eth_fiber_mapping = one_phy_eth_fiber_mapping;
        }
    }

    if (onoff) {
        /*enable fiber, disable copper */
        if ((rc = sig_pwr_ctrl(pname, DISABLE_SIG, SIG_COPPER)) != PASSED) {  
            cterr('f',0, "%s: disable copper failed", pname);
            return(FAILED);
        }        
        if ((rc = sig_pwr_ctrl(pname, ENABLE_SIG, SIG_FIBER)) != PASSED) {  
            cterr('f',0, "%s: enable fiber failed", pname);
            return(FAILED);
        }
        /* turn on advertise full duplex reg */
        if ((rc = adv_full_duplex(ENABLE_SIG, sku_eth_fiber_mapping[port]))
             != PASSED) {
            cterr('f',0, "%s: turn on full duplex failed", pname);
            return(FAILED);
            }   
        } else {
        /*enable copper, disable fiber */
        if ((rc = sig_pwr_ctrl(pname, DISABLE_SIG, SIG_FIBER)) != PASSED) {  
            cterr('f',0, "%s: disable fiber failed", pname);
            return(FAILED);
        }
        if ((rc = sig_pwr_ctrl(pname, ENABLE_SIG, SIG_COPPER)) != PASSED) {  
            cterr('f',0, "%s: enable copper failed", pname);
            return(FAILED);
        }
        /* turn on advertise full duplex reg */
        if ((rc = adv_full_duplex(DISABLE_SIG, sku_eth_fiber_mapping[port]))
             != PASSED) {
            cterr('f',0, "%s:  turn off full duplex failed", pname);
            return (FAILED);
        }
    }

   return (PASSED);
} 

/*------------------------------------------------------------------
 *
 * Function: sfp_set_phy_lpbk
 *  initial and setup loopback type on sgmii, both internal lpbk and 
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
    char cmd_str[32];
   
    sprintf(pname,"%s%d", type, port);   
    
    /* init as 100Mbps, duplex full, autoneg off */
    if ((rc = set_port_speed(pname, speed)) != PASSED) {
        printf("%s set port speed %d failed\n", pname, speed);
        return (FAILED);
    }
    
    if ((rc = woodlawn_cavium_is_linkup(SEL_PORT_ETH, port)) != PASSED){
        printf("after init port, sfp link up time out after 1 second \n");
    }

    /* turn on fiber and off the copper */
    switch_fiber(type, port, ENABLE_SIG);
   
    /* to ensure the test stay on full duplex and setup speed */
    if ((rc = cfg_phy_setting(pname, speed, FULL_DUPLEX, AUTONEG_OFF, SIG_FIBER)) != PASSED) {  
        printf("%s: turn on full duplex and set speed %d failed\n", pname, speed);
        return (FAILED);
    }
    
    if ((rc = woodlawn_cavium_is_linkup(SEL_PORT_ETH, port)) != PASSED){
        printf("after set speed, sfp link up time out after 1 second \n");
    }

    /* 1Gbps external loopback need to setup*/
    if ((rc = set_phy_stub(pname, lpbk_typ, SIG_FIBER)) != PASSED) {
        printf("%s: setup external loopback stub failed\n", pname);
        return (FAILED);
    }

    /* woodlawn_phy_soft_reset will turn off Enable loopback reg */
    if ((rc = woodlawn_phy_soft_reset(pname, SIG_FIBER)) != PASSED){
    	printf("woodlawn_phy_soft_reset failed \n");
    	return(FAILED);
    }

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY*3);	

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: sfp_set_packet
 * set up packet info for tx and rx using.
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
    int sku_id, *sku_eth_fiber_mapping;
    
    sku_id = get_sku_id();

    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;
    
    if (sku_id == WOODLAWN_6GE_1XAUI) {
        /* Not official SKU */
        sku_eth_fiber_mapping = old_eth_fiber_mapping;
    } else {
        /* Official SKUs */
        if (sku_id == WOODLAWN_6GE) {
            sku_eth_fiber_mapping = two_phy_eth_fiber_mapping;
        } else if (sku_id == WOODLAWN_4GE_1XAUI) {
            sku_eth_fiber_mapping = one_phy_eth_fiber_mapping;
        }
    }
    for (typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
         /* set packet */
         pkt_cnt = pktdata[typ_curr].send_count;  
         pkt_len = pktdata[typ_curr].len;
         pkt_val = pktdata[typ_curr].val;
         hkeepflags |= pktdata[typ_curr].hkpflags;

         prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                            port, speed, pkt_cnt, pkt_len, pkt_val);
                 
         /* prepare to send packet */
         rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
         if (rc != PASSED) {
             printf("%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                     __FUNCTION__, port, speed);
             hkeepflags = orig_hkpflag;
             show_status_info(port + ADDR_MEDIA_PHY);
             return (FAILED);
         }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", port, speed);
    fflush(stdout);
    
    hkeepflags = orig_hkpflag;

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: sfp_phy_ext_lpbk_test
 * This is the entry point for SFP external loopback test only.
 * the internal loopback will be support in the future
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_phy_ext_lpbk_test (int phy) {
    int rc = 0, retry_diag = 0; 
    int port_cnt, port_curr;
    int speed, speed_curr;
    int sfp_port, check_sfp_present;
    char pname[10];
    int *sku_eth_mapping_sfp_num, id;

    if (!check_ext_lpbk_flag()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return (PASSED); /* external loopback is not set, skipped. */
    }

    /* Get the SKU id */
    id = get_sku_id();

    /* get test environment variable */
    if (phy == MRVL_1548_PHY0) {
        port_cnt = 4;
    } else {
        /* New SKU just have 4 GE ports */
        if (id == WOODLAWN_4GE_1XAUI) {
            return (PASSED);
        } else {
            port_cnt = 2;
        }
    }

    /* Not official SKU has different eth number SFP port mapping compared with
     * new official SKU
     */
    if (id == WOODLAWN_6GE_1XAUI) {
        sku_eth_mapping_sfp_num = old_eth_mapping_sfp_num;
    } else if (id == WOODLAWN_6GE) {
        sku_eth_mapping_sfp_num = two_phy_eth_mapping_sfp_num;
    } else if (id == WOODLAWN_4GE_1XAUI) {
        sku_eth_mapping_sfp_num = one_phy_eth_mapping_sfp_num;
    }

    speed = sizeof(sfp_speed_list) / sizeof(int);
    
    /* select port */
    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        /* get test environment variable */
        if (phy == MRVL_1548_PHY0) {
            sfp_port = eth_qlm4_list[port_curr];
            check_sfp_present = sku_eth_mapping_sfp_num[sfp_port];
        } else {
            sfp_port = eth_qlm0_list[port_curr];
            check_sfp_present = sku_eth_mapping_sfp_num[sfp_port];
        }

        /* Need to translate to FPGA SFP port if SKU is 6GE */
        if (id == WOODLAWN_6GE) {
            check_sfp_present = sku_6ge_sfp_fpga_map[check_sfp_present];
        }

        /* check SFP is available. if not, return failed */
        if (is_sfp_present(phy, check_sfp_present) == FALSE) {
            cterr('f', 0, "SFP module %d is not detected",
                  sku_eth_mapping_sfp_num[sfp_port]);
            /* ensure fiber is off before leaving test.*/
            switch_fiber(SEL_PORT_ETH, sfp_port, DISABLE_SIG);
            /* prevent keep "trying speed..." message */
            woodlawn_err_clean_up(sfp_port);
            return (FAILED);
        } else {
            prpass(testpass, "SFP module %d is detected",
                   sku_eth_mapping_sfp_num[sfp_port]);
        } 
        
        /* Enable Tx transmit */
        if (enable_sfp_tx_transmit(phy, check_sfp_present) == FAILED) {
            cterr('f', 0, "Failed to enable transmit on SFP-%d",
                  sku_eth_mapping_sfp_num[sfp_port]);
            /* ensure fiber is off before leaving test.*/
            switch_fiber(SEL_PORT_ETH, sfp_port, DISABLE_SIG);
            /* prevent keep "trying speed..." message */
            woodlawn_err_clean_up(sfp_port);
            return (FAILED);
        }

        /* select speed*/
        for (speed_curr = 0; speed_curr < speed; speed_curr++) {
retry_again:
            sprintf(pname, "SFP%d", sku_eth_mapping_sfp_num[sfp_port]);
            testname("SFP-%d external loopback", sku_eth_mapping_sfp_num[sfp_port]);
            /* setup loopback information */          
            rc = sfp_set_phy_lpbk(SEL_PORT_ETH, sfp_port, EXT_LPBK,
                                  sfp_speed_list[speed_curr]);
                   
            if (rc != PASSED) {
                /* ensure fiber is off before leaving test.*/
                switch_fiber(SEL_PORT_ETH, sfp_port, DISABLE_SIG);
                /* prevent keep "trying speed..." message */
                woodlawn_err_clean_up(sfp_port);
                cterr('f',0,"Setup sfp port - %d loopback information failed", 
                      sku_eth_mapping_sfp_num[sfp_port]);
                break; /* skip to next port */
            }

            /* Check if the SFP is under link up status */
            /*if (check_sfp_link(pname) == FAILED) {
                return (FAILED);
            }*/
            if (woodlawn_cavium_is_linkup(SEL_PORT_ETH, sfp_port) == FAILED) {
                cterr('f',0,"SFP - %d is not link", sku_eth_mapping_sfp_num[sfp_port]);
                return (FAILED);
            }
     
            /* set packet */
            rc = sfp_set_packet(sfp_port, sfp_speed_list[speed_curr]);
     
            if (rc != PASSED) {
                woodlawn_err_clean_up(sfp_port);
                if (retry_diag == 0) {
                    printf("\n ++++ retry the test ++++ \n");
                    reset_quad_phy();
                    diag_88e1340_init();
                    diag_88e1548_init();

                    retry_diag = 1;
                    goto retry_again;
                }

                /* ensure fiber is off before leaving test.*/
                switch_fiber(SEL_PORT_ETH, sfp_port, DISABLE_SIG);
                /* prevent keep "trying speed..." message */
                woodlawn_err_clean_up(sfp_port);
                cterr('f',0,"SFP - %d set packet failed", sku_eth_mapping_sfp_num[sfp_port]);
                break; /* skip to next port */
            }
        } /* speed_curr */
    
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, sfp_port, DISABLE_SIG);
    } /* port_curr */
    
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: fiber_88E1548l_platform_lpbk_test
 *  This is the entry point for SFP line loopback test only.
 *
 * Input:  port : current test fiber port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int fiber_88E1548l_platform_lpbk_test (int port)
{
    /* 881548L fiber port 2 and port 3 connects to platform side. */
    int eth_port, rc, speed_curr;
    
    testname("Backplane loopback");

    speed_curr = 0;
    eth_port = eth_qlm0_list[port];
        
    /* setup loopback information */          
    rc = sfp_set_phy_lpbk(SEL_PORT_ETH, eth_port, EXT_LPBK,
                              sfp_speed_list[speed_curr]);
                   
    if (rc != PASSED) {
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, eth_port, DISABLE_SIG);
        /* prevent keep "trying speed..." message */
        woodlawn_err_clean_up(eth_port);
        cterr('f',0,"sfp_set_phy_lpbk failed, eth%d, fiber port%d", eth_port, port);
        return (rc);
    }
            
    /* set packet */
    rc = sfp_set_packet(eth_port, sfp_speed_list[speed_curr]);

    if (rc != PASSED) {
        woodlawn_err_clean_up(eth_port);
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, eth_port, DISABLE_SIG);
        /* prevent keep "trying speed..." message */
        woodlawn_err_clean_up(eth_port);
        return (rc);
    } else {
        prpass(testpass, "Pass eth%d-fiber port%d line loopback test\n", eth_port, port);
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, eth_port, DISABLE_SIG);
        return (rc);
    }
}

/*------------------------------------------------------------------
 *
 * Function: sfp_ext_lpbk_test_util
 * this function is used to support utiles 
 *
 * Input:  None
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
    /*if(!is_sfp_present(port_curr)) {
        printf("cannot detect SFP on port: %d \n", port_curr);
        return rc;  
    }*/
 
    /* setup loopback information */          
    rc = sfp_set_phy_lpbk(SEL_PORT_ETH, port_curr, 
         EXT_LPBK, sfp_speed_list[speed_curr]);
                   
    if (rc == FAIL) {
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, port_curr, DISABLE_SIG);
        woodlawn_err_clean_up(port_curr);
            cterr('f',0,"sfp_set_phy_lpbk failed, port: %d \n", port_curr);
            return FAILED;
    }
            
    /* set packet */
    rc = sfp_set_packet(port_curr, sfp_speed_list[speed_curr]);
            
    if (rc == FAIL) {
        /* ensure fiber is off before leaving test.*/
        switch_fiber(SEL_PORT_ETH, port_curr, DISABLE_SIG);
        woodlawn_err_clean_up(port_curr);
        cterr('f',0,"sfp_set_packet failed\n");
        return FAILED;
    }
    
    /* ensure fiber is off before leaving test.*/
    switch_fiber(SEL_PORT_ETH, port_curr, DISABLE_SIG); 
   
    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: check_sfp_link
 *  Check if the SFP is link up
 *
 * Input:  ifname - port type
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int check_sfp_link(char *ifname)
{
    int sk;
    struct ifreq ethreq;
    ushort rdval, regnum;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        cterr('f', 0, "%s() Error Creating RX Socket", __FUNCTION__);
        return(FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* Change the page register to page 1 */
    regnum = PHY_REG(1);
    phy_reg_wr(sk, &ethreq, PHY_REG(22), regnum);
    /* Read Fiber Status Register Page 1, Register 1 */
    phy_reg_rd(sk, &ethreq, FIBER_SPECIFIC_STATUS_REG, &rdval);
    if (!(rdval & 0x400)) {
        cterr('f',0, "Fiber Status Register Page 1, Register 1 is %#4x",
              rdval);

        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: platform_sfp_ext_lpbk.c,v $
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.10  2013/03/27 08:45:06  kuangik
 * Code cleanup
 *
 * Revision 1.8  2013/03/20 12:24:34  kuangik
 * Correct SFP port mapping for 6GE SKU
 *
 * Revision 1.19  2013/02/18 08:24:02  leslie
 * Using woodlawn phy reg r/w lib instead of using ovle reg r/w lib
 *
 * Revision 1.18  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.17  2013/01/18 06:42:42  leslie
 * Fix and clean up code.
 *
 * Revision 1.16  2013/01/16 02:19:46  leslie
 * Judge SKU type before do sfp external loopback test.
 *
 * Revision 1.14  2012/12/11 01:52:20  leslie
 * Fix fiber 88E1548L platform loopback test.
 *
 * Revision 1.13  2012/11/20 01:38:58  leslie
 * Fix sfp loopback test and add detect sfp module mechanism.
 *
 * Revision 1.12  2012/11/19 02:49:05  leslie
 * Add check sfp present into sfp loopback test.
 *
 * Revision 1.11  2012/11/02 12:03:49  kody
 * Support SFP port 2 and 3 line loopback test for O2.
 *
 * Revision 1.10  2012/10/24 10:44:39  leslie
 * Fix and clean up code.
 *
 * Revision 1.9  2012/10/18 12:55:02  kody
 * Add 88E1548L fiber line loopback between platform side.
 *
 * Revision 1.8  2012/10/08 09:59:08  leslie
 * Fix the loopback test to use sfp port.
 *
 * Revision 1.7  2012/09/21 11:53:35  kody
 * Add check if the fiber is link function.
 *
 * Revision 1.6  2012/09/05 22:58:36  kody
 * Fix the external sfp loopback test for woodlawn.
 *
 * Revision 1.5  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/05/18 10:25:26  kody
 * Fix the type warning during compile.
 *
 * Revision 1.2  2012/04/06 06:07:45  kuangik
 * Update for GE PHY Test
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.9  2011/12/19 10:23:18  alpeng
 * fixed bug between tx and rx packet sync,
 * modified the tx_rx_diag test flow, increase packet number.
 *
 * Revision 1.1.2.8  2011/12/05 15:04:27  alpeng
 * update bridge PHY internal loopback
 * fix menu item name
 * fix sfp return
 *
 * Revision 1.1.2.7  2011/11/10 08:03:18  alpeng
 * support SFP loopback utils
 *
 * Revision 1.1.2.5  2011/11/09 09:23:17  alpeng
 * clean up code
 *
 * Revision 1.1.2.4  2011/11/08 04:35:46  alpeng
 * fixed initial packet number
 *
 * Revision 1.1.2.3  2011/11/08 02:07:17  alpeng
 * modify the packet mount, type, size
 *
 * Revision 1.1.2.2  2011/11/02 00:55:24  alpeng
 * update loopback test, add util. packet number and length should be increased
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

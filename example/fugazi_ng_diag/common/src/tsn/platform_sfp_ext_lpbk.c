/* $Id: platform_sfp_ext_lpbk.c,v 1.4 2018/05/24 09:47:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_sfp_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_sfp_ext_lpbk.c
 * Description: Main file of TSN SFP external loopback test.
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
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

#include "nvsysvars.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_ext_lpbk.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "platform_cpu.h"
#include "platform_ge_phy.h"
#include "i2c_api.h"
#include "platform_sfp_cookie.h"
#include "platform_i2c.h"

static int set_sfp_glc_ge_100fx(int);
static int set_sfp_glc_t_1000(int);

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

#define ETH_DRIVER_DELAY    1
#define ENHANCE_ERROR_MSG_RDY 1
#define ERR_BUF_SIZE				80

extern int tsn_display_temp_errormsg(void);
extern int show_status_info(int);
extern uint sfp_type;
extern uint sfp_encode;
extern void msleep(int);
/*******************************************************************************
 *
 * Function   : switch_fiber
 * Discription: ensure the copper is truned off before fiber test.
 *   turn the copper after the test is finished.
 *
 * Inputs     :  type - port type
 *         onoff - turn on/off advertise reg.
 *         port - port number
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int switch_fiber (int port, boolean onoff)
{
    if (onoff == ENABLE_SIG) {
        /* Enable Fiber, and Disable Copper. */
        if (sig_pwr_ctrl(port, DISABLE_SIG, SIG_COPPER) != PASSED) {  
    	    printf("Failed to Disable Copper.\n"); 
    	    return (FAILED);
        }
        if (sig_pwr_ctrl(port, ENABLE_SIG, SIG_FIBER) != PASSED) {
            printf("Failed to Enable Fiber.\n"); 
    	    return (FAILED);
        }
    } else {
        /* Enable Fiber, and Disable Copper. */
        if (sig_pwr_ctrl(port, DISABLE_SIG, SIG_FIBER) != PASSED) {  
    	    printf("Failed to Disable Fiber.\n"); 
    	    return (FAILED);
        }
        if (sig_pwr_ctrl(port, ENABLE_SIG, SIG_COPPER) != PASSED) {
            printf("Failed to Enable Copper.\n"); 
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
    int rc = 0, auto_nego = AUTONEG_OFF;
    char cmd_str[32];
   
    sprintf(pname,"%s%d", type, port);   
    
    if (sfp_type == SFP_GE_100FX) {
        /* For GLC-GE-100FX Auto nego ON */
        auto_nego = AUTONEG_ON;
    } 

    /* init as 100Mbps, duplex full, autoneg off */
    if (sig_set_port_speed(port, speed, SIG_FIBER) != PASSED) {
        printf("%s: Failed to set port%d speed %d.\n",
               __FUNCTION__, port, speed);
        return (FAILED);
    }
    
    if((rc = tsn_check_link_status(SEL_PORT_ETH, port)) != PASSED) {
        printf("after init port, sfp link up time out after 1 second \n");
    } 

    /* turn on fiber and off the copper */
    switch_fiber(port, ENABLE_SIG);
   
    /* to ensure the test stay on full duplex and setup speed */
    if ((rc = cfg_phy_setting(port, speed, FULL_DUPLEX, auto_nego, SIG_FIBER)) != PASSED) {  
        printf("%s: turn on full duplex and set speed %d failed\n", pname, speed);
        return (FAILED);
    }
    
    if((rc = tsn_check_link_status(SEL_PORT_ETH, port)) != PASSED) {
        printf("after set speed, sfp link up time out after 1 second \n");
    }

    /* 1Gbps external loopback need to setup*/
    if ((rc = set_phy_stub(port, lpbk_typ, SIG_FIBER)) != PASSED) {
        printf("%s: setup external loopback stub failed\n", pname);
        return (FAILED);
    }

    /* woodlawn_phy_soft_reset will turn off Enable loopback reg */
    if ((rc = tsn_phy_soft_reset(port, SIG_FIBER)) != PASSED){
    	printf("%s: Failed to soft reset PHY.\n", __FUNCTION__);
    	return (FAILED);
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
int sfp_set_packet (int port, int speed) {

    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = FAILED;
    uchar orig_hkpflag = hkeepflags;
    
    pkt_type = (sizeof(pktdata) / sizeof(pktdata_info_t));
    hkeepflags = orig_hkpflag;
    
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
             show_status_info(port);
             return (FAILED);
         }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", port, speed);
    fflush(stdout);
    
    hkeepflags = orig_hkpflag;

    return (rc);
}

/*******************************************************************************
 *
 * Function   : sfp_phy_ext_lpbk_test
 * Description: Entry function of SFP external loopback test.
 *              The internal loopback will be support in the future.
 * Inputs     : phy_num - number of phy(ethX)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sfp_phy_ext_lpbk_test (int phy_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E1112 GE WAN Phy", "SFP");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Change another external loopback fixture.",
                    "If step a. is still failed, try the internal "
                    "loopback to check if internal loopback is OK.",
                    "If step b. is OK, we can know PHY has problems. "
                    "If step b is failed, please try step d.",
                    "Observe MDIO register status to "
                    "check if PHY configuration is normal.",
                    "If step d. is OK, execute the MAC loopback test.",
                    "If step e. is OK, we can assume the interface "
                    "between Host SoC and PHY has problems.");
#endif

    int rc = 0;
    int sfp_is_detected = FALSE; 
    int speed, speed_curr, default_speed;
    int test_result = FAILED;

    char *tname = "SFP external loopback";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    speed = sizeof(sfp_speed_list) / sizeof(int);

    /* Check if SFP is available. if not, return failed */
    if (is_sfp_present(&sfp_is_detected) != PASSED) {
        cterr('f', 0, "Failed to check SFP status.");
    	prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if (sfp_is_detected != TRUE) {
        cterr('f', 0, "SFP module is not detected.");
    	prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        prpass(testpass, "SFP module is detected");
    } 
        
    /* Enable Tx transmit */
    if (sfp_tx_enable_switch(ENABLE) != PASSED) {
        cterr('f', 0, "Failed to Enable transmit on SFP");
    	prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* For GLC-GE-100FX using this default speed and config SFP Internal Reg */
    if ((sfp_encode == SFP_ENCODE_4B5B) && (sfp_type == SFP_GE_100FX)) {
        if (set_sfp_glc_ge_100fx(CPU_I2C1) != PASSED) {
            cterr('f', 0, "Failed to set sfp glc ge 100fx");
            return (FAILED);
        }
        default_speed = SPD_1000MBPS;
    } else if ((sfp_encode == SFP_ENCODE_4B5B) && (sfp_type == SFP_FE_100FX)) {
        /* For GLC-FE-100FX using this default speed */
        default_speed = SPD_100MBPS;
    } else if ((sfp_encode == SFP_ENCODE_8B10B) && (sfp_type == SFP_GLC_TE)) {
        /* For GLC-FE-100FX using this default speed and config SFP Internal Reg */
        if (set_sfp_glc_t_1000(CPU_I2C1) != PASSED) {
            cterr('f', 0, "Failed to set sfp glc ge 100fx");
            return (FAILED);
        }
        default_speed = SPD_1000MBPS;
    } else {
        /* others use default SFP speed */ 
        default_speed = SPD_1000MBPS;
    }

    if (diag_kernel_ver == (uint)LINUX_KERNEL_V4_4_52) {
        /* Based on the change of Marvell driver from SDK 16.05 to 17.10.
         * Currently only Vulcan uses Marvell SDK 17.10 Kernel(v4.4.52),
         * will update/remove this if-judgement after all TSN/Star merged
         * to same Kernel version.
         */

        /* Confirm both CPU MAC side and PHY side are Link up */
        if (tsn_cpu_mac_check_linkstat(phy_num, CPUMAC_LINKUP) != PASSED) {
            printf("%s: Failed, SFP(eth%d) CPU MAC is not link up.\n",
                   __func__, phy_num);
            return (FAILED);
        }

        if (tsn_gephy_check_linkstat(phy_num, GEPHY_FIB,
                                     GEPHY_LINKUP) != PASSED) {
            printf("%s: Failed, GEWAN%d PHY is not link up.\n",
                   __func__, phy_num);
            return (FAILED);
        }

        /* Fiber only test 1000Mpbs*/
        if (tsn_sgmii_lpbk_test(phy_num, SPD_1000MBPS) == PASSED) {
            test_result = PASSED;
        } else {
            printf("%s: Failed at eth%d SPF 1000mbps loopback test.\n",
                   __func__, phy_num);
            test_result = FAILED;
        }

        /* Disable Tx transmit */
        if (sfp_tx_enable_switch(DISABLE) != PASSED) {
            printf("%s: Failed to Disable SFP transmit by FPGA.\n", __func__);
            return (FAILED);
        }
        return (test_result);
    } else {
    /* select speed*/
    for (speed_curr = 0; speed_curr < speed; speed_curr++) {
/* retry_again: */
        /* setup loopback information */        
        rc = sfp_set_phy_lpbk(SEL_PORT_ETH, phy_num, EXT_LPBK,
                             default_speed);
                   
        if (rc != PASSED) {
            cterr('f',0,"Setup SFP loopback information failed");
            break; /* skip to next port */
        }

        /* Check if the SFP is under link up status */
        if(tsn_check_link_status(SEL_PORT_ETH, phy_num) != PASSED) {
            cterr('f', 0, "%s: SFP NOT  link up.\n", __FUNCTION__);                                                        
    	    prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        } 

        /* set packet */
        rc = sfp_set_packet(phy_num, default_speed);
        if (rc != PASSED) {
            cterr('f',0,"SFP set packet failed");
            break; /* skip to next port */
        }
    } /* speed_curr */
    }
    
    /* ensure fiber is off before leaving test.*/
    switch_fiber(phy_num, DISABLE_SIG);
    
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/**********************************************************************
 *
 * Function:    set_sfp_glc_ge_100fx
 *
 * Description:	Configure GLC-GE-100FX to FX mode. Refer to vendor data sheet.
 *
 * Input:	i2c_bus - I2C bus enum.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
int set_sfp_glc_ge_100fx (int i2c_bus)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
    uint8_t reg_hi, reg_lo;
    char err_buf[ERR_BUF_SIZE * 2];
    
    /* Setup theeset_sfp_glc_ge_100fxI2C struct */
    /* I2C bus number, and device enum */
    i2c_if.i2c_dev = SFP_I2C_DEV_ADDR;
    i2c_if.i2c_bus_type = i2c_bus;

    i2c_if.i2c_speed = N2G_I2C_100KHZ;	/* I2C bus speed */

    /* Open the device */
    if ((rc = n2g_i2c_open(&i2c_if)) != PASSED) {
    	cterr('f', 0, "%s() Unable to open. rc = %#x", __FUNCTION__, rc);
    	return(FAILED);
    }
    
    /* Write FX100 Enable register to set FX mode */
    i2c_if.offset = SFP_GE_100FX_REG; /* offset */
    reg_lo = SFP_GE_100FX_REG_FX_L;   /* Set the FX mode */
    reg_hi = SFP_GE_100FX_REG_FX_H;

    /* Ready to write the high byte */
    i2c_if.size = sizeof(reg_hi);
    i2c_if.buf = (char *)&reg_hi;

    msleep(SFP_I_INIT_TIME);    /* wait for t_init */

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s %s () Unable to write FX Enable Register "
                         "High byte in GLC-GE-100FX SFP.\nrc = %#x",
			 __FILE__, __FUNCTION__, rc);
        rc = FAILED;
    } else {
        /* Ready to write the low byte */
        i2c_if.size = sizeof(reg_lo);
        i2c_if.buf = (char *)&reg_lo;

        rc = n2g_i2c_write(&i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "%s %s() Unable to write FX Enable Register "
                             "Low byte in GLC-GE-100FX SFP.\nrc = %#x",
			        __FILE__, __FUNCTION__, rc);
            rc = FAILED;
        } else {
            /* Write the Edge Control register for the loopback plug */
            i2c_if.offset = SFP_GE_100FX_REG18; /* Offset */
            reg_lo = SFP_GE_100FX_REG_EC_L;     /* New Edge Control */
            reg_hi = SFP_GE_100FX_REG_EC_H;

            /* Ready to write the high byte */
            i2c_if.size = sizeof(reg_hi);
            i2c_if.buf = (char *)&reg_hi;

            msleep(SFP_I_INIT_TIME);	/* wait for t_init */

            rc = n2g_i2c_write(&i2c_if);
            if (rc != PASSED) {
            	sprintf(err_buf, "%s() Unable to write GLC-GE-FX100 "
                                     "Edge Control Register High bytes.\n"
                                     "rc = %#x", __FUNCTION__, rc);
            	rc = FAILED;
            } else {
            	/* Ready to write the low byte */
            	i2c_if.size = sizeof(reg_lo);
            	i2c_if.buf = (char *)&reg_lo;
            	rc = n2g_i2c_write(&i2c_if);
            	if (rc != PASSED) {
                	    sprintf(err_buf, "%s() Unable to write GLC-GE-FX100 "
                                             "Edge Control Register Low byte.\n"
                                             "rc = %#x", __FUNCTION__, rc);
                 	    rc = FAILED;
                    } /* endof if write Edge Control low byte */
            } /* endof if write Edge Control high byte */
	    } /* endof if write FX Enable low byte */
    } /* endof if write FX Enable high bytes */

    /* Close the device */
    n2g_i2c_close(&i2c_if);

    if (rc != PASSED) {
    	cterr('f', 0, err_buf);
    }

    return(rc);

}


/**********************************************************************
 *
 * Function:    sfp_I2C_oper
 *
 * Description: This function porting from FinMcMissile.
 *              For configure SFP internal register	
 *              write operation.
 * Input:	reg_high - for register endian
 *          reg_low - for register endian
 *          i2c structure.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int sfp_I2C_oper (uint8_t reg_high, uint8_t reg_low, n2g_i2c_if_t *i2c_if)
{
    uint32_t rc;
    uint8_t reg_hi, reg_lo;
    char err_buf[ERR_BUF_SIZE * 2];
    
    reg_hi = reg_high;
    reg_lo = reg_low;

    /* Ready to write the high byte */
    i2c_if->size = sizeof(reg_hi);
    i2c_if->buf = (char *)&reg_hi;

    /* Open the device */
    if ((rc = n2g_i2c_open(i2c_if)) != PASSED) {
    	cterr('f', 0, "%s() Unable to open. rc = %#x", __FUNCTION__, rc);
    	return(FAILED);
    }

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        sprintf(err_buf, "%s %s () Unable to write GLC-T SFP.\nrc = %#x",
			 __FILE__, __FUNCTION__, rc);
        rc = FAILED;
    } else {
        /* Ready to write the low byte */
        i2c_if->size = sizeof(reg_lo);
        i2c_if->buf = (char *)&reg_lo;

        rc = n2g_i2c_write(i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "%s %s() Unable to write GLC-T SFP.\nrc = %#x",
			        __FILE__, __FUNCTION__, rc);
            rc = FAILED;
        }
    }

    /* Close the device */
    n2g_i2c_close(i2c_if);

    if (rc != PASSED) {
    	cterr('f', 0, err_buf);
    }

    return(rc);

}

/**********************************************************************
 *
 * Function:    set_sfp_glc_t_1000
 *
 * Description:	Configure SFP Internal Register for SFP GLC-t 1000 
 *              to support loopback test. Refer to Avago vendor data sheet 
 *              "Frequently Asked Questins - Question 14".
 *
 * Input:	i2c_bus - I2C bus enum.
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int set_sfp_glc_t_1000 (int i2c_bus)
{
    n2g_i2c_if_t i2c_if;

    /* Setup the I2C struct */
    /* I2C bus number, and device enum */
    switch(i2c_if.i2c_bus_type = i2c_bus) {
        case CPU_I2C1:
        case CPU_I2C0:
            /* SFP 1 */
            i2c_if.i2c_dev = SFP_I2C_DEV_ADDR;
            break;
        default:
        	/* Invalid SFP */
        	cterr('f', 0, "%s() Invalid SFP I2C bus %#x", __FUNCTION__, i2c_bus);
        	return(FAILED);
    } /* endof switch */

    i2c_if.i2c_speed = N2G_I2C_100KHZ;	/* I2C bus speed */

    /* Clear all interrupts */
    i2c_if.offset = SFP_COPPER_INT_REG;
    if (sfp_I2C_oper(SFP_CLR_INT_H, SFP_CLR_INT_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Force Master mode */
    i2c_if.offset = SFP_COPPER_MA_SL_CR;
    if (sfp_I2C_oper(SFP_FRC_MASTER_H, SFP_FRC_MASTER_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Apply soft reset and enable auto-negotiation */
    i2c_if.offset = SFP_COPPER_CONTROL;
    if (sfp_I2C_oper(SFP_RES_EN_AUTO_NEG_H, SFP_RES_EN_AUTO_NEG_L,
                &i2c_if) == FAILED) {
        return (FAILED);
    }

    msleep(SFP_PHY_RESET_DELAY);

    /* Select page 7 of reg 30 */
    i2c_if.offset = 0x1D;
    if (sfp_I2C_oper(SFP_SEL_P7_REG30_H, SFP_SEL_P7_REG30_L,
                &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Force Gigabit mode */
    i2c_if.offset = 0x1E;
    if (sfp_I2C_oper(SFP_FRC_GBPS_MODE_H, SFP_FRC_GBPS_MODE_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Select page 16 of reg 30 */
    i2c_if.offset = 0x1D;
    if (sfp_I2C_oper(SFP_SEL_P16_REG30_H, SFP_SEL_P16_REG30_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Enable Stub loopback */
    i2c_if.offset = 0x1E;
    if (sfp_I2C_oper(SFP_EN_LBPK_STUB_H, SFP_EN_LBPK_STUB_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Select page 18 of reg 30 */
    i2c_if.offset = 0x1D;
    if (sfp_I2C_oper(SFP_SEL_P18_REG30_H, SFP_SEL_P18_REG30_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    /* Disable Near End Crosstalk (Next) canceller. */
    i2c_if.offset = 0x1E;
    if (sfp_I2C_oper(SFP_DIS_NEXT_H, SFP_DIS_NEXT_L, &i2c_if) == FAILED) {
        return (FAILED);
    }

    return(PASSED);

}
/*-------------------------------------------------
 * $Log: platform_sfp_ext_lpbk.c,v $
 * Revision 1.4  2018/05/24 09:47:10  steja
 * CSCvj57981-Enhance SFP GLC-GE-100FX Support
 *
 * Revision 1.3  2018/04/15 22:03:30  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.2  2017/08/02 14:21:49  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:20  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/24 14:14:11  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:07  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.4.2.1  2017/07/17 13:54:44  palin2
 * Code cleanup.
 *
 * Revision 1.1.4.4  2016/11/01 07:29:22  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.3  2016/10/04 06:39:08  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.2  2016/06/30 06:22:51  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.1  2016/04/26 20:48:49  palin2
 * Updated code after bring up SFP external loopback test.
 *
 * $Endlog$
 *-------------------------------------------------
 */


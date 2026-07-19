/* $Id: platform_macsec.c,v 1.2 2017/07/14 02:51:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_macsec.c,v $
 *------------------------------------------------------------------
 * 
 * platform_macsec.c  
 * support 88E1549P PHY macsec test
 *
 * Dec 2012 Alan Peng
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
  
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<strings.h>  /* for bzero*/
#include<string.h>
#include<sys/socket.h>
#include<features.h>
#include<netinet/in.h>  /* for including the linux_eth.h */
#include<net/if.h>  /* for ethreq */

#include "defs.h"
#include "proto.h" /* msleep */
#include "types.h"
#include "common.h"
#include "error.h"
#include "monitor.h"
#include "queryflags.h"
#include "ethernet.h" /* port definition */
#include "nvmonvars.h" /* D_VERBOSE */
#include "menu.h"
#include "linux_eth.h"

#include "platform_eth.h"
#include "platform_ext_lpbk.h" /*page definition */
#include "platform_macsec.h"

extern int set_media_int_lpbk(boolean, int);
extern int set_media_phy_int_lpbk(char *, int, int);
extern int set_macsec(boolean, int);

static int eth_port_list[] = { SGMII0, SGMII1, SGMII2, SGMII3 };
static int eth_speed_list[] = { SPD_10MBPS, SPD_100MBPS };

#define F_GRP        (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E      (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL        (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t macsec_tests_submenu_table[] = {

    {"MACsec test on 1548 PHY", (type_t(*)())ovld_macsec_test,   0,
        F_GRP_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"MACsec test on 1548 PHY Utility", (type_t(*)())macsec_test_88e1548p_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Dump Statistic", (type_t(*)())dump_statistic,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"set internal loopback(macsec off)", (type_t(*)())set_internal_loopback_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"init macsec setting", (type_t(*)())init_macsec_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"clear macsec counter(statistic)", (type_t(*)())clr_macsec_cnt_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"config drop bad tag", (type_t(*)())set_drop_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"config macsec ", (type_t(*)())set_macsec_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"send packet temp", (type_t(*)())send_packet_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"read page 16 regs", (type_t(*)())read_reg_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"write page 16 regs", (type_t(*)())write_reg_util,   0,
        0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define MACSEC_TESTS_SUBMENU_TABLE_SIZE (sizeof(macsec_tests_submenu_table) / \
                                         sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t macsec_tests_primary_items[MACSEC_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];
static mitem_t macsec_tests_secondary_items[MACSEC_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];

menuinfo_t macsec_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    macsec_tests_primary_items,
};
menuinfo_t *macsec_submenup = &macsec_subtest_menu;


static pktdata_info_t pktdata[] = {
  {0xa0, ETH_PKT_MIN_LEN, H_INCFILL, 10},
  {0xa2, (ETH_PKT_MIN_LEN + 1), H_INCFILL, 10},
  {0xa4, ((ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN - 1)), H_INCFILL, 10},
  {0xa6, (ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN), H_INCFILL, 10},
};


/*------------------------------------------------------------------
 *
 * Function: macsec_test_main
 *      This is the entry point for the macsec main test.
 *
 * Input:  dummy
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int macsec_test_main (int dummy)
{

    build_primary_submenu(macsec_tests_submenu_table,
                          MACSEC_TESTS_SUBMENU_TABLE_SIZE,
                          "MACsec", &macsec_submenup);
    build_secondary_submenu(macsec_tests_submenu_table,
                            MACSEC_TESTS_SUBMENU_TABLE_SIZE,
                            macsec_tests_secondary_items);

    menu(macsec_submenup, macsec_tests_secondary_items, '\0' );

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: ovld_macsec_test 
 *      a testing wrapper for macsec test
 *
 * Input: NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int ovld_macsec_test (void) {

    uint port, port_curr, rc = FAILED;
    uint speed, speed_curr;
    uint speed_cnt, port_cnt;
    uint try, retry_limit = 2;

    testname("88E1548p MACsec");

    port_cnt = sizeof(eth_port_list) / sizeof(int);
    speed_cnt = sizeof(eth_speed_list) / sizeof(int);

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        port = eth_port_list[port_curr];

        for (speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
           speed = eth_speed_list[speed_curr];

           prpass(testpass, "Test port-%d speed-%d, ", port, speed);
           for (try=0; try < retry_limit; try++) {
               rc = macsec_test_88e1548p(port,speed);
               if ((rc == PASSED) || (try == (retry_limit - 1))) {
                    break;
               } else {
                    printf("####### retry the test #########\n");
                    reset_quad_phy();
               }
           }
          
           if (rc != PASSED) {
               cterr('f',0,"MACsec test failed on port%d with spd%d\n", port, speed);
               return(FAILED);
           }

        }  /* for speed */
    }  /* for port */

    return(PASSED);

}
/*------------------------------------------------------------------
 *
 * Function: macsec_test_88e1548p
 *
 * Description: testing macsec on 88E1548P.
 *              steps: turn on macsec on PHY,
 *              disable drop_bad_tag, send packets and
 *              check the statistic bit on PHY.
 *
 * Input:  port - test port 
 *         speed - test speed 
 *
 * Output: PASSED/FAILED
 *
 * From Marvell Eng:
 * After power-up, MACSec is by default enabled.
 * The drop_bad_tag is by default is set.
 * If MACSec is enabled (not bypassed)
 * and it receives packets that considered badtag
 * (see MACSec Header Validation section of the datasheet
 * for more details), it will be dropped by default.
 * This is an expected behavior when MACSec is enabled.
 *
 * The following registers initialization sequence is the minimum
 * MACSec configuration that needs to be done if MACSec is enabled
 * (these register initialization must be done on all ports):
 *
 *------------------------------------------------------------------
 */
static int macsec_test_88e1548p (uint port, uint speed) {

    uint phy_addr, result = FAILED;

    prpass(testpass, "Test port-%d speed-%d, ", port, speed);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;

    /* init macsec setting and cleanup macsec counter(statistic) */
    init_macsec(port, phy_addr);
    clr_macsec_cnt(port, phy_addr);

    /* using internal loopback so far */
    if (set_media_phy_int_lpbk(SEL_PORT_ETH, port, speed)) {
      printf("Port%d set internal loopback failed with spd %d\n",
             port, speed);
      return (FAILED);
    }

    /* this function is negative logic. DISABLE will enable macsec */
    set_macsec(DISABLE, phy_addr);

    /* back to page 0 before send packet. */
    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, PHY_REG(0));
    msleep(1000); 

    /* send packets */
    result = ovld_set_packet(port, speed);

    /* clean up setting of 88E1548P before leaving test. */
    cleanup_setting(port, phy_addr);

    if (result != PASSED) {
        printf("ovld_set_packet failed %s\n",__FUNCTION__);
        return (result);
    }

    result |= check_status(port, phy_addr);

    if (result != PASSED) {
       printf("Statistics have failure case.\n");
       return (result);
    }

    return (result);
}

/*------------------------------------------------------------------
 *
 * Function: macsec_reg_wr
 *
 * Description: macsec register write function which is based on 
 *              88E1548 register read/write
 *
 * Input:  phy_addr - phy address
 *         addr - register addr
 *         offset - register offset
 *         data - the data we write into macsec register. 
 *
 * Output: NONE
 *------------------------------------------------------------------
 */
static void macsec_reg_wr (uint phy_addr, ushort addr, ushort offset, uint data) {

    ushort datahi, datalo, reg_addr;

    datahi = ((data >> 16) & 0xFFFF);
    datalo = (data & 0xFFFF);
    reg_addr = addr + offset;

    ovld_phy_reg_wr(phy_addr, 22, 16);

    ovld_phy_reg_wr(phy_addr, 1, reg_addr);

    ovld_phy_reg_wr(phy_addr, 2, datalo);
    ovld_phy_reg_wr(phy_addr, 3, datahi);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("MACsec addr:wrval = %#.4x:%#.8x ,rd back\n", reg_addr, data);
    }

    macsec_reg_rd(phy_addr, addr, offset);

    ovld_phy_reg_wr(phy_addr, 22, 0);

    return;
}

/*------------------------------------------------------------------
 *
 * Function: macsec_reg_rd
 *
 * Description: macsec register read function which is based on 
 *              88E1548 register read/write
 *
 * Input:  phy_addr - phy address
 *         addr - register addr
 *         offset - register offset
 *
 * Output: rdval - read value of macsec reg
 *------------------------------------------------------------------
 */
static int macsec_reg_rd (uint phy_addr, ushort addr, ushort offset) {

    ushort datahi, datalo, reg_addr;
    uint rdval;

    reg_addr = addr + offset;

    ovld_phy_reg_wr(phy_addr, 22, 16);

    ovld_phy_reg_wr(phy_addr, 0, reg_addr);

    datalo = ovld_phy_reg_rd(phy_addr, 2);
    datahi = ovld_phy_reg_rd(phy_addr, 3);

    rdval = datalo;
    rdval |= (datahi << 16);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("MACsec addr:rdval = %#.4x:%#.8x\n", reg_addr, rdval);
    }

    ovld_phy_reg_wr(phy_addr, 22, 0);

    return (rdval);
}

/*------------------------------------------------------------------
 *
 * Function: init_macsec
 *
 * Description: we configure macsec related setting on its mem/reg.
 *
 * Input:  port - setup port
 *         phy_addr - phy address
 *
 * Output: NONE
 *
 * NOTE: the data we write into macsec registers are based on 
 * Marvell FAE suggestion. 
 * 0x0000 - 0x07FF Port0 registers/memory 
 * 0x0800 - 0x0FFF Port1 registers/memory 
 * 0x1000 - 0x17FF Port2 registers/memory 
 * 0x1800 - 0x1FFF Port3 registers/memory 
 *------------------------------------------------------------------
 */
static void init_macsec (uint port, uint phy_addr) {

    uint offset;

    offset = (GENERAL_PORT_OFFSET * port);
    
    /* disable drop_bad_tag */
    macsec_reg_wr(phy_addr, CRYPT_IGR_GEN, offset, 0xFB40000);

    /* set VFL to only check no drop */
    macsec_reg_wr(phy_addr, CRYPT_ISC_GEN, offset, 0x2);
    
    /* egress default match, encrypt+auth */
    macsec_reg_wr(phy_addr, ELU_TBL_OFF4, offset, 0x40060000); 
    
    /* ingress decrypt+auth and VFL setting*/  
    macsec_reg_wr(phy_addr, ILU_TBL_OFF6, offset, 0x220000); 
    
    /* igr default match */  
    macsec_reg_wr(phy_addr, ILU_TBL_OFF7, offset, 0x4000);   
    
    /* set sci[31:0] and sci [63:32] */
    macsec_reg_wr(phy_addr, EGR_CTXT_OFF0, offset, 0xF00DBEEF);  
    macsec_reg_wr(phy_addr, EGR_CTXT_OFF1, offset, 0xCAFEFEED);  
    
    /* set tci[7:0] */
    macsec_reg_wr(phy_addr, EGR_CTXT_OFF3, offset, 0x2C);  
    
    /* set encrypt key */
    macsec_reg_wr(phy_addr, ENC_KEY_OFF0, offset, 0x102);   
    macsec_reg_wr(phy_addr, ENC_KEY_OFF1, offset, 0x3040500);
    macsec_reg_wr(phy_addr, ENC_KEY_OFF2, offset, 0x0);
    macsec_reg_wr(phy_addr, ENC_KEY_OFF3, offset, 0x0);
    
    /* set egres hash key */
    macsec_reg_wr(phy_addr, EGR_HKEY_OFF0, offset, 0x102);  
    macsec_reg_wr(phy_addr, EGR_HKEY_OFF1, offset, 0x3040500);
    macsec_reg_wr(phy_addr, EGR_HKEY_OFF2, offset, 0x0);
    macsec_reg_wr(phy_addr, EGR_HKEY_OFF3, offset, 0x0);
    
    /*  set decrypt key */
    macsec_reg_wr(phy_addr, DEC_KEY_OFF0, offset, 0x102);
    macsec_reg_wr(phy_addr, DEC_KEY_OFF1, offset, 0x3040500);
    macsec_reg_wr(phy_addr, DEC_KEY_OFF2, offset, 0x0);
    macsec_reg_wr(phy_addr, DEC_KEY_OFF3, offset, 0x0);
    
    /*  set ingress hash key */
    macsec_reg_wr(phy_addr, IGR_HKEY_OFF0, offset, 0x102);  
    macsec_reg_wr(phy_addr, IGR_HKEY_OFF1, offset, 0x3040500);
    macsec_reg_wr(phy_addr, IGR_HKEY_OFF2, offset, 0x0);
    macsec_reg_wr(phy_addr, IGR_HKEY_OFF3, offset, 0x0);

    return;
}

/*------------------------------------------------------------------
 *
 * Function: clr_macsec_cnt
 *
 * Description: cleanup macsec counter(statistic) via reading them.
 *
 * Input:  port - setup port
 *         phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
static void clr_macsec_cnt (uint port, uint phy_addr) {

    uint offset, ia;

    offset = (MACSEC_CNT_OFFSET * port);

    for (ia = 0; ia < MACSEC_CNT_OFFSET; ia++) {
        macsec_reg_rd(phy_addr, (ia+IGR_HIT), offset);
    }
}

/*------------------------------------------------------------------
 *
 * Function: cleanup_setting
 *
 * Description: cleanup current setting before leaving test
 *
 * Input:  port - setup port
 *         phy_addr - phy address 
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
static void cleanup_setting (uint port, uint phy_addr) {

    /* set_media_int_lpbk() is already enabled on 
     * function: set_media_phy_int_lpbk() */ 

    set_macsec(DISABLE, phy_addr);
    set_media_int_lpbk(DISABLE_SIG, port);

    return;
}

/*------------------------------------------------------------------
 *
 * Function: dump_statistic
 *
 * Description: dumping macsec statictic
 *
 * Input:  none
 *
 * Output: PASS/FAILED
 *
 *------------------------------------------------------------------
 */
static void dump_statistic (void) {

    uint phy_addr, port, offset, retval[32];

    port = getdec_answer("Enter port >", SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    offset = (MACSEC_CNT_OFFSET * port);
    phy_addr = port + ADDR_MEDIA_PHY;

    printf("##Dumpping MACsec counter..\n");
    printf("##Some of values are count in bytes\n");
    printf("##IGR_OK and IGR_MISS will be read/clean during testing\n");

    /* checking all the status */
    retval[0] = macsec_reg_rd(phy_addr, IGR_HIT, offset);
    retval[1] = macsec_reg_rd(phy_addr, IGR_UNCHK, offset);
    retval[2] = macsec_reg_rd(phy_addr, IGR_DELAY, offset);
    retval[3] = macsec_reg_rd(phy_addr, IGR_LATE, offset);
    retval[4] = macsec_reg_rd(phy_addr, IGR_INVLD, offset);
    retval[5] = macsec_reg_rd(phy_addr, IGR_NOTVLD, offset);
    retval[6] = macsec_reg_rd(phy_addr, EGR_PKT_PORT, offset);
    retval[7] = macsec_reg_rd(phy_addr, EGR_PKT_ENC, offset);
    retval[8] = macsec_reg_rd(phy_addr, EGR_HIT, offset);
    retval[9] = macsec_reg_rd(phy_addr, IGR_OCT_VAL, offset);
    retval[10] = macsec_reg_rd(phy_addr, IGR_OCT_DEC, offset);
    retval[11] = macsec_reg_rd(phy_addr, IGR_UNTAG, offset);
    retval[12] = macsec_reg_rd(phy_addr, IGR_NOTAG, offset);
    retval[13] = macsec_reg_rd(phy_addr, IGR_BADTAG, offset);
    retval[14] = macsec_reg_rd(phy_addr, IGR_UNKSCI, offset);
    retval[15] = macsec_reg_rd(phy_addr, IGR_NOSCI, offset);
    retval[16] = macsec_reg_rd(phy_addr, IGR_UNUSSA, offset);
    retval[17] = macsec_reg_rd(phy_addr, IGR_NOUSSA, offset);
    retval[18] = macsec_reg_rd(phy_addr, IGR_OCT_TOT, offset);
    retval[19] = macsec_reg_rd(phy_addr, EGR_OCT_PORT, offset);
    retval[20] = macsec_reg_rd(phy_addr, EGR_OCT_ENC, offset);
    retval[21] = macsec_reg_rd(phy_addr, EGR_OCT_TOT, offset);
    retval[22] = macsec_reg_rd(phy_addr, IGR_MISS, offset);
    retval[23] = macsec_reg_rd(phy_addr, EGR_MISS, offset);
    retval[24] = macsec_reg_rd(phy_addr, IGR_REDIR, offset);


    if (retval[0])
      printf("IGR pkts which hit this entry are %d\n", retval[0]);
    if (retval[1]) {
      printf("IGR pkts ICV not checked, but passed replay or ");
      printf("replay was disable are %d\n", retval[1]);
    }
    if (retval[2])
      printf("IGR pkts failed replay check are %d\n", retval[2]);
    if (retval[3])
      printf("IGR pkts failed replay check and dropped are %d\n", retval[3]);
    if (retval[4])
      printf("IGR pkts failed ICV check and dropped are %d\n", retval[4]);

    if (retval[5])
      printf("IGR pkts failed ICV check are %d\n", retval[5]);
    if (retval[6])
      printf("EGR pkts sent auth-only are %d\n", retval[6]);
    if (retval[7])
      printf("EGR pkts sent encrypted are %d\n", retval[7]);
    if (retval[8])
      printf("EGR pkts hit this entry %d\n", retval[8]);
    if (retval[9])
      printf("IGR pkts octet count of authenticated are %d\n", retval[9]);
    if (retval[10])
      printf("IGR pkts octet count of decrypted are %d\n", retval[10]);

    if (retval[11])
      printf("IGR pkts without MACsec tag are %d\n", retval[11]);
    if (retval[12])
      printf("IGR pkts without MACsec tag during STRICT mode are %d\n", retval[12]);
    if (retval[13])
      printf("IGR pkts with incorrect MACsec tag are %d\n", retval[13]);
    if (retval[14])
      printf("IGR pkts SCI not found in lookup table are %d\n", retval[14]);
    if (retval[15])
      printf("IGR pkts SCI not found in lookup table are %d\n", retval[15]);

    if (retval[16])
      printf("IGR pkts SCI found but AN not active are %d\n", retval[16]);
    if (retval[17])
      printf("IGR pkts SCI found but AN not active are %d\n", retval[17]);
    if (retval[18])
      printf("IGR pkts octet count of all received are %d\n", retval[18]);
    if (retval[19])
      printf("EGR pkts octet count of all sent auth-only are %d\n", retval[19]);
    if (retval[20])
      printf("EGR pkts octet count of sent encrypted are %d\n", retval[20]);

    if (retval[21])
      printf("EGR pkts octet count of all sent are %d\n", retval[21]);
    if (retval[22])
      printf("IGR num of pkts did not hit any igr entry are %d\n", retval[22]);
    if (retval[23])
      printf("EGR num of pkts did not hit any egr entry are %d\n", retval[23]);
    if (retval[24])
      printf("IGR num of pkts redirected are %d\n", retval[24]);

}

/*------------------------------------------------------------------
 *
 * Function: check_status
 *
 * Description: read macsec counter to ensure packets are encrypted, 
 *              decrypted and authenticated.
 *
 * Input:  port - setup port
 *         phy_addr - phy address
 *         manually - 1 for chech status manually
 *
 * Output: PASS/FAILED
 *
 *------------------------------------------------------------------
 */
static int check_status (uint port, uint phy_addr) {

    uint offset, result_igrok, result_igrmiss;

    offset = (MACSEC_CNT_OFFSET * port);

    /* check IGR_OK and IGR_MISS */
    result_igrok = macsec_reg_rd(phy_addr, IGR_OK, offset);
    result_igrmiss = macsec_reg_rd(phy_addr, IGR_MISS, offset);

    /* IGR is not OK, packet is not decrypted or authenticated */
    if ((!result_igrok) || (result_igrmiss)) {
      printf("Packets decrypted/authenticated OK num0x%x\n", result_igrok);
      printf("Packets igress missed num0x%x\n", result_igrmiss);

      printf("Packets validation failed. Detailed refer to Dump statistic Utility\n");
  
      return (FAILED);
    } else {
    
      printf("Packets transfer passed.\n");
      return (PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: send_packets
 *
 * Description: for sending packets on specific ports.
 *
 * Input: NONE
 *
 * Output: NONE
 *------------------------------------------------------------------
 */
static int send_packets (int port, int speed, int pkt_cnt) {

    int pkt_len, pkt_val;
    int typ_curr, pkt_type;
    uchar orig_hkpflag = hkeepflags;
    int rc = FAILED;

    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    printf("testing.");
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
    /* each dot '.' means a pattern */
    printf(".");
    fflush(stdout);
         /* set packet */
         pkt_len = pktdata[typ_curr].len;
         pkt_val = pktdata[typ_curr].val;
         hkeepflags |= pktdata[typ_curr].hkpflags;

         /* prepare to send packet */
         rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);

         if (rc != PASSED)
           break;

    } /* typ_curr */

    if (rc != PASSED){
        printf("tx_rx_diag failed Port: %d Speed: %d, rc = %d\n",port, speed, rc);
        show_status_info(port + ADDR_MEDIA_PHY);
        printf("%s failed\n", __FUNCTION__);
    } else {
        printf("Pass\n");
    }

    fflush(stdout);
    hkeepflags = orig_hkpflag;

    return rc;
}


/*******************below is left for utilities *******************/
/*------------------------------------------------------------------
 *
 * Function: macsec_test_88e1548p_util
 *
 * Description: a utility for testing macsec on 88E1548P.
 *              steps: turn on macsec on PHY,
 *              disable drop_bad_tag, send packets and
 *              check the statistic bit on PHY.
 *
 * Input: NONE  
 *
 * Output: PASSED/FAILED
 *------------------------------------------------------------------
 */
static int macsec_test_88e1548p_util (void) {

    uint port, phy_addr, result = FAILED;
    uint spdsel, speed, pkt_cnt;
    char qrybuf[64];

    testname("88E1548p MACsec test");

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS)");
    spdsel = getdec_answer(qrybuf, 0, 0, 1);
    speed = (spdsel == 0) ? SPD_10MBPS : SPD_100MBPS;

    sprintf(qrybuf, "\nEnter packet number (%d - %d)", 1, 100);
    pkt_cnt = getdec_answer(qrybuf, 1, 1, 100);

    prpass(testpass, "Test port-%d speed-%d pkt-%d, ", port, speed, pkt_cnt);


    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;

    /* init macsec setting and cleanup macsec counter(statistic) */
    init_macsec(port, phy_addr);
    clr_macsec_cnt(port, phy_addr);

    /* using internal loopback so far */
    if (set_media_phy_int_lpbk(SEL_PORT_ETH, port, speed)) {
      printf("Port%d set internal loopback failed with spd %d\n",
             port, speed);
      return (FAILED);
    }

    /* this function is negative logic. DISABLE will enable macsec */
    set_macsec(DISABLE, phy_addr);

    /* back to page 0 before send packet. */
    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, PHY_REG(0));
    msleep(100);

    /* send packets */
    result = send_packets(port, speed, pkt_cnt);
    if (result != PASSED) {
      printf("ovld_set_packet failed %s\n",__FUNCTION__);
    }

    result |= check_status(port, phy_addr);

    /* clean up setting of 88E1548P before leaving test. */
    cleanup_setting(port, phy_addr);

    if (result != PASSED) {
       cterr('f',0,"MACsec test failed on port%d with spd%d\n", port, speed);
    }

    return (result);
}

/*------------------------------------------------------------------
 *
 * Function: set_internal_loopback_util
 *
 * Description: set internal loopback 
 *
 * Input: NONE
 *
 * Output: NONE
 *------------------------------------------------------------------
 */
static int set_internal_loopback_util (void) {

    int rc = FAILED, port;
    int spdsel, speed;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS)");
    spdsel = getdec_answer(qrybuf, 0, 0, 1);
    speed = (spdsel == 0) ? SPD_10MBPS : SPD_100MBPS;

    rc = set_media_phy_int_lpbk(SEL_PORT_ETH, port, speed);

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: set_macsec_util
 *
 * Description: en/disable macsec
 *
 * Input: NONE
 *
 * Output: NONE
 *------------------------------------------------------------------
 */
static void set_macsec_util (void) {

    int set, port, phy_addr;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;

    set = getdec_answer("0:Disable 1:Enable", 0, 0, 1);
    set_macsec(set, phy_addr);

    ovld_phy_reg_wr(phy_addr, 22, 0);

    return;
}

/*------------------------------------------------------------------
 *
 * Function: set_drop_util
 *
 * Description: en/disable drop_bad_tag 
 *
 * Input: NONE
 *
 * Output: NONE
 *
 * NOTE: The reason we disable drop_bad_tag bit is to avoid dropping
 * layer2 packet with random content (random content may have etype
 * that looks like macsec etype but do not have proper macsec header
 * and considered bad packet). If sending packets are like ipv4,
 * it should be fine. Since we are not define packets clearly,
 * we using this function.
 *
 *------------------------------------------------------------------
 */
static void set_drop_util (void) {

    uint set, port, phy_addr, offset, igr_gen_reg;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;
    offset = (GENERAL_PORT_OFFSET * port);

    igr_gen_reg = macsec_reg_rd(phy_addr, CRYPT_IGR_GEN, offset);
    set = getdec_answer("1:Enable 0:Disable", 0, 0, 1);

    if (set) {
        igr_gen_reg |= PHY_REG_BIT(29); 
    } else {
        igr_gen_reg &= ~PHY_REG_BIT(29); 
    }

    macsec_reg_wr(phy_addr, CRYPT_IGR_GEN, offset, igr_gen_reg);

    igr_gen_reg = macsec_reg_rd(phy_addr, CRYPT_IGR_GEN, offset);
    if (igr_gen_reg & PHY_REG_BIT(29)) {
      printf("Enable drop bad tag, port%d igr_gen_reg:%#.8x\n", port, igr_gen_reg);
    } else {
      printf("Disable drop bad tag, port%d igr_gen_reg:%#.8x\n", port, igr_gen_reg);
    }

    return;
}

/*------------------------------------------------------------------
 *
 * Function: send_packet_util
 *
 * Description: called the function on platfrom_ext_lpbk.c 
 *              for sending packets on specific ports.
 *
 * Input: NONE
 *
 * Output: NONE
 *------------------------------------------------------------------
 */
static int send_packet_util (void) {

    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    uchar orig_hkpflag = hkeepflags;
   
    int rc = FAILED, port;
    int spdsel, speed;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS)");
    spdsel = getdec_answer(qrybuf, 0, 0, 1);
    speed = (spdsel == 0) ? SPD_10MBPS : SPD_100MBPS;

    sprintf(qrybuf, "\nEnter packet number (%d - %d)", 1, 100);
    pkt_cnt = getdec_answer(qrybuf, 1, 1, 100);

    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    printf("testing.");
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
    /* each dot '.' means a pattern */
    printf(".");
    fflush(stdout);
         /* set packet */
         pkt_len = pktdata[typ_curr].len;
         pkt_val = pktdata[typ_curr].val;
         hkeepflags |= pktdata[typ_curr].hkpflags;

         /* prepare to send packet */
         rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);

         if (rc != PASSED)
           break;

    } /* typ_curr */

    if (rc != PASSED){
        printf("tx_rx_diag failed Port: %d Speed: %d, rc = %d\n",port, speed, rc);
        show_status_info(port + ADDR_MEDIA_PHY);
        printf("%s failed\n", __FUNCTION__);
    } else {
        printf("Pass\n");
    }

    fflush(stdout);
    hkeepflags = orig_hkpflag;

    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: read_reg_util
 *
 * Description: for user to read macsec register.
 *
 * Input: NONE
 *
 * Output: NONE
 *------------------------------------------------------------------
 */
static void read_reg_util (void) {

    ushort port, phy_addr, regnum, regnum_max = 0xFFFF;
    uint tempval, offset;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;
    offset = (GENERAL_PORT_OFFSET * port);

    do {

        regnum = gethex_answer("\nEnter PHY reg number", 0, 0, regnum_max);
        tempval = macsec_reg_rd(phy_addr, regnum, offset);

        printf("Port%d reg%d = %#.8x\n", port, regnum ,tempval);

    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return;
}

/*------------------------------------------------------------------
 *
 * Function: write_reg_util
 *
 * Description: for user to write macsec register.
 *
 * Input: NONE
 *
 * Output: NONE
 *------------------------------------------------------------------
*/
static void write_reg_util (void) {

    ushort port, phy_addr, regnum, regnum_max = 0xFFFF;
    uint rdval, wrval, offset;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;
    offset = (GENERAL_PORT_OFFSET * port);

    do {

        regnum = gethex_answer("\nEnter PHY reg number", 0, 0, regnum_max);
        wrval = gethex_answer("\nInput data", 0, 0, 0xFFFFFFFF);

        macsec_reg_wr(phy_addr, regnum, offset, wrval);

        rdval = macsec_reg_rd(phy_addr, regnum, offset);

        printf("WR%#.8x on port%d reg%d RDback%#.8x\n",
                wrval, port, regnum ,rdval);

    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return; 
}

/*------------------------------------------------------------------
 *
 * Function: init_macsec_util
 *
 * Description: write decrypt, encrypt and hash key on macsec regs. 
 *
 * Input:  NONE
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
static void init_macsec_util (void) {

    ushort port, phy_addr;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;

    init_macsec(port, phy_addr);
    return;
}

/*------------------------------------------------------------------
 *
 * Function: clr_macsec_cnt_util
 *
 * Description: cleanup macsec counter(statistic) via reading them.
 *
 * Input:  NONE
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
static void clr_macsec_cnt_util (void) {

    ushort port, phy_addr;
    char qrybuf[64];

    sprintf(qrybuf, "\nEnter port number (%d - %d)", SGMII0, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, SGMII0, SGMII0, PLAT_SGMII_NUM_MAX);

    /* add offset for PHY addr */
    phy_addr = port + ADDR_MEDIA_PHY;

    clr_macsec_cnt(port, phy_addr);
    return;
}

/*
$Log: platform_macsec.c,v $
Revision 1.2  2017/07/14 02:51:39  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.1  2013/05/31 11:03:41  alpeng
support front panel GE loopback test

Revision 1.1  2013/01/25 10:47:02  alpeng
support macsec util


$Endlog$
*/

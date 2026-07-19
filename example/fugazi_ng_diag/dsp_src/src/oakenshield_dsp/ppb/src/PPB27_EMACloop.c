/* $Id: PPB27_EMACloop.c,v 1.2 2017/07/28 07:58:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/PPB27_EMACloop.c,v $
 *------------------------------------------------------------------
 * PPB27_EMACloop.c
 *     Ethernet functions 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c)2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*****************************************************************************
 *
 * Copyright (c) 2011 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * $RCSfile: PPB27_EMACloop.c,v $
 *
 * Description:	MAC0_TXD=>MAC1_PCE SS-SMII or SGMII loopback (need external cable)
 *
 * NOTE: This test uses the SS-SMII ports on the SP2704 RabbitEars Eval Board with SS-SMII PHY0
 *       connected to SS-SMII PHY1 via a CAT5 cable. It can also be run using the
 *       SerDes Loopback capability of the SGMII MACs with or without a cable.
 *
 * Valid UDP packets are generated and sent out on both MAC0 (UDP port = 4000)
 * and MAC1 (UDP port = 4001.) Valid UDP packets with port 4000 have a recirculating
 * serial number in the range 0->7. Valid UDP packets with port 4001 have a
 * recirculating serial number in the range 0->7 << 4 (i.e. 0x0, 0x10, 0x20 .. 0x70)
 *
 * Both PCE0 and PCE1 are configured to route UDP packets with port number 4000 to
 * the DSS0 queue and route UDP packets with port number 4001 to the DSS1 queue.
 * All other packets will be sent to the PPB queue.
 *
 * All queues are then scoured for packets. Each packet from a DSS queue has it's
 * serial number displayed on the 8 LEDs connected to the PPB GPIOs. All other
 * packets are sorted by error (see EMAC_ERR_* in libsp27ppb/ethernet/emac27_hal.h) and
 * an 'all ones' pattern is displayed on the PPB LEDs.
 *
 * For example, removing the ethernet cable will stop packet flow (and subsequent
 * counting on the PPB LEDs.) Reinsertion will restart packet flow but first a
 * an error response (all PPB LEDs on) may occur followed by likely some 'out of
 * sequence' packets.
 *
 * HARDWARE SETUP AND OPERATION (MAC_TYPE == MAC_SGMII):
 *
 * ---- If INTERNAL_LOOPBACK 1 is defined ----
 * -> No Ethernet cables needed
 * -> All packets are sent and received via SerDes serial loopbacks
 *
 * ---- If INTERNAL LOOPBACK 1 is not defined ----
 *
 * -- SP2704 RabbitEars Eval Board --
 * -> Insert Ethernet loopback cable into GIGE port 0
 * -> All packets on GIGE0 are looped back via cable and all packets on GIGE1 are looped back via SerDes serial loopback
 *
 * -- SP2704 GreenTea Software Development Board --
 * -> Insert one end of Ethernet cable into the GIGE0 port and one end into GIGE1 port
 * -> All packets are exchanged between GIGE0 and GIGE1 via cable
 *
 * -- SP2716 TigerPaws Eval Board --
 * -> No ethernet cables needed
 * -> Only internal loopbacks are tested on this board type
 * -> All packets are sent and received via SerDes serial loopbacks
 *
 * HARDWARE SETUP AND OPERATION (MAC_TYPE == MAC_SSSMII):
 *
 * -> Insert one end of Ethernet cable into the SSSMII0 port and one end into SSSMII1 port
 * -> All packets are exchanged between ports 0 and 1 via cable
 * -> Internal loopbacks cannot be performed on SS-SMII ports; leaving INTERNAL_LOOPBACK 1 defined will be ignored
 *
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include "lsi_sp27xx_reg.h"
#include "diag_ppb.h"
/* PPB library header files */
#include "libarmtimer.h"
#include "libgpio.h"
#include "libgeneric.h"
#include "libeth.h"
#include "libuart.h"
#include "libserdes.h"
#include "uart.h"
#include "debug_console.h"
#include "common.h"
#include "diag_common.h"

#ifdef __SEMIHOSTING_SUPPORT__
#include <stdio.h>
#endif

#define MAC_SSSMII 0
#define MAC_SGMII 1
#define MAC_TYPE	MAC_SGMII

/* Align the TX BDs to 8 bytes or 64 bit boundary */
TXD_BQUE TxBuffDes0[BuffTxElements] __attribute__ ((aligned(8)));
TXD_BQUE TxBuffDes1[BuffTxElements] __attribute__ ((aligned(8)));

/* Create a large array to receive the data */
RX_BQUE RxBuffDes0[BuffRxElements] __attribute__ ((aligned(8)));
RX_BQUE RxBuffDes1[BuffRxElements] __attribute__ ((aligned(8)));

/* Create a large array to receive the data */
uint8_t rx_frame_array0[BuffRxElements*RCV_BUF_MAX_SIZE] __attribute__((aligned(4*1024)));
uint8_t rx_frame_array1[BuffRxElements*RCV_BUF_MAX_SIZE] __attribute__((aligned(4*1024)));

////SR orig uint8_t out_frame[3][RCV_BUF_MAX_SIZE*2] __attribute__((aligned(4*1024)));
uint8_t out_frame[3][2048] __attribute__((aligned(4*1024)));
//SR orig uint8_t	in_frame[2][RCV_BUF_MAX_SIZE*2] __attribute__((aligned(16)));
uint8_t	in_frame[2][1024*2] __attribute__((aligned(16)));

extern void do_mem_md(char* cmdargs);
extern void bsp_debug_printf (const char *fmt, ...);
extern volatile dspif_info_t *hd_if;

static uint32_t cmp_pkt(uint8_t *obuf, uint32_t osize, uint8_t *inbuf, uint32_t insize);


uint32_t sp_SendTx(int, uint32_t, uint8_t *, uint32_t, uint32_t);
uint32_t sp_RecvRx (int, uint32_t *size, uint32_t total_size);

/*----------------------------------*/
/*		DEFAULT VALUES				*/
/*----------------------------------*/
/* EMAC0 default configuration */
#if 0 /* change to current settings */
#define SP_PPB_EMAC0_ETH_ADDR		(0x00FACE270306LL)
#define SP_PPB_EMAC0_CTRL_IP_ADDR_DFLT		SP_PPB_IPv4_ADDR(192,168,0,6)

#define SP_PPB_EMAC1_ETH_ADDR		(0x00FACE270301LL)
#define SP_PPB_EMAC1_CTRL_IP_ADDR_DFLT		SP_PPB_IPv4_ADDR(192,168,0,1)
#endif
#define SP_PPB_EMAC0_ETH_ADDR		(0x001122334477LL)
#define SP_PPB_EMAC0_CTRL_IP_ADDR_DFLT		SP_PPB_IPv4_ADDR(192,123,123,50)

#define SP_PPB_EMAC1_ETH_ADDR		(0x00FACE270406LL)
#define SP_PPB_EMAC1_CTRL_IP_ADDR_DFLT		SP_PPB_IPv4_ADDR(192,123,123,51)

MAC_ADDR eth0_macaddr[2] = {SP_PPB_EMAC0_ETH_ADDR/* local mac addr */, 0/* remote mac addr */};
MAC_ADDR eth1_macaddr[2] = {SP_PPB_EMAC1_ETH_ADDR/* local mac addr */, 0/* remote mac addr */};

/******************************************************************************
 *
 * Function to generate a pattern of data
 * This will return address of where the data is
 *
 ******************************************************************************/

const uint8_t frame_template[128]=
 {
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
};

/* assure proper buffer alignment. From section 4.7.7.6.1 'TXD Buffers and 
 * Buffer Descriptors' Buffer address must be aligned on the word boundary 
 * (3 LSB bits of address must be 0).
 * Additionally, buffers must be aligned such that an AXI burst does not 
 * cross a 4K address boundary. This can be accomplished by keeping the 
 * entire buffer within a 4K page or by ensuring the alignment to the burst 
 * size (128 byte bursts are used for buffers with length longer than 128).
 */

void countLED(uint32_t count) 
{
	GPIO_REG->data[255].reg = count;
#ifdef PALLADIUM
	lsi_mg_delay(100);
#else
	lsi_mg_delay(80000);
#endif
}

void loc_error(uint32_t errnum) 
{
	GPIO_REG->data[255].reg = 0xff;
#ifdef PALLADIUM
	lsi_mg_delay(100);
#else
	lsi_mg_delay(80000);
#endif
}

/*  ---------------------- START MAIN ----------------------  */

int rcv_cnt, snd_cnt;
int start_discard, end_discard;
volatile uint32_t m1cnt1, m0cnt1;
int loopSelf;

int ethernet_test (uint32_t set_pll, int lpbk, int which_emac)
{
    uint32_t status;
    uint32_t fin, size, count;
    //uint32_t error, output_divider, multiplier;
    uint8_t i;
    uint8_t frame_temp[2048];
    unsigned char print_mac[6], lpstr[15];

    SP27XXEvalType_t board;

    phyMode_t phyMode;
    unsigned short pak_size[5] = {1024, 108, 256, 65, 83};

    if (which_emac == EMAC0) {
        print_mac[0] = (eth0_macaddr[0] >> 40);
        print_mac[1] = (eth0_macaddr[0] >> 32);
        print_mac[2] = (eth0_macaddr[0] >> 24);
        print_mac[3] = (eth0_macaddr[0] >> 16);
        print_mac[4] = (eth0_macaddr[0] >> 8);
        print_mac[5] = (eth0_macaddr[0] >> 0);

    } else {
        print_mac[0] = (eth1_macaddr[0] >> 40);
        print_mac[1] = (eth1_macaddr[0] >> 32);
        print_mac[2] = (eth1_macaddr[0] >> 24);
        print_mac[3] = (eth1_macaddr[0] >> 16);
        print_mac[4] = (eth1_macaddr[0] >> 8);
        print_mac[5] = (eth1_macaddr[0] >> 0);
 
    }


#ifdef PCE_DEBUG
    sp_SerialPutS("\r\n Oakenshield MAC is ");
    bsp_debug_printf("0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", print_mac[0],print_mac[1],
                       print_mac[2], print_mac[3], print_mac[4], print_mac[5]);
    sp_SerialPutS(" \r\n PCE0_REG = ");
    sp_SerialPutLong((int)PCE0_REG, 'h');
#endif
    sp_SerialPutS(" \r\n 1. Reset both Ethernet0 and 1 sp_EthSoftRst()");
    sp_EthSoftRst();

    board = sp_check27XXboardConfig();

    fin = 50;
    status = sp_GigeSerdesInit(GBAUD_1_25, CHAN(0)|CHAN(1), fin);

    if (status != SUCCESS) {
        sp_SerialPutS(" \r\n*** ERROR sp_GigeSerdesInit()");
        PRINT_STR("***Error while setting up Serdes\r");
        lsi_mg_report(status);
    } else {
        sp_SerialPutS(" \r\n 2. sp_GigeSerdesInit()");
    }
    if (lpbk == INTERNAL) {
        sprintf((char *)lpstr, "Internal");
        loopSelf = 1;

        sp_SerialPutS(" \r\n 3. sp_GigeSerdesSetupLB()");
        sp_GigeSerdesSetupLB(CHAN(0)|CHAN(1), SERIAL_LOOPBACK, fin);
 
#ifdef PCE_DEBUG
        sp_SerialPutS(" \r\n sp_GigeSerdesSetupLB()");
        bsp_debug_printf("\r\n EMAC = %d, Setup Internal lpbk ", EMAC0);
#endif
    } else {
        sprintf((char *)lpstr, "External");
    }
    phyMode = PHYMODE_NO_PHY;

    status = sp_EthSetupMAC(which_emac, SGMII, phyMode);
    if (status != SUCCESS) {
        sp_SerialPutS(" \r\n*** ERROR sp_EthSetupMAC()");

        PRINT_STR("*** Error while setting up MAC0\r");
        lsi_mg_report(status);
    }
    sp_SerialPutS(" \r\n 4. sp_EthSetupMAC()");


    /************* Start of some PCE setup *********************/

#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n EMAC port = ");
    sp_SerialPutLong((int)EMAC0, 'd');
    bsp_debug_printf(" \r\n rx_frame_array0 addr = 0x%x, # elements %d, Buffer size 0x%x", &rx_frame_array0[0], BuffRxElements, RCV_BUF_MAX_SIZE);
    sp_SerialPutS(" \r\n RxBuffDes0 addr = ");
    sp_SerialPutLong((int)RxBuffDes0, 'h');
#endif
    /* SR ?? Please check the eth0_mac */
    if (which_emac == EMAC0) {
    status = sp_EthSetupPCE(which_emac, NO_FORWARD, &eth0_macaddr[0],
                            0 /* dlt_init on */, 0 /* don't use DLTB */, 
                            NORMAL_ROUTING, 0 /* classification on */, 
                            (void *) RxBuffDes0, BuffRxElements, &rx_frame_array0[0]);
    } else {
    status = sp_EthSetupPCE(which_emac, NO_FORWARD, &eth1_macaddr[0],
                            0 /* dlt_init on */, 0 /* don't use DLTB */, 
                            NORMAL_ROUTING, 0 /* classification on */, 
                            (void *) RxBuffDes1, BuffRxElements, &rx_frame_array1[0]);
    }    

    if(status == ERROR)
    {
        sp_SerialPutS(" \r\n*** ERROR sp_EthSetupPCE()");

        PRINT_STR("***Error while setting up PCE0\r");
        lsi_mg_report(status);
    }

    sp_SerialPutS(" \r\n 5. sp_EthSetupPCE()");
    if (which_emac == EMAC0) {
        status = sp_EthSetupTXD(which_emac, EXTERNAL_FWD_PATH, (void *) TxBuffDes0, BuffTxElements);
    } else {
        status = sp_EthSetupTXD(which_emac, EXTERNAL_FWD_PATH, (void *) TxBuffDes1, BuffTxElements);
    }
    if(status == ERROR) {
        sp_SerialPutS(" \r\n*** ERROR sp_EthSetupTXD()");
        PRINT_STR("***Error while setting up TXD0\r");
        lsi_mg_report(status);
    }

    sp_SerialPutS(" \r\n 6. sp_EthSetupTXD()");
    emac_enable(which_emac);
    sp_SerialPutS(" \r\n 7. emac_enable()");

    if ((lpbk == INTERNAL) || (lpbk == EXTERNAL)) {
        for (i=0; i <1; i++) {
            for (count = 0; count < pak_size[i]; count++) {
                frame_temp[count] = count;
            }

            if (sp_SendTx (which_emac, lpbk, (uint8_t *)frame_temp, pak_size[i], 
                           pak_size[i]+42) != SUCCESS) {
                sprintf((char *)&(hd_if->errmsg[0]), "*** Failed to Transmit packet %d of size %d", i, pak_size[i]);
                uart_puts((char *)hd_if->errmsg); 

                cterr('f', 0, (char *)hd_if->errmsg);
                return (FAILED);
            }
            /* Add some delay here */
            lsi_mg_delay(1000);

            status = sp_RecvRx (which_emac, &size, 0);
            if (status != EMAC_SUCCESS) {
                bsp_debug_printf("\r\n*** %s loopback failed for packet %d", lpstr, i);
            } else {
                bsp_debug_printf("\r\n %s loopback pkt %d received size = %d", lpstr, i, size);
            }
            if (cmp_pkt((uint8_t *)&out_frame[1], pak_size[i], (uint8_t *)&in_frame[0], size) != SUCCESS) {
                sprintf((char *)&(hd_if->errmsg[0]), "\r\n*** Ethernet %s loopback failed for packet %d", lpstr, i); 
                uart_puts((char *)hd_if->errmsg);
                cterr('f', 0, (char *)hd_if->errmsg);
                return (FAILED);
            }
        }
    } else {
        ;
        //sp_SendTx (0, (uint8_t *)frame_template, 64, 64+42);
    }
    return (status);

}

uint32_t cmp_pkt (uint8_t *obuf, uint32_t osize, uint8_t *inbuf, uint32_t insize)
{
    uint32_t count;
    char mem_disp[200];
    char omem_disp[200];

    if ((osize+42) != insize) {
        bsp_debug_printf("\r\n*** pkt transmit size %d does not match pkt recv size %d",
            osize, insize);
        return (ERROR);
    }
    /* Now compare data */
    for (count = 42; count < osize; count++) {
        if (obuf[count] != inbuf[count]) {
            bsp_debug_printf("\r\n*** pkt mismatch at 0x%x expect 0x%x received 0x%x\r\n Received pkt:",
                              count, obuf[count], inbuf[count]);
            sprintf(mem_disp, "0x%x %d", (unsigned int)inbuf, (int)osize);
            sp_SerialPutS("\r\n");
            do_mem_md(mem_disp);
            bsp_debug_printf("\r\n\r\n Tx pkt:");
            sprintf(omem_disp, "0x%x %d", (unsigned int)obuf, (int)osize);
            sp_SerialPutS("\r\n");
            do_mem_md(omem_disp);
            return (ERROR);
        }
    }
    return (SUCCESS);
}

uint32_t sp_RecvRx (int which_emac, uint32_t *size, uint32_t total_size)
{
    int rcv_cnt,  status, ii, jj;;
    volatile uint32_t m0cnt1, *bufAddr;
    uint8_t *tp = (uint8_t *)out_frame;

#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n MAC0_REG = ");
    sp_SerialPutLong((int)MAC0_REG, 'h');
#endif

    if (which_emac == EMAC0) {
        m0cnt1 = MAC0_REG->mac_cnt1.reg;
    } else {
        m0cnt1 = MAC1_REG->mac_cnt1.reg;
    }

    tp = (uint8_t *)&(in_frame[0]);
#ifdef PCE_DEBUG
    sp_SerialPutS("\r\n In Frame @addr : ");
    sp_SerialPutLong((int)tp, 'h');
    sp_SerialPutS("\r\n ");
#endif

    rcv_cnt = 0;
    for (ii = which_emac; ii < which_emac+1; ii++) {
        for (jj = 0; jj <= DLT_DEST_MAX; jj++) {
            while (1) {
                status = sp_EthRxRawPkt(ii, jj, (uint32_t*)size, in_frame[0]);
                if (status == EMAC_SUCCESS){
                    bsp_debug_printf("\r\n Status = EMAC_SUCCESS for DLT Q %d,\
                                     size of pkt = %d", jj, (int)*size);
                    rcv_cnt++;
                    bufAddr = (uint32_t *) in_frame;
                    if (which_emac == EMAC0) { 
                        end_discard = PCE0_REG->pce_bdl_empty_discard_cnt.reg;
                    } else {
                        end_discard = PCE0_REG->pce_bdl_empty_discard_cnt.reg;
                    }
                    return (status); 
                } else if (status == EMAC_ERR_NO_PKT) {
                    sp_SerialPutS("\r\n Status = EMAC_ERR_NO_PKT for DLT Q = ");
                    sp_SerialPutLong(jj, 'd');
                    break;
                } else {
                    loc_error(status);
                    sp_SerialPutS("\r\n Status = ERRORS, for DLT Q = ");
                    sp_SerialPutLong(jj, 'd');
                }
            }
        }
    }
    if (which_emac == EMAC0) {
        end_discard = PCE0_REG->pce_bdl_empty_discard_cnt.reg;
    } else {
        end_discard = PCE1_REG->pce_bdl_empty_discard_cnt.reg;
    }
    return (status);

}

ge_packet_t   st_packet  __attribute__((aligned(4*1024)));
extern uint8_t src_macaddr[6];
extern uint8_t dest_macaddr[6];

uint32_t sp_SendTx (int which_emac, uint32_t lpbk, uint8_t *hwAddr, 
                    uint32_t size, uint32_t total_size)
{
    uint32_t i, serial, status, result;
    uint8_t *srcmac_p, *destmac_p;
    ge_packet_t   *pkt__p;

#ifdef PCE_DEBUG
    char mem_disp[200];
#endif
    pkt__p = (ge_packet_t *) out_frame[1];

    if (lpbk == EXTERNAL) {
        srcmac_p = (uint8_t *)&src_macaddr; /* set in diag_init_mac.c */
        destmac_p = (uint8_t *)&dest_macaddr; /* set in diag_init_mac.c */
        for (i = 0; i < sizeof(mac_address_t); ++i) {
            pkt__p->pkt_hdr.dest_addr[i] = (uint8_t)*destmac_p; /* send as boardcast */
            pkt__p->pkt_hdr.src_addr[i] = (uint8_t)*srcmac_p; /* DSP MAC addr - which DSP???? */
            srcmac_p++;
            destmac_p++;
        }
        pkt__p->pkt_hdr.pkt_len = SWAP16(size);
        memcpy(((uint8_t *)&(pkt__p->pkt_data) - 2), (const uint8_t *)hwAddr, size);
        srcmac_p = (uint8_t *)pkt__p;
        bsp_debug_printf("\r\n In sp_SendTx()");
        for (i=0;i<60;i++) {
            bsp_debug_printf("0x%x ", srcmac_p[i]);
        }

    } else if ((lpbk != INTERNAL) && (lpbk != EXTERNAL))  {
#if 0
        status = sp_EthBuildUdpPktIPv4(0xFFFFFFFFFFFFLL, /* DEST MAC ADDRESS */
                                   0x005555555555LL, /* SRC MAC ADDRESS */
                                   (192<<24)|(123<<16)|(123<<8)|(1<<0), /* DEST IP */
                                   (192<<24)|(123<<16)|(123<<8)|(50<<0), /* SRC IP */
                                   4001, /* DST UDP PORT */ 4010, /* SRC UDP PORT */
                                   hwAddr,/* payload */ size,/* size of payload */
                                   //out);
                                   out_frame[1]);/* area where pkt will be built */
#endif
        /* insert checksum into packet */
        StarProPPB_UTILS_memcpy8((uint8_t*) out_frame[1], hwAddr, size);
    } else if (lpbk == INTERNAL) {
        if (which_emac == EMAC0) {
            status = sp_EthBuildUdpPktIPv4(0x04c5a496cf4aLL, /* DEST MAC ADDRESS */
                                   0x005555555555LL, /* SRC MAC ADDRESS */
                                   (192<<24)|(123<<16)|(123<<8)|(50<<0), /* DEST IP */
                                   (192<<24)|(123<<16)|(123<<8)|(1<<0), /* SRC IP */
                                   4000, /* DST UDP PORT */ 4010, /* SRC UDP PORT */
                                   hwAddr,/* payload */ size,/* size of payload */
                                   //out);
                                   out_frame[1]);/* area where pkt will be built */
        } else {
            status = sp_EthBuildUdpPktIPv4(0x001122334477LL, /* DEST MAC ADDRESS */
                                   0x005555555555LL, /* SRC MAC ADDRESS */
                                   (192<<24)|(123<<16)|(123<<8)|(50<<0), /* DEST IP */
                                   (192<<24)|(123<<16)|(123<<8)|(1<<0), /* SRC IP */
                                   4001, /* DST UDP PORT */ 4010, /* SRC UDP PORT */
                                   hwAddr,/* payload */ size,/* size of payload */
                                   //out);
                                   out_frame[1]);/* area where pkt will be built */
        }
        if (status) {
            PRINT_STR("*** Error while building up a udp packet\r");
            lsi_mg_report(status);
        }
    }
#ifdef PCE_DEBUG
    bsp_debug_printf("\r\n sp_SendTx()\r\n out_frame addr = 0x%x, size = %d\n", out_frame[1], size);
    sp_SerialPutS("\r\n First 60 bytes:\r\n");
    sprintf(mem_disp, "0x%x %d",(unsigned int)out_frame[1], 60);
    sp_SerialPutS(mem_disp);
    sp_SerialPutS("\r\n");
    do_mem_md(mem_disp);
#endif

    if (lpbk == INTERNAL) {
         pce_set_udp_dest_max_min_porta(which_emac, 6047, 4000);

         status = sp_EthSetupDLTEntry(which_emac, DLT_DEST_DSS0, 4000, 4000);
        if (status != EMAC_SUCCESS) {
            PRINT_STR("*** Error while setting up DLT table entry\r");
            lsi_mg_report(status);
        }
#ifdef PCE_DEBUG
        sp_SerialPutS(" \r\n sp_EthSetupDLTEntry()");
#endif

        status = sp_EthSetupDLTEntry(which_emac, DLT_DEST_DSS1, 4001, 4001);
        if (status != EMAC_SUCCESS) {
            PRINT_STR("*** Error while setting up DLT table entry\r");
            lsi_mg_report(status);
        }
#ifdef PCE_DEBUG
        sp_SerialPutS(" \r\n sp_EthSetupDLTEntry() DLT_DEST_DSS1 ");
#endif
        if (loopSelf) {
            /* poke the 'serial number' into the outgoing frames & update the UDP checksums */
            * (uint32_t *) (&out_frame[1][44]) = serial;
            //sp_EthAddUdpChksum(out_frame[0], udp_payload0_sz);
        }
        sp_EthAddUdpChksum(out_frame[1], size);
    }

    /* link frames into TXD1 ouput buffer queue */
    if (which_emac == EMAC0) {
        if ((result = sp_EthRegRawPktToTxq(which_emac, TXD_Q0, out_frame[1],
                                       total_size)) != 0) {
            /* retry or error response here */
            lsi_mg_report(result);
            result = ERROR;
        }
        status = sp_EthTxStart(which_emac, TXD_Q0);
        if(status != EMAC_SUCCESS)
        {
            PRINT_STR("*** Error while transmitting pkt(s)\r");
            //lsi_mg_report(result);
            result = ERROR;
        } else {
            result = SUCCESS;
        }
    } else {
        if ((result = sp_EthRegRawPktToTxq(which_emac, TXD_Q1, out_frame[1],
                                       total_size)) != 0) {
            /* retry or error response here */
            lsi_mg_report(result);
            result = ERROR;
        }
        status = sp_EthTxStart(which_emac, TXD_Q1);
        if(status != EMAC_SUCCESS)
        {
            PRINT_STR("*** Error while transmitting pkt(s)\r");
            //lsi_mg_report(result);
            result = ERROR;
        } else {
#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n sp_EthTxStart() Success ");
#endif
            result = SUCCESS;
        }
    }
#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n sp_EthTxStart()");
#endif
    if (which_emac == EMAC0) 
        PCE0_REG->pce_bdl_empty_discard_cnt.reg;
    else
        PCE1_REG->pce_bdl_empty_discard_cnt.reg;
    return (result);

}

/******** History ********
$Log: PPB27_EMACloop.c,v $
Revision 1.2  2017/07/28 07:58:50  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.6.84.2  2017/04/26 01:58:29  harrchan
Optimize oakenshield  FXS calibration

Revision 1.6.84.1  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.6  2012/08/15 15:02:56  srane
Add support for EMAC1 loopback test.

Revision 1.5  2012/07/17 20:34:38  srane
cleanup

Revision 1.4  2012/06/28 21:25:56  srane
fix TDM isr, add delay for ethernet loopback etc

Revision 1.3  2012/06/07 22:50:59  srane
TDM external loopback, ECC memory test

Revision 1.2  2012/05/10 22:57:58  srane
Add TDM support. Adjust the linker sections.

Revision 1.1  2012/04/18 09:44:02  srane
Initial checkin


$Endlog$
*/


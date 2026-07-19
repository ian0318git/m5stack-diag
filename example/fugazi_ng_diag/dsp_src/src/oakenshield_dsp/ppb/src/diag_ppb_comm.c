/* $Id: diag_ppb_comm.c,v 1.3 2021/04/15 00:53:07 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/diag_ppb_comm.c,v $
 *------------------------------------------------------------------
 * diag_ppb_comm.c
 *      Oakenshield host-dsp communciation functions.
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "diag_ppb.h"
#include "common.h"
#include "diag_common.h"
#include "diag_ver.h"
#include "libeth.h"   
#include "libuart.h"   
#include "uart.h"   

/* Ethernet frame buffer size	*/
#define ETH_FRAME_BUF_SIZE			1514
#define PHOENIX_CPLD_UPGRADE_SIZE   0x10000

dspif_ether_t test_msg;
dspif_ether_t *test_msg_p = &test_msg;
dspif_ether_t result_msg;
dspif_ether_t *result_msg_p = &result_msg;
dspif_ready_t ready_msg;
dspif_ready_t *ready_msg_p =  &ready_msg;
dspif_lpbk_t  lpbk_msg;
dspif_lpbk_t  *lpbk_msg_p = &lpbk_msg;
dspif_mem_t  mem_msg;
dspif_mem_t  *mem_msg_p = &mem_msg;
fpga_packet_t fpga_re_packet;
fpga_packet_t *fpga_re_packet_p = &fpga_re_packet;
fpga_packet_t fpga_tx_packet;
fpga_packet_t *fpga_tx_packet_p = &fpga_tx_packet;
ge_packet_t   host_packet  __attribute__((aligned(4*1024)));
ge_packet_t   *pkt_p = &host_packet;

extern int dc_slot;
extern dsp_type_t *dsp_device_p;
extern dspif_ether_t *hostif_msg_p;
extern dsp_type_t *dsp_device_p;
extern dspid_t *dspid_p;
extern dspif_info_t *hd_if; /* this will get initialized with core interface */
extern uint8_t src_macaddr[6];
extern uint8_t dest_macaddr[6];
extern dspif_info_t *ppb_dss0_if;
extern dspif_info_t *ppb_dss1_if;
extern dspif_info_t *ppb_dss2_if;
extern unsigned long menu_display;

extern uint8_t in_frame[2][1024*2] __attribute__((aligned(16)));
extern void bsp_debug_printf (const char *fmt, ...);
extern uint32_t sp_SendTx(int which_emac, uint32_t lpbk, uint8_t *hwAddr, uint32_t size, uint32_t);
extern void msleep(uint32_t);

uint8_t fpga_image[] __attribute__((section(".fpga_image"))) = {};
uint32_t fpga_image_size = 0x100000;

/***********************************************************************
 *
 * Function: wait_host_msg
 *
 * Description: waiting for host to send commands 
 *
 * Input : type - select field check in the received pkt
 *
 * Returns: NULL - no pkt received, pkt received not from expected host,
 *                 pkt received not of the type we expected, pkt received
 *                 not for us (dest_id).
 *          received packet pointer - after the dest mac, src mac and 
 *                                    length field.
 *
 **********************************************************************
 */
uint8_t *wait_host_msg (int type) 
{
    uint8_t *pkt_ptr;
    int status;
    uint32_t	pkt_size;
    dspif_ether_t *host_if_p;

    pkt_ptr = (uint8_t *)test_msg_p;

    status = sp_EthRxRawPkt(EMAC0, 0 /* DQ */, (uint32_t*)&pkt_size, in_frame[0]);

    /* Host sends a etheret L2 packet not a UDP packet. Hence status will be
       UDP checksum error */
    if ((status == EMAC_ERR_UDP_CKSUM) || (status == EMAC_SUCCESS)) {
        uart_puts("\r\n Received SUCCESS Frame in wait_host_msg pkt size = ");
        uart_put_long(pkt_size, 10);
        StarProPPB_UTILS_memcpy8((uint8_t *)pkt_ptr,
            ((const uint8_t *)(in_frame[0]) + 14), pkt_size);
        uart_puts("\r\n After copy the frame to local buffer");
        host_if_p = (dspif_ether_t *)pkt_ptr;
        /* check if this message is from host */
        /* Plese do the received pkt checks in the following order.
           1. Is message from expected host_id ?
           2. Are we looking for a certain pkt type ? if so did we receive it?
           3. Is the pkt we expect of type READY then return.
           4. IS the dest_id of the received pkt same as ours.
         */
        if (SWAP32(host_if_p->dspif_hdr.src_id) != HOST_ID) {
            uart_puts("\r\n src_id not equal to HOST_ID");
            uart_puts("\r\n HOST_ID=0xFACEFEED , src_id =  ");
            uart_put_long(SWAP32(host_if_p->dspif_hdr.src_id), 16);
            pkt_ptr = NULL;
        }
        if (type != 0) /* Looking for a specific type of packet */ {
            if (host_if_p->dspif_info.select != type ) {
                uart_puts("\r\n Expected command ");
                uart_put_long(type, 16);
                uart_puts(" does not match with packet %d \n");
                uart_put_long(host_if_p->dspif_info.select, 16);
                pkt_ptr = NULL;
            }
        }
        if (type == SELECT_READY) {
            menu_display = host_if_p->dspif_info.errmsg[1];
            return (pkt_ptr);
        }
        if (SWAP32(host_if_p->dspif_hdr.dest_id) != dc_slot) {
            uart_puts("\r\n dest_id not correct \r\n dest_id should be ");
            uart_put_long(dc_slot, 10);
            uart_puts("\r\n dest_id =  ");
            uart_put_long(SWAP32(host_if_p->dspif_hdr.dest_id), 16);
            pkt_ptr = NULL;
        }
           
    } else {
        //uart_puts("\r\n Received NOT SUCCESS Frame in wait_host_msg");
        pkt_ptr = NULL;
    }
    /*
     * Make sure the ethernet protocol field is correct
     */
    return (pkt_ptr);
}

/***********************************************************************
 *
 * Function: send_host_msg
 *
 * Description: Send message to host via GE interface 
 *
 * Input : message pointer, size of the message
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t send_host_msg (int which_emac, uint8_t *hwAddr, uint32_t size)
{
    int i;
    uint16_t result = FAILED;
    uint8_t *srcmac_p, *destmac_p;
	
    srcmac_p = (uint8_t *)&src_macaddr; /* set in diag_init_mac.c */
    destmac_p = (uint8_t *)&dest_macaddr; /* set in diag_init_mac.c */
    for (i = 0; i < sizeof(mac_address_t); ++i) {
        pkt_p->pkt_hdr.dest_addr[i] = (uint8_t)*destmac_p; /* send as boardcast */
        pkt_p->pkt_hdr.src_addr[i] = (uint8_t)*srcmac_p; /* DSP MAC addr - which DSP???? */
        pkt_p->pkt_hdr.pkt_type = SWAP16(0x0800); /* 0x0800 IPV4 pkt type */
        srcmac_p++;
        destmac_p++;
    }
    pkt_p->pkt_hdr.pkt_len = SWAP16(size - 14);
    sprintf((char *)&(hd_if->bufmsg[64]), "len = 0x%x", (unsigned int) size);
    memcpy(((uint8_t *)&(pkt_p->pkt_data) - 2), (const uint8_t *)hwAddr, size);

    srcmac_p = (uint8_t *)pkt_p;
    /* with XMIT_BUF_MAX_SIZE = 1024 and BLEN = 1024 can transmit uptp 1152 bytes */
    result = sp_SendTx(which_emac, 0, (uint8_t *)pkt_p, 1152, 1152);


    return (result);
}

/***********************************************************************
 *
 * Function: build_host_msg
 *
 * Description: build return message to be sent to host 
 *
 * Input : READY, TEST, or TIMEOUT
 *
 * Returns: none
 *
 **********************************************************************
 */
void build_host_msg (uint32_t msg_type)
{
    StarProPPB_UTILS_memset8((uint8_t *)ready_msg_p, 0, sizeof(dspif_ready_t));
    StarProPPB_UTILS_memset8((uint8_t *)result_msg_p, 0, sizeof(dspif_ether_t));
    StarProPPB_UTILS_memset8((uint8_t *)mem_msg_p, 0, sizeof(dspif_mem_t));
    if (msg_type == MSG_READY) {
        ready_msg_p->dspif_hdr.src_id = 
            (((dspid_p->slot_id) << 24) |
            ((dspid_p->module_id) << 16) |
            ((dspid_p->dsp_id) << 8) |
            (dspid_p->core_id));

        ready_msg_p->dspif_hdr.dest_id = HOST_ID;
        ready_msg_p->dspif_hdr.op_type = OP_READY;
        ready_msg_p->dspif_hdr.data_len = sizeof(dspif_ready_t);
        /* copy dsp_device */
        ready_msg_p->dsp_device.device_type = dsp_device_p->device_type;
        ready_msg_p->dsp_device.device_freq = dsp_device_p->device_freq;
        ready_msg_p->dsp_device.chip_id = dsp_device_p->chip_id;
        ready_msg_p->dsp_device.core_id = dsp_device_p->core_id;
        /* provide fw version number */
        ready_msg_p->fw_ver.major_num = DIAGFW_MAJ_REL;
        ready_msg_p->fw_ver.minor_num = DIAGFW_MIN_REL;
        ready_msg_p->fw_ver.debug_num = DIAGFW_DEBUG_VER;
    }
    if (msg_type == MSG_TEST) {
        result_msg_p->dspif_hdr.src_id = 
            (((dspid_p->slot_id) << 24) |
            ((dspid_p->module_id) << 16) |
            ((dspid_p->dsp_id) << 8) |
            (dspid_p->core_id));
        result_msg_p->dspif_hdr.dest_id = HOST_ID;
        result_msg_p->dspif_hdr.op_type = OP_RESPONSE;
        result_msg_p->dspif_hdr.data_len = sizeof(dspif_ether_t);
        /* copy test command/result interface */
        /* use copy functions later */
        StarProPPB_UTILS_memcpy8((uint8_t *)&(result_msg_p->dspif_info),
             (const uint8_t *)hd_if,
        sizeof(dspif_info_t));
    }
    if (msg_type == MSG_MEM) {
        mem_msg_p->dspif_hdr.src_id =
            (((dspid_p->slot_id) << 24) |
            ((dspid_p->module_id) << 16) |
            ((dspid_p->dsp_id) << 8) |
            (dspid_p->core_id));
        mem_msg_p->dspif_hdr.dest_id = HOST_ID;
        mem_msg_p->dspif_hdr.op_type = OP_RESPONSE;
        mem_msg_p->dspif_hdr.data_len = sizeof(dspif_mem_t);
        /* copy test command/result interface */
        /* use copy functions later */
        StarProPPB_UTILS_memcpy8((uint8_t *)&(mem_msg_p->dspif_info),
             (const uint8_t *)hd_if, sizeof(dspif_info_t));
        mem_msg_p->dspif_info.result = RESULT_SUCCESSFUL;
        bsp_debug_printf("\r\n param1 = 0x%x, param2 = 0x%x\n", hd_if->param1, hd_if->param2);
        StarProPPB_UTILS_memcpy8((uint8_t *)&(mem_msg_p->pkt_data),
             (const uint8_t *)hd_if->param1, hd_if->param2);
    }
    if (msg_type == MSG_TIMEOUT) {
        result_msg_p->dspif_hdr.src_id = 
            (((dspid_p->slot_id) << 24) |
            ((dspid_p->module_id) << 16) |
            ((dspid_p->dsp_id) << 8) |
            (dspid_p->core_id));
        result_msg_p->dspif_hdr.dest_id = HOST_ID;
        result_msg_p->dspif_hdr.op_type = OP_RESPONSE;
        result_msg_p->dspif_hdr.data_len = sizeof(dspif_ether_t);
        /* copy test command/result interface */
        StarProPPB_UTILS_memcpy8((uint8_t *)&(result_msg_p->dspif_info),
             (const uint8_t *)hd_if, sizeof(dspif_info_t));
        result_msg_p->dspif_info.result = RESULT_TIMEOUT;
        sprintf((char *)&(hd_if->errmsg), "Timed out waiting for result from test %x",
            (unsigned int)result_msg_p->dspif_info.select);
    }
}

/***********************************************************************
 *
 * Function: send_host_readymsg
 *
 * Description: send host through ge interface that DSP is ready
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t send_host_readymsg (void)
{
    /* set up packet content */
    build_host_msg(MSG_READY); /* from ARM */
    /* send packet */
    if (send_host_msg(EMAC0, (uint8_t *)ready_msg_p, sizeof(dspif_ready_t))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/***********************************************************************
 *
 * Function: semd_host_testmsg
 *
 * Description: send to host the response of test message
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t send_host_testmsg (void)
{
    build_host_msg(MSG_TEST); /* test on CORE */
    /* send packet */
    if (send_host_msg(EMAC0, (uint8_t *)result_msg_p, sizeof(dspif_ether_t))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/***********************************************************************
 *
 * Function: send_host_timeoutmsg
 *
 * Description: send to host that either init timeout or test timeout
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t send_host_timeoutmsg (void)
{
    build_host_msg(MSG_TIMEOUT); /* test on CORE */
    /* send packet */
    if (send_host_msg(EMAC0, (uint8_t *)result_msg_p, sizeof(dspif_ether_t))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: send_host_memmsg
 *
 * Description: send to host that either init timeout or test timeout
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t send_host_memmsg (void)
{
    build_host_msg(MSG_MEM); /* test on CORE */
    /* send packet */
    bsp_debug_printf("\r\n size of dspif_hdr_t = %d, size of dspif_info_t = %d,\
                size of dspif_mem_t = %d, size of ge_packet_t = %d\n",
                sizeof(dspif_hdr_t), sizeof(dspif_info_t),
                sizeof(dspif_mem_t), sizeof(ge_packet_t));
    if (send_host_msg(EMAC0, (uint8_t *)mem_msg_p, sizeof(dspif_mem_t))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: process_lpbk_msg
 *
 * Description: process loopback data or stop the test 
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t process_lpbk_msg (int which_emac, uint8_t *lpbk_msg, uint32_t pkt_size)
{
    /* copy msg to send pkt */
    StarProPPB_UTILS_memcpy8((uint8_t *)lpbk_msg_p, 
                             (const uint8_t *)(lpbk_msg + 14),
                             pkt_size);
    if (send_host_msg(which_emac, (uint8_t *)lpbk_msg_p, pkt_size)) {
        return (FAILED);
    } 
        return (PASSED);
}

/***********************************************************************
 *
 * Function: wait_lpbk_msg
 *
 * Description: waiting for loopback test data or stop test command.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
uint16_t wait_lpbk_msg (int which_emac)
{
    int	status;
    uint32_t	pkt_size;
    uint16_t result = PASSED;
    dspif_ether_t *pkt_ptr;

    pkt_ptr = test_msg_p;

    while (1) {
        status = sp_EthRxRawPkt(which_emac, 0 /* DQ */, (uint32_t*)&pkt_size, in_frame[0]);

        if ((status == EMAC_ERR_UDP_CKSUM) || (status == EMAC_SUCCESS)) {
            uart_puts("\r\n Received SUCCESS Loopback Frame in wait_lpbk_msg pkt size = ");
            uart_put_long(pkt_size, 10);

            StarProPPB_UTILS_memcpy8((uint8_t *)pkt_ptr,
                                     ((const uint8_t *)in_frame[0] + 14),
                                     ETH_FRAME_BUF_SIZE);
            if (pkt_ptr->dspif_hdr.op_type == OP_TEST_STOP) {
                uart_puts("\r\n Received STOP LOOPBACK pkt from host\n");
                break;
            }

            result = process_lpbk_msg(which_emac, in_frame[0], pkt_size);
            if (result == FAILED) {
                break;	
            }
        } else {
            uart_puts("\r\n Did not receive lpbl Frame in wait_lpbk_msg ");
            continue;
        }
     }
     return (result);
}

uint16_t wait_fpga_data (int which_emac, fpga_packet_t *pkt_ptr)
{
    int	status;
    uint32_t pkt_size;
    uint16_t result = PASSED;
    uint8_t rev_data[1200];
    int timeout = 1000*1000;

    while (timeout--) {
        status = sp_EthRxRawPkt(which_emac, 0 , (uint32_t*)&pkt_size, rev_data);

        if ((status == EMAC_ERR_UDP_CKSUM) || (status == EMAC_SUCCESS)) {

            if (rev_data[14] == PACKET_TO_DSP) {
                StarProPPB_UTILS_memcpy8((uint8_t *)pkt_ptr->pkt_data, 
                                         (const uint8_t *)(rev_data + 15),
                                         pkt_size);
                result = PASSED;
                break;
            }

        } else {
            //uart_puts("\r\n Did not receive Host Frame in wait_fpga_data ");
            continue;
        }
     }
     if (timeout == 0) {
         uart_puts("\r\n Did not receive Host Frame in wait_fpga_data ");
         return FAILED;
     }


     return (result);
}

uint16_t send_host_get_fpga_image (fpga_packet_t *tx_pkt_ptr)
{
    /* send packet */
    if (send_host_msg(EMAC0, (uint8_t *)tx_pkt_ptr, sizeof(fpga_packet_t))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}

int fpga_image_download (uint32_t image_size) 
{
    fpga_packet_t *tx_pkt_ptr;
    fpga_packet_t *rx_pkt_ptr;
    unsigned int fpga_data_index = 0;
    int pkt_size;

    if (image_size == PHOENIX_CPLD_UPGRADE_SIZE)
        bsp_debug_printf("\r\n CPLD Image Download");
    else
        bsp_debug_printf("\r\n FPGA Image Download");

    tx_pkt_ptr = fpga_tx_packet_p;
    rx_pkt_ptr = fpga_re_packet_p;
    
    fpga_image_size = image_size;

    while (fpga_image_size != 0) {
        if (fpga_image_size >= 1000) {
            pkt_size = 1000;
            fpga_image_size -= 1000;
        } else {
            pkt_size = fpga_image_size;
            fpga_image_size = 0;
        }

        /* Send ACK to Host */
        tx_pkt_ptr->pkt_hdr = PACKET_TO_HOST;
        send_host_get_fpga_image(tx_pkt_ptr);

        if (wait_fpga_data(EMAC0, rx_pkt_ptr) == FAILED) {
            printf("FPGA Download FAILED!!!!!\n");
            return (FAILED);
        }

        if (image_size == PHOENIX_CPLD_UPGRADE_SIZE) {
            bsp_debug_printf("\r\n !!!!cpld_data_index: %d \r", fpga_data_index);
        } else {
            bsp_debug_printf("\r\n !!!!fpga_data_index: %d \r", fpga_data_index);
        }
        StarProPPB_UTILS_memcpy8((uint8_t *)&(fpga_image[fpga_data_index]), 
                                 (uint8_t *)&(rx_pkt_ptr->pkt_data),
                                  pkt_size);
        fpga_data_index += pkt_size;

    }
    return (PASSED);
}

/*
 * $Log: diag_ppb_comm.c,v $
 * Revision 1.3  2021/04/15 00:53:07  achiu2
 * [PRRQ:CSCvx56970-2]Phoenix code review for ER
 *
 * Revision 1.2  2017/07/28 07:58:51  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.10.80.1  2017/03/30 10:25:50  harrchan
 * Add fpga upgrade utility
 *
 * Revision 1.10  2012/12/24 00:10:36  srane
 * Support the NGVM interface SYNC signal test, firmware version host
 * command and eeprom read/write utility.
 *
 * Revision 1.9  2012/09/24 01:15:52  srane
 * return PASSED for SELECT_MEM_DISP command from host.
 *
 * Revision 1.8  2012/09/10 06:39:02  srane
 * Add support for dsp memory display pkt transfer.
 *
 * Revision 1.7  2012/08/28 18:24:28  srane
 * Add check for the dest_id in received messages from host and DC for TDM
 * test.
 *
 * Revision 1.6  2012/08/15 15:02:56  srane
 * Add support for EMAC1 loopback test.
 *
 * Revision 1.5  2012/07/17 20:46:07  srane
 * Add GPIO I2C support, use ethernet to send/receive command/result to the
 * host. General cleanup.
 *
 * Revision 1.4  2012/06/07 22:50:59  srane
 * TDM external loopback, ECC memory test
 *
 * Revision 1.3  2012/05/24 23:25:38  srane
 * Add GPIO code to set ready bit, uart test, support both
 * uart mode and ethernet mode, other cleanup
 *
 * Revision 1.2  2012/05/10 22:57:58  srane
 * Add TDM support. Adjust the linker sections.
 *
 * Revision 1.1  2012/04/18 09:44:02  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
*/


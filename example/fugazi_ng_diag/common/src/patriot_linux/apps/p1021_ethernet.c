/* $Id: p1021_ethernet.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: p1021_ethernet.c
 *
 * Description: Port from p1021_etsec.c and p1021_eth_frames.c to support
 *              send and receive ethernet frames
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
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
#include "patriot_main.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "p1021_etsec.h"
#include "p1021_immap.h"
#include "common_utils.h"

extern uchar err_msg[];
extern mac_addr_t module_mac_addr;
extern mac_addr_t host_mac_addr;

static int		crc_table_inited;
static unsigned int	crc_table[256];

/*
 * 3 ETSECs numbered 1 - 3
 * but etsec_port numbering is 0 - 2, corresponding to ETSEC 1 - 3
 */
static struct tsec_info_struct tsec_info[3];

fe_packet_t tx_packet;
fe_packet_t *tx_packet_p = &tx_packet;

mac_addr_t bcast_mac_addr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


static volatile uint tx_bd_vir, tx_bd_phy;
static volatile uint rx_bd_vir, rx_bd_phy;



/* Ethernet packet 4 byte CRC calculationg
 */
unsigned int
crc32(unsigned int crc, unsigned char *data, int len)
{
    int			i;

    if (!crc_table_inited) {
	int		j;
	unsigned int		accum;

	for (i = 0; i < 256; i++) {
	    accum = i;

	    for (j = 0; j < 8; j++) {
		if (accum & 1) {
		    accum = accum >> 1 ^ 0xedb88320UL;
		} else {
		    accum = accum >> 1;
		}
	    }

	    crc_table[i] = SWAP32(accum);
	}

	crc_table_inited = 1;
    }

    for (i = 0; i < len; i++) {
	crc = crc << 8 ^ crc_table[crc >> 24 ^ data[i]];
    }

    return crc;
}


/***********************************************************************
 * Name: get_etsec_addr
 *
 * Description: Returns the etsec address of the requested etsec
 *
 * Input: etsec_num  - etsec number
 *        cfg_mode   - etsec mode
 *
 * Output: etsec address or 0 if invalid etsec_num
 *
 ***********************************************************************
 */
ccsr_tsec_t *get_etsec_addr (int etsec_num, int cfg_mode)
{
    ccsr_tsec_t *val;

    switch(etsec_num) {
        /* for etsec number */
        case ETSEC1:
            switch(cfg_mode) {
                /* for etsec configuration mode */
                case ETSEC_MDIO:
                    val = &(REGB->im_tsec1);
                    break;
                case ETSEC_GROUP0:
                    val = (ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB + 
                                          ETSEC1_GROUP0_OFFSET);
                    break;
                case ETSEC_GROUP1:
                    val = (ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB + 
                                          ETSEC1_GROUP1_OFFSET);
                    break;
                default:
                    val = 0;
                    break;
            }
            break;
        case ETSEC2:
            switch(cfg_mode) {
                case ETSEC_MDIO:
                    val = &(REGB->im_tsec2);
                    break;
                case ETSEC_GROUP0:
                    val = (ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB + 
                                          ETSEC2_GROUP0_OFFSET);
                    break;
                case ETSEC_GROUP1:
                    val = (ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB + 
                                          ETSEC2_GROUP1_OFFSET);
                    break;
                default:
                    val = 0;
                    break;
            }
            break;
        case ETSEC3:
            switch(cfg_mode) {
                case ETSEC_MDIO:
                    val = &(REGB->im_tsec3);
                    break;
                case ETSEC_GROUP0:
                    val = (ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB + 
                                          ETSEC3_GROUP0_OFFSET);
                    break;
                case ETSEC_GROUP1:
                    val = (ccsr_tsec_t *)(ADRSPC_PQUICC_IMEMB + 
                                          ETSEC3_GROUP1_OFFSET);
                    break;
                default:
                    val = 0;
                    break;
            }
            break;
        default:
            val = 0;
            break;
    }
    return (val);
}


/**********************************************************************
 *
 * Function: etsec_init_tsec ()
 *
 * Description: This function returns the mac address
 *
 * Input:  etsec_num
 *
 * Output: address of tsec_info_struct_t
 *
 **********************************************************************
 */
tsec_info_struct_t *etsec_init_tsec (int etsec_num)
{
    tsec_info_struct_t *tsec_p;

    tsec_p = (tsec_info_struct_t *)&tsec_info[etsec_num - 1];

    switch(etsec_num) {
    case ETSEC1:
	tsec_p->name = CONFIG_TSEC1_NAME;
	tsec_p->reg_base_addr = TSEC1_BASE_ADDR;
	tsec_p->phyaddr = TSEC1_PHY_ADDR;
	tsec_p->flags = CONFIG_TSEC1_FLAG;
	tsec_p->mac_addr[0] = TSEC1_MAC_ADD_0;
	tsec_p->mac_addr[1] = TSEC1_MAC_ADD_1;
	tsec_p->mac_addr[2] = TSEC1_MAC_ADD_2;
	tsec_p->mac_addr[3] = TSEC1_MAC_ADD_3;
	tsec_p->mac_addr[4] = TSEC1_MAC_ADD_4;
	tsec_p->mac_addr[5] = TSEC1_MAC_ADD_5;
	tsec_p->ip_addr = TSEC1_IP_ADD;
	tsec_p->tsec_num = 1;
	tsec_p->tx_bd = 0;
	tsec_p->tx_buf = 0;
	tsec_p->rx_bd = 0;
	tsec_p->rx_buf = 0;
	break;
    case ETSEC2:
	tsec_p->name = CONFIG_TSEC2_NAME;
	tsec_p->reg_base_addr = TSEC2_BASE_ADDR;
	tsec_p->phyaddr = TSEC2_PHY_ADDR;
	tsec_p->flags = CONFIG_TSEC2_FLAG;
	tsec_p->mac_addr[0] = TSEC2_MAC_ADD_0;
	tsec_p->mac_addr[1] = TSEC2_MAC_ADD_1;
	tsec_p->mac_addr[2] = TSEC2_MAC_ADD_2;
	tsec_p->mac_addr[3] = TSEC2_MAC_ADD_3;
	tsec_p->mac_addr[4] = TSEC2_MAC_ADD_4;
	tsec_p->mac_addr[5] = TSEC2_MAC_ADD_5;
	tsec_p->ip_addr = TSEC2_IP_ADD;
	tsec_p->tsec_num = 2;
	tsec_p->tx_bd = 0;
	tsec_p->tx_buf = 0;
	tsec_p->rx_bd = 0;
	tsec_p->rx_buf = 0;	
	break;
    case ETSEC3:
	tsec_p->name = CONFIG_TSEC3_NAME;
	tsec_p->reg_base_addr = TSEC3_BASE_ADDR;
	tsec_p->phyaddr = TSEC3_PHY_ADDR;
	tsec_p->flags = CONFIG_TSEC3_FLAG;
	tsec_p->mac_addr[0] = TSEC3_MAC_ADD_0;
	tsec_p->mac_addr[1] = TSEC3_MAC_ADD_1;
	tsec_p->mac_addr[2] = TSEC3_MAC_ADD_2;
	tsec_p->mac_addr[3] = TSEC3_MAC_ADD_3;
	tsec_p->mac_addr[4] = TSEC3_MAC_ADD_4;
	tsec_p->mac_addr[5] = TSEC3_MAC_ADD_5;
	tsec_p->ip_addr = TSEC3_IP_ADD;
	tsec_p->tsec_num = 3;
	tsec_p->tx_bd = 0;
	tsec_p->tx_buf = 0;
	tsec_p->rx_bd = 0;
	tsec_p->rx_buf = 0;	
	break;	
    default:
	break;
    }

	
    return (tsec_p);
}


/**********************************************************************
 *
 * Function: etsec_get_info_ptr ()
 *
 * Description: This function returns the mac address
 *
 * Input:  tsec_num
 *
 * Output: address of tsec_info_struct_t
 *
 **********************************************************************
 */
int
etsec_get_info_ptr (int tsec_num)
{
    int index;
    tsec_info_struct_t *tsec_p;

    index = tsec_num - 1;
    tsec_p = (tsec_info_struct_t *)&tsec_info[index];
    return((int)tsec_p);
}

 
/***********************************************************************
 * Name: etsec_ck_ievent
 *
 * Description: Will check ievent for pending status, save
 *    info, then clear pending bit
 *    We may enter this function as a result of an interrupt
 *    or a call from a poll routine. 
 *
 *    Transmit interrupts are set whenever TXB or TXF is set;
 *    to clear this hardware interrupt, must clear both bits.
 *    Receive interrupts are set whenever RXB or RXF is set;
 *    to clear this hardware interrupt, must clear both bits.
 *    Error and diagnostic interrupts are set whenever bits
 *    MAG, GTSC, GRSC, TXC, RXC, BABR, BABT, LC, CRL, FIR, FIQ,
 *    DPE, PERR, EBERR, TXE, XFUN, BSY are set.  Must clear
 *    all of these bits to clear a hardware/diagnostic interrupt.
 *
 * Input: etsec_num
 *        mode
 *
 * Output: none
 *
 ***********************************************************************
 */
void etsec_ck_ievent (int etsec_num, int mode)
{
    int tsec_ev, dummy;
    volatile ccsr_tsec_t *tsec_reg;

    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(etsec_num, ETSEC_GROUP0);
    tsec_ev = tsec_reg->ievent;
    etsec_tx_nframes[etsec_num - 1]++;
    if (mode == INTR_MODE) {
#ifdef DEBUG
	printf("\nIn eTSEC%d_intr_hndlr(), ievent @%#.8x = %#.8x\n",
	       etsec_num, &tsec_reg->ievent, tsec_ev);
#endif        
    }
    
    if (tsec_ev & TSEC_IEVENT_TXF) {
        tsec_reg->ievent = TSEC_IEVENT_TXF | TSEC_IEVENT_TXB;
        etsec_tx_nframes[etsec_num - 1]++;
    }

    if (tsec_ev & TSEC_IEVENT_RXFO) {
        tsec_reg->ievent = TSEC_IEVENT_RXFO | TSEC_IEVENT_RXBO;
        etsec_recv_nframes[etsec_num - 1]++;
    }

    if (tsec_ev & TSEC_ERR_IEVENTS) {
        tsec_reg->ievent = (tsec_ev & TSEC_ERR_IEVENTS);
        if (tsec_ev & (TSEC_IEVENT_RXC | TSEC_IEVENT_TXC)) {
            dummy = tsec_reg->tctrl;
            return;
        }
        
        etsec_recv_nframes[etsec_num - 1]++;
        printf("\n*** Unexpected eTSEC%d event status, "
               "ievent @%#.8x=%#.8x ***\n",
               etsec_num, &tsec_reg->ievent, tsec_ev);
    }

#ifdef DEBUG_INTR
    /* used when etsec1 interrupt was being handled by etsec2
     * interrupt handler.  Problem turned out to be that when
     * the processor internal interrupts was being programmed,
     * IIVPR13 was initialized to a wrong vector (22)
     */
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(1, ETSEC_GROUP0);
    printf("\neTSEC1 ievent @%#.8x = %#.8x, imask %#.8x\n",
           &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    tsec_ev = tsec_reg->ievent;
    if (tsec_ev) {
        tsec_reg->ievent = TSEC_IEVENTS;
        printf("eTSEC1 ievent @%#.8x = %#.8x, imask %#.8x\n",
               &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    }
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(2, ETSEC_GROUP0);
    printf("eTSEC2 ievent @%#.8x = %#.8x, imask %#.8x\n",
           &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    tsec_ev = tsec_reg->ievent;
    if (tsec_ev) {
        tsec_reg->ievent = TSEC_IEVENTS;
        printf("eTSEC2 ievent @%#.8x = %#.8x, imask %#.8x\n",
               &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    }
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(3, ETSEC_GROUP0);
    printf("eTSEC3 ievent @%#.8x = %#.8x, imask %#.8x\n",
           &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    tsec_ev = tsec_reg->ievent;
    if (tsec_ev) {
        tsec_reg->ievent = TSEC_IEVENTS;
        printf("eTSEC3 ievent @%#.8x = %#.8x, imask %#.8x\n",
               &tsec_reg->ievent, tsec_reg->ievent, tsec_reg->imask);
    }
#endif
}


/***********************************************************************
 * Name: etsec1_intr_hndlr
 *
 * Description: ETSEC1 interrupt handler
 *
 * Input: none
 *
 * Output: none
 *
 ***********************************************************************
 */
void etsec1_intr_hndlr (void)
{
    int etsec_num;

    etsec_num = ETSEC1;
    etsec_ck_ievent(etsec_num, INTR_MODE);
}

/***********************************************************************
 * Name: etsec2_intr_hndlr
 *
 * Description: ETSEC2 interrupt handler
 *
 * Input: none
 *
 * Output: none
 *
 ***********************************************************************
 */
void etsec2_intr_hndlr (void)
{
    int etsec_num;

    etsec_num = ETSEC2;
    etsec_ck_ievent(etsec_num, INTR_MODE);
}

/***********************************************************************
 * Name: etsec3_intr_hndlr
 *
 * Description: ETSEC3 interrupt handler
 *
 * Input: none
 *
 * Output: none
 *
 ***********************************************************************
 */
void etsec3_intr_hndlr (void)
{
    int etsec_num;

    etsec_num = ETSEC3;
    etsec_ck_ievent(etsec_num, INTR_MODE);
}



/**********************************************************************
 *
 * Function: etsec_get_txbd()
 *
 * Description: This function will scan the TxBD descriptor ring
 *      and return the address of the first empty transmit buffer
 *    descriptor.
 *
 *    During debug, found that scanning the TxBD ring for
 *    TX_RDY = 0 is not a good way to determine the next free
 *    TxBD since the etsec clears this bit after the buffer
 *    is transmitted or after an error condition is encountered
 *    so that we cannot depend on this bit being a 1 after a
 *    frame has been transmitted.  Instead, we will use the
 *    contents of the etsec tbptr to tell us the location of
 *    the next TxBD.
 *
 * Input:  tsec_p
 *
 * Output: pointer to TxBD that contains the empty transmit descriptor
 *
 **********************************************************************
 */
int etsec_get_txbd (tsec_info_struct_t *tsec_p)
{
    int txbd;
    volatile ccsr_tsec_t *regs;

    regs = (volatile ccsr_tsec_t *) tsec_p->reg_base_addr;
    txbd = regs->tbptr;

    return (txbd);
}

/**********************************************************************
 *
 * Function: etsec_get_rxbd()
 *
 * Description: This function will check the RxBD descriptor ring
 *      and return the address of the receive buffer descriptor
 *      that has data.
 *
 * Input:  tsec_p
 *
 * Output: pointer to RxBD that contains the received data
 *
 **********************************************************************
 */
int etsec_get_rxbd (tsec_info_struct_t *tsec_p)
{
    int rxbd_addr, idx;
    volatile tsec_bd_t *rxbd, *rxbd_vir_addr;

    rxbd = (tsec_bd_t *)tsec_p->rx_bd;
#ifdef DEBUG    
    printf("\n%s: rxbd = 0x%08x", __FUNCTION__, rxbd);
#endif
    /* scan for buffer descriptor with receive data */
    for (idx = 0; idx < NUM_RX_BD; idx++, rxbd++) {
	rxbd_vir_addr = (tsec_bd_t *)vir_addr(rxbd);

#ifdef DEBUG	
	printf("\n***%s: rxbd = 0x%08x", __FUNCTION__, rxbd);
	printf("\nrxbd_vir_addr->buf_ptr = 0x%08x", rxbd_vir_addr->buf_ptr);
	printf("\n%s %d: rxbd_vir_addr = 0x%08x, rxbd_vir_addr->status = 0x%08x, idx = %d",
	       __FUNCTION__, __LINE__, rxbd_vir_addr, rxbd_vir_addr->status, idx);
#endif	
        if (((rxbd_vir_addr->status & PQUICC_BDSTAT_RX_EMPTY) == 0) &&
            (rxbd_vir_addr->status & PQUICC_BDSTAT_RX_RO1)) {
            break;
        }
    }

    /* check if any frames were received */
    if(idx == NUM_RX_BD) {
        rxbd_addr = 0;
    } else {
        rxbd_addr = (int)rxbd;
    }

    return (rxbd_addr);
}


/**********************************************************************
 *
 * Function: etsec_recv_frame_ready()
 *
 * Description: This function will check the receive frames array
 *      and return the value.  This value will indicate the number
 *      of frames that have been received for the given etsec_num.
 *
 * Input: etsec_num
 *        mode - POLL_MODE or INTR_MODE
 *
 * Output: number of frames that have been received
 *
 **********************************************************************
 */
int etsec_recv_frame_ready (int etsec_num, int mode)
{
    if (mode == POLL_MODE) {
        etsec_ck_ievent(etsec_num, mode);
    } else {
	patriot_get_eth_intr_count();
    }
    return (etsec_recv_nframes[etsec_num - 1]);
}

/***********************************************************************
 * Name: check_tsec_tx_status
 *
 * Description: Check the eTSEC transmit buffer descriptor status
 *        for packet transmission and see if any transmission
 *        errors were reported.
 *
 * Input: tx_bd_ptr - Points to the transmit buffer descriptor.
 *        packet_num
 *
 * Output: Transmit buffer status.
 *
 ***********************************************************************
 */
int check_tsec_tx_status (volatile tsec_bd_t *tx_bd_ptr)
{
    int err = 0, i;

    for (i = 0; i < SPIN_100; i++) {
	if ((tx_bd_ptr->status & PQUICC_BDSTAT_TX_RDY) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }

    if (i == SPIN_100) {
	if (tx_bd_ptr->status & PQUICC_BDSTAT_TX_RDY) {
        sprintf(err_msg, "%s, [#%d]:Packet%d transmit did not occur, "
            "status @%#.8x=%#x", __FUNCTION__, __LINE__,
            &tx_bd_ptr->status, tx_bd_ptr->status);
        print_err(FALSE, err_msg, LVL_1);
        err |= POLL_TX_FAIL;
        }
    if (tx_bd_ptr->status & TX_BD_ERR_MASK) {
        sprintf(err_msg, "%s, [#%d]:Packet Tx Error, status @%#.8x=%#x,"
            " buf_ptr @%#.8x", __FUNCTION__, __LINE__,
            &tx_bd_ptr->status, tx_bd_ptr->status,
            &tx_bd_ptr->buf_ptr);
         print_err(FALSE, err_msg, LVL_1);
         err |= POLL_TX_ERR;
        }
    }

    return (err);
}



/***********************************************************************
 * Name: check_tsec_rx_status
 *
 * Description: Check the eTSEC receive buffer descriptor status
 *        for packet reception and, if a packet was received,
 *        see if there were any receive errors reported.
 *        Passing the transmit buffer descriptor to allow
 *        error display to display the address of both the
 *        transmit and receive buffer descriptors.
 *
 * Input: rx_bd_ptr - Points to the receive buffer descriptor
 *
 * Output: Receive buffer status.
 *
 ***********************************************************************
 */
int check_tsec_rx_status (volatile tsec_bd_t *rx_bd_ptr)
{
    int err = 0, i;

    for (i = 0; i < SPIN_100; i++) {
	if ((rx_bd_ptr->status & PQUICC_BDSTAT_RX_EMPTY) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == SPIN_100) {
    if (rx_bd_ptr->status & PQUICC_BDSTAT_RX_EMPTY) {
        sprintf(err_msg, "%s, [#%d]:Did not receive, "
            "rx status @%#.8x = %#x \ntx status @%#.8x = %#x",
            __FUNCTION__, __LINE__, &rx_bd_ptr->status,
            rx_bd_ptr->status);
        print_err(FALSE, err_msg, LVL_1);
        err |= POLL_RX_FAIL;
    }
    if (rx_bd_ptr->status & RX_BD_ERR_MASK) {
        sprintf(err_msg, "%s, [#%d]:Packet Rx Error, status @%#.8x=%#x, "
            "buf_ptr @%#.8x, tx status @%#.8x = %#x",
            __FUNCTION__, __LINE__, &rx_bd_ptr->status, rx_bd_ptr->status,
            rx_bd_ptr->buf_ptr);
        print_err(FALSE, err_msg, LVL_1);
        err |= POLL_RX_ERR;
    }
    }
    return (err);
}


/***********************************************************************
 * Name: patriot_receive_frames
 *
 * Description:
 *      This function receives frames
 *
 * Input: etsec_num - ETSEC Number
 *        tsec_p    - pointer to tsec info structure
 *        mode      - POLL_MODE or INTR_MODE
 *
 * Output: PASSED or FAILED
 *         NO_RX_FRAME
 *         INTR_RX_FAIL
 *         PKT_TIMEOUT
 *
 ***********************************************************************
 */
int 
patriot_receive_frames(int etsec_num, tsec_info_struct_t *tsec_p, int mode)
{
    
    char       base_val, inc_val;
    int        result, wait_time, *rd_ptr;
    tsec_bd_t  *tx_bd, *rx_bd, *rx_bd_vir_addr;
    if_ether_t *tx_buf;
    volatile ccsr_tsec_t *regs;
    uchar *ptr;
    

    result = PASSED;

    
    regs = (volatile ccsr_tsec_t *)tsec_p->reg_base_addr;
    if (mode == POLL_MODE) {
	etsec_recv_nframes[etsec_num - 1] = 0; /* Clear receive frame counter */
	etsec_tx_nframes[etsec_num - 1] = 0;   /* Clear transmit frame counter */
    } else {
	patriot_init_eth_intr_count();
    }
    /* wait for frame reception */    
    wait_time = SPIN_1000;

    while (1) {
	if (etsec_recv_frame_ready(etsec_num, mode)) {
#ifdef DEBUG	    
	    printf("\netsec_recv_nframes = %d\n",
		   etsec_recv_nframes[etsec_num -1]);fflush(0);
#endif	    
	    break;
	}

        if (--wait_time <= 0) {
#ifdef DEBUG	    
            printf("\nievent @%#.8x = %#.8x, imask %#.8x, %s mode",
                   &regs->ievent, regs->ievent, regs->imask, 
                   mode == INTR_MODE ? "intr" : "poll");
#endif
            if (regs->ievent & TSEC_IEVENT_RXFO) {
                /* receive ievent set but did not get the interrupt */
                return (INTR_RX_FAIL);
            } else {
                /* receive ievent not set */
                return (PKT_TIMEOUT);
            }
        }
        msleep(1);
    }

    /*
     * now get the receive frame from the RxBD ring
     * if we do not get a receive frame, check that the
     * frame was transmitted and then check rbptr and the
     * receive counters to see if maybe the frame was dropped
     */
    rx_bd = (tsec_bd_t *)etsec_get_rxbd(tsec_p);
#if DEBUG    
    printf("\nrx_bd = 0x%08x\n", rx_bd);
#endif    
    if (rx_bd == 0) {
#ifdef DEBUG
        printf("\neTSEC%d RxBD", etsec_num);
        dismem((uchar *)tsec_p->rx_bd, 0x40, tsec_p->rx_bd, BW_32BITS);
        printf("\nError receiving frame on eTSEC%d, rbptr @%#x=%#x",
              etsec_num, &regs->rbptr, regs->rbptr);
#endif
	sprintf(err_msg, "\n%s, [#%d]:Unable to get rx_bd\n"
			, __FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_1);
        return (NO_RX_FRAME);
    }

    rx_bd_vir_addr = (tsec_bd_t *)vir_addr(rx_bd);
    result |= check_tsec_rx_status(rx_bd_vir_addr);

#ifdef DEBUG
    printf("\nresult = 0x%02x", result);
    printf("\nrx_bd_vir_addr = 0x%08x", rx_bd_vir_addr);
    dismem((unsigned char *)(rx_bd_vir_addr), 40,
	   (unsigned)(rx_bd_vir_addr), 4);
    printf("\nrx_bd_vir_addr->buf_ptr = 0x%08x", rx_bd_vir_addr->buf_ptr);
    rd_ptr = (int *)vir_addr(rx_bd_vir_addr->buf_ptr);
    printf("\nvirtual addres of the buffer = 0x%08x\n", rd_ptr);
    printf("\nRX buffer : \n");
    dismem((unsigned char *)(rd_ptr), 128,
	   (unsigned)(rd_ptr), 4);
#endif

    ptr = (uchar *)vir_addr(rx_bd_vir_addr->buf_ptr);
    memcpy(&host_mac_addr[0], (uchar *)&ptr[6], sizeof(mac_addr_t));
#if DEBUG
    printf("Host MAC address %02x:%02x:%02x:%02x:%02x:%02x",
	   host_mac_addr[0],
	   host_mac_addr[1],
	   host_mac_addr[2],
	   host_mac_addr[3],
	   host_mac_addr[4],
	   host_mac_addr[5]);fflush(0);
#endif    
    return (result);

}
    


/***********************************************************************
 * Name: build_eth_header
 *
 * Description: Build the header for an Ethernet frame
 *      An ethernet header is composed of the following:
 *      - 6 bytes of destination address
 *      - 6 bytes of source address
 *      - 2 bytes of type
 *
 * Input: data_ptr   - Points to the buffer
 *        dest_addr  - Points to destination address array
 *        src_addr   - Points to source address array
 *        type       - type
 *
 * Output: None.
 *
 ***********************************************************************
 */
void build_eth_header (fe_packet_t *framep, mac_addr_t dst_mac_addr,
                       mac_addr_t src_mac_addr, uint16 type)
{
    memcpy((char *)&(framep->eth_hdr.dest_addr), (char *)dst_mac_addr,
           MAC_ADDR_SIZE);
    memcpy((char *)&(framep->eth_hdr.src_addr), (char *)src_mac_addr,
           MAC_ADDR_SIZE);
    framep->eth_hdr.pkt_len = type;
}

/**********************************************************************
 *
 * Function: build_eth_frame ()
 *
 * Description: This function builds an ethernet frame ready to be
 *    sent out for traffic test.  An ethernet frame is composed
 *    of the following:
 *    - 6 byte destination address
 *    - 6 byte source address
 *    - 2 byte length/type field
 *    - 46 to 1500 bytes of data (payload)
 *    - 4 bytes CRC
 *    Thus, the minimum ethernet frame size is 64 bytes and the
 *    maximum is 1518 bytes.  This size includes the 4 bytes of
 *    CRC.  However, the length field in the transmit buffer
 *    descriptor should not include the 4 bytes of CRC since
 *    these bytes are added by the hardware when composing the
 *    final ethernet frame.  Thus, the minimum/maximum values
 *    of the TxBD length field are 60/1514.
 *    Gigabit ethernet allows for jumbo frames of up to 9000
 *    bytes of data.
 *
 *    NOTE:
 *    even though the structure if_ether_t is referenced, we
 *    do not make use of it completely, i.e., for this test
 *    we ignore the if_info_t portion and consider the contents
 *    of the structure starting from pad for 1500 bytes as
 *    frame payload.  if_info_t is used by build_eth_msg()
 *    to build an ethernet message with total ethernet frame
 *    size less than 1518 bytes (if_ether_t total size = 1548);
 *
 * Input:  frame_ptr    - pointer to start of ether frame buffer
 *         dst_mac_addr - Destination MAC address
 *         src_mac_addr - Source MAC address
 *         frm_size     - size of frame
 *         
 * Output: PASSED if completed successfully.
 *         FAILED otherwise
 *
 **********************************************************************
 */
int build_eth_frame (fe_packet_t *frame_ptr, mac_addr_t dst_mac_addr,
                     mac_addr_t src_mac_addr, uint16 frm_size)
{
    int data_len, count;
    uchar data;
    uchar *datap;
    fe_packet_t *framep;
    unsigned int crc;
    
#ifdef DEBUG
    printf("\nbuild_eth_frame(%#x, %#x, %#x, %#x, %#x, %#x)",
	   frame_ptr, dst_mac_addr, src_mac_addr, frm_size);
#endif
    
    /* build ethernet frame header */
    framep = (fe_packet_t *)frame_ptr;
    build_eth_header(framep, dst_mac_addr, src_mac_addr,
                     0x0800);   /* Internet Protocol, Version 4 */
    
    /* build ethernet frame payload */
    datap = (uchar *)&framep->data[0];
    data_len = frm_size - sizeof(ether_hdr_t);

    for (count = 0; count < data_len - CRC_SIZE; count++) {
        *datap++ = tx_packet_p->data[count];
    }
    
    /* Add crc after the payload
     */
    crc = ~crc32(~0, (unsigned char *)frame_ptr, sizeof(fe_packet_t));
    *datap++ = (crc >> 24) & 0xff;
    *datap++ = (crc >> 16) & 0xff;
    *datap++ = (crc >> 8) & 0xff;
    *datap++ = crc & 0xff;

#ifdef DEBUG
    dismem((uchar *)framep, frm_size, (unsigned)framep, BW_32BITS);
#endif

    return (PASSED);
}

/**********************************************************************
 *
 * Function: etsec_send()
 *
 * Description: This function will verify that the transmitter is
 *      not busy, transmit the etsec ethernet packet, and wait
 *      for the packet to be transmitted
 *
 * Input:  etsec_num
 *         txbd
 *
 * Output: TxBD status if the frame is transmitted (should be 0) or
 *         -1 if there is an error prior to packet transmission
 *        (since if transmission occurs, then any tx error will be 
 *         shown in the TxBD status)
 *
 **********************************************************************
 */
int etsec_send (int etsec_num, tsec_bd_t *tx_bd)
{
    volatile ccsr_tsec_t *regs;
    int idx, result = 0;

#ifdef DEBUG
    printf("\netsec_send(%#x, %#x)", etsec_num, tx_bd);
#endif

    regs = (volatile ccsr_tsec_t *)get_etsec_addr(etsec_num, ETSEC_GROUP0);

    if (tx_bd->status & PQUICC_BDSTAT_TX_RDY) {
        /* 1 means data ready, ie, not transmitted */
        printf("\nTSEC Error: txbd busy for eTSEC%d\n", etsec_num);
        return -1;
    }
    tx_bd->status |= (PQUICC_BDSTAT_TX_RDY | PQUICC_BDSTAT_TX_LAST |
                      PQUICC_BDSTAT_TX_TC  | PQUICC_BDSTAT_TX_INT);

#ifdef DEBUG
    printf("\ntx_bd @%#x, status %#x, len %#x, buf_ptr %#x",
           &tx_bd->status, tx_bd->status, tx_bd->length, tx_bd->buf_ptr);
    printf("\ntstat @%#.8x = %#.8x", &regs->tstat, regs->tstat);
#endif

    /* Tell the DMA to go */
    regs->tstat = TSEC_TSTAT_THLT;

    /* Wait for buffer to be transmitted */
    for (idx = 0; tx_bd->status & PQUICC_BDSTAT_TX_RDY; idx++) {
        usleep(1);
        if (idx >= SPIN_10000) {
            printf("\nTx Error: Current BD for eTSEC%d not sent!\n"
                  "TxBD @%#x, tx_stat=%#x, tx_buf @%#x",
                  etsec_num, &tx_bd->status, tx_bd->status, tx_bd->buf_ptr);
            return -1;
        }
    }

#ifdef DEBUG
        printf("\netsec_send: tx_bd @%#x, status %#x, len %#x, buf_ptr %#x",
               &tx_bd->status, tx_bd->status, tx_bd->length, tx_bd->buf_ptr);
        printf("\ntstat @%#.8x = %#.8x", &regs->tstat, regs->tstat);
#endif

    result = tx_bd->status & PQUICC_BDSTAT_TX_STATS;
    return result;
}


/***********************************************************************
 * Name: patriot_send_frames
 *
 * Description:
 *      This function send Ethernet frames
 *
 * Input: etsec_num - ETSEC Number
 *        tsec_p    - pointer to tsec info structure
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int 
patriot_send_frames(int etsec_num, tsec_info_struct_t *tsec_p)
{

    ccsr_tsec_t *regs;
    tsec_bd_t *tx_bd, *tx_bd_vir_addr;
    fe_packet_t *tx_buf, *tx_buf_vir_addr;
    
    regs = (ccsr_tsec_t *)tsec_p->reg_base_addr;
    
    tx_bd = (tsec_bd_t *)etsec_get_txbd(tsec_p);
    tx_bd_vir_addr = (tsec_bd_t *)vir_addr(tx_bd);
    tx_buf = (fe_packet_t *)tx_bd_vir_addr->buf_ptr;
    tx_buf_vir_addr = (fe_packet_t *)vir_addr(tx_buf);

    /* make ethernet frame */
    if (build_eth_frame(tx_buf_vir_addr, host_mac_addr, module_mac_addr,
			sizeof(fe_packet_t)) == FAILED) {
        return (BUILD_PKT_ERR);
    }

    tx_bd_vir_addr->length = sizeof(fe_packet_t);

    etsec_recv_nframes[etsec_num - 1] = 0;    /* Clear receive frame counter */
    etsec_tx_nframes[etsec_num - 1] = 0;    /* Clear transmit frame counter */

    /* Transmit the frame */
    if (etsec_send(etsec_num, tx_bd_vir_addr) != 0) {
        printf("\nUnable to transmit frame to eTSEC%d, txbd @%#x",
              etsec_num, tx_bd);
        return (XMISSION_ERR);
    }
    
    return (PASSED);
}    

/***********************************************************************
 * Name: etsec_start
 *
 * Description: Start the Tx/Rx DMA
 *    This is done by clearing both GRS and GTS bits in the DMACTRL
 *    register then enabling Tx/Rx in MACCFG1
 *
 * Input: etsec_num - etsec number
 *        flag     - TRUE = perform startup,
 *             FALSE = do not perform startup
 *
 * Output: none
 *
 ***********************************************************************
 */
void etsec_start (int etsec_num, boolean flag)
{
    volatile ccsr_tsec_t *tsec_regs;

    if (flag == TRUE) {
        tsec_regs = (volatile ccsr_tsec_t *)get_etsec_addr(etsec_num, 
                                                           ETSEC_GROUP0);
        /* Tell the DMA it is clear to go */
        tsec_regs->dmactrl |= TSEC_DMACTRL_TDSEN | TSEC_DMACTRL_TBDSEN |
                              TSEC_DMACTRL_WWR | TSEC_DMACTRL_WOP;

        tsec_regs->tstat = TSEC_TSTAT_THLT;
#ifdef DEBUG
	printf("\nStart %s: \n", __FUNCTION__);
	printf("\ntsec_regs->dmactrl = 0x%08x", tsec_regs->dmactrl);	
	printf("\ntsec_regs->tstat = 0x%08x", tsec_regs->tstat);
#endif	
        /* Turn off the stop bits */
        tsec_regs->dmactrl &= ~(DMACTRL_SET_GRS | DMACTRL_SET_GTS);

        /* Enable Transmit and Receive */
        tsec_regs->maccfg1 |= (MACCFG1_TX_ENABLE | MACCFG1_RX_ENABLE);
#ifdef DEBUG	
	printf("\ntsec_regs->dmactrl = 0x%08x", tsec_regs->dmactrl);	
	printf("\ntsec_regs->maccfg1 = 0x%08x", tsec_regs->maccfg1);
	printf("\nEnd %s: \n", __FUNCTION__);
#endif
    }
    asm volatile ("msync");

}




/***********************************************************************
 * Name: etsec_stop
 *
 * Description: Gracefully stop the Tx/Rx DMA if not already stopped
 *    This is done by setting both GRS and GTS bits in the DMACTRL
 *    register then waiting for both GRSC and GTSC bits to be set
 *    in the IEVENT register.  This must be done before resetting
 *    the MAC or changing tx/rx parameters.
 *
 * Input: etsec_num - etsec number
 *
 * Output: PASSED/FAILED
 *
 ***********************************************************************
 */
int etsec_stop (int etsec_num)
{
    uint ievent, sav_msk, i;
    volatile ccsr_tsec_t *tsec_reg;

    /* Disable interrupts */
    tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(etsec_num, ETSEC_GROUP0);
    sav_msk = tsec_reg->imask;
    tsec_reg->imask = 0;

    if ((tsec_reg->dmactrl & DMACTRL_SET_GTS) == 0) {
        tsec_reg->dmactrl = DMACTRL_SET_GTS;
	for (i = 0; i < SPIN_100; i++) {
	    if ((tsec_reg->ievent & IEVENT_GTSC_SET) != IEVENT_GTSC_SET) {
		msleep(1);
	    } else {
		break;
	    }
	}
	if (i == SPIN_100) {
            printf("Did not get the GTSC event for eTSEC%d, "
		   "ievent @%#x = %#x, dmactrl %#x\n",
		   etsec_num, &tsec_reg->ievent, tsec_reg->ievent,
		   tsec_reg->dmactrl);
            return (FAILED);
        }
        tsec_reg->maccfg1 &= ~MACCFG1_TX_ENABLE;

        /* clear the interrupt event register */
        ievent = tsec_reg->ievent;
        tsec_reg->ievent = ievent;
    }

    if ((tsec_reg->dmactrl & DMACTRL_SET_GRS) == 0) {
        /* do this so we don't turn off GTS when writing GRS */
        tsec_reg->dmactrl = DMACTRL_SET_GTS | DMACTRL_SET_GRS;
	for (i = 0; i < SPIN_100; i++) {
	    if ((tsec_reg->ievent & IEVENT_GRSC_SET) != IEVENT_GRSC_SET) {
		msleep(1);
	    } else {
		break;
	    }
	}
	if (i == SPIN_100) {
            printf("Did not get the GRSC event for eTSEC%d, "
		   "ievent @%#x = %#x, dmactrl %#x\n",
		   etsec_num, &tsec_reg->ievent, tsec_reg->ievent,
		   tsec_reg->dmactrl);
            return (FAILED);
        }
        tsec_reg->maccfg1 &= ~MACCFG1_RX_ENABLE;

        /* clear the interrupt event register */
        ievent = tsec_reg->ievent;
        tsec_reg->ievent = ievent;
    }

    /* restore imask to previous state */
    tsec_reg->imask = sav_msk;
    asm volatile ("msync");

    return (PASSED);
}

/***********************************************************************
 * Name: init_tsec_regs
 *
 * Description: Initialize TSEC address registers (individual and group).
 *
 * Input: tsec_reg - Points to the base of the TSEC registers.
 *
 * Output: None.
 *
 ***********************************************************************
 */
static void init_tsec_regs (volatile ccsr_tsec_t *tsec_reg)
{
    tsec_reg->iaddr0 = 0;
    tsec_reg->iaddr1 = 0;
    tsec_reg->iaddr2 = 0;
    tsec_reg->iaddr3 = 0;
    tsec_reg->iaddr4 = 0;
    tsec_reg->iaddr5 = 0;
    tsec_reg->iaddr6 = 0;
    tsec_reg->iaddr7 = 0;

    tsec_reg->gaddr0 = 0;
    tsec_reg->gaddr1 = 0;
    tsec_reg->gaddr2 = 0;
    tsec_reg->gaddr3 = 0;
    tsec_reg->gaddr4 = 0;
    tsec_reg->gaddr5 = 0;
    tsec_reg->gaddr6 = 0;
    tsec_reg->gaddr7 = 0;
}


/***********************************************************************
 * Name: setup_miimgnt_ifc
 *
 * Description: MII management setup.
 *    We default to PREAMBLE and MIICFG_DIVIDE_28 but cuisinart used
 *    NO_PREAMBLE and MIICFG_DIVIDE_10
 * This function is for accessing external PHYs. It may not be needed
 * for Patriot but is used for Eval board.
 *
 * Input: tsec_mdio - Points to the SMI controller base address.
 *
 * Output: TRUE or FALSE.
 *
 ***********************************************************************
 */
int setup_miimgnt_ifc (volatile ccsr_tsec_t *tsec_mdio)
{
    int i;
    /* Reset the MIIMCFG management interface */
    tsec_mdio->miimcfg |= MIICFG_RESET;
    usleep(100);    /* Minimum 50 us (p.8 of Micrel KS8721B/BT spec */

    tsec_mdio->miimcfg = MIICFG_DIVIDE_10;

    /* MII Management bus is idle */
    for (i = 0; i < SPIN_200; i++) {
	if ((tsec_mdio->miimind & MIIMIND_BUSY) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == SPIN_200) {
	sprintf(err_msg, "\n%s, [#%d]:Setup_miimgnt_ifc timed out. miimind %#x"
			, __FUNCTION__, __LINE__, tsec_mdio->miimind);
	print_err(TRUE, err_msg, LVL_0);
	return (FALSE);
    }

    return (TRUE);
}

/**********************************************************************
 *
 * Function: init_tx_desc ()
 *
 * Description: Initialize the TX BDs for this ring
 *
 *    Because of pre-fetching, a minimum of 4 BDs per ring is required.
 *    Transmit rings are limited to a maximum size of 65536 BDs due
 *    to BD and frame data prefetching.  Because there is no next
 *    BD pointer, all BDs must reside sequentially in memory.  A
 *    wrap bit in the last BD informs the eTSEC to loop back to
 *    the beginning of the BD chain.  Software must initialize the
 *    TBASE register that point to the beginning BDs for an eTSEC.
 *    The R bit of the first TxBD in a frame must not be set
 *    until at least one entire frame can be fetched.  If eTSEC
 *    prefetches TxBDs and fails to reach a last TxBD (with bit
 *    L set), it will halt further transmission from the current
 *    TxBD ring and report an underrun error as IEVENT[XFUN];
 *    this indicates an incomplete frame was fetched, but
 *    remains unprocessed.  The relevant TBPTR register points
 *    to the next unread TxBD following the error.
 *    By default, only Tx Ring 0 is enabled.
 *    Software must not write tbptr0-tbptr7 while eTSEC is actively
 *    transmitting frames.  However, tbptr0-tbptr7 can be modified
 *    when the transmitter is disabled or when no Tx buffer is in
 *    use after a GRACEFUL STOP TRANSMIT command is issued and the
 *    frame completes its transmission).
 *
 * Input: pointer to first TxBD for the given ring
 *
 * Output: none
 *
 **********************************************************************
 */
void init_tx_desc (tsec_bd_t *txbd, int tx_buf)
{
    int count;
    volatile tsec_bd_t *tx_bd;

    tx_bd = txbd;
#ifdef DEBUG    
    printf("\n%s %d: tx_bd = 0x%08x", __FUNCTION__, __LINE__, tx_bd);
#endif    
    for (count = 0; count < NUM_TX_BD; count++, tx_bd++) {
        tx_bd->status = PQUICC_BDSTAT_TX_TC;
        tx_bd->length = MAX_TX_BUF;
        tx_bd->buf_ptr = (uchar *)phy_addr(tx_buf + (MAX_TX_BUF * count));
#ifdef DEBUG	
	printf("\ntx_bd->buf_ptr = 0x%08x", tx_bd->buf_ptr);
#endif	
    }
    /* set wrap bit at last BD */
    tx_bd--;
    tx_bd->status |= PQUICC_BDSTAT_TX_WRAP;
}

/**********************************************************************
 *
 * Function: init_rx_desc ()
 *
 * Description: Initialize the RX BDs for this ring
 *
 *    Because of pre-fetching, a minimum of 4 BDs per ring is required.
 *    Receive rings are limited to a maximum size of 65536 BDs due
 *    to BD and frame data prefetching.  Because there is no next
 *    BD pointer, all BDs must reside sequentially in memory.  A
 *    wrap bit in the last BD informs the eTSEC to loop back to
 *    the beginning of the BD chain.  Software must initialize the
 *    RBASE register that point to the beginning BDs for an eTSEC.
 *    By default, only Rx Ring 0 is enabled.
 *    Software must not write rbptr0-rbptr7 while eTSEC is actively
 *    receiving frames.  However, rbptr0-rbptr7 can be modified
 *    when the receiver is disabled or when no Rx buffer is in use
 *    after a GRACEFUL STOP RECEIVE command is issued and the
 *    frame completes its reception). 
 *
 *    Note: RO1 is used by the software to indicate whether a
 *    non-empty receive BD has been processed or not.  If RO1
 *    is 1, then the non-empty received BD has not beenn processed.
 *    If RO1 is 0, then the non-empty received BD has been
 *    processed.
 *
 * Input: pointer to the first RxBD for the given ring
 *
 * Output: none
 *
 **********************************************************************
 */
void init_rx_desc (tsec_bd_t *rxbd, int rx_buf)
{
    int count;
    volatile tsec_bd_t *rx_bd, *rx_bd_phy;

    rx_bd = rxbd;
#ifdef DEBUG    
    printf("\n%s %d: rx_bd = 0x%08x", __FUNCTION__, __LINE__, rx_bd);
#endif    
    for (count = 0; count < NUM_RX_BD; count++, rx_bd++) {
        rx_bd->status = PQUICC_BDSTAT_RX_EMPTY | PQUICC_BDSTAT_RX_INT |
                        PQUICC_BDSTAT_RX_RO1;
        rx_bd->length = 0;
        rx_bd->buf_ptr = (uchar *)phy_addr(rx_buf + (MAX_RX_BUF * count));
	rx_bd_phy = (tsec_bd_t *)phy_addr(rx_bd);
#ifdef DEBUG	
	printf("\nrx_bd = 0x%08x, rx_bd->buf_ptr = 0x%08x",rx_bd,rx_bd->buf_ptr);
	printf("\nrx_bd_phy = 0x%08x", rx_bd_phy);
#endif	
    }
    /* set wrap bit at last BD */
    rx_bd--;
    rx_bd->status |= PQUICC_BDSTAT_RX_WRAP;
#ifdef DEBUG    
    dismem((uchar *)rxbd, 0x80, rxbd, BW_32BITS);
#endif    
}


/**********************************************************************
 *
 * Function: init_tx_rings ()
 *
 * Description: Initialize the TX Rings
 *
 *    Because of pre-fetching, a minimum of 4 BDs per ring is required.
 *    Transmit rings are limited to a maximum size of 65536 BDs due
 *    to BD and frame data prefetching.  Because there is no next
 *    BD pointer, all BDs must reside sequentially in memory.  A
 *    wrap bit in the last BD informs the eTSEC to loop back to
 *    the beginning of the BD chain.  Software must initialize the
 *    TBASE register that point to the beginning BDs for an eTSEC.
 *    The R bit of the first TxBD in a frame must not be set
 *    until at least one entire frame can be fetched.  If eTSEC
 *    prefetches TxBDs and fails to reach a last TxBD (with bit
 *    L set), it will halt further transmission from the current
 *    TxBD ring and report an underrun error as IEVENT[XFUN];
 *    this indicates an incomplete frame was fetched, but
 *    remains unprocessed.  The relevant TBPTR register points
 *    to the next unread TxBD following the error.
 *    By default, only Tx Ring 0 is enabled.
 *    Software must not write tbptr0-tbptr7 while eTSEC is actively
 *    transmitting frames.  However, tbptr0-tbptr7 can be modified
 *    when the transmitter is disabled or when no Tx buffer is in
 *    use after a GRACEFUL STOP TRANSMIT command is issued and the
 *    frame completes its transmission).
 *
 *    Memory allocation of Tx rings, BDs:
 *    Tx Ring0: TxBD 0 - 11
 *    Tx Ring1: TxBD 0 - 11
 *      .   .   .   .   .
 *    Tx Ring7: TxBD 0 - 11
 *
 *    malloc will be called 4 times per etsec, once each for
 *    TxBD, txbuf, RxBD, and rxbuf
 *    malloc will also be called 4 time for each etsec so
 *    total malloc calls will be 4 x 4 = 16.
 *
 * Input: pointer to tsec_info_struct for the given etsec_num
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int init_tx_rings (tsec_info_struct_t *tsec_p)
{
    int ring, bdsize, bufsize, tsec_num;
    ulong tx_buf;
    uint *bdp, *basep;
    volatile uint tx_bd_vir_temp;
    
    volatile ccsr_tsec_t *tsec_reg;
    tsec_bd_t *tx_bd, *tx_bdp;

    /* get pointer to the tsec registers in the configuration memory */
    tsec_num = tsec_p->tsec_num;
    tsec_reg = (ccsr_tsec_t *)tsec_p->reg_base_addr;


    /*
     * malloc space for tx BDs and buffers if not already done
     * we will malloc enough space for all 8 Tx rings, which will
     * be placed in memory consecutively
     */
    bdsize = sizeof(tsec_bd_t) * NUM_TX_BD * NUM_TX_BD_RINGS;

    tx_bd_vir = (uint)malloc_nm(bdsize * 64);  /* to round of to 4K boundary */
#ifdef DEBUG    
    printf("\ntx_bd_vir = 0x%08x", tx_bd_vir);
#endif    
    if (tx_bd_vir == 0) {
        sprintf(err_msg, "\n%s, [#%d]:unable to malloc space for tx_bd"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    memset((char *)tx_bd_vir, 0, bdsize * 64);
    tx_bd_phy = (uint)phy_addr(tx_bd_vir);
    tsec_p->tx_bd = tx_bd_phy;
    
    tx_bd = (tsec_bd_t *)tsec_p->tx_bd;

#ifdef DEBUG
        printf("after malloc: tsec_p->tx_bd = %#x\n", tsec_p->tx_bd);
#endif

    bufsize = MAX_TX_BUF * NUM_TX_BD * NUM_TX_BD_RINGS;

    tsec_p->tx_buf = (uint)malloc_nm(bufsize);
    if (tsec_p->tx_buf == 0) {
        sprintf(err_msg, "\n%s, [#%d]:unable to malloc space for tx_buf"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return(FAILED);
    }
    tx_buf = tsec_p->tx_buf;
    memset((char *)tx_buf, 0, bufsize);
#ifdef DEBUG    
    printf("after malloc_nm: tsec_p->tx_buf = %#x\n", tsec_p->tx_buf);
    printf(" etsec%d tsec_p @%#x tx_bd @%#.8x for %#x, tx_buf @%#.8x for %#x\n",
               tsec_num, &tsec_p->name, tx_bd, bdsize, tx_buf, bufsize);
#endif

    /* place tx BDs and rings in first 4G of memory */
    tsec_reg->tbptrh = 0;
    tsec_reg->tbaseh = 0;

    /*
     * initialize the tx_bd rings and Tx BDs for each ring
     * tbase has the base address of each TxBD ring
     * tbptr has the addr of the next tx BD to be processed for
     * the respective TxBD ring
     */
    /* gracefully stop the Tx/Rx */
    etsec_stop(tsec_num);

    bdp = (uint *)&tsec_reg->tbptr;
    basep = (uint *)&tsec_reg->tbase;
    tx_bd_vir_temp = tx_bd_vir;
    for (ring = 0; ring < NUM_TX_BD_RINGS; ring++) {
        *basep = (uint)tx_bd;
        *bdp = (uint)tx_bd;
        tx_bdp = tx_bd;
#ifdef DEBUG	
	printf("\n*basep = 0x%08x", *basep);
	printf("\n*bdp = 0x%08x", *bdp);
	printf("\ntx_bdp = 0x%08x", tx_bdp);
#endif	
        init_tx_desc((tsec_bd_t *)tx_bd_vir_temp, (int)tx_buf);

        tx_bd += NUM_TX_BD;
	tx_bd_vir_temp += (NUM_TX_BD * sizeof(tsec_bd_t)); /* vir is different */
        /* increment to next free Tx buffer */
        tx_buf += MAX_TX_BUF * NUM_TX_BD;
#ifdef DEBUG
	printf(" tbptr%d @%#.8x = %#.8x, tbase%d @%#.8x = %#.8x\n",
	       ring, bdp, *bdp, ring, basep, *basep);
#endif
        /* inc to next etsec tbptr, tbase */
        bdp += 2;
        basep += 2;
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: init_rx_rings ()
 *
 * Description: Initialize the RX BDs.
 *
 *    Because of pre-fetching, a minimum of 4 BDs per ring is required.
 *    Receive rings are limited to a maximum size of 65536 BDs due
 *    to BD and frame data prefetching.  Because there is no next
 *    BD pointer, all BDs must reside sequentially in memory.  A
 *    wrap bit in the last BD informs the eTSEC to loop back to
 *    the beginning of the BD chain.  Software must initialize the
 *    RBASE register that point to the beginning BDs for an eTSEC.
 *    By default, only Rx Ring 0 is enabled.
 *    Software must not write rbptr0-rbptr7 while eTSEC is actively
 *    receiving frames.  However, rbptr0-rbptr7 can be modified
 *    when the receiver is disabled or when no Rx buffer is in use
 *    after a GRACEFUL STOP RECEIVE command is issued and the
 *    frame completes its reception). 
 *
 *    Memory allocation of Rx rings, BDs:
 *    Rx Ring0: RxBD 0 - 11
 *    Rx Ring1: RxBD 0 - 11
 *      .   .   .   .   .
 *    Rx Ring7: RxBD 0 - 11
 *
 *    Note: RO1 is used by the software to indicate whether a
 *    non-empty receive BD has been processed or not.  If RO1
 *    is 1, then the non-empty received BD has not beenn processed.
 *    If RO1 is 0, then the non-empty received BD has been
 *    processed.
 *
 * Input: pointer to tsec_info_struct for the given etsec_num
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int init_rx_rings (tsec_info_struct_t *tsec_p)
{
    int count, ring, bdsize, bufsize, tsec_num, temp;
    ulong rx_buf;
    uint *bdp, *basep, i;
    volatile uint rx_bd_vir_temp;
    volatile ccsr_tsec_t *tsec_reg;
    tsec_bd_t *rx_bd, *rx_bdp;

    /* get pointer to the tsec registers in the configuration memory */
    tsec_num = tsec_p->tsec_num;
    tsec_reg = (ccsr_tsec_t *)tsec_p->reg_base_addr;


    /*
     * malloc space for rx BDs and buffers if not already done
     * we will malloc enough space for all 8 Rx rings, which will
     * be placed in memory consecutively
     */
    bdsize = sizeof(tsec_bd_t) * NUM_RX_BD * NUM_RX_BD_RINGS;
    rx_bd_vir = (uint)malloc_nm(bdsize * 64); /* to round of to 4K boundary */
    
#ifdef DEBUG
    printf("\nbdsize = 0x%08x\n", bdsize);
    printf("\nrx_bd_vir = 0x%08x", rx_bd_vir);
#endif    
    if (rx_bd_vir == 0) {
        sprintf(err_msg, "\n%s, [#%d]:unable to malloc space for tx_bd"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    memset((char *)rx_bd_vir, 0, bdsize * 64);
    rx_bd_phy = (uint)phy_addr(rx_bd_vir);
    tsec_p->rx_bd = rx_bd_phy;

    rx_bd = (tsec_bd_t *)tsec_p->rx_bd;
#ifdef DEBUG
    printf("\nafter malloc: tsec_p->rx_bd = %#x\n", tsec_p->rx_bd);
#endif

    bufsize = MAX_RX_BUF * NUM_RX_BD * NUM_RX_BD_RINGS;
#ifdef DEBUG    
    printf("\nbufsize = 0x%08x\n", bufsize);
#endif    
    tsec_p->rx_buf = (uint)malloc_nm(bufsize);
    if (tsec_p->rx_buf == 0) {
        sprintf(err_msg, "\n%s, [#%d]:unable to malloc space for rx_buf"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }
    rx_buf = tsec_p->rx_buf;
#ifdef DEBUG    
    printf("\nrx_buf = 0x%08x\n", rx_buf);
#endif    
    memset((char *)rx_buf, 0, bufsize);
#ifdef DEBUG
    printf("after malloc_nm: tsec_p->rx_buf = %#x\n", tsec_p->rx_buf);
    printf(" etsec%d tsec_p @%#x rx_bd @%#.8x for %#x, rxbuf @%#.8x for %#x\n",
        tsec_num, &tsec_p->name, rx_bd, bdsize, rx_buf, bufsize);
    printf("\nrx_buf = 0x%08x", rx_buf);
#endif
    /* initialize the rx buffers */
    bdp = (uint *)rx_buf;
#ifdef DEBUG    
    for (i = 0; i< 8; i++) {
	printf("\nbdp = 0x%02x", *bdp);
	bdp++;
    }
#endif    

    for (count = 0; count < bufsize/4; count++) {
        *bdp++ = 0;
    }

    /* place rx BD and rings in first 4G of memory */
    tsec_reg->rbptrh = 0;
    tsec_reg->rbaseh = 0;
    tsec_reg->mrblr = MAX_RX_BUF;

    /*
     * initialize the rx_bd rings and Rx BDs for each ring
     * rbase has the base address of each RxBD ring
     * rbptr has the addr of the next RxBD to be processed 
     * for the respective RxBD ring
     */
    /* gracefully stop the Tx/Rx */
    etsec_stop(tsec_num);

    bdp = (uint *)&tsec_reg->rbptr;
    basep = (uint *)&tsec_reg->rbase;
    rx_bd_vir_temp = rx_bd_vir;

    for (ring = 0; ring < NUM_RX_BD_RINGS; ring++) {
        *basep = (uint)rx_bd;
        *bdp = (uint)rx_bd;
        rx_bdp = rx_bd;
        init_rx_desc((tsec_bd_t *)rx_bd_vir_temp, rx_buf);
        rx_bd += NUM_RX_BD;
	
	rx_bd_vir_temp += (NUM_RX_BD * sizeof(tsec_bd_t)); /* for virtual need sizeof */
#ifdef DEBUG	
	printf("\nrx_bdp = 0x%08x", rx_bdp);
	printf("\nrx_bd = 0x%08x", rx_bd);
	printf("\nrx_bd_vir_temp = 0x%08x", rx_bd_vir_temp);
#endif	
        /* increment to next free Rx buffer */
        rx_buf += MAX_RX_BUF * NUM_RX_BD;
#ifdef DEBUG
	printf(" rbptr%d @%#.8x = %#.8x, rbase%d @%#.8x = %#.8x\n",
	       ring, bdp, *bdp, ring, basep, *basep);
#endif
        /* inc to next etsec rbptr, rbase */
        bdp += 2;
        basep += 2;
    }

    return (PASSED);
}

/***********************************************************************
 * Name: tsec_phy_write
 *
 * Description: Write to a specified TSEC PHY register
 *
 * Input: tsec_mdio - Points to the SMI controller base address.
 *        dev_id   - TSEC SMI device ID
 *        reg      - Register offset to be written
 *        value    - Data to be written
 *
 * Output: PASSED or FAILED
 *
 ***********************************************************************
 */
int tsec_phy_write (volatile ccsr_tsec_t *tsec_mdio, uint dev_id, 
                    uint reg, uint value)
{
    int i;
    /* set for normal read operation */
    tsec_mdio->miimcom = 0;

    /*
     * bits 19-23 represent the 5 bit PHY address field of Mgmt
     * cycles.  Up to 31 PHYs can be addressed (0 is reserved).
     * bits 27-31 represent the 5 bit register address field of
     * Mgmt cycles.  Up to 32 registers can be accessed.
     */
    tsec_mdio->miimadd = (dev_id << 8) |  reg;

    for (i = 0; i < 200; i++) {
	if ((tsec_mdio->miimind & (MIIMIND_BUSY | MIIMIND_SCAN)) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == 200) {   
	sprintf(err_msg, "\n%s, [#%d]:tsec_phy_write timed out before the write. "
			"miimind %#x, miimadd %#x", __FUNCTION__, __LINE__,
			tsec_mdio->miimind, tsec_mdio->miimadd);
	print_err(TRUE, err_msg, LVL_0);
	return (FAILED);
    }

    /*
     * If written, an MII Mgmt write cycle is performed using the data
     * in bits 16:31, the pre-configured PHY address (at MIIMADD[PHY_Addr])
     * and the register address at (MIIMADD[Register Address]).
     */
    tsec_mdio->miimcon = value;
    /* Make sure read is complete */
    for (i = 0; i < 200; i++) {
	if ((tsec_mdio->miimind & MIIMIND_BUSY) == MIIMIND_BUSY) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == 200) {   
        printf("\nTSEC PHY Write to %#x with %#x timed out miimind %#x",
	       tsec_mdio->miimadd, value, tsec_mdio->miimind);	
	return (FAILED);
    }

    for (i = 0; i < 200; i++) {
	if ((tsec_mdio->miimind & (MIIMIND_BUSY | MIIMIND_SCAN)) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == 200) {   
        printf("\ntsec_phy_write to %#x with %#x timed out after the write"
	       ", miimind %#x", tsec_mdio->miimadd, value, tsec_mdio->miimind);
    }

    return (PASSED);
}

/***********************************************************************
 * Name: tsec_phy_read
 *
 * Description: Read a specified TSEC PHY register
 *
 * Input: tsec_mdio - Points to the SMI controller base address.
 *        dev_id   - TSEC SMI device ID
 *        reg      - Register offset to be read
 *
 * Output: Register read
 *
 ***********************************************************************
 */
uint tsec_phy_read (volatile ccsr_tsec_t *tsec_mdio, uint dev_id, uint reg)
{
    int i;
    /* set for normal read operation */
    tsec_mdio->miimcom = 0;
    /*
     * bits 19-23 represent the 5 bit PHY address filed of Mgmt
     * cycles.  Up to 31 PHYs can be addressed (0 is reserved).
     * bits 27-31 represent the 5 bit register address field of
     * Mgmt cycles.  Up to 32 registers can be accessed.
     */
    tsec_mdio->miimadd = (dev_id << 8) |  reg;
    
    for (i = 0; i < SPIN_200; i++) {
	if ((tsec_mdio->miimind & MIIMIND_BUSY) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == SPIN_200) {
        sprintf(err_msg, "\n%s, [#%d]:tsec_phy_read of %#x timed out "
        		"before the read. miimind %#x",
        		__FUNCTION__, __LINE__, tsec_mdio->miimadd, tsec_mdio->miimind);
        print_err(TRUE, err_msg, LVL_0);
        return (0);
    }

    tsec_mdio->miimcom = MIIMCOM_READ_CYCLE;
    /* put some delay here */
    for (i = 0; i < SPIN_200; i++) {
	if ((tsec_mdio->miimind & MIIMIND_BUSY) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == SPIN_200) {
        sprintf(err_msg, "\n%s, [#%d]:TSEC PHY READ of %#x timed out miimind %#x",
        		__FUNCTION__, __LINE__, tsec_mdio->miimadd, tsec_mdio->miimind);
        print_err(TRUE, err_msg, LVL_0);
        return (0);
    }
    
    for (i = 0; i < SPIN_200; i++) {
	if ((tsec_mdio->miimind & (MIIMIND_NOTVALID | MIIMIND_BUSY |
				   MIIMIND_SCAN)) != 0) {
	    msleep(1);
	} else {
	    break;
	}
    }
    if (i == SPIN_200) {
        sprintf(err_msg, "\n%s, [#%d]:tsec_phy_read of %#x timed out after "
        		"the read. miimind %#x",__FUNCTION__, __LINE__,
	       tsec_mdio->miimadd, tsec_mdio->miimind);
        print_err(TRUE, err_msg, LVL_0);
        return (0);
    }

    return (tsec_mdio->miimstat);
}



/***********************************************************************
 * Name: etsec_init
 *
 * Description: Initialize eTSEC for Ethernet operation.
 *    The following are the steps necessary for eTSEC initialization
 *    (per MPC8572 PowerQUICC III Integrated Host Processor
 *    Family Reference Manual, Rev 6, Sec 15.6.3.1.2):
 *    - Set and clear MACCFG1 (soft reset)
 *    - Initialize MACCFG2
 *    - Initialize MAC station addresses
 *    - Set up the PHY using the MII Mgmt Interface
 *    - Configure the TBI control to GMII
 *    - Clear IEVENT
 *    - Initialize IMASK
 *    - Initialize RCTRL
 *    - Initialize DMACTRL
 *
 *    The last 2 items will be performed later and not in this function.
 *    etsec 1 - 4 corresponds to PHY0, PHY1, PHY2, GE_SW
 *    The interface configuration indicated by registers ECNTRL
 *    and MACCFG2 for SGMII 1 Gbps are (table 15-11, eTSEC Interface
 *    Configurations, MPC8572E Reference Manual, Rev 0):
 *    ECNTRL : FIFM=0, GMIIM=0, TBIM=1, RPM=0, SGMIIM=1
 *    MACCFG2: IF_MODE = 0x10 (byte mode)
 *    Note that the ECNTRL bits are RO, having been set after
 *    sampling signals at power-on-reset.
 *    The etsec will be initialized in the following manner:
 *    default preamble, full duplex, pad append CRC, speed 1000 Mbps,
 *    flow control, Rx snooping. Loopback type (MAC or NONE)
 *    and interrupts (Enable/Disable) will be set per caller
 *    request.  Caller can modify certain parameters (speed,
 *    duplicity, flow_control) by calling etsec_adjust_link()
 *    after this function and before calling etsec_start().
 *
 * NOTE:
 *      Before issuing a soft reset to and/or reconfiguring the
 *      MAC with new parameters, the user must properly shutdown
 *      the DMA and make sure it is in an idle state for the
 *      entire duration.  User must gracefully stop the DMA by
 *      setting both GRS and GTS bits in the DMACTRL register,
 *      then wait for both GRSC and GTSC bits to be set in the
 *      IEVENT register before resetting the MAC or changing
 *      parameters.  Both GRS and GTS bits must be cleared
 *      before re-enabling the MAC to resume the DMA.
 *
 * Input: etsec_num - etsec number
 *        lpbk - Loopback type, SGMII_LPBK_MAC or SGMII_LPBK_NONE
 *        mode - POLL_MODE or INTR_MODE
 *        flag - TRUE = do etsec initialization,
 *               FALSE = skip etsec initialization
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int etsec_init (int etsec_num, int lpbk, int mode, boolean flag)
{
    uint index, i, dev_id, rdval;
    tsec_info_struct_t *tsec_p;
    volatile ccsr_tsec_t *tsec_reg, *tsec_mdio;
    unsigned char mac_addr[MAC_ADDR_SIZE];

    if (flag == TRUE) {
        /* gracefully stop etsec */
        if (etsec_stop(etsec_num) == FAILED) {
            return (FAILED);
        }

        /* initialize tsec structure */
        tsec_p = (tsec_info_struct_t *)etsec_init_tsec(etsec_num);
        tsec_reg = (volatile ccsr_tsec_t *)tsec_p->reg_base_addr;
        /* get SMI controller base address. */
        tsec_mdio = (volatile ccsr_tsec_t *)get_etsec_addr(etsec_num, 
                                                           ETSEC_MDIO);
        /*
         * set and clear MACCFG1 soft reset bit
         * soft reset must remain set for at least 3 Tx clocks
         */
        tsec_reg->maccfg1 = MACCFG1_SOFT_RESET;
        asm volatile ("msync");
        tsec_reg->maccfg1 = 0;
        asm volatile ("msync");
        msleep(1);

        /*
         * Init MACCFG2 for default preamble, full duplex, Pad append CRC,
         * @1000Mbps (for 10/100Mbps, use MACCFG2_NIBBLE_MODE)
         */
        tsec_reg->maccfg2 = MACCFG2_PREAM_DEFAULT | MACCFG2_FULL_DUPLEX | 
                            MACCFG2_PAD_CRC;

        if (tsec_p->flags & SGMII_SPEED_1000) {
            tsec_reg->maccfg2 |= MACCFG2_BYTE_MODE;
        } else {
            tsec_reg->maccfg2 |= MACCFG2_NIBBLE_MODE;
        }

        /*
         * Init ecntrl 
         * ECNTRL_SGMIIM and ECNTRL_TBIM are RO bits and are pin configured
         * at reset to 1 for SGMII mode
         * ECNTRL_R100M is ignored for 1000Mbps but is used if maccfg2
         * is set for NIBBLE_MODE, 1 = 100Mbps, 0 = 10Mbps
         */
        tsec_reg->ecntrl = ECNTRL_STEN | ECNTRL_CLRCNT;
	
        /* Init MAC station address */
	tsec_reg->macstnaddr1 = (module_mac_addr[5] << 24) |
	    (module_mac_addr[4] << 16) |
	    (module_mac_addr[3] << 8) |
	    (module_mac_addr[2]);
	tsec_reg->macstnaddr2 = (module_mac_addr[1] << 24) |
	    (module_mac_addr[0] << 16);
#ifdef DEBUG
	printf("\ntsec_reg->macstnaddr1 = 0x%08x\n", tsec_reg->macstnaddr1);
	fflush(0);
	printf("\ntsec_reg->macstnaddr2 = 0x%08x\n", tsec_reg->macstnaddr2);
	fflush(0);
#endif	
        /* Init TBI physical address for MII management configuration */
        index = etsec_num - 1;
        tsec_reg->tbipa = TSEC1_TBIPA_ADDR + index;
	
        /* initialize etsec address registers */
        init_tsec_regs(tsec_reg);
	
        /* setup attribute register to enable RxBD and Rx data snooping */
        tsec_reg->attr = TSEC_ATTR_RDSEN | TSEC_ATTR_RBDSEN;

        /* use flow control */
        tsec_reg->maccfg1 = MACCFG1_TX_FLOW | MACCFG1_RX_FLOW;
	
        /* setup flow control parameters */
        tsec_reg->rqprm0 = TSEC_RQPRM_FBTHR_FOUR | NUM_RX_BD;

        /* setup etsec for loopback at MAC or for normal operation */
        if (lpbk == SGMII_LPBK_MAC) {
            tsec_reg->maccfg1 |= MACCFG1_LOOPBACK;
            tsec_mdio->miimcfg |= MIICFG_RESET;
            usleep(100);    /* Minimum 50 us (p8 of Micrel KS8721B/BT spec */
        } else {
            tsec_reg->maccfg1 &= ~MACCFG1_LOOPBACK;
            if (setup_miimgnt_ifc(tsec_mdio) == FALSE) {
                return (FAILED);
            }
        }
	
        /* initialize the tx/rx rings */
        init_tx_rings(tsec_p);
	
        init_rx_rings(tsec_p);
	
        /* clear the interrupt event register */
        tsec_reg->ievent = TSEC_IEVENTS;
#ifdef DEBUG	
	printf("\ntsec_reg->maccfg2 = 0x%08x\n", tsec_reg->maccfg2);
	printf("\ntsec_reg->ecntrl = 0x%08x", tsec_reg->ecntrl);
	printf("\ntsec_reg->macstnaddr1 = 0x%08x, tsec_reg->macstnaddr2 = 0x%08x",
	       tsec_reg->macstnaddr1, tsec_reg->macstnaddr2);
	printf("\ntsec_reg->tbipa = 0x%08x", tsec_reg->tbipa);
	printf("\ntsec_reg->attr = 0x%08x", tsec_reg->attr);
	printf("\ntsec_reg->maccfg1 = 0x%08x", tsec_reg->maccfg1);
	printf("\ntsec_reg->rqprm0 = 0x%08x", tsec_reg->rqprm0);
	printf("\ntsec_reg->maccfg1 = 0x%08x", tsec_reg->maccfg1);
	printf("\ntsec_mdio->miimcfg = 0x%08x", tsec_mdio->miimcfg);
	printf("\ntsec_reg->ievent = 0x%08x", tsec_reg->ievent);
#endif	
        /* enable etsec interrupts, if requested */
        if (mode == INTR_MODE) {
            tsec_reg->imask = TSEC_IEVENT_TXF | TSEC_IEVENT_RXFO |
                              TSEC_ERR_IEVENTS;
        } else {
            tsec_reg->imask = 0;
        }
	
        if (tsec_p->flags & ETSEC_SGMII) {
            /*
             * Configure the AN Enable bit of etsec TBI Control Register to 0.
             * For the TBI connected to the GE switch, this is because the etsec
             * supports SGMI operating mode but the GE switch expects 1000Base-BX.
             * For the PHYs, this is because there is no outside connections
             * so nothing to negotiate.
             */
            dev_id = tsec_reg->tbipa;
            rdval = tsec_phy_read(tsec_mdio, dev_id, TBI_CTRL_REG);
            rdval &= ~TBI_AN_ENABLE;
            rdval |= TBI_RESET_AN;
            tsec_phy_write(tsec_mdio, dev_id, TBI_CTRL_REG, rdval);
        
            /* set TBI Control to single RX clock mode */
            rdval = tsec_phy_read(tsec_mdio, dev_id, TBI_CONTROL);
            rdval |= TBI_CLOCK_SELECT;
            tsec_phy_write(tsec_mdio, dev_id, TBI_CONTROL, rdval);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: cleanup_tsec ()
 *
 * Description: Cleanup etsec after testing
 *
 * Input: etsec number
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int cleanup_tsec (int etsec_num)
{
    volatile ccsr_tsec_t *tsec_reg, *tsec_mdio;
    tsec_info_struct_t *tsec_p;
    

    /* gracefully stop the Tx/Rx for this etsec */
    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
    
    etsec_stop(etsec_num);

    /* free allocated memory for this etsec and update pointers
     * debug prints added to help determine which free was
     * causing the free problems, header ptr of returned malloc
     * block overwritten, encountered during debug
     */

    free_nm((void *)tx_bd_vir);

    tsec_p->tx_bd = 0;

    free_nm((void *)tsec_p->tx_buf);
    tsec_p->tx_buf = 0;
    
    free_nm((void *)rx_bd_vir);
    tsec_p->rx_bd = 0;

    free_nm((void *)tsec_p->rx_buf);
    tsec_p->rx_buf = 0;

    /* insure tsec mac loopback is turned off */
    tsec_reg = (ccsr_tsec_t *)tsec_p->reg_base_addr;
    tsec_reg->maccfg1 &= ~MACCFG1_LOOPBACK;

    /*
     * take the PHY out of reset
     * else the GE PHY tests will not be able to access the PHYs
     * through the processor SMI when run after this test
     */
    tsec_mdio = (volatile ccsr_tsec_t *)get_etsec_addr(etsec_num, ETSEC_MDIO);
    tsec_mdio->miimcfg &= ~MIICFG_RESET;

    /* mask tsec interrupts */
    tsec_reg->imask = 0;

    /* clear ievent register */
    tsec_reg->ievent = TSEC_IEVENTS;

    return (PASSED);
}


/***********************************************************************
 * Name: show_phy_reg
 *
 * Description: Display etsec TBI MII registers.
 *    Device ID defaults to contents of TBIPA.
 *
 * Input: tsec_mdio - points to SMI controller base address.
 *        dev_id   - MII ID
 *
 * Output: None
 *
 ***********************************************************************
 */
static void show_phy_reg (volatile ccsr_tsec_t *tsec_mdio, uint dev_id)
{
    printf("Control Register   @%#.2x = %#.4x\n", TBI_CTRL_REG,
           tsec_phy_read(tsec_mdio, dev_id, TBI_CTRL_REG));
    printf("Status Register    @%#.2x = %#.4x\n", TBI_STAT_REG,
           tsec_phy_read(tsec_mdio, dev_id, TBI_STAT_REG));
    printf("AN Advertisement   @%#.2x = %#.4x\n", TBI_AN_ADVERTISEMENT,
           tsec_phy_read(tsec_mdio, dev_id, TBI_AN_ADVERTISEMENT));
    printf("AN LinkPartnerBPA  @%#.2x = %#.4x\n", TBI_ANLPBPA,
           tsec_phy_read(tsec_mdio, dev_id, TBI_ANLPBPA));
    printf("AN Expansion       @%#.2x = %#.4x\n", TBI_AN_EXPANSION,
           tsec_phy_read(tsec_mdio, dev_id, TBI_AN_EXPANSION));
    printf("AN Next Page Tx    @%#.2x = %#.4x\n", TBI_AN_NEXT_PAGE_TX,
           tsec_phy_read(tsec_mdio, dev_id, TBI_AN_NEXT_PAGE_TX));
    printf("AN LinkPartnerNPA  @%#.2x = %#.4x\n", TBI_ANLPANP,
           tsec_phy_read(tsec_mdio, dev_id, TBI_ANLPANP));
    printf("Extended Status    @%#.2x = %#.4x\n", TBI_EXTENDED_STATUS,
           tsec_phy_read(tsec_mdio, dev_id, TBI_EXTENDED_STATUS));
    printf("Jitter Diagnostics @%#.2x = %#.4x\n", TBI_JITTER_DIAGNOSTICS,
           tsec_phy_read(tsec_mdio, dev_id, TBI_JITTER_DIAGNOSTICS));
    printf("TBI Control        @%#.2x = %#.4x\n", TBI_CONTROL,
           tsec_phy_read(tsec_mdio, dev_id, TBI_CONTROL));
}


/***********************************************************************
 * Name: show_etsec_regs
 *
 * Description: Display contents of the eTSEC registers.
 *    The GE ports are numbered 0 - 3 while the eTSEC blocks
 *    are numbered 1 - 4.  GE port 0 attaches to eTSEC block 1,
 *    GE port 1 attaches to eTSEC block 2, and so on.
 *
 * Input: etsec_num
 *        disp_blks  - tsec register blocks to display
 *        if disp_blks = 0, then ask user
 *
 * Output: None 
 *         
 ***********************************************************************
 */
void show_etsec_regs (int etsec_num, int disp_blks)
{
    int num, count, cnt, dev_id;
    volatile ccsr_tsec_t *tsec_reg, *tsec_mdio;

    if (etsec_num == 0) {
        num = 1;
        count = NUM_ETSEC;
    } else {
        num = etsec_num;
        count = num;
    }

    for (cnt = num; cnt <= count; cnt++) {
        tsec_reg = (volatile ccsr_tsec_t *)get_etsec_addr(cnt, ETSEC_GROUP0);
        tsec_mdio = (volatile ccsr_tsec_t *)get_etsec_addr(cnt, ETSEC_MDIO);
        if (disp_blks & 0x8000) {
            printf("\neTSEC%d Tx/Rx Loopback Test Related Registers -\n",
                   cnt);
            printf("IEVENT       @%#.8x = %#.8x\n",
                   &tsec_reg->ievent, tsec_reg->ievent);
            printf("IMASK        @%#.8x = %#.8x\n",
                   &tsec_reg->imask, tsec_reg->imask);
            printf("EDIS         @%#.8x = %#.8x\n",
                   &tsec_reg->edis, tsec_reg->edis);
            printf("ECNTRL       @%#.8x = %#.8x\n",
                   &tsec_reg->ecntrl, tsec_reg->ecntrl);
            printf("DMACTRL      @%#.8x = %#.8x\n",
                   &tsec_reg->dmactrl, tsec_reg->dmactrl);
            printf("TCTRL        @%#.8x = %#.8x\n",
                   &tsec_reg->tctrl, tsec_reg->tctrl);
            printf("TSTAT        @%#.8x = %#.8x\n",
                   &tsec_reg->tstat, tsec_reg->tstat);
            printf("TBPTR0       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr, tsec_reg->tbptr);
            printf("TBASE0       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase, tsec_reg->tbase);
            printf("RCTRL        @%#.8x = %#.8x\n",
                   &tsec_reg->rctrl, tsec_reg->rctrl);
            printf("RSTAT        @%#.8x = %#.8x\n",
                   &tsec_reg->rstat, tsec_reg->rstat);
            printf("RBPTR0       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptr, tsec_reg->rbptr);
            printf("RBASE0       @%#.8x = %#.8x\n",
                   &tsec_reg->rbase, tsec_reg->rbase);
            printf("MACCFG1      @%#.8x = %#.8x\n",
                   &tsec_reg->maccfg1, tsec_reg->maccfg1);
            printf("MACCFG2      @%#.8x = %#.8x\n",
                   &tsec_reg->maccfg2, tsec_reg->maccfg2);
        }
        if (disp_blks & 0x0001) {
            printf("\neTSEC%d General Control and Status Registers -\n",
                   cnt);
            printf("TSEC_ID      @%#.8x = %#.8x\n",
                   &tsec_reg->tsec_id, tsec_reg->tsec_id);
            printf("TSEC_ID2     @%#.8x = %#.8x\n",
                   &tsec_reg->tsec_id2, tsec_reg->tsec_id2);
            printf("IEVENT       @%#.8x = %#.8x\n",
                   &tsec_reg->ievent, tsec_reg->ievent);
            printf("IMASK        @%#.8x = %#.8x\n",
                   &tsec_reg->imask, tsec_reg->imask);
            printf("EDIS         @%#.8x = %#.8x\n",
                   &tsec_reg->edis, tsec_reg->edis);
            printf("ECNTRL       @%#.8x = %#.8x\n",
                   &tsec_reg->ecntrl, tsec_reg->ecntrl);
            printf("MINFLR       @%#.8x = %#.8x\n",
                   &tsec_reg->minflr, tsec_reg->minflr);
            printf("PTV          @%#.8x = %#.8x\n",
                   &tsec_reg->ptv, tsec_reg->ptv);
            printf("DMACTRL      @%#.8x = %#.8x\n",
                   &tsec_reg->dmactrl, tsec_reg->dmactrl);
            printf("TBIPA        @%#.8x = %#.8x\n",
                   &tsec_reg->tbipa, tsec_reg->tbipa);
        }

        if (disp_blks & 0x0002) {
            printf("\neTSEC%d Transmit Control and Status Registers -\n",
                   cnt);
            printf("TCTRL        @%#.8x = %#.8x\n",
                   &tsec_reg->tctrl, tsec_reg->tctrl);
            printf("TSTAT        @%#.8x = %#.8x\n",
                   &tsec_reg->tstat, tsec_reg->tstat);
            printf("DFVLAN       @%#.8x = %#.8x\n",
                   &tsec_reg->dfvlan, tsec_reg->dfvlan);
            printf("TBDLEN       @%#.8x = %#.8x\n",
                   &tsec_reg->tbdlen, tsec_reg->tbdlen);
            printf("TXIC         @%#.8x = %#.8x\n",
                   &tsec_reg->txic, tsec_reg->txic);
            printf("TQUEUE       @%#.8x = %#.8x\n",
                   &tsec_reg->tqueue, tsec_reg->tqueue);
            printf("TR03WT       @%#.8x = %#.8x\n",
                   &tsec_reg->tr03wt, tsec_reg->tr03wt);
            printf("TR47WT       @%#.8x = %#.8x\n",
                   &tsec_reg->tr47wt, tsec_reg->tr47wt);
            printf("TBPTRH       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptrh, tsec_reg->tbptrh);
            printf("TBPTR0       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr, tsec_reg->tbptr);
            printf("TBPTR1       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr1, tsec_reg->tbptr1);
            printf("TBPTR2       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr2, tsec_reg->tbptr2);
            printf("TBPTR3       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr3, tsec_reg->tbptr3);
            printf("TBPTR4       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr4, tsec_reg->tbptr4);
            printf("TBPTR5       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr5, tsec_reg->tbptr5);
            printf("TBPTR6       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr6, tsec_reg->tbptr6);
            printf("TBPTR7       @%#.8x = %#.8x\n",
                   &tsec_reg->tbptr7, tsec_reg->tbptr7);
            printf("TBASEH       @%#.8x = %#.8x\n",
                   &tsec_reg->tbaseh, tsec_reg->tbaseh);
            printf("TBASE0       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase, tsec_reg->tbase);
            printf("TBASE1       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase1, tsec_reg->tbase1);
            printf("TBASE2       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase2, tsec_reg->tbase2);
            printf("TBASE3       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase3, tsec_reg->tbase3);
            printf("TBASE4       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase4, tsec_reg->tbase4);
            printf("TBASE5       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase5, tsec_reg->tbase5);
            printf("TBASE6       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase6, tsec_reg->tbase6);
            printf("TBASE7       @%#.8x = %#.8x\n",
                   &tsec_reg->tbase7, tsec_reg->tbase7);
            printf("TMR_TXTS1_ID @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_txts1_id, tsec_reg->tmr_txts1_id);
            printf("TMR_TXTS2_ID @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_txts2_id, tsec_reg->tmr_txts2_id);
            printf("TMR_TXTS1_H  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_txts1_h, tsec_reg->tmr_txts1_h);
            printf("TMR_TXTS1_L  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_txts1_l, tsec_reg->tmr_txts1_l);
            printf("TMR_TXTS2_H  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_txts2_h, tsec_reg->tmr_txts2_h);
            printf("TMR_TXTS2_L  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_txts2_l, tsec_reg->tmr_txts2_l);
        }

        if (disp_blks & 0x0004) {
            printf("\neTSEC%d Receive Control and Status Registers -\n",
                   cnt);
            printf("RCTRL        @%#.8x = %#.8x\n",
                   &tsec_reg->rctrl, tsec_reg->rctrl);
            printf("RSTAT        @%#.8x = %#.8x\n",
                   &tsec_reg->rstat, tsec_reg->rstat);
            printf("RXIC         @%#.8x = %#.8x\n",
                   &tsec_reg->rxic, tsec_reg->rxic);
            printf("RQUEUE       @%#.8x = %#.8x\n",
                   &tsec_reg->rqueue, tsec_reg->rqueue);
            printf("MRBLR        @%#.8x = %#.8x\n",
                   &tsec_reg->mrblr, tsec_reg->mrblr);
            printf("RBDBPH       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrh, tsec_reg->rbptrh);
            printf("RBPTR0       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptr, tsec_reg->rbptr);
            printf("RBPTR1       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl1, tsec_reg->rbptrl1);
            printf("RBPTR2       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl2, tsec_reg->rbptrl2);
            printf("RBPTR3       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl3, tsec_reg->rbptrl3);
            printf("RBPTR4       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl4, tsec_reg->rbptrl4);
            printf("RBPTR5       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl5, tsec_reg->rbptrl5);
            printf("RBPTR6       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl6, tsec_reg->rbptrl6);
            printf("RBPTR7       @%#.8x = %#.8x\n",
                   &tsec_reg->rbptrl7, tsec_reg->rbptrl7);
            printf("RBASEH       @%#.8x = %#.8x\n",
                   &tsec_reg->rbaseh, tsec_reg->rbaseh);
            printf("RBASE0       @%#.8x = %#.8x\n",
                   &tsec_reg->rbase, tsec_reg->rbase);
            printf("RBASE1       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel1, tsec_reg->rbasel1);
            printf("RBASE2       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel2, tsec_reg->rbasel2);
            printf("RBASE3       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel3, tsec_reg->rbasel3);
            printf("RBASE4       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel4, tsec_reg->rbasel4);
            printf("RBASE5       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel5, tsec_reg->rbasel5);
            printf("RBASE6       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel6, tsec_reg->rbasel6);
            printf("RBASE7       @%#.8x = %#.8x\n",
                   &tsec_reg->rbasel7, tsec_reg->rbasel7);
            printf("TMR_RXTS_H   @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_rxts_h, tsec_reg->tmr_rxts_h);
            printf("TMR_RXTS_L   @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_rxts_l, tsec_reg->tmr_rxts_l);
        }

        if (disp_blks & 0x0008) {
            printf("\neTSEC%d MAC Registers -\n", cnt);
            printf("MACCFG1      @%#.8x = %#.8x\n",
                   &tsec_reg->maccfg1, tsec_reg->maccfg1);
            printf("MACCFG2      @%#.8x = %#.8x\n",
                   &tsec_reg->maccfg2, tsec_reg->maccfg2);
            printf("IPGIFG       @%#.8x = %#.8x\n",
                   &tsec_reg->ipgifg, tsec_reg->ipgifg);
            printf("HAFDUP       @%#.8x = %#.8x\n",
                   &tsec_reg->hafdup, tsec_reg->hafdup);
            printf("MAXFRM       @%#.8x = %#.8x\n",
                   &tsec_reg->maxfrm, tsec_reg->maxfrm);
            printf("MIIMCFG      @%#.8x = %#.8x\n",
                   &tsec_reg->miimcfg, tsec_reg->miimcfg);
            printf("MIIMCOM      @%#.8x = %#.8x\n",
                   &tsec_reg->miimcom, tsec_reg->miimcom);
            printf("MIIMADD      @%#.8x = %#.8x\n",
                   &tsec_reg->miimadd, tsec_reg->miimadd);
            printf("MIIMCON      @%#.8x = %#.8x\n",
                   &tsec_reg->miimcon, tsec_reg->miimcon);
            printf("MIIMSTAT     @%#.8x = %#.8x\n",
                   &tsec_reg->miimstat, tsec_reg->miimstat);
            printf("MIIMIND      @%#.8x = %#.8x\n",
                   &tsec_reg->miimind, tsec_reg->miimind);
            printf("IFSTAT       @%#.8x = %#.8x\n",
                   &tsec_reg->ifstat, tsec_reg->ifstat);
            printf("MACSTNADDR1  @%#.8x = %#.8x\n",
                   &tsec_reg->macstnaddr1, tsec_reg->macstnaddr1);
            printf("MACSTNADDR2  @%#.8x = %#.8x\n",
                   &tsec_reg->macstnaddr2, tsec_reg->macstnaddr2);
            printf("MAC01ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac01addr1, tsec_reg->mac01addr1);
            printf("MAC01ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac01addr2, tsec_reg->mac01addr2);
            printf("MAC02ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac02addr1, tsec_reg->mac02addr1);
            printf("MAC02ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac02addr2, tsec_reg->mac02addr2);
            printf("MAC03ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac03addr1, tsec_reg->mac03addr1);
            printf("MAC03ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac03addr2, tsec_reg->mac03addr2);
            printf("MAC04ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac04addr1, tsec_reg->mac04addr1);
            printf("MAC04ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac04addr2, tsec_reg->mac04addr2);
            printf("MAC05ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac05addr1, tsec_reg->mac05addr1);
            printf("MAC05ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac05addr2, tsec_reg->mac05addr2);
            printf("MAC06ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac06addr1, tsec_reg->mac06addr1);
            printf("MAC06ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac06addr2, tsec_reg->mac06addr2);
            printf("MAC07ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac07addr1, tsec_reg->mac07addr1);
            printf("MAC07ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac07addr2, tsec_reg->mac07addr2);
            printf("MAC08ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac08addr1, tsec_reg->mac08addr1);
            printf("MAC08ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac08addr2, tsec_reg->mac08addr2);
            printf("MAC09ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac09addr1, tsec_reg->mac09addr1);
            printf("MAC09ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac09addr2, tsec_reg->mac09addr2);
            printf("MAC10ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac10addr1, tsec_reg->mac10addr1);
            printf("MAC10ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac10addr2, tsec_reg->mac10addr2);
            printf("MAC11ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac11addr1, tsec_reg->mac11addr1);
            printf("MAC11ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac11addr2, tsec_reg->mac11addr2);
            printf("MAC12ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac12addr1, tsec_reg->mac12addr1);
            printf("MAC12ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac12addr2, tsec_reg->mac12addr2);
            printf("MAC13ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac13addr1, tsec_reg->mac13addr1);
            printf("MAC13ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac13addr2, tsec_reg->mac13addr2);
            printf("MAC14ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac14addr1, tsec_reg->mac14addr1);
            printf("MAC14ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac14addr2, tsec_reg->mac14addr2);
            printf("MAC15ADDR1   @%#.8x = %#.8x\n",
                   &tsec_reg->mac15addr1, tsec_reg->mac15addr1);
            printf("MAC15ADDR2   @%#.8x = %#.8x\n",
                   &tsec_reg->mac15addr2, tsec_reg->mac15addr2);
        }

        if (disp_blks & 0x0010) {
            printf("\neTSEC%d Transmit and Receive Counters -\n", cnt);
            printf("TR64         @%#.8x = %#.8x\n",
                   &tsec_reg->tr64, tsec_reg->tr64);
            printf("TR127        @%#.8x = %#.8x\n",
                   &tsec_reg->tr127, tsec_reg->tr127);
            printf("TR255        @%#.8x = %#.8x\n",
                   &tsec_reg->tr255, tsec_reg->tr255);
            printf("TR511        @%#.8x = %#.8x\n",
                   &tsec_reg->tr511, tsec_reg->tr511);
            printf("TR1K         @%#.8x = %#.8x\n",
                   &tsec_reg->tr1k, tsec_reg->tr1k);
            printf("TRMAX        @%#.8x = %#.8x\n",
                   &tsec_reg->trmax, tsec_reg->trmax);
            printf("TRMGV        @%#.8x = %#.8x\n",
                   &tsec_reg->trmgv, tsec_reg->trmgv);
        }

        if (disp_blks & 0x0020) {
            printf("\neTSEC%d Receive Counters -\n", cnt);
            printf("RBYT         @%#.8x = %#.8x\n",
                   &tsec_reg->rbyt, tsec_reg->rbyt);
            printf("RPKT         @%#.8x = %#.8x\n",
                   &tsec_reg->rpkt, tsec_reg->rpkt);
            printf("RFCS         @%#.8x = %#.8x\n",
                   &tsec_reg->rfcs, tsec_reg->rfcs);
            printf("RMCA         @%#.8x = %#.8x\n",
                   &tsec_reg->rmca, tsec_reg->rmca);
            printf("RBCA         @%#.8x = %#.8x\n",
                   &tsec_reg->rbca, tsec_reg->rbca);
            printf("RXCF         @%#.8x = %#.8x\n",
                   &tsec_reg->rxcf, tsec_reg->rxcf);
            printf("RXPF         @%#.8x = %#.8x\n",
                   &tsec_reg->rxpf, tsec_reg->rxpf);
            printf("RXUO         @%#.8x = %#.8x\n",
                   &tsec_reg->rxuo, tsec_reg->rxuo);
            printf("RALN         @%#.8x = %#.8x\n",
                   &tsec_reg->raln, tsec_reg->raln);
            printf("RFLR         @%#.8x = %#.8x\n",
                   &tsec_reg->rflr, tsec_reg->rflr);
            printf("RCDE         @%#.8x = %#.8x\n",
                   &tsec_reg->rcde, tsec_reg->rcde);
            printf("RCSE         @%#.8x = %#.8x\n",
                   &tsec_reg->rcse, tsec_reg->rcse);
            printf("RUND         @%#.8x = %#.8x\n",
                   &tsec_reg->rund, tsec_reg->rund);
            printf("ROVR         @%#.8x = %#.8x\n",
                   &tsec_reg->rovr, tsec_reg->rovr);
            printf("RFRG         @%#.8x = %#.8x\n",
                   &tsec_reg->rfrg, tsec_reg->rfrg);
            printf("RJBR         @%#.8x = %#.8x\n",
                   &tsec_reg->rjbr, tsec_reg->rjbr);
            printf("RDRP         @%#.8x = %#.8x\n",
                   &tsec_reg->rdrp, tsec_reg->rdrp);
        }

        if (disp_blks & 0x0040) {
            printf("\neTSEC%d Transmit Counters -\n", cnt);
            printf("TBYT         @%#.8x = %#.8x\n",
                   &tsec_reg->tbyt, tsec_reg->tbyt);
            printf("TPKT         @%#.8x = %#.8x\n",
                   &tsec_reg->tpkt, tsec_reg->tpkt);
            printf("TMCA         @%#.8x = %#.8x\n",
                   &tsec_reg->tmca, tsec_reg->tmca);
            printf("TBCA         @%#.8x = %#.8x\n",
                   &tsec_reg->tbca, tsec_reg->tbca);
            printf("TXPF         @%#.8x = %#.8x\n",
                   &tsec_reg->txpf, tsec_reg->txpf);
            printf("TDFR         @%#.8x = %#.8x\n",
                   &tsec_reg->tdfr, tsec_reg->tdfr);
            printf("TEDF         @%#.8x = %#.8x\n",
                   &tsec_reg->tedf, tsec_reg->tedf);
            printf("TSCL         @%#.8x = %#.8x\n",
                   &tsec_reg->tscl, tsec_reg->tscl);
            printf("TMCL         @%#.8x = %#.8x\n",
                   &tsec_reg->tmcl, tsec_reg->tmcl);
            printf("TLCL         @%#.8x = %#.8x\n",
                   &tsec_reg->tlcl, tsec_reg->tlcl);
            printf("TXCL         @%#.8x = %#.8x\n",
                   &tsec_reg->txcl, tsec_reg->txcl);
            printf("TNCL         @%#.8x = %#.8x\n",
                   &tsec_reg->tncl, tsec_reg->tncl);
            printf("TDRP         @%#.8x = %#.8x\n",
                   &tsec_reg->tdrp, tsec_reg->tdrp);
            printf("TJBR         @%#.8x = %#.8x\n",
                   &tsec_reg->tjbr, tsec_reg->tjbr);
            printf("TFCS         @%#.8x = %#.8x\n",
                   &tsec_reg->tfcs, tsec_reg->tfcs);
            printf("TXCF         @%#.8x = %#.8x\n",
                   &tsec_reg->txcf, tsec_reg->txcf);
            printf("TOVR         @%#.8x = %#.8x\n",
                   &tsec_reg->tovr, tsec_reg->tovr);
            printf("TUND         @%#.8x = %#.8x\n",
                   &tsec_reg->tund, tsec_reg->tund);
            printf("TFRG         @%#.8x = %#.8x\n",
                   &tsec_reg->tfrg, tsec_reg->tfrg);
        }

        if (disp_blks & 0x0080) {
            printf("\neTSEC%d Counter Control and TOE Statistics Registers -\n",
                   cnt);
            printf("CAR1         @%#.8x = %#.8x\n",
                   &tsec_reg->car1, tsec_reg->car1);
            printf("CAR2         @%#.8x = %#.8x\n",
                   &tsec_reg->car2, tsec_reg->car2);
            printf("CAM1         @%#.8x = %#.8x\n",
                   &tsec_reg->cam1, tsec_reg->cam1);
            printf("CAM2         @%#.8x = %#.8x\n",
                   &tsec_reg->cam2, tsec_reg->cam2);
            printf("RREJ         @%#.8x = %#.8x\n",
                   &tsec_reg->rrej, tsec_reg->rrej);
        }

        if (disp_blks & 0x0100) {
            printf("\neTSEC%d HASH Function Registers -\n", cnt);
            printf("IGADDR0      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr0, tsec_reg->iaddr0);
            printf("IGADDR1      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr1, tsec_reg->iaddr1);
            printf("IGADDR2      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr2, tsec_reg->iaddr2);
            printf("IGADDR3      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr3, tsec_reg->iaddr3);
            printf("IGADDR4      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr4, tsec_reg->iaddr4);
            printf("IGADDR5      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr5, tsec_reg->iaddr5);
            printf("IGADDR6      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr6, tsec_reg->iaddr6);
            printf("IGADDR7      @%#.8x = %#.8x\n",
                   &tsec_reg->iaddr7, tsec_reg->iaddr7);
            printf("GADDR0       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr0, tsec_reg->gaddr0);
            printf("GADDR1       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr1, tsec_reg->gaddr1);
            printf("GADDR2       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr2, tsec_reg->gaddr2);
            printf("GADDR3       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr3, tsec_reg->gaddr3);
            printf("GADDR4       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr4, tsec_reg->gaddr4);
            printf("GADDR5       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr5, tsec_reg->gaddr5);
            printf("GADDR6       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr6, tsec_reg->gaddr6);
            printf("GADDR7       @%#.8x = %#.8x\n",
                   &tsec_reg->gaddr7, tsec_reg->gaddr7);
        }

        if (disp_blks & 0x0400) {
            printf("\neTSEC%d DMA Attribute Registers -\n", cnt);
            printf("ATTR         @%#.8x = %#.8x\n",
                   &tsec_reg->attr, tsec_reg->attr);
            printf("ATTRELI      @%#.8x = %#.8x\n",
                   &tsec_reg->attreli, tsec_reg->attreli);
        }

        if (disp_blks & 0x0800) {
            printf("\neTSEC%d Lossless Flow Control Registers -\n", cnt);
            printf("RQPRM0       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm0, tsec_reg->rqprm0);
            printf("RQPRM1       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm1, tsec_reg->rqprm1);
            printf("RQPRM2       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm2, tsec_reg->rqprm2);
            printf("RQPRM3       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm3, tsec_reg->rqprm3);
            printf("RQPRM4       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm4, tsec_reg->rqprm4);
            printf("RQPRM5       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm4, tsec_reg->rqprm5);
            printf("RQPRM6       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm6, tsec_reg->rqprm6);
            printf("RQPRM7       @%#.8x = %#.8x\n",
                   &tsec_reg->rqprm7, tsec_reg->rqprm7);
            printf("RFBPTR0      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr0, tsec_reg->rfbptr0);
            printf("RFBPTR1      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr1, tsec_reg->rfbptr1);
            printf("RFBPTR2      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr2, tsec_reg->rfbptr2);
            printf("RFBPTR3      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr3, tsec_reg->rfbptr3);
            printf("RFBPTR4      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr4, tsec_reg->rfbptr4);
            printf("RFBPTR5      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr5, tsec_reg->rfbptr5);
            printf("RFBPTR6      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr6, tsec_reg->rfbptr6);
            printf("RFBPTR7      @%#.8x = %#.8x\n",
                   &tsec_reg->rfbptr7, tsec_reg->rfbptr7);
        }

    /*
     * Not all timer registers are required for all etsecs.
     * Only one of the following registers is required for the
     * entire group of etsecs:
     * TMR_CTRL, TMC_CNTH/L, TMR_ADD, TMR_ACC, TMR_PRSC, FIPER[1:3],
     * TMR_ALARM1H/L, TMRALARM2H/L, TMR_ETTS1H/L, TMR_ETTS2H/L
     */
        if (disp_blks & 0x1000) {
            printf("\neTSEC%d Timer Registers -\n", cnt);
            printf("TMR_CTRL     @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_ctrl, tsec_reg->tmr_ctrl);
            printf("TMR_TEVENT   @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_tevent, tsec_reg->tmr_tevent);
            printf("TMR_TEMASK   @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_temask, tsec_reg->tmr_temask);
            printf("TMR_PEVENT   @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_pevent, tsec_reg->tmr_pevent);
            printf("TMR_PEMASK   @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_pemask, tsec_reg->tmr_pemask);
            printf("TMR_STAT     @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_stat, tsec_reg->tmr_stat);
            printf("TMR_CNT_H    @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_cnt_h, tsec_reg->tmr_cnt_h);
            printf("TMR_CNT_L    @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_cnt_l, tsec_reg->tmr_cnt_l);
            printf("TMR_ADD      @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_add, tsec_reg->tmr_add);
            printf("TMR_ACC      @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_acc, tsec_reg->tmr_acc);
            printf("TMR_PRSC     @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_prsc, tsec_reg->tmr_prsc);
            printf("TMROFF_H     @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_off_h, tsec_reg->tmr_off_h);
            printf("TMROFF_L     @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_off_l, tsec_reg->tmr_off_l);
            printf("TMR_ALARM1_H @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_alarm1_h, tsec_reg->tmr_alarm1_h);
            printf("TMR_ALARM1_L @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_alarm1_l, tsec_reg->tmr_alarm1_l);
            printf("TMR_ALARM2_H @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_alarm2_h, tsec_reg->tmr_alarm2_h);
            printf("TMR_ALARM2_L @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_alarm2_l, tsec_reg->tmr_alarm2_l);
            printf("FIPER1       @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_fiper1, tsec_reg->tmr_fiper1);
            printf("FIPER2       @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_fiper2, tsec_reg->tmr_fiper2);
            printf("FIPER3       @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_fiper3, tsec_reg->tmr_fiper3);
            printf("TMR_ETTS1_H  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_etts1_h, tsec_reg->tmr_etts1_h);
            printf("TMR_ETTS1_L  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_etts1_l, tsec_reg->tmr_etts1_l);
            printf("TMR_ETTS2_H  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_etts2_h, tsec_reg->tmr_etts2_h);
            printf("TMR_ETTS2_L  @%#.8x = %#.8x\n",
                   &tsec_reg->tmr_etts2_l, tsec_reg->tmr_etts2_l);
        }
        if (disp_blks & 0x2000) {
            printf("\neTSEC%d Interrupt Registers -\n", cnt);
            printf("IEVENT       @%#.8x = %#.8x\n",
                   &tsec_reg->ievent, tsec_reg->ievent);
            printf("IMASK        @%#.8x = %#.8x\n",
                   &tsec_reg->imask, tsec_reg->imask);
            printf("EDIS         @%#.8x = %#.8x\n",
                   &tsec_reg->edis, tsec_reg->edis);
        }
        if (disp_blks & 0x4000) {
            dev_id = tsec_reg->tbipa;
            printf("\neTSEC%d TBI MII Registers, dev_id %#x -\n",
                   cnt, dev_id);
            show_phy_reg(tsec_mdio, dev_id);
        }
    }
}

/***********************************************************************
 * Name: display_etsec_regs
 *
 * Description: Display contents of the eTSEC registers.
 *    The GE ports are numbered 0 - 3 while the eTSEC blocks
 *    are numbered 1 - 4.  GE port 0 attaches to eTSEC block 1
 *    and so on.
 *
 * Input: none
 *
 * Output: PASS/FAIL 
 *         
 ***********************************************************************
 */
int display_etsec_regs (void)
{
    int etsec_num, disp_blks;

    etsec_num = getdec_answer("Enter eTSEC port number (1-3, 0 for all)",
                               3, 0, 3);

    printf("\n      ---eTSEC Register Display---");
    printf("\n 0x8000: Tx/Rx Loopback Test Related Registers");
    printf("\n 0x0001: General Control and Status Registers");
    printf("\n 0x0002: Transmit Control and Status Registers");
    printf("\n 0x0004: Receive Control and Status Registers");
    printf("\n 0x0008: MAC Registers");
    printf("\n 0x0010: Transmit and Receive Counters");
    printf("\n 0x0020: Receive Counters");
    printf("\n 0x0040: Transmit Counters");
    printf("\n 0x0080: Counter Control and TOE Statistics Registers");
    printf("\n 0x0100: HASH Function Registers");
    printf("\n 0x0200: FIFO Control Registers");
    printf("\n 0x0400: DMA Attribute Registers");
    printf("\n 0x0800: Lossless Flow Control Registers");
    printf("\n 0x1000: Timer Registers");
    printf("\n 0x2000: Interrupt Registers");
    printf("\n      ---TBI Register Display---");
    printf("\n 0x4000: TBI MII Registers");

    disp_blks = gethex_answer("\n\nEnter eTSEC registers to display (Bitwise OR)",
                              0x8000, 0, 0x7fff);

    show_etsec_regs(etsec_num, disp_blks);

    return(PASSED);
}


/***********************************************************************
 * Name: check_tsec_rx_frame
 *
 * Description: Check for validity of TSEC receive buffer size and data
 *
 * Input: tx_bd_ptr - Points to the transmit buffer descriptor
 *        rx_bd_ptr - Points to the receive buffer descriptor
 *        packet_num
 *
 * Output: Receive buffer status, CNT_MISMATCH, DATA_MISMATCH
 *
 ***********************************************************************
 */
int check_tsec_rx_frame (volatile tsec_bd_t *tx_bd, 
                         volatile tsec_bd_t *rx_bd)
{
    int count, error = 0;
    int *rd_ptr, *wr_ptr;

    /*
     * verify that we received the correct number of bytes
     * the byte count in tx_bd->length does not include 4 bytes of CRC
     * while the byte count in rx_bd->length does include it
     */
    if (tx_bd->length + CRC_SIZE != rx_bd->length) {
        sprintf(err_msg, "%s, [#%d]:Packet tx/rx length error, sent %#x bytes, "
              "received %#x bytes\ntx_bd @%#.8x, rx_bd @%#.8x",
            __FUNCTION__, __LINE__, tx_bd->length + CRC_SIZE, rx_bd->length,
            &tx_bd->status, &rx_bd->status);
        print_err(FALSE, err_msg, LVL_1);
        error |= CNT_MISMATCH;
    }

    /*
     * If no errors then compare the tx and rx
     * otherwise don't since may not have gotten any data
     */
   
    if (!error) {
        rd_ptr = (int *)vir_addr(rx_bd->buf_ptr);
        wr_ptr = (int *)vir_addr(tx_bd->buf_ptr);
#ifdef DEBUG	
	printf("\nRX buffer : \n");
	dismem((unsigned char *)(rd_ptr), 128,
	   (unsigned)(rd_ptr), 4);
	printf("\nTX buffer : \n");
	dismem((unsigned char *)(wr_ptr), 128,
	   (unsigned)(wr_ptr), 4);
#endif	
        for (count = 0; count < (tx_bd->length) / 4; count++) {
            if (*rd_ptr != *wr_ptr) {
                sprintf(err_msg, "%s, [#%d]:Packet data mismatch at offset %#.8x, "
                      "sent %#.8x, rcvd %#.8x, tx bd @%#.8x, rx bd @%#.8x",
                    __FUNCTION__, __LINE__, count, *wr_ptr, *rd_ptr, tx_bd, rx_bd);
                print_err(FALSE, err_msg, LVL_1);
                error |= DATA_MISMATCH;
                break;
            }
            rd_ptr++;
            wr_ptr++;
        }
    } 

    /* mark frame as processed */
    rx_bd->status &= ~PQUICC_BDSTAT_RX_RO1;
    rx_bd->status |= PQUICC_BDSTAT_RX_EMPTY;

    return (error);
}

/***********************************************************************
 * Name: check_for_bd_wrap
 *
 * Description:
 *    This function will compare the etsec tbptr/rbptr with
 *    tbase/rbase.  If they are equal, it means that a BD wrap
 *    has occurred, in which case, we need to reinitialize the
 *    buffer descriptor ring.
 *
 * Input: option: etsec_num
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int check_for_bd_wrap (int etsec_num)
{
    int flag, retval;
    ccsr_tsec_t *regs;
    tsec_info_struct_t *tsec_p;

    flag = FALSE;
    retval = PASSED;
    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
    regs = (ccsr_tsec_t *)tsec_p->reg_base_addr;

    if (regs->rbptr == regs->rbase) {
        retval = etsec_stop(etsec_num);
        if (retval == PASSED) {
            init_rx_desc((tsec_bd_t *)vir_addr(tsec_p->rx_bd), tsec_p->rx_buf);
            flag = TRUE;
            msleep(1000);
        }
    }

    if (regs->tbptr == regs->tbase) {
        retval = etsec_stop(etsec_num);
        if (retval == PASSED) {
            init_tx_desc((tsec_bd_t *)vir_addr(tsec_p->tx_bd), tsec_p->tx_buf);
            flag = TRUE;
            msleep(1000);
        }
    }

    if (flag == TRUE) {
        etsec_start(etsec_num, TRUE);
    }

    return (retval);
}


/***********************************************************************
 * Name: patriot_mac_lpbk_test
 *
 * Description:
 *    This test will send/receive ethernet message packets using intrs.
 *    The number of ethernet frames sent will be more than there are
 *    buffer descriptors in the Tx/Rx ring to verify the ability to
 *    send an unlimited number of frames.  
 *    This test should only be run from module submenu.
 *
 * Input: option: none
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int patriot_mac_lpbk_test(void)
{
    int retval, count, frame, etsec_num, num_bytes;
    int cnt, speed, i, j;
    uchar       base_val, inc_val = 0;
    ccsr_tsec_t *regs;
    tsec_bd_t *tx_bd, *rx_bd, *tx_bd_vir_addr, *rx_bd_vir_addr;
    fe_packet_t *tx_buf, *tx_buf_vir_addr;
    tsec_info_struct_t *tsec_p;
    unsigned char *temp_p, *rx_buf_phy, *rx_buf_vir;
    unsigned char *tx_buf_phy, *tx_buf_vir;

    unsigned short pak_size[NUM_RX_BD] = {64, 108, 512, 256,
                                          1490, 65, 1411, 128,
                                          66, 719};
    
    etsec_num = ETSEC2;

    /*
     * initialize the etsec for Ethernet operation
     * default preamble, full duplex, append CRC, 1000Mbps, flow control
     */
    if (etsec_init(etsec_num, SGMII_LPBK_MAC, /*INTR_MODE*/POLL_MODE, TRUE) == FAILED) {
        return(TO_HOST_FREESCALE_MAC_LPBK_TEST_FAIL);
    }

    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
    /* start the etsec Tx/Rx DMA */
    etsec_start(etsec_num, TRUE);
#ifdef DEBUG
    dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x80, vir_addr(tsec_p->rx_bd),
		       BW_32BITS);
#endif
    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
#ifdef DEBUG
    printf("\ntsec_p = 0x%08x", tsec_p);
    printf("\n%s", tsec_p->name);
    printf("\ntsec_p->tx_bd = 0x%08x", tsec_p->tx_bd);
#endif    
    regs = (ccsr_tsec_t *)tsec_p->reg_base_addr;

    retval = TO_HOST_FREESCALE_MAC_LPBK_TEST_OK;
    for (i = 0; i < NUM_RX_BD; i++) {
	tx_bd = (tsec_bd_t *)etsec_get_txbd(tsec_p);
	tx_bd_vir_addr = (tsec_bd_t *)vir_addr(tx_bd);
	tx_buf = (fe_packet_t *)tx_bd_vir_addr->buf_ptr;
	tx_buf_vir_addr = (fe_packet_t *)vir_addr(tx_buf);
#ifdef DEBUG
	printf("\ntx_bd = 0x%08x", tx_bd);
	printf("\ntx_bd_vir_addr = 0x%08x", tx_bd_vir_addr);
	printf("\ntx_buf = 0x%08x", tx_buf);
	printf("\ntx_buf_vir_addr = 0x%08x", tx_buf_vir_addr);
#endif	
	memcpy((char *)&(tx_buf_vir_addr->eth_hdr.dest_addr),
	       (char *)host_mac_addr,
	       MAC_ADDR_SIZE);
	memcpy((char *)&(tx_buf_vir_addr->eth_hdr.src_addr),
	       (char *)module_mac_addr,
	       MAC_ADDR_SIZE);

	for (j = 0; j < pak_size[i] - sizeof(ether_hdr_t); j++ ) {
	    tx_buf_vir_addr->data[j] = j;
	}
	
	tx_bd_vir_addr->length = pak_size[i];

	    
#ifdef DEBUG
	printf("\necntrl %#.8x, maccfg1 @%#x=%#.8x, maccfg2 @%#x=%#.8x\n",
	       regs->ecntrl, &regs->maccfg1, regs->maccfg1,
	       &regs->maccfg2, regs->maccfg2);
#endif
	
	etsec_recv_nframes[etsec_num - 1] = 0; /* Clear receive frame counter */
	etsec_tx_nframes[etsec_num - 1] = 0; /* Clear transmit frame counter */
#ifdef DEBUG		
	printf("\n Before TX\n");
	dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x80, vir_addr(tsec_p->rx_bd),
	       BW_32BITS);
#endif
	tx_bd_vir_addr = (tsec_bd_t *)vir_addr(tx_bd);
	tx_buf_phy = (uchar *)tx_bd_vir_addr->buf_ptr;
	tx_buf_vir = (uchar *)vir_addr(tx_buf_phy);
	printf("\ntx_buf_vir = 0x%08x\n", tx_buf_vir);
	temp_p = tx_buf_vir - 32;
#ifdef DEBUG	
	dismem((uchar *)tx_buf_vir, 0x40, tx_buf_vir, BW_32BITS);
#endif
	/* Transmit the frame */
	if (etsec_send(etsec_num, tx_bd_vir_addr) != 0) {
	    sprintf(err_msg, "\n%s, [#%d]:Unable to transmit frame%d to eTSEC%d, "
		   "txbd @%#x", __FUNCTION__, __LINE__, frame, etsec_num, tx_bd);
	    print_err(TRUE, err_msg, LVL_0);
	    retval = TO_HOST_FREESCALE_MAC_LPBK_TEST_FAIL;
	    break;
	}
#ifdef DEBUG		
	printf("\nAfter etsec_send");
	dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x40, vir_addr(tsec_p->rx_bd),
	       BW_32BITS);
#endif		
	
	/* wait for frame reception */
	for (count = 100; count > 0; count--) {
	    if (etsec_recv_frame_ready(etsec_num, POLL_MODE)) {
		break;
	    }
	    msleep(1);
	}
	
	if (count == 0) {
	    break;
	}
	
	/* get received frame */
	rx_bd = (tsec_bd_t *)etsec_get_rxbd(tsec_p);
	
	if (rx_bd == 0) {
	    sprintf(err_msg, "\n%s, [#%d]:Unable to get rx_bd\n",
	    		__FUNCTION__, __LINE__);
	    print_err(TRUE, err_msg, LVL_0);
	    break;
	}
	
	rx_bd_vir_addr = (tsec_bd_t *)vir_addr(rx_bd);
	rx_buf_phy = (uchar *)rx_bd_vir_addr->buf_ptr;
	rx_buf_vir = (uchar *)vir_addr(rx_buf_phy);
#ifdef DEBUG	
	printf("\nrx_buf_vir = 0x%08x\n", rx_buf_vir);
	dismem((uchar *)rx_buf_vir, 0x40, rx_buf_vir, BW_32BITS);
#endif	
	if (rx_bd == 0) {
	    sprintf(err_msg, "%s, [#%d]:Error receiving frame %d on eTSEC%d, "
		   "rxbd @%#x=%#x", __FUNCTION__, __LINE__,
		   frame, etsec_num, &regs->rbptr, regs->rbptr);
	    print_err(TRUE, err_msg, LVL_0);
	    retval = TO_HOST_FREESCALE_MAC_LPBK_TEST_FAIL;
	    break;
	} else {
	    /* verify transmission and reception status */
	    retval  = check_tsec_tx_status(tx_bd_vir_addr);
	    retval |= check_tsec_rx_status(rx_bd_vir_addr);
	    retval |= check_tsec_rx_frame (tx_bd_vir_addr, rx_bd_vir_addr);
	    
	    /*
	     * if wrap occurs, we must re-initialize the tx and rx
	     * buffer descriptors so that we can Tx/Rx more frames
	     */
	    retval = check_for_bd_wrap(etsec_num);
	    if (retval == TO_HOST_FREESCALE_MAC_LPBK_TEST_FAIL) {
		break;
	    }
	}
    }
    cleanup_tsec(etsec_num);
    return (retval);
}

#ifdef DEBUG_SEND_A_PACKET
int patriot_send_a_packet(void)
{
    int retval, count, frame, etsec_num, num_bytes;
    int cnt, speed, i, j;
    uchar       base_val, inc_val = 0;
    ccsr_tsec_t *regs;
    tsec_bd_t *tx_bd, *rx_bd, *tx_bd_vir_addr, *rx_bd_vir_addr;
    fe_packet_t *tx_buf, *tx_buf_vir_addr;
    tsec_info_struct_t *tsec_p;
    unsigned char *temp_p;
    uchar src_mac_addr[MAC_ADDR_SIZE] = {0x00, 0x04, 0x9f, 0xef, 0x01, 0x01}; /* Eval */
    uchar dst_mac_addr[MAC_ADDR_SIZE] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    unsigned short pak_size[NUM_RX_BD] = {64, 108, 512, 256,
                                          1490, 65, 1411, 128,
                                          66, 719};
    
    etsec_num = ETSEC2;
#if 0
    /*
     * initialize the etsec for Ethernet operation
     * default preamble, full duplex, append CRC, 1000Mbps, flow control
     */
    if (etsec_init(etsec_num, SGMII_LPBK_MAC, POLL_MODE, TRUE) == FAILED) {
        return(FAILED);
    }

    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
    /* start the etsec Tx/Rx DMA */
    etsec_start(etsec_num, TRUE);
#ifdef DEBUG
    dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x80, vir_addr(tsec_p->rx_bd),
		       BW_32BITS);
#endif
#endif    
    tsec_p = (tsec_info_struct_t *)etsec_get_info_ptr(etsec_num);
#ifdef DEBUG
    printf("\ntsec_p = 0x%08x", tsec_p);
    printf("\n%s", tsec_p->name);
    printf("\ntsec_p->tx_bd = 0x%08x", tsec_p->tx_bd);
#endif    
    regs = (ccsr_tsec_t *)tsec_p->reg_base_addr;

    retval = PASSED;
    for (i = 0; i < 1; i++) {
	tx_bd = (tsec_bd_t *)etsec_get_txbd(tsec_p);
	tx_bd_vir_addr = (tsec_bd_t *)vir_addr(tx_bd);
	tx_buf = (fe_packet_t *)tx_bd_vir_addr->buf_ptr;
	tx_buf_vir_addr = (fe_packet_t *)vir_addr(tx_buf);
#ifdef DEBUG
	printf("\ntx_bd = 0x%08x", tx_bd);
	printf("\ntx_bd_vir_addr = 0x%08x", tx_bd_vir_addr);
	printf("\ntx_buf = 0x%08x", tx_buf);
	printf("\ntx_buf_vir_addr = 0x%08x", tx_buf_vir_addr);
#endif	
	memcpy((char *)&(tx_buf_vir_addr->eth_hdr.dest_addr), (char *)dst_mac_addr,
	       MAC_ADDR_SIZE);
	memcpy((char *)&(tx_buf_vir_addr->eth_hdr.src_addr), (char *)src_mac_addr,
	       MAC_ADDR_SIZE);

	for (j = 0; j < pak_size[i] - sizeof(ether_hdr_t); j++ ) {
	    tx_buf_vir_addr->data[j] = j;
	}
	
	tx_bd_vir_addr->length = pak_size[i];

	    
#ifdef DEBUG
	printf("\necntrl %#.8x, maccfg1 @%#x=%#.8x, maccfg2 @%#x=%#.8x\n",
	       regs->ecntrl, &regs->maccfg1, regs->maccfg1,
	       &regs->maccfg2, regs->maccfg2);
#endif
	
#ifdef DEBUG		
	printf("\n Before TX\n");
	dismem((uchar *)vir_addr(tsec_p->rx_bd), 0x80, vir_addr(tsec_p->rx_bd),
	       BW_32BITS);
#endif		
	/* Transmit the frame */
	if (etsec_send(etsec_num, tx_bd_vir_addr) != 0) {
	    printf("\nUnable to transmit frame%d to eTSEC%d, "
		   "txbd @%#x", frame, etsec_num, tx_bd);
	    retval = FAILED;
	    break;
	}
    }
    return retval;
}
#endif
/*------------------------------------------------------------------------------
 * $Log: p1021_ethernet.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.5  2012/12/04 13:04:44  steja
 * 1. backing back the DLB to ALB for framer interrupt
 * 2. add missing error message that left before.
 *
 * Revision 1.4  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.3  2012/10/15 21:15:40  huanngo
 * Adding CRC to the ethernet packet and reading host MAC address from ethernet header
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.9  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.8  2012/03/27 07:50:00  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.7  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.6  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.5  2011/12/01 18:51:05  huanngo
 * Support new command to write MAC address to EEPROM and fix bugs
 *
 * Revision 1.1.4.4  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.3  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:23  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.7  2011/08/06 00:17:39  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.6  2011/07/19 06:11:34  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.5  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.4  2011/07/08 00:08:48  huanngo
 * Clean up code
 *
 * Revision 1.1.2.3  2011/06/28 06:27:55  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.2  2011/06/09 01:28:09  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.1  2011/05/02 23:33:22  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */








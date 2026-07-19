/* $Id: libeth.c,v 1.3 2012/08/15 15:02:52 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libeth.c,v $
 *------------------------------------------------------------------
 * libeth.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * libeth.c
 *
 *  Created on: Dec 9, 2009
 *      Author: dokim
 */

#include <stdint.h>
#include <stdio.h>

#include "lsi_sp27xx_reg.h"
#include "sp_ppb_eth.h"
#include "sp_ppb_icmp.h"
#include "sp_ppb_ipv4.h"
#include "sp_ppb_udp.h"
#include "libeth.h"
#include "libgeneric.h"
#include "libserdes.h"
#include "libuart.h"

#define PCE_DEBUG 1

PCE_DEVICE_S pce0;
PCE_DEVICE_S pce1;
TXD_DEVICE_S txd0;
TXD_DEVICE_S txd1;

extern void bsp_debug_printf (const char *fmt, ...);

extern void do_mem_md(char* cmdargs);
#ifdef DEBUG_ETHERNET_LIB
/* for debug */
lsi_sp27xx_pce_reg_s  MyPCE0reg, *p_MyPCE0reg;
lsi_sp27xx_pce_reg_s  MyPCE1reg, *p_MyPCE1reg;
#endif

inline static void
memcpy8 (						/* ARM version of memcpy can't handle un-aligned buffers */
	uint8_t *pdest, 			/* in: buffer to copy into NOTE: buffer overlap not checked!!! */
	const uint8_t *psrc, 		/* in: buffer to copy from */
	uint32_t size)				/* in: number of bytes to copy */
{
	while( size-- )
	{
		*pdest++ = *psrc++;
	}
}

void
sp_EthSoftRst(			/* reset both EMAC interfaces: MAC, PCE & TXD - call after every loopback test */
	void)
{
	REG32_WRITE(LSI_SP27XX_CAR_RSTPROT_RA, 0x50F1C917);

	REG32_SET_BITS(LSI_SP27XX_CAR_RSTCTL_RA, LSI_SP27XX_RSTCTL_SWRESET_MC0_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_MS0_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_PCE0_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_TXD0_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_MC1_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_MS1_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_PCE1_BM\
											|LSI_SP27XX_RSTCTL_SWRESET_TXD1_BM);

	REG32_RESET_BITS(LSI_SP27XX_CAR_RSTCTL_RA, LSI_SP27XX_RSTCTL_SWRESET_MC0_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_MS0_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_PCE0_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_TXD0_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_MC1_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_MS1_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_PCE1_BM\
											  |LSI_SP27XX_RSTCTL_SWRESET_TXD1_BM);

	REG32_WRITE(LSI_SP27XX_CAR_RSTPROT_RA, 0x0);

//	if(sp_GigeSerdesInit(GBAUD_1_25)<0)
//	{
//		while(1);
//	}
}

int32_t sp_EthSetupMAC(sp27xxMacID_t eth_port, ethMode_t eth_mode, phyMode_t phymode)
{
	int32_t ret = 0;

	if(eth_mode == SSSMII)
	{
		ret = emac_init_ssmii(eth_port, phymode);
	}
	else
	{
		ret = emac_init_sgmii(eth_port, phymode);
	}

	return ret;
}

int32_t sp_EthSetupPCE(sp27xxMacID_t eth_port,	fwdMode_t fwd_mode,	MAC_ADDR* mac_addr, uint32_t dlt_init, uint32_t dltb_en, routeOption mac_route, uint32_t classification\
		, void* qstart, uint32_t n_elements, void* rxbuf)
{
#if 0
	/* for test */
	/* clear all the pce interrupt status */
	REG32_SET_BITS(LSI_SP27XX_PCE_INTERRUPT_STATUS_0_RA(eth_port), 0xFFFFFFFF);
	REG32_SET_BITS(LSI_SP27XX_PCE_INTERRUPT_STATUS_1_RA(eth_port), 0xFFFFFFFF);

	for(i=0; i<6; i++)
	{
		REG32_SET_BITS(LSI_SP27XX_PCE_BD_INT_STATUS_RA(eth_port,i), 0xFFFFFFFF);
	}
#endif
//***#if PCE_DEBUG
    //***sp_SerialPutS(" \r\n In sp_EthSetupPCE()");
    //***bsp_debug_printf(" \r\n parameters : eth_port:%d, fwd_mode:%d,  dlt_init:%d, dltb_en:%d, classification:%d, qstart:0x%x, n_elements:%d, rxbuf:0x%x ", eth_port, fwd_mode, dlt_init, dltb_en, classification, qstart, n_elements, rxbuf);
//***#endif
	if(pce_reset(eth_port)<0)
	{
		return ERROR;
	}
//***#ifdef PCE_DEBUG
    //***sp_SerialPutS(" \r\n pce_reset()");
    //***sp_SerialPutS(" \r\n eth_port at beginig = ");
    //***sp_SerialPutLong(eth_port, 'd');
//***#endif

	if(pce_init_bd(eth_port, (void *)qstart, n_elements, rxbuf)<0)
	{
		return ERROR;
	}
//---#ifdef PCE_DEBUG
    //---sp_SerialPutS(" \r\n eth_port after pce_init_bd  = ");
    //---sp_SerialPutLong(eth_port, 'd');
    //---sp_SerialPutS(" \r\n pce_init_bd()");
//---#endif

	pce_set_fwd_mode(eth_port, fwd_mode);
//---#ifdef PCE_DEBUG
    //---sp_SerialPutS(" \r\n pce_set_fwd_mode()");
    //---sp_SerialPutS(" \r\n eth_port = ");
    //---sp_SerialPutLong((int)eth_port, 'd');
    //---bsp_debug_printf(" \r\n parameters : eth_port:%d, fwd_mode:%d,  dlt_init:%d, dltb_en:%d, classification:%d, qstart:0x%x, n_elements:%d, rxbuf:0x%x ", eth_port, fwd_mode, dlt_init, dltb_en, classification, qstart, n_elements, rxbuf);
//---#endif
	pce_disable_COR(eth_port);
//---#ifdef PCE_DEBUG
    //---sp_SerialPutS(" \r\n pce_disable_COR()");
//---#endif

	pce_set_loaddr(eth_port, mac_addr[0]);
//---#ifdef PCE_DEBUG
    //---sp_SerialPutS(" \r\n pce_set_loaddr()");
//---#endif
	pce_set_rmtaddr(eth_port, mac_addr[1]);
//---#ifdef PCE_DEBUG
    //---sp_SerialPutS(" \r\n pce_set_rmtaddr()");
//---#endif

	if(mac_route == MAC_ADDR_ROUTING)
	{
		pce_set_mac_based_route(eth_port);

		pce_set_maddr_dss0que(eth_port, mac_addr[2]);
		pce_set_maddr_dss1que(eth_port, mac_addr[3]);
		pce_set_maddr_dss2que(eth_port, mac_addr[4]);
		pce_set_maddr_dss3que(eth_port, mac_addr[5]);
#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n after MAC_ADDR_ROUTING()");
#endif
	}

	if(dlt_init!=0)
	{
		if(pce_set_dlt_init(eth_port)<0)
		{
			return ERROR;
		}
#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n pce_set_dlt_init()");
#endif
	}

	if(dltb_en!=0)
	{
		if(pce_set_dltb_en(eth_port)<0)
		{
			return ERROR;
		}
#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n pce_set_dltb_en()");
#endif
	}

	pce_enable(eth_port, classification);
#ifdef PCE_DEBUG
    sp_SerialPutS(" \r\n pce_enable()");
#endif

	return SUCCESS;
}

int32_t sp_EthChkMACLinkOk(sp27xxMacID_t eth_port)
{
	return (int32_t)CHK_REG_MASK(LSI_SP27XX_MAC_ANSTAT_RA(eth_port), LSI_SP27XX_MAC_ANSTAT_GMII_LINK_OK_BM);
}

int32_t sp_EthSetupDLTEntry(sp27xxMacID_t eth_port,	uint32_t dest_rxq,	uint32_t dlt_entry_from, uint32_t dlt_entry_to)
{
	return (pce_set_dlt_entry(eth_port, dlt_entry_from, dlt_entry_to, dest_rxq));
}

int32_t sp_EthSetupDLTEntryMaxMin(sp27xxMacID_t eth_port, uint32_t dlta_dltb, uint16_t max,	uint16_t min)
{
	if(dlta_dltb == 0)
	{
		pce_set_udp_dest_max_min_porta(eth_port, max, min);
	}
	else
	{
		pce_set_udp_dest_max_min_portb(eth_port, max, min);
	}
	return (0);
}

int32_t sp_EthSetupTXD(sp27xxMacID_t eth_port,	fwdPath_t fwd_path, void* qstart, uint32_t n_elements)
{
	uint32_t i;

	if(txd_init_bd(eth_port, (void *)qstart, n_elements)<0)
	{
		return ERROR;
	}

	/* setup water-mark registers for both dma and remote fifos*/
	txd_dma_watermark_set(eth_port, 224, 128);
	txd_rmt_watermark_set(eth_port, 1000);

	txd_txfwd_set(eth_port, fwd_path);
	txd_txsched_set(eth_port, 9);

	for(i=0; i<9; i++)
	{
		txd_txsched_add_q_slot(eth_port, i, i);
		txd_txq_rst(eth_port, i);
	}

	return SUCCESS;
}

int32_t sp_EthRegRawPktToTxq(sp27xxMacID_t eth_port, uint32_t txq, uint8_t *tx_buf, uint32_t pkt_size)
{
	uint32_t xmit_sz = pkt_size;
	uint8_t *p_buf = tx_buf;
	int32_t i, ii;
	int32_t current_index, start_index;

	TXD_BQUE* p_current;
	TXD_DEVICE_S* txd_temp;

	int32_t done = 0;

	/* src_q should be less or equal than highest number of que */
	if(txq > (HW_REG_ACCESS(LSI_SP27XX_TXD_SCHEDCFG_RA(eth_port)) & 0x1F))
	{
		return (-1);
	}

	if(eth_port<1)
	{
		txd_temp = &txd0;
	}
	else
	{
		txd_temp = &txd1;
	}

	i = txd_temp->curr_buf_idx[txq];
	ii = txd_temp->curr_buf_idx[txq];

	txd_temp->tbq_current = &(txd_temp->tbq_start[ii]);

	p_current = txd_temp->tbq_current;
	current_index = txd_temp->curr_buf_idx[txq];
	start_index = txd_temp->start_buf_idx[txq];

	/* wait for DMA to finish */
	while(CHK_REG_MASK(LSI_SP27XX_TXD_STATUS_RA(eth_port), LSI_SP27XX_TXD_STATUS_ACTIVE_BM));

	if ( LSI_SP27XX_IS_V10_DEVICE() )
	{
		/* if the chip version is 1.0, to compensate BD prefetching, we should reset current BDPTR whenever
		 * packets are sent out
		 */

		/* check if previous pkt has been sent out */
		if(txd_temp->tbq_start[(ii==0)?(txd_temp->tbq_elements[txq]-1):(ii-1)].status.bits.own == 1)
		{
			REG32_WRITE(LSI_SP27XX_TXD_BDPTR_RA(eth_port,txq), txd_temp->tbq_current);
		}
	}

	/* Set up the buffers but don't allocate them to TXD yet  */
	while (1) {
		if (p_current->status.bits.own == 0) {
			/* not enough buffers */
			xmit_sz = -1;
			break;
		}

		/* all buffers but last are XMIT_BUF_MAX_SIZE,
		 * this will be overwritten when we reach the last buffer
		 */
		p_current->status.bits.length = XMIT_BUF_MAX_SIZE;
		p_current->status.bits.eof = 0;
		p_current->status.bits.sof = 0;

		/* set address in TX data frame */
		p_current->address = (uint32_t) p_buf;

		if (xmit_sz <= XMIT_BUF_MAX_SIZE) {
			break;
		}

		/* increment current and check for end of buffer pool */
		if (((p_current->status.reg) & TBQE_WRAP) == TBQE_WRAP) {
							i = current_index =
							    start_index;
						} else {
							current_index = i = i + 1;
						}

		p_current = &(txd_temp->tbq_start[i]);

		xmit_sz = xmit_sz - XMIT_BUF_MAX_SIZE;
		p_buf += XMIT_BUF_MAX_SIZE;
	}

	/* overwrite last buffer with residual size */
	p_current->status.bits.length = xmit_sz;

	if (xmit_sz > 0) {
	    /* ready to allocate the buffers we set up */
		txd_temp->tbq_current->status.bits.sof = 1;
	    // mark first buf in frame
	    p_current->status.bits.eof = 1;
	    // mark last buf in frame
	    do {
	    	txd_temp->tbq_current->status.bits.own = 0;
		// pass to TXD
		if (txd_temp->tbq_current->status.bits.eof == 1) {
		    done = 1;
		}

		if (((txd_temp->tbq_current->status.reg) & TBQE_WRAP) == TBQE_WRAP) {
							ii = txd_temp->curr_buf_idx[txq] =
									txd_temp->start_buf_idx[txq];
						} else {
							txd_temp->curr_buf_idx[txq] = ii = ii + 1;
						}
		txd_temp->tbq_current = &(txd_temp->tbq_start[ii]);

	    } while (done == 0) ;
	} else {
		/* We had a problem */
		return (PKT_ERR_NO_BUFFERS);
	}

	return (PKT_SUCCESS);
}

int32_t sp_EthTxStart(sp27xxMacID_t eth_port, uint32_t txq)
{
	return txd_xmit(eth_port, txq);
}

int32_t sp_EthRxRawPkt (sp27xxMacID_t eth_port,	uint32_t rxq, uint32_t* msg_sz, 
                        uint8_t *rx_buf)
{
//#ifdef PCE_DEBUG
    //int32_t i;
//#endif
    int32_t ii, cpy_sz;
    int32_t status = EMAC_IN_PROGRESS;
    uint32_t bdstat;
    int32_t num_buf = 0;
    int32_t sof = 0;
    int32_t eof = 0;
    char mem_disp[200];

    PCE_DEVICE_S* pce_temp;

//#ifdef PCE_DEBUG
 //   sp_SerialPutS(" \r\nIn sp_EthRxRawPkt \n");
//#endif

    if(eth_port<1) {
        pce_temp = &pce0;
    } else {
        pce_temp = &pce1;
    }

    ii = pce_temp->curr_buf_idx[rxq];

    bdstat = pce_temp->rbq_start[ii].status.reg;

    while (bdstat & RX_BQUE_STAT_OWN_BM) {
        bsp_debug_printf(" \r\nStart RX PCE#%d activity on Queue %d, bdstat = 0x%x, cur buf index = %d", eth_port, rxq, bdstat, ii);
        /* there is PCE activity to decode */
        if (bdstat & RX_BQUE_STAT_SOF_BM) {
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutS(" \r\n   (bdstat & RX_BQUE_STAT_SOF_BM) num_buf = 0");
//+++#endif
            num_buf = 0;
        } else {
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutS(" \r\n   !(bdstat & RX_BQUE_STAT_SOF_BM) num_buf = ");
//+++#endif
            num_buf++;
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutLong(num_buf, 'd');
//+++#endif
        }
        if (bdstat & RX_BQUE_STAT_EOF_BM) {
            cpy_sz = 0;
            *msg_sz = bdstat & RX_BQUE_STAT_LEN_BM;
            sp_SerialPutS(" \r\n   (bdstat & RX_BQUE_STAT_EOF_BM) msg_sz = ");
            cpy_sz = *msg_sz;
            sp_SerialPutLong(cpy_sz, 'd');
            if ((bdstat & RX_BQUE_STAT_LEN_BM) < num_buf * RCV_BUF_MAX_SIZE) {
                /* length of packet not consistent with LEN in BD */
                sp_SerialPutS(" \r\nlength of packet not consistent with LEN in BD too small");
                status = EMAC_ERR_LEN_TO_SMALL;
            } else if ((bdstat & RX_BQUE_STAT_LEN_BM) >=
                       (num_buf * RCV_BUF_MAX_SIZE + RCV_BUF_MAX_SIZE)){
                /* length of packet not consistent with LEN in BD */
                sp_SerialPutS(" \r\nlength of packet not consistent with LEN in BD too big");
                status = EMAC_ERR_LEN_TO_BIG;
            } else {	/* valid copy size */
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutS(" \r\n valid rx copy size = ");
//+++#endif
                cpy_sz = *msg_sz % RCV_BUF_MAX_SIZE;
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutLong(cpy_sz, 'd');
//+++#endif
            }
        } else {
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutS(" \r\n RCV_BUF_MAX_SIZE size = ");
//+++#endif
            cpy_sz = RCV_BUF_MAX_SIZE;
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutLong(cpy_sz, 'd');
//+++#endif
        }

/* copy the data, then release the buffer */
//+++#ifdef PCE_DEBUG
//+++sp_SerialPutS(" \r\n Rx rbq @addr: ");
//+++sp_SerialPutLong((uint32_t)pce_temp->rbq_start[ii].address, 'h');
//+++#endif
        memcpy8((uint8_t *)&rx_buf[(num_buf) * RCV_BUF_MAX_SIZE],
                (uint8_t *)pce_temp->rbq_start[ii].address, cpy_sz);
//+++#ifdef PCE_DEBUG
//+++sprintf(mem_disp, "0x%x %d",(unsigned int)&(pce_temp->rbq_start[ii]), 40);
//+++sp_SerialPutS("\r\n");
//+++sp_SerialPutS(mem_disp);
//+++sp_SerialPutS("\r\n");
//+++do_mem_md(mem_disp);
//+++#endif
        uint8_t *tp;
        tp = (uint8_t *)&(rx_buf[num_buf* RCV_BUF_MAX_SIZE]);
        bsp_debug_printf(" \r\n copied to local Buffer Rx Packet @addr: 0x%x, size = 0x%x[%d]", tp, cpy_sz, cpy_sz);
//sp_SerialPutLong((uint32_t)tp, 'h');
//sp_SerialPutS("\r\n");
//for (i=0;i<60;i++) {
//sp_SerialPutLong(tp[i], 'h');
//}
//sp_SerialPutS("\r\n Test if sprintf works : ");
        if (bdstat & RX_BQUE_STAT_EOF_BM) {
            sprintf(mem_disp, "0x%x %d",(unsigned int)rx_buf, 
                    (int)((num_buf* RCV_BUF_MAX_SIZE)+cpy_sz)/4);
            bsp_debug_printf("\r\n  %s (4bytes)", mem_disp);
            do_mem_md(mem_disp);
        }
        if ((bdstat & RX_BQUE_STAT_WRAP_BM) == RX_BQUE_STAT_WRAP_BM) {
        // emac->pce->rbq_start[ii].status.reg = RX_BQUE_STAT_WRAP_BM;
        } else {
        // emac->pce->rbq_start[ii].status.reg = 0;
        }
        pce_temp->rbq_start[ii].status.bits.own = 0;

        if (status != EMAC_IN_PROGRESS) {
            /* problems already - no need to look further */
            sp_SerialPutS(" \r\n problems already - no need to look further ");
        } else if ((num_buf * RCV_BUF_MAX_SIZE) > SP_PPB_ETH_MAX_FRM_SIZE) {
            status = EMAC_ERR_FRM_TOO_BIG;
            sp_SerialPutS(" \r\n EMAC_ERR_FRM_TOO_BIG ");
        } else {
            /* process buffers */
            if (bdstat & RX_BQUE_STAT_SOF_BM) {
                //sp_SerialPutS(" \r\n bdstat & RX_BQUE_STAT_SOF_BM ");
                if (sof) {
                    sp_SerialPutS(" \r\n SOF EMAC_ERR_SOFSOF");
                    status = EMAC_ERR_SOFSOF;
                }
                sof = 1;
                eof = 0;
            }
            if (bdstat & RX_BQUE_STAT_EOF_BM) {
                //sp_SerialPutS(" \r\n bdstat & RX_BQUE_STAT_EOF_BM ");
                if (bdstat & RX_BQUE_STAT_COR_BM) {
                    //sp_SerialPutS(" \r\n bdstat & RX_BQUE_STAT_COR_BM ");
                    /* PCE forces EOF on corrupt frames */
                    sof = eof = 0;
                    status = EMAC_ERR_CORRUPT;
                    sp_SerialPutS(" \r\n EMAC_ERR_CORRUPT");
                    /* BD status field is valid */
                } else if (!sof) {
                    /* found eof w/o sof */
                    status = EMAC_ERR_EOFNOSOF;
                    sp_SerialPutS(" \r\n EMAC_ERR_EOFNOSOF");
                } else if (eof) {
                    /* found two eofs w/o sof */
                    status = EMAC_ERR_EOFEOF;
                    sp_SerialPutS(" \r\n EMAC_ERR_EOFEOF");
                } else {
                    eof = 1;
                    sof = 0;
                    if (bdstat & RX_BQUE_STAT_CKSM_ERR_BM) {
                        //sp_SerialPutS(" \r\n bdstat & RX_BQUE_STAT_CKSM_ERR_BM");
                        status = EMAC_ERR_UDP_CKSUM;
                        sp_SerialPutS(" \r\n EMAC_ERR_UDP_CKSUM");
                    } else if (bdstat & RX_BQUE_STAT_MAC_ERR_BM) {
                        status = EMAC_ERR_MAC_ERROR;
                        sp_SerialPutS(" \r\n EMAC_ERR_MAC_ERROR");
                    } else if (bdstat & RX_BQUE_STAT_UNREC_L2_ERR_BM) {
                        status = EMAC_ERR_FMT_ERROR;
                        sp_SerialPutS(" \r\n RX_BQUE_STAT_UNREC_L2_ERR_BM EMAC_ERR_FMT_ERROR");
                    } else if (bdstat & RX_BQUE_STAT_UNREC_L3L4_ERR_BM) {
                        sp_SerialPutS(" \r\n bdstat & RX_BQUE_STAT_UNREC_L3L4_ERR_BM EMAC_ERR_FMT_ERROR");
                        status = EMAC_ERR_FMT_ERROR;
                    } else if (((bdstat & RX_BQUE_STAT_TYPE_BM) >> RX_BQUE_STAT_TYPE_BO)                                == PCE_BD_TYPE_ETH_IPV4_UDP) {
                        sp_SerialPutS(" \r\n RX_BQUE_STAT_TYPE_BM  RX_BQUE_STAT_T");
status = EMAC_SUCCESS;
                        sp_SerialPutS(" \r\n EMAC_SUCCESS");
                    } else {
                        status = EMAC_ERR_FMT_ERROR;
                        sp_SerialPutS(" \r\n EMAC_ERR_FMT_ERROR bdstat = ");
                        sp_SerialPutLong(bdstat, 'h');

                    }
                }
            }
        }
        /* get index of next buffer descriptor from circular queue */
        if ((bdstat & RX_BQUE_STAT_WRAP_BM) == RX_BQUE_STAT_WRAP_BM) {
            //sp_SerialPutS(" \r\n RX_BQUE_STAT_WRAP_BM ");

            ii = pce_temp->curr_buf_idx[rxq] =
            pce_temp->start_buf_idx[rxq];
        } else {
            //sp_SerialPutS(" \r\n ! RX_BQUE_STAT_WRAP_BM ");

            pce_temp->curr_buf_idx[rxq] = ii = ii + 1;
        }
        //sp_SerialPutS(" \r\n Read the next bd status ");
        bdstat = pce_temp->rbq_start[ii].status.reg;
        if (status != EMAC_IN_PROGRESS) {
            sp_SerialPutS(" \r\n Done RX (NOT EMAC_IN_PROGRESS) ");
            return(status);
        }
    }
    //sp_SerialPutS(" \r\n Put of while NOT bdstat & RX_BQUE_STAT_OWN_BM ");

    if (status == EMAC_IN_PROGRESS) {
        //sp_SerialPutS(" \r\n EMAC_ERR_NO_PKT ");

        return(EMAC_ERR_NO_PKT);
    }
    return(status);
}

/******************************************************************************/
static uint16_t calculateUdpChecksum(
	uint8_t *pIpAddresses,		/* pointer to packet's IP addresses */
	uint8_t *pUdpHeader,		/* pointer to packet's UDP Header */
	uint16_t udpLength			/* UDP length (bytes) */
	)
{
	uint32_t sum;				/* summation */
	uint16_t bytePair;			/* pair of bytes */

	/* Include the pseudo IP header. */
	sum = SP_PPB_IPV4_PROTO_UDP + ((uint32_t) udpLength);
	sum += (((uint32_t) pIpAddresses[0]) << 8) | ((uint32_t) pIpAddresses[1]);
	sum += (((uint32_t) pIpAddresses[2]) << 8) | ((uint32_t) pIpAddresses[3]);
	sum += (((uint32_t) pIpAddresses[4]) << 8) | ((uint32_t) pIpAddresses[5]);
	sum += (((uint32_t) pIpAddresses[6]) << 8) | ((uint32_t) pIpAddresses[7]);

	/* Add all byte pairs from the UDP packet. */
	while (udpLength >= 2)
	{
		bytePair = (((uint16_t) pUdpHeader[0]) << 8) |
				   ((uint16_t) pUdpHeader[1]);
		sum += (uint32_t) bytePair;
		pUdpHeader += 2;
		udpLength -= 2;
	}

	/* Add the last byte and a pad byte if the UDP length is odd. */
	if (udpLength > 0)
	{
		sum += ((uint32_t) *pUdpHeader) << 8;
	}

	/* Take care of end around carry. */
	while ((sum >> 16) != 0)
	{
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	/* Return with the 1's complement of the sum. */
	return (uint16_t) (~sum & 0xFFFF);
}

static uint16_t csum16(
	const uint16_t *p_data,
	uint32_t size
	)
{
	register uint32_t sum = 0;

	/* Add all 16-bit values */
	for( ;; )
	{
		if( size < 2 )
		{
			break;
		}
		else
		{
			sum += *p_data++;
			size -= 2;
		}
    }

    /* Add remaining 8 b-bit values */
	if( size )
	{
		sum += *((uint8_t *) p_data);
	}

	/* fold upper 16-bits */
	while ((size = (uint16_t) (sum >> 16)) != 0)
	{
		sum = ( (uint16_t) sum ) + size;
	}

    return (uint16_t) ( sum ^ ((uint32_t) 0x0000FFFF));
}

struct {
	int ethernet;
	int ipv4;
	int udp;
	int txhdr;
} sizes = {
	 sizeof(StarProPPB_ETH_HDR_t), sizeof(StarProPPB_IPv4_HDR_t),
	 sizeof(StarProPPB_UDP_HDR_t), sizeof(StarProPPB_TxHDR_t)
};

void
sp_EthAddUdpChksum(
	uint8_t *tx_data_start,  /* in: pointer to data buffer with UDP packet */
	uint16_t udp_payload_sz) /* in: UDP payload size in bytes (w/o any header) */
{
	uint16_t csum = 0;
	uint32_t offset = 0;

	/* check if the packet has vlan tag */
	if((*(tx_data_start+12)==0x81)&&(*(tx_data_start+13)==0))
	{
		offset += 4; /* vlan tag */

		if((unsigned short)((*(tx_data_start+16)<<8)+*(tx_data_start+17))<0x600)
		{
			offset += 8; /* snap llc header + snap field */
		}
	}
	else
	{
		if((unsigned short)((*(tx_data_start+12)<<8)+*(tx_data_start+13))<0x600)
		{
			offset += 8; /* snap llc header + snap field */
		}
	}

	/* clear checksum field in packet */
	memcpy8((uint8_t*) &tx_data_start[( sizeof(StarProPPB_ETH_HDR_t) + offset
		+ sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t)) - 2],
		(uint8_t*) &csum, 2);

	csum = calculateUdpChecksum(
		&tx_data_start[sizeof(StarProPPB_ETH_HDR_t) + SP_PPB_IPv4_HDR_WORD3_RO + offset],
		&tx_data_start[sizeof(StarProPPB_ETH_HDR_t) + sizeof(StarProPPB_IPv4_HDR_t) + offset],
		udp_payload_sz + sizeof(StarProPPB_UDP_HDR_t));
	csum = SP_PPB_HTONS(csum);

	/* insert checksum into packet */
	memcpy8((uint8_t*) &tx_data_start[( sizeof(StarProPPB_ETH_HDR_t) + offset
		+ sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t)) - 2],
		(uint8_t*) &csum, 2);
}

int32_t						/* ret: PKT_SUCCESS or error indication */
sp_EthBuildUdpPktIPv4(					/* NOTE: p_buf area > (payload_len + 48) bytes */
	MAC_ADDR	dst_mac_addr,	/* in: MACADDR to send to */
	MAC_ADDR	src_mac_addr,	/* in: MACADDR to send from */
	uint32_t 	dst_ip_addr,	/* in: IPADDR to send to  */
	uint32_t 	src_ip_addr,	/* in: IPADDR for src  */
	uint16_t 	dst_udp_port,	/* in: UDP port to send to */
	uint16_t 	src_udp_port,	/* in: UDP port for src */
	const uint8_t 	*p_payload,	/* in: UDP payload buffer */
	uint16_t 	udp_payload_sz,	/* in: UDP payload size in bytes (w/o any header) */
	uint8_t		*p_buf)			/* in: area where pkt will be built */
{
	uint32_t	status = PKT_SUCCESS;

	StarProPPB_TxHDR_t *phdr = 			// Ptr to the generated header
		(StarProPPB_TxHDR_t *) p_buf;
	StarProPPB_IPv4_UDP_HDR_t p_pkt;	// assemble IPv4/UDP header here
	uint16_t	tmp_u16;
	uint32_t	tmp_u32;
	uint32_t	ip_pkt_size;
	StarProPPB_IPv4_HDR_t *p_iphdr;

	BUILD_MAC_ADDR(phdr->eth_hdr.dest_addr,
		(dst_mac_addr&0xFFFF),
		((dst_mac_addr>>16)&0xFFFFFFFF));

	BUILD_MAC_ADDR(phdr->eth_hdr.src_addr,
		(src_mac_addr&0xFFFF),
		((src_mac_addr>>16)&0xFFFFFFFF));

	phdr->eth_hdr.proto[0] = (uint8_t)((SP_PPB_ETH_TYPE_IP >> 8) & 0xFF);
	phdr->eth_hdr.proto[1] = (uint8_t)((SP_PPB_ETH_TYPE_IP) & 0xFF);

	/*
	 * IP header - Source and destination addresses
	 */
	/* Source and destination addresses */
	p_pkt.ip_hdr.dest_addr.word[0] = SP_PPB_HTONL(dst_ip_addr);
	p_pkt.ip_hdr.src_addr.word[0] = SP_PPB_HTONL(src_ip_addr);

	/* Word0 */
	ip_pkt_size = ( udp_payload_sz + sizeof(StarProPPB_IPv4_HDR_t) +
		sizeof(StarProPPB_UDP_HDR_t) );

	if (ip_pkt_size > SP_PPB_ETH_MAX_FRM_SIZE) {
		return (PKT_ERR_INVALID_SIZE);
	}

	tmp_u32 = (uint32_t) ( (SP_PPB_IPv4_HDR_WORD0_DFLT ) |
		(SP_PPB_IPv4_HDR_WORD0_TOS_DFLT << SP_PPB_IPv4_HDR_WORD2_PROTO_OFST) |
		((uint32_t) ip_pkt_size) );

	p_pkt.ip_hdr.word0.reg = SP_PPB_HTONL( tmp_u32 );

	/* Word1 - constant is defined in network byte order */
	p_pkt.ip_hdr.word1.reg = SP_PPB_IPv4_HDR_WORD1_DFLT;

	/* Word2 - built in network order */
	p_pkt.ip_hdr.word2.reg = (uint32_t) ( (SP_PPB_IPV4_PROTO_UDP << 8) |
		(SP_PPB_IPv4_HDR_WORD2_TTL_DFLT << 0) );

	p_iphdr = &(p_pkt.ip_hdr);

	/* Calculate the IP header checksum */
	p_pkt.ip_hdr.word2.fields.csum = csum16( (const uint16_t *) p_iphdr,
						sizeof(StarProPPB_IPv4_HDR_t) );

	/*
	 * UDP header
	 */
	p_pkt.udp_hdr.dst_port = SP_PPB_HTONS( dst_udp_port );
	p_pkt.udp_hdr.src_port = SP_PPB_HTONS( src_udp_port );
	p_pkt.udp_hdr.csum = 0;		/* is 0 for purposes of checksum calculation */

	/* tmp_u16 is UDP payload + head size and is used later for cksum calculation */
	tmp_u16 = ( uint16_t ) (udp_payload_sz + sizeof(StarProPPB_UDP_HDR_t));
	p_pkt.udp_hdr.len = SP_PPB_HTONS( tmp_u16 );

    /* copy the header into buffer */
	memcpy8((void *) &p_buf[sizeof(StarProPPB_ETH_HDR_t)], (void *) &p_pkt,
		sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t));

	/* Then the payload */
	memcpy8((uint8_t*) &p_buf[( sizeof(StarProPPB_ETH_HDR_t) +
		sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t))],
			(uint8_t*) p_payload, udp_payload_sz);

	/* fill in acceptable UDP checksum */
	tmp_u16 = 0;

	/* insert checksum into packet */
	memcpy8((uint8_t*) &p_buf[( sizeof(StarProPPB_ETH_HDR_t) +
		sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t)) - 2],
		(uint8_t*) &tmp_u16, 2);

	return status;
}

int32_t
sp_EthBuildUdpPktIPv6(
	MAC_ADDR	dst_mac_addr,	/* in: MACADDR to send to */
	MAC_ADDR	src_mac_addr,	/* in: MACADDR to send from */
	uint32_t 	dst_ipv6_addr[4],	/* in: IPADDR to send to  */
	uint32_t 	src_ipv6_addr[4],	/* in: IPADDR for src  */
	uint16_t 	dst_udp_port, 		/* destination udp port */
	uint16_t 	src_udp_port, 		/* source udp port */
	const uint8_t 	*frame, 		/* payload data */
	uint16_t 	data_len, 			/* size of the payload in bytes without header */
	uint8_t 	*buf,				/* area where the packet is built */
	uint8_t 	packet_type)  		/* packet type: ETHERNET_V2, ETHERNET_V2_with_VLAN, SNAP, SNAP_with_VLAN */
{
	uint32_t i;

	/*---- Ethernet Header-----*/
	/* dest_mac (6 bytes = 48 bits)*/
	*buf++ = (uint8_t)(dst_mac_addr>>40);
	*buf++ = (uint8_t)(dst_mac_addr>>32);
	*buf++ = (uint8_t)(dst_mac_addr>>24);
	*buf++ = (uint8_t)(dst_mac_addr>>16);
	*buf++ = (uint8_t)(dst_mac_addr>>8);
	*buf++ = (uint8_t)dst_mac_addr;

	/* source_mac (6 bytes = 48 bits)*/
	*buf++ = (uint8_t)(src_mac_addr>>40);
	*buf++ = (uint8_t)(src_mac_addr>>32);
	*buf++ = (uint8_t)(src_mac_addr>>24);
	*buf++ = (uint8_t)(src_mac_addr>>16);
	*buf++ = (uint8_t)(src_mac_addr>>8);
	*buf++ = (uint8_t)src_mac_addr;

	if (packet_type == ETHERNET_V2){
		/* Type: IPv6 (2 bytes = 16 bits)*/
		*buf++ = 0x86;
		*buf++ = 0xDD;
	}
	else if (packet_type == ETHERNET_V2_with_VLAN){
		/* VLAN tag */
		*buf++ = 0x81;
		*buf++ = 0x00;
		*buf++ = 0x00;
		*buf++ = 0x0C;

		/* Type: IPv6 (2 bytes = 16 bits)*/
		*buf++ = 0x86;
		*buf++ = 0xDD;
	}
	else if (packet_type == SNAP){
		/* 802.3 SNAP : Length Field */
		*buf++ = (data_len + UDP_HEADER_LEN + IPv6_HEADER_LEN + 8)>>8; 	//+8 for LLC and SNAP headers
		*buf++ = (data_len + UDP_HEADER_LEN + IPv6_HEADER_LEN + 8);		//+8 for LLC and SNAP headers

		/* 802.3 SNAP : LLC header */
		*buf++ = 0xAA;
		*buf++ = 0xAA;
		*buf++ = 0x03;

		/* 802.3 SNAP : SNAP header */
		*buf++ = 0x00;
		*buf++ = 0xFA;
		*buf++ = 0xCE;
		*buf++ = 0x86;
		*buf++ = 0xDD;
	}
	else if (packet_type == SNAP_with_VLAN){
		/* VLAN tag */
		*buf++ = 0x81;
		*buf++ = 0x00;
		*buf++ = 0x00;
		*buf++ = 0x0C;

		/* 802.3 SNAP : Length Field */
		*buf++ = (data_len + UDP_HEADER_LEN + IPv6_HEADER_LEN + 8)>>8; 	//+8 for LLC and SNAP headers
		*buf++ = (data_len + UDP_HEADER_LEN + IPv6_HEADER_LEN + 8);		//+8 for LLC and SNAP headers

		/* 802.3 SNAP : LLC header */
		*buf++ = 0xAA;
		*buf++ = 0xAA;
		*buf++ = 0x03;

		/* 802.3 SNAP : SNAP header */
		*buf++ = 0x00;
		*buf++ = 0xFA;
		*buf++ = 0xCE;
		*buf++ = 0x86;
		*buf++ = 0xDD;
	}

	/*------IPv6 Header------*/
	/* Version (4 bits), Traffic Class (8 bits), and Flow Label (20 bits)*/
	*buf++ = 0x60;
	*buf++ = 0x00; //the second half and rest is flow label stuff
	*buf++ = 0x00;
	*buf++ = 0x00;

	/* Payload Length (16 bits)*/
	*buf++ = (uint8_t) (data_len + UDP_HEADER_LEN)>>8;
	*buf++ = (uint8_t) (data_len + UDP_HEADER_LEN); //payload plus 8 byte udp header

	/* Next Header: UDP (Protocol Number) (8 bits) protocol #17*/
	*buf++ = 0x11; //UDP

	/* Hop Limit (Time to Live) (8 bits)*/
	*buf++ = 0x80;

	/* Source IP Address (128 bits) */
	*buf++ = (uint8_t)(src_ipv6_addr[3]>>24);
	*buf++ = (uint8_t)(src_ipv6_addr[3]>>16);
	*buf++ = (uint8_t)(src_ipv6_addr[3]>>8);
	*buf++ = (uint8_t)src_ipv6_addr[3];
	*buf++ = (uint8_t)(src_ipv6_addr[2]>>24);
	*buf++ = (uint8_t)(src_ipv6_addr[2]>>16);
	*buf++ = (uint8_t)(src_ipv6_addr[2]>>8);
	*buf++ = (uint8_t)src_ipv6_addr[2];
	*buf++ = (uint8_t)(src_ipv6_addr[1]>>24);
	*buf++ = (uint8_t)(src_ipv6_addr[1]>>16);
	*buf++ = (uint8_t)(src_ipv6_addr[1]>>8);
	*buf++ = (uint8_t)src_ipv6_addr[1];
	*buf++ = (uint8_t)(src_ipv6_addr[0]>>24);
	*buf++ = (uint8_t)(src_ipv6_addr[0]>>16);
	*buf++ = (uint8_t)(src_ipv6_addr[0]>>8);
	*buf++ = (uint8_t)src_ipv6_addr[0];

	/* Destination IP Address (128 bits) */
	*buf++ = (uint8_t)(dst_ipv6_addr[3]>>24);
	*buf++ = (uint8_t)(dst_ipv6_addr[3]>>16);
	*buf++ = (uint8_t)(dst_ipv6_addr[3]>>8);
	*buf++ = (uint8_t)dst_ipv6_addr[3];
	*buf++ = (uint8_t)(dst_ipv6_addr[2]>>24);
	*buf++ = (uint8_t)(dst_ipv6_addr[2]>>16);
	*buf++ = (uint8_t)(dst_ipv6_addr[2]>>8);
	*buf++ = (uint8_t)dst_ipv6_addr[2];
	*buf++ = (uint8_t)(dst_ipv6_addr[1]>>24);
	*buf++ = (uint8_t)(dst_ipv6_addr[1]>>16);
	*buf++ = (uint8_t)(dst_ipv6_addr[1]>>8);
	*buf++ = (uint8_t)dst_ipv6_addr[1];
	*buf++ = (uint8_t)(dst_ipv6_addr[0]>>24);
	*buf++ = (uint8_t)(dst_ipv6_addr[0]>>16);
	*buf++ = (uint8_t)(dst_ipv6_addr[0]>>8);
	*buf++ = (uint8_t)dst_ipv6_addr[0];

/*------EXTENSION HEADER: UDP------*/
	/* Source port (16 bits) */
	*buf++ = src_udp_port >> 8;
	*buf++ = src_udp_port;

	/* Destination port (16 bits) */
	*buf++ = dst_udp_port >> 8;
	*buf++ = dst_udp_port;

	/* Length (16 bits)*/
	*buf++ = (uint8_t) (data_len + UDP_HEADER_LEN)>>8;
	*buf++ = (uint8_t) (data_len + UDP_HEADER_LEN);

	/* UDP Checksum (16 bits) */
	*buf++ = 0x00;
	*buf++ = 0x00;

	/* UDP Data Portion */
	for (i=0;i<data_len;i++) //data length
	*buf++ = *frame++;
	return (0);
}

void sp_EthAddVlanTag(uint8_t *p_buf, uint16_t vlan_id, uint32_t payload_sz)
{
	uint8_t tmp_buf[256]; /* max pkt size for the test*/
	uint32_t i;
	uint32_t pkt_size;

	/* identify the packet already has vlan, in this case, it shouldn't do anything */
	/* only if it already has vlan tag */
	if((*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2) != 0x81)||(*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-1) != 0x0))
	{
		/* check if the packet is ethernet v2 or 802.3 SNAP */
		/* if it's 802.3 SNAP packet */
		if((unsigned short)((*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2)<<8)+*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-1))<0x601)
		{
			pkt_size = sizeof(StarProPPB_ETH_HDR_t)+8 /* size of LLC_SNAP header + SNAP */\
					+ sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t) + payload_sz;
		}
		else /* if it's ethernet V2 */
		{
			pkt_size = sizeof(StarProPPB_ETH_HDR_t)\
					+ sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t) + payload_sz;
		}

		/* make a backup of the chunk of data after mac addresses in ethernet V2 field */
		for(i=0; i<pkt_size-sizeof(StarProPPB_ETH_HDR_t)-2; i++)
		{
			tmp_buf[i] = *(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2+i);
		}

		*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2) = 0x81;
		*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2+1) = 0x00;
		*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2+2) = ((vlan_id>>8)&0xff);
		*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2+3) = (vlan_id&0xff);

		/* restore backup */
		for(i=0; i<pkt_size-sizeof(StarProPPB_ETH_HDR_t)-2; i++)
		{
			*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2+4+i) = tmp_buf[i];
		}
	}
}

void sp_EthConvertPktLLCSnapType(uint8_t *p_buf, uint32_t payload_sz)
{
	uint8_t tmp_buf[256]; /* max pkt size for the test*/
	uint32_t i;
	uint32_t offset;
	uint32_t pkt_size;

	/* if vlan tag exist */
	if((*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2) == 0x81)&&(*(p_buf+sizeof(StarProPPB_ETH_HDR_t)-2+1) == 0x0))
	{
		offset = sizeof(StarProPPB_ETH_VLAN_HDR_t);
	}
	else /* no vlan tag found */
	{
		offset = sizeof(StarProPPB_ETH_HDR_t);
	}

	/* only if it's not 802.3 SNAP packet */
	if((unsigned short)((*(p_buf+offset-2)<<8)+*(p_buf+offset-1))>0x600)
	{
		pkt_size = offset + sizeof(StarProPPB_IPv4_HDR_t) + sizeof(StarProPPB_UDP_HDR_t) + payload_sz;

		/* make a backup of the chunk of data after mac addresses in ethernet V2 field */
		for(i=0; i<pkt_size-offset; i++)
		{
			tmp_buf[i] = *(p_buf+offset+i);
		}

		/* SNAP LLC HEADER */
		*(p_buf+offset) = 0xaa; /* dedicated value */
		*(p_buf+offset+1) = 0xaa; /* dedicated value */
		*(p_buf+offset+2) = 0x03; /* dedicated value */

		/* SNAP 5 bytes */
		*(p_buf+offset+3) = 0;
		*(p_buf+offset+4) = 0;
		*(p_buf+offset+5) = 0;

		*(p_buf+offset+6) = *(p_buf+offset-2); /* MSB of ethertype */
		*(p_buf+offset+7) = *(p_buf+offset-1); /* LSB of ethertype */
		*(p_buf+offset-2) = (uint8_t)((pkt_size>>8)&0xFF);
		*(p_buf+offset-1) = (uint8_t)(pkt_size&0xFF);

		/* restore backup */
		for(i=0; i<pkt_size-offset; i++)
		{
			*(p_buf+offset+8+i) = tmp_buf[i];
		}
	}
}

/**** FUNCTIONS for setting up and controlling PHY ****/

/* setup mdio with default clk speed */
void sp_EthPhyMDIOPortSetupDflt(void)
{
    /*** Set clock and period of MDIO ****/
	REG32_WRITE(LSI_SP27XX_MDIO_CLKPER_RA, 24); /* set MDIO clock to 5.33 MHz based on sys clock of 375 MHz */
	REG32_WRITE(LSI_SP27XX_MDIO_CLKOFST_RA, 12); /* set offset */
}

/* read data from a register in PHY */
uint16_t sp_EthPhyMDIOPortRd(uint8_t dev_addr, uint8_t reg_addr)
{
   uint32_t mdio_rd_val = 0;
   uint32_t mdio_data;

   mdio_data = 0x50000000;     // set up basic read
   mdio_data |= ( (dev_addr << LSI_SP27XX_MDIO_CTRL_PORTADDR_BO) | (reg_addr << LSI_SP27XX_MDIO_CTRL_DEVADDR_BO) );

   REG32_WRITE(LSI_SP27XX_MDIO_CTRL_RA, mdio_data);

   lsi_mg_delay(1000);

   REG32_READ(LSI_SP27XX_MDIO_CTRL_RA, mdio_rd_val);

   return (mdio_rd_val & 0xFFFF);
}

/* write data into a register in PHY */
void sp_EthPhyMDIOPortWr(uint8_t dev_addr, uint8_t reg_addr, uint32_t wr_data)
{
    uint32_t mdio_data = 0x48000000; /* setup basic write */

    mdio_data |= ( (reg_addr << LSI_SP27XX_MDIO_CTRL_DEVADDR_BO) | (dev_addr << LSI_SP27XX_MDIO_CTRL_PORTADDR_BO) | wr_data);

    REG32_WRITE(LSI_SP27XX_MDIO_CTRL_RA, mdio_data);
    lsi_mg_delay(1000);
}

/* poll SFP to make sure that auto-negotiation is complete and that the sgmii link is up */
int32_t emac_wait_link_PHY(uint8_t dev_addr)
{
	uint32_t reg_addr;
	uint32_t temp;
	uint32_t timeout;

	/* Set up the MDIO port */
	sp_EthPhyMDIOPortSetupDflt();

	/* Keep polling the Mode Status Register in the PHY until bits 2 and 5
	 * are set, signifying that a link is up and that auto-negotiation
	 * has been completed. */
	timeout = 0xFFFF;
	reg_addr = 0x1;

	while (--timeout)
	{
		temp = sp_EthPhyMDIOPortRd(dev_addr, reg_addr);

		if ((temp & 0x24) == 0x24)
			break;
	}

	if ((temp & 0x24) != 0x24)
		return -1; /* A timeout was experienced, and the PHY never linked up successfully */
	else
		return 0; /* The PHY linked up successfully */
}

/******** History ********
$Log: libeth.c,v $
Revision 1.3  2012/08/15 15:02:52  srane
Add support for EMAC1 loopback test.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/


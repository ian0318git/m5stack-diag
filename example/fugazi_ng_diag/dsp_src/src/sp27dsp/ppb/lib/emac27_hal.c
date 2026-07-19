/* $Id: emac27_hal.c,v 1.2 2012/05/10 22:48:10 srane Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/emac27_hal.c,v $
 *------------------------------------------------------------------
 * emac27_hal.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdint.h>
#include <string.h>
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "emac27_hal.h"
#include "libuart.h"


extern PCE_DEVICE_S pce0;
extern PCE_DEVICE_S pce1;

extern TXD_DEVICE_S txd0;
extern TXD_DEVICE_S txd1;

void
emac_enable(			/* enable MAC - call after PCE is set up to avoid losing packets */
		uint32_t port)
{
	/* set port_en, gmac_rx_en but not gmac_clk_mux_select */
	REG32_SET_BITS(LSI_SP27XX_MAC_ENPAUSE_RA(port),
		LSI_SP27XX_MAC_ENPAUSE_PORT_ENABLE_BM | LSI_SP27XX_MAC_ENPAUSE_GMAC_RX_EN_BM);
}

/* Set MAC port to run in SGMII mode */
int32_t emac_init_sgmii(uint32_t port, uint32_t phyType)
{
	uint32_t timeout;
	/* disabling mac */
	REG32_WRITE(LSI_SP27XX_MAC_ENPAUSE_RA(port), 0);


	REG32_RESET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_SPMODE_BM);
	REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), (2 << LSI_SP27XX_MAC_ANCTRL_SPMODE_BO) | LSI_SP27XX_MAC_ANCTRL_FDUPLEX_BM);

	/*** MAC SMII enable Control Reg # 22 ****/
	REG32_SET_BITS(LSI_SP27XX_MAC_SGMII_RA(port), LSI_SP27XX_MAC_SGMII_SGMIIEN_BM);
	REG32_SET_BITS(LSI_SP27XX_MAC_CONTROL_RA(port), LSI_SP27XX_MAC_CONTROL_RXCRCSTRIP_BM);

	/* NOTE: enabling autonegotiation (ANCTRL_ENABLE) must be last MAC operation or autonegotiation may hang */
	if (phyType == 1) /*PHYMODE_WAIT_AUTONEG */
	{
		REG32_RESET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_GOODF_BM); /* goodf=0 */
		REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_ENABLE_BM); /* enabling auto-negotiation */

		timeout = 1000000;
		while (!CHK_REG_MASK(LSI_SP27XX_MAC_ANSTAT_RA(port), LSI_SP27XX_MAC_ANSTAT_GMII_LINK_OK_BM))
		{
			lsi_mg_delay(1000);
			timeout--;
			if(timeout == 0)
			{
				/* link fail */
				return -1;
			}
		}
	}
	else if (phyType == 2) /* PHYMODE_NO_WAIT_AUTONEG */
	{
		REG32_RESET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_GOODF_BM); /* goodf=0 */
		REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_ENABLE_BM); /* enabling auto-negotiation */

		/* do not wait */
	}
	else  /* PHYMODE_NO_PHY */
	{
		REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_GOODF_BM); /* goodf=1 */
//		REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_ENABLE_BM); /* disabling auto-negotiation */
		REG32_RESET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_ENABLE_BM); /* disabling auto-negotiation */
	}
	return (0);
}

/* Set MAC port to run in SSSMII mode - call after PCE is set up to avoid losing packets */
int32_t emac_init_ssmii(uint32_t port, uint32_t phyType)
{
	/* disabling mac */
	REG32_WRITE(LSI_SP27XX_MAC_ENPAUSE_RA(port), 0);

	/* set port_en, gmac_rx_en and gmac_clk_mux_select */
	REG32_SET_BITS(LSI_SP27XX_MAC_ENPAUSE_RA(port), LSI_SP27XX_MAC_ENPAUSE_GPIO_CLK_MUX_SELECT_BM);

	REG32_RESET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_SPMODE_BM);

	REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), (1 << LSI_SP27XX_MAC_ANCTRL_SPMODE_BO)\
													|LSI_SP27XX_MAC_ANCTRL_GOODF_BM\
													|LSI_SP27XX_MAC_ANCTRL_FDUPLEX_BM);

	REG32_SET_BITS(LSI_SP27XX_MAC_ANCTRL_RA(port), LSI_SP27XX_MAC_ANCTRL_ENABLE_BM); /* enabling auto-negotiation */

	REG32_SET_BITS(LSI_SP27XX_MAC_SMIICTRL_RA(port), LSI_SP27XX_MAC_SMIICTRL_SMIIEN_BM\
													| LSI_SP27XX_MAC_SMIICTRL_SMIISYNC_BM\
													| LSI_SP27XX_MAC_SMIICTRL_SMIIMASK_BM);

	if (phyType == 0) /* PHYMODE_NO_PHY */
	{
		REG32_SET_BITS(LSI_SP27XX_MAC_SMIICTRL_RA(port), LSI_SP27XX_MAC_SMIICTRL_SMIIPHY_BM);
	}
	else
	{
		REG32_RESET_BITS(LSI_SP27XX_MAC_SMIICTRL_RA(port), LSI_SP27XX_MAC_SMIICTRL_SMIIPHY_BM);
	}

	REG32_SET_BITS(LSI_SP27XX_MAC_CONTROL_RA(port), LSI_SP27XX_MAC_CONTROL_RXCRCSTRIP_BM);
	REG32_SET_BITS(LSI_SP27XX_MAC_ENPAUSE_RA(port), LSI_SP27XX_MAC_ENPAUSE_PORT_ENABLE_BM \
													|LSI_SP27XX_MAC_ENPAUSE_GMAC_RX_EN_BM \
													|LSI_SP27XX_MAC_ENPAUSE_GPIO_CLK_MUX_SELECT_BM);
	return (0);
}

void emac_flush_tx_path(uint32_t port)
{
	/* completely flush tx path */
	REG32_SET_BITS(LSI_SP27XX_MAC_CONTROL_RA(port), LSI_SP27XX_MAC_CONTROL_TXFLUSH_BM);
	lsi_mg_delay(50);
	REG32_RESET_BITS(LSI_SP27XX_MAC_CONTROL_RA(port), LSI_SP27XX_MAC_CONTROL_TXFLUSH_BM);
}

/**** FUNCTIONS for setting up PCE ****/

/* Write some default values into RX BD */
int32_t pce_init_bd (uint32_t port, void *rx_qstart, int32_t num_elements, void * rx_frame_array)
{
	uint32_t i;
	uint32_t temp_addr;
	PCE_DEVICE_S* rxd;
#ifdef PCE_DEBUG
sp_SerialPutS("\r\n In pce_init_bd port = ");
sp_SerialPutLong(port, 'd');
#endif
	if((rx_qstart == NULL)||(rx_frame_array == NULL))
	{
		return -1;
	}

	if(((uint32_t)rx_qstart & 0x7)!=0) /* bd space should be at least aligned to 8 bytes, if prefetch size is more than 1,
										it should be aligned to 16 bytes or 64 bytes */
	{
		return -1;
	}

	if(port<1)
	{
		rxd = &pce0;
	}
	else
	{
		rxd = &pce1;
	}

	rxd->rbq_start =(RX_BQUE *) rx_qstart;
	//fill entire BD structure with known values.
	for(i=0;i<num_elements;i++)
	{
		temp_addr = (uint32_t) ((uint8_t *)rx_frame_array + RCV_BUF_MAX_SIZE*i);

		/* check if the rx buf is aligned correctly */
		/* PCE writes packet data as 16 beat 8B bursts (128B).  To avoid a 4K boundary-crossing error,
		 * the BufferStartAddress field of all buffer descriptors (BD.BufferStartAddress) must be provisioned according to the following constraints:
		 *
		 * 	BD.BufferStartAddress[2:0] must be 0.
		 * 	BD.BufferStartAddress[11:0] + 128 must be less than 4097 or BufferStartAddress[6:0] must be 0.
		 *
		 *  (from PCE HDS)
		 */

		if((temp_addr&0x7)!=0)
		{
			return -1;
		}

		if(((temp_addr&0xFFF)+128>4096)&&((temp_addr&0x7F)!=0))
		{
			return -1;
		}

		rxd->rbq_start[i].address = temp_addr;

		// 32 BIT access of register
		rxd->rbq_start[i].status.reg = 0x80000000 ; /* set owner-wrap */
		rxd->rbq_start[i].status.bits.own = 0; //release owner bit
		rxd->rbq_start[i].status.bits.wrap = 0; // release wrap bit
	}

#ifdef PCE_DEBUG
sp_SerialPutS("\r\n In pce_init_bd port later = ");
sp_SerialPutLong(port, 'd');
#endif
 	while (CHK_REG_MASK(LSI_SP27XX_PCE_CFG_STATUS_RA(port), LSI_SP27XX_PCE_CFG_STATUS_CFG_DLT_IN_PROGRESS_BM));

	/* Enable MIU */
	REG32_SET_BITS(LSI_SP27XX_PCE_MIU_CONTROL_RA(port), LSI_SP27XX_PCE_MIU_CONTROL_ENABLE_MIU_BM);

	/* Allocate 1/6 of BDs for BD descriptors in each queue*/
	for(i=0; i<6; i++)
	{
		rxd->start_buf_idx[i] = i*num_elements/6;

		REG32_WRITE(LSI_SP27XX_PCE_BD_BOT_RA(port,i), (uint32_t)&rxd->rbq_start[i*num_elements/6]);
		REG32_WRITE(LSI_SP27XX_PCE_BD_TOP_RA(port,i), (uint32_t)&rxd->rbq_start[(i+1)*num_elements/6-1]);

		rxd->rbq_start[(i+1)*num_elements/6-1].status.bits.wrap = 1;
		rxd->curr_buf_idx[i] = rxd->start_buf_idx[i];
		rxd->rbq_elements[i] = num_elements/6;
	}

	/* if num_elements/que * 128 is bigger than 1500, than use 1500 */
	REG32_WRITE(LSI_SP27XX_PCE_MAX_FRAME_SIZE_RA(port), ((num_elements/6*128)>1500)?1500:(num_elements/6*128));

#ifdef PCE_DEBUG
sp_SerialPutS("\r\n In pce_init_bd port end = ");
sp_SerialPutLong(port, 'd');
#endif
	return 0;
}

int32_t pce_reset(uint32_t port)
{
	uint32_t timeout;
	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_RESETPCE_BM);
	/* wait until PCE reset operation on PCEx is complete */
	timeout = 1000;
	while (CHK_REG_MASK(LSI_SP27XX_PCE_CONTROL_RA(port) ,LSI_SP27XX_PCE_CONTROL_RESETPCE_BM))
	{
		lsi_mg_delay(100);
		timeout--;
		if(timeout==0)
		{
			/* timeout */
			return (-1);
		}
	}
	return (0);
}

void pce_enable(uint32_t port, uint32_t classification)
{
	if(classification == 1)
	{
		REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_ENABLEPCECLASSIFICATION_BM);
	}

	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_ENABLEPCEINTERFACE_BM);
}

int32_t pce_set_dlt_init(uint32_t port)
{
	/* pce interface is disabled */
	if(CHK_REG_MASK(LSI_SP27XX_PCE_CONTROL_RA(port),LSI_SP27XX_PCE_CONTROL_ENABLEPCEINTERFACE_BM))
	{
		return (-1);
	}

	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_DLT_INIT_BM);

	return (0);
}

int32_t pce_set_dltb_en(uint32_t port)
{
	/* pce interface is disabled */
	if(CHK_REG_MASK(LSI_SP27XX_PCE_CONTROL_RA(port),LSI_SP27XX_PCE_CONTROL_ENABLEPCEINTERFACE_BM))
	{
		return (-1);
	}

	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_DLTB_ENABLE_BM);

	return (0);
}

void pce_set_mac_based_route(uint32_t port)
{
	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_MAC_ADDR_ROUTING_BM);
}

void pce_set_fwd_mode(uint32_t port, uint32_t fwd_mode)
{
	switch(fwd_mode)
	{
	case 1:
		REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_PCE_FORWARD_ALWAYS_BM);
		break;

	case 2:
		REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_PCE_FORWARD_ENABLE_BM);
		break;

	case 3:
		REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_PCE_FORWARD_ENABLE_BM\
														 |LSI_SP27XX_PCE_CONTROL_PCE_FORWARD_MODE_BM);
		break;

	case 4:
		REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_PCE_FORWARD_ENABLE_BM\
														|LSI_SP27XX_PCE_CONTROL_PCE_FORWARD_MODE_BM\
														|LSI_SP27XX_PCE_CONTROL_USE_REMOTE_AS_LOCAL_BM);
		break;

	default:
		/* do nothing */
		break;
	}
}

void pce_disable_COR(uint32_t port)
{

#ifdef PCE_DEBUG
sp_SerialPutS(" \r\n In pce_disable_COR()");
sp_SerialPutS(" \r\n LSI_SP27XX_PCE_BASE = ");
sp_SerialPutLong((int)LSI_SP27XX_PCE_BASE, 'h');
sp_SerialPutS(" \r\n LSI_SP27XX_PCE_CONTROL_RA(port) = ");
sp_SerialPutLong((int)(LSI_SP27XX_PCE_CONTROL_RA(port)), 'h');
sp_SerialPutS(" \r\n port = ");
sp_SerialPutLong(port, 'd');
sp_SerialPutS(" \r\n LSI_SP27XX_PCE_CONTROL_DISABLE_CNT_COR_BM = ");
sp_SerialPutLong((int)(LSI_SP27XX_PCE_CONTROL_DISABLE_CNT_COR_BM), 'h');
#endif

	/* Disable clear-on-read (COR) on PCE counter registers */
	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_DISABLE_CNT_COR_BM);
#ifdef PCE_DEBUG
sp_SerialPutS(" \r\n In  1 pce_disable_COR()");
#endif

	/* Clear all counter values */
	REG32_SET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_DISABLE_COUNTS_BM);
#ifdef PCE_DEBUG
sp_SerialPutS(" \r\n In  2 pce_disable_COR()");
#endif
	REG32_RESET_BITS(LSI_SP27XX_PCE_CONTROL_RA(port), LSI_SP27XX_PCE_CONTROL_DISABLE_COUNTS_BM);
#ifdef PCE_DEBUG
sp_SerialPutS(" \r\n In  3 pce_disable_COR()");
#endif
}

void pce_set_loaddr(uint32_t port,	long long eth_addr)
{
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_LOCAL_LS_RA(port), (eth_addr&0xFFFFFFFF));
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_LOCAL_MS_RA(port), (eth_addr>>32)&0xFFFF);
}

void pce_set_rmtaddr(uint32_t port, long long eth_addr)
{
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_REMOTE_LS_RA(port), (eth_addr&0xFFFFFFFF));
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_REMOTE_MS_RA(port), (eth_addr>>32)&0xFFFF);
}

void pce_set_maddr_dss0que(uint32_t port, long long eth_addr)
{
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_2_LS_RA(port), (eth_addr&0xFFFFFFFF));
	REG32_RESET_BITS(LSI_SP27XX_PCE_MAC_DEST_23_MS_RA(port), 0x0000FFFF); /* clearing upper 16 bits */
	REG32_SET_BITS(LSI_SP27XX_PCE_MAC_DEST_23_MS_RA(port),(eth_addr>>32)&0xFFFF);

}

void pce_set_maddr_dss1que(uint32_t port, long long eth_addr)
{
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_3_LS_RA(port), (eth_addr&0xFFFFFFFF));
	REG32_RESET_BITS(LSI_SP27XX_PCE_MAC_DEST_23_MS_RA(port), 0xFFFF0000); /* clearing lower 16 bits */
	REG32_SET_BITS(LSI_SP27XX_PCE_MAC_DEST_23_MS_RA(port),(eth_addr>>16)&0xFFFF0000);
}

void pce_set_maddr_dss2que(uint32_t port, long long eth_addr)
{
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_4_LS_RA(port), (eth_addr&0xFFFFFFFF));
	REG32_RESET_BITS(LSI_SP27XX_PCE_MAC_DEST_45_MS_RA(port), 0x0000FFFF); /* clearing upper 16 bits */
	REG32_SET_BITS(LSI_SP27XX_PCE_MAC_DEST_45_MS_RA(port),(eth_addr>>32)&0xFFFF);
}

void pce_set_maddr_dss3que(uint32_t port, long long eth_addr)
{
	REG32_WRITE(LSI_SP27XX_PCE_MAC_DEST_5_LS_RA(port), (eth_addr&0xFFFFFFFF));
	REG32_RESET_BITS(LSI_SP27XX_PCE_MAC_DEST_45_MS_RA(port), 0xFFFF0000); /* clearing lower 16 bits */
	REG32_SET_BITS(LSI_SP27XX_PCE_MAC_DEST_45_MS_RA(port),(eth_addr>>16)&0xFFFF0000);
}

void pce_set_udp_dest_max_min_porta(uint32_t port, uint32_t max_port, uint32_t min_port)
{
	REG32_WRITE(LSI_SP27XX_PCE_UDP_DEST_MAX_MIN_PORTA_RA(port), (max_port << 16) | (min_port & 0xFFFF));
}

void pce_set_udp_dest_max_min_portb(uint32_t port, uint32_t max_port, uint32_t min_port)
{
	REG32_WRITE(LSI_SP27XX_PCE_UDP_DEST_MAX_MIN_PORTB_RA(port), (max_port << 16) | (min_port & 0xFFFF));
}

int32_t pce_set_dlt_entry(uint32_t port, uint32_t from_udp_port, uint32_t to_udp_port, uint32_t dest)
{
	uint32_t max_min_val;
	uint32_t access_dlta;
	uint32_t i;

	if (dest > DLT_DEST_MAX) {
		return(EMAC_ERR_INVALID_DESTQ);
	}

	if (from_udp_port>to_udp_port)
	{
		return(EMAC_ERR_INVALID_DESTQ);
	}

	REG32_READ(LSI_SP27XX_PCE_UDP_DEST_MAX_MIN_PORTA_RA(port), max_min_val);
	access_dlta = 1;

	/* check the range is valid in dlta */
	if ((from_udp_port < (max_min_val & 0xFFFF))||(to_udp_port > ((max_min_val >> 16) & 0xFFFF)))
	{
		REG32_READ(LSI_SP27XX_PCE_UDP_DEST_MAX_MIN_PORTB_RA(port), max_min_val);
		access_dlta = 0;
		if ((from_udp_port < (max_min_val & 0xFFFF))||(to_udp_port > ((max_min_val >> 16) & 0xFFFF)))
		{
			return(EMAC_ERR_INVALID_UDPDESTPORT);
		}
	}

	if(access_dlta == 1)
	{
		for(i=0; i<to_udp_port-from_udp_port+1; i++)
		{
			REG32_WRITE(LSI_SP27XX_PCE_DLT_ACCESS_RA(port), 0); /* use DLTA */
			REG32_WRITE(LSI_SP27XX_PCE_DLT_ENTRY_RA(port,from_udp_port+i-(max_min_val& 0xFFFF))\
					,(dest<<LSI_SP27XX_PCE_DLT_DESTINATION_QUEUE_BO)|LSI_SP27XX_PCE_DLT_DEST_CONN_VALID_BM);
		}
	}
	else
	{
		for(i=0; i<to_udp_port-from_udp_port+1; i++)
		{
			REG32_WRITE(LSI_SP27XX_PCE_DLT_ACCESS_RA(port), 1); /* use DLTB */
			REG32_WRITE(LSI_SP27XX_PCE_DLT_ENTRY_RA(port,from_udp_port+i-(max_min_val& 0xFFFF))\
					,LSI_SP27XX_PCE_DLT_DLTAB_BM|(dest << LSI_SP27XX_PCE_DLT_DESTINATION_QUEUE_BO)\
					|LSI_SP27XX_PCE_DLT_DEST_CONN_VALID_BM);
		}
	}

	return(EMAC_SUCCESS);
}

void pce_rxq_rst(uint32_t port, uint32_t pce_q)
{
	uint32_t q_mask;
	uint32_t i;

	q_mask = (0x1<<pce_q);

	//perform reset of the bdptr for the intended queue
	REG32_SET_BITS(LSI_SP27XX_PCE_BD_PTR_RST_RA(port), q_mask);
	while (!CHK_REG_MASK(LSI_SP27XX_PCE_BD_PTR_RST_RA(port), q_mask));

	//release the reset bit for the bdptr
	REG32_RESET_BITS(LSI_SP27XX_PCE_BD_PTR_RST_RA(port), q_mask);
	while (CHK_REG_MASK(LSI_SP27XX_PCE_BD_PTR_RST_RA(port), q_mask));

	//clear the own bit of all buffer descriptors
	for (i=0; i<54; i++) {
		if(port<1)
		{
			pce0.rbq_start[i].status.reg &= ~RX_BQUE_STAT_OWN_BM;
		}
		else
		{
			pce1.rbq_start[i].status.reg &= ~RX_BQUE_STAT_OWN_BM;
		}
	}
}

/**** FUNCTIONS for setting up TXD ****/

int32_t txd_init_bd(uint32_t port,	void *tx_qstart,	uint32_t num_elements)
{
	uint32_t i = 0;
	TXD_DEVICE_S* txd;

	if(tx_qstart == NULL)
	{
		return -1;
	}

	if(((uint32_t)tx_qstart&0x7)!=0)
	{
		return -1;
	}

	if(port<1)
	{
		txd = &txd0;
	}
	else
	{
		txd = &txd1;
	}

	txd->tbq_start = (TXD_BQUE *) tx_qstart;

	// populate every single one of the buffer descriptors
	while(i<num_elements) {
	    txd->tbq_start[i].address = 0x55aa55aa;
	    txd->tbq_start[i].status.reg = 0; /* initialize bd status */
	    txd->tbq_start[i].status.bits.own = 1;
	    i++;
	}

	/* Allocate first 1/9 of BDs for queue0 descriptors */
	for(i=0; i<9; i++)
	{
		txd->start_buf_idx[i] = i*num_elements/9;

		REG32_WRITE(LSI_SP27XX_TXD_BDBASE_RA(port,i), &(txd->tbq_start[i*num_elements/9]));
		REG32_WRITE(LSI_SP27XX_TXD_BDLIMIT_RA(port,i), &(txd->tbq_start[(i+1)*num_elements/9-1]));
		REG32_WRITE(LSI_SP27XX_TXD_BDPTR_RA(port,i), &(txd->tbq_start[i*num_elements/9]));

		txd->tbq_start[(i+1)*num_elements/9-1].status.bits.wrap = 1;

		txd->curr_buf_idx[i] = txd->start_buf_idx[i];
		txd->tbq_elements[i] = num_elements/9; /* not real */
	}

	/* size of outgoing buffers */
	REG32_WRITE(LSI_SP27XX_TXD_BLEN_RA(port), (num_elements/9*128));
	//REG32_WRITE(LSI_SP27XX_TXD_BLEN_RA(port), 1024);
	return 0;
}

void txd_clr_path(uint32_t port)
{
	REG32_SET_BITS(LSI_SP27XX_TXD_CONFIG_RA(port), LSI_SP27XX_TXD_CONFIG_CCLEAR_BM|LSI_SP27XX_TXD_CONFIG_TFLUSH_BM|LSI_SP27XX_TXD_CONFIG_DFLUSH_BM);
}

void txd_txq_rst(uint32_t port, uint32_t txd_q)
{
	/* disable the queue */
	while (HW_REG_ACCESS(LSI_SP27XX_TXD_QCTL_RA(port,txd_q)) & LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM) {
		REG32_RESET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,txd_q), LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM);
	}

	/* reset current ptr */
	REG32_WRITE(LSI_SP27XX_TXD_BDPTR_RA(port,txd_q), (HW_REG_ACCESS(LSI_SP27XX_TXD_BDBASE_RA(port,txd_q))));

	/* activate the queue */
	while ((HW_REG_ACCESS(LSI_SP27XX_TXD_QCTL_RA(port,txd_q)) & LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM) == 0) {
		REG32_SET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,txd_q), LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM);
	}
	/* pointer is not going to be updated until emac_xmit is called */
}

void txd_txfwd_set(uint32_t port, uint32_t local_remote_pce)
{
	if(local_remote_pce == 0) /* if forwarding packet is from local PCE unit */
	{
		REG32_SET_BITS(LSI_SP27XX_TXD_CONFIG_RA(port), LSI_SP27XX_TXD_CONFIG_LRMUX_BM);
	}
	else /* if forwarding paket is from remote PCE unit */
	{
		REG32_RESET_BITS(LSI_SP27XX_TXD_CONFIG_RA(port), LSI_SP27XX_TXD_CONFIG_LRMUX_BM);
	}
}

void txd_dma_watermark_set(uint32_t port, uint32_t wf, uint32_t af)
{
	REG32_WRITE(LSI_SP27XX_TXD_FIFOWM_RA(port), ((af&0x1ff)<<16)|(wf&0x1ff));
}

void txd_rmt_watermark_set(uint32_t port, uint32_t wf)
{
	REG32_WRITE(LSI_SP27XX_TXD_TRFWM_RA(port), (wf&0xfff));
}

void txd_txsched_set(uint32_t port, uint32_t num_q_using)
{
	if(num_q_using>9)
	{
		/* maximum # of tx que */
		num_q_using = 9;
	}

	REG32_WRITE(LSI_SP27XX_TXD_SCHEDCFG_RA(port), num_q_using-1);
}

void txd_txsched_add_q_slot(uint32_t port, uint32_t slot, uint32_t txq)
{
	/* clearing the field */
	REG32_RESET_BITS(LSI_SP27XX_TXD_SCHED_RA(port, (uint32_t)(slot/8)), (0xF<<((4*slot)%32)));

	/* fill the field out with que number */
	REG32_SET_BITS(LSI_SP27XX_TXD_SCHED_RA(port, (uint32_t)(slot/8)), ((txq&0xF)<<((4*slot)%32)));
}

/* give a certain tx que high priority */
void txd_qpri_set(uint32_t port, uint32_t txque)
{
	REG32_SET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,txque), LSI_SP27XX_TXD_QCTL_QPRIORITY_BM);
}

/* make tx que normal priority */
void txd_qpri_reset(uint32_t port, uint32_t txque)
{
	REG32_RESET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,txque), LSI_SP27XX_TXD_QCTL_QPRIORITY_BM);
}

int32_t txd_xmit(uint32_t port, uint32_t txque)
{
	uint32_t i;
	uint32_t timeout;
	uint32_t num_tx_slots, k;

	/* check if tx scheduler is set to send out packets from all ques */
	REG32_READ(LSI_SP27XX_TXD_SCHEDCFG_RA(port), num_tx_slots);
	num_tx_slots &= 0x1F;
	num_tx_slots++;

	if(txque > 8) /* xmit packets in all ques */
	{
		if(num_tx_slots<9)
		{
			return (-1);
		}

		for(i=0; i<9; i++) /* make all queue active */
		{
			k = num_tx_slots;

			while(((HW_REG_ACCESS(LSI_SP27XX_TXD_SCHED_RA(port, ((k-1)*4)>>5))>>(((k-1)*4)%32))&0xF)!=i)
			{
				k--;
				if(k==0)
				{
					return (-1);
				}
			}

			/* if the que is not active, then make it active */
			while (!CHK_REG_MASK(LSI_SP27XX_TXD_QCTL_RA(port,i), LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM))
			{
				REG32_SET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,i), LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM);
			}
		}
	}

	else
	{
		/* if it's zero, then no packet can be sent out */
		if(num_tx_slots<1)
		{
			return (-1);
		}

		k = num_tx_slots;
		while(((HW_REG_ACCESS(LSI_SP27XX_TXD_SCHED_RA(port, ((k-1)*4)>>5))>>(((k-1)*4)%32))&0xF)!=txque)
		{
			k--;
			if (k==0)
			{
				return (-1);
			}
		}

		/* disable all ques */
		for (i=0; i<9; i++)
		{
			while (HW_REG_ACCESS(LSI_SP27XX_TXD_QCTL_RA(port,i)) & LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM) {
				REG32_RESET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,i), LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM);
			}
		}
		/* then enable just "txque"th que */
		while ((HW_REG_ACCESS(LSI_SP27XX_TXD_QCTL_RA(port,txque)) & LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM) == 0) {
			REG32_SET_BITS(LSI_SP27XX_TXD_QCTL_RA(port,txque), LSI_SP27XX_TXD_QCTL_Q_ACTIVE_BM);
		}
	}

	timeout = 1000;
	/* wait until previous TX DMA tranfer is done */
	while(CHK_REG_MASK(LSI_SP27XX_TXD_STATUS_RA(port), LSI_SP27XX_TXD_STATUS_ACTIVE_BM))
	{
		lsi_mg_delay(100);
		timeout--;
		if(timeout==0)
		{
			/* timeout error */
			return (-1);
		}
	}

	if (CHK_REG_MASK(LSI_SP27XX_TXD_START_RA(port), LSI_SP27XX_TXD_START_START_BM))
	{
		/* Error; we already made sure there is no TX dma transfer. */
		return (-1);
	}

	REG32_SET_BITS(LSI_SP27XX_TXD_START_RA(port), LSI_SP27XX_TXD_START_START_BM);

	return (0);
}
/******** History ********
$Log: emac27_hal.c,v $
Revision 1.2  2012/05/10 22:48:10  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:30  srane
Initial checkin


$Endlog$
*/


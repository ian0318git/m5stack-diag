/* $Id: emac27_hal.h,v 1.2 2017/07/28 07:58:39 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/emac27_hal.h,v $
 *------------------------------------------------------------------
 * emac27_hal.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/************** This is for PCE **************** */
/* Define buffer descriptor for the PCE receive. */
#define RX_BQUE_STAT_LEN_BO				0
#define RX_BQUE_STAT_LEN_BM				0x00003FFF
#define RX_BQUE_STAT_MAC_ERR_BO			14
#define RX_BQUE_STAT_MAC_ERR_BM			0x00004000
#define RX_BQUE_STAT_CKSM_ERR_BO		15
#define RX_BQUE_STAT_CKSM_ERR_BM		0x00008000
#define RX_BQUE_STAT_DLT_FAIL_ERR_BO	16
#define RX_BQUE_STAT_DLT_FAIL_ERR_BM	0x00010000
#define RX_BQUE_STAT_L2_FMT_BO			17
#define RX_BQUE_STAT_L2_FMT_BM			0x00060000
#define RX_BQUE_STAT_UNREC_L2_ERR_BO	19
#define RX_BQUE_STAT_UNREC_L2_ERR_BM	0x00080000
#define RX_BQUE_STAT_UNREC_L3L4_ERR_BO	20
#define RX_BQUE_STAT_UNREC_L3L4_ERR_BM	0x00100000
#define RX_BQUE_STAT_BAD_IP_HDR_ERR_BO	21
#define RX_BQUE_STAT_BAD_IP_HDR_ERR_BM	0x00200000
#define RX_BQUE_STAT_TYPE_BO			22
#define RX_BQUE_STAT_TYPE_BM			0x03C00000
#define RX_BQUE_STAT_SVT_FAIL_BO		26
#define RX_BQUE_STAT_SVT_FAIL_BM		0x04000000
#define RX_BQUE_STAT_COR_BO				27
#define RX_BQUE_STAT_COR_BM				0x08000000
#define RX_BQUE_STAT_EOF_BO				28
#define RX_BQUE_STAT_EOF_BM				0x10000000
#define RX_BQUE_STAT_SOF_BO				29
#define RX_BQUE_STAT_SOF_BM				0x20000000
#define RX_BQUE_STAT_OWN_BO				30
#define RX_BQUE_STAT_OWN_BM				0x40000000
#define RX_BQUE_STAT_WRAP_BO			31
#define RX_BQUE_STAT_WRAP_BM			0x80000000

/* PCE Buffer Descriptor TYPE (T) field definitions */
#define PCE_BD_TYPE_UNDEF				0
#define PCE_BD_TYPE_ETH_IPV4_UDP		1
#define PCE_BD_TYPE_IPV4_UDP			2
#define PCE_BD_TYPE_ETH_IPV6_UDP		3
#define PCE_BD_TYPE_IPV6_UDP			4
#define PCE_BD_TYPE_ARP					7
#define PCE_BD_TYPE_UDL0				8
#define PCE_BD_TYPE_UDL1				9
#define PCE_BD_TYPE_UDL2				10
#define PCE_BD_TYPE_UDL3				11
#define PCE_BD_TYPE_MAC_ADDR_MISS		15

#define EMAC_IN_PROGRESS				-1
#define EMAC_SUCCESS					0
#define EMAC_ERR_INVALID_MACDEST		1
#define EMAC_ERR_INVALID_MACSRC			2
#define EMAC_ERR_INVALID_IPDEST			3
#define EMAC_ERR_INVALID_IPSRC			4
#define EMAC_ERR_INVALID_SIZE			5
#define EMAC_ERR_NO_BUFFERS				6
#define EMAC_ERR_INVALID_UDPDESTPORT	7
#define EMAC_ERR_INVALID_UDPSRCPORT		8
#define EMAC_ERR_INVALID_DESTQ			9

#define EMAC_ERR_NO_PKT					32
#define EMAC_ERR_SOFSOF					33
#define EMAC_ERR_EOFEOF					34
#define EMAC_ERR_EOFNOSOF				35
#define EMAC_ERR_UDP_CKSUM				36
#define EMAC_ERR_MAC_ERROR				37
#define EMAC_ERR_FMT_ERROR				38
#define EMAC_ERR_FRM_TOO_BIG			39
#define EMAC_ERR_CORRUPT				40
#define EMAC_ERR_LEN_TO_SMALL			41
#define EMAC_ERR_LEN_TO_BIG				42
#define EMAC_ERR_BD_ERROR				43

//SR orig ?? #define XMIT_BUF_MAX_SIZE 	0x80
#define XMIT_BUF_MAX_SIZE 	1024
#define RCV_BUF_MAX_SIZE 	0x80
//#define RCV_BUF_MAX_SIZE 	512

#define DLT_DEST_FAILQ		0
#define DLT_DEST_PPB		1
#define DLT_DEST_DSS0		2
#define DLT_DEST_DSS1		3
#define DLT_DEST_DSS2		4
#define DLT_DEST_DSS3		5
#define DLT_DEST_MAX		DLT_DEST_DSS3

#define ETHERNET_V2				0
#define ETHERNET_V2_with_VLAN	1
#define SNAP					2
#define SNAP_with_VLAN			3

/* Transmit buffer descriptor status words bit positions. */
#define TBQE_WRAP       (1 << 31)  		/* Wrap bit */
#define TBQE_OWN        (1 << 30)   	/* Own bit. */
#define TBQE_SOF        (1 << 29)   	/* Start of Frame */
#define TBQE_EOF        (1 << 28)   	/* EOF */
#define TBQE_DUMMY      (0xFBBFABCD) 	/* Sets some value into BD that does not to cause problems */
#define RBQE_DUMMY      (0x4000000F)

typedef struct {
   uint32_t  address;               /* 32-bit transmit buffer address */
   union {
      uint32_t reg;                 /* 32-bit access of word1 */
      struct {                     	/* Bit-field access */
         unsigned length   : 14;  	/* Length of frame to transmit */
         unsigned mac_error :1;		/* MAC error */
         unsigned chksum_error : 1; /* Cehcksum fail */
         unsigned DLT_fail :1;		/* Destination lookup fail */
         unsigned L2format :2; 		/* L2 format seen */
         unsigned unrecog_L2 :1;    /* uncognized L2 protocol */
         unsigned unrecog_L3_L4 :1;	/* uncognized L3 or L4 protocol */
         unsigned badip_head :1;	/* bad ip hesder */
         unsigned type :4;			/* packettype */
         unsigned svtfail :1;		/* Source svt fail */
         unsigned cor  :1;			/* Corrupt */
         unsigned eof  :1;	  		/* EOF End of frame */
         unsigned sof  :1;   		/* SOF Start of frame Retry limit exceeded */
         unsigned own  :1;      	/* When set BD in use, when clear may be written by software*/
         unsigned wrap :1;      	/* Wrap, Marks last buffer in BD list*/
      } volatile bits;             	/* For bit-field access */
   } volatile status;
} volatile RX_BQUE;        			/* Transmit Buffer Queue Element */

/* Define the structure for each queue element in the transmit buffers descriptor
 * for the TxD. */
typedef struct {
   union {
      uint32_t reg;              		/* 32-bit access of word1 */
      struct {                      	/* Bit-field access */
         unsigned length   :14;     	/* Length of frame to transmit */
         const volatile unsigned :14;   /* reserved */
         unsigned eof  :1;				/* EOF End of frame */
         unsigned sof  :1;   			/* SOF Start of frame Retry limit exceeded */
         unsigned own  :1;      		/* When set BD in use, when clear may be written by software*/
         unsigned wrap :1;      		/* Wrap, Marks last buffer in BD list*/
      } volatile bits;              	/* For bit-field access */
   } volatile status;
   uint32_t  address;               	/* 32-bit transmit buffer address */
} volatile TXD_BQUE;        			/* Transmit Buffer Queue Element */

/* This is a structure that will be passed for all TXD operations, it
 * consists of pointers to the various TXD transmit queue structure and TxD registers. */
typedef struct {
	TXD_BQUE *tbq_start;              	/* Pointer to start of tx descriptor queue */
	TXD_BQUE *tbq_current;            	/* Current position in tx descriptor queue */
	TXD_BQUE  *tbq_end;				  	/* Position in queue for next tx buffer. */
	int start_buf_idx[9];			 	/* Indicate index of first tx buf */
	int curr_buf_idx[9];				/* Indicate index of last processed tx buf */
	int tbq_elements[9];           		/* Number of elements in each tx descriptor queue */
	int txd_elements;           		/* Number of buffer descriptors in the TXD device */
} TXD_DEVICE_S;

/* Structure to hold actual register of PCE and some pointers */
typedef struct {
   RX_BQUE  *rbq_start;             	/* Pointer to start of rx descriptor queue */
   RX_BQUE  *rbq_end;               	/* Position in queue for next rx buffer. */
   int  start_buf_idx[DLT_DEST_MAX+1]; 	/* Indicate index of first rx buf */
   int  curr_buf_idx[DLT_DEST_MAX+1]; 	/* Indicate index of last processed rx buf */
   int  rbq_elements[DLT_DEST_MAX+1]; 	/* # elements in each rx descriptor queue */
} PCE_DEVICE_S;

typedef struct {
	uint32_t	bd_ptr_base;
	uint32_t	bd_ptr_max;
	uint32_t	sw_ptr;
} txd_bd_sw_ptr;

/* Setup the RX buffer descriptors in memory */
int32_t
pce_init_bd (
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		void * rx_qstart,			/* in: ptr to bds */
		int32_t num_elements,		/* in: total # of bds for all ques */
		void * rx_frame_array);		/* in: ptr to rx buffers */

void
pce_set_loaddr(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		long long eth_addr);		/* in: MAC address i.e. 0x00FACE260306 */

void
pce_set_rmtaddr(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		long long eth_addr);		/* in: MAC address i.e. 0x00FACE260306 */

void
pce_set_maddr_dss0que(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		long long eth_addr);		/* in: MAC address i.e. 0x00FACE260306 */

void
pce_set_maddr_dss1que(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		long long eth_addr);		/* in: MAC address i.e. 0x00FACE260306 */

void
pce_set_maddr_dss2que(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		long long eth_addr);		/* in: MAC address i.e. 0x00FACE260306 */

void
pce_set_maddr_dss3que(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		long long eth_addr);		/* in: MAC address i.e. 0x00FACE260306 */

void
pce_rxq_rst(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t pce_q);			/* in: start addr of pce_q */

/* resetting pce */
int32_t
pce_reset(
		uint32_t port);				/* in: EMAC0 or EMAC1 */

void
pce_enable(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t classification);	/* in: '1' in case classification is on, '0' otherwise */


int32_t
pce_set_dlt_init(					/* initialize DLT table */
		uint32_t port);				/* in: EMAC0 or EMAC1 */


int32_t
pce_set_dltb_en(					/* enable DLT B */
		uint32_t port);				/* in: EMAC0 or EMAC1 */

/* enable MAC based route */
void
pce_set_mac_based_route(
		uint32_t port);				/* in: EMAC0 or EMAC1 */


void
pce_set_fwd_mode(					/* set the forwarding mode of PCE */
		uint32_t port, 				/* in: EMAC0 or EMAC1 */
		uint32_t fwd_mode);			/* in: forward mode */


void
pce_disable_COR(					/* disable 'COR' attribute of counter registers */
		uint32_t port);				/* in: EMAC0 or EMAC1 */

void
pce_set_udp_dest_max_min_porta(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t max_port,			/* in: maximum UDP port to classify */
		uint32_t min_port);			/* in: minimum UDP port to classify */

void
pce_set_udp_dest_max_min_portb(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t max_port,			/* in: maximum UDP port to classify */
		uint32_t min_port);			/* in: minimum UDP port to classify */

int32_t								/* ret: EMAC_SUCCESS, EMAC_ERR_INVALID_DESTQ or EMAC_ERR_INVALID_UDPDESTPORT */
pce_set_dlt_entry(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t from_udp_port,		/* in: from UDP port to process */
		uint32_t to_udp_port,		/* in: to UDP port to process */
		uint32_t destination);		/* in: DLT_DEST_DSS<0-2> or DLT_DEST_PPB */


int32_t
txd_init_bd (						/* Setup TXD buffer descriptors in memory */
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		void *tx_qstart,			/* in: ptr to bds */
		uint32_t num_elements);		/* in: total # of bds for all ques */

void txd_txq_rst(
		uint32_t port,			/* in: EMAC0 or EMAC1 */
		uint32_t txd_q);		/* in: start addr of txd_q */


void
txd_dma_watermark_set(			/* set watermark register for DMA fifo */
		uint32_t port, 			/* in: EMAC0 or EMAC1 */
		uint32_t wf, 			/* in: water mark level (2 < wf <= 256) */
		uint32_t af);			/* in: almost full level (2 < af <= 256) */


void
txd_rmt_watermark_set(			/* set water-mark register for remote fifo (in the forwarding path) */
		uint32_t port, 			/* in: EMAC0 or EMAC1 */
		uint32_t wf); 			/* in: water mark level (2 <= wf < 2512) */

void
txd_txfwd_set(
		uint32_t port,			/* in: EMAC0 or EMAC1 */
		uint32_t local_remote_pce); /* in: local PCE(0) or remote PCE(1) */


void
txd_txsched_set(				/* configure schedule config register */
		uint32_t port,			/* in: EMAC0 or EMAC1 */
		uint32_t num_q_using);	/* in: number of tx que */


void
txd_txsched_add_q_slot(				/* register a txq into the scheduler */
		uint32_t port, 				/* in: EMAC0 or EMAC1 */
		uint32_t slot, 				/* in: slot where txq will be programmed to */
		uint32_t txq);				/* in: txq that will be registered */


/* make tx que high priority */
void
txd_qpri_set(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t txque);			/* in: txque */

/* make tx que normal priority */
void
txd_qpri_reset(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t txque);			/* in: txque */

int32_t
txd_xmit(
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t que);				/* in: which queue to utilize for transmission */

int32_t
emac_init_ssmii(					/* configure MAC to run in SS-SMII mode */
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t phyType);			/* in: PHYMODE_NO_PHY, PHYMODE_WAIT_AUTONEG, or
									   PHYMODE_NO_WAIT_AUTONEG */
int32_t
emac_init_sgmii(					/* configure MAC to run in SGMII mode */
		uint32_t port,				/* in: EMAC0 or EMAC1 */
		uint32_t phyType);			/* in: PHYMODE_NO_PHY, PHYMODE_WAIT_AUTONEG, or
									   PHYMODE_NO_WAIT_AUTONEG */
void
emac_enable(			/* enable MAC - call after PCE is set up to avoid losing packets */
		uint32_t port);	/* in: EMAC0 or EMAC1 */

void
emac_flush_tx_path(
		uint32_t port);				/* in: EMAC0 or EMAC1 */

/* resetting whole ethernet interface completely */
int32_t
emac_hw_reset(
		uint32_t serdes_used); /* in: '1', in case serdes is being used by the application */

/******** History ********
$Log: emac27_hal.h,v $
Revision 1.2  2017/07/28 07:58:39  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:48:10  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:30  srane
Initial checkin


$Endlog$
*/


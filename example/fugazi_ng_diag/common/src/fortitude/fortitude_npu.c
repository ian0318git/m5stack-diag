/* $Id: fortitude_npu.c,v 1.11 2013/04/19 18:33:37 ywen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fortitude/fortitude_npu.c,v $
 *------------------------------------------------------------------
 *
 * fortitude_npu.c - This file contains functions for Fortitude NPU.
 *
 * Christine Wen -- Oct. 2011
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "common.h"
#include "types.h" 
#include "error.h"
#include "fortitude.h"
#include "dev_object.h"
#include "dev_4359.h"
#include "pcmap.h"
#include "fortitude_fpga.h"

/***************************************************************************/
/*                       WDDI Include                                      */
/***************************************************************************/
#include "wp_wddi.h"
#include "wp_host_io.h"
#include "wp_types.h"
#include "wp_enet.h"
#include "wpx_app_data.h"
#include "wpx_board_if.h"

/* Defines */
#define DATA_LENGTH              78

/* IW  */
#define MAX_FLOWS                128
#define DEFAULT_VLAN             7
#define APP_LEARNING_QUEUE_SIZE  128
#define POOL_LIMIT               9

/* define for statistic display */
#define ENET_DEV                 5
#define BR_PORT                  30
#define FLOW_AGG                 35
#define TDM_DEV                  50 

#define NUM_OF_ALL_HOST_CHANNELS 2

/* Function prototypes */
static void APP_DisplayStat(WP_handle object, WP_U8 type, WP_CHAR *name);
static unsigned int APP_TerminateOnError(WP_handle handle, WP_CHAR *s, WP_U32 LineNum);

extern WP_U32 dps_ProgramImage[];
extern WP_U16 dps_PC_Table_Init[];

/* Global variables */
boolean ngvm_init = FALSE;
WP_handle qniw, iw_pool;
WP_handle h_pool_256, h_pool_ring_host, h_qnode_host;
WP_handle host_port,host_dev, host_rx_channel, host_tx_channel, host_default_agg;
WP_handle enet_port[NUM_ENET], enet_dev[NUM_ENET], enet_iwport[NUM_ENET], enet_rx_channel[NUM_ENET], enet_tx_channel[NUM_ENET], enet_agg[NUM_ENET];
WP_handle iw_sys;   
WP_handle tdi_port[NUM_TDI], tdi_dev[NUM_TDI], tdi_rx_channel[NUM_TDI],tdi_tx_channel[NUM_TDI], tdi_iwport[NUM_TDI], tdi_agg[NUM_TDI];

WP_U8 cell[DATA_LENGTH] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x07, \
			   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, \
			   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, \
			   0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, \
			   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, \
			   0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, \
			   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, \
			   0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, \
			   0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, \
			   0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d};

/*****************************************************************************
 *  WP_int_queue_table      Interrupt queue configuration used with WP_SysInit
 *
 *  The WDDI supports 4 interrupt event queues. The following structure has
 *  four pairs of elements: size and ratio.
 *  This structure is used as part of the WinPath context structure.
 *
 *  Size -- Number of entries in interrupt queue table. If the table becomes 
 *          full before events have been serviced, subsequent events are not 
 *          recorded in the table. Therefore, the size of table should be 
 *          greater than:
 *          (avg. rate of events / avg. frequency of interrupt servicing)
 *
 *  Ratio --Number of event occurrences to interrupts generated
 *          For each n event occurrences associated with this interrupt queue, 
 *          an interrupt is raised. If the ratio is set to 0 then the queue is
 *          established as a polled queue instead of in interrupting queue.
 *
 *****************************************************************************/
WP_int_queue_table int_queue_tables = {{{24, 1}, {10, 1}, {10, 1}, {10, 1}}};

WP_pkt_shaping_wfq wfq =
    {
	/* weight;  */ 1,
	/* weight_fraction;*/ 0
    };

WP_iw_global  max_iw =
    {
	/* max_iw_aggregations */        200,
	/* max_routing_iw_systems;*/     2,
	/* max_directmap_iw_systems;*/   2,
	/* max_bridging_iw_systems;*/    2,
	/* max_mpls_iw_systems;*/        0,
	/* max_vlan_priority_maps; */    1,
	/* iw_bkgnd_fifo_size */         4000,
	/* cong_pt       */              NULL,
	/* iw_host_limits */             {32,0,0,32,32},
	/* mpls_config*/                 {0,0},
	/* iw_modes;*/
	{
	    /*policer_mode;*/                WP_IW_POLICER_DISABLE,
	    /*statistics_bundle_stat_mode;*/ WP_IW_STATISTICS_BUNDLE_STAT_DISABLE,
	    /*l2_prefix_ext_mode;*/          WP_IW_L2_HEADER_EXTENSION_ENABLE,
	    /*enhanced_flow_stat_mode;*/     WP_IW_ENHANCED_FLOW_STAT_ENABLE,
	    /*flow_stat_mode;*/              WP_IW_FLOW_STAT_ENABLE,
	    /*fr_tx_flow_stat_mode*/         WP_IW_FR_TX_FLOW_STAT_DISABLE,
	    /*mfc_alloc_mode*/               WP_IW_MFC_ALLOC_ENABLE,
	    /*learning_queue_mode */         WP_IW_LEARNING_ENABLE,
	    /*port_filtering_mode */         WP_IW_PORT_FILTERING_DISABLE,
	},
	/* max_iw_stat_bundles*/ 0,
	/* max_fr_s_iw_systems*/ 0,
	/* max_bridging_ports */ 200,
	/* max_iw_mc_groups   */ 1
    };

WP_atm_global atm_params =
    {
	/* max_pm_tables */        0,
	/* max_ubrplus_channels */ 10,
	/* max_upc_tables */       0,
	/* max_rx_cids */          0,
	/* max_tx_cids */          0,
	/* max_cid_range */        0,
	/* qsr_bus */              WP_BUS_NONE,
	/* qsr_bank */             0,
	{  /* modes */
	    WP_ATM_RAS_TIMESTAMP_DISABLE,
	    WP_AAL2_RX_3FE_DISABLE
	},
	/* max_stats_extensions */ 2,
	/* max_hier_shaping_groups */ 0
    };

WP_context context =
{
   3, /* qnodes */
   /* the first four assignments are mandatory bus assignments */
   {
       /*WP_VB_PARAM = 0,                       */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_PACKET,                          */{WP_BUS_PACKET, APP_BANK_PACKET},
       /*WP_VB_INTERNAL,                        */{WP_BUS_INTERNAL, APP_BANK_INTERNAL},
       /*WP_VB_HOST,                            */{WP_BUS_HOST, APP_BANK_HOST},
       /*WP_VB_INTQUEUE_DEBUG,                  */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_TIMESTAMP,                       */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_CAS,                             */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_TRANS_TSAT,                      */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_TPT,                             */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_CW,                              */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_ASU_TXQUEUE,                     */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_STATISTICS,                      */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_ATM_ADDLOOKUP,                   */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_ATM_CPT_EBMT_EXTHEADER,          */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_ETH_HDLC_CPT,                    */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_FBP_BD,                          */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_FBP_DATA,                        */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_L2_BKGND,                        */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_BRIDGE_LEARNING_DFC_PFMT         */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_IMA_MLPPP_STATE,                 */{WP_BUS_INTERNAL, APP_BANK_INTERNAL},
       /*WP_VB_IMA_MLPPP_BUFFER,                */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_HASH_IW_LPM_EMC,                 */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_IW_MFC_RULES,                    */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_IW_FIWT_QAT,                     */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_IW_MFCPT_DIFFSERV_UPPT_D         */{WP_BUS_INTERNAL, APP_BANK_INTERNAL},
       /*WP_VB_STATISTICS2,                     */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_STATISTICS3,                     */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_WIMAX_TABLE_BUS/WP_VB_IMA_RX_BUS */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_PKT_GLOBAL_TCPT,                 */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_AAL2_QSR/WP_VB_IMA_TX_BUS	*/{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_UNASSIGNED9,                     */{WP_BUS_NONE, 0},
       /*WP_VB_UNASSIGNED10,                    */{WP_BUS_NONE, 0},
       /*WP_VB_MCH_DATA,                        */{WP_BUS_NONE, 0},
       /*WP_VB_OAM_FM_QUERY,                    */{WP_BUS_NONE, 0},
       /*WP_VB_HASH_ON_HEAP,                    */{WP_BUS_NONE, 0},
       /*WP_VB_MFC_INTERNAL,                    */{WP_BUS_PARAM, APP_BANK_PARAM},
       /*WP_VB_MFR_RAS_TBL,                     */{WP_BUS_NONE, 0},
       /*WP_VB_CESOP_DATA,                      */{WP_BUS_PARAM, APP_BANK_PARAM}
   },
   {
    dps_ProgramImage, dps_ProgramImage
   },
   {
    dps_PC_Table_Init, dps_PC_Table_Init
   },
   &int_queue_tables, /* Interrupt queue tables         */
   POOL_LIMIT,        /* Maximum number of pools        */
   &atm_params,       /* ATM global parameters          */
   &max_iw            /* Interworking global parameters */
};

/***********         BRIDGING      ***********/
WP_iw_sys_bridging config_iw_sys =
    {
	/* tag */                   0,
	/* max_flows; */            0,
	/* classification_mode */   WP_IW_NULL_CLASSIFIER,
	/* classifier_config */     { WP_IW_NULL_CLASSIFIER, 128, 0, NULL },
	/* learning_mode;*/         WP_IW_BRIDGE_LEARNING_DISABLE,
	/* learning_queue */
	{
		/*int_queue_num*/         WP_IW_IRQT0,
		/*learning_queue_size*/   0,
		/*interrupt_enbale*/      WP_LEARNING_POLLING_MODE,
		/*interrupt_rate*/        1
	},
	/*forwarding_table_size*/    WP_IW_32_HASH_ENTRIES,
	/*member_set_size*/          WP_IW_32_HASH_ENTRIES,
	/*buffer_gap;*/              0x40,
	/*max_bridging_ports*/       2,
	/* *dfc_info */              NULL
    };

WP_bridge_port config_bridge_port =
{
	/* tag */                    0,
	/* direct_mapping */         WP_IW_DIRECT_MAP_ENABLE,
	/* flow_agg */               0,
	/* flooding_term_mode */     WP_IW_HOST_TERM_MODE,
	/* learning_mode */          WP_IW_LEARNING_DISABLE,
	/* in_filter_mode */         WP_IW_INGRESS_FILTER_DISABLE,
	/* vlan_param */
	{
		/* vlan_acceptance_mode */WP_IW_ACCEPT_TAGGED_UNTAGGED,
		/* vlan_tag */ 10,
	},
	/* max_mac_addresses */      0,
	/* group_tag */              0,
	/* group_filtering_mode */   WP_IW_GROUP_FILTER_DISABLE,
	/* unk_mac_sa_filter*/       WP_IW_UNK_MACSA_FILTER_DISABLE,
	/* unk_mc_mode;*/            WP_IW_UNK_MC_HT,
	/* bc_ht_mode;*/             WP_IW_BC_HT_DISABLE,
	/* input_filters_mask;*/     0,
	/* output_filters_mask;*/    0,
	/* statmode;*/               WP_IW_PORT_STAT_ENABLE,
	/* unk_uc_mode */            WP_IW_UNK_UC_SR_DISABLE,
	/* classification_flag */    WP_IW_BPORT_CLASSIFICATION_DISABLED
};

WP_iw_agg_bridging flow_agg_cfg =
    {
	/*tag*/                     0,
	/*txfunc*/                  0,
	/*output_bport*/            0,
	/*rfcs*/                    WP_IW_RFCS_ENABLE,
	/*l2_header_insert_mode;*/  WP_IW_L2H_INSERT_DISABLE,
	/*vlan_tag_mode*/           WP_IW_VLAN_TAG_DISABLE,
	/*interruptqueue;*/         WP_IW_IRQT1,
	/*error_pkt_mode*/          WP_IW_ERRPKT_DISCARD,
	/*intmode;*/                WP_IW_INT_ENABLE,
	/*statmode;*/               WP_IW_STAT_ENABLE,
	/*timestamp_mode;*/         WP_IW_TIME_STAMP_DISABLE,
	/*ov_pool_mode */           WP_IW_OV_POOL_DISABLE,
	/*fbp_drop_threshold;*/     0,
	/*replace_vlan_tag*/        WP_IW_REPLACE_VTAG_DISABLE,
	/*vlan_tag*/                0,
	/*vpmt */                   0,
	/*mtu;*/                    1536,
	/*prefix_length */          0,
	/*prefix_header[22];*/      {0},
	/*policer_enable*/          WP_IW_POLICER_DISABLE,
	/*policer_config;*/        NULL,
	/*cong_mode;*/              WP_IW_CONGESTION_DISABLE,
	/*cong_threshold_param;*/  NULL
    };

WP_rx_binding_bridging rx_binding_cfg=
    {
	/*  encap_mode */0,
	/*  mru;*/       1536,
	/*  vcfcs;*/     0,
	/*  input_port;*/0
    };

WP_tx_binding tx_binding_cfg=
    {
	/* res0 */     0,
	/* dci_mode;*/ WP_IW_DYN_CH_INSERT_ENABLE,
	/* maxt;*/     1024
    };

WP_ch_iw_rx ch_config_iw =
    {
	/* pqblock */         0,
	/* pqlevel */         0,
	/* tx_binding_type */ WP_IW_TX_BINDING,
	/* tx_binding_config*/ &tx_binding_cfg
    };

/******     Pools and QNodes configurations      ******/
WP_pool_buffer_data buffer_data_2048[2] =
    {
	{
	    /* n_buffers */ 1000,
	    /* offset */    64,
	    /* size */      1984,
	    /* pad */       0,
	    /* bus */       WP_BUS_PARAM,
	    /* bank */      APP_BANK_PARAM
	},
	{
	    /* n_buffers */ 2048,
	    /* offset */    64,
	    /* size */      1984,
	    /* pad */       0,
	    /* bus */       WP_BUS_PARAM,
	    /* bank */      APP_BANK_PARAM
	}
    };   

/* Function: Structs used in wp_create_host_Qnode */
WP_pool_buffer_data buffers_host_config =
      { /* n_buffers */ 24 * NUM_OF_ALL_HOST_CHANNELS,/*n_rings * ring_length*/
        /* offset */    0,
        /* size */      1536, /*Expect only OAM cells come to MIPS.
                               so reserving 2*cell size. cell size is... 48 */
        /* pad */       0,
        /* bus */       WP_BUS_PARAM,
        /* bank */      APP_BANK_PARAM
      };

WP_pool_ring_data ring_host_config =
      {
         /* n_rings */  NUM_OF_ALL_HOST_CHANNELS, /* Host Termination part
                             of All RX ATM channels + Non IW Host TX channels */
         /* ring_length */ 24,    /* 8*3. Rcving OAM cells for 8 pvcs.
                           Reserving ring len for 3 times the PVC channels*/
         /* bus */         WP_BUS_PARAM,
         /* bank */        APP_BANK_PARAM
      };

WP_qnode_hostq qnode_host_cfg =
      {
         /* pool_buffer */    0,
         /* pool_ring */      0,
         /* interruptqueue */ WP_IRQT1, /* This interrupt queue  will be
           used for all the events generated for the pkts rcvd at MIPS */
      };

WP_qnode_iwq qn_iw =
    {
	/* interruptqueue */ 0,
	/* num_buffers */    1000,
	/* adjunct_pool */   0
    };

WP_pool_ring_data ring_data_host =
    {
	/* n_rings */     256,
	/* ring_length */ 32,
	/* bus */         WP_BUS_PARAM,
	/* bank */        APP_BANK_PARAM
    };

WP_qnode_hostq qn_host =
    {
	/* pool_buffer */    0,
	/* pool_ring */      0,
	/* interruptqueue */ WP_IRQT0,
    };

/******     Direct map      ******/
WP_iw_sys_directmap iw_sys_directmap = 
    {
	/* max_flows */   100,
	/* buffer_gap */  64
    };

WP_rx_binding_directmap rx_binding_directmap=
    {
	/*  default_aggregation */ 0,
	/*  mru;*/                 1536
    };

/***************************** ENET **********************************/
WP_port_enet config_port_enet =
    {
	/* pkt_limits */     {64, 64, 0, 0},
	/* flowmode */       WP_FLOWMODE_FAST,
	/* interface_mode */ WP_ENET_1000_BASE_X,
	/* rx_iw_bkgnd */    WP_IW_BKGND_NOT_USED             
    };

WP_device_enet config_device_enet =
    {
	/* max_tx_channels */ 10,
	/* tx_maxsdu */       1536,
	/* operating speed */ WP_UNUSED,
	/* mac_addr */ {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
	/* tx_bit_rate */     1000000000,
	/* loopbackmode */    WP_ENET_NORMAL,
	/* extended_params */ NULL
    };

WP_ch_enet config_channel_enet =
    {
	/* intmode */         WP_PKTCH_INT_DISABLE,
	/* iwmode */          WP_PKTCH_IWM_ENABLE,
	/* testmode */        WP_PKTCH_TEST_DISABLE,
	/* tx_pqblock */      0,
	/* tx_pqlevel */      0,
	/* tx_shaping_type */ WP_FMU_SHAPING_TYPE_STRICT,
	/* tx_shaping_params */ NULL,
	/* rx_maxsdu */       1536
    };

/* for the interworking between the host and SGMII port */
WP_iw_agg_bridging tx_agg_h[1] =
    {
	{
            /*tag*/ 2,
            /*txfunc*/ 0,
            /*input_bport*/0,
            /*rfcs*/WP_IW_RFCS_ENABLE,
            /*l2_header_insert_mode;*/WP_IW_L2H_INSERT_DISABLE,
            /*vlan_tag_mode*/WP_IW_VLAN_TAG_DISABLE,
            /*interruptqueue;*/WP_IW_IRQT1,
            /*error_pkt_mode*/WP_IW_ERRPKT_DISCARD,
            /*intmode;*/WP_IW_INT_ENABLE,
            /*statmode;*/WP_IW_STAT_ENABLE,
            /*timestamp_mode;*/WP_IW_TIME_STAMP_DISABLE,
            /*ov_pool_mode */ WP_IW_OV_POOL_DISABLE,
            /*fbp_drop_threshold;*/0,
            /*replace_vlan_tag*/WP_IW_REPLACE_VTAG_DISABLE,
            /*vlan_tag*/0,
            /*vpmt_handle */0,
            /*mtu;*/1536,
            /*prefix_length */ 0,
            /*prefix_header[28];*/ {0,0,0,0,0,0,0,
                                    0,0,0,0,0,0,0,
                                    0,0,0,0,0,0,0,
                                    0,0,0,0,0,0,0},
            /*policer_enable*/ WP_IW_POLICER_DISABLE,
            /**policer_config;*/NULL,
            /*cong_mode;*/WP_IW_CONGESTION_DISABLE,
            /**cong_threshold_param;*/NULL
	}
    };

/***************************** TDI **********************************/
WP_ch_hdlc config_channel_hdlc =
    {
	/* intmode */ WP_PKTCH_INT_DISABLE,
	/* iwmmode */  WP_PKTCH_IWM_ENABLE,
	/* testmode */ WP_PKTCH_TEST_DISABLE,
	/* tx_pqblock */ 0,
	/* tx_pqlevel */ 0,
	/* tx_shaping_type */ WP_PKT_SHAPING_WFQ,
	/* tx_shaping_params */ &wfq,
	/* rx_maxsdu */ 1536,
	/* tw_cwid */ WP_CW_ID_A,
	/* tx_tq */ 3
    };

/******************** FUNCTIONS  ************************/

static unsigned int 
APP_TerminateOnError (WP_handle handle, WP_CHAR *s, WP_U32 LineNum)
{
   if (WP_ERROR_P(handle)) {
       printf("**** ERROR: %s %s 0x%x Line:%d\n", s,
	      WP_ErrorString(handle), handle, LineNum);
       return (FAILED);
   }
   return (PASSED);
}

static void 
APP_ErrCallback(WP_CHAR *file, WP_CHAR *function,WP_U32 line, WP_U32 error_id) 
{
    printf("%s \nFunction = %s \nLine %d \nError_Id = %d \n", 
	   file, function, line, error_id);
    return;
}

WP_status WT_WddiLogged(WP_S32 level, const WP_CHAR *string)
{
    if (level == 2)
	printf("\n%s", string);
}

void WT_CallTraceCallBack (WP_CHAR *file, WP_CHAR *function, WP_U32 line) 
{
    printf("\nWDDI call from ... \nFile = %s \nFunction =%s \nLine = %d \n", 
	   file, function, line);
}

/* sgmii_num: SGMII_INTERFACE_1 
   tdi_num: 0 - 7
*/
void
npu_print_dev_statistic (int bp_sgmii, int dc_sgmii, int tdi_num, int loop_mode)
{
    WP_handle bp_dev, bp_agg, bp_iwport, dc_dev, dc_agg, dc_iwport;
    WP_handle tdm_dev, tdm_agg, tdm_iwport;
    WP_CHAR tdi_name[8];

    if (bp_sgmii == SGMII_INTERFACE_1) {
	bp_dev = enet_dev[0];
	bp_agg = enet_agg[0];
	bp_iwport = enet_iwport[0];
    } 

    if ((loop_mode == NPU_TDI_LPBK) || 
	(loop_mode == NPU_TDI_PASSTHRU_MASTER) ||
	(loop_mode == NPU_TDI_PASSTHRU_SLAVE)) {
	tdm_dev = tdi_dev[tdi_num];
	tdm_agg = tdi_agg[tdi_num];
	tdm_iwport = tdi_iwport[tdi_num];
	sprintf(tdi_name, "tdi_%d", tdi_num);
	APP_DisplayStat(tdm_dev, TDM_DEV, tdi_name);
	APP_DisplayStat(tdm_iwport, BR_PORT, "tdm_iwport");
	APP_DisplayStat(tdm_agg, FLOW_AGG, "tdm_agg");
    } else {
	APP_DisplayStat(bp_dev, ENET_DEV, "bp_dev");
	APP_DisplayStat(bp_iwport, BR_PORT, "bp_iwport");
	APP_DisplayStat(bp_agg, FLOW_AGG, "bp_agg");
	APP_DisplayStat(host_default_agg, FLOW_AGG, "host_default_agg");

	if (loop_mode == NPU_DC_ENET_LPBK) {
	    if (dc_sgmii == SGMII_INTERFACE_2) {
		dc_dev = enet_dev[1];
		dc_agg = enet_agg[1];
		dc_iwport = enet_iwport[1];
	    } 

	    APP_DisplayStat(dc_dev, ENET_DEV, "dc_dev");
	    APP_DisplayStat(dc_iwport, BR_PORT, "dc_iw_port");
	    APP_DisplayStat(dc_agg, FLOW_AGG, "dc_agg");
	}
    }
}


static void
APP_DisplayStat(WP_handle object, WP_U8 type, WP_CHAR *name)
{
    WP_stats_enet stats_enet;
    WP_bport_stats bport_stats;
    WP_iw_flow_stats flow_stats;
    WP_handle status;
    /* Initialized statistics block for TDI HDLC device */
    WP_stats_tdi_hdlc tdi_hdlc_stats;
    
    switch (type){
    case ENET_DEV:
	 memset( &stats_enet, 0, sizeof(WP_stats_enet) );
         status = WP_DeviceStatistics(object, &stats_enet);
         APP_TerminateOnError (status, "WP_DeviceStatistics ENET",__LINE__);
         printf("\nENET device: %s\n", name);
	 printf("txrx_frames_64 = %d\n",(WP_U32)stats_enet.txrx_frames_64);
	 printf("txrx_frames_127 = %d\n",(WP_U32)stats_enet.txrx_frames_127);
	 printf("txrx_frames_255 = %d\n",(WP_U32)stats_enet.txrx_frames_255);
	 printf("txrx_frames_511 = %d\n",(WP_U32)stats_enet.txrx_frames_511);
	 printf("txrx_frames_1023 = %d\n",(WP_U32)stats_enet.txrx_frames_1023);
	 printf("txrx_frames_1518 = %d\n",(WP_U32)stats_enet.txrx_frames_1518);
	 printf("txrx_frames_1522 = %d\n",(WP_U32)stats_enet.txrx_frames_1522);
	 printf("rx_bytes = %d\n",(WP_U32)stats_enet.rx_bytes);
	 printf("rx_packets = %d\n",(WP_U32)stats_enet.rx_packets);
	 printf("rx_err_fcs = %d\n",(WP_U32)stats_enet.rx_err_fcs);
	 printf("rx_multicast = %d\n",(WP_U32)stats_enet.rx_multicast);
	 printf("rx_broadcast = %d\n",(WP_U32)stats_enet.rx_broadcast);
	 printf("rx_mac_control = %d\n",(WP_U32)stats_enet.rx_mac_control);
	 printf("rx_mac_pause = %d\n",(WP_U32)stats_enet.rx_mac_pause);
	 printf("rx_mac_unknown = %d\n",(WP_U32)stats_enet.rx_mac_unknown);
	 printf("rx_err_alignment = %d\n",(WP_U32)stats_enet.rx_err_alignment);
	 printf("rx_err_length = %d\n",(WP_U32)stats_enet.rx_err_length);
	 printf("rx_err_code = %d\n",(WP_U32)stats_enet.rx_err_code);
	 printf("rx_false_carrier = %d\n",(WP_U32)stats_enet.rx_false_carrier);
	 printf("rx_undersize = %d\n",(WP_U32)stats_enet.rx_undersize);
	 printf("rx_oversize = %d\n",(WP_U32)stats_enet.rx_oversize);
	 printf("rx_fragments = %d\n",(WP_U32)stats_enet.rx_fragments);
	 printf("rx_jabber = %d\n",(WP_U32)stats_enet.rx_jabber);
	 printf("rx_dropped = %d\n",(WP_U32)stats_enet.rx_dropped);
	 printf("tx_bytes = %d\n",(WP_U32)stats_enet.tx_bytes);
	 printf("tx_packets = %d\n",(WP_U32)stats_enet.tx_packets);
	 printf("tx_multicast = %d\n",(WP_U32)stats_enet.tx_multicast);
	 printf("tx_broadcast = %d\n",(WP_U32)stats_enet.tx_broadcast);
	 printf("tx_mac_pause = %d\n",(WP_U32)stats_enet.tx_mac_pause);
	 printf("tx_defer = %d\n",(WP_U32)stats_enet.tx_defer);
	 printf("tx_excess_defer = %d\n",(WP_U32)stats_enet.tx_excess_defer);
	 printf("tx_single_collision = %d\n",(WP_U32)stats_enet.tx_single_collision);
	 printf("tx_multi_collision = %d\n",(WP_U32)stats_enet.tx_multi_collision);
	 printf("tx_late_collision = %d\n",(WP_U32)stats_enet.tx_late_collision);
	 printf("tx_excess_collision = %d\n",(WP_U32)stats_enet.tx_excess_collision);
	 printf("tx_no_collision = %d\n",(WP_U32)stats_enet.tx_no_collision);
	 printf("tx_mac_pause_honored = %d\n",(WP_U32)stats_enet.tx_mac_pause_honored);
	 printf("tx_dropped = %d\n",(WP_U32)stats_enet.tx_dropped);
	 printf("tx_jabber = %d\n",(WP_U32)stats_enet.tx_jabber);
	 printf("tx_err_fcs = %d\n",(WP_U32)stats_enet.tx_err_fcs);
	 printf("tx_control = %d\n",(WP_U32)stats_enet.tx_control);
	 printf("tx_oversize = %d\n",(WP_U32)stats_enet.tx_oversize);
	 printf("tx_undersize = %d\n",(WP_U32)stats_enet.tx_undersize);
	 printf("tx_fragments = %d\n",(WP_U32)stats_enet.tx_fragments);
 
	 printf("rx_host_frames = %d\n",(WP_U32)stats_enet.rx_host_frames);
	 printf("rx_err_host_full = %d\n",(WP_U32)stats_enet.rx_err_host_full);
	 printf("rx_err_fbp_underrun = %d\n",(WP_U32)stats_enet.rx_err_fbp_underrun);
	 printf("rx_err_nonvalid_mac = %d\n",(WP_U32)stats_enet.rx_err_nonvalid_mac);
	 printf("rx_err_mru = %d\n",(WP_U32)stats_enet.rx_err_mru);
	 printf("rx_err_sdu = %d\n",(WP_U32)stats_enet.rx_err_sdu);
	 printf("tx_err_underrun = %d\n",(WP_U32)stats_enet.tx_err_underrun);
	 printf("rx_err_overrun = %d\n",(WP_U32)stats_enet.rx_err_overrun);
	 printf("tx_frames = %d\n",(WP_U32)stats_enet.tx_frames);
 	 printf("rx_iw_frames = %d\n",(WP_U32)stats_enet.rx_iw_frames);

	 break;

    case BR_PORT:
         memset(&bport_stats, 0, sizeof(bport_stats));
         status = WP_IwPortStatistics(object, &bport_stats);
         APP_TerminateOnError(status, "WP_BportStatistics", __LINE__);
         printf("\nBridge Port: %s\n", name);
         printf("rx_valid_packets = %d\n",(WP_U32)bport_stats.rx_valid_packets);
         printf("discard_vlan_acceptable_filter = %d\n",(WP_U32)bport_stats.discard_vlan_acceptable_filter);
         printf("discard_ingress_filter = %d\n",(WP_U32)bport_stats.discard_ingress_filter);
         printf("discard_bridge_classifier = %d\n",(WP_U32)bport_stats.discard_bridge_classifier);
         printf("discard_unk_macsa = %d\n",(WP_U32)bport_stats.discard_unk_macsa);
         printf("deny_mac_sa = %d\n",(WP_U32)bport_stats.deny_mac_sa);
         printf("deny_mac_da = %d\n",(WP_U32)bport_stats.deny_mac_da);
         printf("rx_bc_valid_packets = %d\n",(WP_U32)bport_stats.rx_bc_valid_packets);
         printf("rx_mc_valid_packets = %d\n",(WP_U32)bport_stats.rx_mc_valid_packets);
         printf("forwarded_uc_packets = %d\n",(WP_U32)bport_stats.forwarded_uc_packets);
         printf("forwarded_bc_packets = %d\n",(WP_U32)bport_stats.forwarded_bc_packets);
         printf("forwarded_mc_packets = %d\n",(WP_U32)bport_stats.forwarded_mc_packets);
	 break;

    case FLOW_AGG:
         memset(&flow_stats,0,sizeof(flow_stats));
         status = WP_IwFlowStatistics(object,WP_IW_FLOW_STAT,&flow_stats);
         APP_TerminateOnError(status, "WP_IwFlowStatistics", __LINE__);
         printf("\nFlow aggregation: %s\n", name);
         printf("forward_packet = %d\n",(WP_U32)flow_stats.forward_packet);
         printf("fbp_drop_packets = %d\n",(WP_U32)flow_stats.fbp_drop_packets);
         printf("mtu_drop_packets = %d\n",(WP_U32)flow_stats.mtu_drop_packets);
         printf("ttl_drop_packets = %d\n",(WP_U32)flow_stats.ttl_drop_packets);
         printf("tx_queue_drop_packets = %d\n",(WP_U32)flow_stats.tx_queue_drop_packets);
         printf("mpls_drop = %d\n",(WP_U32)flow_stats.mpls_drop);
         printf("denied_packets = %d\n",(WP_U32)flow_stats.denied_packets);
         printf("group_filtered_packets = %d\n",(WP_U32)flow_stats.group_filtered_packets);
         printf("forwarded_bytes = %d\n",(WP_U32)flow_stats.forwarded_bytes);
         printf("gtp_bad_headers = %d\n",(WP_U32)flow_stats.gtp_bad_headers);
	 break;

    case TDM_DEV:
	memset(&tdi_hdlc_stats, 0, sizeof(WP_stats_tdi_hdlc));
	status = WP_DeviceStatistics(object, &tdi_hdlc_stats);
	APP_TerminateOnError(status, "WP_DeviceStatistics TDM", __LINE__);
	printf("\nTDM device: %s\n", name);
	printf("TDI HDLC App: UPI Hdlc Device Statistics \n");
	printf("Rx Frames = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_frames >> 32),
                (WP_U32)(tdi_hdlc_stats.rx_frames & 0xffff));
	printf("Tx Frames = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.tx_frames >> 32),
		(WP_U32)(tdi_hdlc_stats.tx_frames & 0xffff));
	printf("Rx Err CRC = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_crc >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_crc & 0xffff));
	printf("Rx Err abort = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_abort >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_abort & 0xffff));
	printf("Rx Err addr mismatch = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_addr_mismatch >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_addr_mismatch & 0xffff));
	printf("Rx Err buffer overrun = %#8.8x%#8.8x\n",(WP_U32)
		(tdi_hdlc_stats.rx_err_buffer_overrun >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_buffer_overrun & 0xffff));
	printf("Rx Err overrun = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_overrun >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_overrun & 0xffff));
	printf("Tx Err underrun = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.tx_err_underrun >> 32),
		(WP_U32)(tdi_hdlc_stats.tx_err_underrun & 0xffff));
	printf("Rx Err iw buffer underrun = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_iw_buffer_underrun >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_iw_buffer_underrun & 0xffff));
	printf("Rx Err iw MRU = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_iw_mru >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_iw_mru & 0xffff));
	printf("Rx Err max sdu = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_maxsdu >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_maxsdu & 0xffff));
	printf("Rx iw frames = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_iw_frames >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_iw_frames & 0xffff));
	printf("Rx err non octet = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_non_octet >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_non_octet & 0xffff));
	printf("Rx bytes = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_bytes >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_bytes & 0xffff));
	printf("Rx err bytes = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_bytes >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_bytes & 0xffff));
	printf("Tx bytes = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.tx_bytes >> 32),
		(WP_U32)(tdi_hdlc_stats.tx_bytes & 0xffff));
	printf("Rx err iw l2 parse = %#8.8x%#8.8x\n",
		(WP_U32)(tdi_hdlc_stats.rx_err_iw_l2_parse >> 32),
		(WP_U32)(tdi_hdlc_stats.rx_err_iw_l2_parse & 0xffff));
	break;
	
    default:
	break;
   }
}


static int  
tdm_init_t1 (int loop_mode)
{
   WP_tdm_slotgroup tdm_slots[2] =
       {
	   {1, 24},
	   {0, 0}
       };

   WP_port_tdm tdmport_s =
       {
	   /* atm_limits */{32,32,0,0,0,0,0,0,0,2},
	   /* pkt_limits */{32,32,0,0},
	   /* trans_limits */{ {32,0,0,0}, {32,0,0,0} },
	   /* slot */      tdm_slots,
	   /* tdmmode */   WP_TDM_NORMAL,
	   /* framemode */ WP_TDM_FRAMED,
	   /* pinconfig */ WP_TDM_SEPARATE,
	   /* edge */      WP_TDM_FALLING,
	   /* delay */     3,
	   /* intmode */   WP_TDM_INT_DISABLE,
	   /* rx_iw_bkgnd */ WP_IW_BKGND_USED
       };

   WP_tdm_binding tdm_binding[2] =
       {
	   {0, 23}
       };

   WP_device_tdm_hdlc tdmdev_hdlc =
       {
	   /* n_timeslot_binding */ 1,
	   /* timeslot_binding */ tdm_binding,
	   /* tx_statmode */ WP_PKT_STAT_ENABLE,
	   /* tx_maxsdu */ 1536,
	   /* rx_statmode */ WP_PKT_STAT_ENABLE,
	   /* crctype */ WP_HDLC_CRC16,
	   /* numflags */ 2,
	   /* idlemode */ WP_TDI_IDLEMODE_FLAG,
	   /* flowmode */ WP_FLOWMODE_MULTI,
	   /* addr_mask */ 0,
	   /* address1 */ 0,
	   /* address2 */ 0,
	   /* tx_tq_types */ {WP_PKT_SCHED_WFQ,
			      WP_PKT_SCHED_WFQ,
			      WP_PKT_SCHED_WFQ,
			      WP_PKT_SCHED_WFQ}
       };

   WP_U8 i;
   WP_U32 app_tdm_ports[16] = {WP_PORT_TDM1, WP_PORT_TDM2, WP_PORT_TDM3, WP_PORT_TDM4, WP_PORT_TDM5, WP_PORT_TDM6, WP_PORT_TDM7, WP_PORT_TDM8, WP_PORT_TDM9, WP_PORT_TDM10, WP_PORT_TDM11, WP_PORT_TDM12, WP_PORT_TDM13, WP_PORT_TDM14, WP_PORT_TDM15, WP_PORT_TDM16};

    if (loop_mode == NPU_TDI_LPBK) {
	/* loopback mode */
	tdmport_s.tdmmode = WP_TDM_LOOPBACK;
    } else {
	/* normal mode */
	tdmport_s.tdmmode = WP_TDM_NORMAL;
    }
   
   /* Create TDM ports and devices, we will use WP_PORT_TDM9 - WP_PORT_TDM16 */
   for(i = 0;i < NUM_TDI;i++) {
       tdi_port[i] = WP_PortCreate(WP_WINPATH(0), app_tdm_ports[i+8], &tdmport_s);
       if (APP_TerminateOnError(tdi_port[i], "WP_PortCreate TDM", __LINE__)
	   == FAILED)
	   return (FAILED);

       tdi_dev[i] = WP_DeviceCreate(tdi_port[i], WP_UNUSED, WP_DEVICE_HDLC, &tdmdev_hdlc);
       if (APP_TerminateOnError(tdi_dev[i], "WP_DeviceCreate TDM", __LINE__)
	   == FAILED)
	   return (FAILED);
   }
   return (PASSED);
}

static int  
tdm_init_e1 (int loop_mode)
{
    WP_tdm_slotgroup tdm_slots[] =
	{
	    {/* repeat */ 1, /* size */ 1},
	    {/* repeat */ 1, /* size */ 15},
	    {/* repeat */ 1, /* size */ 1},
	    {/* repeat */ 1, /* size */ 15},
	    {/* repeat */ 0, /* size */ 0}
	};

   WP_port_tdm tdmport_s =
       {
	   /* atm_limits */{32,32,0,0,0,0,0,0,0,2},
	   /* pkt_limits */{32,32,0,0},
	   /* trans_limits */{ {32,0,0,0}, {32,0,0,0} },
	   /* slot */      tdm_slots,
	   /* tdmmode */   WP_TDM_NORMAL,
	   /* framemode */ WP_TDM_FRAMED,
	   /* pinconfig */ WP_TDM_SEPARATE,
	   /* edge */      WP_TDM_FALLING,
	   /* delay */     3,
	   /* intmode */   WP_TDM_INT_DISABLE,
	   /* rx_iw_bkgnd */ WP_IW_BKGND_USED
       };

   WP_tdm_binding tdm_binding[] =
       {
	   {1, 15}, 
	   {17,31}
       };

   WP_device_tdm_hdlc tdmdev_hdlc =
       {
	   /* n_timeslot_binding */ 2,
	   /* timeslot_binding */ tdm_binding,
	   /* tx_statmode */ WP_PKT_STAT_ENABLE,
	   /* tx_maxsdu */ 1536,
	   /* rx_statmode */ WP_PKT_STAT_ENABLE,
	   /* crctype */ WP_HDLC_CRC16,
	   /* numflags */ 2,
	   /* idlemode */ WP_TDI_IDLEMODE_FLAG,
	   /* flowmode */ WP_FLOWMODE_MULTI,
	   /* addr_mask */ 0,
	   /* address1 */ 0,
	   /* address2 */ 0,
	   /* tx_tq_types */ {WP_PKT_SCHED_WFQ,
			      WP_PKT_SCHED_WFQ,
			      WP_PKT_SCHED_WFQ,
			      WP_PKT_SCHED_WFQ}
       };

   WP_U8 i;
   WP_U32 app_tdm_ports[16] = {WP_PORT_TDM1, WP_PORT_TDM2, WP_PORT_TDM3, WP_PORT_TDM4, WP_PORT_TDM5, WP_PORT_TDM6, WP_PORT_TDM7, WP_PORT_TDM8, WP_PORT_TDM9, WP_PORT_TDM10, WP_PORT_TDM11, WP_PORT_TDM12, WP_PORT_TDM13, WP_PORT_TDM14, WP_PORT_TDM15, WP_PORT_TDM16};

   if (loop_mode == NPU_TDI_LPBK) {
	/* loopback mode */
	tdmport_s.tdmmode = WP_TDM_LOOPBACK;
    } else {
	/* normal mode */
	tdmport_s.tdmmode = WP_TDM_NORMAL;
	if (loop_mode == NPU_TDI_PASSTHRU_SLAVE) {
	    tdmport_s.delay = 1;
	} else {
	    tdmport_s.delay = 2;
	}
    }
   
   /* Create TDM ports and devices */
   for(i = 0;i < NUM_TDI;i++) {
       tdi_port[i] = WP_PortCreate(WP_WINPATH(0), app_tdm_ports[i+8], &tdmport_s);
       if (APP_TerminateOnError(tdi_port[i], "WP_PortCreate TDM", __LINE__)
	   == FAILED)
	   return (FAILED);

       tdi_dev[i] = WP_DeviceCreate(tdi_port[i], WP_UNUSED, WP_DEVICE_HDLC, &tdmdev_hdlc);
       if (APP_TerminateOnError(tdi_dev[i], "WP_DeviceCreate TDM", __LINE__)
	   == FAILED)
	   return (FAILED);
   }
   return (PASSED);
}

static int 
host_sgmii_init (int enet_num, int loop_mode)
{
    WP_handle bp_port, bp_dev;

    if (enet_num == SGMII_INTERFACE_1) {
	bp_port = enet_port[0];
	bp_dev = enet_dev[0];
	config_device_enet.mac_addr[0] = 0x11;
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    if (loop_mode == TRUE) {
	/* loopback mode */
	config_device_enet.loopbackmode = WP_ENET_LOOPBACK;
    } else {
	/* normal mode */
	config_device_enet.loopbackmode = WP_ENET_NORMAL;
	config_device_enet.mac_addr[0] = 0xff;
	config_device_enet.mac_addr[1] = 0xff;
	config_device_enet.mac_addr[2] = 0xff;
	config_device_enet.mac_addr[3] = 0xff;
	config_device_enet.mac_addr[4] = 0xff;
	config_device_enet.mac_addr[5] = 0xff;
    }

    /* create SGMII port and device */
    bp_port = WP_PortCreate(WP_WINPATH(0), enet_num, &config_port_enet);
    if (APP_TerminateOnError(bp_port, "WP_PortCreate", __LINE__) == FAILED)
       return (FAILED);

    bp_dev = WP_DeviceCreate(bp_port, WP_PHY(0), WP_DEVICE_ENET, &config_device_enet);
    if (APP_TerminateOnError(bp_dev, "WP_DeviceCreate", __LINE__) == FAILED)
       return (FAILED);

    /* Create IW buffer pool and queue node */
    iw_pool = WP_PoolCreate(WP_WINPATH(0), WP_pool_iwbuffer, &buffer_data_2048[0]);
    if (APP_TerminateOnError(iw_pool, "WP_PoolCreate() iw_pool", __LINE__)
	== FAILED)
	   return (FAILED);

    qn_iw.adjunct_pool = iw_pool;
    qniw = WP_QNodeCreate(0, WP_QNODE_IWQ | WP_QNODE_OPT_DEDICATED_RX_HWQ | WP_QNODE_OPT_FMU, &qn_iw);
    if (APP_TerminateOnError(qniw, "WP_QNodeCreate() qniw", __LINE__)
	== FAILED)
	   return (FAILED);

    /* Pool for host termination buffers */
    h_pool_256 = WP_PoolCreate(WP_WINPATH(0), WP_pool_buffer,
			       &buffers_host_config);
    
    h_pool_ring_host = WP_PoolCreate(WP_WINPATH(0), WP_pool_ring,
				     &ring_host_config);
    /* Host termination qnode */
    qnode_host_cfg.pool_buffer = h_pool_256;
    qnode_host_cfg.pool_ring = h_pool_ring_host;
    h_qnode_host = WP_QNodeCreate(WP_WINPATH(0), WP_QNODE_HOSTQ, &qnode_host_cfg);

    /* Create host port and device. */
    host_port =  WP_PortCreate(WP_WINPATH(0),WP_PORT_IW_HOST,NULL);
    if (APP_TerminateOnError(host_port,"Host Port Create", __LINE__) == FAILED)
       return (FAILED);

    host_dev = WP_DeviceCreate(host_port,0,WP_DEVICE_IW_HOST,NULL);
    if (APP_TerminateOnError(host_dev,"Host Device Create", __LINE__) == FAILED)
       return (FAILED);

    /* save back to global variables. */
    if (enet_num == SGMII_INTERFACE_1) {
	enet_port[0] = bp_port;
	enet_dev[0] = bp_dev;
    } 

    return (PASSED);
}

static int 
host_sgmii_iwcreate (int enet_num)
{
    WP_handle bp_port, bp_dev, bp_iwport, bp_rx_channel, bp_tx_channel, bp_agg;
    WP_ch_iw_tx ch_iw_tx = {
	/* iw_system */                 0,
	/* reserved */                  0,
	/* group_id */                  WP_IW_CH_GROUP_ID_NULL,
	/* bridging_group_tag */        0,
	/* input_port */                0
    };
    WP_handle status;

    if (enet_num == SGMII_INTERFACE_1) {
	bp_port = enet_port[0];
	bp_dev = enet_dev[0];
	bp_iwport = enet_iwport[0];
	bp_rx_channel = enet_rx_channel[0];
	bp_tx_channel = enet_tx_channel[0];
	bp_agg = enet_agg[0];
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    /*******************  HOST   ****************************/
    /* Host RX channel for default flow agg */
    host_rx_channel = WP_ChannelCreate(0, host_dev, qniw, WP_CH_RX, WP_IW_HOST, &ch_config_iw);
    if (APP_TerminateOnError(host_rx_channel, "WP_ChannelCreate IW Rx", __LINE__)
	== FAILED)
	return (FAILED);

    flow_agg_cfg.txfunc = host_rx_channel;  
    flow_agg_cfg.l2_header_insert_mode = WP_IW_L2H_INSERT_DISABLE;
    flow_agg_cfg.prefix_length = 0;

    host_default_agg = WP_IwFlowAggregationCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &flow_agg_cfg);
    if (APP_TerminateOnError(host_default_agg, "WP_IwFlowAggregationCreate HT", __LINE__) == FAILED)
	return (FAILED);

    /* backplane GE bridge port on IW */
    config_bridge_port.flow_agg = host_default_agg;
    config_bridge_port.tag = 1 ;
    bp_iwport = WP_IwPortCreate(iw_sys, &config_bridge_port);
    if (APP_TerminateOnError(bp_iwport, "WP_IwPortCreate", __LINE__)
	== FAILED)
	return (FAILED);

    /* Host TX channel for HostSend */
    ch_iw_tx.iw_system = iw_sys;
    ch_iw_tx.input_port = bp_iwport;

    host_tx_channel = WP_ChannelCreate(0, host_dev, qniw, WP_CH_TX, WP_IW_HOST, &ch_iw_tx);
    if (APP_TerminateOnError(host_tx_channel, "WP_ChannelCreate IW TX", __LINE__)
	== FAILED)
	return (FAILED);

    /* backplane GE RX channels */
    bp_rx_channel = WP_ChannelCreate(0x20, bp_dev, qniw, WP_CH_RX, WP_ENET, &config_channel_enet);
    if (APP_TerminateOnError(bp_rx_channel,"WP_ChannelCreate() gbe_rx", __LINE__)
       == FAILED)
       return (FAILED);    

    rx_binding_cfg.input_port = bp_iwport;   
    status = WP_IwRxBindingCreate(bp_rx_channel,iw_sys,qniw,&rx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwRxBindingCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    /* backplane GE TX channels */
    bp_tx_channel = WP_ChannelCreate(0x19, bp_dev, qniw, WP_CH_TX, WP_ENET, &config_channel_enet);
    if (APP_TerminateOnError(bp_tx_channel,"WP_ChannelCreate() gbe_tx", __LINE__) 
	== FAILED)
	return (FAILED);

    status = WP_IwTxBindingCreate(bp_tx_channel, WP_IW_TX_BINDING, &tx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwTxBindingCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    /* backplane GE flow aggs */
    flow_agg_cfg.iw_port = bp_iwport;
    flow_agg_cfg.txfunc = bp_tx_channel;
    flow_agg_cfg.l2_header_insert_mode = WP_IW_L2H_INSERT_DISABLE;
    flow_agg_cfg.prefix_length = 0;

    bp_agg = WP_IwFlowAggregationCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &flow_agg_cfg);

    if (APP_TerminateOnError(bp_agg, "WP_IwFlowAggregationCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    /* save back to global variables. */
    if (enet_num == SGMII_INTERFACE_1) {
	enet_iwport[0] = bp_iwport;
	enet_rx_channel[0] = bp_rx_channel;
	enet_tx_channel[0] = bp_tx_channel;
	enet_agg[0] = bp_agg;
    } 

    return (PASSED);
}


static int 
host_sgmii_enable (int enet_num)
{
    WP_handle bp_port, bp_dev, bp_rx_channel, bp_tx_channel;
    WP_handle status;

    if (enet_num == SGMII_INTERFACE_1) {
	bp_port = enet_port[0];
	bp_dev = enet_dev[0];
	bp_rx_channel = enet_rx_channel[0];
	bp_tx_channel = enet_tx_channel[0];
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    status = WP_PortEnable(bp_port, WP_DIRECTION_DUPLEX);
    if (APP_TerminateOnError(status,"WP_PortEnable() bp_port", __LINE__) 
	== FAILED)
	return (FAILED);  

    status = WP_DeviceEnable(bp_dev,WP_DIRECTION_DUPLEX);
    if (APP_TerminateOnError(status,"WP_DeviceEnable() bp_dev", __LINE__) 
	== FAILED)
	return (FAILED);  

    status = WP_ChannelEnable(bp_tx_channel);
    if (APP_TerminateOnError(status, "WP_ChannelEnable bp_tx_channel", __LINE__)
	== FAILED)
	return (FAILED);  
    
    status = WP_ChannelEnable(bp_rx_channel);
    if (APP_TerminateOnError(status, "WP_ChannelEnable bp_rx_channel", __LINE__)
	== FAILED)
	return (FAILED);  

    return (PASSED);
}


static int 
npu_tdi_config (int enet_num, int tdi_num, int loop_mode, int op_mode)
{
    WP_handle status;
    WP_handle bp_port, bp_dev, bp_iwport, bp_agg, bp_rx_channel, bp_tx_channel;
    WP_handle tdm_port, tdm_dev, tdm_iwport, tdm_agg, tdm_rx_channel, tdm_tx_channel;
#ifdef DEBUG
    ulong debug_addr, i;
#endif

    /* WDDI debug help */
    status = WP_ControlRegister(WP_DEBUG_CALLBACK_FILE_LINE_ERROR, &APP_ErrCallback);
    status = WP_ControlRegister(WP_DEBUG_CALLBACK_WDDI_LOG, &WT_WddiLogged);

    //    WP_ControlRegister(WP_DEBUG_CALLBACK_WDDI_CALL, &WT_CallTraceCallBack);

    status = WP_DriverInit();
    if (APP_TerminateOnError(status, "WP_DriverInit", __LINE__) == FAILED)
       return (FAILED);

    status = WPX_BoardSerdesInit(0, enet_num, WPX_SERDES_NORMAL);
    if (APP_TerminateOnError(status, "WPX_BoardSerdesInit for port 11", __LINE__) == FAILED)
       return (FAILED);

    status = WP_SysInit(WP_WINPATH(0), &context);
    if (APP_TerminateOnError(status, "WP_SysInit", __LINE__) == FAILED)
       return (FAILED);

    /* create host and SGMII ports and devices */
    if (host_sgmii_init(enet_num, TRUE) == FAILED)
	return (FAILED);

    if (enet_num == SGMII_INTERFACE_1) {
	bp_port = enet_port[0];
	bp_dev = enet_dev[0];
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    /* Create TDI ports and devices, we only support HDLC mode */
    if (op_mode == CMQ_MODE_T1) {
	if (tdm_init_t1(loop_mode) == FAILED)
	    return (FAILED);
    } else if (op_mode == CMQ_MODE_E1) {
	if (tdm_init_e1(loop_mode) == FAILED)
	    return (FAILED);
    } else {
	cterr('f',0,"Wrong op_mode type");
	return (FAILED);
    }

    tdm_port = tdi_port[tdi_num];
    tdm_dev = tdi_dev[tdi_num];
    tdm_iwport = tdi_iwport[tdi_num];
    tdm_agg = tdi_agg[tdi_num];
    tdm_rx_channel = tdi_rx_channel[tdi_num];
    tdm_tx_channel = tdi_tx_channel[tdi_num];

    status = WP_SysCommit();
    if (APP_TerminateOnError(status, "WP_SysCommit", __LINE__)
	== FAILED)
	   return (FAILED);

    /*******************  IW   ****************************/
    /* Create learning bridge IW system */
    iw_sys = WP_IwSystemCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &config_iw_sys);
    if (APP_TerminateOnError(iw_sys, "WP_IwSystemCreate", __LINE__)
	== FAILED)
	return (FAILED);

    if (host_sgmii_iwcreate(enet_num) == FAILED)
	return (FAILED);

    if (enet_num == SGMII_INTERFACE_1) {
	bp_iwport = enet_iwport[0];
	bp_agg = enet_agg[0];
	bp_rx_channel = enet_rx_channel[0];
	bp_tx_channel = enet_tx_channel[0];
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    /*******************  TDI   ****************************/
    /* TDI bridge port on IW */
    config_bridge_port.flow_agg = host_default_agg;
    config_bridge_port.tag = 2 ;
    tdm_iwport = WP_IwPortCreate(iw_sys, &config_bridge_port);
    if (APP_TerminateOnError(tdm_iwport, "WP_IwPortCreate tdm_iwport", __LINE__)
	== FAILED)
	return (FAILED);

    /* TDI RX channel */
    tdm_rx_channel = WP_ChannelCreate(0x22, tdm_dev, h_qnode_host, WP_CH_RX, WP_HDLC, &config_channel_hdlc);
    if (APP_TerminateOnError(tdm_rx_channel, "WP_Channel_Create() TDI RX", __LINE__)
	== FAILED)
	return (FAILED);

    rx_binding_cfg.input_port = tdm_iwport;   
    status = WP_IwRxBindingCreate(tdm_rx_channel, iw_sys, qniw, &rx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwRxBindingCreate TDI", __LINE__)
	== FAILED)
	return (FAILED);

    /* TDI TX channel */
    tdm_tx_channel = WP_ChannelCreate(0x23, tdm_dev, qniw, WP_CH_TX, WP_HDLC, &config_channel_hdlc);
    if (APP_TerminateOnError(tdm_tx_channel,"WP_ChannelCreate() tdm_tx", __LINE__) 
	== FAILED)
	return (FAILED);

    status = WP_IwTxBindingCreate(tdm_tx_channel, WP_IW_TX_BINDING, &tx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwTxBindingCreate TDI", __LINE__)
	== FAILED)
	return (FAILED);

    /* TDI flow aggs */
    flow_agg_cfg.iw_port = tdm_iwport;
    flow_agg_cfg.txfunc = tdm_tx_channel;
    flow_agg_cfg.l2_header_insert_mode = WP_IW_L2H_INSERT_ENABLE;
    flow_agg_cfg.prefix_length = 6;
    flow_agg_cfg.prefix_header[0] = 0xff;
    flow_agg_cfg.prefix_header[1] = 0x03;
    flow_agg_cfg.prefix_header[2] = 0x00;
    flow_agg_cfg.prefix_header[3] = 0x31;
    flow_agg_cfg.prefix_header[4] = 0x00;
    flow_agg_cfg.prefix_header[5] = 0x01;

    tdm_agg = WP_IwFlowAggregationCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &flow_agg_cfg);
    if (APP_TerminateOnError(tdm_agg, "WP_IwFlowAggregationCreate TDI", __LINE__)
	== FAILED)
	return (FAILED);

    config_bridge_port.flow_agg = tdm_agg;
    status = WP_IwPortModify(bp_iwport, WP_IW_PORT_MODIFY_FLOW_AGG, &config_bridge_port);
    if (APP_TerminateOnError(status, "WP_IwPortModify", __LINE__)
	== FAILED)
	return (FAILED);

    rx_binding_cfg.input_port = bp_iwport;
    status = WP_IwRxBindingModify(bp_rx_channel, iw_sys, qniw, WP_IW_RX_BIND_MOD_IWPORT,&rx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwRxBindingCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    /*******************  ENABLES   ****************************/
    /* Build IW systems */
    status = WP_IwSystemBuild(iw_sys);
    if (APP_TerminateOnError(status," WP_IwSystemBuild() ", __LINE__ )
	== FAILED)
	return (FAILED);
   
    /* Enable ENET port, devices and tx channels */
    if (host_sgmii_enable(enet_num) == FAILED)
	return (FAILED);

    /*  Enable TDI system */
    status = WP_PortEnable(tdm_port, WP_DIRECTION_DUPLEX);
    if (APP_TerminateOnError(status, "WP_PortEnable TDM", __LINE__)
	== FAILED)
	return (FAILED);
    
    status = WP_DeviceEnable(tdm_dev, WP_DIRECTION_DUPLEX);
    if (APP_TerminateOnError(status, "WP_DeviceEnable TDM", __LINE__)
	== FAILED)
	return (FAILED);
#ifdef DEBUG
    debug_addr = get_npu_rif_base() + 0x12080 + 0x100 * (tdi_num+8);
    printf("TDM control register @ %#x = %#x\n", debug_addr, *(int *)debug_addr);
#endif
    status = WP_ChannelEnable(tdm_tx_channel);
    if (APP_TerminateOnError(status, "WP_ChannelEnable TDM", __LINE__)
	== FAILED)
	return (FAILED);

    /* save back to global variables */
    tdi_iwport[tdi_num] = tdm_iwport;
    tdi_agg[tdi_num] = tdm_agg;
    tdi_rx_channel[tdi_num] = tdm_rx_channel;
    tdi_tx_channel[tdi_num] = tdm_tx_channel;

    return (PASSED);
}


static int 
npu_bp_sgmii_config (int enet_num, int loop_mode)
{
    WP_handle status;

    /* WDDI debug help */
    status = WP_ControlRegister(WP_DEBUG_CALLBACK_FILE_LINE_ERROR, &APP_ErrCallback);

    status = WP_DriverInit();
    if (APP_TerminateOnError(status, "WP_DriverInit", __LINE__) == FAILED)
       return (FAILED);

    status = WPX_BoardSerdesInit(0, enet_num, WPX_SERDES_NORMAL);
    if (APP_TerminateOnError(status, "WPX_BoardSerdesInit for port 11", __LINE__) == FAILED)
       return (FAILED);

    status = WP_SysInit(WP_WINPATH(0), &context);
    if (APP_TerminateOnError(status, "WP_SysInit", __LINE__) == FAILED)
       return (FAILED);

    /* create host and SGMII ports and devices */
    if (host_sgmii_init(enet_num, loop_mode) == FAILED)
	return (FAILED);

    status = WP_SysCommit();
    if (APP_TerminateOnError(status, "WP_SysCommit", __LINE__)
	== FAILED)
	   return (FAILED);

    /*******************  IW   ****************************/
    /* Create learning bridge IW system */
    iw_sys = WP_IwSystemCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &config_iw_sys);
    if (APP_TerminateOnError(iw_sys, "WP_IwSystemCreate", __LINE__) == FAILED)
	return (FAILED);

    if (host_sgmii_iwcreate(enet_num) == FAILED)
	return (FAILED);

    /* Build IW systems */
    status = WP_IwSystemBuild(iw_sys);
    if (APP_TerminateOnError(status," WP_IwSystemBuild() ", __LINE__ )
	== FAILED)
	return (FAILED);
    
    if (host_sgmii_enable(enet_num) == FAILED)
	return (FAILED);

    return (PASSED);
}

static int 
npu_dc_sgmii_config (int bp_enet, int dc_enet, int loop_mode)
{
    WP_handle status;
    WP_handle bp_port, bp_dev, bp_iwport, bp_rx_channel, bp_tx_channel, bp_agg;
    WP_handle dc_port, dc_dev, dc_iwport, dc_rx_channel, dc_tx_channel, dc_agg;

    if (dc_enet == SGMII_INTERFACE_2) {
	dc_port = enet_port[1];
	dc_dev = enet_dev[1];
	dc_iwport = enet_iwport[1];
	dc_rx_channel = enet_rx_channel[1];
	dc_tx_channel = enet_tx_channel[1];
	dc_agg = enet_agg[1];
	config_device_enet.mac_addr[0] = 0x22;
    } else {
	cterr('f', 0, "Wrong daughter card SGMII interface");
	return (FAILED);
    }

    /* WDDI debug help */
    status = WP_ControlRegister(WP_DEBUG_CALLBACK_FILE_LINE_ERROR, &APP_ErrCallback);

    status = WP_DriverInit();
    if (APP_TerminateOnError(status, "WP_DriverInit", __LINE__) == FAILED)
       return (FAILED);

    status = WPX_BoardSerdesInit(0, dc_enet, WPX_SERDES_NORMAL);
    if (APP_TerminateOnError(status, "WPX_BoardSerdesInit for port 9", __LINE__) == FAILED)
       return (FAILED);

    status = WPX_BoardSerdesInit(0, bp_enet, WPX_SERDES_NORMAL);
    if (APP_TerminateOnError(status, "WPX_BoardSerdesInit for port 11", __LINE__) == FAILED)
       return (FAILED);

    status = WP_SysInit(WP_WINPATH(0), &context);
    if (APP_TerminateOnError(status, "WP_SysInit", __LINE__) == FAILED)
       return (FAILED);

    /* create host and backplane SGMII ports and devices */
    if (loop_mode == TRUE) {
	if (host_sgmii_init(bp_enet, TRUE) == FAILED)
	    return (FAILED);
    } else {
	if (host_sgmii_init(bp_enet, FALSE) == FAILED)
	    return (FAILED);
    }

    if (bp_enet == SGMII_INTERFACE_1) {
	bp_port = enet_port[0];
	bp_dev = enet_dev[0];
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    if (loop_mode == TRUE) {
	/* loopback mode */
	config_device_enet.loopbackmode = WP_ENET_LOOPBACK;
    } else {
	/* normal mode */
	config_device_enet.loopbackmode = WP_ENET_NORMAL;
    }

    /* create daughter card SGMII port and device */
    dc_port = WP_PortCreate(WP_WINPATH(0), dc_enet, &config_port_enet);
    if (APP_TerminateOnError(dc_port, "WP_PortCreate dc_port", __LINE__) 
	== FAILED)
       return (FAILED);

    dc_dev = WP_DeviceCreate(dc_port, WP_PHY(0), WP_DEVICE_ENET, &config_device_enet);
    if (APP_TerminateOnError(dc_dev, "WP_DeviceCreate dc_dev", __LINE__) == FAILED)
       return (FAILED);

    status = WP_SysCommit();
    if (APP_TerminateOnError(status, "WP_SysCommit", __LINE__)
	== FAILED)
	   return (FAILED);           

    /*******************  IW   ****************************/
    /* Create learning bridge IW system */
    iw_sys = WP_IwSystemCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &config_iw_sys);
    if (APP_TerminateOnError(iw_sys, "WP_IwSystemCreate", __LINE__) == FAILED)
	return (FAILED);

    if (host_sgmii_iwcreate(bp_enet) == FAILED)
	return (FAILED);

    if (bp_enet == SGMII_INTERFACE_1) {
	bp_iwport = enet_iwport[0];
	bp_rx_channel = enet_rx_channel[0];
	bp_tx_channel = enet_tx_channel[0];
	bp_agg = enet_agg[0];
    } else {
	cterr('f', 0, "Wrong backplane SGMII interface");
	return (FAILED);
    }

    /* DC SGMII bridge port on IW */
    if (loop_mode == TRUE) {
	config_bridge_port.flow_agg = host_default_agg;
    } else {
	config_bridge_port.flow_agg = bp_agg;
    }	
    config_bridge_port.tag = 3;
    dc_iwport = WP_IwPortCreate(iw_sys, &config_bridge_port);
    if (APP_TerminateOnError(dc_iwport, "WP_IwPortCreate", __LINE__)
	== FAILED)
	return (FAILED);

    /* DC GE RX channels */
    dc_rx_channel = WP_ChannelCreate(0x19, dc_dev, qniw, WP_CH_RX, WP_ENET, &config_channel_enet);
    if (APP_TerminateOnError(dc_rx_channel,"WP_ChannelCreate() dc_rx", __LINE__)
       == FAILED)
       return (FAILED);    

    rx_binding_cfg.input_port = dc_iwport;   
    status = WP_IwRxBindingCreate(dc_rx_channel,iw_sys,qniw,&rx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwRxBindingCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    /* DC GE TX channels */
    dc_tx_channel = WP_ChannelCreate(0x18, dc_dev, qniw, WP_CH_TX, WP_ENET, &config_channel_enet);
    if (APP_TerminateOnError(dc_tx_channel,"WP_ChannelCreate() dc_tx", __LINE__) 
	== FAILED)
	return (FAILED);

    status = WP_IwTxBindingCreate(dc_tx_channel, WP_IW_TX_BINDING, &tx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwTxBindingCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    /* DC GE flow aggs */
    flow_agg_cfg.iw_port = dc_iwport;
    flow_agg_cfg.txfunc = dc_tx_channel;
    flow_agg_cfg.l2_header_insert_mode = WP_IW_L2H_INSERT_DISABLE;
    flow_agg_cfg.prefix_length = 0;

    dc_agg = WP_IwFlowAggregationCreate(WP_WINPATH(0), WP_IW_VLAN_AWARE_BRIDGING_MODE, &flow_agg_cfg);
    if (APP_TerminateOnError(dc_agg, "WP_IwFlowAggregationCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

    config_bridge_port.flow_agg = dc_agg;
    status = WP_IwPortModify(bp_iwport, WP_IW_PORT_MODIFY_FLOW_AGG, &config_bridge_port);
    if (APP_TerminateOnError(status, "WP_IwPortModify", __LINE__)
	== FAILED)
	return (FAILED);

    rx_binding_cfg.input_port = bp_iwport;
    status = WP_IwRxBindingModify(bp_rx_channel, iw_sys, qniw, WP_IW_RX_BIND_MOD_IWPORT,&rx_binding_cfg);
    if (APP_TerminateOnError(status, "WP_IwRxBindingCreate ENET", __LINE__)
	== FAILED)
	return (FAILED);

/*******************  ENABLES   ****************************/
    /* Build IW systems */
    status = WP_IwSystemBuild(iw_sys);
    if (APP_TerminateOnError(status," WP_IwSystemBuild() ", __LINE__ )
	== FAILED)
	return (FAILED);

    if (host_sgmii_enable(bp_enet) == FAILED)
	return (FAILED);

    status = WP_PortEnable(dc_port, WP_DIRECTION_DUPLEX);
    if (APP_TerminateOnError(status,"WP_PortEnable() dc_port", __LINE__) 
	== FAILED)
	return (FAILED);  

    status = WP_DeviceEnable(dc_dev,WP_DIRECTION_DUPLEX);
    if (APP_TerminateOnError(status,"WP_DeviceEnable() dc_dev", __LINE__) 
	== FAILED)
	return (FAILED);  

    status = WP_ChannelEnable(dc_tx_channel);
    if (APP_TerminateOnError(status, "WP_ChannelEnable dc_tx_channel", __LINE__)
	== FAILED)
	return (FAILED);  
    
    status = WP_ChannelEnable(dc_rx_channel);
    if (APP_TerminateOnError(status, "WP_ChannelEnable dc_rx_channel", __LINE__)
	== FAILED)
	return (FAILED);  

    /* save back to global variables */
    if (dc_enet == SGMII_INTERFACE_2) {
	enet_port[1] = dc_port;
	enet_dev[1] = dc_dev;
	enet_iwport[1] = dc_iwport;
	enet_rx_channel[1] = dc_rx_channel;
	enet_tx_channel[1] = dc_tx_channel;
	enet_agg[1] = dc_agg;
    } 

    return (PASSED);
}

static int 
npu_host_send (WP_handle object, WP_handle dev_handle)
{
    WP_data_unit data_unit;
    WP_data_segment segment;
    WP_status status;

    memset(&data_unit, 0, sizeof(WP_data_unit));
    memset(&segment, 0, sizeof(WP_data_segment));
    
    segment.data_size = DATA_LENGTH;
    segment.pool_handle = iw_pool;
    segment.displacement = 0;
    segment.next = NULL;
    segment.data = WP_PoolAlloc(iw_pool);
    memcpy(segment.data,cell,78);
    
    data_unit.type = WP_DATA_IW;
    data_unit.segment = &segment;
    data_unit.n_segments = 1;
    data_unit.n_active = 1;
    data_unit.data_size = segment.data_size;
    data_unit.control = WP_HT_IW_B_MODE_FIELD(WP_HT_IW_B_FAST_MODE)|
	WP_HT_IW_B_VA_DEST_TYPE_FIELD(WP_HT_IW_B_VA_DESTINATION_FLOW_AGG)|
	WP_HT_IW_FSP_FLOW_AGG_FIELD(object) |
	WP_HT_IW_CRC_FIELD(0);

    status = WP_HostSend(dev_handle, &data_unit);
    if (APP_TerminateOnError (status, "WP_HostSend",__LINE__) == FAILED)
	return (FAILED);

    return (PASSED);
};

static int
npu_host_receive (WP_handle dev_handle)
{
    WP_status status;
    WP_data_unit du;
    WP_data_segment seg;
    WP_data_segment* curr_buf;
    WP_U32 j,k;

    du.segment = &seg;
    du.type = WP_DATA_IW;
    du.n_segments = 1;

    status = WP_HostReceive(dev_handle, &du);
    if (APP_TerminateOnError (status, "WP_HostReceive",__LINE__) == FAILED)
	return (FAILED);

    curr_buf = du.segment;
    for (j = 0; j < du.n_active; j++, curr_buf++) {
	if (curr_buf->data_size != DATA_LENGTH) {
	    cterr('f', 0, "Received data length mismatch. Expect: %d, received: %d", DATA_LENGTH, curr_buf->data_size);
	    return (FAILED);
	}

	for (k = 0; k < curr_buf->data_size; k++) {
#ifdef DEBUG
	    printf("%2.2x", curr_buf->data[k]);
#endif
	    if (curr_buf->data[k] != cell[k]) {
		cterr('f', 0, "Received data mismatch. Expect: %d, received: %d", cell[k], curr_buf->data[k]);
		return (FAILED);
	    }    
	}
	WP_PoolFree(curr_buf->pool_handle, curr_buf->data);
#ifdef DEBUG
	printf("(Received %d bytes) \n", curr_buf->data_size);
#endif
    }
    return (PASSED);
}

void 
npu_release_driver ()
{
    /* Release WDDI */
    WP_DriverRelease();
}


int
npu_loopback_test (int bp_enet, int dc_enet, int tdi_num, 
		   npu_lpbk_mode loop_mode, int op_mode)
{
    WP_handle agg;

    if (loop_mode == NPU_BP_ENET_LPBK) {
	if (npu_bp_sgmii_config(bp_enet, TRUE) == FAILED) {
	    cterr('f', 0, "Failed to configure NPU for backplane SGMII loopback");
	    return (FAILED);
	}
	agg = enet_agg[0];
    } else if (loop_mode == NPU_BP_ENET_PASSTHRU) {
	if (npu_bp_sgmii_config(bp_enet, FALSE) == FAILED) {
	    cterr('f', 0, "Failed to configure NPU for backplane SGMII transmission");
	    return (FAILED);
	}
	agg = enet_agg[0];
	/* need to wait for backplane GE switch to detect the  link up. */
	sleep(1);
    } else if (loop_mode == NPU_DC_ENET_LPBK) {
	if (npu_dc_sgmii_config(bp_enet, dc_enet, TRUE) == FAILED) {
	    cterr('f', 0, "Failed to configure NPU for DC SGMII loopback");
	    return (FAILED);
	}
	agg = enet_agg[1];	
    } else if (loop_mode == NPU_TDI_LPBK) {
	if (npu_tdi_config(bp_enet, tdi_num, loop_mode, op_mode) == FAILED) {
	    cterr('f', 0, "Failed to configure NPU for TDI loopback");
	    return (FAILED);
	} 
	agg = tdi_agg[tdi_num];
    } else {
	/* NPU_TDI_PASSTHRU_MASTER or NPU_TDI_PASSTHRU_SLAVE */
	if (npu_tdi_config(bp_enet, tdi_num, loop_mode, op_mode) == FAILED) {
	    cterr('f', 0, "Failed to configure NPU for TDI transmission");
	    return (FAILED);
	} 
	agg = tdi_agg[tdi_num];
    }

    if (npu_host_send(agg, host_tx_channel) == FAILED) {
	cterr('f', 0, "NPU host send data failed");
	return (FAILED);
    }

    /* TDI port is very slow */ 
    sleep(1);

    if (npu_host_receive(host_rx_channel) == FAILED) {
	printf("\nNPU host receive data failed!\n");
	return (FAILED);
    }

    return (PASSED);
}

 
/*********************************************************************
 *
 * Function: npu_bp_sgmii_lpbk_test
 * 
 * Description: GigE Transmit and Recieve using the host
 * This test creates a transparent bridge system and 1 GigE ports and devices.
 * The test uses WP_HostSend to send a packet out a GigE port using an
 * interworking Tx channel is used.
 * No learning is done.  No mac addresses are statically inserted
 * into the forwarding database.  This means that when a packet is
 * received on the GigE ports the packet will always go to the default
 * flow aggregation of the bridge port.
 * The default aggregation on each bridge port (one for each GigE)
 * are set to forward the packets to a host interworking recieve channel.
 * When loopback mode is set each GigE port will recieve its own packets.
 *
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
int 
npu_bp_sgmii_lpbk_test ()
{
    int ret_val = PASSED;

    prpass(testpass, "NPU backplane SGMII lpbk test ");
    ret_val = npu_loopback_test(SGMII_INTERFACE_1, 0, 0, NPU_BP_ENET_LPBK, 0);
    if (ret_val == FAILED) {
	npu_print_dev_statistic(SGMII_INTERFACE_1, 0, 0, NPU_BP_ENET_LPBK);
	npu_release_driver();
	cterr('f', 0, "Failed NPU backplane SGMII lpbk test");
	return (FAILED);
    } else {
	npu_release_driver();
	return (PASSED);
    }
}

/*********************************************************************
 *
 * Function: npu_dc_sgmii_lpbk_test()
 *
 * Description: This is the wrapper function for NPU DC SGMII port lpbk test.
 *              Data will be sent from the NPU to BP SGMII port, and looped at
 *              DC SGMII port through interworking.
 * 
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
int 
npu_dc_sgmii_lpbk_test ()
{
    int ret_val = PASSED;

    prpass(testpass, "NPU daughtercard SGMII lpbk test ");
    ret_val = npu_loopback_test(SGMII_INTERFACE_1, SGMII_INTERFACE_2, 0, 
				NPU_DC_ENET_LPBK, 0);
    if (ret_val == FAILED) {	
	npu_print_dev_statistic(SGMII_INTERFACE_1, SGMII_INTERFACE_2, 0, 
				NPU_DC_ENET_LPBK);
	npu_release_driver();
	cterr('f', 0, "Failed NPU daughtercard SGMII lpbk test");
	return (FAILED);
    } else {
	npu_release_driver();
	return (PASSED);
    }
}

/*********************************************************************
 *
 * Function: npu_tdi_lpbk_test()
 *
 * Description: This is the wrapper function for NPU TDI port lpbk test.
 *              Data will be sent from the NPU to BP SGMII port, and looped at
 *              TDI port through interworking.
 * 
 * Inputs: None
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
int 
npu_tdi_lpbk_test ()
{
    int i, port_max;
    int ret_val = PASSED;

    port_max = get_num_ports();

    for (i = 0; i < port_max; i++) {
	prpass(testpass, "NPU TDI port %d lpbk test ", i);
	ret_val = npu_loopback_test(SGMII_INTERFACE_1, 0, i, 
				    NPU_TDI_LPBK, CMQ_MODE_E1);
	if (ret_val == FAILED) {
	    npu_print_dev_statistic(SGMII_INTERFACE_1, 0, i, 
				    NPU_TDI_LPBK);
	    npu_release_driver();	
	    cterr('f', 0, "Failed NPU TDI port %d lpbk test", i);
	    return (FAILED);
	} else {
	    npu_release_driver();
	}
    }

    return (PASSED);
}


#ifdef DEBUG
int debug_bp_test()
{
    if (npu_host_send(enet_agg[0], host_tx_channel) == FAILED) {
	cterr('f', 0, "NPU host send data failed");
	return (FAILED);
    }

    sleep(1);

    if (npu_host_receive(host_rx_channel) == FAILED) {
	cterr('f', 0, "NPU host receive data failed");
	return (FAILED);
    }

    npu_print_dev_statistic(SGMII_INTERFACE_1, 0, 0, NPU_BP_ENET_LPBK);
}
#endif

int 
config_ngvm_enet ()
{
    if (ngvm_init == FALSE) {
	ngvm_init = TRUE;
	if (npu_dc_sgmii_config(SGMII_INTERFACE_1, SGMII_INTERFACE_2, FALSE) 
	    == FAILED) {
	    cterr('f', 0, "Failed to configure NPU for NGVM communication with host.");
	    return (FAILED);
	}
    }

    return (PASSED);
}

/******** History ********
$Log: fortitude_npu.c,v $
Revision 1.11  2013/04/19 18:33:37  ywen
update NPU and framer setting to work with the timing change in the new FPGA.

Revision 1.10  2013/02/05 21:24:06  ywen
Fix issue when setting NPU NGVM mode multiple times

Revision 1.9  2012/08/21 23:14:53  ywen
code cleanup.

Revision 1.8  2012/06/13 17:54:34  ywen
Add support for TDMSW16 and 2 port SKU.

Revision 1.7  2012/05/14 23:21:10  ywen
Code cleanup and add debug information if test fails.

Revision 1.6  2012/05/09 17:43:26  ywen
Add support to do daughter card DSP FW download through Fortitude NGWIC.

Revision 1.5  2012/04/20 21:22:21  ywen
Code cleanup.

Revision 1.4  2012/04/12 23:17:38  ywen
Fix a bug in IW port create.

Revision 1.3  2012/04/02 21:05:18  ywen
Code cleanup.

Revision 1.2  2012/03/28 00:38:17  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/

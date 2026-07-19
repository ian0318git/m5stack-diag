/* $Id: cpssDxChDiagDataIntegrity.h,v 1.1 2015/02/13 11:31:53 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChDiagDataIntegrity.h
*
* DESCRIPTION:
*       Diag Data Integrity module APIs for CPSS DxCh.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDxChDiagDataIntegrityh
#define __cpssDxChDiagDataIntegrityh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/diag/cpssDiag.h>
#include <cpss/generic/events/cpssGenEventRequests.h>





/*
 * Typedef: enum CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT
 *
 * Description:
 *      This enum defines Data Integrity module Memory types
 * Enumerations:
 * 
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_CONTROL_ACCESS_TABLE_E - 
 *          BM control access table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_LINKED_LIST_BUFFERS_E
 *          BM linked list buffers
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_FINAL_UDB_CLEAR_FIFO_E
 *          BMA final UDB clear FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E
 *          BMA MC clear shifter
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_DIST_FIFO_E - 
 *          BMA MC dist FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E - 
 *          BMA MC counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_UC_DIST_FIFO_E - 
 *          BMA UC dist FIIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E - 
 *          CNC counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CPFC_PFC_IND_FIFO_E - 
 *          CPFC ind FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DBM_E - 
 *          CTU DBM
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DESC_UNUSED_FIFO_E - 
 *          CTU descriptor unused FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_CONF_TABLE_E - 
 *          EPCL configuration table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_DATA_FIFO_E - 
 *          EPCL data FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_EPLR_DESC_FIFO_E - 
 *          EPCL EPLR descriptor FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_TCAM_DESC_FIFO_E - 
 *          EPCL TCAM descriptor FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_BILLING_EGRESS_E - 
 *          EPLR billing egress
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_AGING_E - 
 *          EPLR ipfix aging
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_WRAPAROUND_E - 
 *          EPLR ipfix wraparound
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_EGRESS_E - 
 *          EPLR metering egress
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_POLICER_COUNTERS_E - 
 *          EPLR policer counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_QOS_REMARKING_TABLE_EGRESS_E - 
 *          EPLR qos remarking table egress
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CONF_LIMIT_TABLE_E - 
 *          EQ configuration limit table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CPU_CODE_TABLE_E - 
 *          EQ cpu code table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_QOS_TABLE_E - 
 *          EQ QOS table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_RATE_LIMIT_TABLE_E - 
 *          EQ rate limit table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E - 
 *          EQ STC table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_TRUNK_TABLE_E - 
 *          EQ trunk table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_VPM_E - 
 *          EQ VPM
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_MAC_SA_TABLE_E - 
 *          HA MAC SA table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_TSARP_TUNNEL_TABLE_E - 
 *          HA TSARP tunnel table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_VLAN_TRANSLATION_TABLE_E - 
 *          HA VLAN translation table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_AGING_E - 
 *          IP NHE table aging
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E - 
 *          IP NHE table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E - 
 *          IP STG2 descriptor return two fifo
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E - 
 *          IP STG4 descriptor return two fifo
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E - 
 *          IP unused data fifo
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_VLAN_URPF_TABLE_E - 
 *          IP VLAN URPF table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_PCR_REGISTERS_E - 
 *          L2I PCR registers
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UNUSED_DATA_FIFO_E - 
 *          L2I unused data fifo
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UPDATE_FIFO_E - 
 *          L2I update FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_CONFI_PROCESSOR_E - 
 *          MG confi processor
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_GRD_E - 
 *          MG GRD
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_DESCRIPTOR_FILE_E - 
 *          MG SDMA descriptor file
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_REG_FILE_E - 
 *          MG SDMA REG file
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_RX_FIFO_2_E - 
 *          MG SDMA RX FIFO 2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E - 
 *          MG SDMA TX FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_WRR_E - 
 *          MG SDMA WRR
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E - 
 *          MLL configuration table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E - 
 *          MLL PCL2MLL unused FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E - 
 *          MLL replications in use FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E - 
 *          MLL table FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E - 
 *          MPPM data buffer memory
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E - 
 *          MPPM data buffer memory ecc
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_MAC_TABLE_E - 
 *          MT MAC table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_RF_TOP_MAC_UPD_OUT_E - 
 *          MT RF top mac update out
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E - 
 *          PCL config
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CRC_HASH_MASK_TABLE_E - 
 *          PCL CRC hash mask table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_0_E - 
 *          PCL lookup FIFO 0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_1_E - 
 *          PCL lookup FIFO 1
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_2_E - 
 *          PCL lookup FIFO 2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UNUSED_DATA_FIFO_E - 
 *          PCL unused data FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E - 
 *          PLR billing ingress plr0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E - 
 *          PLR billing ingress plr1
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E - 
 *          PLR descriptor FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E - 
 *          PLR ipfix aging
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E - 
 *          PLR ipfix wraparound
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E - 
 *          PLR metering ingress plr0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E - 
 *          PLR metering ingress plr1
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_POLICER_COUNTERS_E - 
 *          PLR policer counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E - 
 *          PLR QOS remarking table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ACTION_TABLE_LOWER_E - 
 *          TCC lower action table lower
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_DATA_LOWER_E - 
 *          TCC lower ecc table data lower
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_MASK_LOWER_E - 
 *          TCC lower ecc table mask lower
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_LOWER_ANSWER_FIFO_E - 
 *          TCC lower lower answer FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ACTION_TABLE_UPPER_E - 
 *          TCC upper action table upper
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ECC_TABLE_DATA_UPPER_E - 
 *          TCC upper ecc table data upper
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL0_1_E - 
 *          TCC upper upper answer fifo cl0 1
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL2_E - 
 *          TCC_UPPER_UPPER_ANSWER_FIFO_CL0_2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_LOOKUP_FIFO_E - 
 *          TTI lookup FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_PROTOCOL_E - 
 *          TTI port protocol
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_VLAN_QOS_E - 
 *          TTI port vlan qos
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_TXQ_E2E_FC_FIFO_E - 
 *          TTI TXQ E2E FC FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_CFG_E - 
 *          TTI UDB CFG
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_PROFILE_ID_E - 
 *          TTI UDB profile id
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UNUSED_DATA_FIFO_E - 
 *          TTI unused data FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_VLAN_TRANSLATION_E - 
 *          TTI VLAN translation
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_CPU_TX_FIFO_E - 
 *          TXDMA CPU TX FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_DATA_FROM_MPPM_E - 
 *          TXDMA data from MPPM
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E - 
 *          TXDMA HA info descriptor prefetch
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_MEM_CLEAR_FIFO_E - 
 *          TXDMA mem clear FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_NEXT_LIST_BUFFERS_E - 
 *          TXDMA next list buffers
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_READ_BURST_STATE_FIFO_E - 
 *          TXDMA read burst state FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_TX_FIFO_E - 
 *          TXDMA TX FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DIST_BURST_FIFO_E - 
 *          TXQ dist burst FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E - 
 *          TXQ DQ STC
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E - 
 *          TXQ DQ tb giga port
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E - 
 *          TXQ DQ tb giga prio
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E - 
 *          TXQ DQ wrr state variables
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_DESIGNATED_TBL_E - 
 *          TXQ egress designated table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E - 
 *          TXQ egress MC FIFO 0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E - 
 *          TXQ egress MC FIFO 1
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E - 
 *          TXQ egress MC FIFO 2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL2_E - 
 *          TXQ egress non trunk member table 2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL_E - 
 *          TXQ egress non trunk member table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SEC_TRG_TBL_E - 
 *          TXQ egress sec trg table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SST_TBL_E - 
 *          TXQ egress sst table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E - 
 *          TXQ ll free bufs
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E - 
 *          TXQ ll link list
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E - 
 *          TXQ ll qhead
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E - 
 *          TXQ ll qtail
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E - 
 *          TXQ Q buffer FIFO
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_BUF_LIMIT_DP0_E - 
 *          TXQ Q DQ Q buffer limit DP0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_DESC_LIMIT_DP0_E - 
 *          TXQ Q DQ Q descriptor limit DP0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E - 
 *          TXQ Q EQ Q limit DP0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP12_E - 
 *          TXQ Q EQ Q limit DP12
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_0_E - 
 *          TXQ Q PFC ind FIFO pipe 0
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_1_E - 
 *          TXQ Q PFC ind FIFO pipe 1
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_2_E - 
 *          TXQ Q PFC ind FIFO pipe 2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_3_E - 
 *          TXQ Q PFC ind FIFO pipe 3
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_4_E - 
 *          TXQ Q PFC ind FIFO pipe 4
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_5_E - 
 *          TXQ Q PFC ind FIFO pipe 5
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_6_E - 
 *          TXQ Q PFC ind FIFO pipe 6
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_7_E - 
 *          TXQ Q PFC ind FIFO pipe 7
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E - 
 *          TXQ Q PFC source pipe 0 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E - 
 *          TXQ Q PFC source pipe 1 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E - 
 *          TXQ Q PFC source pipe 2 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E - 
 *          TXQ Q PFC source pipe 3 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E - 
 *          TXQ Q PFC source pipe 4 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E - 
 *          TXQ Q PFC source pipe 5 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E - 
 *          TXQ Q PFC source pipe 6 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E - 
 *          TXQ Q PFC source pipe 7 counters
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XOFF_THRES_E - 
 *          TXQ Q PFC XOFF threshold
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XON_THRES_E - 
 *          TXQ Q PFC XON threshold
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SAMPLE_INTERVALS_E - 
 *          TXQ Q sample intervals
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SHARED_Q_LIMIT_E - 
 *          TXQ Q shared Q limit
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_SPT_E - 
 *          TXQ SHT egress STP table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E - 
 *          TXQ SHT egress VLAN table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_SPT_E - 
 *          TXQ SHT ingress STP table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E - 
 *          TXQ SHT ingress VLAN
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_MAP_DEV_E - 
 *          TXQ SHT device map table
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E - 
 *          TXQ SHT port isolation L2
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E - 
 *          TXQ SHT port isolation L3
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E - 
 *          TXQ SHT VIDX
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_WRDMA_IBUF_BANK_RAM_E - 
 *          WRDMA ibuffer bank ram
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E - 
 *          router TCAM
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E - 
 *          policy TCAM
 *
 *   CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E - 
 *          should be last
 *  Comments:
 */

typedef enum
{
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_CONTROL_ACCESS_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_LINKED_LIST_BUFFERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_FINAL_UDB_CLEAR_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_DIST_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_UC_DIST_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CPFC_PFC_IND_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DBM_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DESC_UNUSED_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_CONF_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_DATA_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_EPLR_DESC_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_TCAM_DESC_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_BILLING_EGRESS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_AGING_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_WRAPAROUND_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_EGRESS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_POLICER_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_QOS_REMARKING_TABLE_EGRESS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CONF_LIMIT_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CPU_CODE_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_QOS_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_RATE_LIMIT_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_TRUNK_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_VPM_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_MAC_SA_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_TSARP_TUNNEL_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_VLAN_TRANSLATION_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_AGING_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_VLAN_URPF_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_PCR_REGISTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UNUSED_DATA_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UPDATE_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_CONFI_PROCESSOR_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_GRD_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_DESCRIPTOR_FILE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_REG_FILE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_RX_FIFO_2_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_WRR_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_MAC_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_RF_TOP_MAC_UPD_OUT_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CRC_HASH_MASK_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_1_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_2_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UNUSED_DATA_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_POLICER_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ACTION_TABLE_LOWER_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_DATA_LOWER_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_MASK_LOWER_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_LOWER_ANSWER_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ACTION_TABLE_UPPER_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ECC_TABLE_DATA_UPPER_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL0_1_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL2_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_LOOKUP_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_PROTOCOL_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_VLAN_QOS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_TXQ_E2E_FC_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_CFG_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_PROFILE_ID_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UNUSED_DATA_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_VLAN_TRANSLATION_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_CPU_TX_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_DATA_FROM_MPPM_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_MEM_CLEAR_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_NEXT_LIST_BUFFERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_READ_BURST_STATE_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_TX_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DIST_BURST_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_DESIGNATED_TBL_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL2_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SEC_TRG_TBL_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SST_TBL_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_BUF_LIMIT_DP0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_DESC_LIMIT_DP0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP12_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_0_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_1_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_2_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_3_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_4_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_5_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_6_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_7_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XOFF_THRES_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XON_THRES_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SAMPLE_INTERVALS_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SHARED_Q_LIMIT_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_SPT_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_SPT_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_MAP_DEV_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_WRDMA_IBUF_BANK_RAM_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E,
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E /* should be last */
}CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;



/*
 * Typedef: struct CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC
 *
 * Description: Data Integrity Event structure
 *
 * Fields:
 *      eventsType          - event type
 *      memType             - memory type
 *      causePortGroupId    - port group event has happened 
 *      location            - memory location indexes
 *
 * Comments:
 *      None
 */
typedef struct
{
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   eventsType;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    GT_U32                                          causePortGroupId;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    location;
}CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC;


/*******************************************************************************
* cpssDxChDiagDataIntegrityEventsGet
*
* DESCRIPTION:
*       Function returns array of data integrity events.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum        - PP device number
*       evExtData     - event external data
*       eventsNumPtr  - (pointer to) max num of enabled events to
*                                retrieve - this value refer to the number of
*                                members that the array of eventsTypeArr[] and
*                                memTypeArr[] can retrieve.
*
* OUTPUTS:
*       eventsNumPtr      - (pointer to) the actual num of found events
*       eventsArr         - array of ECC/parity events
*       isNoMoreEventsPtr - (pointer to) status of events scan process
*                              GT_TRUE - no more events found
*                              GT_FALSE - there are more events found
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, portGroup
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityEventsGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityEventMaskSet
*
* DESCRIPTION:
*       Function sets mask/unmask for ECC/Parity event.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*       errorType   - type of error interrupt
*                     relevant only for ECC protected memories 
*       operation   - mask/unmask interrupt
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityEventMaskSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityEventMaskGet
*
* DESCRIPTION:
*       Function gets mask/unmask for ECC/Parity interrupt.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*                     
*
* OUTPUTS:
*       errorTypePtr   - (pointer to) type of error interrupt
*       operationPtr   - (pointer to) mask/unmask interrupt
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityEventMaskGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityErrorInfoGet
*
* DESCRIPTION:
*       Function gets ECC/Parity error info.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum         - PP device number
*       memType        - type of memory(table)
*       locationPtr    - (pointer to) memory location indexes
*
* OUTPUTS:
*       errorCounterPtr    - (pointer to) error counter
*       failedRowPtr       - (pointer to) failed raw
*       failedSyndromePtr  - (pointer to) failed syndrome
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, portGroup, memType
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Error status or counter that isn't supported returns 0xFFFFFFFF
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityErrorInfoGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityErrorInjectionConfigSet
*
* DESCRIPTION:
*       Function enables/disable injection of error during next write operation.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum       - PP device number
*       memType      - type of memory(table)
*       injectMode   - error injection mode
*                      relevant only for ECC protected memories
*       injectEnable - enable/disable error injection
*                       GT_TRUE - enable
*                       GT_FALSE - disable
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, memType, injectMode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Memory with parity can not be configured with multiple error injection
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityErrorInjectionConfigGet
*
* DESCRIPTION:
*       Function gets status of error injection.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum       - PP device number
*       memType      - type of memory(table)
*
* OUTPUTS:
*       injectModePtr   - (pointer to) error injection mode
*       injectEnablePtr - (pointer to) enable/disable error injection
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, memType
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityErrorCountEnableSet
*
* DESCRIPTION:
*       Function enables/disable error counter.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*       errorType   - error type
*       countEnable - enable/disable error counter
*                       GT_TRUE - enable
*                       GT_FALSE - disable
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, memType, errorType
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         - on not suppoted memory type
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityErrorCountEnableGet
*
* DESCRIPTION:
*       Function gets status of error counter.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*
* OUTPUTS:
*       errorTypePtr   - (pointer to) error type
*       countEnablePtr - (pointer to) status of error counter
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT GT_BOOL                                         *countEnablePtr
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityProtectionTypeGet
*
* DESCRIPTION:
*       Function gets memory protection type.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*
* OUTPUTS:
*       protectionTypePtr - (pointer to) memory protection type
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, portGroup, memType
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityProtectionTypeGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT              memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
*
* DESCRIPTION:
*       Function enables/disables TCAM parity daemon.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*                     only Router TCAM and Policy TCAM supported
*       enable      - GT_TRUE - enable daemon
*                     GT_FALSE - disable daemon
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, memType
*       GT_BAD_STATE             - on tcamParityCalcEnable is disabled
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       API is supported only if tcamParityCalcEnable is enabled by cpssDxChPpPhase1Init
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    IN  GT_BOOL                                     enable
);


/*******************************************************************************
* cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet
*
* DESCRIPTION:
*       Function gets status of TCAM parity daemon.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion3.
*
* INPUTS:
*       devNum      - PP device number
*       memType     - type of memory(table)
*                     only Router TCAM and Policy TCAM supported
*
* OUTPUTS:
*       enablePtr   - (pointer to) daemon status
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device, memType
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    OUT GT_BOOL                                     *enablePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChDiagDataIntegrityh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChDiagDataIntegrity.h,v $
 * Revision 1.1  2015/02/13 11:31:53  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

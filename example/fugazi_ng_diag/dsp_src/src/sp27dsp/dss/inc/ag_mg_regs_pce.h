/* $Id: ag_mg_regs_pce.h,v 1.1 2012/04/18 18:08:26 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_mg_regs_pce.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_pce.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_pce.h
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * The Packet Processor Block (PPB) contains two each of these blocks:
 * Packet Classification Engine (PCE), Transmit Direct Memory Access (TXD),
 * and Gigabit Ethernet Media Access Control (MAC). One PCE is attached to
 * the receive side of a MAC and one TXD is connected to the transmit side
 * of a MAC. The MAC transmits/receives packet traffic via SGMII. The PCE
 * receives the packets from the MAC, sorts them and sends them to the
 * appropriate destination via a programmable DMA engine. The TXD grabs data
 * from the source and loads it into the MAC which transmits the data off chip.
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *  PCE_DLT
 *  PCE_CONTROL
 *  PCE_MAC_DEST_LOCAL_MS
 *  PCE_MAC_DEST_LOCAL_LS
 *  PCE_MAC_DEST_REMOTE_MS
 *  PCE_MAC_DEST_REMOTE_LS
 *  PCE_MAC_DEST_23_MS
 *  PCE_MAC_DEST_2_LS
 *  PCE_MAC_DEST_3_LS
 *  PCE_MAC_DEST_45_MS
 *  PCE_MAC_DEST_4_LS
 *  PCE_MAC_DEST_5_LS
 *  PCE_MAX_FRAME_SIZE
 *  PCE_UDP_DEST_MAX_MIN_PORTA
 *  PCE_UDP_DEST_MAX_MIN_PORTB
 *  PCE_IPDEST0_0
 *  PCE_IPDEST0_1
 *  PCE_IPDEST0_2
 *  PCE_IPDEST0_3
 *  PCE_IPDEST1_0
 *  PCE_IPDEST1_1
 *  PCE_IPDEST1_2
 *  PCE_IPDEST1_3
 *  PCE_BDL_CONTROL
 *  PCE_SVT_TOP
 *  PCE_SVT_BOT
 *  PCE_FDQ_CONTROL
 *  PCE_FOQ_CONTROL
 *  PCE_EXINT_CONTROL
 *  PCE_FINT_CONTROL
 *  PCE_INTERRUPT_STATUS_MASK_0
 *  PCE_INTERRUPT_STATUS_MASK_1
 *  PCE_MIU_CONTROL
 *  PCE_DLT_ACCESS
 *  PCE_BD_PTR_RST
 *  PCE_REGKEY
 *  PCE_INTERRUPT_SUMMARY
 *  PCE_INTERRUPT_STATUS_0
 *  PCE_INTERRUPT_STATUS_1
 *  PCE_CFG_STATUS
 *  PCE_PAR_STATUS
 *  PCE_DLU_STATUS
 *  PCE_SLU_STATUS
 *  PCE_FWR_STATUS_LO
 *  PCE_MIU_STATUS
 *  PCE_L2C_STATUS_REG
 *  PCE_FWR_STATUS_HI
 *  PCE_SOF_COUNTER
 *  PCE_EOF_COUNTER
 *  PCE_SOF_SOF_COUNTER
 *  PCE_EOF_EOF_COUNTER
 *  PCE_RX_BYTE_COUNTER
 *  PCE_DLU_COUNTER
 *  PCE_DLT_READ_ERR_COUNTER
 *  PCE_DLU_FAIL_COUNTER
 *  PCE_DLU_OOR_COUNTER
 *  PCE_DLU_SVT_OOR_COUNTER
 *  PCE_SLU_COUNTER
 *  PCE_SLU_FAIL_COUNTER
 *  PCE_IPV4_CHKSUM_FAIL_CNT
 *  PCE_UDP_CHKSUM_FAIL_CNT
 *  PCE_UDP_CHKSUM_MASK_CNT
 *  PCE_UNKNOWN_FRAME_CNT
 *  PCE_PARSER_FLUSH_COUNTER
 *  PCE_UDL_COUNTER
 *  PCE_V2_COUNTER
 *  PCE_VLAN_COUNTER
 *  PCE_SNAP_COUNTER
 *  PCE_IPV4_COUNTER
 *  PCE_IPV6_COUNTER
 *  PCE_UDP_COUNTER
 *  PCE_UDPLITE_COUNTER
 *  PCE_FDQFULL_DISCARD_CNT
 *  PCE_CORRUPTED_FRAME_CNT
 *  PCE_FOQFULL_DISCARD_CNT
 *  PCE_MAX_SVT_STALL_COUNTER
 *  PCE_MAX_BDT_STALL_COUNTER
 *  PCE_PAUSE_FRAME_COUNTER
 *  PCE_IPV4_LEN_ERR_COUNTER
 *  PCE_IPV6_LEN_ERR_COUNTER
 *  PCE_IPV4UDP_LEN_ERR_CNT
 *  PCE_IPV6UDP_LEN_ERR_CNT
 *  PCE_IPV4UDPLITE_COV_ERR_CNT
 *  PCE_IPV6UDPLITE_COV_ERR_CNT
 *  PCE_BDL_EMPTY_DISCARD_CNT
 *  PCE_BD_BOT
 *  PCE_BD_TOP
 *  PCE_BUF_SENT_COUNTER
 *  PCE_FRAME_SENT_CNT
 *  PCE_MAX_BDL_LATENCY
 *  PCE_BDF_STATUS
 *  PCE_BD_INT_STATUS
 *  PCE_BD_INT_MASK
 *  PCE_MMR_MATCH
 *  PCE_MMR_MATCH_1
 *  PCE_MMR_MATCH_2
 *  PCE_MMR_MATCH_3
 *  PCE_MMR_MATCH_4
 *  PCE_MMR_MATCH_5
 *  PCE_MMR_MATCH_6
 *  PCE_MMR_MATCH_7
 *  PCE_MMR_MASK
 *  PCE_MMR_MASK_1
 *  PCE_MMR_MASK_2
 *  PCE_MMR_MASK_3
 *  PCE_MMR_MASK_4
 *  PCE_MMR_MASK_5
 *  PCE_MMR_MASK_6
 *  PCE_MMR_MASK_7
 *  PCE_UPR_INDEX_ADJUST
 *  PCE_UPR_PATTERN_CONTROL
 *  PCE_ULT_ENTRY
 *
 * Prefix naming conventions :
 *   AG_MG_REGS_XXX_RA : register/memory physical address
 *   AG_MG_REGS_XXX_RO : register/memory address offset
 *   AG_MG_REGS_XXX_RM : register mask
 *   AG_MG_REGS_XXX_BO : bit/field offset from LSB
 *   AG_MG_REGS_XXX_BM : bit/field mask
 *   AG_MG_REGS_XXX_U  : bitfields in C union typedef
 *   AG_MG_REGS_XXX_S  : registers in C struct typedef
 *   AG_MG_REGS_XX_RPT : number of identical registers in array
 *   AG_MG_REGS_XX_IVL : interval between registers in array
 *
 * NOTE: user may redefine ag_mg_regs_register
 *       in ag_mg_regs_regops.h if necessary
 * NOTE: access mode of individual bit fields matches that
 *       of containing register unless indicated otherwise
 */

#ifndef AG_MG_REGS_PCE_REGISTERS_H
#define AG_MG_REGS_PCE_REGISTERS_H

#include "ag_mg_regs_regops.h"
/*
 * Generated by HSI Designer release 2.3.5.
 */

/*
 * PCE_DLT (PCE Destination Lookup Table Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_DLT_RO                                                  0x00000000
#define AG_MG_REGS_PCE_DLT_RM                                                  0x81FFFFFF
#define AG_MG_REGS_PCE_DLT_RPT                                                 2048
#define AG_MG_REGS_PCE_DLT_IVL                                                 0x4

#define AG_MG_REGS_PCE_DLT_SVT_OFFSET_BO                                       0
#define AG_MG_REGS_PCE_DLT_SVT_OFFSET_BM                                       0x00007FFF

#define AG_MG_REGS_PCE_DLT_CHECK_SOURCE_BO                                     15
#define AG_MG_REGS_PCE_DLT_CHECK_SOURCE_BM                                     0x00018000

#define AG_MG_REGS_PCE_DLT_CHECK_IP_DEST_BO                                    17
#define AG_MG_REGS_PCE_DLT_CHECK_IP_DEST_BM                                    0x00060000

#define AG_MG_REGS_PCE_DLT_STRIP_L2_HEADER_BO                                  19
#define AG_MG_REGS_PCE_DLT_STRIP_L2_HEADER_BM                                  0x00080000

#define AG_MG_REGS_PCE_DLT_DEST_CONN_VALID_BO                                  20
#define AG_MG_REGS_PCE_DLT_DEST_CONN_VALID_BM                                  0x00100000

#define AG_MG_REGS_PCE_DLT_DESTINATION_QUEUE_BO                                21
#define AG_MG_REGS_PCE_DLT_DESTINATION_QUEUE_BM                                0x01E00000

#define AG_MG_REGS_PCE_DLT_DLTAB_BO                                            31
#define AG_MG_REGS_PCE_DLT_DLTAB_BM                                            0x80000000
#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLT_U
{
    struct
    {
        ag_mg_regs_register
            svt_offset : 15,
            check_source : 2,
            check_ip_dest : 2,
            strip_l2_header : 1,
            dest_conn_valid : 1,
            destination_queue : 4,
            fill0 : 6,
            dltab : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlt_u;
#endif


/*
 * PCE_CONTROL (PCE Control Register)
 * Initialization value: 0x00080000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_CONTROL_RO                                              0x00002000
#define AG_MG_REGS_PCE_CONTROL_RM                                              0x01FFFFFF

#define AG_MG_REGS_PCE_CONTROL_RESETPCE_BO                                     0
#define AG_MG_REGS_PCE_CONTROL_RESETPCE_BM                                     0x00000001

#define AG_MG_REGS_PCE_CONTROL_ENABLEPCEINTERFACE_BO                           1
#define AG_MG_REGS_PCE_CONTROL_ENABLEPCEINTERFACE_BM                           0x00000002

#define AG_MG_REGS_PCE_CONTROL_ENABLEPCECLASSIFICATION_BO                      2
#define AG_MG_REGS_PCE_CONTROL_ENABLEPCECLASSIFICATION_BM                      0x00000004

#define AG_MG_REGS_PCE_CONTROL_ENABLEPAUSEFRAMES_BO                            3
#define AG_MG_REGS_PCE_CONTROL_ENABLEPAUSEFRAMES_BM                            0x00000008

#define AG_MG_REGS_PCE_CONTROL_IP_DES1_IS_V6_BO                                4
#define AG_MG_REGS_PCE_CONTROL_IP_DES1_IS_V6_BM                                0x00000010

#define AG_MG_REGS_PCE_CONTROL_IP_DES0_IS_V6_BO                                5
#define AG_MG_REGS_PCE_CONTROL_IP_DES0_IS_V6_BM                                0x00000020

#define AG_MG_REGS_PCE_CONTROL_SVT_IPV4_BO                                     6
#define AG_MG_REGS_PCE_CONTROL_SVT_IPV4_BM                                     0x00000040

#define AG_MG_REGS_PCE_CONTROL_DISABLE_COUNTS_BO                               7
#define AG_MG_REGS_PCE_CONTROL_DISABLE_COUNTS_BM                               0x00000080

#define AG_MG_REGS_PCE_CONTROL_DLT_INIT_BO                                     8
#define AG_MG_REGS_PCE_CONTROL_DLT_INIT_BM                                     0x00000100

#define AG_MG_REGS_PCE_CONTROL_PCE_FORWARD_ALWAYS_BO                           9
#define AG_MG_REGS_PCE_CONTROL_PCE_FORWARD_ALWAYS_BM                           0x00000200

#define AG_MG_REGS_PCE_CONTROL_PCE_FORWARD_ENABLE_BO                           10
#define AG_MG_REGS_PCE_CONTROL_PCE_FORWARD_ENABLE_BM                           0x00000400

#define AG_MG_REGS_PCE_CONTROL_ENABLE_UDL_BO                                   11
#define AG_MG_REGS_PCE_CONTROL_ENABLE_UDL_BM                                   0x00000800

#define AG_MG_REGS_PCE_CONTROL_RECEIVEFRAMEOFFSET_BO                           12
#define AG_MG_REGS_PCE_CONTROL_RECEIVEFRAMEOFFSET_BM                           0x00003000

#define AG_MG_REGS_PCE_CONTROL_BLOCKDSSFRAMEINTSVTMISS_BO                      14
#define AG_MG_REGS_PCE_CONTROL_BLOCKDSSFRAMEINTSVTMISS_BM                      0x00004000

#define AG_MG_REGS_PCE_CONTROL_BACKPRESSURE_MAC_BO                             15
#define AG_MG_REGS_PCE_CONTROL_BACKPRESSURE_MAC_BM                             0x00008000

#define AG_MG_REGS_PCE_CONTROL_FDTX_WAIT_BO                                    16
#define AG_MG_REGS_PCE_CONTROL_FDTX_WAIT_BM                                    0x00010000

#define AG_MG_REGS_PCE_CONTROL_SVT_FETCH_COUNT_BO                              17
#define AG_MG_REGS_PCE_CONTROL_SVT_FETCH_COUNT_BM                              0x00060000

#define AG_MG_REGS_PCE_CONTROL_UDP_CKSUM_MODE_BO                               19
#define AG_MG_REGS_PCE_CONTROL_UDP_CKSUM_MODE_BM                               0x00080000

#define AG_MG_REGS_PCE_CONTROL_PCE_FORWARD_MODE_BO                             20
#define AG_MG_REGS_PCE_CONTROL_PCE_FORWARD_MODE_BM                             0x00100000

#define AG_MG_REGS_PCE_CONTROL_USE_REMOTE_AS_LOCAL_BO                          21
#define AG_MG_REGS_PCE_CONTROL_USE_REMOTE_AS_LOCAL_BM                          0x00200000

#define AG_MG_REGS_PCE_CONTROL_DISABLE_CNT_COR_BO                              22
#define AG_MG_REGS_PCE_CONTROL_DISABLE_CNT_COR_BM                              0x00400000

#define AG_MG_REGS_PCE_CONTROL_MAC_ADDR_ROUTING_BO                             23
#define AG_MG_REGS_PCE_CONTROL_MAC_ADDR_ROUTING_BM                             0x00800000

#define AG_MG_REGS_PCE_CONTROL_DLTB_ENABLE_BO                                  24
#define AG_MG_REGS_PCE_CONTROL_DLTB_ENABLE_BM                                  0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            resetpce : 1,
            enablepceinterface : 1,
            enablepceclassification : 1,
            enablepauseframes : 1,
            ip_des1_is_v6 : 1,
            ip_des0_is_v6 : 1,
            svt_ipv4 : 1,
            disable_counts : 1,
            dlt_init : 1,
            forward_always : 1,
            forward_enable : 1,
            enable_udl : 1,
            receiveframeoffset : 2,
            blockdssframeintsvtmiss : 1,
            backpressure_mac : 1,
            fdtx_wait : 1,
            svt_fetch_count : 2,
            udp_cksum_mode : 1,
            forward_mode : 1,
            use_remote_as_local : 1,
            disable_cnt_cor : 1,
            mac_addr_routing : 1,
            dltb_enable : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_control_u;
#endif


/*
 * PCE_MAC_DEST_LOCAL_MS (PCE MAC Destination Address Local MS Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_RO                                    0x00002004
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_RM                                    0x0000FFFF

#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_MAC_DEST_LOCAL_MS_BO                  0
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_MAC_DEST_LOCAL_MS_BM                  0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_local_ms : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_local_ms_u;
#endif


/*
 * PCE_MAC_DEST_LOCAL_LS (PCE MAC Destination Address Local LS Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_RO                                    0x00002008
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_RM                                    0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_MAC_DEST_LOCAL_LS_BO                  0
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_MAC_DEST_LOCAL_LS_BM                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_local_ls;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_local_ls_u;
#endif


/*
 * PCE_MAC_DEST_REMOTE_MS (PCE MAC Destination Address Remote MS Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_RO                                   0x0000200C
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_RM                                   0x0000FFFF

#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_MAC_DEST_REMOTE_MS_BO                0
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_MAC_DEST_REMOTE_MS_BM                0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_remote_ms : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_remote_ms_u;
#endif


/*
 * PCE_MAC_DEST_REMOTE_LS (PCE MAC Destination Address Remote LS Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_RO                                   0x00002010
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_RM                                   0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_MAC_DEST_REMOTE_LS_BO                0
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_MAC_DEST_REMOTE_LS_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_remote_ls;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_remote_ls_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_23_MS_RO                                       0x00002014
#define AG_MG_REGS_PCE_MAC_DEST_23_MS_RM                                       0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_23_MS_MAC_DEST_2_MS_BO                         0
#define AG_MG_REGS_PCE_MAC_DEST_23_MS_MAC_DEST_2_MS_BM                         0x0000FFFF

#define AG_MG_REGS_PCE_MAC_DEST_23_MS_MAC_DEST_3_MS_BO                         16
#define AG_MG_REGS_PCE_MAC_DEST_23_MS_MAC_DEST_3_MS_BM                         0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_23_MS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_2_ms : 16,
            mac_dest_3_ms : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_23_ms_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_2_LS_RO                                        0x00002018
#define AG_MG_REGS_PCE_MAC_DEST_2_LS_RM                                        0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_2_LS_MAC_DEST_2_LS_BO                          0
#define AG_MG_REGS_PCE_MAC_DEST_2_LS_MAC_DEST_2_LS_BM                          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_2_LS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_2_ls;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_2_ls_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_3_LS_RO                                        0x0000201C
#define AG_MG_REGS_PCE_MAC_DEST_3_LS_RM                                        0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_3_LS_MAC_DEST_3_LS_BO                          0
#define AG_MG_REGS_PCE_MAC_DEST_3_LS_MAC_DEST_3_LS_BM                          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_3_LS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_3_ls;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_3_ls_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_45_MS_RO                                       0x00002020
#define AG_MG_REGS_PCE_MAC_DEST_45_MS_RM                                       0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_45_MS_MAC_DEST_4_MS_BO                         0
#define AG_MG_REGS_PCE_MAC_DEST_45_MS_MAC_DEST_4_MS_BM                         0x0000FFFF

#define AG_MG_REGS_PCE_MAC_DEST_45_MS_MAC_DEST_5_MS_BO                         16
#define AG_MG_REGS_PCE_MAC_DEST_45_MS_MAC_DEST_5_MS_BM                         0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_45_MS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_4_ms : 16,
            mac_dest_5_ms : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_45_ms_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_4_LS_RO                                        0x00002024
#define AG_MG_REGS_PCE_MAC_DEST_4_LS_RM                                        0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_4_LS_MAC_DEST_4_LS_BO                          0
#define AG_MG_REGS_PCE_MAC_DEST_4_LS_MAC_DEST_4_LS_BM                          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_4_LS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_4_ls;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_4_ls_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAC_DEST_5_LS_RO                                        0x00002028
#define AG_MG_REGS_PCE_MAC_DEST_5_LS_RM                                        0xFFFFFFFF

#define AG_MG_REGS_PCE_MAC_DEST_5_LS_MAC_DEST_5_LS_BO                          0
#define AG_MG_REGS_PCE_MAC_DEST_5_LS_MAC_DEST_5_LS_BM                          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAC_DEST_5_LS_U
{
    struct
    {
        ag_mg_regs_register
            mac_dest_5_ls;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mac_dest_5_ls_u;
#endif


/*
 * PCE_MAX_FRAME_SIZE (PCE Maximum Frame Size Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MAX_FRAME_SIZE_RO                                       0x0000202C
#define AG_MG_REGS_PCE_MAX_FRAME_SIZE_RM                                       0x0000FFFF

#define AG_MG_REGS_PCE_MAX_FRAME_SIZE_MAX_FRAME_SIZE_BO                        0
#define AG_MG_REGS_PCE_MAX_FRAME_SIZE_MAX_FRAME_SIZE_BM                        0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAX_FRAME_SIZE_U
{
    struct
    {
        ag_mg_regs_register
            max_frame_size : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_max_frame_size_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_RO                               0x00002030
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_RM                               0xFFFFFFFF

#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_UDP_DEST_MIN_PORTA_BO            0
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_UDP_DEST_MIN_PORTA_BM            0x0000FFFF

#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_UDP_DEST_MAX_PORTA_BO            16
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_UDP_DEST_MAX_PORTA_BM            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_U
{
    struct
    {
        ag_mg_regs_register
            udp_dest_min_porta : 16,
            udp_dest_max_porta : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udp_dest_max_min_porta_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_RO                               0x00002034
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_RM                               0xFFFFFFFF

#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_UDP_DEST_MIN_PORTB_BO            0
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_UDP_DEST_MIN_PORTB_BM            0x0000FFFF

#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_UDP_DEST_MAX_PORTB_BO            16
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_UDP_DEST_MAX_PORTB_BM            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_U
{
    struct
    {
        ag_mg_regs_register
            udp_dest_min_portb : 16,
            udp_dest_max_portb : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udp_dest_max_min_portb_u;
#endif


/*
 * PCE_IPDEST0_0 (PCE Destination IP Address 0 Register 0)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST0_0_RO                                            0x0000203C
#define AG_MG_REGS_PCE_IPDEST0_0_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST0_0_IPDEST0_0_BO                                  0
#define AG_MG_REGS_PCE_IPDEST0_0_IPDEST0_0_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST0_0_U
{
    struct
    {
        ag_mg_regs_register
            ipdest0_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest0_0_u;
#endif


/*
 * PCE_IPDEST0_1 (PCE Destination IP Address 0 Register 1)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST0_1_RO                                            0x00002040
#define AG_MG_REGS_PCE_IPDEST0_1_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST0_1_IPDEST0_1_BO                                  0
#define AG_MG_REGS_PCE_IPDEST0_1_IPDEST0_1_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST0_1_U
{
    struct
    {
        ag_mg_regs_register
            ipdest0_1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest0_1_u;
#endif


/*
 * PCE_IPDEST0_2 (PCE Destination IP Address 0 Register 2)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST0_2_RO                                            0x00002044
#define AG_MG_REGS_PCE_IPDEST0_2_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST0_2_IPDEST0_2_BO                                  0
#define AG_MG_REGS_PCE_IPDEST0_2_IPDEST0_2_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST0_2_U
{
    struct
    {
        ag_mg_regs_register
            ipdest0_2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest0_2_u;
#endif


/*
 * PCE_IPDEST0_3 (PCE Destination IP Address 0 Register 3)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST0_3_RO                                            0x00002048
#define AG_MG_REGS_PCE_IPDEST0_3_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST0_3_IPDEST0_3_BO                                  0
#define AG_MG_REGS_PCE_IPDEST0_3_IPDEST0_3_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST0_3_U
{
    struct
    {
        ag_mg_regs_register
            ipdest0_3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest0_3_u;
#endif


/*
 * PCE_IPDEST1_0 (PCE Destination IP Address 1 Register 0)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST1_0_RO                                            0x0000204C
#define AG_MG_REGS_PCE_IPDEST1_0_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST1_0_IPDEST1_0_BO                                  0
#define AG_MG_REGS_PCE_IPDEST1_0_IPDEST1_0_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST1_0_U
{
    struct
    {
        ag_mg_regs_register
            ipdest1_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest1_0_u;
#endif


/*
 * PCE_IPDEST1_1 (PCE Destination IP Address 1 Register 1)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST1_1_RO                                            0x00002050
#define AG_MG_REGS_PCE_IPDEST1_1_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST1_1_IPDEST1_1_BO                                  0
#define AG_MG_REGS_PCE_IPDEST1_1_IPDEST1_1_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST1_1_U
{
    struct
    {
        ag_mg_regs_register
            ipdest1_1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest1_1_u;
#endif


/*
 * PCE_IPDEST1_2 (PCE Destination IP Address 1 Register 2)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST1_2_RO                                            0x00002054
#define AG_MG_REGS_PCE_IPDEST1_2_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST1_2_IPDEST1_2_BO                                  0
#define AG_MG_REGS_PCE_IPDEST1_2_IPDEST1_2_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST1_2_U
{
    struct
    {
        ag_mg_regs_register
            ipdest1_2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest1_2_u;
#endif


/*
 * PCE_IPDEST1_3 (PCE Destination IP Address 1 Register 3)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_IPDEST1_3_RO                                            0x00002058
#define AG_MG_REGS_PCE_IPDEST1_3_RM                                            0xFFFFFFFF

#define AG_MG_REGS_PCE_IPDEST1_3_IPDEST1_3_BO                                  0
#define AG_MG_REGS_PCE_IPDEST1_3_IPDEST1_3_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPDEST1_3_U
{
    struct
    {
        ag_mg_regs_register
            ipdest1_3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipdest1_3_u;
#endif


/*
 * PCE_BDL_CONTROL (PCE Buffer Descriptor List Control Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_BDL_CONTROL_RO                                          0x0000205C
#define AG_MG_REGS_PCE_BDL_CONTROL_RM                                          0x001FF033

#define AG_MG_REGS_PCE_BDL_CONTROL_PREFETCH_AMOUNT_BO                          0
#define AG_MG_REGS_PCE_BDL_CONTROL_PREFETCH_AMOUNT_BM                          0x00000003

#define AG_MG_REGS_PCE_BDL_CONTROL_PREFETCH_TRESH_BO                           4
#define AG_MG_REGS_PCE_BDL_CONTROL_PREFETCH_TRESH_BM                           0x00000030

#define AG_MG_REGS_PCE_BDL_CONTROL_REFETCH_DISCARD_THRESH_BO                   12
#define AG_MG_REGS_PCE_BDL_CONTROL_REFETCH_DISCARD_THRESH_BM                   0x000FF000

#define AG_MG_REGS_PCE_BDL_CONTROL_REFETCH_FOREVER_BO                          20
#define AG_MG_REGS_PCE_BDL_CONTROL_REFETCH_FOREVER_BM                          0x00100000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BDL_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            prefetch_amount : 2,
            fill2 : 2,
            prefetch_tresh : 2,
            fill1 : 6,
            refetch_discard_thresh : 8,
            refetch_forever : 1,
            fill0 : 11;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bdl_control_u;
#endif


/*
 * PCE_SVT_TOP (PCE Software Verification Table TOP Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_SVT_TOP_RO                                              0x00002060
#define AG_MG_REGS_PCE_SVT_TOP_RM                                              0xFFFFFFF8

#define AG_MG_REGS_PCE_SVT_TOP_SVT_TOP_BO                                      3
#define AG_MG_REGS_PCE_SVT_TOP_SVT_TOP_BM                                      0xFFFFFFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SVT_TOP_U
{
    struct
    {
        ag_mg_regs_register
            fill0 : 3,
            svt_top : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_svt_top_u;
#endif


/*
 * PCE_SVT_BOT (PCE Software Verification Table BOT Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_SVT_BOT_RO                                              0x00002064
#define AG_MG_REGS_PCE_SVT_BOT_RM                                              0xFFFFFFF8

#define AG_MG_REGS_PCE_SVT_BOT_SVT_BOT_BO                                      3
#define AG_MG_REGS_PCE_SVT_BOT_SVT_BOT_BM                                      0xFFFFFFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SVT_BOT_U
{
    struct
    {
        ag_mg_regs_register
            fill0 : 3,
            svt_bot : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_svt_bot_u;
#endif


/*
 * PCE_FDQ_CONTROL (PCE Frame Data Queue Control Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_FDQ_CONTROL_RO                                          0x00002068
#define AG_MG_REGS_PCE_FDQ_CONTROL_RM                                          0x001FF1FF

#define AG_MG_REGS_PCE_FDQ_CONTROL_FDQ_RESUME_THRESH_BO                        0
#define AG_MG_REGS_PCE_FDQ_CONTROL_FDQ_RESUME_THRESH_BM                        0x000001FF

#define AG_MG_REGS_PCE_FDQ_CONTROL_FDQ_FULL_THRESH_BO                          12
#define AG_MG_REGS_PCE_FDQ_CONTROL_FDQ_FULL_THRESH_BM                          0x001FF000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FDQ_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            fdq_resume_thresh : 9,
            fill1 : 3,
            fdq_full_thresh : 9,
            fill0 : 11;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_fdq_control_u;
#endif


/*
 * PCE_FOQ_CONTROL (PCE Frame Operation Queue Control Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_FOQ_CONTROL_RO                                          0x0000206C
#define AG_MG_REGS_PCE_FOQ_CONTROL_RM                                          0x0003F03F

#define AG_MG_REGS_PCE_FOQ_CONTROL_FOQ_RESUME_THRESH_BO                        0
#define AG_MG_REGS_PCE_FOQ_CONTROL_FOQ_RESUME_THRESH_BM                        0x0000003F

#define AG_MG_REGS_PCE_FOQ_CONTROL_FOQ_FULL_THRESH_BO                          12
#define AG_MG_REGS_PCE_FOQ_CONTROL_FOQ_FULL_THRESH_BM                          0x0003F000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FOQ_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            foq_resume_thresh : 6,
            fill1 : 6,
            foq_full_thresh : 6,
            fill0 : 14;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_foq_control_u;
#endif


/*
 * Initialization value: 0x40000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_EXINT_CONTROL_RO                                        0x00002070
#define AG_MG_REGS_PCE_EXINT_CONTROL_RM                                        0x000FFFFE

#define AG_MG_REGS_PCE_EXINT_CONTROL_ARM_EXCEPTIONS_BO                         1
#define AG_MG_REGS_PCE_EXINT_CONTROL_ARM_EXCEPTIONS_BM                         0x00000002

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKEV2_BO                                 2
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKEV2_BM                                 0x00000004

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKEV2WVLAN_BO                            3
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKEV2WVLAN_BM                            0x00000008

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKSNAP_BO                                4
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKSNAP_BM                                0x00000010

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKSNAPWVLAN_BO                           5
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKSNAPWVLAN_BM                           0x00000020

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKIPV4_BO                                6
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKIPV4_BM                                0x00000040

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKIPV6_BO                                7
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKIPV6_BM                                0x00000080

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKUDP_BO                                 8
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKUDP_BM                                 0x00000100

#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKUDPLITE_BO                             9
#define AG_MG_REGS_PCE_EXINT_CONTROL_BLKUDPLITE_BM                             0x00000200

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_IHL_CHK_BO                  10
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_IHL_CHK_BM                  0x00000400

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_ID_CHK_BO                   11
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_ID_CHK_BM                   0x00000800

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_FRAG_CHK_BO                 12
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_FRAG_CHK_BM                 0x00001000

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_TTL_CHK_BO                  13
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_TTL_CHK_BM                  0x00002000

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_LEN_CHK_BO                  14
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_LEN_CHK_BM                  0x00004000

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV6_LEN_CHK_BO                  15
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV6_LEN_CHK_BM                  0x00008000

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEUDPV4_LEN_CHK_BO               16
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEUDPV4_LEN_CHK_BM               0x00010000

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEUDPV6_LEN_CHK_BO               17
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEUDPV6_LEN_CHK_BM               0x00020000

#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_HEADERCKSUM_ERR_BO          18
#define AG_MG_REGS_PCE_EXINT_CONTROL_PCE_DISABLEV4_HEADERCKSUM_ERR_BM          0x00040000

#define AG_MG_REGS_PCE_EXINT_CONTROL_L4_DISABLEUDP_CHECKSUM_ERR_BO             19
#define AG_MG_REGS_PCE_EXINT_CONTROL_L4_DISABLEUDP_CHECKSUM_ERR_BM             0x00080000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_EXINT_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 1,
            arm_exceptions : 1,
            blkev2 : 1,
            blkev2wvlan : 1,
            blksnap : 1,
            blksnapwvlan : 1,
            blkipv4 : 1,
            blkipv6 : 1,
            blkudp : 1,
            blkudplite : 1,
            disablev4_ihl_chk : 1,
            disablev4_id_chk : 1,
            disablev4_frag_chk : 1,
            disablev4_ttl_chk : 1,
            disablev4_len_chk : 1,
            disablev6_len_chk : 1,
            disableudpv4_len_chk : 1,
            disableudpv6_len_chk : 1,
            disablev4_headercksum_err : 1,
            l4_disableudp_checksum_err : 1,
            fill0 : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_exint_control_u;
#endif


/*
 * Initialization value: 0x40000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_FINT_CONTROL_RO                                         0x00002074
#define AG_MG_REGS_PCE_FINT_CONTROL_RM                                         0xFFFF003F

#define AG_MG_REGS_PCE_FINT_CONTROL_MASK_FRAME_COMPLETE_BO                     0
#define AG_MG_REGS_PCE_FINT_CONTROL_MASK_FRAME_COMPLETE_BM                     0x0000003F

#define AG_MG_REGS_PCE_FINT_CONTROL_MIN_FRM_BTWEEN_FRAME_CMPLT_BO              16
#define AG_MG_REGS_PCE_FINT_CONTROL_MIN_FRM_BTWEEN_FRAME_CMPLT_BM              0x00FF0000

#define AG_MG_REGS_PCE_FINT_CONTROL_MIN_CLKS_BTWEEN_FRAME_CMPLT_BO             24
#define AG_MG_REGS_PCE_FINT_CONTROL_MIN_CLKS_BTWEEN_FRAME_CMPLT_BM             0xFF000000

#define AG_MG_REGS_PCE_RX_FAIL_QUEUE	0
#define AG_MG_REGS_PCE_RX_PPB_QUEUE		1
#define AG_MG_REGS_PCE_RX_DSS0_QUEUE	2
#define AG_MG_REGS_PCE_RX_DSS1_QUEUE	3
#define AG_MG_REGS_PCE_RX_DSS2_QUEUE	4
#define AG_MG_REGS_PCE_RX_DSS3_QUEUE	5

typedef enum ag_mg_regs_pce_rx_queue {
	RX_FAIL_QUE	= AG_MG_REGS_PCE_RX_FAIL_QUEUE,
	RX_PPB_QUE	= AG_MG_REGS_PCE_RX_PPB_QUEUE,
	RX_DSS0_QUE	= AG_MG_REGS_PCE_RX_DSS0_QUEUE,
	RX_DSS1_QUE	= AG_MG_REGS_PCE_RX_DSS1_QUEUE,
	RX_DSS2_QUE	= AG_MG_REGS_PCE_RX_DSS2_QUEUE,
	RX_DSS3_QUE	= AG_MG_REGS_PCE_RX_DSS3_QUEUE
} AG_MG_REGS_PCE_RX_QUEUE;

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FINT_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            mask_frame_complete : 6,
            fill0 : 10,
            min_frm_btween_frame_cmplt : 8,
            min_clks_btween_frame_cmplt : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_fint_control_u;
#endif


/*
 * PCE_INTERRUPT_STATUS_MASK_0 (PCE Interrupt Status Mask Register 0)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_RO                              0x00002078
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_RM                              0xFFFFFFFF

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_MASK_STATUS_0_BO                0
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_MASK_STATUS_0_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_U
{
    struct
    {
        ag_mg_regs_register
            mask_status_0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_interrupt_status_mask_0_u;
#endif


/*
 * PCE_INTERRUPT_STATUS_MASK_1 (PCE Interrupt Status Mask Register 1)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_RO                              0x0000207C
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_RM                              0xFFFFFFFF

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_MASK_STATUS_1_BO                0
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_MASK_STATUS_1_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_U
{
    struct
    {
        ag_mg_regs_register
            mask_status_1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_interrupt_status_mask_1_u;
#endif


/*
 * PCE_MIU_CONTROL (PCE MIU Control Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MIU_CONTROL_RO                                          0x00002080
#define AG_MG_REGS_PCE_MIU_CONTROL_RM                                          0x87FFFFFF

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_FWDQ_START_WM_BO                        0
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_FWDQ_START_WM_BM                        0x0000003F

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_FWDQ_STOP_WM_BO                         6
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_FWDQ_STOP_WM_BM                         0x00000FC0

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_BDWDQ_STOP_WM_BO                        12
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_BDWDQ_STOP_WM_BM                        0x00003000

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_FWCQ_STOP_WM_BO                         14
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_FWCQ_STOP_WM_BM                         0x0000C000

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_BDWCQ_STOP_WM_BO                        16
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_BDWCQ_STOP_WM_BM                        0x00030000

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_ACKCNT_BP_THRESHOLD_BO                  18
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_ACKCNT_BP_THRESHOLD_BM                  0x00FC0000

#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_BDACKQ_BP_THRESH_BO                     24
#define AG_MG_REGS_PCE_MIU_CONTROL_PCE_BDACKQ_BP_THRESH_BM                     0x07000000

#define AG_MG_REGS_PCE_MIU_CONTROL_ENABLE_MIU_BO                               31
#define AG_MG_REGS_PCE_MIU_CONTROL_ENABLE_MIU_BM                               0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MIU_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            fwdq_start_wm : 6,
            fwdq_stop_wm : 6,
            bdwdq_stop_wm : 2,
            fwcq_stop_wm : 2,
            bdwcq_stop_wm : 2,
            ackcnt_bp_threshold : 6,
            bdackq_bp_thresh : 3,
            fill0 : 4,
            enable_miu : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_miu_control_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_DLT_ACCESS_RO                                           0x00002084
#define AG_MG_REGS_PCE_DLT_ACCESS_RM                                           0x00000001

#define AG_MG_REGS_PCE_DLT_ACCESS_DLT_SELECT_BO                                0
#define AG_MG_REGS_PCE_DLT_ACCESS_DLT_SELECT_BM                                0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLT_ACCESS_U
{
    struct
    {
        ag_mg_regs_register
            dlt_select : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlt_access_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_BD_PTR_RST_RO                                           0x00002088
#define AG_MG_REGS_PCE_BD_PTR_RST_RM                                           0x0000003F

#define AG_MG_REGS_PCE_BD_PTR_RST_BD_PTR_RST_BO                                0
#define AG_MG_REGS_PCE_BD_PTR_RST_BD_PTR_RST_BM                                0x0000003F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BD_PTR_RST_U
{
    struct
    {
        ag_mg_regs_register
            bd_ptr_rst : 6,
            fill0 : 26;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bd_ptr_rst_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_REGKEY_RO                                               0x0000208C
#define AG_MG_REGS_PCE_REGKEY_RM                                               0x8000FFFF

#define AG_MG_REGS_PCE_REGKEY_REGKEY_BO                                        0
#define AG_MG_REGS_PCE_REGKEY_REGKEY_BM                                        0x0000FFFF

#define AG_MG_REGS_PCE_REGKEY_PCE_WRITE_STAT_BO                                31
#define AG_MG_REGS_PCE_REGKEY_PCE_WRITE_STAT_BM                                0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_REGKEY_U
{
    struct
    {
        ag_mg_regs_register
            regkey : 16,
            fill0 : 15,
            write_stat : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_regkey_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_RO                                    0x00002200
#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_RM                                    0x003F0003

#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_INT0_BO                               0
#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_INT0_BM                               0x00000001

#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_INT1_BO                               1
#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_INT1_BM                               0x00000002

#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_BDINT_BO                              16
#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_BDINT_BM                              0x003F0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_INTERRUPT_SUMMARY_U
{
    struct
    {
        ag_mg_regs_register
            int0 : 1,
            int1 : 1,
            fill1 : 14,
            bdint : 6,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_interrupt_summary_u;
#endif


/*
 * PCE_INTERRUPT_STATUS_0 (PCE Interrupt Status Register 0)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_RO                                   0x00002204
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_RM                                   0x7FFFFFFC

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKEV2_BO                        2
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKEV2_BM                        0x00000004

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKEV2WVLAN_BO                   3
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKEV2WVLAN_BM                   0x00000008

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLK820P3_BO                      4
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLK820P3_BM                      0x00000010

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLK802P3WVLAN_BO                 5
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLK802P3WVLAN_BM                 0x00000020

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKIPV4_BO                       6
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKIPV4_BM                       0x00000040

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKIPV6_BO                       7
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKIPV6_BM                       0x00000080

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKUDP_BO                        8
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKUDP_BM                        0x00000100

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKUDPLITE_BO                    9
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_BLKUDPLITE_BM                    0x00000200

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_IHL_CHK_BO                    10
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_IHL_CHK_BM                    0x00000400

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_ID_CHK_BO                     11
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_ID_CHK_BM                     0x00000800

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_FRAG_CHK_BO                   12
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_FRAG_CHK_BM                   0x00001000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_TTL_CHK_BO                    13
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_V4_TTL_CHK_BM                    0x00002000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_DLT_LOOKUP_FAILED_BO             14
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_DLT_LOOKUP_FAILED_BM             0x00004000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_DLT_UDP_OOR_BO                   15
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_DLT_UDP_OOR_BM                   0x00008000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_SV_SOURCE_INDEX_OOR_BO               16
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_SV_SOURCE_INDEX_OOR_BM               0x00010000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_SV_SVT_LOOKUP_FAILED_BO              17
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_SV_SVT_LOOKUP_FAILED_BM              0x00020000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_FW_PAUSE_FRAME_SENT_BO               18
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_FW_PAUSE_FRAME_SENT_BM               0x00040000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_FW_SOF_SOF_BO                        19
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_FW_SOF_SOF_BM                        0x00080000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_FW_EOF_EOF_BO                        20
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_FW_EOF_EOF_BM                        0x00100000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_SIMUL_SOF_EOF_BO                  21
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_SIMUL_SOF_EOF_BM                  0x00200000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_FDQ_OVERRUN_BO                    22
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_FDQ_OVERRUN_BM                    0x00400000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_FOQ_UNDERRUN_BO                   23
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_FOQ_UNDERRUN_BM                   0x00800000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_FRAME_TOO_BIG_BO                  24
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_HP_FRAME_TOO_BIG_BM                  0x01000000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV4_LEN_ERR_BO                  25
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV4_LEN_ERR_BM                  0x02000000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV6_LEN_ERR_BO                  26
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV6_LEN_ERR_BM                  0x04000000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV4_UDP_LEN_ERR_BO              27
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV4_UDP_LEN_ERR_BM              0x08000000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV6_UDP_LEN_ERR_BO              28
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_PCE_IPV6_UDP_LEN_ERR_BM              0x10000000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_IPV4_UDPLITE_COVERAGE_ERR_BO         29
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_IPV4_UDPLITE_COVERAGE_ERR_BM         0x20000000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_IPV6_UDPLITE_COVERAGE_ERR_BO         30
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_IPV6_UDPLITE_COVERAGE_ERR_BM         0x40000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_INTERRUPT_STATUS_0_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 2,
            blkev2 : 1,
            blkev2wvlan : 1,
            blk820p3 : 1,
            blk802p3wvlan : 1,
            blkipv4 : 1,
            blkipv6 : 1,
            blkudp : 1,
            blkudplite : 1,
            v4_ihl_chk : 1,
            v4_id_chk : 1,
            v4_frag_chk : 1,
            v4_ttl_chk : 1,
            dlt_lookup_failed : 1,
            dlt_udp_oor : 1,
            sv_source_index_oor : 1,
            sv_svt_lookup_failed : 1,
            fw_pause_frame_sent : 1,
            fw_sof_sof : 1,
            fw_eof_eof : 1,
            hp_simul_sof_eof : 1,
            hp_fdq_overrun : 1,
            hp_foq_underrun : 1,
            hp_frame_too_big : 1,
            ipv4_len_err : 1,
            ipv6_len_err : 1,
            ipv4_udp_len_err : 1,
            ipv6_udp_len_err : 1,
            ipv4_udplite_coverage_err : 1,
            ipv6_udplite_coverage_err : 1,
            fill0 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_interrupt_status_0_u;
#endif


/*
 * PCE_INTERRUPT_STATUS_1 (PCE Interrupt Status Register 1)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_RO                                   0x00002208
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_RM                                   0x01FFF803

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FDQ_FOQ_ERR_BO                       0
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FDQ_FOQ_ERR_BM                       0x00000001

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FSV_OVERFLOW_BO                      1
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FSV_OVERFLOW_BM                      0x00000002

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FWCQ_OVERFLOW_BO                     11
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FWCQ_OVERFLOW_BM                     0x00000800

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FWDQ_OVERFLOW_BO                     12
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_FWDQ_OVERFLOW_BM                     0x00001000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_BDWQ_OVERFLOW_BO                     13
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_BDWQ_OVERFLOW_BM                     0x00002000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_READ_EXCL_ERR_BO                 14
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_READ_EXCL_ERR_BM                 0x00004000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_READ_SLAVE_ERR_BO                15
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_READ_SLAVE_ERR_BM                0x00008000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_READ_DECODE_ERR_BO               16
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_READ_DECODE_ERR_BM               0x00010000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_WRITE_EXCL_ERR_BO                17
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_WRITE_EXCL_ERR_BM                0x00020000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_WRITE_SLAVE_ERR_BO               18
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_WRITE_SLAVE_ERR_BM               0x00040000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_WRITE_DECODE_ERR_BO              19
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_AXI_WRITE_DECODE_ERR_BM              0x00080000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_READ_ALIGN_ERR_BO                20
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_READ_ALIGN_ERR_BM                0x00100000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_WRITE_ALIGN_ERR_BO               21
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_WRITE_ALIGN_ERR_BM               0x00200000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_SVT_4KBOUND_ERR_BO               22
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_SVT_4KBOUND_ERR_BM               0x00400000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_BD_4KBOUND_ERR_BO                23
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_BD_4KBOUND_ERR_BM                0x00800000

#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_DATA_4KBOUND_ERR_BO              24
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_MIU_DATA_4KBOUND_ERR_BM              0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_INTERRUPT_STATUS_1_U
{
    struct
    {
        ag_mg_regs_register
            fdq_foq_err : 1,
            fsv_overflow : 1,
            fill1 : 9,
            fwcq_overflow : 1,
            fwdq_overflow : 1,
            bdwq_overflow : 1,
            axi_read_excl_err : 1,
            axi_read_slave_err : 1,
            axi_read_decode_err : 1,
            axi_write_excl_err : 1,
            axi_write_slave_err : 1,
            axi_write_decode_err : 1,
            miu_read_align_err : 1,
            miu_write_align_err : 1,
            miu_svt_4kbound_err : 1,
            miu_bd_4kbound_err : 1,
            miu_data_4kbound_err : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_interrupt_status_1_u;
#endif


/*
 * PCE_CFG_STATUS (PCE Configuration Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_CFG_STATUS_RO                                           0x0000220C
#define AG_MG_REGS_PCE_CFG_STATUS_RM                                           0x0000000F

#define AG_MG_REGS_PCE_CFG_STATUS_CFG_HW_ERR_0_BO                              0
#define AG_MG_REGS_PCE_CFG_STATUS_CFG_HW_ERR_0_BM                              0x00000001

#define AG_MG_REGS_PCE_CFG_STATUS_CFG_HW_ERR_1_BO                              1
#define AG_MG_REGS_PCE_CFG_STATUS_CFG_HW_ERR_1_BM                              0x00000002

#define AG_MG_REGS_PCE_CFG_STATUS_CFG_DLT_INIT_DONE_BO                         2
#define AG_MG_REGS_PCE_CFG_STATUS_CFG_DLT_INIT_DONE_BM                         0x00000004

#define AG_MG_REGS_PCE_CFG_STATUS_CFG_DLT_IN_PROGRESS_BO                       3
#define AG_MG_REGS_PCE_CFG_STATUS_CFG_DLT_IN_PROGRESS_BM                       0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_CFG_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            cfg_hw_err_0 : 1,
            cfg_hw_err_1 : 1,
            cfg_dlt_init_done : 1,
            cfg_dlt_in_progress : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_cfg_status_u;
#endif


/*
 * PCE_PAR_STATUS (PCE Parser Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_PAR_STATUS_RO                                           0x00002210
#define AG_MG_REGS_PCE_PAR_STATUS_RM                                           0x000007FF

#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_0_BO                            0
#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_0_BM                            0x00000001

#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_1_BO                            1
#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_1_BM                            0x00000002

#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_2_BO                            2
#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_2_BM                            0x00000004

#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_3_BO                            3
#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_3_BM                            0x00000008

#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_4_BO                            4
#define AG_MG_REGS_PCE_PAR_STATUS_PAR_HW_ERROR_4_BM                            0x00000010

#define AG_MG_REGS_PCE_PAR_STATUS_PAR_STATE_BO                                 5
#define AG_MG_REGS_PCE_PAR_STATUS_PAR_STATE_BM                                 0x000007E0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_PAR_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            par_hw_error_0 : 1,
            par_hw_error_1 : 1,
            par_hw_error_2 : 1,
            par_hw_error_3 : 1,
            par_hw_error_4 : 1,
            par_state : 6,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_par_status_u;
#endif


/*
 * PCE_DLU_STATUS (PCE DLU Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_DLU_STATUS_RO                                           0x00002214
#define AG_MG_REGS_PCE_DLU_STATUS_RM                                           0x0000001B

#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_0_BO                            0
#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_0_BM                            0x00000001

#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_1_BO                            1
#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_1_BM                            0x00000002

#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_2_BO                            3
#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_2_BM                            0x00000008

#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_3_BO                            4
#define AG_MG_REGS_PCE_DLU_STATUS_DLU_HW_ERROR_3_BM                            0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLU_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            dlu_hw_error_0 : 1,
            dlu_hw_error_1 : 1,
            fill1 : 1,
            dlu_hw_error_2 : 1,
            dlu_hw_error_3 : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlu_status_u;
#endif


/*
 * PCE_SLU_STATUS (PCE SLU Status Register)
 * Initialization value: 0x00008000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_SLU_STATUS_RO                                           0x00002218
#define AG_MG_REGS_PCE_SLU_STATUS_RM                                           0x0001FFFF

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_0_BO                            0
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_0_BM                            0x00000001

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_1_BO                            1
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_1_BM                            0x00000002

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_2_BO                            2
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_2_BM                            0x00000004

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_3_BO                            3
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_3_BM                            0x00000008

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_4_BO                            4
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_4_BM                            0x00000010

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_5_BO                            5
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_5_BM                            0x00000020

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_6_BO                            6
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_6_BM                            0x00000040

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_7_BO                            7
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_7_BM                            0x00000080

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_8_BO                            8
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_8_BM                            0x00000100

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_9_BO                            9
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_9_BM                            0x00000200

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_10_BO                           10
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_10_BM                           0x00000400

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_11_BO                           11
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_11_BM                           0x00000800

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_12_BO                           12
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_HW_ERROR_12_BM                           0x00001000

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_UNDERFLOW_BO                     13
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_UNDERFLOW_BM                     0x00002000

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_OVERFLOW_BO                      14
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_OVERFLOW_BM                      0x00004000

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_EMPTY_BO                         15
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_EMPTY_BM                         0x00008000

#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_FULL_BO                          16
#define AG_MG_REGS_PCE_SLU_STATUS_SLU_SVFQ_FF_FULL_BM                          0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SLU_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            slu_hw_error_0 : 1,
            slu_hw_error_1 : 1,
            slu_hw_error_2 : 1,
            slu_hw_error_3 : 1,
            slu_hw_error_4 : 1,
            slu_hw_error_5 : 1,
            slu_hw_error_6 : 1,
            slu_hw_error_7 : 1,
            slu_hw_error_8 : 1,
            slu_hw_error_9 : 1,
            slu_hw_error_10 : 1,
            slu_hw_error_11 : 1,
            slu_hw_error_12 : 1,
            slu_svfq_ff_underflow : 1,
            slu_svfq_ff_overflow : 1,
            slu_svfq_ff_empty : 1,
            slu_svfq_ff_full : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_slu_status_u;
#endif


/*
 * PCE_FWR_STATUS_LO (PCE FWR Status (LO) Register)
 * Initialization value: 0x50007000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_FWR_STATUS_LO_RO                                        0x0000221C
#define AG_MG_REGS_PCE_FWR_STATUS_LO_RM                                        0x7FFFFFFF

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_0_LO_BO                      0
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_0_LO_BM                      0x00000001

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_1_LO_BO                      1
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_1_LO_BM                      0x00000002

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_2_LO_BO                      2
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_2_LO_BM                      0x00000004

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_3_LO_BO                      3
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_3_LO_BM                      0x00000008

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_4_LO_BO                      4
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_4_LO_BM                      0x00000010

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_5_LO_BO                      5
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_HW_ERROR_5_LO_BM                      0x00000020

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_UNDERFLOW_BO                     6
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_UNDERFLOW_BM                     0x00000040

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_UNDERFLOW_BO                     7
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_UNDERFLOW_BM                     0x00000080

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_UNDERFLOW_BO                     8
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_UNDERFLOW_BM                     0x00000100

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_OVERFLOW_BO                      9
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_OVERFLOW_BM                      0x00000200

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_OVERFLOW_BO                      10
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_OVERFLOW_BM                      0x00000400

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_OVERFLOW_BO                      11
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_OVERFLOW_BM                      0x00000800

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_EMPTY_BO                         12
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_EMPTY_BM                         0x00001000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_EMPTY_BO                         13
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_EMPTY_BM                         0x00002000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_EMPTY_BO                         14
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_EMPTY_BM                         0x00004000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_FULL_BO                          15
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWDQ_FULL_BM                          0x00008000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_FULL_BO                          16
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FWCQ_FULL_BM                          0x00010000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_FULL_BO                          17
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDWQ_FULL_BM                          0x00020000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FDTX_STATE_BO                         18
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FDTX_STATE_BM                         0x000C0000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDTX_STATE_BO                         20
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_BDTX_STATE_BM                         0x00300000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_STATE_BO                              22
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_STATE_BM                              0x07C00000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FOQ_UNDERFLOW_BO                      27
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FOQ_UNDERFLOW_BM                      0x08000000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FOQ_EMPTY_BO                          28
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FOQ_EMPTY_BM                          0x10000000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FDQ_UNDERFLOW_BO                      29
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FDQ_UNDERFLOW_BM                      0x20000000

#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FDQ_EMPTY_BO                          30
#define AG_MG_REGS_PCE_FWR_STATUS_LO_FWR_FDQ_EMPTY_BM                          0x40000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FWR_STATUS_LO_U
{
    struct
    {
        ag_mg_regs_register
            fwr_hw_error_0_lo : 1,
            fwr_hw_error_1_lo : 1,
            fwr_hw_error_2_lo : 1,
            fwr_hw_error_3_lo : 1,
            fwr_hw_error_4_lo : 1,
            fwr_hw_error_5_lo : 1,
            fwr_fwdq_underflow : 1,
            fwr_fwcq_underflow : 1,
            fwr_bdwq_underflow : 1,
            fwr_fwdq_overflow : 1,
            fwr_fwcq_overflow : 1,
            fwr_bdwq_overflow : 1,
            fwr_fwdq_empty : 1,
            fwr_fwcq_empty : 1,
            fwr_bdwq_empty : 1,
            fwr_fwdq_full : 1,
            fwr_fwcq_full : 1,
            fwr_bdwq_full : 1,
            fwr_fdtx_state : 2,
            fwr_bdtx_state : 2,
            fwr_state : 5,
            fwr_foq_underflow : 1,
            fwr_foq_empty : 1,
            fwr_fdq_underflow : 1,
            fwr_fdq_empty : 1,
            fill0 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_fwr_status_lo_u;
#endif


/*
 * PCE_MIU_STATUS (PCE MIU Status Registe)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_MIU_STATUS_RO                                           0x00002220
#define AG_MG_REGS_PCE_MIU_STATUS_RM                                           0x0001FFFF

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_0_BO                            0
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_0_BM                            0x00000001

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_1_BO                            1
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_1_BM                            0x00000002

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_2_BO                            2
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_2_BM                            0x00000004

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_3_BO                            3
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_3_BM                            0x00000008

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_4_BO                            4
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_4_BM                            0x00000010

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_5_BO                            5
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_5_BM                            0x00000020

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_6_BO                            6
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_6_BM                            0x00000040

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_7_BO                            7
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_7_BM                            0x00000080

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_8_BO                            8
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_8_BM                            0x00000100

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_9_BO                            9
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_9_BM                            0x00000200

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_10_BO                           10
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_10_BM                           0x00000400

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_11_BO                           11
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_11_BM                           0x00000800

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_12_BO                           12
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_12_BM                           0x00001000

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_13_BO                           13
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_HW_ERROR_13_BM                           0x00002000

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_WR_STATE_BO                              14
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_WR_STATE_BM                              0x0000C000

#define AG_MG_REGS_PCE_MIU_STATUS_MIU_WR_SEL_BO                                16
#define AG_MG_REGS_PCE_MIU_STATUS_MIU_WR_SEL_BM                                0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MIU_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            miu_hw_error_0 : 1,
            miu_hw_error_1 : 1,
            miu_hw_error_2 : 1,
            miu_hw_error_3 : 1,
            miu_hw_error_4 : 1,
            miu_hw_error_5 : 1,
            miu_hw_error_6 : 1,
            miu_hw_error_7 : 1,
            miu_hw_error_8 : 1,
            miu_hw_error_9 : 1,
            miu_hw_error_10 : 1,
            miu_hw_error_11 : 1,
            miu_hw_error_12 : 1,
            miu_hw_error_13 : 1,
            miu_wr_state : 2,
            miu_wr_sel : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_miu_status_u;
#endif


/*
 * PCE_L2C_STATUS_REG (PCE L2C Status Register)
 * Initialization value: 0x00000140  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_L2C_STATUS_REG_RO                                       0x00002224
#define AG_MG_REGS_PCE_L2C_STATUS_REG_RM                                       0x000003FF

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_0_BO                        0
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_0_BM                        0x00000001

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_1_BO                        1
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_1_BM                        0x00000002

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_2_BO                        2
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_2_BM                        0x00000004

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_3_BO                        3
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_3_BM                        0x00000008

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_4_BO                        4
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_4_BM                        0x00000010

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_5_BO                        5
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_HW_ERROR_5_BM                        0x00000020

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_IPV4_HDRCKSUM_EMPTY_BO               6
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_IPV4_HDRCKSUM_EMPTY_BM               0x00000040

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_IPV4_HDRCKSUM_FULL_BO                7
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_IPV4_HDRCKSUM_FULL_BM                0x00000080

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_UDP_CKSUM_EMPTY_BO                   8
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_UDP_CKSUM_EMPTY_BM                   0x00000100

#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_UDP_CKSUM_FULL_BO                    9
#define AG_MG_REGS_PCE_L2C_STATUS_REG_L2C_UDP_CKSUM_FULL_BM                    0x00000200

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_L2C_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            l2c_hw_error_0 : 1,
            l2c_hw_error_1 : 1,
            l2c_hw_error_2 : 1,
            l2c_hw_error_3 : 1,
            l2c_hw_error_4 : 1,
            l2c_hw_error_5 : 1,
            l2c_ipv4_hdrcksum_empty : 1,
            l2c_ipv4_hdrcksum_full : 1,
            l2c_udp_cksum_empty : 1,
            l2c_udp_cksum_full : 1,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_l2c_status_reg_u;
#endif


/*
 * PCE_FWR_STATUS_HI (PCE FWR Status (HI) Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_FWR_STATUS_HI_RO                                        0x00002228
#define AG_MG_REGS_PCE_FWR_STATUS_HI_RM                                        0x0000007F

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_HW_ERROR_0_HI_BO                      0
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_HW_ERROR_0_HI_BM                      0x00000001

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_HW_ERROR_1_HI_BO                      1
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_HW_ERROR_1_HI_BM                      0x00000002

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_HW_ERROR_2_HI_BO                      2
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_HW_ERROR_2_HI_BM                      0x00000004

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FDQ_OVERFLOW_BO                       3
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FDQ_OVERFLOW_BM                       0x00000008

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FDQ_FULL_BO                           4
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FDQ_FULL_BM                           0x00000010

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FOQ_OVERFLOW_BO                       5
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FOQ_OVERFLOW_BM                       0x00000020

#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FOQ_FULL_BO                           6
#define AG_MG_REGS_PCE_FWR_STATUS_HI_FWR_FOQ_FULL_BM                           0x00000040

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FWR_STATUS_HI_U
{
    struct
    {
        ag_mg_regs_register
            fwr_hw_error_0_hi : 1,
            fwr_hw_error_1_hi : 1,
            fwr_hw_error_2_hi : 1,
            fwr_fdq_overflow : 1,
            fwr_fdq_full : 1,
            fwr_foq_overflow : 1,
            fwr_foq_full : 1,
            fill0 : 25;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_fwr_status_hi_u;
#endif


/*
 * PCE_SOF_COUNTER (PCE SOF Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_SOF_COUNTER_RO                                          0x00002240
#define AG_MG_REGS_PCE_SOF_COUNTER_RM                                          0xFFFFFFFF

#define AG_MG_REGS_PCE_SOF_COUNTER_SOF_COUNT_BO                                0
#define AG_MG_REGS_PCE_SOF_COUNTER_SOF_COUNT_BM                                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SOF_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            sof_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_sof_counter_u;
#endif


/*
 * PCE_EOF_COUNTER (PCE EOF Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_EOF_COUNTER_RO                                          0x00002244
#define AG_MG_REGS_PCE_EOF_COUNTER_RM                                          0xFFFFFFFF

#define AG_MG_REGS_PCE_EOF_COUNTER_EOF_COUNT_BO                                0
#define AG_MG_REGS_PCE_EOF_COUNTER_EOF_COUNT_BM                                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_EOF_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            eof_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_eof_counter_u;
#endif


/*
 * PCE_SOF_SOF_COUNTER (PCE SOF_SOF Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_SOF_SOF_COUNTER_RO                                      0x00002248
#define AG_MG_REGS_PCE_SOF_SOF_COUNTER_RM                                      0xFFFFFFFF

#define AG_MG_REGS_PCE_SOF_SOF_COUNTER_SOF_SOF_COUNT_BO                        0
#define AG_MG_REGS_PCE_SOF_SOF_COUNTER_SOF_SOF_COUNT_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SOF_SOF_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            sof_sof_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_sof_sof_counter_u;
#endif


/*
 * PCE_EOF_EOF_COUNTER (PCE EOF_EOF Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_EOF_EOF_COUNTER_RO                                      0x0000224C
#define AG_MG_REGS_PCE_EOF_EOF_COUNTER_RM                                      0xFFFFFFFF

#define AG_MG_REGS_PCE_EOF_EOF_COUNTER_EOF_EOF_COUNT_BO                        0
#define AG_MG_REGS_PCE_EOF_EOF_COUNTER_EOF_EOF_COUNT_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_EOF_EOF_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            eof_eof_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_eof_eof_counter_u;
#endif


/*
 * PCE_RX_BYTE_COUNTER (PCE Bytes Received Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_RX_BYTE_COUNTER_RO                                      0x00002250
#define AG_MG_REGS_PCE_RX_BYTE_COUNTER_RM                                      0xFFFFFFFF

#define AG_MG_REGS_PCE_RX_BYTE_COUNTER_RX_BYTE_COUNT_BO                        0
#define AG_MG_REGS_PCE_RX_BYTE_COUNTER_RX_BYTE_COUNT_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_RX_BYTE_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            rx_byte_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_rx_byte_counter_u;
#endif


/*
 * PCE_DLU_COUNTER (PCE Destination Lookup Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_DLU_COUNTER_RO                                          0x00002254
#define AG_MG_REGS_PCE_DLU_COUNTER_RM                                          0xFFFFFFFF

#define AG_MG_REGS_PCE_DLU_COUNTER_DLU_COUNT_BO                                0
#define AG_MG_REGS_PCE_DLU_COUNTER_DLU_COUNT_BM                                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLU_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            dlu_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlu_counter_u;
#endif


/*
 * PCE_DLT_READ_ERR_COUNTER (PCE DLT Read Error Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_RO                                 0x00002258
#define AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_RM                                 0xFFFFFFFF

#define AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_DLT_READ_ERR_COUNT_BO              0
#define AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_DLT_READ_ERR_COUNT_BM              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            dlt_read_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlt_read_err_counter_u;
#endif


/*
 * PCE_DLU_FAIL_COUNTER (PCE Destination Lookup Failures Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_DLU_FAIL_COUNTER_RO                                     0x0000225C
#define AG_MG_REGS_PCE_DLU_FAIL_COUNTER_RM                                     0xFFFFFFFF

#define AG_MG_REGS_PCE_DLU_FAIL_COUNTER_DLU_FAIL_COUNT_BO                      0
#define AG_MG_REGS_PCE_DLU_FAIL_COUNTER_DLU_FAIL_COUNT_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLU_FAIL_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            dlu_fail_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlu_fail_counter_u;
#endif


/*
 * PCE_DLU_OOR_COUNTER (PCE DLU OOR Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_DLU_OOR_COUNTER_RO                                      0x00002260
#define AG_MG_REGS_PCE_DLU_OOR_COUNTER_RM                                      0xFFFFFFFF

#define AG_MG_REGS_PCE_DLU_OOR_COUNTER_DLU_OOR_COUNT_BO                        0
#define AG_MG_REGS_PCE_DLU_OOR_COUNTER_DLU_OOR_COUNT_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLU_OOR_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            dlu_oor_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlu_oor_counter_u;
#endif


/*
 * PCE_DLU_SVT_OOR_COUNTER (PCE DLU SVT OOR Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_RO                                  0x00002264
#define AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_SVT_OOR_COUNT_BO                    0
#define AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_SVT_OOR_COUNT_BM                    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            svt_oor_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_dlu_svt_oor_counter_u;
#endif


/*
 * PCE_SLU_COUNTER (PCE SLU Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_SLU_COUNTER_RO                                          0x00002268
#define AG_MG_REGS_PCE_SLU_COUNTER_RM                                          0xFFFFFFFF

#define AG_MG_REGS_PCE_SLU_COUNTER_SLU_COUNT_BO                                0
#define AG_MG_REGS_PCE_SLU_COUNTER_SLU_COUNT_BM                                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SLU_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            slu_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_slu_counter_u;
#endif


/*
 * PCE_SLU_FAIL_COUNTER (PCE SLU Failures Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_SLU_FAIL_COUNTER_RO                                     0x0000226C
#define AG_MG_REGS_PCE_SLU_FAIL_COUNTER_RM                                     0xFFFFFFFF

#define AG_MG_REGS_PCE_SLU_FAIL_COUNTER_SLU_FAIL_COUNT_BO                      0
#define AG_MG_REGS_PCE_SLU_FAIL_COUNTER_SLU_FAIL_COUNT_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SLU_FAIL_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            slu_fail_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_slu_fail_counter_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_RO                                 0x00002270
#define AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_RM                                 0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_IPV4_CHKSUM_FAIL_CNT_BO            0
#define AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_IPV4_CHKSUM_FAIL_CNT_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_U
{
    struct
    {
        ag_mg_regs_register
            ipv4_chksum_fail_cnt;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv4_chksum_fail_cnt_u;
#endif


/*
 * PCE_UDP_CHKSUM_FAIL_CNT (PCE UDP Checksum Failures Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_RO                                  0x00002274
#define AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_UDP_CHKSUM_FAIL_COUNT_BO            0
#define AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_UDP_CHKSUM_FAIL_COUNT_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_U
{
    struct
    {
        ag_mg_regs_register
            udp_chksum_fail_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udp_chksum_fail_cnt_u;
#endif


/*
 * PCE_UDP_CHKSUM_MASK_CNT (PCE UDP Checksum Mask Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_RO                                  0x00002278
#define AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_UDP_CHKSUM_MASK_CNT_BO              0
#define AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_UDP_CHKSUM_MASK_CNT_BM              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_U
{
    struct
    {
        ag_mg_regs_register
            udp_chksum_mask_cnt;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udp_chksum_mask_cnt_u;
#endif


/*
 * PCE_UNKNOWN_FRAME_CNT (PCE Unknown Frame Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_RO                                    0x0000227C
#define AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_RM                                    0xFFFFFFFF

#define AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_UNKNOWN_FRAME_CNT_BO                  0
#define AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_UNKNOWN_FRAME_CNT_BM                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_U
{
    struct
    {
        ag_mg_regs_register
            unknown_frame_cnt;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_unknown_frame_cnt_u;
#endif


/*
 * PCE_PARSER_FLUSH_COUNTER (PCE Runt Frame Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_RO                                 0x00002280
#define AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_RM                                 0xFFFFFFFF

#define AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_PAR_FLUSH_COUNT_BO                 0
#define AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_PAR_FLUSH_COUNT_BM                 0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            par_flush_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_parser_flush_counter_u;
#endif


/*
 * PCE_UDL_COUNTER (PCE UDL Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_UDL_COUNTER_RO                                          0x00002284
#define AG_MG_REGS_PCE_UDL_COUNTER_RM                                          0xFFFFFFFF

#define AG_MG_REGS_PCE_UDL_COUNTER_UDL_COUNT_BO                                0
#define AG_MG_REGS_PCE_UDL_COUNTER_UDL_COUNT_BM                                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDL_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            udl_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udl_counter_u;
#endif


/*
 * PCE_V2_COUNTER (PCE V2 Frames Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_V2_COUNTER_RO                                           0x00002288
#define AG_MG_REGS_PCE_V2_COUNTER_RM                                           0xFFFFFFFF

#define AG_MG_REGS_PCE_V2_COUNTER_V2_COUNT_BO                                  0
#define AG_MG_REGS_PCE_V2_COUNTER_V2_COUNT_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_V2_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            v2_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_v2_counter_u;
#endif


/*
 * PCE_VLAN_COUNTER (PCE V2 VLAN Frames Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_VLAN_COUNTER_RO                                         0x0000228C
#define AG_MG_REGS_PCE_VLAN_COUNTER_RM                                         0xFFFFFFFF

#define AG_MG_REGS_PCE_VLAN_COUNTER_VLAN_COUNT_BO                              0
#define AG_MG_REGS_PCE_VLAN_COUNTER_VLAN_COUNT_BM                              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_VLAN_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            vlan_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_vlan_counter_u;
#endif


/*
 * PCE_SNAP_COUNTER (PCE SNAP Frames Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_SNAP_COUNTER_RO                                         0x00002290
#define AG_MG_REGS_PCE_SNAP_COUNTER_RM                                         0xFFFFFFFF

#define AG_MG_REGS_PCE_SNAP_COUNTER_SNAP_COUNT_BO                              0
#define AG_MG_REGS_PCE_SNAP_COUNTER_SNAP_COUNT_BM                              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_SNAP_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            snap_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_snap_counter_u;
#endif


/*
 * PCE_IPV4_COUNTER (PCE IPV4 Packets Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV4_COUNTER_RO                                         0x00002294
#define AG_MG_REGS_PCE_IPV4_COUNTER_RM                                         0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV4_COUNTER_IPV4_COUNT_BO                              0
#define AG_MG_REGS_PCE_IPV4_COUNTER_IPV4_COUNT_BM                              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV4_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            ipv4_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv4_counter_u;
#endif


/*
 * PCE_IPV6_COUNTER (PCE IPV6 Packets Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV6_COUNTER_RO                                         0x00002298
#define AG_MG_REGS_PCE_IPV6_COUNTER_RM                                         0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV6_COUNTER_IPV6_COUNT_BO                              0
#define AG_MG_REGS_PCE_IPV6_COUNTER_IPV6_COUNT_BM                              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV6_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            ipv6_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv6_counter_u;
#endif


/*
 * PCE_UDP_COUNTER (PCE UDP Packets Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_UDP_COUNTER_RO                                          0x0000229C
#define AG_MG_REGS_PCE_UDP_COUNTER_RM                                          0xFFFFFFFF

#define AG_MG_REGS_PCE_UDP_COUNTER_UDP_COUNT_BO                                0
#define AG_MG_REGS_PCE_UDP_COUNTER_UDP_COUNT_BM                                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDP_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            udp_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udp_counter_u;
#endif


/*
 * PCE_UDPLITE_COUNTER (PCE UDPLite Packets Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_UDPLITE_COUNTER_RO                                      0x000022A0
#define AG_MG_REGS_PCE_UDPLITE_COUNTER_RM                                      0xFFFFFFFF

#define AG_MG_REGS_PCE_UDPLITE_COUNTER_UDPLITE_COUNT_BO                        0
#define AG_MG_REGS_PCE_UDPLITE_COUNTER_UDPLITE_COUNT_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UDPLITE_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            udplite_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_udplite_counter_u;
#endif


/*
 * PCE_FDQFULL_DISCARD_CNT (PCE FDQ Full Discards Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_RO                                  0x000022A4
#define AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_FDQ_FULL_DISCARD_CNT_BO             0
#define AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_FDQ_FULL_DISCARD_CNT_BM             0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_U
{
    struct
    {
        ag_mg_regs_register
            fdq_full_discard_cnt;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_fdqfull_discard_cnt_u;
#endif


/*
 * PCE_CORRUPTED_FRAME_CNT (PCE Corrupted Frames Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_RO                                  0x000022A8
#define AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_CORRUPTED_FRAME_CNT_BO              0
#define AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_CORRUPTED_FRAME_CNT_BM              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_U
{
    struct
    {
        ag_mg_regs_register
            corrupted_frame_cnt;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_corrupted_frame_cnt_u;
#endif


/*
 * PCE_FOQFULL_DISCARD_CNT (PCE FOQ Full Discards Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_RO                                  0x000022AC
#define AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_FOQ_FULL_DISCARD_CNT_BO             0
#define AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_FOQ_FULL_DISCARD_CNT_BM             0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_U
{
    struct
    {
        ag_mg_regs_register
            foq_full_discard_cnt;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_foqfull_discard_cnt_u;
#endif


/*
 * PCE_MAX_SVT_STALL_COUNTER (PCE Maximum SVT Stall Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_RO                                0x000022B0
#define AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_RM                                0x0000FFFF

#define AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_MAX_SVT_STALL_COUNT_BO            0
#define AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_MAX_SVT_STALL_COUNT_BM            0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            max_svt_stall_count : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_max_svt_stall_counter_u;
#endif


/*
 * PCE_MAX_BDT_STALL_COUNTER (PCE Maximum BDT Stall Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_RO                                0x000022B4
#define AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_RM                                0x0000FFFF

#define AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_MAX_BDT_STALL_COUNT_BO            0
#define AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_MAX_BDT_STALL_COUNT_BM            0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            max_bdt_stall_count : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_max_bdt_stall_counter_u;
#endif


/*
 * PCE_PAUSE_FRAME_COUNTER (PCE PAUSE Frames Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_RO                                  0x000022B8
#define AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_PAUSE_FRAME_COUNT_BO                0
#define AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_PAUSE_FRAME_COUNT_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            pause_frame_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_pause_frame_counter_u;
#endif


/*
 * PCE_IPV4_LEN_ERR_COUNTER (PCE IPV4 Short Frame Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_RO                                 0x000022BC
#define AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_RM                                 0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_IPV4_LEN_ERR_COUNT_BO              0
#define AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_IPV4_LEN_ERR_COUNT_BM              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            ipv4_len_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv4_len_err_counter_u;
#endif


/*
 * PCE_IPV6_LEN_ERR_COUNTER (PCE IPV6 Short Frame Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_RO                                 0x000022C0
#define AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_RM                                 0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_IPV6_LEN_ERR_COUNT_BO              0
#define AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_IPV6_LEN_ERR_COUNT_BM              0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            ipv6_len_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv6_len_err_counter_u;
#endif


/*
 * PCE_IPV4UDP_LEN_ERR_CNT (PCE IPV4 UDP Short Frame Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_RO                                  0x000022C4
#define AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_IPV4UDP_LEN_ERR_COUNT_BO            0
#define AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_IPV4UDP_LEN_ERR_COUNT_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_U
{
    struct
    {
        ag_mg_regs_register
            ipv4udp_len_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv4udp_len_err_cnt_u;
#endif


/*
 * PCE_IPV6UDP_LEN_ERR_CNT (PCE IPV6 UDP Short Frame Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_RO                                  0x000022C8
#define AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_RM                                  0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_IPV6UDP_LEN_ERR_COUNT_BO            0
#define AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_IPV6UDP_LEN_ERR_COUNT_BM            0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_U
{
    struct
    {
        ag_mg_regs_register
            ipv6udp_len_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv6udp_len_err_cnt_u;
#endif


/*
 * PCE_IPV4UDPLITE_COV_ERR_CNT (PCE IPV4 UDPLite Coverage Error Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_RO                              0x000022CC
#define AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_RM                              0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_IPV4UDPLITE_COV_ERR_COUNT_BO    0
#define AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_IPV4UDPLITE_COV_ERR_COUNT_BM    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_U
{
    struct
    {
        ag_mg_regs_register
            ipv4udplite_cov_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv4udplite_cov_err_cnt_u;
#endif


/*
 * PCE_IPV6UDPLITE_COV_ERR_CNT (PCE IPV6 UDPLite Coverage Error Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_RO                              0x000022D0
#define AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_RM                              0xFFFFFFFF

#define AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_IPV6UDPLITE_COV_ERR_COUNT_BO    0
#define AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_IPV6UDPLITE_COV_ERR_COUNT_BM    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_U
{
    struct
    {
        ag_mg_regs_register
            ipv6udplite_cov_err_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ipv6udplite_cov_err_cnt_u;
#endif


/*
 * PCE_BDL_EMPTY_DISCARD_CNT (PCE BDL Empty Discard Count Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_RO                                0x000022D4
#define AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_RM                                0xFFFFFFFF

#define AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_BDL_EMPTY_DISCARD_COUNT_BO        0
#define AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_BDL_EMPTY_DISCARD_COUNT_BM        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_U
{
    struct
    {
        ag_mg_regs_register
            bdl_empty_discard_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bdl_empty_discard_cnt_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_BD_BOT_RO                                               0x00002400
#define AG_MG_REGS_PCE_BD_BOT_RM                                               0xFFFFFFF8
#define AG_MG_REGS_PCE_BD_BOT_RPT                                              6
#define AG_MG_REGS_PCE_BD_BOT_IVL                                              0x20

#define AG_MG_REGS_PCE_BD_BOT_BD_BOT_BO                                        3
#define AG_MG_REGS_PCE_BD_BOT_BD_BOT_BM                                        0xFFFFFFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BD_BOT_U
{
    struct
    {
        ag_mg_regs_register
            fill0 : 3,
            bd_bot : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bd_bot_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_BD_TOP_RO                                               0x00002404
#define AG_MG_REGS_PCE_BD_TOP_RM                                               0xFFFFFFF8
#define AG_MG_REGS_PCE_BD_TOP_RPT                                              6
#define AG_MG_REGS_PCE_BD_TOP_IVL                                              0x20

#define AG_MG_REGS_PCE_BD_TOP_BD_TOP_BO                                        3
#define AG_MG_REGS_PCE_BD_TOP_BD_TOP_BM                                        0xFFFFFFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BD_TOP_U
{
    struct
    {
        ag_mg_regs_register
            fill0 : 3,
            bd_top : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bd_top_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_RO                                     0x00002408
#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_RM                                     0xFFFFFFFF
#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_RPT                                    6
#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_IVL                                    0x20

#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_BUF_SENT_COUNT_BO                      0
#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_BUF_SENT_COUNT_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BUF_SENT_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            buf_sent_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_buf_sent_counter_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_FRAME_SENT_CNT_RO                                       0x0000240C
#define AG_MG_REGS_PCE_FRAME_SENT_CNT_RM                                       0xFFFFFFFF
#define AG_MG_REGS_PCE_FRAME_SENT_CNT_RPT                                      6
#define AG_MG_REGS_PCE_FRAME_SENT_CNT_IVL                                      0x20

#define AG_MG_REGS_PCE_FRAME_SENT_CNT_FRAME_SENT_COUNT_BO                      0
#define AG_MG_REGS_PCE_FRAME_SENT_CNT_FRAME_SENT_COUNT_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_FRAME_SENT_CNT_U
{
    struct
    {
        ag_mg_regs_register
            frame_sent_count;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_frame_sent_cnt_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Clear on Read Only
 */
#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_RO                                      0x00002410
#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_RM                                      0x0000FFFF
#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_RPT                                     6
#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_IVL                                     0x20

#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_MAX_BDL_LATENCY_BO                      0
#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_MAX_BDL_LATENCY_BM                      0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MAX_BDL_LATENCY_U
{
    struct
    {
        ag_mg_regs_register
            max_bdl_latency : 16,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_max_bdl_latency_u;
#endif


/*
 * PCE_BDF_STATUS (PCE BDF Status Register)
 * Initialization value: 0x00F00000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCE_BDF_STATUS_RO                                           0x00002414
#define AG_MG_REGS_PCE_BDF_STATUS_RM                                           0x000000FF
#define AG_MG_REGS_PCE_BDF_STATUS_RPT                                          6
#define AG_MG_REGS_PCE_BDF_STATUS_IVL                                          0x20

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_HW_ERROR_0_BO                            0
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_HW_ERROR_0_BM                            0x00000001

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_HW_ERROR_1_BO                            1
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_HW_ERROR_1_BM                            0x00000002

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_HW_ERROR_2_BO                            2
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_HW_ERROR_2_BM                            0x00000004

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_UNDERFLOW_BO                         3
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_UNDERFLOW_BM                         0x00000008

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_OVERFLOW_BO                          4
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_OVERFLOW_BM                          0x00000010

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_EMPTY_BO                             5
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_EMPTY_BM                             0x00000020

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_FULL_BO                              6
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_FULL_BM                              0x00000040

#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_REFETCH_OVR_THRESH_BO                7
#define AG_MG_REGS_PCE_BDF_STATUS_BDF_BDL_REFETCH_OVR_THRESH_BM                0x00000080

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BDF_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            bdf_hw_error_0 : 1,
            bdf_hw_error_1 : 1,
            bdf_hw_error_2 : 1,
            bdf_bdl_underflow : 1,
            bdf_bdl_overflow : 1,
            bdf_bdl_empty : 1,
            bdf_bdl_full : 1,
            bdf_bdl_refetch_ovr_thresh : 1,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bdf_status_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_PCE_BD_INT_STATUS_RO                                        0x00002418
#define AG_MG_REGS_PCE_BD_INT_STATUS_RM                                        0x0000000F
#define AG_MG_REGS_PCE_BD_INT_STATUS_RPT                                       6
#define AG_MG_REGS_PCE_BD_INT_STATUS_IVL                                       0x20

#define AG_MG_REGS_PCE_BD_INT_STATUS_PCE_IPV4_HEADERCKSUM_ERR_BO               0
#define AG_MG_REGS_PCE_BD_INT_STATUS_PCE_IPV4_HEADERCKSUM_ERR_BM               0x00000001

#define AG_MG_REGS_PCE_BD_INT_STATUS_L4_UDP_CKSUM_ERR_BO                       1
#define AG_MG_REGS_PCE_BD_INT_STATUS_L4_UDP_CKSUM_ERR_BM                       0x00000002

#define AG_MG_REGS_PCE_BD_INT_STATUS_BD_PREFETCH_OWNED_BO                      2
#define AG_MG_REGS_PCE_BD_INT_STATUS_BD_PREFETCH_OWNED_BM                      0x00000004

#define AG_MG_REGS_PCE_BD_INT_STATUS_BD_PREFETCH_OOR_BO                        3
#define AG_MG_REGS_PCE_BD_INT_STATUS_BD_PREFETCH_OOR_BM                        0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BD_INT_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            ipv4_headercksum_err : 1,
            l4_udp_cksum_err : 1,
            bd_prefetch_owned : 1,
            bd_prefetch_oor : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bd_int_status_u;
#endif


/*
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_BD_INT_MASK_RO                                          0x0000241C
#define AG_MG_REGS_PCE_BD_INT_MASK_RM                                          0x0000000F
#define AG_MG_REGS_PCE_BD_INT_MASK_RPT                                         6
#define AG_MG_REGS_PCE_BD_INT_MASK_IVL                                         0x20

#define AG_MG_REGS_PCE_BD_INT_MASK_BDMASK_BO                                   0
#define AG_MG_REGS_PCE_BD_INT_MASK_BDMASK_BM                                   0x0000000F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_BD_INT_MASK_U
{
    struct
    {
        ag_mg_regs_register
            bdmask : 4,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_bd_int_mask_u;
#endif


/*
 * PCE_MMR_MATCH<0-7> (PCE MMR Pattern Match Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MMR_MATCH_RO                                            0x00002800
#define AG_MG_REGS_PCE_MMR_MATCH_RM                                            0xFFFFFFFF
#define AG_MG_REGS_PCE_MMR_MATCH_RPT                                           8
#define AG_MG_REGS_PCE_MMR_MATCH_IVL                                           0x40

#define AG_MG_REGS_PCE_MMR_MATCH_MATCH_BO                                      0
#define AG_MG_REGS_PCE_MMR_MATCH_MATCH_BM                                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MMR_MATCH_U
{
    struct
    {
        ag_mg_regs_register
            match;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mmr_match_u;
#endif


/*
 * PCE_MMR_MASK<0-7> (PCE MMR Pattern Mask Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_MMR_MASK_RO                                             0x00002820
#define AG_MG_REGS_PCE_MMR_MASK_RM                                             0xFFFFFFFF
#define AG_MG_REGS_PCE_MMR_MASK_IVL                                            0x40

#define AG_MG_REGS_PCE_MMR_MASK_MASK_BO                                        0
#define AG_MG_REGS_PCE_MMR_MASK_MASK_BM                                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_MMR_MASK_U
{
    struct
    {
        ag_mg_regs_register
            mask;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_mmr_mask_u;
#endif


/*
 * PCE_UPR_INDEX_ADJUST<0-3> (PCE UPR Index Adjust Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_UPR_INDEX_ADJUST_RO                                     0x00002900
#define AG_MG_REGS_PCE_UPR_INDEX_ADJUST_RM                                     0xFFFFFFFF
#define AG_MG_REGS_PCE_UPR_INDEX_ADJUST_IVL                                    0x8

#define AG_MG_REGS_PCE_UPR_INDEX_ADJUST_INDEX_ADJUST_BO                        0
#define AG_MG_REGS_PCE_UPR_INDEX_ADJUST_INDEX_ADJUST_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UPR_INDEX_ADJUST_U
{
    struct
    {
        ag_mg_regs_register
            index_adjust;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_upr_index_adjust_u;
#endif


/*
 * PCE_UPR_PATTERN_CONTROL<0-3> (PCE UPR Pattern Control Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_RO                                  0x00002904
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_RM                                  0x0007FFFF
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_RPT                                 4
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_IVL                                 0x8

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_ADDSUBINDEX_BO                      0
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_ADDSUBINDEX_BM                      0x00000001

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_STOP_PROGRAM_BO                     1
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_STOP_PROGRAM_BM                     0x00000002

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_VALID_PROGRAM_BO                    2
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_VALID_PROGRAM_BM                    0x00000004

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_FORCE_DQ_BO                         3
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_FORCE_DQ_BM                         0x00000008

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_NIBBLE_LENGTH_BO                    4
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_NIBBLE_LENGTH_BM                    0x00000070

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_START_NIBBLE_BO                     7
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_START_NIBBLE_BM                     0x00001F80

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_L2_802_3_BO                         13
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_L2_802_3_BM                         0x00002000

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_BYTEBITMUX_BO                       14
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_BYTEBITMUX_BM                       0x00004000

#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_PREDEFINED_DQ_BO                    15
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_PREDEFINED_DQ_BM                    0x00078000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            addsubindex : 1,
            stop_program : 1,
            valid_program : 1,
            force_dq : 1,
            nibble_length : 3,
            start_nibble : 6,
            l2_802_3 : 1,
            bytebitmux : 1,
            predefined_dq : 4,
            fill0 : 13;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_upr_pattern_control_u;
#endif


/*
 * PCE_ULT_ENTRY<0-127> (PCE ULT Destination Queue Lookup Entry Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCE_ULT_ENTRY_RO                                            0x00002C00
#define AG_MG_REGS_PCE_ULT_ENTRY_RM                                            0xFFFFFFFF
#define AG_MG_REGS_PCE_ULT_ENTRY_RPT                                           256
#define AG_MG_REGS_PCE_ULT_ENTRY_IVL                                           0x4

#define AG_MG_REGS_PCE_ULT_ENTRY_ULT_ENTRY_BO                                  0
#define AG_MG_REGS_PCE_ULT_ENTRY_ULT_ENTRY_BM                                  0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCE_ULT_ENTRY_U
{
    struct
    {
        ag_mg_regs_register
            ult_entry ;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pce_ult_entry_u;
#endif

/*
* Physical register addresses for accessing PCEp, where p = 0 or 1)
*/
#ifdef AG_MG_REGS_BUILT_FOR_DSS
#define AG_MG_REGS_PCE_BASE		0xC3040000
#else
#define AG_MG_REGS_PCE_BASE		0x30040000
#endif
#define AG_MG_REGS_PCE_REG(p,ro)		(AG_MG_REGS_PCE_BASE+(0x8000*(p)+(ro)))

#define AG_MG_REGS_PCE_DLT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLT_RO)
#define AG_MG_REGS_PCE_DLT_ENTRY_RA(p,udp_p_num)        AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLT_RO+0x4*(udp_p_num))
#define AG_MG_REGS_PCE_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_CONTROL_RO)
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_LOCAL_MS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_LOCAL_LS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_REMOTE_MS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_REMOTE_LS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_23_MS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_23_MS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_2_LS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_2_LS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_3_LS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_3_LS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_45_MS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_45_MS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_4_LS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_4_LS_RO)
#define AG_MG_REGS_PCE_MAC_DEST_5_LS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAC_DEST_5_LS_RO)
#define AG_MG_REGS_PCE_MAX_FRAME_SIZE_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAX_FRAME_SIZE_RO)
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTA_RO)
#define AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDP_DEST_MAX_MIN_PORTB_RO)
#define AG_MG_REGS_PCE_IPDEST0_0_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST0_0_RO)
#define AG_MG_REGS_PCE_IPDEST0_1_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST0_1_RO)
#define AG_MG_REGS_PCE_IPDEST0_2_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST0_2_RO)
#define AG_MG_REGS_PCE_IPDEST0_3_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST0_3_RO)
#define AG_MG_REGS_PCE_IPDEST1_0_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST1_0_RO)
#define AG_MG_REGS_PCE_IPDEST1_1_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST1_1_RO)
#define AG_MG_REGS_PCE_IPDEST1_2_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST1_2_RO)
#define AG_MG_REGS_PCE_IPDEST1_3_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPDEST1_3_RO)
#define AG_MG_REGS_PCE_BDL_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BDL_CONTROL_RO)
#define AG_MG_REGS_PCE_SVT_TOP_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SVT_TOP_RO)
#define AG_MG_REGS_PCE_SVT_BOT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SVT_BOT_RO)
#define AG_MG_REGS_PCE_FDQ_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FDQ_CONTROL_RO)
#define AG_MG_REGS_PCE_FOQ_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FOQ_CONTROL_RO)
#define AG_MG_REGS_PCE_EXINT_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_EXINT_CONTROL_RO)
#define AG_MG_REGS_PCE_FINT_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FINT_CONTROL_RO)
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_0_RO)
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_INTERRUPT_STATUS_MASK_1_RO)
#define AG_MG_REGS_PCE_MIU_CONTROL_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MIU_CONTROL_RO)
#define AG_MG_REGS_PCE_DLT_ACCESS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLT_ACCESS_RO)
#define AG_MG_REGS_PCE_BD_PTR_RST_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BD_PTR_RST_RO)
#define AG_MG_REGS_PCE_REGKEY_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_REGKEY_RO)
#define AG_MG_REGS_PCE_INTERRUPT_SUMMARY_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_INTERRUPT_SUMMARY_RO)
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_0_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_INTERRUPT_STATUS_0_RO)
#define AG_MG_REGS_PCE_INTERRUPT_STATUS_1_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_INTERRUPT_STATUS_1_RO)
#define AG_MG_REGS_PCE_CFG_STATUS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_CFG_STATUS_RO)
#define AG_MG_REGS_PCE_PAR_STATUS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_PAR_STATUS_RO)
#define AG_MG_REGS_PCE_DLU_STATUS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLU_STATUS_RO)
#define AG_MG_REGS_PCE_SLU_STATUS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SLU_STATUS_RO)
#define AG_MG_REGS_PCE_FWR_STATUS_LO_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FWR_STATUS_LO_RO)
#define AG_MG_REGS_PCE_MIU_STATUS_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MIU_STATUS_RO)
#define AG_MG_REGS_PCE_L2C_STATUS_REG_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_L2C_STATUS_REG_RO)
#define AG_MG_REGS_PCE_FWR_STATUS_HI_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FWR_STATUS_HI_RO)
#define AG_MG_REGS_PCE_SOF_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SOF_COUNTER_RO)
#define AG_MG_REGS_PCE_EOF_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_EOF_COUNTER_RO)
#define AG_MG_REGS_PCE_SOF_SOF_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SOF_SOF_COUNTER_RO)
#define AG_MG_REGS_PCE_EOF_EOF_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_EOF_EOF_COUNTER_RO)
#define AG_MG_REGS_PCE_RX_BYTE_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_RX_BYTE_COUNTER_RO)
#define AG_MG_REGS_PCE_DLU_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLU_COUNTER_RO)
#define AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLT_READ_ERR_COUNTER_RO)
#define AG_MG_REGS_PCE_DLU_FAIL_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLU_FAIL_COUNTER_RO)
#define AG_MG_REGS_PCE_DLU_OOR_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLU_OOR_COUNTER_RO)
#define AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_DLU_SVT_OOR_COUNTER_RO)
#define AG_MG_REGS_PCE_SLU_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SLU_COUNTER_RO)
#define AG_MG_REGS_PCE_SLU_FAIL_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SLU_FAIL_COUNTER_RO)
#define AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV4_CHKSUM_FAIL_CNT_RO)
#define AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDP_CHKSUM_FAIL_CNT_RO)
#define AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDP_CHKSUM_MASK_CNT_RO)
#define AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UNKNOWN_FRAME_CNT_RO)
#define AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_PARSER_FLUSH_COUNTER_RO)
#define AG_MG_REGS_PCE_UDL_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDL_COUNTER_RO)
#define AG_MG_REGS_PCE_V2_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_V2_COUNTER_RO)
#define AG_MG_REGS_PCE_VLAN_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_VLAN_COUNTER_RO)
#define AG_MG_REGS_PCE_SNAP_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_SNAP_COUNTER_RO)
#define AG_MG_REGS_PCE_IPV4_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV4_COUNTER_RO)
#define AG_MG_REGS_PCE_IPV6_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV6_COUNTER_RO)
#define AG_MG_REGS_PCE_UDP_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDP_COUNTER_RO)
#define AG_MG_REGS_PCE_UDPLITE_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_UDPLITE_COUNTER_RO)
#define AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FDQFULL_DISCARD_CNT_RO)
#define AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_CORRUPTED_FRAME_CNT_RO)
#define AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FOQFULL_DISCARD_CNT_RO)
#define AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAX_SVT_STALL_COUNTER_RO)
#define AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAX_BDT_STALL_COUNTER_RO)
#define AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_PAUSE_FRAME_COUNTER_RO)
#define AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV4_LEN_ERR_COUNTER_RO)
#define AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV6_LEN_ERR_COUNTER_RO)
#define AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV4UDP_LEN_ERR_CNT_RO)
#define AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV6UDP_LEN_ERR_CNT_RO)
#define AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV4UDPLITE_COV_ERR_CNT_RO)
#define AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_IPV6UDPLITE_COV_ERR_CNT_RO)
#define AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_RO)
#define AG_MG_REGS_PCE_BD_BOT_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BD_BOT_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_BD_TOP_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BD_TOP_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_BUF_SENT_COUNTER_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BUF_SENT_COUNTER_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_FRAME_SENT_CNT_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_FRAME_SENT_CNT_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_MAX_BDL_LATENCY_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MAX_BDL_LATENCY_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_BDF_STATUS_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BDF_STATUS_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_BD_INT_STATUS_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BD_INT_STATUS_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_BD_INT_MASK_RA(p,bd)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_BD_INT_MASK_RO)+((bd)*0x20)
#define AG_MG_REGS_PCE_MMR_MATCH_1_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_1_RO)
#define AG_MG_REGS_PCE_MMR_MATCH_2_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_2_RO)
#define AG_MG_REGS_PCE_MMR_MATCH_3_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_3_RO)
#define AG_MG_REGS_PCE_MMR_MATCH_4_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_4_RO)
#define AG_MG_REGS_PCE_MMR_MATCH_5_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_5_RO)
#define AG_MG_REGS_PCE_MMR_MATCH_6_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_6_RO)
#define AG_MG_REGS_PCE_MMR_MATCH_7_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MATCH_7_RO)
#define AG_MG_REGS_PCE_MMR_MASK_1_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_1_RO)
#define AG_MG_REGS_PCE_MMR_MASK_2_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_2_RO)
#define AG_MG_REGS_PCE_MMR_MASK_3_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_3_RO)
#define AG_MG_REGS_PCE_MMR_MASK_4_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_4_RO)
#define AG_MG_REGS_PCE_MMR_MASK_5_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_5_RO)
#define AG_MG_REGS_PCE_MMR_MASK_6_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_6_RO)
#define AG_MG_REGS_PCE_MMR_MASK_7_RA(p)	AG_MG_REGS_PCE_REG(p,AG_MG_REGS_PCE_MMR_MASK_7_RO)

#define AG_MG_REGS_PCE_MMR_MASK_RPT     8

#define AG_MG_REGS_PCE_MMR_PATTERN_RPT  8
#define AG_MG_REGS_PCE_MMR_PATTERN_IVL      ((AG_MG_REGS_PCE_MMR_MATCH_RPT * AG_MG_REGS_PCE_MMR_MATCH_IVL)       + (AG_MG_REGS_PCE_MMR_MASK_RPT * AG_MG_REGS_PCE_MMR_MASK_IVL))
// NOTE: pce can range from 0 to 1,
//       pat can range from 0 to (AG_MG_REGS_PCE_MMR_PATTERN_RPT - 1)
#define AG_MG_REGS_PCE_MMR_PATTERN_RA(pce,pat)      AG_MG_REGS_PCE_REG(pce,(pat*AG_MG_REGS_PCE_MMR_PATTERN_IVL)+AG_MG_REGS_PCE_MMR_MATCH_RO)

// NOTE: pce can range from 0 to 1,
//       pat can range from 0 to (AG_MG_REGS_PCE_MMR_PATTERN_RPT - 1),
//       m can range from 0 to (AG_MG_REGS_PCE_MMR_MATCH_RPT - 1)
#define AG_MG_REGS_PCE_MMR_MATCH_RA(pce,pat,m)      (AG_MG_REGS_PCE_MMR_PATTERN_RA(pce,pat) + (m*AG_MG_REGS_PCE_MMR_MATCH_IVL))

// NOTE: pce can range from 0 to 1,
//       pat can range from 0 to (AG_MG_REGS_PCE_MMR_PATTERN_RPT - 1),
//       m can range from 0 to (AG_MG_REGS_PCE_MMR_MASK_RPT - 1)
#define AG_MG_REGS_PCE_MMR_MASK_RA(pce,pat,m)      (AG_MG_REGS_PCE_MMR_PATTERN_RA(pce,pat) +      (AG_MG_REGS_PCE_MMR_MATCH_RPT * AG_MG_REGS_PCE_MMR_MATCH_IVL) +      (m*AG_MG_REGS_PCE_MMR_MASK_IVL))

#define AG_MG_REGS_PCE_UPR_RPT  4
#define AG_MG_REGS_PCE_UPR_IVL  8
// NOTE: pce can range from 0 to 1,
//       u can range from 0 to (AG_MG_REGS_PCE_UPR_RPT - 1)
#define AG_MG_REGS_PCE_UPR_INDEX_ADJUST_RA(pce,u)     AG_MG_REGS_PCE_REG(pce,(u*AG_MG_REGS_PCE_UPR_IVL)+AG_MG_REGS_PCE_UPR_INDEX_ADJUST_RO)
#define AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_RA(pce,u)     AG_MG_REGS_PCE_REG(pce,(u*AG_MG_REGS_PCE_UPR_IVL)+AG_MG_REGS_PCE_UPR_PATTERN_CONTROL_RO)

#define AG_MG_REGS_PCE_ULT_BITS_PER_FIELD            2
#define AG_MG_REGS_PCE_ULT_FIELDS_PER_ENTRY          ((AG_MG_REGS_PCE_ULT_ENTRY_IVL*8)/AG_MG_REGS_PCE_ULT_BITS_PER_FIELD)
#define AG_MG_REGS_PCE_ULT_NUM_FIELDS                (AG_MG_REGS_PCE_ULT_FIELDS_PER_ENTRY*AG_MG_REGS_PCE_ULT_ENTRY_RPT)
// NOTE: pce can range from 0 to 1,
//       dq can range from 0 to (AG_MG_REGS_PCE_ULT_NUM_FIELDS - 1)
#define AG_MG_REGS_PCE_ULT_ENTRY_RA(pce,dq)          AG_MG_REGS_PCE_REG(pce,AG_MG_REGS_PCE_ULT_ENTRY_RO+(AG_MG_REGS_PCE_ULT_ENTRY_IVL*((dq)/AG_MG_REGS_PCE_ULT_FIELDS_PER_ENTRY)))
#define AG_MG_REGS_PCE_ULT_FIELD_OFFSET(dq)          ((dq)&(AG_MG_REGS_PCE_ULT_FIELDS_PER_ENTRY - 1))
#define AG_MG_REGS_PCE_ULT_FIELD_BO(dq)              (30 - (2 * AG_MG_REGS_PCE_ULT_FIELD_OFFSET(dq)))
#define AG_MG_REGS_PCE_ULT_FIELD_BM(dq)              (0xC0000000 >> (2 * AG_MG_REGS_PCE_ULT_FIELD_OFFSET(dq)))
#define AG_MG_REGS_PCE_GET_ULT_DQ_BITS(pce,dq)     (((*((unsigned int *)AG_MG_REGS_PCE_ULT_ENTRY_RA(pce,dq))) >> AG_MG_REGS_PCE_ULT_FIELD_BO(dq)) & 3)
#define AG_MG_REGS_PCE_SET_ULT_DQ_BITS(pce,dq,bits)     ((*((unsigned int *)AG_MG_REGS_PCE_ULT_ENTRY_RA(pce,dq))) |=       (((bits)&3) << AG_MG_REGS_PCE_ULT_FIELD_BO(dq)))


#ifdef AG_MG_REGS_USE_C_STRUCTURES
#define AG_MG_REGS_PCE_CONTROL_BOUNDARY                                            (AG_MG_REGS_PCE_MIU_CONTROL_RO + sizeof(ag_mg_regs_pce_miu_control_u))
#define AG_MG_REGS_PCE_CONTROL_STATUS_GAP_SIZE                                     ((AG_MG_REGS_PCE_INTERRUPT_STATUS_0_RO - AG_MG_REGS_PCE_CONTROL_BOUNDARY)         / sizeof(ag_mg_regs_register))

#define AG_MG_REGS_PCE_STATUS_BOUNDARY                                             (AG_MG_REGS_PCE_FWR_STATUS_HI_RO + sizeof(ag_mg_regs_pce_fwr_status_hi_u))
#define AG_MG_REGS_PCE_STATUS_COUNTER_GAP_SIZE                                     ((AG_MG_REGS_PCE_SOF_COUNTER_RO - AG_MG_REGS_PCE_STATUS_BOUNDARY)         / sizeof(ag_mg_regs_register))

#define AG_MG_REGS_PCE_COUNTER_BOUNDARY                                            (AG_MG_REGS_PCE_BDL_EMPTY_DISCARD_CNT_RO + sizeof(ag_mg_regs_pce_bdl_empty_discard_cnt_u))
#define AG_MG_REGS_PCE_COUNTER_MMR_GAP_SIZE                                        ((AG_MG_REGS_PCE_MMR_MATCH_RO - AG_MG_REGS_PCE_COUNTER_BOUNDARY)         / sizeof(ag_mg_regs_register))

#define AG_MG_REGS_PCE_MMR_BOUNDARY                                                (AG_MG_REGS_PCE_MMR_MATCH_RO       + (AG_MG_REGS_PCE_MMR_PATTERN_RPT * AG_MG_REGS_PCE_MMR_PATTERN_IVL))
#define AG_MG_REGS_PCE_MMR_UPR_GAP_SIZE                                            ((AG_MG_REGS_PCE_UPR_INDEX_ADJUST_RO - AG_MG_REGS_PCE_MMR_BOUNDARY)          / sizeof(ag_mg_regs_register))

#define AG_MG_REGS_PCE_UPR_BOUNDARY                                                (AG_MG_REGS_PCE_UPR_INDEX_ADJUST_RO       + (AG_MG_REGS_PCE_UPR_RPT * AG_MG_REGS_PCE_UPR_IVL))
#define AG_MG_REGS_PCE_UPR_ULT_GAP_SIZE                                            ((AG_MG_REGS_PCE_ULT_ENTRY_RO - AG_MG_REGS_PCE_UPR_BOUNDARY)          / sizeof(ag_mg_regs_register))

typedef struct AG_MG_REGS_PCE_PATTERN_S
{
ag_mg_regs_pce_mmr_match_u  match[AG_MG_REGS_PCE_MMR_MATCH_RPT];
ag_mg_regs_pce_mmr_mask_u   mask[AG_MG_REGS_PCE_MMR_MASK_RPT];
} ag_mg_regs_pce_pattern_s;

typedef struct AG_MG_REGS_PCE_UPR_S
{
ag_mg_regs_pce_upr_index_adjust_u     index_adjust;
ag_mg_regs_pce_upr_pattern_control_u  pattern_control;
} ag_mg_regs_pce_upr_s;

typedef struct AG_MG_REGS_PCE_QUE_S
{
	ag_mg_regs_pce_bd_bot_u			pce_bd_bot ;
	ag_mg_regs_pce_bd_top_u 		pce_bd_top ;
	ag_mg_regs_pce_buf_sent_counter_u	pce_buf_sent_counter ;
	ag_mg_regs_pce_frame_sent_cnt_u		pce_frame_sent_cnt ;
	ag_mg_regs_pce_max_bdl_latency_u	pce_max_bdl_latency ;
	ag_mg_regs_pce_bdf_status_u		pce_bdf_status ;
	ag_mg_regs_pce_bd_int_status_u		pce_bd_int_status ;
	ag_mg_regs_pce_bd_int_mask_u		pce_bd_int_mask ;
}ag_mg_regs_pce_que_s;

typedef struct AG_MG_REGS_PCE_REGS_S
{
	ag_mg_regs_pce_dlt_u	pce_dlt[AG_MG_REGS_PCE_DLT_RPT] ;
	ag_mg_regs_pce_control_u	pce_control ;
	ag_mg_regs_pce_mac_dest_local_ms_u	pce_mac_dest_local_ms ;
	ag_mg_regs_pce_mac_dest_local_ls_u	pce_mac_dest_local_ls ;
	ag_mg_regs_pce_mac_dest_remote_ms_u	pce_mac_dest_remote_ms ;
	ag_mg_regs_pce_mac_dest_remote_ls_u	pce_mac_dest_remote_ls ;
	ag_mg_regs_pce_mac_dest_23_ms_u	pce_mac_dest_23_ms ;
	ag_mg_regs_pce_mac_dest_2_ls_u	pce_mac_dest_2_ls ;
	ag_mg_regs_pce_mac_dest_3_ls_u	pce_mac_dest_3_ls ;
	ag_mg_regs_pce_mac_dest_45_ms_u	pce_mac_dest_45_ms ;
	ag_mg_regs_pce_mac_dest_4_ls_u	pce_mac_dest_4_ls ;
	ag_mg_regs_pce_mac_dest_5_ls_u	pce_mac_dest_5_ls ;
	ag_mg_regs_pce_max_frame_size_u	pce_max_frame_size ;
	ag_mg_regs_pce_udp_dest_max_min_porta_u	pce_udp_dest_max_min_porta ;
	ag_mg_regs_pce_udp_dest_max_min_portb_u	pce_udp_dest_max_min_portb ;
	ag_mg_regs_register		RESERVED_1[1] ;
	ag_mg_regs_pce_ipdest0_0_u	pce_ipdest0_0 ;
	ag_mg_regs_pce_ipdest0_1_u	pce_ipdest0_1 ;
	ag_mg_regs_pce_ipdest0_2_u	pce_ipdest0_2 ;
	ag_mg_regs_pce_ipdest0_3_u	pce_ipdest0_3 ;
	ag_mg_regs_pce_ipdest1_0_u	pce_ipdest1_0 ;
	ag_mg_regs_pce_ipdest1_1_u	pce_ipdest1_1 ;
	ag_mg_regs_pce_ipdest1_2_u	pce_ipdest1_2 ;
	ag_mg_regs_pce_ipdest1_3_u	pce_ipdest1_3 ;
	ag_mg_regs_pce_bdl_control_u	pce_bdl_control ;
	ag_mg_regs_pce_svt_top_u	pce_svt_top ;
	ag_mg_regs_pce_svt_bot_u	pce_svt_bot ;
	ag_mg_regs_pce_fdq_control_u	pce_fdq_control ;
	ag_mg_regs_pce_foq_control_u	pce_foq_control ;
	ag_mg_regs_pce_exint_control_u	pce_exint_control ;
	ag_mg_regs_pce_fint_control_u	pce_fint_control ;
	ag_mg_regs_pce_interrupt_status_mask_0_u	pce_interrupt_status_mask_0 ;
	ag_mg_regs_pce_interrupt_status_mask_1_u	pce_interrupt_status_mask_1 ;
	ag_mg_regs_pce_miu_control_u	pce_miu_control ;
	ag_mg_regs_pce_dlt_access_u	pce_dlt_access ;
	ag_mg_regs_pce_bd_ptr_rst_u	pce_bd_ptr_rst ;
	ag_mg_regs_pce_regkey_u	pce_regkey ;
	ag_mg_regs_register		RESERVED_2[92] ;
	ag_mg_regs_pce_interrupt_summary_u	pce_interrupt_summary ;
	ag_mg_regs_pce_interrupt_status_0_u	pce_interrupt_status_0 ;
	ag_mg_regs_pce_interrupt_status_1_u	pce_interrupt_status_1 ;
	ag_mg_regs_pce_cfg_status_u	pce_cfg_status ;
	ag_mg_regs_pce_par_status_u	pce_par_status ;
	ag_mg_regs_pce_dlu_status_u	pce_dlu_status ;
	ag_mg_regs_pce_slu_status_u	pce_slu_status ;
	ag_mg_regs_pce_fwr_status_lo_u	pce_fwr_status_lo ;
	ag_mg_regs_pce_miu_status_u	pce_miu_status ;
	ag_mg_regs_pce_l2c_status_reg_u	pce_l2c_status_reg ;
	ag_mg_regs_pce_fwr_status_hi_u	pce_fwr_status_hi ;
	ag_mg_regs_register		RESERVIED_3[5] ;
	ag_mg_regs_pce_sof_counter_u	pce_sof_counter ;
	ag_mg_regs_pce_eof_counter_u	pce_eof_counter ;
	ag_mg_regs_pce_sof_sof_counter_u	pce_sof_sof_counter ;
	ag_mg_regs_pce_eof_eof_counter_u	pce_eof_eof_counter ;
	ag_mg_regs_pce_rx_byte_counter_u	pce_rx_byte_counter ;
	ag_mg_regs_pce_dlu_counter_u	pce_dlu_counter ;
	ag_mg_regs_pce_dlt_read_err_counter_u	pce_dlt_read_err_counter ;
	ag_mg_regs_pce_dlu_fail_counter_u	pce_dlu_fail_counter ;
	ag_mg_regs_pce_dlu_oor_counter_u	pce_dlu_oor_counter ;
	ag_mg_regs_pce_dlu_svt_oor_counter_u	pce_dlu_svt_oor_counter ;
	ag_mg_regs_pce_slu_counter_u	pce_slu_counter ;
	ag_mg_regs_pce_slu_fail_counter_u	pce_slu_fail_counter ;
	ag_mg_regs_pce_ipv4_chksum_fail_cnt_u	pce_ipv4_chksum_fail_cnt ;
	ag_mg_regs_pce_udp_chksum_fail_cnt_u	pce_udp_chksum_fail_cnt ;
	ag_mg_regs_pce_udp_chksum_mask_cnt_u	pce_udp_chksum_mask_cnt ;
	ag_mg_regs_pce_unknown_frame_cnt_u	pce_unknown_frame_cnt ;
	ag_mg_regs_pce_parser_flush_counter_u	pce_parser_flush_counter ;
	ag_mg_regs_pce_udl_counter_u	pce_udl_counter ;
	ag_mg_regs_pce_v2_counter_u	pce_v2_counter ;
	ag_mg_regs_pce_vlan_counter_u	pce_vlan_counter ;
	ag_mg_regs_pce_snap_counter_u	pce_snap_counter ;
	ag_mg_regs_pce_ipv4_counter_u	pce_ipv4_counter ;
	ag_mg_regs_pce_ipv6_counter_u	pce_ipv6_counter ;
	ag_mg_regs_pce_udp_counter_u	pce_udp_counter ;
	ag_mg_regs_pce_udplite_counter_u	pce_udplite_counter ;
	ag_mg_regs_pce_fdqfull_discard_cnt_u	pce_fdqfull_discard_cnt ;
	ag_mg_regs_pce_corrupted_frame_cnt_u	pce_corrupted_frame_cnt ;
	ag_mg_regs_pce_foqfull_discard_cnt_u	pce_foqfull_discard_cnt ;
	ag_mg_regs_pce_max_svt_stall_counter_u	pce_max_svt_stall_counter ;
	ag_mg_regs_pce_max_bdt_stall_counter_u	pce_max_bdt_stall_counter ;
	ag_mg_regs_pce_pause_frame_counter_u	pce_pause_frame_counter ;
	ag_mg_regs_pce_ipv4_len_err_counter_u	pce_ipv4_len_err_counter ;
	ag_mg_regs_pce_ipv6_len_err_counter_u	pce_ipv6_len_err_counter ;
	ag_mg_regs_pce_ipv4udp_len_err_cnt_u	pce_ipv4udp_len_err_cnt ;
	ag_mg_regs_pce_ipv6udp_len_err_cnt_u	pce_ipv6udp_len_err_cnt ;
	ag_mg_regs_pce_ipv4udplite_cov_err_cnt_u	pce_ipv4udplite_cov_err_cnt ;
	ag_mg_regs_pce_ipv6udplite_cov_err_cnt_u	pce_ipv6udplite_cov_err_cnt ;
	ag_mg_regs_pce_bdl_empty_discard_cnt_u	pce_bdl_empty_discard_cnt ;
	ag_mg_regs_register		RESERVED_4[74] ;
	ag_mg_regs_pce_que_s 		pce_q[6];
	ag_mg_regs_register		RESERVED_5[208] ;
	ag_mg_regs_pce_mmr_match_u	pce_mmr0_match[AG_MG_REGS_PCE_MMR_MATCH_RPT] ;
	ag_mg_regs_pce_mmr_mask_u	pce_mmr0_mask[AG_MG_REGS_PCE_MMR_MASK_RPT] ;
	ag_mg_regs_pce_mmr_match_u	pce_mmr1_match[AG_MG_REGS_PCE_MMR_MATCH_RPT] ;
	ag_mg_regs_pce_mmr_mask_u	pce_mmr1_mask[AG_MG_REGS_PCE_MMR_MASK_RPT] ;
	ag_mg_regs_pce_mmr_match_u	pce_mmr2_match[AG_MG_REGS_PCE_MMR_MATCH_RPT] ;
	ag_mg_regs_pce_mmr_mask_u	pce_mmr2_mask[AG_MG_REGS_PCE_MMR_MASK_RPT] ;
	ag_mg_regs_pce_mmr_match_u	pce_mmr3_match[AG_MG_REGS_PCE_MMR_MATCH_RPT] ;
	ag_mg_regs_pce_mmr_mask_u	pce_mmr3_mask[AG_MG_REGS_PCE_MMR_MASK_RPT] ;
	ag_mg_regs_pce_upr_index_adjust_u	pce_upr0_index_adjust ;
	ag_mg_regs_pce_upr_pattern_control_u	pce_upr0_pattern_control ;
	ag_mg_regs_pce_upr_index_adjust_u	pce_upr1_index_adjust ;
	ag_mg_regs_pce_upr_pattern_control_u	pce_upr1_pattern_control ;
	ag_mg_regs_pce_upr_index_adjust_u	pce_upr2_index_adjust ;
	ag_mg_regs_pce_upr_pattern_control_u	pce_upr2_pattern_control ;
	ag_mg_regs_pce_upr_index_adjust_u	pce_upr3_index_adjust ;
	ag_mg_regs_pce_upr_pattern_control_u	pce_upr3_pattern_control ;
	ag_mg_regs_register		RESERVED_6[184] ;
	ag_mg_regs_pce_ult_entry_u	ag_mg_regs_pce_ult_entry[AG_MG_REGS_PCE_ULT_ENTRY_RPT] ;
} ag_mg_regs_pce_reg_s ;

/*
* Recommended C syntax for typical usage :
*   volatile ag_mg_regs_pce_reg_s *pceM_regs =
*       (volatile ag_mg_regs_pce_reg_s *)(AG_MG_REGS_PCE_BASE + (0x8000*M));
* where M = 0 or 1
*/
#endif

#endif

/******** History ********
$Log: ag_mg_regs_pce.h,v $
Revision 1.1  2012/04/18 18:08:26  srane
Initial checkin


$Endlog$
*/


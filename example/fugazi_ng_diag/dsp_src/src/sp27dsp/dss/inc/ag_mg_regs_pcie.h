/* $Id: ag_mg_regs_pcie.h,v 1.1 2012/04/18 18:08:26 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_mg_regs_pcie.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_pcie.h
 *      Graffham - DSS uart 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*
 * ag_mg_regs_pcie.h
 *
 * Copyright (c) 2011 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * The PCIE module contains the Root Complex (RC) Application, Application RC & End Point (EP),
 * Transaction (TL), Link (LL) and Pipe Layers.
 *
 * A separate header file (ag_mg_regs_serdes.h) defined the SerDes interface connected to the PCIE
 * to form the PCI-Express solution for the SP2704 design. 
 *
 * Registers defined in this file:
 *  PCIE Offset		DBM AHB Space	NAME 			Layer - Mode
 *  0X0000_0000 	0X9B01_0000 	REVISION_VENDOR_ID 	AP - Configuration -RC Mode Only
 *  0X0000_0004 	0X9B01_0004 	CMD_STATUS 		AP - Configuration -RC Mode Only
 *  0X0000_0008 	0X9B01_0008 	REVISION_CODE 		AP - Configuration -RC Mode Only
 *  0X0000_000C 	0X9B01_000C 	CLINE_LATENCY_HT_BIST 	AP - Configuration -RC Mode Only
 *  0X0000_0010 	0X9B01_0010 	BASE_ADDR0 		AP - Configuration -RC Mode Only
 *  0X0000_0014 	0X9B01_0014 	BASE_ADDR1 		AP - Configuration -RC Mode Only
 *  0X0000_0018 	0X9B01_0018 	BUS_NUMBER 		AP - Configuration -RC Mode Only
 *  0X0000_001C 	0X9B01_001C 	IOLIMIT_SEC_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_0020 	0X9B01_0020 	MEMORY_BASE_LIMIT 	AP - Configuration -RC Mode Only
 *  0X0000_0024 	0X9B01_0024 	PREFETCHABLE_MEMORY_BASE_LIMIT 	AP - Configuration -RC Mode Only
 *  0X0000_0028 	0X9B01_0028 	PREFETCHABLE_BASE_U32BITS 	AP - Configuration -RC Mode Only
 *  0X0000_002C 	0X9B01_002C 	PREFETCHABLE_LIMIT_U32BITS 	AP - Configuration -RC Mode Only
 *  0X0000_0030 	0X9B01_0030 	IO_LIMIT_BASE_U32BITS 	AP - Configuration -RC Mode Only
 *  0X0000_0034 	0X9B01_0034 	CAPABILITIES_PTR 	AP - Configuration -RC Mode Only
 *  0X0000_0038 	0X9B01_0038 	EXPANSION_ROM_BASE_ADDR 	AP - Configuration -RC Mode Only
 *  0X0000_003C 	0X9B01_003C 	INTERRUPT_BCTRL 	AP - Configuration -RC Mode Only
 *  0X0000_0040 	0X9B01_0040 	PME_CAPABILITY 		AP - Configuration -RC Mode Only
 *  0X0000_0044 	0X9B01_0044 	PME_CTRL_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_0060 	0X9B01_0060 	CAPABILITY 		AP - Configuration -RC Mode Only
 *  0X0000_0064 	0X9B01_0064 	DEV_CAPABILITIES 	AP - Configuration -RC Mode Only
 *  0X0000_0068 	0X9B01_0068 	DEV_CTRL_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_006C 	0X9B01_006C 	LINK_CAPABILITIES_APRC 	AP - Configuration -RC Mode Only
 *  0X0000_0070 	0X9B01_0070 	LINK_CTRL_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_0074 	0X9B01_0074 	SLOT_CAPABILITIES 	AP - Configuration -RC Mode Only
 *  0X0000_0078 	0X9B01_0078 	SLOT_CTRL_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_007C 	0X9B01_007C 	ROOT_CTRL_CAPABILITIES 	AP - Configuration -RC Mode Only
 *  0X0000_0084 	0X9B01_0084 	DEV_CAPABILITIES2 	AP - Configuration -RC Mode Only
 *  0X0000_0088 	0X9B01_0088 	DEV_CTRL_STATUS2 	AP - Configuration -RC Mode Only
 *  0X0000_008C 	0X9B01_008C 	LINK_CAP2 		AP - Configuration -RC Mode Only
 *  0X0000_0090 	0X9B01_0090 	LINK_CTRL_STATUS2 	AP - Configuration -RC Mode Only
 *  0X0000_0094 	0X9B01_0094 	DEV_CAP_RSVD1 		AP - Configuration -RC Mode Only
 *  0X0000_0098 	0X9B01_0098 	DEV_CAP_RSVD2 		AP - Configuration -RC Mode Only
 *  0X0000_0100 	0X9B01_0100 	PCIE_ENHANCED_CAP_HEADER_APRC 	AP - Configuration -RC Mode Only
 *  0X0000_0104 	0X9B01_0104 	UNC_ERR_STATUS 		AP - Configuration -RC Mode Only
 *  0X0000_0108 	0X9B01_0108 	UNC_ERR_MASK 		AP - Configuration -RC Mode Only
 *  0X0000_010C 	0X9B01_010C 	UNC_ERR_SEVERITY 	AP - Configuration -RC Mode Only
 *  0X0000_0110 	0X9B01_0110 	CORR_ERR_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_0114 	0X9B01_0114 	CORR_ERR_MASK 		AP - Configuration -RC Mode Only
 *  0X0000_0118 	0X9B01_0118 	ERR_CAP_CTRL 		AP - Configuration -RC Mode Only
 *  0X0000_011C 	0X9B01_011C 	HEADER_LOG1 		AP - Configuration -RC Mode Only
 *  0X0000_0120 	0X9B01_0120 	HEADER_LOG2 		AP - Configuration -RC Mode Only
 *  0X0000_0124 	0X9B01_0124 	HEADER_LOG3 		AP - Configuration -RC Mode Only
 *  0X0000_0128 	0X9B01_0128 	HEADER_LOG4 		AP - Configuration -RC Mode Only
 *  0X0000_012C 	0X9B01_012C 	ROOT_ERR_CMD 		AP - Configuration -RC Mode Only
 *  0X0000_0130 	0X9B01_0130 	ROOT_ERR_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_0134 	0X9B01_0134 	ERR_SRC_ID 		AP - Configuration -RC Mode Only
 *  0X0000_0140 	0X9B01_0140 	VC_CHANNEL_CAP_HEADER 	AP - Configuration -RC Mode Only
 *  0X0000_0144 	0X9B01_0144 	PORT_VC_CAPABILITY1 	AP - Configuration -RC Mode Only
 *  0X0000_0148 	0X9B01_0148 	PORT_VC_CAPAILITY2 	AP - Configuration -RC Mode Only
 *  0X0000_014C 	0X9B01_014C 	PORT_VC_CTRL_STATUS 	AP - Configuration -RC Mode Only
 *  0X0000_0150 	0X9B01_0150 	VC_RESOURCE_CAP 	AP - Configuration -RC Mode Only
 *  0X0000_0154 	0X9B01_0154 	VC_RESPONSE_CTRL 	AP - Configuration -RC Mode Only
 *  0X0000_0158 	0X9B01_0158 	VC_RESOURCE_STATUS 	AP - Configuration -RC Mode Only
 *  0x0000_1000 	0x9B01_1000 	Configure_reg 		AP- DSR - RC & EP Mode
 *  0x0000_1004 	0x9B01_1004 	status_reg 		AP- DSR - RC & EP Mode
 *  0x0000_1008 	0x9B01_1008 	core_debug_reg 		AP- DSR - RC & EP Mode
 *  0x0000_100C 	0x9B01_100C 	loopback_fail_status 	AP- DSR - RC & EP Mode
 *  0x0000_1010 	0x9B01_1010 	mpage0_up 		AP- DSR - RC & EP Mode
 *  0x0000_1014 	0x9B01_1014 	mpage0_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1018 	0x9B01_1018 	mpage1_up 		AP- DSR - RC & EP Mode
 *  0x0000_101C 	0x9B01_101C 	mpage1_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1020 	0x9B01_1020 	mpage2_up 		AP- DSR - RC & EP Mode
 *  0x0000_1024 	0x9B01_1024 	mpage2_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1028 	0x9B01_1028 	mpage3_up 		AP- DSR - RC & EP Mode
 *  0x0000_102C 	0x9B01_102C 	mpage3_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1030 	0x9B01_1030 	mpage4_up 		AP- DSR - RC & EP Mode
 *  0x0000_1034 	0x9B01_1034 	mpage4_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1038 	0x9B01_1038 	mpage5_up 		AP- DSR - RC & EP Mode
 *  0x0000_103C 	0x9B01_103C 	mpage5_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1040 	0x9B01_1040 	mpage6_up 		AP- DSR - RC & EP Mode
 *  0x0000_1044 	0x9B01_1044 	mpage6_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1048 	0x9B01_1048 	mpage7_up 		AP- DSR - RC & EP Mode
 *  0x0000_104C 	0x9B01_104C 	mpage7_lo 		AP- DSR - RC & EP Mode
 *  0x0000_1050 	0x9B01_1050 	tpage0_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1054 	0x9B01_1054 	tpage1_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1058 	0x9B01_1058 	tpage2_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_105C 	0x9B01_105C 	tpage3_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1060 	0x9B01_1060 	tpage4_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1064 	0x9B01_1064 	tpage5_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1068 	0x9B01_1068 	tpage6_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_106C 	0x9B01_106C 	tpage7_bar0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1070 	0x9B01_1070 	tpage0_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1074 	0x9B01_1074 	tpage1_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1078 	0x9B01_1078 	tpage2_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_107C 	0x9B01_107C 	tpage3_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1080 	0x9B01_1080 	tpage4_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1084 	0x9B01_1084 	tpage5_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1088 	0x9B01_1088 	tpage6_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_108C 	0x9B01_108C 	tpage7_bar1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1090 	0x9B01_1090 	tpage0_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1094 	0x9B01_1094 	tpage1_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1098 	0x9B01_1098 	tpage2_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_109C 	0x9B01_109C 	tpage3_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10A0 	0x9B01_10A0 	tpage4_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10A4 	0x9B01_10A4 	tpage5_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10A8 	0x9B01_10A8 	tpage6_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10AC 	0x9B01_10AC 	tpage7_bar2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10B0 	0x9B01_10B0 	message_in_fifo 	AP- DSR - RC & EP Mode
 *  0x0000_10B4 	0x9B01_10B4 	message_in_fifo_status 	AP- DSR - RC & EP Mode
 *  0x0000_10B8 	0x9B01_10B8 	message_out 		AP- DSR - RC & EP Mode
 *  0x0000_10C0 	0x9B01_10C0 	PCIE_Interrupt_Status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10C4 	0x9B01_10C4 	PCIE_Interrupt_Enable_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10C8 	0x9B01_10C8 	PCIE_Interrupt_Force_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10CC 	0x9B01_10CC 	phy_pcie_sta0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10D0 	0x9B01_10D0 	phy_pcie_sta1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10D4 	0x9B01_10D4 	pcie_phy_ctrl0_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10D8 	0x9B01_10D8 	pcie_phy_ctrl1_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10DC 	0x9B01_10DC 	pcie_phy_ctrl2_reg 	AP- DSR - RC & EP Mode
 *  0x0000_10E0 	0x9B01_10E0 	conf_wr_cpl_tmp_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_10E4 	0x9B01_10E4 	dec_error_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_10E8 	0x9B01_10E8 	message_in_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_10EC 	0x9B01_10EC 	pcie_reserved1 		AP- DSR - RC & EP Mode
 *  0x0000_10F0 	0x9B01_10F0 	mr_cpl_data_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_10F4 	0x9B01_10F4 	mr_cpl_hdr_fifo_dtat 	AP- DSR - RC & EP Mode
 *  0x0000_10F8 	0x9B01_10F8 	mr_req_hdr_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_10FC 	0x9B01_10FC 	mw_data_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1100 	0x9B01_1100 	rx_cpl_tmp_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1104 	0x9B01_1104 	slv_rd_compl_aligned_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1108 	0x9B01_1108 	slv_rx_cpl_err_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_110C 	0x9B01_110C 	sr_hdr_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1110 	0x9B01_1110 	sw_conf_hdr_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1114 	0x9B01_1114 	sw_data_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1118 	0x9B01_1118 	sw_hdr_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_111C 	0x9B01_111C 	slv_rd_compl_timeout_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1120 	0x9B01_1120 	axi_id_freelist_fifo_status_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1124 	0x9B01_1124 	assigned_axi_id_fifo_status_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1128 	0x9B01_1128 	slv_rd_cpl_0_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_112C 	0x9B01_112C 	slv_rd_cpl_1_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1130 	0x9B01_1130 	slv_rd_cpl_2_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1134 	0x9B01_1134 	slv_rd_cpl_3_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1138 	0x9B01_1138 	slv_rd_cpl_4_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_113C 	0x9B01_113C 	slv_rd_cpl_5_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1140 	0x9B01_1140 	slv_rd_cpl_6_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1144 	0x9B01_1144 	slv_rd_cpl_7_fifo_stat 	AP- DSR - RC & EP Mode
 *  0x0000_1148 	0x9B01_1148 	slv_wrt_conf_wrt_rtn_timeout_id_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_114C 	0x9B01_114C 	slv_wrt_conf_wrt_rtn_pkt_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1150 	0x9B01_1150 	slv_wrt_conf_wrt_rtn_compl_id_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1154 	0x9B01_1154 	slv_rd_compl_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1158 	0x9B01_1158 	slv_rd_cpl_timeout_completer_id_reg 	AP- DSR - RC & EP Mode
 *  0x0000_115C 	0x9B01_115C 	tl_fsm_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1160 	0x9B01_1160 	t2a_egr_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1164 	0x9B01_1164 	t2a_req_proc_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1168 	0x9B01_1168 	t2a_cpl_to_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_116C 	0x9B01_116C 	t2a_igr_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1170 	0x9B01_1170 	t2a_fn_indp_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1174 	0x9B01_1174 	t2a_fn_indp_other_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1178 	0x9B01_1178 	t2a_parity_err_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_117C 	0x9B01_117C 	config_link_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1180 	0x9B01_1180 	captured_mw_err_resp_bar_addr_status_reg 	AP- DSR - RC & EP Mode
 *  0x0000_1184 	0x9B01_1184 	pcie_reserved2 		AP- DSR - RC & EP Mode
 *  0x0000_1188 	0x9B01_1188 	pcie_reserved3 		AP- DSR - RC & EP Mode
 *  0x0000_118C 	0x9B01_118C 	l2t_ce_status 		AP- DSR - RC & EP Mode
 *  0x0000_1190 	0x9B01_1190 	axi_addr_for_msi_notification 	AP- DSR - RC & EP Mode
 *  0x0000_1194 	0x9B01_1194 	mgw_cpu_int_msi_vectors 	AP- DSR - RC & EP Mode
 *  0x0000_1198 	0x9B01_1198 	MGW_Interrupt_Status_Register 	AP- DSR - RC & EP Mode
 *  0x0000_119C 	0x9B01_119C 	MGW_Interrupt_Enable_Register 	AP- DSR - RC & EP Mode
 *  0x0000_11A0 	0x9B01_11A0 	MGW_Interrupt_Force_Register 	AP- DSR - RC & EP Mode
 *  0x0000_2000 	0x9B01_2000 	Device_and_vendor_ID 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2004 	0x9B01_2004 	Status_and_Command 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2008 	0x9B01_2008 	Class_and_Revision 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_200C 	0x9B01_200C 	Test_Configuration_Register 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2010 	0x9B01_2010 	BAR0_Lo 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2014 	0x9B01_2014 	BAR0_Hi 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2018 	0x9B01_2018 	BAR1_Lo 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_201C 	0x9B01_201C 	BAR1_Hi 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2020 	0x9B01_2020 	BAR2_Lo 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2024 	0x9B01_2024 	BAR2_Hi 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2028 	0x9B01_2028 	CARDBUS 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_202C 	0x9B01_202C 	Subsystem 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2030 	0x9B01_2030 	Expansion 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2034 	0x9B01_2034 	Capabilities_Pointer_reg 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_203C 	0x9B01_203C 	Interrupt 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2040 	0x9B01_2040 	PCIE_PWR_Mgn_Capabilities 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2044 	0x9B01_2044 	PCIE_PWR_Mgn_Status_Ctrl 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2050 	0x9B01_2050 	MESSAGE_CONTROL 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2054 	0x9B01_2054 	TABLE_OFFSET_BIR 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2058 	0x9B01_2058 	PBA_Offset 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2060 	0x9B01_2060 	PCIE_Capabilities 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2064 	0x9B01_2064 	Device_capabilities 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2068 	0x9B01_2068 	Device_Status_and_Control 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_206C 	0x9B01_206C 	Link_Capabilities 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2070 	0x9B01_2070 	Link_Status_and_Ctrl 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2084 	0x9B01_2084 	Device_Cap2 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2088 	0x9B01_2088 	Device_Ctrl_2 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2090 	0x9B01_2090 	Link_Status_and_Control_2 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2100 	0x9B01_2100 	Advanced_Error_Report_Capability_Header 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2104 	0x9B01_2104 	Uncorrectable_Error_Status 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2108 	0x9B01_2108 	Uncorrectable_Error_Mask 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_210C 	0x9B01_210C 	Uncorrectable_Error_Sevrerity 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2110 	0x9B01_2110 	Correctable_Error_Status 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2114 	0x9B01_2114 	Correctable_Error_Mask 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2118 	0x9B01_2118 	Adv_Error_cap_and_Ctrl 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_211C 	0x9B01_211C 	Header_Log_register0 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2120 	0x9B01_2120 	Head_Log_Register1 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2124 	0x9B01_2124 	Head_Log_Register2 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2128 	0x9B01_2128 	Head_Log_Register3 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2140 	0x9B01_2140 	Enhanced_Capability_Header 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2144 	0x9B01_2144 	Port_VC_Capability_reg_1 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2148 	0x9B01_2148 	Port_VC_Cap_Reg_2 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_214C 	0x9B01_214C 	Port_VC_Control_Status 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2150 	0x9B01_2150 	VC_Resource_Cap_Reg_0 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2154 	0x9B01_2154 	VC_Resource_Control_Reg_0 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2158 	0x9B01_2158 	VC_Resource_Status_Reg_0 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2180 	0x9B01_2180 	Enhanced_cap_Header 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2184 	0x9B01_2184 	Data_Select_Reg 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2188 	0x9B01_2188 	Data_Register 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_218C 	0x9B01_218C 	PWR_Budget_Cap 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2190 	0x9B01_2190 	PCIE_Enhanced_Cap_Header 	TL - LSI Core Configuration - EP Mode Only
 *  0x0000_2194 	0x9B01_2194 	ARI_CAP_CTRL 		TL - LSI Core Configuration - EP Mode Only
 *  0x0000_3000 	0x9B01_3000 	Link_Core_ID 		LL. Specific Registers - RC & EP
 *  0x0000_3004 	0x9B01_3004 	Link_Status 		LL. Specific Registers - RC & EP
 *  0x0000_3008 	0x9B01_3008 	Interrupt_Status 	LL. Specific Registers - RC & EP
 *  0x0000_300C 	0x9B01_300C 	Performance_Counter_Interrupt_Test 	LL. Specific Registers - RC & EP
 *  0x0000_3010 	0x9B01_3010 	Interrupt_Mask 		LL. Specific Registers - RC & EP
 *  0x0000_3020 	0x9B01_3020 	Test_Control1 		LL. Specific Registers - RC & EP
 *  0x0000_3024 	0x9B01_3024 	Test_Control2 		LL. Specific Registers - RC & EP
 *  0x0000_3028 	0x9B01_3028 	Test_Data 		LL. Specific Registers - RC & EP
 *  0x0000_3030 	0x9B01_3030 	Memory_Address_Control 	LL. Specific Registers - RC & EP
 *  0x0000_3034 	0x9B01_3034 	MemoryDataLoad0 	LL. Specific Registers - RC & EP
 *  0x0000_3038 	0x9B01_3038 	MemoryDataLoad1 	LL. Specific Registers - RC & EP
 *  0x0000_303C 	0x9B01_303C 	MemoryDataLoad2 	LL. Specific Registers - RC & EP
 *  0x0000_3040 	0x9B01_3040 	MemeyDataLoad3 		LL. Specific Registers - RC & EP
 *  0x0000_3044 	0x9B01_3044 	MemoryDataLoad4 	LL. Specific Registers - RC & EP
 *  0x0000_3080 	0x9B01_3080 	LinkPerformanceCounterSelect 	LL. Specific Registers - RC & EP
 *  0x0000_3084 	0x9B01_3084 	LinkPerformanceCounterControl 	LL. Specific Registers - RC & EP
 *  0x0000_3088 	0x9B01_3088 	LinkPerformanceCounterSampleDuration 	LL. Specific Registers - RC & EP
 *  0x0000_3090 	0x9B01_3090 	LinkPerformanceCounter1 	LL. Specific Registers - RC & EP
 *  0x0000_3094 	0x9B01_3094 	LinkPerformanceCounter1_Test 	LL. Specific Registers - RC & EP
 *  0x0000_3098 	0x9B01_3098 	LinkPerformanceCounter_2 	LL. Specific Registers - RC & EP
 *  0x0000_309C 	0x9B01_309C 	PerformanceCounter2_Test 	LL. Specific Registers - RC & EP
 *  0x0000_30A0 	0x9B01_30A0 	Debug_Status 		LL. Specific Registers - RC & EP
 *  0x0000_30A4 	0x9B01_30A4 	DebugConfiguration 	LL. Specific Registers - RC & EP
 *  0x0000_3100 	0x9B01_3100 	TX_Configuration 	LL - Transmit Registers - RC & EP
 *  0x0000_3104 	0x9B01_3104 	TX_Link_Status 		LL - Transmit Registers - RC & EP
 *  0x0000_3108 	0x9B01_3108 	TX_Interrupt_and_Status 	LL - Transmit Registers - RC & EP
 *  0x0000_310C 	0x9B01_310C 	TX_Interrupt_and_Status_Test 	LL - Transmit Registers - RC & EP
 *  0x0000_3110 	0x9B01_3110 	TX_Interrupt_Mask 	LL - Transmit Registers - RC & EP
 *  0x0000_3120 	0x9B01_3120 	Flow_Control_Update_Timeout_Value 	LL - Transmit Registers - RC & EP
 *  0x0000_3130 	0x9B01_3130 	ACK_NAK_Latency_Threshold 	LL - Transmit Registers - RC & EP
 *  0x0000_3134 	0x9B01_3134 	ReplayTimeoutThreshold 	LL - Transmit Registers - RC & EP
 *  0x0000_3138 	0x9B01_3138 	Replay_Number_Status 	LL - Transmit Registers - RC & EP
 *  0x0000_3140 	0x9B01_3140 	Retry_Buffer_Pointer 	LL - Transmit Registers - RC & EP
 *  0x0000_3144 	0x9B01_3144 	Sequence_Counter 	LL - Transmit Registers - RC & EP
 *  0x0000_3148 	0x9B01_3148 	Sequence_Buffer_Pointers 	LL - Transmit Registers - RC & EP
 *  0x0000_3150 	0x9B01_3150 	SkipTimerThreshold 	LL - Transmit Registers - RC & EP
 *  0x0000_3154 	0x9B01_3154 	EIES_Counter_Threshold 	LL - Transmit Registers - RC & EP
 *  0x0000_3200 	0x9B01_3200 	RX_Configuration 	LL -Receiver Configuration - RC & EP
 *  0x0000_3204 	0x9B01_3204 	RX_Status 		LL -Receiver Configuration - RC & EP
 *  0x0000_3208 	0x9B01_3208 	RX_Interrupt_and_Status 	LL -Receiver Configuration - RC & EP
 *  0x0000_320C 	0x9B01_320C 	RX_Interrupt_Status_Test 	LL -Receiver Configuration - RC & EP
 *  0x0000_3210 	0x9B01_3210 	RX_Interrupt_Mask 	LL -Receiver Configuration - RC & EP
 *  0x0000_3214 	0x9B01_3214 	RX_TS_Control 		LL -Receiver Configuration - RC & EP
 *  0x0000_3218 	0x9B01_3218 	Next_RCV_Sequence_Counter 	LL -Receiver Configuration - RC & EP
 *  0x0000_321C 	0x9B01_321C 	UnknownDLLP0_Received 	LL -Receiver Configuration - RC & EP
 *  0x0000_3220 	0x9B01_3220 	Unknown_DLLP1_Received 	LL -Receiver Configuration - RC & EP
 *  0x0000_3300 	0x9B01_3300 	LTSSM_Configuration_1 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3304 	0x9B01_3304 	LTSSM_Status 		LL - Training & Status Machine - RC & EP 
 *  0x0000_3308 	0x9B01_3308 	LTSSM_Interupt_and_Status 	LL - Training & Status Machine - RC & EP 
 *  0x0000_330C 	0x9B01_330C 	LTSSM_Interrupt_Test 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3310 	0x9B01_3310 	LTSSM_Interrupt_Mask 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3320 	0x9B01_3320 	LTSSM_Timer_Threshold1 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3324 	0x9B01_3324 	LTSSm_Timer_Threshold2 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3328 	0x9B01_3328 	LTSSM_Timer_Threshold3 	LL - Training & Status Machine - RC & EP
 *  0x0000_332C 	0x9B01_332C 	LTSSM_Threshold4 	LL - Training & Status Machine - RC & EP
 *  0x0000_3330 	0x9B01_3330 	LTSSM_REQUEST 		LL - Training & Status Machine - RC & EP 
 *  0x0000_3334 	0x9B01_3334 	LTSSM_Training_Configuration 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3338 	0x9B01_3338 	LTSSM_Status_2 		LL - Training & Status Machine - RC & EP 
 *  0x0000_333C 	0x9B01_333C 	LTSSM_RX_Command_Status 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3340 	0x9B01_3340 	LTSSM_TX_Command_Status 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3350 	0x9B01_3350 	LTSSM_GEN_2_Timer_Threshold1 	LL - Training & Status Machine - RC & EP
 *  0x0000_3354 	0x9B01_3354 	LSSM_GEN_2_Timer_THRESHOLD2 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3358 	0x9B01_3358 	LTSSM_Gen2_Speed_NFTS_NUMBER 	LL - Training & Status Machine - RC & EP 
 *  0x0000_3E00 	0x9B01_3E00 	TL_Gen_Debug_Reg 		TL - DSR Core - EP & RC Mode
 *  0x0000_3E04 	0x9B01_3E04 	TLSB_SIG 			TL - DSR Core - EP & RC Mode
 *  0x0000_3E08 	0x9B01_3E08 	Completion_Time_Out_Register0 	TL - DSR Core - EP & RC Mode
 *  0x0000_3E0C 	0x9B01_3E0C 	Completion_Time_Out_Register1 	TL - DSR Core - EP & RC Mode
 *  0x0000_3E40 	0x9B01_3E40 	VC0_Ingress_FC_Control_Reg 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E44 	0x9B01_3E44 	VC0_Posted_Prog_Credit_Freed_Threshold 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E48 	0x9B01_3E48 	VC0_NonPosted_Programmable_Credit_Freed_Threshold 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E4C 	0x9B01_3E4C 	VC0_Ingress_FC_Posted_Credits_Received 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E50 	0x9B01_3E50 	VC0_Ingress_FC_NonPosted_Credits_Received 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E54 	0x9B01_3E54 	VC0_Ingress_FC_Posted_Credits_Allocated 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E58 	0x9B01_3E58 	VC0_Ingress_FC_NonPosted_Credits_allocated 	LL - Ingress Flow Control - Ep & RC
 *  0x0000_3E80 	0x9B01_3E80 	VC0_Egress_Posted_Credits_Consumed 	LL - Egress Flow Control - EP & RC
 *  0x0000_3E84 	0x9B01_3E84 	VC0_Egress_NonPosted_Credits_Consumed 	LL - Egress Flow Control - EP & RC
 *  0x0000_3E88 	0x9B01_3E88 	VC0_Egress_Completion_Credits_Consumed 	LL - Egress Flow Control - EP & RC
 *  0x0000_3E8C 	0x9B01_3E8C 	VC0_Egress_Posted_Credit_Limit_Register 	LL - Egress Flow Control - EP & RC
 *  0x0000_3E90 	0x9B01_3E90 	VC0_Egress_NonPosted_Limit 	LL - Egress Flow Control - EP & RC
 *  0x0000_3E94 	0x9B01_3E94 	VC0_Egress_Completion_Limit 	LL - Egress Flow Control - EP & RC
 *  0x0000_3E98 	0x9B01_3E98 	VC0_Egress_Flow_Control_Timeout_Timer 	LL - Egress Flow Control - EP & RC
 *  0x0000_3400 	0x9B01_3400 	PIPE_RST_EN 		Pipe Layer - EP & RC Mode
 *  0x0000_3404 	0x9B01_3404 	PIPE_CONTROL 		Pipe Layer - EP & RC Mode
 *  0x0000_3408 	0x9B01_3408 	PIPE_COMMON_STATUS 	Pipe Layer - EP & RC Mode
 *  0x0000_340C 	0x9B01_340C 	tx_emph_cntrl_g1 	Pipe Layer - EP & RC Mode
 *  0x0000_3410 	0x9B01_3410 	tx_emph_cntrl_g2 	Pipe Layer - EP & RC Mode
 *  0x0000_3414 	0x9B01_3414 	tx_emph_cntrl_g3_1 	Pipe Layer - EP & RC Mode
 *  0x0000_3418 	0x9B01_3418 	tx_emph_cntrl_g3_3 	Pipe Layer - EP & RC Mode
 *  0x0000_341C 	0x9B01_341C 	tx_mrgn_cntrl_g1 	Pipe Layer - EP & RC Mode
 *  0x0000_3420 	0x9B01_3420 	tx_mrgn_cntrl_g2 	Pipe Layer - EP & RC Mode
 *  0x0000_3424 	0x9B01_3424 	tx_mrgn_cntrl_g3 	Pipe Layer - EP & RC Mode
 *  0x0000_3428 	0x9B01_3428 	tx_common_param 	Pipe Layer - EP & RC Mode
 *  0x0000_342C 	0x9B01_342C 	rx_common_param 	Pipe Layer - EP & RC Mode
 *  0x0000_3430 	0x9B01_3430 	rx_common_param_g1_1 	Pipe Layer - EP & RC Mode
 *  0x0000_3434 	0x9B01_3434 	rx_common_param_g1_2 	Pipe Layer - EP & RC Mode
 *  0x0000_3438 	0x9B01_3438 	rx_common_param_g2_1 	Pipe Layer - EP & RC Mode
 *  0x0000_343C 	0x9B01_343C 	rx_common_param_g2_2 	Pipe Layer - EP & RC Mode
 *  0x0000_3440 	0x9B01_3440 	rx_common_param_g3_1 	Pipe Layer - EP & RC Mode
 *  0x0000_3444 	0x9B01_3444 	rx_common_param_g3_2 	Pipe Layer - EP & RC Mode
 *  0x0000_3460 	0x9B01_3460 	rx__param_ln0 		Pipe Layer - EP & RC Mode
 *  0x0000_3464 	0x9B01_3464 	pipe_status_ln0 	Pipe Layer - EP & RC Mode
 *  0x0000_3468 	0x9B01_3468 	rx__param_ln1 		Pipe Layer - EP & RC Mode
 *  0x0000_346C 	0x9B01_346C 	pipe_status_ln1 	Pipe Layer - EP & RC Mode
 * 
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
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

#ifndef AG_MG_REGS_PCIE_REGISTERS_H
#define AG_MG_REGS_PCIE_REGISTERS_H

#include "ag_mg_regs_regops.h"

/* 
 * Generated by HSI Designer release 2.3.5.
 */

/* Application Layer (AP) - RC Mode ONLY section */

 /* 
 * Initialization value: 0xED1111C1  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_REVISION_VENDOR_ID_RO                      0x00000000
#define AG_MG_REGS_REVISION_VENDOR_ID_RM                      0xFFFFFFFF

#define AG_MG_REGS_REVISION_VENDOR_ID_DEVICE_ID_BO            0
#define AG_MG_REGS_REVISION_VENDOR_ID_DEVICE_ID_BM            0x0000FFFF

#define AG_MG_REGS_REVISION_VENDOR_ID_VENDOR_ID_BO            16
#define AG_MG_REGS_REVISION_VENDOR_ID_VENDOR_ID_BM            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_REVISION_VENDOR_ID_U
{
    struct
    {
        ag_mg_regs_register
            device_id : 16,
            vendor_id : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_revision_vendor_id_u;
#endif


/* 
 * Initialization value: 0x00100000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CMD_STATUS_RO                              0x00000004
#define AG_MG_REGS_CMD_STATUS_RM                              0xFFFF07FF

#define AG_MG_REGS_CMD_STATUS_IOACC_BO                        0
#define AG_MG_REGS_CMD_STATUS_IOACC_BM                        0x00000001

#define AG_MG_REGS_CMD_STATUS_MSACC_BO                        1
#define AG_MG_REGS_CMD_STATUS_MSACC_BM                        0x00000002

#define AG_MG_REGS_CMD_STATUS_BME_BO                          2
#define AG_MG_REGS_CMD_STATUS_BME_BM                          0x00000004

#define AG_MG_REGS_CMD_STATUS_SCE_BO                          3
#define AG_MG_REGS_CMD_STATUS_SCE_BM                          0x00000008

#define AG_MG_REGS_CMD_STATUS_MWI_BO                          4
#define AG_MG_REGS_CMD_STATUS_MWI_BM                          0x00000010

#define AG_MG_REGS_CMD_STATUS_VGAPS_BO                        5
#define AG_MG_REGS_CMD_STATUS_VGAPS_BM                        0x00000020

#define AG_MG_REGS_CMD_STATUS_PER_BO                          6
#define AG_MG_REGS_CMD_STATUS_PER_BM                          0x00000040

#define AG_MG_REGS_CMD_STATUS_IDSEL_BO                        7
#define AG_MG_REGS_CMD_STATUS_IDSEL_BM                        0x00000080

#define AG_MG_REGS_CMD_STATUS_SERR_BO                         8
#define AG_MG_REGS_CMD_STATUS_SERR_BM                         0x00000100

#define AG_MG_REGS_CMD_STATUS_FB2BTE_BO                       9
#define AG_MG_REGS_CMD_STATUS_FB2BTE_BM                       0x00000200

#define AG_MG_REGS_CMD_STATUS_ID_BO                           10
#define AG_MG_REGS_CMD_STATUS_ID_BM                           0x00000400

#define AG_MG_REGS_CMD_STATUS_SRVD_BO                         16
#define AG_MG_REGS_CMD_STATUS_SRVD_BM                         0x00070000

#define AG_MG_REGS_CMD_STATUS_IS_BO                           19
#define AG_MG_REGS_CMD_STATUS_IS_BM                           0x00080000

#define AG_MG_REGS_CMD_STATUS_CLIST_BO                        20
#define AG_MG_REGS_CMD_STATUS_CLIST_BM                        0x00100000

#define AG_MG_REGS_CMD_STATUS_MHZC_BO                         21
#define AG_MG_REGS_CMD_STATUS_MHZC_BM                         0x00200000

#define AG_MG_REGS_CMD_STATUS_RVD1_BO                         22
#define AG_MG_REGS_CMD_STATUS_RVD1_BM                         0x00400000

#define AG_MG_REGS_CMD_STATUS_FB2BTC_BO                       23
#define AG_MG_REGS_CMD_STATUS_FB2BTC_BM                       0x00800000

#define AG_MG_REGS_CMD_STATUS_MDPE_BO                         24
#define AG_MG_REGS_CMD_STATUS_MDPE_BM                         0x01000000

#define AG_MG_REGS_CMD_STATUS_DEVSEL_BO                       25
#define AG_MG_REGS_CMD_STATUS_DEVSEL_BM                       0x06000000

#define AG_MG_REGS_CMD_STATUS_STA_BO                          27
#define AG_MG_REGS_CMD_STATUS_STA_BM                          0x08000000

#define AG_MG_REGS_CMD_STATUS_RTA_BO                          28
#define AG_MG_REGS_CMD_STATUS_RTA_BM                          0x10000000

#define AG_MG_REGS_CMD_STATUS_RMA_BO                          29
#define AG_MG_REGS_CMD_STATUS_RMA_BM                          0x20000000

#define AG_MG_REGS_CMD_STATUS_SSE_BO                          30
#define AG_MG_REGS_CMD_STATUS_SSE_BM                          0x40000000

#define AG_MG_REGS_CMD_STATUS_DPE_BO                          31
#define AG_MG_REGS_CMD_STATUS_DPE_BM                          0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CMD_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            ioacc : 1,
            msacc : 1,
            bme : 1,
            sce : 1,
            mwi : 1,
            vgaps : 1,
            per : 1,
            idsel : 1,
            serr : 1,
            fb2bte : 1,
            id : 1,
            fill0 : 5,
            srvd : 3,
            is : 1,
            clist : 1,
            mhzc : 1,
            rvd1 : 1,
            fb2btc : 1,
            mdpe : 1,
            devsel : 2,
            sta : 1,
            rta : 1,
            rma : 1,
            sse : 1,
            dpe : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_cmd_status_u;
#endif


/* 
 * Initialization value: 0x02800000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_REVISION_CODE_RO                           0x00000008
#define AG_MG_REGS_REVISION_CODE_RM                           0xFFFFFFFF

#define AG_MG_REGS_REVISION_CODE_RID_BO                       0
#define AG_MG_REGS_REVISION_CODE_RID_BM                       0x000000FF

#define AG_MG_REGS_REVISION_CODE_CCODE_BO                     8
#define AG_MG_REGS_REVISION_CODE_CCODE_BM                     0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_REVISION_CODE_U
{
    struct
    {
        ag_mg_regs_register
            rid : 8,
            ccode : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_revision_code_u;
#endif


/* 
 * Initialization value: 0x00010000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_RO                   0x0000000C
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_RM                   0xCFFFFFFF

#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_CLZ_BO               0
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_CLZ_BM               0x000000FF

#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_MLT_BO               8
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_MLT_BM               0x0000FF00

#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_HT_BO                16
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_HT_BM                0x00FF0000

#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_BISTR_BO             24
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_BISTR_BM             0x0F000000

#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_BISTI_BO             30
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_BISTI_BM             0x40000000

#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_BISTS_BO             31
#define AG_MG_REGS_CLINE_LATENCY_HT_BIST_BISTS_BM             0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CLINE_LATENCY_HT_BIST_U
{
    struct
    {
        ag_mg_regs_register
            clz : 8,
            mlt : 8,
            ht : 8,
            bistr : 4,
            fill0 : 2,
            bisti : 1,
            bists : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_cline_latency_ht_bist_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BASE_ADDR0_RO 0x00000010  
#define AG_MG_REGS_BASE_ADDR0_RM 0xFFFFFFFF

#define AG_MG_REGS_BASE_ADDR0_MSI_BM 0x00000001
#define AG_MG_REGS_BASE_ADDR0_MSI_BO 0

#define AG_MG_REGS_BASE_ADDR0_TYPE_BM 0x00000006
#define AG_MG_REGS_BASE_ADDR0_TYPE_BO 1

#define AG_MG_REGS_BASE_ADDR0_PREF_BM 0x00000008
#define AG_MG_REGS_BASE_ADDR0_PREF_BO 3

#define AG_MG_REGS_BASE_ADDR0_ADDRE_BM 0xFFFFFFF0
#define AG_MG_REGS_BASE_ADDR0_ADDRE_BO 4

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BASE_ADDR0_U
{
    struct
    {
        ag_mg_regs_register
            msi : 1,
            type : 2,
            pref : 1,
            addre : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_base_addr0_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BASE_ADDR1_RO 0x00000014  
#define AG_MG_REGS_BASE_ADDR1_RM 0xFFFFFFFF

#define AG_MG_REGS_BASE_ADDR1_UPPER_ADDR_BM 0xFFFFFFFF
#define AG_MG_REGS_BASE_ADDR1_UPPER_ADDR_BO 0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BASE_ADDR1_U
{
    struct
    {
        ag_mg_regs_register
            upper_addr;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_base_addr1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BUS_NUMBER_RO                              0x00000018
#define AG_MG_REGS_BUS_NUMBER_RM                              0xFFFFFFFF

#define AG_MG_REGS_BUS_NUMBER_PBN_BO                          0
#define AG_MG_REGS_BUS_NUMBER_PBN_BM                          0x000000FF

#define AG_MG_REGS_BUS_NUMBER_SBN_BO                          8
#define AG_MG_REGS_BUS_NUMBER_SBN_BM                          0x0000FF00

#define AG_MG_REGS_BUS_NUMBER_SUBN_BO                         16
#define AG_MG_REGS_BUS_NUMBER_SUBN_BM                         0x00FF0000

#define AG_MG_REGS_BUS_NUMBER_SLT_BO                          24
#define AG_MG_REGS_BUS_NUMBER_SLT_BM                          0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BUS_NUMBER_U
{
    struct
    {
        ag_mg_regs_register
            pbn : 8,
            sbn : 8,
            subn : 8,
            slt : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bus_number_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RO                      0x0000001C
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RM                      0xFFA0FFFF

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_IOBASE_BO               0
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_IOBASE_BM               0x000000FF

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_IOLIMIT_BO              8
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_IOLIMIT_BM              0x0000FF00

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_MHZC_BO                 21
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_MHZC_BM                 0x00200000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_FB2BTC_BO               23
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_FB2BTC_BM               0x00800000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_MDPE_BO                 24
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_MDPE_BM                 0x01000000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_DEVSEL_BO               25
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_DEVSEL_BM               0x06000000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_STA_BO                  27
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_STA_BM                  0x08000000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RTA_BO                  28
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RTA_BM                  0x10000000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RMA_BO                  29
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RMA_BM                  0x20000000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RSE_BO                  30
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_RSE_BM                  0x40000000

#define AG_MG_REGS_IOLIMIT_SEC_STATUS_DPE_BO                  31
#define AG_MG_REGS_IOLIMIT_SEC_STATUS_DPE_BM                  0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_IOLIMIT_SEC_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            iobase : 8,
            iolimit : 8,
            fill1 : 5,
            mhzc : 1,
            fill0 : 1,
            fb2btc : 1,
            mdpe : 1,
            devsel : 2,
            sta : 1,
            rta : 1,
            rma : 1,
            rse : 1,
            dpe : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_iolimit_sec_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MEMORY_BASE_LIMIT_RO 0x00000020  
#define AG_MG_REGS_MEMORY_BASE_LIMIT_RM 0xFFFFFFFF

#define AG_MG_REGS_MEMORY_BASE_LIMIT_BASE_BM 0x0000FFFF
#define AG_MG_REGS_MEMORY_BASE_LIMIT_BASE_BO 0

#define AG_MG_REGS_MEMORY_BASE_LIMIT_LIMIT_BM 0xFFFF0000
#define AG_MG_REGS_MEMORY_BASE_LIMIT_LIMIT_BO 16

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORY_BASE_LIMIT_U
{
    struct
    {
        ag_mg_regs_register
            base : 16,
            limit : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memory_base_limit_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_RO 0x00000024  
#define AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_RM 0xFFFFFFFF

#define AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_BASE_BM 0x0000FFFF
#define AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_BASE_BO 0

#define AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_LIMIT_BM 0xFFFF0000
#define AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_LIMIT_BO 16

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_U
{
    struct
    {
        ag_mg_regs_register
            base : 16,
            limit : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_prefetchable_memory_base_limit_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PREFETCHABLE_BASE_U32BITS_RO 0x00000028  
#define AG_MG_REGS_PREFETCHABLE_BASE_U32BITS_RM 0xFFFFFFFF

#define AG_MG_REGS_PREFETCHABLE_BASE_U32BITS_BASE_BM 0xFFFFFFFF
#define AG_MG_REGS_PREFETCHABLE_BASE_U32BITS_BASE_BO 0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PREFETCHABLE_BASE_U32BITS_U
{
    struct
    {
        ag_mg_regs_register
            base;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_prefetchable_base_u32bits_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PREFETCHABLE_LIMIT_U32BITS_RO              0x0000002C
#define AG_MG_REGS_PREFETCHABLE_LIMIT_U32BITS_RM              0xFFFFFFFF

#define AG_MG_REGS_PREFETCHABLE_LIMIT_U32BITS_LIMIT_BO        0
#define AG_MG_REGS_PREFETCHABLE_LIMIT_U32BITS_LIMIT_BM        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PREFETCHABLE_LIMIT_U32BITS_U
{
    struct
    {
        ag_mg_regs_register
            limit;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_prefetchable_limit_u32bits_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_IO_LIMIT_BASE_U32BITS_RO 0x00000030  
#define AG_MG_REGS_IO_LIMIT_BASE_U32BITS_RM 0xFFFFFFFF

#define AG_MG_REGS_IO_LIMIT_BASE_U32BITS_BASE_BM 0x0000FFFF
#define AG_MG_REGS_IO_LIMIT_BASE_U32BITS_BASE_BO 0

#define AG_MG_REGS_IO_LIMIT_BASE_U32BITS_LIMIT_BM 0xFFFF0000
#define AG_MG_REGS_IO_LIMIT_BASE_U32BITS_LIMIT_BO 16

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_IO_LIMIT_BASE_U32BITS_U
{
    struct
    {
        ag_mg_regs_register
            base : 16,
            limit : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_io_limit_base_u32bits_u;
#endif


/* 
 * Initialization value: 0x00000040  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_CAPABILITIES_PTR_RO                        0x00000034
#define AG_MG_REGS_CAPABILITIES_PTR_RM                        0x000000FF

#define AG_MG_REGS_CAPABILITIES_PTR_PTR_BO                    0
#define AG_MG_REGS_CAPABILITIES_PTR_PTR_BM                    0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CAPABILITIES_PTR_U
{
    struct
    {
        ag_mg_regs_register
            ptr : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_capabilities_ptr_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_RO 0x00000038  
#define AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_RM 0xFFFFF801

#define AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_ENABLE_BM 0x00000001
#define AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_ENABLE_BO 0

#define AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_ADDRE_BM 0xFFFFF800
#define AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_ADDRE_BO 11

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_U
{
    struct
    {
        ag_mg_regs_register
            enable : 1,
            fill0 : 10,
            addre : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_expansion_rom_base_addr_u;
#endif


/* 
 * Initialization value: 0x00000100  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_INTERRUPT_BCTRL_RO                         0x0000003C
#define AG_MG_REGS_INTERRUPT_BCTRL_RM                         0xFFFFFFFF

#define AG_MG_REGS_INTERRUPT_BCTRL_ILR_BO                     0
#define AG_MG_REGS_INTERRUPT_BCTRL_ILR_BM                     0x000000FF

#define AG_MG_REGS_INTERRUPT_BCTRL_IPR_BO                     8
#define AG_MG_REGS_INTERRUPT_BCTRL_IPR_BM                     0x0000FF00

#define AG_MG_REGS_INTERRUPT_BCTRL_PERE_BO                    16
#define AG_MG_REGS_INTERRUPT_BCTRL_PERE_BM                    0x00010000

#define AG_MG_REGS_INTERRUPT_BCTRL_SERR_BO                    17
#define AG_MG_REGS_INTERRUPT_BCTRL_SERR_BM                    0x00020000

#define AG_MG_REGS_INTERRUPT_BCTRL_MAM_BO                     18
#define AG_MG_REGS_INTERRUPT_BCTRL_MAM_BM                     0x00040000

#define AG_MG_REGS_INTERRUPT_BCTRL_SBR_BO                     19
#define AG_MG_REGS_INTERRUPT_BCTRL_SBR_BM                     0x00080000

#define AG_MG_REGS_INTERRUPT_BCTRL_FB2BTE_BO                  20
#define AG_MG_REGS_INTERRUPT_BCTRL_FB2BTE_BM                  0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_INTERRUPT_BCTRL_U
{
    struct
    {
        ag_mg_regs_register
            ilr : 8,
            ipr : 8,
            pere : 1,
            serr : 1,
            mam : 1,
            sbr : 1,
            fb2bte : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_interrupt_bctrl_u;
#endif


/* 
 * Initialization value: 0x02036001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PME_CAPABILITY_RO                          0x00000040
#define AG_MG_REGS_PME_CAPABILITY_RM                          0xFFEFFFFF

#define AG_MG_REGS_PME_CAPABILITY_CAPID_BO                    0
#define AG_MG_REGS_PME_CAPABILITY_CAPID_BM                    0x000000FF

#define AG_MG_REGS_PME_CAPABILITY_NCPTR_BO                    8
#define AG_MG_REGS_PME_CAPABILITY_NCPTR_BM                    0x0000FF00

#define AG_MG_REGS_PME_CAPABILITY_CAPV_BO                     16
#define AG_MG_REGS_PME_CAPABILITY_CAPV_BM                     0x00070000

#define AG_MG_REGS_PME_CAPABILITY_PMEC_BO                     19
#define AG_MG_REGS_PME_CAPABILITY_PMEC_BM                     0x00080000

#define AG_MG_REGS_PME_CAPABILITY_DSI_BO                      21
#define AG_MG_REGS_PME_CAPABILITY_DSI_BM                      0x00200000

#define AG_MG_REGS_PME_CAPABILITY_AUXC_BO                     22
#define AG_MG_REGS_PME_CAPABILITY_AUXC_BM                     0x01C00000

#define AG_MG_REGS_PME_CAPABILITY_D1S_BO                      25
#define AG_MG_REGS_PME_CAPABILITY_D1S_BM                      0x02000000

#define AG_MG_REGS_PME_CAPABILITY_D2S_BO                      26
#define AG_MG_REGS_PME_CAPABILITY_D2S_BM                      0x04000000

#define AG_MG_REGS_PME_CAPABILITY_PMES_BO                     27
#define AG_MG_REGS_PME_CAPABILITY_PMES_BM                     0xF8000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PME_CAPABILITY_U
{
    struct
    {
        ag_mg_regs_register
            capid : 8,
            ncptr : 8,
            capv : 3,
            pmec : 1,
            fill0 : 1,
            dsi : 1,
            auxc : 3,
            d1s : 1,
            d2s : 1,
            pmes : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pme_capability_u;
#endif


/* 
 * Initialization value: 0x00000008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PME_CTRL_STATUS_RO                         0x00000044
#define AG_MG_REGS_PME_CTRL_STATUS_RM                         0xFFC0FF0B

#define AG_MG_REGS_PME_CTRL_STATUS_PS_BO                      0
#define AG_MG_REGS_PME_CTRL_STATUS_PS_BM                      0x00000003

#define AG_MG_REGS_PME_CTRL_STATUS_NSR_BO                     3
#define AG_MG_REGS_PME_CTRL_STATUS_NSR_BM                     0x00000008

#define AG_MG_REGS_PME_CTRL_STATUS_PMEE_BO                    8
#define AG_MG_REGS_PME_CTRL_STATUS_PMEE_BM                    0x00000100

#define AG_MG_REGS_PME_CTRL_STATUS_PMES_BO                    15
#define AG_MG_REGS_PME_CTRL_STATUS_PMES_BM                    0x00008000

#define AG_MG_REGS_PME_CTRL_STATUS_B2B3S_BO                   22
#define AG_MG_REGS_PME_CTRL_STATUS_B2B3S_BM                   0x00400000

#define AG_MG_REGS_PME_CTRL_STATUS_BPCC_EN_BO                 23
#define AG_MG_REGS_PME_CTRL_STATUS_BPCC_EN_BM                 0x00800000

#define AG_MG_REGS_PME_CTRL_STATUS_DATA_BO                    24
#define AG_MG_REGS_PME_CTRL_STATUS_DATA_BM                    0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PME_CTRL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            ps : 2,
            fill3 : 1,
            nsr : 1,
            fill2 : 4,
            pmee : 1,
            fill1 : 6,
            pmes : 1,
            fill0 : 6,
            b2b3s : 1,
            bpcc_en : 1,
            data : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pme_ctrl_status_u;
#endif


/* 
 * Initialization value: 0x01420010  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_CAPABILITY_RO                              0x00000060
#define AG_MG_REGS_CAPABILITY_RM                              0x7FFFFFFF

#define AG_MG_REGS_CAPABILITY_CAPID_BO                        0
#define AG_MG_REGS_CAPABILITY_CAPID_BM                        0x000000FF

#define AG_MG_REGS_CAPABILITY_NCPTR_BO                        8
#define AG_MG_REGS_CAPABILITY_NCPTR_BM                        0x0000FF00

#define AG_MG_REGS_CAPABILITY_CAPV_BO                         16
#define AG_MG_REGS_CAPABILITY_CAPV_BM                         0x000F0000

#define AG_MG_REGS_CAPABILITY_DEVTYPE_BO                      20
#define AG_MG_REGS_CAPABILITY_DEVTYPE_BM                      0x00F00000

#define AG_MG_REGS_CAPABILITY_SLOT_BO                         24
#define AG_MG_REGS_CAPABILITY_SLOT_BM                         0x01000000

#define AG_MG_REGS_CAPABILITY_IMNUM_BO                        25
#define AG_MG_REGS_CAPABILITY_IMNUM_BM                        0x3E000000

#define AG_MG_REGS_CAPABILITY_TCSR_BO                         30
#define AG_MG_REGS_CAPABILITY_TCSR_BM                         0x40000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CAPABILITY_U
{
    struct
    {
        ag_mg_regs_register
            capid : 8,
            ncptr : 8,
            capv : 4,
            devtype : 4,
            slot : 1,
            imnum : 5,
            tcsr : 1,
            fill0 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_capability_u;
#endif


/* 
 * Initialization value: 0x00008021  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEV_CAPABILITIES_RO                        0x00000064
#define AG_MG_REGS_DEV_CAPABILITIES_RM                        0xFFFFFFFF

#define AG_MG_REGS_DEV_CAPABILITIES_MPSIZE_BO                 0
#define AG_MG_REGS_DEV_CAPABILITIES_MPSIZE_BM                 0x00000007

#define AG_MG_REGS_DEV_CAPABILITIES_PFS_BO                    3
#define AG_MG_REGS_DEV_CAPABILITIES_PFS_BM                    0x00000018

#define AG_MG_REGS_DEV_CAPABILITIES_ETFS_BO                   5
#define AG_MG_REGS_DEV_CAPABILITIES_ETFS_BM                   0x00000020

#define AG_MG_REGS_DEV_CAPABILITIES_EL0AL_BO                  6
#define AG_MG_REGS_DEV_CAPABILITIES_EL0AL_BM                  0x000001C0

#define AG_MG_REGS_DEV_CAPABILITIES_EL1AL_BO                  9
#define AG_MG_REGS_DEV_CAPABILITIES_EL1AL_BM                  0x00000E00

#define AG_MG_REGS_DEV_CAPABILITIES_AB_BO                     12
#define AG_MG_REGS_DEV_CAPABILITIES_AB_BM                     0x00001000

#define AG_MG_REGS_DEV_CAPABILITIES_AI_BO                     13
#define AG_MG_REGS_DEV_CAPABILITIES_AI_BM                     0x00002000

#define AG_MG_REGS_DEV_CAPABILITIES_PI_BO                     14
#define AG_MG_REGS_DEV_CAPABILITIES_PI_BM                     0x00004000

#define AG_MG_REGS_DEV_CAPABILITIES_RBER_BO                   15
#define AG_MG_REGS_DEV_CAPABILITIES_RBER_BM                   0x00008000

#define AG_MG_REGS_DEV_CAPABILITIES_RVD_BO                    16
#define AG_MG_REGS_DEV_CAPABILITIES_RVD_BM                    0x00030000

#define AG_MG_REGS_DEV_CAPABILITIES_CSPLV_BO                  18
#define AG_MG_REGS_DEV_CAPABILITIES_CSPLV_BM                  0x03FC0000

#define AG_MG_REGS_DEV_CAPABILITIES_CSPLS_BO                  26
#define AG_MG_REGS_DEV_CAPABILITIES_CSPLS_BM                  0x0C000000

#define AG_MG_REGS_DEV_CAPABILITIES_FLRC_BO                   28
#define AG_MG_REGS_DEV_CAPABILITIES_FLRC_BM                   0x10000000

#define AG_MG_REGS_DEV_CAPABILITIES_RVD2_BO                   29
#define AG_MG_REGS_DEV_CAPABILITIES_RVD2_BM                   0xE0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEV_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            mpsize : 3,
            pfs : 2,
            etfs : 1,
            el0al : 3,
            el1al : 3,
            ab : 1,
            ai : 1,
            pi : 1,
            rber : 1,
            rvd : 2,
            csplv : 8,
            cspls : 2,
            flrc : 1,
            rvd2 : 3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dev_capabilities_u;
#endif


/* 
 * Initialization value: 0x00005930  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEV_CTRL_STATUS_RO                         0x00000068
#define AG_MG_REGS_DEV_CTRL_STATUS_RM                         0xFFFFFFFF

#define AG_MG_REGS_DEV_CTRL_STATUS_CERE_BO                    0
#define AG_MG_REGS_DEV_CTRL_STATUS_CERE_BM                    0x00000001

#define AG_MG_REGS_DEV_CTRL_STATUS_NFER_BO                    1
#define AG_MG_REGS_DEV_CTRL_STATUS_NFER_BM                    0x00000002

#define AG_MG_REGS_DEV_CTRL_STATUS_FERE_BO                    2
#define AG_MG_REGS_DEV_CTRL_STATUS_FERE_BM                    0x00000004

#define AG_MG_REGS_DEV_CTRL_STATUS_URRE_BO                    3
#define AG_MG_REGS_DEV_CTRL_STATUS_URRE_BM                    0x00000008

#define AG_MG_REGS_DEV_CTRL_STATUS_ERO_BO                     4
#define AG_MG_REGS_DEV_CTRL_STATUS_ERO_BM                     0x00000010

#define AG_MG_REGS_DEV_CTRL_STATUS_MPSIZE_BO                  5
#define AG_MG_REGS_DEV_CTRL_STATUS_MPSIZE_BM                  0x000000E0

#define AG_MG_REGS_DEV_CTRL_STATUS_ETFE_BO                    8
#define AG_MG_REGS_DEV_CTRL_STATUS_ETFE_BM                    0x00000100

#define AG_MG_REGS_DEV_CTRL_STATUS_PFE_BO                     9
#define AG_MG_REGS_DEV_CTRL_STATUS_PFE_BM                     0x00000200

#define AG_MG_REGS_DEV_CTRL_STATUS_APPME_BO                   10
#define AG_MG_REGS_DEV_CTRL_STATUS_APPME_BM                   0x00000400

#define AG_MG_REGS_DEV_CTRL_STATUS_ENS_BO                     11
#define AG_MG_REGS_DEV_CTRL_STATUS_ENS_BM                     0x00000800

#define AG_MG_REGS_DEV_CTRL_STATUS_MRRS_BO                    12
#define AG_MG_REGS_DEV_CTRL_STATUS_MRRS_BM                    0x00007000

#define AG_MG_REGS_DEV_CTRL_STATUS_BCRE_BO                    15
#define AG_MG_REGS_DEV_CTRL_STATUS_BCRE_BM                    0x00008000

#define AG_MG_REGS_DEV_CTRL_STATUS_CED_BO                     16
#define AG_MG_REGS_DEV_CTRL_STATUS_CED_BM                     0x00010000

#define AG_MG_REGS_DEV_CTRL_STATUS_NFED_BO                    17
#define AG_MG_REGS_DEV_CTRL_STATUS_NFED_BM                    0x00020000

#define AG_MG_REGS_DEV_CTRL_STATUS_FED_BO                     18
#define AG_MG_REGS_DEV_CTRL_STATUS_FED_BM                     0x00040000

#define AG_MG_REGS_DEV_CTRL_STATUS_URD_BO                     19
#define AG_MG_REGS_DEV_CTRL_STATUS_URD_BM                     0x00080000

#define AG_MG_REGS_DEV_CTRL_STATUS_AUXPD_BO                   20
#define AG_MG_REGS_DEV_CTRL_STATUS_AUXPD_BM                   0x00100000

#define AG_MG_REGS_DEV_CTRL_STATUS_TP_BO                      21
#define AG_MG_REGS_DEV_CTRL_STATUS_TP_BM                      0x00200000

#define AG_MG_REGS_DEV_CTRL_STATUS_RSVD3_BO                   22
#define AG_MG_REGS_DEV_CTRL_STATUS_RSVD3_BM                   0xFFC00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEV_CTRL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            cere : 1,
            nfer : 1,
            fere : 1,
            urre : 1,
            ero : 1,
            mpsize : 3,
            etfe : 1,
            pfe : 1,
            appme : 1,
            ens : 1,
            mrrs : 3,
            bcre : 1,
            ced : 1,
            nfed : 1,
            fed : 1,
            urd : 1,
            auxpd : 1,
            tp : 1,
            rsvd3 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dev_ctrl_status_u;
#endif


/* 
 * Initialization value: 0x0010BC42  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_RO                  0x0000006C
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_RM                  0xFF3FFFFF

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_SLS_BO              0
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_SLS_BM              0x0000000F

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_MLW_BO              4
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_MLW_BM              0x000003F0

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_ASPM_BO             10
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_ASPM_BM             0x00000C00

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_L0EL_BO             12
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_L0EL_BM             0x00007000

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_L1EL_BO             15
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_L1EL_BM             0x00038000

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_CPM_BO              18
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_CPM_BM              0x00040000

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_SEDRC_BO            19
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_SEDRC_BM            0x00080000

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_DLLARC_BO           20
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_DLLARC_BM           0x00100000

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_LBNC_BO             21
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_LBNC_BM             0x00200000

#define AG_MG_REGS_LINK_CAPABILITIES_APRC_PORTNUM_BO          24
#define AG_MG_REGS_LINK_CAPABILITIES_APRC_PORTNUM_BM          0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_CAPABILITIES_APRC_U
{
    struct
    {
        ag_mg_regs_register
            sls : 4,
            mlw : 6,
            aspm : 2,
            l0el : 3,
            l1el : 3,
            cpm : 1,
            sedrc : 1,
            dllarc : 1,
            lbnc : 1,
            fill0 : 2,
            portnum : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_capabilities_aprc_u;
#endif


/* 
 * Initialization value: 0x00410008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINK_CTRL_STATUS_RO                        0x00000070
#define AG_MG_REGS_LINK_CTRL_STATUS_RM                        0xFFFF0FFB

#define AG_MG_REGS_LINK_CTRL_STATUS_ASPM_BO                   0
#define AG_MG_REGS_LINK_CTRL_STATUS_ASPM_BM                   0x00000003

#define AG_MG_REGS_LINK_CTRL_STATUS_RCB_BO                    3
#define AG_MG_REGS_LINK_CTRL_STATUS_RCB_BM                    0x00000008

#define AG_MG_REGS_LINK_CTRL_STATUS_LD_BO                     4
#define AG_MG_REGS_LINK_CTRL_STATUS_LD_BM                     0x00000010

#define AG_MG_REGS_LINK_CTRL_STATUS_RL_BO                     5
#define AG_MG_REGS_LINK_CTRL_STATUS_RL_BM                     0x00000020

#define AG_MG_REGS_LINK_CTRL_STATUS_CCC_BO                    6
#define AG_MG_REGS_LINK_CTRL_STATUS_CCC_BM                    0x00000040

#define AG_MG_REGS_LINK_CTRL_STATUS_ESYNC_BO                  7
#define AG_MG_REGS_LINK_CTRL_STATUS_ESYNC_BM                  0x00000080

#define AG_MG_REGS_LINK_CTRL_STATUS_ECPM_BO                   8
#define AG_MG_REGS_LINK_CTRL_STATUS_ECPM_BM                   0x00000100

#define AG_MG_REGS_LINK_CTRL_STATUS_HAWD_BO                   9
#define AG_MG_REGS_LINK_CTRL_STATUS_HAWD_BM                   0x00000200

#define AG_MG_REGS_LINK_CTRL_STATUS_LBMIE_BO                  10
#define AG_MG_REGS_LINK_CTRL_STATUS_LBMIE_BM                  0x00000400

#define AG_MG_REGS_LINK_CTRL_STATUS_LABIE_BO                  11
#define AG_MG_REGS_LINK_CTRL_STATUS_LABIE_BM                  0x00000800

#define AG_MG_REGS_LINK_CTRL_STATUS_CLS_BO                    16
#define AG_MG_REGS_LINK_CTRL_STATUS_CLS_BM                    0x000F0000

#define AG_MG_REGS_LINK_CTRL_STATUS_NLW_BO                    20
#define AG_MG_REGS_LINK_CTRL_STATUS_NLW_BM                    0x03F00000

#define AG_MG_REGS_LINK_CTRL_STATUS_UNDEF_BO                  26
#define AG_MG_REGS_LINK_CTRL_STATUS_UNDEF_BM                  0x04000000

#define AG_MG_REGS_LINK_CTRL_STATUS_LT_BO                     27
#define AG_MG_REGS_LINK_CTRL_STATUS_LT_BM                     0x08000000

#define AG_MG_REGS_LINK_CTRL_STATUS_SCC_BO                    28
#define AG_MG_REGS_LINK_CTRL_STATUS_SCC_BM                    0x10000000

#define AG_MG_REGS_LINK_CTRL_STATUS_DLLA_BO                   29
#define AG_MG_REGS_LINK_CTRL_STATUS_DLLA_BM                   0x20000000

#define AG_MG_REGS_LINK_CTRL_STATUS_LBMS_BO                   30
#define AG_MG_REGS_LINK_CTRL_STATUS_LBMS_BM                   0x40000000

#define AG_MG_REGS_LINK_CTRL_STATUS_LABS_BO                   31
#define AG_MG_REGS_LINK_CTRL_STATUS_LABS_BM                   0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_CTRL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            aspm : 2,
            fill1 : 1,
            rcb : 1,
            ld : 1,
            rl : 1,
            ccc : 1,
            esync : 1,
            ecpm : 1,
            hawd : 1,
            lbmie : 1,
            labie : 1,
            fill0 : 4,
            cls : 4,
            nlw : 6,
            undef : 1,
            lt : 1,
            scc : 1,
            dlla : 1,
            lbms : 1,
            labs : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_ctrl_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLOT_CAPABILITIES_RO                       0x00000074
#define AG_MG_REGS_SLOT_CAPABILITIES_RM                       0xFFFFFFFF

#define AG_MG_REGS_SLOT_CAPABILITIES_RVD_BO                   0
#define AG_MG_REGS_SLOT_CAPABILITIES_RVD_BM                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLOT_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            rvd;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slot_capabilities_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLOT_CTRL_STATUS_RO                        0x00000078
#define AG_MG_REGS_SLOT_CTRL_STATUS_RM                        0xFFFFFFFF

#define AG_MG_REGS_SLOT_CTRL_STATUS_RVD_BO                    0
#define AG_MG_REGS_SLOT_CTRL_STATUS_RVD_BM                    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLOT_CTRL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            rvd;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slot_ctrl_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_RO                  0x0000007C
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_RM                  0x0000001F

#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_SECEE_BO            0
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_SECEE_BM            0x00000001

#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_SENFEE_BO           1
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_SENFEE_BM           0x00000002

#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_SEFEE_BO            2
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_SEFEE_BM            0x00000004

#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_PMEIE_BO            3
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_PMEIE_BM            0x00000008

#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_CRSSVE_BO           4
#define AG_MG_REGS_ROOT_CTRL_CAPABILITIES_CRSSVE_BM           0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ROOT_CTRL_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            secee : 1,
            senfee : 1,
            sefee : 1,
            pmeie : 1,
            crssve : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_root_ctrl_capabilities_u;
#endif


/* 
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEV_CAPABILITIES2_RO                       0x00000084
#define AG_MG_REGS_DEV_CAPABILITIES2_RM                       0x0000001F

#define AG_MG_REGS_DEV_CAPABILITIES2_CTRS_BO                  0
#define AG_MG_REGS_DEV_CAPABILITIES2_CTRS_BM                  0x0000000F

#define AG_MG_REGS_DEV_CAPABILITIES2_CTDS_BO                  4
#define AG_MG_REGS_DEV_CAPABILITIES2_CTDS_BM                  0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEV_CAPABILITIES2_U
{
    struct
    {
        ag_mg_regs_register
            ctrs : 4,
            ctds : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dev_capabilities2_u;
#endif


/* 
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEV_CTRL_STATUS2_RO                        0x00000088
#define AG_MG_REGS_DEV_CTRL_STATUS2_RM                        0x0000001F

#define AG_MG_REGS_DEV_CTRL_STATUS2_CTOV_BO                   0
#define AG_MG_REGS_DEV_CTRL_STATUS2_CTOV_BM                   0x0000000F

#define AG_MG_REGS_DEV_CTRL_STATUS2_CTOD_BO                   4
#define AG_MG_REGS_DEV_CTRL_STATUS2_CTOD_BM                   0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEV_CTRL_STATUS2_U
{
    struct
    {
        ag_mg_regs_register
            ctov : 4,
            ctod : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dev_ctrl_status2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINK_CAP2_RO                               0x0000008C
#define AG_MG_REGS_LINK_CAP2_RM                               0xFFFFFFFF

#define AG_MG_REGS_LINK_CAP2_RSVDP_BO                         0
#define AG_MG_REGS_LINK_CAP2_RSVDP_BM                         0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_CAP2_U
{
    struct
    {
        ag_mg_regs_register
            rsvdp;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_cap2_u;
#endif


/* 
 * Initialization value: 0x00000042  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINK_CTRL_STATUS2_RO                       0x00000090
#define AG_MG_REGS_LINK_CTRL_STATUS2_RM                       0x00011FFF

#define AG_MG_REGS_LINK_CTRL_STATUS2_TLS_BO                   0
#define AG_MG_REGS_LINK_CTRL_STATUS2_TLS_BM                   0x0000000F

#define AG_MG_REGS_LINK_CTRL_STATUS2_EC_BO                    4
#define AG_MG_REGS_LINK_CTRL_STATUS2_EC_BM                    0x00000010

#define AG_MG_REGS_LINK_CTRL_STATUS2_HASD_BO                  5
#define AG_MG_REGS_LINK_CTRL_STATUS2_HASD_BM                  0x00000020

#define AG_MG_REGS_LINK_CTRL_STATUS2_SELDEEM_BO               6
#define AG_MG_REGS_LINK_CTRL_STATUS2_SELDEEM_BM               0x00000040

#define AG_MG_REGS_LINK_CTRL_STATUS2_TM_BO                    7
#define AG_MG_REGS_LINK_CTRL_STATUS2_TM_BM                    0x00000380

#define AG_MG_REGS_LINK_CTRL_STATUS2_EMC_BO                   10
#define AG_MG_REGS_LINK_CTRL_STATUS2_EMC_BM                   0x00000400

#define AG_MG_REGS_LINK_CTRL_STATUS2_CSOS_BO                  11
#define AG_MG_REGS_LINK_CTRL_STATUS2_CSOS_BM                  0x00000800

#define AG_MG_REGS_LINK_CTRL_STATUS2_CDEEM_BO                 12
#define AG_MG_REGS_LINK_CTRL_STATUS2_CDEEM_BM                 0x00001000

#define AG_MG_REGS_LINK_CTRL_STATUS2_CDEEML_BO                16
#define AG_MG_REGS_LINK_CTRL_STATUS2_CDEEML_BM                0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_CTRL_STATUS2_U
{
    struct
    {
        ag_mg_regs_register
            tls : 4,
            ec : 1,
            hasd : 1,
            seldeem : 1,
            tm : 3,
            emc : 1,
            csos : 1,
            cdeem : 1,
            fill1 : 3,
            cdeeml : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_ctrl_status2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEV_CAP_RSVD1_RO                           0x00000094
#define AG_MG_REGS_DEV_CAP_RSVD1_RM                           0x00000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEV_CAP_RSVD1_U
{
    struct
    {
        ag_mg_regs_register dev_cap_rsvd1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dev_cap_rsvd1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEV_CAP_RSVD2_RO                           0x00000098
#define AG_MG_REGS_DEV_CAP_RSVD2_RM                           0x00000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEV_CAP_RSVD2_U
{
    struct
    {
        ag_mg_regs_register dev_cap_rsvd2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dev_cap_rsvd2_u;
#endif


/* 
 * Initialization value: 0x14010001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_RO           0x00000100
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_RM           0xFFFFFFFF

#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_ECID_BO      0
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_ECID_BM      0x0000FFFF

#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_CV_BO        16
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_CV_BM        0x000F0000

#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_NCO_BO       20
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_NCO_BM       0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_U
{
    struct
    {
        ag_mg_regs_register
            ecid : 16,
            cv : 4,
            nco : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_enhanced_cap_header_aprc_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UNC_ERR_STATUS_RO                          0x00000104
#define AG_MG_REGS_UNC_ERR_STATUS_RM                          0x003FF030

#define AG_MG_REGS_UNC_ERR_STATUS_DLPES_BO                    4
#define AG_MG_REGS_UNC_ERR_STATUS_DLPES_BM                    0x00000010

#define AG_MG_REGS_UNC_ERR_STATUS_PTLPS_BO                    12
#define AG_MG_REGS_UNC_ERR_STATUS_PTLPS_BM                    0x00001000

#define AG_MG_REGS_UNC_ERR_STATUS_FCPES_BO                    13
#define AG_MG_REGS_UNC_ERR_STATUS_FCPES_BM                    0x00002000

#define AG_MG_REGS_UNC_ERR_STATUS_CTS_BO                      14
#define AG_MG_REGS_UNC_ERR_STATUS_CTS_BM                      0x00004000

#define AG_MG_REGS_UNC_ERR_STATUS_CAS_BO                      15
#define AG_MG_REGS_UNC_ERR_STATUS_CAS_BM                      0x00008000

#define AG_MG_REGS_UNC_ERR_STATUS_UCS_BO                      16
#define AG_MG_REGS_UNC_ERR_STATUS_UCS_BM                      0x00010000

#define AG_MG_REGS_UNC_ERR_STATUS_ROS_BO                      17
#define AG_MG_REGS_UNC_ERR_STATUS_ROS_BM                      0x00020000

#define AG_MG_REGS_UNC_ERR_STATUS_MTLPS_BO                    18
#define AG_MG_REGS_UNC_ERR_STATUS_MTLPS_BM                    0x00040000

#define AG_MG_REGS_UNC_ERR_STATUS_ECRCES_BO                   19
#define AG_MG_REGS_UNC_ERR_STATUS_ECRCES_BM                   0x00080000

#define AG_MG_REGS_UNC_ERR_STATUS_URES_BO                     20
#define AG_MG_REGS_UNC_ERR_STATUS_URES_BM                     0x00100000

#define AG_MG_REGS_UNC_ERR_STATUS_ACSVS_BO                    21
#define AG_MG_REGS_UNC_ERR_STATUS_ACSVS_BM                    0x00200000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNC_ERR_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            fill2 : 4,
            dlpes : 1,
            fill1 : 7,
            ptlps : 1,
            fcpes : 1,
            cts : 1,
            cas : 1,
            ucs : 1,
            ros : 1,
            mtlps : 1,
            ecrces : 1,
            ures : 1,
            acsvs : 1,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_unc_err_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UNC_ERR_MASK_RO                            0x00000108
#define AG_MG_REGS_UNC_ERR_MASK_RM                            0x003FF030

#define AG_MG_REGS_UNC_ERR_MASK_DLPEM_BO                      4
#define AG_MG_REGS_UNC_ERR_MASK_DLPEM_BM                      0x00000010

#define AG_MG_REGS_UNC_ERR_MASK_PTLPM_BO                      12
#define AG_MG_REGS_UNC_ERR_MASK_PTLPM_BM                      0x00001000

#define AG_MG_REGS_UNC_ERR_MASK_FCPEM_BO                      13
#define AG_MG_REGS_UNC_ERR_MASK_FCPEM_BM                      0x00002000

#define AG_MG_REGS_UNC_ERR_MASK_CTM_BO                        14
#define AG_MG_REGS_UNC_ERR_MASK_CTM_BM                        0x00004000

#define AG_MG_REGS_UNC_ERR_MASK_CAM_BO                        15
#define AG_MG_REGS_UNC_ERR_MASK_CAM_BM                        0x00008000

#define AG_MG_REGS_UNC_ERR_MASK_UCM_BO                        16
#define AG_MG_REGS_UNC_ERR_MASK_UCM_BM                        0x00010000

#define AG_MG_REGS_UNC_ERR_MASK_ROM_BO                        17
#define AG_MG_REGS_UNC_ERR_MASK_ROM_BM                        0x00020000

#define AG_MG_REGS_UNC_ERR_MASK_MTLPM_BO                      18
#define AG_MG_REGS_UNC_ERR_MASK_MTLPM_BM                      0x00040000

#define AG_MG_REGS_UNC_ERR_MASK_ECRCEM_BO                     19
#define AG_MG_REGS_UNC_ERR_MASK_ECRCEM_BM                     0x00080000

#define AG_MG_REGS_UNC_ERR_MASK_UREM_BO                       20
#define AG_MG_REGS_UNC_ERR_MASK_UREM_BM                       0x00100000

#define AG_MG_REGS_UNC_ERR_MASK_ACSVM_BO                      21
#define AG_MG_REGS_UNC_ERR_MASK_ACSVM_BM                      0x00200000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNC_ERR_MASK_U
{
    struct
    {
        ag_mg_regs_register
            fill2 : 4,
            dlpem : 1,
            fill1 : 7,
            ptlpm : 1,
            fcpem : 1,
            ctm : 1,
            cam : 1,
            ucm : 1,
            rom : 1,
            mtlpm : 1,
            ecrcem : 1,
            urem : 1,
            acsvm : 1,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_unc_err_mask_u;
#endif


/* 
 * Initialization value: 0x00062030  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UNC_ERR_SEVERITY_RO                        0x0000010C
#define AG_MG_REGS_UNC_ERR_SEVERITY_RM                        0x003FF030

#define AG_MG_REGS_UNC_ERR_SEVERITY_DLPES_BO                  4
#define AG_MG_REGS_UNC_ERR_SEVERITY_DLPES_BM                  0x00000010

#define AG_MG_REGS_UNC_ERR_SEVERITY_PTLPS_BO                  12
#define AG_MG_REGS_UNC_ERR_SEVERITY_PTLPS_BM                  0x00001000

#define AG_MG_REGS_UNC_ERR_SEVERITY_FCPES_BO                  13
#define AG_MG_REGS_UNC_ERR_SEVERITY_FCPES_BM                  0x00002000

#define AG_MG_REGS_UNC_ERR_SEVERITY_CTS_BO                    14
#define AG_MG_REGS_UNC_ERR_SEVERITY_CTS_BM                    0x00004000

#define AG_MG_REGS_UNC_ERR_SEVERITY_CAS_BO                    15
#define AG_MG_REGS_UNC_ERR_SEVERITY_CAS_BM                    0x00008000

#define AG_MG_REGS_UNC_ERR_SEVERITY_UCS_BO                    16
#define AG_MG_REGS_UNC_ERR_SEVERITY_UCS_BM                    0x00010000

#define AG_MG_REGS_UNC_ERR_SEVERITY_ROS_BO                    17
#define AG_MG_REGS_UNC_ERR_SEVERITY_ROS_BM                    0x00020000

#define AG_MG_REGS_UNC_ERR_SEVERITY_MTLPS_BO                  18
#define AG_MG_REGS_UNC_ERR_SEVERITY_MTLPS_BM                  0x00040000

#define AG_MG_REGS_UNC_ERR_SEVERITY_ECRCES_BO                 19
#define AG_MG_REGS_UNC_ERR_SEVERITY_ECRCES_BM                 0x00080000

#define AG_MG_REGS_UNC_ERR_SEVERITY_URES_BO                   20
#define AG_MG_REGS_UNC_ERR_SEVERITY_URES_BM                   0x00100000

#define AG_MG_REGS_UNC_ERR_SEVERITY_AVSVS_BO                  21
#define AG_MG_REGS_UNC_ERR_SEVERITY_AVSVS_BM                  0x00200000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNC_ERR_SEVERITY_U
{
    struct
    {
        ag_mg_regs_register
            fill2 : 4,
            dlpes : 1,
            fill1 : 7,
            ptlps : 1,
            fcpes : 1,
            cts : 1,
            cas : 1,
            ucs : 1,
            ros : 1,
            mtlps : 1,
            ecrces : 1,
            ures : 1,
            avsvs : 1,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_unc_err_severity_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_CORR_ERR_STATUS_RO                         0x00000110
#define AG_MG_REGS_CORR_ERR_STATUS_RM                         0x000031C1

#define AG_MG_REGS_CORR_ERR_STATUS_RES_BO                     0
#define AG_MG_REGS_CORR_ERR_STATUS_RES_BM                     0x00000001

#define AG_MG_REGS_CORR_ERR_STATUS_BTLPS_BO                   6
#define AG_MG_REGS_CORR_ERR_STATUS_BTLPS_BM                   0x00000040

#define AG_MG_REGS_CORR_ERR_STATUS_BDLLPS_BO                  7
#define AG_MG_REGS_CORR_ERR_STATUS_BDLLPS_BM                  0x00000080

#define AG_MG_REGS_CORR_ERR_STATUS_RNROS_BO                   8
#define AG_MG_REGS_CORR_ERR_STATUS_RNROS_BM                   0x00000100

#define AG_MG_REGS_CORR_ERR_STATUS_RTTOS_BO                   12
#define AG_MG_REGS_CORR_ERR_STATUS_RTTOS_BM                   0x00001000

#define AG_MG_REGS_CORR_ERR_STATUS_ANFES_BO                   13
#define AG_MG_REGS_CORR_ERR_STATUS_ANFES_BM                   0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CORR_ERR_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            res : 1,
            fill2 : 5,
            btlps : 1,
            bdllps : 1,
            rnros : 1,
            fill1 : 3,
            rttos : 1,
            anfes : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_corr_err_status_u;
#endif


/* 
 * Initialization value: 0x00002000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CORR_ERR_MASK_RO                           0x00000114
#define AG_MG_REGS_CORR_ERR_MASK_RM                           0x000031C1

#define AG_MG_REGS_CORR_ERR_MASK_REM_BO                       0
#define AG_MG_REGS_CORR_ERR_MASK_REM_BM                       0x00000001

#define AG_MG_REGS_CORR_ERR_MASK_BTLPM_BO                     6
#define AG_MG_REGS_CORR_ERR_MASK_BTLPM_BM                     0x00000040

#define AG_MG_REGS_CORR_ERR_MASK_BDLLPM_BO                    7
#define AG_MG_REGS_CORR_ERR_MASK_BDLLPM_BM                    0x00000080

#define AG_MG_REGS_CORR_ERR_MASK_RNROM_BO                     8
#define AG_MG_REGS_CORR_ERR_MASK_RNROM_BM                     0x00000100

#define AG_MG_REGS_CORR_ERR_MASK_RTTOM_BO                     12
#define AG_MG_REGS_CORR_ERR_MASK_RTTOM_BM                     0x00001000

#define AG_MG_REGS_CORR_ERR_MASK_ANFEM_BO                     13
#define AG_MG_REGS_CORR_ERR_MASK_ANFEM_BM                     0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CORR_ERR_MASK_U
{
    struct
    {
        ag_mg_regs_register
            rem : 1,
            fill2 : 5,
            btlpm : 1,
            bdllpm : 1,
            rnrom : 1,
            fill1 : 3,
            rttom : 1,
            anfem : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_corr_err_mask_u;
#endif


/* 
 * Initialization value: 0x000000A0  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ERR_CAP_CTRL_RO                            0x00000118
#define AG_MG_REGS_ERR_CAP_CTRL_RM                            0x000001FF

#define AG_MG_REGS_ERR_CAP_CTRL_FEPTR_BO                      0
#define AG_MG_REGS_ERR_CAP_CTRL_FEPTR_BM                      0x0000001F

#define AG_MG_REGS_ERR_CAP_CTRL_ECRCGC_BO                     5
#define AG_MG_REGS_ERR_CAP_CTRL_ECRCGC_BM                     0x00000020

#define AG_MG_REGS_ERR_CAP_CTRL_ECRCGE_BO                     6
#define AG_MG_REGS_ERR_CAP_CTRL_ECRCGE_BM                     0x00000040

#define AG_MG_REGS_ERR_CAP_CTRL_ECRCCC_BO                     7
#define AG_MG_REGS_ERR_CAP_CTRL_ECRCCC_BM                     0x00000080

#define AG_MG_REGS_ERR_CAP_CTRL_ECRCCE_BO                     8
#define AG_MG_REGS_ERR_CAP_CTRL_ECRCCE_BM                     0x00000100

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ERR_CAP_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            feptr : 5,
            ecrcgc : 1,
            ecrcge : 1,
            ecrccc : 1,
            ecrcce : 1,
            fill0 : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_err_cap_ctrl_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEADER_LOG1_RO                             0x0000011C
#define AG_MG_REGS_HEADER_LOG1_RM                             0xFFFFFFFF

#define AG_MG_REGS_HEADER_LOG1_ERRLOG_BO                      0
#define AG_MG_REGS_HEADER_LOG1_ERRLOG_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEADER_LOG1_U
{
    struct
    {
        ag_mg_regs_register
            errlog;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_header_log1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEADER_LOG2_RO                             0x00000120
#define AG_MG_REGS_HEADER_LOG2_RM                             0xFFFFFFFF

#define AG_MG_REGS_HEADER_LOG2_ERRLOG_BO                      0
#define AG_MG_REGS_HEADER_LOG2_ERRLOG_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEADER_LOG2_U
{
    struct
    {
        ag_mg_regs_register
            errlog;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_header_log2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEADER_LOG3_RO                             0x00000124
#define AG_MG_REGS_HEADER_LOG3_RM                             0xFFFFFFFF

#define AG_MG_REGS_HEADER_LOG3_ERRLOG_BO                      0
#define AG_MG_REGS_HEADER_LOG3_ERRLOG_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEADER_LOG3_U
{
    struct
    {
        ag_mg_regs_register
            errlog;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_header_log3_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEADER_LOG4_RO                             0x00000128
#define AG_MG_REGS_HEADER_LOG4_RM                             0xFFFFFFFF

#define AG_MG_REGS_HEADER_LOG4_ERRLOG_BO                      0
#define AG_MG_REGS_HEADER_LOG4_ERRLOG_BM                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEADER_LOG4_U
{
    struct
    {
        ag_mg_regs_register
            errlog;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_header_log4_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ROOT_ERR_CMD_RO                            0x0000012C
#define AG_MG_REGS_ROOT_ERR_CMD_RM                            0x00000007

#define AG_MG_REGS_ROOT_ERR_CMD_CERE_BO                       0
#define AG_MG_REGS_ROOT_ERR_CMD_CERE_BM                       0x00000001

#define AG_MG_REGS_ROOT_ERR_CMD_NFERE_BO                      1
#define AG_MG_REGS_ROOT_ERR_CMD_NFERE_BM                      0x00000002

#define AG_MG_REGS_ROOT_ERR_CMD_FERE_BO                       2
#define AG_MG_REGS_ROOT_ERR_CMD_FERE_BM                       0x00000004

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ROOT_ERR_CMD_U
{
    struct
    {
        ag_mg_regs_register
            cere : 1,
            nfere : 1,
            fere : 1,
            fill0 : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_root_err_cmd_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ROOT_ERR_STATUS_RO                         0x00000130
#define AG_MG_REGS_ROOT_ERR_STATUS_RM                         0xF800007F

#define AG_MG_REGS_ROOT_ERR_STATUS_ERR_CORR_BO                0
#define AG_MG_REGS_ROOT_ERR_STATUS_ERR_CORR_BM                0x00000001

#define AG_MG_REGS_ROOT_ERR_STATUS_MERR_CORR_BO               1
#define AG_MG_REGS_ROOT_ERR_STATUS_MERR_CORR_BM               0x00000002

#define AG_MG_REGS_ROOT_ERR_STATUS_ERR_FNFR_BO                2
#define AG_MG_REGS_ROOT_ERR_STATUS_ERR_FNFR_BM                0x00000004

#define AG_MG_REGS_ROOT_ERR_STATUS_MERR_FNFR_BO               3
#define AG_MG_REGS_ROOT_ERR_STATUS_MERR_FNFR_BM               0x00000008

#define AG_MG_REGS_ROOT_ERR_STATUS_FUNCF_BO                   4
#define AG_MG_REGS_ROOT_ERR_STATUS_FUNCF_BM                   0x00000010

#define AG_MG_REGS_ROOT_ERR_STATUS_NFEMR_BO                   5
#define AG_MG_REGS_ROOT_ERR_STATUS_NFEMR_BM                   0x00000020

#define AG_MG_REGS_ROOT_ERR_STATUS_FEMR_BO                    6
#define AG_MG_REGS_ROOT_ERR_STATUS_FEMR_BM                    0x00000040

#define AG_MG_REGS_ROOT_ERR_STATUS_AEIMN_BO                   27
#define AG_MG_REGS_ROOT_ERR_STATUS_AEIMN_BM                   0xF8000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ROOT_ERR_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            err_corr : 1,
            merr_corr : 1,
            err_fnfr : 1,
            merr_fnfr : 1,
            funcf : 1,
            nfemr : 1,
            femr : 1,
            fill0 : 20,
            aeimn : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_root_err_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_ERR_SRC_ID_RO                              0x00000134
#define AG_MG_REGS_ERR_SRC_ID_RM                              0xFFFFFFFF

#define AG_MG_REGS_ERR_SRC_ID_ERR_COR_SRCID_BO                0
#define AG_MG_REGS_ERR_SRC_ID_ERR_COR_SRCID_BM                0x0000FFFF

#define AG_MG_REGS_ERR_SRC_ID_ERR_FNF_SRC_ID_BO               16
#define AG_MG_REGS_ERR_SRC_ID_ERR_FNF_SRC_ID_BM               0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ERR_SRC_ID_U
{
    struct
    {
        ag_mg_regs_register
            err_cor_srcid : 16,
            err_fnf_src_id : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_err_src_id_u;
#endif


/* 
 * Initialization value: 0x00010002  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_RO                   0x00000140
#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_RM                   0xFFFFFFFF

#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_ECID_BO              0
#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_ECID_BM              0x0000FFFF

#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_VC_BO                16
#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_VC_BM                0x000F0000

#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_NCO_BO               20
#define AG_MG_REGS_VC_CHANNEL_CAP_HEADER_NCO_BM               0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_CHANNEL_CAP_HEADER_U
{
    struct
    {
        ag_mg_regs_register
            ecid : 16,
            vc : 4,
            nco : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_channel_cap_header_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PORT_VC_CAPABILITY1_RO                     0x00000144
#define AG_MG_REGS_PORT_VC_CAPABILITY1_RM                     0xFFFFFFFF

#define AG_MG_REGS_PORT_VC_CAPABILITY1_EVC_BO                 0
#define AG_MG_REGS_PORT_VC_CAPABILITY1_EVC_BM                 0x00000007

#define AG_MG_REGS_PORT_VC_CAPABILITY1_RSVD1_BO               3
#define AG_MG_REGS_PORT_VC_CAPABILITY1_RSVD1_BM               0x00000008

#define AG_MG_REGS_PORT_VC_CAPABILITY1_LPEVC_BO               4
#define AG_MG_REGS_PORT_VC_CAPABILITY1_LPEVC_BM               0x00000070

#define AG_MG_REGS_PORT_VC_CAPABILITY1_RSVD2_BO               7
#define AG_MG_REGS_PORT_VC_CAPABILITY1_RSVD2_BM               0x00000080

#define AG_MG_REGS_PORT_VC_CAPABILITY1_RC_BO                  8
#define AG_MG_REGS_PORT_VC_CAPABILITY1_RC_BM                  0x00000300

#define AG_MG_REGS_PORT_VC_CAPABILITY1_PATES_BO               10
#define AG_MG_REGS_PORT_VC_CAPABILITY1_PATES_BM               0x00000C00

#define AG_MG_REGS_PORT_VC_CAPABILITY1_RSVD3_BO               12
#define AG_MG_REGS_PORT_VC_CAPABILITY1_RSVD3_BM               0xFFFFF000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PORT_VC_CAPABILITY1_U
{
    struct
    {
        ag_mg_regs_register
            evc : 3,
            rsvd1 : 1,
            lpevc : 3,
            rsvd2 : 1,
            rc : 2,
            pates : 2,
            rsvd3 : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_port_vc_capability1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PORT_VC_CAPAILITY2_RO                      0x00000148
#define AG_MG_REGS_PORT_VC_CAPAILITY2_RM                      0xFFFFFFFF

#define AG_MG_REGS_PORT_VC_CAPAILITY2_VAC_BO                  0
#define AG_MG_REGS_PORT_VC_CAPAILITY2_VAC_BM                  0x000000FF

#define AG_MG_REGS_PORT_VC_CAPAILITY2_RSVD1_BO                8
#define AG_MG_REGS_PORT_VC_CAPAILITY2_RSVD1_BM                0x00FFFF00

#define AG_MG_REGS_PORT_VC_CAPAILITY2_VTF_BO                  24
#define AG_MG_REGS_PORT_VC_CAPAILITY2_VTF_BM                  0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PORT_VC_CAPAILITY2_U
{
    struct
    {
        ag_mg_regs_register
            vac : 8,
            rsvd1 : 16,
            vtf : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_port_vc_capaility2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_RO                     0x0000014C
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_RM                     0xFFFFFFFF

#define AG_MG_REGS_PORT_VC_CTRL_STATUS_LVT_BO                 0
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_LVT_BM                 0x00000001

#define AG_MG_REGS_PORT_VC_CTRL_STATUS_VAS_BO                 1
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_VAS_BM                 0x0000000E

#define AG_MG_REGS_PORT_VC_CTRL_STATUS_RSVD1_BO               4
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_RSVD1_BM               0x0000FFF0

#define AG_MG_REGS_PORT_VC_CTRL_STATUS_VATS_BO                16
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_VATS_BM                0x00010000

#define AG_MG_REGS_PORT_VC_CTRL_STATUS_RSVD2_BO               17
#define AG_MG_REGS_PORT_VC_CTRL_STATUS_RSVD2_BM               0xFFFE0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PORT_VC_CTRL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            lvt : 1,
            vas : 3,
            rsvd1 : 12,
            vats : 1,
            rsvd2 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_port_vc_ctrl_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC_RESOURCE_CAP_RO                         0x00000150
#define AG_MG_REGS_VC_RESOURCE_CAP_RM                         0xFF7FFFFF

#define AG_MG_REGS_VC_RESOURCE_CAP_PAC_BO                     0
#define AG_MG_REGS_VC_RESOURCE_CAP_PAC_BM                     0x000000FF

#define AG_MG_REGS_VC_RESOURCE_CAP_RSVD1_BO                   8
#define AG_MG_REGS_VC_RESOURCE_CAP_RSVD1_BM                   0x00003F00

#define AG_MG_REGS_VC_RESOURCE_CAP_RSVD2_BO                   14
#define AG_MG_REGS_VC_RESOURCE_CAP_RSVD2_BM                   0x00004000

#define AG_MG_REGS_VC_RESOURCE_CAP_RSTS_BO                    15
#define AG_MG_REGS_VC_RESOURCE_CAP_RSTS_BM                    0x00008000

#define AG_MG_REGS_VC_RESOURCE_CAP_MTS_BO                     16
#define AG_MG_REGS_VC_RESOURCE_CAP_MTS_BM                     0x007F0000

#define AG_MG_REGS_VC_RESOURCE_CAP_PATO_BO                    24
#define AG_MG_REGS_VC_RESOURCE_CAP_PATO_BM                    0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_RESOURCE_CAP_U
{
    struct
    {
        ag_mg_regs_register
            pac : 8,
            rsvd1 : 6,
            rsvd2 : 1,
            rsts : 1,
            mts : 7,
            fill0 : 1,
            pato : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_resource_cap_u;
#endif


/* 
 * Initialization value: 0x800000FF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_VC_RESPONSE_CTRL_RO                        0x00000154
#define AG_MG_REGS_VC_RESPONSE_CTRL_RM                        0x87FFFFFF

#define AG_MG_REGS_VC_RESPONSE_CTRL_TVM0_BO                   0
#define AG_MG_REGS_VC_RESPONSE_CTRL_TVM0_BM                   0x00000001

#define AG_MG_REGS_VC_RESPONSE_CTRL_TVM_BO                    1
#define AG_MG_REGS_VC_RESPONSE_CTRL_TVM_BM                    0x000000FE

#define AG_MG_REGS_VC_RESPONSE_CTRL_RSVD1_BO                  8
#define AG_MG_REGS_VC_RESPONSE_CTRL_RSVD1_BM                  0x0000FF00

#define AG_MG_REGS_VC_RESPONSE_CTRL_LPAT_BO                   16
#define AG_MG_REGS_VC_RESPONSE_CTRL_LPAT_BM                   0x00010000

#define AG_MG_REGS_VC_RESPONSE_CTRL_PAS_BO                    17
#define AG_MG_REGS_VC_RESPONSE_CTRL_PAS_BM                    0x00FE0000

#define AG_MG_REGS_VC_RESPONSE_CTRL_VCID_BO                   24
#define AG_MG_REGS_VC_RESPONSE_CTRL_VCID_BM                   0x07000000

#define AG_MG_REGS_VC_RESPONSE_CTRL_VCEN_BO                   31
#define AG_MG_REGS_VC_RESPONSE_CTRL_VCEN_BM                   0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_RESPONSE_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            tvm0 : 1,
            tvm : 7,
            rsvd1 : 8,
            lpat : 1,
            pas : 7,
            vcid : 3,
            fill0 : 4,
            vcen : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_response_ctrl_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC_RESOURCE_STATUS_RO                      0x00000158
#define AG_MG_REGS_VC_RESOURCE_STATUS_RM                      0xFFFFFFFF

#define AG_MG_REGS_VC_RESOURCE_STATUS_PATS_BO                 0
#define AG_MG_REGS_VC_RESOURCE_STATUS_PATS_BM                 0x00000001

#define AG_MG_REGS_VC_RESOURCE_STATUS_VCNP_BO                 1
#define AG_MG_REGS_VC_RESOURCE_STATUS_VCNP_BM                 0x00000002

#define AG_MG_REGS_VC_RESOURCE_STATUS_RSVD1_BO                2
#define AG_MG_REGS_VC_RESOURCE_STATUS_RSVD1_BM                0x0000FFFC

#define AG_MG_REGS_VC_RESOURCE_STATUS_RSVD2_BO                16
#define AG_MG_REGS_VC_RESOURCE_STATUS_RSVD2_BM                0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_RESOURCE_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            pats : 1,
            vcnp : 1,
            rsvd1 : 14,
            rsvd2 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_resource_status_u;
#endif


/* Application Layer (AP) section */

/* 
 * Initialization value: 0x00000008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CONFIGURE_REG_RO                                                        0x00001000
#define AG_MG_REGS_CONFIGURE_REG_RM                                                        0xFF01FFFF

#define AG_MG_REGS_CONFIGURE_REG_SOFT_RESET_EN_BO                                                 0
#define AG_MG_REGS_CONFIGURE_REG_SOFT_RESET_EN_BM                                                 0x00000001

#define AG_MG_REGS_CONFIGURE_REG_FORCE_HOT_RESET_BO                                               1
#define AG_MG_REGS_CONFIGURE_REG_FORCE_HOT_RESET_BM                                               0x00000002

#define AG_MG_REGS_CONFIGURE_REG_BYTE_SWAPPING_PAYLOAD_BO                                         2
#define AG_MG_REGS_CONFIGURE_REG_BYTE_SWAPPING_PAYLOAD_BM                                         0x00000004

#define AG_MG_REGS_CONFIGURE_REG_CFG_PAR_PDD_BO                                                   3
#define AG_MG_REGS_CONFIGURE_REG_CFG_PAR_PDD_BM                                                   0x00000008

#define AG_MG_REGS_CONFIGURE_REG_A2T_BKEND_REG_BO                                                 4
#define AG_MG_REGS_CONFIGURE_REG_A2T_BKEND_REG_BM                                                 0x00000010

#define AG_MG_REGS_AG_MG_REGS_CONFIGURE_REG_RC_BUS_BO_RPT                                         5
#define AG_MG_REGS_AG_MG_REGS_CONFIGURE_REG_RC_BUS_BM_RPT                                         0x00001FE0

#define AG_MG_REGS_CONFIGURE_REG_SOFTWARE_POWER_STATE_BO                                          13
#define AG_MG_REGS_CONFIGURE_REG_SOFTWARE_POWER_STATE_BM                                          0x00006000

#define AG_MG_REGS_CONFIGURE_REG_MW_STRICT_ORDER_BO                                               15
#define AG_MG_REGS_CONFIGURE_REG_MW_STRICT_ORDER_BM                                               0x00008000

#define AG_MG_REGS_CONFIGURE_REG_MESSAGE_IN_DROP_N_BO                                             16
#define AG_MG_REGS_CONFIGURE_REG_MESSAGE_IN_DROP_N_BM                                             0x00010000

#define AG_MG_REGS_AG_MG_REGS_CONFIGURE_REG_PCIE_CONFIG_FPGA_REV_BO_RPT                            24
#define AG_MG_REGS_AG_MG_REGS_CONFIGURE_REG_PCIE_CONFIG_FPGA_REV_BM_RPT                            0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CONFIGURE_REG_U
{
    struct
    {
        ag_mg_regs_register
            soft_reset_en : 1,
            force_hot_reset : 1,
            byte_swapping_payload : 1,
            cfg_par_pdd : 1,
            a2t_bkend_reg : 1,
            rc_bus_num : 8,
            software_power_state : 2,
            mw_strict_order : 1,
            message_in_drop_n : 1,
            fill0 : 7,
            pcie_config_fpga_rev_num : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_configure_reg_u;
#endif


/* 
 * Initialization value: 0x00000008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_STATUS_REG_RO                                                        0x00001004
#define AG_MG_REGS_STATUS_REG_RM                                                        0x00003FFF

#define AG_MG_REGS_STATUS_REG_SOFTWARE_PROGRESS_BO                                                0
#define AG_MG_REGS_STATUS_REG_SOFTWARE_PROGRESS_BM                                                0x00000001

#define AG_MG_REGS_STATUS_REG_T2A_BKEND_ACK_BO                                                    1
#define AG_MG_REGS_STATUS_REG_T2A_BKEND_ACK_BM                                                    0x00000002

#define AG_MG_REGS_STATUS_REG_EXTERNAL_HOST_BO                                                    2
#define AG_MG_REGS_STATUS_REG_EXTERNAL_HOST_BM                                                    0x00000004

#define AG_MG_REGS_STATUS_REG_PCIE_ENABLE_BO                                                       3
#define AG_MG_REGS_STATUS_REG_PCIE_ENABLE_BM                                                       0x00000008

#define AG_MG_REGS_STATUS_REG_RC_EP_N_BO                                                        4
#define AG_MG_REGS_STATUS_REG_RC_EP_N_BM                                                        0x00000010

#define AG_MG_REGS_STATUS_REG_MPAGE_SELECTED_BO                                                   5
#define AG_MG_REGS_STATUS_REG_MPAGE_SELECTED_BM                                                   0x000000E0

#define AG_MG_REGS_STATUS_REG_T2A_LINK_LTSSM_STATE_BO                                             8
#define AG_MG_REGS_STATUS_REG_T2A_LINK_LTSSM_STATE_BM                                             0x00003F00

#define AG_MG_REGS_STATUS_REG_AXI_TRANSFER_READY_BO                                             14
#define AG_MG_REGS_STATUS_REG_AXI_TRANSFER_READY_BM                                             0x00004000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            software_progress : 1,
            t2a_bkend_ack : 1,
            external_host : 1,
            pcie_enable : 1,
            rc_ep_n : 1,
            mpage_selected : 3,
            t2a_link_ltssm_state : 6,
            axi_transfer_ready : 1,
            fill0 : 17;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_status_reg_u;
#endif


/* 
 * Initialization value: 0x80000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CORE_DEBUG_REG_RO                                                        0x00001008
#define AG_MG_REGS_CORE_DEBUG_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_CORE_DEBUG_REG_A2T_LINK_DEBUG_SEL_BO                                           0
#define AG_MG_REGS_CORE_DEBUG_REG_A2T_LINK_DEBUG_SEL_BM                                           0x0000FFFF

#define AG_MG_REGS_CORE_DEBUG_REG_A2T_T1_DEBUG_SEL_BO                                             16
#define AG_MG_REGS_CORE_DEBUG_REG_A2T_T1_DEBUG_SEL_BM                                             0x00FF0000

#define AG_MG_REGS_CORE_DEBUG_REG_A2T_LINK_LOOPBACK_BO                                            24
#define AG_MG_REGS_CORE_DEBUG_REG_A2T_LINK_LOOPBACK_BM                                            0x01000000

#define AG_MG_REGS_CORE_DEBUG_REG_A2T_LINK_CONFIG_BO                                              25
#define AG_MG_REGS_CORE_DEBUG_REG_A2T_LINK_CONFIG_BM                                              0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CORE_DEBUG_REG_U
{
    struct
    {
        ag_mg_regs_register
            a2t_link_debug_sel : 16,
            a2t_t1_debug_sel : 8,
            a2t_link_loopback : 1,
            a2t_link_config : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_core_debug_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_LOOPBACK_FAIL_STATUS_RO                                                        0x0000100C
#define AG_MG_REGS_LOOPBACK_FAIL_STATUS_RM                                                        0x000001FF

#define AG_MG_REGS_LOOPBACK_FAIL_STATUS_T2A_LINK_LOOPBACK_VALID_BO                                0
#define AG_MG_REGS_LOOPBACK_FAIL_STATUS_T2A_LINK_LOOPBACK_VALID_BM                                0x00000001

#define AG_MG_REGS_LOOPBACK_FAIL_STATUS_T2A_LINK_LOOPBACK_FAIL_BO                                 1
#define AG_MG_REGS_LOOPBACK_FAIL_STATUS_T2A_LINK_LOOPBACK_FAIL_BM                                 0x000001FE

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LOOPBACK_FAIL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            t2a_link_loopback_valid : 1,
            t2a_link_loopback_fail : 8,
            fill0 : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_loopback_fail_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE0_UP_RO                                                        0x00001010
#define AG_MG_REGS_MPAGE0_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE0_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE0_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE0_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage0_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE0_LO_RO                                                        0x00001014
#define AG_MG_REGS_MPAGE0_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE0_LO_MPAGE_EN_BO                                               0
#define AG_MG_REGS_MPAGE0_LO_MPAGE_EN_BM                                               0x00000001

#define AG_MG_REGS_MPAGE0_LO_MPAGE_TR_CLASS_BO                                         1
#define AG_MG_REGS_MPAGE0_LO_MPAGE_TR_CLASS_BM                                         0x0000000E

#define AG_MG_REGS_MPAGE0_LO_MPAGE_CONFIG_EN_BO                                        4
#define AG_MG_REGS_MPAGE0_LO_MPAGE_CONFIG_EN_BM                                        0x00000010

#define AG_MG_REGS_MPAGE0_LO_MPAGE_CONFIG_TYPE_BO                                      5
#define AG_MG_REGS_MPAGE0_LO_MPAGE_CONFIG_TYPE_BM                                      0x00000020

#define AG_MG_REGS_MPAGE0_LO_MPAGE_DEVICE_NUMBER_BO                                    6
#define AG_MG_REGS_MPAGE0_LO_MPAGE_DEVICE_NUMBER_BM                                    0x000007C0

#define AG_MG_REGS_MPAGE0_LO_MPAGE_BUS_NUMBER_BO                                       11
#define AG_MG_REGS_MPAGE0_LO_MPAGE_BUS_NUMBER_BM                                       0x0007F800

#define AG_MG_REGS_MPAGE0_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE0_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE0_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                               25
#define AG_MG_REGS_MPAGE0_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                               0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE0_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage0_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE1_UP_RO                                                        0x00001018
#define AG_MG_REGS_MPAGE1_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE1_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE1_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE1_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage1_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE1_LO_RO                                                        0x0000101C
#define AG_MG_REGS_MPAGE1_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE1_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE1_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE1_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE1_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE1_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE1_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE1_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE1_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE1_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE1_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE1_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE1_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE1_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE1_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE1_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE1_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE1_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage1_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE2_UP_RO                                                        0x00001020
#define AG_MG_REGS_MPAGE2_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE2_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE2_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE2_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage2_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE2_LO_RO                                                        0x00001024
#define AG_MG_REGS_MPAGE2_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE2_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE2_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE2_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE2_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE2_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE2_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE2_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE2_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE2_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE2_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE2_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE2_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE2_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE2_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE2_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE2_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE2_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage2_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE3_UP_RO                                                        0x00001028
#define AG_MG_REGS_MPAGE3_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE3_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE3_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE3_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage3_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE3_LO_RO                                                        0x0000102C
#define AG_MG_REGS_MPAGE3_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE3_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE3_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE3_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE3_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE3_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE3_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE3_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE3_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE3_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE3_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE3_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE3_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE3_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE3_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE3_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE3_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE3_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage3_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE4_UP_RO                                                        0x00001030
#define AG_MG_REGS_MPAGE4_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE4_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE4_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE4_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage4_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE4_LO_RO                                                        0x00001034
#define AG_MG_REGS_MPAGE4_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE4_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE4_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE4_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE4_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE4_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE4_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE4_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE4_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE4_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE4_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE4_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE4_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE4_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE4_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE4_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE4_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE4_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage4_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE5_UP_RO                                                        0x00001038
#define AG_MG_REGS_MPAGE5_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE5_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE5_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE5_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage5_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE5_LO_RO                                                        0x0000103C
#define AG_MG_REGS_MPAGE5_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE5_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE5_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE5_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE5_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE5_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE5_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE5_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE5_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE5_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE5_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE5_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE5_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE5_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE5_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE5_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE5_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE5_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage5_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE6_UP_RO                                                        0x00001040
#define AG_MG_REGS_MPAGE6_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE6_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE6_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE6_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage6_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE6_LO_RO                                                        0x00001044
#define AG_MG_REGS_MPAGE6_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE6_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE6_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE6_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE6_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE6_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE6_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE6_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE6_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE6_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE6_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE6_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE6_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE6_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE6_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE6_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE6_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE6_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage6_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE7_UP_RO                                                        0x00001048
#define AG_MG_REGS_MPAGE7_UP_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MPAGE7_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BO                                   0
#define AG_MG_REGS_MPAGE7_UP_MPAGE_PCIE_UPPER_ADDRESS_63TO32_BM                                   0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE7_UP_U
{
    struct
    {
        ag_mg_regs_register
            mpage_pcie_upper_address_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage7_up_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MPAGE7_LO_RO                                                        0x0000104C
#define AG_MG_REGS_MPAGE7_LO_RM                                                        0xFE07FFFF

#define AG_MG_REGS_MPAGE7_LO_MPAGE_EN_BO                                                        0
#define AG_MG_REGS_MPAGE7_LO_MPAGE_EN_BM                                                        0x00000001

#define AG_MG_REGS_MPAGE7_LO_MPAGE_TR_CLASS_BO                                                    1
#define AG_MG_REGS_MPAGE7_LO_MPAGE_TR_CLASS_BM                                                    0x0000000E

#define AG_MG_REGS_MPAGE7_LO_MPAGE_CONFIG_EN_BO                                                   4
#define AG_MG_REGS_MPAGE7_LO_MPAGE_CONFIG_EN_BM                                                   0x00000010

#define AG_MG_REGS_MPAGE7_LO_MPAGE_CONFIG_TYPE_BO                                                 5
#define AG_MG_REGS_MPAGE7_LO_MPAGE_CONFIG_TYPE_BM                                                 0x00000020

#define AG_MG_REGS_MPAGE7_LO_MPAGE_DEVICE_NUMBER_BO                                               6
#define AG_MG_REGS_MPAGE7_LO_MPAGE_DEVICE_NUMBER_BM                                               0x000007C0

#define AG_MG_REGS_MPAGE7_LO_MPAGE_BUS_NUMBER_BO                                                  11
#define AG_MG_REGS_MPAGE7_LO_MPAGE_BUS_NUMBER_BM                                                  0x0007F800

#define AG_MG_REGS_MPAGE7_LO_MPAGE_FUNCTION_NUMBER_BO                                  19
#define AG_MG_REGS_MPAGE7_LO_MPAGE_FUNCTION_NUMBER_BM                                  0x00380000

#define AG_MG_REGS_MPAGE7_LO_MPAGE_PCIE_UPPER_ADDRESS_BO                                   25
#define AG_MG_REGS_MPAGE7_LO_MPAGE_PCIE_UPPER_ADDRESS_BM                                   0xFE000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MPAGE7_LO_U
{
    struct
    {
        ag_mg_regs_register
            mpage_en : 1,
            mpage_tr_class : 3,
            mpage_config_en : 1,
            mpage_config_type : 1,
            mpage_device_number : 5,
            mpage_bus_number : 8,
            mpage_function_number : 3,
            fill0 : 3,
            mpage_pcie_upper_address_31to27 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mpage7_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE0_BAR0_REG_RO                                                  0x00001050
#define AG_MG_REGS_TPAGE0_BAR0_REG_RM                                                  0x80001FFF

#define AG_MG_REGS_TPAGE0_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                        0
#define AG_MG_REGS_TPAGE0_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                        0x00001FFF

#define AG_MG_REGS_TPAGE0_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                 31
#define AG_MG_REGS_TPAGE0_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                 0x80000000
#define AG_MG_REGS_TPAGE0_BAR0_REG_AXI_SZ128_BM									AG_MG_REGS_TPAGE0_BAR0_REG_TPAGE_BAR0_CONFIG_BM

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE0_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage0_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE1_BAR0_REG_RO                                                        0x00001054
#define AG_MG_REGS_TPAGE1_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE1_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE1_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE1_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE1_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE1_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage1_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE2_BAR0_REG_RO                                                        0x00001058
#define AG_MG_REGS_TPAGE2_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE2_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE2_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE2_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE2_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE2_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage2_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE3_BAR0_REG_RO                                                        0x0000105C
#define AG_MG_REGS_TPAGE3_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE3_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE3_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE3_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE3_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE3_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage3_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE4_BAR0_REG_RO                                                        0x00001060
#define AG_MG_REGS_TPAGE4_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE4_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE4_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE4_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE4_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE4_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage4_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE5_BAR0_REG_RO                                                        0x00001064
#define AG_MG_REGS_TPAGE5_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE5_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE5_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE5_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE5_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE5_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage5_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE6_BAR0_REG_RO                                                        0x00001068
#define AG_MG_REGS_TPAGE6_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE6_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE6_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE6_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE6_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE6_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage6_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE7_BAR0_REG_RO                                                        0x0000106C
#define AG_MG_REGS_TPAGE7_BAR0_REG_RM                                                        0x80001FFF

#define AG_MG_REGS_TPAGE7_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BO                                   0
#define AG_MG_REGS_TPAGE7_BAR0_REG_TPAGE_BAR0_AXI_UPPER_ADDR_BM                                   0x00001FFF

#define AG_MG_REGS_TPAGE7_BAR0_REG_TPAGE_BAR0_CONFIG_BO                                           31
#define AG_MG_REGS_TPAGE7_BAR0_REG_TPAGE_BAR0_CONFIG_BM                                           0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE7_BAR0_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar0_axi_upper_addr : 13,
            fill0 : 18,
            tpage_bar0_config : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage7_bar0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE0_BAR1_REG_RO                                                        0x00001070
#define AG_MG_REGS_TPAGE0_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE0_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE0_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE0_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE0_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE0_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage0_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE1_BAR1_REG_RO                                                        0x00001074
#define AG_MG_REGS_TPAGE1_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE1_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE1_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE1_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE1_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE1_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage1_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE2_BAR1_REG_RO                                                        0x00001078
#define AG_MG_REGS_TPAGE2_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE2_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE2_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE2_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE2_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE2_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage2_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE3_BAR1_REG_RO                                                        0x0000107C
#define AG_MG_REGS_TPAGE3_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE3_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE3_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE3_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE3_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE3_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage3_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE4_BAR1_REG_RO                                                        0x00001080
#define AG_MG_REGS_TPAGE4_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE4_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE4_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE4_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE4_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE4_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage4_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE5_BAR1_REG_RO                                                        0x00001084
#define AG_MG_REGS_TPAGE5_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE5_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE5_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE5_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE5_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE5_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage5_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE6_BAR1_REG_RO                                                        0x00001088
#define AG_MG_REGS_TPAGE6_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE6_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE6_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE6_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE6_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE6_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage6_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE7_BAR1_REG_RO                                                        0x0000108C
#define AG_MG_REGS_TPAGE7_BAR1_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE7_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE7_BAR1_REG_TPAGE_BAR1_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE7_BAR1_REG_TPAGE_BAR1_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE7_BAR1_REG_TPAGE_BAR1_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE7_BAR1_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar1_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar1_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage7_bar1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE0_BAR2_REG_RO                                                        0x00001090
#define AG_MG_REGS_TPAGE0_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE0_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE0_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE0_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE0_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE0_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage0_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE1_BAR2_REG_RO                                                        0x00001094
#define AG_MG_REGS_TPAGE1_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE1_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE1_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE1_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE1_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE1_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage1_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE2_BAR2_REG_RO                                                        0x00001098
#define AG_MG_REGS_TPAGE2_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE2_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE2_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE2_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE2_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE2_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage2_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE3_BAR2_REG_RO                                                        0x0000109C
#define AG_MG_REGS_TPAGE3_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE3_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE3_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE3_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE3_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE3_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage3_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE4_BAR2_REG_RO                                                        0x000010A0
#define AG_MG_REGS_TPAGE4_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE4_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE4_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE4_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE4_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE4_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage4_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE5_BAR2_REG_RO                                                        0x000010A4
#define AG_MG_REGS_TPAGE5_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE5_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE5_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE5_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE5_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE5_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage5_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE6_BAR2_REG_RO                                                        0x000010A8
#define AG_MG_REGS_TPAGE6_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE6_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE6_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE6_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE6_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE6_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage6_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TPAGE7_BAR2_REG_RO                                                        0x000010AC
#define AG_MG_REGS_TPAGE7_BAR2_REG_RM                                                        0x80003FFF

#define AG_MG_REGS_TPAGE7_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BO                               0
#define AG_MG_REGS_TPAGE7_BAR2_REG_TPAGE_BAR2_AXI_UPPER_ADDR_BM                               0x00003FFF

#define AG_MG_REGS_TPAGE7_BAR2_REG_TPAGE_BAR2_BIT31_BO                                        31
#define AG_MG_REGS_TPAGE7_BAR2_REG_TPAGE_BAR2_BIT31_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TPAGE7_BAR2_REG_U
{
    struct
    {
        ag_mg_regs_register
            tpage_bar2_axi_upper_addr : 14,
            fill0 : 17,
            tpage_bar2_bit31 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tpage7_bar2_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MESSAGE_IN_FIFO_RO                                                        0x000010B0
#define AG_MG_REGS_MESSAGE_IN_FIFO_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MESSAGE_IN_FIFO_INPUT_MESSAGE_BO                                               0
#define AG_MG_REGS_MESSAGE_IN_FIFO_INPUT_MESSAGE_BM                                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MESSAGE_IN_FIFO_U
{
    struct
    {
        ag_mg_regs_register
            input_message;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_message_in_fifo_u;
#endif


/* 
 * Initialization value: 0x00000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MESSAGE_IN_FIFO_STATUS_RO                                                      0x000010B4
#define AG_MG_REGS_MESSAGE_IN_FIFO_STATUS_RM                                                      0x00000003

#define AG_MG_REGS_MESSAGE_IN_FIFO_STATUS_MESSAGE_IN_FIFO_FLAGS_BO                                0
#define AG_MG_REGS_MESSAGE_IN_FIFO_STATUS_MESSAGE_IN_FIFO_FLAGS_BM                                0x00000003

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MESSAGE_IN_FIFO_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            message_in_fifo_flags : 2,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_message_in_fifo_status_u;
#endif


/* 
 * Initialization value: 0x00000400  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MESSAGE_OUT_RO                                                        0x000010B8
#define AG_MG_REGS_MESSAGE_OUT_RM                                                        0x01FF0FFF

#define AG_MG_REGS_MESSAGE_OUT_MESSAGE_OUT_PACKET_RDY_BO                                          0
#define AG_MG_REGS_MESSAGE_OUT_MESSAGE_OUT_PACKET_RDY_BM                                          0x00000001

#define AG_MG_REGS_MESSAGE_OUT_MESSAGE_CODE_BO                                                    1
#define AG_MG_REGS_MESSAGE_OUT_MESSAGE_CODE_BM                                                    0x000001FE

#define AG_MG_REGS_MESSAGE_OUT_MESSAGE_TYPE_R2_R0_BO                                              9
#define AG_MG_REGS_MESSAGE_OUT_MESSAGE_TYPE_R2_R0_BM                                              0x00000E00

#define AG_MG_REGS_AG_MG_REGS_MESSAGE_OUT_DESTINATION_BUS_BO_RPT                                  16
#define AG_MG_REGS_AG_MG_REGS_MESSAGE_OUT_DESTINATION_BUS_BM_RPT                                  0x00FF0000

#define AG_MG_REGS_MESSAGE_OUT_VENDOR_DEFINED_MSG_BO                                              24
#define AG_MG_REGS_MESSAGE_OUT_VENDOR_DEFINED_MSG_BM                                              0x01000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MESSAGE_OUT_U
{
    struct
    {
        ag_mg_regs_register
            message_out_packet_rdy : 1,
            message_code : 8,
            message_type_r2_r0 : 3,
            fill1 : 4,
            destination_bus_num : 8,
            vendor_defined_msg : 1,
            fill0 : 7;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_message_out_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_RO                                                        0x000010C0
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ERR_INT_BO                   0
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ERR_INT_BM                   0x00000001

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_WRT_CONF_WRT_RTN_PKT_ERR_INT_BO                       1
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_WRT_CONF_WRT_RTN_PKT_ERR_INT_BM                       0x00000002

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_WRT_CONF_WRT_RTN_COPL_ID_ERR_INT_BO                   2
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_WRT_CONF_WRT_RTN_COPL_ID_ERR_INT_BM                   0x00000004

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MESSAGE_DROP_INT_BO                                       3
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MESSAGE_DROP_INT_BM                                       0x00000008

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MESSAGE_ARRIVED_INT_BO                                    4
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MESSAGE_ARRIVED_INT_BM                                    0x00000010

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_RD_REQ_DEC_ERROR_INT_BO                               5
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_RD_REQ_DEC_ERROR_INT_BM                               0x00000020

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_RD_COMPL_ERR_INT_BO                                   6
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_RD_COMPL_ERR_INT_BM                                   0x00000040

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_RD_COMPL_TIMEOUT_ERR_INT_BO                           7
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SLV_RD_COMPL_TIMEOUT_ERR_INT_BM                           0x00000080

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MST_WR_PKT_DROP_INT_BO                                    8
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MST_WR_PKT_DROP_INT_BM                                    0x00000100

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MW_BAR_MISMATCH_INT_BO                                    9
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MW_BAR_MISMATCH_INT_BM                                    0x00000200

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MST_RD_REQ_PKT_DROP_INT_BO                                10
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MST_RD_REQ_PKT_DROP_INT_BM                                0x00000400

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MR_REQ_MISMATCH_INT_BO                                    11
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MR_REQ_MISMATCH_INT_BM                                    0x00000800

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_LINK_TL_FSM_ERR_INT_BO                                    12
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_LINK_TL_FSM_ERR_INT_BM                                    0x00001000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_EGR_ERR_INT_BO                                        13
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_EGR_ERR_INT_BM                                        0x00002000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_REQ_PROC_ERR_INT_BO                                   14
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_REQ_PROC_ERR_INT_BM                                   0x00004000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_CPL_TO_ERR_INT_BO                                     15
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_CPL_TO_ERR_INT_BM                                     0x00008000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_IGR_ERR_INT_BO                                        16
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_IGR_ERR_INT_BM                                        0x00010000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_FN_INDP_ERR_INT_BO                                    17
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_FN_INDP_ERR_INT_BM                                    0x00020000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_FN_INDP_OTHER_ERR_INT_BO                              18
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_FN_INDP_OTHER_ERR_INT_BM                              0x00040000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_PARITY_ERR_INT_BO                                     19
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_T2A_PARITY_ERR_INT_BM                                     0x00080000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MW_RESP_ERR_INT_BO                                        20
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MW_RESP_ERR_INT_BM                                        0x00100000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_APPL_FIFO_ERROR_INT_BO                                    21
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_APPL_FIFO_ERROR_INT_BM                                    0x00200000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_APPL_AHB_ERR_STATE_BO                                     22
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_APPL_AHB_ERR_STATE_BM                                     0x00400000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MR_REQ_NO_LENGTH_RECVD_INT_BO                             23
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MR_REQ_NO_LENGTH_RECVD_INT_BM                             0x00800000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_ERROR_MESSAGE_RECEIVED_INT_BO                             24
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_ERROR_MESSAGE_RECEIVED_INT_BM                             0x01000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_RC_PME_TO_ACK_RCVD_INT_BO                                 25
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_RC_PME_TO_ACK_RCVD_INT_BM                                 0x02000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_DESERRT_INTA_RCVD_INT_BO                                  26
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_DESERRT_INTA_RCVD_INT_BM                                  0x04000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_ASSERT_INTA_RCVD_INT_BO                                   27
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_ASSERT_INTA_RCVD_INT_BM                                   0x08000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_HOT_RESET_RECVD_INT_BO                                    28
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_HOT_RESET_RECVD_INT_BM                                    0x10000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_PME_TURN_OFF_MESSAGE_RECVD_INT_BO                         29
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_PME_TURN_OFF_MESSAGE_RECVD_INT_BM                         0x20000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SYS_ERR_INT_RCVD_INT_BO                                   30
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_SYS_ERR_INT_RCVD_INT_BM                                   0x40000000

#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MSI_RECEIVED_INT_BO                                       31
#define AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_MSI_RECEIVED_INT_BM                                       0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            slv_wrt_conf_wrt_rtn_timeout_err_int : 1,
            slv_wrt_conf_wrt_rtn_pkt_err_int : 1,
            slv_wrt_conf_wrt_rtn_copl_id_err_int : 1,
            message_drop_int : 1,
            message_arrived_int : 1,
            slv_rd_req_dec_error_int : 1,
            slv_rd_compl_err_int : 1,
            slv_rd_compl_timeout_err_int : 1,
            mst_wr_pkt_drop_int : 1,
            mw_bar_mismatch_int : 1,
            mst_rd_req_pkt_drop_int : 1,
            mr_req_mismatch_int : 1,
            link_tl_fsm_err_int : 1,
            t2a_egr_err_int : 1,
            t2a_req_proc_err_int : 1,
            t2a_cpl_to_err_int : 1,
            t2a_igr_err_int : 1,
            t2a_fn_indp_err_int : 1,
            t2a_fn_indp_other_err_int : 1,
            t2a_parity_err_int : 1,
            mw_resp_err_int : 1,
            appl_fifo_error_int : 1,
            appl_ahb_err_state : 1,
            mr_req_no_length_recvd_int : 1,
            error_message_received_int : 1,
            rc_pme_to_ack_rcvd_int : 1,
            deserrt_inta_rcvd_int : 1,
            assert_inta_rcvd_int : 1,
            hot_reset_recvd_int : 1,
            pme_turn_off_message_recvd_int : 1,
            sys_err_int_rcvd_int : 1,
            msi_received_int : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_interrupt_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_RO                                                        0x000010C4
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ERR_INT_BO                   0
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ERR_INT_BM                   0x00000001

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_WRT_CONF_WRT_RTN_PKT_ERR_INT_BO                       1
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_WRT_CONF_WRT_RTN_PKT_ERR_INT_BM                       0x00000002

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_WRT_CONF_WRT_RTN_COPL_ID_ERR_INT_BO                   2
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_WRT_CONF_WRT_RTN_COPL_ID_ERR_INT_BM                   0x00000004

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MESSAGE_DROP_INT_BO                                       3
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MESSAGE_DROP_INT_BM                                       0x00000008

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MESSAGE_ARRIVED_INT_BO                                    4
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MESSAGE_ARRIVED_INT_BM                                    0x00000010

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_RD_REQ_DEC_ERROR_INT_BO                               5
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_RD_REQ_DEC_ERROR_INT_BM                               0x00000020

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_RD_COMPL_ERR_INT_BO                                   6
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_RD_COMPL_ERR_INT_BM                                   0x00000040

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_RD_COMPL_TIMEOUT_ERR_INT_BO                           7
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SLV_RD_COMPL_TIMEOUT_ERR_INT_BM                           0x00000080

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MST_WR_PKT_DROP_INT_BO                                    8
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MST_WR_PKT_DROP_INT_BM                                    0x00000100

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MW_BAR_MISMATCH_INT_BO                                    9
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MW_BAR_MISMATCH_INT_BM                                    0x00000200

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MST_RD_REQ_PKT_DROP_INT_BO                                10
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MST_RD_REQ_PKT_DROP_INT_BM                                0x00000400

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MR_REQ_MISMATCH_INT_BO                                    11
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MR_REQ_MISMATCH_INT_BM                                    0x00000800

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T1_FSM_ERR_INT_BO                                         12
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T1_FSM_ERR_INT_BM                                         0x00001000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_EGR_ERR_INT_BO                                        13
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_EGR_ERR_INT_BM                                        0x00002000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_REQ_PROC_ERR_INT_BO                                   14
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_REQ_PROC_ERR_INT_BM                                   0x00004000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_CPL_TO_ERR_INT_BO                                     15
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_CPL_TO_ERR_INT_BM                                     0x00008000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_IGR_ERR_INT_BO                                        16
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_IGR_ERR_INT_BM                                        0x00010000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_FN_INDP_ERR_INT_BO                                    17
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_FN_INDP_ERR_INT_BM                                    0x00020000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_FN_INDP_OTHER_ERR_INT_BO                              18
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_FN_INDP_OTHER_ERR_INT_BM                              0x00040000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_PARITY_ERR_INT_BO                                     19
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_T2A_PARITY_ERR_INT_BM                                     0x00080000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MW_RESP_ERR_INT_BO                                        20
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MW_RESP_ERR_INT_BM                                        0x00100000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_APPL_FIFO_ERROR_INT_BO                                    21
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_APPL_FIFO_ERROR_INT_BM                                    0x00200000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_APPL_AHB_ERR_STATE_BO                                     22
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_APPL_AHB_ERR_STATE_BM                                     0x00400000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MR_REQ_NO_LATCH_RCVD_INT_BO                               23
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MR_REQ_NO_LATCH_RCVD_INT_BM                               0x00800000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_ERROR_MESSAGE_RECEIVED_INT_BO                             24
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_ERROR_MESSAGE_RECEIVED_INT_BM                             0x01000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_RC_PME_TO_ACK_RCVD_INT_BO                                 25
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_RC_PME_TO_ACK_RCVD_INT_BM                                 0x02000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_DESSERT_INTA_RCVD_INT_BO                                  26
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_DESSERT_INTA_RCVD_INT_BM                                  0x04000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_ASSERT_INTA_RCVD_INT_BO                                   27
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_ASSERT_INTA_RCVD_INT_BM                                   0x08000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_HOT_RESET_RCVD_INT_BO                                     28
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_HOT_RESET_RCVD_INT_BM                                     0x10000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_PME_TURN_OFF_MESSAGE_RCVD_INT_BO                          29
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_PME_TURN_OFF_MESSAGE_RCVD_INT_BM                          0x20000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SYS_ERR_INT_RECVD_INT_BO                                  30
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_SYS_ERR_INT_RECVD_INT_BM                                  0x40000000

#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MSI_RECEIVED_INT_BO                                       31
#define AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_MSI_RECEIVED_INT_BM                                       0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_U
{
    struct
    {
        ag_mg_regs_register
            slv_wrt_conf_wrt_rtn_timeout_err_int : 1,
            slv_wrt_conf_wrt_rtn_pkt_err_int : 1,
            slv_wrt_conf_wrt_rtn_copl_id_err_int : 1,
            message_drop_int : 1,
            message_arrived_int : 1,
            slv_rd_req_dec_error_int : 1,
            slv_rd_compl_err_int : 1,
            slv_rd_compl_timeout_err_int : 1,
            mst_wr_pkt_drop_int : 1,
            mw_bar_mismatch_int : 1,
            mst_rd_req_pkt_drop_int : 1,
            mr_req_mismatch_int : 1,
            t1_fsm_err_int : 1,
            t2a_egr_err_int : 1,
            t2a_req_proc_err_int : 1,
            t2a_cpl_to_err_int : 1,
            t2a_igr_err_int : 1,
            t2a_fn_indp_err_int : 1,
            t2a_fn_indp_other_err_int : 1,
            t2a_parity_err_int : 1,
            mw_resp_err_int : 1,
            appl_fifo_error_int : 1,
            appl_ahb_err_state : 1,
            mr_req_no_latch_rcvd_int : 1,
            error_message_received_int : 1,
            rc_pme_to_ack_rcvd_int : 1,
            dessert_inta_rcvd_int : 1,
            assert_inta_rcvd_int : 1,
            hot_reset_rcvd_int : 1,
            pme_turn_off_message_rcvd_int : 1,
            sys_err_int_recvd_int : 1,
            msi_received_int : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_interrupt_enable_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_RO                                                        0x000010C8
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ERR_INT_BO                    0
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ERR_INT_BM                    0x00000001

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_WRT_CONF_WRT_RTN_PKT_ERR_INT_BO                        1
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_WRT_CONF_WRT_RTN_PKT_ERR_INT_BM                        0x00000002

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_INT_BO                   2
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_INT_BM                   0x00000004

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MESSAGE_DROP_INT_BO                                        3
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MESSAGE_DROP_INT_BM                                        0x00000008

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MESSAGE_ARRIVED_INT_BO                                     4
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MESSAGE_ARRIVED_INT_BM                                     0x00000010

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_RD_REQ_DEC_ERROR_INT_BO                                5
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_RD_REQ_DEC_ERROR_INT_BM                                0x00000020

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_RD_COMPL_ERR_INT_BO                                    6
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_RD_COMPL_ERR_INT_BM                                    0x00000040

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_RD_COMPL_TIMEOUT_ERR_INT_BO                            7
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SLV_RD_COMPL_TIMEOUT_ERR_INT_BM                            0x00000080

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MST_WR_PKT_DROP_INT_BO                                     8
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MST_WR_PKT_DROP_INT_BM                                     0x00000100

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MW_BAR_MISMATCH_INT_BO                                     9
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MW_BAR_MISMATCH_INT_BM                                     0x00000200

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MST_RD_REQ_PKT_DROP_INT_BO                                 10
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MST_RD_REQ_PKT_DROP_INT_BM                                 0x00000400

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MR_REQ_BAR_MISMATCH_INT_BO                                 11
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MR_REQ_BAR_MISMATCH_INT_BM                                 0x00000800

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T1_FSM_ERR_INT_BO                                          12
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T1_FSM_ERR_INT_BM                                          0x00001000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_EGR_ERR_INT_BO                                         13
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_EGR_ERR_INT_BM                                         0x00002000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_REQ_PROC_ERR_INT_BO                                    14
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_REQ_PROC_ERR_INT_BM                                    0x00004000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_CPL_TO_ERR_INT_BO                                      15
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_CPL_TO_ERR_INT_BM                                      0x00008000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_IGR_ERR_INT_BO                                         16
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_IGR_ERR_INT_BM                                         0x00010000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_FN_INDP_ERR_INT_BO                                     17
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_FN_INDP_ERR_INT_BM                                     0x00020000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_FN_INDP_OTHER_ERR_INT_BO                               18
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_FN_INDP_OTHER_ERR_INT_BM                               0x00040000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_PARITY_ERR_INT_BO                                      19
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_T2A_PARITY_ERR_INT_BM                                      0x00080000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MW_RESP_ERR_INT_BO                                         20
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MW_RESP_ERR_INT_BM                                         0x00100000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_APPL_FIFO_ERROR_INT_BO                                     21
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_APPL_FIFO_ERROR_INT_BM                                     0x00200000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_APPL_AHB_ERR_STATE_BO                                      22
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_APPL_AHB_ERR_STATE_BM                                      0x00400000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MR_REQ_NO_LENGTH_RECVD_INT_BO                              23
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MR_REQ_NO_LENGTH_RECVD_INT_BM                              0x00800000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_ERROR_MESSAGE_RECEIVED_INT_BO                              24
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_ERROR_MESSAGE_RECEIVED_INT_BM                              0x01000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_RC_PME_TO_ACK_RCVD_INT_BO                                  25
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_RC_PME_TO_ACK_RCVD_INT_BM                                  0x02000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_DESSERT_INTA_RCVD_INT_BO                                   26
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_DESSERT_INTA_RCVD_INT_BM                                   0x04000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_ASSERT_INTA_RCVD_INT_BO                                    27
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_ASSERT_INTA_RCVD_INT_BM                                    0x08000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_HOT_RESET_RECVD_INT_BO                                     28
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_HOT_RESET_RECVD_INT_BM                                     0x10000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_PME_TURN_OFF_MESSAGE_RECVD_INT_BO                          29
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_PME_TURN_OFF_MESSAGE_RECVD_INT_BM                          0x20000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SYS_ERR_INT_RCVD_INT_BO                                    30
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_SYS_ERR_INT_RCVD_INT_BM                                    0x40000000

#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MSI_RECEIVED_INT_BO                                        31
#define AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_MSI_RECEIVED_INT_BM                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_U
{
    struct
    {
        ag_mg_regs_register
            slv_wrt_conf_wrt_rtn_timeout_err_int : 1,
            slv_wrt_conf_wrt_rtn_pkt_err_int : 1,
            slv_wrt_conf_wrt_rtn_compl_id_err_int : 1,
            message_drop_int : 1,
            message_arrived_int : 1,
            slv_rd_req_dec_error_int : 1,
            slv_rd_compl_err_int : 1,
            slv_rd_compl_timeout_err_int : 1,
            mst_wr_pkt_drop_int : 1,
            mw_bar_mismatch_int : 1,
            mst_rd_req_pkt_drop_int : 1,
            mr_req_bar_mismatch_int : 1,
            t1_fsm_err_int : 1,
            t2a_egr_err_int : 1,
            t2a_req_proc_err_int : 1,
            t2a_cpl_to_err_int : 1,
            t2a_igr_err_int : 1,
            t2a_fn_indp_err_int : 1,
            t2a_fn_indp_other_err_int : 1,
            t2a_parity_err_int : 1,
            mw_resp_err_int : 1,
            appl_fifo_error_int : 1,
            appl_ahb_err_state : 1,
            mr_req_no_length_recvd_int : 1,
            error_message_received_int : 1,
            rc_pme_to_ack_rcvd_int : 1,
            dessert_inta_rcvd_int : 1,
            assert_inta_rcvd_int : 1,
            hot_reset_recvd_int : 1,
            pme_turn_off_message_recvd_int : 1,
            sys_err_int_rcvd_int : 1,
            msi_received_int : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_interrupt_force_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PHY_PCIE_STA0_REG_RO                                                        0x000010CC
#define AG_MG_REGS_PHY_PCIE_STA0_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PHY_PCIE_STA0_REG_PHY_INITIALIZATION_BO                                               0
#define AG_MG_REGS_PHY_PCIE_STA0_REG_PHY_INITIALIZATION_BM                                               0x00000001

#define AG_MG_REGS_PHY_PCIE_STA0_REG_PHY_PLL_LOCK_STATUS_BO                                               31
#define AG_MG_REGS_PHY_PCIE_STA0_REG_PHY_PLL_LOCK_STATUS_BM                                               0x80000000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PHY_PCIE_STA0_REG_U
{
    struct
    {
        ag_mg_regs_register
            phy_initialization : 1,
            fill0 : 30,
            phy_pll_lock_status : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_phy_pcie_sta0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PHY_PCIE_STA1_REG_RO                                                        0x000010D0
#define AG_MG_REGS_PHY_PCIE_STA1_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNAPRBSSYNC_BO                                         0
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNAPRBSSYNC_BM                                         0x00000001

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNAPRBSERRH_BO                                         1
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNAPRBSERRH_BM                                         0x00000002

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNAPRBSERR_BO                                          2
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNAPRBSERR_BM                                          0x00000004

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNBPRBSSYNC_BO                                         8
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNBPRBSSYNC_BM                                         0x00000100

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNBPRBSERRH_BO                                         9
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNBPRBSERRH_BM                                         0x00000200

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNBPRBSERR_BO                                          10
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNBPRBSERR_BM                                          0x00000400

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNCPRBSSYNC_BO                                         16
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNCPRBSSYNC_BM                                         0x00010000

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNCPRBSERRH_BO                                         17
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNCPRBSERRH_BM                                         0x00020000

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNCPRBSER_BO                                           18
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNCPRBSER_BM                                           0x00040000

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNDPRBSSYNC_BO                                         24
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNDPRBSSYNC_BM                                         0x01000000

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNDPRBSERRH_BO                                         25
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNDPRBSERRH_BM                                         0x02000000

#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNDPRBSERR_BO                                          26
#define AG_MG_REGS_PHY_PCIE_STA1_REG_HSSLNDPRBSERR_BM                                          0x04000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PHY_PCIE_STA1_REG_U
{
    struct
    {
        ag_mg_regs_register
		hsslnaprbssync : 1,
		hsslnaprbserrh : 1,
		hsslnaprbserr : 1,
		fill0 : 5,
		hsslnbprbssync : 1,
		hsslnbprbserrh : 1,
		hsslnbprbserr : 1,
		fill1 : 5,
		hsslncprbssync : 1,
		hsslncprbserrh : 1,
		hsslncprbser : 1,
		fill2 : 5,
		hsslndprbssync : 1,
		hsslndprbserrh : 1,
		hsslndprbserr : 1,
		fill3 : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_phy_pcie_sta1_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_PHY_CTRL0_REG_RO                                                        0x000010D4
#define AG_MG_REGS_PCIE_PHY_CTRL0_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PCIE_PHY_CTRL0_REG_PHY_PLL_LOCKED_BY_SOFTWARE_BO                             2
#define AG_MG_REGS_PCIE_PHY_CTRL0_REG_PHY_PLL_LOCKED_BY_SOFTWARE_BM                             0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_PHY_CTR0_REG_U
{
    struct
    {
        ag_mg_regs_register
			fill0 : 1,
			phy_pll_locked_by_software : 1,
            fill1 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_phy_ctrl0_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RO                                                         0x000010DC
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RM                                                         0xFFFFFFFF

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNAPRBSSEL_BO                                           0
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNAPRBSSEL_BM                                           0x00000007

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNADATALOPEN_BO                                         3
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNADATALOPEN_BM                                         0x00000008

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNAPARLPBK_BO                                           4
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNAPARLPBK_BM                                           0x00000010

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXAPRBSEN_BO                                               5
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXAPRBSEN_BM                                               0x00000020

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNAPRBSRT_BO                                            6
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNAPRBSRT_BM                                            0x00000040

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXAPRBSEN_BO                                               7
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXAPRBSEN_BM                                               0x00000080

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBPRBSSEL_BO                                           8
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBPRBSSEL_BM                                           0x00000700

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBDATALOOPEN_BO                                        11
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBDATALOOPEN_BM                                        0x00000800

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBPARLPBK_BO                                           12
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBPARLPBK_BM                                           0x00001000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXBPRBSEN_BO                                               13
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXBPRBSEN_BM                                               0x00002000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBPRBSRST_BO                                           14
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNBPRBSRST_BM                                           0x00004000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXBPRBSEN_BO                                               15
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXBPRBSEN_BM                                               0x00008000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCPRBSSEL_BO                                           16
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCPRBSSEL_BM                                           0x00070000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCDATALOOPEN_BO                                        19
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCDATALOOPEN_BM                                        0x00080000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCPARLPBK_BO                                           20
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCPARLPBK_BM                                           0x00100000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXCPRBSEN_BO                                               21
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXCPRBSEN_BM                                               0x00200000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCPRBSRST_BO                                           22
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNCPRBSRST_BM                                           0x00400000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXCPRBSEN_BO                                               23
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXCPRBSEN_BM                                               0x00800000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDPRBSSEL_BO                                           24
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDPRBSSEL_BM                                           0x07000000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDDATALOOPEN_BO                                        27
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDDATALOOPEN_BM                                        0x08000000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDPARLPBK_BO                                           28
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDPARLPBK_BM                                           0x10000000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXDPRBSEN_BO                                               29
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_TXDPRBSEN_BM                                               0x20000000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDPRBSRT_BO                                            30
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_HSSLNDPRBSRT_BM                                            0x40000000

#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXDPRBSEN_BO                                               31
#define AG_MG_REGS_PCIE_PHY_CTRL2_REG_RXDPRBSEN_BM                                               0x80000000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_PHY_CTRL2_REG_U
{
    struct
    {
        ag_mg_regs_register
        hsslnaprbssel : 3,
        hsslnadatalopen : 1,
        hsslnaparlpbk : 1,
        txaprbsen : 1,
        hsslnaprbsrt : 1,
        rxaprbsen : 1,
        hsslnbprbssel : 3,
        hsslnbdataloopen : 1,
        hsslnbparlpbk : 1,
        txbprbsen : 1,
        hsslnbprbsrst : 1,
        rxbprbsen : 1,
        hsslncprbsse : 3,
        hsslncdataloopen : 1,
        hsslncparlpbk : 1,
        txcprbsen : 1,
        hsslncprbsrst : 1,
        rxcprbsen : 1,
        hsslndprbssel : 3,
        hsslnddataloopen : 1,
        hsslndparlpbk : 1,
        txdprbsen : 1,
        hsslndprbsrt : 1,
        rxdprbsen : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_phy_ctrl2_reg_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_RO                                                   0x000010E0
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_RM                                                   0x000003FF

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_UNDERFLOW_BO                                    0
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_UNDERFLOW_BM                                    0x00000001

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_OVERFLOW_BO                                     1
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_OVERFLOW_BM                                     0x00000002

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_EMPTY_BO                                        2
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_EMPTY_BM                                        0x00000004

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_AEMPTY_BO                                       3
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_AEMPTY_BM                                       0x00000008

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_AFULL_BO                                        4
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_AFULL_BM                                        0x00000010

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_FULL_BO                                         5
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_FULL_BM                                         0x00000020

#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_DEPTH_BO                                        6
#define AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_FIFO_DEPTH_BM                                        0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_conf_wr_cpl_tmp_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_RO                                                        0x000010E4
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_UNDERFLOW_BO                                          0
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_UNDERFLOW_BM                                          0x00000001

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_OVERFLOW_BO                                           1
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_OVERFLOW_BM                                           0x00000002

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_EMPTY_BO                                              2
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_EMPTY_BM                                              0x00000004

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_AEMPTY_BO                                             3
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_AEMPTY_BM                                             0x00000008

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_AFULL_BO                                              4
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_AFULL_BM                                              0x00000010

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_FULL_BO                                               5
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_FULL_BM                                               0x00000020

#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_DEPTH_BO                                              6
#define AG_MG_REGS_DEC_ERROR_FIFO_STAT_FIFO_DEPTH_BM                                              0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEC_ERROR_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_dec_error_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_RO                                                        0x000010E8
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_UNDERFLOW_BO                                         0
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_UNDERFLOW_BM                                         0x00000001

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_OVERFLOW_BO                                          1
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_OVERFLOW_BM                                          0x00000002

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_EMPTY_BO                                             2
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_EMPTY_BM                                             0x00000004

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_AEMPTY_BO                                            3
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_AEMPTY_BM                                            0x00000008

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_AFULL_BO                                             4
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_AFULL_BM                                             0x00000010

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_FULL_BO                                              5
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_FULL_BM                                              0x00000020

#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_DEPTH_BO                                             6
#define AG_MG_REGS_MESSAGE_IN_FIFO_STAT_FIFO_DEPTH_BM                                             0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MESSAGE_IN_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_message_in_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_RESERVED1_U
{
    struct
    {
        ag_mg_regs_register pcie_reserved1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_reserved1_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_RO                                                       0x000010F0
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_RM                                                       0x000003FF

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_UNDERFLOW_BO                                        0
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_UNDERFLOW_BM                                        0x00000001

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_OVERFLOW_BO                                         1
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_OVERFLOW_BM                                         0x00000002

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_EMPTY_BO                                            2
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_EMPTY_BM                                            0x00000004

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_AEMPTY_BO                                           3
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_AEMPTY_BM                                           0x00000008

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_AFULL_BO                                            4
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_AFULL_BM                                            0x00000010

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_FULL_BO                                             5
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_FULL_BM                                             0x00000020

#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_DEPTH_BO                                            6
#define AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_FIFO_DEPTH_BM                                            0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mr_cpl_data_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_RO                                                        0x000010F4
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_RM                                                        0x000003FF

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_UNDERFLOW_BO                                         0
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_UNDERFLOW_BM                                         0x00000001

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_OVERFLOW_BO                                          1
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_OVERFLOW_BM                                          0x00000002

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_EMPTY_BO                                             2
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_EMPTY_BM                                             0x00000004

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_AEMPTY_BO                                            3
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_AEMPTY_BM                                            0x00000008

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_AFULL_BO                                             4
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_AFULL_BM                                             0x00000010

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_FULL_BO                                              5
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_FULL_BM                                              0x00000020

#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_DEPTH_BO                                             6
#define AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_FIFO_DEPTH_BM                                             0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mr_cpl_hdr_fifo_dtat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_RO                                                        0x000010F8
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_UNDERFLOW_BO                                         0
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_UNDERFLOW_BM                                         0x00000001

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_OVERFLOW_BO                                          1
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_OVERFLOW_BM                                          0x00000002

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_EMPTY_BO                                             2
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_EMPTY_BM                                             0x00000004

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_AEMPTY_BO                                            3
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_AEMPTY_BM                                            0x00000008

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_AFULL_BO                                             4
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_AFULL_BM                                             0x00000010

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_FULL_BO                                              5
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_FULL_BM                                              0x00000020

#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_DEPTH_BO                                             6
#define AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_FIFO_DEPTH_BM                                             0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mr_req_hdr_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_MW_DATA_FIFO_STAT_RO                                                        0x000010FC
#define AG_MG_REGS_MW_DATA_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_UNDERFLOW_BO                                            0
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_UNDERFLOW_BM                                            0x00000001

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_OVERFLOW_BO                                             1
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_OVERFLOW_BM                                             0x00000002

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_EMPTY_BO                                                2
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_EMPTY_BM                                                0x00000004

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_AEMPTY_BO                                               3
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_AEMPTY_BM                                               0x00000008

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_AFULL_BO                                                4
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_AFULL_BM                                                0x00000010

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_FULL_BO                                                 5
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_FULL_BM                                                 0x00000020

#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_DEPTH_BO                                                6
#define AG_MG_REGS_MW_DATA_FIFO_STAT_FIFO_DEPTH_BM                                                0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MW_DATA_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mw_data_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_RO                                                        0x00001100
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_UNDERFLOW_BO                                         0
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_UNDERFLOW_BM                                         0x00000001

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_OVERFLOW_BO                                          1
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_OVERFLOW_BM                                          0x00000002

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_EMPTY_BO                                             2
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_EMPTY_BM                                             0x00000004

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_AEMPTY_BO                                            3
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_AEMPTY_BM                                            0x00000008

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_AFULL_BO                                             4
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_AFULL_BM                                             0x00000010

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_FULL_BO                                              5
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_FULL_BM                                              0x00000020

#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_DEPTH_BO                                             6
#define AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_FIFO_DEPTH_BM                                             0x00001FC0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 7,
            fill0 : 19;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_cpl_tmp_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_RO                                              0x00001104
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_RM                                              0x000003FF

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_UNDERFLOW_BO                               0
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_UNDERFLOW_BM                               0x00000001

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_OVERFLOW_BO                                1
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_OVERFLOW_BM                                0x00000002

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_EMPTY_BO                                   2
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_EMPTY_BM                                   0x00000004

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_AEMPTY_BO                                  3
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_AEMPTY_BM                                  0x00000008

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_AFULL_BO                                   4
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_AFULL_BM                                   0x00000010

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_FULL_BO                                    5
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_FULL_BM                                    0x00000020

#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_DEPTH_BO                                   6
#define AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_FIFO_DEPTH_BM                                   0x00001FC0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 7,
            fill0 : 19;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_compl_aligned_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_RO                                                    0x00001108
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_RM                                                    0x000003FF

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_UNDERFLOW_BO                                     0
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_UNDERFLOW_BM                                     0x00000001

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_OVERFLOW_BO                                      1
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_OVERFLOW_BM                                      0x00000002

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_EMPTY_BO                                         2
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_EMPTY_BM                                         0x00000004

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_AEMPTY_BO                                        3
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_AEMPTY_BM                                        0x00000008

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_AFULL_BO                                         4
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_AFULL_BM                                         0x00000010

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_FULL_BO                                          5
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_FULL_BM                                          0x00000020

#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_DEPTH_BO                                         6
#define AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_FIFO_DEPTH_BM                                         0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rx_cpl_err_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SR_HDR_FIFO_STAT_RO                                                        0x0000110C
#define AG_MG_REGS_SR_HDR_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_UNDERFLOW_BO                                             0
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_UNDERFLOW_BM                                             0x00000001

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_OVERFLOW_BO                                              1
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_OVERFLOW_BM                                              0x00000002

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_EMPTY_BO                                                 2
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_EMPTY_BM                                                 0x00000004

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_AEMPTY_BO                                                3
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_AEMPTY_BM                                                0x00000008

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_AFULL_BO                                                 4
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_AFULL_BM                                                 0x00000010

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_FULL_BO                                                  5
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_FULL_BM                                                  0x00000020

#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_DEPTH_BO                                                 6
#define AG_MG_REGS_SR_HDR_FIFO_STAT_FIFO_DEPTH_BM                                                 0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SR_HDR_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sr_hdr_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_RO                                                       0x00001110
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_RM                                                       0x000003FF

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_UNDERFLOW_BO                                        0
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_UNDERFLOW_BM                                        0x00000001

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_OVERFLOW_BO                                         1
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_OVERFLOW_BM                                         0x00000002

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_EMPTY_BO                                            2
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_EMPTY_BM                                            0x00000004

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_AEMPTY_BO                                           3
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_AEMPTY_BM                                           0x00000008

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_AFULL_BO                                            4
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_AFULL_BM                                            0x00000010

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_FULL_BO                                             5
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_FULL_BM                                             0x00000020

#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_DEPTH_BO                                            6
#define AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_FIFO_DEPTH_BM                                            0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sw_conf_hdr_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SW_DATA_FIFO_STAT_RO                                                        0x00001114
#define AG_MG_REGS_SW_DATA_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_UNDERFLOW_BO                                            0
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_UNDERFLOW_BM                                            0x00000001

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_OVERFLOW_BO                                             1
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_OVERFLOW_BM                                             0x00000002

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_EMPTY_BO                                                2
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_EMPTY_BM                                                0x00000004

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_AEMPTY_BO                                               3
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_AEMPTY_BM                                               0x00000008

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_AFULL_BO                                                4
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_AFULL_BM                                                0x00000010

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_FULL_BO                                                 5
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_FULL_BM                                                 0x00000020

#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_DEPTH_BO                                                6
#define AG_MG_REGS_SW_DATA_FIFO_STAT_FIFO_DEPTH_BM                                                0x00003FC0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SW_DATA_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 8,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sw_data_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SW_HDR_FIFO_STAT_RO                                                        0x00001118
#define AG_MG_REGS_SW_HDR_FIFO_STAT_RM                                                        0x000003FF

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_UNDERFLOW_BO                                             0
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_UNDERFLOW_BM                                             0x00000001

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_OVERFLOW_BO                                              1
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_OVERFLOW_BM                                              0x00000002

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_EMPTY_BO                                                 2
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_EMPTY_BM                                                 0x00000004

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_AEMPTY_BO                                                3
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_AEMPTY_BM                                                0x00000008

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_AFULL_BO                                                 4
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_AFULL_BM                                                 0x00000010

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_FULL_BO                                                  5
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_FULL_BM                                                  0x00000020

#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_DEPTH_BO                                                 6
#define AG_MG_REGS_SW_HDR_FIFO_STAT_FIFO_DEPTH_BM                                                 0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SW_HDR_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sw_hdr_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_RO                                              0x0000111C
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_RM                                              0x000003FF

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_UNDERFLOW_BO                               0
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_UNDERFLOW_BM                               0x00000001

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_OVERFLOW_BO                                1
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_OVERFLOW_BM                                0x00000002

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_EMPTY_BO                                   2
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_EMPTY_BM                                   0x00000004

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_AEMPTY_BO                                  3
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_AEMPTY_BM                                  0x00000008

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_AFULL_BO                                   4
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_AFULL_BM                                   0x00000010

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_FULL_BO                                    5
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_FULL_BM                                    0x00000020

#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_DEPTH_BO                                   6
#define AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_FIFO_DEPTH_BM                                   0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_compl_timeout_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_RO                                            0x00001120
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_RM                                            0x000003FF

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_UNDERFLOW_BO                             0
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_UNDERFLOW_BM                             0x00000001

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_OVERFLOW_BO                              1
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_OVERFLOW_BM                              0x00000002

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_EMPTY_BO                                 2
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_EMPTY_BM                                 0x00000004

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_AEMPTY_BO                                3
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_AEMPTY_BM                                0x00000008

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_AFULL_BO                                 4
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_AFULL_BM                                 0x00000010

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_FULL_BO                                  5
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_FULL_BM                                  0x00000020

#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_DEPTH_BO                                 6
#define AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_FIFO_DEPTH_BM                                 0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_axi_id_freelist_fifo_status_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_RO                                       0x00001124
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_RM                                       0x000003FF

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_UNDERFLOW_BO                        0
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_UNDERFLOW_BM                        0x00000001

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_OVERFLOW_BO                         1
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_OVERFLOW_BM                         0x00000002

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_EMPTY_BO                            2
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_EMPTY_BM                            0x00000004

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_AEMPTY_BO                           3
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_AEMPTY_BM                           0x00000008

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_AFULL_BO                            4
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_AFULL_BM                            0x00000010

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_FULL_BO                             5
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_FULL_BM                             0x00000020

#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_DEPTH_BO                            6
#define AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_FIFO_DEPTH_BM                            0x000003C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 4,
            fill0 : 22; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_assigned_axi_id_fifo_status_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_RO                                                      0x00001128
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_0_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_RO                                                      0x0000112C
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_1_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_RO                                                      0x00001130
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_2_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_RO                                                      0x00001134
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_3_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_RO                                                      0x00001138
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_4_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_RO                                                      0x0000113C
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_5_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_RO                                                      0x00001140
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_6_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x0000000C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_RO                                                      0x00001144
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_RM                                                      0x000003FF

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_UNDERFLOW_BO                                       0
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_UNDERFLOW_BM                                       0x00000001

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_OVERFLOW_BO                                        1
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_OVERFLOW_BM                                        0x00000002

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_EMPTY_BO                                           2
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_EMPTY_BM                                           0x00000004

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_AEMPTY_BO                                          3
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_AEMPTY_BM                                          0x00000008

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_AFULL_BO                                           4
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_AFULL_BM                                           0x00000010

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_FULL_BO                                            5
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_FULL_BM                                            0x00000020

#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_DEPTH_BO                                           6
#define AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_FIFO_DEPTH_BM                                           0x000007C0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_U
{
    struct
    {
        ag_mg_regs_register
            fifo_underflow : 1,
            fifo_overflow : 1,
            fifo_empty : 1,
            fifo_aempty : 1,
            fifo_afull : 1,
            fifo_full : 1,
            fifo_depth : 5,
            fill0 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_7_fifo_stat_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_RO                                  0x00001148
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_RM                                  0x0000FFFF

#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_COMPLETER_ID_BO                     0
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_COMPLETER_ID_BM                     0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            completer_id : 16,
            fill0 : 16; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_wrt_conf_wrt_rtn_timeout_id_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_RO                                     0x0000114C
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_RM                                     0x0000003F

#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_COMP_STAT_BO                           0
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_COMP_STAT_BM                           0x00000007

#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_CORE_CPL_STAT_BO                       3
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_CORE_CPL_STAT_BM                       0x00000038

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            comp_stat : 3,
            core_cpl_stat : 3,
            fill0 : 26; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_wrt_conf_wrt_rtn_pkt_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_RO                                0x00001150
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_RM                                0x0000FFFF

#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_ORG_REG_COMPL_ID_BO               0
#define AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_ORG_REG_COMPL_ID_BM               0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            org_reg_compl_id : 16,
            fill0 : 16; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_wrt_conf_wrt_rtn_compl_id_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_RO                                                 0x00001154
#define AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_RM                                                 0x0007FFFF

#define AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_HEADER_COMP_STATUS_BO                              0
#define AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_HEADER_COMP_STATUS_BM                              0x00000007

#define AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_COMPLETER_ID_BO                                    3
#define AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_COMPLETER_ID_BM                                    0x0007FFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            header_comp_status : 3,
            completer_id : 16,
            fill0 : 13;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_compl_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_RO                                         0x00001158
#define AG_MG_REGS_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_RM                                         0x0000FFFF

#define AG_MG_REGS_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_COMPLETER_ID_BO                            0
#define AG_MG_REGS_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_COMPLETER_ID_BM                            0x0000FFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_U
{
    struct
    {
        ag_mg_regs_register
            completer_id : 16,
            fill0 : 16; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_slv_rd_cpl_timeout_completer_id_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_RO                                            0x0000115C
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_RM                                            0x000000FF

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_AHB_SLAVE_FSM_ERR_BO                          0
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_AHB_SLAVE_FSM_ERR_BM                          0x00000001

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_AHB_MASTER_FSM_ERR_BO                         1
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_AHB_MASTER_FSM_ERR_BM                         0x00000002

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_REQ_HDLR_FSM_ERR_BO                           2
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_REQ_HDLR_FSM_ERR_BM                           0x00000004

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_PWR_MGMT_FSM_ERR_BO                           3
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_PWR_MGMT_FSM_ERR_BM                           0x00000008

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_INGRESS_FC_UPDATE_DLLP_FSM_ERR_BO             4
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_INGRESS_FC_UPDATE_DLLP_FSM_ERR_BM             0x00000010

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_TRANSMITTER_FSM_ERR_BO                        5
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_TRANSMITTER_FSM_ERR_BM                        0x00000020

#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_LINK_INTR_STATUS_ERR_BO                       8
#define AG_MG_REGS_TL_FSM_ERR_STATUS_REG_LINK_INTR_STATUS_ERR_BM                       0x00000100

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TL_FSM_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            ahb_slave_fsm_err : 1,
            ahb_master_fsm_err : 1,
            req_hdlr_fsm_err : 1,
            pwr_mgmt_fsm_err : 1,
            ingress_fc_update_dllp_fsm_err : 1,
            transmitter_fsm_err : 1,
            fill0 : 2,
            link_intr_status_err : 1,
            fill1 : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tl_fsm_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_RO                                                      0x00001160
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_RM                                                      0x0000FFFF

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BO                           0
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BM                           0x00000001

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BO                               1
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BM                               0x00000002

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BO                                  2
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BM                                  0x00000004

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_FATAL_ERR_DETECT_BO                                     3
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_FATAL_ERR_DETECT_BM                                     0x00000008

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_POISONED_TLP_BO                                         4
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_POISONED_TLP_BM                                         0x00000010

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_COMPLETER_ABORT_BO                                      5
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_COMPLETER_ABORT_BM                                      0x00000020

#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_UNSUPPORTED_REQUEST_BO                                  6
#define AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_UNSUPPORTED_REQUEST_BM                                  0x00000040

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            advise_nonfatal_err_detect : 1,
            correctable_err_detect : 1,
            nonfatal_err_detect : 1,
            fatal_err_detect : 1,
            poisoned_tlp : 1,
            completer_abort : 1,
            unsupported_request : 1,
            fill0 : 25;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_egr_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_T2A_REQ_PROC_ERR_STATUS_REG_RO                                                 0x00001164
#define AG_MG_REGS_T2A_REQ_PROC_ERR_STATUS_REG_RM                                                 0x0000007F

#define AG_MG_REGS_T2A_REQ_PROC_ERR_STATUS_REG_T2A_REQ_PROC_ERRS_BO                               0
#define AG_MG_REGS_T2A_REQ_PROC_ERR_STATUS_REG_T2A_REQ_PROC_ERRS_BM                               0x0000007F

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_REQ_PROC_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            t2a_req_proc_errs : 7,
            fill0 : 25;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_req_proc_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_RO                                                   0x00001168
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_RM                                                   0x0000001F

#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BO                        0
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BM                        0x00000001

#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BO                            1
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BM                            0x00000002

#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BO                               2
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BM                               0x00000004

#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_FATAL_ERR_DETECT_BO                                  3
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_FATAL_ERR_DETECT_BM                                  0x00000008

#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_COMPLETION_TIMEOUT_BO                                4
#define AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_COMPLETION_TIMEOUT_BM                                0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            advise_nonfatal_err_detect : 1,
            correctable_err_detect : 1,
            nonfatal_err_detect : 1,
            fatal_err_detect : 1,
            completion_timeout : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_cpl_to_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_RO                                                      0x0000116C
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_RM                                                      0x000001FF

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_T2A_IGR_ERRS_BO                                         0
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_T2A_IGR_ERRS_BM                                         0x000001FF

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BO                           0
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BM                           0x00000001

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BO                               1
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BM                               0x00000002

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BO                                  2
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BM                                  0x00000004

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_FATAL_ERR_DETECT_BO                                     3
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_FATAL_ERR_DETECT_BM                                     0x00000008

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_POISONED_TLP_BO                                         4
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_POISONED_TLP_BM                                         0x00000010

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_UNEXPECTED_COMPLETION_BO                                5
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_UNEXPECTED_COMPLETION_BM                                0x00000020

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_MASTER_ABORT_RECEIVED_BO                                6
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_MASTER_ABORT_RECEIVED_BM                                0x00000040

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_TARGET_ABORT_RECEIVED_BO                                7
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_TARGET_ABORT_RECEIVED_BM                                0x00000080

#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_MASTER_DATA_PARITY_ERR_BO                               8
#define AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_MASTER_DATA_PARITY_ERR_BM                               0x00000100

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            advise_nonfatal_err_detect : 1,
            correctable_err_detect : 1,
            nonfatal_err_detect : 1,
            fatal_err_detect : 1,
            poisoned_tlp : 1,
            unexpected_completion : 1,
            master_abort_received : 1,
            target_abort_received : 1,
            master_data_parity_err : 1,
            fill0 : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_igr_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RO                                       0x00001170
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RM                                       0x00000FFF

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RECEIVE_ERR_BO                           0
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RECEIVE_ERR_BM                           0x00000001

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_BAD_TLP_STATUS_BO                        1
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_BAD_TLP_STATUS_BM                        0x00000002

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_BAD_DLLP_STATUS_BO                       2
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_BAD_DLLP_STATUS_BM                       0x00000004

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RELAY_NUM_ROLLOVER_BO                    3
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RELAY_NUM_ROLLOVER_BM                    0x00000008

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_REPLAY_TIMER_TIMEOUT_BO                  4
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_REPLAY_TIMER_TIMEOUT_BM                  0x00000010

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_DATALINK_PROTOCOL_ERR_BO                 5
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_DATALINK_PROTOCOL_ERR_BM                 0x00000020

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_FLOW_CTRL_PROTOCOL_ERR_BO                6
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_FLOW_CTRL_PROTOCOL_ERR_BM                0x00000040

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_UNEXPECTED_COMPLETION_ERR_BO             7
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_UNEXPECTED_COMPLETION_ERR_BM             0x00000080

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RECEIVER_OVERFLOW_ERR_BO                 8
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RECEIVER_OVERFLOW_ERR_BM                 0x00000100

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_MALFORMED_TLP_ERR_BO                     9
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_MALFORMED_TLP_ERR_BM                     0x00000200

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_ECRC_ERR_BO                              10
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_ECRC_ERR_BM                              0x00000400

#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_UNSUPPORTED_REQUEST_ERR_BO               11 
#define AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_UNSUPPORTED_REQUEST_ERR_BM               0x00000800

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            receiver_error_status : 1,
            bad_tlp : 1,
            bad_dllp : 1,
            replay_number_rollover : 1,
            replay_timer_timeout : 1,
            data_link_protocol : 1,
            flow_control_protocol : 1,
            unexpected_completion : 1,
            receiver_overflow : 1,
            malformed_tlp : 1,
            ecrc : 1,
            unsupported_request : 1,
            fill0 : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_fn_indp_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_RO                                            0x00001174
#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_RM                                            0x0000000F

#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_FATAL_ERR_DETECT_BO                           0
#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_FATAL_ERR_DETECT_BM                           0x00000001

#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BO                        1
#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_NONFATAL_ERR_DETECT_BM                        0x00000002

#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BO                     2
#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_CORRECTABLE_ERR_DETECT_BM                     0x00000004

#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BO                 3
#define AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_ADVISE_NONFATAL_ERR_DETECT_BM                 0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            fatal_err_detect : 1,
            nonfatal_err_detect : 1,
            correctable_err_detect : 1,
            advise_nonfatal_err_detect : 1,
            fill0 : 28; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_fn_indp_other_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_RO                                                   0x00001178
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_RM                                                   0x0000003F

#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_OUTREQ_BUFFER_PORTB_PARITY_BO                          0
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_OUTREQ_BUFFER_PORTB_PARITY_BM                          0x00000001

#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_OUTREQ_BUFFER_PORTA_PARITY_BO                          1
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_OUTREQ_BUFFER_PORTA_PARITY_BM                          0x00000002

#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_NONPOSTED_BUFFER_OUT_OF_BAND_PARITY_BO               2
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_NONPOSTED_BUFFER_OUT_OF_BAND_PARITY_BM               0x00000004

#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_READ_COMPLETION_BUFFER_OUT_OF_BAND_PARITY_BO         3
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_READ_COMPLETION_BUFFER_OUT_OF_BAND_PARITY_BM         0x00000008

#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_POSTED_BUFFER_OUT_OF_BAND_PARITY_BO                  4
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_POSTED_BUFFER_OUT_OF_BAND_PARITY_BM                  0x00000010

#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_INGRESS_OUT_OF_BAND_PARITY_BO                        5
#define AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_INGRESS_OUT_OF_BAND_PARITY_BM                        0x00000020

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            outrequest_buffer_portb_parity : 1,
            outrequest_buffer_porta_parity : 1,
            nonposted_buffer_out_of_band_parity : 1,
            completion_buffer_out_of_band_parity : 1,
            posted_buffer_out_of_band_parity : 1,
            ingress_out_of_band_parity : 1,
            fill0 : 26;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_t2a_parity_err_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_CONFIG_LINK_STATUS_REG_RO                                                      0x0000117C
#define AG_MG_REGS_CONFIG_LINK_STATUS_REG_RM                                                      0x0001FFFF

#define AG_MG_REGS_CONFIG_LINK_STATUS_REG_LINK_STATUS_BO                                          0
#define AG_MG_REGS_CONFIG_LINK_STATUS_REG_LINK_STATUS_BM                                          0x0000FFFF

#define AG_MG_REGS_CONFIG_LINK_STATUS_REG_LINK_SPD_CHG_INIT_BO                                    16
#define AG_MG_REGS_CONFIG_LINK_STATUS_REG_LINK_SPD_CHG_INIT_BM                                    0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CONFIG_LINK_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            link_status : 16,
            link_spd_chg_init : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_config_link_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_RO                                    0x00001180
#define AG_MG_REGS_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_RM                                    0x00FFFFFF

#define AG_MG_REGS_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_CAPTURED_MW_ERR_RESP_BAR_STATUS_BO    0
#define AG_MG_REGS_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_CAPTURED_MW_ERR_RESP_BAR_STATUS_BM    0x00FFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_U
{
    struct
    {
        ag_mg_regs_register
            captured_mw_err_resp_bar_status : 24,
            fill0 : 8; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_captured_mw_err_resp_bar_addr_status_reg_u;
#endif


/* 
 * Initialization value: 0x00000004  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_RESERVED2_U
{
    struct
    {
        ag_mg_regs_register pcie_reserved2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_reserved2_u;
#endif


/* 
 * Initialization value: 0x00000004  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_RESERVED3_U
{
    struct
    {
        ag_mg_regs_register pcie_reserved3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_reserved3_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_L2T_CE_STATUS_RO                                                        0x0000118C
#define AG_MG_REGS_L2T_CE_STATUS_RM                                                        0x7FFFFFFF

#define AG_MG_REGS_L2T_CE_STATUS_RECEIVER_ERROR_STATUS_BO                                         0
#define AG_MG_REGS_L2T_CE_STATUS_RECEIVER_ERROR_STATUS_BM                                         0x00000001

#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG_1_BO                                           1
#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG_1_BM                                           0x00000002

#define AG_MG_REGS_L2T_CE_STATUS_BAD_TLP_BO                                                       2
#define AG_MG_REGS_L2T_CE_STATUS_BAD_TLP_BM                                                       0x00000004

#define AG_MG_REGS_L2T_CE_STATUS_BAD_DLLP_BO                                                      3
#define AG_MG_REGS_L2T_CE_STATUS_BAD_DLLP_BM                                                      0x00000008

#define AG_MG_REGS_L2T_CE_STATUS_REPLAY_NUMBER_ROLLOVER_BO                                        4
#define AG_MG_REGS_L2T_CE_STATUS_REPLAY_NUMBER_ROLLOVER_BM                                        0x00000010

#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG_5_BO                                           5
#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG_5_BM                                           0x00000020

#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG_6_BO                                           6
#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG_6_BM                                           0x00000040

#define AG_MG_REGS_L2T_CE_STATUS_REPLY_TIMER_TIMEOUT_BO                                           7
#define AG_MG_REGS_L2T_CE_STATUS_REPLY_TIMER_TIMEOUT_BM                                           0x00000080

#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG31_9_BO                                         8
#define AG_MG_REGS_L2T_CE_STATUS_L2T_CE_STATUS_REG31_9_BM                                         0x7FFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_L2T_CE_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            receiver_error_status : 1,
            l2t_ce_status_reg_1 : 1,
            bad_tlp : 1,
            bad_dllp : 1,
            replay_number_rollover : 1,
            l2t_ce_status_reg_5 : 1,
            l2t_ce_status_reg_6 : 1,
            reply_timer_timeout : 1,
            l2t_ce_status_reg31_9 : 23,
            fill0 : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_l2t_ce_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_AXI_ADDR_FOR_MSI_NOTIFICATION_RO                                               0x00001190
#define AG_MG_REGS_AXI_ADDR_FOR_MSI_NOTIFICATION_RM                                               0x0FFFFFFF

#define AG_MG_REGS_AXI_ADDR_FOR_MSI_NOTIFICATION_AXI_ADDR_31_10_BO                                0
#define AG_MG_REGS_AXI_ADDR_FOR_MSI_NOTIFICATION_AXI_ADDR_31_10_BM                                0x003FFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_AXI_ADDR_FOR_MSI_NOTIFICATION_U
{
    struct
    {
        ag_mg_regs_register
            axi_addr_31_10 : 22,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_axi_addr_for_msi_notification_u;
#endif


/* 
 * Initialization value: 0x76543210  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_RO                                                     0x00001194
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_RM                                                     0xFFFFFFFF

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_PCIE_VECTORS_BO                                         0
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_PCIE_VECTORS_BM                                         0x0000000F

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU0_VECTOR_BO                                         4
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU0_VECTOR_BM                                         0x000000F0

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU1_VECTOR_BO                                         8
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU1_VECTOR_BM                                         0x00000F00

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU2_VECTOR_BO                                         12
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU2_VECTOR_BM                                         0x0000F000

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU3_VECTOR_BO                                         16
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU3_VECTOR_BM                                         0x000F0000

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU4_VECTOR_BO                                         20
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU4_VECTOR_BM                                         0x00F00000

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU5_VECTOR_BO                                         24
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU5_VECTOR_BM                                         0x0F000000

#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU6_VECTOR_BO                                         28
#define AG_MG_REGS_MGW_INT_MSI_VECTORS_CPU6_VECTOR_BM                                         0xF0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MGW_INT_MSI_VECTORS_U
{
    struct
    {
        ag_mg_regs_register
            pcie_vectors : 4,
            cpu0_vector : 4,
            cpu1_vector : 4,
            cpu2_vector : 4,
            cpu3_vector : 4,
            cpu4_vector : 4,
            cpu5_vector : 4,
            cpu6_vector : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mgw_int_msi_vectors_u;
#endif


/* 
 * Initialization value: 0x76543212  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Clear on Write to 1
 */
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_RO                                               0x00001198
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_RM                                               0x0000007F

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_0_INTERRUPT_BO                               0
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_0_INTERRUPT_BM                               0x00000001

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_1_INTERRUPT_BO                               1
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_1_INTERRUPT_BM                               0x00000002

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_2_INTERRUPT_BO                               2
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_2_INTERRUPT_BM                               0x00000004

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_3_INTERRUPT_BO                               3
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_3_INTERRUPT_BM                               0x00000008

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_4_INTERRUPT_BO                               4
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_4_INTERRUPT_BM                               0x00000010

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_5_INTERRUPT_BO                               5
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_5_INTERRUPT_BM                               0x00000020

#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_6_INTERRUPT_BO                               6
#define AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_CPU_6_INTERRUPT_BM                               0x00000040

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_U
{
    struct
    {
        ag_mg_regs_register
            cpu_0_interrupt : 1,
            cpu_1_interrupt : 1,
            cpu_2_interrupt : 1,
            cpu_3_interrupt : 1,
            cpu_4_interrupt : 1,
            cpu_5_interrupt : 1,
            cpu_6_interrupt : 1,
            fill0 : 25;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mgw_interrupt_status_register_u;
#endif


/* 
 * Initialization value: 0x76543212  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_RO                                                   0x0000119C
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_RM                                                   0x0000007F

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_0_INTERRUPT_ENABLE_BO                            0
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_0_INTERRUPT_ENABLE_BM                            0x00000001

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_1_INTERRUPT_ENABLE_BO                            1
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_1_INTERRUPT_ENABLE_BM                            0x00000002

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_2_INTERRUPT_ENABLE_BO                            2
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_2_INTERRUPT_ENABLE_BM                            0x00000004

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_3_INTERRUPT_ENABLE_BO                            3
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_3_INTERRUPT_ENABLE_BM                            0x00000008

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_4_INTERRUPT_ENABLE_BO                            4
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_4_INTERRUPT_ENABLE_BM                            0x00000010

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_5_INTERRUPT_ENABLE_BO                            5
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_5_INTERRUPT_ENABLE_BM                            0x00000020

#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_6_INTERRUPT_ENABLE_BO                            6
#define AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_CPU_6_INTERRUPT_ENABLE_BM                            0x00000040

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MGW_INTERRUPT_ENABLE_REGISTER_U
{
    struct
    {
        ag_mg_regs_register
            cpu_0_interrupt_enable : 1,
            cpu_1_interrupt_enable : 1,
            cpu_2_interrupt_enable : 1,
            cpu_3_interrupt_enable : 1,
            cpu_4_interrupt_enable : 1,
            cpu_5_interrupt_enable : 1,
            cpu_6_interrupt_enable : 1,
            fill0 : 25;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mgw_interrupt_enable_register_u;
#endif


/* 
 * Initialization value: 0x76543212  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_RO                                                    0x000011A0
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_RM                                                    0x0000007F

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_0_INTERRUPT_FORCE_BO                              0
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_0_INTERRUPT_FORCE_BM                              0x00000001

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_1_INTERRUPT_FORCE_BO                              1
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_1_INTERRUPT_FORCE_BM                              0x00000002

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_2_INTERRUPT_FORCE_BO                              2
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_2_INTERRUPT_FORCE_BM                              0x00000004

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_3_INTERRUPT_FORCE_BO                              3
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_3_INTERRUPT_FORCE_BM                              0x00000008

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_4_INTERRUPT_FORCE_BO                              4
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_4_INTERRUPT_FORCE_BM                              0x00000010

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_5_INTERRUPT_FORCE_BO                              5
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_5_INTERRUPT_FORCE_BM                              0x00000020

#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_6_INTERRUPT_FORCE_BO                              6
#define AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_CPU_6_INTERRUPT_FORCE_BM                              0x00000040

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MGW_INTERRUPT_FORCE_REGISTER_U
{
    struct
    {
        ag_mg_regs_register
            cpu_0_interrupt_force : 1,
            cpu_1_interrupt_force : 1,
            cpu_2_interrupt_force : 1,
            cpu_3_interrupt_force : 1,
            cpu_4_interrupt_force : 1,
            cpu_5_interrupt_force : 1,
            cpu_6_interrupt_force : 1,
            fill0 : 25;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_mgw_interrupt_force_register_u;
#endif

/* Transaction Layer (TL) section */

/* 
 * Initialization value: 0xED1111C1  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEVICE_AND_VENDOR_ID_RO                                                      0x00002000
#define AG_MG_REGS_DEVICE_AND_VENDOR_ID_RM                                                      0xFFFFFFFF

#define AG_MG_REGS_DEVICE_AND_VENDOR_ID_VENDOR_ID_BO                                            0
#define AG_MG_REGS_DEVICE_AND_VENDOR_ID_VENDOR_ID_BM                                            0x0000FFFF

#define AG_MG_REGS_DEVICE_AND_VENDOR_ID_DEVICE_ID_BO                                            16
#define AG_MG_REGS_DEVICE_AND_VENDOR_ID_DEVICE_ID_BM                                            0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEVICE_AND_VENDOR_ID_U
{
    struct
    {
        ag_mg_regs_register
            vendor_id : 16,
            device_id : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_device_and_vendor_id_u;
#endif


/* 
 * Initialization value: 0x00100000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_STATUS_AND_COMMAND_RO                                                        0x00002004
#define AG_MG_REGS_STATUS_AND_COMMAND_RM                                                        0xFFFFFFFE

#define AG_MG_REGS_STATUS_AND_COMMAND_MEMORY_SPACE_ENABLE_BO                                    1
#define AG_MG_REGS_STATUS_AND_COMMAND_MEMORY_SPACE_ENABLE_BM                                    0x00000002

#define AG_MG_REGS_STATUS_AND_COMMAND_BUS_MASTER_ENABLE_BO                                      2
#define AG_MG_REGS_STATUS_AND_COMMAND_BUS_MASTER_ENABLE_BM                                      0x00000004

#define AG_MG_REGS_STATUS_AND_COMMAND_SPECIAL_CYCLE_ENABLE_BO                                   3
#define AG_MG_REGS_STATUS_AND_COMMAND_SPECIAL_CYCLE_ENABLE_BM                                   0x00000008

#define AG_MG_REGS_STATUS_AND_COMMAND_MEMORY_WRITE_AND_INVALIDATE_ENABLE_BO                     4
#define AG_MG_REGS_STATUS_AND_COMMAND_MEMORY_WRITE_AND_INVALIDATE_ENABLE_BM                     0x00000010

#define AG_MG_REGS_STATUS_AND_COMMAND_PALETTE_SNOOP_ENABLE_BO                                   5
#define AG_MG_REGS_STATUS_AND_COMMAND_PALETTE_SNOOP_ENABLE_BM                                   0x00000020

#define AG_MG_REGS_STATUS_AND_COMMAND_PARITY_ERROR_RESPONSE_BO                                  6
#define AG_MG_REGS_STATUS_AND_COMMAND_PARITY_ERROR_RESPONSE_BM                                  0x00000040

#define AG_MG_REGS_STATUS_AND_COMMAND_WAIT_CYCLE_CONTROL_BO                                     7
#define AG_MG_REGS_STATUS_AND_COMMAND_WAIT_CYCLE_CONTROL_BM                                     0x00000080

#define AG_MG_REGS_STATUS_AND_COMMAND_SERR_ENABLE_BO                                            8
#define AG_MG_REGS_STATUS_AND_COMMAND_SERR_ENABLE_BM                                            0x00000100

#define AG_MG_REGS_STATUS_AND_COMMAND_FAST_BACK_TO_BACK_ENABLE_BO                               9
#define AG_MG_REGS_STATUS_AND_COMMAND_FAST_BACK_TO_BACK_ENABLE_BM                               0x00000200

#define AG_MG_REGS_STATUS_AND_COMMAND_INTERRUPT_DISABLE_BO                                      10
#define AG_MG_REGS_STATUS_AND_COMMAND_INTERRUPT_DISABLE_BM                                      0x00000400

#define AG_MG_REGS_STATUS_AND_COMMAND_RSVDZ_COMMAND_BO                                          11
#define AG_MG_REGS_STATUS_AND_COMMAND_RSVDZ_COMMAND_BM                                          0x0000F800

#define AG_MG_REGS_STATUS_AND_COMMAND_RSVDZ_STATUS_BO                                           16
#define AG_MG_REGS_STATUS_AND_COMMAND_RSVDZ_STATUS_BM                                           0x00070000

#define AG_MG_REGS_STATUS_AND_COMMAND_INTERRUPT_STATUS_BO                                       19
#define AG_MG_REGS_STATUS_AND_COMMAND_INTERRUPT_STATUS_BM                                       0x00080000

#define AG_MG_REGS_STATUS_AND_COMMAND_CAPABILITIES_LIST_BO                                      20
#define AG_MG_REGS_STATUS_AND_COMMAND_CAPABILITIES_LIST_BM                                      0x00100000

#define AG_MG_REGS_STATUS_AND_COMMAND_CAPABLE_66_MHZ_BO                                         21
#define AG_MG_REGS_STATUS_AND_COMMAND_CAPABLE_66_MHZ_BM                                         0x00200000

#define AG_MG_REGS_STATUS_AND_COMMAND_UDF_SUPPORTED_BO                                          22
#define AG_MG_REGS_STATUS_AND_COMMAND_UDF_SUPPORTED_BM                                          0x00400000

#define AG_MG_REGS_STATUS_AND_COMMAND_FAST_BACK_TO_BACK_CAPABLE_BO                              23
#define AG_MG_REGS_STATUS_AND_COMMAND_FAST_BACK_TO_BACK_CAPABLE_BM                              0x00800000

#define AG_MG_REGS_STATUS_AND_COMMAND_MASTER_DATA_PARITY_ERROR_BO                               24
#define AG_MG_REGS_STATUS_AND_COMMAND_MASTER_DATA_PARITY_ERROR_BM                               0x01000000

#define AG_MG_REGS_STATUS_AND_COMMAND_DEVSEL_TIMING_BO                                          25
#define AG_MG_REGS_STATUS_AND_COMMAND_DEVSEL_TIMING_BM                                          0x06000000

#define AG_MG_REGS_STATUS_AND_COMMAND_SIGNALED_TARGET_ABORT_BO                                  27
#define AG_MG_REGS_STATUS_AND_COMMAND_SIGNALED_TARGET_ABORT_BM                                  0x08000000

#define AG_MG_REGS_STATUS_AND_COMMAND_RECEIVED_TARGET_ABORT_BO                                  28
#define AG_MG_REGS_STATUS_AND_COMMAND_RECEIVED_TARGET_ABORT_BM                                  0x10000000

#define AG_MG_REGS_STATUS_AND_COMMAND_RECEIVED_MASTER_ABORT_BO                                  29
#define AG_MG_REGS_STATUS_AND_COMMAND_RECEIVED_MASTER_ABORT_BM                                  0x20000000

#define AG_MG_REGS_STATUS_AND_COMMAND_SIGNALED_SYSTEM_ERROR_BO                                  30
#define AG_MG_REGS_STATUS_AND_COMMAND_SIGNALED_SYSTEM_ERROR_BM                                  0x40000000

#define AG_MG_REGS_STATUS_AND_COMMAND_DETECT_PARITY_ERROR_BO                                    31
#define AG_MG_REGS_STATUS_AND_COMMAND_DETECT_PARITY_ERROR_BM                                    0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_STATUS_AND_COMMAND_U
{
    struct
    {
        ag_mg_regs_register
            fill0 : 1,
            memory_space_enable : 1,
            bus_master_enable : 1,
            special_cycle_enable : 1,
            memory_write_and_invalidate_enable : 1,
            palette_snoop_enable : 1,
            parity_error_response : 1,
            wait_cycle_control : 1,
            serr_enable : 1,
            fast_back_to_back_enable : 1,
            interrupt_disable : 1,
            rsvdz_command : 5,
            rsvdz_status : 3,
            interrupt_status : 1,
            capabilities_list : 1,
            capable_66_mhz : 1,
            udf_supported : 1,
            fast_back_to_back_capable : 1,
            master_data_parity_error : 1,
            devsel_timing : 2,
            signaled_target_abort : 1,
            received_target_abort : 1,
            received_master_abort : 1,
            signaled_system_error : 1,
            detect_parity_error : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_status_and_command_u;
#endif


/* 
 * Initialization value: 0x02800000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_CLASS_AND_REVISION_RO                                                        0x00002008
#define AG_MG_REGS_CLASS_AND_REVISION_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_CLASS_AND_REVISION_REVISION_ID_BO                                            0
#define AG_MG_REGS_CLASS_AND_REVISION_REVISION_ID_BM                                            0x000000FF

#define AG_MG_REGS_CLASS_AND_REVISION_CLASS_CODE_BO                                             8
#define AG_MG_REGS_CLASS_AND_REVISION_CLASS_CODE_BM                                             0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CLASS_AND_REVISION_U
{
    struct
    {
        ag_mg_regs_register
            revision_id : 8,
            class_code : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_class_and_revision_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_RO                                               0x0000200C
#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_RM                                               0xFFFFFFFF

#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_CACHE_LINE_SIZE_BO                               0
#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_CACHE_LINE_SIZE_BM                               0x000000FF

#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_MASTER_LATENCY_TIMER_BO                          8
#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_MASTER_LATENCY_TIMER_BM                          0x0000FF00

#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_TYPES_BO                                         16
#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_TYPES_BM                                         0x00FF0000

#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_BIST_BO                                          24
#define AG_MG_REGS_TEST_CONFIGURATION_REGISTER_BIST_BM                                          0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEST_CONFIGURATION_REGISTER_U
{
    struct
    {
        ag_mg_regs_register
            cache_line_size : 8,
            master_latency_timer : 8,
            types : 8,
            bist : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_test_configuration_register_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BAR0_LO_RO                                                    0x00002010
#define AG_MG_REGS_BAR0_LO_RM                                                    0xFFFFFFFF

#define AG_MG_REGS_BAR0_LO_MEMORY_SPACE_INDICATOR_BAR0_BO                         0
#define AG_MG_REGS_BAR0_LO_MEMORY_SPACE_INDICATOR_BAR0_BM                         0x00000001

#define AG_MG_REGS_BAR0_LO_TYPE_BAR0_BO                                           1
#define AG_MG_REGS_BAR0_LO_TYPE_BAR0_BM                                           0x00000006

#define AG_MG_REGS_BAR0_LO_PREFETCH_BO                                            3
#define AG_MG_REGS_BAR0_LO_PREFETCH_BM                                            0x00000008

#define AG_MG_REGS_BAR0_LO_ADDRESS_BITS_31TO4_BO                                  4
#define AG_MG_REGS_BAR0_LO_ADDRESS_BITS_31TO4_BM                                  0xFFFFFFF0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BAR0_LO_U
{
    struct
    {
        ag_mg_regs_register
            memory_space_indicator_bar0 : 1,
            type_bar0 : 2,
            prefetch : 1,
            address_bits_31to4 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bar0_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BAR0_HI_RO                                            0x00002014
#define AG_MG_REGS_BAR0_HI_RM                                            0xFFFFFFFF

#define AG_MG_REGS_BAR0_HI_ADDRESS_BITS_63TO32_BO                        0
#define AG_MG_REGS_BAR0_HI_ADDRESS_BITS_63TO32_BM                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BAR0_HI_U
{
    struct
    {
        ag_mg_regs_register
            address_bits_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bar0_hi_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BAR1_LO_RO                                          0x00002018
#define AG_MG_REGS_BAR1_LO_RM                                          0xFFFFFFFF

#define AG_MG_REGS_BAR1_LO_MEMORY_SPACE_IO_INDICATOR_BAR1_BO           0
#define AG_MG_REGS_BAR1_LO_MEMORY_SPACE_IO_INDICATOR_BAR1_BM           0x00000001

#define AG_MG_REGS_BAR1_LO_TYPE_BAR1_BO                                1
#define AG_MG_REGS_BAR1_LO_TYPE_BAR1_BM                                0x00000006

#define AG_MG_REGS_BAR1_LO_PREFETCH_BO                                 3
#define AG_MG_REGS_BAR1_LO_PREFETCH_BM                                 0x00000008

#define AG_MG_REGS_BAR1_LO_ADDRESS_BITS_31TO4_BO                       4
#define AG_MG_REGS_BAR1_LO_ADDRESS_BITS_31TO4_BM                       0xFFFFFFF0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BAR1_LO_U
{
    struct
    {
        ag_mg_regs_register
            memory_space_io_indicator_bar1 : 1,
            type_bar1 : 2,
            prefetch : 1,
            address_bits_31to4 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bar1_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BAR1_HI_RO                          		          0x0000201C
#define AG_MG_REGS_BAR1_HI_RM                                         0xFFFFFFFF

#define AG_MG_REGS_BAR1_HI_ADDRESS_BITS_63TO32_BO                     0
#define AG_MG_REGS_BAR1_HI_ADDRESS_BITS_63TO32_BM                     0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BAR1_HI_U
{
    struct
    {
        ag_mg_regs_register
            address_bits_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bar1_hi_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BAR2_LO_RO                                     0x00002020
#define AG_MG_REGS_BAR2_LO_RM                                     0xFFFFFFFF

#define AG_MG_REGS_BAR2_LO_MEMORY_SPACE_INDICATOR_BAR2_BO         0
#define AG_MG_REGS_BAR2_LO_MEMORY_SPACE_INDICATOR_BAR2_BM         0x00000001

#define AG_MG_REGS_BAR2_LO_TYPE_BAR2_BO                           1
#define AG_MG_REGS_BAR2_LO_TYPE_BAR2_BM                           0x00000006

#define AG_MG_REGS_BAR2_LO_PREFETCH_BO                            3
#define AG_MG_REGS_BAR2_LO_PREFETCH_BM                            0x00000008

#define AG_MG_REGS_BAR2_LO_ADDRESS_BITS_31TO4_BO                  4
#define AG_MG_REGS_BAR2_LO_ADDRESS_BITS_31TO4_BM                  0xFFFFFFF0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BAR2_LO_U
{
    struct
    {
        ag_mg_regs_register
            memory_space_indicator_bar2 : 1,
            type_bar2 : 2,
            prefetch : 1,
            address_bits_31to4 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bar2_lo_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_BAR2_HI_RO                                    0x00002024
#define AG_MG_REGS_BAR2_HI_RM                                    0xFFFFFFFF

#define AG_MG_REGS_BAR2_HI_ADDRESS_BITS_63TO32_BO                0
#define AG_MG_REGS_BAR2_HI_ADDRESS_BITS_63TO32_BM                0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_BAR2_HI_U
{
    struct
    {
        ag_mg_regs_register
            address_bits_63to32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_bar2_hi_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_CARDBUS_RO                                   0x00002028
#define AG_MG_REGS_CARDBUS_RM                                   0xFFFFFFFF

#define AG_MG_REGS_CARDBUS_CARDBUS_CIS_POINTER_BO               0
#define AG_MG_REGS_CARDBUS_CARDBUS_CIS_POINTER_BM               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CARDBUS_U
{
    struct
    {
        ag_mg_regs_register
            cardbus_cis_pointer;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_cardbus_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SUBSYSTEM_RO                               0x0000202C
#define AG_MG_REGS_SUBSYSTEM_RM                               0xFFFFFFFF

#define AG_MG_REGS_SUBSYSTEM_SUBSYSTEM_ID_BO                  0
#define AG_MG_REGS_SUBSYSTEM_SUBSYSTEM_ID_BM                  0x0000FFFF

#define AG_MG_REGS_SUBSYSTEM_SUBSYSTEM_VENDOR_ID_BO           16
#define AG_MG_REGS_SUBSYSTEM_SUBSYSTEM_VENDOR_ID_BM           0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SUBSYSTEM_U
{
    struct
    {
        ag_mg_regs_register
            subsystem_id : 16,
            subsystem_vendor_id : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_subsystem_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_EXPANSION_RO                                 0x00002030
#define AG_MG_REGS_EXPANSION_RM                                 0xFFFFFFFF

#define AG_MG_REGS_EXPANSION_EXPANSION_ROM_ENABLE_BO            0
#define AG_MG_REGS_EXPANSION_EXPANSION_ROM_ENABLE_BM            0x00000001

#define AG_MG_REGS_EXPANSION_RVSD_BO                            1
#define AG_MG_REGS_EXPANSION_RVSD_BM                            0x000007FE

#define AG_MG_REGS_EXPANSION_EXPANSION_ROM_BASE_ADDRESS_BM 		0xFFFFF800
#define AG_MG_REGS_EXPANSION_EXPANSION_ROM_BASE_ADDRESS_BO 		11

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_EXPANSION_U
{
    struct
    {
        ag_mg_regs_register
            expansion_rom_enable : 1,
            rvsd : 10,
            expansion_rom_base_address : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_expansion_u;
#endif


/* 
 * Initialization value: 0x00000040  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CAPABILITIES_POINTER_REG_RO                            0x00002034
#define AG_MG_REGS_CAPABILITIES_POINTER_REG_RM                            0x000000FF

#define AG_MG_REGS_CAPABILITIES_POINTER_REG_CAPABILITIES_POINTER_BO       0
#define AG_MG_REGS_CAPABILITIES_POINTER_REG_CAPABILITIES_POINTER_BM       0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CAPABILITIES_POINTER_REG_U
{
    struct
    {
        ag_mg_regs_register
            capabilities_pointer : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_capabilities_pointer_reg_u;
#endif


/* 
 * Initialization value: 0x00000100  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_INTERRUPT_RO                                   0x0000203C
#define AG_MG_REGS_INTERRUPT_RM                                   0xFFFFFFFF

#define AG_MG_REGS_INTERRUPT_INTERRUPT_LINE_BO                    0
#define AG_MG_REGS_INTERRUPT_INTERRUPT_LINE_BM                    0x000000FF

#define AG_MG_REGS_INTERRUPT_INTERRUPT_PIN_BO                     8
#define AG_MG_REGS_INTERRUPT_INTERRUPT_PIN_BM                     0x0000FF00

#define AG_MG_REGS_INTERRUPT_MIN_GNT_BO                           16
#define AG_MG_REGS_INTERRUPT_MIN_GNT_BM                           0x00FF0000

#define AG_MG_REGS_INTERRUPT_MAT_LAT_BO                           24
#define AG_MG_REGS_INTERRUPT_MAT_LAT_BM                           0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_INTERRUPT_U
{
    struct
    {
        ag_mg_regs_register
            interrupt_line : 8,
            interrupt_pin : 8,
            min_gnt : 8,
            mat_lat : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_interrupt_u;
#endif


/* 
 * Initialization value: 0x02035001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_RO                                                 0x00002040
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_RM                                                 0xFFFFFFFF

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PCIE_CAPABILITY_ID_BO                              0
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PCIE_CAPABILITY_ID_BM                              0x000000FF

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_NEXT_CAPABILITY_POINTER_BO                         8
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_NEXT_CAPABILITY_POINTER_BM                         0x0000FF00

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_VERSION_BO                                         16
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_VERSION_BM                                         0x00070000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PME_CLOCK_BO                                       19
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PME_CLOCK_BM                                       0x00080000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PCIE_CAP_RSVDP_BO                                  20
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PCIE_CAP_RSVDP_BM                                  0x00100000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_DEVICE_SPECIFIC_INITIALIZATION_BO                  21
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_DEVICE_SPECIFIC_INITIALIZATION_BM                  0x00200000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_AUX_CURRENT_BO                                     22
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_AUX_CURRENT_BM                                     0x01C00000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_D1_SUPPORT_BO                                      25
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_D1_SUPPORT_BM                                      0x02000000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_D2_SUPPORT_BO                                      26
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_D2_SUPPORT_BM                                      0x04000000

#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PME_SUPPORT_BO                                     27
#define AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_PME_SUPPORT_BM                                     0xF8000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            pcie_capability_id : 8,
            next_capability_pointer : 8,
            version : 3,
            pme_clock : 1,
            pcie_cap_rsvdp : 1,
            device_specific_initialization : 1,
            aux_current : 3,
            d1_support : 1,
            d2_support : 1,
            pme_support : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_pwr_mgn_capabilities_u;
#endif


/* 
 * Initialization value: 0x00000008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_RO                                                  0x00002044
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_RM                                                  0xFF00FF0B

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_POWER_STATE_BO                                      0
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_POWER_STATE_BM                                      0x00000003

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_NO_SOFT_RESET_BO                                    3
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_NO_SOFT_RESET_BM                                    0x00000008

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_PME_ENABLE_BO                                       8
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_PME_ENABLE_BM                                       0x00000100

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_DATA_SELECT_BO                                      9
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_DATA_SELECT_BM                                      0x00001E00

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_DATA_SCALE_BO                                       13
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_DATA_SCALE_BM                                       0x00006000

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_PME_STATUS_BO                                       15
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_PME_STATUS_BM                                       0x00008000

#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_DATA_BO                                             24
#define AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_DATA_BM                                             0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            power_state : 2,
            fill2 : 1,
            no_soft_reset : 1,
            fill1 : 4,
            pme_enable : 1,
            data_select : 4,
            data_scale : 2,
            pme_status : 1,
            fill0 : 8,
            data : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_pwr_mgn_status_ctrl_u;
#endif


/* 
 * Initialization value: 0x00006011  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MESSAGE_CONTROL_RO                                                 0x00002050
#define AG_MG_REGS_MESSAGE_CONTROL_RM                                                 0xC7FFFFFF

#define AG_MG_REGS_MESSAGE_CONTROL_PCIE_CAPABILITY_ID_BO                              0
#define AG_MG_REGS_MESSAGE_CONTROL_PCIE_CAPABILITY_ID_BM                              0x000000FF

#define AG_MG_REGS_MESSAGE_CONTROL_NEXT_CAPABILITY_POINTER_BO                         8
#define AG_MG_REGS_MESSAGE_CONTROL_NEXT_CAPABILITY_POINTER_BM                         0x0000FF00

#define AG_MG_REGS_MESSAGE_CONTROL_TABLE_SIZE_BO                                      16
#define AG_MG_REGS_MESSAGE_CONTROL_TABLE_SIZE_BM                                      0x07FF0000

#define AG_MG_REGS_MESSAGE_CONTROL_FUNCTION_MASK_BO                                   30
#define AG_MG_REGS_MESSAGE_CONTROL_FUNCTION_MASK_BM                                   0x40000000

#define AG_MG_REGS_MESSAGE_CONTROL_MSI_X_ENABLE_BO                                    31
#define AG_MG_REGS_MESSAGE_CONTROL_MSI_X_ENABLE_BM                                    0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MESSAGE_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            pcie_capability_id : 8,
            next_capability_pointer : 8,
            table_size : 11,
            fill0 : 3,
            function_mask : 1,
            msi_x_enable : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_message_control_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TABLE_OFFSET_BIR_RO                                                        0x00002054
#define AG_MG_REGS_TABLE_OFFSET_BIR_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TABLE_OFFSET_BIR_TABLE_BIR_BO                                              0
#define AG_MG_REGS_TABLE_OFFSET_BIR_TABLE_BIR_BM                                              0x00000007

#define AG_MG_REGS_TABLE_OFFSET_BIR_TABLE_OFFSET_BO                                           3
#define AG_MG_REGS_TABLE_OFFSET_BIR_TABLE_OFFSET_BM                                           0xFFFFFFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TABLE_OFFSET_BIR_U
{
    struct
    {
        ag_mg_regs_register
            table_bir : 3,
            table_offset : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_table_offset_bir_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PBA_OFFSET_RO                                                        0x00002058
#define AG_MG_REGS_PBA_OFFSET_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PBA_OFFSET_PBA_BIR_BO                                                0
#define AG_MG_REGS_PBA_OFFSET_PBA_BIR_BM                                                0x00000007

#define AG_MG_REGS_PBA_OFFSET_PBA_OFFSET_BO                                             3
#define AG_MG_REGS_PBA_OFFSET_PBA_OFFSET_BM                                             0xFFFFFFF8

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PBA_OFFSET_U
{
    struct
    {
        ag_mg_regs_register
            pba_bir : 3,
            pba_offset : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pba_offset_u;
#endif


/* 
 * Initialization value: 0x00020010  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_CAPABILITIES_RO                                                        0x00002060
#define AG_MG_REGS_PCIE_CAPABILITIES_RM                                                        0x3EFFFFFF

#define AG_MG_REGS_PCIE_CAPABILITIES_PCIE_EXPRESS_CAPABILITY_ID_BO                              0
#define AG_MG_REGS_PCIE_CAPABILITIES_PCIE_EXPRESS_CAPABILITY_ID_BM                              0x000000FF

#define AG_MG_REGS_PCIE_CAPABILITIES_NEXT_CAPABILITY_POINTER_BO                                 8
#define AG_MG_REGS_PCIE_CAPABILITIES_NEXT_CAPABILITY_POINTER_BM                                 0x0000FF00

#define AG_MG_REGS_PCIE_CAPABILITIES_CAPABILITY_VERSION_BO                                      16
#define AG_MG_REGS_PCIE_CAPABILITIES_CAPABILITY_VERSION_BM                                      0x000F0000

#define AG_MG_REGS_PCIE_CAPABILITIES_DEVICE_PORT_TYPE_BO                                        20
#define AG_MG_REGS_PCIE_CAPABILITIES_DEVICE_PORT_TYPE_BM                                        0x00F00000

#define AG_MG_REGS_PCIE_CAPABILITIES_INTERRUPT_MESSAGE_NUMBER_BO                                25
#define AG_MG_REGS_PCIE_CAPABILITIES_INTERRUPT_MESSAGE_NUMBER_BM                                0x3E000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            pcie_express_capability_id : 8,
            next_capability_pointer : 8,
            capability_version : 4,
            device_port_type : 4,
            fill1 : 1,
            interrupt_message_number : 5,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_capabilities_u;
#endif


/* 
 * Initialization value: 0x000082E0  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEVICE_CAPABILITIES_RO                                                       0x00002064
#define AG_MG_REGS_DEVICE_CAPABILITIES_RM                                                       0x1FFC8FFF

#define AG_MG_REGS_DEVICE_CAPABILITIES_MAX_PAYLOAD_SUPPORTED_BO                                 0
#define AG_MG_REGS_DEVICE_CAPABILITIES_MAX_PAYLOAD_SUPPORTED_BM                                 0x00000007

#define AG_MG_REGS_DEVICE_CAPABILITIES_PHANTOM_FUNC_SUPPORTED_BO                                3
#define AG_MG_REGS_DEVICE_CAPABILITIES_PHANTOM_FUNC_SUPPORTED_BM                                0x00000018

#define AG_MG_REGS_DEVICE_CAPABILITIES_EXTENDED_TAG_FIELD_SUPPORTED_BO                          5
#define AG_MG_REGS_DEVICE_CAPABILITIES_EXTENDED_TAG_FIELD_SUPPORTED_BM                          0x00000020

#define AG_MG_REGS_DEVICE_CAPABILITIES_ENDPOINT_L0S_LATENCY_BO                                  6
#define AG_MG_REGS_DEVICE_CAPABILITIES_ENDPOINT_L0S_LATENCY_BM                                  0x000001C0

#define AG_MG_REGS_DEVICE_CAPABILITIES_ENDPOINT_L1_LATENCY_BO                                   9
#define AG_MG_REGS_DEVICE_CAPABILITIES_ENDPOINT_L1_LATENCY_BM                                   0x00000E00

#define AG_MG_REGS_DEVICE_CAPABILITIES_ROLE_VASED_ERROR_REPORTING_BO                            15
#define AG_MG_REGS_DEVICE_CAPABILITIES_ROLE_VASED_ERROR_REPORTING_BM                            0x00008000

#define AG_MG_REGS_DEVICE_CAPABILITIES_CAPTURED_SLOT_POWER_LIMIT_VALUE_BO                       18
#define AG_MG_REGS_DEVICE_CAPABILITIES_CAPTURED_SLOT_POWER_LIMIT_VALUE_BM                       0x03FC0000

#define AG_MG_REGS_DEVICE_CAPABILITIES_CAPTURED_SLOT_POWER_LIMIT_BO                             26
#define AG_MG_REGS_DEVICE_CAPABILITIES_CAPTURED_SLOT_POWER_LIMIT_BM                             0x0C000000

#define AG_MG_REGS_DEVICE_CAPABILITIES_FUNCTIONAL_LEVEL_RESET_CABILITY_BO                       28
#define AG_MG_REGS_DEVICE_CAPABILITIES_FUNCTIONAL_LEVEL_RESET_CABILITY_BM                       0x10000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEVICE_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            max_payload_supported : 3,
            phantom_func_supported : 2,
            extended_tag_field_supported : 1,
            endpoint_l0s_latency : 3,
            endpoint_l1_latency : 3,
            fill2 : 3,
            role_vased_error_reporting : 1,
            fill1 : 2,
            captured_slot_power_limit_value : 8,
            captured_slot_power_limit : 2,
            functional_level_reset_cability : 1,
            fill0 : 3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_device_capabilities_u;
#endif


/* 
 * Initialization value: 0x00002810  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_RO                                                 0x00002068
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_RM                                                 0x003FFFFF

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_CORRECTABLE_ERROR_REPORTING_ENABLE_BO              0
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_CORRECTABLE_ERROR_REPORTING_ENABLE_BM              0x00000001

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_NON_FATAL_ERROR_REPORTING_ENABLE_BO                1
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_NON_FATAL_ERROR_REPORTING_ENABLE_BM                0x00000002

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_FATAL_ERROR_REPORTING_BO                           2
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_FATAL_ERROR_REPORTING_BM                           0x00000004

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_UNSUPPORTED_REQUEST_REPORTING_ENABLE_BO            3
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_UNSUPPORTED_REQUEST_REPORTING_ENABLE_BM            0x00000008

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_ENABLE_RELAXED_ORDERING_BO                         4
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_ENABLE_RELAXED_ORDERING_BM                         0x00000010

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_MAX_PAYLOAD_SIZE_BO                                5
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_MAX_PAYLOAD_SIZE_BM                                0x000000E0

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_EXTENDED_FIELD_ENABLE_BO                           8
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_EXTENDED_FIELD_ENABLE_BM                           0x00000100

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_PHANTON_FUNC_ENABLE_BO                             9
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_PHANTON_FUNC_ENABLE_BM                             0x00000200

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_AUX_POWER_PM_ENABLE_BO                             10
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_AUX_POWER_PM_ENABLE_BM                             0x00000400

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_ENABLE_NO_SNOOP_BO                                 11
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_ENABLE_NO_SNOOP_BM                                 0x00000800

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_MAX_READ_REQUEST_SIZE_BO                           12
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_MAX_READ_REQUEST_SIZE_BM                           0x00007000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_INITIATE_FUNCTIONAL_LEVEL_RESET_BO                 15
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_INITIATE_FUNCTIONAL_LEVEL_RESET_BM                 0x00008000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_CORRECT_ERROR_DETECTED_BO                          16
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_CORRECT_ERROR_DETECTED_BM                          0x00010000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_NON_FATAL_ERROR_DETECTED_BO                        17
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_NON_FATAL_ERROR_DETECTED_BM                        0x00020000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_FATAL_ERROR_DETECTED_BO                            18
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_FATAL_ERROR_DETECTED_BM                            0x00040000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_UNSUPPORTED_REQUESTED_DETECTED_BO                  19
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_UNSUPPORTED_REQUESTED_DETECTED_BM                  0x00080000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_AUX_POWER_DETECTED_BO                              20
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_AUX_POWER_DETECTED_BM                              0x00100000

#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_TRANSACTIONS_PENDING_BO                            21
#define AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_TRANSACTIONS_PENDING_BM                            0x00200000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            correctable_error_reporting_enable : 1,
            non_fatal_error_reporting_enable : 1,
            fatal_error_reporting : 1,
            unsupported_request_reporting_enable : 1,
            enable_relaxed_ordering : 1,
            max_payload_size : 3,
            extended_field_enable : 1,
            phanton_func_enable : 1,
            aux_power_pm_enable : 1,
            enable_no_snoop : 1,
            max_read_request_size : 3,
            initiate_functional_level_reset : 1,
            correct_error_detected : 1,
            non_fatal_error_detected : 1,
            fatal_error_detected : 1,
            unsupported_requested_detected : 1,
            aux_power_detected : 1,
            transactions_pending : 1,
            fill0 : 10;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_device_status_and_control_u;
#endif


/* 
 * Initialization value: 0x0010BC42  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LINK_CAPABILITIES_RO                                                        0x0000206C
#define AG_MG_REGS_LINK_CAPABILITIES_RM                                                        0xFF3FFFFF

#define AG_MG_REGS_LINK_CAPABILITIES_SUPPORTED_LINK_SPEED_BO                                    0
#define AG_MG_REGS_LINK_CAPABILITIES_SUPPORTED_LINK_SPEED_BM                                    0x0000000F

#define AG_MG_REGS_LINK_CAPABILITIES_MAX_LINK_WIDTH_BO                                          4
#define AG_MG_REGS_LINK_CAPABILITIES_MAX_LINK_WIDTH_BM                                          0x000003F0

#define AG_MG_REGS_LINK_CAPABILITIES_ASPM_SUPPORT_BO                                            10
#define AG_MG_REGS_LINK_CAPABILITIES_ASPM_SUPPORT_BM                                            0x00000C00

#define AG_MG_REGS_LINK_CAPABILITIES_L0S_EXIT_LATENCY_BO                                        12
#define AG_MG_REGS_LINK_CAPABILITIES_L0S_EXIT_LATENCY_BM                                        0x00007000

#define AG_MG_REGS_LINK_CAPABILITIES_L1_EXIT_LATENCY_BO                                         15
#define AG_MG_REGS_LINK_CAPABILITIES_L1_EXIT_LATENCY_BM                                         0x00038000

#define AG_MG_REGS_LINK_CAPABILITIES_CLOCK_POWER_MANAGEMENT_BO                                  18
#define AG_MG_REGS_LINK_CAPABILITIES_CLOCK_POWER_MANAGEMENT_BM                                  0x00040000

#define AG_MG_REGS_LINK_CAPABILITIES_SURPRISE_DOWN_ERROR_REPORTING_CAPABLE_BO                   19
#define AG_MG_REGS_LINK_CAPABILITIES_SURPRISE_DOWN_ERROR_REPORTING_CAPABLE_BM                   0x00080000

#define AG_MG_REGS_LINK_CAPABILITIES_DATA_LINK__LAYER_ACTIVE_REPORTING_CAPABLE_BO               20
#define AG_MG_REGS_LINK_CAPABILITIES_DATA_LINK__LAYER_ACTIVE_REPORTING_CAPABLE_BM               0x00100000

#define AG_MG_REGS_LINK_CAPABILITIES_LINK_BANDWIDTH_NOTIFICATION_CAPABILITY_BO                  21
#define AG_MG_REGS_LINK_CAPABILITIES_LINK_BANDWIDTH_NOTIFICATION_CAPABILITY_BM                  0x00200000

#define AG_MG_REGS_LINK_CAPABILITIES_PORT_NUMBER_BO                                             24
#define AG_MG_REGS_LINK_CAPABILITIES_PORT_NUMBER_BM                                             0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            supported_link_speed : 4,
            max_link_width : 6,
            aspm_support : 2,
            l0s_exit_latency : 3,
            l1_exit_latency : 3,
            clock_power_management : 1,
            surprise_down_error_reporting_capable : 1,
            data_link__layer_active_reporting_capable : 1,
            link_bandwidth_notification_capability : 1,
            fill0 : 2,                 
            port_number : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_capabilities_u;
#endif


/* 
 * Initialization value: 0x00410000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_RO                                                      0x00002070
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_RM                                                      0xFBFF0FFB

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_ASPM_CTRL_BO                                            0
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_ASPM_CTRL_BM                                            0x00000003

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_RCB_BO                                                  3
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_RCB_BM                                                  0x00000008

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_DISABLED_BO                                        4
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_DISABLED_BM                                        0x00000010

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_RETAIN_LINK_BO                                          5
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_RETAIN_LINK_BM                                          0x00000020

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_COMMON_CLK_CONFIG_BO                                    6
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_COMMON_CLK_CONFIG_BM                                    0x00000040

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_EXTENDED_SYNC_BO                                        7
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_EXTENDED_SYNC_BM                                        0x00000080

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_ENABLE_CLK_PWR_MNG_BO                                   8
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_ENABLE_CLK_PWR_MNG_BM                                   0x00000100

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_HW_AUTONOMOUS_WIDTH_DISABLE_BO                          9
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_HW_AUTONOMOUS_WIDTH_DISABLE_BM                          0x00000200

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_BW_MNG_INTERRUPT_ENABLE_BO                         10
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_BW_MNG_INTERRUPT_ENABLE_BM                         0x00000400

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_AUTONOMOUS_BW_INTERRUPT_ENABLE_BO                  11
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_AUTONOMOUS_BW_INTERRUPT_ENABLE_BM                  0x00000800

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_CURRENT_LINK_SPEED_BO                                   16
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_CURRENT_LINK_SPEED_BM                                   0x000F0000

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_NEGOTIATED_LINK_WIDTH_BO                                20
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_NEGOTIATED_LINK_WIDTH_BM                                0x03F00000

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_TRAINING_BO                                        27
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_TRAINING_BM                                        0x08000000

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_SLOT_CLOCK_CONFIG_BO                                    28
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_SLOT_CLOCK_CONFIG_BM                                    0x10000000

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_DATA_LINK_LAYER_ACTIVE_BO                               29
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_DATA_LINK_LAYER_ACTIVE_BM                               0x20000000

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_BW_MGN_STSTUS_BO                                   30
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_BW_MGN_STSTUS_BM                                   0x40000000

#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_AUTO_BW_STATUS_BO                                  31
#define AG_MG_REGS_LINK_STATUS_AND_CTRL_LINK_AUTO_BW_STATUS_BM                                  0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_STATUS_AND_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            aspm_ctrl : 2,
            fill2 : 1,
            rcb : 1,
            link_disabled : 1,
            retain_link : 1,
            common_clk_config : 1,
            extended_sync : 1,
            enable_clk_pwr_mng : 1,
            hw_autonomous_width_disable : 1,
            link_bw_mng_interrupt_enable : 1,
            link_autonomous_bw_interrupt_enable : 1,
            fill1 : 4,
            current_link_speed : 4,
            negotiated_link_width : 6,
            fill0 : 1,
            link_training : 1,
            slot_clock_config : 1,
            data_link_layer_active : 1,
            link_bw_mgn_ststus : 1,
            link_auto_bw_status : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_status_and_ctrl_u;
#endif


/* 
 * Initialization value: 0x0000001F  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEVICE_CAP2_RO                                                     0x00002084
#define AG_MG_REGS_DEVICE_CAP2_RM                                                     0x0000001F

#define AG_MG_REGS_DEVICE_CAP2_COMPLETION_TIME_OUT_RANGES_SUPPORTED_BO                0
#define AG_MG_REGS_DEVICE_CAP2_COMPLETION_TIME_OUT_RANGES_SUPPORTED_BM                0x0000000F

#define AG_MG_REGS_DEVICE_CAP2_COMPLETION_TIME_OUT_DISABLE_SUPPORT_BO                 4
#define AG_MG_REGS_DEVICE_CAP2_COMPLETION_TIME_OUT_DISABLE_SUPPORT_BM                 0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEVICE_CAP2_U
{
    struct
    {
        ag_mg_regs_register
            completion_time_out_ranges_supported : 4,
            completion_time_out_disable_support : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_device_cap2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DEVICE_CTRL_2_RO                                                  0x00002088
#define AG_MG_REGS_DEVICE_CTRL_2_RM                                                  0x0000001F

#define AG_MG_REGS_DEVICE_CTRL_2_COMPLETION_TIME_OUT_VALUE_BO                        0
#define AG_MG_REGS_DEVICE_CTRL_2_COMPLETION_TIME_OUT_VALUE_BM                        0x0000000F

#define AG_MG_REGS_DEVICE_CTRL_2_COMPLETION_TIME_OUT_DISABLE_BO                      4
#define AG_MG_REGS_DEVICE_CTRL_2_COMPLETION_TIME_OUT_DISABLE_BM                      0x00000010

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEVICE_CTRL_2_U
{
    struct
    {
        ag_mg_regs_register
            completion_time_out_value : 4,
            completion_time_out_disable : 1,
            fill0 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_device_ctrl_2_u;
#endif


/*
 * Initialization value: 0x00000002  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_RO                                                 0x00002090
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_RM                                                 0x00011FFF

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_TARGET_LINK_SPEED_BO                               0
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_TARGET_LINK_SPEED_BM                               0x0000000F

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_ENTER_COMPLIANCE_BO                                4
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_ENTER_COMPLIANCE_BM                                0x00000010

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_HW_AUTONOMOUS_SPEED_DISABLE_BO                     5
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_HW_AUTONOMOUS_SPEED_DISABLE_BM                     0x00000020

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_SELECTABLE_DEMPHASIS_BO                            6
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_SELECTABLE_DEMPHASIS_BM                            0x00000040

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_TRANSMIT_MARGIN_BO                                 7
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_TRANSMIT_MARGIN_BM                                 0x00000380

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_ENTER_MODIFIED_COMPLIANCE_BO                       10
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_ENTER_MODIFIED_COMPLIANCE_BM                       0x00000400

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_COMPLIANCE_DEEMPHASIS_BO                           11
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_COMPLIANCE_DEEMPHASIS_BM                           0x00000800

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_COMPLIANCE_SOS_BO                                  12
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_COMPLIANCE_SOS_BM                                  0x00001000

#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_CURRENT_DEEMPHASIS_LEVEL_BO                        16
#define AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_CURRENT_DEEMPHASIS_LEVEL_BM                        0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_U
{
    struct
    {
        ag_mg_regs_register
            target_link_speed : 4,
            enter_compliance : 1,
            hw_autonomous_speed_disable : 1,
            selectable_demphasis : 1,
            transmit_margin : 3,
            enter_modified_compliance : 1,
            compliance_deemphasis : 1,
            compliance_sos : 1,
            fill1 : 3,
            current_deemphasis_level : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_status_and_control_2_u;
#endif

/*
 * Initialization value: 0x00020010  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MSI_CAPABILITIES_RO                                               0x000020A0
#define AG_MG_REGS_MSI_CAPABILITIES_RM                                               0x00FFFFFF

#define AG_MG_REGS_MSI_CAPABILITIES_MSI_CAPABILITY_ID_BO                             0
#define AG_MG_REGS_MSI_CAPABILITIES_MSI_CAPABILITY_ID_BM                             0x000000FF

#define AG_MG_REGS_MSI_CAPABILITIES_NEXT_CAPABILITY_POINTER_BO                       8
#define AG_MG_REGS_MSI_CAPABILITIES_NEXT_CAPABILITY_POINTER_BM                       0x0000FF00

#define AG_MG_REGS_MSI_CAPABILITIES_MSI_ENABLE_BO                                    16
#define AG_MG_REGS_MSI_CAPABILITIES_MSI_ENABLE_BM                       		     0x00010000

#define AG_MG_REGS_MSI_CAPABILITIES_MSI_MULTIPLE_MSG_CAPABLE_BO                      17
#define AG_MG_REGS_MSI_CAPABILITIES_MSI_MULTIPLE_MSG_CAPABLE_BM              	     0x000E0000

#define AG_MG_REGS_MSI_CAPABILITIES_MSI_MULTIPLE_MSG_ENABLE_BO                       20
#define AG_MG_REGS_MSI_CAPABILITIES_MSI_MULTIPLE_MSG_ENABLE_BM              	     0x00700000

#define AG_MG_REGS_MSI_CAPABILITIES_MSI_64BIT_ADDRESS_CAPABLE_BO                     23
#define AG_MG_REGS_MSI_CAPABILITIES_MSI_64BIT_ADDRESS_CAPABLE_BM              	     0x00800000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MSI_CAPABILITIES_U
{
    struct
    {
        ag_mg_regs_register
            pcie_express_capability_id : 8,
            next_capability_pointer : 8,
            msi_en : 1,
            msi_msg_cap : 3,
            msi_msg_en : 3,
            msi_64bit_cap : 1,
            fill0 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_msi_capabilities_u;
#endif

/*
 * Initialization value: 0x00020010  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MSI_ADDRESS_RO                                     0x000020A4
#define AG_MG_REGS_MSI_ADDRESS_RM                                     0xFFFFFFFC

#define AG_MG_REGS_MSI_ADDRESS_MSI_ADDR_BO                            0
#define AG_MG_REGS_MSI_ADDRESS_MSI_ADDR_BM                            0xFFFFFFFC


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MSI_ADDRESS_U
{
    struct
    {
        ag_mg_regs_register
            msi_addr : 32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_msi_address_u;
#endif

/*
 * Initialization value: 0x00020010  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MSI_UPPER_ADDRESS_RO                                     0x000020A8
#define AG_MG_REGS_MSI_UPPER_ADDRESS_RM                                     0xFFFFFFFC

#define AG_MG_REGS_MSI_UPPER_ADDRESS_MSI_ADDR_BO                            0
#define AG_MG_REGS_MSI_UPPER_ADDRESS_MSI_ADDR_BM                            0xFFFFFFFC


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MSI_UPPER_ADDRESS_U
{
    struct
    {
        ag_mg_regs_register
            msi_upper_addr : 32;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_msi_upper_address_u;
#endif

/*
 * Initialization value: 0x00020010  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_RO                           0x000020AC
#define AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_RM                           0x8000FFFF

#define AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_MSG_DATA_BO                  0
#define AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_MSG_DATA_BM                  0x0000FFFF

#define AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_PCIE_SYS_RESET_BO            31
#define AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_PCIE_SYS_RESET_BM            0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MSI_MESSAGE_DATA_FOR_MSI_U
{
    struct
    {
        ag_mg_regs_register
            msi_upper_addr : 16,
            fill0 : 15,
            pcie_sys_reset : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_msi_message_data_for_msi_u;
#endif


/* 
 * Initialization value: 0x14010001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_RO                                   0x00002100
#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_RM                                   0xFFFFFFFF

#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_EXTENDED_CAP_ID_BO                   0
#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_EXTENDED_CAP_ID_BM                   0x0000FFFF

#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_CAP_VERSIONS_BO                      16
#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_CAP_VERSIONS_BM                      0x000F0000

#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_NEXT_CAPABILITY_OFFSET_BO            20
#define AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_NEXT_CAPABILITY_OFFSET_BM            0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_U
{
    struct
    {
        ag_mg_regs_register
            extended_cap_id : 16,
            cap_versions : 4,
            next_capability_offset : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_advanced_error_report_capability_header_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_RO                                                0x00002104
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_RM                                                0x003FF031

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_UNDEFINED_BO                                      0
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_UNDEFINED_BM                                      0x00000001

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_DATA_LINK_PROTOCOL_ERROR_BO                       4
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_DATA_LINK_PROTOCOL_ERROR_BM                       0x00000010

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_POISONED_LP_STATUS_BO                             12
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_POISONED_LP_STATUS_BM                             0x00001000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_FLOW_CTRL_PROTOCOL_ERR_STATUS_BO                  13
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_FLOW_CTRL_PROTOCOL_ERR_STATUS_BM                  0x00002000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_COMPLETION_TIMEOUT_STATUS_BO                      14
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_COMPLETION_TIMEOUT_STATUS_BM                      0x00004000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_COMPLETER_ABORT_STATUS_BO                         15
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_COMPLETER_ABORT_STATUS_BM                         0x00008000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_UNEXPECTED_COMPLETION_STATUS_BO                   16
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_UNEXPECTED_COMPLETION_STATUS_BM                   0x00010000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_RECEIVER_OVERFLOW_STATUS_BO                       17
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_RECEIVER_OVERFLOW_STATUS_BM                       0x00020000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_MALFORMED_TLP_STATUS_BO                          18
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_MALFORMED_TLP_STATUS_BM                          0x00040000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_ECRC_ERROR_STATUS_BO                              19
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_ECRC_ERROR_STATUS_BM                              0x00080000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_UNSUPPORTED_REQUEST_ERROR_STATUS_BO               20
#define AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_UNSUPPORTED_REQUEST_ERROR_STATUS_BM               0x00100000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            undefined : 1,
            fill2 : 3,
            data_link_protocol_error : 1,
            fill1 : 7,
            poisoned_lp_status : 1,
            flow_ctrl_protocol_err_status : 1,
            completion_timeout_status : 1,
            completer_abort_status : 1,
            unexpected_completion_status : 1,
            receiver_overflow_status : 1,
            malformed_tlp_status : 1,
            ecrc_error_status : 1,
            unsupported_request_error_status : 1,
            fill0 : 11;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_uncorrectable_error_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_RO                                                  0x00002108
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_RM                                                  0x003FF031

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_UNDEFINED_BO                                        0
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_UNDEFINED_BM                                        0x00000001

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_DATA_LINK_PROTOCOL_ERROR_MASK_BO                    4
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_DATA_LINK_PROTOCOL_ERROR_MASK_BM                    0x00000010

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_POISONED_TLP_STATUS_MASK_BO                         12
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_POISONED_TLP_STATUS_MASK_BM                         0x00001000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_FLOW_CTRL_PROTOCOL_ERROR_STATUS_MASK_BO             13
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_FLOW_CTRL_PROTOCOL_ERROR_STATUS_MASK_BM             0x00002000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_COMPLETION_ABORT_STATUS_MASK_BO                     14
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_COMPLETION_ABORT_STATUS_MASK_BM                     0x00004000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_COMPLETER_ABORT_STATUS_BO                           15
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_COMPLETER_ABORT_STATUS_BM                           0x00008000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_UNEXPECTED_COMPLETION_STATUS_MASK_BO                16
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_UNEXPECTED_COMPLETION_STATUS_MASK_BM                0x00010000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_RECEIVER_OVERFLOW_STATUS_MASK_BO                    17
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_RECEIVER_OVERFLOW_STATUS_MASK_BM                    0x00020000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_MALFORMED_TLP_STATUS_MASK_BO                        18
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_MALFORMED_TLP_STATUS_MASK_BM                        0x00040000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_ECRC_ERROR_STATUS_MASK_BO                           19
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_ECRC_ERROR_STATUS_MASK_BM                           0x00080000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_UNSUPPORTED_REQ_ERR_STATUS_MASK_BO                  20
#define AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_UNSUPPORTED_REQ_ERR_STATUS_MASK_BM                  0x00100000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_U
{
    struct
    {
        ag_mg_regs_register
            undefined : 1,
            fill2 : 3,
            data_link_protocol_error_mask : 1,
            fill1 : 7,
            poisoned_tlp_status_mask : 1,
            flow_ctrl_protocol_error_status_mask : 1,
            completion_abort_status_mask : 1,
            completer_abort_status : 1,
            unexpected_completion_status_mask : 1,
            receiver_overflow_status_mask : 1,
            malformed_tlp_status_mask : 1,
            ecrc_error_status_mask : 1,
            unsupported_req_err_status_mask : 1,
            fill0 : 11;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_uncorrectable_error_mask_u;
#endif


/* 
 * Initialization value: 0x00062030  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_RO                                             0x0000210C
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_RM                                             0x003FF031

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_UNDEFINED_BO                                   0
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_UNDEFINED_BM                                   0x00000001

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_DATA_LINK_PROTOCOL_ERROR_SEVERITY_BO           4
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_DATA_LINK_PROTOCOL_ERROR_SEVERITY_BM           0x00000010

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_POISONED_TLP_STATUS_SEVERITY_BO                12
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_POISONED_TLP_STATUS_SEVERITY_BM                0x00001000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_FLOW_CTRL_PROTOCOL_ERROR_STATUS_SEVERITY_BO    13
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_FLOW_CTRL_PROTOCOL_ERROR_STATUS_SEVERITY_BM    0x00002000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_COMPLETION_TIME_OUT_STATUS_SEVERITY_BO         14
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_COMPLETION_TIME_OUT_STATUS_SEVERITY_BM         0x00004000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_COMPLETER_ABORT_STATUS_SEVR_BO                 15
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_COMPLETER_ABORT_STATUS_SEVR_BM                 0x00008000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_UNEXPECETD_COMPLETION_STATUS_SEVR_BO           16
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_UNEXPECETD_COMPLETION_STATUS_SEVR_BM           0x00010000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_RECEIVER_OVERFLOW_STATS_SEVR_BO                17
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_RECEIVER_OVERFLOW_STATS_SEVR_BM                0x00020000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_MALFORMED_TLP_STATUS_SEVR_BO                   18
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_MALFORMED_TLP_STATUS_SEVR_BM                   0x00040000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_ECRC_ERR_STAT_SEVR_BO                          19
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_ECRC_ERR_STAT_SEVR_BM                          0x00080000

#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_UNSUPPORTED_REQ_ERR_STUS_SEVR_BO               20
#define AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_UNSUPPORTED_REQ_ERR_STUS_SEVR_BM               0x00100000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_U
{
    struct
    {
        ag_mg_regs_register
            undefined : 1,
            fill2 : 3,    
            data_link_protocol_error_severity : 1,
            fill1 : 7,
            poisoned_tlp_status_severity : 1,
            flow_ctrl_protocol_error_status_severity : 1,
            completion_time_out_status_severity : 1,
            completer_abort_status_sevr : 1,
            unexpecetd_completion_status_sevr : 1,
            receiver_overflow_stats_sevr : 1,
            malformed_tlp_status_sevr : 1,
            ecrc_err_stat_sevr : 1,
            unsupported_req_err_stus_sevr : 1,
            fill0 : 11;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_uncorrectable_error_severity_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_RO                                                  0x00002110
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_RM                                                  0x000031C1

#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_RECEIVER_ERROR_STATUS_BO                            0
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_RECEIVER_ERROR_STATUS_BM                            0x00000001

#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_BAD_TLP_STATUS_BO                                   6
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_BAD_TLP_STATUS_BM                                   0x00000040

#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_BAD_DLLP_STATUS_BO                                  7
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_BAD_DLLP_STATUS_BM                                  0x00000080

#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_REPLAY_NUM_ROLLOVER_STATUS_BO                       8
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_REPLAY_NUM_ROLLOVER_STATUS_BM                       0x00000100

#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_REPLAY_TIMER_TIME_OUT_STATUS_BO                     12
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_REPLAY_TIMER_TIME_OUT_STATUS_BM                     0x00001000

#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_ADVISORY_NON_FATAL_ERROR_STATUS_BO                  13
#define AG_MG_REGS_CORRECTABLE_ERROR_STATUS_ADVISORY_NON_FATAL_ERROR_STATUS_BM                  0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CORRECTABLE_ERROR_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            receiver_error_status : 1,
            fill2 : 5,
            bad_tlp_status : 1,
            bad_dllp_status : 1,
            replay_num_rollover_status : 1,
            fill1 : 3,
            replay_timer_time_out_status : 1,
            advisory_non_fatal_error_status : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_correctable_error_status_u;
#endif


/* 
 * Initialization value: 0x00002000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_RO                                                    0x00002114
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_RM                                                    0x000031C1

#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_RECEIVER_ERROR_MASK_BO                                0
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_RECEIVER_ERROR_MASK_BM                                0x00000001

#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_BAD_TLP_MASK_BO                                       6
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_BAD_TLP_MASK_BM                                       0x00000040

#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_BAD_DLLP_STATUS_BO                                    7
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_BAD_DLLP_STATUS_BM                                    0x00000080

#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_REPLAY_NUM_ROLLOVER_STATUS_BO                         8
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_REPLAY_NUM_ROLLOVER_STATUS_BM                         0x00000100

#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_REPLAY_TIMER_TIME_OUT_MASK_BO                         12
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_REPLAY_TIMER_TIME_OUT_MASK_BM                         0x00001000

#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_ADVISORY_NON_FATAL_ERROR_MASK_BO                      13
#define AG_MG_REGS_CORRECTABLE_ERROR_MASK_ADVISORY_NON_FATAL_ERROR_MASK_BM                      0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_CORRECTABLE_ERROR_MASK_U
{
    struct
    {
        ag_mg_regs_register
            receiver_error_mask : 1,
            fill2 : 5,
            bad_tlp_mask : 1,
            bad_dllp_status : 1,
            replay_num_rollover_status : 1,
            fill1 : 3,
            replay_timer_time_out_mask : 1,
            advisory_non_fatal_error_mask : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_correctable_error_mask_u;
#endif


/* 
 * Initialization value: 0x000000A0  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_RO                                                    0x00002118
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_RM                                                    0x000001FF

#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_FIRST_ERROR_POINTER_BO                                0
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_FIRST_ERROR_POINTER_BM                                0x0000001F

#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_GEN_CAPABLE_BO                                   5
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_GEN_CAPABLE_BM                                   0x00000020

#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_GEN_ENABLE_BO                                    6
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_GEN_ENABLE_BM                                    0x00000040

#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_CHECK_CAPABLE_BO                                 7
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_CHECK_CAPABLE_BM                                 0x00000080

#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_CHECK_ENABLE_BO                                  8
#define AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_ECRC_CHECK_ENABLE_BM                                  0x00000100

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            first_error_pointer : 5,
            ecrc_gen_capable : 1,
            ecrc_gen_enable : 1,
            ecrc_check_capable : 1,
            ecrc_check_enable : 1,
            fill0 : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_adv_error_cap_and_ctrl_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEADER_LOG_REGISTER0_RO                                                      0x0000211C
#define AG_MG_REGS_HEADER_LOG_REGISTER0_RM                                                      0xFFFFFFFF

#define AG_MG_REGS_HEADER_LOG_REGISTER0_HEAD_LOG_REGISTER_BO                                    0
#define AG_MG_REGS_HEADER_LOG_REGISTER0_HEAD_LOG_REGISTER_BM                                    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEADER_LOG_REGISTER0_U
{
    struct
    {
        ag_mg_regs_register
            head_log_register;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_header_log_register0_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEAD_LOG_REGISTER1_RO                                                        0x00002120
#define AG_MG_REGS_HEAD_LOG_REGISTER1_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_HEAD_LOG_REGISTER1_HEAD_LOG_REGISTER_BO                                      0
#define AG_MG_REGS_HEAD_LOG_REGISTER1_HEAD_LOG_REGISTER_BM                                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEAD_LOG_REGISTER1_U
{
    struct
    {
        ag_mg_regs_register
            head_log_register;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_head_log_register1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEAD_LOG_REGISTER2_RO                                                        0x00002124
#define AG_MG_REGS_HEAD_LOG_REGISTER2_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_HEAD_LOG_REGISTER2_HEAD_LOG_REGISTER_BO                                      0
#define AG_MG_REGS_HEAD_LOG_REGISTER2_HEAD_LOG_REGISTER_BM                                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEAD_LOG_REGISTER2_U
{
    struct
    {
        ag_mg_regs_register
            head_log_register;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_head_log_register2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_HEAD_LOG_REGISTER3_RO                                                        0x00002128
#define AG_MG_REGS_HEAD_LOG_REGISTER3_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_HEAD_LOG_REGISTER3_HEAD_LOG_REGISTER_BO                                      0
#define AG_MG_REGS_HEAD_LOG_REGISTER3_HEAD_LOG_REGISTER_BM                                      0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_HEAD_LOG_REGISTER3_U
{
    struct
    {
        ag_mg_regs_register
            head_log_register;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_head_log_register3_u;
#endif


/* 
 * Initialization value: 0x18010002  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_RO                                                0x00002140
#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_RM                                                0xFFFFFFFF

#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_PCIE_EXTENDED_CAP_ID_BO                           0
#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_PCIE_EXTENDED_CAP_ID_BM                           0x0000FFFF

#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_CAPABILITY_VERSIOSN_BO                            16
#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_CAPABILITY_VERSIOSN_BM                            0x000F0000

#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_NEXT_CAPABILITY_OFFSET_BO                         20
#define AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_NEXT_CAPABILITY_OFFSET_BM                         0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_U
{
    struct
    {
        ag_mg_regs_register
            pcie_extended_cap_id : 16,
            capability_versiosn : 4,
            next_capability_offset : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_enhanced_capability_header_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_RO                                                  0x00002144
#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_RM                                                  0x00000F77

#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_EXTENDED_VC_COUNT_BO                                0
#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_EXTENDED_VC_COUNT_BM                                0x00000007

#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_LOW_PRIORITY_EXTENDED_VC_COUNT_BO                   4
#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_LOW_PRIORITY_EXTENDED_VC_COUNT_BM                   0x00000070

#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_REFERENCE_CLOCK_BO                                  8
#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_REFERENCE_CLOCK_BM                                  0x00000300

#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_PORT_ARB_TBL_ENTRY_SIZE_BO                          10
#define AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_PORT_ARB_TBL_ENTRY_SIZE_BM                          0x00000C00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_U
{
    struct
    {
        ag_mg_regs_register
            extended_vc_count : 3,
            fill2 : 1,
            low_priority_extended_vc_count : 3,
            fill1 : 1,
            reference_clock : 2,
            port_arb_tbl_entry_size : 2,
            fill0 : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_port_vc_capability_reg_1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PORT_VC_CAP_REG_2_RO                                                        0x00002148
#define AG_MG_REGS_PORT_VC_CAP_REG_2_RM                                                        0xFF0000FF

#define AG_MG_REGS_PORT_VC_CAP_REG_2_VC_ARB_CAP_BO                                              0
#define AG_MG_REGS_PORT_VC_CAP_REG_2_VC_ARB_CAP_BM                                              0x000000FF

#define AG_MG_REGS_PORT_VC_CAP_REG_2_VC_ARB_TBLE_OFFSET_BO                                      24
#define AG_MG_REGS_PORT_VC_CAP_REG_2_VC_ARB_TBLE_OFFSET_BM                                      0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PORT_VC_CAP_REG_2_U
{
    struct
    {
        ag_mg_regs_register
            vc_arb_cap : 8,
            fill0 : 16,
            vc_arb_tble_offset : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_port_vc_cap_reg_2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_RO                                                    0x0000214C
#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_RM                                                    0x0001000F

#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_LOAD_VC_ARB_TABLE_BO                                  0
#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_LOAD_VC_ARB_TABLE_BM                                  0x00000001

#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_VC_ARB_SLE_BO                                         1
#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_VC_ARB_SLE_BM                                         0x0000000E

#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_VC_ARB_TABLE_STATUS_BO                                16
#define AG_MG_REGS_PORT_VC_CONTROL_STATUS_VC_ARB_TABLE_STATUS_BM                                0x00010000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PORT_VC_CONTROL_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            load_vc_arb_table : 1,
            vc_arb_sle : 3,
            fill1 : 12,
            vc_arb_table_status : 1,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_port_vc_control_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_RO                                                     0x00002150
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_RM                                                     0xFFFFC0FF

#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_PORT_ARB_CAP_BO                                        0
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_PORT_ARB_CAP_BM                                        0x000000FF

#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_UNDEFINED_BO                                           14
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_UNDEFINED_BM                                           0x00004000

#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_REJECT_SNOOP_TRANSACTION_BO                            15
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_REJECT_SNOOP_TRANSACTION_BM                            0x00008000

#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_MAX_TIME_SLOTS_BO                                      16
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_MAX_TIME_SLOTS_BM                                      0x007F0000

#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_RESREVED2_0X150_BO                                     23
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_RESREVED2_0X150_BM                                     0x00800000

#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_PORT_ARB_TABLE_OFFSET_BO                               24
#define AG_MG_REGS_VC_RESOURCE_CAP_REG_0_PORT_ARB_TABLE_OFFSET_BM                               0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_RESOURCE_CAP_REG_0_U
{
    struct
    {
        ag_mg_regs_register
            port_arb_cap : 8,
            fill0 : 6,
            undefined : 1,
            reject_snoop_transaction : 1,
            max_time_slots : 7,
            resreved2_0x150 : 1,
            port_arb_table_offset : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_resource_cap_reg_0_u;
#endif


/* 
 * Initialization value: 0x800000FF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_RO                                                 0x00002154
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_RM                                                 0x870F00FF

#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_TV_VC_MAP_BO                                       0
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_TV_VC_MAP_BM                                       0x000000FF

#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_LOAD_POINT_ARB_TABLE_BO                            16
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_LOAD_POINT_ARB_TABLE_BM                            0x00010000

#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_PORT_ARB_SEL_BO                                    17
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_PORT_ARB_SEL_BM                                    0x000E0000

#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_VC_ID_BO                                           24
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_VC_ID_BM                                           0x07000000

#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_VC_ENABLE_BO                                       31
#define AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_VC_ENABLE_BM                                       0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_U
{
    struct
    {
        ag_mg_regs_register
            tv_vc_map : 8,
            fill2 : 8,
            load_point_arb_table : 1,
            port_arb_sel : 3,
            fill1 : 4,
            vc_id : 3,
            fill0 : 4,
            vc_enable : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_resource_control_reg_0_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_RO                                                  0x00002158
#define AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_RM                                                  0x0003FFFF

#define AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_PORT_ARB_TABLE_STATUS_BO                            16
#define AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_PORT_ARB_TABLE_STATUS_BM                            0x00010000

#define AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_VC_NEG_PENDING_BO                                   17
#define AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_VC_NEG_PENDING_BM                                   0x00020000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 16,
            port_arb_table_status : 1,
            vc_neg_pending : 1,
            fill0 : 14;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc_resource_status_reg_0_u;
#endif


/* 
 * Initialization value: 0x19010004  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ENHANCED_CAP_HEADER_RO                                                       0x00002180
#define AG_MG_REGS_ENHANCED_CAP_HEADER_RM                                                       0xFFFFFFFF

#define AG_MG_REGS_ENHANCED_CAP_HEADER_PCIE_EXTENDED_CAP_ID_BO                                  0
#define AG_MG_REGS_ENHANCED_CAP_HEADER_PCIE_EXTENDED_CAP_ID_BM                                  0x0000FFFF

#define AG_MG_REGS_ENHANCED_CAP_HEADER_CAPABILITY_VER_BO                                        16
#define AG_MG_REGS_ENHANCED_CAP_HEADER_CAPABILITY_VER_BM                                        0x000F0000

#define AG_MG_REGS_ENHANCED_CAP_HEADER_NEXT_CAP_OFFSET_BO                                       20
#define AG_MG_REGS_ENHANCED_CAP_HEADER_NEXT_CAP_OFFSET_BM                                       0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ENHANCED_CAP_HEADER_U
{
    struct
    {
        ag_mg_regs_register
            pcie_extended_cap_id : 16,
            capability_ver : 4,
            next_cap_offset : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_enhanced_cap_header_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_DATA_SELECT_REG_RO                                                        0x00002184
#define AG_MG_REGS_DATA_SELECT_REG_RM                                                        0x000000FF

#define AG_MG_REGS_DATA_SELECT_REG_DATA_SELECT_BO                                               0
#define AG_MG_REGS_DATA_SELECT_REG_DATA_SELECT_BM                                               0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DATA_SELECT_REG_U
{
    struct
    {
        ag_mg_regs_register
            data_select : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_data_select_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DATA_REGISTER_RO                                                        0x00002188
#define AG_MG_REGS_DATA_REGISTER_RM                                                        0x001FFFFF

#define AG_MG_REGS_DATA_REGISTER_BASE_POWER_BM 0x000000FF
#define AG_MG_REGS_DATA_REGISTER_BASE_POWER_BO 0

#define AG_MG_REGS_DATA_REGISTER_DATA_SCALE_BO                                                  8
#define AG_MG_REGS_DATA_REGISTER_DATA_SCALE_BM                                                  0x00000300

#define AG_MG_REGS_DATA_REGISTER_PM_SUB_STATE_BO                                                10
#define AG_MG_REGS_DATA_REGISTER_PM_SUB_STATE_BM                                                0x00001C00

#define AG_MG_REGS_DATA_REGISTER_PM_STATE_BO                                                    13
#define AG_MG_REGS_DATA_REGISTER_PM_STATE_BM                                                    0x00006000

#define AG_MG_REGS_DATA_REGISTER_TYPE_BO                                                        15
#define AG_MG_REGS_DATA_REGISTER_TYPE_BM                                                        0x00038000

#define AG_MG_REGS_DATA_REGISTER_POWER_RAIL_BO                                                  18
#define AG_MG_REGS_DATA_REGISTER_POWER_RAIL_BM                                                  0x001C0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DATA_REGISTER_U
{
    struct
    {
        ag_mg_regs_register
            base_power : 8,
            data_scale : 2,
            pm_sub_state : 3,
            pm_state : 2,
            type : 3,
            power_rail : 3,
            fill0 : 11;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_data_register_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PWR_BUDGET_CAP_RO                                                        0x0000218C
#define AG_MG_REGS_PWR_BUDGET_CAP_RM                                                        0x00000001

#define AG_MG_REGS_PWR_BUDGET_CAP_SYSTEM_ALLOCATED_BO                                           0
#define AG_MG_REGS_PWR_BUDGET_CAP_SYSTEM_ALLOCATED_BM                                           0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PWR_BUDGET_CAP_U
{
    struct
    {
        ag_mg_regs_register
            system_allocated : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pwr_budget_cap_u;
#endif


/* 
 * Initialization value: 0x0001000E  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_RO                                                  0x00002190
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_RM                                                  0xFFFFFFFF

#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_PCIE_EXTENDED_CAP_ID_BO                             0
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_PCIE_EXTENDED_CAP_ID_BM                             0x0000FFFF

#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_CAP_VERSIONS_BO                                     16
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_CAP_VERSIONS_BM                                     0x000F0000

#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_NEXT_CAP_OFFSET_BO                                  20
#define AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_NEXT_CAP_OFFSET_BM                                  0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_U
{
    struct
    {
        ag_mg_regs_register
            pcie_extended_cap_id : 16,
            cap_versions : 4,
            next_cap_offset : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_enhanced_cap_header_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_ARI_CAP_CTRL_RO                                                        0x00002194
#define AG_MG_REGS_ARI_CAP_CTRL_RM                                                        0x0073FF03

#define AG_MG_REGS_ARI_CAP_CTRL_MFVC_FCT_GROUPS_CAP_BO                                          0
#define AG_MG_REGS_ARI_CAP_CTRL_MFVC_FCT_GROUPS_CAP_BM                                          0x00000001

#define AG_MG_REGS_ARI_CAP_CTRL_ACS_FCT_GROUPS_CAP_BO                                           1
#define AG_MG_REGS_ARI_CAP_CTRL_ACS_FCT_GROUPS_CAP_BM                                           0x00000002

#define AG_MG_REGS_ARI_CAP_CTRL_NEXT_FCT_NUMBER_BO                                              8
#define AG_MG_REGS_ARI_CAP_CTRL_NEXT_FCT_NUMBER_BM                                              0x0000FF00

#define AG_MG_REGS_ARI_CAP_CTRL_MFVC_FCT_GROUPS_ENABLE_CTRL_BO                                  16
#define AG_MG_REGS_ARI_CAP_CTRL_MFVC_FCT_GROUPS_ENABLE_CTRL_BM                                  0x00010000

#define AG_MG_REGS_ARI_CAP_CTRL_ACS_FCT_GRPS_ENABLE_CTRL_BO                                     17
#define AG_MG_REGS_ARI_CAP_CTRL_ACS_FCT_GRPS_ENABLE_CTRL_BM                                     0x00020000

#define AG_MG_REGS_ARI_CAP_CTRL_FCT_GROUP_CTRL_BO                                               20
#define AG_MG_REGS_ARI_CAP_CTRL_FCT_GROUP_CTRL_BM                                               0x00700000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PCIE_ARI_CAP_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            mfvc_fct_groups_cap : 1,
            acs_fct_groups_cap : 1,
	    fill0 : 6,
            next_fct_number : 8,
            mfvc_fct_groups_enable_cntrl : 1,
            acs_fct_groups_enable_cntrl : 1,
	    fill1 : 2,
            fct_group_cntrl : 3,
	    fill2 : 9;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pcie_ari_cap_control_u;
#endif

/* Link Layer (LL) section */

/* 
 * Initialization value: 0x00310005  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LINK_CORE_ID_RO                                                        0x00003000
#define AG_MG_REGS_LINK_CORE_ID_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LINK_CORE_ID_REVISION_ID_BO                                                        0
#define AG_MG_REGS_LINK_CORE_ID_REVISION_ID_BM                                                        0x0000FFFF

#define AG_MG_REGS_LINK_CORE_ID_R2_BO                                                        16
#define AG_MG_REGS_LINK_CORE_ID_R2_BM                                                        0x000F0000

#define AG_MG_REGS_LINK_CORE_ID_LINKWIDTHTYPE_BO                                                      20
#define AG_MG_REGS_LINK_CORE_ID_LINKWIDTHTYPE_BM                                                      0x00F00000

#define AG_MG_REGS_LINK_CORE_ID_R1_BO                                                        24
#define AG_MG_REGS_LINK_CORE_ID_R1_BM                                                        0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_CORE_ID_U
{
    struct
    {
        ag_mg_regs_register
            revision_id : 16,
            r2 : 4,
            linkwidthtype : 4,
            r1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_core_id_u;
#endif


/* 
 * Initialization value: 0x00000041  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LINK_STATUS_RO                                                        0x00003004
#define AG_MG_REGS_LINK_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LINK_STATUS_LINKSPEED_BO                                                        0
#define AG_MG_REGS_LINK_STATUS_LINKSPEED_BM                                                        0x0000000F

#define AG_MG_REGS_LINK_STATUS_NEGOTIATED_WIDTH_BO                                                    4
#define AG_MG_REGS_LINK_STATUS_NEGOTIATED_WIDTH_BM                                                    0x000003F0

#define AG_MG_REGS_LINK_STATUS_LTC_BO                                                        10
#define AG_MG_REGS_LINK_STATUS_LTC_BM                                                        0x00000400

#define AG_MG_REGS_LINK_STATUS_LTI_BO                                                        11
#define AG_MG_REGS_LINK_STATUS_LTI_BM                                                        0x00000800

#define AG_MG_REGS_LINK_STATUS_SC_BO                                                        12
#define AG_MG_REGS_LINK_STATUS_SC_BM                                                        0x00001000

#define AG_MG_REGS_LINK_STATUS_DL_BO                                                        13
#define AG_MG_REGS_LINK_STATUS_DL_BM                                                        0x00002000

#define AG_MG_REGS_LINK_STATUS_LBMS_BO                                                        14
#define AG_MG_REGS_LINK_STATUS_LBMS_BM                                                        0x00004000

#define AG_MG_REGS_LINK_STATUS_LABS_BO                                                        15
#define AG_MG_REGS_LINK_STATUS_LABS_BM                                                        0x00008000

#define AG_MG_REGS_LINK_STATUS_R_BO                                                        16
#define AG_MG_REGS_LINK_STATUS_R_BM                                                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINK_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            linkspeed : 4,
            negotiated_width : 6,
            ltc : 1,
            lti : 1,
            sc : 1,
            dl : 1,
            lbms : 1,
            labs : 1,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_link_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_INTERRUPT_STATUS_RO                                                        0x00003008
#define AG_MG_REGS_INTERRUPT_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_INTERRUPT_STATUS_RXI_BO                                                        0
#define AG_MG_REGS_INTERRUPT_STATUS_RXI_BM                                                        0x00000001

#define AG_MG_REGS_INTERRUPT_STATUS_TXI_BO                                                        1
#define AG_MG_REGS_INTERRUPT_STATUS_TXI_BM                                                        0x00000002

#define AG_MG_REGS_INTERRUPT_STATUS_LTSSM_BO                                                        2
#define AG_MG_REGS_INTERRUPT_STATUS_LTSSM_BM                                                        0x00000004

#define AG_MG_REGS_INTERRUPT_STATUS_P1O_BO                                                        3
#define AG_MG_REGS_INTERRUPT_STATUS_P1O_BM                                                        0x00000008

#define AG_MG_REGS_INTERRUPT_STATUS_P2O_BO                                                        4
#define AG_MG_REGS_INTERRUPT_STATUS_P2O_BM                                                        0x00000010

#define AG_MG_REGS_INTERRUPT_STATUS_R_BO                                                        5
#define AG_MG_REGS_INTERRUPT_STATUS_R_BM                                                        0x7FFFFFE0

#define AG_MG_REGS_INTERRUPT_STATUS_AIA_BO                                                        31
#define AG_MG_REGS_INTERRUPT_STATUS_AIA_BM                                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_INTERRUPT_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            rxi : 1,
            txi : 1,
            ltssm : 1,
            p1o : 1,
            p2o : 1,
            r : 26,
            aia : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_interrupt_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_RO                                              0x0000300C
#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_RM                                              0xFFFFFFFF

#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_R2_BO                                           0
#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_R2_BM                                           0x00000007

#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_SP1_BO                                          3
#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_SP1_BM                                          0x00000008

#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_SP2_BO                                          4
#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_SP2_BM                                          0x00000010

#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_R1_BO                                           5
#define AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_R1_BM                                           0xFFFFFFE0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_U
{
    struct
    {
        ag_mg_regs_register
            r2 : 3,
            sp1 : 1,
            sp2 : 1,
            r1 : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_performance_counter_interrupt_test_u;
#endif


/* 
 * Initialization value: 0x8000001F  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_INTERRUPT_MASK_RO                                                        0x00003010
#define AG_MG_REGS_INTERRUPT_MASK_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_INTERRUPT_MASK_RXI_BO                                                        0
#define AG_MG_REGS_INTERRUPT_MASK_RXI_BM                                                        0x00000001

#define AG_MG_REGS_INTERRUPT_MASK_TXI_BO                                                        1
#define AG_MG_REGS_INTERRUPT_MASK_TXI_BM                                                        0x00000002

#define AG_MG_REGS_INTERRUPT_MASK_LTI_BO                                                        2
#define AG_MG_REGS_INTERRUPT_MASK_LTI_BM                                                        0x00000004

#define AG_MG_REGS_INTERRUPT_MASK_PC1_BO                                                        3
#define AG_MG_REGS_INTERRUPT_MASK_PC1_BM                                                        0x00000008

#define AG_MG_REGS_INTERRUPT_MASK_PC2_BO                                                        4
#define AG_MG_REGS_INTERRUPT_MASK_PC2_BM                                                        0x00000010

#define AG_MG_REGS_INTERRUPT_MASK_R_BO                                                        5
#define AG_MG_REGS_INTERRUPT_MASK_R_BM                                                        0x7FFFFFE0

#define AG_MG_REGS_INTERRUPT_MASK_IEM_BO                                                        31
#define AG_MG_REGS_INTERRUPT_MASK_IEM_BM                                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_INTERRUPT_MASK_U
{
    struct
    {
        ag_mg_regs_register
            rxi : 1,
            txi : 1,
            lti : 1,
            pc1 : 1,
            pc2 : 1,
            r : 26,
            iem : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_interrupt_mask_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TEST_CONTROL1_RO                                                        0x00003020
#define AG_MG_REGS_TEST_CONTROL1_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TEST_CONTROL1_NU_BO                                                        0
#define AG_MG_REGS_TEST_CONTROL1_NU_BM                                                        0x00000001

#define AG_MG_REGS_TEST_CONTROL1_DSA_BO                                                        1
#define AG_MG_REGS_TEST_CONTROL1_DSA_BM                                                        0x00000002

#define AG_MG_REGS_TEST_CONTROL1_DSR_BO                                                        2
#define AG_MG_REGS_TEST_CONTROL1_DSR_BM                                                        0x00000004

#define AG_MG_REGS_TEST_CONTROL1_DNTT_BO                                                        3
#define AG_MG_REGS_TEST_CONTROL1_DNTT_BM                                                        0x00000008

#define AG_MG_REGS_TEST_CONTROL1_IRXZP_BO                                                        4
#define AG_MG_REGS_TEST_CONTROL1_IRXZP_BM                                                        0x00000010

#define AG_MG_REGS_TEST_CONTROL1_FRBE_BO                                                        5
#define AG_MG_REGS_TEST_CONTROL1_FRBE_BM                                                        0x00000020

#define AG_MG_REGS_TEST_CONTROL1_FSBE_BO                                                        6
#define AG_MG_REGS_TEST_CONTROL1_FSBE_BM                                                        0x00000040

#define AG_MG_REGS_TEST_CONTROL1_R_BO                                                        7
#define AG_MG_REGS_TEST_CONTROL1_R_BM                                                        0x7FFFFF80

#define AG_MG_REGS_TEST_CONTROL1_TME_BO                                                        31
#define AG_MG_REGS_TEST_CONTROL1_TME_BM                                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEST_CONTROL1_U
{
    struct
    {
        ag_mg_regs_register
            nu : 1,
            dsa : 1,
            dsr : 1,
            dntt : 1,
            irxzp : 1,
            frbe : 1,
            fsbe : 1,
            r : 24,
            tme : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_test_control1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TEST_CONTROL2_RO                                                        0x00003024
#define AG_MG_REGS_TEST_CONTROL2_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TEST_CONTROL2_FRTT_BO                                                        0
#define AG_MG_REGS_TEST_CONTROL2_FRTT_BM                                                        0x00000001

#define AG_MG_REGS_TEST_CONTROL2_FILC_BO                                                        1
#define AG_MG_REGS_TEST_CONTROL2_FILC_BM                                                        0x00000002

#define AG_MG_REGS_TEST_CONTROL2_TEDB_BO                                                        2
#define AG_MG_REGS_TEST_CONTROL2_TEDB_BM                                                        0x00000004

#define AG_MG_REGS_TEST_CONTROL2_TLPW_BO                                                        3
#define AG_MG_REGS_TEST_CONTROL2_TLPW_BM                                                        0x00000008

#define AG_MG_REGS_TEST_CONTROL2_ATDR_BO                                                        4
#define AG_MG_REGS_TEST_CONTROL2_ATDR_BM                                                        0x00000010

#define AG_MG_REGS_TEST_CONTROL2_NTDR_BO                                                        5
#define AG_MG_REGS_TEST_CONTROL2_NTDR_BM                                                        0x00000020

#define AG_MG_REGS_TEST_CONTROL2_FDBD_BO                                                        6
#define AG_MG_REGS_TEST_CONTROL2_FDBD_BM                                                        0x00000040

#define AG_MG_REGS_TEST_CONTROL2_FDWE_BO                                                        7
#define AG_MG_REGS_TEST_CONTROL2_FDWE_BM                                                        0x00000080

#define AG_MG_REGS_TEST_CONTROL2_R_BO                                                        8
#define AG_MG_REGS_TEST_CONTROL2_R_BM                                                        0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEST_CONTROL2_U
{
    struct
    {
        ag_mg_regs_register
            frtt : 1,
            filc : 1,
            tedb : 1,
            tlpw : 1,
            atdr : 1,
            ntdr : 1,
            fdbd : 1,
            fdwe : 1,
            r : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_test_control2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TEST_DATA_RO                                                        0x00003028
#define AG_MG_REGS_TEST_DATA_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TEST_DATA_TEST_DATA_BO                                                        0
#define AG_MG_REGS_TEST_DATA_TEST_DATA_BM                                                        0x00000FFF

#define AG_MG_REGS_TEST_DATA_R_BO                                                        12
#define AG_MG_REGS_TEST_DATA_R_BM                                                        0xFFFFF000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TEST_DATA_U
{
    struct
    {
        ag_mg_regs_register
            test_data : 12,
            r : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_test_data_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_RO                                                        0x00003030
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_MEMORY_ADDRESS_BO                                           0
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_MEMORY_ADDRESS_BM                                           0x0000FFFF

#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_R_BO                                                        16
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_R_BM                                                        0x0FFF0000

#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_FIFS_BO                                                     28
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_FIFS_BM                                                     0x10000000

#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_RWSL_BO                                                     29
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_RWSL_BM                                                     0x20000000

#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_GOBIT_BO                                                    30
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_GOBIT_BM                                                    0x40000000

#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_DONE_BO                                                     31
#define AG_MG_REGS_MEMORY_ADDRESS_CONTROL_DONE_BM                                                     0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORY_ADDRESS_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            memory_address : 16,
            r : 12,
            fifs : 1,
            rwsl : 1,
            gobit : 1,
            done : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memory_address_control_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MEMORYDATALOAD0_RO                                                        0x00003034
#define AG_MG_REGS_MEMORYDATALOAD0_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MEMORYDATALOAD0_DATA0_BO                                                        0
#define AG_MG_REGS_MEMORYDATALOAD0_DATA0_BM                                                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORYDATALOAD0_U
{
    struct
    {
        ag_mg_regs_register
            data0;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memorydataload0_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MEMORYDATALOAD1_RO                                                        0x00003038
#define AG_MG_REGS_MEMORYDATALOAD1_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MEMORYDATALOAD1_DATA1_BO                                                        0
#define AG_MG_REGS_MEMORYDATALOAD1_DATA1_BM                                                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORYDATALOAD1_U
{
    struct
    {
        ag_mg_regs_register
            data1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memorydataload1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MEMORYDATALOAD2_RO                                                        0x0000303C
#define AG_MG_REGS_MEMORYDATALOAD2_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MEMORYDATALOAD2_DATA2_BO                                                        0
#define AG_MG_REGS_MEMORYDATALOAD2_DATA2_BM                                                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORYDATALOAD2_U
{
    struct
    {
        ag_mg_regs_register
            data2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memorydataload2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MEMORYDATALOAD3_RO                                                        0x00003040
#define AG_MG_REGS_MEMORYDATALOAD3_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MEMORYDATALOAD3_DATA3_BO                                                        0
#define AG_MG_REGS_MEMORYDATALOAD3_DATA3_BM                                                        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORYDATALOAD3_U
{
    struct
    {
        ag_mg_regs_register
            data3;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memorydataload3_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_MEMORYDATALOAD4_RO                                                        0x00003044
#define AG_MG_REGS_MEMORYDATALOAD4_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_MEMORYDATALOAD4_DATA4_BO                                                        0
#define AG_MG_REGS_MEMORYDATALOAD4_DATA4_BM                                                        0x0000FFFF

#define AG_MG_REGS_MEMORYDATALOAD4_R_BO                                                        16
#define AG_MG_REGS_MEMORYDATALOAD4_R_BM                                                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_MEMORYDATALOAD4_U
{
    struct
    {
        ag_mg_regs_register
            data4 : 16,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_memorydataload4_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_RO                                                    0x00003080
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_RM                                                    0xFFFFFFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_COUNTERSELECT1_BO                                     0
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_COUNTERSELECT1_BM                                     0x0000FFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_COUNTERSELECT2_BO                                     16
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_COUNTERSELECT2_BM                                     0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_U
{
    struct
    {
        ag_mg_regs_register
            counterselect1 : 16,
            counterselect2 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_linkperformancecounterselect_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RO                                                   0x00003084
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RM                                                   0xFFFFFFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC1_BO                                              0
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC1_BM                                              0x00000001

#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC1O_BO                                             1
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC1O_BM                                             0x00000002

#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC2_BO                                              2
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC2_BM                                              0x00000004

#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC2O_BO                                             3
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RPC2O_BM                                             0x00000008

#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_R_BO                                                 4
#define AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_R_BM                                                 0xFFFFFFF0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_U
{
    struct
    {
        ag_mg_regs_register
            rpc1 : 1,
            rpc1o : 1,
            rpc2 : 1,
            rpc2o : 1,
            r : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_linkperformancecountercontrol_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_RO                                            0x00003088
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_RM                                            0xFFFFFFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_SAMPLEDURATIONVALUE_BO                        0
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_SAMPLEDURATIONVALUE_BM                        0x0000001F

#define AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_R_BO                                          5
#define AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_R_BM                                          0xFFFFFFE0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_U
{
    struct
    {
        ag_mg_regs_register
            sampledurationvalue : 5,
            r : 27;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_linkperformancecountersampleduration_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_RO                                                        0x00003090
#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_PERFORMANCECOUNTER1_BO                                     0
#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_PERFORMANCECOUNTER1_BM                                     0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINKPERFORMANCECOUNTER1_U
{
    struct
    {
        ag_mg_regs_register
            performancecounter1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_linkperformancecounter1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_TEST_RO                                                    0x00003094
#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_TEST_RM                                                    0xFFFFFFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_TEST_PERFORMANCE_COUNTER1_TEST_BO                          0
#define AG_MG_REGS_LINKPERFORMANCECOUNTER1_TEST_PERFORMANCE_COUNTER1_TEST_BM                          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINKPERFORMANCECOUNTER1_TEST_U
{
    struct
    {
        ag_mg_regs_register
            performance_counter1_test;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_linkperformancecounter1_test_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LINKPERFORMANCECOUNTER_2_RO                                                        0x00003098
#define AG_MG_REGS_LINKPERFORMANCECOUNTER_2_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LINKPERFORMANCECOUNTER_2_PERFORMANCECOUNTER2_BO                                    0
#define AG_MG_REGS_LINKPERFORMANCECOUNTER_2_PERFORMANCECOUNTER2_BM                                    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LINKPERFORMANCECOUNTER_2_U
{
    struct
    {
        ag_mg_regs_register
            performancecounter2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_linkperformancecounter_2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Write Only
 */
#define AG_MG_REGS_PERFORMANCECOUNTER2_TEST_RO                                                        0x0000309C
#define AG_MG_REGS_PERFORMANCECOUNTER2_TEST_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_PERFORMANCECOUNTER2_TEST_PERFORMANCECOUNTER2_TEST_BO                               0
#define AG_MG_REGS_PERFORMANCECOUNTER2_TEST_PERFORMANCECOUNTER2_TEST_BM                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PERFORMANCECOUNTER2_TEST_U
{
    struct
    {
        ag_mg_regs_register
            performancecounter2_test;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_performancecounter2_test_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEBUG_STATUS_RO                                                        0x000030A0
#define AG_MG_REGS_DEBUG_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_DEBUG_STATUS_DEBUG_STATUS_BO                                                       0
#define AG_MG_REGS_DEBUG_STATUS_DEBUG_STATUS_BM                                                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEBUG_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            debug_status;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_debug_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_DEBUGCONFIGURATION_RO                                                        0x000030A4
#define AG_MG_REGS_DEBUGCONFIGURATION_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_DEBUGCONFIGURATION_DEBUGCONFIG_BO                                                  0
#define AG_MG_REGS_DEBUGCONFIGURATION_DEBUGCONFIG_BM                                                  0x0000FFFF

#define AG_MG_REGS_DEBUGCONFIGURATION_R_BO                                                        16
#define AG_MG_REGS_DEBUGCONFIGURATION_R_BM                                                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_DEBUGCONFIGURATION_U
{
    struct
    {
        ag_mg_regs_register
            debugconfig : 16,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_debugconfiguration_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_CONFIGURATION_RO                                                        0x00003100
#define AG_MG_REGS_TX_CONFIGURATION_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TX_CONFIGURATION_AUD_BO                                                        0
#define AG_MG_REGS_TX_CONFIGURATION_AUD_BM                                                        0x00000001

#define AG_MG_REGS_TX_CONFIGURATION_LTWE_BO                                                        1
#define AG_MG_REGS_TX_CONFIGURATION_LTWE_BM                                                        0x00000002

#define AG_MG_REGS_TX_CONFIGURATION_RAFD_BO                                                        2
#define AG_MG_REGS_TX_CONFIGURATION_RAFD_BM                                                        0x00000004

#define AG_MG_REGS_TX_CONFIGURATION_EEBP_BO                                                        3
#define AG_MG_REGS_TX_CONFIGURATION_EEBP_BM                                                        0x00000008

#define AG_MG_REGS_TX_CONFIGURATION_R_BO                                                        4
#define AG_MG_REGS_TX_CONFIGURATION_R_BM                                                        0xFFFFFFF0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_CONFIGURATION_U
{
    struct
    {
        ag_mg_regs_register
            aud : 1,
            ltwe : 1,
            rafd : 1,
            eebp : 1,
            r : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_configuration_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TX_LINK_STATUS_RO                                                        0x00003104
#define AG_MG_REGS_TX_LINK_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TX_LINK_STATUS_LSMS_BO                                                        0
#define AG_MG_REGS_TX_LINK_STATUS_LSMS_BM                                                        0x00000007

#define AG_MG_REGS_TX_LINK_STATUS_DLUS_BO                                                        3
#define AG_MG_REGS_TX_LINK_STATUS_DLUS_BM                                                        0x00000008

#define AG_MG_REGS_TX_LINK_STATUS_R_BO                                                        4
#define AG_MG_REGS_TX_LINK_STATUS_R_BM                                                        0xFFFFFFF0

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_LINK_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            lsms : 3,
            dlus : 1,
            r : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_link_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RO                                                        0x00003108
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_DLPE_BO                                                    0
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_DLPE_BM                                                    0x00000001

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RNR_BO                                                     1
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RNR_BM                                                     0x00000002

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RTT_BO                                                     2
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RTT_BM                                                     0x00000004

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_U_BO                                                       3
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_U_BM                                                       0x00000008

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_EIPE_BO                                                    4
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_EIPE_BM                                                    0x00000010

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_ETPE_BO                                                    5
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_ETPE_BM                                                    0x00000020

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RBPE_BO                                                    6
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RBPE_BM                                                    0x00000040

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_SBPE_BO                                                    7
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_SBPE_BM                                                    0x00000080

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RBOE_BO                                                    8
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RBOE_BM                                                    0x00000100

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RBUE_BO                                                    9
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RBUE_BM                                                    0x00000200

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_SBOE_BO                                                    10
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_SBOE_BM                                                    0x00000400

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_SBUE_BO                                                    11
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_SBUE_BM                                                    0x00000800

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TXPE_BO                                                    12
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TXPE_BM                                                    0x00001000

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_R2_BO                                                      13
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_R2_BM                                                      0x7FFFE000

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_AUE_BO                                                     31
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_AUE_BM                                                     0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_INTERRUPT_AND_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            dlpe : 1,
            rnr : 1,
            rtt : 1,
            u : 1,
            eipe : 1,
            etpe : 1,
            rbpe : 1,
            sbpe : 1,
            rboe : 1,
            rbue : 1,
            sboe : 1,
            sbue : 1,
            txpe : 1,
            r2 : 18,
            aue : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_interrupt_and_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RO                                                    0x0000310C
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RM                                                    0xFFFFFFFF

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_DLPET_BO                                              0
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_DLPET_BM                                              0x00000001

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RNRT_BO                                               1
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RNRT_BM                                               0x00000002

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RTTT_BO                                               2
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RTTT_BM                                               0x00000004

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_U_BO                                                  3
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_U_BM                                                  0x00000008

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_EIPET_BO                                              4
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_EIPET_BM                                              0x00000010

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_ETPET_BO                                              5
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_ETPET_BM                                              0x00000020

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RBPET_BO                                              6
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RBPET_BM                                              0x00000040

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_SBPET_BO                                              7
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_SBPET_BM                                              0x00000080

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RBOET_BO                                              8
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RBOET_BM                                              0x00000100

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RBUET_BO                                              9
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RBUET_BM                                              0x00000200

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_SBOET_BO                                              10
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_SBOET_BM                                              0x00000400

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_SBUET_BO                                              11
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_SBUET_BM                                              0x00000800

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_TXPET_BO                                              12
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_TXPET_BM                                              0x00001000

#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_R_BO                                                  13
#define AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_R_BM                                                  0xFFFFE000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_U
{
    struct
    {
        ag_mg_regs_register
            dlpet : 1,
            rnrt : 1,
            rttt : 1,
            u : 1,
            eipet : 1,
            etpet : 1,
            rbpet : 1,
            sbpet : 1,
            rboet : 1,
            rbuet : 1,
            sboet : 1,
            sbuet : 1,
            txpet : 1,
            r : 19;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_interrupt_and_status_test_u;
#endif


/* 
 * Initialization value: 0x80001FFF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_INTERRUPT_MASK_RO                                                        0x00003110
#define AG_MG_REGS_TX_INTERRUPT_MASK_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TX_INTERRUPT_MASK_DLPEM_BO                                                        0
#define AG_MG_REGS_TX_INTERRUPT_MASK_DLPEM_BM                                                        0x00000001

#define AG_MG_REGS_TX_INTERRUPT_MASK_RNRM_BO                                                        1
#define AG_MG_REGS_TX_INTERRUPT_MASK_RNRM_BM                                                        0x00000002

#define AG_MG_REGS_TX_INTERRUPT_MASK_RTTM_BO                                                        2
#define AG_MG_REGS_TX_INTERRUPT_MASK_RTTM_BM                                                        0x00000004

#define AG_MG_REGS_TX_INTERRUPT_MASK_U_BO                                                        3
#define AG_MG_REGS_TX_INTERRUPT_MASK_U_BM                                                        0x00000008

#define AG_MG_REGS_TX_INTERRUPT_MASK_EIPEM_BO                                                        4
#define AG_MG_REGS_TX_INTERRUPT_MASK_EIPEM_BM                                                        0x00000010

#define AG_MG_REGS_TX_INTERRUPT_MASK_ETPEM_BO                                                        5
#define AG_MG_REGS_TX_INTERRUPT_MASK_ETPEM_BM                                                        0x00000020

#define AG_MG_REGS_TX_INTERRUPT_MASK_RBPEM_BO                                                        6
#define AG_MG_REGS_TX_INTERRUPT_MASK_RBPEM_BM                                                        0x00000040

#define AG_MG_REGS_TX_INTERRUPT_MASK_SBPEM_BO                                                        7
#define AG_MG_REGS_TX_INTERRUPT_MASK_SBPEM_BM                                                        0x00000080

#define AG_MG_REGS_TX_INTERRUPT_MASK_RBOEM_BO                                                        8
#define AG_MG_REGS_TX_INTERRUPT_MASK_RBOEM_BM                                                        0x00000100

#define AG_MG_REGS_TX_INTERRUPT_MASK_RBUEM_BO                                                        9
#define AG_MG_REGS_TX_INTERRUPT_MASK_RBUEM_BM                                                        0x00000200

#define AG_MG_REGS_TX_INTERRUPT_MASK_SBOEM_BO                                                        10
#define AG_MG_REGS_TX_INTERRUPT_MASK_SBOEM_BM                                                        0x00000400

#define AG_MG_REGS_TX_INTERRUPT_MASK_SBUM_BO                                                        11
#define AG_MG_REGS_TX_INTERRUPT_MASK_SBUM_BM                                                        0x00000800

#define AG_MG_REGS_TX_INTERRUPT_MASK_TPEM_BO                                                        12
#define AG_MG_REGS_TX_INTERRUPT_MASK_TPEM_BM                                                        0x00001000

#define AG_MG_REGS_TX_INTERRUPT_MASK_R_BO                                                        13
#define AG_MG_REGS_TX_INTERRUPT_MASK_R_BM                                                        0x7FFFE000

#define AG_MG_REGS_TX_INTERRUPT_MASK_AUTM_BO                                                        31
#define AG_MG_REGS_TX_INTERRUPT_MASK_AUTM_BM                                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_INTERRUPT_MASK_U
{
    struct
    {
        ag_mg_regs_register
            dlpem : 1,
            rnrm : 1,
            rttm : 1,
            u : 1,
            eipem : 1,
            etpem : 1,
            rbpem : 1,
            sbpem : 1,
            rboem : 1,
            rbuem : 1,
            sboem : 1,
            sbum : 1,
            tpem : 1,
            r : 18,
            autm : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_interrupt_mask_u;
#endif


/* 
 * Initialization value: 0x109A1D4C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_RO                                               0x00003120
#define AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_RM                                               0xFFFFFFFF

#define AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_FLOWCONTROLUPDATETIMEOUT_VALUES_BO               0
#define AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_FLOWCONTROLUPDATETIMEOUT_VALUES_BM               0x0000FFFF

#define AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_INIT_FLOW_CONTROL_TIMEOUT_VALUE_BO               16
#define AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_INIT_FLOW_CONTROL_TIMEOUT_VALUE_BM               0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_U
{
    struct
    {
        ag_mg_regs_register
            flowcontrolupdatetimeout_values : 16,
            init_flow_control_timeout_value : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_flow_control_update_timeout_value_u;
#endif


/* 
 * Initialization value: 0x00000043  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_RO                                                       0x00003130
#define AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_RM                                                       0xFFFFFFFF

#define AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_ACK_NAK_LATENCY_THRESHOLD_BO                             0
#define AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_ACK_NAK_LATENCY_THRESHOLD_BM                             0x0000FFFF

#define AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_R_BO                                                     16
#define AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_R_BM                                                     0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_U
{
    struct
    {
        ag_mg_regs_register
            ack_nak_latency_threshold : 16,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ack_nak_latency_threshold_u;
#endif


/* 
 * Initialization value: 0x000000FC  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_RO                                                        0x00003134
#define AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_RTT_BO                                                      0
#define AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_RTT_BM                                                      0x000FFFFF

#define AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_R_BO                                                        20
#define AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_R_BM                                                        0xFFF00000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_U
{
    struct
    {
        ag_mg_regs_register
            rtt : 20,
            r : 12;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_replaytimeoutthreshold_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_REPLAY_NUMBER_STATUS_RO                                                        0x00003138
#define AG_MG_REGS_REPLAY_NUMBER_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_REPLAY_NUMBER_STATUS_RNC_BO                                                        0
#define AG_MG_REGS_REPLAY_NUMBER_STATUS_RNC_BM                                                        0x00000003

#define AG_MG_REGS_REPLAY_NUMBER_STATUS_R_BO                                                        2
#define AG_MG_REGS_REPLAY_NUMBER_STATUS_R_BM                                                        0xFFFFFFFC

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_REPLAY_NUMBER_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            rnc : 2,
            r : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_replay_number_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_RETRY_BUFFER_POINTER_RO                                                        0x00003140
#define AG_MG_REGS_RETRY_BUFFER_POINTER_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_RETRY_BUFFER_POINTER_RETRY_BUFFER_WRITE_POINTER_BO                                 0
#define AG_MG_REGS_RETRY_BUFFER_POINTER_RETRY_BUFFER_WRITE_POINTER_BM                                 0x0000FFFF

#define AG_MG_REGS_RETRY_BUFFER_POINTER_RETRY_BUFFER_READ_POINTER_BO                                  16
#define AG_MG_REGS_RETRY_BUFFER_POINTER_RETRY_BUFFER_READ_POINTER_BM                                  0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RETRY_BUFFER_POINTER_U
{
    struct
    {
        ag_mg_regs_register
            retry_buffer_write_pointer : 16,
            retry_buffer_read_pointer : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_retry_buffer_pointer_u;
#endif


/* 
 * Initialization value: 0x0FFF0000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SEQUENCE_COUNTER_RO                                                        0x00003144
#define AG_MG_REGS_SEQUENCE_COUNTER_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_SEQUENCE_COUNTER_NEXT_TRANSMIT_SEQUENCE_COUNTER_BO                                 0
#define AG_MG_REGS_SEQUENCE_COUNTER_NEXT_TRANSMIT_SEQUENCE_COUNTER_BM                                 0x00000FFF

#define AG_MG_REGS_SEQUENCE_COUNTER_R2_BO                                                        12
#define AG_MG_REGS_SEQUENCE_COUNTER_R2_BM                                                        0x0000F000

#define AG_MG_REGS_SEQUENCE_COUNTER_ACKD_SEQUENCE_COUNTER_BO                                          16
#define AG_MG_REGS_SEQUENCE_COUNTER_ACKD_SEQUENCE_COUNTER_BM                                          0x0FFF0000

#define AG_MG_REGS_SEQUENCE_COUNTER_R_BO                                                        28
#define AG_MG_REGS_SEQUENCE_COUNTER_R_BM                                                        0xF0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SEQUENCE_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            next_transmit_sequence_counter : 12,
            r2 : 4,
            ackd_sequence_counter : 12,
            r : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sequence_counter_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_RO                                                        0x00003148
#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_SEQUENCEBUFFERWRITEPOINTER_BO                             0
#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_SEQUENCEBUFFERWRITEPOINTER_BM                             0x00000FFF

#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_R2_BO                                                     12
#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_R2_BM                                                     0x0000F000

#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_SEQUENCEBUFFERREADPOINTER_BO                              16
#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_SEQUENCEBUFFERREADPOINTER_BM                              0x0FFF0000

#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_R_BO                                                      28
#define AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_R_BM                                                      0xF0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_U
{
    struct
    {
        ag_mg_regs_register
            sequencebufferwritepointer : 12,
            r2 : 4,
            sequencebufferreadpointer : 12,
            r : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_sequence_buffer_pointers_u;
#endif


/* 
 * Initialization value: 0x0000049C  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_SKIPTIMERTHRESHOLD_RO                                                        0x00003150
#define AG_MG_REGS_SKIPTIMERTHRESHOLD_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_SKIPTIMERTHRESHOLD_SKIPTIMERTHRESHOLDVALUE_BO                                      0
#define AG_MG_REGS_SKIPTIMERTHRESHOLD_SKIPTIMERTHRESHOLDVALUE_BM                                      0x00000FFF

#define AG_MG_REGS_SKIPTIMERTHRESHOLD_R_BO                                                        12
#define AG_MG_REGS_SKIPTIMERTHRESHOLD_R_BM                                                        0xFFFFF000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_SKIPTIMERTHRESHOLD_U
{
    struct
    {
        ag_mg_regs_register
            skiptimerthresholdvalue : 12,
            r : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_skiptimerthreshold_u;
#endif


/* 
 * Initialization value: 0x0000001F  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_EIES_COUNTER_THRESHOLD_RO                                                        0x00003154
#define AG_MG_REGS_EIES_COUNTER_THRESHOLD_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_EIES_COUNTER_THRESHOLD_EIES_COUNTER_THRESHOLD_BO                                   0
#define AG_MG_REGS_EIES_COUNTER_THRESHOLD_EIES_COUNTER_THRESHOLD_BM                                   0x0000FFFF

#define AG_MG_REGS_EIES_COUNTER_THRESHOLD_R_BO                                                        16
#define AG_MG_REGS_EIES_COUNTER_THRESHOLD_R_BM                                                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_EIES_COUNTER_THRESHOLD_U
{
    struct
    {
        ag_mg_regs_register
            eies_counter_threshold : 16,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_eies_counter_threshold_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_CONFIGURATION_RO                                                        0x00003200
#define AG_MG_REGS_RX_CONFIGURATION_RM                                                        0x00000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_CONFIGURATION_U
{
    struct
    {
        ag_mg_regs_register rx_configuration;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_configuration_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_RX_STATUS_RO                                                        0x00003204
#define AG_MG_REGS_RX_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_RX_STATUS_POLARITYREVERSAL_FOR_LANES_BO                                            0
#define AG_MG_REGS_RX_STATUS_POLARITYREVERSAL_FOR_LANES_BM                                            0x000000FF

#define AG_MG_REGS_RX_STATUS_ALS_BO                                                        8
#define AG_MG_REGS_RX_STATUS_ALS_BM                                                        0x00000100

#define AG_MG_REGS_RX_STATUS_R_BO                                                        9
#define AG_MG_REGS_RX_STATUS_R_BM                                                        0xFFFFFE00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            polarityreversal_for_lanes : 8,
            als : 1,
            r : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RO                                                        0x00003208
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RM                                                        0xFFFFFFF7

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_BTLP_BO                                                    0
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_BTLP_BM                                                    0x00000001

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_STLP_BO                                                    1
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_STLP_BM                                                    0x00000002

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RETT_BO                                                    2
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RETT_BM                                                    0x00000004

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_BLDL_BO                                                    4
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_BLDL_BM                                                    0x00000010

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_DLRE_BO                                                    5
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_DLRE_BM                                                    0x00000020

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_UDL_BO                                                     6
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_UDL_BM                                                     0x00000040

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_DLPE_BO                                                    7
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_DLPE_BM                                                    0x00000080

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_R_BO                                                       8
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_R_BM                                                       0x0003FF00

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RER_BO                                                     18
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RER_BM                                                     0x00040000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RX_PARITY_ERROR_BO                                         19
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RX_PARITY_ERROR_BM                                         0x00080000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_AFOE_BO                                                    20
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_AFOE_BM                                                    0x00100000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_AFUE_BO                                                    21
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_AFUE_BM                                                    0x00200000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_ALR_BO                                                     22
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_ALR_BM                                                     0x00400000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_PE_BO                                                      23
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_PE_BM                                                      0x00800000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_DE_8_BO                                                    24
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_DE_8_BM                                                    0x01000000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_EDO_BO                                                     25
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_EDO_BM                                                     0x02000000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_EBU_BO                                                     26
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_EBU_BM                                                     0x04000000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RDE_BO                                                     27
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RDE_BM                                                     0x08000000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_R2_BO                                                      28
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_R2_BM                                                      0x70000000

#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_AUR_BO                                                     31
#define AG_MG_REGS_RX_INTERRUPT_AND_STATUS_AUR_BM                                                     0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_INTERRUPT_AND_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            btlp : 1,
            stlp : 1,
            rett : 1,
            fill0 : 1,
            bldl : 1,
            dlre : 1,
            udl : 1,
            dlpe : 1,
            r : 10,
            rer : 1,
            rx_parity_error : 1,
            afoe : 1,
            afue : 1,
            alr : 1,
            pe : 1,
            de_8 : 1,
            edo : 1,
            ebu : 1,
            rde : 1,
            r2 : 3,
            aur : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_interrupt_and_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RO                                                        0x0000320C
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BTLP_BO                                                   0
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BTLP_BM                                                   0x00000001

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_STLP_BO                                                   1
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_STLP_BM                                                   0x00000002

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_TRET_BO                                                   2
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_TRET_BM                                                   0x00000004

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BLPT_BO                                                   4
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BLPT_BM                                                   0x00000010

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_DRET_BO                                                   5
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_DRET_BM                                                   0x00000020

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_UDLT_BO                                                   6
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_UDLT_BM                                                   0x00000040

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_DPET_BO                                                   7
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_DPET_BM                                                   0x00000080

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BR_BO                                                     8
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BR_BM                                                     0x0003FF00

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RERT_BO                                                   18
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RERT_BM                                                   0x00040000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RXPT_BO                                                   19
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RXPT_BM                                                   0x00080000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_AFOT_BO                                                   20
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_AFOT_BM                                                   0x00100000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_AFET_BO                                                   21
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_AFET_BM                                                   0x00200000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_ALRT_BO                                                   22
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_ALRT_BM                                                   0x00400000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_PET_BO                                                    23
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_PET_BM                                                    0x00800000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_DET8_BO                                                   24
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_DET8_BM                                                   0x01000000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_EBOT_BO                                                   25
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_EBOT_BM                                                   0x02000000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_EBUT_BO                                                   26
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_EBUT_BM                                                   0x04000000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RDET_BO                                                   27
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RDET_BM                                                   0x08000000

#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BR2_BO                                                    28
#define AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_BR2_BM                                                    0xF0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_U
{
    struct
    {
        ag_mg_regs_register
            btlp : 1,
            stlp : 1,
            tret : 1,
            fill0 : 1,
            blpt : 1,
            dret : 1,
            udlt : 1,
            dpet : 1,
            fill1 : 10,
            rert : 1,
            rxpt : 1,
            afot : 1,
            afet : 1,
            alrt : 1,
            pet : 1,
            det8 : 1,
            ebot : 1,
            ebut : 1,
            rdet : 1,
            fill2 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_interrupt_status_test_u;
#endif


/* 
 * Initialization value: 0x8FFC00FF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_INTERRUPT_MASK_RO                                                        0x00003210
#define AG_MG_REGS_RX_INTERRUPT_MASK_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_RX_INTERRUPT_MASK_BTLM_BO                                                        0
#define AG_MG_REGS_RX_INTERRUPT_MASK_BTLM_BM                                                        0x00000001

#define AG_MG_REGS_RX_INTERRUPT_MASK_STLM_BO                                                        1
#define AG_MG_REGS_RX_INTERRUPT_MASK_STLM_BM                                                        0x00000002

#define AG_MG_REGS_RX_INTERRUPT_MASK_TREM_BO                                                        2
#define AG_MG_REGS_RX_INTERRUPT_MASK_TREM_BM                                                        0x00000004

#define AG_MG_REGS_RX_INTERRUPT_MASK_UM_BO                                                        3
#define AG_MG_REGS_RX_INTERRUPT_MASK_UM_BM                                                        0x00000008

#define AG_MG_REGS_RX_INTERRUPT_MASK_BLPM_BO                                                        4
#define AG_MG_REGS_RX_INTERRUPT_MASK_BLPM_BM                                                        0x00000010

#define AG_MG_REGS_RX_INTERRUPT_MASK_DREM_BO                                                        5
#define AG_MG_REGS_RX_INTERRUPT_MASK_DREM_BM                                                        0x00000020

#define AG_MG_REGS_RX_INTERRUPT_MASK_UDLM_BO                                                        6
#define AG_MG_REGS_RX_INTERRUPT_MASK_UDLM_BM                                                        0x00000040

#define AG_MG_REGS_RX_INTERRUPT_MASK_DPEM_BO                                                        7
#define AG_MG_REGS_RX_INTERRUPT_MASK_DPEM_BM                                                        0x00000080

#define AG_MG_REGS_RX_INTERRUPT_MASK_R_BO                                                        8
#define AG_MG_REGS_RX_INTERRUPT_MASK_R_BM                                                        0x0003FF00

#define AG_MG_REGS_RX_INTERRUPT_MASK_RERM_BO                                                        18
#define AG_MG_REGS_RX_INTERRUPT_MASK_RERM_BM                                                        0x00040000

#define AG_MG_REGS_RX_INTERRUPT_MASK_RXPM_BO                                                        19
#define AG_MG_REGS_RX_INTERRUPT_MASK_RXPM_BM                                                        0x00080000

#define AG_MG_REGS_RX_INTERRUPT_MASK_AFOM_BO                                                        20
#define AG_MG_REGS_RX_INTERRUPT_MASK_AFOM_BM                                                        0x00100000

#define AG_MG_REGS_RX_INTERRUPT_MASK_AFEM_BO                                                        21
#define AG_MG_REGS_RX_INTERRUPT_MASK_AFEM_BM                                                        0x00200000

#define AG_MG_REGS_RX_INTERRUPT_MASK_ALRM_BO                                                        22
#define AG_MG_REGS_RX_INTERRUPT_MASK_ALRM_BM                                                        0x00400000

#define AG_MG_REGS_RX_INTERRUPT_MASK_PEM_BO                                                        23
#define AG_MG_REGS_RX_INTERRUPT_MASK_PEM_BM                                                        0x00800000

#define AG_MG_REGS_RX_INTERRUPT_MASK_DEM_8_BO                                                        24
#define AG_MG_REGS_RX_INTERRUPT_MASK_DEM_8_BM                                                        0x01000000

#define AG_MG_REGS_RX_INTERRUPT_MASK_EBOM_BO                                                        25
#define AG_MG_REGS_RX_INTERRUPT_MASK_EBOM_BM                                                        0x02000000

#define AG_MG_REGS_RX_INTERRUPT_MASK_EBUM_BO                                                        26
#define AG_MG_REGS_RX_INTERRUPT_MASK_EBUM_BM                                                        0x04000000

#define AG_MG_REGS_RX_INTERRUPT_MASK_RDEM_BO                                                        27
#define AG_MG_REGS_RX_INTERRUPT_MASK_RDEM_BM                                                        0x08000000

#define AG_MG_REGS_RX_INTERRUPT_MASK_R2_BO                                                        28
#define AG_MG_REGS_RX_INTERRUPT_MASK_R2_BM                                                        0x70000000

#define AG_MG_REGS_RX_INTERRUPT_MASK_AURA_BO                                                        31
#define AG_MG_REGS_RX_INTERRUPT_MASK_AURA_BM                                                        0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_INTERRUPT_MASK_U
{
    struct
    {
        ag_mg_regs_register
            btlm : 1,
            stlm : 1,
            trem : 1,
            um : 1,
            blpm : 1,
            drem : 1,
            udlm : 1,
            dpem : 1,
            r : 10,
            rerm : 1,
            rxpm : 1,
            afom : 1,
            afem : 1,
            alrm : 1,
            pem : 1,
            dem_8 : 1,
            ebom : 1,
            ebum : 1,
            rdem : 1,
            r2 : 3,
            aura : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_interrupt_mask_u;
#endif


/* 
 * Initialization value: 0x00020000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_RX_TS_CONTROL_RO                                                        0x00003214
#define AG_MG_REGS_RX_TS_CONTROL_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_RX_TS_CONTROL_RECEIVEDLINKNUMBER_BO                                                0
#define AG_MG_REGS_RX_TS_CONTROL_RECEIVEDLINKNUMBER_BM                                                0x000000FF

#define AG_MG_REGS_RX_TS_CONTROL_RECEIVEDFTS_BO                                                       8
#define AG_MG_REGS_RX_TS_CONTROL_RECEIVEDFTS_BM                                                       0x0000FF00

#define AG_MG_REGS_RX_TS_CONTROL_RECEIVEDDATARATE_BO                                                  16
#define AG_MG_REGS_RX_TS_CONTROL_RECEIVEDDATARATE_BM                                                  0x00FF0000

#define AG_MG_REGS_RX_TS_CONTROL_RHR_BO                                                        24
#define AG_MG_REGS_RX_TS_CONTROL_RHR_BM                                                        0x01000000

#define AG_MG_REGS_RX_TS_CONTROL_RDL_BO                                                        25
#define AG_MG_REGS_RX_TS_CONTROL_RDL_BM                                                        0x02000000

#define AG_MG_REGS_RX_TS_CONTROL_REL_BO                                                        26
#define AG_MG_REGS_RX_TS_CONTROL_REL_BM                                                        0x04000000

#define AG_MG_REGS_RX_TS_CONTROL_RDS_BO                                                        27
#define AG_MG_REGS_RX_TS_CONTROL_RDS_BM                                                        0x08000000

#define AG_MG_REGS_RX_TS_CONTROL_R_BO                                                        28
#define AG_MG_REGS_RX_TS_CONTROL_R_BM                                                        0xF0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_TS_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            receivedlinknumber : 8,
            receivedfts : 8,
            receiveddatarate : 8,
            rhr : 1,
            rdl : 1,
            rel : 1,
            rds : 1,
            r : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_ts_control_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_RO                                                       0x00003218
#define AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_RM                                                       0xFFFFFFFF

#define AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_NEXT_RCV_SEQ_BO                                          0
#define AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_NEXT_RCV_SEQ_BM                                          0x00000FFF

#define AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_R_BO                                                     12
#define AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_R_BM                                                     0xFFFFF000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_U
{
    struct
    {
        ag_mg_regs_register
            next_rcv_seq : 12,
            r : 20;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_next_rcv_sequence_counter_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_RO                                                        0x0000321C
#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE0_BO                                                     0
#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE0_BM                                                     0x000000FF

#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE1_BO                                                     8
#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE1_BM                                                     0x0000FF00

#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE2_BO                                                     16
#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE2_BM                                                     0x00FF0000

#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE3_BO                                                     24
#define AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_BYTE3_BM                                                     0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_U
{
    struct
    {
        ag_mg_regs_register
            byte0 : 8,
            byte1 : 8,
            byte2 : 8,
            byte3 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_unknowndllp0_received_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_RO                                                        0x00003220
#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE0_BO                                                    0
#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE0_BM                                                    0x000000FF

#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE1_BO                                                    8
#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE1_BM                                                    0x0000FF00

#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE2_BO                                                    16
#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE2_BM                                                    0x00FF0000

#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE3_BO                                                    24
#define AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_BYTE3_BM                                                    0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_U
{
    struct
    {
        ag_mg_regs_register
            byte0 : 8,
            byte1 : 8,
            byte2 : 8,
            byte3 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_unknown_dllp1_received_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_RO                                                        0x00003300
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_HDRS_BO                                                      0
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_HDRS_BM                                                      0x00000001

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_DISS_BO                                                      1
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_DISS_BM                                                      0x00000002

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_DSPC_BO                                                      2
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_DSPC_BM                                                      0x00000004

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_RDTM_BO                                                      3
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_RDTM_BM                                                      0x00000008

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LSDQ_BO                                                      4
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LSDQ_BM                                                      0x00000010

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LSHR_BO                                                      5
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LSHR_BM                                                      0x00000020

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LGSL_BO                                                      6
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LGSL_BM                                                      0x00000040

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LRVS_BO                                                      7
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_LRVS_BM                                                      0x00000080

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_PTMD_BO                                                      8
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_PTMD_BM                                                      0x00000100

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_WUE_BO                                                       9
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_WUE_BM                                                       0x00000200

#define AG_MG_REGS_LTSSM_CONFIGURATION_1_R_BO                                                        10
#define AG_MG_REGS_LTSSM_CONFIGURATION_1_R_BM                                                        0xFFFFFC00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_CONFIGURATION_1_U
{
    struct
    {
        ag_mg_regs_register
            hdrs : 1,
            diss : 1,
            dspc : 1,
            rdtm : 1,
            lsdq : 1,
            lshr : 1,
            lgsl : 1,
            lrvs : 1,
            ptmd : 1,
            wue : 1,
            r : 22;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_configuration_1_u;
#endif


/* 
 * Initialization value: 0x0F0F0003  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_STATUS_RO                                                        0x00003304
#define AG_MG_REGS_LTSSM_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_STATUS_CONFIG_LW_BO                                                        0
#define AG_MG_REGS_LTSSM_STATUS_CONFIG_LW_BM                                                        0x0000000F

#define AG_MG_REGS_LTSSM_STATUS_LTSSM_STATE_BO                                                        4
#define AG_MG_REGS_LTSSM_STATUS_LTSSM_STATE_BM                                                        0x000003F0

#define AG_MG_REGS_LTSSM_STATUS_LUDS_BO                                                        10
#define AG_MG_REGS_LTSSM_STATUS_LUDS_BM                                                        0x00000400

#define AG_MG_REGS_LTSSM_STATUS_LARV_BO                                                        11
#define AG_MG_REGS_LTSSM_STATUS_LARV_BM                                                        0x00000800

#define AG_MG_REGS_LTSSM_STATUS_LOTX_BO                                                        12
#define AG_MG_REGS_LTSSM_STATUS_LOTX_BM                                                        0x00001000

#define AG_MG_REGS_LTSSM_STATUS_LORX_BO                                                        13
#define AG_MG_REGS_LTSSM_STATUS_LORX_BM                                                        0x00002000

#define AG_MG_REGS_LTSSM_STATUS_MALS_BO                                                        14
#define AG_MG_REGS_LTSSM_STATUS_MALS_BM                                                        0x00004000

#define AG_MG_REGS_LTSSM_STATUS_RALC_BO                                                        15
#define AG_MG_REGS_LTSSM_STATUS_RALC_BM                                                        0x00008000

#define AG_MG_REGS_LTSSM_STATUS_RECEIVE_LANE_ENABLE_MASK_BO                                           16
#define AG_MG_REGS_LTSSM_STATUS_RECEIVE_LANE_ENABLE_MASK_BM                                           0x00FF0000

#define AG_MG_REGS_LTSSM_STATUS_RECEIVED_ELECTRICAL_IDLE_BO                                           24
#define AG_MG_REGS_LTSSM_STATUS_RECEIVED_ELECTRICAL_IDLE_BM                                           0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            config_lw : 4,
            ltssm_state : 6,
            luds : 1,
            larv : 1,
            lotx : 1,
            lorx : 1,
            mals : 1,
            ralc : 1,
            receive_lane_enable_mask : 8,
            received_electrical_idle : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_RO                                                       0x00003308
#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_RM                                                       0xFFFFFFFF

#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_TRER_BO                                                  0
#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_TRER_BM                                                  0x00000001

#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_R1_BO                                                    1
#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_R1_BM                                                    0x7FFFFFFE

#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_AUIA_BO                                                  31
#define AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_AUIA_BM                                                  0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            trer : 1,
            r1 : 30,
            auia : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_interupt_and_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_INTERRUPT_TEST_RO                                                        0x0000330C
#define AG_MG_REGS_LTSSM_INTERRUPT_TEST_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_INTERRUPT_TEST_TRET_BO                                                       0
#define AG_MG_REGS_LTSSM_INTERRUPT_TEST_TRET_BM                                                       0x00000001

#define AG_MG_REGS_LTSSM_INTERRUPT_TEST_R_BO                                                        1
#define AG_MG_REGS_LTSSM_INTERRUPT_TEST_R_BM                                                        0xFFFFFFFE

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_INTERRUPT_TEST_U
{
    struct
    {
        ag_mg_regs_register
            tret : 1,
            r : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_interrupt_test_u;
#endif


/* 
 * Initialization value: 0x80000001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_RO                                                        0x00003310
#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_TREM_BO                                                       0
#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_TREM_BM                                                       0x00000001

#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_R_BO                                                        1
#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_R_BM                                                        0x7FFFFFFE

#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_AUIM_BO                                                       31
#define AG_MG_REGS_LTSSM_INTERRUPT_MASK_AUIM_BM                                                       0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_INTERRUPT_MASK_U
{
    struct
    {
        ag_mg_regs_register
            trem : 1,
            r : 30,
            auim : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_interrupt_mask_u;
#endif


/* 
 * Initialization value: 0x00000519  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_RO                                                        0x00003320
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_LTSSM_20NS_TIMEOUT_VALUE_BO                                 0
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_LTSSM_20NS_TIMEOUT_VALUE_BM                                 0x000000FF

#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_LTSSM_8NS_TIMEOUT_VALUE_BO                                  8
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_LTSSM_8NS_TIMEOUT_VALUE_BM                                  0x0000FF00

#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_R_BO                                                        16
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_R_BM                                                        0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_U
{
    struct
    {
        ag_mg_regs_register
            ltssm_20ns_timeout_value : 8,
            ltssm_8ns_timeout_value : 8,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_timer_threshold1_u;
#endif


/* 
 * Initialization value: 0x002DC6C0  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD2_RO                                                        0x00003324
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD2_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD2_TIMER_12MS_BO                                               0
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD2_TIMER_12MS_BM                                               0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_TIMER_THRESHOLD2_U
{
    struct
    {
        ag_mg_regs_register
            timer_12ms;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_timer_threshold2_u;
#endif


/* 
 * Initialization value: 0x0007A120  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD3_RO                                                        0x00003328
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD3_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD3_TIMER_VALUE_2MS_BO                                         0
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD3_TIMER_VALUE_2MS_BM                                         0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_TIMER_THRESHOLD3_U
{
    struct
    {
        ag_mg_regs_register
            timer_value_2ms;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_timer_threshold3_u;
#endif


/* 
 * Initialization value: 0x0001E848  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD4_RO                                                        0x0000332C
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD4_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD4_LTSSM_500US_TIMEOUT_VALUE_BO                                     0
#define AG_MG_REGS_LTSSM_TIMER_THRESHOLD4_LTSSM_500US_TIMEOUT_VALUE_BM                                     0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_TIMER_THRESHOLD4_U
{
    struct
    {
        ag_mg_regs_register
            ltssm_500us_timeout_value;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_threshold4_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_REQUEST_RO                                                        0x00003330
#define AG_MG_REGS_LTSSM_REQUEST_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_REQUEST_HRR_BO                                                        0
#define AG_MG_REGS_LTSSM_REQUEST_HRR_BM                                                        0x00000001

#define AG_MG_REGS_LTSSM_REQUEST_GDQ_BO                                                        1
#define AG_MG_REGS_LTSSM_REQUEST_GDQ_BM                                                        0x00000002

#define AG_MG_REGS_LTSSM_REQUEST_RCC_BO                                                        2
#define AG_MG_REGS_LTSSM_REQUEST_RCC_BM                                                        0x00000004

#define AG_MG_REGS_LTSSM_REQUEST_LRR_BO                                                        3
#define AG_MG_REGS_LTSSM_REQUEST_LRR_BM                                                        0x00000008

#define AG_MG_REGS_LTSSM_REQUEST_R0_BO                                                        4
#define AG_MG_REGS_LTSSM_REQUEST_R0_BM                                                        0x00000030

#define AG_MG_REGS_LTSSM_REQUEST_CHW_BO                                                        6
#define AG_MG_REGS_LTSSM_REQUEST_CHW_BM                                                        0x00000040

#define AG_MG_REGS_LTSSM_REQUEST_R1_BO                                                        7
#define AG_MG_REGS_LTSSM_REQUEST_R1_BM                                                        0x00000080

#define AG_MG_REGS_LTSSM_REQUEST_TLW_BO                                                        8
#define AG_MG_REGS_LTSSM_REQUEST_TLW_BM                                                        0x00000700

#define AG_MG_REGS_LTSSM_REQUEST_R2_BO                                                        11
#define AG_MG_REGS_LTSSM_REQUEST_R2_BM                                                        0xFFFFF800

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_REQUEST_U
{
    struct
    {
        ag_mg_regs_register
            hrr : 1,
            gdq : 1,
            rcc : 1,
            lrr : 1,
            r0 : 2,
            chw : 1,
            r1 : 1,
            tlw : 3,
            r2 : 21;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_request_u;
#endif


/* 
 * Initialization value: 0x00069C00  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_RO                                                    0x00003334
#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_RM                                                    0xFFFFFFFF

#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_LINK_NUMBER_BO                                        0
#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_LINK_NUMBER_BM                                        0x000000FF

#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_TRANSMIT_FTS_NUMBER_BO                                8
#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_TRANSMIT_FTS_NUMBER_BM                                0x0000FF00

#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_TRANSMIT_DATA_RATE_BO                                 16
#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_TRANSMIT_DATA_RATE_BM                                 0x00FF0000

#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_TRANSMIT_TRAINING_CONTROL_BO                          24
#define AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_TRANSMIT_TRAINING_CONTROL_BM                          0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_U
{
    struct
    {
        ag_mg_regs_register
            link_number : 8,
            transmit_fts_number : 8,
            transmit_data_rate : 8,
            transmit_training_control : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_training_configuration_u;
#endif


/* 
 * Initialization value: 0x00001080  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LTSSM_STATUS_2_RO                                                        0x00003338
#define AG_MG_REGS_LTSSM_STATUS_2_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_STATUS_2_BPRMR_BO                                                        0
#define AG_MG_REGS_LTSSM_STATUS_2_BPRMR_BM                                                        0x00000007

#define AG_MG_REGS_LTSSM_STATUS_2_R0_BO                                                        3
#define AG_MG_REGS_LTSSM_STATUS_2_R0_BM                                                        0x00000008

#define AG_MG_REGS_LTSSM_STATUS_2_PRMS_BO                                                        4
#define AG_MG_REGS_LTSSM_STATUS_2_PRMS_BM                                                        0x00000070

#define AG_MG_REGS_LTSSM_STATUS_2_LRC_BO                                                        7
#define AG_MG_REGS_LTSSM_STATUS_2_LRC_BM                                                        0x00000080

#define AG_MG_REGS_LTSSM_STATUS_2_R1_BO                                                        8
#define AG_MG_REGS_LTSSM_STATUS_2_R1_BM                                                        0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_STATUS_2_U
{
    struct
    {
        ag_mg_regs_register
            bprmr : 3,
            r0 : 1,
            prms : 3,
            lrc : 1,
            r1 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_status_2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_RO                                                        0x0000333C
#define AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_RECEIVE_COMMAND_TO_RECEIVE_PHY_BO                          0
#define AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_RECEIVE_COMMAND_TO_RECEIVE_PHY_BM                          0x0000FFFF

#define AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_R_BO                                                       16
#define AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_R_BM                                                       0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            receive_command_to_receive_phy : 16,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_rx_command_status_u;
#endif


/* 
 * Initialization value: 0x00000040  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_RO                                                        0x00003340
#define AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_TRANSMIT_COMMAND_TO_TRANSMIT_PHY_BO                        0
#define AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_TRANSMIT_COMMAND_TO_TRANSMIT_PHY_BM                        0x00000001

#define AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_R_BO                                                       1
#define AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_R_BM                                                       0xFFFFFFFE

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            transmit_command_to_transmit_phy : 1,
            r : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_tx_command_status_u;
#endif


/* 
 * Initialization value: 0x05DC7D00  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_RO                                                   0x00003350
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_RM                                                   0xFFFFFFFF

#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_LTSSM_128US_TIMEOUT_VALUE_BO                         0
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_LTSSM_128US_TIMEOUT_VALUE_BM                         0x0000FFFF

#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_LTSSM_6US_TIMEOUT_VALUE_BO                           16
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_LTSSM_6US_TIMEOUT_VALUE_BM                           0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_U
{
    struct
    {
        ag_mg_regs_register
            ltssm_128us_timeout_value : 16,
            ltssm_6us_timeout_value : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_gen_2_timer_threshold1_u;
#endif


/* 
 * Initialization value: 0x0000C880  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_RO                                                     0x00003354
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_RM                                                     0xFFFFFFFF

#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_LTSSM_1280_UI_TIMEOUT_VALUE_BO                         0
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_LTSSM_1280_UI_TIMEOUT_VALUE_BM                         0x000000FF

#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_LTSSM_2000_UI_TIMEOUT_VALUE_BO                         8
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_LTSSM_2000_UI_TIMEOUT_VALUE_BM                         0x0000FF00

#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_R_BO                                                   16
#define AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_R_BM                                                   0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_U
{
    struct
    {
        ag_mg_regs_register
            ltssm_1280_ui_timeout_value : 8,
            ltssm_2000_ui_timeout_value : 8,
            r : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_lssm_gen_2_timer_threshold2_u;
#endif


/* 
 * Initialization value: 0x000000FC  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_RO                                                    0x00003358
#define AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_RM                                                    0xFFFFFFFF

#define AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_GEN2_SPEED_NFTS_NUMBER_BO                             0
#define AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_GEN2_SPEED_NFTS_NUMBER_BM                             0x000000FF

#define AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_R_BO                                                  8
#define AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_R_BM                                                  0xFFFFFF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_U
{
    struct
    {
        ag_mg_regs_register
            gen2_speed_nfts_number : 8,
            r : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_ltssm_gen2_speed_nfts_number_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TL_GEN_DEBUG_REG_RO                                                        0x00003E00
#define AG_MG_REGS_TL_GEN_DEBUG_REG_RM                                                        0xFFFFFFFF

#define AG_MG_REGS_TL_GEN_DEBUG_REG_TGEN_DBG_BO                                                       0
#define AG_MG_REGS_TL_GEN_DEBUG_REG_TGEN_DBG_BM                                                       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TL_GEN_DEBUG_REG_U
{
    struct
    {
        ag_mg_regs_register
            tgen_dbg;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tl_gen_debug_reg_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TLSB_SIG_RO                                                        0x00003E04
#define AG_MG_REGS_TLSB_SIG_RM                                                        0x0000FFF8

#define AG_MG_REGS_TLSB_SIG_DEVICE_NUMBER_BO                                                        3
#define AG_MG_REGS_TLSB_SIG_DEVICE_NUMBER_BM                                                        0x000000F8

#define AG_MG_REGS_TLSB_SIG_BUS_NUMBER_BO                                                        8
#define AG_MG_REGS_TLSB_SIG_BUS_NUMBER_BM                                                        0x0000FF00

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TLSB_SIG_U
{
    struct
    {
        ag_mg_regs_register
            fill1 : 3,
            device_number : 5,
            bus_number : 8,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tlsb_sig_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_RO                                                   0x00003E08
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_RM                                                   0xFFFFFFFF

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_0_TIME_OUT_LIMIT_BO                         0
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_0_TIME_OUT_LIMIT_BM                         0x000000FF

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_1_TIME_OUT_LIMIT_BO                         8
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_1_TIME_OUT_LIMIT_BM                         0x0000FF00

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_2_TIME_OUT_LIMIT_BO                         16
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_2_TIME_OUT_LIMIT_BM                         0x00FF0000

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_3_TIME_OUT_LIMIT_BO                         24
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_FUNCTION_3_TIME_OUT_LIMIT_BM                         0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_U
{
    struct
    {
        ag_mg_regs_register
            function_0_time_out_limit : 8,
            function_1_time_out_limit : 8,
            function_2_time_out_limit : 8,
            function_3_time_out_limit : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_completion_time_out_register0_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_RO                                                   0x00003E0C
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_RM                                                   0xFFFFFFFF

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_4_TIME_OUT_LIMIT_BO                         0
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_4_TIME_OUT_LIMIT_BM                         0x000000FF

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_5_TIME_OUT_LIMIT_BO                         8
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_5_TIME_OUT_LIMIT_BM                         0x0000FF00

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_6_TIME_OUT_LIMIT_BO                         16
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_6_TIME_OUT_LIMIT_BM                         0x00FF0000

#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_7_TIME_OUT_LIMIT_BO                         24
#define AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_FUNCTION_7_TIME_OUT_LIMIT_BM                         0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_U
{
    struct
    {
        ag_mg_regs_register
            function_4_time_out_limit : 8,
            function_5_time_out_limit : 8,
            function_6_time_out_limit : 8,
            function_7_time_out_limit : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_completion_time_out_register1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_RO                                                      0x00003E40
#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_RM                                                      0x0000000F

#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_PD_ENABLE_BO                                            0
#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_PD_ENABLE_BM                                            0x00000001

#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_PH_ENABLE_BO                                            1
#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_PH_ENABLE_BM                                            0x00000002

#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_NPD_ENABLE_BO                                           2
#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_NPD_ENABLE_BM                                           0x00000004

#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_NPH_ENABLE_BO                                           3
#define AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_NPH_ENABLE_BM                                           0x00000008

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_U
{
    struct
    {
        ag_mg_regs_register
            pd_enable : 1,
            ph_enable : 1,
            npd_enable : 1,
            nph_enable : 1,
            fill0 : 28;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_ingress_fc_control_reg_u;
#endif


/* 
 * Initialization value: 0x0FFF0001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_RO                                          0x00003E44
#define AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_RM                                          0x0FFF00FF

#define AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_MAX_HEADER_CREDITS_FREED_BO                 0
#define AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_MAX_HEADER_CREDITS_FREED_BM                 0x000000FF

#define AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_MAXIMUM_DATA_CREDITS_FREED_BO               16
#define AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_MAXIMUM_DATA_CREDITS_FREED_BM               0x0FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_U
{
    struct
    {
        ag_mg_regs_register
            max_header_credits_freed : 8,
            fill1 : 8, 
            maximum_data_credits_freed : 12,
            fill0 : 4; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_posted_prog_credit_freed_threshold_u;
#endif


/* 
 * Initialization value: 0x0FFF0001  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_RO                               0x00003E48
#define AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_RM                               0x0FFF00FF

#define AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_MAX_HEADER_CREDITS_FREED_BO      0
#define AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_MAX_HEADER_CREDITS_FREED_BM      0x000000FF

#define AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_MAXIMUM_DATA_CREDITS_FREED_BO    16
#define AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_MAXIMUM_DATA_CREDITS_FREED_BM    0x0FFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_U
{
    struct
    {
        ag_mg_regs_register
            max_header_credits_freed : 8,
            fill1 : 8, 
            maximum_data_credits_freed : 12,
            fill0 : 4; 
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_nonposted_programmable_credit_freed_threshold_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_RO                                          0x00003E4C
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_RM                                          0xFFFF00FF

#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_HEADER_CREDITS_RECEIVED_BO                  0
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_HEADER_CREDITS_RECEIVED_BM                  0x000000FF

#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_DATA_RECEIVED_CREDITS_BO                    16
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_DATA_RECEIVED_CREDITS_BM                    0x00FF0000

#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_RECEIVED1_BO                                24
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_RECEIVED1_BM                                0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_U
{
    struct
    {
        ag_mg_regs_register
            header_credits_received : 8,
            fill0 : 8, 
            data_received_credits : 8,
            received1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_ingress_fc_posted_credits_received_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_RO                                       0x00003E50
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_RM                                       0xFFFF00FF

#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_HEADER_CREDITS_RECEIVED_BO               0
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_HEADER_CREDITS_RECEIVED_BM               0x000000FF

#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_DATA_RECEIVED_CREDITS_BO                 16
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_DATA_RECEIVED_CREDITS_BM                 0x00FF0000

#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_RECEIVED1_BO                             24
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_RECEIVED1_BM                             0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_U
{
    struct
    {
        ag_mg_regs_register
            header_credits_received : 8,
            fill0 : 8, 
            data_received_credits : 8,
            received1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_ingress_fc_nonposted_credits_received_u;
#endif


/* 
 * Initialization value: 0x00400008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_RO                                         0x00003E54
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_RM                                         0xFFFF00FF

#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_HEADER_CREDITS_ALLOCATED_BO                0
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_HEADER_CREDITS_ALLOCATED_BM                0x000000FF

#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_DATA_CREDITS_ALLOCATED_BO                  16
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_DATA_CREDITS_ALLOCATED_BM                  0x00FF0000

#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_RECEIVED1_BO                               24
#define AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_RECEIVED1_BM                               0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_U
{
    struct
    {
        ag_mg_regs_register
            header_credits_allocated : 8,
            fill0 : 8, 
            data_credits_allocated : 8,
            received1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_ingress_fc_posted_credits_allocated_u;
#endif


/* 
 * Initialization value: 0x00080008  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_RO                                      0x00003E58
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_RM                                      0xFFFF00FF

#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_HEADER_CREDITS_RECEIVED_BO              0
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_HEADER_CREDITS_RECEIVED_BM              0x000000FF

#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_DATA_CREDITS_RECEIVED_BO                16
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_DATA_CREDITS_RECEIVED_BM                0x00FF0000

#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_RECEIVED1_BO                            24
#define AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_RECEIVED1_BM                            0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_U
{
    struct
    {
        ag_mg_regs_register
            header_credits_received : 8,
            fill0 : 8, 
            data_credits_received : 8,
            received1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_ingress_fc_nonposted_credits_allocated_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RO                                              0x00003E80
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RM                                              0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_POSTED_HEADER_CREDITS_CONSUMED_BO               0
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_POSTED_HEADER_CREDITS_CONSUMED_BM               0x000000FF

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RVSD0_BO                                        8
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RVSD0_BM                                        0x0000FF00

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_POSTED_DATA_CREDITS_CONSUMED_BO                 16
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_POSTED_DATA_CREDITS_CONSUMED_BM                 0x00FF0000

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RSVD1_BO                                        24
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RSVD1_BM                                        0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_U
{
    struct
    {
        ag_mg_regs_register
            posted_header_credits_consumed : 8,
            rvsd0 : 8,
            posted_data_credits_consumed : 8,
            rsvd1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_posted_credits_consumed_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RO                                           0x00003E84
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RM                                           0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_NONPOSTED_HEADER_CREDITS_CONSUMED_BO            0
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_NONPOSTED_HEADER_CREDITS_CONSUMED_BM            0x000000FF

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RVSD0_BO                                     8
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RVSD0_BM                                     0x0000FF00

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_NONPOSTED_DATA_CREDITS_CONSUMED_BO              16
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_NONPOSTED_DATA_CREDITS_CONSUMED_BM              0x00FF0000

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RSVD1_BO                                     24
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RSVD1_BM                                     0xFF000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_U
{
    struct
    {
        ag_mg_regs_register
            nonposted_header_credits_consumed : 8,
            rvsd0 : 8,
            nonposted_data_credits_consumed : 8,
            rsvd1 : 8;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_nonposted_credits_consumed_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RO                                          0x00003E88
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RM                                          0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_COMPLETION_HEADER_CREDITS_CONSUMED_BO       0
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_COMPLETION_HEADER_CREDITS_CONSUMED_BM       0x000000FF

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RSVD0_BO                                    8
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RSVD0_BM                                    0x0000FF00

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_COMPLETION_DATA_CREDITS_CONSUMED_BO         16
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_COMPLETION_DATA_CREDITS_CONSUMED_BM         0x0FFF0000

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RSVD1_BO                                    28
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RSVD1_BM                                    0xF0000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_U
{
    struct
    {
        ag_mg_regs_register
            completion_header_credits_consumed : 8,
            rsvd0 : 8,
            completion_data_credits_consumed : 12,
            rsvd1 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_completion_credits_consumed_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_RO                                         0x00003E8C
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_RM                                         0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_HEADER_CREDIT_LIMIT_BO              0
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_HEADER_CREDIT_LIMIT_BM              0x000000FF

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_RSVD0_BO                                   8
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_RSVD0_BM                                   0x00007F00

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_HEADER_CREDIT_LIMIT_INFINITE_BO     15
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_HEADER_CREDIT_LIMIT_INFINITE_BM     0x00008000

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_DATA_CREDIT_LIMIT_BO               16
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_DATA_CREDIT_LIMIT_BM               0x0FFF0000

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_RSVD1_BO                                   28
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_RSVD1_BM                                   0x70000000

#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_DATA_CREDIT_LIMIT_INFINITE_BO       31
#define AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_POSTED_DATA_CREDIT_LIMIT_INFINITE_BM       0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_U
{
    struct
    {
        ag_mg_regs_register
            posted_header_credit_limit : 8,
            rsvd0 : 7,
            posted_header_credit_limit_infinite : 1,
            posted_data_credit_limit : 12,
            rsvd1 : 3,
            posted_data_credit_limit_infinite : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_posted_credit_limit_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_RO                                                      0x00003E90
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_RM                                                      0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_HEADER_CREDIT_LIMIT_BO                      0
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_HEADER_CREDIT_LIMIT_BM                      0x000000FF

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_RSVD0_BO                                                8
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_RSVD0_BM                                                0x00007F00

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_HEADER_CREDIT_LIMIT_INFINITE_BO               15
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_HEADER_CREDIT_LIMIT_INFINITE_BM               0x00008000

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_DATA_CREDIT_LIMIT_BO                          16
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_DATA_CREDIT_LIMIT_BM                          0x00FF0000

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_RSVD1_BO                                                24
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_RSVD1_BM                                                0x7F000000

#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_DATA_CREDIT_LIMIT_INFINITE_BO                 31
#define AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_NONPOSTED_DATA_CREDIT_LIMIT_INFINITE_BM                 0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDIT_LIMIT_U
{
    struct
    {
        ag_mg_regs_register
            nonposted_header_credit_limit : 8,
            rsvd0 : 7,
            nonposted_header_credit_limit_infinite : 1,
            nonposted_data_credit_limit : 8,
            rsvd1 : 7,
            nonposted_data_credit_limit_infinite : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_nonposted_limit_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RO                                                     0x00003E94
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RM                                                     0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_COMPLETION_HEADER_CREDITS_LIMITS_BO                    0
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_COMPLETION_HEADER_CREDITS_LIMITS_BM                    0x000000FF

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RSVD0_BO                                               8
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RSVD0_BM                                               0x00007F00

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_NONPOSTED_HEADER_CREDIT_LIMIT_INFINITE_BO              15
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_NONPOSTED_HEADER_CREDIT_LIMIT_INFINITE_BM              0x00008000

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_COMPLETION_HEADER_CREDITS_LIMIT_INFINITE_BO            16
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_COMPLETION_HEADER_CREDITS_LIMIT_INFINITE_BM            0x00FF0000

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RSVD1_BO                                               24
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RSVD1_BM                                               0x7F000000

#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_COMPLETION_DATA_CREDIT_LIMIT_INFINITE_BO               31
#define AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_COMPLETION_DATA_CREDIT_LIMIT_INFINITE_BM               0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_U
{
    struct
    {
        ag_mg_regs_register
            completion_header_credits_limits : 8,
            rsvd0 : 7,
            nonposted_header_credit_limit_infinite : 1,
            completion_header_credits_limit_infinite : 8,
            rsvd1 : 7,
            completion_data_credit_limit_infinite : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_completion_limit_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_RO                                           0x00003E98
#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_RM                                           0xFFFFFFFF

#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_BO         0
#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_BM         0x0000FFFF

#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_RVSD0_BO                                     16
#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_RVSD0_BM                                     0x7FFF0000

#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_EGRESS_FLOW_CONTROL_TIMEOUT_DETECTED_BO      31
#define AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_EGRESS_FLOW_CONTROL_TIMEOUT_DETECTED_BM      0x80000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_VC0_EGRESS_FLOW_CONROL_TIMEOUT_TIMER_U
{
    struct
    {
        ag_mg_regs_register
            timeout_timer : 16,
            rsvd0 : 15,
            timeout_detected : 1;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_vc0_egress_flow_control_timeout_timer_u;
#endif




/* PIPE Layer section */

/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PIPE_RST_EN_RO                           0x00003400
#define AG_MG_REGS_PIPE_RST_EN_RM                           0x000000FF

#define AG_MG_REGS_PIPE_RST_EN_PPE_RSTN_EN_BO               0
#define AG_MG_REGS_PIPE_RST_EN_PPE_RSTN_EN_BM               0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PIPE_RST_EN_U
{
    struct
    {
        ag_mg_regs_register
            ppe_rstn_en : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pipe_rst_en_u;
#endif


/* 
 * Initialization value: 0xFA000006  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_PIPE_CONTROL_RO                          0x00003404
#define AG_MG_REGS_PIPE_CONTROL_RM                          0xFFFF000F

#define AG_MG_REGS_PIPE_CONTROL_WMLO_SEL_BO                 0
#define AG_MG_REGS_PIPE_CONTROL_WMLO_SEL_BM                 0x00000001

#define AG_MG_REGS_PIPE_CONTROL_WMSELECT_BO                 1
#define AG_MG_REGS_PIPE_CONTROL_WMSELECT_BM                 0x0000000E

#define AG_MG_REGS_PIPE_CONTROL_RXSYNC_TOV_BO               16
#define AG_MG_REGS_PIPE_CONTROL_RXSYNC_TOV_BM               0xFFFF0000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PIPE_CONTROL_U
{
    struct
    {
        ag_mg_regs_register
            wmlo_sel : 1,
            wmselect : 3,
            fill0 : 12,
            rxsync_tov : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pipe_control_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PIPE_COMMON_STATUS_RO                    0x00003408
#define AG_MG_REGS_PIPE_COMMON_STATUS_RM                    0x000001FF

#define AG_MG_REGS_PIPE_COMMON_STATUS_PD_P2_R_BO            0
#define AG_MG_REGS_PIPE_COMMON_STATUS_PD_P2_R_BM            0x00000001

#define AG_MG_REGS_PIPE_COMMON_STATUS_PD_P2_T_BO            1
#define AG_MG_REGS_PIPE_COMMON_STATUS_PD_P2_T_BM            0x00000002

#define AG_MG_REGS_PIPE_COMMON_STATUS_PD_SYS_CUR_BO         2
#define AG_MG_REGS_PIPE_COMMON_STATUS_PD_SYS_CUR_BM         0x0000000C

#define AG_MG_REGS_PIPE_COMMON_STATUS_RATE_O_BO             4
#define AG_MG_REGS_PIPE_COMMON_STATUS_RATE_O_BM             0x00000010

#define AG_MG_REGS_PIPE_COMMON_STATUS_MTC_ISOLATE_BO        5
#define AG_MG_REGS_PIPE_COMMON_STATUS_MTC_ISOLATE_BM        0x00000020

#define AG_MG_REGS_PIPE_COMMON_STATUS_MTC_SLEEP_RDY_BO      6
#define AG_MG_REGS_PIPE_COMMON_STATUS_MTC_SLEEP_RDY_BM      0x00000040

#define AG_MG_REGS_PIPE_COMMON_STATUS_MTC_EN_BO             7
#define AG_MG_REGS_PIPE_COMMON_STATUS_MTC_EN_BM             0x00000080

#define AG_MG_REGS_PIPE_COMMON_STATUS_PHY_CONF_DONE_BO      8
#define AG_MG_REGS_PIPE_COMMON_STATUS_PHY_CONF_DONE_BM      0x00000100

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PIPE_COMMON_STATUS_U
{
    struct
    {
        ag_mg_regs_register
            pd_p2_r : 1,
            pd_p2_t : 1,
            pd_sys_cur : 2,
            rate_o : 1,
            mtc_isolate : 1,
            mtc_sleep_rdy : 1,
            mtc_en : 1,
            phy_conf_done : 1,
            fill0 : 23;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pipe_common_status_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_RO                      0x0000340C
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_RM                      0x3DDF3DDF

#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_C0_G1_1_BO           0
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_C0_G1_1_BM           0x0000001F

#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CM1_G1_1_BO          6
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CM1_G1_1_BM          0x000001C0

#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CP1_G1_1_BO          10
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CP1_G1_1_BM          0x00003C00

#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_C0_G1_2_BO           16
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_C0_G1_2_BM           0x001F0000

#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CM1_G1_2_BO          22
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CM1_G1_2_BM          0x01C00000

#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CP1_G1_2_BO          26
#define AG_MG_REGS_TX_EMPH_CNTRL_G1_TX_CP1_G1_2_BM          0x3C000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_EMPH_CNTRL_G1_U
{
    struct
    {
        ag_mg_regs_register
            tx_c0_g1_1 : 5,
            fill5 : 1,
            tx_cm1_g1_1 : 3,
            fill4 : 1,
            tx_cp1_g1_1 : 4,
            fill3 : 2,
            tx_c0_g1_2 : 5,
            fill2 : 1,
            tx_cm1_g1_2 : 3,
            fill1 : 1,
            tx_cp1_g1_2 : 4,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_emph_cntrl_g1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_RO                      0x00003410
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_RM                      0x3DDF3DDF

#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_C0_G2_1_BO           0
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_C0_G2_1_BM           0x0000001F

#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CM1_G2_1_BO          6
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CM1_G2_1_BM          0x000001C0

#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CP1_G2_1_BO          10
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CP1_G2_1_BM          0x00003C00

#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_C0_G2_2_BO           16
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_C0_G2_2_BM           0x001F0000

#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CM1_G2_2_BO          22
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CM1_G2_2_BM          0x01C00000

#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CP1_G2_2_BO          26
#define AG_MG_REGS_TX_EMPH_CNTRL_G2_TX_CP1_G2_2_BM          0x3C000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_EMPH_CNTRL_G2_U
{
    struct
    {
        ag_mg_regs_register
            tx_c0_g2_1 : 5,
            fill5 : 1,
            tx_cm1_g2_1 : 3,
            fill4 : 1,
            tx_cp1_g2_1 : 4,
            fill3 : 2,
            tx_c0_g2_2 : 5,
            fill2 : 1,
            tx_cm1_g2_2 : 3,
            fill1 : 1,
            tx_cp1_g2_2 : 4,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_emph_cntrl_g2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_RO                    0x00003414
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_RM                    0x3DDF3DDF

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_C0_G3_1_BO         0
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_C0_G3_1_BM         0x0000001F

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CM1_G3_1_BO        6
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CM1_G3_1_BM        0x000001C0

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CP1_G3_1_BO        10
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CP1_G3_1_BM        0x00003C00

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_C0_G3_2_BO         16
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_C0_G3_2_BM         0x001F0000

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CM1_G3_2_BO        22
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CM1_G3_2_BM        0x01C00000

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CP1_G3_2_BO        26
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_1_TX_CP1_G3_2_BM        0x3C000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_EMPH_CNTRL_G3_1_U
{
    struct
    {
        ag_mg_regs_register
            tx_c0_g3_1 : 5,
            fill5 : 1,
            tx_cm1_g3_1 : 3,
            fill4 : 1,
            tx_cp1_g3_1 : 4,
            fill3 : 2,
            tx_c0_g3_2 : 5,
            fill2 : 1,
            tx_cm1_g3_2 : 3,
            fill1 : 1,
            tx_cp1_g3_2 : 4,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_emph_cntrl_g3_1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_RO                    0x00003418
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_RM                    0x3DDF3DDF

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_C0_G3_1_BO         0
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_C0_G3_1_BM         0x0000001F

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CM1_G3_3_BO        6
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CM1_G3_3_BM        0x000001C0

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CP1_G3_3_BO        10
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CP1_G3_3_BM        0x00003C00

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_C0_G3_4_BO         16
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_C0_G3_4_BM         0x001F0000

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CM1_G3_4_BO        22
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CM1_G3_4_BM        0x01C00000

#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CP1_G3_4_BO        26
#define AG_MG_REGS_TX_EMPH_CNTRL_G3_2_TX_CP1_G3_4_BM        0x3C000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_EMPH_CNTRL_G3_2_U
{
    struct
    {
        ag_mg_regs_register
            tx_c0_g3_1 : 5,
            fill5 : 1,
            tx_cm1_g3_3 : 3,
            fill4 : 1,
            tx_cp1_g3_3 : 4,
            fill3 : 2,
            tx_c0_g3_4 : 5,
            fill2 : 1,
            tx_cm1_g3_4 : 3,
            fill1 : 1,
            tx_cp1_g3_4 : 4,
            fill0 : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_emph_cntrl_g3_2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_RO                      0x0000341C
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_RM                      0x0F3CF3FF

#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_0_BO          0
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_0_BM          0x0000000F

#define AG_MG_REGS_TX_MRGN_CNTRL_G1_RESEVED_0_BO            4
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_RESEVED_0_BM            0x00000030

#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_1_BO          6
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_1_BM          0x000003C0

#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_2_BO          12
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_2_BM          0x0000F000

#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_3_BO          18
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_3_BM          0x003C0000

#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_4_BO          24
#define AG_MG_REGS_TX_MRGN_CNTRL_G1_TX_AMP_G1_4_BM          0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_MRGN_CNTRL_G1_U
{
    struct
    {
        ag_mg_regs_register
            tx_amp_g1_0 : 4,
            reseved_0 : 2,
            tx_amp_g1_1 : 4,
            fill3 : 2,
            tx_amp_g1_2 : 4,
            fill2 : 2,
            tx_amp_g1_3 : 4,
            fill1 : 2,
            tx_amp_g1_4 : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_mrgn_cntrl_g1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_RO                      0x00003420
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_RM                      0x0F3CF3FF

#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_0_BO          0
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_0_BM          0x0000000F

#define AG_MG_REGS_TX_MRGN_CNTRL_G2_RESEVED_0_BO            4
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_RESEVED_0_BM            0x00000030

#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_1_BO          6
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_1_BM          0x000003C0

#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_2_BO          12
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_2_BM          0x0000F000

#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_3_BO          18
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_3_BM          0x003C0000

#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_4_BO          24
#define AG_MG_REGS_TX_MRGN_CNTRL_G2_TX_AMP_G2_4_BM          0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_MRGN_CNTRL_G2_U
{
    struct
    {
        ag_mg_regs_register
            tx_amp_g2_0 : 4,
            reseved_0 : 2,
            tx_amp_g2_1 : 4,
            fill3 : 2,
            tx_amp_g2_2 : 4,
            fill2 : 2,
            tx_amp_g2_3 : 4,
            fill1 : 2,
            tx_amp_g2_4 : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_mrgn_cntrl_g2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_RO                      0x00003424
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_RM                      0x0F3CF3FF

#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_0_BO          0
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_0_BM          0x0000000F

#define AG_MG_REGS_TX_MRGN_CNTRL_G3_RESEVED_0_BO            4
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_RESEVED_0_BM            0x00000030

#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_1_BO          6
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_1_BM          0x000003C0

#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_2_BO          12
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_2_BM          0x0000F000

#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_3_BO          18
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_3_BM          0x003C0000

#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_4_BO          24
#define AG_MG_REGS_TX_MRGN_CNTRL_G3_TX_AMP_G3_4_BM          0x0F000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_MRGN_CNTRL_G3_U
{
    struct
    {
        ag_mg_regs_register
            tx_amp_g3_0 : 4,
            reseved_0 : 2,
            tx_amp_g3_1 : 4,
            fill3 : 2,
            tx_amp_g3_2 : 4,
            fill2 : 2,
            tx_amp_g3_3 : 4,
            fill1 : 2,
            tx_amp_g3_4 : 4,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_mrgn_cntrl_g3_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TX_COMMON_PARAM_RO                       0x00003428
#define AG_MG_REGS_TX_COMMON_PARAM_RM                       0x0EDDB9C7

#define AG_MG_REGS_TX_COMMON_PARAM_TX_RISEFALL_G1_BO        0
#define AG_MG_REGS_TX_COMMON_PARAM_TX_RISEFALL_G1_BM        0x00000007

#define AG_MG_REGS_TX_COMMON_PARAM_TX_RISEFALL_G2_BO        6
#define AG_MG_REGS_TX_COMMON_PARAM_TX_RISEFALL_G2_BM        0x000001C0

#define AG_MG_REGS_TX_COMMON_PARAM_TX_RISEFALL_G3_BO        11
#define AG_MG_REGS_TX_COMMON_PARAM_TX_RISEFALL_G3_BM        0x00003800

#define AG_MG_REGS_TX_COMMON_PARAM_TX_TZ_G12_BO             15
#define AG_MG_REGS_TX_COMMON_PARAM_TX_TZ_G12_BM             0x00018000

#define AG_MG_REGS_TX_COMMON_PARAM_TX_TP_G12_BO             18
#define AG_MG_REGS_TX_COMMON_PARAM_TX_TP_G12_BM             0x001C0000

#define AG_MG_REGS_TX_COMMON_PARAM_TX_TX_G3_BO              22
#define AG_MG_REGS_TX_COMMON_PARAM_TX_TX_G3_BM              0x00C00000

#define AG_MG_REGS_TX_COMMON_PARAM_TX_TP_G3_BO              25
#define AG_MG_REGS_TX_COMMON_PARAM_TX_TP_G3_BM              0x0E000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TX_COMMON_PARAM_U
{
    struct
    {
        ag_mg_regs_register
            tx_risefall_g1 : 3,
            fill6 : 3,
            tx_risefall_g2 : 3,
            fill5 : 2,
            tx_risefall_g3 : 3,
            fill4 : 1,
            tx_tz_g12 : 2,
            fill3 : 1,
            tx_tp_g12 : 3,
            fill2 : 1,
            tx_tx_g3 : 2,
            fill1 : 1,
            tx_tp_g3 : 3,
            fill0 : 4;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_tx_common_param_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RO                   0x0000342C
#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RM                   0x0001F7DF

#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RX_LOSVREF_G1_BO     0
#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RX_LOSVREF_G1_BM     0x0000001F

#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RX_LOSVREF_G2_BO     6
#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RX_LOSVREF_G2_BM     0x000007C0

#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RX_LOSVREF_G3_BO     12
#define AG_MG_REGS_RX_LOS_COMMON_PARAM_RX_LOSVREF_G3_BM     0x0001F000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_LOS_COMMON_PARAM_U
{
    struct
    {
        ag_mg_regs_register
            rx_losvref_g1 : 5,
            fill2 : 1,
            rx_losvref_g2 : 5,
            fill1 : 1,
            rx_losvref_g3 : 5,
            fill0 : 15;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_los_common_param_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RO                  0x00003430
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RM                  0xFBEFBEF8

#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPLGLNR_BO    3
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPLGLNR_BM    0x000000F8

#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPLPLNC_BO    9
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPLPLNC_BM    0x00003E00

#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPUGLNR_BO    15
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPUGLNR_BM    0x000F8000

#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPUPLNC_BO    21
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQADPUPLNC_BM    0x03E00000

#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQDFEPLINC_BO    27
#define AG_MG_REGS_RX_COMMON_PARAM_G1_1_RX_EQDFEPLINC_BM    0xF8000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_COMMON_PARAM_G1_1_U
{
    struct
    {
        ag_mg_regs_register
            fill4 : 3,
            rx_eqadplglnr : 5,
            fill3 : 1,
            rx_eqadplplnc : 5,
            fill2 : 1,
            rx_eqadpuglnr : 5,
            fill1 : 1,
            rx_eqadpuplnc : 5,
            fill0 : 1,
            rx_eqdfeplinc : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_common_param_g1_1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RO                  0x00003434
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RM                  0x38E3321F

#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_EQDFEGLINR_BO    0
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_EQDFEGLINR_BM    0x0000001F

#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_EQDFEENA_BO      9
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_EQDFEENA_BM      0x00000200

#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_RG_BO    	    12
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_RG_BM            0x00003000

#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_RXD_BO    	 	16
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_RXD_BM        	0x00030000

#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_RZR_BO        	21
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_RX_RZR_BM        	0x00E00000

#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_EQADPALGOR0_BO    	27
#define AG_MG_REGS_RX_COMMON_PARAM_G1_2_EQADPALGOR0_BM    	0x38000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_COMMON_PARAM_G1_2_U
{
    struct
    {
        ag_mg_regs_register
            rx_eqdfeglinr : 5,
            fill4 : 4,
            rx_eqdfeena : 1,
            fill3 : 2,
            rx_rg : 2,
            fill2 : 2,
            rx_rxd : 2,
            fill1 : 3,
            rx_rzr : 3,
            fill0 : 3,
            eqadpalgor0 : 3,
            fill : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_common_param_g1_2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RO                  0x00003438
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RM                  0xFBEFBEF8

#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPLGLNR_BO    3
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPLGLNR_BM    0x000000F8

#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPLPLNC_BO    9
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPLPLNC_BM    0x00003E00

#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPUGLNR_BO    15
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPUGLNR_BM    0x000F8000

#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPUPLNC_BO    21
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQADPUPLNC_BM    0x03E00000

#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQDFEPLINC_BO    27
#define AG_MG_REGS_RX_COMMON_PARAM_G2_1_RX_EQDFEPLINC_BM    0xF8000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_COMMON_PARAM_G2_1_U
{
    struct
    {
        ag_mg_regs_register
            fill4 : 3,
            rx_eqadplglnr : 5,
            fill3 : 1,
            rx_eqadplplnc : 5,
            fill2 : 1,
            rx_eqadpuglnr : 5,
            fill1 : 1,
            rx_eqadpuplnc : 5,
            fill0 : 1,
            rx_eqdfeplinc : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_common_param_g2_1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RO                  0x0000343C
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RM                  0x38E3321F

#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_EQDFEGLINR_BO    0
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_EQDFEGLINR_BM    0x0000001F

#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_EQDFEENA_BO      9
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_EQDFEENA_BM      0x00000200

#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_RG_BO    	    12
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_RG_BM            0x00003000

#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_RXD_BO    	 	16
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_RXD_BM        	0x00030000

#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_RZR_BO        	21
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_RX_RZR_BM        	0x00E00000

#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_EQADPALGOR0_BO    	27
#define AG_MG_REGS_RX_COMMON_PARAM_G2_2_EQADPALGOR0_BM    	0x38000000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_COMMON_PARAM_G2_2_U
{
    struct
    {
        ag_mg_regs_register
            rx_eqdfeglinr : 5,
            fill4 : 4,
            rx_eqdfeena : 1,
            fill3 : 2,
            rx_rg : 2,
            fill2 : 2,
            rx_rxd : 2,
            fill1 : 3,
            rx_rzr : 3,
            fill0 : 3,
            eqadpalgor0 : 3,
            fill : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_common_param_g2_2_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RO                  0x00003440
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RM                  0xFBEFBEF8

#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPLGLNR_BO    3
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPLGLNR_BM    0x000000F8

#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPLPLNC_BO    9
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPLPLNC_BM    0x00003E00

#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPUGLNR_BO    15
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPUGLNR_BM    0x000F8000

#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPUPLNC_BO    21
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQADPUPLNC_BM    0x03E00000

#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQDFEPLINC_BO    27
#define AG_MG_REGS_RX_COMMON_PARAM_G3_1_RX_EQDFEPLINC_BM    0xF8000000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_COMMON_PARAM_G3_1_U
{
    struct
    {
        ag_mg_regs_register
            fill4 : 3,
            rx_eqadplglnr : 5,
            fill3 : 1,
            rx_eqadplplnc : 5,
            fill2 : 1,
            rx_eqadpuglnr : 5,
            fill1 : 1,
            rx_eqadpuplnc : 5,
            fill0 : 1,
            rx_eqdfeplinc : 5;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_common_param_g3_1_u;
#endif


/* 
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RO                  0x00003444
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RM                  0x38E3321F

#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_EQDFEGLINR_BO    0
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_EQDFEGLINR_BM    0x0000001F

#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_EQDFEENA_BO      9
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_EQDFEENA_BM      0x00000200

#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_RG_BO    	    12
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_RG_BM            0x00003000

#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_RXD_BO    	 	16
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_RXD_BM        	0x00030000

#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_RZR_BO        	21
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_RX_RZR_BM        	0x00E00000

#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_EQADPALGOR0_BO    	27
#define AG_MG_REGS_RX_COMMON_PARAM_G3_2_EQADPALGOR0_BM    	0x38000000


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX_COMMON_PARAM_G3_2_U
{
    struct
    {
        ag_mg_regs_register
        rx_eqdfeglinr : 5,
        fill4 : 4,
        rx_eqdfeena : 1,
        fill3 : 2,
        rx_rg : 2,
        fill2 : 2,
        rx_rxd : 2,
        fill1 : 3,
        rx_rzr : 3,
        fill0 : 3,
        eqadpalgor0 : 3,
        fill : 2;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx_common_param_g3_2_u;
#endif


/* 
 * Initialization value: 0xFA000006  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX__PARAM_LN0_RO                         0x00003460
#define AG_MG_REGS_RX__PARAM_LN0_RM                         0x0000FBDF

#define AG_MG_REGS_RX__PARAM_LN0_RX_LOSVREF_G1_BO           0
#define AG_MG_REGS_RX__PARAM_LN0_RX_LOSVREF_G1_BM           0x0000001F

#define AG_MG_REGS_RX__PARAM_LN0_RX_LOVREF_G2_BO            6
#define AG_MG_REGS_RX__PARAM_LN0_RX_LOVREF_G2_BM            0x000003C0

#define AG_MG_REGS_RX__PARAM_LN0_RX_LOSVREF_G3_BO           11
#define AG_MG_REGS_RX__PARAM_LN0_RX_LOSVREF_G3_BM           0x0000F800

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX__PARAM_LN0_U
{
    struct
    {
        ag_mg_regs_register
            rx_losvref_g1 : 5,
            fill2 : 1,
            rx_lovref_g2 : 4,
            fill1 : 1,
            rx_losvref_g3 : 5,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx__param_ln0_u;
#endif


/* 
 * Initialization value: 0x00000243  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PIPE_STATUS_LN0_RO                       0x00003464
#define AG_MG_REGS_PIPE_STATUS_LN0_RM                       0x00003FFF

#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P1_R_BO               0
#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P1_R_BM               0x00000001

#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P1_T_BO               1
#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P1_T_BM               0x00000002

#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P0S_R_BO              2
#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P0S_R_BM              0x00000004

#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P0S_T_BO              3
#define AG_MG_REGS_PIPE_STATUS_LN0_PD_P0S_T_BM              0x00000008

#define AG_MG_REGS_PIPE_STATUS_LN0_RXSYNC_SM_ERR_BO         4
#define AG_MG_REGS_PIPE_STATUS_LN0_RXSYNC_SM_ERR_BM         0x00000010

#define AG_MG_REGS_PIPE_STATUS_LN0_RXEFIFO_ERR_BO           5
#define AG_MG_REGS_PIPE_STATUS_LN0_RXEFIFO_ERR_BM           0x00000020

#define AG_MG_REGS_PIPE_STATUS_LN0_LOS_DET_BO               6
#define AG_MG_REGS_PIPE_STATUS_LN0_LOS_DET_BM               0x00000040

#define AG_MG_REGS_PIPE_STATUS_LN0_EN_SYNCDET_BO            7
#define AG_MG_REGS_PIPE_STATUS_LN0_EN_SYNCDET_BM            0x00000080

#define AG_MG_REGS_PIPE_STATUS_LN0_RX_EN_BO                 8
#define AG_MG_REGS_PIPE_STATUS_LN0_RX_EN_BM                 0x00000100

#define AG_MG_REGS_PIPE_STATUS_LN0_TERM_RX_BO               9
#define AG_MG_REGS_PIPE_STATUS_LN0_TERM_RX_BM               0x00000200

#define AG_MG_REGS_PIPE_STATUS_LN0_RX_RDY_BO                10
#define AG_MG_REGS_PIPE_STATUS_LN0_RX_RDY_BM                0x00000400

#define AG_MG_REGS_PIPE_STATUS_LN0_TX_RDY_BO                11
#define AG_MG_REGS_PIPE_STATUS_LN0_TX_RDY_BM                0x00000800

#define AG_MG_REGS_PIPE_STATUS_LN0_TRANSMIT_EN_BO           12
#define AG_MG_REGS_PIPE_STATUS_LN0_TRANSMIT_EN_BM           0x00001000

#define AG_MG_REGS_PIPE_STATUS_LN0_RECEIVER_EN_BO           13
#define AG_MG_REGS_PIPE_STATUS_LN0_RECEIVER_EN_BM           0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PIPE_STATUS_LN0_U
{
    struct
    {
        ag_mg_regs_register
            pd_p1_r : 1,
            pd_p1_t : 1,
            pd_p0s_r : 1,
            pd_p0s_t : 1,
            rxsync_sm_err : 1,
            rxefifo_err : 1,
            los_det : 1,
            en_syncdet : 1,
            rx_en : 1,
            term_rx : 1,
            rx_rdy : 1,
            tx_rdy : 1,
            transmit_en : 1,
            receiver_en : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pipe_status_ln0_u;
#endif


/* 
 * Initialization value: 0xFA000006  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_RX__PARAM_LN1_RO                         0x00003468
#define AG_MG_REGS_RX__PARAM_LN1_RM                         0x0000FBDF

#define AG_MG_REGS_RX__PARAM_LN1_RX_LOSVREF_G1_BO           0
#define AG_MG_REGS_RX__PARAM_LN1_RX_LOSVREF_G1_BM           0x0000001F

#define AG_MG_REGS_RX__PARAM_LN1_RX_LOVREF_G2_BO            6
#define AG_MG_REGS_RX__PARAM_LN1_RX_LOVREF_G2_BM            0x000003C0

#define AG_MG_REGS_RX__PARAM_LN1_RX_LOSVREF_G3_BO           11
#define AG_MG_REGS_RX__PARAM_LN1_RX_LOSVREF_G3_BM           0x0000F800

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_RX__PARAM_LN1_U
{
    struct
    {
        ag_mg_regs_register
            rx_losvref_g1 : 5,
            fill2 : 1,
            rx_lovref_g2 : 4,
            fill1 : 1,
            rx_losvref_g3 : 5,
            fill0 : 16;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_rx__param_ln1_u;
#endif


/* 
 * Initialization value: 0x00000203  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_PIPE_STATUS_LN1_RO                       0x0000346C
#define AG_MG_REGS_PIPE_STATUS_LN1_RM                       0x00003FFF

#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P1_R_BO               0
#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P1_R_BM               0x00000001

#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P1_T_BO               1
#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P1_T_BM               0x00000002

#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P0S_R_BO              2
#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P0S_R_BM              0x00000004

#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P0S_T_BO              3
#define AG_MG_REGS_PIPE_STATUS_LN1_PD_P0S_T_BM              0x00000008

#define AG_MG_REGS_PIPE_STATUS_LN1_RXSYNC_SM_ERR_BO         4
#define AG_MG_REGS_PIPE_STATUS_LN1_RXSYNC_SM_ERR_BM         0x00000010

#define AG_MG_REGS_PIPE_STATUS_LN1_RXEFIFO_ERR_BO           5
#define AG_MG_REGS_PIPE_STATUS_LN1_RXEFIFO_ERR_BM           0x00000020

#define AG_MG_REGS_PIPE_STATUS_LN1_LOS_DET_BO               6
#define AG_MG_REGS_PIPE_STATUS_LN1_LOS_DET_BM               0x00000040

#define AG_MG_REGS_PIPE_STATUS_LN1_EN_SYNCDET_BO            7
#define AG_MG_REGS_PIPE_STATUS_LN1_EN_SYNCDET_BM            0x00000080

#define AG_MG_REGS_PIPE_STATUS_LN1_RX_EN_BO                 8
#define AG_MG_REGS_PIPE_STATUS_LN1_RX_EN_BM                 0x00000100

#define AG_MG_REGS_PIPE_STATUS_LN1_TERM_RX_BO               9
#define AG_MG_REGS_PIPE_STATUS_LN1_TERM_RX_BM               0x00000200

#define AG_MG_REGS_PIPE_STATUS_LN1_RX_RDY_BO                10
#define AG_MG_REGS_PIPE_STATUS_LN1_RX_RDY_BM                0x00000400

#define AG_MG_REGS_PIPE_STATUS_LN1_TX_RDY_BO                11
#define AG_MG_REGS_PIPE_STATUS_LN1_TX_RDY_BM                0x00000800

#define AG_MG_REGS_PIPE_STATUS_LN1_TRANSMIT_EN_BO           12
#define AG_MG_REGS_PIPE_STATUS_LN1_TRANSMIT_EN_BM           0x00001000

#define AG_MG_REGS_PIPE_STATUS_LN1_RECEIVER_EN_BO           13
#define AG_MG_REGS_PIPE_STATUS_LN1_RECEIVER_EN_BM           0x00002000

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_PIPE_STATUS_LN1_U
{
    struct
    {
        ag_mg_regs_register
            pd_p1_r : 1,
            pd_p1_t : 1,
            pd_p0s_r : 1,
            pd_p0s_t : 1,
            rxsync_sm_err : 1,
            rxefifo_err : 1,
            los_det : 1,
            en_syncdet : 1,
            rx_en : 1,
            term_rx : 1,
            rx_rdy : 1,
            tx_rdy : 1,
            transmit_en : 1,
            receiver_en : 1,
            fill0 : 18;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_pipe_status_ln1_u;
#endif


/*
* Physical register addresses (for ARM accessing PCIE)
*/
#define AG_MG_REGS_PCIE_BASE				0x9B010000

/*
 * Physical register addresses (for arm accessing pcie_rc_application_layer)
 */
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(ro)	(AG_MG_REGS_PCIE_BASE+(ro))

/*
 * Physical register addresses (for arm accessing pcie_application_layer)
 */

#define AG_MG_REGS_PCIE_APPS_LAYER_REG(ro)		(AG_MG_REGS_PCIE_BASE+(ro))
 
/*
 * Physical register addresses (for arm accessing pcie_trans_layer)
 */

#define AG_MG_REGS_PCIE_TRANS_LAYER_REG(ro)		(AG_MG_REGS_PCIE_BASE+(ro))
 
/*
 * Physical register addresses (for arm accessing pcie_trans_layer)
 */

#define AG_MG_REGS_PCIE_LINK_LAYER_REG(ro)		(AG_MG_REGS_PCIE_BASE+(ro))
 
/*
 * Physical register addresses (for arm accessing pcie_pipe_layer)
 */
#define AG_MG_REGS_PCIE_PIPE_LAYER_REG(ro)		(AG_MG_REGS_PCIE_BASE+(ro))

#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REVISION_VENDOR_ID_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_REVISION_VENDOR_ID_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_CMD_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_CMD_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REVISION_CODE_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_REVISION_CODE_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_CLINE_LATENCY_HT_BIST_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_CLINE_LATENCY_HT_BIST_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_BASE_ADDR0_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_BASE_ADDR0_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_BASE_ADDR1_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_BASE_ADDR1_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_BUS_NUMBER_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_BUS_NUMBER_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_IOLIMIT_SEC_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_IOLIMIT_SEC_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_MEMORY_BASE_LIMIT_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_MEMORY_BASE_LIMIT_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PREFETCHABLE_MEMORY_BASE_LIMIT_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PREFETCHABLE_MEMORY_BASE_LIMIT_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PREFETCHABLE_BASE_U32BITS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PREFETCHABLE_BASE_U32BITS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PREFETCHABLE_LIMIT_U32BITS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PREFETCHABLE_LIMIT_U32BITS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_IO_LIMIT_BASE_U32BITS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_IO_LIMIT_BASE_U32BITS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_CAPABILITIES_PTR_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_CAPABILITIES_PTR_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_EXPANSION_ROM_BASE_ADDR_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_EXPANSION_ROM_BASE_ADDR_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_INTERRUPT_BCTRL_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_INTERRUPT_BCTRL_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PME_CAPABILITY_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PME_CAPABILITY_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PME_CTRL_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PME_CTRL_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_CAPABILITY_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_CAPABILITY_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_DEV_CAPABILITIES_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_DEV_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_DEV_CTRL_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_DEV_CTRL_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_LINK_CAPABILITIES_APRC_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_LINK_CAPABILITIES_APRC_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_LINK_CTRL_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_LINK_CTRL_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_SLOT_CAPABILITIES_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_SLOT_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_SLOT_CTRL_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_SLOT_CTRL_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_ROOT_CTRL_CAPABILITIES_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_ROOT_CTRL_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_DEV_CAPABILITIES2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_DEV_CAPABILITIES2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_DEV_CTRL_STATUS2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_DEV_CTRL_STATUS2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_LINK_CAP2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_LINK_CAP2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_LINK_CTRL_STATUS2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_LINK_CTRL_STATUS2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_DEV_CAP_RSVD1_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_DEV_CAP_RSVD1_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_DEV_CAP_RSVD2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_DEV_CAP_RSVD2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PCIE_ENHANCED_CAP_HEADER_APRC_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_APRC_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_UNC_ERR_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_UNC_ERR_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_UNC_ERR_MASK_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_UNC_ERR_MASK_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_UNC_ERR_SEVERITY_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_UNC_ERR_SEVERITY_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_CORR_ERR_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_CORR_ERR_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_CORR_ERR_MASK_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_CORR_ERR_MASK_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_ERR_CAP_CTRL_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_ERR_CAP_CTRL_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_HEADER_LOG1_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_HEADER_LOG1_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_HEADER_LOG2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_HEADER_LOG2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_HEADER_LOG3_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_HEADER_LOG3_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_HEADER_LOG4_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_HEADER_LOG4_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_ROOT_ERR_CMD_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_ROOT_ERR_CMD_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_ROOT_ERR_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_ROOT_ERR_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_ERR_SRC_ID_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_ERR_SRC_ID_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_VC_CHANNEL_CAP_HEADER_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_VC_CHANNEL_CAP_HEADER_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PORT_VC_CAPABILITY1_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PORT_VC_CAPABILITY1_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PORT_VC_CAPAILITY2_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PORT_VC_CAPAILITY2_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_PORT_VC_CTRL_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_PORT_VC_CTRL_STATUS_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_VC_RESOURCE_CAP_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_VC_RESOURCE_CAP_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_VC_RESPONSE_CTRL_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_VC_RESPONSE_CTRL_RO)
#define AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_VC_RESOURCE_STATUS_RA	AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REG(AG_MG_REGS_VC_RESOURCE_STATUS_RO)

/* ------------- */

#define AG_MG_REGS_PCIE_APPS_LAYER_CONFIGURE_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_CONFIGURE_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_CORE_DEBUG_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_CORE_DEBUG_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_LOOPBACK_FAIL_STATUS_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_LOOPBACK_FAIL_STATUS_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE0_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE0_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE0_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE0_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE1_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE1_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE1_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE1_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE2_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE2_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE2_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE2_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE3_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE3_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE3_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE3_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE4_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE4_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE4_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE4_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE5_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE5_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE5_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE5_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE6_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE6_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE6_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE6_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE7_UP_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE7_UP_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MPAGE7_LO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MPAGE7_LO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE0_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE0_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE1_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE1_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE2_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE2_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE3_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE3_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE4_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE4_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE5_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE5_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE6_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE6_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE7_BAR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE7_BAR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE0_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE0_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE1_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE1_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE2_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE2_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE3_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE3_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE4_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE4_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE5_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE5_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE6_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE6_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE7_BAR1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE7_BAR1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE0_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE0_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE1_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE1_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE2_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE2_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE3_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE3_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE4_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE4_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE5_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE5_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE6_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE6_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TPAGE7_BAR2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TPAGE7_BAR2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MESSAGE_IN_FIFO_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MESSAGE_IN_FIFO_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MESSAGE_IN_FIFO_STATUS_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MESSAGE_IN_FIFO_STATUS_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MESSAGE_OUT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MESSAGE_OUT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_INTERRUPT_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_INTERRUPT_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_INTERRUPT_ENABLE_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_INTERRUPT_ENABLE_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_INTERRUPT_FORCE_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_INTERRUPT_FORCE_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PHY_PCIE_STA0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PHY_PCIE_STA0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PHY_PCIE_STA1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PHY_PCIE_STA1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PCIE_PHY_CTR0_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_PHY_CTR0_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PCIE_PHY_CTRL1_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_PHY_CTRL1_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PCIE_PHY_CTRL2_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_PHY_CTRL2_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_CONF_WR_CPL_TMP_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_CONF_WR_CPL_TMP_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_DEC_ERROR_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_DEC_ERROR_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MESSAGE_IN_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MESSAGE_IN_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PCIE_RESERVED1_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_RESERVED1_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MR_CPL_DATA_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MR_CPL_DATA_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MR_CPL_HDR_FIFO_DTAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MR_CPL_HDR_FIFO_DTAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MR_REQ_HDR_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MR_REQ_HDR_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MW_DATA_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MW_DATA_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_RX_CPL_TMP_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_RX_CPL_TMP_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_COMPL_ALIGNED_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_COMPL_ALIGNED_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RX_CPL_ERR_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RX_CPL_ERR_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SR_HDR_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SR_HDR_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SW_CONF_HDR_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SW_CONF_HDR_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SW_DATA_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SW_DATA_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SW_HDR_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SW_HDR_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_COMPL_TIMEOUT_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_AXI_ID_FREELIST_FIFO_STATUS_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_AXI_ID_FREELIST_FIFO_STATUS_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_ASSIGNED_AXI_ID_FIFO_STATUS_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_0_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_0_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_1_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_1_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_2_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_2_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_3_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_3_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_4_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_4_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_5_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_5_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_6_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_6_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_7_FIFO_STAT_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_7_FIFO_STAT_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_TIMEOUT_ID_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_PKT_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_WRT_CONF_WRT_RTN_COMPL_ID_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_COMPL_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_COMPL_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_SLV_RD_CPL_TIMEOUT_COMPLETER_ID_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_TL_FSM_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_TL_FSM_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_EGR_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_EGR_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_REQ_PROC_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_REQ_PROC_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_CPL_TO_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_CPL_TO_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_IGR_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_IGR_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_FN_INDP_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_FN_INDP_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_FN_INDP_OTHER_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_FN_INDP_OTHER_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_T2A_PARITY_ERR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_T2A_PARITY_ERR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_CONFIG_LINK_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_CONFIG_LINK_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_CAPTURED_MW_ERR_RESP_BAR_ADDR_STATUS_REG_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PCIE_RESERVED2_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_RESERVED2_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_PCIE_RESERVED3_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_PCIE_RESERVED3_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_L2T_CE_STATUS_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_L2T_CE_STATUS_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_AXI_ADDR_FOR_MSI_NOTIFICATION_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_AXI_ADDR_FOR_MSI_NOTIFICATION_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MGW_CPU_INT_MSI_VECTORS_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MGW_CPU_INT_MSI_VECTORS_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_MGW_INTERRUPT_STATUS_REGISTER_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_MGW_INTERRUPT_STATUS_REGISTER_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_INTERRUPT_ENABLE_REGISTER_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_INTERRUPT_ENABLE_REGISTER_RO)
#define AG_MG_REGS_PCIE_APPS_LAYER_INTERRUPT_FORCE_REGISTER_RA	AG_MG_REGS_PCIE_APPS_LAYER_REG(AG_MG_REGS_INTERRUPT_FORCE_REGISTER_RO)

/* ------------- */

#define AG_MG_REGS_PCIE_TRANS_LAYER_DEVICE_AND_VENDOR_ID_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DEVICE_AND_VENDOR_ID_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_STATUS_AND_COMMAND_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_STATUS_AND_COMMAND_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_CLASS_AND_REVISION_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_CLASS_AND_REVISION_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_TEST_CONFIGURATION_REGISTER_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_TEST_CONFIGURATION_REGISTER_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_BAR0_LO_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_BAR0_LO_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_BAR0_HI_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_BAR0_HI_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_BAR1_LO_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_BAR1_LO_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_BAR1_HI_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_BAR1_HI_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_BAR2_LO_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_BAR2_LO_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_BAR2_HI_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_BAR2_HI_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_CARDBUS_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_CARDBUS_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_SUBSYSTEM_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_SUBSYSTEM_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_EXPANSION_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_EXPANSION_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_CAPABILITIES_POINTER_REG_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_CAPABILITIES_POINTER_REG_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_INTERRUPT_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_INTERRUPT_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PCIE_PWR_MGN_CAPABILITIES_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PCIE_PWR_MGN_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PCIE_PWR_MGN_STATUS_CTRL_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PCIE_PWR_MGN_STATUS_CTRL_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_MESSAGE_CONTROL_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_MESSAGE_CONTROL_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_TABLE_OFFSET_BIR_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_TABLE_OFFSET_BIR_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PBA_OFFSET_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PBA_OFFSET_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PCIE_CAPABILITIES_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PCIE_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_DEVICE_CAPABILITIES_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DEVICE_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_DEVICE_STATUS_AND_CONTROL_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DEVICE_STATUS_AND_CONTROL_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_LINK_CAPABILITIES_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_LINK_CAPABILITIES_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_LINK_STATUS_AND_CTRL_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_LINK_STATUS_AND_CTRL_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_DEVICE_CAP2_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DEVICE_CAP2_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_DEVICE_CTRL_2_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DEVICE_CTRL_2_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_LINK_STATUS_AND_CONTROL_2_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_LINK_STATUS_AND_CONTROL_2_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_ADVANCED_ERROR_REPORT_CAPABILITY_HEADER_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_UNCORRECTABLE_ERROR_STATUS_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_UNCORRECTABLE_ERROR_STATUS_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_UNCORRECTABLE_ERROR_MASK_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_UNCORRECTABLE_ERROR_MASK_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_UNCORRECTABLE_ERROR_SEVERITY_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_UNCORRECTABLE_ERROR_SEVERITY_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_CORRECTABLE_ERROR_STATUS_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_CORRECTABLE_ERROR_STATUS_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_CORRECTABLE_ERROR_MASK_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_CORRECTABLE_ERROR_MASK_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_ADV_ERROR_CAP_AND_CTRL_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_ADV_ERROR_CAP_AND_CTRL_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_HEADER_LOG_REGISTER0_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_HEADER_LOG_REGISTER0_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_HEAD_LOG_REGISTER1_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_HEAD_LOG_REGISTER1_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_HEAD_LOG_REGISTER2_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_HEAD_LOG_REGISTER2_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_HEAD_LOG_REGISTER3_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_HEAD_LOG_REGISTER3_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_ENHANCED_CAPABILITY_HEADER_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_ENHANCED_CAPABILITY_HEADER_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PORT_VC_CAPABILITY_REG_1_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PORT_VC_CAPABILITY_REG_1_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PORT_VC_CAP_REG_2_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PORT_VC_CAP_REG_2_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PORT_VC_CONTROL_STATUS_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PORT_VC_CONTROL_STATUS_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_VC_RESOURCE_CAP_REG_0_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_VC_RESOURCE_CAP_REG_0_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_VC_RESOURCE_CONTROL_REG_0_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_VC_RESOURCE_CONTROL_REG_0_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_VC_RESOURCE_STATUS_REG_0_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_VC_RESOURCE_STATUS_REG_0_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_ENHANCED_CAP_HEADER_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_ENHANCED_CAP_HEADER_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_DATA_SELECT_REG_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DATA_SELECT_REG_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_DATA_REGISTER_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_DATA_REGISTER_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PWR_BUDGET_CAP_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PWR_BUDGET_CAP_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_PCIE_ENHANCED_CAP_HEADER_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_PCIE_ENHANCED_CAP_HEADER_RO)
#define AG_MG_REGS_PCIE_TRANS_LAYER_ARI_CAP_CTRL_RA	AG_MG_REGS_PCIE_TRANS_LAYER_REG(AG_MG_REGS_ARI_CAP_CTRL_RO)

/* ------------- */


#define AG_MG_REGS_PCIE_LINK_LAYER_LINK_CORE_ID_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINK_CORE_ID_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINK_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINK_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_INTERRUPT_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_INTERRUPT_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_PERFORMANCE_COUNTER_INTERRUPT_TEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_PERFORMANCE_COUNTER_INTERRUPT_TEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_INTERRUPT_MASK_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_INTERRUPT_MASK_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TEST_CONTROL1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TEST_CONTROL1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TEST_CONTROL2_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TEST_CONTROL2_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TEST_DATA_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TEST_DATA_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_MEMORY_ADDRESS_CONTROL_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_MEMORY_ADDRESS_CONTROL_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_MEMORYDATALOAD0_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_MEMORYDATALOAD0_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_MEMORYDATALOAD1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_MEMORYDATALOAD1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_MEMORYDATALOAD2_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_MEMORYDATALOAD2_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_MEMORYDATALOAD3_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_MEMORYDATALOAD3_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_MEMORYDATALOAD4_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_MEMORYDATALOAD4_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINKPERFORMANCECOUNTERSELECT_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINKPERFORMANCECOUNTERSELECT_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINKPERFORMANCECOUNTERCONTROL_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINKPERFORMANCECOUNTERCONTROL_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINKPERFORMANCECOUNTERSAMPLEDURATION_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINKPERFORMANCECOUNTERSAMPLEDURATION_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINKPERFORMANCECOUNTER1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINKPERFORMANCECOUNTER1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINKPERFORMANCECOUNTER1_TEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINKPERFORMANCECOUNTER1_TEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LINKPERFORMANCECOUNTER_2_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LINKPERFORMANCECOUNTER_2_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_PERFORMANCECOUNTER2_TEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_PERFORMANCECOUNTER2_TEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_DEBUG_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_DEBUG_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_DEBUGCONFIGURATION_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_DEBUGCONFIGURATION_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TX_CONFIGURATION_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TX_CONFIGURATION_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TX_LINK_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TX_LINK_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TX_INTERRUPT_AND_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TX_INTERRUPT_AND_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TX_INTERRUPT_AND_STATUS_TEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TX_INTERRUPT_AND_STATUS_TEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TX_INTERRUPT_MASK_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TX_INTERRUPT_MASK_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_FLOW_CONTROL_UPDATE_TIMEOUT_VALUE_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_ACK_NAK_LATENCY_THRESHOLD_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_ACK_NAK_LATENCY_THRESHOLD_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_REPLAYTIMEOUTTHRESHOLD_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_REPLAYTIMEOUTTHRESHOLD_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_REPLAY_NUMBER_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_REPLAY_NUMBER_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RETRY_BUFFER_POINTER_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RETRY_BUFFER_POINTER_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_SEQUENCE_COUNTER_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_SEQUENCE_COUNTER_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_SEQUENCE_BUFFER_POINTERS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_SEQUENCE_BUFFER_POINTERS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_SKIPTIMERTHRESHOLD_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_SKIPTIMERTHRESHOLD_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_EIES_COUNTER_THRESHOLD_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_EIES_COUNTER_THRESHOLD_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RX_CONFIGURATION_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RX_CONFIGURATION_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RX_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RX_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RX_INTERRUPT_AND_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RX_INTERRUPT_AND_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RX_INTERRUPT_STATUS_TEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RX_INTERRUPT_STATUS_TEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RX_INTERRUPT_MASK_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RX_INTERRUPT_MASK_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_RX_TS_CONTROL_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_RX_TS_CONTROL_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_NEXT_RCV_SEQUENCE_COUNTER_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_NEXT_RCV_SEQUENCE_COUNTER_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_UNKNOWNDLLP0_RECEIVED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_UNKNOWNDLLP0_RECEIVED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_UNKNOWN_DLLP1_RECEIVED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_UNKNOWN_DLLP1_RECEIVED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_CONFIGURATION_1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_CONFIGURATION_1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_INTERRUPT_AND_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_INTERRUPT_AND_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_INTERRUPT_TEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_INTERRUPT_TEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_INTERRUPT_MASK_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_INTERRUPT_MASK_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_TIMER_THRESHOLD1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_TIMER_THRESHOLD1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_TIMER_THRESHOLD2_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_TIMER_THRESHOLD2_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_TIMER_THRESHOLD3_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_TIMER_THRESHOLD3_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_TIMER_THRESHOLD4_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_TIMER_THRESHOLD4_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_REQUEST_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_REQUEST_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_TRAINING_CONFIGURATION_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_TRAINING_CONFIGURATION_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_STATUS_2_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_STATUS_2_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_RX_COMMAND_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_RX_COMMAND_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_TX_COMMAND_STATUS_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_TX_COMMAND_STATUS_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_GEN_2_TIMER_THRESHOLD1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_GEN_2_TIMER_THRESHOLD2_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_GEN_2_TIMER_THRESHOLD2_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_LTSSM_GEN2_SPEED_NFTS_NUMBER_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_LTSSM_GEN2_SPEED_NFTS_NUMBER_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TL_GEN_DEBUG_REG_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TL_GEN_DEBUG_REG_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_TLSB_SIG_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_TLSB_SIG_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_COMPLETION_TIME_OUT_REGISTER0_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER0_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_COMPLETION_TIME_OUT_REGISTER1_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_COMPLETION_TIME_OUT_REGISTER1_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_INGRESS_FC_CONTROL_REG_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_INGRESS_FC_CONTROL_REG_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_POSTED_PROG_CREDIT_FREED_THRESHOLD_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_NONPOSTED_PROGRAMMABLE_CREDIT_FREED_THRESHOLD_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_RECEIVED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_RECEIVED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_INGRESS_FC_POSTED_CREDITS_ALLOCATED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_INGRESS_FC_NONPOSTED_CREDITS_ALLOCATED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_POSTED_CREDITS_CONSUMED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_NONPOSTED_CREDITS_CONSUMED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_COMPLETION_CREDITS_CONSUMED_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_POSTED_CREDIT_LIMIT_REGISTER_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_POSTED_CREDIT_LIMIT_REGISTER_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_NONPOSTED_LIMIT_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_NONPOSTED_LIMIT_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_COMPLETION_LIMIT_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_COMPLETION_LIMIT_RO)
#define AG_MG_REGS_PCIE_LINK_LAYER_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_RA	AG_MG_REGS_PCIE_LINK_LAYER_REG(AG_MG_REGS_VC0_EGRESS_FLOW_CONTROL_TIMEOUT_TIMER_RO)

/* ------------- */

#define AG_MG_REGS_PCIE_PIPE_LAYER_PIPE_RST_EN_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_PIPE_RST_EN_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_PIPE_CONTROL_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_PIPE_CONTROL_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_PIPE_COMMON_STATUS_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_PIPE_COMMON_STATUS_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_EMPH_CNTRL_G1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_EMPH_CNTRL_G1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_EMPH_CNTRL_G2_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_EMPH_CNTRL_G2_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_EMPH_CNTRL_G3_1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_EMPH_CNTRL_G3_1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_EMPH_CNTRL_G3_2_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_EMPH_CNTRL_G3_2_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_MRGN_CNTRL_G1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_MRGN_CNTRL_G1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_MRGN_CNTRL_G2_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_MRGN_CNTRL_G2_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_MRGN_CNTRL_G3_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_MRGN_CNTRL_G3_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_TX_COMMON_PARAM_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_TX_COMMON_PARAM_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_LOS_COMMON_PARAM_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_LOS_COMMON_PARAM_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_COMMON_PARAM_G1_1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_COMMON_PARAM_G1_1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_COMMON_PARAM_G1_2_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_COMMON_PARAM_G1_2_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_COMMON_PARAM_G2_1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_COMMON_PARAM_G2_1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_COMMON_PARAM_G2_2_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_COMMON_PARAM_G2_2_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_COMMON_PARAM_G3_1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_COMMON_PARAM_G3_1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX_COMMON_PARAM_G3_2_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX_COMMON_PARAM_G3_2_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX__PARAM_LN0_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX__PARAM_LN0_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_PIPE_STATUS_LN0_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_PIPE_STATUS_LN0_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_RX__PARAM_LN1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_RX__PARAM_LN1_RO)
#define AG_MG_REGS_PCIE_PIPE_LAYER_PIPE_STATUS_LN1_RA	AG_MG_REGS_PCIE_PIPE_LAYER_REG(AG_MG_REGS_PIPE_STATUS_LN1_RO)

#ifdef AG_MG_REGS_USE_C_STRUCTURES

typedef struct AG_MG_REGS_PCIE_RC_APPLICATION_LAYER_REGS_S
{
	ag_mg_regs_revision_vendor_id_u	revision_vendor_id ;
	ag_mg_regs_cmd_status_u	cmd_status ;
	ag_mg_regs_revision_code_u	revision_code ;
	ag_mg_regs_cline_latency_ht_bist_u	cline_latency_ht_bist ;
	ag_mg_regs_base_addr0_u	base_addr0 ;
	ag_mg_regs_base_addr1_u	base_addr1 ;
	ag_mg_regs_bus_number_u	bus_number ;
	ag_mg_regs_iolimit_sec_status_u	iolimit_sec_status ;
	ag_mg_regs_memory_base_limit_u	memory_base_limit ;
	ag_mg_regs_prefetchable_memory_base_limit_u	prefetchable_memory_base_limit ;
	ag_mg_regs_prefetchable_base_u32bits_u	prefetchable_base_u32bits ;
	ag_mg_regs_prefetchable_limit_u32bits_u	prefetchable_limit_u32bits ;
	ag_mg_regs_io_limit_base_u32bits_u	io_limit_base_u32bits ;
	ag_mg_regs_capabilities_ptr_u	capabilities_ptr ;
	ag_mg_regs_expansion_rom_base_addr_u	expansion_rom_base_addr ;
	ag_mg_regs_interrupt_bctrl_u	interrupt_bctrl ;
	ag_mg_regs_pme_capability_u	pme_capability ;
	ag_mg_regs_pme_ctrl_status_u	pme_ctrl_status ;
	ag_mg_regs_register		FILL0[6] ;
	ag_mg_regs_capability_u	capability ;
	ag_mg_regs_dev_capabilities_u	dev_capabilities ;
	ag_mg_regs_dev_ctrl_status_u	dev_ctrl_status ;
	ag_mg_regs_link_capabilities_aprc_u	link_capabilities_aprc ;
	ag_mg_regs_link_ctrl_status_u	link_ctrl_status ;
	ag_mg_regs_slot_capabilities_u	slot_capabilities ;
	ag_mg_regs_slot_ctrl_status_u	slot_ctrl_status ;
	ag_mg_regs_root_ctrl_capabilities_u	root_ctrl_capabilities ;
	ag_mg_regs_register		reserved;
	ag_mg_regs_dev_capabilities2_u	dev_capabilities2 ;
	ag_mg_regs_dev_ctrl_status2_u	dev_ctrl_status2 ;
	ag_mg_regs_link_cap2_u	link_cap2 ;
	ag_mg_regs_link_ctrl_status2_u	link_ctrl_status2 ;
	ag_mg_regs_dev_cap_rsvd1_u	dev_cap_rsvd1 ;
	ag_mg_regs_dev_cap_rsvd2_u	dev_cap_rsvd2 ;
	ag_mg_regs_register		FILL1[25] ;
	ag_mg_regs_pcie_enhanced_cap_header_aprc_u	pcie_enhanced_cap_header_aprc ;
	ag_mg_regs_unc_err_status_u	unc_err_status ;
	ag_mg_regs_unc_err_mask_u	unc_err_mask ;
	ag_mg_regs_unc_err_severity_u	unc_err_severity ;
	ag_mg_regs_corr_err_status_u	corr_err_status ;
	ag_mg_regs_corr_err_mask_u	corr_err_mask ;
	ag_mg_regs_err_cap_ctrl_u	err_cap_ctrl ;
	ag_mg_regs_header_log1_u	header_log1 ;
	ag_mg_regs_header_log2_u	header_log2 ;
	ag_mg_regs_header_log3_u	header_log3 ;
	ag_mg_regs_header_log4_u	header_log4 ;
	ag_mg_regs_root_err_cmd_u	root_err_cmd ;
	ag_mg_regs_root_err_status_u	root_err_status ;
	ag_mg_regs_err_src_id_u	err_src_id ;
	ag_mg_regs_register		FILL2[2] ;
	ag_mg_regs_vc_channel_cap_header_u	vc_channel_cap_header ;
	ag_mg_regs_port_vc_capability1_u	port_vc_capability1 ;
	ag_mg_regs_port_vc_capaility2_u	port_vc_capaility2 ;
	ag_mg_regs_port_vc_ctrl_status_u	port_vc_ctrl_status ;
	ag_mg_regs_vc_resource_cap_u	vc_resource_cap ;
	ag_mg_regs_vc_response_ctrl_u	vc_response_ctrl ;
	ag_mg_regs_vc_resource_status_u	vc_resource_status;

	ag_mg_regs_register		FILL3[937] ;

// ==> END OF ag_mg_regs_pcie_rc_application_layer_regs ;

	ag_mg_regs_configure_reg_u	configure_reg ;
	ag_mg_regs_status_reg_u	status_reg ;
	ag_mg_regs_core_debug_reg_u	core_debug_reg ;
	ag_mg_regs_loopback_fail_status_u	loopback_fail_status ;
	ag_mg_regs_mpage0_up_u	mpage0_up ;
	ag_mg_regs_mpage0_lo_u	mpage0_lo ;
	ag_mg_regs_mpage1_up_u	mpage1_up ;
	ag_mg_regs_mpage1_lo_u	mpage1_lo ;
	ag_mg_regs_mpage2_up_u	mpage2_up ;
	ag_mg_regs_mpage2_lo_u	mpage2_lo ;
	ag_mg_regs_mpage3_up_u	mpage3_up ;
	ag_mg_regs_mpage3_lo_u	mpage3_lo ;
	ag_mg_regs_mpage4_up_u	mpage4_up ;
	ag_mg_regs_mpage4_lo_u	mpage4_lo ;
	ag_mg_regs_mpage5_up_u	mpage5_up ;
	ag_mg_regs_mpage5_lo_u	mpage5_lo ;
	ag_mg_regs_mpage6_up_u	mpage6_up ;
	ag_mg_regs_mpage6_lo_u	mpage6_lo ;
	ag_mg_regs_mpage7_up_u	mpage7_up ;
	ag_mg_regs_mpage7_lo_u	mpage7_lo ;
	ag_mg_regs_tpage0_bar0_reg_u	tpage0_bar0_reg ;
	ag_mg_regs_tpage1_bar0_reg_u	tpage1_bar0_reg ;
	ag_mg_regs_tpage2_bar0_reg_u	tpage2_bar0_reg ;
	ag_mg_regs_tpage3_bar0_reg_u	tpage3_bar0_reg ;
	ag_mg_regs_tpage4_bar0_reg_u	tpage4_bar0_reg ;
	ag_mg_regs_tpage5_bar0_reg_u	tpage5_bar0_reg ;
	ag_mg_regs_tpage6_bar0_reg_u	tpage6_bar0_reg ;
	ag_mg_regs_tpage7_bar0_reg_u	tpage7_bar0_reg ;
	ag_mg_regs_tpage0_bar1_reg_u	tpage0_bar1_reg ;
	ag_mg_regs_tpage1_bar1_reg_u	tpage1_bar1_reg ;
	ag_mg_regs_tpage2_bar1_reg_u	tpage2_bar1_reg ;
	ag_mg_regs_tpage3_bar1_reg_u	tpage3_bar1_reg ;
	ag_mg_regs_tpage4_bar1_reg_u	tpage4_bar1_reg ;
	ag_mg_regs_tpage5_bar1_reg_u	tpage5_bar1_reg ;
	ag_mg_regs_tpage6_bar1_reg_u	tpage6_bar1_reg ;
	ag_mg_regs_tpage7_bar1_reg_u	tpage7_bar1_reg ;
	ag_mg_regs_tpage0_bar2_reg_u	tpage0_bar2_reg ;
	ag_mg_regs_tpage1_bar2_reg_u	tpage1_bar2_reg ;
	ag_mg_regs_tpage2_bar2_reg_u	tpage2_bar2_reg ;
	ag_mg_regs_tpage3_bar2_reg_u	tpage3_bar2_reg ;
	ag_mg_regs_tpage4_bar2_reg_u	tpage4_bar2_reg ;
	ag_mg_regs_tpage5_bar2_reg_u	tpage5_bar2_reg ;
	ag_mg_regs_tpage6_bar2_reg_u	tpage6_bar2_reg ;
	ag_mg_regs_tpage7_bar2_reg_u	tpage7_bar2_reg ;
	ag_mg_regs_message_in_fifo_u	message_in_fifo ;
	ag_mg_regs_message_in_fifo_status_u	message_in_fifo_status ;
	ag_mg_regs_message_out_u	message_out ;
	ag_mg_regs_register		FILL4 ;
	ag_mg_regs_pcie_interrupt_status_reg_u	pcie_interrupt_status_reg ;
	ag_mg_regs_pcie_interrupt_enable_reg_u	pcie_interrupt_enable_reg ;
	ag_mg_regs_pcie_interrupt_force_reg_u	pcie_interrupt_force_reg ;
	ag_mg_regs_phy_pcie_sta0_reg_u	phy_pcie_sta0_reg ;
	ag_mg_regs_phy_pcie_sta1_reg_u	phy_pcie_sta1_reg ;
	ag_mg_regs_pcie_phy_ctrl0_reg_u	pcie_phy_ctrl0_reg ;
	ag_mg_regs_register	FILL4A ;
	ag_mg_regs_pcie_phy_ctrl2_reg_u	pcie_phy_ctrl2_reg ;
	ag_mg_regs_conf_wr_cpl_tmp_fifo_stat_u	conf_wr_cpl_tmp_fifo_stat ;
	ag_mg_regs_dec_error_fifo_stat_u	dec_error_fifo_stat ;
	ag_mg_regs_message_in_fifo_stat_u	message_in_fifo_stat ;
	ag_mg_regs_pcie_reserved1_u	pcie_reserved1 ;
	ag_mg_regs_mr_cpl_data_fifo_stat_u	mr_cpl_data_fifo_stat ;
	ag_mg_regs_mr_cpl_hdr_fifo_dtat_u	mr_cpl_hdr_fifo_dtat ;
	ag_mg_regs_mr_req_hdr_fifo_stat_u	mr_req_hdr_fifo_stat ;
	ag_mg_regs_mw_data_fifo_stat_u	mw_data_fifo_stat ;
	ag_mg_regs_rx_cpl_tmp_fifo_stat_u	rx_cpl_tmp_fifo_stat ;
	ag_mg_regs_slv_rd_compl_aligned_fifo_stat_u	slv_rd_compl_aligned_fifo_stat ;
	ag_mg_regs_slv_rx_cpl_err_fifo_stat_u	slv_rx_cpl_err_fifo_stat ;
	ag_mg_regs_sr_hdr_fifo_stat_u	sr_hdr_fifo_stat ;
	ag_mg_regs_sw_conf_hdr_fifo_stat_u	sw_conf_hdr_fifo_stat ;
	ag_mg_regs_sw_data_fifo_stat_u	sw_data_fifo_stat ;
	ag_mg_regs_sw_hdr_fifo_stat_u	sw_hdr_fifo_stat ;
	ag_mg_regs_slv_rd_compl_timeout_fifo_stat_u	slv_rd_compl_timeout_fifo_stat ;
	ag_mg_regs_axi_id_freelist_fifo_status_stat_u	axi_id_freelist_fifo_status_stat ;
	ag_mg_regs_assigned_axi_id_fifo_status_fifo_stat_u	assigned_axi_id_fifo_status_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_0_fifo_stat_u	slv_rd_cpl_0_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_1_fifo_stat_u	slv_rd_cpl_1_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_2_fifo_stat_u	slv_rd_cpl_2_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_3_fifo_stat_u	slv_rd_cpl_3_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_4_fifo_stat_u	slv_rd_cpl_4_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_5_fifo_stat_u	slv_rd_cpl_5_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_6_fifo_stat_u	slv_rd_cpl_6_fifo_stat ;
	ag_mg_regs_slv_rd_cpl_7_fifo_stat_u	slv_rd_cpl_7_fifo_stat ;
	ag_mg_regs_slv_wrt_conf_wrt_rtn_timeout_id_status_reg_u	slv_wrt_conf_wrt_rtn_timeout_id_status_reg ;
	ag_mg_regs_slv_wrt_conf_wrt_rtn_pkt_err_status_reg_u	slv_wrt_conf_wrt_rtn_pkt_err_status_reg ;
	ag_mg_regs_slv_wrt_conf_wrt_rtn_compl_id_err_status_reg_u	slv_wrt_conf_wrt_rtn_compl_id_err_status_reg ;
	ag_mg_regs_slv_rd_compl_err_status_reg_u	slv_rd_compl_err_status_reg ;
	ag_mg_regs_slv_rd_cpl_timeout_completer_id_reg_u	slv_rd_cpl_timeout_completer_id_reg ;
	ag_mg_regs_tl_fsm_err_status_reg_u	tl_fsm_err_status_reg ;
	ag_mg_regs_t2a_egr_err_status_reg_u	t2a_egr_err_status_reg ;
	ag_mg_regs_t2a_req_proc_err_status_reg_u	t2a_req_proc_err_status_reg ;
	ag_mg_regs_t2a_cpl_to_err_status_reg_u	t2a_cpl_to_err_status_reg ;
	ag_mg_regs_t2a_igr_err_status_reg_u	t2a_igr_err_status_reg ;
	ag_mg_regs_t2a_fn_indp_err_status_reg_u	t2a_fn_indp_err_status_reg ;
	ag_mg_regs_t2a_fn_indp_other_err_status_reg_u	t2a_fn_indp_other_err_status_reg ;
	ag_mg_regs_t2a_parity_err_status_reg_u	t2a_parity_err_status_reg ;
	ag_mg_regs_config_link_status_reg_u	config_link_status_reg ;
	ag_mg_regs_captured_mw_err_resp_bar_addr_status_reg_u	captured_mw_err_resp_bar_addr_status_reg ;
	ag_mg_regs_pcie_reserved2_u	pcie_reserved2 ;
	ag_mg_regs_pcie_reserved3_u	pcie_reserved3 ;
	ag_mg_regs_l2t_ce_status_u	l2t_ce_status ;
	ag_mg_regs_axi_addr_for_msi_notification_u	axi_addr_for_msi_notification ;
	ag_mg_regs_mgw_int_msi_vectors_u	mgw_int_msi_vectors ;
	ag_mg_regs_mgw_interrupt_status_register_u	mgw_interrupt_status_register ;
	ag_mg_regs_mgw_interrupt_enable_register_u	mgw_interrupt_enable_register ;
	ag_mg_regs_mgw_interrupt_force_register_u	mgw_interrupt_force_register ;

	ag_mg_regs_register		FILL5[919] ;

// ==> END OF ag_mg_regs_pcie_apps_layer_regs

	ag_mg_regs_device_and_vendor_id_u	device_and_vendor_id ;
	ag_mg_regs_status_and_command_u	status_and_command ;
	ag_mg_regs_class_and_revision_u	class_and_revision ;
	ag_mg_regs_test_configuration_register_u	test_configuration_register ;
	ag_mg_regs_bar0_lo_u	bar0_lo ;
	ag_mg_regs_bar0_hi_u	bar0_hi ;
	ag_mg_regs_bar1_lo_u	bar1_lo ;
	ag_mg_regs_bar1_hi_u	bar1_hi ;
	ag_mg_regs_bar2_lo_u	bar2_lo ;
	ag_mg_regs_bar2_hi_u	bar2_hi ;
	ag_mg_regs_cardbus_u	cardbus ;
	ag_mg_regs_subsystem_u	subsystem ;
	ag_mg_regs_expansion_u	expansion ;
	ag_mg_regs_capabilities_pointer_reg_u	capabilities_pointer_reg ;
	ag_mg_regs_register			FILL6 ;
	ag_mg_regs_interrupt_u	interrupt ;
	ag_mg_regs_pcie_pwr_mgn_capabilities_u	pcie_pwr_mgn_capabilities ;
	ag_mg_regs_pcie_pwr_mgn_status_ctrl_u	pcie_pwr_mgn_status_ctrl ;
	ag_mg_regs_register			FILL7[2] ;
	ag_mg_regs_message_control_u	message_control ;
	ag_mg_regs_table_offset_bir_u	table_offset_bir ;
	ag_mg_regs_pba_offset_u	pba_offset ;
	ag_mg_regs_register			FILL8 ;
	ag_mg_regs_pcie_capabilities_u	pcie_capabilities ;
	ag_mg_regs_device_capabilities_u	device_capabilities ;
	ag_mg_regs_device_status_and_control_u	device_status_and_control ;
	ag_mg_regs_link_capabilities_u	link_capabilities ;
	ag_mg_regs_link_status_and_ctrl_u	link_status_and_ctrl ;
	ag_mg_regs_register			FILL9[4] ;
	ag_mg_regs_device_cap2_u	device_cap2 ;
	ag_mg_regs_device_ctrl_2_u	device_ctrl_2 ;
	ag_mg_regs_register			FILL10 ;
	ag_mg_regs_link_status_and_control_2_u	link_status_and_control_2 ;
	ag_mg_regs_register			FILL10a[27] ;
	ag_mg_regs_advanced_error_report_capability_header_u	advanced_error_report_capability_header ;
	ag_mg_regs_uncorrectable_error_status_u	uncorrectable_error_status ;
	ag_mg_regs_uncorrectable_error_mask_u	uncorrectable_error_mask ;
	ag_mg_regs_uncorrectable_error_severity_u	uncorrectable_error_severity ;
	ag_mg_regs_correctable_error_status_u	correctable_error_status ;
	ag_mg_regs_correctable_error_mask_u	correctable_error_mask ;
	ag_mg_regs_adv_error_cap_and_ctrl_u	adv_error_cap_and_ctrl ;
	ag_mg_regs_header_log_register0_u	header_log_register0 ;
	ag_mg_regs_head_log_register1_u	head_log_register1 ;
	ag_mg_regs_head_log_register2_u	head_log_register2 ;
	ag_mg_regs_head_log_register3_u	head_log_register3 ;
	ag_mg_regs_register			FILL11[5] ;
	ag_mg_regs_enhanced_capability_header_u	enhanced_capability_header ;
	ag_mg_regs_port_vc_capability_reg_1_u	port_vc_capability_reg_1 ;
	ag_mg_regs_port_vc_cap_reg_2_u	port_vc_cap_reg_2 ;
	ag_mg_regs_port_vc_control_status_u	port_vc_control_status ;
	ag_mg_regs_vc_resource_cap_reg_0_u	vc_resource_cap_reg_0 ;
	ag_mg_regs_vc_resource_control_reg_0_u	vc_resource_control_reg_0 ;
	ag_mg_regs_vc_resource_status_reg_0_u	vc_resource_status_reg_0 ;
	ag_mg_regs_register			FILL12[9] ;
	ag_mg_regs_enhanced_cap_header_u	enhanced_cap_header ;
	ag_mg_regs_data_select_reg_u		data_select_reg ;
	ag_mg_regs_data_register_u		data_register ;
	ag_mg_regs_pwr_budget_cap_u		pwr_budget_cap ;
	ag_mg_regs_pcie_enhanced_cap_header_u	pcie_enhanced_cap_header ;
	ag_mg_regs_pcie_ari_cap_control_u	ari_cap_control_u;

	ag_mg_regs_register		FILL13[922] ;

// ==> END OF ag_mg_regs_pcie_trans_layer_regs

	ag_mg_regs_link_core_id_u	link_core_id ;
	ag_mg_regs_link_status_u	link_status ;
	ag_mg_regs_interrupt_status_u	interrupt_status ;
	ag_mg_regs_performance_counter_interrupt_test_u	performance_counter_interrupt_test ;
	ag_mg_regs_interrupt_mask_u	interrupt_mask ;
	ag_mg_regs_register		FILL14[3] ;
	ag_mg_regs_test_control1_u	test_control1 ;
	ag_mg_regs_test_control2_u	test_control2 ;
	ag_mg_regs_test_data_u	test_data ;
	ag_mg_regs_register		FILL15 ;
	ag_mg_regs_memory_address_control_u	memory_address_control ;
	ag_mg_regs_memorydataload0_u	memorydataload0 ;
	ag_mg_regs_memorydataload1_u	memorydataload1 ;
	ag_mg_regs_memorydataload2_u	memorydataload2 ;
	ag_mg_regs_memorydataload3_u	memorydataload3 ;
	ag_mg_regs_memorydataload4_u	memorydataload4 ;
	ag_mg_regs_register		FILL16[14] ;
	ag_mg_regs_linkperformancecounterselect_u	linkperformancecounterselect ;
	ag_mg_regs_linkperformancecountercontrol_u	linkperformancecountercontrol ;
	ag_mg_regs_linkperformancecountersampleduration_u	linkperformancecountersampleduration ;
	ag_mg_regs_register		FILL17 ;
	ag_mg_regs_linkperformancecounter1_u	linkperformancecounter1 ;
	ag_mg_regs_linkperformancecounter1_test_u	linkperformancecounter1_test ;
	ag_mg_regs_linkperformancecounter_2_u	linkperformancecounter_2 ;
	ag_mg_regs_performancecounter2_test_u	performancecounter2_test ;
	ag_mg_regs_debug_status_u	debug_status ;
	ag_mg_regs_debugconfiguration_u	debugconfiguration ;
	ag_mg_regs_register		FILL18[22] ;
	ag_mg_regs_tx_configuration_u	tx_configuration ;
	ag_mg_regs_tx_link_status_u	tx_link_status ;
	ag_mg_regs_tx_interrupt_and_status_u	tx_interrupt_and_status ;
	ag_mg_regs_tx_interrupt_and_status_test_u	tx_interrupt_and_status_test ;
	ag_mg_regs_tx_interrupt_mask_u	tx_interrupt_mask ;
	ag_mg_regs_register		FILL19[3] ;
	ag_mg_regs_flow_control_update_timeout_value_u	flow_control_update_timeout_value ;
	ag_mg_regs_register		FILL20[3] ;
	ag_mg_regs_ack_nak_latency_threshold_u	ack_nak_latency_threshold ;
	ag_mg_regs_replaytimeoutthreshold_u	replaytimeoutthreshold ;
	ag_mg_regs_replay_number_status_u	replay_number_status ;
	ag_mg_regs_register		FILL21 ;
	ag_mg_regs_retry_buffer_pointer_u	retry_buffer_pointer ;
	ag_mg_regs_sequence_counter_u	sequence_counter ;
	ag_mg_regs_sequence_buffer_pointers_u	sequence_buffer_pointers ;
	ag_mg_regs_register		FILL22 ;
	ag_mg_regs_skiptimerthreshold_u	skiptimerthreshold ;
	ag_mg_regs_eies_counter_threshold_u	eies_counter_threshold ;
	ag_mg_regs_register		FILL23[42] ;
	ag_mg_regs_rx_configuration_u	rx_configuration ;
	ag_mg_regs_rx_status_u	rx_status ;
	ag_mg_regs_rx_interrupt_and_status_u	rx_interrupt_and_status ;
	ag_mg_regs_rx_interrupt_status_test_u	rx_interrupt_status_test ;
	ag_mg_regs_rx_interrupt_mask_u	rx_interrupt_mask ;
	ag_mg_regs_rx_ts_control_u	rx_ts_control ;
	ag_mg_regs_next_rcv_sequence_counter_u	next_rcv_sequence_counter ;
	ag_mg_regs_unknowndllp0_received_u	unknowndllp0_received ;
	ag_mg_regs_unknown_dllp1_received_u	unknown_dllp1_received ;
	ag_mg_regs_register		FILL24[55] ;
	ag_mg_regs_ltssm_configuration_1_u	ltssm_configuration_1 ;
	ag_mg_regs_ltssm_status_u	ltssm_status ;
	ag_mg_regs_ltssm_interupt_and_status_u	ltssm_interupt_and_status ;
	ag_mg_regs_ltssm_interrupt_test_u	ltssm_interrupt_test ;
	ag_mg_regs_ltssm_interrupt_mask_u	ltssm_interrupt_mask ;
	ag_mg_regs_register		FILl11[3] ;
	ag_mg_regs_ltssm_timer_threshold1_u	ltssm_timer_threshold1 ;
	ag_mg_regs_ltssm_timer_threshold2_u	ltssm_timer_threshold2 ;
	ag_mg_regs_ltssm_timer_threshold3_u	ltssm_timer_threshold3 ;
	ag_mg_regs_ltssm_threshold4_u	ltssm_threshold4 ;
	ag_mg_regs_ltssm_request_u	ltssm_request ;
	ag_mg_regs_ltssm_training_configuration_u	ltssm_training_configuration ;
	ag_mg_regs_ltssm_status_2_u	ltssm_status_2 ;
	ag_mg_regs_ltssm_rx_command_status_u	ltssm_rx_command_status ;
	ag_mg_regs_ltssm_tx_command_status_u	ltssm_tx_command_status ;
	ag_mg_regs_register		FILL25[3] ;
	ag_mg_regs_ltssm_gen_2_timer_threshold1_u	ltssm_gen_2_timer_threshold1 ;
	ag_mg_regs_lssm_gen_2_timer_threshold2_u	lssm_gen_2_timer_threshold2 ;
	ag_mg_regs_ltssm_gen2_speed_nfts_number_u	ltssm_gen2_speed_nfts_number ;
	
	ag_mg_regs_register		FILL26[41] ;

// ==> First part of ag_mg_regs_pcie_link_layer_regs

	ag_mg_regs_pipe_rst_en_u	pipe_rst_en ;
	ag_mg_regs_pipe_control_u	pipe_control ;
	ag_mg_regs_pipe_common_status_u	pipe_common_status ;
	ag_mg_regs_tx_emph_cntrl_g1_u	tx_emph_cntrl_g1 ;
	ag_mg_regs_tx_emph_cntrl_g2_u	tx_emph_cntrl_g2 ;
	ag_mg_regs_tx_emph_cntrl_g3_1_u	tx_emph_cntrl_g3_1 ;
	ag_mg_regs_tx_emph_cntrl_g3_2_u	tx_emph_cntrl_g3_2 ;
	ag_mg_regs_tx_mrgn_cntrl_g1_u	tx_mrgn_cntrl_g1 ;
	ag_mg_regs_tx_mrgn_cntrl_g2_u	tx_mrgn_cntrl_g2 ;
	ag_mg_regs_tx_mrgn_cntrl_g3_u	tx_mrgn_cntrl_g3 ;
	ag_mg_regs_tx_common_param_u	tx_common_param ;
	ag_mg_regs_rx_los_common_param_u	rx_los_common_param ;
	ag_mg_regs_rx_common_param_g1_1_u	rx_common_param_g1_1 ;
	ag_mg_regs_rx_common_param_g1_2_u	rx_common_param_g1_2 ;
	ag_mg_regs_rx_common_param_g2_1_u	rx_common_param_g2_1 ;
	ag_mg_regs_rx_common_param_g2_2_u	rx_common_param_g2_2 ;
	ag_mg_regs_rx_common_param_g3_1_u	rx_common_param_g3_1 ;
	ag_mg_regs_rx_common_param_g3_2_u	rx_common_param_g3_2 ;
	ag_mg_regs_register		FILL27[6] ;
	ag_mg_regs_rx__param_ln0_u	rx__param_ln0 ;
	ag_mg_regs_pipe_status_ln0_u	pipe_status_ln0 ;
	ag_mg_regs_rx__param_ln1_u	rx__param_ln1 ;
	ag_mg_regs_pipe_status_ln1_u	pipe_status_ln1 ;

	ag_mg_regs_register		FILL28[612] ;

// ==> END OF ag_mg_regs_pcie_pipe_layer_regs

	ag_mg_regs_tl_gen_debug_reg_u	tl_gen_debug_reg ;
	ag_mg_regs_tlsb_sig_u	tlsb_sig ;
	ag_mg_regs_completion_time_out_register0_u	completion_time_out_register0 ;
	ag_mg_regs_completion_time_out_register1_u	completion_time_out_register1 ;
	ag_mg_regs_register		FILL29[12] ;
	ag_mg_regs_vc0_ingress_fc_control_reg_u	vc0_ingress_fc_control_reg ;
	ag_mg_regs_vc0_posted_prog_credit_freed_threshold_u	vc0_posted_prog_credit_freed_threshold ;
	ag_mg_regs_vc0_nonposted_programmable_credit_freed_threshold_u	vc0_nonposted_programmable_credit_freed_threshold ;
	ag_mg_regs_vc0_ingress_fc_posted_credits_received_u	vc0_ingress_fc_posted_credits_received ;
	ag_mg_regs_vc0_ingress_fc_nonposted_credits_received_u	vc0_ingress_fc_nonposted_credits_received ;
	ag_mg_regs_vc0_ingress_fc_posted_credits_allocated_u	vc0_ingress_fc_posted_credits_allocated ;
	ag_mg_regs_vc0_ingress_fc_nonposted_credits_allocated_u	vc0_ingress_fc_nonposted_credits_allocated ;
	ag_mg_regs_register		FILL30[9] ;
	ag_mg_regs_vc0_egress_posted_credits_consumed_u	vc0_egress_posted_credits_consumed ;
	ag_mg_regs_vc0_egress_nonposted_credits_consumed_u	vc0_egress_nonposted_credits_consumed ;
	ag_mg_regs_vc0_egress_completion_credits_consumed_u	vc0_egress_completion_credits_consumed ;
	ag_mg_regs_vc0_egress_posted_credit_limit_u	vc0_egress_posted_credit_limit ;
	ag_mg_regs_vc0_egress_nonposted_limit_u	vc0_egress_nonposted_limit ;
	ag_mg_regs_vc0_egress_completion_limit_u	vc0_egress_completion_limit ;
	ag_mg_regs_vc0_egress_flow_control_timeout_timer_u	vc0_egress_flow_control_timeout_timer;

// ==> END OF ag_mg_regs_pcie_link_layer_regs


} ag_mg_regs_pcie_reg_s ;

/*
* Recommended C syntax for typical usage :
*   volatile ag_mg_regs_pcie_reg_s *pcie_regs =
*       (volatile ag_mg_regs_pcie_reg_s *)AG_MG_REGS_PCIE_BASE;
*/
#endif

#endif
/* 
 * $Log: ag_mg_regs_pcie.h,v $
 * Revision 1.1  2012/04/18 18:08:26  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


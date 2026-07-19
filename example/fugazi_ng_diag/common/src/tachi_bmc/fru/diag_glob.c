/* $Id: diag_glob.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_glob.c,v $
 *
 *      File:   diag_glob.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Diag infra structure 
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

char parm_count[]      =   "Count";
char parm_len[]        =   "Length";
char parm_start[]      =   "Start";
char parm_end[]        =   "End";
char parm_data[]       =   "Data";
char parm_mode[]       =   "Mode";
char parm_mode_s[]     =   "Normal/Complement/Increment/Random/All";
char parm_normal[]     =   "Normal";
char parm_random[]     =   "Random";
char parm_increment[]  =   "Increment";
char parm_complement[] =   "Complement";
char parm_all[]        =   "All";
char parm_sleep[]      =   "Sleep";
char parm_revision[]   =   "Revision";


char parm_start_addr[] =   "Start Address";
char parm_end_addr[]   =   "End Address";
char parm_seed_data[]  =   "Seed Data";
char parm_data_mode[]  =   "Data Mode";
char parm_inc_data[]   =   "Increment Data";

char parm_run_cnt[]      =   "RunCount";
char parm_verbose[]      =   "Verbose";
char parm_stop_on_fail[] =   "stop_on_fail";
char parm_debug[]        =   "Debug";
char parm_err_cnt[]      =   "ErrCount";
char parm_loopback[]     =   "Loopback";
char parm_fploop[]       =   "FPloop";
char parm_bploop[]       =   "BPloop";
char parm_loopback_s[]   =   "/Mac/Phy/Line/Ext/";
char parm_loopback_b[]   =   "/Mac/Phy/";
char parm_loopback_p[]   =   "/Phy/Ext/";
char parm_line[]   	 =   "Line";
char parm_mac[]          =   "Mac";
char parm_serdes[]       =   "Serdes";
char parm_phy[]          =   "Phy";
char parm_ext[]          =   "Ext";
char parm_cable[]         =  "Cable";
char parm_portmask[]     =   "PortMask";
char parm_pagemask[]     =   "PageMask";
char parm_fpport[]       =   "FPport";
char parm_bpport[]       =   "BPport";
char parm_instmask[]     =   "InstMask";
char parm_himask[]       =   "HiMask";
char parm_nimask[]       =   "NiMask";
char parm_mvphy_addr[]   =   "Mvphy";
char parm_i2c_addr[]     =   "i2caddr";
char parm_flip_pol[]     =   "RxFlipPol";
char parm_tx_inv_pol[]   =   "TxInvPol";
char parm_rx_inv_pol[]   =   "RxInvPol";
char parm_nfs[]          =   "NFS";
char parm_extended[]     =   "Extended";
char parm_local[]     	 =   "Local";
char parm_ipg[]     	 =   "IPG";
char parm_speed_s[]      =   "/Lo/Med/Hi/All/";
char parm_speed[]        =   "Speed";
char parm_hi[]		 =   "Hi";
char parm_lo[]		 =   "Lo";
char parm_med[]		 =   "Med";
char parm_memtype[]	 =   "MemType";
char parm_rdwacc[]	 =   "micacc";
char parm_laneswap[]	 =   "Laneswap";
char parm_delta[]	 =   "Delta";
char parm_flag[]	 =   "Flag";
char parm_errthresh[]    =   "ErrThresh";
char parm_sfp_display[]    =   "SFP_Display";
char parm_hifloop[]       =   "HIFloop";
char parm_nifloop[]       =   "NIFloop";
char parm_hifport[]       =   "HIFport";
char parm_nifport[]       =   "NIFport";
char parm_linerate[]       =   "Linerate";

#ifdef HOST_GOODING
char parm_subslot[] = "subslot";
#endif

int diag_debug = 0;

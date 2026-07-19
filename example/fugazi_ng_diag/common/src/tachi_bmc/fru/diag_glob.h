/* $Id: diag_glob.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_glob.h,v $
 *
 *      File:   diag_glob.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/
#ifndef _DIAG_GLOB_H_
#define _DIAG_GLOB_H_
extern char parm_count[];
extern char parm_len[];
extern char parm_mode[];
extern char parm_mode_s[];
extern char parm_normal[];
extern char parm_random[];
extern char parm_increment[];
extern char parm_complement[];
extern char parm_all[];
extern char parm_sleep[];
extern char parm_revision[];

extern char parm_start[];
extern char parm_end[];
extern char parm_data[];
extern char parm_delta[];

extern char parm_start_addr[];
extern char parm_end_addr[];
extern char parm_seed_data[];
extern char parm_data_mode[];
extern char parm_inc_data[];

extern char parm_run_cnt[];
extern char parm_verbose[];
extern char parm_stop_on_fail[];
extern char parm_debug[];
extern char parm_err_cnt[];
extern char parm_loopback[];
extern char parm_speed[];
extern char parm_speed_s[];
extern char parm_fpport[];
extern char parm_bpport[];
extern char parm_fploop[];
extern char parm_bploop[];
extern char parm_loopback_s[];
extern char parm_loopback_b[];
extern char parm_loopback_p[];
extern char parm_mac[];
extern char parm_serdes[];
extern char parm_phy[];
extern char parm_ext[];
extern char parm_line[];
extern char parm_cable[];
extern char parm_portmask[];
extern char parm_pagemask[];
extern char parm_instmask[];

extern char parm_himask[];
extern char parm_nimask[];
extern char parm_mvphy_addr[];
extern char parm_i2c_addr[];
extern char parm_flip_pol[];
extern char parm_rx_inv_pol[];
extern char parm_tx_inv_pol[];
extern char parm_nfs[];
extern char parm_extended[];
extern char parm_local[];
extern char parm_ipg[];
extern char parm_memtype[];
extern char parm_rdwacc[];
extern char parm_laneswap[];

extern char parm_hi[];
extern char parm_lo[];
extern char parm_med[];
extern char parm_flag[];
extern char parm_hifloop[];
extern char parm_nifloop[];
extern char parm_hifport[];
extern char parm_nifport[];
extern char parm_subslot[];
extern char parm_linerate[];

#ifdef HOST_GOODING
extern char parm_subslot[];
#endif

extern char parm_errthresh[];
extern char parm_sfp_display[];
#endif // _DIAG_GLOB_H_

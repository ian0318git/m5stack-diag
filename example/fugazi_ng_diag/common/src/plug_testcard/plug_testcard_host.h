/* $Id: plug_testcard_host.h,v 1.4 2019/11/25 08:55:51 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_host.h,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_host.h - Header file for Pluggable Testcard Host 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_TC_HOST__
#define __PLUG_TC_HOST__

extern int plug_tc_host_gephy_set_auto_neg(void);
extern int plug_tc_host_gephy_set_test_speed(int);
extern int plug_tc_host_ge_send_packet_util(int);
extern int plug_tc_host_gephy_set_txtype_util(int);
extern void plug_tc_host_get_nvme_info(int, char *);
extern void plug_tc_host_get_pcie_dev_info(int, uint *,
                                           uint *, uint *, uint *);
extern int plug_tc_host_pcie_present(int); 
extern int plug_tc_host_check_nvme_existence(int);
#endif

/*-------------------------------------------------
$Log: plug_testcard_host.h,v $
Revision 1.4  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.3  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.2  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.2  2018/11/13 07:10:28  hondwang
Base on PRRQ comment add gephy_set_test_mode back

Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
pluggable common code re-instruct add and remove files




*/


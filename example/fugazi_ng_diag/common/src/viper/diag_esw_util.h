 /* $Id: diag_esw_util.h,v 1.2 2018/08/06 02:31:50 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_esw_util.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : diag_esw_util.h 
 * Description: Diagnostic ethernet switch utility header
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_ESW_UTIL__
#define __DIAG_ESW_UTIL__

extern int diag_esw_reg_rd_util(void);
extern int diag_esw_reg_wr_util(void);
extern int diag_esw_phy_reg_rd_util(void);
extern int diag_esw_phy_reg_wr_util(void);
extern int diag_esw_set_allport_forward_util(void);
extern int diag_esw_adjust_port_vod_util(void);
extern int diag_smi_c45_rd_util(void);
extern int diag_smi_c45_wr_util(void);
extern int diag_esw_set_allport_forward_util(void);
extern int diag_esw_config_vlan_profile(void);
extern int esw_set_1k_testmode_util(void);


#define GEPHY_MAX_RETRY   100
#define OCR_TESTMODE_NORMAL          0
#define ESW_REG_PAGE_252  0xFC

#endif   /* __DIAG_ESW_UTIL__ */

/*------------------------------------------------------------------
$Log: diag_esw_util.h,v $
Revision 1.2  2018/08/06 02:31:50  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/03/05 08:54:21  harrchan
Initial hydra application code base

Revision 1.1.2.1  2018/02/27 08:06:39  harrchan
Initial viper application code base


$Endlog$
*/

/* $Id: draco_rbcp_main.h,v 1.2 2016/01/21 01:50:03 olin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/draco/draco_rbcp_main.h,v $
 *------------------------------------------------------------------
 * Filename: draco_rbcp_main.h
 *
 * Description: The RBCP main code header file
 * Author: Times Huang
 *
 * Copyright (c) 2016 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef DRACO_RBCP_MAIN_H_
#define DRACO_RBCP_MAIN_H_

#define MAX_NUM_DRACO_SLOTS     (2)

#define DRACO_BP_GE0            (0)
#define DRACO_BP_GE1            (1)

extern long build_draco_rbcp_menu(int);
extern int draco_rbcp_con_sw_bmc(void);
extern int draco_rbcp_con_sw_intel(void);
extern int draco_rbcp_bmc_console_switch(void);
extern int draco_rbcp_intel_console_switch(void);
extern int draco_rbcp_heartbeat_test(int);
extern int draco_rbcp_registration_test(int);
extern void clear_draco_regis_done_flag(int);

#endif /* DRACO_RBCP_MAIN_H_ */
/*------------------------------------------------------------------
 * $Log: draco_rbcp_main.h,v $
 * Revision 1.2  2016/01/21 01:50:03  olin2
 * Collapse Draco-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2015/07/27 02:05:51  olin2
 * Initial commit code for Draco
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

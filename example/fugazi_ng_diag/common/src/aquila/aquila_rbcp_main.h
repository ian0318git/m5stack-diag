/* $Id: aquila_rbcp_main.h,v 1.2 2017/03/21 08:41:55 olin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/aquila/aquila_rbcp_main.h,v $
 *------------------------------------------------------------------
 * Filename: aquila_rbcp_main.h
 *
 * Description: The RBCP main code header file
 * Author: Times Huang
 *
 * Copyright (c) 2016-2017 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef AQUILA_RBCP_MAIN_H_
#define AQUILA_RBCP_MAIN_H_

#define MAX_NUM_AQUILA_SLOTS     (2)

#define AQUILA_BP_GE0            (0)
#define AQUILA_BP_GE1            (1)

extern long build_aquila_rbcp_menu(int);
extern int aquila_rbcp_con_sw_bmc(void);
extern int aquila_rbcp_con_sw_intel(void);
extern int aquila_rbcp_bmc_console_switch(void);
extern int aquila_rbcp_intel_console_switch(void);
extern int aquila_rbcp_lsi_console_switch(void);
extern int aquila_rbcp_heartbeat_test(int);
extern int aquila_rbcp_registration_test(int);
extern void clear_aquila_regis_done_flag(int);

#endif /* AQUILA_RBCP_MAIN_H_ */
/*------------------------------------------------------------------
 * $Log: aquila_rbcp_main.h,v $
 * Revision 1.2  2017/03/21 08:41:55  olin2
 * Collapse Aquila-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2016/10/27 08:06:50  olin2
 * Support LSI console switch utility
 *
 * Revision 1.1.2.1  2016/04/12 06:30:12  olin2
 * Initial commit code for Aquila
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

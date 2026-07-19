/* $Id: platform_cpu.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_cpu.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_cpu.h
 * Description: Header file of Platform_CPU Library.
 *
 * Copyright (c) 2018-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

#define ENHANCE_ERROR_MSG_RDY   1
#define FUGAZI_CPU_NUM          24
#define CPU_STRESS_LOG          "/tmp/cpu_core_log"
#define CPU_STRESS_RLT          "/tmp/cpu_core_rlt"

/* Common defines */

/* Register map */

/* Type struct of device bus configure registers */
typedef struct devbus_conf_t_ {
    char *desc;
    int  bus_num;
    int  rd_param;
    int  wr_param;
    int  polarity;
    int  ignore;
} devbus_conf_t;



/* Externs */
extern int cpu_core_test(int);
extern int build_cpu_test_menu(int);

#endif /* __PLATFORM_CPU_H__ */

/*-------------------------------------------------
 * $Log: platform_cpu.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */


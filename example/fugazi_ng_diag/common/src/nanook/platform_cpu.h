 /* $Id: platform_cpu.h,v 1.2 2019/12/11 10:10:33 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_cpu.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_cpu.h
 * Description: Header file of CPU Library.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

/* Common defines */

#define NANOOK_CPU_NUM 4

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
extern int nanook_cpu_ondie_temp (int opt);

#endif /* __PLATFORM_CPU_H__ */

/*------------------------------------------------------------------
$Log: platform_cpu.h,v $
Revision 1.2  2019/12/11 10:10:33  lucywang
Merged Nanook to main trunk


$Endlog$
*/


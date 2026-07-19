 /* $Id: platform_cpu.h,v 1.2 2018/08/06 02:31:52 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/platform_cpu.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_cpu.h
 * Description: Header file of Viper CPU Library.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

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
extern int viper_cpu_ondie_temp (int opt);

#endif /* __PLATFORM_CPU_H__ */

/*------------------------------------------------------------------
$Log: platform_cpu.h,v $
Revision 1.2  2018/08/06 02:31:52  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/06/14 01:33:02  harrchan
Remove TSN keyword

Revision 1.1.2.3  2018/05/03 08:48:40  lucywang
Added Temperature to System Information

Revision 1.1.2.2  2018/03/28 09:18:13  lucywang
Added CPU test

Revision 1.1.2.1  2018/02/27 08:06:52  harrchan
Initial viper application code base

$Endlog$
*/


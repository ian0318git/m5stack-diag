 /* $Id: platform_cpu.h,v 1.3 2020/10/07 08:20:48 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_cpu.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_cpu.h
 * Description: Header file of CPU Library.
 *
 * Copyright (c) 2018-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_CPU_H__
#define __PLATFORM_CPU_H__

/* Common defines */

#define TABEI_CPU_NUM 8
#define PMTM_L_CPU_NUM 4

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
extern int tabei_cpu_ondie_temp (int opt);

#endif /* __PLATFORM_CPU_H__ */

/*------------------------------------------------------------------
$Log: platform_cpu.h,v $
Revision 1.3  2020/10/07 08:20:48  kehuang2
CSCvv99413: Collapse Promethium-L into main trunk

Revision 1.2  2019/10/17 02:16:26  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.3  2018/11/16 05:42:12  olin2
Clean up code

Revision 1.1.2.2  2018/10/16 07:58:11  harrchan
CPU multi core test

Revision 1.1.2.1  2018/10/02 01:50:03  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/


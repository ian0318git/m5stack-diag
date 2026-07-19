/* $Id: hightower_sub6.h,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/hightower_sub6.h,v $
 *********************************************************************
 *
 * hightower_sub6.h -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

/*------------------------------------------------------------------
 *
 * hightower_sub6.h - specific APIs header file for Hightower platform
 *
 * May 2019, markzha
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _HIGHTOWER_SUB6_H_
#define _HIGHTOWER_SUB6_H_

#include <stdio.h>

#define GPIO20 32
#define GPIO21 33
#define GPIO22 34
#define GPIO23 35
#define GPIO24 36
#define GPIO25 37
#define GPIO26 38
#define GPIO27 39
#define GPIO32 44
#define GPIO34 46
#define GPIO36 48
#define GPIO44 56
#define GPIO47 59
#define GPIO48 60
#define GPIO49 61
#define GPIO50 62
#define GPIO51 63
#define GPIO52 64
#define GPIO53 65
#define GPIO54 66
#define GPIO69 81
#define GPIO73 85
#define GPIO74 86
#define GPIO75 87
#define GPIO82 94

#define CP_MPP0 GPIO20
#define CP_MPP1 GPIO21
#define CP_MPP6 GPIO26
#define CP_MPP12 GPIO32
#define CP_MPP16 GPIO36
#define CP_MPP24 GPIO44
#define CP_MPP27 GPIO47
#define CP_MPP62 GPIO82
#define CP_MPP31 GPIO51
#define CP_MPP54 GPIO74

#define CPLD_CPU_INT_L                 CP_MPP0
#define USB_MUX_DEBUG_EN               CP_MPP1
#define THERM_CPU_INT_L                CP_MPP12
#define SIM0_DETECT_L                  CP_MPP27
#define SIM1_DETECT_L                  CP_MPP24
#define DDR4_CPU_ALERT_L               CP_MPP62
#define CPU_TO_CPLD_STATUS_0           CP_MPP31
#define CPU_TO_CPLD_STATUS_1           CP_MPP54
#define SIM_SELECT                     CP_MPP16

#define INFRA_ERR_HANDLE(msg, rc, terminate)                          \
{                                                                             \
    if (rc) {                                                                 \
        printf("Error: %s - %s line: %d\n", msg, __FUNCTION__, __LINE__);\
                                                                              \
        if (terminate == 1) {                                                 \
            return (-1);                                                      \
        }                                                                     \
    }                                                                         \
}

#define HOST_USB0_DEVINFO                   "1-1"
#define HOST_USB1_2P0_DEVINFO               "1-1"
#define HOST_USB1_3P0_DEVINFO               "2-1"
#define DIAG_FOLDER                         "/diag"

#define ETH_PHY_3310_GE_UP           "ifconfig eth0 up > /dev/null"


extern int ht_init(void);
extern int highrise_reset_act2_chip(void);
extern int highrise_unreset_act2_chip(void);

#endif

/*********************************************************************
 * $Log: hightower_sub6.h,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */


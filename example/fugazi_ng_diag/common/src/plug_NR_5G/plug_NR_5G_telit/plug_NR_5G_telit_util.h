/*
 * $Id: plug_NR_5G_telit_util.h,v 1.2 2021/06/02 02:56:20 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_util.h,v $
 *------------------------------------------------------------------
 *
 * plug_NR_5G_telit_util.h - Header File for Pluggable Telit
 *                         Utility
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_NR_5G_TELIT_UTIL_H__
#define __PLUG_NR_5G_TELIT_UTIL_H__

#define AT_COMMAND_UTIL_DELAY                 (1000)
#define TESTMSG_BUFSZ                     (256)

typedef enum {
    OPT_READ,
    OPT_WRITE
} reg_util_opt_t;

typedef enum {
    OPT_OFF,
    OPT_ON 
} led_util_opt_t;

typedef enum {
    OPT_GREEN = 0,
    OPT_YELLOW 
} led_color_opt_t;

typedef enum {
    OPT_DISABLE,
    OPT_ENABLE
} debug_usb_util_opt_t;

extern int plug_NR_5g_telit_util(void);
extern int plug_NR_5g_telit_modem_temp_util(int);

#endif /* __PLUG_NR_5G_TELIT_UTIL_H__ */

/*********************************************************************
 * $Log: plug_NR_5G_telit_util.h,v $
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.2  2020/12/02 03:57:23  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */


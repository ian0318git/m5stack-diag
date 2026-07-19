/* $Id: diag_temp_sensor_lib.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_temp_sensor_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_TEMP_SENSOR_LIB_H__
#define __DIAG_TEMP_SENSOR_LIB_H__

/* Externs */
extern int diag_ts_dev_create(dev_max31730_object_t *, n2g_i2c_if_t *);

#endif   /* __DIAG_TEMP_SENSOR_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_temp_sensor_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

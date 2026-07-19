/* $Id: diag_temperature_sensor_lib.h,v 1.2 2013/10/08 08:48:29 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_temperature_sensor_lib.h,v $ 
 *------------------------------------------------------------------
 * diag_temperature_sensor_lib.h 
 * 
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_TEMPERATURE_LIB_H__
#define __DIAG_TEMPERATURE_LIB_H__

extern int temperature_sensor_utility(int); 
extern int temperature_sensor_register_test(void);
extern int temperature_sensor_id_check_test(void);
extern long show_temperature(void);
extern long dump_TMP421AID_registers(void);
extern dev_object_t *get_tmp421_obj(void);

#endif
/*-------------------------------------------------
 * $Log: diag_temperature_sensor_lib.h,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:18  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:53  kuangik
 * Add for the first time
 *
 * Revision 1.4  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/03/26 07:19:57  kody
 * Modify and add TMP421 temperature sensor test code.
 *
 * Revision 1.1  2012/02/10 07:03:34  leslie
 * Add Woodlawn temperature sensor lib header file.
 * 
 *
 * $Endlog$
 *-------------------------------------------------
 */

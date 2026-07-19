/* $Id: diag_temp_sensor_util.h,v 1.3 2017/03/30 08:30:54 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_temp_sensor_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_util.h - Header file for I2C Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_TEMP_SENSOR_UTIL__
#define __DIAG_TEMP_SENSOR_UTIL__

typedef struct {
    unsigned int addr;
    char *desc;
} temp_desc;

#define TS_TEMP_MAX         0x7D00      /* 0x7D0: 125 Celcius */
#define TS_TEMP_RESOLUTION  (0.0625)    /* One LSB: 0.0625 Celcius */
#define BOARD_TEMP_SIZE             (sizeof(board_temp)/sizeof(temp_desc))


extern int diag_temp_sensor_util(void);
extern int diag_show_temperature(void);
extern int diag_show_temperature_lib(void);
extern temp_desc board_temp[];

#endif /* __DIAG_TEMP_SENSOR_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_temp_sensor_util.h,v $
Revision 1.3  2017/03/30 08:30:54  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2016/11/04 19:08:54  benchen2
Modify Enhanced error message

Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.5  2015/12/04 09:15:10  benchen2
for fix cdets CSCux41949

Revision 1.1.2.4  2015/09/21 13:09:16  tirawan
Display temperature sensor and FPGA version during boot up

Revision 1.1.2.3  2015/08/21 11:31:21  benchen2
add temperature sensor utility

Revision 1.1.2.2  2015/07/31 07:43:23  hondwang
temp sensor r, w

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/

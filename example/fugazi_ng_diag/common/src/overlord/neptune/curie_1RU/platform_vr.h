/* $Id: platform_vr.h,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_vr.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_vr.h       
 *
 * Description: Voltage Regulator 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_VR_H__
#define __PLATFORM_VR_H__

#define    VR_OPERATION                       0x1
#define    VR_CLEAR_FAULTS                    0x03
#define    VR_CAPABILITY                      0x19
#define    VR_VOUT_MODE                       0x20
#define    VR_VOUT_MARGIN_HIGH                 0x25
#define    VR_VOUT_MARGIN_LOW                  0x26
#define    VR_STATUS_BTYE                       0x78
#define    VR_STATUS_WORD                       0x79
#define    VR_STATUS_TEMPERATURE                0x7D
#define    VR_STATUS_CML                        0x7E
#define    VR_STATUS_MFR_SPECIFIC               0x80
#define    VR_READ_VIN                          0x88
#define    VR_READ_IIN                          0x89
#define    VR_READ_VOUT                         0x8B
#define    VR_READ_IOUT                         0x8C
#define    VR_READ_TEMPERATURE_1                0x8D
#define    VR_READ_TEMPERATURE_2                0x8E
#define    VR_READ_POUT                         0x96
#define    VR_READ_PIN                          0x97
#define    VR_PMBUS_REVISION                    0x98
#define    VR_MFR_MODEL                         0x9A
#define    VR_MFR_REVISION    /*byte conunt = 2 */0x9B
#define    VR_WRITE_REGISTER_PROCESS_CALL         0xD0
#define    VR_READ_REGISTER_PROCESS_CALL          0xD1
#define    VR_GAMER_COMMAND                       0xD2
#define    VR_SET_POINTER                         0xD3
#define    VR_GET_POINTER                         0xD4
#define    VR_WRITE_REGISTER                      0xD5
#define    VR_SET_I2C                           0xD6
#define    VR_READ_EFFICIENCY                     0xD7
#define    VR_MASK_STATUS_WORD                    0xD8
#define    VR_MASK_TEMPERATUE                     0xD9
#define    VR_MASK_CML                            0xDA
#define    VR_MASK_MANUFACTURER                  0xDB

/* Functions prototype */

#endif /* __PLATFORM_VR_H__ */

/*------------------------------------------------------------------
$Log: platform_vr.h,v $
Revision 1.2  2019/08/06 06:56:14  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2018/06/22 08:05:20  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:38  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:25:01  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2018/02/02 06:45:32  leschen
Initial check in to support IR3570 voltage regulator utility.


*
*/

/* $Id: platform_30w_poe.h,v 1.1 2013/05/09 05:42:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_30w_poe.h,v $
 *------------------------------------------------------------------
 *
 * File name: platform_30w_poe.h
 *
 * Description: PoE Diagnostic Test (Porting from Sunridges)
 *
 * April 2010 by tirawan 
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *------------------------------------------------------------------
 */

#ifndef __OVERLORD_30V_POE_H__
#define __OVERLORD_30V_POE_H__


/* from Overlord HFS EDC-999274 section 4.4.2.1	"External Device Reset Register" */

#define FPGA_EXT_DEV_RST_REG_OFFSET 0x4
#define MB_IOFPGA_POE_RESET         0x80 
#define MB_IOFPGA_POE_PSU_INT_MASK_ALL 0x3FF
#define MB_IOFPGA_POE_PSU_INT_MASK_POE 0x200
/* from Overlord HFS */

#define POE_ERR_BUF_SIZE                    500

#define POE_INTR_TIMEOUT                    4000

#define POE_ALERT_RESPONSE_I2C_ADDR         0x0C
#define POE_P50_P03_I2C_ADDR                0x20
#define POE_BCAST_I2C_ADDR                  0x30
#define POE_ILP_EEPROM_I2C_ADDR             0x50

#define MAX_PORTS_PER_ILP                   2

extern int build_30w_poe_menu(int);
extern void ilp_int_handler(void);  
extern int is_poe_present(char *);

#endif /* __OVERLORD_30V_POE_H__ */

/******** History ******** 
$Log: platform_30w_poe.h,v $
Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.3  2013/03/17 02:04:14  mcharon
support command line for testing 30w poe

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: diag_plat_cookie.h,v 1.5 2017/04/05 03:46:19 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_plat_cookie.h,v $
 *------------------------------------------------------------------
 *
 * diag_plat_cookie.h - Header file for Platform cookie
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2015-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_PLAT_COOKIE__
#define __DIAG_PLAT_COOKIE__

#define PLATFORM_BUFF_SIZE                      (259)
#define CONTROL_TYPE_LEN                        (20)
#define PRODUCT_NAME_LEN                        (256)
#define PRODUCT_SERIAL_LEN                      (20)
#define VID_LEN                                 (20)
#define MB_PID_FILE                             "/tmp/mb_pid"
#define MB_POE_SKU                              "/tmp/POE_SKU"

#define COOKIE_FIELD_LEN					    (20)
#define FAB_VERSION_TYPE                        (0x02)
#define PCB_REVISION_TYPE                       (0x42)
#define P1B_PCB_REVISION_VAL                    "01"
#define P1B_FAB_FIELD_VAL                       (0x2)
#define P2A_PCB_REVISION_VAL                    "02"
#define P2A_FAB_FIELD_VAL                       (0x3)
#define P2B_PCB_REVISION_VAL_05                 "05"
#define P2B_PCB_REVISION_VAL_0A                 "0A"
#define P2B_FAB_FIELD_VAL                       (0x4)
#define P2C_PCB_REVISION_VAL                    "01"
#define P2C_FAB_FIELD_VAL                       (0x5)

extern int alter_mb_cookie(void);
extern int alter_poe_cookie(void);
extern int alter_nim_cookie(int);
extern int alter_raid_cookie(void);
extern void init_cookie_4_default_x(int, int, uchar *, int);
extern int read_eeprom_block(unsigned int, unsigned int, unsigned char *);
extern ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
extern int get_cookie_pid(int, int, unsigned char *, char *);

extern int smartchip(int);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial(uchar *, char *, uchar);

int get_pid(uchar *, char *);
int print_cookie(int, char **);
extern int is_poe_sku(void); 
extern void get_mb_pid(char *);
extern int get_ngio_mac_addr(int, int, uchar *);
extern int get_cookie_field_val(int, char *);
extern boolean fx3_switch_usb(void);
extern int get_board_ver(void);

#endif /* __DIAG_PLAT_COOKIE__ */

/*---------------------------------------------------------------
$Log: diag_plat_cookie.h,v $
Revision 1.5  2017/04/05 03:46:19  kodko
CSCvd79127: Adds a PCB revision type for P2B.

Revision 1.4  2017/02/05 01:05:22  kodko
Get the Fab Version and PCB Revision cookie field values to distinguish the board type and do the USB3.0/USB2.0 or USB3.0 only test.

Revision 1.3  2016/06/06 18:30:40  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.8  2016/01/18 07:02:28  alpeng
update cookie info for read mac

Revision 1.1.2.7  2015/12/29 12:31:06  alpeng
support get_mb_pid for check MB sku

Revision 1.1.2.6  2015/11/02 10:22:55  tirawan
Add PoE Cookie Utility

Revision 1.1.2.5  2015/10/28 07:55:04  benchen2
add raid act2 cookies utility

Revision 1.1.2.4  2015/09/30 09:15:29  benchen2
add poe act2

Revision 1.1.2.3  2015/07/31 10:39:59  alpeng
first check in for testcard

Revision 1.1.2.2  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/

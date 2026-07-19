/* $Id: platform_tam_cookie.h,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_tam_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platfrom_tam_cookie.h - copy from tachi bmc 
 * diag_plat_cookie.h - Header file for Platform cookie
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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
extern int alter_nim_cookie(void); 
extern int alter_pim_cookie(void);
extern int alter_raid_cookie(void);
extern void init_cookie_4_default_x(int, int, uchar *, int);
extern ushort get_cookie_id(int, int, uchar *, uint16_t *, char *);
extern int get_cookie_pid(int, int, unsigned char *, char *);

extern int smartchip(int);
extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial(uchar *, char *, uchar);

int get_pid(uchar *, char *);
int print_cookie(int, char **);
extern int is_poe_sku_test(void); 
extern void get_mb_pid(char *);
extern int get_ngio_mac_addr(int, int, uchar *);
extern int get_cookie_field_val(int, char *);
extern boolean fx3_switch_usb(void);
extern int get_board_ver(void);
extern int get_plat_sku_cookie(void);

#endif /* __DIAG_PLAT_COOKIE__ */

/*-------------------------------------------------
 * $Log: platform_tam_cookie.h,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.4  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.3  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:28  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */


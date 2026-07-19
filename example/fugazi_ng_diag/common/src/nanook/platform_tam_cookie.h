/* $Id: platform_tam_cookie.h,v 1.2 2019/12/11 10:10:35 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/platform_tam_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_tam_cookie.h - platform tam library header files
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#define QUACK_RETRY                                     8
#define ACT2_RESET_UNRESET_DELAY                        (500)
#define ACT2_UNRESET_DELAY                              (5000)
/*#define PLATFORM_BUFF_SIZE                      (259)
#define CONTROL_TYPE_LEN                        (20)
#define PRODUCT_NAME_LEN                        (256)
#define PRODUCT_SERIAL_LEN                      (20)
#define VID_LEN                                 (20)
#define MB_PID_FILE                             "/tmp/mb_pid"
#define MB_POE_SKU                              "/tmp/POE_SKU"*/

extern void i2c_act2_reset(sc_context *);
extern int i2c_act2_write_bytes(sc_context *, char *, int);
extern int i2c_act2_read_bytes(sc_context *, char *);
extern void get_mb_pid(char *);

/*---------------------------------------------------------------
$Log: platform_tam_cookie.h,v $
Revision 1.2  2019/12/11 10:10:35  lucywang
Merged Nanook to main trunk


$Endlog$
*/

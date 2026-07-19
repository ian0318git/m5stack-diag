/* $Id: mb_tests.h,v 1.5 2018/05/09 06:53:12 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2016, Sofian Teja adapted from Xformers
 *
 * Copyright (c) 2008-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MB_TESTS_H_
#define _MB_TESTS_H_


/*
 * Used in sub menu for mb tests.
 */
/* For VDSL Port */
#define GIGA_PORT_3_MAC_CTL_REG2   0xf2133e08
#define GIGA_PORT_3_ADDR           0xf2133e0c
#define RESET_VAL                  0x0000
#define DIS_AN_EN_FORCE_1G_SPD     0x8042
#define DIS_AN_1G_SPD              0x9040
#define CLEAR_RESERVED_BITS        0xFC7F
/* For USB test */
#define USB2                       2 
#define USB3                       3
#define DELAY_SYSCMD 200
#define DELAY_USBHUBCMD 1000
#define DELAY_USBCMD 5000   /* Wait for 5 sec until sys stable */
#define UEVENT_BUFFER_SIZE      1024
/* USB0 */
#define USB_POWER_OFF_CMD              "devmem 0xfa001104 32 0x0000"
#define USB_POWER_ON_CMD               "devmem 0xfa001104 32 0x0001"
#define USB_20_CUSTOM_REG1_CMD         "devmem 0xf212292c 32 0x0440"
#define USB_20_CUSTOM_REG2_CMD         "devmem 0xf2122930 32 0x80F0"
#define USB_MISC_CTRL_1_REG_CMD        "devmem 0xf21229cc 32 0x0651"
#define USB_20_CTRL_REG_CMD            "devmem 0xf2122920 32 0x4064"
#define USB_30_CUSTOM_REG1_CMD         "devmem 0xf212292c 32 0x0040"
#define USB_30_CUSTOM_REG2_CMD         "devmem 0xf2122930 32 0x00f0"
#define USB_30_CTRL_REG_CMD            "devmem 0xf2122920 32 0x4060"
#define USB0_CUSTOM_REG1                    (0xF212292C)
#define USB0_CUSTOM_REG2                    (0xF2122930)
#define USB0_MISC_CTRL_1_REG                (0xF21229CC)
#define USB0_CTRL_REG                       (0xF2122920)
/* USB1 */
#define USB1_20_CUSTOM_REG1_CMD         "devmem 0xf212392c 32 0x0440"
#define USB1_20_CUSTOM_REG2_CMD         "devmem 0xf2123930 32 0x80F0"
#define USB1_MISC_CTRL_1_REG_CMD        "devmem 0xf21239cc 32 0x0651"
#define USB1_20_CTRL_REG_CMD            "devmem 0xf2123920 32 0x4064"
#define USB1_30_CUSTOM_REG1_CMD         "devmem 0xf212392c 32 0x0040"
#define USB1_30_CUSTOM_REG2_CMD         "devmem 0xf2123930 32 0x00f0"
#define USB1_30_CTRL_REG_CMD            "devmem 0xf2123920 32 0x4060"
#define USB1_CUSTOM_REG1                    (0xF212392C)
#define USB1_CUSTOM_REG2                    (0xF2123930)
#define USB1_MISC_CTRL_1_REG                (0xF21239CC)
#define USB1_CTRL_REG                       (0xF2123920)

#define USB_MISC_CTRL_1_REG_VAL             (0x0651)

#define USB20_CUSTOM_REG1_VAL               (0x0440)
#define USB20_CUSTOM_REG2_VAL               (0x80F0)
#define USB20_CTRL_REG_VAL                  (0x4064)

#define USB30_CUSTOM_REG1_VAL               (0x0040)
#define USB30_CUSTOM_REG2_VAL               (0x00F0)
#define USB30_CTRL_REG_VAL                  (0x4060)

/* For bootflash test */
#define INSMOD "insmod /diag/m25p80.ko"
#define RMMOD  "rmmod m25p80.ko"

#define FPGA_EXTERNAL_DEV     0x1004
#define CPU_CP_MPP_8_15       0xf2440004
#define CPU_CP_MPP_16_23      0xf2440008
#define CPU_SPI_1_ENABLE      0x33300010
#define CPU_SPI_1_DISABLE     0x11000010
#define CPU_SPI_1_CLK_EN      0x11111113
#define CPU_SPI_1_CLK_DIS     0x11111111
/*-----------------------------------------------------------------------
 *  Externs                                                             *
 *----------------------------------------------------------------------*/
extern menuinfo_t mb_subtest_menu;
extern menuinfo_t *mb_submenup;
extern int mb_tests(int);
extern int usb_tests(int);
extern int usb_get_speed(int);
extern int usb_slot_tests(int);
extern int usb_get_info(void);
#endif                          /* MB_TESTS_H__ */

/*-------------------------------------------------
$Log: mb_tests.h,v $
Revision 1.5  2018/05/09 06:53:12  letsai
Add TSN GSHDSL portion

Revision 1.4  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3  2018/01/23 11:38:18  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.2.20.3  2018/02/08 07:16:05  lucywang
Merged LTE USB2.0 detect test from trunk

Revision 1.2.20.2  2018/02/07 10:23:01  lucywang
Followed coding rule

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.18.1  2018/01/18 13:13:50  steja
Add LTE USB 2.0 Detection Test

Revision 1.2.4.2  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:46  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3.2.3  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.3.2.2  2017/07/18 06:10:36  steja
Code cleanup

Revision 1.1.4.3.2.1  2017/04/14 00:52:15  steja
Fix USB switch mode 3.0/2.0 issue (CSCvd89346)

Revision 1.1.4.3  2016/11/25 08:11:42  steja
Fix CSCvc13983: TSN-M VDSL bootup fail randomly during EEDVT

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/14 14:32:03  steja
Add memory test


*/

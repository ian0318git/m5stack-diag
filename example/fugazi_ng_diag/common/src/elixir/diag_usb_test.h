/* $Id: diag_usb_test.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_usb_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_usb_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

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


/*-------------------------------------------------
 * $Log: diag_usb_test.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2020/11/27 03:30:11  illiu
 * Add usb dongle feature into test item(External USB test)
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

/* $Id: plug_common_host_impl.h,v 1.2 2019/01/10 06:36:29 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/plug_common_host_impl.h,v $
 *------------------------------------------------------------------
 * 
 * plug_common_host_impl.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_COMMON_HOST_IMPL__
#define __PLUG_COMMON_HOST_IMPL__

/* Pluggable FPGA on Star side I2C bus number */
#define PLUG_FPGA         4

/* For USB test */
#define USB2                       2 
#define USB3                       3
#define DELAY_SYSCMD                   200
#define DELAY_USBHUBCMD                1000
#define DELAY_USBCMD                   5000   /* Wait for 5 sec until sys stable */
#define UEVENT_BUFFER_SIZE             1024
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

/* Slot 1 - Pluggable FPGA I2C Ctrl 0/UART Ctrl 0
 * Slot 2 - Pluggable FPGA I2C Ctrl 1/UART Ctrl 1
 */
#define PLUG_I2C_CTRL(slot)                             (slot - 1)

extern int plug_common_host_i2c_ctrl(int);
extern void plug_common_host_usb_hub_reset(int);
extern ushort plug_common_host_get_cookie_id(int, int, uchar *,uint16_t *, char *);
extern int plug_common_host_plug_fpga_reg_write(uint, uint);
extern int plug_common_host_plug_fpga_reg_read(uint, uint *);
extern int plug_common_host_diag_fpga_reg_bitops(uint, uint, uint);
extern int plug_common_host_usb_3p0_mode_set(int);
extern int plug_common_host_usb_2p0_mode_set(int);
extern int plug_common_host_get_max_plug_slots(void);
extern int plug_common_host_i2c_rd(uint8_t, uint8_t, uint32, char *);
extern int plug_common_host_i2c_wr(uint8_t, uint8_t, uint32, char);
extern int plug_common_host_i2c_rd_2bytes(uint8_t, uint8_t, uint32, ushort *);
extern int plug_common_host_i2c_wr_2bytes(uint8_t, uint8_t, uint32, ushort);

typedef struct plug_module_sku_info_t {
    char           *plug_module_name;
    uint16_t       cook_contype;
} plug_module_sku_info;

extern ushort get_cookie_id(int, int, uchar *,uint16_t *, char *);

#endif

/*-------------------------------------------------
 * $Log: plug_common_host_impl.h,v $
 * Revision 1.2  2019/01/10 06:36:29  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

/* $Id: plug_common_host_impl.h,v 1.1 2020/01/09 01:02:05 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/plug_common_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_common_host_impl.h - Header file for Pluggable Common Host 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_COMMON_HOST_IMPL__
#define __PLUG_COMMON_HOST_IMPL__


/* For USB test */
#define DELAY_USBCMD                   5000   /* Wait for 5 sec until sys stable */

/* Slot 1 - Pluggable FPGA I2C Ctrl-20/UART Ctrl-6
 */
#define PLUG_I2C_CTRL    20

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
extern void plug_common_host_pcie_dev_disable(int);

typedef struct plug_module_sku_info_t {
    char           *plug_module_name;
    uint16_t       cook_contype;
} plug_module_sku_info;

#endif

/*
 *-----------------------------------------------------------------------------
$Log: plug_common_host_impl.h,v $
Revision 1.1  2020/01/09 01:02:05  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

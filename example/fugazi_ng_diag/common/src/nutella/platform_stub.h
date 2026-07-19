/* $Id: platform_stub.h,v 1.4 2019/07/11 12:31:32 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/platform_stub.h,v $
 *------------------------------------------------------------------
 *
 * platform_stub.h - The header file for creating the dummy functions
 *                   compiler issue.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_STUB_H_
#define _PLATFORM_STUB_H_

#define SCC_I2C_IF      0x0008

typedef struct ngio_intf_t {
    char dummy[48];             /* dummy name */
} ngio_if;

typedef struct dev_if_info_ {
    uint16_t interface;
    uint8_t parm1;              /* i2c_bus_no */
    uint8_t parm2;              /* i2c_dev */
    uint8_t parm3;              /* i2c_mux */
    uint8_t parm4;              /* i2c_ctrl */
    uint16_t cookie_size;
    uint16_t offset;
} dev_if_info_t;

/*
 * PA management typedef
 */
typedef struct pas_management_t_ {

    uint pa_bay;                /* Port Adaptor bay */

    /*
     * PA ID eeprom clock register
     */
    ulong clk_reg_width;
    ulong clk_mask;
    volatile void *clk_reg;

    /*
     * PA ID eeprom select register
     */
    ulong select_reg_width;
    ulong select_mask;
    ulong enable_select_bits;
    ulong disable_select_bits;
    volatile void *select_reg;

    /*
     * PA ID eeprom datain register
     */
    ulong datain_reg_width;
    ulong datain_mask;
    volatile void *datain_reg;

    /*
     * PA ID eeprom dataout register
     */
    ulong dataout_reg_width;
    ulong dataout_mask;
    volatile void *dataout_reg;

} pas_management_t;

typedef struct sc_context_ {
    uchar *cookie_contents;
    pas_management_t *pa;
    PFT quack_read_2bytes;
    PFT quack_write_2bytes;
    PFT quack_write_read_2words;
    PFT quack_reset;
    char *info_string;
    uchar type;
    uchar slot;
    dev_if_info_t *dev_if_p;
} sc_context;

extern int i2c_quack_reset(void);
extern int ilp_poe_reset(void);

#endif                          /* _PLATFORM_STUB_H_ */
/*-------------------------------------------------
$Log: platform_stub.h,v $
Revision 1.4  2019/07/11 12:31:32  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/

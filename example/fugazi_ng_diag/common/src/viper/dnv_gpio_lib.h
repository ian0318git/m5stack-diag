 /* $Id: dnv_gpio_lib.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/dnv_gpio_lib.h,v $
 *------------------------------------------------------------------
 * 
 * dnv_gpio_lib.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DNV_GPIO_LIB_H_
#define _DNV_GPIO_LIB_H_


#define DENVERTON_GPIO_RX_VAL_MASK      (0x2)
#define CONFIG_VIPER_PCIE_ADDR          0xE0000000

#define PAD_CFG_DW0_GBE0_SDP0           0xC20000 + 0x400
#define PAD_CFG_DW1_GBE0_SDP0           0xC20000 + 0x404
#define PAD_CFG_DW0_GPIO_0              0xC20000 + 0x4D8
#define PAD_CFG_DW1_GPIO_0              0xC20000 + 0x4DC
#define PAD_CFG_DW0_GPIO_1              0xC20000 + 0x508
#define PAD_CFG_DW1_GPIO_1              0xC20000 + 0x50C
#define PAD_CFG_DW0_GPIO_2              0xC20000 + 0x510
#define PAD_CFG_DW1_GPIO_2              0xC20000 + 0x514
#define PAD_CFG_DW0_GPIO_3              0xC50000 + 0x780
#define PAD_CFG_DW1_GPIO_3              0xC50000 + 0x784
#define PAD_CFG_DW0_GPIO_4              0xC50000 + 0x568
#define PAD_CFG_DW1_GPIO_4              0xC50000 + 0x56C
#define PAD_CFG_DW0_GPIO_5              0xC50000 + 0x570
#define PAD_CFG_DW1_GPIO_5              0xC50000 + 0x574
#define PAD_CFG_DW0_GPIO_6              0xC50000 + 0x578
#define PAD_CFG_DW1_GPIO_6              0xC50000 + 0x57C
#define PAD_CFG_DW0_GPIO_7              0xC50000 + 0x580
#define PAD_CFG_DW1_GPIO_7              0xC50000 + 0x584
#define PAD_CFG_DW0_GPIO_8              0xC50000 + 0x5C8
#define PAD_CFG_DW1_GPIO_8              0xC50000 + 0x5CC
#define PAD_CFG_DW0_GPIO_9              0xC50000 + 0x5D0
#define PAD_CFG_DW1_GPIO_9              0xC50000 + 0x5D4


#define PADCFG0                         0x000
#define PADCFG0_RXEVCFG_SHIFT           25
#define PADCFG0_RXEVCFG_MASK            (3 << PADCFG0_RXEVCFG_SHIFT)
#define PADCFG0_RXEVCFG_LEVEL           0
#define PADCFG0_RXEVCFG_EDGE            1
#define PADCFG0_RXEVCFG_DISABLED        2
#define PADCFG0_RXEVCFG_EDGE_BOTH       3
#define PADCFG0_PREGFRXSEL              BIT(24)
#define PADCFG0_RXINV                   BIT(23)
#define PADCFG0_GPIROUTIOXAPIC          BIT(20)
#define PADCFG0_GPIROUTSCI              BIT(19)
#define PADCFG0_GPIROUTSMI              BIT(18)
#define PADCFG0_GPIROUTNMI              BIT(17)
#define PADCFG0_PMODE_SHIFT             10
#define PADCFG0_PMODE_MASK              (0xf << PADCFG0_PMODE_SHIFT)
#define PADCFG0_GPIORXDIS               BIT(9)
#define PADCFG0_GPIOTXDIS               BIT(8)
#define PADCFG0_GPIORXSTATE             BIT(1)
#define PADCFG0_GPIOTXSTATE             BIT(0)
#define PADCFG0_GPIOTXRXDIS_MASK        0xF00
#define PADCFG0_GPIOTXRXDIS_VAL         0x100

#define PADCFG1                         0x004
#define PADCFG1_TERM_UP                 BIT(13)
#define PADCFG1_TERM_SHIFT              10
#define PADCFG1_TERM_MASK               (7 << PADCFG1_TERM_SHIFT)
#define PADCFG1_TERM_20K                4
#define PADCFG1_TERM_2K                 3
#define PADCFG1_TERM_5K                 2
#define PADCFG1_TERM_1K                 1

#define DNV_PCIE_1F_1      "/sys/devices/pci0000:00/0000:00:1f.1/resource0"   
#define DNV_PCIE_RESCANE   "/sys/bus/pci/rescan"

#define DNV_P2SB_CONTOL_HIDE_DEV        0xE1
#define DNV_BUS_FUNC_SHIFT_1M           20
#define DNV_DEV_NUM_SHIFT_32K           15
#define DNV_FUNC_NUM_SHIFT_4K           12

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

typedef enum {
    DNV_GPIO_0 = 0,
    DNV_GPIO_1,
    DNV_GPIO_2,
    DNV_GPIO_3,
    DNV_GPIO_4,
    DNV_GPIO_5,
    DNV_GPIO_6,
    DNV_GPIO_7,
    DNV_GPIO_8,
    DNV_GPIO_9,
    DNV_GBE0_SDP0,  /* 10 */
} dnv_gpio_port;


typedef enum {
    GPIO_LOW = 0,
    GPIO_HIGH,
} gpio_txrx_val_t;


/* Externs */
extern int  dnv_gpio_write (uint , uint );
extern int  dnv_gpio_read (uint , uint *);
extern int  dnv_gpio_read_rx_val (uint, uint *);
extern int  dnv_set_pcie_1f_1(void);
extern int  dnv_gpio_read_util(void);
extern int  dnv_gpio_write_util(void);
extern int  dnv_p2sb_read_util(void);
extern int  dnv_p2sb_write_util(void);


#endif /* _DNV_GPIO_LIB_H_ */

/*-------------------------------------------------
 * $Log: dnv_gpio_lib.h,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/06/27 06:18:17  harrchan
 * Modify menu name DNV to Denverton and remove menu item Display PCIE 00:1f.1
 *
 * Revision 1.1.2.2  2018/04/10 06:33:16  lucywang
 * Modified GPIO setting before interrupt test for ViperJ
 *
 * Revision 1.1.2.1  2018/02/27 08:06:49  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

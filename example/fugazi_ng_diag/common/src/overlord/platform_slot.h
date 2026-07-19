/* $Id: platform_slot.h,v 1.13 2021/02/24 03:46:27 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_slot.h,v $
 *------------------------------------------------------------------
 *
 * platform_slot.h - Contains structure of all supported NM on xformers. 
 *
 * Aug 2008 - Ian Chang 
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Alan O'Sullivan
 */

#ifndef __PLATFORM_SLOT_H__
#define __PLATFORM_SLOT_H__

extern struct module_info *get_platform_slot_table(int *index, unsigned short id);
extern int get_wic_serdes_no(int slot);
extern int get_max_sm_slots(void);
extern int get_max_wic_slots(void);
extern int get_sm_real_slot(int slot);
extern int get_wic_real_slot(int slot);
extern int get_vm_real_slot(int slot);
extern int get_wic_device_no(int slot);
extern int get_sm_device_no(int slot);
extern int get_max_vm_slots(void);
extern int hwic_slot_start_with(void);
extern int get_max_hwic_slots(void);
extern int get_dc_real_slot(int);
extern uint8_t get_wic_uart_ctrl(int); 
extern uint8_t get_sm_uart_ctrl(int); 
extern int wic_test_wrapper(int);
extern int sm_test_wrapper(int);
extern int get_ngio_testing_now(void);

extern uint8_t get_sm_dc_wic_i2c_ctrl(int slot);
extern unsigned long get_sm_dc_wic_i2c_base(int i2c_ctrl);
extern unsigned long get_sm_dc_wic_ngio(int slot);
extern void* get_sm_dc_wic_i2c_quack(int slot);

enum {
    SM1_UART_CTRL = 0,
    SM2_UART_CTRL,
    WIC1_UART_CTRL,
    WIC2_UART_CTRL,
    WIC3_UART_CTRL,
    VM1_UART_CTRL,
    CAV0_UART_CTRL,
    CAV1_UART_CTRL,
    SM3_UART_CTRL = 10,  /* sm3 start from 10 */
    SM4_UART_CTRL,
    UART_UART_CTRL
};

enum {
    NOW_TESTING_SM1 = 0, 
    NOW_TESTING_SM2 = 1, 
    NOW_TESTING_SM3 = 2, 
    NOW_TESTING_SM4 = 3, 
    NOW_TESTING_SM5 = 4, 
    NOW_TESTING_NIM1 = 5, 
    NOW_TESTING_NIM2 = 6, 
    NOW_TESTING_NIM3 = 7, 
    NOW_TESTING_NIM4 = 8, 
    NOW_TESTING_NIM5 = 9,
    NOW_TESTING_SM1_NIM1 = 10, /* Switzer-carrier Adapter card has two WIC slots */
    NOW_TESTING_SM1_NIM2 = 11,
    NOW_TESTING_SM2_NIM1 = 12,
    NOW_TESTING_SM2_NIM2 = 13,
    NOW_TESTING_NONE = 0xFF, 
}; 

#define DRACO_PID "UCS-E160S-M3/K9" 
#define DRACO_PID_LEN_S 9
#define OAKENSHIELD_PID "SM-X-"
#define OAKENSHIELD_PID_LEN_S 5
#define AQUILA_12C_PID "UCS-E1120D-M3/K9"
#define AQUILA_8C_PID "UCS-E180D-M3/K9"
#define AQUILA_PID_LEN_S 12
#define NEPTUNE_SM4_PCIE_UPPORT_PATH " /sys/devices/pci0000:00/0000:00:03.1/"
#define NEPTUNE_SM4_PCIE_UPPORT_BUS   0
#define NEPTUNE_SM4_PCIE_UPPORT_DEV   3
#define NEPTUNE_SM4_PCIE_UPPORT_FUNC  1
#define NEPTUNE_SM4_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define NEPTUNE_SM4_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define NEPTUNE_SM4_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#define CURIE_RADIUM_SM1_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:03.3/"
#define CURIE_RADIUM_SM1_PCIE_UPPORT_BUS   0
#define CURIE_RADIUM_SM1_PCIE_UPPORT_DEV   3
#define CURIE_RADIUM_SM1_PCIE_UPPORT_FUNC  3
#define CURIE_RADIUM_SM1_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define CURIE_RADIUM_SM1_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define CURIE_RADIUM_SM1_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#define CURIE_RADIUM_NIM1_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:1c.0/"
#define CURIE_RADIUM_NIM1_PCIE_UPPORT_BUS   0
#define CURIE_RADIUM_NIM1_PCIE_UPPORT_DEV   0x1c
#define CURIE_RADIUM_NIM1_PCIE_UPPORT_FUNC  0
#define CURIE_RADIUM_NIM1_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define CURIE_RADIUM_NIM1_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define CURIE_RADIUM_NIM1_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#define CURIE_URANIUM_SM1_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:1d.2/"
#define CURIE_URANIUM_SM1_PCIE_UPPORT_BUS   0
#define CURIE_URANIUM_SM1_PCIE_UPPORT_DEV   0x1d
#define CURIE_URANIUM_SM1_PCIE_UPPORT_FUNC  2
#define CURIE_URANIUM_SM1_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define CURIE_URANIUM_SM1_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define CURIE_URANIUM_SM1_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#define CURIE_URANIUM_SM2_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:1d.0/"
#define CURIE_URANIUM_SM2_PCIE_UPPORT_BUS   0
#define CURIE_URANIUM_SM2_PCIE_UPPORT_DEV   0x1d
#define CURIE_URANIUM_SM2_PCIE_UPPORT_FUNC  0
#define CURIE_URANIUM_SM2_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define CURIE_URANIUM_SM2_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define CURIE_URANIUM_SM2_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#define CURIE_URANIUM_NIM1_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:1c.4/"
#define CURIE_URANIUM_NIM1_PCIE_UPPORT_BUS   0
#define CURIE_URANIUM_NIM1_PCIE_UPPORT_DEV   0x1c
#define CURIE_URANIUM_NIM1_PCIE_UPPORT_FUNC  4
#define CURIE_URANIUM_NIM1_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define CURIE_URANIUM_NIM1_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define CURIE_URANIUM_NIM1_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#define CURIE_URANIUM_NIM2_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:1c.5/"
#define CURIE_URANIUM_NIM2_PCIE_UPPORT_BUS   0
#define CURIE_URANIUM_NIM2_PCIE_UPPORT_DEV   0x1c
#define CURIE_URANIUM_NIM2_PCIE_UPPORT_FUNC  5
#define CURIE_URANIUM_NIM2_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define CURIE_URANIUM_NIM2_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */
#define CURIE_URANIUM_NIM2_PCIE_UPPORT_2ND_LINK_OFF  0xa0  /* 2nd bus link offset */

#endif  /* end __PLATFORM_SLOT_H */

/* ------ End of Module ------ */



/*
 *------------------------------------------------------------------
$Log: platform_slot.h,v $
Revision 1.13  2021/02/24 03:46:27  xiaolaya
Fix bug for Switzer-Carrier SM Daughter NIM Daughter VM cookie

Revision 1.12  2021/01/12 04:04:58  xiaolaya
switzer-carrier daughter card eeprom access bug fix

Revision 1.11  2020/05/22 02:28:34  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.10  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.9  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.8.2.3  2018/10/25 10:10:23  alpeng
when power off ngio, bypass ngio without pcie device

Revision 1.8.2.2  2018/08/20 23:09:45  alpeng
enhance testcard pcie link test

Revision 1.8.2.1  2018/08/16 18:22:37  alpeng
remove useless info on i2c_drv; fixed get_sgmii_port on platform_eth_pkt_txrx.c for curie; add wrapper for wic_test and sm_test for prepare eth info on platform_slot.c; support ge1 for SM on testcard;

Revision 1.8  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.7  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.6  2017/06/13 06:46:17  olin2
Increase compared length (CSCve77190)

Revision 1.5  2017/03/21 08:41:57  olin2
Collapse Aquila-branch to Main Trunk.

Revision 1.4.6.5  2018/04/18 09:10:23  alpeng
add workaround to remove testcard on neptune sm4 gracefully

Revision 1.4.6.4  2017/09/19 10:18:51  alpeng
support oakenshield; fix oakenshield andf2w uart issue

Revision 1.4.6.3  2017/06/07 08:50:45  alpeng
add size of string cmp for Canis new sku

Revision 1.4.6.2  2017/04/05 06:45:03  leschen
Sync with <ng_diag-tag-032917>

Revision 1.4.6.1  2016/11/04 05:13:03  alpeng
update uart info to return uart ctrl number on slot.c

Revision 1.7  2017/07/28 07:49:43  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.6  2017/06/13 06:46:17  olin2
Increase compared length (CSCve77190)

Revision 1.5  2017/03/21 08:41:57  olin2
Collapse Aquila-branch to Main Trunk.

Revision 1.4  2016/01/21 01:50:04  olin2
Collapse Draco-branch to Main Trunk.

Revision 1.3  2014/06/03 06:03:09  alpeng
first check in for plx on testcard; update the code for tlk10232 on testcard

Revision 1.2  2014/05/15 07:46:18  alpeng
update pcie device num for ngio

Revision 1.1  2013/07/16 10:02:00  alpeng
put platform_slot.c for general using

Revision 1.1  2013/05/09 05:53:00  alpeng
add utah tree

Revision 1.5  2013/04/23 17:18:16  mcharon
add get_wic_device_no to support overdrive

Revision 1.4  2012/06/05 09:33:47  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/05/31 14:24:40  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

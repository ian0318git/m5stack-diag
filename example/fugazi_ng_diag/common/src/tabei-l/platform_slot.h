/* $Id: platform_slot.h,v 1.2 2019/10/17 02:16:26 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_slot.h,v $
 *------------------------------------------------------------------
 *
 * platform_slot.h - Contains structure of all supported NIM 
 * 
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Alan O'Sullivan
 */

#ifndef __PLATFORM_SLOT_H__
#define __PLATFORM_SLOT_H__

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





#define TABEI_NIM1_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:0e.0/"
#define TABEI_NIM1_PCIE_UPPORT_BUS   0
#define TABEI_NIM1_PCIE_UPPORT_DEV   0xe
#define TABEI_NIM1_PCIE_UPPORT_FUNC  0
#define TABEI_NIM1_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define TABEI_NIM1_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */



#endif  /* end __PLATFORM_SLOT_H */

/* ------ End of Module ------ */



/*
 *------------------------------------------------------------------
$Log: platform_slot.h,v $
Revision 1.2  2019/10/17 02:16:26  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.6  2018/11/05 12:24:29  olin2
Update NIM disable

Revision 1.1.2.5  2018/11/05 08:49:37  olin2
Support NIM disable

Revision 1.1.2.4  2018/11/02 07:42:59  olin2
Update PCIE setting

Revision 1.1.2.3  2018/10/16 11:33:14  olin2
Update NIM test

Revision 1.1.2.2  2018/10/15 11:48:29  olin2
Update for using common slot.c

Revision 1.1.2.1  2018/10/09 09:22:05  olin2
Initial commit for NIM test

Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

$Endlog$
*/

/* $Id: platform_slot.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_slot.h,v $
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
    NOW_TESTING_NONE = 0xFF, 
}; 

#define DYNAMO_NIM1_PID "NIM-" 
#define DYNAMO_PID_LEN_S 4

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


#define PHOENIX_NIM1_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:0f.0/"
#define PHOENIX_NIM1_PCIE_UPPORT_BUS   0
#define PHOENIX_NIM1_PCIE_UPPORT_DEV   0xf
#define PHOENIX_NIM1_PCIE_UPPORT_FUNC  0
#define PHOENIX_NIM1_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define PHOENIX_NIM1_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */

#define PHOENIX_NIM2_PCIE_UPPORT_PATH "/sys/devices/pci0000:00/0000:00:0e.0/"
#define PHOENIX_NIM2_PCIE_UPPORT_BUS   0
#define PHOENIX_NIM2_PCIE_UPPORT_DEV   0xe
#define PHOENIX_NIM2_PCIE_UPPORT_FUNC  0
#define PHOENIX_NIM2_PCIE_UPPORT_2ND_BUS_OFF   0x19  /* secondary bus offset */
#define PHOENIX_NIM2_PCIE_UPPORT_2ND_RESETOFF  0x3e  /* 2nd bus reset offset */


#endif  /* end __PLATFORM_SLOT_H */

/* ------ End of Module ------ */




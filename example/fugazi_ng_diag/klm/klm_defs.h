/* $Id: klm_defs.h,v 1.2 2012/03/28 00:38:26 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/klm/klm_defs.h,v $
 *-----------------------------------------------------------------------------
 * File: definition needed for klms
 *
 * March. 2008, mcharon
 *
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __KLM_DEFS__
#define __KLM_DEFS__
#ifdef LINUX_APP
#include <linux/ioctl.h>
#endif

#ifdef LINUX_KLM
#include <linux/ioctl.h>
#endif

#if defined( LINUX_KLM) || defined(LINUX_APP)
//#define NUM_HWIC_SLOTS              4
//#define NUM_SM_TYPE_SLOTS           4
//#define MAX_HWIC_SLOT               NUM_HWIC_SLOTS
//#define MAX_NUM_PVDM_SLOTS          4

#endif

#define KLM_PVDM_NAME "klm_pvdm"

/* WARNING:........check this to make sure it's consistent with diagmon code */


#define GOOFY_VENDOR_ID                                    0x1137
#define GOOFY_DEVICE_ID                                    0x001E


#define GOOFY_HWIC_LOCAL_BLK_SZ    0x4000           /* 16K each */
#define GOOFY_HWIC0_LOCAL_OFFSET   0
#define GOOFY_HWIC1_LOCAL_OFFSET   (GOOFY_HWIC0_LOCAL_OFFSET + GOOFY_HWIC_LOCAL_BLK_SZ)
#define GOOFY_HWIC2_LOCAL_OFFSET   (GOOFY_HWIC1_LOCAL_OFFSET + GOOFY_HWIC_LOCAL_BLK_SZ)
#define GOOFY_HWIC3_LOCAL_OFFSET   (GOOFY_HWIC2_LOCAL_OFFSET + GOOFY_HWIC_LOCAL_BLK_SZ)
#define GOOFY_QHWIC_LOCAL_BLK_SZ   (GOOFY_HWIC_PORT_MAX *  GOOFY_HWIC_LOCAL_BLK_SZ)
//#define GOOFY_MASTER_BAR0             0x10000000
#if 0
#define GOOFY_MASTER_QHWIC_RM_OFST    0x00000000
#define GOOFY_MASTER_WAN_OFST         0x00800000
#define GOOFY_MASTER_GLBREG_OFST      0x00900000
#define GOOFY_MASTER_QHWIC_LC_OFST    0x00A00000
#define GOOFY_MASTER_HSIB_CONF_OFST   0x00FF0000
#define GOOFY_MASTER_DS0_OFST         0x20000000
#define GOOFY_MASTER_DS1_OFST         0x40000000
#define GOOFY_MASTER_DS2_OFST         0x50000000
#define GOOFY_MASTER_DS3_OFST         0x80000000
#define GOOFY_MASTER_DS_BLK_SZ        0x02000000
#define GOOFY_HDLC_BLK_SZ             0x10000

#define GOOFY_HDLC_CORE_MAX           8
#define GOOFY_HDLC_OFFSET             0
#define GOOFY_HDLC_MAX_NUM            GOOFY_HDLC_CORE_MAX
#define GOOFY_HDLC_BLK_SZ             0x10000
#define GOOFY_PKT_PUMP_OFFSET  (GOOFY_HDLC_OFFSET + (GOOFY_HDLC_MAX_NUM * GOOFY_HDLC_BLK_SZ))

#define GOOFY_HWIC_REMOTE_BLK_SZ   0x200000
#define GOOFY_HWIC0_REMOTE_OFFSET  0
#define GOOFY_HWIC1_REMOTE_OFFSET  (GOOFY_HWIC0_REMOTE_OFFSET + GOOFY_HWIC_REMOTE_BLK_SZ)
#define GOOFY_HWIC2_REMOTE_OFFSET  (GOOFY_HWIC1_REMOTE_OFFSET + GOOFY_HWIC_REMOTE_BLK_SZ)
#define GOOFY_HWIC3_REMOTE_OFFSET  (GOOFY_HWIC2_REMOTE_OFFSET + GOOFY_HWIC_REMOTE_BLK_SZ)
#define GOOFY_QHWIC_BLK_SZ         (GOOFY_HWIC_PORT_MAX * GOOFY_HWIC_REMOTE_BLK_SZ)

#define GOOFY_HDLC_CORE_MAX   8
#define GOOFY_HDLC_MAX_NUM     GOOFY_HDLC_CORE_MAX
#define GOOFY_HDLC_BLK_SZ      0x10000
#define GOOFY_HDLC_OFFSET      0
#define GOOFY_PKT_PUMP_OFFSET  (GOOFY_HDLC_OFFSET + (GOOFY_HDLC_MAX_NUM * GOOFY_HDLC_BLK_SZ))
#define GOOFY_PKT_PUMP_BLK_SZ  0x10000
#define GOOFY_SCC_OFFSET       (GOOFY_PKT_PUMP_OFFSET + GOOFY_PKT_PUMP_BLK_SZ)
#define GOOFY_SCC_BLK_SZ       0x10000
#define GOOFY_TDM_OFFSET       (GOOFY_SCC_OFFSET + GOOFY_SCC_BLK_SZ)
#define GOOFY_TDM_BLK_SZ       0x20000

#define GOOFY_INTERRUPT_OFFSET     0
#define GOOFY_INTERRUPT_BLK_SZ     0x200
#define GOOFY_BLK_SZ               0x1000000


#define MASK_NET_INTR_HWIC0        0x00000100
#define MASK_ERR_INTR_HWIC0_ERR    0x00000100
#endif

/*************above defines are from diagmon code....need to share....*/

//#define SET_MB_I2C_ADDR         _IOWR(0x54, 0x54, int)
/* cavium linux has way too many reserved ioctl and their _IOC macros
   don't return valid command numbers so need to find a block of
   available commands to use. one such block is 0x8B00 to 0x8B36 and
   0x5301 to 0x5330...
   0x5301 to 5330 are reserved for hwic */
#define MB_I2C_SET_MSG                                     0x8B00

#define MEM_MGR_GET_MEM                                    0x8B10
#define MEM_MGR_FREE_MEM                                   0x8B11

//#define SM_ENABLE_MSI                                      0x8B20
//#define SM_DISABLE_MSI                                     0x8B21
//#define SM_PCIE_MSI                                          0x8B22
//#define NM_ENABLE_MSI                                      0x8B22
//#define NM_DISABLE_MSI                                     0x8B23
#define GET_SM_IFACE                                       0x8B24


#define KLM_HWIC_GE                                        0x5301
#define KLM_HWIC_ARCHER                                    0x5302


#define NET_ISR                          0xF4
#define MAN_ISR                          0xF5
#define ERR_ISR                          0xF6

#define ASIC_DEFAULT_HWIC_STATE          0xF0
#define ASIC_HWIC_COMMON_INTR_TEST       0xF1
#define ASIC_INTERNAL_LPBK_TEST          0xF2
#define ASIC_GET_HWIC_INFO               0xF3
#define ASIC_INIT_HWIC_INFO              0xF4
#define ASIC_GET_INTR_CNT                0xF5
#define ASIC_INIT_GOOFY_INTR_CNT         0xF6
#define ASIC_INIT_HWIC_INTR_CNT          0xF7
#define ASIC_INIT_PKTPUMP_INTR_CNT       0xF8
#define ASIC_GET_HDLC_INTR_INFO          0xFA
#define ASIC_INIT_HDLC_INTR_CNT          0xFB
#define ASIC_GET_PKTPUMP_INTR_INFO          1

typedef struct sm_cmd_iface_t_ {
    int slot;
    int cmd;
    int param1;
} sm_cmd_iface_t;

typedef struct asic_cmd_t_ {
    unsigned int id;         /* what type of asic test we will run */
    unsigned int subtest;
    unsigned int slot;
    unsigned int port;
    unsigned int channel;
    unsigned int param1;
} asic_cmd_t;

#endif /* __KLM_DEFS__*/
/**********history*************
$log: $
$Endlog: $
*/

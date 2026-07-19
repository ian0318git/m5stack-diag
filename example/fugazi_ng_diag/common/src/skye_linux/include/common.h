 /*$Id: common.h,v 1.2 2015/05/25 03:59:09 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/common.h,v $
 *------------------------------------------------------------------
 *
 * common.h: Headfile for common definitions.
 *
 * April 17, 2013 - palin2 ported from Overlord.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __COMMON_H__
#define __COMMON_H__

/*
 * The purpose of this file is for generic C defines only. It is not
 * being used for any platform related defines.
 *
 * If the defines is cross platform, then please use cross_platform.h
 */
#define TRUE       1
#define FALSE      0

#define EQUAL      1
#define NOT_EQUAL  0
#define EQ	   EQUAL
#define NEQ        NOT_EQUAL
#define NOTEQUAL   NOT_EQUAL

#define PASSED     0
#define FAILED     1
#define OK	   PASSED
#define NOT_OK     FAILED
#define NOTOK      FAILED
#define PASS	   PASSED
#define FAIL	   FAILED
#define FAIL_RETRY 0x88
#define ERROR     -1

#define ENABLE	   1
#define DISABLE	   0

#define FULL_TEST  1
#define IFACE_TEST 2

#ifndef NULL
#define NULL	   0
#endif
#define NULL_PTR   (void *)0

#define POLL_MODE        0
#define INTERRUPT_MODE   1
#define INTR_MODE        INTERRUPT_MODE
#define MAX_MODE         2  /* DO NOT change this number which is
				used for number of modes such as
				interrupt and poll mode */
#define CHECK_INTR       1
/* defines for diagflag_xram */
#define D_SET_OPTIONS   0x00000001  /* get optional outputs */
#define D_TRACE         0x00000002  /* display for tracing code flow */
#define D_WARNING       0x00000004  /* display accumulated warnings */
#define D_PR_TASKSWAPS  0x00000008  /* print multitasking task swaps */
#define D_MIN_TEST_TIME 0x00000010  /* minimize test time of diag */
#define D_XEC_AUTH      0x00000020  /* execute authentication code */
#define D_PERMU_TEST    0x00000040  /* Run different permutation of tests */
#define D_DEBUG_OPTIONS 0x00000080  /* Display on screen and Debug Buffer */
#define D_POWER_ON      0x10        /* leave power on after module test */
#define D_EXT_CUSTOMER  0x00000100  /* External Customer Flag */

#define PATTERN   0x5ADBA56C

/*
 * Basic memory sizes
 */
#define ONE_K            0x00000400
#define TWO_K            0x00000800
#define FOUR_K           0x00001000
#define EIGHT_K          0x00002000
#define SIXTEEN_K        0x00004000
#define THIRTYTWO_K      0x00008000
#define SIXTYFOUR_K      0x00010000
#define ONE28_K          0x00020000
#define TWO56_K          0x00040000
#define FIVE12_K         0x00080000
 
#define HALF_MEG         0x00080000
#define ONE_MEG          0x00100000
#define TWO_MEG          0x00200000
#define THREE_MEG        0x00300000 
#define FOUR_MEG         0x00400000
#define EIGHT_MEG        0x00800000
#define SIXTEEN_MEG      0x01000000
#define THIRTYTWO_MEG    0x02000000
#define SIXTYFOUR_MEG    0x04000000
#define ONE28_MEG        0x08000000
#define TWO56_MEG        0x10000000
#define FIVE12_MEG       0x20000000
#define MEG(x)           ((x) * 1024 * 1024)
 
#define ONE_GIG          0x40000000
#define TWO_GIG          0x80000000
#define THREE_GIG        0xC0000000

#define MASK_2B          0x00000001      /* 2byte - 16-bit aligned */
#define MASK_4B          0x00000003
#define MASK_8B          0x00000007
#define MASK_16B         0x0000000f
#define MASK_32B         0x0000001f
#define MASK_64B         0x0000003f
#define MASK_128B        0x0000007f
#define MASK_4K          0x00000FFF      /* 4Kbyte aligned */
#define MASK_8K          0x00001FFF
#define MASK_16K         0x00003FFF
#define MASK_32K         0x00007FFF
#define MASK_64K         0x0000FFFF

#define ALLIGN_8B(x)   ((void *)((uchar *)((ulong)(x + MASK_8B) & ~MASK_8B)))
#define ALLIGN_16B(x)  ((void *)((uchar *)((ulong)(x + MASK_16B) & ~MASK_16B)))
#define ALLIGN_32B(x)  ((void *)((uchar *)((ulong)(x + MASK_32B) & ~MASK_32B)))
#define ALLIGN_64B(x)  ((void *)((uchar *)((ulong)(x + MASK_64B) & ~MASK_64B)))
#define ALLIGN_128B(x)  ((void *)((uchar *)((ulong)(x + MASK_128B) & ~MASK_128B)))
#define ALLIGN_4K(x)   ((void *)((uchar *)((ulong)(x + MASK_4K) & ~MASK_4K)))
#define ALLIGN_64K(x)  ((void *)((uchar *)((ulong)(x + MASK_64K) & ~MASK_64K)))

#define ALIGN(ptr, sz) (((ulong)(ptr) + (sz - 1)) & ~(sz - 1))

typedef enum {
    DISPLAY_HCI = 0,	/* Human Computer Interface */
    DISPLAY_M2M,	/* Machine to Machine */
} display_format_t;

enum {
    ACT_LOW = 0,
    ACT_HIGH,
    FALLING_E,
    RISING_E
};

enum {
    MODE_T1 = 0,
    MODE_E1,
    MODE_CHAN_T1,
    MODE_CHAN_E1,
    MODE_8M,
    MODE_8M_INT,         /* 8M Interleave */
    MODE_8M_INT_T1,
    MODE_UNF_T1,
    MODE_UNF_E1,
    MODE_T3,
    MODE_E3,
    MODE_CHAN_T3,
    MODE_CHAN_E3,
};

enum {
    TYPE_PHY = 0,
    TYPE_SWITCH,
};

typedef enum {
    LOOP_NONE = 0,
    LOOP_MAC,
    LOOP_PHY,
    LOOP_PCI,
    LOOP_EXT,
    LOOP_INT,
    LOOP_TBI,
    LOOP_REMOTE,
    LOOP_XAUI_BP
} loop_mode_t;

#define LONG_SWAP(x)   ((((x)&0xff)<<24) + (((x)&0xff00)<<8) + \
                         (((x)&0xff0000)>>8) + (((x)&0xff000000)>>24))

#define BIT_0                        0x00000001
#define BIT_1                        0x00000002
#define BIT_2                        0x00000004
#define BIT_3                        0x00000008
#define BIT_4                        0x00000010
#define BIT_5                        0x00000020
#define BIT_6                        0x00000040
#define BIT_7                        0x00000080
#define BIT_8                        0x00000100
#define BIT_9                        0x00000200
#define BIT_10                       0x00000400
#define BIT_11                       0x00000800
#define BIT_12                       0x00001000
#define BIT_13                       0x00002000
#define BIT_14                       0x00004000
#define BIT_15                       0x00008000
#define BIT_16                       0x00010000
#define BIT_17                       0x00020000
#define BIT_18                       0x00040000
#define BIT_19                       0x00080000
#define BIT_20                       0x00100000
#define BIT_21                       0x00200000
#define BIT_22                       0x00400000
#define BIT_23                       0x00800000
#define BIT_24                       0x01000000
#define BIT_25                       0x02000000
#define BIT_26                       0x04000000
#define BIT_27                       0x08000000
#define BIT_28                       0x10000000
#define BIT_29                       0x20000000
#define BIT_30                       0x40000000
#define BIT_31                       0x80000000

/*******************************************************************************
 GPIO Configuration on CPU#0/1
*******************************************************************************/

#define MASTER_CPU  0
#define SLAVE_CPU   1
#define CPU_CPU_ID_REGISTER         0x00000400 /* GPIO 10 : master or slave */
#define CPU_PCIE_PLUG0_REGISTER     0x00000800 /* GPIO 11 :PCIe hot plug - 1 */
#define CPU_PCIE_PLUG1_REGISTER     0x00001000 /* GPIO 12 :PCIe hot plug - 2 */
#define CPU_INT_TO_CPU0_REGISTER    0x00002000 /* GPIO 13 :FPGA interrupt - CPU0 only */
#define CPU_GPIO14_REGISTER         0x00004000 /* GPIO 14 :Not Used */
#define CPU_XAUI_10GKR              0x00008000 /* GPIO 15 :ISR-NG BP XAUI / 10GKR */
#define CPU_ACT_WDT_REGISTER        0x00010000 /* GPIO 16 :Activity signal to WDT 0/1 */ 
#define P_PINS (CPU_CPU_ID_REGISTER | CPU_PCIE_PLUG0_REGISTER | \
                CPU_PCIE_PLUG1_REGISTER | CPU_INT_TO_CPU0_REGISTER | \
                CPU_GPIO14_REGISTER | CPU_XAUI_10GKR |CPU_ACT_WDT_REGISTER)
#define O_PINS CPU_ACT_WDT_REGISTER         /* output */
#define A_PINS CPU_INT_TO_CPU0_REGISTER     /* assert */
#define D_PINS 0 /* deassert */
/* GPIO[12:11]
   1,1 = CPU#1 is powered ON
   1,0 = CPU#1 is going to power ON
   0,1 = CPU#1 is going to power OFF
   0,0 = CPU#1 is powered OFF
 */
#define CPU1_POWERED_ON              BIT_11 | BIT_12
#define CPU1_GOING_POWER_ON          BIT_12
#define CPU1_GOING_POWER_OFF         BIT_11
#define CPU1_POWERED_OFF             0x0

/*
 * Global variables
 */
extern unsigned long spin_one_usec;   /* timer calibrated in platform init */ 
extern void cterr(char, int, char*, ...);


#endif /* __COMMON_H__ */

/******** History ******** 
$Log: common.h,v $
Revision 1.2  2015/05/25 03:59:09  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:24  steja
Code check-in to skye-branch2 for ER code review

*
*------------------------------------------------------------------
Revision 1.1.2.2  2014/08/22 04:58:47  palin2
First check-in to enhance Skye error message.

Revision 1.1.2.1  2014/07/21 01:56:36  palin2
Initial check-in Skye module side Diag code.

$Endlog$
*/

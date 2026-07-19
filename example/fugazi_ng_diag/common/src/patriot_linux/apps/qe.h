/* $Id: qe.h,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 ***********************************************************************
 * File Name: qe.h
 *
 * Description: QUICC Engine register definitions
 *
 *      Ported from Freddo by Robert Julian, May 2006
 *	Ported from Steelers by Khalid Sabzwari, March 2010
 *
 * Copyright (c)2006-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#ifndef __QE__
#define __QE__


/* short cuts */
#define _PackedType        __attribute__ ((packed))
#define SPC(_x, _n)        volatile uint8_t __res_##_x[_n];

#define MURAM_START        (uint32 *)&(REGB->qe.muram.ucc1[0])
#define MURAM_END          (uint32 *)((uint32)&(REGB->qe.muram.ucc1[0]) + 0xC000)


#define MURAM_ECC_EN       0
#define IRAM_ECC_EN        1
#define MURAM_IRAM_ECC_EN  2


/**************************************************************************
 * QE Internal Memory Map
 *   Register Space (0x00000 - 0x0FFFF)
 *        General Space (0x00000 - 0x0407F)
 **************************************************************************/

/**************************************************************************
 * Instruction RAM Registers (I-RAM)        (0x00000 - 0x007F)
 **************************************************************************/

typedef struct {
    volatile uint32_t  iadd;          /* I-RAM address: 0x0000 */
    volatile uint32_t  idata;         /* I-RAM data:	0x0004 */
    SPC(a, 0x4)
    volatile uint32_t  iready;        /* I-RAM ready:	0x000c */
    SPC(b, 0x70)
} _PackedType QEIRAMMap_t, qe_iram_t;

#define QE_IRAM_IADD_AIE              0x80000000
#define QE_IRAM_IADD_BADDR            0x000F0000
#define QE_IRAM_IADD_IADDR            0x0000FFFF
#define QE_IRAM_IADD_BADDR_SHIFT      16
#define QE_IRAM_IADD_BADDR_VALUE      0x00080000

#define QE_IRAM_RISC0_START           0x00000000
#define QE_IRAM_RISC0_END             0x00005FFC
#define QE_IRAM_RISC1_START           0x00008000
#define QE_IRAM_RISC1_END             0x0000DFFC

#define QE_IRAM_COMMON_START          0x00000000
#define QE_IRAM_COMMON_END            0x0000BFFC

#define QE_IRAM_IREADY                0x80000000


/**************************************************************************
 * Interrupt Controller (IRQ)                (0x00080 - 0x000FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  cicr;          /* QE system interrupt config */
    volatile uint32_t  civec;         /* QE system interrupt vector */
    volatile uint32_t  cripnr;        /* QE RISC interrupt pending */
    volatile uint32_t  cipnr;         /* QE systerm interrupt pending */
    volatile uint32_t  cipxcc;        /* QE interrupt priority */
    volatile uint32_t  cipycc;        /* QE interrupt priority */
    volatile uint32_t  cipwcc;        /* QE interrupt priority */
    volatile uint32_t  cipzcc;        /* QE interrupt priority */
    volatile uint32_t  cimr;          /* QE system interrupt mask */
    volatile uint32_t  crimr;         /* QE RISC interrupt mask */
    volatile uint32_t  cicnr;         /* QE system intrt control */
    SPC(a, 4)
    volatile uint32_t  ciprta;        /* QE sys intr prio for RISC tasks A */
    volatile uint32_t  ciprtb;        /* QE sys intr prio for RISC tasks B */
    SPC(b, 4)
    volatile uint32_t  cricr;         /* QE system RISC intr control */
    SPC(c, 0x20)
    volatile uint32_t  chivec;        /* QE high system interrupt vec */
    SPC(d, 0x1C)
} _PackedType QEIRQMap_t, qe_irq_t;

#define QE_IRQ_IVEC_MASK  0x3f        /* QE interrup vector mask */

/*
 * QE SIMSR register
 */
#define QE_INT_CIMR_UCC1         0x80000000     /* UCC 1  */
#define QE_INT_CIMR_UCC2         0x40000000     /* UCC 2  */
#define QE_INT_CIMR_UCC3         0x20000000     /* UCC 3  */
#define QE_INT_CIMR_UCC4         0x10000000     /* UCC 4  */
#define QE_INT_CIMR_MCC1         0x08000000     /* MCC 1  */
#define QE_INT_CIMR_UCC5         0x00800000     /* UCC 5  */
#define QE_INT_CIMR_UCC6         0x00400000     /* UCC 6  */
#define QE_INT_CIMR_UCC7         0x00200000     /* UCC 7  */
#define QE_INT_CIMR_UCC8         0x00100000     /* UCC 8  */
#define QE_INT_CIMR_SPI2         0x00008000     /* SPI 2  */
#define QE_INT_CIMR_SPI1         0x00004000     /* SPI 1  */
#define QE_INT_CIMR_RTT          0x00002000     /* RTT    */
#define QE_INT_CIMR_SDMA         0x00000040     /* SDMA   */
#define QE_INT_CIMR_USB          0x00000020     /* USB    */
#define QE_INT_CIMR_TIMER1       0x00000010     /* TIMER1 */
#define QE_INT_CIMR_TIMER2       0x00000080     /* TIMER2 */
#define QE_INT_CIMR_TIMER3       0x00000040     /* TIMER3 */
#define QE_INT_CIMR_TIMER4       0x00000020     /* TIMER4 */

#define QE_INT_CIPNR_UCC1         0x80000000    /* UCC 1  */
#define QE_INT_CIPNR_UCC2         0x40000000    /* UCC 2  */
#define QE_INT_CIPNR_UCC3         0x20000000    /* UCC 3  */
#define QE_INT_CIPNR_UCC4         0x10000000    /* UCC 4  */
#define QE_INT_CIPNR_MCC1         0x08000000    /* MCC 1  */
#define QE_INT_CIPNR_UCC5         0x00800000    /* UCC 5  */
#define QE_INT_CIPNR_UCC6         0x00400000    /* UCC 6  */
#define QE_INT_CIPNR_UCC7         0x00200000    /* UCC 7  */
#define QE_INT_CIPNR_UCC8         0x00100000    /* UCC 8  */
#define QE_INT_CIPNR_SPI2         0x00008000    /* SPI 2  */
#define QE_INT_CIPNR_SPI1         0x00004000    /* SPI 1  */
#define QE_INT_CIPNR_RTT          0x00002000    /* RTT    */
#define QE_INT_CIPNR_SDMA         0x00000040    /* SDMA   */
#define QE_INT_CIPNR_USB          0x00000020    /* USB    */
#define QE_INT_CIPNR_TIMER1       0x00000010    /* TIMER1 */
#define QE_INT_CIPNR_TIMER2       0x00000080    /* TIMER2 */
#define QE_INT_CIPNR_TIMER3       0x00000040    /* TIMER3 */
#define QE_INT_CIPNR_TIMER4       0x00000020    /* TIMER4 */



/**************************************************************************
 * RISC Configuration Registers                (0x00100 - 0x001FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  cecr;          /* QE command */
    volatile uint32_t  ceccr;         /* QE controller configuration */
    volatile uint32_t  cecdr;         /* QE command data */
    SPC(a, 10)
    volatile uint16_t  ceter;         /* QE timer event */
    SPC(b, 2)
    volatile uint16_t  cetmr;         /* QE timers mask */
    volatile uint32_t  cetscr;        /* QE time-stamp timer control */
    volatile uint32_t  cetsr1;        /* QE time-stamp 1 */
    volatile uint32_t  cetsr2;        /* QE time-stamp 2 */
    SPC(c, 8)
    volatile uint32_t  cevter;        /* QE virtual tasks event */
    volatile uint32_t  cevtmr;        /* QE virtual tasks mask */
    volatile uint16_t  cercr;         /* QE RAM control */
    SPC(d, 38)
    volatile uint16_t  ceexe1;        /* QE external request 1 event */
    SPC(e, 2)
    volatile uint16_t  ceexm1;        /* QE external request 1 mask */
    SPC(f, 2)
    volatile uint16_t  ceexe2;        /* QE external request 2 event */
    SPC(g, 2)
    volatile uint16_t  ceexm2;        /* QE external request 2 mask */
    SPC(h, 2)
    volatile uint16_t  ceexe3;        /* QE external request 3 event */
    SPC(i, 2)
    volatile uint16_t  ceexm3;        /* QE external request 3 mask */
    SPC(j, 2)
    volatile uint16_t  ceexe4;        /* QE external request 4 event */
    SPC(k, 2)
    volatile uint16_t  ceexm4;        /* QE external request 4 mask */
    SPC(l, 0x82)
} _PackedType QERISCMap_t, qe_cp_t;

/*
 * cp_cr - RISC Command Register
 */
#define QE_CPCR_RST              0x80000000     /* Software Reset Command */
#define QE_CPCR_SEMA             0x00010000     /* Command Semaphore Flag */
#define QE_CPCR_SUBBLOCK(x)      ((x) << 17)    /* Subblock number */
#define QE_CPCR_SNUM(x)          ((x) << 17)    /* SNUM number */
#define QE_CPCR_CHANNEL(x)       ((x) <<  6)    /* Channel number */
#define QE_CPCR_OPCODE(x)        ((x) & 0x3f)   /* Operation Code */

/*
 * QE CECR register
 */
#define QE_CECR_RESET            0x80000000     // RST(0)

#define QE_CECR_SBC(x)           ((x) << 17)
#define QE_CECR_SNUM(x)          ((x) << 17)
#define QE_CECR_CHANNEL(x)       ((x) <<  6)
#define QE_CECR_OPCODE(x)        ((x) & 0x3f)

#define FAST_UCC1_SBC       0x100      // SBC(6-14) = 0b100000000
#define FAST_UCC3_SBC       0x120      // SBC(6-14) = 0b100100000
#define FAST_UCC4_SBC       0x130      // SBC(6-14) = 0b100110000
#define FAST_UCC5_SBC       0x140      // SBC(6-14) = 0b101000000
#define SLOW_UCC1_SBC       0x000      // SBC(6-14) = 0b000000000
#define SLOW_UCC4_SBC       0x030      // SBC(6-14) = 0b000110000
#define SPI1_SBC            0x0A0      // SBC(6-14) = 0b010100000

#define QE_CECR_FLG              0x00010000     // FLG(15)
#define QE_CECR_READY_FOR_NEW_CMD 0x00000000

#define QE_CECR_MCN_QMC          0x00000080     // MCN(18-25) = 0x02
#define QE_CECR_MCN_ETHERNET     0x00000300     // MCN(18-25) = 0x0C
#define QE_CECR_MCN_ATM          0x00000280     // MCN(18-25) = 0x0A

#define QE_CECR_ASSIGN_PAGE      0x00000012     // OPCODE(26-31) = 0b010010
#define QE_CECR_STOP_TX          0x00000004

/*
 * Risc Sub-Block Codes
 */
#define QE_CECR_SBC_UCC1         0x100
#define QE_CECR_SBC_UCC2         0x110
#define QE_CECR_SBC_UCC3         0x120
#define QE_CECR_SBC_UCC4         0x130
#define QE_CECR_SBC_UCC5         0x140
#define QE_CECR_SBC_UCC6         0x150
#define QE_CECR_SBC_UCC7         0x160
#define QE_CECR_SBC_UCC8         0x170
#define QE_CECR_SBC_UCC1_SL      0x000
#define QE_CECR_SBC_UCC2_SL      0x010
#define QE_CECR_SBC_UCC3_SL      0x020
#define QE_CECR_SBC_UCC4_SL      0x030
#define QE_CECR_SBC_UCC5_SL      0x040
#define QE_CECR_SBC_UCC6_SL      0x050
#define QE_CECR_SBC_UCC7_SL      0x060
#define QE_CECR_SBC_UCC8_SL      0x070
#define QE_CECR_SBC_USB          0x190
#define QE_CECR_SBC_MCC          0x1c0
#define QE_CECR_SBC_GEN          0x1e0
#define QE_CECR_SBC_SPI1         0x0a0
#define QE_CECR_SBC_SPI2         0x0b0
#define QE_CECR_SBC_TIMER        0x0f0

/*
 * Risc ASSIGN_PAGE SNUM Codes
 */
#define QE_CECR_SNUM_UCC1_TX      0x00
#define QE_CECR_SNUM_UCC1_RX      0x01
#define QE_CECR_SNUM_UCC2_TX      0x10
#define QE_CECR_SNUM_UCC2_RX      0x11
#define QE_CECR_SNUM_UCC3_TX      0x20
#define QE_CECR_SNUM_UCC3_RX      0x21
#define QE_CECR_SNUM_UCC4_TX      0x30
#define QE_CECR_SNUM_UCC4_RX      0x31
#define QE_CECR_SNUM_UCC5_TX      0x40
#define QE_CECR_SNUM_UCC5_RX      0x41
#define QE_CECR_SNUM_UCC6_TX      0x50
#define QE_CECR_SNUM_UCC6_RX      0x51
#define QE_CECR_SNUM_UCC7_TX      0x60
#define QE_CECR_SNUM_UCC7_RX      0x61
#define QE_CECR_SNUM_UCC8_TX      0x70
#define QE_CECR_SNUM_UCC8_RX      0x71
#define QE_CECR_SNUM_USB_TX       0x90
#define QE_CECR_SNUM_USB_RX       0x91
#define QE_CECR_SNUM_SPI1_TX      0xa0
#define QE_CECR_SNUM_SPI1_RX      0xa1
#define QE_CECR_SNUM_SPI2_TX      0xb0
#define QE_CECR_SNUM_SPI2_RX      0xb1
#define QE_CECR_SNUM_MCC_TX       0xc0
#define QE_CECR_SNUM_MCC_RX       0xc1

/*
 * RISC commands
 */
#define QE_CECR_INIT_RXTX        0x00           /* Initalize both Rx and Tx */
#define QE_CECR_INIT_RX          0x01           /* Initalize Rx Parameters */
#define QE_CECR_INIT_TX          0x02           /* Initalize Tx Parameters */
#define QE_CECR_ENTER_HUNT       0x03           /* Enter Hunt Mode */
#define QE_CECR_TX_STOP          0x04           /* Ignorant Stop Tx */
#define QE_CECR_MCC_TX_STOP      0x04           /* Ignorant Stop Tx on ind ch*/
#define QE_CECR_GR_TX_STOP       0x05           /* Polite Stop Tx */
#define QE_CECR_TX_RESTART       0x06           /* Restart Tx */
#define QE_CECR_L2_SWITCH        0x07
#define QE_CECR_SET_GROUP        0x08           /* Set Ethernet Group Address */
#define QE_CECR_SET_TIMER        0x08           /* [De-]Activate Risc Timers */
#define QE_CECR_MCC_RX_STOP      0x09           /* force rx to terminate
                                                 *   reception on selected
                                                 *   channel */
#define QE_CECR_ATM_TX_CMD       0x0A           /* ATM transmit command */
#define QE_CECR_SET_PAGE         0x12
#define QE_CECR_SET_DEV_PAGE     0x16
#define QE_CECR_GR_RX_STOP       0x1A
#define QE_CECR_RX_RESTART       0x1B

#define QE_CECR_PUSHSCHED     0x0000000F          /* Push Sched */
#define QE_CECR_TIMER_CMD     0x01E10000          /* RISC Timer Command */
#define QE_CECDR_QE_INST_RAM  0x00800000          /* QE Instruction RAM */

/*
 * QMC channel commands
 */
#define QE_CECR_QMC_TX_STOP      0x1            /* Stop tx on the channel */
#define QE_CECR_QMC_RX_STOP      0x0            /* Stop rx on the channel */

/*
 * Risc Channel numbers.
 */
#define QE_CECR_CHAN_HDLC        0x0000
#define QE_CECR_CHAN_QMC         0x0002
#define QE_CECR_CHAN_UART        0x0004
#define QE_CECR_CHAN_ASYNC_HDLC  0x0006
#define QE_CECR_CHAN_BISYNC      0x0008
#define QE_CECR_CHAN_ATM         0x000A
#define QE_CECR_CHAN_ETHERNET    0x000C
#define QE_CECR_CHAN_L2SWITCH    0x000D

#define QE_CECR_SPIN_MAX         100

/*
 * QE RAM Control Defines */
#define QE_CERCR_MEE             0x8000
#define QE_CERCR_IEE             0x4000
#define QE_CERCR_CIR             0x0800


/**************************************************************************
 * QE Clock Multiplexer Registers        (0x00400 - 0x0043F)
 **************************************************************************/

typedef struct {
    volatile uint32_t  cmxgcr;        /* CMX general clock route */
    volatile uint32_t  cmxsi1cl_l;    /* CMX SI1 clock route low */
    volatile uint32_t  cmxsi1cr_h;    /* CMX SI1 clock route high */
    volatile uint32_t  cmxsi1syr;     /* CMX SI1 SYNC route */
    volatile uint32_t  cmxucr1;       /* CMX UCC1, UCC3 clock route */
    volatile uint32_t  cmxucr2;       /* CMX UCC5 clock route */
    volatile uint32_t  cmxucr3;       /* CMX UCC2, UCC4 clock route */
    volatile uint32_t  cmxucr4;       /* CMX UCC2, UCC4 clock route */
    volatile uint32_t  cmxupcr;       /* CMX UPC clock route */
    SPC(b, 0x1C)
} _PackedType QEMuxMap_t, qe_mux_t;


#define QE_CMXGCR_TS1CLS            0x30000000
#define QE_CMXGCR_TS1CLS_CLK11      0x00000000
#define QE_CMXGCR_TS1CLS_CLK12      0x10000000
#define QE_CMXGCR_TS1CLS_CLK21      0x20000000
#define QE_CMXGCR_TS1CLS_BRG11      0x30000000
#define QE_CMXGCR_TS2CLS            0x03000000
#define QE_CMXGCR_TS2CLS_CLK11      0x00000000
#define QE_CMXGCR_TS2CLS_CLK12      0x01000000
#define QE_CMXGCR_TS2CLS_CLK21      0x02000000
#define QE_CMXGCR_TS2CLS_BRG11      0x03000000
#define QE_CMXGCR_CTCLS             0x00300000
#define QE_CMXGCR_CTCLS_CLK11       0x00000000
#define QE_CMXGCR_CTCLS_CLK12       0x00100000
#define QE_CMXGCR_CTCLS_CLK21       0x00200000
#define QE_CMXGCR_CTCLS_BRG11       0x00300000
#define QE_CMXGCR_MEM               0x00007000
#define QE_CMXGCR_MEM_UCC1          0x00000000
#define QE_CMXGCR_MEM_UCC2          0x00001000
#define QE_CMXGCR_MEM_UCC3          0x00002000
#define QE_CMXGCR_MEM_UCC4          0x00003000
#define QE_CMXGCR_MEM_UCC5          0x00004000
#define QE_CMXGCR_MEM_UCC6          0x00005000
#define QE_CMXGCR_MEM_UCC7          0x00006000
#define QE_CMXGCR_MEM_UCC8          0x00007000
#define QE_CMXGCR_USBCS             0x0000000F
#define QE_CMXGCR_USBCS_DISABLED    0x00000000
#define QE_CMXGCR_USBCS_CLK3        0x00000001
#define QE_CMXGCR_USBCS_CLK5        0x00000002
#define QE_CMXGCR_USBCS_CLK7        0x00000003
#define QE_CMXGCR_USBCS_CLK9        0x00000004
#define QE_CMXGCR_USBCS_CLK13       0x00000005
#define QE_CMXGCR_USBCS_CLK17       0x00000006
#define QE_CMXGCR_USBCS_CLK19       0x00000007
#define QE_CMXGCR_USBCS_CLK21       0x00000008
#define QE_CMXGCR_USBCS_BRG9        0x00000009
#define QE_CMXGCR_USBCS_BRG10       0x0000000A

#define QE_CMXSI1CRL_RTA1CS         0x70000000
#define QE_CMXSI1CRL_RTA1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_RTA1CS_BRG3    0x10000000
#define QE_CMXSI1CRL_RTA1CS_BRG4    0x20000000
#define QE_CMXSI1CRL_RTA1CS_CLK1    0x40000000
#define QE_CMXSI1CRL_RTA1CS_CLK2    0x50000000
#define QE_CMXSI1CRL_RTA1CS_CLK3    0x60000000
#define QE_CMXSI1CRL_RTA1CS_CLK8    0x70000000
#define QE_CMXSI1CRL_RTB1CS         0x07000000
#define QE_CMXSI1CRL_RTB1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_RTB1CS_BRG3    0x01000000
#define QE_CMXSI1CRL_RTB1CS_BRG4    0x02000000
#define QE_CMXSI1CRL_RTB1CS_CLK1    0x04000000
#define QE_CMXSI1CRL_RTB1CS_CLK2    0x05000000
#define QE_CMXSI1CRL_RTB1CS_CLK5    0x06000000
#define QE_CMXSI1CRL_RTB1CS_CLK10   0x07000000
#define QE_CMXSI1CRL_RTC1CS         0x00700000
#define QE_CMXSI1CRL_RTC1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_RTC1CS_BRG3    0x00100000
#define QE_CMXSI1CRL_RTC1CS_BRG4    0x00200000
#define QE_CMXSI1CRL_RTC1CS_CLK1    0x00400000
#define QE_CMXSI1CRL_RTC1CS_CLK2    0x00500000
#define QE_CMXSI1CRL_RTC1CS_CLK7    0x00600000
#define QE_CMXSI1CRL_RTC1CS_CLK12   0x00700000
#define QE_CMXSI1CRL_RTD1CS         0x00070000
#define QE_CMXSI1CRL_RTD1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_RTD1CS_BRG3    0x00010000
#define QE_CMXSI1CRL_RTD1CS_BRG4    0x00020000
#define QE_CMXSI1CRL_RTD1CS_CLK1    0x00040000
#define QE_CMXSI1CRL_RTD1CS_CLK2    0x00050000
#define QE_CMXSI1CRL_RTD1CS_CLK9    0x00060000
#define QE_CMXSI1CRL_RTD1CS_CLK14   0x00070000
#define QE_CMXSI1CRL_TTA1CS         0x00007000
#define QE_CMXSI1CRL_TTA1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_TTA1CS_BRG3    0x00001000
#define QE_CMXSI1CRL_TTA1CS_BRG4    0x00002000
#define QE_CMXSI1CRL_TTA1CS_CLK1    0x00004000
#define QE_CMXSI1CRL_TTA1CS_CLK2    0x00005000
#define QE_CMXSI1CRL_TTA1CS_CLK4    0x00006000
#define QE_CMXSI1CRL_TTA1CS_CLK9    0x00007000
#define QE_CMXSI1CRL_TTB1CS         0x00000700
#define QE_CMXSI1CRL_TTB1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_TTB1CS_BRG3    0x00000100
#define QE_CMXSI1CRL_TTB1CS_BRG4    0x00000200
#define QE_CMXSI1CRL_TTB1CS_CLK1    0x00000400
#define QE_CMXSI1CRL_TTB1CS_CLK2    0x00000500
#define QE_CMXSI1CRL_TTB1CS_CLK6    0x00000600
#define QE_CMXSI1CRL_TTB1CS_CLK11   0x00000700
#define QE_CMXSI1CRL_TTC1CS         0x00000070
#define QE_CMXSI1CRL_TTC1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_TTC1CS_BRG3    0x00000010
#define QE_CMXSI1CRL_TTC1CS_BRG4    0x00000020
#define QE_CMXSI1CRL_TTC1CS_CLK1    0x00000040
#define QE_CMXSI1CRL_TTC1CS_CLK2    0x00000050
#define QE_CMXSI1CRL_TTC1CS_CLK8    0x00000060
#define QE_CMXSI1CRL_TTC1CS_CLK13   0x00000070
#define QE_CMXSI1CRL_TTD1CS         0x00000007
#define QE_CMXSI1CRL_TTD1CS_DISABLED 0x00000000
#define QE_CMXSI1CRL_TTD1CS_BRG3    0x00000001
#define QE_CMXSI1CRL_TTD1CS_BRG4    0x00000002
#define QE_CMXSI1CRL_TTD1CS_CLK1    0x00000004
#define QE_CMXSI1CRL_TTD1CS_CLK2    0x00000005
#define QE_CMXSI1CRL_TTD1CS_CLK10   0x00000006
#define QE_CMXSI1CRL_TTD1CS_CLK15   0x00000007

#define QE_CMXSI1CRH_RTE1CS         0x70000000
#define QE_CMXSI1CRH_RTE1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_RTE1CS_BRG12   0x10000000
#define QE_CMXSI1CRH_RTE1CS_BRG13   0x20000000
#define QE_CMXSI1CRH_RTE1CS_CLK23   0x40000000
#define QE_CMXSI1CRH_RTE1CS_CLK24   0x50000000
#define QE_CMXSI1CRH_RTE1CS_CLK11   0x60000000
#define QE_CMXSI1CRH_RTE1CS_CLK16   0x70000000
#define QE_CMXSI1CRH_RTF1CS         0x07000000
#define QE_CMXSI1CRH_RTF1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_RTF1CS_BRG12   0x01000000
#define QE_CMXSI1CRH_RTF1CS_BRG13   0x02000000
#define QE_CMXSI1CRH_RTF1CS_CLK23   0x04000000
#define QE_CMXSI1CRH_RTF1CS_CLK24   0x05000000
#define QE_CMXSI1CRH_RTF1CS_CLK13   0x06000000
#define QE_CMXSI1CRH_RTF1CS_CLK18   0x07000000
#define QE_CMXSI1CRH_RTG1CS         0x00700000
#define QE_CMXSI1CRH_RTG1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_RTG1CS_BRG12   0x00100000
#define QE_CMXSI1CRH_RTG1CS_BRG13   0x00200000
#define QE_CMXSI1CRH_RTG1CS_CLK23   0x00400000
#define QE_CMXSI1CRH_RTG1CS_CLK24   0x00500000
#define QE_CMXSI1CRH_RTG1CS_CLK15   0x00600000
#define QE_CMXSI1CRH_RTG1CS_CLK20   0x00700000
#define QE_CMXSI1CRH_RTH1CS         0x00070000
#define QE_CMXSI1CRH_RTH1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_RTH1CS_BRG12   0x00010000
#define QE_CMXSI1CRH_RTH1CS_BRG13   0x00020000
#define QE_CMXSI1CRH_RTH1CS_CLK23   0x00040000
#define QE_CMXSI1CRH_RTH1CS_CLK24   0x00050000
#define QE_CMXSI1CRH_RTH1CS_CLK17   0x00060000
#define QE_CMXSI1CRH_RTH1CS_CLK22   0x00070000
#define QE_CMXSI1CRH_TTE1CS         0x00007000
#define QE_CMXSI1CRH_TTE1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_TTE1CS_BRG12   0x00001000
#define QE_CMXSI1CRH_TTE1CS_BRG13   0x00002000
#define QE_CMXSI1CRH_TTE1CS_CLK23   0x00004000
#define QE_CMXSI1CRH_TTE1CS_CLK24   0x00005000
#define QE_CMXSI1CRH_TTE1CS_CLK12   0x00006000
#define QE_CMXSI1CRH_TTE1CS_CLK17   0x00007000
#define QE_CMXSI1CRH_TTF1CS         0x00000700
#define QE_CMXSI1CRH_TTF1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_TTF1CS_BRG12   0x00000100
#define QE_CMXSI1CRH_TTF1CS_BRG13   0x00000200
#define QE_CMXSI1CRH_TTF1CS_CLK23   0x00000400
#define QE_CMXSI1CRH_TTF1CS_CLK24   0x00000500
#define QE_CMXSI1CRH_TTF1CS_CLK14   0x00000600
#define QE_CMXSI1CRH_TTF1CS_CLK19   0x00000700
#define QE_CMXSI1CRH_TTG1CS         0x00000070
#define QE_CMXSI1CRH_TTG1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_TTG1CS_BRG12   0x00000010
#define QE_CMXSI1CRH_TTG1CS_BRG13   0x00000020
#define QE_CMXSI1CRH_TTG1CS_CLK23   0x00000040
#define QE_CMXSI1CRH_TTG1CS_CLK24   0x00000050
#define QE_CMXSI1CRH_TTG1CS_CLK16   0x00000060
#define QE_CMXSI1CRH_TTG1CS_CLK21   0x00000070
#define QE_CMXSI1CRH_TTH1CS         0x00000007
#define QE_CMXSI1CRH_TTH1CS_DISABLED 0x00000000
#define QE_CMXSI1CRH_TTH1CS_BRG12   0x00000001
#define QE_CMXSI1CRH_TTH1CS_BRG13   0x00000002
#define QE_CMXSI1CRH_TTH1CS_CLK23   0x00000004
#define QE_CMXSI1CRH_TTH1CS_CLK24   0x00000005
#define QE_CMXSI1CRH_TTH1CS_CLK18   0x00000006
#define QE_CMXSI1CRH_TTH1CS_CLK3    0x00000007

#define QE_CMXSI1SYR_RTA1SS           0xC0000000
#define QE_CMXSI1SYR_RTA1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTA1SS_BRG9      0x40000000
#define QE_CMXSI1SYR_RTA1SS_BRG10     0x80000000
#define QE_CMXSI1SYR_RTB1SS           0x30000000
#define QE_CMXSI1SYR_RTB1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTB1SS_BRG9      0x10000000
#define QE_CMXSI1SYR_RTB1SS_BRG10     0x20000000
#define QE_CMXSI1SYR_RTC1SS           0x0C000000
#define QE_CMXSI1SYR_RTC1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTC1SS_BRG9      0x04000000
#define QE_CMXSI1SYR_RTC1SS_BRG11     0x08000000
#define QE_CMXSI1SYR_RTD1SS           0x03000000
#define QE_CMXSI1SYR_RTD1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTD1SS_BRG9      0x01000000
#define QE_CMXSI1SYR_RTD1SS_BRG11     0x02000000
#define QE_CMXSI1SYR_RTE1SS           0x00C00000
#define QE_CMXSI1SYR_RTE1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTE1SS_BRG13     0x00400000
#define QE_CMXSI1SYR_RTE1SS_BRG14     0x00800000
#define QE_CMXSI1SYR_RTF1SS           0x00300000
#define QE_CMXSI1SYR_RTF1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTF1SS_BRG13     0x00100000
#define QE_CMXSI1SYR_RTF1SS_BRG14     0x00200000
#define QE_CMXSI1SYR_RTG1SS           0x000C0000
#define QE_CMXSI1SYR_RTG1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTG1SS_BRG13     0x00040000
#define QE_CMXSI1SYR_RTG1SS_BRG15     0x00080000
#define QE_CMXSI1SYR_RTH1SS           0x00030000
#define QE_CMXSI1SYR_RTH1SS_RSYNC     0x00000000
#define QE_CMXSI1SYR_RTH1SS_BRG13     0x00010000
#define QE_CMXSI1SYR_RTH1SS_BRG15     0x00020000
#define QE_CMXSI1SYR_TTA1SS           0x0000C000
#define QE_CMXSI1SYR_TTA1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTA1SS_BRG9      0x00004000
#define QE_CMXSI1SYR_TTA1SS_BRG10     0x00008000
#define QE_CMXSI1SYR_TTB1SS           0x00003000
#define QE_CMXSI1SYR_TTB1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTB1SS_BRG9      0x00001000
#define QE_CMXSI1SYR_TTB1SS_BRG10     0x00002000
#define QE_CMXSI1SYR_TTC1SS           0x00000C00
#define QE_CMXSI1SYR_TTC1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTC1SS_BRG9      0x00000400
#define QE_CMXSI1SYR_TTC1SS_BRG11     0x00000800
#define QE_CMXSI1SYR_TTD1SS           0x00000300
#define QE_CMXSI1SYR_TTD1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTD1SS_BRG9      0x00000100
#define QE_CMXSI1SYR_TTD1SS_BRG11     0x00000200
#define QE_CMXSI1SYR_TTE1SS           0x000000C0
#define QE_CMXSI1SYR_TTE1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTE1SS_BRG13     0x00000040
#define QE_CMXSI1SYR_TTE1SS_BRG14     0x00000080
#define QE_CMXSI1SYR_TTF1SS           0x00000030
#define QE_CMXSI1SYR_TTF1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTF1SS_BRG13     0x00000010
#define QE_CMXSI1SYR_TTF1SS_BRG14     0x00000020
#define QE_CMXSI1SYR_TTG1SS           0x0000000C
#define QE_CMXSI1SYR_TTG1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTG1SS_BRG13     0x00000004
#define QE_CMXSI1SYR_TTG1SS_BRG15     0x00000008
#define QE_CMXSI1SYR_TTH1SS           0x00000003
#define QE_CMXSI1SYR_TTH1SS_TSYNC     0x00000000
#define QE_CMXSI1SYR_TTH1SS_BRG13     0x00000001
#define QE_CMXSI1SYR_TTH1SS_BRG15     0x00000002

#define QE_CMXUCR1_GR1           0x80000000
#define QE_CMXUCR1_UC1           0x40000000
#define QE_CMXUCR1_RU1CS         0x00F00000
#define QE_CMXUCR1_RU1CS_DISABLED 0x00000000
#define QE_CMXUCR1_RU1CS_BRG1    0x00100000
#define QE_CMXUCR1_RU1CS_BRG2    0x00200000
#define QE_CMXUCR1_RU1CS_BRG7    0x00300000
#define QE_CMXUCR1_RU1CS_BRG8    0x00400000
#define QE_CMXUCR1_RU1CS_CLK9    0x00500000
#define QE_CMXUCR1_RU1CS_CLK10   0x00600000
#define QE_CMXUCR1_RU1CS_CLK11   0x00700000
#define QE_CMXUCR1_RU1CS_CLK12   0x00800000
#define QE_CMXUCR1_RU1CS_CLK15   0x00900000
#define QE_CMXUCR1_RU1CS_CLK16   0x00A00000
#define QE_CMXUCR1_TU1CS         0x000F0000
#define QE_CMXUCR1_TU1CS_DISABLED 0x00000000
#define QE_CMXUCR1_TU1CS_BRG1    0x00010000
#define QE_CMXUCR1_TU1CS_BRG2    0x00020000
#define QE_CMXUCR1_TU1CS_BRG7    0x00030000
#define QE_CMXUCR1_TU1CS_BRG8    0x00040000
#define QE_CMXUCR1_TU1CS_CLK9    0x00050000
#define QE_CMXUCR1_TU1CS_CLK10   0x00060000
#define QE_CMXUCR1_TU1CS_CLK11   0x00070000
#define QE_CMXUCR1_TU1CS_CLK12   0x00080000
#define QE_CMXUCR1_TU1CS_CLK15   0x00090000
#define QE_CMXUCR1_TU1CS_CLK16   0x000A0000
#define QE_CMXUCR1_GR3           0x00008000
#define QE_CMXUCR1_UC3           0x00004000
#define QE_CMXUCR1_HBM3          0x00002000
#define QE_CMXUCR1_RU3CS         0x000000F00000
#define QE_CMXUCR1_RU3CS_DISABLED 0x00000000
#define QE_CMXUCR1_RU3CS_BRG1    0x00000010
#define QE_CMXUCR1_RU3CS_BRG2    0x00000020
#define QE_CMXUCR1_RU3CS_BRG7    0x00000030
#define QE_CMXUCR1_RU3CS_BRG8    0x00000040
#define QE_CMXUCR1_RU3CS_CLK9    0x00000050
#define QE_CMXUCR1_RU3CS_CLK10   0x00000060
#define QE_CMXUCR1_RU3CS_CLK11   0x00000070
#define QE_CMXUCR1_RU3CS_CLK12   0x00000080
#define QE_CMXUCR1_RU3CS_CLK15   0x00000090
#define QE_CMXUCR1_RU3CS_CLK16   0x000000A0
#define QE_CMXUCR1_TU3CS         0x0000000F
#define QE_CMXUCR1_TU3CS_DISABLED 0x00000000
#define QE_CMXUCR1_TU3CS_BRG1    0x00000001
#define QE_CMXUCR1_TU3CS_BRG2    0x00000002
#define QE_CMXUCR1_TU3CS_BRG7    0x00000003
#define QE_CMXUCR1_TU3CS_BRG8    0x00000004
#define QE_CMXUCR1_TU3CS_CLK9    0x00000005
#define QE_CMXUCR1_TU3CS_CLK10   0x00000006
#define QE_CMXUCR1_TU3CS_CLK11   0x00000007
#define QE_CMXUCR1_TU3CS_CLK12   0x00000008
#define QE_CMXUCR1_TU3CS_CLK15   0x00000009
#define QE_CMXUCR1_TU3CS_CLK16   0x0000000A

#define QE_CMXUCR2_GR5           0x80000000
#define QE_CMXUCR2_UC5           0x40000000
#define QE_CMXUCR2_RU5CS         0x00F00000
#define QE_CMXUCR2_RU5CS_DISABLED 0x00000000
#define QE_CMXUCR2_RU5CS_BRG5    0x00100000
#define QE_CMXUCR2_RU5CS_BRG6    0x00200000
#define QE_CMXUCR2_RU5CS_BRG7    0x00300000
#define QE_CMXUCR2_RU5CS_BRG8    0x00400000
#define QE_CMXUCR2_RU5CS_CLK13   0x00500000
#define QE_CMXUCR2_RU5CS_CLK14   0x00600000
#define QE_CMXUCR2_RU5CS_CLK19   0x00700000
#define QE_CMXUCR2_RU5CS_CLK20   0x00800000
#define QE_CMXUCR2_RU5CS_CLK15   0x00900000
#define QE_CMXUCR2_RU5CS_CLK16   0x00A00000
#define QE_CMXUCR2_TU5CS         0x000F0000
#define QE_CMXUCR2_TU5CS_DISABLED 0x00000000
#define QE_CMXUCR2_TU5CS_BRG5    0x00010000
#define QE_CMXUCR2_TU5CS_BRG6    0x00020000
#define QE_CMXUCR2_TU5CS_BRG7    0x00030000
#define QE_CMXUCR2_TU5CS_BRG8    0x00040000
#define QE_CMXUCR2_TU5CS_CLK13   0x00050000
#define QE_CMXUCR2_TU5CS_CLK14   0x00060000
#define QE_CMXUCR2_TU5CS_CLK19   0x00070000
#define QE_CMXUCR2_TU5CS_CLK20   0x00080000
#define QE_CMXUCR2_TU5CS_CLK15   0x00090000
#define QE_CMXUCR2_TU5CS_CLK16   0x000A0000
#define QE_CMXUCR2_GR7           0x00008000
#define QE_CMXUCR2_UC7           0x00004000
#define QE_CMXUCR2_RU7CS         0x000000F00000
#define QE_CMXUCR2_RU7CS_DISABLED 0x00000000
#define QE_CMXUCR2_RU7CS_BRG5    0x00000010
#define QE_CMXUCR2_RU7CS_BRG6    0x00000020
#define QE_CMXUCR2_RU7CS_BRG7    0x00000030
#define QE_CMXUCR2_RU7CS_BRG8    0x00000040
#define QE_CMXUCR2_RU7CS_CLK13   0x00000050
#define QE_CMXUCR2_RU7CS_CLK14   0x00000060
#define QE_CMXUCR2_RU7CS_CLK19   0x00000070
#define QE_CMXUCR2_RU7CS_CLK20   0x00000080
#define QE_CMXUCR2_RU7CS_CLK15   0x00000090
#define QE_CMXUCR2_RU7CS_CLK16   0x000000A0
#define QE_CMXUCR2_TU7CS         0x0000000F
#define QE_CMXUCR2_TU7CS_DISABLED 0x00000000
#define QE_CMXUCR2_TU7CS_BRG5    0x00000001
#define QE_CMXUCR2_TU7CS_BRG6    0x00000002
#define QE_CMXUCR2_TU7CS_BRG7    0x00000003
#define QE_CMXUCR2_TU7CS_BRG8    0x00000004
#define QE_CMXUCR2_TU7CS_CLK13   0x00000005
#define QE_CMXUCR2_TU7CS_CLK14   0x00000006
#define QE_CMXUCR2_TU7CS_CLK19   0x00000007
#define QE_CMXUCR2_TU7CS_CLK20   0x00000008
#define QE_CMXUCR2_TU7CS_CLK15   0x00000009
#define QE_CMXUCR2_TU7CS_CLK16   0x0000000A

#define QE_CMXUCR3_GR2           0x80000000
#define QE_CMXUCR3_UC2           0x40000000
#define QE_CMXUCR3_RU2CS         0x00F00000
#define QE_CMXUCR3_RU2CS_DISABLED 0x00000000
#define QE_CMXUCR3_RU2CS_BRG3    0x00100000
#define QE_CMXUCR3_RU2CS_BRG4    0x00200000
#define QE_CMXUCR3_RU2CS_BRG15   0x00300000
#define QE_CMXUCR3_RU2CS_BRG16   0x00400000
#define QE_CMXUCR3_RU2CS_CLK3    0x00500000
#define QE_CMXUCR3_RU2CS_CLK4    0x00600000
#define QE_CMXUCR3_RU2CS_CLK17   0x00700000
#define QE_CMXUCR3_RU2CS_CLK18   0x00800000
#define QE_CMXUCR3_RU2CS_CLK7    0x00900000
#define QE_CMXUCR3_RU2CS_CLK8    0x00A00000
#define QE_CMXUCR3_RU2CS_CLK16   0x00B00000
#define QE_CMXUCR3_TU2CS         0x000F0000
#define QE_CMXUCR3_TU2CS_DISABLED 0x00000000
#define QE_CMXUCR3_TU2CS_BRG3    0x00010000
#define QE_CMXUCR3_TU2CS_BRG4    0x00020000
#define QE_CMXUCR3_TU2CS_BRG15   0x00030000
#define QE_CMXUCR3_TU2CS_BRG16   0x00040000
#define QE_CMXUCR3_TU2CS_CLK3    0x00050000
#define QE_CMXUCR3_TU2CS_CLK4    0x00060000
#define QE_CMXUCR3_TU2CS_CLK17   0x00070000
#define QE_CMXUCR3_TU2CS_CLK18   0x00080000
#define QE_CMXUCR3_TU2CS_CLK7    0x00090000
#define QE_CMXUCR3_TU2CS_CLK8    0x000A0000
#define QE_CMXUCR3_TU2CS_CLK16   0x000B0000
#define QE_CMXUCR3_GR4           0x00008000
#define QE_CMXUCR3_UC4           0x00004000
#define QE_CMXUCR3_RU4CS         0x000000F0
#define QE_CMXUCR3_RU4CS_DISABLED 0x00000000
#define QE_CMXUCR3_RU4CS_BRG3    0x00000010
#define QE_CMXUCR3_RU4CS_BRG4    0x00000020
#define QE_CMXUCR3_RU4CS_BRG15   0x00000030
#define QE_CMXUCR3_RU4CS_BRG16   0x00000040
#define QE_CMXUCR3_RU4CS_CLK3    0x00000050
#define QE_CMXUCR3_RU4CS_CLK4    0x00000060
#define QE_CMXUCR3_RU4CS_CLK17   0x00000070
#define QE_CMXUCR3_RU4CS_CLK18   0x00000080
#define QE_CMXUCR3_RU4CS_CLK7    0x00000090
#define QE_CMXUCR3_RU4CS_CLK8    0x000000A0
#define QE_CMXUCR3_RU4CS_CLK16   0x000000B0
#define QE_CMXUCR3_TU4CS         0x0000000F
#define QE_CMXUCR3_TU4CS_DISABLED 0x00000000
#define QE_CMXUCR3_TU4CS_BRG3    0x00000001
#define QE_CMXUCR3_TU4CS_BRG4    0x00000002
#define QE_CMXUCR3_TU4CS_BRG15   0x00000003
#define QE_CMXUCR3_TU4CS_BRG16   0x00000004
#define QE_CMXUCR3_TU4CS_CLK3    0x00000005
#define QE_CMXUCR3_TU4CS_CLK4    0x00000006
#define QE_CMXUCR3_TU4CS_CLK17   0x00000007
#define QE_CMXUCR3_TU4CS_CLK18   0x00000008
#define QE_CMXUCR3_TU4CS_CLK7    0x00000009
#define QE_CMXUCR3_TU4CS_CLK8    0x0000000A
#define QE_CMXUCR3_TU4CS_CLK16   0x0000000B

#define QE_CMXUCR4_GR6           0x80000000
#define QE_CMXUCR4_UC6           0x40000000
#define QE_CMXUCR4_RU6CS         0x00F00000
#define QE_CMXUCR4_RU6CS_DISABLED 0x00000000
#define QE_CMXUCR4_RU6CS_BRG13   0x00100000
#define QE_CMXUCR4_RU6CS_BRG14   0x00200000
#define QE_CMXUCR4_RU6CS_BRG15   0x00300000
#define QE_CMXUCR4_RU6CS_BRG16   0x00400000
#define QE_CMXUCR4_RU6CS_CLK5    0x00500000
#define QE_CMXUCR4_RU6CS_CLK6    0x00600000
#define QE_CMXUCR4_RU6CS_CLK21   0x00700000
#define QE_CMXUCR4_RU6CS_CLK22   0x00800000
#define QE_CMXUCR4_RU6CS_CLK7    0x00900000
#define QE_CMXUCR4_RU6CS_CLK8    0x00A00000
#define QE_CMXUCR4_RU6CS_CLK16   0x00B00000
#define QE_CMXUCR4_TU6CS         0x000F0000
#define QE_CMXUCR4_TU6CS_DISABLED 0x00000000
#define QE_CMXUCR4_TU6CS_BRG13   0x00010000
#define QE_CMXUCR4_TU6CS_BRG14   0x00020000
#define QE_CMXUCR4_TU6CS_BRG15   0x00030000
#define QE_CMXUCR4_TU6CS_BRG16   0x00040000
#define QE_CMXUCR4_TU6CS_CLK5    0x00050000
#define QE_CMXUCR4_TU6CS_CLK6    0x00060000
#define QE_CMXUCR4_TU6CS_CLK21   0x00070000
#define QE_CMXUCR4_TU6CS_CLK22   0x00080000
#define QE_CMXUCR4_TU6CS_CLK7    0x00090000
#define QE_CMXUCR4_TU6CS_CLK8    0x000A0000
#define QE_CMXUCR4_TU6CS_CLK16   0x000B0000
#define QE_CMXUCR4_GR8           0x00008000
#define QE_CMXUCR4_UC8           0x00004000
#define QE_CMXUCR4_RU8CS         0x000000F0
#define QE_CMXUCR4_RU8CS_DISABLED 0x00000000
#define QE_CMXUCR4_RU8CS_BRG13   0x00000010
#define QE_CMXUCR4_RU8CS_BRG14   0x00000020
#define QE_CMXUCR4_RU8CS_BRG15   0x00000030
#define QE_CMXUCR4_RU8CS_BRG16   0x00000040
#define QE_CMXUCR4_RU8CS_CLK5    0x00000050
#define QE_CMXUCR4_RU8CS_CLK6    0x00000060
#define QE_CMXUCR4_RU8CS_CLK21   0x00000070
#define QE_CMXUCR4_RU8CS_CLK22   0x00000080
#define QE_CMXUCR4_RU8CS_CLK7    0x00000090
#define QE_CMXUCR4_RU8CS_CLK8    0x000000A0
#define QE_CMXUCR4_RU8CS_CLK16   0x000000B0
#define QE_CMXUCR4_TU8CS         0x0000000F
#define QE_CMXUCR4_TU8CS_DISABLED 0x00000000
#define QE_CMXUCR4_TU8CS_BRG13   0x00000001
#define QE_CMXUCR4_TU8CS_BRG14   0x00000002
#define QE_CMXUCR4_TU8CS_BRG15   0x00000003
#define QE_CMXUCR4_TU8CS_BRG16   0x00000004
#define QE_CMXUCR4_TU8CS_CLK5    0x00000005
#define QE_CMXUCR4_TU8CS_CLK6    0x00000006
#define QE_CMXUCR4_TU8CS_CLK21   0x00000007
#define QE_CMXUCR4_TU8CS_CLK22   0x00000008
#define QE_CMXUCR4_TU8CS_CLK7    0x00000009
#define QE_CMXUCR4_TU8CS_CLK8    0x0000000A
#define QE_CMXUCR4_TU8CS_CLK16   0x0000000B

#define QE_CMXUPCR_UPC1RCS          0xF0000000
#define QE_CMXUPCR_UPC1RCS_DISABLED 0x00000000
#define QE_CMXUPCR_UPC1RCS_BRG5     0x10000000
#define QE_CMXUPCR_UPC1RCS_BRG6     0x20000000
#define QE_CMXUPCR_UPC1RCS_BRG7     0x30000000
#define QE_CMXUPCR_UPC1RCS_BRG8     0x40000000
#define QE_CMXUPCR_UPC1RCS_CLK13    0x50000000
#define QE_CMXUPCR_UPC1RCS_CLK14    0x60000000
#define QE_CMXUPCR_UPC1RCS_CLK15    0x70000000
#define QE_CMXUPCR_UPC1RCS_CLK16    0x80000000
#define QE_CMXUPCR_UPC1RCS_CLK19    0x90000000
#define QE_CMXUPCR_UPC1RCS_CLK20    0xA0000000
#define QE_CMXUPCR_UPC1TCS          0x0F000000
#define QE_CMXUPCR_UPC1TCS_DISABLED 0x00000000
#define QE_CMXUPCR_UPC1TCS_BRG5     0x01000000
#define QE_CMXUPCR_UPC1TCS_BRG6     0x02000000
#define QE_CMXUPCR_UPC1TCS_BRG7     0x03000000
#define QE_CMXUPCR_UPC1TCS_BRG8     0x04000000
#define QE_CMXUPCR_UPC1TCS_CLK13    0x05000000
#define QE_CMXUPCR_UPC1TCS_CLK14    0x06000000
#define QE_CMXUPCR_UPC1TCS_CLK15    0x07000000
#define QE_CMXUPCR_UPC1TCS_CLK16    0x08000000
#define QE_CMXUPCR_UPC1TCS_CLK19    0x09000000
#define QE_CMXUPCR_UPC1TCS_CLK20    0x0A000000
#define QE_CMXUPCR_UPC1IRCS         0x00070000
#define QE_CMXUPCR_UPC1IRCS_UPC1    0x00000000
#define QE_CMXUPCR_UPC1IRCS_BRG3    0x00010000
#define QE_CMXUPCR_UPC1IRCS_BRG4    0x00020000
#define QE_CMXUPCR_UPC1IRCS_CLK18   0x00030000
#define QE_CMXUPCR_UPC1IRCS_CLK19   0x00040000
#define QE_CMXUPCR_UPC1IRCS_CLK17   0x00050000
#define QE_CMXUPCR_UPC2RCS          0x0000F000
#define QE_CMXUPCR_UPC2RCS_DISABLED 0x00000000
#define QE_CMXUPCR_UPC2RCS_BRG13    0x00001000
#define QE_CMXUPCR_UPC2RCS_BRG14    0x00002000
#define QE_CMXUPCR_UPC2RCS_BRG15    0x00003000
#define QE_CMXUPCR_UPC2RCS_BRG16    0x00004000
#define QE_CMXUPCR_UPC2RCS_CLK5     0x00005000
#define QE_CMXUPCR_UPC2RCS_CLK6     0x00006000
#define QE_CMXUPCR_UPC2RCS_CLK7     0x00007000
#define QE_CMXUPCR_UPC2RCS_CLK8     0x00008000
#define QE_CMXUPCR_UPC2RCS_CLK19    0x00009000
#define QE_CMXUPCR_UPC2RCS_CLK20    0x0000A000
#define QE_CMXUPCR_UPC2TCS          0x00000F00
#define QE_CMXUPCR_UPC2TCS_DISABLED 0x00000000
#define QE_CMXUPCR_UPC2TCS_BRG13    0x00000100
#define QE_CMXUPCR_UPC2TCS_BRG14    0x00000200
#define QE_CMXUPCR_UPC2TCS_BRG15    0x00000300
#define QE_CMXUPCR_UPC2TCS_BRG16    0x00000400
#define QE_CMXUPCR_UPC2TCS_CLK5     0x00000500
#define QE_CMXUPCR_UPC2TCS_CLK6     0x00000600
#define QE_CMXUPCR_UPC2TCS_CLK7     0x00000700
#define QE_CMXUPCR_UPC2TCS_CLK8     0x00000800
#define QE_CMXUPCR_UPC2TCS_CLK19    0x00000900
#define QE_CMXUPCR_UPC2TCS_CLK20    0x00000A00
#define QE_CMXUPCR_UPC2IRCS         0x00000007
#define QE_CMXUPCR_UPC2IRCS_UPC2    0x00000000
#define QE_CMXUPCR_UPC2IRCS_BRG3    0x00000001
#define QE_CMXUPCR_UPC2IRCS_BRG4    0x00000002
#define QE_CMXUPCR_UPC2IRCS_CLK18   0x00000003
#define QE_CMXUPCR_UPC2IRCS_CLK19   0x00000004
#define QE_CMXUPCR_UPC2IRCS_CLK20   0x00000005



/**************************************************************************
 * QE Timers                                (0x00440 - 0x0047F)
 **************************************************************************/

typedef struct {
    volatile uint8_t   gtcfr1;        /* Tmr 1 and tmr 2 global config */
    SPC(a, 3)
    volatile uint8_t   gtcfr2;        /* Tmr 3 and tmr 4 global config */
    SPC(b, 11)
    volatile uint16_t  gtmdr1;        /* Timer 1 mode */
    volatile uint16_t  gtmdr2;        /* Timer 2 mode */
    volatile uint16_t  gtrfr1;        /* Timer 1 reference */
    volatile uint16_t  gtrfr2;        /* Timer 2 reference */
    volatile uint16_t  gtcpr1;        /* Timer 1 capture */
    volatile uint16_t  gtcpr2;        /* Timer 2 capture */
    volatile uint16_t  gtcnr1;        /* Timer 1 coutner */
    volatile uint16_t  gtcnr2;        /* Timer 2 coutner */
    volatile uint16_t  gtmdr3;        /* Timer 3 mode */
    volatile uint16_t  gtmdr4;        /* Timer 4 mode */
    volatile uint16_t  gtrfr3;        /* Timer 3 reference */
    volatile uint16_t  gtrfr4;        /* Timer 4 reference */
    volatile uint16_t  gtcpr3;        /* Timer 3 capture */
    volatile uint16_t  gtcpr4;        /* Timer 4 capture */
    volatile uint16_t  gtcnr3;        /* Timer 3 coutner */
    volatile uint16_t  gtcnr4;        /* Timer 4 coutner */
    volatile uint16_t  gtevr1;        /* Timer 1 event */
    volatile uint16_t  gtevr2;        /* Timer 2 event */
    volatile uint16_t  gtevr3;        /* Timer 3 event */
    volatile uint16_t  gtevr4;        /* Timer 4 event */
    volatile uint16_t  gtps;          /* Timer prescale */
    SPC(c, 6)
} _PackedType QETimerMap_t, qe_timer_t;



/**************************************************************************
 * SPI 1 Registers                        (0x004C0 - 0x004FF)
 * SPI 2 Registers                        (0x00500 - 0x0053F)
 * RJULIAN - Is it 4c0 or 4e0?
 **************************************************************************/

typedef struct {
    SPC(a, 0x20)
    volatile uint32_t  spmode;        /* SPI mode */
    SPC(b, 2)
    volatile uint8_t   spie;          /* SPI event */
    SPC(c, 3)
    volatile uint8_t   spim;          /* SPI mask */
    SPC(d, 2)
    volatile uint8_t   spcom;         /* SPI command */
    SPC(e, 2)
    volatile uint32_t  spitd;         /* SPI transmit data */
    volatile uint32_t  spird;         /* SPI receive data */
    SPC(f, 8)
} _PackedType QESPIMap_t, qe_spi_t;





/*
 * spi_spmode - SPI Mode Reg
 */
#define QE_SPI_SPMODE_EM             0x80000000
#define QE_SPI_SPMODE_LOOP           0x40000000
#define QE_SPI_SPMODE_CI             0x20000000
#define QE_SPI_SPMODE_CP             0x10000000
#define QE_SPI_SPMODE_DIV16          0x08000000
#define QE_SPI_SPMODE_REV            0x04000000
#define QE_SPI_SPMODE_MS             0x02000000
#define QE_SPI_SPMODE_EN             0x01000000
#define QE_SPI_SPMODE_LEN(y)         (((y) & 0xf) << 20)
#define QE_SPI_SPMODE_PM(y)          (((y) & 0xf) << 16)
#define QE_SPI_SPMODE_OP             0x00004000
#define QE_SPI_SPMODE_MII            0x00002000
#define PQICCC_SPI_SPMODE_CG(y)          (((y) & 0x1f) << 7)

// SNH FIXME
#define QE_DEFAULT_SPI_MODE          (QE_SPI_SPMODE_MS | \
                                          QE_SPI_SPMODE_LEN(7) | \
                                          QE_SPI_SPMODE_REV |    \
                                          QE_SPI_SPMODE_DIV16 |  \
                                          QE_SPI_SPMODE_PM(15))

#define QE_QUACK_SPI_MODE             QE_DEFAULT_SPI_MODE

/* default largest SPI comment */
#define QE_SPI_DEFAULT_MTU            4

/* RJULIAN - Which one is correct? */
#define QE_DEFAULT_SPI_MODE1         (QE_SPMODE_DIV16     | \
                                          QE_SPMODE_REV       | \
                                          QE_SPMODE_MASTER    | \
                                          QE_SPMODE_LEN(9) )

/*
 * spi_spie - SPI Event Reg
 */
#define QE_SPI_SPIE_MME              0x20
#define QE_SPI_SPIE_TXE              0x10
#define QE_SPI_SPIE_BSY              0x04
#define QE_SPI_SPIE_TXB              0x02
#define QE_SPI_SPIE_RXB              0x01
#define QE_SPI_SPIE_ALL              0x37

/*
 * spi_spim - SPI Mask Reg
 */
#define QE_SPI_SPIM_MME              0x20
#define QE_SPI_SPIM_TXE              0x10
#define QE_SPI_SPIM_BSY              0x04
#define QE_SPI_SPIM_TXB              0x02
#define QE_SPI_SPIM_RXB              0x01
#define QE_SPI_SPIM_ALL              0x37

/*
 * spi_spcom - SPI Command Reg
 */
#define QE_SPI_SPCOM_STR             0x80

/*
 * PowerQUICC Buffer Descriptors - A single generic buffer descriptor is used
 *   for most interfaces and protocols.  Exceptions are currently limited to
 *   the IDMA, which requires function-code registers, as well as independant
 *   pointers for source and destination data.
 */
typedef struct qe_bd_ {
    volatile uint16_t        status;     /* Status and Control */
    volatile uint16_t        length;     /* Length of Data in buffer */
    volatile uint8_t         *buf_ptr;   /* Pointer to Data buffer */
} _PackedType qe_bd_t __attribute__ ((aligned (8)));

/*
 * Buffer Descriptor Flags - The various bits of the 'status' field of 
 *    a buffer descriptor are described here.
 *
 *    All bits which do not have a specific name associated with them are
 *    reserved, and should be set to 0.
 */
#define QE_SPI_RX_BDSTAT_EMPTY       0x8000        /* Empty */
#define QE_SPI_RX_BDSTAT_WRAP        0x2000        /* Wrap */
#define QE_SPI_RX_BDSTAT_RUPT        0x1000        /* Interrupt */
#define QE_SPI_RX_BDSTAT_LAST        0x0800        /* Last Character */
#define QE_SPI_RX_BDSTAT_CM          0x0200        /* Continuous Mode */
#define QE_SPI_RX_BDSTAT_OV          0x0002        /* Overrun */
#define QE_SPI_RX_BDSTAT_ME          0x0001        /* Multimaster Error */

#define QE_SPI_TX_BDSTAT_READY       0x8000        /* Ready */
#define QE_SPI_TX_BDSTAT_WRAP        0x2000        /* Wrap */
#define QE_SPI_TX_BDSTAT_RUPT        0x1000        /* Interrupt */
#define QE_SPI_TX_BDSTAT_LAST        0x0800        /* Last Character */
#define QE_SPI_TX_BDSTAT_CM          0x0200        /* Continuous Mode */
#define QE_SPI_TX_BDSTAT_UN          0x0002        /* Underrun */
#define QE_SPI_TX_BDSTAT_ME          0x0001        /* Multimaster Error */

/*
 * Misc constants and macros
 */

/* max time for SPI cmd to complete */
#define QE_SPI_SPIN_MAX              0x4000

#if defined(MPC836x)
/**************************************************************************
 * MCC                                         (0x00540 - 0x0057F)
 * Only for MPC836x
 **************************************************************************/

typedef struct {
    volatile uint32_t  mcce;          /* MCC event */
    volatile uint32_t  mccm;          /* MCC mask */
    volatile uint32_t  mccf;          /* MCC configuration */
    volatile uint32_t  merl;          /* MCC emergency request level */
    SPC(a, 0x30)
} _PackedType QEMCCMap_t, qe_mcc_t;
#endif

/**************************************************************************
 * Baud Rate Generator Registers (BRG)        (0x00640 - 0x006BF)
 **************************************************************************/
typedef struct {
    volatile uint32_t  brgc1;         /* BRG  1 configuration */
    volatile uint32_t  brgc2;         /* BRG  2 configuration */
    volatile uint32_t  brgc3;         /* BRG  3 configuration */
    volatile uint32_t  brgc4;         /* BRG  4 configuration */
    volatile uint32_t  brgc5;         /* BRG  5 configuration */
    volatile uint32_t  brgc6;         /* BRG  6 configuration */
    volatile uint32_t  brgc7;         /* BRG  7 configuration */
    volatile uint32_t  brgc8;         /* BRG  8 configuration */
    volatile uint32_t  brgc9;         /* BRG  9 configuration */
    volatile uint32_t  brgc10;        /* BRG 10 configuration */
    volatile uint32_t  brgc11;        /* BRG 11 configuration */

    volatile uint32_t  brgc12;        /* BRG 12 configuration */
    volatile uint32_t  brgc13;        /* BRG 13 configuration */
    volatile uint32_t  brgc14;        /* BRG 14 configuration */

    volatile uint32_t  brgc15;        /* BRG 15 configuration */
    volatile uint32_t  brgc16;        /* BRG 16 configuration */
    SPC(b, 0x40)
} _PackedType QEBRGMap_t, qe_brg_t;

#define QE_BRGCX_RST               0x00020000
#define QE_BRGCX_EN                0x00010000
#define QE_BRGCX_EXTC              0x0000C000
#define QE_BRGCX_EXTC_BRGCLK       0x00000000
#define QE_BRGCX_EXTC_01           0x00004000
#define QE_BRGCX_EXTC_10           0x00008000
#define QE_BRGCX_ATB               0x00002000


#define QE_BRGCX_CD(y)		       ((y&0xfff) << 1)
#define QE_BRGCX_CD_SHIFT          0x00000001
#define QE_BRGCX_DIV16             0x00000001

#define QE_BRG1                    0x1     /* Baud Rate Generator 1 */
#define QE_BRG2                    0x2     /* Baud Rate Generator 2 */
#define QE_BRG3                    0x3     /* Baud Rate Generator 3 */
#define QE_BRG4                    0x4     /* Baud Rate Generator 4 */
#define QE_BRG5                    0x5     /* Baud Rate Generator 5 */
#define QE_BRG6                    0x6     /* Baud Rate Generator 6 */
#define QE_BRG7                    0x7     /* Baud Rate Generator 7 */
#define QE_BRG8                    0x8     /* Baud Rate Generator 8 */
#define QE_BRG9                    0x9     /* Baud Rate Generator 9 */
#define QE_BRG10                   0xa     /* Baud Rate Generator 10 */
#define QE_BRG11                   0xb     /* Baud Rate Generator 11 */
#define QE_BRG12                   0xc     /* Baud Rate Generator 12 */
#define QE_BRG13                   0xd     /* Baud Rate Generator 13 */
#define QE_BRG14                   0xe     /* Baud Rate Generator 14 */
#define QE_BRG15                   0xf     /* Baud Rate Generator 15 */
#define QE_BRG16                   0x10    /* Baud Rate Generator 16 */

/* BRG configuration register */
#define QE_BRGC_DIVISOR_VALUE    3
#define QE_BRGC_RESET_TIMEOUT    1000
#define QE_BRGC_MASK        0x01ffe

/**************************************************************************
 * USB 1.0 Registers (USB1.0)                (0x006C0 - 0x006FF)
 **************************************************************************/
typedef struct {
    volatile uint8_t   usmod;         /* USB mode */
    volatile uint8_t   usadd;         /* USB address */
    volatile uint8_t   uscom;         /* USB command */
    SPC(a, 1)
    volatile uint16_t  usep0;         /* USB endpoint 0 */
    volatile uint16_t  usep1;         /* USB endpoint 1 */
    volatile uint16_t  usep2;         /* USB endpoint 2 */
    volatile uint16_t  usep3;         /* USB endpoint 3 */
    SPC(b, 4)
    volatile uint16_t  usber;         /* USB event */
    SPC(c, 2)
    volatile uint16_t  usbmr;         /* USB mask */
    SPC(d, 1)
    volatile uint8_t   usbs;          /* USB status */
    volatile uint32_t  ussft;         /* USB start of frame timer */
    SPC(e, 0x24)
} _PackedType QEUSBMap_t, qe_usb_t;

/**************************************************************************
 * SI1 Registers (SI1)                        (0x00700 - 0x0077F)
 **************************************************************************/

typedef struct {
    volatile uint16_t  siamr1;        /* SI1 TDMA mode */
    volatile uint16_t  sibmr1;        /* SI1 TDMB mode */
    volatile uint16_t  sicmr1;        /* SI1 TDMC mode */
    volatile uint16_t  sidmr1;        /* SI1 TDMD mode */
    volatile uint8_t   siglmr1_h;     /* SI1 global mode high */
    SPC(a, 1)
    volatile uint8_t   sicmdr1_h;     /* SI1 command hight */
    SPC(b, 1)
    volatile uint8_t   sistr1_h;      /* SI1 status high */
    SPC(c, 1)
    volatile uint16_t  sirsr1_h;      /* SI1 RAM shadow address high */
    volatile uint8_t   sitarc1;       /* SI1 RAM counter Tx TDMA */
    volatile uint8_t   sitbrc1;       /* SI1 RAM counter Tx TDMB */
    volatile uint8_t   sitcrc1;       /* SI1 RAM counter Tx TDMC */
    volatile uint8_t   sitdrc1;       /* SI1 RAM counter Tx TDMD */
    volatile uint8_t   sirarc1;       /* SI1 RAM counter Rx TDMA */
    volatile uint8_t   sirbrc1;       /* SI1 RAM counter Rx TDMB */
    volatile uint8_t   sircrc1;       /* SI1 RAM counter Rx TDMC */
    volatile uint8_t   sirdrc1;       /* SI1 RAM counter Rx TDMD */

#if defined(MPC836x)
    volatile uint8_t   siemr1;        /* SI1 TDME mode */
    volatile uint8_t   sifmr1;        /* SI1 TDMF mode */
    volatile uint8_t   sigmr1;        /* SI1 TDMG mode */
    volatile uint8_t   sihmr1;        /* SI1 TDMH mode */
    SPC(d, 0xC)
#else
    SPC(d, 0x10)
#endif

    volatile uint8_t   siglmr1_l;     /* SI1 global mode low */
    SPC(e, 1)
    volatile uint8_t   sicmdr1_l;     /* SI1 command low */
    SPC(f, 1)
    volatile uint8_t   sistr1_l;      /* SI1 status low */
    SPC(g, 1)
    volatile uint16_t  sirsr1_l;      /* SI1 RAM shadow address low */

#if defined(MPC836x)
    volatile uint8_t   siterc1;       /* SI1 RAM countefr Tx TDME */
    volatile uint8_t   sitfrc1;       /* SI1 RAM countefr Tx TDMF */
    volatile uint8_t   sitgrc1;       /* SI1 RAM countefr Tx TDMG */
    volatile uint8_t   sithrc1;       /* SI1 RAM countefr Tx TDMH */
    volatile uint8_t   sirerc1;       /* SI1 RAM countefr Rx TDME */
    volatile uint8_t   sirfrc1;       /* SI1 RAM countefr Rx TDMF */
    volatile uint8_t   sirgrc1;       /* SI1 RAM countefr Rx TDMG */
    volatile uint8_t   sirhrc1;       /* SI1 RAM countefr Rx TDMH */
    SPC(h, 8)
    volatile uint32_t  siml1;         /* SI1 multiframe limit */
    volatile uint8_t   siedm1;        /* SI1 extended diag mode */
    volatile uint8_t   siens1;        /* SI1 extended diagnostic mode */
    SPC(i, 0x3A)
#else
    SPC(j, 0x14)
    volatile uint8_t   sispd;         /* SI1 speed mode */
    SPC(k, 0x3B)
#endif

} _PackedType QESIMap_t, qe_si_t;

/**************************************************************************
 * SI1 Routing Table (SI1RT)                (0x01000 - 0x017FF)
 **************************************************************************/

typedef struct {
    volatile uint8_t   sitxram[0x400]; /* SI1 Tx routing table */
    volatile uint8_t   sirxram[0x400]; /* SI1 Tx routing table */
} _PackedType QESIRTMap_t, qe_sirt_t;

/**************************************************************************
 * UCC Registers                        UCC1: 0x02000 - 0x021FF
 *                                        UCC2: 0x03000 - 0x031FF
 *                                        UCC3: 0x02200 - 0x023FF
 *                                        UCC4: 0x03200 - 0x033FF
 *                                        UCC5: 0x02400 - 0x025FF
 *                                        UCC6: 0x03400 - 0x035FF
 *                                        UCC7: 0x02600 - 0x027FF
 *                                        UCC8: 0x03600 - 0x037FF
 **************************************************************************/

/**************************************************************************
 * Slow Mode (UART, BISYNC, QMC)        (0xnn000 - 0xnn1FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  gumr_l;        /* UCC general mode (low) */
    volatile uint32_t  gumr_h;        /* UCC general mode (high) */
    volatile uint16_t  upsmr;         /* UCC protocol-specific mode */
    SPC(a, 2)
    volatile uint16_t  utodr;         /* UCC transmit on demand */
    volatile uint16_t  udsr;          /* UCC data synchronization */
    volatile uint16_t  ucce;          /* UCC event */
    SPC(b, 2)
    volatile uint16_t  uccm;          /* UCC mask */
    SPC(c, 1)
    volatile uint8_t   uccs;          /* UCC status */
    SPC(d, 0x24)
    volatile uint16_t   utpt;         /* UCC transmit polling timer */
    SPC(e, 0x52)
    volatile uint8_t   guemr;         /* UCC general extended mode register */
    SPC(f, 0x16F)
} _PackedType QEUCCSlowMap_t, qe_ucc_slow_t;

/*
 * UCC registers defines
 */
#define GUMR_ENT                     0x00000010
#define GUMR_ENR                     0x00000020
#define GUMR_TCRC                    0x000000C0
#define GUMR_TEND                    0x00040000
#define GUMR_TINV                    0x01000000
#define GUMR_RINV                    0x02000000
#define GUMR_TCI                     0x10000000
#define GUMR_DIAG_LPBK               0x40000000
#define GUMR_MODE_ETH                0x0000000C
#define GUMR_URMODE                 0x00000002
#define GUMR_UTMODE                 0x00000001

#define GUMR_L_MODE_QMC    0x02

#define GUMR_MODE_FAST_ATM           0x0000000A
#define GUMR_MODE_FAST_ETH           0x0000000C
#define GUMR_MODE_FAST_HDLC          0x00000000
#define GUMR_MODE_FAST_TRANS         0x30000000
#define GUMR_MODE_FAST_TX_TRANS      0x20000000
#define GUMR_MODE_FAST_RX_TRANS      0x10000000
#define GUMR_MODE_SLOW_QMC           0x30000002
#define GUMR_MODE_SLOW_UART          0x00000004
#define GUMR_MODE_SLOW_HDLC          0x00000006
#define GUMR_MODE_SLOW_BISYNC        0x00000008

#define FAST_MODE 0x3

#define GUEMR_URMODE_FAST       0x02
#define GUEMR_UTMODE_FAST       0x01

#define SLOW_UCC_GUMR_L_DIAG(y)               (((y) & 3) << 6)

/*
 * Slow UCC
 */

#define SLOW_UCC_GUMR_L_TCI                   0x10000000
#define SLOW_UCC_GUMR_L_RINV                  0x02000000
#define SLOW_UCC_GUMR_L_RDCR                  0x0000C000
#define SLOW_UCC_GUMR_L_TDCR                  0x00030000
#define SLOW_UCC_GUMR_L_TINV                  0x01000000
#define SLOW_UCC_GUMR_L_TEND                  0x00040000
#define SLOW_UCC_GUMR_L_ENR                   0x00000020
#define SLOW_UCC_GUMR_L_ENT                   0x00000010
#define SLOW_UCC_GUMR_L_TENC                  0x00000700
#define SLOW_UCC_GUMR_L_RENC                  0x00003800
#define SLOW_UCC_GUMR_L_MODE                  0x0000000F

#define GUMR_L_TDCR(y)               (((y) & 3) << 16)
#define GUMR_L_RDCR(y)               (((y) & 3) << 14)
#define GUMR_L_RENC(y)               (((y) & 3) << 11)
#define GUMR_L_TENC(y)               (((y) & 3) << 8)
#define GUMR_L_DIAG(y)               (((y) & 3) << 6)
#define GUMR_L_MODE(y)               ((y) & 0xf)

#define SLOW_UCC_GUMR_H_SAM                   0x00008000
#define SLOW_UCC_GUMR_H_REVD                  0x00002000
#define SLOW_UCC_GUMR_H_TRX                   0x00001000
#define SLOW_UCC_GUMR_H_TTX                   0x00000800
#define SLOW_UCC_GUMR_H_CDP                   0x00000400
#define SLOW_UCC_GUMR_H_CTSP                  0x00000200
#define SLOW_UCC_GUMR_H_CDS                   0x00000100
#define SLOW_UCC_GUMR_H_CTSS                  0x00000080
#define SLOW_UCC_GUMR_H_TFL                   0x00000040
#define SLOW_UCC_GUMR_H_RFW                   0x00000020
#define SLOW_UCC_GUMR_H_TXSY                  0x00000010
#define SLOW_UCC_GUMR_H_RTSM                  0x00000002
#define SLOW_UCC_GUMR_H_RSYN                  0x00000001

#define GUMR_H_TRX(y)		      (((y) & 0x1) << 12)
#define GUMR_H_TTX(y)		      (((y) & 0x1) << 11)

/*
 * QE_UCC_GUMRH_RTSM Parameters.
 */
#define QE_HDLC_IDLE_ONES        0x0000    /* Send ones to idle between
                         *   frames */
#define QE_HDLC_IDLE_FLAGS       0x0002    /* Send flags to idle between
                         *   frames */

/*
 * QE_UCC_GUMRL_TPL Parameters
 */
#define QE_TX_PREAM_0BITS        0x0
#define QE_TX_PREAM_48BITS       0x4       /* for ethernet operation */

/*
 * QE_UCC_GUMRL_TPP Parameters
 */
#define QE_TX_PREAM_PAT_00S      0x0       /* all zero */
#define QE_TX_PREAM_PAT_10S      0x1       /* repeating 10's (ethernet)*/
#define QE_TX_PREAM_PAT_01S      0x2       /* repeating 01's */
#define QE_TX_PREAM_PAT_11S      0x3       /* all ones (LocalTalk) */

/*
 * QE_UCC_GUMRL_TDCR Parameters
 */
#define QE_TX_DIV_CLK_1X         0x0
#define QE_TX_DIV_CLK_8X         0x1
#define QE_TX_DIV_CLK_16X        0x2       /* (UART and AppleTalk) */
#define QE_TX_DIV_CLK_32X        0x3

/*
 * QE_UCC_GUMRL_RDCR Parameters
 */
#define QE_RX_DIV_CLK_1X         0x0
#define QE_RX_DIV_CLK_8X         0x1
#define QE_RX_DIV_CLK_16X        0x2       /* (UART and AppleTalk) */
#define QE_RX_DIV_CLK_32X        0x3

/*
 * QE_UCC_GUMRL_RENC Parameters
 */
#define QE_ENC_NRZ               0x0
#define QE_ENC_NRZI              0x1
#define QE_ENC_FM0               0x2
#define QE_ENC_MANCHESTER        0x4
#define QE_ENC_DIFF_MANCHESTER   0x6


/*
 * QE_UCC_GUMRL_DIAG Parameters
 */
#define QE_NORMAL_OP             0x0
#define QE_LOCAL_LOOPBACK        0x1
#define QE_AUTO_ECHO             0x2
#define QE_LOOPBACK_N_ECHO       0x3
#define QE_SCC_GSMRL_DIAG_MASK   0x000000c0

#define UCCE_RXF                     0x000000FF

#define QE_UCC_FAST_MODE                0x3
#define QE_UCC_SLOW_MODE                0x0


/*
 * Buffer Descriptor Flags - The various bits of the 'status' field of a buffer
 *   descriptor are described here.
 *
 *   All bits which do not have a specific name associated with them are
 *   reserved, and should be set to 0.
 */
#define QE_UART_RX_BDSTAT_E           0x8000
#define QE_UART_RX_BDSTAT_W           0x2000
#define QE_UART_RX_BDSTAT_I           0x1000
#define QE_UART_RX_BDSTAT_CC          0x0800
#define QE_UART_RX_BDSTAT_A           0x0400
#define QE_UART_RX_BDSTAT_CM          0x0200
#define QE_UART_RX_BDSTAT_ID          0x0100
#define QE_UART_RX_BDSTAT_AM          0x0080
#define QE_UART_RX_BDSTAT_BR          0x0020
#define QE_UART_RX_BDSTAT_FR          0x0010
#define QE_UART_RX_BDSTAT_PR          0x0008
#define QE_UART_RX_BDSTAT_OV          0x0002
#define QE_UART_RX_BDSTAT_CD          0x0001

#define QE_UART_TX_BDSTAT_R           0x8000
#define QE_UART_TX_BDSTAT_W           0x2000
#define QE_UART_TX_BDSTAT_I           0x1000
#define QE_UART_TX_BDSTAT_CR          0x0800
#define QE_UART_TX_BDSTAT_A           0x0400
#define QE_UART_TX_BDSTAT_CM          0x0200
#define QE_UART_TX_BDSTAT_P           0x0100
#define QE_UART_TX_BDSTAT_NS          0x0080
#define QE_UART_TX_BDSTAT_CT          0x0001

#define QE_AHDLC_RX_BDSTAT_E          0x8000
#define QE_AHDLC_RX_BDSTAT_W          0x2000
#define QE_AHDLC_RX_BDSTAT_I          0x1000
#define QE_AHDLC_RX_BDSTAT_L          0x0800
#define QE_AHDLC_RX_BDSTAT_F          0x0400
#define QE_AHDLC_RX_BDSTAT_CM         0x0200
#define QE_AHDLC_RX_BDSTAT_BRK        0x0080
#define QE_AHDLC_RX_BDSTAT_BOF        0x0040
#define QE_AHDLC_RX_BDSTAT_AB         0x0008
#define QE_AHDLC_RX_BDSTAT_CR         0x0004
#define QE_AHDLC_RX_BDSTAT_OV         0x0002
#define QE_AHDLC_RX_BDSTAT_CD         0x0001

#define QE_AHDLC_TX_BDSTAT_R          0x8000
#define QE_AHDLC_TX_BDSTAT_W          0x2000
#define QE_AHDLC_TX_BDSTAT_I          0x1000
#define QE_AHDLC_TX_BDSTAT_L          0x0800
#define QE_AHDLC_TX_BDSTAT_CM         0x0200
#define QE_AHDLC_TX_BDSTAT_CT         0x0001

#define QE_QMC_RX_BDSTAT_E            0x8000
#define QE_QMC_RX_BDSTAT_W            0x2000
#define QE_QMC_RX_BDSTAT_I            0x1000
#define QE_QMC_RX_BDSTAT_L            0x0800
#define QE_QMC_RX_BDSTAT_F            0x0400
#define QE_QMC_RX_BDSTAT_CM           0x0200
#define QE_QMC_RX_BDSTAT_UB           0x0080
#define QE_QMC_RX_BDSTAT_LG           0x0020
#define QE_QMC_RX_BDSTAT_NO           0x0010
#define QE_QMC_RX_BDSTAT_AB           0x0008
#define QE_QMC_RX_BDSTAT_CR           0x0004

#define QE_QMC_TX_BDSTAT_R            0x8000
#define QE_QMC_TX_BDSTAT_W            0x2000
#define QE_QMC_TX_BDSTAT_I            0x1000
#define QE_QMC_TX_BDSTAT_L            0x0800
#define QE_QMC_TX_BDSTAT_TC           0x0400
#define QE_QMC_TX_BDSTAT_CM           0x0200
#define QE_QMC_TX_BDSTAT_UB           0x0080

#define QE_BISYNC_RX_BDSTAT_E         0x8000
#define QE_BISYNC_RX_BDSTAT_W         0x2000
#define QE_BISYNC_RX_BDSTAT_I         0x1000
#define QE_BISYNC_RX_BDSTAT_C         0x0800
#define QE_BISYNC_RX_BDSTAT_B         0x0400
#define QE_BISYNC_RX_BDSTAT_CM        0x0200
#define QE_BISYNC_RX_BDSTAT_DL        0x0010
#define QE_BISYNC_RX_BDSTAT_PR        0x0008
#define QE_BISYNC_RX_BDSTAT_CR        0x0004
#define QE_BISYNC_RX_BDSTAT_OV        0x0002
#define QE_BISYNC_RX_BDSTAT_CD        0x0001

#define QE_BISYNC_TX_BDSTAT_R         0x8000
#define QE_BISYNC_TX_BDSTAT_W         0x2000
#define QE_BISYNC_TX_BDSTAT_I         0x1000
#define QE_BISYNC_TX_BDSTAT_L         0x0800
#define QE_BISYNC_TX_BDSTAT_TB        0x0400
#define QE_BISYNC_TX_BDSTAT_CM        0x0200
#define QE_BISYNC_TX_BDSTAT_BR        0x0100
#define QE_BISYNC_TX_BDSTAT_TD        0x0080
#define QE_BISYNC_TX_BDSTAT_TR        0x0040
#define QE_BISYNC_TX_BDSTAT_B         0x0020
#define QE_BISYNC_TX_BDSTAT_UN        0x0002
#define QE_BISYNC_TX_BDSTAT_CT        0x0001

#define QE_UPSMR_PRO                    0x00400000
#define QE_SLOW_UPSMR_FLC               0x8000

#define QE_SLOW_UPSMR_SL                0x4000
#define QE_SLOW_UPSMR_CL                0x3000
#define QE_SLOW_UPSMR_UM                0x0C00
#define QE_SLOW_UPSMR_FRZ               0x0200
#define QE_SLOW_UPSMR_RZS               0x0100
#define QE_SLOW_UPSMR_SYN               0x0080
#define QE_SLOW_UPSMR_DRT               0x0040
#define QE_SLOW_UPSMR_PEN               0x0010
#define QE_SLOW_UPSMR_RPM               0x000C
#define QE_SLOW_UPSMR_TPM               0x0003


/* UART protocol specific RAM */
/* RCCM (control character mask) */
#define QE_UART_NO_CTRL_CHAR_MASK       0xc0ff		/* No mask */
#define QE_UART_RXBUF                   0x0001

#define QE_UCC_UART_UCCM_AB                      0x0200
#define QE_UCC_UART_UCCM_IDL                     0x0100
#define QE_UCC_UART_UCCM_GRA                     0x0080
#define QE_UCC_UART_UCCM_BRKE                    0x0040
#define QE_UCC_UART_UCCM_BRKS                    0x0020
#define QE_UCC_UART_UCCM_CCR                     0x0008
#define QE_UCC_UART_UCCM_BSY                     0x0004
#define QE_UCC_UART_UCCM_TX                      0x0002
#define QE_UCC_UART_UCCM_RX                      0x0001
#define QE_UCC_UART_UCCM_MASK_ALL                0x0000
#define QE_UCC_UART_UCCM_ENABLE_ALL              0x03EF

#define QE_UCC_UART_UCCE_AB                      0x0200
#define QE_UCC_UART_UCCE_IDL                     0x0100
#define QE_UCC_UART_UCCE_GRA                     0x0080
#define QE_UCC_UART_UCCE_BRKE                    0x0040
#define QE_UCC_UART_UCCE_BRKS                    0x0020
#define QE_UCC_UART_UCCE_CCR                     0x0008
#define QE_UCC_UART_UCCE_BSY                     0x0004
#define QE_UCC_UART_UCCE_TX                      0x0002
#define QE_UCC_UART_UCCE_RX                      0x0001
#define QE_UCC_UART_UCCE_CLEAR_ALL               0x03EF


/**************************************************************************
 * Fast Mode (Ethernet, ATM, HDLC, Transparent)        (0xnnn00-0xnnnFF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  gumr;          /* UCC general mode */
    volatile uint32_t  upsmr;         /* UCC protocol-specific mode */
    volatile uint16_t  utodr;         /* UCC transmit on demand */
    SPC(a, 2)
    volatile uint16_t  udsr;          /* UCC data synchronization */
    SPC(b, 2)
    volatile uint32_t  ucce;          /* UCC event */
    volatile uint32_t  uccm;          /* UCC mask */
    volatile uint8_t   uccs;          /* UCC status */
    SPC(c, 7)
    volatile uint32_t  urfb;          /* UCC receive FIFO base */
    volatile uint16_t  urfs;          /* UCC receive FIFO size */
    SPC(d, 2)
    volatile uint16_t  urfet;         /* UCC Rx FIFO emergency threshold */
    volatile uint16_t  urfset;        /* UCC Rx FIFO special emergency thrsh*/
    volatile uint32_t  utfb;          /* UCC transmit FIFO base */
    volatile uint16_t  utfs;          /* UCC transmit FIFO size */
    SPC(e, 2)
    volatile uint16_t  utfet;         /* UCC Tx FIFO emergency threshold */
    SPC(f, 2)
    volatile uint16_t  utftt;         /* UCC transmit FIFO tx threshold */
    SPC(g, 2)
    volatile uint16_t  utpt;          /* UCC transmit polling timer */
    SPC(h, 2)
    volatile uint32_t  urtry;         /* UCC retry counter */
    SPC(i, 0x4C)
    volatile uint8_t   guemr;         /* UCC general extended mode */
    SPC(j, 0x6F)
} _PackedType QEUCCFastRegMap_t, qe_ucc_gen_t;

#define MASK_ALL_QE_INTERRUPTS        0xffffffff
#define ALL_QE_INTERRUPTS             0xffffffff
#define QE_INTERRUPT_MASK             0xf8f0e07e

/* Values for 10/100 */
#define QE_MIIRX_VIR_FIFO_SIZE             512
#define QE_MIIRX_VIR_FIFO_EMER_THHLD       QE_MIIRX_VIR_FIFO_SIZE / 2
#define QE_MIIRX_VIR_FIFO_SPEC_EMER_THHLD  (QE_MIIRX_VIR_FIFO_SIZE * 3) / 4
#define QE_MIITX_VIR_FIFO_SIZE             QE_MIIRX_VIR_FIFO_SIZE
#define QE_MIITX_VIR_FIFO_EMER_THHLD       QE_MIITX_VIR_FIFO_SIZE / 2
#define QE_MIITX_VIR_FIFO_TX_THHLD         QE_MIITX_VIR_FIFO_SIZE / 4

/* Values for 1000 */
#define QE_GMIIRX_VIR_FIFO_SIZE            8192
#define QE_GMIIRX_VIR_FIFO_EMER_THHLD      QE_GMIIRX_VIR_FIFO_SIZE / 2
#define QE_GMIIRX_VIR_FIFO_SPEC_EMER_THHLD 6144
#define QE_GMIITX_VIR_FIFO_SIZE            QE_GMIIRX_VIR_FIFO_SIZE
#define QE_GMIITX_VIR_FIFO_EMER_THHLD      QE_GMIITX_VIR_FIFO_SIZE / 2
#define QE_GMIITX_VIR_FIFO_TX_THHLD        QE_GMIITX_VIR_FIFO_SIZE / 4
#define QE_CLEAR_URTRY                     0x00000001

#define UEC_UCCM_MPD                  0x80000000
#define UEC_UCCM_SCAR                 0x40000000
#define UEC_UCCM_GRA                  0x20000000
#define UEC_UCCM_CBPR                 0x10000000
#define UEC_UCCM_BSY                  0x08000000
#define UEC_UCCM_RXC                  0x04000000
#define UEC_UCCM_TXC                  0x02000000
#define UEC_UCCM_TXE                  0x01000000
#define UEC_UCCM_TXB0                 0x00800000
#define UEC_UCCM_TXB1                 0x00400000
#define UEC_UCCM_TXB2                 0x00200000
#define UEC_UCCM_TXB3                 0x00100000
#define UEC_UCCM_TXB4                 0x00080000
#define UEC_UCCM_TXB5                 0x00040000
#define UEC_UCCM_TXB6                 0x00020000
#define UEC_UCCM_TXB7                 0x00010000
#define UEC_UCCM_RXB0                 0x00008000
#define UEC_UCCM_RXB1                 0x00004000
#define UEC_UCCM_RXB2                 0x00002000
#define UEC_UCCM_RXB3                 0x00001000
#define UEC_UCCM_RXB4                 0x00000800
#define UEC_UCCM_RXB5                 0x00000400
#define UEC_UCCM_RXB6                 0x00000200
#define UEC_UCCM_RXB7                 0x00000100
#define UEC_UCCM_RXF0                 0x00000080
#define UEC_UCCM_RXF1                 0x00000040
#define UEC_UCCM_RXF2                 0x00000020
#define UEC_UCCM_RXF3                 0x00000010
#define UEC_UCCM_RXF4                 0x00000008
#define UEC_UCCM_RXF5                 0x00000004
#define UEC_UCCM_RXF6                 0x00000002
#define UEC_UCCM_RXF7                 0x00000001

#define UEC_UCCE_MPD                  0x80000000
#define UEC_UCCE_SCAR                 0x40000000
#define UEC_UCCE_GRA                  0x20000000
#define UEC_UCCE_CBPR                 0x10000000
#define UEC_UCCE_BSY                  0x08000000
#define UEC_UCCE_RXC                  0x04000000
#define UEC_UCCE_TXC                  0x02000000
#define UEC_UCCE_TXE                  0x01000000
#define UEC_UCCE_TXB0                 0x00800000
#define UEC_UCCE_TXB1                 0x00400000
#define UEC_UCCE_TXB2                 0x00200000
#define UEC_UCCE_TXB3                 0x00100000
#define UEC_UCCE_TXB4                 0x00080000
#define UEC_UCCE_TXB5                 0x00040000
#define UEC_UCCE_TXB6                 0x00020000
#define UEC_UCCE_TXB7                 0x00010000
#define UEC_UCCE_RXB0                 0x00008000
#define UEC_UCCE_RXB1                 0x00004000
#define UEC_UCCE_RXB2                 0x00002000
#define UEC_UCCE_RXB3                 0x00001000
#define UEC_UCCE_RXB4                 0x00000800
#define UEC_UCCE_RXB5                 0x00000400
#define UEC_UCCE_RXB6                 0x00000200
#define UEC_UCCE_RXB7                 0x00000100
#define UEC_UCCE_RXF0                 0x00000080
#define UEC_UCCE_RXF1                 0x00000040
#define UEC_UCCE_RXF2                 0x00000020
#define UEC_UCCE_RXF3                 0x00000010
#define UEC_UCCE_RXF4                 0x00000008
#define UEC_UCCE_RXF5                 0x00000004
#define UEC_UCCE_RXF6                 0x00000002
#define UEC_UCCE_RXF7                 0x00000001


/**************************************************************************
 * Ethernet General Configuration        (0xnn100 - 0xnn17F)
 **************************************************************************/

typedef struct {
    volatile uint32_t  maccfg1;       /* MAC configuration 1 */
    volatile uint32_t  maccfg2;       /* MAC configuration 2 */
    volatile uint16_t  ipgifg;        /* Interframe gap */
    SPC(a, 2)
    volatile uint32_t  hafdup;        /* Half-duplex */
    SPC(b, 12)
    volatile uint32_t  emtr;          /* Ethernet MAC test */
    volatile uint32_t  miimcfg;       /* MII mgmt configuration */
    volatile uint32_t  miimcom;       /* MII mgmt command */
    volatile uint32_t  miimadd;       /* MII mgmt address */
    volatile uint32_t  miimcon;       /* MII mgmt control */
    volatile uint32_t  miimstat;      /* MII mgmt status */
    volatile uint32_t  miimind;       /* MII mgmt indication */
    volatile uint32_t  ifctl;         /* Interface control */
    volatile uint32_t  ifstat;        /* Interface status */
    volatile uint32_t  macstnaddr1;   /* Station address part 1 */
    volatile uint32_t  macstnaddr2;   /* Station address part 2 */
    SPC(c, 8)
    volatile uint32_t  uempr;         /* UCC Ethernet MAC parameter */

#if defined(MPC836x)
    volatile uint32_t  ytbipa;        /* UCC TBI address */
#elif defined(MPC832x)
    SPC(d, 4)
#endif

    volatile uint16_t  uescr;         /* UCC Ethernet statistics control */
    SPC(e, 0x26)
} _PackedType QEUCCEthernetMap_t, qe_ucc_eth_t;

#define MACCFG1_RX_EN       0x00000004
#define MACCFG1_TX_EN       0x00000001

#define MACCFG2_IFM_MASK    0x00000300
#define MACCFG2_IFM_NIBBLE  0x00000100
#define MACCFG2_IFM_BYTE    0x00000200


/**************************************************************************
 * Ethernet Statistics Counters                (0xnn180 - 0xnn1FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  tx64;          /* Tx/Rx 64 byte frame counter */
    volatile uint32_t  tx127;         /* Tx/Rx 65-127 byte frame counter */
    volatile uint32_t  tx255;         /* Tx/Rx 128-255 byte frame counter */
    volatile uint32_t  rx64;          /* Rx/Rx 64 byte frame counter */
    volatile uint32_t  rx127;         /* Rx/Rx 65-127 byte frame counter */
    volatile uint32_t  rx255;         /* Rx/Rx 128-255 byte frame counter */
    volatile uint32_t  txok;          /* Transmit good bytes counter */
    volatile uint16_t  txcf;          /* Transmit control frame counter */
    SPC(a, 2)
    volatile uint32_t  tmca;          /* Tx multicast control frame counter */
    volatile uint32_t  tbca;          /* Transmit broadcast packet counter */
    volatile uint32_t  rxfok;         /* Receive frame OK counter */
    volatile uint32_t  rbyt;          /* Receive good and bad bytes counter */
    volatile uint32_t  rxbok;         /* Receive bytes OK counter */
    volatile uint32_t  rmca;          /* Receive multicast packet counter */
    volatile uint32_t  rbca;          /* Receive broadcast packet counter */
    volatile uint32_t  scar;          /* Statistics carry */
    volatile uint32_t  scam;          /* Statistics carry mask */
    SPC(b, 0x3C)
} _PackedType QEUCCMIBMap_t, qe_ucc_mib_t;

/**************************************************************************
 * UCC Fast Mode
 **************************************************************************/

typedef struct {
    qe_ucc_gen_t  reg;            /* Fast mode generic */
    qe_ucc_eth_t  eth;            /* Ethernet general configuration */
    qe_ucc_mib_t  mib;            /* Ethernet statistics counters */
} _PackedType QEUCCFastMap_t, qe_ucc_fast_t;

/**************************************************************************
 * UCC (Slow or Fast mode)
 **************************************************************************/

typedef struct {
    union {
        qe_ucc_slow_t  slow;       /* Slow mode */
        qe_ucc_fast_t  fast;       /* Fast mode */
    } mode;
} _PackedType t_QEUCCMap, qe_ucc_t;

/* Short Name */
#define ucc_slow        mode.slow
#define ucc_gen         mode.fast.reg
#define ucc_eth         mode.fast.eth
#define ucc_mib         mode.fast.mib

/*
 * UCC defines
 */

/* Peripheral Defines */

#define QE_UCC1                         0x1
#define QE_UCC2                         0x2
#define QE_UCC3                         0x3
#define QE_UCC4                         0x4
#define QE_UCC5                         0x5
#define QE_UCC6                         0x6
#define QE_UCC7                         0x7
#define QE_UCC8                         0x8

/* UCC sub-defines */

#define QE_UCC_ATM                      0xA
#define QE_UCC_ETHERNET                 0xC
#define QE_UCC_HDLC                     0x0
#define QE_UCC_TRANSPARENT              0x0
#define QE_UCC_TRANSPARENT_RX_HDLC      0x0
#define QE_UCC_TRANSPARENT_TX_HDLC      0x0
#define QE_UCC_QMC                      0x2
#define QE_UCC_SERIAL_ATM               0x2
#define QE_UCC_UART                     0x4
#define QE_UCC_ASYNC_HDLC               0x6
#define QE_UCC_BISYNC                   0x8
#define QE_UCC_TDM                      0x0


/**************************************************************************
 * Multi-PHY controller (MPHY)                (0x02E00 - 0x02FFF)
 **************************************************************************/

typedef struct {
    volatile uint16_t  upgcr;         /* UTOPIA general configuration */
    SPC(a, 2)
    volatile uint16_t  uplpa;         /* UTOPIA last PHY address */
    SPC(b, 2)
    volatile uint16_t  uphec;         /* ATM HEC */
    SPC(c, 2)
    volatile uint32_t  upuc;          /* UTOPIA UCC configuraiton */
    volatile uint32_t  updc1;         /* UTOPIA device 1 configuration */
    volatile uint32_t  updc2;         /* UTOPIA device 2 configuration */
    volatile uint32_t  updc3;         /* UTOPIA device 3 configuration */
    volatile uint32_t  updc4;         /* UTOPIA device 4 configuration */
    volatile uint8_t   upstpa;        /* UTOPIA STPA threshold */
    SPC(d, 15)
    volatile uint32_t  updrs1_h;      /* UTOPIA device 1 rate select high */
    volatile uint32_t  updrs1_l;      /* UTOPIA device 1 rate select low */
    volatile uint32_t  updrs2_h;      /* UTOPIA device 2 rate select high */
    volatile uint32_t  updrs2_l;      /* UTOPIA device 2 rate select low */
    volatile uint32_t  updrs3_h;      /* UTOPIA device 3 rate select high */
    volatile uint32_t  updrs3_l;      /* UTOPIA device 3 rate select low */
    volatile uint32_t  updrs4_h;      /* UTOPIA device 4 rate select high */
    volatile uint32_t  updrs4_l;      /* UTOPIA device 4 rate select low */
    volatile uint32_t  updrp1;        /* UTOPIA device 1 Rx priority low */
    volatile uint32_t  updrp2;        /* UTOPIA device 2 Rx priority low */
    volatile uint32_t  updrp3;        /* UTOPIA device 3 Rx priority low */
    volatile uint32_t  updrp4;        /* UTOPIA device 4 Rx priority low */
    volatile uint32_t  upde1;         /* UTOPIA device 1 event */
    volatile uint32_t  upde2;         /* UTOPIA device 2 event */
    volatile uint32_t  upde3;         /* UTOPIA device 3 event */
    volatile uint32_t  upde4;         /* UTOPIA device 4 event */
    volatile uint16_t  uprp1;         /* UTOPIA dev 1 internal rate config */
    volatile uint16_t  uprp2;         /* UTOPIA dev 2 internal rate config */
    volatile uint16_t  uprp3;         /* UTOPIA dev 3 internal rate config */
    volatile uint16_t  uprp4;         /* UTOPIA dev 4 internal rate config */
    SPC(e, 8)
    volatile uint16_t  uptirr1_0;     /* Device 1 transmit internal rate 0 */
    volatile uint16_t  uptirr1_1;     /* Device 1 transmit internal rate 1 */
    volatile uint16_t  uptirr1_2;     /* Device 1 transmit internal rate 2 */
    volatile uint16_t  uptirr1_3;     /* Device 1 transmit internal rate 3 */
    volatile uint16_t  uptirr2_0;     /* Device 2 transmit internal rate 0 */
    volatile uint16_t  uptirr2_1;     /* Device 2 transmit internal rate 1 */
    volatile uint16_t  uptirr2_2;     /* Device 2 transmit internal rate 2 */
    volatile uint16_t  uptirr2_3;     /* Device 2 transmit internal rate 3 */
    volatile uint16_t  uptirr3_0;     /* Device 3 transmit internal rate 0 */
    volatile uint16_t  uptirr3_1;     /* Device 3 transmit internal rate 1 */
    volatile uint16_t  uptirr3_2;     /* Device 3 transmit internal rate 2 */
    volatile uint16_t  uptirr3_3;     /* Device 3 transmit internal rate 3 */
    volatile uint16_t  uptirr4_0;     /* Device 4 transmit internal rate 0 */
    volatile uint16_t  uptirr4_1;     /* Device 4 transmit internal rate 1 */
    volatile uint16_t  uptirr4_2;     /* Device 4 transmit internal rate 2 */
    volatile uint16_t  uptirr4_3;     /* Device 4 transmit internal rate 3 */
    volatile uint32_t  uper1;         /* Device 1 port enable */
    volatile uint32_t  uper2;         /* Device 2 port enable */
    volatile uint32_t  uper3;         /* Device 3 port enable */
    volatile uint32_t  uper4;         /* Device 4 port enable */
    SPC(f, 0x150)
} _PackedType t_QEUTOPIAMap, qe_utopi_t;

#define QE_UTOPIA_UPGCR_TMS 0x4000
#define QE_UTOPIA_UPGCR_RMS 0x2000
#define QE_UTOPIA_UPGCR_DIAG    0x0100

/**************************************************************************
 * Serial DMA (SDMA)                        (0x04000 - 0x0407F)
 **************************************************************************/

typedef struct {
    volatile uint32_t  sdsr;          /* Serial DMA status */
    volatile uint32_t  sdmr;          /* Serial DMA mode */
    volatile uint32_t  sdtr1;         /* SDMA system bus threshold */
    volatile uint32_t  sdtr2;         /* SDMA secondary bus threshold */
    volatile uint32_t  sdhy1;         /* SDMA system bus hysteresis */
    volatile uint32_t  sdhy2;         /* SDMA secondary bus hysteresis */
    volatile uint32_t  sdta1;         /* SDMA sytem bus address */
    volatile uint32_t  sdta2;         /* SDMA secondary bus address */
    volatile uint32_t  sdtm1;         /* SDMA system bus MSNUM */
    volatile uint32_t  sdtm2;         /* SDMA secondary bus MSNUM */
    SPC(a, 0x10)
    volatile uint32_t  sdaqr;         /* SDMA address bus quality */
    volatile uint32_t  sdaqmr;        /* SDMA address bus quality mask */
    SPC(b, 4)
    volatile uint32_t  sdebcr;        /* SDMA CAM entries base */
    SPC(c, 0x38)
} _PackedType t_QESDMAMap, qe_sdma_t;

/* BRG configuration register */
#define QE_BRGC_ENABLE                0x00010000
#define QE_BRGC_DIVISOR_SHIFT         1
#define QE_BRGC_DIVISOR_MAX           0xFFF
#define QE_BRGC_EXTC_SHIFT            14

/* CMXUPCR Field Descriptions */
#define RECEIVE_CLOCK_SOURCE_SHIFT        12
#define TRANSMIT_CLOCK_SOURCE_SHIFT       8
#define INTERNAL_RATE_CLOCK_SOURCE_SHIFT  0
#define CMXUPCR_UPC1_SHIFT                16
#define CMXUPCR_UPC2_SHIFT                0

/* CETSCR register */
#define QE_CETSCR_RTE1                0x04000000
#define QE_CETSCR_RTE2                0x00000400
#define QE_CETSCR_CETPS1_SHIFT        16

/* QE Timers registers */
#define QE_GTCFR1_PCAS                0x80
#define QE_GTCFR1_STP2                0x20
#define QE_GTCFR1_RST2                0x10
#define QE_GTCFR1_GM2                 0x08
#define QE_GTCFR1_GM1                 0x04
#define QE_GTCFR1_STP1                0x02
#define QE_GTCFR1_RST1                0x01

/* SDMA registers */
#define QE_SDSR_BER1                  0x02000000
#define QE_SDSR_BER2                  0x01000000
#define QE_SDMR_GLB_1_MSK             0x80000000
#define QE_SDMR_ADR_SEL               0x20000000
#define QE_SDMR_BER1_MSK              0x02000000
#define QE_SDMR_BER2_MSK              0x01000000
#define QE_SDMR_EB1_MSK               0x00800000
#define QE_SDMR_ER1_MSK               0x00080000
#define QE_SDMR_ER2_MSK               0x00040000
#define QE_SDMR_CEN_MASK              0x0000E000
#define QE_SDMR_SBER_1                0x00000200
#define QE_SDMR_SBER_2                0x00000200
#define QE_SDMR_EB1_PR_MASK           0x000000C0
#define QE_SDMR_ER1_PR                0x00000008

#define QE_SDMR_CEN_SHIFT             13
#define QE_SDMR_EB1_PR_SHIFT          6

#define QE_SDTM_MSNUM_SHIFT           24

#define QE_SDEBCR_BA_MASK             0x01FFFFFF

/**************************************************************************
 * Debug Space                                (0x04080 - 0x07FFF)
 **************************************************************************/

/**************************************************************************
 * Debug Breakpoint Registers                 (0x04080 - 0x040FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  bpdcr;         /* Breakpoint debug command */
    volatile uint32_t  bpdsr;         /* Breakpoint debug status */
    volatile uint32_t  bpdmr;         /* Breakpoint debug mask */
    volatile uint32_t  bprmrr0;       /* Breakpoint request mode risc 0 */
    volatile uint32_t  bprmrr1;       /* Breakpoint request mode risc 1 */
    SPC(a, 8)
    volatile uint32_t  bprmtr0;       /* Breakpoint request mode trb 0 */
    volatile uint32_t  bprmtr1;       /* Breakpoint request mode trb 1 */
    SPC(b, 8)
    volatile uint32_t  bprmir;        /* Breakpoint req mode immediate */
    volatile uint32_t  bprmsr;        /* Breakpoint req mode serial */
    volatile uint32_t  bpemr;         /* Breakpoint exit mode */
    SPC(c, 0x48)
} _PackedType t_QEDebugBrkptMap, qe_debug_t;

/**************************************************************************
 * RISC Special Registers (trap and breakpoint) (RISC1 SPCL) (0x04100 - 0x041FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  tibcr[16];     /* Trap/Breakpoint control */
    SPC(a, 0x40)
    volatile uint32_t  ibcr0;         /* Instruction brkpt ctrl 0 */
    volatile uint32_t  ibs0;          /* Instruction brkpt Snum 0 */
    volatile uint32_t  ibcnr0;        /* Instruction brkpt counter 0 */
    SPC(b, 4)
    volatile uint32_t  ibcr1;         /* Instruction brkpt ctrl 1 */
    volatile uint32_t  ibs1;          /* Instruction brkpt snum 1 */
    volatile uint32_t  ibcnr1;        /* Instruction brkpt counter 1 */
    volatile uint32_t  npcr;          /* Next program counter */
    volatile uint32_t  dbcr;          /* Data breakpoint - control */
    volatile uint32_t  dbar;          /* Data breakpoint - address */
    volatile uint32_t  dbamr;         /* Data breakpoint - addr mask */
    volatile uint32_t  dbsr;          /* Data breakpoint - Snum */
    volatile uint32_t  dbcnr;         /* Data breakpoint - counter */
    SPC(c, 12)
    volatile uint32_t  dbdr_h;        /* Data breakpoint - data high */
    volatile uint32_t  dbdr_l;        /* Data breakpoint - data low */
    volatile uint32_t  dbdmr_h;       /* Data brkpt - data mask high */
    volatile uint32_t  dbdmr_l;       /* Data brkpt - data mask low */
    volatile uint32_t  bsr;           /* Breakpoint status */
    volatile uint32_t  bor;           /* Breakpoint opcode */
    SPC(d, 0x18)
    volatile uint32_t  eccr;          /* Exception control config */
    volatile uint32_t  eicr;          /* External interrupt control */
    SPC(e, 8)
} _PackedType t_QERISCSPCLMap, qe_risc_t;

/**************************************************************************
 * Test Registers (Test)                (0x04500 - 0x045FF)
 **************************************************************************/

typedef struct {
    volatile uint32_t  reg[64];       /* Test registers */
} _PackedType t_QETestMap, qe_test_t;

/**************************************************************************
 * RISC Trace Buffer Registers (RISC TRBR)        (0x04600 - 0x0467F)
 **************************************************************************/

typedef struct {
    volatile uint32_t  buf[32];       /* RISC trace buffer */
} _PackedType t_QERISCTRBRMap, qe_trace_t;

/**************************************************************************
 * RAM Space                                (0x08000 - 0x3FFFF)
 **************************************************************************/

typedef struct {
    volatile uint8_t ucc1[2048];	/* UCC1  PARAM	(0x10000 - 0x107FF) */
    volatile uint8_t ucc2[2048];	/* UCC2  PARAM  (0x10800 - 0x10FFF) */
    volatile uint8_t ucc3[2048];	/* UCC3  PARAM  (0x11000 - 0x117FF) */
    volatile uint8_t ucc4[2048];	/* UCC4  PARAM	(0x11800 - 0x11FFF) */
    volatile uint8_t ucc5[2048];	/* UCC5  PARAM	(0x12000 - 0x127FF) */
    volatile uint8_t spi1[1024];	/* SPI1  PARAM	(0x12800 - 0x12BFF) */
    volatile uint8_t spi2[1024];	/* SPI2  PARAM	(0x12C00 - 0x12FFF) */
    volatile uint8_t timer[512];	/* Timer PARAM	(0x13000 - 0x131FF) */
    volatile uint8_t usb[2048];		/* USB   PARAM  (0x13200 - 0x139FF) */
    volatile uint8_t fifo[1536];	/* FIFO  PARAM	(0x13A00 - 0x13FFF) */
    volatile uint8_t ucc6[2048];    /* UCC6  PARAM  (0x14000 - 0x147FF) */
    volatile uint8_t ucc7[2048];    /* UCC7  PARAM  (0x14800 - 0x14FFF) */
    volatile uint8_t ucc8[2048];    /* UCC8  PARAM  (0x15000 - 0x157FF) */
} _PackedType t_QEMultiRAMMap, qe_muram_template_t;

/* QE Default Parameter RAM Base Address from Multi-user RAM */
#define QE_PRAM_UCC_SIZE         0x0800
#define QE_PRAM_BASE             ((uchar *)&REGB->qe.muram)
#define QE_PRAM_BASE_UCC1        (QE_PRAM_BASE)
#define QE_PRAM_BASE_UCC2        (QE_PRAM_BASE + 0x0800)
#define QE_PRAM_BASE_UCC3        (QE_PRAM_BASE + 0x1000)
#define QE_PRAM_BASE_UCC4        (QE_PRAM_BASE + 0x1800)
#define QE_PRAM_BASE_UCC5        (QE_PRAM_BASE + 0x2000)
#define QE_PRAM_BASE_SPI1        (QE_PRAM_BASE + 0x2800)
#define QE_PRAM_BASE_SPI2        (QE_PRAM_BASE + 0x2C00)
#define QE_PRAM_BASE_TIMER       (QE_PRAM_BASE + 0x3000)
#define QE_PRAM_BASE_USB         (QE_PRAM_BASE + 0x3200)
#define QE_PRAM_BASE_MIIRX_FIFO  (QE_PRAM_BASE + 0x3700)
#define QE_PRAM_BASE_MIITX_FIFO  (QE_PRAM_BASE + 0x3B00)
#define QE_PRAM_BASE_UCC6        (QE_PRAM_BASE + 0x3800)
#define QE_PRAM_BASE_UCC7        (QE_PRAM_BASE + 0x4000)
#define QE_PRAM_BASE_UCC8        (QE_PRAM_BASE + 0x4800)
#define QE_PRAM_BASE_GMIIRX_FIFO (QE_PRAM_BASE + 0x5000)
#define QE_PRAM_BASE_GMIITX_FIFO (QE_PRAM_BASE + 0x7000)
#define QE_PRAM_MASK             0x0001ffff
#define QE_PRAM_SHIFT            17
#define QE_PRAM_BASE_MCC         (QE_PRAM_BASE + 0x2000)
#define QE_PRAM_BASE_USB_EP      (QE_PRAM_BASE + 0x8c00)

#define QE_PRAM_BASE_TX_BD_SPI1  (QE_PRAM_BASE_SPI1  + 0x100)
#define QE_PRAM_BASE_RX_BD_SPI1  (QE_PRAM_BASE_SPI1  + 0x180)
#define QE_PRAM_BASE_TX_BD_SPI2  (QE_PRAM_BASE_SPI2  + 0x100)
#define QE_PRAM_BASE_RX_BD_SPI2  (QE_PRAM_BASE_SPI2  + 0x180)
#define QE_PRAM_BASE_TX_BD_TIMER (QE_PRAM_BASE_SPARE)
#define QE_PRAM_BASE_RX_BD_TIMER (QE_PRAM_BASE_SPARE + 0x100)
#define QE_PRAM_BASE_TX_BD_USB   (QE_PRAM_BASE_USB   + 0x200)
#define QE_PRAM_BASE_RX_BD_USB   (QE_PRAM_BASE_USB   + 0x400)
#define QE_PRAM_BASE_RX_TX_BD_UCC (QE_PRAM_BASE   + 0x100)

#define QE_PRAM_BASE_M2MHDLC     (QE_PRAM_BASE + 0x18A00)
#define QE_PRAM_BASE_INST1       (QE_PRAM_BASE + 0x10A00)

/*
 * The convention for all parameter ram structures is as follows:
 *
 *        quicc_XXX_param where:
 *                XXX - section of ram the structure represents.
 * All structures have a name and a type. The type is the name
 * with an _t suffix.  When naming the members of the structure
 * please use the name as given in the QUICC user manual.
 */

/*
 * QUICC Parameter RAM.
 */
typedef struct qe_ucc_param {
    uint16_t     rbase;                 /* Receive BD Base Address */
    uint16_t     tbase;                 /* Transmit BD Base Address */
    uint8_t      rbmr;                  /* Receive Function Code */
    uint8_t      tbmr;                  /* Transmit Function Code */
    uint16_t     mrblr;                 /* Receive Buffer Length */
    uint32_t     rstate;                /* Receive Internal State */
    uint32_t     rptr;                  /* Receive Internal Data Pointer */
    uint16_t     rbptr;                 /* Receive BD Pointer */
    uint16_t     rcount;                /* Receive Internal Byte Count */
    uint32_t     rtemp;                 /* Receive Temp */
    uint32_t     tstate;                /* Transmit Internal State */
    uint32_t     tptr;                  /* Transmit Internal Data Pointer */
    uint16_t     tbptr;                 /* Transmit BD Pointer */
    uint16_t     tcount;                /* Transmit Byte Count */
    uint32_t     ttemp;                 /* Transmit Temp */
    uint32_t     rcrc;                  /* Temp Receive CRC */
    uint32_t     tcrc;                  /* Temp Transmit CRC */
} qe_ucc_param_t;

typedef struct qe_hdlc_param {
    qe_ucc_param_t ucc_param;       /* Common to all protocols */
    uint8_t      hdlc_res_0[0x4];       /* Reserved */
    uint32_t     c_mask;                /* CRC Constant */
    uint32_t     c_pres;                /* CRC Preset */
    uint16_t     bof;                   /* Beginning-of-flag-character */
    uint16_t     eof;                   /* End-of-flag character */
    uint16_t     esc;                   /* Control escape sequence */
    uint8_t      hdlc_res_1[0x4];
    uint16_t     zero;                  /* Clear this field */
    uint8_t      hdlc_res_2[0x4];
    uint16_t     rfthr;                 /* Received frames threshold */
    uint8_t      hdlc_res_3[0x4];
    uint32_t     txctl_tbl;             /* Control character tables */
    uint32_t     rxctl_tbl;             /* Control character tables */
    uint16_t     nof;                   /* Number of opening flags to be sent */
} qe_hdlc_param_t;

typedef struct qe_uart_param {
    qe_ucc_param_t ucc_param;       /* Common to all protocols */
    uint8_t      uart_res_0[0x8];       /* Reserved */
    uint16_t     max_idl;               /* Maximum Idle Characters */
    uint16_t     idlc;                  /* Temporary Idle Counter */
    uint16_t     brkcr;                 /* Break Count Register (Transmit) */
    uint16_t     parec;                 /* Receive Parity Error Counter */
    uint16_t     frmer;                 /* Receive Framing Error Counter */
    uint16_t     nosec;                 /* Receive Noise Counter */
    uint16_t     brkec;                 /* Receive Break Character Counter */
    uint16_t     brkln;                 /* Last Received Break Length */
    uint16_t     uaddr1;                /* UART Address Character 1 */
    uint16_t     uaddr2;                /* UART Address Character 2 */
    uint16_t     rtemp;                 /* Temp Storage */
    uint16_t     toseq;                 /* Transmit Out-of-Sequence Char */
    uint16_t     cc[0x8];               /* Receive Control Characters */
    uint16_t     rccm;                  /* Receive Control Character Mask */
    uint16_t     rccr;                  /* Receive Control Character Register */
    uint16_t     rlbc;                  /* Receive Last Break Character */
    /* The following is for the soft UART fix */
    uint8_t      uart_res_1[41];        /* Reserved */
    uint16_t     soft_upsmr;
    uint8_t      uart_res_2[2];        /* Reserved */
    uint32_t     soft_rxstate;
    uint32_t     soft_rxcnt;
    uint8_t      soft_rxcharlen;
    uint8_t      soft_rxbitmark;
    uint8_t      soft_rxtempdlst;
    uint8_t      uart_res_3[30];        /* Reserved */
    uint32_t     soft_txframetmprem;
    uint8_t      soft_txframetmpremsize;
    uint8_t      soft_txmode;
    uint8_t      soft_txstate;
} qe_uart_param_t;

typedef struct qe_bisync_param {
    qe_ucc_param_t ucc_param;           /* Common to all protocols */
    uint8_t     uart_res_0[0x4];        /* Reserved */
    uint16_t    crcc;                   /* CRC constant temp value */
    uint16_t    prcrc;                  /* Preset receiver CRC16/LRC */
    uint16_t    ptcrc;                  /* Preset transmitter CRC16/LRC */
    uint16_t    parec;                  /* Receive Parity Error Counter */
    uint16_t    bsync;                  /* BISYNC SYNC register */
    uint16_t    bdle;                   /* BISYNC DLE register */ 
    uint16_t    cc[0x8];                /* Receive Control Characters */
    uint16_t    rccm;                   /* Receive Control Character Mask */
} qe_bisync_param_t;


/*
 * UEC Tx Parameter RAM structures
 */
typedef struct TxGblPram {
    volatile uint16_t      temoder;
    volatile uint8_t       res1[0x36];
    volatile uint32_t      sqptr;
    volatile uint32_t      sch_base_ptr;
    volatile uint32_t      tx_rmon_base_ptr;
    volatile uint32_t      tstate;
    volatile uint8_t       iph_offset[8];
    volatile uint32_t      vlan_tag_table[8];
    volatile uint32_t      tqptr;
    volatile uint32_t      res2;
    volatile uint8_t       rev3[8]; /* bytes padded to make 64bytes aligned */
} TxGblPram_t;


typedef struct TxThreadParam {         // See MPC8325's  section 29.5.3.4
    volatile uint8_t       res1[64];   // Multiply it by number of Tx Threads
} TxThreadParam_t;


typedef struct TxThreadData {    /* See MPC8325's section 29.5.3.3.1 . 
                                     Used by QE */
    volatile uint8_t       qe_tx_thd_data[192]; /* 64bytes * 3. this is 
                                                       to make it 64bytes 
                                                       aligned */
} TxThreadData_t;        


typedef struct TxSendQD {                // See MPC8325's section 29.5.3.3.2
    volatile uint32_t      bd_ring_base; // Multiply it by number of Tx Queues
    volatile uint8_t       res1[8];
    volatile uint32_t      last_bd_completed_addr;
    volatile uint8_t       res2[48];
} TxSendQD_t;


typedef struct TxSchedular {
    volatile uint16_t      cpu_count0;
    volatile uint16_t      cpu_count1;
    volatile uint16_t      cec_count0;
    volatile uint16_t      cec_count1;
    volatile uint16_t      cpu_count2;
    volatile uint16_t      cpu_count3;
    volatile uint16_t      cec_count2;
    volatile uint16_t      cec_count3;
    volatile uint16_t      cpu_count4;
    volatile uint16_t      cpu_count5;
    volatile uint16_t      cec_count4;
    volatile uint16_t      cec_count5;
    volatile uint16_t      cpu_count6;
    volatile uint16_t      cpu_count7;
    volatile uint16_t      cec_count6;
    volatile uint16_t      cec_count7;
    volatile uint32_t      weight_status0;
    volatile uint32_t      weight_status1;
    volatile uint32_t      weight_status2;
    volatile uint32_t      weight_status3;
    volatile uint32_t      weight_status4;
    volatile uint32_t      weight_status5;
    volatile uint32_t      weight_status6;
    volatile uint32_t      weight_status7;
    volatile uint32_t      rtsr_shadow;
    volatile uint32_t      time;
    volatile uint32_t      ttl;
    volatile uint32_t      mbl_interval;
    volatile uint16_t      nor_tsr_byte_time;
    volatile uint8_t       frac_siz;
    volatile uint8_t       res1;
    volatile uint8_t       strict_priority_q;
    volatile uint8_t       tx_asap;
    volatile uint8_t       extra_bw;
    volatile uint8_t       old_wfq_mask;
    volatile uint8_t       weight_factor0;
    volatile uint8_t       weight_factor1;
    volatile uint8_t       weight_factor2;
    volatile uint8_t       weight_factor3;
    volatile uint8_t       weight_factor4;
    volatile uint8_t       weight_factor5;
    volatile uint8_t       weight_factor6;
    volatile uint8_t       weight_factor7;
    volatile uint32_t      min_w;
    volatile uint32_t      res2;
    volatile uint8_t       res3[8];
    volatile uint8_t       res4[16]; /* pad 16 bytes to make this struct 64bytes aligned */
} TxSchedular_t;


typedef struct TxFirmwareCounter {
    volatile uint32_t      si_col_tx;
    volatile uint32_t      mul_col_tx;        
    volatile uint32_t      late_col_tx_Fr;
    volatile uint32_t      fr_abort_due_col;
    volatile uint32_t      fr_lost_in_mac_tx_err;
    volatile uint32_t      carrier_sense_err_tx;
    volatile uint32_t      fr_tx_ok;
    volatile uint32_t      tx_fr_excessive_differ;
    volatile uint32_t      tx_pkts_256;
    volatile uint32_t      tx_pkts_512;
    volatile uint32_t      tx_pkts_1024;
    volatile uint32_t      tx_pkts_jumbo;
    volatile uint8_t       res1[16];        /* padded 16 bytes to make this struct 64bytes aligned */
} TxFirmwareCounter_t;


/*
 * UEC Rx Parameter RAM structures
 */
typedef struct RxGblPram {
    volatile uint32_t      remoder;
    volatile uint32_t      rqptr;
    volatile uint8_t       res1[24];
    volatile uint16_t      type_or_len;
    volatile uint16_t      res2        :15;
    volatile uint16_t      rx_gstp_ack :1;
    volatile uint32_t      rx_rmon_base_ptr;
    volatile uint8_t       res3[8];
    volatile uint32_t      int_coalescing_ptr;
    volatile uint8_t       busy_vector;
    volatile uint8_t       res4;
    volatile uint8_t       rstate;
    volatile uint8_t       res5[14];
    volatile uint8_t       res6;
    volatile uint16_t      mrblr;
    volatile uint32_t      rbdq_ptr;
    volatile uint16_t      mflr;
    volatile uint16_t      minflr;
    volatile uint16_t      maxd1;
    volatile uint16_t      maxd2;
    volatile uint32_t      ecamPtr;
    volatile uint32_t      l2qt;
    volatile uint8_t       l3qt[32];
    volatile uint16_t      vlanType;
    volatile uint16_t      tci;
    volatile uint8_t       af[64];
    volatile uint32_t      exp_global_param;
    volatile uint8_t       res7[0x34];
    volatile uint8_t       zero[8];
} RxGblPram_t;


typedef struct RxThreadParam {
    volatile uint8_t       res1[128];  // 128 bytes for each thread, 
                                       // Max thread=8 (see MPC8325 29.5.3.6)
    /*                                 // Multiply it by number of Rx threads
     * If extended parsing used then 
     * add 64 or 128 or 160 bytes
     * See MPC8325 section 29.5.3.6
     */

} RxThreadParam_t;


typedef struct RxThreadData {     /* This structure is used by QE */
    volatile uint8_t       qe_rx_thd_data[64];
} RxThreadData_t;        



typedef struct RxFirmwareCounter {
    volatile uint32_t      fr_rx_fcs_err;
    volatile uint32_t      fr_align_err;
    volatile uint32_t      in_rang_len_rx_err;
    volatile uint32_t      out_rang_len_rx_err;
    volatile uint32_t      fr_too_long_rx;
    volatile uint32_t      runt;
    volatile uint32_t      very_long_event_rx;
    volatile uint32_t      symbol_err_rx;
    volatile uint32_t      ether_stats_drop_rx_bsy;
    volatile uint32_t      res1;
    volatile uint32_t      res2;
    volatile uint32_t      mis_match_drop;
    volatile uint32_t      ether_stats_under_pkts;
    volatile uint32_t      ether_stats_pkts_256;
    volatile uint32_t      ether_stats_pkts_512;
    volatile uint32_t      ether_stats_pkts_1024;
    volatile uint32_t      ether_stats_pkts_jumbo;
    volatile uint32_t      fr_loss_in_mac_rx_err;
    volatile uint32_t      pause_fr_rx;
    volatile uint32_t      res3;
    volatile uint32_t      rx_rm_vlan_cnt;
    volatile uint32_t      rx_rp_vlan_cnt;
    volatile uint32_t      rx_in_vlan_cnt;
    volatile uint8_t       res4[36];  /* padded bytes, to make this struct 
                                        64 bytes aligned */
} RxFirmwareCounter_t;


typedef struct RxIntCoalescingTbl { // See MPC8325's  Table 29-43: 
                                     // Rx Interrupt Coalescing structure 
    volatile uint32_t      q_int_coalescing_max;  // Multiply it by number of 
                                                  // Rx queues
    volatile uint32_t      q_int_coalescing_cnt;
} RxIntCoalescingTbl_t;



typedef struct RxBDQueueTable { // See MPC8325's  Table 29-40: 
                                 // RxBD Queue data structure
    volatile uint32_t      pr_bd_base_ptr; // Multiply it by number of Rx queues
    volatile uint32_t      pr_bd_ptr;
    volatile uint32_t      pr_ebd_base_ptr;
    volatile uint32_t      pr_ebd_ptr;
} RxBDQueueTable_t;


typedef struct InitEnet {
    volatile uint8_t       res0;                               // CECDR + 0
    volatile uint8_t       res1;                               // CECDR + 1
    volatile uint8_t       res2;                               // CECDR + 2
    volatile uint8_t       res3;                               // CECDR + 3
    volatile uint16_t      res4;                               // CECDR + 4
    volatile uint16_t      largest_lookup_key_sz;              // CECDR + 6
    volatile uint32_t      rgf                      :4;        // CECDR + 8
    volatile uint32_t      tgf                      :4;
    volatile uint32_t      rx_glb_param_page        :18;
    volatile uint32_t      res6                     :2;
    volatile uint32_t      rx_risc_alloc            :4;
    volatile uint32_t      rx_th0_snum              :8;        // CECDR + c
    volatile uint32_t      res7                     :18;
    volatile uint32_t      res8                     :2;
    volatile uint32_t      rx_risc_alloc0           :4;
    volatile uint32_t      rx_th1_snum              :8;        // CECDR + 10 
    volatile uint32_t      rx_th1_param_page        :18;
    volatile uint32_t      res9                     :2;
    volatile uint32_t      rx_risc_alloc1           :4;
    volatile uint32_t      rx_th2_snum              :8;        // CECDR + 14 
    volatile uint32_t      rx_th2_param_page        :18;
    volatile uint32_t      res10                    :2;
    volatile uint32_t      rx_risc_alloc2           :4;
    volatile uint32_t      rx_th3_snum              :8;        // CECDR + 18 
    volatile uint32_t      rx_th3_param_page        :18;
    volatile uint32_t      res11                    :2;
    volatile uint32_t      rx_risc_alloc3           :4;
    volatile uint32_t      rx_th4_snum              :8;        // CECDR + 1c 
    volatile uint32_t      rx_th4_param_page        :18;
    volatile uint32_t      res12                    :2;
    volatile uint32_t      rx_risc_alloc4           :4;
    volatile uint32_t      rx_th5_snum              :8;        // CECDR + 20 
    volatile uint32_t      rx_th5_param_page        :18;
    volatile uint32_t      res13                    :2;
    volatile uint32_t      rx_risc_alloc5           :4;
    volatile uint32_t      rx_th6_snum              :8;        // CECDR + 24 
    volatile uint32_t      rx_th6_param_page        :18;
    volatile uint32_t      res14                    :2;
    volatile uint32_t      rx_risc_alloc6           :4;
    volatile uint32_t      rx_th7_snum              :8;        // CECDR + 28 
    volatile uint32_t      rx_th7_param_page        :18;
    volatile uint32_t      res15                    :2;
    volatile uint32_t      rx_risc_alloc7           :4;
    volatile uint32_t      rx_th8_snum              :8;        // CECDR + 2c 
    volatile uint32_t      rx_th8_param_page        :18;
    volatile uint32_t      res16                    :2;
    volatile uint32_t      rx_risc_alloc8           :4;
    volatile uint32_t      res17;                              // CECDR + 30 
    volatile uint32_t      res18;                              // CECDR + 34 
    volatile uint32_t      res19                    :8;        // CECDR + 38 
    volatile uint32_t      tx_glb_param_page        :18;
    volatile uint32_t      res20                    :2;
    volatile uint32_t      tx_risc_alloc            :4;
    volatile uint32_t      tx_th1_snum              :8;        // CECDR + 3c 
    volatile uint32_t      tx_th1_param_page        :18;
    volatile uint32_t      res21                    :2;
    volatile uint32_t      tx_risc_alloc1           :4;
    volatile uint32_t      tx_th2_snum              :8;        // CECDR + 40 
    volatile uint32_t      tx_th2_param_page        :18;
    volatile uint32_t      res22                    :2;
    volatile uint32_t      tx_risc_alloc2           :4;
    volatile uint32_t      tx_th3_snum              :8;        // CECDR + 44 
    volatile uint32_t      tx_th3_param_page        :18;
    volatile uint32_t      res23                    :2;
    volatile uint32_t      tx_risc_alloc3           :4;
    volatile uint32_t      tx_th4_snum              :8;        // CECDR + 48 
    volatile uint32_t      tx_th4_param_page        :18;
    volatile uint32_t      res24                    :2;
    volatile uint32_t      tx_risc_alloc4           :4;
    volatile uint32_t      tx_th5_snum              :8;        // CECDR + 4c 
    volatile uint32_t      tx_th5_param_page        :18;
    volatile uint32_t      res25                    :2;
    volatile uint32_t      tx_risc_alloc5           :4;
    volatile uint32_t      tx_th6_snum              :8;        // CECDR + 50 
    volatile uint32_t      tx_th6_param_page        :18;
    volatile uint32_t      res26                    :2;
    volatile uint32_t      tx_risc_alloc6           :4;
    volatile uint32_t      tx_th7_snum              :8;        // CECDR + 54 
    volatile uint32_t      tx_th7_param_page        :18;
    volatile uint32_t      res27                    :2;
    volatile uint32_t      tx_risc_alloc7           :4;
    volatile uint32_t      tx_th8_snum              :8;        // CECDR + 58 
    volatile uint32_t      tx_th8_param_page        :18;
    volatile uint32_t      res28                    :2;
    volatile uint32_t      tx_risc_alloc8           :4;
    volatile uint32_t      res29;                                // CECDR + 5c 
    volatile uint8_t       res30[32]; /* bytes padded to make 64bytes aligned */
} InitEnet_t;

/*
 * Ethernet Tx/Rx Global Parameter RAM defines
 */
#define UEC_TXPRAM_SOF                0x8000
#define UEC_TXPRAM_SOBD               0x4000
#define UEC_TXPRAM_SCHD_EN            0x2000
#define UEC_TXPRAM_RMON_EN            0x0100
#define UEC_TXPRAM_1_Q                0x0000
#define UEC_TXPRAM_2_Q                0x0001
#define UEC_TXPRAM_3_Q                0x0002
#define UEC_TXPRAM_4_Q                0x0003
#define UEC_TXPRAM_5_Q                0x0004
#define UEC_TXPRAM_6_Q                0x0005
#define UEC_TXPRAM_7_Q                0x0006
#define UEC_TXPRAM_8_Q                0x0007
#define UEC_TXPRAM_Q(x)               (x - 1)

#define UEC_BMR_GBL                   0x20
#define UEC_BMR_BO                    0x10    // BO(3-4) = 0b10 <-- Big Endian
                                              // byte ordering
#define UEC_BMR_DTB                   0x02    // 0 <-- on coherent system bus,
                                              // 1 <-- on QE secondary bus
#define UEC_BMR_BDB                   0x01    // 0 <-- on coherent system bus,
                                              //1 <-- on QE secondary bus


#define UEC_RXPRAM_RFSE               0x00001000
#define UEC_RXPRAM_1_Q                0x00000000
#define UEC_RXPRAM_2_Q                0x00000100
#define UEC_RXPRAM_3_Q                0x00000200
#define UEC_RXPRAM_4_Q                0x00000300
#define UEC_RXPRAM_5_Q                0x00000400
#define UEC_RXPRAM_6_Q                0x00000500
#define UEC_RXPRAM_7_Q                0x00000600
#define UEC_RXPRAM_8_Q                0x00000700


#define TX_THREAD                     1
#define TX_QUEUE                      4
#define RX_THREAD                     1
#define RX_QUEUE                      4

#define MAX_RX_QUEUE                  8
#define MAX_TX_QUEUE                  8
#define MAX_RX_INT_COALESCING_CTR     8

#define MURAM_BASE_OFFSET_MASK        0x0000ffc0
#define RX_TX_PARAM_BASE_MASK         0x001fffc0


/*
 * ATM Structures and Defines
 */

/* Loopback/Scheduling sources */
#define INTERNAL            1     /* Internal loopback or schedule tick */ 
#define EXTERNAL            2     /* External loopback or schedule tick */ 

#define AAL5_BD_RX_ERROR    0xB   /* Check for CRC, Abort, and Length errors */
#define AAL0_BD_RX_ERROR    0x8   /* Check only if the "Abort" bit was set */ 
#define AAL1_BD_RX_ERROR    0x800 /* Check for Sequence Number Error bit */ 

/*--------------------*/
/* PHY Types          */
/*--------------------*/
#define SERIAL_PHY          1     /* NOT SUPPORTED IN 1ST Release of 8260 */ 
#define UTOPIA_PHY          2

/*------------------------*/
/* Adaptation Layers      */
/*------------------------*/
#define AAL0                1
#define AAL5                2
#define AAL1                3

/*--------------------------------------------------------------*/
/* 8260 ATM Exception Queue Entry Bit Definitions               */
/*--------------------------------------------------------------*/
#define RXB                 0x10000    /* Rcv Buffer event    */
#define TXB                 0x20000    /* Xmit bufffer event  */
#define BSY                 0x40000    /* Receiver busy event */
#define RXF                 0x80000    /* Received frame event */
#define TBNR                0x100000   /* Tx Buffer not ready */ 

#define Q_ENTRY_VALID       0x80000000 /* Interrupt Queue entry valid bit */
#define Q_WRAP              0x20000000 /* Interrupt Queue entry wrap bit  */

/*-----------------------------------------------------------------*/
/* Number of Instructions in Vector Table for particular Interrupt */
/*-----------------------------------------------------------------*/
#define VECTOR_BLOCK_LEN    0x100

/*----------------------------------------------------------*/
/* FCCx Interrupt Vector Code in CPM Vector Register (CIVR) */
/*----------------------------------------------------------*/
#define FCC1_VECTOR         0x20
#define READY_TO_RX_CMD     0          /* Ready to receive a command */


/**************************************************************/
/* NOTE:                                                      */
/* For this example, set the Frame length to a multiple of 47 */ 
/* when using AAL1.                                           */
/**************************************************************/
#define NUM_FRAMES          2   /* Number of Frames to Transmit */
#define FRAME_LENGTH        96  /* Length of each frame (bytes) */
#define RAW_CELL_BUFF_SIZE  64  /* Raw cell buffer size (bytes) */
#define RAW_CELL_SIZE       52  /* Size of cell in (bytes), includes header */
#define MAX_NUM_CH          8   /* Maximum number of channels */

/*----------------------------------------------------------*/
/* Data structure definitions.                          */
/*----------------------------------------------------------*/

/* ATM Receive UTOPIA master mode.  Bit set to 1 indicates */
/* Receiver is acting as a slave.                          */
#define FPSMR_ATM_RUMS      0x00200000
#define FPSMR_ICD           0x00800000
#define FPSMR_RxP           0x00000400
#define FPSMR_UPLM          0x00000010

/* UCCE and UCCM initial values */
#define ATM_UCCE            0x0000
#define ATM_UCCM            0x07FF
#define ATM_UCCE_GINT0      0x0010
#define ATM_UCCE_GINT1      0x0020
#define ATM_UCCE_GINT2      0x0040
#define ATM_UCCE_GINT3      0x0080
#define ATM_UCCE_TIRU       0x0400
#define ATM_UCCE_GRLI       0x0200
#define ATM_UCCE_GBPB       0x0100
#define ATM_UCCE_INTQ3      0x0008
#define ATM_UCCE_INTQ2      0x0004
#define ATM_UCCE_INTQ1      0x0002
#define ATM_UCCE_INTQ0      0x0001


/* GFMR masks */
#define GFMR_LOOPBACK       0x40000000 /* Internal Loopback mode */ 

/* Buffer Descriptor initialization parameters */ 
typedef struct
{
    uint32_t  ch_num;          /* Initialize THIS channel's BD */
    uint8_t   interrupt;       /* Enable interrupts? TRUE/FALSE */
    uint8_t   type;            /* BD type, NORMAL/PORT_TO_PORT */ 
    uint8_t   oam_ch;          /* If 1, then its a OAM channel */
} BD_param_t; 

/**********************************************/
/* Receive Connection Table (RCT) structures. */ 
/**********************************************/

/*--------------------------------------------*/
/* RCT status and control bit data structure. */ 
/*--------------------------------------------*/
typedef struct 
{
    unsigned reserved1:2;
    unsigned gbl:      1;        /* Global - enable snooping */ 
    unsigned bo:       2;        /* Byte ordering            */     
    unsigned cetm:     1; 
    unsigned dtb:      1;        /* Data Buffer bus selection */ 
    unsigned bib:      1;        /* BD/Int queue bus selection */ 
    unsigned reserved3:1;
    unsigned bufm:     1;        /* Buffer mode               */
    unsigned segf:     1;        /* F5 segment filtering      */
    unsigned endf:     1;        /* F5 end-to-end segment filtering */ 
    unsigned cpuu:     1;        /* CPUU copying - enable     */ 
    unsigned reserved4:1;
    unsigned intq:     2;        /* Int. queue selection.     */
    unsigned reserved5:1;
    unsigned inf:      1;        /* In-frame indication.      */ 
    unsigned reserved6:10;
    unsigned reserved7:1;        /* ABR Flow control.         */
    unsigned aal:      3;        /* AAL selection.            */ 
} Rct_stat_t; 

/*---------------------*/
/* RCT data structure. */ 
/*---------------------*/

typedef struct 
{
    union                                /* RCT Status Fields */
    {
        volatile uint32_t Stat_word;   /* RCT status/control word */
        Rct_stat_t        Stat_field;  /* RCT status/control word */
                                       /* data structure */
    } RctStatus; 

    volatile uint32_t p_RxData;        /* Rx data buffer pointer*/
    volatile uint32_t CellTimeStamp;   /* Cell time stamp */

    union                              /* Protocol specific RCT */
    {
        /* AAL5 protocol specific RCT */
        struct  
        {            
            volatile uint16_t RxBdOffset;    /* Rx BD offset from Rx BD */
                                             /* base address */
            volatile uint16_t Tml;                
            volatile uint32_t CRC_TempRes;        
            volatile uint16_t RxBdCnt;
            volatile uint16_t Pcr;                
            volatile uint16_t StatusMask;        
            volatile uint16_t Mrblr;         /* Maximum receive buffer length */
        } AAL_5;

        /* AAL1 protocol specific RCT */
        struct 
        {            
            volatile uint16_t RxBdOffset;    /* Rx BD offset from Rx BD base address */
            volatile uint16_t StatusBits;    /* The SRTV, SRT, PFM and STF status bits */

            volatile uint16_t SrtsDevice;    /* SRTS device (0-15) */
            volatile uint8_t Vos;            /*
                                              * Valid octet size ( 1-47 for 
                                              * unstructured format and 1-46 
                                              * for structured). Used in 
                                              * partially filled cell mode 
                                              * only. 
                                              */

            volatile uint8_t Sp;             /* Structured pointer. */
            volatile uint16_t RxBdCnt;       /* Receive BD count */
            volatile uint16_t SeqNumber;     /* Sequence number */

            volatile uint16_t StatusMask;    /*
                                              * RXBM (Receive buffer mask) and 
                                              * SNEM (Sequence number error 
                                              * flag) interrupt masks 
                                              */    
            volatile uint16_t Mrblr;         /* Maximum receive buffer length */
        } AAL_1;

           /* AAL0 protocol specific RCT */
           struct
           {
              volatile uint16_t RxBdOffset;  /* Rx BD offset from Rx BD base 
                                                address */
              volatile uint16_t StatusBits;        
              volatile uint8_t reserved3[8];
              volatile uint16_t StatusMask;
              volatile uint16_t Mrblr;       /* Maximum receive buffer length */
           } AAL_0;

           struct
           {
              volatile uint16_t RxBdOffset;  /*
                                              * Rx BD offset from Rx BD base 
                                              * address 
                                              */
              volatile uint8_t reserved4[10];
              volatile uint16_t StatusMask;        
              volatile uint16_t Mrblr;       /* Maximum receive buffer length */
           } General;

    } ProtocolSpecificRct;

    volatile uint32_t RxBdBase;              /* Rx BD base address */

} Rct_t;


/*
 * ATM Cell header data structure used in TCT.
 *
 * Note: This data structure is Big Endian.
 *
*/ 

typedef struct
{
    unsigned gfc:4;         /* Generic Flow Control;      */ 
    unsigned vpi:8;         /* Virtual Path Identifier    */ 
    unsigned vci:16;        /* Virtual Channel Identifier */ 
    unsigned pti:3;         /* Payload type Identifier    */
    unsigned clp:1;         /* Cell Loss Priority         */
} Cell_header_t; 

/*********************************************/
/* Transmit Connection Table (TCT) structure */ 
/*********************************************/

typedef struct 
{
    unsigned reserved1:2;
    unsigned gbl:1;       /* Global - enable snooping */ 
    unsigned bo:2;        /* Byte ordering.           */ 
    unsigned cetm:1;      /* QE Transaction Mark      */
    unsigned dtb:1;       /* Data buffer bus - local/ppc   */
    unsigned bib:1;       /* BD/int. queue bus - local/ppc */
    unsigned avcf:1;      /* Auto VC                       */ 
    unsigned reserved3:1;
    unsigned att:2;       /* ATM traffic type - e.g. CBR   */ 
    unsigned cpuu:1;      /* CPUU insertion.               */ 
    unsigned vcon:1;      /* Virtual Channel On            */ 
    unsigned intq:2;      /* Interrupt queue selection.    */ 
    unsigned reserved4:1;
    unsigned inf:1;       /* In-frame, xmit state.         */ 
    unsigned reserved5:10;
    unsigned abrf:1;      /* ABR Flow control.             */ 
    unsigned aal:3;       /* AAL selection.                */ 
} Tct_stat_t; 

/*---------------------*/
/* TCT data structure. */ 
/*---------------------*/

typedef struct 
{
    union                             /* TCT Status Fields */
    {
        volatile uint32_t Stat_word;  /* TCT status/control word. */
        Tct_stat_t        Stat_field; /* TCT status/control data structure */
    } TctStatus; 

    volatile uint32_t p_TxData;       /* Tx data buffer pointer */
    volatile uint16_t  TxBdCnt;       /* Tx BD count */
    volatile uint16_t  TxBdOffset;    /* Tx BD offset from TBD_BASE */
    volatile uint8_t  RateReminder;   /* Rate reminder */
    volatile uint8_t  PcrFraction;    /* Peak cell rate fraction */
    volatile uint16_t  Pcr;           /* Peak cell rate */

    union                             /* Protocol specific TCT */
    {
        /* AAL5 protocol specific TCT */
        struct 
        {
            volatile uint32_t CRC_TempRes;        /* CRC32 temporary result */
            volatile uint16_t  Tml;               /* Total message length */
            volatile uint16_t  ApcLinkedChannel;  /* APC linked channel */
        } AAL_5;

        /* AAL1 protocol specific TCT */
        struct 
        {
            volatile uint8_t  Vos;               /* Valid octed size (1-47 for                                                    * unstructured format, 1-46 
                                                  * for structured) Used in 
                                                  * partial fill cell mode only
                                                  */

            volatile uint8_t  StatusBits;        /*
                                                  * SRT - Synchronous residuum 
                                                  *       time stamp;
                                                  * PFM - Partially filled mode;
                                                  * STF - Structured format;
                                                  * SPF - Structured pointer 
                                                  *       flag;
                                                  * SN - Sequence number bits 
                                                  */

            volatile uint16_t  BlockSize;        /*
                                                  * Structured block size. 
                                                  * (In structured format only)
                                                  */

            volatile uint16_t  Sp;               /*
                                                  * Structured pointer (12 least
                                                  * significant bits). 
                                                  */
            volatile uint16_t  ApcLinkedChannel; /* APC linked channel */
        } AAL_1;

        /* AAL0 protocol specific TCT */
        struct 
        {
            volatile uint16_t  StatusBits;       /* CR10 and ACHC bits */
            volatile uint16_t  RESERVED4;
            volatile uint16_t  RESERVED5;
            volatile uint16_t  ApcLinkedChannel; /* APC linked channel */
        } AAL_0;

    } ProtocolSpecificTct;
    volatile Cell_header_t CellHeader;                /* ATM cell header */
    volatile uint32_t TxBdBase;                  /*
                                                  * Holds the PMT number Tx BD 
                                                  * base address and interrupt 
                                                  * masks 
                                                  */
} Tct_t;

/*----------------------------------------*/ 
/* Interrupt Queue (INTQ) Parameter Table.*/ 
/*----------------------------------------*/ 

typedef struct
{
    volatile uint32_t IntqBase;        /* Base address of interrupt queue */
    volatile uint32_t p_IntQ;          /* Pointer to interrupt queue entry */
    volatile uint16_t IntCnt;          /* Interrupt counter */
    volatile uint16_t IntInitCnt;      /* Interrupt counter initial value */
    volatile uint32_t IntqEntry;       /* Pointer to interrupt queue entry */
} IntQParameterTbl_t;

/************************/
/* APC Parameter Table  */
/************************/

typedef struct
{
    volatile uint16_t ApcLevFirst;  /* Address of first APC priority level */
    volatile uint16_t ApcLevLast;   /* Address of last APC priority level */
    volatile uint16_t ApcLevPtr;    /* Address of current APC priority level */
    volatile uint8_t  Cps;          /* Number of cells sent per APC slot */
    volatile uint8_t  CpsCnt;       /* Cells per APC slot counter */
    volatile uint8_t  MaxIteration; /* Max iteration allowed */
    volatile uint8_t  res;
    volatile uint16_t FirstUBR_lvl; /* The line rate of the port for ABR protocol only */
    volatile uint32_t RealTstPtr;   /* Real time stamp pointer */
    volatile uint32_t ApcState;
    volatile uint32_t res1;
    volatile uint32_t ApcSlotDurValInt;
    volatile uint16_t ApcSlotDurValFrac;
    volatile uint16_t SchedulerMode;
} ApcParameterTbl_t;

/***********************/
/* APC Priority table  */
/***********************/

typedef struct
{
    volatile uint16_t ApcLevBase;        /* APC level i base address */
    volatile uint16_t ApcLevEnd;         /* APC level i end address */
    volatile uint16_t ApcLevRptr;        /* APC level i real-time pointer */
    volatile uint16_t ApcLevSptr;        /* APC level i service pointer */
} ApcPriorityTbl_t;

/* RCT initialization parameters */ 
typedef struct
{
    uint32_t  ch_num;      /* Initialize this channel's RCT entry */
    uint32_t  bd_addr;     /* Address of BD */ 
    uint8_t   reset;
    uint8_t   interrupt;   /* Enable interrupt */  
    uint8_t   oam_ch;
} RCT_param_t; 
        
/* TCT initialization parameters */ 
typedef struct
{
    uint32_t  ch_num;      /* Initialize this channel's TCT entry */ 
    uint32_t  bd_addr;     /* BD address */
    uint32_t  be_header;   /* Big Endian header */ 
    uint32_t  vc_rate; 
    uint8_t   reset;
} TCT_param_t; 

/* AAL0 data buffer format */ 
typedef struct
{
    Cell_header_t cell_header; 
    uint8_t payload[48]; 
    uint8_t reserved[12];
} AAL0_buf_t; 


/* QE CECR - bit definitions. */ 
typedef struct
{
    unsigned rst:1; 
    unsigned unused1:5; 
    unsigned sbc:9; 
    unsigned flg:1; 
    unsigned unused2:2; 
    unsigned mcn:8; 
    unsigned opcode:6; 
} QE_cecr_t; 

//#pragma pack(pop)

/*----------------------------------------------------------*/
/* Memory Address and Offset Definitions.  Before modifying */
/* keep in mind that some addresses have boundary re-       */
/* quirements.  All offsets are from DPR unless otherwise   */
/* specified.                                               */
/*----------------------------------------------------------*/

/* Start of Internal Memory - VBUG */ 
#define BD_SIZE 0x100             /* Space allocated for each channel's */
                                  /* BD list (number of bytes)          */
#define BUFF_SIZE 0x400           /* Space allocated for each channel's */
                                  /* xmit or rcv buffer (number of bytes) */
#define NUM_QUEUE_ENTRIES 0x100   /* Number of queue entries */

#define NUM_RAW_CELL_BUFFS 40

/****************************************/
/* DPR address definitions.             */ 
/****************************************/

#define ATM_EXT_MEM_BASE    0x81000000
#define UCC_PARAM_OFFSET    0x3100
#define UCC_PARAM_BASE      MURAM_BASE + UCC_PARAM_OFFSET // 68513100

#define EXT_CAM_BASE        ATM_EXT_MEM_BASE + 0x1000 // 81001000

#define VCT_BASE                   ATM_EXT_MEM_BASE + 0x1000

#define Tx_Virtual_FIFO_offset      0x00002a00
#define Tx_Virtual_FIFO_base_address    MURAM+0x2a00

#define Idle_Cell_offset        0x2000
#define Idle_Cell_base          MURAM + Idle_Cell_offset

#define INTQ_PARAM_OFFSET       0x10
#define INTQ_PARAM_BASE         MURAM + INTQ_PARAM_OFFSET
#define INTQ_BASE           ATM_EXT_MEM_BASE

#define UNI_STAT_TBL_OFFSET     0x20
#define UNI_STAT_TBL_BASE       MURAM + UNI_STAT_TBL_OFFSET

#define APC_table_offset        0x1000
#define APC_table_base          MURAM + APC_table_offset

#define EXT_RCT_BASE   ATM_EXT_MEM_BASE + 0x9000   //81009000
#define EXT_TCT_BASE   ATM_EXT_MEM_BASE + 0xd000   //8100d000
#define EXT_TCTE_BASE  ATM_EXT_MEM_BASE + 0x20000  //8100d000

/* RCT and TCT are equal to base + 0x2000 (for channel 256 only) */
#define ExternalRCT_ch256   EXT_RCT_BASE+0x2000  //8100b000
#define ExternalTCT_ch256   EXT_TCT_BASE+0x2000  //8100f000

#define BD_BASE_EXT__MSB    (ATM_EXT_MEM_BASE & 0xff000000) >> 24 // 81

#define RCT_BD_ch256   0x81008000
#define TCT_BD_ch256   0x81007000

#define RxBD_1_addr     ATM_EXT_MEM_BASE + 0x0100
#define RxBD_2_addr     ATM_EXT_MEM_BASE + 0x0200
#define RxBD_3_addr     ATM_EXT_MEM_BASE + 0x0300

#define TxBD_1_addr         ATM_EXT_MEM_BASE + 0x5100
#define TxBD_2_addr         ATM_EXT_MEM_BASE + 0x5200
#define TxBD_3_addr         ATM_EXT_MEM_BASE + 0x5300


#define DPR_BASE                   PRAM_OFFSET
#define MURAM_BASE_OFFSET          PRAM_OFFSET
#define MURAM_BASE_PHY             ADRSPC_PQUICC_REGB + MURAM_BASE_OFFSET

#define ATM_MURAM_BASE_OFFSET      0x2000

#define ATM_MURAM_BASE_PHY         ADRSPC_PQUICC_REGB + MURAM_BASE + \
                                   ATM_MURAM_BASE_OFFSET        

#define GLB_ATM_MURAM_OFFSET       ATM_MURAM_BASE_OFFSET + 0x100

/* Idle Cell Template - offset */ 
#define IDLET_OFFSET               ATM_MURAM_BASE_OFFSET + 0x300 

/* Sequence Number Protection Table */
/* (SNPT) offset, AAL 1 mode.       */
#define SNPT_OFFSET                ATM_MURAM_BASE_OFFSET + 0x340 

/* UNI Statistics */ 
#define UNI_STAT_OFFSET            ATM_MURAM_BASE_OFFSET + 0x360 

/* Connection Table */ 
#define RCT_BASE_OFFSET            ATM_MURAM_BASE_OFFSET + 0x400

                                   /* 8 ch * 0x20 = 0x100 */
#define TCT_BASE_OFFSET            (RCT_BASE_OFFSET + (MAX_NUM_CH * \
                                                       sizeof(t_Rct)))

/* Start of APC Parameters */
#define APCP_OFFSET                ATM_MURAM_BASE_OFFSET + 0x600 

/* Prioritiy Tables Offset */
#define PRTY_TBL_OFFSET            ATM_MURAM_BASE_OFFSET + 0x700 

/* Schedulinng Table Offset */ 
#define SCHD_TBL_OFFSET            ATM_MURAM_BASE_OFFSET + 0x7c0 

/* Start or Interrupt Queue Parameter */
#define INTQP_OFFSET               ATM_MURAM_BASE_OFFSET + 0xbf0 


/* Table 30-30: Address Look-Up Table */
#define ADR_LOOKUP_TABLE_OFFSET    ATM_MURAM_BASE_OFFSET + 0xd00        

#define ADR_LOOKUP_TABLE_PHY_ADDR  MURAM_BASE_PHY + ADR_LOOKUP_TABLE_OFFSET 

/* Start of VP Table (0x3000 - 0x31ff) */
#define VPT_OFFSET                 ATM_MURAM_BASE_OFFSET + 0x1000 

/* Start of INT_RCT_TMP_ptr (table 30-16) */
/* range: 0x3200 - 0x323F */
#define INT_RCT_TMP_PTR            ATM_MURAM_BASE_OFFSET + 0x1200 

/* range: 0x3240 - 0x325F */ 
#define RX_ADDR_LOOKUP             ATM_MURAM_BASE_OFFSET + 0x1240 

/* range: 0x3260 - 0x327F */
#define OAM_CH_RCT_PTR             ATM_MURAM_BASE_OFFSET + 0x1260 

/* range: 0x3280 - 0x329F */
#define INT_TCTE_TMP_PTR           ATM_MURAM_BASE_OFFSET + 0x1280 

/* range: 0x32A0 - 0x32BF (only 12 bytes used) */
#define DYN_ADD_COMP_BASE          ATM_MURAM_BASE_OFFSET + 0x12A0 

/* range: 0x3300 - 0x333F */
#define UPC0_TEMP                  ATM_MURAM_BASE_OFFSET + 0x1300 

/* range: 0x3340 - 0x337F */
#define UPC1_TEMP                  ATM_MURAM_BASE_OFFSET + 0x1340 

/* range: 0x3380 - 0x33BF */
#define UPC2_TEMP                  ATM_MURAM_BASE_OFFSET + 0x1380 

/* range: 0x33C0 - 0x343F */
#define FREE_BUF_POOL_OFFSET       ATM_MURAM_BASE_OFFSET + 0x13C0 

#endif

/* MURAM Memory Map 0x0000 - 0x4000 */
/* offset 0x0 - 0x800 is for the UCC */
#define GLOBAL_ATM_PARAM_TBL       0x0860
#define OAM_CH_RCT                 0x0890
#define ADDR_LOOKUP_TABLE_BASE     0x0900
#define VPT_BASE                   0x3D00
#define APCP_BASE                  0x1000
#define FBT_BASE                   0x1340
#define INTT_BASE                  0x1360
#define UNI_STATT_BASE             0x1440
#define IDLE_BASE                  0x1460
#define PMT_BASE                   0x1500
#define INT_RTCRT_BASE             0x1D00
#define AAL1_SNPT_BASE             0x1D40
#define INT_RTC_BASE               0x1E00
#define INT_TCT_BASE               0x1E80
#define INT_TCTE_BASE              0x1F00
#define RX_VIR_FIFO_BASE           0x2200
#define TX_VIR_FIFO_BASE           0x2A00
#define SER_DMA_TEMP_BUF_BASE      0x3C00


/*
 * UCC - ATM parameter RAM Page (see Table 30-15)
 *       offset from UCC default base address
 *       (i.e. UCC5 is at offset 0x8000 from RAM_BASE
 *          RAM_BASE is at PRAM_OFFSET )
 *
 *          Size: 0x10
 */
typedef struct ucc_atm_param_header_ {
    uint16_t local_page_parameters_ptr;             // offset 0x00
    uint16_t sub_page0_configuration_table_ptr;     // offset 0x02
    uint16_t sub_page0_Rx_tmp_table_ptr;            // offset 0x04
    uint16_t sub_page0_Tx_tmp_table_ptr;            // offset 0x06
    uint16_t sub_page1_configuration_table_ptr;     // offset 0x08
    uint16_t sub_page1_Rx_tmp_table_ptr;            // offset 0x0a
    uint16_t sub_page1_Tx_tmp_table_ptr;            // offset 0x0c
    uint8_t  res[2];                                // offset 0x0e
} ucc_atm_param_header_t;


/*
 * UCC - Local Page Parameter table
 *       (see Table 30-16)
 *
 *         Size: 0x10
 */
typedef struct ucc_local_page_param_table_ {
    uint16_t int_rct_tmp_ptr;        // offset 0x00
    uint16_t ima_temp;               // offset 0x02
    uint16_t rx_tmp;                 // offset 0x04
    uint16_t rxqd_tmp;               // offset 0x06
    uint16_t pprs_int_ptr;           // offset 0x08
    uint16_t res1;                   // offset 0x0a
    uint     res2;                   // offset 0x0c
} ucc_local_page_param_table_t;



/* UCC - Sub-page0 Configuration table
 *       (see Table 30-21)
 *
 *         Size: 0x60
 */
typedef struct ucc_subpage0_config_table_ {
    uint16_t GlobalAtmParamTblPtr;
    uint16_t OamChRctPtr;
    uint16_t ImaRoot;
    uint16_t UCCMode;
    uint32_t CamMask;
    uint16_t Gmode;             /* Global mode */
    uint16_t IntTcteTmpPtr;
    uint16_t ExtCamBaseMsb;
    uint16_t ExtCamBaseLsb;
    uint16_t RxToutReqPeriod;
    uint16_t VciFiltering;      /* VCI filtering enable bits. If bit i is set,
                                   the cell with VCI=i will be sent to the
                                   raw cell queue. The bits 0-2 and 5 should
                                   be zero. */
    uint16_t ApcParamBase;      /* APC Parameters table base address */
    uint16_t FbpParamBase;      /* Free buffer pool parameters base address */
    uint16_t IntQParamBase;     /* Interrupt queue parameters table base */
    uint16_t UniStatTableBase;  /* UNI statistics table base */
    uint16_t UeadOffset;        /* The offset in half-wordunits of the UEAD
                                   entry in the UDC extra header. Should be
                                   even address. If little-endian format is
                                   used, the UeadOffset is of the little-endian
                                   format. */
    uint16_t IdleBase;          /* Idle cell base address */
    uint16_t IdleSize;          /* Idle cell size: 52, 56, 60, 64 */
    uint8_t  BdBaseExt;         /* BD ring base address extension */
    uint8_t  res1;              /* Reserved. Should be cleared */
    uint32_t TcellTmpBaseExt;   /* For AAL2 ... */
    uint16_t RxQDBaseInt;
    uint16_t PmtBase;           /* Performance monitoring table base address */
    uint32_t AAL1ExtStatsBase;
    uint16_t ALL1DummyCellBase;
    uint16_t IntRtcrtBase;
    uint32_t ExtRtcrtBase;
    uint16_t AAL1SnpTableBase;  /* AAL1 SNP protection look-up table base */
    uint16_t AdpThrBase;
    uint32_t SrtsBase;          /* External SRTS logic base address. For AAL1
                                   only. Should be 16 bytes aligned */
    uint16_t InCasBlkBase;
    uint16_t OutCasBlkBase;
    uint16_t AAL1OutCasStatReg;
    uint16_t res2;
    uint16_t AAL1IntStatsBase;
    uint16_t EAAL2PadTmpBase;
    uint32_t RasTimerDuration;
    uint32_t RxQDBase_ext;
    uint32_t RxUDCBase;
    uint32_t TxUDCBase;

} ucc_subpage0_config_table_t;


/* UCC - Rx Temp Table 
 *       (see Figure 30-23)
 *
 *         Size: 0x40
 */
typedef struct ucc_Rx_tmp_table_ {
    uint8_t RxCellTmpBase[0x40];    /* Rx cell temporary base address */

} ucc_Rx_tmp_table_t;

 
/* UCC - Tx Temp Table 
 *       (see Figure 30-23)
 *
 *         Size: 0x40
 */
typedef struct ucc_Tx_tmp_table_ {
    uint16_t TxCellTmpBase[0x40];    /* Tx cell temporary base address */

} ucc_Tx_tmp_table_t;


/* UCC - Main UCC ATM Paramter Page structure
 *         (See figure 30-23) 
 *
 *         Size: 0x100 (256) bytes
 */
typedef struct ucc_atm_param_page_ {
    ucc_atm_param_header_t       atm_header;
    ucc_local_page_param_table_t local_page;
    ucc_subpage0_config_table_t  config_table;
    ucc_Rx_tmp_table_t           rx_tmp_table;
    ucc_Tx_tmp_table_t           tx_tmp_table;
} ucc_atm_param_page_t;



/* UCC - Global ATM parameters Table (see Table 30-22)
 *         pointed to by GlobalAtmParamTblPtr in
 *         sub-page0 Configuration table.
 *
 */
typedef struct ucc_global_atm_param_table_ {
    uint16_t IntRctBase;      /* Internal RCT base address */
    uint16_t IntTctBase;      /* Internal TCT base address */
    uint32_t ExtRctBase;      /* Extrnal RCT base address */
    uint32_t ExtTctBase;      /* Extrnal TCT base address */
    uint32_t ExtTcteBase;     /* Extrnal TCTE base address */
    uint16_t IntTcteBase;     /* Internal ACT base address */
    uint16_t CommInfoCtl;     /* The information field associated with the */
    uint16_t Channel_num;     /* the last host command (cpcr). Channel num */
    uint16_t Burst_tol;       /* and burst tolerance (VBR channels). */
    uint32_t CommSusTmp;      /* Reserved. Should be initialized to 0 */ 
    uint16_t CommLkTmp;       /* Reserved. Should be initialized to 0 */ 
    uint16_t AtmAvconBase;    /* ATM Auto VC ON Table Base */
    uint8_t  res[16];         /* Reserved */
} ucc_global_atm_param_table_t;


/* UCC - Address Look-Up Table (see Table 30-30)
 *         pointed to by ADD_COMP_LOOKUP_BASE in
 *         sub-page0 Configuration table.
 *
 */
#ifdef UIKI 
typedef struct address_lookup_table_ {
    uint32_t VptBase;          /* VP-level addressing table base address */
    uint32_t VctBase;          /* VC-level addressing table base address */
    uint32_t Vclt_sf_VpMask;   /* VC level Scale factore (high byte)
                                   VP mask for address compression look-up 
                                   (lower 3-bytes) */
} address_lookup_table_t;
#else
typedef struct address_lookup_table_ {
    uint32_t VptBase;          /* VP-level addressing table base address */
    uint32_t VctBase;          /* VC-level addressing table base address */
    uint32_t Vclt_sf_lvl_mask;             /* VC level Scale factore (high bytes) and */
                                                                 /*  VP mask for address compression look-up */
} address_lookup_table_t;

#endif


/*
 * QMC Global Mutilchannel Parameters
 */

/* SCC QMC defines */
#define ISDN_BRI_QMC_MRBLR          1600
#define ISDN_BRI_GRFTHR             1
#define ISDN_BRI_GRFCNT             1
#define ISDN_QMC_CHANNEL_HDLC_MODE  1   /* bri_usage struct */
#define ISDN_QMC_CHANNEL_TRANS_MODE 0

/* Channel Programming defines */
/* Global Channel Parameter defines */

#define ISDN_QMC_TSA_ENTRY_UNMASK_BITS_2    0x03
#define ISDN_QMC_TSA_ENTRY_UNMASK_BITS_4    0x0f
#define ISDN_QMC_TSA_ENTRY_UNMASK_BITS_6    0x3F

#define QMC_CHANNEL_ONE_BD_SIZE     8
#define QMC_B_CHANNEL_RXBD_NUM      1   /* 16 */
#define QMC_B_CHANNEL_TXBD_NUM      1   /* 4 */
#define QMC_B_CHANNEL_BD_NUM        (QMC_B_CHANNEL_RXBD_NUM + \
                                     QMC_B_CHANNEL_TXBD_NUM)
#define QMC_B_CHANNEL_ALL_BD_SIZE   (QMC_B_CHANNEL_BD_NUM * \
                                     QMC_CHANNEL_ONE_BD_SIZE)

#define QMC_D_CHANNEL_RXBD_NUM      1   /* used to be 2 */
#define QMC_D_CHANNEL_TXBD_NUM      1
#define QMC_D_CHANNEL_BD_NUM        (QMC_D_CHANNEL_RXBD_NUM + \
                                     QMC_D_CHANNEL_TXBD_NUM)
#define QMC_D_CHANNEL_ALL_BD_SIZ    (QMC_D_CHANNEL_BD_NUM * \
                                     QMC_CHANNEL_ONE_BD_SIZE)
#define QMC_CHANNELS_ALL_BD_SIZE    ((QMC_B_CHANNEL_ALL_BD_SIZE * 2) + \
                                     QMC_D_CHANNEL_ALL_BD_SIZE)

/* QMC Global Multi-Channel Parameters */

#define QMC_GLOBL_CHNL_CMASK32      0xDEBB20E3
#define QMC_GLOBL_CHNL_CMASK16      0xF0B8
#define QMC_INIT_STATE_MC           0x8000

/* channel mode register */

#define QMC_CHAMR_HDLC_MODE         0x8000
#define QMC_CHAMR_IDLM              0x2000
#define QMC_CHAMR_ENT               0x1000
#define QMC_CHAMR_POL               0x0100
#define QMC_CHAMR_CRC               0x0080
#define QMC_CHAMR_NOF_7             0x0007
#define QMC_TRANSP_CHAMR_MODE       0x2000
#define QMC_CHAMR_SYNC              0x0400


/*
 * QE QMC Parameter RAM defines
 */
#define QMC_TSAT_TX_ENTRIES         32
#define QMC_TSAT_RX_ENTRIES         32
#define QMC_MAX_CHANNELS            32
#define QMC_UCC_NUM                 3

/* tstate, rstate */


#define QMC_CHANNEL_TSTATE          0x20000000
#define QMC_CHANNEL_RSTATE          0x20000000

#define QMC_CHANNEL_ZISTATE         0x200
#define QMC_CHANNEL_ZDSTATE         0x80FFFFE0
#define QMC_CHANNEL_TRANSP_ZDSTATE  0x18000080
#define QMC_CHANNEL_TRANSP_ZDSTATE1 0x02FF33E0
#define QMC_CHANNEL_TRANSP_ZDSTATE2 0x003FFFE2

#define QMC_CHANNEL_INTR_MASKS              0x3F
#define QMC_CHANNEL_CLEAR_INTR_ENTRY        0x4800
#define QMC_CHANNEL_ENABLE_INT(chan_id)     (chan_id << 5)
#define QMC_GET_CHANID_FROM_INTR_ENTRY(entry)   ((entry & 0x7c0)>>6)

/* QMC SCCE register */

#define QMC_SCCE_IQOV               0x8
#define QMC_SCCE_GINT               0x4
#define QMC_SCCE_GUN                0x2
#define QMC_SCCE_GOV                0x1

/* QMC intr table entry */

#define QMC_INTR_VALID              0x8000
#define QMC_INTR_RX                 0x9
#define QMC_INTR_TX                 0x2
#define QMC_INTR_BSY                0x4
#define QMC_INTR_UN                 0x10
#define QMC_INTR_MRF                0x20
#define QMC_INTR_IDL                0x1000
#define QMC_INTR_NID                0x2000
#define QMC_INTR_WRAP               0x4000

#define QMC_INTR_MISC               0x34 /* MRF, UN, or BSY */

#define QMC_CHANNEL_MAX             32
#define QMC_INTR_TBL_SIZE           64
#define QMC_MAX_INTR_SPIN           10
#define QMC_HDLC_CRC_LENGTH         2  /* 16 bit CRC */

#define QMC_BRI_CHNL_START          0  /* jump  0 for  NO u-code */

#define MAX_BUFFER_SIZE             256 /* an arbitrarily chosen buffer size 
                                         * for Xmit/rec. buffers 
                                         */

/*
 * UCC RX BD status bits defines
 */
#define QE_QMC_RX_BDSTAT_EMPTY      0x8000        /* Empty */
#define QE_QMC_RX_BDSTAT_WRAP       0x2000        /* Wrap */
#define QE_QMC_RX_BDSTAT_INT        0x2000        /* Wrap */
#define QE_QMC_RX_BDSTAT_LAST       0x0800        /* Last */
#define QE_QMC_RX_BDSTAT_FIRST      0x0400        /* First */
#define QE_QMC_RX_BDSTAT_CONT       0x0200        /* Continuous Mode */
#define QE_QMC_RX_BDSTAT_UB         0x0080        /* User Bit */
#define QE_QMC_RX_BDSTAT_LG         0x0020        /* Length Violation */
#define QE_QMC_RX_BDSTAT_NO         0x0010        /* Non-Octal Aligned */
#define QE_QMC_RX_BDSTAT_AB         0x0008        /* RX Abort */
#define QE_QMC_RX_BDSTAT_CR         0x0004        /* CRC Error */

/*
 * UCC TX BD status bits defines
 */
#define QE_QMC_TX_BDSTAT_RDY        0x8000        /* Ready */
#define QE_QMC_TX_BDSTAT_WRAP       0x2000        /* Wrap */
#define QE_QMC_TX_BDSTAT_INT        0x1000        /* Interrupt */
#define QE_QMC_TX_BDSTAT_LAST       0x0800        /* Last */
#define QE_QMC_TX_BDSTAT_TC         0x0400        /* Transmit CRC */
#define QE_QMC_TX_BDSTAT_CM         0x0200        /* Continuous Mode */
#define QE_QMC_TX_BDSTAT_UB         0x0080        /* User Bit */
#define QE_QMC_TX_BDSTAT_PAD        0x000F        /* Padding Bits */


typedef struct qmc_tsat {
    volatile uint16_t     valid:1;
    volatile uint16_t     wrap:1;
    volatile uint16_t     mask1:2;
    volatile uint16_t     channel_ptr:6;
    volatile uint16_t     mask2:6;
} qmc_tsat_t;


typedef struct qe_qmc_ucc {
    volatile uint32_t     mcbase;
    volatile uint16_t     qmcstate;
    volatile uint16_t     mrblr;
    volatile uint16_t     tx_s_ptr;
    volatile uint16_t     rxptr;
    volatile uint16_t     grfthr;
    volatile uint16_t     grfcnt;
    volatile uint32_t     intbase;
    volatile uint32_t     intptr;
    volatile uint16_t     rx_s_ptr;
    volatile uint16_t     txptr;
    volatile uint32_t     c_mask32;
    volatile qmc_tsat_t   tsatrx[QMC_TSAT_RX_ENTRIES];
    volatile qmc_tsat_t   tsattx[QMC_TSAT_TX_ENTRIES];
    volatile uint16_t     c_mask16;
    volatile uint32_t     temp_rba;
    volatile uint32_t     temp_crc;
    volatile uint16_t     rx_frm_base;
    volatile uint16_t     tx_frm_base;
    volatile uint8_t      res[20];
    volatile uint32_t     qmc_glb_ch_sp_base;
} qe_qmc_ucc_t;

/*
 * QMC Channel-Specific HDLC Parameters
 */
typedef struct qe_ch_sp_hdlc {
    volatile uint16_t     tbase;
    volatile uint16_t     chamr;
    volatile uint32_t     tstate;
    volatile uint32_t     res1;
    volatile uint16_t     tbptr;
    volatile uint16_t     res2;
    volatile uint32_t     tupack;
    volatile uint32_t     zistate;
    volatile uint32_t     tcrc;
    volatile uint16_t     intmsk;
    volatile uint16_t     bdflags;
    volatile uint16_t     rbase;
    volatile uint16_t     mflr;
    volatile uint32_t     rstate;
    volatile uint32_t     res3;
    volatile uint16_t     rbptr;
    volatile uint16_t     res4;
    volatile uint32_t     rpack;
    volatile uint32_t     zdstate;
    volatile uint32_t     rcrc;
    volatile uint16_t     max_cnt;
    volatile uint16_t     tmp_mb;
} qe_ch_sp_hdlc_t;

typedef struct qe_qmc_param {
    qe_ch_sp_hdlc_t       channel[QMC_MAX_CHANNELS];
    qe_qmc_ucc_t          ucc[QMC_UCC_NUM];
} qe_qmc_param_t;


/*
 * ATM General Defines
 */

#define ATM_LPBK        1

#ifndef FCC_ATM_H
#define FCC_ATM_H

#define PORTA           0
#define PORTB           1
#define PORTC           2
#define PORTD           3

/*-----------------------------------------------------------------------*/
/*                     8260 CONSTANTS AND DEFINITIONS                    */
/*-----------------------------------------------------------------------*/

#define BASE_EVT        0x0  /* Base Address of Exception Vector Table */

#define EXT_INT_VECTOR ((BASE_EVT) + 0x500)  /* Base address of 
                                                external interrupt 
                                                code               */

#define NEXT_VECTOR (EXT_INT_VECTOR + 0x100)

/*-----------------------------------*/
/* CPM Command Register definitions. */
/*-----------------------------------*/
#define CR_FLG          0x00010000   /* Command Register "flag" bit */ 
#define CECR_ATM        0x0A         /* ATM Protocol specification  */ 
#define INIT_TX_RX      0x00000000   /* "Initialize Rx/Tx param." command */ 
#define ATM_TRANSMIT    0x0000000A   /* "Activate ATM Transmit" command   */ 
#define ATM_TX_STOP     0x00000004

/*-----------------------------------------------*/
/* General FCC Mode Register (GFMR) definitions. */
/*-----------------------------------------------*/
#define GFMR_ENT        0x00000010  /* Enable Transmitter */ 
#define GFMR_ENR        0x00000020  /* Enable Receiver    */ 
#define GFMR_ATM        0x0000000A  /* ATM protocol def.  */ 
#define GFMR_DIAG_LPB   0x40000000  /* Enable Receiver    */ 

/*-------------------------------------------------*/
/* SIU Interrupt Mask Register (SIMR) definitions. */ 
/*-------------------------------------------------*/
#define SIMR_FCC1       0x80000000  /* FCC1 Mask bit */ 


/*
 * Serial Interface register macros
 */

/*
 * si_siram - SI RAM Word
 */
#define QE_SIRAM_ENTRIES          256     /* */
#define QE_SIRAM_CONVERT(x) (x)

#define QE_SIRAM_LOOP             0x8000  /* Loop Back on This Times lot */
#define QE_SIRAM_SWTR             0x4000  /* Switch Tx & Rx */
#define QE_SIRAM_SSEL1            0x2000  /* Strobe Select 1 */
#define QE_SIRAM_SSEL2            0x1000  /* Strobe Select 2 */
#define QE_SIRAM_SSEL3            0x0800  /* Strobe Select 3 */
#define QE_SIRAM_SSEL4            0x0400  /* Strobe Select 4 */
//#define QE_SIRAM_CSEL(y)    (((y)&7)<<5)/* Channel Select */
#define QE_SIRAM_CSEL(y)          (((y) & 0xf) << 5) /* Channel Select */
#define QE_SIRAM_CNT(y)           (((y)& 0xf) << 2)  /* Count */
#define QE_SIRAM_BYT              0x0002  /* Byte Resolution */
#define QE_SIRAM_BIT              0x0000  /* Bit Resolution */
#define QE_SIRAM_LST              0x0001  /* Last Entry in SIRAM */


/*
 * si_simode -  SI Mode Reg
 */
#define QE_SIMODE_SDM(y)   (((y)&3)<<10)   /* SI Diag Mode TDM X */
#define QE_SIMODE_RFSD(y)  (((y)&3)<<8)    /* Rx Frame Sync Delay TDM X */
#define QE_SIMODE_DSC      0x00000080      /* Dbl Speed Clock TDM X */
#define QE_SIMODE_CRT      0x0040          /* Common Rx & Tx Pins TDM X */
#define QE_SIMODE_TFSD(y)  ((y)&3)         /* Tx Frame Sync Delay TDM X */
#define QE_SIMODE_GM       0x0004          /* Grant Mode in SIRAM */
#define QE_SIMODE_CE       0x0010          /* Clock edge for TDM data */
#define QE_SIMODE_FE       0x0008          /* Frame sync edge for TDM */

#define TDM_LOOPBK             2

/*
 * si_sigmr - SI Global Mode Reg
 */
#define QE_SIGMR_ENA           0x01            /* Enable TDM A */
#define QE_SIGMR_ENB           0x02            /* Enable TDM B */
#define QE_SIGMR_ENC           0x04            /* Enable TDM C */
#define QE_SIGMR_END           0x08            /* Enable TDM D */

/* BRI Channels - Channels numbers & Time Slot IDs */
#define BRI_B1_TSA_SLOT_ID     0
#define BRI_B2_TSA_SLOT_ID     1
#define BRI_D_TSA_SLOT_ID      2

#define BRI_B1_CHANNEL_ID      0
#define BRI_B2_CHANNEL_ID      1
#define BRI_MON0_CHANNEL_ID    2
#define BRI_D_CHANNEL_ID       3

/*
 * TOTAL_CHANNELS indicates how many timeslots (channels)
 * UCC will be sending to SI
 */
#define TOTAL_CHANNELS         4

#define UCC_LOOPB              1
#define TDM_LOOPBK             2


/*
 * UCC UEC RX BD status bits defines
 */
#define QE_UEC_RX_BDSTAT_EMPTY        0x8000     /* Empty */
#define QE_UEC_RX_BDSTAT_WRAP         0x2000     /* Wrap */
#define QE_UEC_RX_BDSTAT_INT          0x1000     /* Interrupt */
#define QE_UEC_RX_BDSTAT_LAST         0x0800     /* Last */
#define QE_UEC_RX_BDSTAT_FIRST        0x0400     /* First */
#define QE_UEC_RX_BDSTAT_CMR          0x0200     /* CAM Match Result */
#define QE_UEC_RX_BDSTAT_MISS         0x0100     /* MISS or Promiscuous */
#define QE_UEC_RX_BDSTAT_BC           0x0080     /* Broadcast */
#define QE_UEC_RX_BDSTAT_MC           0x0040     /* Multicast */
#define QE_UEC_RX_BDSTAT_LG           0x0020     /* Length Violation */
#define QE_UEC_RX_BDSTAT_NO           0x0010     /* Non-Octal Aligned */
#define QE_UEC_RX_BDSTAT_SH           0x0008     /* Short Frame */
#define QE_UEC_RX_BDSTAT_CR           0x0004     /* CRC Error */
#define QE_UEC_RX_BDSTAT_OV           0x0002     /* Overrun Error */
#define QE_UEC_RX_BDSTAT_IPCH         0x0001     /* IPCH Status */

#define QE_UEC_RX_BDSTAT_LG_MESS      0
#define QE_UEC_RX_BDSTAT_NO_MESS      1
#define QE_UEC_RX_BDSTAT_SH_MESS      2
#define QE_UEC_RX_BDSTAT_CR_MESS      3
#define QE_UEC_RX_BDSTAT_OV_MESS      4
#define QE_UEC_RX_BDSTAT_IPCH_MESS    5

/*
 * UCC UEC TX BD status bits defines
 */
#define QE_UEC_TX_BDSTAT_RDY          0x8000     /* Ready */
#define QE_UEC_TX_BDSTAT_PAD          0x4000     /* Short Frame Padding */
#define QE_UEC_TX_BDSTAT_WRAP         0x2000     /* Wrap */
#define QE_UEC_TX_BDSTAT_INT          0x1000     /* Interrupt */
#define QE_UEC_TX_BDSTAT_LAST         0x0800     /* Last */
#define QE_UEC_TX_BDSTAT_TRCRC        0x0400     /* Transmit CRC */
#define QE_UEC_TX_BDSTAT_DEF          0x0200     /* Defer Frame */
#define QE_UEC_TX_BDSTAT_PP_EXDEF     0x0100     /* Programmable Preamble
                                                    or Ex. Defer Frame */
#define QE_UEC_TX_BDSTAT_LC           0x0080     /* Late Collision */
#define QE_UEC_TX_BDSTAT_IPCH0_RL     0x0040     /* IPH0 Offset or 
                                                    Tranmission Failure */
#define QE_UEC_TX_BDSTAT_RC_VID       0x003C     /* Retry Count or 
                                                    VLAN Tag */
#define QE_UEC_TX_BDSTAT_RC_VID_SHIFT 0x0002     /* Retry Count or 
                                                    VLAN Tag Shift */
#define QE_UEC_TX_BDSTAT_IPCH1_UN     0x0002     /* IPH1 Offset or 
                                                    Underrun Failure */
#define QE_UEC_TX_BDSTAT_IPCH2_CSL    0x0001     /* IPH2 Offset or 
                                                    Carrier Sense Loss */

#define QE_UEC_TX_BDSTAT_LC_MESS      0
#define QE_UEC_TX_BDSTAT_IPCH0_RL_MESS 1
#define QE_UEC_TX_BDSTAT_IPCH1_UN_MESS 2
#define QE_UEC_TX_BDSTAT_IPCH2_CSL_MESS 3


/*
 * QE Ethernet Parameter RAM structure
 */
typedef struct qe_ethernet_param {
    TxGblPram_t           tx_glb_pram;
    RxGblPram_t           rx_glb_pram;
    TxThreadParam_t       tx_thread_pram[TX_THREAD];
    TxThreadData_t        tx_thread_data[TX_THREAD];
    TxSendQD_t            tx_bd_q[MAX_TX_QUEUE];
    TxSchedular_t         tx_sched;
    TxFirmwareCounter_t   tx_statistics;
    RxThreadParam_t       rx_thread_pram[RX_THREAD];
    RxThreadData_t        rx_thread_data[RX_THREAD];
    RxFirmwareCounter_t   rx_statistics;
    RxIntCoalescingTbl_t  rx_int_coalesc[MAX_RX_INT_COALESCING_CTR];
    RxBDQueueTable_t      rx_bd_q[MAX_RX_QUEUE];
    InitEnet_t            init_rx_tx;
} qe_ethernet_param_t;

typedef struct qe_fast_hdlc_param {
    uint16_t    riptr;                        /* Rx BD Base Address */
    uint16_t    tiptr;                        /* Tx BD Base Address */
    uint16_t    fasthdlc_res_0;
    uint16_t    mrblr;                        /* Rx Buffer Length */
    uint32_t    rstate;                        /* Rx Internal State */
    uint32_t    rbase;                        /* RxBD Base Address */
    uint16_t    rbdstat;                /* RxBD status and control */
    uint16_t    rbdlen;                        /* RxBD data length */
    uint32_t    rdptr;                        /* RxBD data pointer */
    uint32_t    tstate;                        /* Tx Internal State */
    uint32_t    tbase;                        /* TxBD Base Address */
    uint16_t    tbdstat;                /* TxBD status and control */
    uint16_t    tbdlen;                        /* TxBD data length */
    uint32_t    tdptr;                        /* TxBD data pointer */
    uint32_t    rbptr;                  /* RxBD pointer */
    uint32_t    tbptr;                  /* TxBD pointer */
    uint32_t    rcrc;                   /* Temporary receive CRC */
    uint16_t    fasthdlc_res_1[0x2];
    uint32_t    tcrc;                   /* Temporary transmit CRC */
    uint16_t    fasthdlc_res_2[0x4];
    uint32_t    c_mask;                 /* CRC constant */
    uint32_t    c_pres;                 /* CRC preset */
    uint16_t    disfc;                  /* Discard frame counter */
    uint16_t    crcec;                  /* CRC error counter */
    uint16_t    abtsc;                  /* Abort sequence counter */
    uint16_t    nmarc;                  /* Non-matching address Rx counter */
    uint32_t    max_cnt;                /* Max_length counter */
    uint16_t    mflr;                   /* Max frame length register */
    uint16_t    rfthr;                  /* Received frames threshold */
    uint16_t    rfcnt;                  /* Received frames count */
    uint16_t    hmask;                  /* */
    uint16_t    haddr1;
    uint16_t    haddr2;
    uint16_t    haddr3;
    uint16_t    haddr4;
    uint16_t    ts_tmp;                 /* Temporary storage */
    uint16_t    tmp_mb;                 /* Temporary storage */
} qe_fast_hdlc_param_t;


#define QE_HDLC_RX_BDSTAT_E          0x8000
#define QE_HDLC_RX_BDSTAT_W          0x2000
#define QE_HDLC_RX_BDSTAT_I          0x1000
#define QE_HDLC_RX_BDSTAT_L          0x0800
#define QE_HDLC_RX_BDSTAT_F          0x0400
#define QE_HDLC_RX_BDSTAT_CM         0x0200
#define QE_HDLC_RX_BDSTAT_LG         0x0020
#define QE_HDLC_RX_BDSTAT_NO         0x0010
#define QE_HDLC_RX_BDSTAT_AB         0x0008
#define QE_HDLC_RX_BDSTAT_CR         0x0004
#define QE_HDLC_RX_BDSTAT_OV         0x0002
#define QE_HDLC_RX_BDSTAT_CD         0x0001

#define QE_HDLC_TX_BDSTAT_R          0x8000
#define QE_HDLC_TX_BDSTAT_W          0x2000
#define QE_HDLC_TX_BDSTAT_I          0x1000
#define QE_HDLC_TX_BDSTAT_L          0x0800
#define QE_HDLC_TX_BDSTAT_TC         0x0400
#define QE_HDLC_TX_BDSTAT_CM         0x0200
#define QE_HDLC_TX_BDSTAT_UN         0x0002
#define QE_HDLC_TX_BDSTAT_CT         0x0001

typedef struct qe_smc_gci_param {
    uint16_t    m_rxbd;                 /* Monitor Channel Rx BD */
    uint16_t    m_txbd;                 /* Monitor Channel Tx BD */
    uint16_t    ci_rxbd;                /* C/I Channel Rx BD */
    uint16_t    ci_txbd;                /* C/I Channel Tx BD */
    uint32_t    rstate;                 /* Rx & Tx Internal State */
    uint16_t    m_rxd;                  /* Monitor Rx Data */
    uint16_t    m_txd;                  /* Monitor Tx Data */
    uint16_t    ci_rxd;                 /* C/I Rx Data */
    uint16_t    ci_txd;                 /* C/I Tx Data */
} qe_smc_gci_param_t;

typedef struct qe_misc_param {
    uint16_t    rev_num;            /* Microcode Revision Number */
    uint8_t     misc_res_0[0xA];        /* Reserved */
    uint8_t     misc_pad[0x4];                /* Misc Param Overall Size = 0x10 */
} qe_misc_param_t;

typedef struct qe_spi_param {
    uint16_t    rbase;                        /* Rx BD Base Address */
    uint16_t    tbase;                        /* Tx BD Base Address */
    uint8_t     rfcr;                        /* Rx Function Code */
    uint8_t     tfcr;                        /* Tx Function Code */
    uint16_t    mrblr;                        /* Rx Buffer Length */
    uint32_t    rstate;                        /* Rx Internal State */
    uint32_t    rptr;                        /* Rx Internal Data Pointer */
    uint16_t    rbptr;                        /* Rx BD Pointer */
    uint16_t    rcount;                        /* Rx Internal Byte Count */
    uint32_t    rtemp;                        /* Rx Temp */
    uint32_t    tstate;                        /* Tx Internal State */
    uint32_t    tptr;                        /* Tx Internal Data Pointer */
    uint16_t    tbptr;                        /* Tx BD Pointer */
    uint16_t    tcount;                        /* Tx Byte Count */
    uint32_t    ttemp;                        /* Tx Temp */
    uint8_t     pad[8];         /* spi param overall size = 0x30 */
} qe_spi_param_t;

typedef struct qe_timer_param {
    uint16_t    tm_base;                /* RISC Timer Table Base Address */
    uint16_t    tm_ptr;                        /* RISC Timer Table Pointer */
    uint16_t    r_tmr;                        /* RISC Timer Mode Register */
    uint16_t    r_tmv;                        /* RISC Timer Valid Register */
    uint32_t    tm_cmd;                        /* RISC Timer Command Register */
    uint32_t    tm_cnt;                        /* RISC Timer Internal Count */
} qe_timer_param_t;

typedef struct qe_idma_param {
    uint16_t    ibase;                  /* IDMA BD Base Address */
    uint16_t    ibptr;                  /* IDMA BD Pointer */
    uint32_t    istate;                 /* IDMA Internal State */
    uint32_t    itemp;                  /* IDMA Temp */
    uint8_t     idma_pad[0x4];                /* IDMA Overall Size = 0x10 */
} qe_idma_param_t;

/*
 * Function Code Register - The following macros & constants are used to
 *   manipulate the fields of the function code register, used to describe the
 *   processor state during an external bus transaction.
 */
#define QE_FCR_BO_INTEL_LE   (0x0 << 3)       /* DEC/Intel little-endian */
#define QE_FCR_BO_PPC_LE     (0x1 << 3)       /* PowerPC little-endian */
#define QE_FCR_BO_MOT        (0x2 << 3)       /* big-endian */
#define QE_FCR_AT(y)         ((y) & 7)        /* Address Type */

/*
 * For QE 3, FCRs are different.
 */
#define QE_FCR_GBL           0x20           /* Global access bit */
#define QE_FCR_BO            0x10           /* Big-endian byte ordering */
#define QE_FCR_DTB           0x02           /* Local bus for SDMA */




/**************************************************************************
 **************************************************************************
 * QUICC Engine (QE) Registers        (0x100000 - 0x1FFFFF)
 * Note: This is a template of whole QE memory space
 * 	 This can be used as reference when definig QE data structure
 *	 (ie. ccsr_qe_t) in the cpu ccsr data structure header file
 **************************************************************************
 **************************************************************************/

#define risc             cp
#define si1              si
#define si1rt            sirt

typedef struct {
    qe_iram_t        iram;                  /* Instruction RAM */
    qe_irq_t         irq;                   /* Interrupt controller */
    qe_cp_t          cp;                    /* Communication Processor */
    SPC(a, 0x200)
    qe_mux_t         mux;                   /* QE Multiplexer */
    qe_timer_t       timer;                 /* QE Timers */
    SPC(b, 0x40)
    qe_spi_t         spi1;                  /* SPI 1 */
    qe_spi_t         spi2;                  /* SPI 2 */

#if defined(MPC836x)
    qe_mcc_t         mcc;                   /* MCC */
    SPC(c, 0xC0)
#elif defined(MPC832x)
    SPC(c, 0x100)
#endif

    qe_brg_t         brg;                   /* Baud rate generator */
    qe_usb_t         usb;                   /* USB 1.0 */
    qe_si_t          si;                    /* SI */
    SPC(e, 0x880)
    qe_sirt_t        sirt;                  /* SI routing table */
    SPC(f, 0x800)
    qe_ucc_t         ucc1;                  /* UCC1 */
    qe_ucc_t         ucc3;                  /* UCC3 */
    qe_ucc_t         ucc5;                  /* UCC5 */

#if defined(MPC836x)
    qe_ucc_t         ucc7;                  /* UCC7 */
    SPC(g, 0x600)
#elif defined(MPC832x)
    SPC(g, 0x800)
#endif

    qe_utopi_t       utopia;                /* Multi-PHY controller */
    qe_ucc_t         ucc2;                  /* UCC2 */
    qe_ucc_t         ucc4;                  /* UCC4 */

#if defined(MPC836x)
    qe_ucc_t         ucc6;                  /* UCC6 */
    qe_ucc_t         ucc8;                  /* UCC8 */
    SPC(h, 0x800)
#elif defined(MPC832x)
    SPC(h, 0xC00)
#endif

    qe_sdma_t        sdma;                  /* Serial DMA */
    qe_debug_t       debug;                 /* Debug breakpoint */
    qe_risc_t        risc1_spcl;            /* RISC1 special */

#if defined(MPC836x)
    qe_risc_t        risc2_spcl;            /* RISC2 special */
    SPC(i, 0x200)
#elif defined(MPC832x)
    SPC(i, 0x300)
#endif

    qe_test_t        test;                 /* Test (0x04500 - 0x045FF) */
    qe_trace_t       risc1_trbr;           /* RISC1 trace buffer */
    SPC(j, 0xB980)
    qe_muram_template_t muram;                /* Multi-user RAM */
    SPC(k, 0xE4000)
} _PackedType t_QEmap, mpc_qe_t;




#endif /*__QE__*/

/* -------------------- End of File ------------------- */

/*------------------------------------------------------------------------------
 * $Log: qe.h,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.3  2011/10/07 01:11:46  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:26  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.4  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.3  2011/07/19 06:11:35  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.2  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.1  2011/05/02 23:33:23  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

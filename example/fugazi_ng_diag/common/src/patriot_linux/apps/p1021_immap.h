/* $Id: p1021_immap.h,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *------------------------------------------------------------------
 * Filename: p1021_immap.h
 *
 * Description:  Register definition for the 1MB pointed by CCSRBAR in
 *               P1021 (QorIQ).
 *               Originally provided by Motorola.
 *
 * Author: Xianghua Xiao (x.xiao@motorola.com)
 * 
 * Create Date: March 2010.
 *
 * Copyright (c) 2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __MPC8500_IMMAP__
#define __MPC8500_IMMAP__

#include "qe.h"

#define MPC8500_PCIE_ID           0x01031957
#define MPC8500E_PCIE_ID          0x01021957

#define _PackedType        __attribute__ ((packed))
#define SPC(_x, _n)        volatile uint8_t __res_##_x[_n];

/* Local-Access Registers and ECM Registers(0x0000-0x1fff) */

typedef struct ccsr_local_ecm {
    uint    ccsrbar;    /* 0x0 - Control Configuration Status Registers */
                        /*       Base Address Register */
    char    res1[4];
    uint    altcbar;    /* 0x8 - Alternate Configuration Base Address */
                        /*       Register */
    char    res2[4];
    uint    altcar;     /* 0x10 - Alternate Configuration Attribute Register */
    char    res3[12];
    uint    bptr;       /* 0x20 - Boot Page Translation Register */
    char    res4[3044];
    uint    lawbar0;    /* 0xc08 - Local Access Window 0 Base Address */
                        /*         Register */
    char    res5[4];
    uint    lawar0;     /* 0xc10 - Local Access Window 0 Attributes Register */
    char    res6[20];
    uint    lawbar1;    /* 0xc28 - Local Access Window 1 Base Address */
                        /*         Register */
    char    res7[4];
    uint    lawar1;     /* 0xc30 - Local Access Window 1 Attributes Register */
    char    res8[20];
    uint    lawbar2;    /* 0xc48 - Local Access Window 2 Base Address */
                        /*         Register */
    char    res9[4];
    uint    lawar2;     /* 0xc50 - Local Access Window 2 Attributes Register */
    char    res10[20];
    uint    lawbar3;    /* 0xc68 - Local Access Window 3 Base Address */
                        /*         Register */
    char    res11[4];
    uint    lawar3;     /* 0xc70 - Local Access Window 3 Attributes Register */
    char    res12[20];
    uint    lawbar4;    /* 0xc88 - Local Access Window 4 Base Address */
                        /*         Register */
    char    res13[4];
    uint    lawar4;     /* 0xc90 - Local Access Window 4 Attributes Register */
    char    res14[20];
    uint    lawbar5;    /* 0xca8 - Local Access Window 5 Base Address */
                        /*         Register */
    char    res15[4];
    uint    lawar5;     /* 0xcb0 - Local Access Window 5 Attributes Register */
    char    res16[20];
    uint    lawbar6;    /* 0xcc8 - Local Access Window 6 Base Address */
                        /*         Register */
    char    res17[4];
    uint    lawar6;     /* 0xcd0 - Local Access Window 6 Attributes Register */
    char    res18[20];
    uint    lawbar7;    /* 0xce8 - Local Access Window 7 Base Address */
                        /*         Register */
    char    res19[4];
    uint    lawar7;     /* 0xcf0 - Local Access Window 7 Attributes Register */

    char    res17_1[20];
    uint    lawbar8;    /* 0xd08 - Local Access Window 8 Base Address */
                        /*         Register */
    char    res18_1[4];
    uint    lawar8;     /* 0xd10 - Local Access Window 8 Attributes Register */
    char    res17_2[20];
    uint    lawbar9;    /* 0xd28 - Local Access Window 9 Base Address */
                        /*         Register */
    char    res18_2[4];
    uint    lawar9;     /* 0xd30 - Local Access Window 9 Attributes Register */
    char    res19_2[716];

    uint    eebacr;     /* 0x1000 - ECM CCB Address Configuration Register */
    char    res20[12];
    uint    eebpcr;     /* 0x1010 - ECM CCB Port Configuration Register */
    char    res21[3044];
    uint    ecmiprev1;  /* 0x1bf8 - ECM IP Block Revision Register 1 */
    uint    ecmiprev2;  /* 0x1bfc - ECM IP Block Revision Register 2 */
    char    res22[512];
    uint    eedr;       /* 0x1e00 - ECM Error Detect Register */
    char    res23[4];
    uint    eeer;       /* 0x1e08 - ECM Error Enable Register */
    uint    eeatr;      /* 0x1e0c - ECM Error Attributes Capture Register */
    uint    eeadr;      /* 0x1e10 - ECM Error Address Capture Register */
    uint    eehadr;     /* 0x1e14 - ECM Error High Address Capture Register */
    char    res24[488];
} ccsr_local_ecm_t;

#define MPC8500_CCBPC_C0_PRI_LOW        0x00000000      /* core 0 has lowest ccb bus pri */
#define MPC8500_CCBPC_C0_PRI_MED        0x00000001      /* core 0 has medium ccb bus pri */
#define MPC8500_CCBPC_C0_PRI_HI         0x00000002      /* core 0 has highest ccb bus pri */
#define MPC8500_CCBPC_CCB_PRI_MSK       0xFFFFFFFC      /* core 0 ccb bus pri mask */

#define MPC8500_EEBPCR_CORE1_EN         0x2000000       /* enable core 1 */


/* DDR memory controller registers(0x2000-0x2fff) */

typedef struct ccsr_ddr {
    uint    cs0_bnds;   /* 0x2000 - DDR Chip Select 0 Memory Bounds */
    char    res1[4];
    uint    cs1_bnds;   /* 0x2008 - DDR Chip Select 1 Memory Bounds */
    char    res2[4];
    uint    cs2_bnds;   /* 0x2010 - DDR Chip Select 2 Memory Bounds */
    char    res3[108];
    uint    cs0_config; /* 0x2080 - DDR Chip Select 0 Configuration */
    uint    cs1_config; /* 0x2084 - DDR Chip Select 1 Configuration */
    char    res4[56];
    uint    cs0_config_2;   /* 0x20c0 - DDR Chip Select 0 Configuration 2 */
    uint    cs1_config_2;   /* 0x20c4 - DDR Chip Select 1 Configuration 2 */
    char    res5[56];
    uint    timing_cfg_3;   /* 0x2100 - DDR SDRAM Timing Configuration */
                            /*        Register 3 */
    uint    timing_cfg_0;   /* 0x2104 - DDR SDRAM Timing Configuration */
                            /*        Register 0 */
    uint    timing_cfg_1;   /* 0x2108 - DDR SDRAM Timing Configuration */
                            /*        Register 1 */
    uint    timing_cfg_2;   /* 0x210c - DDR SDRAM Timing Configuration */
                            /*          Register 2 */
    uint    sdram_cfg;      /* 0x2110 - DDR SDRAM Control Configuration */
    uint    sdram_cfg_2;    /* 0x2114 - DDR SDRAM Control Configuration 2 */
    uint    sdram_mode;     /* 0x2118 - DDR SDRAM Mode Configuration */
    uint    sdram_mode_2;   /* 0x211c - DDR SDRAM Mode Configuration 2 */
    uint    sdram_mode_cntl;  /* 0x2120 - DDR SDRAM mode control */
    uint    sdram_interval;   /* 0x2124 - DDR SDRAM interval configuration */
    uint    sdram_data_init;  /* 0x2128 - DDR SDRAM data initialization */
    char    res6[4];
    uint    sdram_clk_cntl; /* 0x2130 - DDR SDRAM clock control */
    char    res7[12];
    uint    sdram_ocd_cntl;        /* 0x2140 - DDR SDRAM OCD Control */
    uint    sdram_ocd_status;    /* 0x2144 - DDR SDRAM OCD Status */
    uint    init_addr;          /* 0x2148 - DDR training initialzation address */
    uint    init_ext_addr;      /* 0x214C - DDR training initialzation extended address */
    char    res8[16];
    uint    timing_cfg_4;   /* 0x2160 - DDR SDRAM Timing Configuration */
                            /*          Register 4 */
    uint    timing_cfg_5;   /* 0x2164 - DDR SDRAM Timing Configuration */
                            /*          Register 5 */
    char    res9[8];
    uint    zq_cntl;        /* 0x2170 - DDR ZQ calibration control */
    uint    wrlvl_cntrl_0;  /* 0x2174 - DDR write leveling control */
    char    res10[24];
    uint    wrlvl_cntrl_1;  /* 0x2190 - DDR write leveling control 1 */
    uint    wrlvl_cntrl_2;  /* 0x2194 - DDR write leveling control 2 */
    char    res11[2440];
    uint    ddrdsr_1;       /* 0x2b20 - DDR Debug Status Register 1 */
    uint    ddrdsr_2;       /* 0x2b24 - DDR Debug Status Register 2 */
    uint    ddrcdr_1;       /* 0x2b28 - DDR Control Driver Register 1 */
    uint    ddrcdr_2;       /* 0x2b2c - DDR Control Driver Register 2 */
    char    res12[200];
    uint    ip_rev1;        /* 0x2BF8 - DDR IP Block Revision 1 */
    uint    ip_rev2;        /* 0x2BFC - DDR IP Block Revision 2 */
    char    res13[512];
    uint    data_err_inject_hi; /* 0x2e00 - DDR Memory Data Path Error */
                                /*          Injection Mask High */
    uint    data_err_inject_lo; /* 0x2e04 - DDR Memory Data Path Error */
                                /*          Injection Mask Low */
    uint    ecc_err_inject;     /* 0x2e08 - DDR Memory Data Path Error */
                                /*          Injection Mask ECC */
    char    res14[20];
    uint    capture_data_hi;    /* 0x2e20 - DDR Memory Data Path Read */
                                /*          Capture High */
    uint    capture_data_lo;    /* 0x2e24 - DDR Memory Data Path Read */
                                /*         Capture Low */
    uint    capture_ecc;        /* 0x2e28 - DDR Memory Data Path Read */
                                /*        Capture ECC */
    char    res15[20];
    uint    err_detect;         /* 0x2e40 - DDR Memory Error Detect */
    uint    err_disable;        /* 0x2e44 - DDR Memory Error Disable */
    uint    err_int_en;         /* 0x2e48 - DDR  */
    uint    capture_attributes; /* 0x2e4c - DDR Memory Error Attributes */
                                /*          Capture */
    uint    capture_address;    /* 0x2e50 - DDR Memory Error Address Capture */
    uint    capture_ext_address;    /* 0x2e54 - DDR Memory Error */
                                    /*          Extended Address Capture */
    uint    err_sbe;            /* 0x2e58 - DDR Memory Single-Bit ECC Error */
                                /*          Management */
    char    res16[164];
    uint    debug_1;            /* 0x2f00 */
    uint    debug_2;
    uint    debug_3;
    uint    debug_4;
    char    res17[240];
} ccsr_ddr_t;

#define MPC8500_CS_N_EN         0x80000000      /* chip select enable */
#define MPC8500_AP_N_EN         0x00800000      /* auto precharge enable */

#define MPC8500_CS_BNDS_SA      0x0FFF0000      /* SA in CS memory bounds */
#define MPC8500_CS_BNDS_EA      0x00000FFF      /* EA in CS memory bounds */ 
#define MPC8500_CS_BNDS_SA_SHIFT (16)

#define MPC8500_MEM_EN          0x80000000      /* enable DDR memory */
#define MPC8500_DDR_SDRAM       0x02000000      /* type -> DDR SDRAM */

#define MPC8500_DDR_SBED        0x00000004      /* DDR single-bit ecc error disable */
#define MPC8500_DDR_MSED        0x00000001      /* DDR multiple-bit ecc error disable */

#define MPC8500_DDR_MSEE        0x00000001      /* DDR memory select error interrupt */
#define MPC8500_DDR_SBEE        0x00000004      /* DDR single-bit ecc error interrupt */
#define MPC8500_DDR_MBEE        0x00000008      /* DDR multiple-bit ecc error interrupt */
#define MPC8500_DDR_ACEE        0x00000080      /* DDR automatic calibration error interrupt */
#define MPC8500_DDR_APEE        0x00000100      /* DDR address parity error interrupt */

#define MPC8500_DDR_SBET_MASK   0x00FF0000      /* DDR error_sbe SBET bits mask */
#define MPC8500_MEM_ERR_INJ_EN  0x00000100      /* MEM error error injection enable bit */
#define MPC8500_MEM_ERR_MME     0x80000000      /* MEM error detect bit MME */
#define MPC8500_MEM_ERR_APE     0x00000100      /* MEM error detect bit APE */
#define MPC8500_MEM_ERR_ACE     0x00000080      /* MEM error detect bit ACE */
#define MPC8500_MEM_ERR_MBE     0x00000008      /* MEM error detect bit MBE */
#define MPC8500_MEM_ERR_SBE     0x00000004      /* MEM error detect bit SBE */
#define MPC8500_MEM_ERR_MSE     0x00000001      /* MEM error detect bit MSE */


/* I2C Registers(0x3000-0x3fff) */

typedef struct ccsr_i2c {
    u_char  i2cadr;     /* 0x3000 - I2C Address Register */
    char    res1[3];
    u_char  i2cfdr;     /* 0x3004 - I2C Frequency Divider Register */
    char    res2[3];
    u_char  i2ccr;      /* 0x3008 - I2C Control Register */
    char    res3[3];
    u_char  i2csr;      /* 0x300c - I2C Status Register */
    char    res4[3];
    u_char  i2cdr;      /* 0x3010 - I2C Data Register */
    char    res5[3];
    u_char  i2cdfsrr;   /* 0x3014 - I2C Digital Filtering Sampling Rate */
                        /*          Register */
    char    res6[235];  /* 0x3015 - 0x30ff */
} ccsr_i2c_t;

/* I2C Address Register defines (i2cadr) */
#define MPC8500_I2CADR_MASK     0xFE

/* I2C Frequency Divider Register defines (i2cfdr) */
#define MPC8500_I2CFDR_MASK     0x3F

/* I2C Control Register defines (i2ccr) */
#define MPC8500_I2CCR_MEN       0x80
#define MPC8500_I2CCR_MIEN      0x40
#define MPC8500_I2CCR_MSTA      0x20
#define MPC8500_I2CCR_MTX       0x10
#define MPC8500_I2CCR_TXAK      0x08
#define MPC8500_I2CCR_RSTA      0x04
#define MPC8500_I2CCR_BCST      0x01

/* I2C Status Register defines (i2csr) */
#define MPC8500_I2CSR_MCF       0x80
#define MPC8500_I2CSR_MAAS      0x40
#define MPC8500_I2CSR_MBB       0x20
#define MPC8500_I2CSR_MAL       0x10
#define MPC8500_I2CSR_BCSTM     0x08
#define MPC8500_I2CSR_SRW       0x04
#define MPC8500_I2CSR_MIF       0x02
#define MPC8500_I2CSR_RXAK      0x01

/* I2C Data Register defines (i2cdr) */
#define MPC8500_I2CDR_DATA      0xFF
#define I2C_WRITE_COMMAND       0x00
#define I2C_READ_COMMAND        0x01

/* I2C Digital Filtering Sampling Rate Register defines (i2cdfsrr) */
#define MPC8500_I2CDFSRR        0x3F


/* DUART Registers(0x4000-0x5000) */
typedef struct ccsr_duart {
    char    res1[1280];
    u_char  urbr1_uthr1_udlb1;  /* 0x4500 - URBR1, UTHR1, UDLB1 with the */
                                /*          same address offset of 0x04500 */
    u_char  uier1_udmb1;        /* 0x4501 - UIER1, UDMB1 with the same */
                                /*          address offset of 0x04501 */
    u_char  uiir1_ufcr1_uafr1;  /* 0x4502 - UIIR1, UFCR1, UAFR1 with the */
                                /*          same address offset of 0x04502 */
    u_char  ulcr1;            /* 0x4503 - UART1 Line Control Register */
    u_char  umcr1;            /* 0x4504 - UART1 Modem Control Register */
    u_char  ulsr1;            /* 0x4505 - UART1 Line Status Register */
    u_char  umsr1;            /* 0x4506 - UART1 Modem Status Register */
    u_char  uscr1;            /* 0x4507 - UART1 Scratch Register */
    char    res2[8];
    u_char  udsr1;            /* 0x4510 - UART1 DMA Status Register */
    char    res3[239];
    u_char  urbr2_uthr2_udlb2;  /* 0x4600 - URBR2, UTHR2, UDLB2 with the */
                                /*          same address offset of 0x04600 */
    u_char  uier2_udmb2;        /* 0x4601 - UIER2, UDMB2 with the same */
                                /*          address offset of 0x04601 */
    u_char  uiir2_ufcr2_uafr2;  /* 0x4602 - UIIR2, UFCR2, UAFR2 with the */
                                /*          same address offset of 0x04602 */
    u_char  ulcr2;            /* 0x4603 - UART2 Line Control Register */
    u_char  umcr2;            /* 0x4604 - UART2 Modem Control Register */
    u_char  ulsr2;            /* 0x4605 - UART2 Line Status Register */
    u_char  umsr2;            /* 0x4606 - UART2 Modem Status Register */
    u_char  uscr2;            /* 0x4607 - UART2 Scratch Register */
    char    res4[8];
    u_char  udsr2;            /* 0x4610 - UART2 DMA Status Register */
    char    res5[2543];
} ccsr_duart_t;

/* Local Bus Controller Registers(0x5000-0x6000) */
/* Omitting OCeaN(0x6000) and Reserved(0x7000) block */

typedef struct ccsr_lbc {
    uint    br0;        /* 0x5000 - LBC Base Register 0 */
    uint    or0;        /* 0x5004 - LBC Options Register 0 */
    uint    br1;        /* 0x5008 - LBC Base Register 1 */
    uint    or1;        /* 0x500c - LBC Options Register 1 */
    uint    br2;        /* 0x5010 - LBC Base Register 2 */
    uint    or2;        /* 0x5014 - LBC Options Register 2 */
    uint    br3;        /* 0x5018 - LBC Base Register 3 */
    uint    or3;        /* 0x501c - LBC Options Register 3 */
    uint    br4;        /* 0x5020 - LBC Base Register 4 */
    uint    or4;        /* 0x5024 - LBC Options Register 4 */
    uint    br5;        /* 0x5028 - LBC Base Register 5 */
    uint    or5;        /* 0x502c - LBC Options Register 5 */
    uint    br6;        /* 0x5030 - LBC Base Register 6 */
    uint    or6;        /* 0x5034 - LBC Options Register 6 */
    uint    br7;        /* 0x5038 - LBC Base Register 7 */
    uint    or7;        /* 0x503c - LBC Options Register 7 */
    char    res1[40];
    uint    mar;        /* 0x5068 - LBC UPM Address Register */
    char    res2[4];
    uint    mamr;       /* 0x5070 - LBC UPMA Mode Register */
    uint    mbmr;       /* 0x5074 - LBC UPMB Mode Register */
    uint    mcmr;       /* 0x5078 - LBC UPMC Mode Register */
    char    res3[8];
    uint    mrtpr;      /* 0x5084 - LBC Memory Refresh Timer Prescaler */
                        /*        Register */
    uint    mdr;        /* 0x5088 - LBC UPM Data Register */
    char    res4[4];
    uint    lsor;       /* 0x5094 - Special operation initiation register */
    char    res5[12];
    uint    lurt;       /* 0x50a0 - LBC UPM Refresh Timer */
    char    res6[12];
    uint    ltesr;      /* 0x50b0 - LBC Transfer Error Status Register */
    uint    ltedr;      /* 0x50b4 - LBC Transfer Error Disable Register */
    uint    lteir;      /* 0x50b8 - LBC Transfer Error Interrupt Register */
    uint    lteatr;     /* 0x50bc - LBC Transfer Error Attributes Register */
    uint    ltear;      /* 0x50c0 - LBC Transfer Error Address Register */
    uint    lteccr;     /* 0x50c4 - Transfer error ECC register */
    char    res7[8];
    uint    lbcr;       /* 0x50d0 - LBC Configuration Register */
    uint    lcrr;       /* 0x50d4 - LBC Clock Ratio Register */
    char    res8[8];
    uint    fmr;        /* 0x50e0 - Flash mode register */
    uint    fir;        /* 0x50e4 - Flash instruction register */
    uint    fcr;        /* 0x50e8 - Flash command register */
    uint    fbar;       /* 0x50ec - Flash block address register */
    uint    fpar;       /* 0x50f0 - Flash page address register */
    uint    fbcr;       /* 0x50f4 - Flash byte count register */
    char    res9[8];
    uint    fecc0;      /* 0x5100 - Flash ECC block 0 register */
    uint    fecc1;      /* 0x5104 - Flash ECC block 1 register */
    uint    fecc2;      /* 0x5108 - Flash ECC block 2 register */
    uint    fecc3;      /* 0x510c - Flash ECC block 3 register */
    char    res9_1[3824];
} ccsr_lbc_t;

/*
 * Base Register (BR) Fields
 */
#define MPC8500_BR_8_BIT      0x00000800 /*  8 bit port size */
#define MPC8500_BR_16_BIT     0x00001000 /* 16 bit port size */
#define MPC8500_BR_32_BIT     0x00001800 /* 32 bit port size */
#define MPC8500_BR_NO_PARITY  0x00000000 /* parity checking disabled */
#define MPC8500_BR_READ_WRITE 0x00000000 /* write protect disabled */
#define MPC8500_BR_READ_ONLY  0x00000100 /* write protect enabled */
#define MPC8500_BR_MACH_MASK  0x000000E0 /* machine select mask */
#define MPC8500_BR_MACH_GPCM  0x00000000 /* use G.P.C.M. */
#define MPC8500_BR_MACH_FCM   0x00000020 /* use F.C.M. */
#define MPC8500_BR_MACH_UPMA  0x00000080 /* use U.P.M.A. */
#define MPC8500_BR_MACH_UPMB  0x000000A0 /* use U.P.M.B. */
#define MPC8500_BR_MACH_UPMC  0x000000C0 /* use U.P.M.C. */
#define MPC8500_BR_VALID      0x00000001 /* bank is valid */
#define MPC8500_BR_INVALID    0x00000000 /* bank is invalid */

/*
 * Option Register (OR) Fields for GPCM
 */
#define MPC8500_OR_ADDR_MASK(n)       (~(n-1))
#define MPC8500_OR_32K_MASK           0xFFFF8000
#define MPC8500_OR_64K_MASK           0xFFFF0000
#define MPC8500_OR_1M_MASK            0xFFF00000
#define MPC8500_OR_16M_MASK           0xFF000000
#define MPC8500_OR_LBCTL_ASSERTED     0x00000000 /* LBCTL asserted on access */
#define MPC8500_OR_LBCTL_NOT_ASSERTED 0x00001000 /* LBCTL not asserted */
#define MPC8500_OR_CS_NEG_NORM        0x00000000 /* CS negation time normal */
#define MPC8500_OR_CS_NEG_QTR_EARLY   0x00000800 /* CS negation 1/4clk early*/
#define MPC8500_OR_CS_NORM            0x00000000 /* CS setup normal */
#define MPC8500_OR_CS_QTR_LATE        0x00000400 /* CS setup 1/4 clk late */
#define MPC8500_OR_CS_HALF_LATE       0x00000600 /* CS setup 1/2 clk late */
#define MPC8500_OR_CS_NO_ADD_EXTRA    0x00000000 /* No extra CS setup time */
#define MPC8500_OR_CS_ADD_EXTRA_LATE  0x00000100 /* CS setup time ext more */
#define MPC8500_OR_WAIT_STATES(n)     (((n)&0xf)<<4) /* num of wait states */
#define MPC8500_OR_INTERNAL_ADDR_TERM 0x00000000 /* Int addr termination */
#define MPC8500_OR_EXTERNAL_ADDR_TERM 0x00000008 /* Ext addr termination */
#define MPC8500_OR_TIMING_NORMAL      0x00000000 /* Normal timing for GPCM */
#define MPC8500_OR_TIMING_RELAXED     0x00000004 /* Relaxed timing for GPCM */
#define MPC8500_OR_NORMAL_HOLD_TIME   0x00000000 /* Normal hold time on reads */
#define MPC8500_OR_EXTEND_HOLD_TIME   0x00000002 /* Ext hold time on reads */
#define MPC8500_OR_NORMAL_LATCH_DELAY 0x00000000 /* No add clocks on LALE */
#define MPC8500_OR_EXTEND_LATCH_DELAY 0x00000001 /* extra clk on LALE */

/*
 * Option Register (OR) Fields for UPM
 * Only unique fields are mentioned
 */
#define MPC8500_OR_BURST_ENABLE  0x00000000 /* Bursts allowed */
#define MPC8500_OR_BURST_INHIBIT 0x00000800 /* Bursts not allowed */

#define MPC8500_OR_EAD          0x1
#define MPC8500_OR_SETA         0x8
#define MPC8500_MXMR_OP_MASK    0x30000000
#define MPC8500_MXMR_OP_NORMAL  0x00000000
#define MPC8500_MXMR_OP_WRITE   0x10000000
#define MPC8500_MXMR_OP_READ    0x20000000
#define MPC8500_MXMR_OP_RUN     0x30000000
#define MPC8500_MXMR_MAD_MASK   0x0000003f
#define MPC8500_MXMR_MAD(n)     ((n) & 0x3f)
#define MPC8500_LCRR_EADC_MASK  0x30000
#define MPC8500_LCRR_EADC_SHIFT 16
#define MPC8500_LCRR_EADC_FOUR  0
#define MPC8500_LCRR_EADC_THREE 3
#define MPC8500_LCRR_EADC_TWO   2
#define MPC8500_LCRR_EADC_ONE   1

/*
 * Local Bus Transfer Errors 
 */
#define MPC8500_LTE_BM        0x80000000
#define MPC8500_LTE_FCT       0x40000000
#define MPC8500_LTE_PAR       0x20000000
#define MPC8500_LTE_WP        0x04000000
#define MPC8500_LTE_ATMW      0x00800000
#define MPC8500_LTE_ATMR      0x00400000
#define MPC8500_LTE_CS        0x00080000
#define MPC8500_LTE_CC        0x00000001

/*
 * Local Bus Transfer Errors Attributes
 */
#define MPC8500_LTEATR_V    0x00000001


/* Enhanced Serial peripheral interface(eSPI) Registers(0x7000-0x7fff) */

typedef struct ccsr_espi {
    uint    spmode;         /* 0x7000 - eSPI mode register */
    uint    spie;           /* 0x7004 - eSPI event register */
    uint    spim;           /* 0x7008 - eSPI mask register */
    uint    spcom;          /* 0x700c - eSPI command register */
    uint    spitf;          /* 0x7010 - eSPI transmit FIFO access register */
    uint    spirf;          /* 0x7014 - eSPI receive FIFO access register */
    char    res1[8];
    uint    spmode0;        /* 0x7020 - eSPI CS0 mode register */
    uint    spmode1;        /* 0x7024 - eSPI CS1 mode register */
    uint    spmode2;        /* 0x7028 - eSPI CS2 mode register */
    uint    spmode3;        /* 0x702c - eSPI CS3 mode register */ 
    char    res2[4052];   
} ccsr_espi_t;

/* 
 * PCI Registers(0x9000--xa000) 
 */
/* Configuration Address (CFG_ADDR) */
#define MPC8500_CFG_ADDR_EN     0x80000000  /* Enable */
#define MPC8500_CFG_ADDR_BN_M   0x00FF0000  /* Bus number mask */
#define MPC8500_CFG_ADDR_BN_S   16          /* Bus number shift count */
#define MPC8500_CFG_ADDR_DN_M   0x0000F800  /* Device number mask */
#define MPC8500_CFG_ADDR_DN_S   11          /* Device number shift count */
#define MPC8500_CFG_ADDR_FN_M   0x00000700  /* Function number mask */
#define MPC8500_CFG_ADDR_FN_S   8           /* Function # shift count */
#define MPC8500_CFG_ADDR_RN_M   0x000000FC  /* Register number mask */
#define MPC8500_CFG_ADDR_RN_S   2           /* Register # shift count */
#define MPC8500_CFG_ADDR_EXTRN_M    0x0F000000  /* EXTRegister number mask */
#define MPC8500_CFG_ADDR_EXTRN_S    16          /* EXTRegister # shift count */

/* PCI/X, PCI Express Outbound Translation Address Registers (POTAR) */
#define MPC8500_POTAR_TA_SHIFT      12      /* TA shift */

/* PCI/X, PCI Express Outbound Window Base Address Registers (POWBAR) */
#define MPC8500_POWBAR_WBA_SHFT     12      /* WBA shift */

/* PCI/X, PCI Express Outbound Window Attributes Registers (POWAR) */
#define MPC8500_POWAR_EN        0x80000000    /* Enable addr translation */
#define MPC8500_POWAR_S_D       0x40000000    /* Swap disable */
#define MPC8500_POWAR_DIEN      0x40000000    /* Data Invariance 
                                                 Enable (PCIe only) */
#define MPC8500_POWAR_ROE       0x10000000    /* Relaxed ordering enable */
#define MPC8500_POWAR_NS        0x08000000    /* No snoop */
#define MPC8500_POWAR_TC0       0x00000000    /* Traffic classes */
#define MPC8500_POWAR_TC1       0x00200000
#define MPC8500_POWAR_TC2       0x00400000
#define MPC8500_POWAR_TC3       0x00600000
#define MPC8500_POWAR_TC4       0x00800000
#define MPC8500_POWAR_TC5       0x00a00000
#define MPC8500_POWAR_TC6       0x00b00000
#define MPC8500_POWAR_TC7       0x00c00000
#define MPC8500_POWAR_RTT_CFG_R 0x00020000    /* Configuration read */
#define MPC8500_POWAR_RTT_MEM_R    0x00040000    /* Memory read */
#define MPC8500_POWAR_RTT_IO_R     0x00080000    /* I/O read */
#define MPC8500_POWAR_WTT_CFG_W    0x00002000    /* Configuration write */
#define MPC8500_POWAR_WTT_MEM_W    0x00004000    /* Memory write */
#define MPC8500_POWAR_WTT_MSG_W    0x00005000    /* Message write */
#define MPC8500_POWAR_WTT_IO_W     0x00008000    /* I/O write */
#define MPC8500_POWAR_CWS_256K     17        /* CWS - 256K */
#define MPC8500_POWAR_CWS_512K     18        /* CWS - 512K */
#define MPC8500_POWAR_CWS_1M       19        /* CWS - 1M */
#define MPC8500_POWAR_CWS_16M      23        /* CWS - 16M */
#define MPC8500_POWAR_CWS_32M      24        /* CWS - 32M */
#define MPC8500_POWAR_CWS_64M      25        /* CWS - 64M */
#define MPC8500_POWAR_CWS_128M     26        /* CWS - 128M */
#define MPC8500_POWAR_CWS_1G       29        /* CWS - 1G */
#define MPC8500_POWAR_CWS_2G       30        /* CWS - 2G */
#define MPC8500_POWAR_CWS_4G       31        /* CWS - 4G */
#define MPC8500_POWAR_CWS_MASK     0x0000003F    /* CWS mask */

/* PCI/X, PCI Express Inbound Translation Address Registers (PITAR) */
#define MPC8500_PITAR_TA_SHIFT    12        /* TA shift */

/* PCI/X, PCI Express Inbound Window Base Address Registers (PIWBAR) */
#define MPC8500_PIWBAR_BA_SHIFT   12        /* BA shift */

/* PCI/X, PCI Express Inbound Window Attributes Registers (PIWAR) */
#define MPC8500_PIWAR_EN      0x80000000    /* Enable */
#define MPC8500_PIWAR_S_D     0x40000000    /* Swap disable */
#define MPC8500_PIWAR_DIEN    0x40000000    /* Data Invariance
                                               Enable (PCIe only) */
#define MPC8500_PIWAR_PF      0x20000000    /* Prefetchable */
/*    Target Interface (TGI) */
#define MPC8500_PIWAR_TGI_RIO    0x00C00000    /* Rapid IO */
#define MPC8500_PIWAR_TGI_LM     0x00F00000    /* DDR, Local bus, SRAM */
/*    Read Transaction Type (RTT) */
#define MPC8500_PIWAR_RTT_RD     0x00040000    /* Read without snoop */
#define MPC8500_PIWAR_RTT_RDSN   0x00050000    /* Read with snoop */
#define MPC8500_PIWAR_RTT_UL2    0x00070000    /* Read unlock L2 cache */
/*    Write Transaction Type (WTT) */
#define MPC8500_PIWAR_WTT_WR     0x00004000    /* Write without snoop */
#define MPC8500_PIWAR_WTT_WRSN   0x00005000    /* Write with snoop */
#define MPC8500_PIWAR_WTT_AL2    0x00006000    /* Write allocate L2 */
#define MPC8500_PIWAR_WTT_ALL2   0x00007000    /* Write allocate & lock L2 */
/*    Inbound Window Size (IWS) */
#define MPC8500_PIWAR_IWS_512K    18        /* 512K */
#define MPC8500_PIWAR_IWS_4M      21        /* 4M */
#define MPC8500_PIWAR_IWS_32M     24        /* 32M */
#define MPC8500_PIWAR_IWS_256M    27        /* 256M */
#define MPC8500_PIWAR_IWS_512M    28        /* 512M */
#define MPC8500_PIWAR_IWS_1G      29        /* 1G */

/* PCI Express PME and Message defines */
#define MPC8500_PME_PTO       0x00008000
#define MPC8500_PME_PTAT      0x00004000
#define MPC8500_PME_ENL23     0x00002000
#define MPC8500_PME_EXL23     0x00001000
#define MPC8500_PME_HRD       0x00000400
#define MPC8500_PME_LDD       0x00000200
#define MPC8500_PME_AION      0x00000040
#define MPC8500_PME_AIB       0x00000020
#define MPC8500_PME_AIOF      0x00000010
#define MPC8500_PME_PION      0x00000008
#define MPC8500_PME_PIB       0x00000004
#define MPC8500_PME_PIOF      0x00000002
#define MPC8500_PME_ABP       0x00000001

/* PCI Express Error Management defines */
#define MPC8500_PERR_ME       0x80000000
#define MPC8500_PERR_PCT      0x00800000
#define MPC8500_PERR_PCAC     0x00200000
#define MPC8500_PERR_PNM      0x00100000
#define MPC8500_PERR_CDNSC    0x00080000
#define MPC8500_PERR_CRSNC    0x00040000
#define MPC8500_PERR_ICCA     0x00020000
#define MPC8500_PERR_IACA     0x00010000
#define MPC8500_PERR_CRST     0x00008000
#define MPC8500_PERR_MIS      0x00004000
#define MPC8500_PERR_IOIS     0x00002000
#define MPC8500_PERR_CIS      0x00001000
#define MPC8500_PERR_CIEP     0x00000800
#define MPC8500_PERR_IOIEP    0x00000400
#define MPC8500_PERR_OAC      0x00000200
#define MPC8500_PERR_IOIA     0x00000100

/*
 * PCIe Device specific config space defines
 */
#define PCIE_DEV_CTRL_OFFSET    0x54
#define PCIE_DCR_CER        0x0001
#define PCIE_DCR_NFER       0x0002
#define PCIE_DCR_FER        0x0004
#define PCIE_DCR_URR        0x0008

#define PCIE_ROOT_CTRL_OFFSET   0x68
#define PCIE_ROOT_CTRL_SECEE    0x0001
#define PCIE_ROOT_CTRL_SENFEE   0x0002
#define PCIE_ROOT_CTRL_SEFEE    0x0004
#define PCIE_ROOT_CTRL_PMEIE    0x0008


/*
 * PCIe Exteneded config space defines
 */
#define PCIE_ADV_ERR_CAP_CNTL_OFFSET    0x118
#define PCIE_ADV_ERR_CAP_CNTL_ECRCGE    0x00000040
#define PCIE_ADV_ERR_CAP_CNTL_ECRCCE    0x00000100

#define PCIE_ROOT_ERR_CMD_OFFSET      0x12c
#define PCIE_ROOT_ERR_CMD_CERE        0x00000001
#define PCIE_ROOT_ERR_CMD_NFERE       0x00000002
#define PCIE_ROOT_ERR_CMD_FERE        0x00000004

#define PCIE_SEC_STS_INTR_MASK_OFFSET   0x5a0
#define PCIE_SEC_STS_INTR_MASK_M_MDPE   0x000000001
#define PCIE_SEC_STS_INTR_MASK_M_STA    0x000000002
#define PCIE_SEC_STS_INTR_MASK_M_RTA    0x000000004
#define PCIE_SEC_STS_INTR_MASK_M_RMA    0x000000008
#define PCIE_SEC_STS_INTR_MASK_M_SSE    0x000000010
#define PCIE_SEC_STS_INTR_MASK_M_DPE    0x000000020



/* PCI Express registers:
 * PCI Express port 2 (0x9000 - 0x9fff)
 * PCI Express port 1 (0xa000 - 0xafff) <-- only these offsets shown below
 */
typedef struct ccsr_pcie {
    uint    pex_config_addr;   /* 0xa000 - PCI Exp Configuration Address Register */
    uint    pex_config_data;   /* 0xa004 - PCI Exp Configuration Data Register */
    char    res1[4];
    uint    pex_otb_cpl_tor;   /* 0xa00c - PCI Exp outbound completion timeout register */
    uint    pex_conf_rty_tor;  /* 0xa010 - PCI Exp configuration retry timeout register */
    uint    pex_config;        /* 0xa014 - PCI Exp configuration register */
    char    res2[8];
    uint    pex_pme_mes_dr;    /* 0xa020 - PCI Exp PME & message detect register */
    uint    pex_pme_mes_disr;  /* 0xa024 - PCI Exp PME & message disable register */
    uint    pex_pme_mes_ier;   /* 0xa028 - PCI Exp PME & message interrupt enable register */
    uint    pex_pmcr;          /* 0xa02c - PCI Exp power management command register */
    char	res3[3016];
    uint    block_rev1;        /* 0xabf8 - PEX Block Revision register 1 */
    uint    block_rev2;	       /* 0xabfc - PEX Block Revision register 2 */
    uint    pexotar0;          /* 0xac00 - PCI Exp outbound translation address register 0 */
    uint    pexotear0;         /* 0xac04 - PCI Exp outbound translation extended address register 0 */
    char    res4[8];
    uint    pexowar0;          /* 0xac10 - PCI Exp outbound window attributes register 0 */
    char    res5[12];

    uint    pexotar1;          /* 0xac20 - PCI Exp outbound translation address register 1 */
    uint    pexotear1;         /* 0xac24 - PCI Exp outbound translation extended address register 1 */
    uint    pexowbar1;         /* 0xac28 - PCI Exp outbound window base address register 1 */
    char    res6[4];
    uint    pexowar1;          /* 0xac30 - PCI Exp outbound window attributes register 1 */
    char    res7[12];

    uint    pexotar2;          /* 0xac40 - PCI Exp outbound translation address register 2 */
    uint    pexotear2;         /* 0xac44 - PCI Exp outbound translation extended address register 2 */
    uint    pexowbar2;         /* 0xac48 - PCI Exp outbound window base address register 2 */
    char    res8[4];
    uint    pexowar2;          /* 0xac50 - PCI Exp outbound window attributes register 2 */
    char    res9[12];

    uint    pexotar3;          /* 0xac60 - PCI Exp outbound translation address register 3 */
    uint    pexotear3;         /* 0xac64 - PCI Exp outbound translation extended address register 3 */
    uint    pexowbar3;         /* 0xac68 - PCI Exp outbound window base address register 3 */
    char    res10[4];
    uint    pexowar3;          /* 0xac70 - PCI Exp outbound window attributes register 3 */
    char    res11[12];

    uint    pexotar4;          /* 0xac80 - PCI Exp outbound translation address register 4 */
    uint    pexotear4;         /* 0xac84 - PCI Exp outbound translation extended address register 4 */
    uint    pexowbar4;         /* 0xac88 - PCI Exp outbound window base address register 4 */
    char    res12[4];
    uint    pexowar4;          /* 0xac90 - PCI Exp outbound window attributes register 4 */
    char    res13[268];

    uint    pexitar3;          /* 0xada0 - PCI Exp inbound translation address register 3 */
    char    res14[4];
    uint    pexiwbar3;         /* 0xada8 - PCI Exp inbound window base address register 3 */
    uint    pexiwbear3;        /* 0xadac - PCI Exp inbound window base extended address register 3 */
    uint    pexiwar3;          /* 0xadb0 - PCI Exp inbound window attributes register 3 */
    char    res15[12];

    uint    pexitar2;          /* 0xadc0 - PCI Exp inbound translation address register 2 */
    char    res16[4];
    uint    pexiwbar2;         /* 0xadc8 - PCI Exp inbound window base address register 2 */
    uint    pexiwbear2;        /* 0xadcc - PCI Exp inbound window base extended address register 2 */
    uint    pexiwar2;          /* 0xadd0 - PCI Exp inbound window attributes register 2 */
    char    res17[12];

    uint    pexitar1;          /* 0xade0 - PCI Exp inbound translation address register 1 */
    char    res18[4];
    uint    pexiwbar1;         /* 0xade8 - PCI Exp inbound window base address register 1 */
    char    res19[4];
    uint    pexiwar1;          /* 0xadf0 - PCI Exp inbound window attributes register 1 */
    char    res20[12];

    uint    pex_err_dr;        /* 0xae00 - PCI Exp error detect register */
    char    res21[4];
    uint    pex_err_en;        /* 0xae08 - PCI Exp error interrupt enable register */
    char    res22[4];
    uint    pex_err_disr;      /* 0xae10 - PCI Exp error disable register */
    char    res23[12];
    uint    pex_err_cap_stat;  /* 0xae20 - PCI Exp error capture status register */
    char    res24[4];
    uint    pex_err_cap_r0;    /* 0xae28 - PCI Exp error capture register 0 */
    uint    pex_err_cap_r1;    /* 0xae2c - PCI Exp error capture register 1 */
    uint    pex_err_cap_r2;    /* 0xae30 - PCI Exp error capture register 2 */
    uint    pex_err_cap_r3;    /* 0xae34 - PCI Exp error capture register 3 */
    uchar   res25[456];        /* 0xae38 - 0xafff: Reserved */
} ccsr_pcie_t;

#define GPIO_PIN_0    0x80000000
#define GPIO_PIN_1    0x40000000
#define GPIO_PIN_2    0x20000000
#define GPIO_PIN_3    0x10000000
#define GPIO_PIN_4    0x08000000
#define GPIO_PIN_5    0x04000000
#define GPIO_PIN_6    0x02000000
#define GPIO_PIN_7    0x01000000

/*
 * CPU GPIO (0xF000 - 0xFCFF)
 */
typedef struct ccsr_gpio {
    char    res[3072]; /* 0xf000 - 0xfbff */
    uint    gp_dir;    /* 0xfc00 - GPIO direction register */
    uint    gp_odr;    /* 0xfc04 - GPIO open drain register */
    uint    gp_dat;    /* 0xfc08 - GPIO data register */
    uint    gp_ier;    /* 0xfc0c - GPIO interrupt event register */
    uint    gp_imr;    /* 0xfc10 - GPIO interrupt mask register */
    uint    gp_icr;    /* 0xfc14 - GPIO external interrupt ctrl register */
    char    res2[232]; /* 0xfc18 - 0xfcff */ 
} ccsr_gpio_t;

/*
 * TDM configuration (0x1_6000¡V0x1_60FF)
 * TDM data (0x1_6100¡V0x1_617F)
 * TDM clock control (0x1_6180¡V0x1_61FF)
 */
typedef struct ccsr_tdm {
    char res1[512];     /* fill this in later */
} ccsr_tdm_t;

/* L2 Cache Registers(0x2_0000-0x2_1000) */

typedef struct ccsr_l2cache {
    uint    l2ctl;      /* 0x20000 - L2 control register */
    char    res1[12];
    uint    l2cewar0;   /* 0x20010 - L2 cache external write address */
                        /*           register 0 */
    char    res2[4];
    uint    l2cewcr0;   /* 0x20018 - L2 cache external write control */
                        /*           register 0 */
    char    res3[4];
    uint    l2cewar1;   /* 0x20020 - L2 cache external write address */
                        /*           register 1 */
    uint    l2cewarea1; /* 0x20024 - L2 cache external write address */
                        /*           register 1 */
    uint    l2cewcr1;   /* 0x20028 - L2 cache external write control */
                        /*           register 1 */
    char    res5[4];
    uint    l2cewar2;   /* 0x20030 - L2 cache external write address */
                        /*           register 2 */
    uint    l2cewarea2; /* 0x20034 - L2 cache external write address */
                        /*           register 2 */
    uint    l2cewcr2;   /* 0x20038 - L2 cache external write control */
                        /*           register 2 */
    char    res7[4];
    uint    l2cewar3;   /* 0x20040 - L2 cache external write address */
                        /*           register 3 */
    uint    l2cewarea3; /* 0x20044 - L2 cache external write address */
                        /*           register 3 */
    uint    l2cewcr3;   /* 0x20048 - L2 cache external write control */
                        /*           register 3 */
    char    res9[180];
    uint    l2srbar0;   /* 0x20100 - L2 memory-mapped SRAM base address */
                        /*           register 0 */
    uint    l2srbarea0; /* 0x20104 - L2 memory-mapped SRAM base address */
                        /*           extended address 0 */
    uint    l2srbar1;   /* 0x20108 - L2 memory-mapped SRAM base address */
                        /*           register 1 */
    uint    l2srbarea1; /* 0x2010c - L2 memory-mapped SRAM base address */
                        /*           extended address 1 */
    char    res11[3312];
    uint    l2errinjhi;     /* 0x20e00 - L2 error injection mask high */
                            /*           register */
    uint    l2errinjlo;     /* 0x20e04 - L2 error injection mask low */
                            /*           register */
    uint    l2errinjctl;    /* 0x20e08 - L2 error injection mask */
                            /*           control register */
    char    res12[20];
    uint    l2captdatahi;   /* 0x20e20 - L2 error data high capture */
                            /*           register */
    uint    l2captdatalo;   /* 0x20e24 - L2 error data low capture */
                            /*           register */
    uint    l2captecc;      /* 0x20e28 - L2 error syndrome register */
    char    res13[20];
    uint    l2errdet;       /* 0x20e40 - L2 error detect register */
    uint    l2errdis;       /* 0x20e44 - L2 error disable register */
    uint    l2errinten;     /* 0x20e48 - L2 error interrupt enable register */
    uint    l2errattr;      /* 0x20e4c - L2 error attributes capture register */
    uint    l2erraddrhi;    /* 0x20e50 - L2 error address capture register high*/
    uint    l2erraddrlo;    /* 0x20e54 - L2 error address capture register low*/
    uint    l2errctl;       /* 0x20e58 - L2 error control register */
    char    res15[420];
} ccsr_l2cache_t;

#define MPC8500_L2E           0x80000000    /* layer 2 cache enable */
#define MPC8500_L2I           0x40000000    /* layer 2 cache invalidate */
#define MPC8500_L2SIZ_256K    0x20000000    /* l2 SRAM size = 256K  */
#define MPC8500_L2SIZ_1024K   0x30000000    /* l2 SRAM size = 1024K */
#define MPC8500_L2BLKSZ_256K  0x08000000    /* l2 SRAM block size = 256K */
#define MPC8500_L2SRAM0       0x00010000    /* l2 block assignment: sram0 */
#define MPC8500_L2SRAM1       0x00010000    /* l2 block assignment: sram1 */
#define MPC8500_L2SRAM7       0x00070000    /* l2 block assignment: sram7 */

/* L2ERRDIS register */
#define MPC8500_L2_TPARDIS     0x10       /* l2 error disable reg */
#define MPC8500_L2_MBECCDIS    0x8        /* l2 error disable reg */
#define MPC8500_L2_SBECCDIS    0x4        /* l2 error disable reg */
#define MPC8500_L2_L2CFGDIS    0x1        /* l2 error disable reg */

/* L2ERRINTEN register */
#define MPC8500_L2_TPARINTEN     0x10       /* l2 interrupt enable reg */
#define MPC8500_L2_MBECCINTEN    0x8        /* l2 interrupt enable reg */
#define MPC8500_L2_SBECCINTEN    0x4        /* l2 interrupt enable reg */
#define MPC8500_L2_L2CFGINTEN    0x1        /* l2 interrupt enable reg */

/* L2ERRDET register */
#define MPC8500_L2_MULL2ERR     0x80000000      /* multiple l2 errors */
#define MPC8500_L2_TPARERR      0x10            /* tag parity error */
#define MPC8500_L2_MBECCERR     0x8             /* multiple bit ECC error */
#define MPC8500_L2_SBECCERR     0x4             /* single bit ECC error */

#define MPC8500_L2_VALINFO        0x1           /* L2 capture registers valid */
#define MPC8500_L2_TRANSSRC_MASK  0x1F0000      /* L2 transaction source mask */
#define MPC8500_L2_TRANSSRC_DATA  0x110000      /* L2 trans processor data */
#define MPC8500_L2_TRANSTYPE_MASK 0x3000        /* L2 transaction type mask */
#define MPC8500_L2_TRANSTYPE_RMW  0x3000        /* L2 read-modify-write trans */
#define MPC8500_L2_CTHRESH_MASK   0xFF0000      /* L2 cache threshold mask */
#define MPC8500_L2_CTHRESH_SHIFT  16            /* L2 cache threshold shift */

/* DMA Registers(0x2_1000-0x2_2000) */
/* DMA offsets for P102 with two dma controllers are at 0x2_1100 and 0x0_C0000 */
typedef struct dma_chan_reg {
    volatile uint32_t   mr;       /* 0x2_1100 - DMA  Mode Register */
    volatile uint32_t   sr;       /* 0x2_1104 - DMA  Status Register */
    volatile uint32_t   eclndar;  /* 0x2_1108 - DMA  Status Register */
    volatile uint32_t   clndar;   /* 0x2_110C - DMA  Current Link Descriptor Address */
    volatile uint32_t   satr;     /* 0x2_1110 - DMA  Source Attributes Register */
    volatile uint32_t   sar;      /* 0x2_1114 - DMA  Source Address Register */
    volatile uint32_t   datr;     /* 0x2_1118 - DMA  Destination Attributes Register */
    volatile uint32_t   dar;      /* 0x2_111c - DMA  Destination Address Register */
    volatile uint32_t   bcr;      /* 0x2_1120 - DMA  Byte Count Register */
    volatile uint32_t   enlndar;  /* 0x2_1124 - DMA  Next Link Descriptor Address */
    volatile uint32_t   nlndar;   /* 0x2_1128 - DMA  Next Link Descriptor Address */
    volatile uint32_t   res2;     /* 0x2_112C */
    volatile uint32_t   eclsdar;  /* 0x2_1130 - DMA  Next List Descriptor Address */
    volatile uint32_t   clsdar;   /* 0x2_1134 - DMA  Current List - Alternate Base */
    volatile uint32_t   enlsdar;  /* 0x2_1138 - DMA  Next List Descriptor Address */
    volatile uint32_t   nlsdar;   /* 0x2_113C - DMA  Next List Descriptor Address */
    volatile uint32_t   ssr;      /* 0x2_1140 - DMA  Source Stride Register */
    volatile uint32_t   dsr;      /* 0x2_1144 - DMA  Destination Stride Register */
    volatile uint32_t   res3[14]; /* 0x2_1148 */
} dma_chan_reg_t;

typedef struct ccsr_dma {
    uint32_t            res0[64];   /* 0x2_1000 */
    dma_chan_reg_t      chan[4];    /* 0x2_1100 - 0x2_12C4 */
    volatile uint32_t   dgsr;       /* 0x2_1300 */
    char                res2[3324];
} ccsr_dma_t;

/*
 * DMA defines
 */
#define DMA_MR_EIE   0x00000040

#define DMA_SR_TE    0x00000080
#define DMA_SR_CH    0x00000020
#define DMA_SR_PE    0x00000010
#define DMA_SR_EOLNI 0x00000008
#define DMA_SR_CB    0x00000004
#define DMA_SR_EOSI  0x00000002
#define DMA_SR_EOLSI 0x00000001

/*
 * USB 0: 0x2_2000-0x2_2FFF
 * USB 1: 0x2_3000-0x2_3FFF
 */
typedef struct ccsr_usb {
    char    res1[256];
    uint    cap_hciver; /* 0x2_2100 - Capability register length */
                        /* 0x2_2102 - Host interface version number */
    uint    hcspar;     /* 0x2_2104 - Host controller structural parameters */
    uint    hccpar;     /* 0x2_2108 - Host controller capability parameters */
    char    res2[20];
    uint    dciver;     /* 0x2_2120 - Device interface version number */
    uint    dccpar;     /* 0x2_2124 - Device controller parameters */
    char    res3[24];
    uint    usbcmd;     /* 0x2_2140 - USB command */
    uint    usbsts;     /* 0x2_2144 - USB status */
    uint    usbintr;    /* 0x2_2148 - USB interrupt enable */
    uint    frindex;    /* 0x2_214c - USB frame index */
    char    res4[4];
    uint    devaddr;    /* 0x2_2154 - USB device address */
                        /*            Frame list base address */
    
    uint    endptlistaddr;  /* 0x2_2158 - Address at endpoint list (device mode) */
                            /*            Next asynchronous list addr (host mode) */
    char    res5[4];
    uint    burstsize;      /* 0x2_2160 - Programmable burst size */
    uint    txfilltuning;   /* 0x2_2164 - Host TT transmit */
                            /*            pre-buffer packet tuning */
    char    res6[8];
    uint    ulpiviewport;   /* 0x2_2170 - ULPI Register Access */
    char    res7[12];
    uint    configflag;     /* 0x2_2180 - Configured flag register */
    uint    portsc;         /* 0x2_2184 - Port status/control */
    char    res8[32];
    uint    usbmode;        /* 0x2_21a8 - USB device mode */
    uint    endptsetupstat; /* 0x2_21ac - Endpoint setup status */
    uint    endptprime;     /* 0x2_21b0 - Endpoint initialization */
    uint    endptflsuh;     /* 0x2_21b4 - Endpoint de-initialize */
    uint    endptstatus;    /* 0x2_21b8 - Endpoint status */
    uint    endptcomplete;  /* 0x2_21bc - Endpoint complete w1c */
    uint    endptctrl0;     /* 0x2_21c0 - Endpoint control 0 */
    uint    endptctrl1;     /* 0x2_21c4 - Endpoint control 1 */
    uint    endptctrl2;     /* 0x2_21c8 - Endpoint control 2 */
    uint    endptctrl3;     /* 0x2_21cc - Endpoint control 3 */
    uint    endptctrl4;     /* 0x2_21d0 - Endpoint control 4 */
    uint    endptctrl5;     /* 0x2_21d4 - Endpoint control 5 */
    char    res9[552];
    uint    snoop1;         /* 0x2_2400 - Snoop1 */
    uint    snoop2;         /* 0x2_2404 - Snoop2 */
    uint    age_cnt_thresh; /* 0x2_2408 - Age count threshold */
    uint    pri_ctrl;       /* 0x2_240c - Priority control */
    uint    si_ctrl;        /* 0x2_2410 - System interface control */
    char    res10[236];
    uint    control;        /* 0x2_2500 - Control */
    char    res11[2812];
} ccsr_usb_t;

/*
 * eTSEC1: 0x2_4000-0x2_4FFF
 * eTSEC2: 0x2_5000-0x2_5FFF
 * eTSEC3: 0x2_6000-0x2_6FFF
 * indicates supported for etsec
 */
typedef struct ccsr_tsec {
    /* eTSEC general control and status registers */
    uint    tsec_id;    /* 0x24000 + Controller ID Register */
    uint    tsec_id2;   /* 0x24004 + Controller ID Register */
    char    res1[8];
    uint    ievent;     /* 0x24010 : Interrupt Event Register */
    uint    imask;      /* 0x24014 : Interrupt Mask Register */
    uint    edis;       /* 0x24018 : Error Disabled Register */
    char    res2[4];
    uint    ecntrl;     /* 0x24020 : Ethernet Control Register */
    uint    minflr;     /* 0x24024 : Minimum Frame Len */
    uint    ptv;        /* 0x24028 : Pause Time Value Register */
    uint    dmactrl;    /* 0x2402c : DMA Control Register */
    uint    tbipa;      /* 0x24030 : TBI PHY Addr */
    char    res4[204];
    /* eTSEC transmit control and status registers */
    uint    tctrl;      /* 0x24100 : Transmit Control Register */
    uint    tstat;      /* 0x24104 : Transmit Status Register */
    uint    dfvlan;     /* 0x24108 + Default VLAN Control Word */
    uint    tbdlen;     /* 0x2410c : TX Buffer Desc Data Len */
    uint    txic;       /* 0x24110 + Transmit Interrupt Coalescing Register */
    uint    tqueue;     /* 0x24114 + Transmit Queue Control Register */
    char    res6[40];
    uint    tr03wt;     /* 0x24140 + Tx BD Rings 0-3 Round Robin Weighings */
    uint    tr47wt;     /* 0x24144 + Tx Be Rings 4-7 Round Robin Weighings */
    char    res7[56];
    uint    tbptrh;     /* 0x24180 - Tx BD Pointer High Register */
                        /* 0x24180 + Tx Data Buffer Pointer High Bits */
    uint    tbptr;      /* 0x24184 - Tx BD Pointer Low Register */
                        /* 0x24184 + TxBD Pointer for Ring 0 */
    char    res9[4];
    uint    tbptr1;     /* 0x2418C + TxBD Pointer for Ring 1 */
    char    res91[4];
    uint    tbptr2;     /* 0x24194 + TxBD Pointer for Ring 2 */
    char    res92[4];
    uint    tbptr3;     /* 0x2419C + TxBD Pointer for Ring 3 */
    char    res93[4];
    uint    tbptr4;     /* 0x241a4 + TxBD Pointer for Ring 4 */
    char    res94[4];
    uint    tbptr5;     /* 0x241aC + TxBD Pointer for Ring 5 */
    char    res95[4];
    uint    tbptr6;     /* 0x241b4 + TxBD Pointer for Ring 6 */
    char    res96[4];
    uint    tbptr7;     /* 0x241bC + TxBD Pointer for Ring 7 */
    char    res97[64];
    uint    tbaseh;     /* 0x24200 : TxBD Base Address High Bits */
    uint    tbase;      /* 0x24204 - Tx Descriptor Base Address Register */
                        /* 0x24204 + TxBD Base Address of Ring 0 */
    char    res10[4];
    uint    tbase1;     /* 0x2420c + TxBD Base Address of Ring 1 */
    char    res101[4];
    uint    tbase2;     /* 0x24214 + TxBD Base Address of Ring 2 */
    char    res102[4];
    uint    tbase3;     /* 0x2421c + TxBD Base Address of Ring 3 */
    char    res103[4];
    uint    tbase4;     /* 0x24224 + TxBD Base Address of Ring 4 */
    char    res104[4];
    uint    tbase5;     /* 0x2422c + TxBD Base Address of Ring 5 */
    char    res105[4];
    uint    tbase6;     /* 0x24234 + TxBD Base Address of Ring 6 */
    char    res106[4];
    uint    tbase7;     /* 0x2423c + TxBD Base Address of Ring 7 */
    char    res107[64];
    uint    tmr_txts1_id;      /* 0x24280 + Tx time stamp id tag (set 1) */
    uint    tmr_txts2_id;      /* 0x24284 + Tx time stamp id tag (set 2) */
    char    res108[56];
    uint    tmr_txts1_h;    /* 0x242c0 + Tx time stamp high (set 1) */
                            /* 0x242c0 - Out-of-Seq 32B Tx Insert Ptr Low */
    uint    tmr_txts1_l;    /* 0x242c4 + Tx time stamp high (set 1) */
                            /* 0x242c4 - Out-of-Seq 32Byte Tx Reserved Reg */
    uint    tmr_txts2_h;    /* 0x242c8 + Tx time stamp high (set 1) */
                            /* 0x242c8 - Out-of-Seq 32Byte Tx Insert */
                            /*           Index/Length Register */
    uint    tmr_txts2_l;    /* 0x242cc + Tx time stamp high (set 2) */
    char    res11[48];
    /* eTSEC receive control and status registers */
    uint    rctrl;      /* 0x24300 - Receive Control Register */
    uint    rstat;      /* 0x24304 - Receive Status Register */
    char    res12[8];
    uint    rxic;       /* 0x24310 + Receive Interrupt Coalescing Register */
    uint    rqueue;     /* 0x24314 + Receive Queue Control Register */
    char    res13[40];
    uint    mrblr;      /* 0x24340 : Maximum Receive Buffer Length Register */
    char    res14[60];
    uint    rbptrh;     /* 0x24380 - Receive Buffer Descriptor Pointer High 0 */
                        /* 0x24380 + Receive Data Buffer Pointer High Bits */
    uint    rbptr;      /* 0x24384 - Receive Buffer Descriptor Pointer */
                        /* 0x24384 + RxBD Pointer for Ring 0 */
    uint    rbptrh1;    /* 0x24388 - Receive Buffer Descriptor Pointer High 1 */
    uint    rbptrl1;    /* 0x2438c - Receive Buffer Descriptor Pointer Low 1 */
                        /* 0x2438c + RxBD Pointer for Ring 1 */
    uint    rbptrh2;    /* 0x24390 - Receive Buffer Descriptor Pointer High 2 */
    uint    rbptrl2;    /* 0x24394 - Receive Buffer Descriptor Pointer Low 2 */
                        /* 0x24394 + RxBD Pointer for Ring 2 */
    uint    rbptrh3;    /* 0x24398 - Receive Buffer Descriptor Pointer High 3 */
    uint    rbptrl3;    /* 0x2439c - Receive Buffer Descriptor Pointer Low 3 */
                        /* 0x2439c + RxBD Pointer for Ring 3 */
    char    res151[4];
    uint    rbptrl4;    /* 0x243a4 + RxBD Pointer for Ring 4 */
    char    res152[4];
    uint    rbptrl5;    /* 0x243ac + RxBD Pointer for Ring 5 */
    char    res153[4];
    uint    rbptrl6;    /* 0x243b4 + RxBD Pointer for Ring 6 */
    char    res154[4];
    uint    rbptrl7;    /* 0x243bc + RxBD Pointer for Ring 7 */
    char    res16[64];
    uint    rbaseh;     /* 0x24400 - Receive Discriptor Base Address High 0 */
    uint    rbase;      /* 0x24404 - Receive Descriptor Base Address */
                        /* 0x24404 + RxBD Base Address of Ring 0 */
    uint    rbaseh1;    /* 0x24408 - Receive Descriptor Base Address High 1 */
    uint    rbasel1;    /* 0x2440c - Receive Descriptor Base Address Low 1 */
                        /* 0x2440c + RxBD Base Address of Ring 1 */
    uint    rbaseh2;    /* 0x24410 - Receive Descriptor Base Address High 2 */
    uint    rbasel2;    /* 0x24414 - Receive Discriptor Base Address Low 2 */
                        /* 0x24414 + RxBD Base Address of Ring 2 */
    uint    rbaseh3;    /* 0x24418 - Receive Descriptor Base Address High 3 */
    uint    rbasel3;    /* 0x2441c - Receive Discriptor Base Address Low 3 */
                        /* 0x2441c + RxBD Base Address of Ring 3 */
    char    res161[4];
    uint    rbasel4;    /* 0x24424 + RxBD Base Address of Ring 4 */
    char    res162[4];
    uint    rbasel5;    /* 0x2442c + RxBD Base Address of Ring 5 */
    char    res163[4];
    uint    rbasel6;    /* 0x24434 + RxBD Base Address of Ring 6 */
    char    res164[4];
    uint    rbasel7;    /* 0x2443c + RxBD Base Address of Ring 7 */
    char    res17[128];
    uint    tmr_rxts_h; /* 0x244c0 + Rx Timer Time Stamp Register High */
    uint    tmr_rxts_l; /* 0x244c4 + Rx Timer Time Stamp Register Low */
    char    res171[56];
    /* eTSEC MAC registers */
    uint    maccfg1;    /* 0x24500 : MAC Configuration 1 Register */
    uint    maccfg2;    /* 0x24504 : MAC Configuration 2 Register */
    uint    ipgifg;     /* 0x24508 : Inter Packet Gap/Inter Frame Gap */
                        /*           Register */
    uint    hafdup;     /* 0x2450c : Half Duplex Register */
    uint    maxfrm;     /* 0x24510 : Maximum Frame Length Register */
    char    res18[12];
    uint    miimcfg;    /* 0x24520 : MII Management Configuration Register */
    uint    miimcom;    /* 0x24524 : MII Management Command Register */
    uint    miimadd;    /* 0x24528 : MII Management Address Register */
    uint    miimcon;    /* 0x2452c : MII Management Control Register */
    uint    miimstat;   /* 0x24530 : MII Management Status Register */
    uint    miimind;    /* 0x24534 : MII Management Indicator Register */
    char    res19[4];
    uint    ifstat;     /* 0x2453c : Interface Status Register */
    uint    macstnaddr1;/* 0x24540 : Station Address Part 1 Register */
    uint    macstnaddr2;/* 0x24544 : Station Address Part 2 Register */
    uint    mac01addr1; /* 0x24548 + MAC Exact Match Address 1, Part 1 */
    uint    mac01addr2; /* 0x2454c + MAC Exact Match Address 1, Part 2 */
    uint    mac02addr1; /* 0x24550 + MAC Exact Match Address 2, Part 1 */
    uint    mac02addr2; /* 0x24554 + MAC Exact Match Address 2, Part 2 */
    uint    mac03addr1; /* 0x24558 + MAC Exact Match Address 3, Part 1 */
    uint    mac03addr2; /* 0x2455c + MAC Exact Match Address 3, Part 2 */
    uint    mac04addr1; /* 0x24560 + MAC Exact Match Address 4, Part 1 */
    uint    mac04addr2; /* 0x24564 + MAC Exact Match Address 4, Part 2 */
    uint    mac05addr1; /* 0x24568 + MAC Exact Match Address 5, Part 1 */
    uint    mac05addr2; /* 0x2456c + MAC Exact Match Address 5, Part 2 */
    uint    mac06addr1; /* 0x24570 + MAC Exact Match Address 6, Part 1 */
    uint    mac06addr2; /* 0x24574 + MAC Exact Match Address 6, Part 2 */
    uint    mac07addr1; /* 0x24578 + MAC Exact Match Address 7, Part 1 */
    uint    mac07addr2; /* 0x2457c + MAC Exact Match Address 7, Part 2 */
    uint    mac08addr1; /* 0x24580 + MAC Exact Match Address 8, Part 1 */
    uint    mac08addr2; /* 0x24584 + MAC Exact Match Address 8, Part 2 */
    uint    mac09addr1; /* 0x24588 + MAC Exact Match Address 9, Part 1 */
    uint    mac09addr2; /* 0x2458c + MAC Exact Match Address 9, Part 2 */
    uint    mac10addr1; /* 0x24590 + MAC Exact Match Address 10, Part 1 */
    uint    mac10addr2; /* 0x24594 + MAC Exact Match Address 10, Part 2 */
    uint    mac11addr1; /* 0x24598 + MAC Exact Match Address 11, Part 1 */
    uint    mac11addr2; /* 0x2459c + MAC Exact Match Address 11, Part 2 */
    uint    mac12addr1; /* 0x245a0 + MAC Exact Match Address 12, Part 1 */
    uint    mac12addr2; /* 0x245a4 + MAC Exact Match Address 12, Part 2 */
    uint    mac13addr1; /* 0x245a8 + MAC Exact Match Address 13, Part 1 */
    uint    mac13addr2; /* 0x245ac + MAC Exact Match Address 13, Part 2 */
    uint    mac14addr1; /* 0x245b0 + MAC Exact Match Address 14, Part 1 */
    uint    mac14addr2; /* 0x245b4 + MAC Exact Match Address 14, Part 2 */
    uint    mac15addr1; /* 0x245b8 + MAC Exact Match Address 15, Part 1 */
    uint    mac15addr2; /* 0x245bc + MAC Exact Match Address 15, Part 2 */
    char    res20[192];
    /* eTSEC transmit and receive counters */
    uint    tr64;       /* 0x24680 : Tx and Rx 64-byte Frame Counter */
    uint    tr127;      /* 0x24684 : Tx and Rx 65-127 byte Frame Counter */
    uint    tr255;      /* 0x24688 : Tx and Rx 128-255 byte Frame Counter */
    uint    tr511;      /* 0x2468c : Transmit and Receive 256-511 byte Frame */
                        /*           Counter */
    uint    tr1k;       /* 0x24690 : Tx and Rx 512-1023 byte Frame Counter */
    uint    trmax;      /* 0x24694 : Tx and Rx 1024-1518 byte Frame Counter */
    uint    trmgv;      /* 0x24698 : Tx and Rx 1519-1522 byte Good VLAN Frame */
    /* eTSEC receive counters */
    uint    rbyt;       /* 0x2469c : Receive Byte Counter */
    uint    rpkt;       /* 0x246a0 : Receive Packet Counter */
    uint    rfcs;       /* 0x246a4 : Receive FCS Error Counter */
    uint    rmca;       /* 0x246a8 : Receive Multicast Packet Counter */
    uint    rbca;       /* 0x246ac : Receive Broadcast Packet Counter */
    uint    rxcf;       /* 0x246b0 : Receive Control Frame Packet Counter */
    uint    rxpf;       /* 0x246b4 : Receive Pause Frame Packet Counter */
    uint    rxuo;       /* 0x246b8 : Receive Unknown OP Code Counter */
    uint    raln;       /* 0x246bc : Receive Alignment Error Counter */
    uint    rflr;       /* 0x246c0 : Receive Frame Length Error Counter */
    uint    rcde;       /* 0x246c4 : Receive Code Error Counter */
    uint    rcse;       /* 0x246c8 : Receive Carrier Sense Error Counter */
    uint    rund;       /* 0x246cc : Receive Undersize Packet Counter */
    uint    rovr;       /* 0x246d0 : Receive Oversize Packet Counter */
    uint    rfrg;       /* 0x246d4 : Receive Fragments Counter */
    uint    rjbr;       /* 0x246d8 : Receive Jabber Counter */
    uint    rdrp;       /* 0x246dc : Receive Drop Counter */
    /* eTSEC transmit counters */
    uint    tbyt;       /* 0x246e0 : Transmit Byte Counter Counter */
    uint    tpkt;       /* 0x246e4 : Transmit Packet Counter */
    uint    tmca;       /* 0x246e8 : Transmit Multicast Packet Counter */
    uint    tbca;       /* 0x246ec : Transmit Broadcast Packet Counter */
    uint    txpf;       /* 0x246f0 : Transmit Pause Control Frame Counter */
    uint    tdfr;       /* 0x246f4 : Transmit Deferral Packet Counter */
    uint    tedf;       /* 0x246f8 : Transmit Excessive Deferral Packet Cntr */
    uint    tscl;       /* 0x246fc : Transmit Single Collision Packet Counter */
    uint    tmcl;       /* 0x24700 : Transmit Multiple Collision Packet Cntr */
    uint    tlcl;       /* 0x24704 : Transmit Late Collision Packet Counter */
    uint    txcl;       /* 0x24708 : Transmit Excessive Collision Packet Cntr */
    uint    tncl;       /* 0x2470c : Transmit Total Collision Counter */
    char    res21[4];
    uint    tdrp;       /* 0x24714 : Transmit Drop Frame Counter */
    uint    tjbr;       /* 0x24718 : Transmit Jabber Frame Counter */
    uint    tfcs;       /* 0x2471c : Transmit FCS Error Counter */
    uint    txcf;       /* 0x24720 : Transmit Control Frame Counter */
    uint    tovr;       /* 0x24724 : Transmit Oversize Frame Counter */
    uint    tund;       /* 0x24728 : Transmit Undersize Frame Counter */
    uint    tfrg;       /* 0x2472c : Transmit Fragments Frame Counter */
    /* eTSEC counter control and TOE statistics registers */
    uint    car1;       /* 0x24730 : Carry Register One */
    uint    car2;       /* 0x24734 : Carry Register Two */
    uint    cam1;       /* 0x24738 : Carry Mask Register One */
    uint    cam2;       /* 0x2473c : Carry Mask Register Two */
    uint    rrej;       /* 0x24740 + Receive Filer Rejected Packet Counter */
    char    res22[188];
    uint    iaddr0;     /* 0x24800 : Indivdual/Group address register 0 */
    uint    iaddr1;     /* 0x24804 : Indivdual/Group address register 1 */
    uint    iaddr2;     /* 0x24808 : Indivdual/Group address register 2 */
    uint    iaddr3;     /* 0x2480c : Indivdual/Group address register 3 */
    uint    iaddr4;     /* 0x24810 : Indivdual/Group address register 4 */
    uint    iaddr5;     /* 0x24814 : Indivdual/Group address register 5 */
    uint    iaddr6;     /* 0x24818 : Indivdual/Group address register 6 */
    uint    iaddr7;     /* 0x2481c : Indivdual/Group address register 7 */
    char    res23[96];
    uint    gaddr0;     /* 0x24880 : Global/Group address register 0 */
    uint    gaddr1;     /* 0x24884 : Global/Group address register 1 */
    uint    gaddr2;     /* 0x24888 : Global/Group address register 2 */
    uint    gaddr3;     /* 0x2488c : Global/Group address register 3 */
    uint    gaddr4;     /* 0x24890 : Global/Group address register 4 */
    uint    gaddr5;     /* 0x24894 : Global/Group address register 5 */
    uint    gaddr6;     /* 0x24898 : Global/Group address register 6 */
    uint    gaddr7;     /* 0x2489c : Global/Group address register 7 */
    char    res24[608];
    /* eTSEC DMA attribute registers */
    char    res73[248];
    uint    attr;       /* 0x24bf8 : Attributes Register */
    uint    attreli;    /* 0x24bfc : Attributes Extract Length and Extract */
                        /*           Index Register */
    /* eTSEC lossless flow control registers */
    uint    rqprm0;     /* 0x24c00 + Receive Queue Parameters Register 0 */
    uint    rqprm1;     /* 0x24c04 + Receive Queue Parameters Register 1 */
    uint    rqprm2;     /* 0x24c08 + Receive Queue Parameters Register 2 */
    uint    rqprm3;     /* 0x24c0c + Receive Queue Parameters Register 3 */
    uint    rqprm4;     /* 0x24c10 + Receive Queue Parameters Register 4 */
    uint    rqprm5;     /* 0x24c14 + Receive Queue Parameters Register 5 */
    uint    rqprm6;     /* 0x24c18 + Receive Queue Parameters Register 6 */
    uint    rqprm7;     /* 0x24c1c + Receive Queue Parameters Register 7 */
    char    res74[36];
    uint    rfbptr0;    /* 0x24c44 + Last Free RxBD pointer for ring 0 */
    char    res75[4];
    uint    rfbptr1;    /* 0x24c4c + Last Free RxBD pointer for ring 1 */
    char    res76[4];
    uint    rfbptr2;    /* 0x24c54 + Last Free RxBD pointer for ring 2 */
    char    res77[4];
    uint    rfbptr3;    /* 0x24c5c + Last Free RxBD pointer for ring 3 */
    char    res78[4];
    uint    rfbptr4;    /* 0x24c64 + Last Free RxBD pointer for ring 4 */
    char    res79[4];
    uint    rfbptr5;    /* 0x24c6c + Last Free RxBD pointer for ring 5 */
    char    res80[4];
    uint    rfbptr6;    /* 0x24c74 + Last Free RxBD pointer for ring 6 */
    char    res81[4];
    uint    rfbptr7;    /* 0x24c7c + Last Free RxBD pointer for ring 7 */
    char    res82[64];
    /* eTSEC future expansion space */
    char    res83[320]; /* 0x24cc0 */
    /* eTSEC IEEE 1588 register */
    uint    tmr_ctrl;   /* 0x24e00 + Timer control register */
    uint    tmr_tevent; /* 0x24e04 + Time stamp event register */
    uint    tmr_temask; /* 0x24e08 + Timer event mask register */
    uint    tmr_pevent; /* 0x24e0c + Time stamp event register */
    uint    tmr_pemask; /* 0x24e10 + Timer event mask register */
    uint    tmr_stat;   /* 0x24e14 + Time stamp status register */
    uint    tmr_cnt_h;  /* 0x24e18 + Timer counter high register */
    uint    tmr_cnt_l;  /* 0x24e1c + Timer counter low register */
    uint    tmr_add;    /* 0x24e20 + Timer drift compensation addend register */
    uint    tmr_acc;    /* 0x24e24 + Timer accumulator register */
    uint    tmr_prsc;   /* 0x24e28 + Timer prescale */
    char    res84[4];
    uint    tmr_off_h;  /* 0x24e30 + Timer offset high */
    uint    tmr_off_l;  /* 0x24e34 + Timer offset low */
    char    res85[8];
    uint    tmr_alarm1_h;  /* 0x24e40 + Timer alarm1 high register */
    uint    tmr_alarm1_l;  /* 0x24e44 + Timer alarm1 low register */
    uint    tmr_alarm2_h;  /* 0x24e48 + Timer alarm2 high register */
    uint    tmr_alarm2_l;  /* 0x24e4c + Timer alarm2 low register */
    char    res86[48];
    uint    tmr_fiper1; /* 0x24e80 + Timer fixed period interval */
    uint    tmr_fiper2; /* 0x24e84 + Timer fixed period interval */
    uint    tmr_fiper3; /* 0x24e88 + Timer fixed period interval */
    char    res87[20];
    uint    tmr_etts1_h; /* 0x24ea0 + Time stamp of general purpose ext trig */
    uint    tmr_etts1_l; /* 0x24ea4 + Time stamp of general purpose ext trig */
    uint    tmr_etts2_h; /* 0x24ea8 + Time stamp of general purpose ext trig */
    uint    tmr_etts2_l; /* 0x24eac + Time stamp of general purpose ext trig */
    char    res88[336];
} ccsr_tsec_t;

/* eSDHC Registers(0x2_E000-0x2_EFFF) */
typedef struct ccsr_esdhc {
    uint    dsa;        /* 0x2_e000 - DMA system address */
    uint    blkattr;    /* 0x2_e004 - Block attributes */
    uint    cmdarg;     /* 0x2_e008 - Command argument */
    uint    xfertyp;    /* 0x2_e00c - Command transfer type */
    uint    cmdrsp0;    /* 0x2_e010 - Command response0 */
    uint    cmdrsp1;    /* 0x2_e014 - Command response1 */
    uint    cmdrsp2;    /* 0x2_e018 - Command response2 */
    uint    cmdrsp3;    /* 0x2_e01c - Command response3 */
    uint    datport;    /* 0x2_e020 - Data buffer access port */
    uint    prsstat;    /* 0x2_e024 - Present state */
    uint    proctl;     /* 0x2_e028 - Protocol control */
    uint    sysctl;     /* 0x2_e02c - System control */
    uint    irqstat;    /* 0x2_e030 - Interrupt status */
    uint    irqstaten;  /* 0x2_e034 - Interrupt status enable */
    uint    irqsigen;   /* 0x2_e038 - Interrupt signal enable */
    uint    autoc12err; /* 0x2_e03c - Auto CMD12 status */
    uint    hostcapblt; /* 0x2_e040 - Host controller capabilities */
    char    res1[4028];
} ccsr_esdhc_t;

/* PIC Registers(0x4_0000-0x8_0000) */
typedef struct ccsr_pic {
    uint    brr1;       /* 0x40000 - Block revision register 1 */
    uint    brr2;       /* 0x40004 - Block revision register 2 */
    char    res1[56];
    uint    ipidr0;     /* 0x40040 - Interprocessor Interrupt Dispatch */
                        /*           Register 0 */
    char    res2[12];
    uint    ipidr1;     /* 0x40050 - Interprocessor Interrupt Dispatch */
                        /*           Register 1 */
    char    res3[12];
    uint    ipidr2;     /* 0x40060 - Interprocessor Interrupt Dispatch */
                        /*           Register 2 */
    char    res4[12];
    uint    ipidr3;     /* 0x40070 - Interprocessor Interrupt Dispatch */
                        /*           Register 3 */
    char    res5[12];
    uint    ctpr;       /* 0x40080 - Current Task Priority Register */
    char    res6[12];
    uint    whoami;     /* 0x40090 - Who Am I Register */
    char    res7[12];
    uint    iack;       /* 0x400a0 - Interrupt Acknowledge Register */
    char    res8[12];
    uint    eoi;        /* 0x400b0 - End Of Interrupt Register */
    char    res9[3916];
    uint    frr;        /* 0x41000 - Feature Reporting Register */
    char    res10[28];
    uint    gcr;        /* 0x41020 - Global Configuration Register */
    char    res11[92];
    uint    vir;        /* 0x41080 - Vendor Identification Register */
    char    res12[12];
    uint    pir;        /* 0x41090 - Processor Initialization Register */
    char    res13[12];
    uint    ipivpr0;    /* 0x410a0 - IPI Vector/Priority Register 0 */
    char    res14[12];
    uint    ipivpr1;    /* 0x410b0 - IPI Vector/Priority Register 1 */
    char    res15[12];
    uint    ipivpr2;    /* 0x410c0 - IPI Vector/Priority Register 2 */
    char    res16[12];
    uint    ipivpr3;    /* 0x410d0 - IPI Vector/Priority Register 3 */
    char    res17[12];
    uint    svr;        /* 0x410e0 - Spurious Vector Register */
    char    res18[12];

    /* Global Timer Group A Registers */
    uint    tfrr;       /* 0x410f0 - Timer Frequency Reporting Register */
    char    res19[12];
    uint    gtccr0;     /* 0x41100 - Global Timer Current Count Register 0 */
    char    res20[12];
    uint    gtbcr0;     /* 0x41110 - Global Timer Base Count Register 0 */
    char    res21[12];
    uint    gtvpr0;     /* 0x41120 - Global Timer Vector/Priority Register 0 */
    char    res22[12];
    uint    gtdr0;      /* 0x41130 - Global Timer Destination Register 0 */
    char    res23[12];
    uint    gtccr1;     /* 0x41140 - Global Timer Current Count Register 1 */
    char    res24[12];
    uint    gtbcr1;     /* 0x41150 - Global Timer Base Count Register 1 */
    char    res25[12];
    uint    gtvpr1;     /* 0x41160 - Global Timer Vector/Priority Register 1 */
    char    res26[12];
    uint    gtdr1;      /* 0x41170 - Global Timer Destination Register 1 */
    char    res27[12];
    uint    gtccr2;     /* 0x41180 - Global Timer Current Count Register 2 */
    char    res28[12];
    uint    gtbcr2;     /* 0x41190 - Global Timer Base Count Register 2 */
    char    res29[12];
    uint    gtvpr2;     /* 0x411a0 - Global Timer Vector/Priority Register 2 */
    char    res30[12];
    uint    gtdr2;      /* 0x411b0 - Global Timer Destination Register 2 */
    char    res31[12];
    uint    gtccr3;     /* 0x411c0 - Global Timer Current Count Register 3 */
    char    res32[12];
    uint    gtbcr3;     /* 0x411d0 - Global Timer Base Count Register 3 */
    char    res33[12];
    uint    gtvpr3;     /* 0x411e0 - Global Timer Vector/Priority Register 3 */
    char    res34[12];
    uint    gtdr3;      /* 0x411f0 - Global Timer Destination Register 3 */
    char    res35[268];
    uint    tcr;        /* 0x41300 - Timer Control Register */
    char    res36[4];
    uint    erqsr;      /* 0x41308 - External interrupt summary register */ 
    char    res37[4];
    uint    irqsr0;     /* 0x41310 - IRQ_OUT Summary Register 0 */
    char    res38[12];
    uint    irqsr1;     /* 0x41320 - IRQ_OUT Summary Register 1 */
    uint    irqsr2;     /* 0x41324 - IRQ_OUT Summary Register 2 */
    char    res39[8];
    uint    cisr0;      /* 0x41330 - Critical Interrupt Summary Register 0 */
    char    res40[12];
    uint    cisr1;      /* 0x41340 - Critical Interrupt Summary Register 1 */
    uint    cisr2;      /* 0x41344 - Critical Interrupt Summary Register 2 */
    char    res41[8];
    uint    pm0mr0;     /* 0x41350 - Performance monitor 0 mask register 0 */
    char    res42[12];
    uint    pm0mr1;     /* 0x41360 - Performance monitor 0 mask register 1 */
    uint    pm0mr2;     /* 0x41364 - Performance monitor 0 mask register 2 */
    char    res43[8];
    uint    pm1mr0;     /* 0x41370 - Performance monitor 1 mask register 0 */
    char    res44[12];
    uint    pm1mr1;     /* 0x41380 - Performance monitor 1 mask register 1 */
    uint    pm1mr2;     /* 0x41384 - Performance monitor 1 mask register 2 */
    char    res45[8];
    uint    pm2mr0;     /* 0x41390 - Performance monitor 2 mask register 0 */
    char    res646[12];
    uint    pm2mr1;     /* 0x413a0 - Performance monitor 2 mask register 1 */
    uint    pm2mr2;     /* 0x413a4 - Performance monitor 2 mask register 2 */
    char    res647[8];
    uint    pm3mr0;     /* 0x413b0 - Performance monitor 3 mask register 0 */
    char    res648[12];
    uint    pm3mr1;     /* 0x413c0 - Performance monitor 3 mask register 1 */
    uint    pm3mr2;     /* 0x413c4 - Performance monitor 3 mask register 2 */
    char    res649[56];
    uint    msgr0;      /* 0x41400 - Message Register 0 */
    char    res650[12];
    uint    msgr1;      /* 0x41410 - Message Register 1 */
    char    res651[12];
    uint    msgr2;      /* 0x41420 - Message Register 2 */
    char    res652[12];
    uint    msgr3;      /* 0x41430 - Message Register 3 */
    char    res653[204];
    uint    mer;        /* 0x41500 - Message Enable Register */
    char    res654[12];
    uint    msr;        /* 0x41510 - Message Status Register */
    char    res210[236];
    uint    msir0;       /* 0x41600 - Message Shared Interrupt */
                         /*           Register 0 */
    char    res200[12];
    uint    msir1;       /* 0x41610 - Message Shared Interrupt */
                         /*           Register 1 */
    char    res201[12];
    uint    msir2;       /* 0x41620 - Message Shared Interrupt */
                         /*           Register 2 */
    char    res202[12];
    uint    msir3;       /* 0x41630 - Message Shared Interrupt */
                         /*           Register 3 */
    char    res203[12];
    uint    msir4;       /* 0x41640 - Message Shared Interrupt */
                         /*           Register 4 */
    char    res204[12];
    uint    msir5;       /* 0x41650 - Message Shared Interrupt */
                         /*           Register 5 */
    char    res205[12];
    uint    msir6;       /* 0x41660 - Message Shared Interrupt */
                         /*           Register 6 */
    char    res206[12];
    uint    msir7;       /* 0x41670 - Message Shared Interrupt */
                         /*           Register 7 */
    char    res207[172];
    uint    msisr;       /* 0x41720 - Message Shared Interrupt */
                         /*           Status Register */
    char    res209[28];
    uint    msiir;       /* 0x41740 - Message Shared Interrupt */
                         /*           Index Register */

    /* Global Timer Group B Registers */
    char    res46[0x420f0 - 0x41740 - 4];
    uint    tfrrb;       /* 0x420f0 - Timer Frequency Reporting Register Group B */
    char    res151[12];
    uint    gtccrb0;     /* 0x42100 - Global Timer Current Count Register Group B 0 */
    char    res152[12];
    uint    gtbcrb0;     /* 0x42110 - Global Timer Base Count Register Group B 0 */
    char    res153[12];
    uint    gtvprb0;     /* 0x42120 - Global Timer Vector/Priority Register Group B 0 */
    char    res154[12];
    uint    gtdrb0;      /* 0x42130 - Global Timer Destination Register Group B 0 */
    char    res155[12];
    uint    gtccrb1;     /* 0x42140 - Global Timer Current Count Register Group B 1 */
    char    res156[12];
    uint    gtbcrb1;     /* 0x42150 - Global Timer Base Count Register Group B 1 */
    char    res157[12];
    uint    gtvprb1;     /* 0x42160 - Global Timer Vector/Priority Register Group B 1 */
    char    res158[12];
    uint    gtdrb1;      /* 0x42170 - Global Timer Destination Register Group B 1 */
    char    res159[12];
    uint    gtccrb2;     /* 0x42180 - Global Timer Current Count Register Group B 2 */
    char    res160[12];
    uint    gtbcrb2;     /* 0x42190 - Global Timer Base Count Register Group B 2 */
    char    res161[12];
    uint    gtvprb2;     /* 0x421a0 - Global Timer Vector/Priority Register Group B 2 */
    char    res162[12];
    uint    gtdrb2;      /* 0x421b0 - Global Timer Destination Register Group B 2 */
    char    res163[12];
    uint    gtccrb3;     /* 0x421c0 - Global Timer Current Count Register Group B 3 */
    char    res164[12];
    uint    gtbcrb3;     /* 0x421d0 - Global Timer Base Count Register Group B 3 */
    char    res165[12];
    uint    gtvprb3;     /* 0x421e0 - Global Timer Vector/Priority Register Group B 3 */
    char    res166[12];
    uint    gtdrb3;      /* 0x421f0 - Global Timer Destination Register Group B 3 */
    char    res167[0x42300 - 0x421f0 - 4];
    uint    tcrb;        /* 0x42300 - Timer Control Register (Group B) */

    char    res168[0x42400 - 0x42300 - 4];

    uint    msgr4;      /* 0x42400 - Message Register 4 */
    char    res169[12];
    uint    msgr5;      /* 0x42410 - Message Register 5 */
    char    res170[12];
    uint    msgr6;      /* 0x42420 - Message Register 6 */
    char    res171[12];
    uint    msgr7;      /* 0x42430 - Message Register 7 */
    char    res172[0x42500 - 0x42430 - 4];
    uint    mer4_7;     /* 0x42500 - Message Enable Register (For MSGR4-7) */
    char    res173[12];
    uint    msr4_7;     /* 0x42510 - Message Status Register (For MSGR4-7) */

    char    res174[0x50000 - 0x42510 - 4];

    /* Interrupt Source Configuration Registers */
    uint    eivpr0;     /* 0x50000 - External Interrupt Vector/Priority */
                        /*           Register 0 */
    char    res47[12];
    uint    eidr0;      /* 0x50010 - External Interrupt Destination Register */
                        /*           0 */
    char    res48[12];
    uint    eivpr1;     /* 0x50020 - External Interrupt Vector/Priority */
                        /*           Register 1 */
    char    res49[12];
    uint    eidr1;      /* 0x50030 - External Interrupt Destination Register */
                        /*           1 */
    char    res50[12];
    uint    eivpr2;     /* 0x50040 - External Interrupt Vector/Priority */
                        /*           Register 2 */
    char    res51[12];
    uint    eidr2;      /* 0x50050 - External Interrupt Destination Register */
                        /*           2 */
    char    res52[12];
    uint    eivpr3;     /* 0x50060 - External Interrupt Vector/Priority */
                        /*           Register 3 */
    char    res53[12];
    uint    eidr3;      /* 0x50070 - External Interrupt Destination Register */
                        /*           3 */
    char    res54[12];
    uint    eivpr4;     /* 0x50080 - External Interrupt Vector/Priority */
                        /*           Register 4 */
    char    res55[12];
    uint    eidr4;      /* 0x50090 - External Interrupt Destination Register */
                        /*           4 */
    char    res56[12];
    uint    eivpr5;     /* 0x500a0 - External Interrupt Vector/Priority */
                        /*           Register 5 */
    char    res57[12];
    uint    eidr5;      /* 0x500b0 - External Interrupt Destination Register */
                        /*           5 */
    char    res58[12];
    uint    eivpr6;     /* 0x500c0 - External Interrupt Vector/Priority */
                        /*           Register 6 */
    char    res59[12];
    uint    eidr6;      /* 0x500d0 - External Interrupt Destination Register */
                        /*           6 */
    char    res60[12];
    uint    eivpr7;     /* 0x500e0 - External Interrupt Vector/Priority */
                        /*           Register 7 */
    char    res61[12];
    uint    eidr7;      /* 0x500f0 - External Interrupt Destination Register */
                        /*           7 */
    char    res62[12];
    uint    eivpr8;     /* 0x50100 - External Interrupt Vector/Priority */
                        /*           Register 8 */
    char    res63[12];
    uint    eidr8;      /* 0x50110 - External Interrupt Destination Register */
                        /*           8 */
    char    res64[12];
    uint    eivpr9;     /* 0x50120 - External Interrupt Vector/Priority */
                        /*           Register 9 */
    char    res65[12];
    uint    eidr9;      /* 0x50130 - External Interrupt Destination Register */
                        /*           9 */
    char    res66[12];
    uint    eivpr10;    /* 0x50140 - External Interrupt Vector/Priority */
                        /*           Register 10 */
    char    res67[12];
    uint    eidr10;     /* 0x50150 - External Interrupt Destination Register */
                        /*           10 */
    char    res68[12];
    uint    eivpr11;    /* 0x50160 - External Interrupt Vector/Priority */
                        /*           Register 11 */
    char    res69[12];
    uint    eidr11;     /* 0x50170 - External Interrupt Destination Register */
                        /*           11 */
    char    res70[140];
    uint    iivpr0;     /* 0x50200 - Internal Interrupt Vector/Priority */
                        /*           Register 0 */
    char    res71[12];
    uint    iidr0;      /* 0x50210 - Internal Interrupt Destination Register */
                        /*           0 */
    char    res72[12];
    uint    iivpr1;     /* 0x50220 - Internal Interrupt Vector/Priority */
                        /*           Register 1 */
    char    res73[12];
    uint    iidr1;      /* 0x50230 - Internal Interrupt Destination Register */
                        /*           1 */
    char    res74[12];
    uint    iivpr2;     /* 0x50240 - Internal Interrupt Vector/Priority */
                        /*           Register 2 */
    char    res75[12];
    uint    iidr2;      /* 0x50250 - Internal Interrupt Destination Register */
                        /*           2 */
    char    res76[12];
    uint    iivpr3;     /* 0x50260 - Internal Interrupt Vector/Priority */
                        /*           Register 3 */
    char    res77[12];
    uint    iidr3;      /* 0x50270 - Internal Interrupt Destination Register */
                        /*           3 */
    char    res78[12];
    uint    iivpr4;     /* 0x50280 - Internal Interrupt Vector/Priority */
                        /*           Register 4 */
    char    res79[12];
    uint    iidr4;      /* 0x50290 - Internal Interrupt Destination Register */
                        /*           4 */
    char    res80[12];
    uint    iivpr5;     /* 0x502a0 - Internal Interrupt Vector/Priority */
                        /*           Register 5 */
    char    res81[12];
    uint    iidr5;      /* 0x502b0 - Internal Interrupt Destination Register */
                        /*           5 */
    char    res82[12];
    uint    iivpr6;     /* 0x502c0 - Internal Interrupt Vector/Priority */
                        /*           Register 6 */
    char    res83[12];
    uint    iidr6;      /* 0x502d0 - Internal Interrupt Destination Register */
                        /*           6 */
    char    res84[12];
    uint    iivpr7;     /* 0x502e0 - Internal Interrupt Vector/Priority */
                        /*           Register 7 */
    char    res85[12];
    uint    iidr7;      /* 0x502f0 - Internal Interrupt Destination Register */
                        /*           7 */
    char    res86[12];
    uint    iivpr8;     /* 0x50300 - Internal Interrupt Vector/Priority */
                        /*           Register 8 */
    char    res87[12];
    uint    iidr8;      /* 0x50310 - Internal Interrupt Destination Register */
                        /*           8 */
    char    res88[12];
    uint    iivpr9;     /* 0x50320 - Internal Interrupt Vector/Priority */
                        /*           Register 9 */
    char    res89[12];
    uint    iidr9;      /* 0x50330 - Internal Interrupt Destination Register */
                        /*           9 */
    char    res90[12];
    uint    iivpr10;    /* 0x50340 - Internal Interrupt Vector/Priority */
                        /*           Register 10 */
    char    res91[12];
    uint    iidr10;     /* 0x50350 - Internal Interrupt Destination Register */
                        /*           10 */
    char    res92[12];
    uint    iivpr11;    /* 0x50360 - Internal Interrupt Vector/Priority */
                        /*           Register 11 */
    char    res93[12];
    uint    iidr11;     /* 0x50370 - Internal Interrupt Destination Register */
                        /*           11 */
    char    res94[12];
    uint    iivpr12;    /* 0x50380 - Internal Interrupt Vector/Priority */
                        /*           Register 12 */
    char    res95[12];
    uint    iidr12;     /* 0x50390 - Internal Interrupt Destination Register */
                        /*           12 */
    char    res96[12];
    uint    iivpr13;    /* 0x503a0 - Internal Interrupt Vector/Priority */
                        /*           Register 13 */
    char    res97[12];
    uint    iidr13;     /* 0x503b0 - Internal Interrupt Destination Register */
                        /*           13 */
    char    res98[12];
    uint    iivpr14;    /* 0x503c0 - Internal Interrupt Vector/Priority */
                        /*           Register 14 */
    char    res99[12];
    uint    iidr14;     /* 0x503d0 - Internal Interrupt Destination Register */
                        /*           14 */
    char    res100[12];
    uint    iivpr15;    /* 0x503e0 - Internal Interrupt Vector/Priority */
                        /*           Register 15 */
    char    res101[12];
    uint    iidr15;     /* 0x503f0 - Internal Interrupt Destination Register */
                        /*           15 */
    char    res102[12];
    uint    iivpr16;    /* 0x50400 - Internal Interrupt Vector/Priority */
                        /*           Register 16 */
    char    res103[12];
    uint    iidr16;     /* 0x50410 - Internal Interrupt Destination Register */
                        /*           16 */
    char    res104[12];
    uint    iivpr17;    /* 0x50420 - Internal Interrupt Vector/Priority */
                        /*           Register 17 */
    char    res105[12];
    uint    iidr17;     /* 0x50430 - Internal Interrupt Destination Register */
                        /*           17 */
    char    res106[12];
    uint    iivpr18;    /* 0x50440 - Internal Interrupt Vector/Priority */
                        /*           Register 18 */
    char    res107[12];
    uint    iidr18;     /* 0x50450 - Internal Interrupt Destination Register */
                        /*           18 */
    char    res108[12];
    uint    iivpr19;    /* 0x50460 - Internal Interrupt Vector/Priority */
                        /*           Register 19 */
    char    res109[12];
    uint    iidr19;     /* 0x50470 - Internal Interrupt Destination Register */
                        /*           19 */
    char    res110[12];
    uint    iivpr20;    /* 0x50480 - Internal Interrupt Vector/Priority */
                        /*           Register 20 */
    char    res111[12];
    uint    iidr20;     /* 0x50490 - Internal Interrupt Destination Register */
                        /*           20 */
    char    res112[12];
    uint    iivpr21;    /* 0x504a0 - Internal Interrupt Vector/Priority */
                        /*           Register 21 */
    char    res113[12];
    uint    iidr21;     /* 0x504b0 - Internal Interrupt Destination Register */
                        /*           21 */
    char    res114[12];
    uint    iivpr22;    /* 0x504c0 - Internal Interrupt Vector/Priority */
                        /*           Register 22 */
    char    res115[12];
    uint    iidr22;     /* 0x504d0 - Internal Interrupt Destination Register */
                        /*           22 */
    char    res116[12];
    uint    iivpr23;    /* 0x504e0 - Internal Interrupt Vector/Priority */
                        /*           Register 23 */
    char    res117[12];
    uint    iidr23;     /* 0x504f0 - Internal Interrupt Destination Register */
                        /*           23 */
    char    res118[12];
    uint    iivpr24;    /* 0x50500 - Internal Interrupt Vector/Priority */
                        /*           Register 24 */
    char    res119[12];
    uint    iidr24;     /* 0x50510 - Internal Interrupt Destination Register */
                        /*           24 */
    char    res120[12];
    uint    iivpr25;    /* 0x50520 - Internal Interrupt Vector/Priority */
                        /*           Register 25 */
    char    res121[12];
    uint    iidr25;     /* 0x50530 - Internal Interrupt Destination Register */
                        /*           25 */
    char    res122[12];
    uint    iivpr26;    /* 0x50540 - Internal Interrupt Vector/Priority */
                        /*           Register 26 */
    char    res123[12];
    uint    iidr26;     /* 0x50550 - Internal Interrupt Destination Register */
                        /*           26 */
    char    res124[12];
    uint    iivpr27;    /* 0x50560 - Internal Interrupt Vector/Priority */
                        /*           Register 27 */
    char    res125[12];
    uint    iidr27;     /* 0x50570 - Internal Interrupt Destination Register */
                        /*           27 */
    char    res126[12];
    uint    iivpr28;    /* 0x50580 - Internal Interrupt Vector/Priority */
                        /*           Register 28 */
    char    res127[12];
    uint    iidr28;     /* 0x50590 - Internal Interrupt Destination Register */
                        /*           28 */
    char    res128[12];
    uint    iivpr29;    /* 0x505a0 - Internal Interrupt Vector/Priority */
                        /*           Register 29 */
    char    res129[12];
    uint    iidr29;     /* 0x505b0 - Internal Interrupt Destination Register */
                        /*           29 */
    char    res130[12];
    uint    iivpr30;    /* 0x505c0 - Internal Interrupt Vector/Priority */
                        /*           Register 30 */
    char    res131[12];
    uint    iidr30;     /* 0x505d0 - Internal Interrupt Destination Register */
                        /*           30 */
    char    res132[12];
    uint    iivpr31;    /* 0x505e0 - Internal Interrupt Vector/Priority */
                        /*           Register 31 */
    char    res133[12];
    uint    iidr31;     /* 0x505f0 - Internal Interrupt Destination Register */
                        /*           31 */
    char    res1170[12];
    uint    iivpr32;    /* 0x50600 - Internal Interrupt Vector/Priority */
                        /*           Register 32 */
    char    res1171[12];
    uint    iidr32;     /* 0x50610 - Internal Interrupt Destination Register */
                        /*           32 */
    char    res1172[12];
    uint    iivpr33;    /* 0x50620 - Internal Interrupt Vector/Priority */
                        /*           Register 33 */
    char    res1173[12];
    uint    iidr33;     /* 0x50630 - Internal Interrupt Destination Register */
                        /*           33 */
    char    res1174[12];
    uint    iivpr34;    /* 0x50640 - Internal Interrupt Vector/Priority */
                        /*           Register 34 */
    char    res1175[12];
    uint    iidr34;     /* 0x50650 - Internal Interrupt Destination Register */
                        /*           34 */
    char    res1176[12];
    uint    iivpr35;    /* 0x50660 - Internal Interrupt Vector/Priority */
                        /*           Register 35 */
    char    res1177[12];
    uint    iidr35;     /* 0x50670 - Internal Interrupt Destination Register */
                        /*           35 */
    char    res1178[12];
    uint    iivpr36;    /* 0x50680 - Internal Interrupt Vector/Priority */
                        /*           Register 36 */
    char    res1179[12];
    uint    iidr36;     /* 0x50690 - Internal Interrupt Destination Register */
                        /*           36 */
    char    res1180[12];
    uint    iivpr37;    /* 0x506a0 - Internal Interrupt Vector/Priority */
                        /*           Register 37 */
    char    res1181[12];
    uint    iidr37;     /* 0x506b0 - Internal Interrupt Destination Register */
                        /*           37 */
    char    res1182[12];
    uint    iivpr38;    /* 0x506c0 - Internal Interrupt Vector/Priority */
                        /*           Register 38 */
    char    res1183[12];
    uint    iidr38;     /* 0x506d0 - Internal Interrupt Destination Register */
                        /*           38 */
    char    res1184[12];
    uint    iivpr39;    /* 0x506e0 - Internal Interrupt Vector/Priority */
                        /*           Register 39 */
    char    res1185[12];
    uint    iidr39;     /* 0x506f0 - Internal Interrupt Destination Register */
                        /*           39 */
    char    res2170[12];
    uint    iivpr40;    /* 0x50700 - Internal Interrupt Vector/Priority */
                        /*           Register 40 */
    char    res2171[12];
    uint    iidr40;     /* 0x50710 - Internal Interrupt Destination Register */
                        /*           40 */
    char    res2172[12];
    uint    iivpr41;    /* 0x50720 - Internal Interrupt Vector/Priority */
                        /*           Register 41 */
    char    res2173[12];
    uint    iidr41;     /* 0x50730 - Internal Interrupt Destination Register */
                        /*           41 */
    char    res2174[12];
    uint    iivpr42;    /* 0x50740 - Internal Interrupt Vector/Priority */
                        /*           Register 42 */
    char    res2175[12];
    uint    iidr42;     /* 0x50750 - Internal Interrupt Destination Register */
                        /*           42 */
    char    res2176[12];
    uint    iivpr43;    /* 0x50760 - Internal Interrupt Vector/Priority */
                        /*           Register 43 */
    char    res2177[12];
    uint    iidr43;     /* 0x50770 - Internal Interrupt Destination Register */
                        /*           43 */
    char    res2178[12];
    uint    iivpr44;    /* 0x50780 - Internal Interrupt Vector/Priority */
                        /*           Register 44 */
    char    res2179[12];
    uint    iidr44;     /* 0x50790 - Internal Interrupt Destination Register */
                        /*           44 */
    char    res2180[12];
    uint    iivpr45;    /* 0x507a0 - Internal Interrupt Vector/Priority */
                        /*           Register 45 */
    char    res2181[12];
    uint    iidr45;     /* 0x507b0 - Internal Interrupt Destination Register */
                        /*           45 */
    char    res2182[12];
    uint    iivpr46;    /* 0x507c0 - Internal Interrupt Vector/Priority */
                        /*           Register 46 */
    char    res2183[12];
    uint    iidr46;     /* 0x507d0 - Internal Interrupt Destination Register */
                        /*           46 */
    char    res2184[12];
    uint    iivpr47;    /* 0x507e0 - Internal Interrupt Vector/Priority */
                        /*           Register 47 */
    char    res2185[12];
    uint    iidr47;     /* 0x507f0 - Internal Interrupt Destination Register */
                        /*           47 */
    char    res3170[12];
    uint    iivpr48;    /* 0x50800 - Internal Interrupt Vector/Priority */
                        /*           Register 48 */
    char    res3171[12];
    uint    iidr48;     /* 0x50810 - Internal Interrupt Destination Register */
                        /*           48 */
    char    res3172[12];
    uint    iivpr49;    /* 0x50820 - Internal Interrupt Vector/Priority */
                        /*           Register 49 */
    char    res3173[12];
    uint    iidr49;     /* 0x50830 - Internal Interrupt Destination Register */
                        /*           49 */
    char    res3174[12];
    uint    iivpr50;    /* 0x50840 - Internal Interrupt Vector/Priority */
                        /*           Register 50 */
    char    res3175[12];
    uint    iidr50;     /* 0x50850 - Internal Interrupt Destination Register */
                        /*           50 */
    char    res3176[12];
    uint    iivpr51;    /* 0x50860 - Internal Interrupt Vector/Priority */
                        /*           Register 51 */
    char    res3177[12];
    uint    iidr51;     /* 0x50870 - Internal Interrupt Destination Register */
                        /*           51 */
    char    res3178[12];
    uint    iivpr52;    /* 0x50880 - Internal Interrupt Vector/Priority */
                        /*           Register 52 */
    char    res3179[12];
    uint    iidr52;     /* 0x50890 - Internal Interrupt Destination Register */
                        /*           52 */
    char    res3180[12];
    uint    iivpr53;    /* 0x508a0 - Internal Interrupt Vector/Priority */
                        /*           Register 53 */
    char    res3181[12];
    uint    iidr53;     /* 0x508b0 - Internal Interrupt Destination Register */
                        /*           53 */
    char    res3182[12];
    uint    iivpr54;    /* 0x508c0 - Internal Interrupt Vector/Priority */
                        /*           Register 54 */
    char    res3183[12];
    uint    iidr54;     /* 0x508d0 - Internal Interrupt Destination Register */
                        /*           54 */
    char    res3184[12];
    uint    iivpr55;    /* 0x508e0 - Internal Interrupt Vector/Priority */
                        /*           Register 55 */
    char    res3185[12];
    uint    iidr55;     /* 0x508f0 - Internal Interrupt Destination Register */
                        /*           55 */
    char    res4170[12];
    uint    iivpr56;    /* 0x50900 - Internal Interrupt Vector/Priority */
                        /*           Register 56 */
    char    res4171[12];
    uint    iidr56;     /* 0x50910 - Internal Interrupt Destination Register */
                        /*           56 */
    char    res4172[12];
    uint    iivpr57;    /* 0x50920 - Internal Interrupt Vector/Priority */
                        /*           Register 57 */
    char    res4173[12];
    uint    iidr57;     /* 0x50930 - Internal Interrupt Destination Register */
                        /*           57 */
    char    res4174[12];
    uint    iivpr58;    /* 0x50940 - Internal Interrupt Vector/Priority */
                        /*           Register 58 */
    char    res4175[12];
    uint    iidr58;     /* 0x50950 - Internal Interrupt Destination Register */
                        /*           58 */
    char    res4176[12];
    uint    iivpr59;    /* 0x50960 - Internal Interrupt Vector/Priority */
                        /*           Register 59 */
    char    res4177[12];
    uint    iidr59;     /* 0x50970 - Internal Interrupt Destination Register */
                        /*           59 */
    char    res4178[12];
    uint    iivpr60;    /* 0x50980 - Internal Interrupt Vector/Priority */
                        /*           Register 60 */
    char    res4179[12];
    uint    iidr60;     /* 0x50990 - Internal Interrupt Destination Register */
                        /*           60 */
    char    res4180[12];
    uint    iivpr61;    /* 0x509a0 - Internal Interrupt Vector/Priority */
                        /*           Register 61 */
    char    res4181[12];
    uint    iidr61;     /* 0x509b0 - Internal Interrupt Destination Register */
                        /*           61 */
    char    res4182[12];
    uint    iivpr62;    /* 0x509c0 - Internal Interrupt Vector/Priority */
                        /*           Register 62 */
    char    res4183[12];
    uint    iidr62;     /* 0x509d0 - Internal Interrupt Destination Register */
                        /*           62 */
    char    res4184[12];
    uint    iivpr63;    /* 0x509e0 - Internal Interrupt Vector/Priority */
                        /*           Register 63 */
    char    res4185[12];
    uint    iidr63;     /* 0x509f0 - Internal Interrupt Destination Register */
                        /*           63 */
    char    res134[3084];
    uint    mivpr0;     /* 0x51600 - Messaging Interrupt Vector/Priority */
                        /*           Register 0 */
    char    res135[12];
    uint    midr0;      /* 0x51610 - Messaging Interrupt Destination Register */
                        /*           0 */
    char    res136[12];
    uint    mivpr1;     /* 0x51620 - Messaging Interrupt Vector/Priority */
                        /*           Register 1 */
    char    res137[12];
    uint    midr1;      /* 0x51630 - Messaging Interrupt Destination Register */
                        /*           1 */
    char    res138[12];
    uint    mivpr2;     /* 0x51640 - Messaging Interrupt Vector/Priority */
                        /*           Register 2 */
    char    res139[12];
    uint    midr2;      /* 0x51650 - Messaging Interrupt Destination Register */
                        /*           2 */
    char    res140[12];
    uint    mivpr3;     /* 0x51660 - Messaging Interrupt Vector/Priority */
                        /*           Register 3 */
    char    res141[12];
    uint    midr3;      /* 0x51670 - Messaging Interrupt Destination */
                        /*           Register 3 */

    char    res141_1[12];
    uint    mivpr4;     /* 0x51680 - Messaging Interrupt Vector/Priority */
                        /*           Register 4 */
    char    res141_2[12];
    uint    midr4;      /* 0x51690 - Messaging Interrupt Destination */
                        /*           Register 4 */
    char    res141_3[12];
    uint    mivpr5;     /* 0x516a0 - Messaging Interrupt Vector/Priority */
                        /*           Register 5 */
    char    res141_4[12];
    uint    midr5;      /* 0x516b0 - Messaging Interrupt Destination */
                        /*           Register 5 */
    char    res141_5[12];
    uint    mivpr6;     /* 0x516c0 - Messaging Interrupt Vector/Priority */
                        /*           Register 6 */
    char    res141_6[12];
    uint    midr6;      /* 0x516d0 - Messaging Interrupt Destination */
                        /*           Register 6 */
    char    res141_7[12];
    uint    mivpr7;     /* 0x516e0 - Messaging Interrupt Vector/Priority */
                        /*           Register 7 */
    char    res141_8[12];
    uint    midr7;      /* 0x516f0 - Messaging Interrupt Destination */
                        /*           Register 7 */
    char    rev1_res210[1292];

    uint    msivpr0;     /* 0x51c00 - Message Shared Interrupt Vector/Priority */
                         /*           Register 0 */
    char    res211[12];
    uint    msidr0;      /* 0x51c10 - Message Shared Interrupt Destination */
                         /*           Register 0 */
    char    res212[12];
    uint    msivpr1;     /* 0x51c20 - Message Shared Interrupt Vector/Priority */
                         /*           Register 1 */
    char    res213[12];
    uint    msidr1;      /* 0x51c30 - Message Shared Interrupt Destination */
                         /*           Register 1 */
    char    res214[12];
    uint    msivpr2;     /* 0x51c40 - Message Shared Interrupt Vector/Priority */
                         /*           Register 2 */
    char    res215[12];
    uint    msidr2;      /* 0x51c50 - Message Shared Interrupt Destination */
                         /*           Register 2 */
    char    res216[12];
    uint    msivpr3;     /* 0x51c60 - Message Shared Interrupt Vector/Priority */
                         /*           Register 3 */
    char    res217[12];
    uint    msidr3;      /* 0x51c70 - Message Shared Interrupt Destination */
                         /*           Register 3 */
    char    res218[12];
    uint    msivpr4;     /* 0x51c80 - Message Shared Interrupt Vector/Priority */
                         /*           Register 4 */
    char    res219[12];
    uint    msidr4;      /* 0x51c90 - Message Shared Interrupt Destination */
                         /*           Register 4 */
    char    res220[12];
    uint    msivpr5;     /* 0x51ca0 - Message Shared Interrupt Vector/Priority */
                         /*           Register 5 */
    char    res221[12];
    uint    msidr5;      /* 0x51cb0 - Message Shared Interrupt Destination */
                         /*           Register 5 */
    char    res222[12];
    uint    msivpr6;     /* 0x51cc0 - Message Shared Interrupt Vector/Priority */
                         /*           Register 6 */
    char    res223[12];
    uint    msidr6;      /* 0x51cd0 - Message Shared Interrupt Destination */
                         /*           Register 6 */
    char    res224[12];
    uint    msivpr7;     /* 0x51ce0 - Message Shared Interrupt Vector/Priority */
                         /*           Register 7 */
    char    res225[12];
    uint    msidr7;      /* 0x51cf0 - Message Shared Interrupt Destination */
                         /* Register 7 */
    char    res226[58188];

    /* Per-CPU Registers Block Base Address */
    uint    ipi0dr0;    /* 0x60040 - Processor 0 Interprocessor Interrupt */
                        /*           Dispatch Register 0 */
    char    res143[12];
    uint    ipi0dr1;    /* 0x60050 - Processor 0 Interprocessor Interrupt */
                        /*           Dispatch Register 1 */
    char    res144[12];
    uint    ipi0dr2;    /* 0x60060 - Processor 0 Interprocessor Interrupt */
                        /*           Dispatch Register 2 */
    char    res145[12];
    uint    ipi0dr3;    /* 0x60070 - Processor 0 Interprocessor Interrupt */
                        /*           Dispatch Register 3 */
    char    res146[12];
    uint    ctpr0;      /* 0x60080 - Current Task Priority Register for */
                        /*           Processor 0 */
    char    res147[12];
    uint    whoami0;    /* 0x60090 - Who Am I Register for Processor 0 */
    char    res148[12];
    uint    iack0;      /* 0x600a0 - Interrupt Acknowledge Register for */
                        /*           Processor 0 */
    char    res149[12];
    uint    eoi0;       /* 0x600b0 - End Of Interrupt Register for Processor */
                        /*           0 */
    char    res150[130892];
} ccsr_pic_t;

#define MPC8500_PIC_GCR_RESET        0x80000000    /* reset */
#define MPC8500_PIC_GCR_MIXED_MODE   0x20000000    /* mixed mode */
#define MPC8500_PIC_FRR_IRQ_MASK     0x07FF0000    /* irq no mask */
#define MPC8500_PIC_FRR_IRQ_SHIFT    16            /* irq no shift */
#define MPC8500_PIC_INTR_MASK        0x80000000    /* interrupt mask */
#define MPC8500_PIC_ACTIVE           0x40000000    /* Active */
#define MPC8500_PIC_ACTIVE_HI        0x00800000    /* Active-high or
                                                    positive edge-trig */
#define MPC8500_PIC_LEVEL_SENS       0x00400000    /* Level sensitive */
#define MPC8500_PIC_DUART_PRIORITY   0x00010000    /* Duart interrupt
                                                    priority    */
#define MPC8500_PIC_DUART_VECTOR     0x00000000    /* Duart interrupt
                                                    vector    */
#define MPC8500_PIC_XDSL_PRIORITY    0x00010000    /* G.shDSL interrupt
                                                    priority    */
#define MPC8500_PIC_XDSL_VECTOR      0x00000003    /* G.shDSL interrupt
                                                    vector    */
#define MPC8500_PIC_USB_PRIORITY     0x00010000    /* USB interrupt
                                                    priority    */
#define MPC8500_PIC_USB_VECTOR       0x00000004    /* USB interrupt
                                                    vector    */
#define MPC8500_PIC_ESW_PRIORITY     0x00010000    /* Switch interrupt
                                                    priority    */
#define MPC8500_PIC_ESW_VECTOR       0x00000006    /* Switch interrupt
                                                    vector    */
#define MPC8500_PIC_ILP_PRIORITY     0x00010000    /* ILP interrupt
                                                    priority    */
#define MPC8500_PIC_ILP_VECTOR       0x00000007    /* ILP interrupt
                                                    vector    */
#define MPC8500_PIC_CPM_PRIORITY     0x00010000    /* CPM interrupt
                                                    priority    */
#define MPC8500_PIC_CPM_VECTOR       0x00000007    /* CPM interrupt
                                                    vector    */
#define MPC8500_PIC_FAN_PRIORITY     0x00010000    /* Fan interupt
                                                    priority    */
#define MPC8500_PIC_FAN_VECTOR       0x00000009    /* Fan interrupt
                                                    vector    */
#define MPC8500_PIC_T1E1_PRIORITY    0x00010000    /* Comet interrupt
                                                    priority    */
#define MPC8500_PIC_T1E1_VECTOR      0x0000000A    /* Comet interrupt
                                                    vector    */
#define MPC8500_PIC_FE01_PRIORITY    0x00010000    /* CPM interrupt
                                                    priority    */
#define MPC8500_PIC_FE01_VECTOR      0x0000000B    /* CPM interrupt
                                                    vector */
/*
 * New index into the cause_table for shinkansen interrupts
 */

/* Priority and vectors for MSI interrupts, used by googy asic */
#define MPC8500_PIC_MSI_PRIORITY      0x00010000
#define MPC8500_PIC_MSI_VECTOR        0x00000010

/* Priority and vector for the MPC8548 internal PCIE interrupt */
#define MPC8500_PIC_PCI_EXP_PRIORITY  0x00010000
#define MPC8500_PIC_PCI_EXP_VECTOR    0x0000000F

#define MPC8500_PIC_EIDR_EP           0x80000000    /* External pin */
#define MPC8500_PIC_EIDR_CI           0x40000000    /* Critical interrupt */
#define MPC8500_PIC_IACK_VECTOR       0x0000FFFF    /* Vector */
#define MPC8500_PIC_EXT_INTR_NO       12
#define MPC8500_PIC_INT_INTR_NO       64 /* P1021 has 64 internal interrupts */
#define MPC8500_PIC_IP_INTR_NO        4
#define MPC8500_PIC_TIMER_INTR_NO     4
#define MPC8500_PIC_MSG_INTR_NO       4
#define MPC8500_PIC_MSI_INTR_NO       8
#define PIC_GCR_RESET_TIMEOUT       1000        /* 10 ms PIC reset */

/* External Interrupt Vector/Priority Register (EIVPRx) */
#define P1021_EIVPR_MASK            0x80000000
#define P1021_EIVPR_ACTIVITY        0x40000000
#define P1021_EIVPR_POLARITY        0x00800000
#define P1021_EIVPR_SENSE           0x04000000

/* P1021 defines */
/* 12 External Interrupt input sources */
#define MPC8500_PIC_EIVPR0_VECTOR    0
#define MPC8500_PIC_EIVPR1_VECTOR    1    
#define MPC8500_PIC_EIVPR2_VECTOR    2    
#define MPC8500_PIC_EIVPR3_VECTOR    3    
#define MPC8500_PIC_EIVPR4_VECTOR    4    
#define MPC8500_PIC_EIVPR5_VECTOR    5    
#define MPC8500_PIC_EIVPR6_VECTOR    6    
#define MPC8500_PIC_EIVPR7_VECTOR    7    
#define MPC8500_PIC_EIVPR8_VECTOR    8    
#define MPC8500_PIC_EIVPR9_VECTOR    9    
#define MPC8500_PIC_EIVPR10_VECTOR   10    
#define MPC8500_PIC_EIVPR11_VECTOR   11    

/* 64 Internal Interrupt sources */
#define MPC8500_PIC_IIVPR0_VECTOR    12    
#define MPC8500_PIC_IIVPR1_VECTOR    13    
#define MPC8500_PIC_IIVPR2_VECTOR    14    
#define MPC8500_PIC_IIVPR3_VECTOR    15    
#define MPC8500_PIC_IIVPR4_VECTOR    16    
#define MPC8500_PIC_IIVPR5_VECTOR    17    
#define MPC8500_PIC_IIVPR6_VECTOR    18    
#define MPC8500_PIC_IIVPR7_VECTOR    19    
#define MPC8500_PIC_IIVPR8_VECTOR    20    
#define MPC8500_PIC_IIVPR9_VECTOR    21    
#define MPC8500_PIC_IIVPR10_VECTOR    22    
#define MPC8500_PIC_IIVPR11_VECTOR    23    
#define MPC8500_PIC_IIVPR12_VECTOR    24    
#define MPC8500_PIC_IIVPR13_VECTOR    25    
#define MPC8500_PIC_IIVPR14_VECTOR    26    
#define MPC8500_PIC_IIVPR15_VECTOR    27    
#define MPC8500_PIC_IIVPR16_VECTOR    28    
#define MPC8500_PIC_IIVPR17_VECTOR    29    
#define MPC8500_PIC_IIVPR18_VECTOR    30    
#define MPC8500_PIC_IIVPR19_VECTOR    31    
#define MPC8500_PIC_IIVPR20_VECTOR    32    
#define MPC8500_PIC_IIVPR21_VECTOR    33    
#define MPC8500_PIC_IIVPR22_VECTOR    34    
#define MPC8500_PIC_IIVPR23_VECTOR    35    
#define MPC8500_PIC_IIVPR24_VECTOR    36    
#define MPC8500_PIC_IIVPR25_VECTOR    37    
#define MPC8500_PIC_IIVPR26_VECTOR    38    
#define MPC8500_PIC_IIVPR27_VECTOR    39    
#define MPC8500_PIC_IIVPR28_VECTOR    40    
#define MPC8500_PIC_IIVPR29_VECTOR    41    
#define MPC8500_PIC_IIVPR30_VECTOR    42    
#define MPC8500_PIC_IIVPR31_VECTOR    43    
#define MPC8500_PIC_IIVPR32_VECTOR    44    
#define MPC8500_PIC_IIVPR33_VECTOR    45    
#define MPC8500_PIC_IIVPR34_VECTOR    46    
#define MPC8500_PIC_IIVPR35_VECTOR    47    
#define MPC8500_PIC_IIVPR36_VECTOR    48    
#define MPC8500_PIC_IIVPR37_VECTOR    49    
#define MPC8500_PIC_IIVPR38_VECTOR    50    
#define MPC8500_PIC_IIVPR39_VECTOR    51    
#define MPC8500_PIC_IIVPR40_VECTOR    52    
#define MPC8500_PIC_IIVPR41_VECTOR    53    
#define MPC8500_PIC_IIVPR42_VECTOR    54    
#define MPC8500_PIC_IIVPR43_VECTOR    55    
#define MPC8500_PIC_IIVPR44_VECTOR    56    
#define MPC8500_PIC_IIVPR45_VECTOR    57    
#define MPC8500_PIC_IIVPR46_VECTOR    58
#define MPC8500_PIC_IIVPR47_VECTOR    59    
#define MPC8500_PIC_IIVPR48_VECTOR    60    
#define MPC8500_PIC_IIVPR49_VECTOR    61    
#define MPC8500_PIC_IIVPR50_VECTOR    62    
#define MPC8500_PIC_IIVPR51_VECTOR    63    
#define MPC8500_PIC_IIVPR52_VECTOR    64    
#define MPC8500_PIC_IIVPR53_VECTOR    65    
#define MPC8500_PIC_IIVPR54_VECTOR    66    
#define MPC8500_PIC_IIVPR55_VECTOR    67    
#define MPC8500_PIC_IIVPR56_VECTOR    68    
#define MPC8500_PIC_IIVPR57_VECTOR    69    
#define MPC8500_PIC_IIVPR58_VECTOR    70    
#define MPC8500_PIC_IIVPR59_VECTOR    71    
#define MPC8500_PIC_IIVPR60_VECTOR    72    
#define MPC8500_PIC_IIVPR61_VECTOR    73    
#define MPC8500_PIC_IIVPR62_VECTOR    74    
#define MPC8500_PIC_IIVPR63_VECTOR    75    

/* 8 Message-Shared Interrupt Sources */
#define MPC8500_PIC_MSIR0_VECTOR    96    
#define MPC8500_PIC_MSIR1_VECTOR    97    
#define MPC8500_PIC_MSIR2_VECTOR    98    
#define MPC8500_PIC_MSIR3_VECTOR    99    
#define MPC8500_PIC_MSIR4_VECTOR    100    
#define MPC8500_PIC_MSIR5_VECTOR    101    
#define MPC8500_PIC_MSIR6_VECTOR    102    
#define MPC8500_PIC_MSIR7_VECTOR    103    

/* Shared Message Signaled Interrupt Status Register(MSISR) */
#define MPC8500_PIC_MSISR_MSIR0_ACTIVE  0x00000001
#define MPC8500_PIC_MSISR_MSIR1_ACTIVE  0x00000002
#define MPC8500_PIC_MSISR_MSIR2_ACTIVE  0x00000004
#define MPC8500_PIC_MSISR_MSIR3_ACTIVE  0x00000008
#define MPC8500_PIC_MSISR_MSIR4_ACTIVE  0x00000010
#define MPC8500_PIC_MSISR_MSIR5_ACTIVE  0x00000020
#define MPC8500_PIC_MSISR_MSIR6_ACTIVE  0x00000040
#define MPC8500_PIC_MSISR_MSIR7_ACTIVE  0x00000080

#define MPC8500_PIC_PIR_RESET_CORE0     0x00000001 /* Processor core0 reset */
#define MPC8500_PIC_PIR_RESET_CORE1     0x00000002 /* Processor core1 reset */

/* Device-Specific Utilities Register Block(0xe_0000-0xf_ffff) */
typedef struct ccsr_gur {
    /* global utilities */
    uint    porpllsr;   /* 0xe0000 - POR PLL ratio status */
    uint    porbmsr;    /* 0xe0004 - POR boot mode status */
    uint    porimpscr;  /* 0xe0008 - POR I/O impedance status and control */
                        /*           register */
    uint    pordevsr;   /* 0xe000c - POR I/O device status register */
    uint    pordbgmsr;  /* 0xe0010 - POR debug mode status register */
    uint    pordevsr2;  /* 0xe0014 - POR I/O device status register 2 */
    char    res1[8];
    uint    gpporcr;    /* 0xe0020 - General purpose POR configuration */
                        /*           register */
    char    res2[60];
    uint    pmuxcr;     /* 0xe0060 - alternate function signal mux control */
    char    res3[12];
    uint    devdisr;    /* 0xe0070 - device disable control */
    char    res4[12];
    uint    powmgtcsr;  /* 0xe0080 - Power management control and status */
    char    res5[8];
    uint    pmcdr;      /* 0xe008c - Power management clock disable register */
    uint    mcpsumr;    /* 0xe0090 - machine check summary register */
    uint    rstrscr;    /* 0xe0094 - reset request status and control register */
    uint    ectrstcr;   /* 0xe0098 - exception reset control register */
    uint    autorstsr;  /* 0xe009c - automatic reset status register */
    uint    pvr;        /* 0xe00a0 - Processor Version register */
    uint    svr;        /* 0xe00a4 - System version register */ 
    char    res7[8];
    uint    rstcr;      /* 0xe00b0 - reset control register */ 
    char    res8[12];
    uint    lbcvselcr;  /* 0xe00c0 - LBC voltage select control register */ 
    char    res9[60];
    uint    cpodra;     /* 0xe0100 - Open drain register */
    uint    cpddata;    /* 0xe0104 - Data register */
    uint    cpdir1a;    /* 0xe0108 - Direction register */
    uint    cpdir2a;    /* 0xe010c - Direction register */
    uint    cppar1a;    /* 0xe0110 - Pin assignment register */
    uint    cppar2a;    /* 0xe0114 - Pin assignment register */
    char    res10[8];
    uint    cpodrb;     /* 0xe0120 - Open drain register */
    uint    cpddatb;    /* 0xe0124 - Data register */
    uint    cpdir1b;    /* 0xe0128 - Direction register */
    uint    cpdir2b;    /* 0xe012c - Direction register */
    uint    cppar1b;    /* 0xe0130 - Pin assignment register */
    uint    cppar2b;    /* 0xe0134 - Pin assignment register */
    char    res11[8];
    uint    cpodrc;     /* 0xe0140 - Open drain register */
    uint    cpddatc;    /* 0xe0144 - Data register */
    uint    cpdir1c;    /* 0xe0148 - Direction register */
    uint    cpdir2c;    /* 0xe014c - Direction register */
    uint    cppar1c;    /* 0xe0150 - Pin assignment register */
    uint    cppar2c;    /* 0xe0154 - Pin assignment register */
    char    res12[2512];
    uint    ddrclkdr;   /* 0xe0b28 - DDR clock disable register */ 
    char    res11_1[212];
    uint    errsumr;    /* 0xe0c00 - Error summary register */
    char    res12_1[508];
    uint    clkocr;     /* 0xe0e00 - clock out control register */ 
    char    res12_2[28];
    uint    tlutrgcr;   /* 0xe0e20 - TLU target control register */ 
    char    res13[476];
    /* performance monitor 0xE_1000-0xE_1FFF*/
    char    res14[4096];    /* fill this in later */
    /* Watchpoint and Debug 0xE_2000-0xF_FFFC */
    char    res15[122876];  /* fill this in later */
} ccsr_gur_t;

#define MPC8500_DEVDISR_PCI1_DIS     0x80000000
#define MPC8500_DEVDISR_PCI2_DIS     0x40000000
#define MPC8500_DEVDISR_CORE1_DIS    0x00002000
#define MPC8500_GPIOCR_PCIOUT_EN     0x00020000
#define MPC8500_GPIOCR_PCIIN_EN      0x00010000
#define MPC8500_GPIOCR_GPOUT_EN      0x00000200

#define MPC8500_MCPSUMR_WRS         0x00000004      /* watchdog reset */
#define MPC8500_MCPSUMR_MCP0_IN     0x00000001      /* mcp0 signal asserted */
#define MPC8500_MCPSUMR_MCP1_IN     0x00000010      /* mcp1 signal asserted */
#define MPC8500_MCPSUMR_MCP_CKSTP_P0    0x00000080      /* mcp0 chk stop asserted */
#define MPC8500_MCPSUMR_SRESET          0x00000002      /* sreset asserted */

#define MPC8500_PMUXCR_QE2        0x00002000
#define MPC8500_PMUXCR_QE5        0x00000400
#define MPC8500_PMUXCR_QE7        0x00000100
#define MPC8500_PMUXCR_QE8        0x00000080
#define MPC8500_PMUXCR_QE9        0x00000040
#define MPC8500_PMUXCR_QE11       0x00000010

#define MPC8500_CPDIR_DISABLED            0x00000000
#define MPC8500_CPDIR1_OUT(x)             ((0x1) << ((15-x)*2))
#define MPC8500_CPDIR1_IN(x)              ((0x2) << ((15-x)*2))
#define MPC8500_CPDIR1_INOUT(x)           ((0x3) << ((15-x)*2))
#define MPC8500_CPDIR2_OUT(x)             ((0x1) << ((31-x)*2))
#define MPC8500_CPDIR2_IN(x)              ((0x2) << ((31-x)*2))
#define MPC8500_CPDIR2_INOUT(x)           ((0x3) << ((31-x)*2))

#define MPC8500_CPPAR1(x,y)               ((y) << ((15-x)*2))     /* x=pin, y=value */
#define MPC8500_CPPAR2(x,y)               ((y) << ((31-x)*2))     /* x=pin, y=value */

#define MPC8500_GPPORCR_POR_CFG_VEC	0xFFFF0000      /* POR_CFG_VEC mask */



/**************************************************************************
 * QE Multi-user RAM (MURAM)                (0x10000 - 0x15FFF)
 **************************************************************************/

typedef struct {
    SPC(a, 0x3000)
    volatile uint8_t ucc5[256];        /* UCC5  PARAM  (0x13000 - 0x130FF) */
    SPC(b, 0x100)
    volatile uint8_t ucc7[256];        /* UCC7  PARAM  (0x13200 - 0x132FF) */
    SPC(c, 0x100)
    volatile uint8_t ucc1[256];        /* UCC1  PARAM  (0x13400 - 0x134FF) */
    SPC(d, 0x100)
    volatile uint8_t ucc3[256];        /* UCC3  PARAM  (0x13600 - 0x136FF) */
    SPC(e, 0x200)
    volatile uint8_t spi1[128];        /* SPI1  PARAM  (0x13900 - 0x1397F) */
    SPC(f, 0x80)
    volatile uint8_t timer[64];        /* Timer PARAM  (0x13A00 - 0x13A3F) */
    SPC(g, 0x25C0) 	                /* Reserved (0x15800 - 0x15FFF) */
} _PackedType qe_muram_t;

/*
 * QE Data structure
 */
typedef struct ccsr_qe_ {
    qe_iram_t        iram;	/* 0x0000 - 0x007F: Instruction RAM */
    qe_irq_t         irq;       /* 0x0080 - 0x00FF: Interrupt controller */
    qe_cp_t          cp;        /* 0x0100 - 0x01FF: Communication Processor */
    SPC(a, 0x200)		/* 0x0200 - 0x03FF: Reserved */
    qe_mux_t         mux;       /* 0x0400 - 0x043F: QE Multiplexer */
    qe_timer_t       timer;     /* 0x0440 - 0x047F: QE Timers */
    SPC(b, 0x40)		/* 0x0480 - 0x04BF: Reserved */
    qe_spi_t         spi1;      /* 0x04C0 - 0x04FF: SPI 1 */
    SPC(c, 0x140)               /* 0x0500 - 0x063F: Reserved */
    qe_brg_t         brg;       /* 0x0640 - 0x06BF: Baud rate generator */
    SPC(d, 0x40)                /* 0x06C0 - 0x06FF: Reserved */
    qe_si_t          si;        /* 0x0700 - 0x077F: SI */
    SPC(e, 0x880)               /* 0x0780 - 0x0FFF: Reserved */
    qe_sirt_t        sirt;      /* 0x1000 - 0x17FF: SI routing table */
    SPC(f, 0x800)               /* 0x1800 - 0x1FFF: Reserved */
    qe_ucc_t         ucc1;      /* 0x2000 - 0x21FF: UCC1 */
    qe_ucc_t         ucc3;      /* 0x2200 - 0x23FF: UCC3 */
    qe_ucc_t         ucc5;      /* 0x2400 - 0x25FF: UCC5 */
    qe_ucc_t         ucc7;      /* 0x2600 - 0x27FF: UCC7 */
    SPC(g, 0x600)               /* 0x2800 - 0x2DFF: Reserved */
    qe_utopi_t       utopia;    /* 0x2E00 - 0x2FFF: Multi-PHY controller */
    SPC(h, 0x1000)              /* 0x3000 - 0x3FFF: Reserved */
    qe_sdma_t        sdma;      /* 0x4000 - 0x407F: Serial DMA */
    SPC(i, 0x780)               /* 0x4080 - 0x47FF: Debug - Reserved */
    SPC(j, 0x800)		/* 0x4800 - 0x4FFF: Debug - IEEE 1588 Registers */
    SPC(k, 0x3000)              /* 0x5000 - 0x7FFF: Debug - Reserved */
    SPC(l, 0x8000) 	        /* 0x8000 - 0xFFFF: RAM Space - Reserved */
    qe_muram_t       muram;     /* 0x1_0000 - 0x1_5FFF: Multi-user RAM */
    SPC(m, 0x1A000) 	        /* 0x1_6000 - 0x3_FFFF: Reserved */

} ccsr_qe_t;

/**************************************************************************
 **************************************************************************
 * 		CCSR Block Base Address Map  (0x100000 - 0x1FFFFF)
 **************************************************************************
 **************************************************************************
 */
typedef struct immap {
    ccsr_local_ecm_t    im_local_ecm;   /* 0x0_0000-0x0_1fff */
    ccsr_ddr_t          im_ddr1;        /* 0x0_2000-0x0_2fff */
    ccsr_i2c_t          im_i2c1;        /* 0x0_3000-0x0_30ff */
    ccsr_i2c_t          im_i2c2;        /* 0x0_3100-0x0_31ff */
    char                res1[3584];     /* 0x0_3200-0x0_3fff */
    ccsr_duart_t        im_duart;       /* 0x0_4000-0x0_4fff */
    ccsr_lbc_t          im_lbc;         /* 0x0_5000-0x0_5fff */
    char                res2[4096];     /* 0x0_6000-0x0_6fff */
    ccsr_espi_t         im_espi;        /* 0x0_7000-0x0_7fff */
    char                res3[4092];     /* 0x0_8000-0x0_8fff */
    ccsr_pcie_t         im_pcie[2];     /* PCIE port1 im_pcie[0]:0x9000-0x9fff */
                                        /* PCIE port0 im_pcie[1]:0xa000-0xafff */
    char                res4[16384];   	/* 0x0_b000-0x0_efff */
    char         qe_intr_blk[0x1000];   /* 0x0_f000-0x0_ffff */
    char                res5[0x10000];  /* 0x0_fd00-0x1_5fff */
    ccsr_l2cache_t      im_l2cache;     /* 0x2_0000-0x2_0fff */
    ccsr_dma_t          im_dma;         /* 0x2_1000-0x2_1fff */
    ccsr_usb_t          im_usb[2];      /* USB port0 im_usb[0]:0x2_2000-0x2_2fff */
                                        /* USB port1 im_usb[1]:0x2_3000-0x2_3fff */
    ccsr_tsec_t         im_tsec1;       /* 0x2_4000-0x2_4fff */
    ccsr_tsec_t         im_tsec2;       /* 0x2_5000-0x2_5fff */
    ccsr_tsec_t         im_tsec3;       /* 0x2_6000-0x2_6fff */
    char                res6[0x7000];   /* 0x2_7000-0x2_bfff */
                                        /* 0x2_c000-0x2_cfff */
    ccsr_esdhc_t        im_esdhc;       /* 0x2_e000-0x2_efff */
    char                res7[0x1000];   /* 0x2_f000-0x2_ffff */
    char        	im_sec[0x10000]; /* 0x2_e000-0x2_efff */
    ccsr_pic_t          im_pic;         /* 0x4_0000-0x7_ffff */
    ccsr_qe_t		qe;		/* 0x8_0000-0xa_ffff */
    char 		res8[0x30000];  /* 0xb_0000-0xd_ffff */
    ccsr_gur_t          im_gur;         /* 0xe_0000-0xf_ffff */
} immap_t;

#define REGB ((immap_t *)ADRSPC_PQUICC_REGB)



/* MACCFG1 field descriptors */
#define MPC8500_TX_EN        0x80000000
#define MPC8500_SYNC_TX_EN   0x40000000
#define MPC8500_RX_EN        0x20000000
#define MPC8500_SYNC_RX_EN   0x10000000
#define MPC8500_TX_FLOW      0x08000000
#define MPC8500_RX_FLOW      0x04000000
#define MPC8500_MAC_LOOPBACK 0x00800000
#define MPC8500_RESET_TX_FUN 0x00008000
#define MPC8500_RESET_RX_FUN 0x00004000
#define MPC8500_RESET_TX_MC  0x00002000
#define MPC8500_RESET_RX_MC  0x00001000
#define MPC8500_SOFT_RESET   0x00000001

/*
 * SerDes1 Control Register field descriptors
 * set for 5/6 vdd-diff-pk, no equalization
 */
#define MPC8500_SRDS1CR_XMITEQ_MASK     0x0000FF00

#define MPC8500_SRDS1CR_XMITEQAD_VAL    0x00003000
#define MPC8500_SRDS1CR_XMITEQEH_VAL    0x00000300

/*
 * SerDes2 Control Register field descriptors
 * set for vdd-diff-pk, 1.33x relative amplitude
 */
#define MPC8500_SRDS2CR_XMITEQAB_VAL    0x00008000
#define MPC8500_SRDS2CR_XMITEQEF_VAL    0x00000800

/* The following structs and defines are from ferrari/old_pquicc.h */
/*
 * PowerQUICC Buffer Descriptors - A single generic buffer descriptor is used
 *   for most interfaces and protocols.  Exceptions are currently limited to
 *   the IDMA, which requires function-code registers, as well as independant
 *   pointers for source and destination data.
 */
typedef struct pquicc_bd_ {
    volatile ushort status;             /* Status and Control */
    volatile ushort length;             /* Length of Data in buffer */
    volatile uchar *buf_ptr;            /* Pointer to Data buffer */
} pquicc_bd_t __attribute__ ((aligned (8)));

/* end of ferrari/old_pquicc.h copy */

/* The following defines are from mantis/__pquicc_spi.h */
/*
 * Serial Peripheral Interface register macros
 *
 *
 * spi_spmode - SPI Mode Reg
 */
#define PQUICC_SPMODE_LOOP      0x4000        /* Loop Mode */
#define PQUICC_SPMODE_CI        0x2000        /* Clock Invert */
#define PQUICC_SPMODE_CP        0x1000        /* Clock Phase */
#define PQUICC_SPMODE_DIV16     0x0800        /* Divide by 16 */
#define PQUICC_SPMODE_REV       0x0400        /* Reverse Data */
#define PQUICC_SPMODE_MASTER    0x0200        /* Master/Slave */
#define PQUICC_SPMODE_EN        0x0100        /* Enable SPI */
#define PQUICC_SPMODE_LEN(y)    (((y)&0xf)<<4)    /* Character Length */
#define PQUICC_SPMODE_PM(y)     ((y)&0xf)    /* Prescale Modulus Select */

#define DEFAULT_SPI_MODE    PQUICC_SPMODE_REV  | PQUICC_SPMODE_MASTER | \
                            PQUICC_SPMODE_PM(1) | PQUICC_SPMODE_LEN(7)

#define QE_SPI1                         0x1
#define QE_SPI2                         0x2

/*
 * spi_spie - SPI Event Reg
 */
#define PQUICC_SPIE_MIME       0x20        /* Multi-Master Error */
#define PQUICC_SPIE_TXE        0x10        /* Tx Error */
#define PQUICC_SPIE_BSY        0x04        /* Busy Condition */
#define PQUICC_SPIE_TXB        0x02        /* Tx Buffer */
#define PQUICC_SPIE_RXB        0x01        /* Rx Buffer */

/*
 * spi_spim - SPI Mask Reg
 */
#define PQUICC_SPIM_MIME       0x20        /* Enbl Multi-Master Error */
#define PQUICC_SPIM_TXE        0x10        /* Enbl Tx Error */
#define PQUICC_SPIM_BSY        0x04        /* Enbl Busy Condition */
#define PQUICC_SPIM_TXB        0x02        /* Enbl Tx Buffer */
#define PQUICC_SPIM_RXB        0x01        /* Enbl Rx Buffer */

/*
 * spi_spcom - SPI Command Reg
 */
#define PQUICC_SPCOM_START    0x80        /* Start Transmit */

/*
 * Buffer Descriptor Flags - The various bits of the 'status' field of a buffer
 *   descriptor are described here.
 *
 *   All bits which do not have a specific name associated with them are
 *   reserved, and should be set to 0.
 */
#define PQUICC_SPI_RX_BDSTAT_EMPTY   0x8000    /* Empty */
#define PQUICC_SPI_RX_BDSTAT_BIT1    0x4000
#define PQUICC_SPI_RX_BDSTAT_WRAP    0x2000    /* Wrap */
#define PQUICC_SPI_RX_BDSTAT_RUPT    0x1000    /* Interrupt */
#define PQUICC_SPI_RX_BDSTAT_LAST    0x0800    /* Last Character */
#define PQUICC_SPI_RX_BDSTAT_BIT5    0x0400
#define PQUICC_SPI_RX_BDSTAT_CM      0x0200    /* Continuous Mode */
#define PQUICC_SPI_RX_BDSTAT_BIT7    0x0100
#define PQUICC_SPI_RX_BDSTAT_BIT8    0x0080
#define PQUICC_SPI_RX_BDSTAT_BIT9    0x0040
#define PQUICC_SPI_RX_BDSTAT_BIT10   0x0020
#define PQUICC_SPI_RX_BDSTAT_BIT11   0x0010
#define PQUICC_SPI_RX_BDSTAT_BIT12   0x0008
#define PQUICC_SPI_RX_BDSTAT_BIT13   0x0004
#define PQUICC_SPI_RX_BDSTAT_OV      0x0002    /* Overrun */
#define PQUICC_SPI_RX_BDSTAT_ME      0x0001    /* Multimaster Error */

#define PQUICC_SPI_TX_BDSTAT_READY   0x8000    /* Ready */
#define PQUICC_SPI_TX_BDSTAT_BIT1    0x4000
#define PQUICC_SPI_TX_BDSTAT_WRAP    0x2000    /* Wrap */
#define PQUICC_SPI_TX_BDSTAT_RUPT    0x1000    /* Interrupt */
#define PQUICC_SPI_TX_BDSTAT_LAST    0x0800    /* Last Character */
#define PQUICC_SPI_TX_BDSTAT_BIT5    0x0400
#define PQUICC_SPI_TX_BDSTAT_CM      0x0200    /* Continuous Mode */
#define PQUICC_SPI_TX_BDSTAT_BIT7    0x0100
#define PQUICC_SPI_TX_BDSTAT_BIT8    0x0080
#define PQUICC_SPI_TX_BDSTAT_BIT9    0x0040
#define PQUICC_SPI_TX_BDSTAT_BIT10   0x0020
#define PQUICC_SPI_TX_BDSTAT_BIT11   0x0010
#define PQUICC_SPI_TX_BDSTAT_BIT12   0x0008
#define PQUICC_SPI_TX_BDSTAT_BIT13   0x0004
#define PQUICC_SPI_TX_BDSTAT_UN      0x0002    /* Underrun */
#define PQUICC_SPI_TX_BDSTAT_ME      0x0001    /* Multimaster Error */

/*
 * Misc constants and macros
 */
#define PQUICC_SPI_SPIN_MAX     0x4000  /* max time for SPI cmd to complete */
#define PQUICC_SPI_DEFAULT_MTU       4  /* default largest SPI comment */

#define PQUICC_DEFAULT_SPI_MODE       ( PQUICC_SPMODE_DIV16     | \
                                        PQUICC_SPMODE_REV       | \
                                        PQUICC_SPMODE_MASTER    | \
                                        PQUICC_SPMODE_LEN(9) )

/* endof mantis/__pquicc_spi.h copy */

#endif /*__MPC8500_IMMAP__*/



/*------------------------------------------------------------------------------
 * $Log: p1021_immap.h,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.3  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.2  2011/08/18 19:43:24  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.4  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.3  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.2  2011/05/09 21:07:23  huanngo
 * Update code for HDLC over TDM loopback
 *
 * Revision 1.1.2.1  2011/05/02 23:33:22  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

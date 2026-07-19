/* $Id: diag_dimm_lib.h,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_dimm_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_dimm_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_DIMM_LIB_H__
#define __DIAG_DIMM_LIB_H__

typedef struct ddr3_dimm {
    uchar size;                 /* 0x00: SPD Byte Used/SPD Byte Total/CRC Coverage */
    uchar revision;             /* 0x01: SPD Revision */
    uchar dram_type;            /* 0x02: DRAM Device Type */
    uchar module_type;          /* 0x03: Module Type */
    uchar desity_bank;          /* 0x04: SDRAM Density and Banks */
    uchar addressing;           /* 0x05: SDRAM Addressing */
    uchar vdd;                  /* 0x06: Module Nominal Voltage, VDD */
    uchar organization;         /* 0x07: Module Organization */
    uchar bus_width;            /* 0x08: Module Memory Bus Width */
    uchar ftb_dividend;         /* 0x09: Fine Timebase(FTB) Dividend / Divisor */
    uchar mtb_dividend;         /* 0x0a: Medium Timebase(MTB) Dividend */
    uchar mtb_divisor;          /* 0x0b: Medium Timebase(MTB) Divisor */
    uchar tckmin;               /* 0x0c: SDRAM Minimum Cycle Time(tCKmin) */
    uchar res1;                 /* 0x0d: Reserved */
    uchar cas_lsb;              /* 0x0e: CAS Latencies Supported, LSB */
    uchar cas_msb;              /* 0x0f: CAS Latencies Supported, MSB */
    uchar taamin;               /* 0x10: MIN CAS Latency(tAAmin) */
    uchar twrmin;               /* 0x11: MIN Write Recovery(tWRmin) */
    uchar trcdmin;              /* 0x12: MIN RAS# to CAS# Delay(tRCDmin) */
    uchar trrdmin;              /* 0x13: MIN Row Act. to Row Act. Delay(tRRDmin) */
    uchar trpmin;               /* 0x14: MIN Row Precharge Delay(tRPmin) */
    uchar u_nib_tras_trc;       /* 0x15: Upper Nibbles for tRAS and tRC */
    uchar trasmin_lsb;          /* 0x16: MIN Act. to Precharge Delay(tRASmin), LSB */
    uchar trcmin_lsb;           /* 0x17: MIN Act. to Act./Refresh Delay(tRCmin), LSB */
    uchar trfcmin_lsb;          /* 0x18: MIN Refresh Recovery Delay(tRFCmin), LSB */
    uchar trfcmin_msb;          /* 0x19: MIN Refresh Recovery Delay(tRFCmin), MSB */
    uchar twtrmin;              /* 0x1a: MIN Internal WR to RD Cmd Delay(tWTRmin) */
    uchar trtpmin;              /* 0x1b: MIN Internal RD to Percharge Cmd Delay(tRTPmin) */
    uchar u_nib_tfaw;           /* 0x1c: Upper Nibble for tFAW */
    uchar tfawmin;              /* 0x1d: MIN Four Activate WIN Delay(tFAWmin) */
    uchar opt_features;         /* 0x1e: SDRAM Optional Features */
    uchar therm_ref_opt;        /* 0x1f: SDRAM Thermal and Refresh Options */
    uchar therm_sensor;         /* 0x20: Module Thermal Sensor */
    uchar sdram_type;           /* 0x21: SDRAM Device Type */
    uchar res2[26];             /* 0x22~0x3b: Reserved, General Section */
    uchar type_spec[57];        /* 0x3c~0x74: Type Spec Sec., Index. by Key Byte 3 */
    uchar mod_jedec_id[2];      /* 0x75~0x76: Module Manufacturer's JEDEC ID Code */
    uchar manu_location;        /* 0x77: Manufacturer's Location */
    uchar manu_date[2];         /* 0x78~0x79: Manufacturer's Date */
    uchar serial_no[4];         /* 0x7a~0x7d: Module Serial Number */
    uchar crc[2];               /* 0x7e~0x7f: Cyclical Redundancy Code */
    uchar part_no[18];          /* 0x80~0x91: Module Part Number */
    uchar rev_code[2];          /* 0x92~0x93: Module Revision Code */
    uchar dram_jedec_id[2];     /* 0x94~0x95: DRAM Manufacturer's JEDEC ID Code */
    uchar manu_spec[26];        /* 0x96~0xaf: Manufacturer's Specific Data */
    uchar customer_use[80];     /* 0xb0~0xff: Open for customer use */
} ddr3_dimm_t;

/* Definition of DIMM mem size */
/* Ref: JEDEC_DDR3_SPD_Specification_Rev1.0_R20.pdf */
#define DIMM_512MB 1
#define DIMM_1GB   2
#define DIMM_2GB   3
#define DIMM_4GB   4
#define DIMM_8GB   5
#define DIMM_16GB  6

/* Definition of DIMM dump type */
#define DIMM_DUMP_ALL      0
#define DIMM_DUMP_RAW      1
#define DIMM_DUMP_MEMSIZE  2

/* Definition of DIMM number */
#define UNKNOWN_DIMM_NO   3

/* Externs */
extern int init_dimm_i2c_struct(n2g_i2c_dev_t *);
extern int dump_dimm_data(n2g_i2c_dev_t *, uint32_t);
extern int read_eeprom_block(unsigned int, unsigned int, uchar *);

#endif   /* __DIAG_DIMM_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_dimm_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

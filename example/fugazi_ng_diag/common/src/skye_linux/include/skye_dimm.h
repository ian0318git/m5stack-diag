/* $Id: skye_dimm.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_dimm.h,v $
 *------------------------------------------------------------------
 * Filename: skye_dimm.h
 *
 * Description: Header file of DDR DIMM info.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SHRINKRAY_DIMM_H__
#define __SHRINKRAY_DIMM_H__

typedef struct ddr3_spd {
    uchar size;            /* 0x00: SPD Byte Used/SPD Byte Total/CRC Coverage */
    uchar revision;        /* 0x01: SPD Revision */
    uchar dram_type;       /* 0x02: DRAM Device Type */
    uchar module_type;     /* 0x03: Module Type */
    uchar desity_bank;     /* 0x04: SDRAM Density and Banks */
    uchar addressing;      /* 0x05: SDRAM Addressing */
    uchar vdd;	           /* 0x06: Module Nominal Voltage, VDD */
    uchar organization;    /* 0x07: Module Organization */
    uchar bus_width;       /* 0x08: Module Memory Bus Width */
    uchar ftb_dividend;    /* 0x09: Fine Timebase(FTB) Dividend / Divisor */
    uchar mtb_dividend;    /* 0x0a: Medium Timebase(MTB) Dividend */
    uchar mtb_divisor;     /* 0x0b: Medium Timebase(MTB) Divisor */
    uchar tckmin;          /* 0x0c: SDRAM Minimum Cycle Time(tCKmin) */
    uchar res1;            /* 0x0d: Reserved */
    uchar cas_lsb;         /* 0x0e: CAS Latencies Supported, LSB */
    uchar cas_msb;         /* 0x0f: CAS Latencies Supported, MSB */
    uchar taamin;          /* 0x10: MIN CAS Latency(tAAmin) */
    uchar twrmin;          /* 0x11: MIN Write Recovery(tWRmin) */
    uchar trcdmin;         /* 0x12: MIN RAS# to CAS# Delay(tRCDmin) */
    uchar trrdmin;         /* 0x13: MIN Row Act. to Row Act. Delay(tRRDmin) */
    uchar trpmin;          /* 0x14: MIN Row Precharge Delay(tRPmin) */
    uchar u_nib_tras_trc;  /* 0x15: Upper Nibbles for tRAS and tRC */
    uchar trasmin_lsb;     /* 0x16: MIN Act. to Precharge Delay(tRASmin), LSB */
    uchar trcmin_lsb;      /* 0x17: MIN Act. to Act./Refresh Delay(tRCmin), LSB */
    uchar trfcmin_lsb;     /* 0x18: MIN Refresh Recovery Delay(tRFCmin), LSB */
    uchar trfcmin_msb;     /* 0x19: MIN Refresh Recovery Delay(tRFCmin), MSB */
    uchar twtrmin;         /* 0x1a: MIN Internal WR to RD Cmd Delay(tWTRmin) */
    uchar trtpmin;         /* 0x1b: MIN Internal RD to Percharge Cmd Delay(tRTPmin) */
    uchar u_nib_tfaw;      /* 0x1c: Upper Nibble for tFAW */
    uchar tfawmin;         /* 0x1d: MIN Four Activate WIN Delay(tFAWmin) */
    uchar opt_features;    /* 0x1e: SDRAM Optional Features */
    uchar therm_ref_opt;   /* 0x1f: SDRAM Thermal and Refresh Options */
    uchar therm_sensor;    /* 0x20: Module Thermal Sensor */
    uchar sdram_type;      /* 0x21: SDRAM Device Type */
    uchar res2[26];        /* 0x22~0x3b: Reserved, General Section */
    uchar type_spec[57];   /* 0x3c~0x74: Type Spec Sec., Index. by Key Byte 3 */
    uchar mod_jedec_id[2]; /* 0x75~0x76: Module Manufacturer's JEDEC ID Code */
    uchar manu_location;   /* 0x77: Manufacturer's Location */
    uchar manu_date[2];    /* 0x78~0x79: Manufacturer's Date */
    uchar serial_no[4];    /* 0x7a~0x7d: Module Serial Number */
    uchar crc[2];          /* 0x7e~0x7f: Cyclical Redundancy Code */
    uchar part_no[18];     /* 0x80~0x91: Module Part Number */
    uchar rev_code[2];     /* 0x92~0x93: Module Revision Code */
    uchar dram_jedec_id[2];/* 0x94~0x95: DRAM Manufacturer's JEDEC ID Code */
    uchar manu_spec[26];   /* 0x96~0xaf: Manufacturer's Specific Data */
    uchar customer_use[80];/* 0xb0~0xff: Open for customer use */
} ddr3_spd_t;

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

/* Definition of DIMM SPD size */
#define DDR3_SPD_SIZE   256

/* Definition of DIMM Thermal Sensor registers' offset */
#define DIMM_TS_CAP_REG_OFF     0x00   /* Capabilities Reg. */
#define DIMM_TS_CONF_REG_OFF    0x01   /* Configuration Reg. */
#define DIMM_TS_HIGH_REG_OFF    0x02   /* Temp. High Limit Reg. */
#define DIMM_TS_LOW_REG_OFF     0x03   /* Temp. Low Limit Reg. */
#define DIMM_TS_TCRIT_REG_OFF   0x04   /* TCRIT Limit Reg. (for Critical Temp.) */
#define DIMM_TS_TEMP_REG_OFF    0x05   /* (Current) Ambitent Temp. Reg. */
#define DIMM_TS_ID_REG_OFF      0x06   /* Manufacturer ID Reg. */
#define DIMM_TS_REV_REG_OFF     0x07   /* Device ID and Revision number Reg. */
#define DIMM_TS_REV1_REG_OFF    0x08   /* (0x08 ~ 0x0F) Vendor specific info Reg. */
#define DIMM_TS_REV2_REG_OFF    0x09
#define DIMM_TS_REV3_REG_OFF    0x0A
#define DIMM_TS_REV4_REG_OFF    0x0B
#define DIMM_TS_REV5_REG_OFF    0x0C
#define DIMM_TS_REV6_REG_OFF    0x0D
#define DIMM_TS_REV7_REG_OFF    0x0E
#define DIMM_TS_REV8_REG_OFF    0x0F


#endif /* __SHRINKRAY_DIMM_H__ */

/*------------------------------------------------------------------
$Log: skye_dimm.h,v $
Revision 1.2  2015/05/25 03:59:10  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:28  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:39  palin2
Initial check-in Skye module side Diag code.

------------------------------------------------------
shrinkray_dimm.h:
Revision 1.2  2014/02/27 15:01:10  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.3  2013/11/13 01:34:26  palin2
Add definitions for ShrinkRay DIMM thermal sensor.

Revision 1.1.4.2  2013/09/13 06:59:59  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.1  2013/07/02 03:14:18  palin2
Add support to dump DDR DIMM SPD info.

------------------------------------------------------
$Endlog$
*/


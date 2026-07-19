// $Id: fpga_smartfan.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
// $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/fpga_smartfan.h,v $
//------------------------------------------------------------------------------
// Copyright (c) 2019-2020 by Cisco Systems, Inc.
// All rights reserved.
//
// File: fpga_smartfan.h
//
// Description:
//   Header file for Smartfan registers in the FPGA
//
// Author:  Niels-Peder Jensen (npjensen@cisco.com)
//
//------------------------------------------------------------------------------


#ifndef _FPGA_SMARTFAN_H_
#define _FPGA_SMARTFAN_H_

#include <string.h>
#include "dash_fpga.h"

#define fpga_reg_write(address,data)    ((*address)=data)
#define fpga_reg_read(address)          (*address)
#define fpga_mem_cpy(dest,src,size)     memcpy((dest),(src),(size))
#define fpga_memset(dest,value,size)    memset((dest),(value),(size))
#define FPGA_SMARTFAN_WAIT_TIME         500

// Structures
//------------------------------------------------------------------------------
typedef struct _smartfan_rawdata_t {
  unsigned char status;
  unsigned char raw_data[256];
} smartfan_rawdata_t;

typedef struct _smartfan_parsedata_t {
  unsigned char cisco_cpn[16];                              // e.g. 33-123456-01
  unsigned char cisco_serial_number[12];                    // e.g. CSC1903ABCD
  unsigned char cisco_rev[4];                               // e.g. A0
  unsigned char mfg_name[10];
  unsigned char mfg_pn[20];
  unsigned char mfg_assy_location[10];
  unsigned char mfg_datecode[12];
  unsigned char mfg_sn[16];
  unsigned char pad[30];                                    // Pad up to 128 for future use
} smartfan_parsedata_t;


// Extern
//------------------------------------------------------------------------------
extern void fpga_smartfan__start (unsigned char);
extern unsigned char fpga_smartfan__parse (smartfan_parsedata_t *, smartfan_rawdata_t *);
extern unsigned char fpga_smartfan__read_fsm (unsigned, smartfan_rawdata_t *, smartfan_parsedata_t *, unsigned int);
extern void fpga_smartfan_display (unsigned int);


// Defines - status
#define DATA_NOT_VALID                            0         // Initial state
#define DATA_IS_VALID                             1         // Data was read successfully
#define DATA_UNABLE_TO_READ                       4         // Data read was unsuccesfully (Not Cisco SmartFan Compliant)
#define DATA_READ_IN_PROGESS                      5         // First Read attempt


// Define Smartfan Type Fields
#define SF_TLV_SOD                                0x15      // Start Of Data
#define SF_TLV_PAD                                0xFA      // Data Padding
#define SF_TLV_EOD                                0xFF      // End Of Data

#define SF_TLV_CRC16                              0xCCCC    // Data Frame CRC
#define SF_TLV_CISCO_CPN                          0x0101    // Cisco Part Number
#define SF_TLV_CISCO_SN                           0x0103    // Cisco Serial number
#define SF_TLV_CISCO_CPN_REV                      0x0104    // Cisco Part Number Revision

#define SF_TLV_MFG_NAME                           0x0A01
#define SF_TLV_MFG_PART_NUMBER                    0x0A02
#define SF_TLV_MFG_ASSEMBLY_LOCATION              0x0A03
#define SF_TLV_MFG_DATECODE                       0x0A04
#define SF_TLV_MFG_SN                             0x0A05

// macro
#define MACRO__SET_DATA_NOT_VALID(raw_ptr)        fpga_reg_write(&((raw_ptr)->status),DATA_NOT_VALID);

#endif

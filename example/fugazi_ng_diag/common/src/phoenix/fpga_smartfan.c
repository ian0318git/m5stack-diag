// $Id: fpga_smartfan.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
// $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/fpga_smartfan.c,v $
//------------------------------------------------------------------
// Copyright (c) 2019 by Cisco Systems, Inc.
// All rights reserved.
//
// File: fpga_smartfan.c
//
// Description:
//    TBD
//
// Author:  Niels-Peder Jensen (npjensen@cisco.com)
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "fpga_smartfan.h"
#include "proto.h"
#include <stdlib.h>


//------------------------------------------------------------------------------
void fpga_smartfan__start (unsigned char fan_num) {
//------------------------------------------------------------------------------
//  Description:
//    Start the smartfan statemachine
//  Parameters:
//    fan_num : Number of the fan to get data from
//  Returns:
//    None
//------------------------------------------------------------------------------

  if (!MACRO_FPGA_SMARTFAN_BUSY) {
    smartfan_start(fan_num);
  }
}


//------------------------------------------------------------------------------
unsigned char fpga_smartfan__parse (smartfan_parsedata_t *parsed, smartfan_rawdata_t *raw) {
//------------------------------------------------------------------------------
//  Description:
//    Read data from smartfan FIFO and store internally for debug.
//    Parse the data from the smartfan FIFO.
//  Parameters:
//    parsed  : Parsed data structure
//    raw     : Raw data
//  Returns:
//    0       : Found data
//    1       : Did not find any data - no Start-Of-Data (SoD)
//------------------------------------------------------------------------------

  unsigned char   fifo_data_byte;
  unsigned char   type_value_len = 0;   //
  unsigned short  type;
  unsigned char   length = 0;
  unsigned char   parsed_data_offset = 0;
  unsigned char   parsing = 0;          // Set to 1 when SoD found

  unsigned short  raw_byte_num = 0;

  if (!MACRO_FPGA_SMARTFAN_BUSY) {
    fpga_memset(parsed, 0, sizeof(smartfan_parsedata_t));
    fpga_memset(raw,    0, sizeof(smartfan_rawdata_t));

    while (!MACRO_FPGA_SMARTFAN_FIFO_EMPTY && raw_byte_num<sizeof(raw->raw_data)) {
      fifo_data_byte = smartfan_fifo_rd() & 0xFF;
      if (raw_byte_num <=255) {
        fpga_reg_write(&raw->raw_data[raw_byte_num], fifo_data_byte);
        raw_byte_num++;
      }

      if ((parsing==1) && (type_value_len == 3) && (length > 0)) {
        // Parse Type
        switch (type) {
          case SF_TLV_CRC16:
            // NPJENSEN: To Be Implemented
            break;
          case SF_TLV_CISCO_CPN:
            if (parsed_data_offset < sizeof(parsed->cisco_cpn)) {
              fpga_reg_write(&parsed->cisco_cpn[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_CISCO_SN:
            if (parsed_data_offset < sizeof(parsed->cisco_serial_number)) {
              fpga_reg_write(&parsed->cisco_serial_number[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_CISCO_CPN_REV:
            //if (parsed_data_offset < sizeof(parsed->cisco_rev)) {
            if (parsed_data_offset < 2) {
              fpga_reg_write(&parsed->cisco_rev[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_MFG_NAME:
            if (parsed_data_offset < sizeof(parsed->mfg_name)) {
              fpga_reg_write(&parsed->mfg_name[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_MFG_PART_NUMBER:
            if (parsed_data_offset < sizeof(parsed->mfg_pn)) {
              fpga_reg_write(&parsed->mfg_pn[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_MFG_ASSEMBLY_LOCATION:
            if (parsed_data_offset < sizeof(parsed->mfg_assy_location)) {
              fpga_reg_write(&parsed->mfg_assy_location[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_MFG_DATECODE:
            if (parsed_data_offset < sizeof(parsed->mfg_datecode)) {
              fpga_reg_write(&parsed->mfg_datecode[parsed_data_offset],fifo_data_byte);
            }
            break;
          case SF_TLV_MFG_SN:
            if (parsed_data_offset < sizeof(parsed->mfg_sn)) {
              fpga_reg_write(&parsed->mfg_sn[parsed_data_offset],fifo_data_byte);
            }
            break;

          default:
            break;
        }

        length--;
        parsed_data_offset++;
        if (length == 0) {
          type_value_len = 0;
          parsed_data_offset = 0;
        }

      } else if (type_value_len == 0) {
        // Check first byte of Type.
        // Some fields have special meaning:
        if (fifo_data_byte == SF_TLV_SOD) {
          parsing = 1;
        } else if (fifo_data_byte == SF_TLV_PAD) {
          // do nothing
        } else if (fifo_data_byte == SF_TLV_EOD) {
          parsing = 2;
          type_value_len = 0;
        } else if (parsing) {
          // First byte of the Type
          type = fifo_data_byte;
          type_value_len = 1;
        }

      } else if (type_value_len == 1) {
        // Add second byte of the Type
        type = ((type&0xFF)<<8) | fifo_data_byte;
        type_value_len = 2;

      } else if (type_value_len == 2) {
        // Add Length
        length = fifo_data_byte;
        type_value_len = 3;
      }
    }

  }
  if (parsing == 2)
    return 1;
  return 0;
}


//------------------------------------------------------------------------------
unsigned char fpga_smartfan__read_fsm ( unsigned fan,
                                        smartfan_rawdata_t *raw_ptr,
                                        smartfan_parsedata_t *parsed_ptr,
                                        unsigned int timeout) {
//------------------------------------------------------------------------------
//  Description:
//    Read
//  Parameters:
//    fan     : Fan
//    raw     : Pointer to raw data
//    parsed  : Pointer to parsed data.  Will be populated if good data is read
//    timeout : Timeout to use - times out after calling this function more than
//              'timeout' times with no valid result
//  Returns:
//    current status
//------------------------------------------------------------------------------

    unsigned int current_timeout = timeout;

    while (current_timeout > 0) {
        if (!MACRO_FPGA_SMARTFAN_BUSY) {
          fpga_smartfan__start(fan);
          fpga_reg_write(&(raw_ptr->status),DATA_READ_IN_PROGESS);
          break;
        }
        msleep(FPGA_SMARTFAN_WAIT_TIME);
        current_timeout--;
    }

    if (current_timeout < 0) {
        fpga_reg_write(&(raw_ptr->status),DATA_UNABLE_TO_READ);
        return fpga_reg_read(&(raw_ptr->status));
    }


    current_timeout = timeout;
    while (current_timeout > 0) {
        if (!MACRO_FPGA_SMARTFAN_BUSY) {
            // Finished. Parse data
            if (!fpga_smartfan__parse (parsed_ptr, raw_ptr)) {
                fpga_reg_write(&(raw_ptr->status),DATA_UNABLE_TO_READ);
                return fpga_reg_read(&(raw_ptr->status));
            } else {
                fpga_reg_write(&(raw_ptr->status),DATA_IS_VALID);
                return fpga_reg_read(&(raw_ptr->status));
            }
        }
        msleep(FPGA_SMARTFAN_WAIT_TIME);
        current_timeout--;
    }

    fpga_reg_write(&(raw_ptr->status),DATA_UNABLE_TO_READ);
    return fpga_reg_read(&(raw_ptr->status));

}


void fpga_smartfan_display (unsigned fan)
{
    smartfan_rawdata_t *raw_ptr = malloc(sizeof(smartfan_rawdata_t));
    smartfan_parsedata_t *parsed_ptr = malloc(sizeof(smartfan_parsedata_t));
    unsigned int timeout=10, ret = 0;

    ret = fpga_smartfan__read_fsm(fan, raw_ptr, parsed_ptr, timeout);
    if (ret == DATA_IS_VALID) {
        printf("\nSmartfan %d : Valid\n", fan+1);
        printf("cisco cpn : %s\n", parsed_ptr->cisco_cpn);
        printf("cisco rev : %s\n", parsed_ptr->cisco_rev);
        printf("cisco sn : %s\n", parsed_ptr->cisco_serial_number);
        printf("mfg name : %s\n", parsed_ptr->mfg_name);
        printf("mfg pn : %s\n", parsed_ptr->mfg_pn);
        printf("mfg assy location : %s\n", parsed_ptr->mfg_assy_location);
        printf("mfg datecode : %s\n", parsed_ptr->mfg_datecode);
        printf("mfg sn : %s\n", parsed_ptr->mfg_sn);
    } else {
        printf("\nSmartfan %d : Data unable to read\n", fan+1);
    }
    free(raw_ptr);
    free(parsed_ptr);
}


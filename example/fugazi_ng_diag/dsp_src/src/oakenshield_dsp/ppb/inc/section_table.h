/* $Id: section_table.h,v 1.2 2017/07/28 07:58:38 harrchan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/section_table.h,v $
 *------------------------------------------------------------------
 * section_table.h
 *      Oakenshield project for dsp fw image checksum
 *              needed for gen fw image, port from DSP
 *
 * Apr 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------
 * section_table.h  Image section attributes table
 *
 * March 2008, Jim Muir
 *
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __SECTION_TABLE_H
#define __SECTION_TABLE_H

/*
 * A table to keep section information for
 * checksumming
 */
#define CHECKSUM_MAGIC 0xCE42CE42
#define CHECKSUM_MAGIC_INVALID 0xFFFFFFFF

#define CHECKSUM_UNCOMPUTED 0

/*
 * section_attributes
 */
#define SECTION_CODE 1
#define SECTION_READONLY 2
#define SECTION_SAVE 4

#define MAX_SECTIONS 50         
typedef struct section_table_s {
    uint32_t checksum_magic;
    struct {
        uint32_t section_length;          /* length in bytes */
        uint32_t section_start;           /* start address */
        uint32_t section_attributes;      /* flags bitmask */
        uint32_t section_sum;             /* little endian Fletcher's checksum */
    } section_table_entry[MAX_SECTIONS];
} section_table_t;

/*
 * DSS local memory addresses
 */
#ifdef _SP27XX_
#define DSS0_LOCAL_MEM_START 0x80000000
#define DSS1_LOCAL_MEM_START 0x82000000
#define DSS2_LOCAL_MEM_START 0x84000000
#define DSS3_LOCAL_MEM_START 0x86000000
#define DSS_LOCAL_MEM_SIZE   0x00040000
#else
#define DSS0_LOCAL_MEM_START 0x80000000
#define DSS1_LOCAL_MEM_START 0x88000000
#define DSS2_LOCAL_MEM_START 0x90000000
#define DSS_LOCAL_MEM_SIZE   0x00040000
#endif


#endif /* __SECTION_TABLE_H */
/*
 * $Log: section_table.h,v $
 * Revision 1.2  2017/07/28 07:58:38  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:32  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1  2012/04/18 09:50:18  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


/* $Id: dreamliner_fpga.h,v 1.2 2019/12/11 10:10:25 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/dreamliner_fpga.h,v $
 *------------------------------------------------------------------
 *
 * dreamliner_fpga.h - This file contains defines for Dreamliner 
 *                    FPGA
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#define FPGA_SMI_WR_ADDR_MSB  0x00
#define FPGA_SMI_WR_ADDR_LSB  0x01
#define FPGA_SMI_WR_DATA_MSB  0x02
#define FPGA_SMI_WR_DATA_LSB  0x03
#define FPGA_SMI_RD_ADDR_MSB  0x04
#define FPGA_SMI_RD_ADDR_LSB  0x05
#define FPGA_SMI_RD_DATA_MSB  0x06
#define FPGA_SMI_RD_DATA_LSB  0x07
#define FPGA_DATE             0x18
#define FPGA_DAY_REV          0x19
#define FPGA_POE_RESET        0x1A
#define FPGA_INTR_STATUS      0x1B
#define FPGA_INTR_MASK        0x1C
#define FPGA_GLOBAL_INTR_MASK 0x1D
#define FPGA_ICAP_CNTL        0x1E
#define FPGA_SMI_STATUS       0x1F

#define SPI_ACCESS            0x2000
#define SPI_ADDR_PRESENT      0x0100
#define LEN_SHIFT             14

/* bit define for FPGA SMI status register */ 
#define FPGA_SMI_READY        0x01
#define FPGA_SMI_RD_DONE      0x02
#define FPGA_SMI_WR_DONE      0x04

#define FPGA_SMI_READY_RETRY  200
#define FPGA_SMI_RD_RETRY     5000
#define FPGA_SMI_WR_RETRY     5000

#define FPGA_POE_INTR_BIT     0x40
#define FPGA_POE_RESET_BIT    0x02

#define SPI_FLASH_TEST_SECTOR_OFFSET    0x3f0000
#define SPI_FLASH_TEST_LENGTH           0x1000

#define SPI_FLASH_UPDATE_IMAGE_ADDR     0x100000
#define SPI_FLASH_UPDATE_IMAGE_SIZE     0x100000
#define SPI_FLASH_SECTOR_SIZE           0x10000
#define SPI_FLASH_SUBSECTOR_SIZE        0x1000

/* opcode for SPI flash */
#define SPI_WRITE_ENABLE      0x06
#define SPI_PROGRAM_BYTE      0x02
#define SPI_READ_BYTE         0x03
#define SPI_FAST_READ_BYTE    0x0B
#define SPI_ERASE_SUBSECTOR   0x20
#define SPI_ERASE_SECTOR      0xD8
#define SPI_READ_DEV_ID       0x9f
#define SPI_READ_STATUS       0x05
#define SPI_WRITE_STATUS      0x01
#define SPI_PROTECT_SECTOR    0x36
#define SPI_UNPROTECT_SECTOR  0x39

#define SPI_MAX_RETRIES       5000

#define SPI_PAGE_SIZE         0x100  /* page boundary 256B */


/*
 *------------------------------------------------------------------
 * $Log: dreamliner_fpga.h,v $
 * Revision 1.2  2019/12/11 10:10:25  lucywang
 * Merged Nanook to main trunk
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */


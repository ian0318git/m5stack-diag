/* $Id: diag_spi_flash_lib.h,v 1.2 2019/07/11 12:31:29 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_spi_flash_lib.h,v $
 *
 * Filename: diag_spi_flash_lib.h
 *
 * Description: Diag spi flash lib header file.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_SPI_FLASH_LIB_H__
#define __DIAG_SPI_FLASH_LIB_H__

extern int denverton_spi_ctrl_mm_read32(uint, uint*);
extern int denverton_spi_ctrl_mm_write32(uint, uint);

#endif                          /* __DIAG_SPI_FLASH_UTIL_H__ */


/******** History ********
$Log: diag_spi_flash_lib.h,v $
Revision 1.2  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/

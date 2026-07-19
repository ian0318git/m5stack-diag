/* $Id: diag_platform.h,v 1.2 2016/04/20 08:41:37 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_platform.h,v $
 *
 *      File:   diag_platform.h
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "diag_cmn.h"

// Bus Defines
#define PLATFORM_I2C_BUS0		  0
#define PLATFORM_I2C_BUS1		  1
#define PLATFORM_I2C_BUS2		  2
#define PLATFORM_I2C_BUS3		  3
#define PLATFORM_I2C_BUS4	      4
#define PLATFORM_I2C_BUS5		  5
#define PLATFORM_I2C_BUS6		  6
#define PLATFORM_I2C_BUS7		  7

// I2C Defines for BUS 1
#define PLATFORM_I2C_HDD_FRU_COUNT  0x02
#define PLATFORM_I2C_HDD0_FRU_ADDR  0xA0
#define PLATFORM_I2C_HDD1_FRU_ADDR  0xA2
#define PLATFORM_I2C_HDD_FRU_BUS    PLATFORM_I2C_BUS1

#define PLATFORM_I2C_TPM_FRU_COUNT  1
#define PLATFORM_I2C_TPM_FRU_ADDR   0xA4
#define PLATFORM_I2C_TPM_FRU_BUS    PLATFORM_I2C_BUS1

// I2C Defines for BUS 0
#define MEZZ_FRU_BUS		PLATFORM_I2C_BUS0
#define MEZZ_FRU_ADDR		0xa4

#define PLATFORM_I2C_TSENS_COUNT     2
#define PLATFORM_I2C_TSENS_BUS       PLATFORM_I2C_BUS1
#define PLATFORM_I2C_TSENS_ADDR_0    0x9C
#define PLATFORM_I2C_TSENS_ADDR_1    0x9E

#define PLATFORM_I2C_FRU_COUNT       1
#define PLATFORM_I2C_FRU_ADDR        0xA4
#define PLATFORM_TACHIL_I2C_FRU_ADDR       0xAE
#define PLATFORM_I2C_FRU_BUS         PLATFORM_I2C_BUS1

/* I2C Defines for BUS 5 */
#define PLATFORM_I2C_SDPROM_ADDR       0xA2

extern int platform_i2c_open();
extern void platform_i2c_close(int fd);

extern section_toc_t platform_section_toc;
extern uint32_t platform_ioctl(diag_dev_t *pdev, uint32_t opcode, va_list arglist);
extern int I2CByteRead_8BitIndex(int Fh, unsigned char Bus, unsigned char Dev,
                          unsigned char Reg, unsigned char *data, int verbose);
extern int I2CByteWrite_8BitIndex (int Fh, unsigned char Bus, unsigned char Dev,
                        unsigned char Reg, unsigned char Val, int send, int verbose);
extern int I2CByteAccess_Variable(int Fh, unsigned char Bus, unsigned char Dev, 
		unsigned char *wbuf, int ws, unsigned char *rbuf, int rs, int verbose);
extern int I2CPing(int Fh, unsigned char Bus, unsigned char Dev, int verbose);
extern int I2CByteWrite_Block(int Fh, unsigned char Bus, unsigned char Dev, 
		unsigned char *txBuf, int bytes, int verbose);
extern int I2CByteRead_Block(int Fh, unsigned char Bus, unsigned char Dev,
		unsigned char Reg, unsigned char *rxBuf, int bytes, int verbose);

#endif /* _PLATFORM_H_ */


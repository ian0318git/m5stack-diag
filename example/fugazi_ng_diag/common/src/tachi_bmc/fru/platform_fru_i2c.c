/* $Id: platform_fru_i2c.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/platform_fru_i2c.c,v $
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 */
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <asm/ioctl.h> 
#include <sys/ioctl.h> 
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "i2c.h"

#define MY_FILE_NAME		I2C_DRIVER_FILE_NAME
#define MEANINGLESS		(0)
#define TRUE			(1)
#define FALSE			(0)
#define TRANSFER_BUFFER_SIZE	(0x10)
#define MEANINGLESS_DATA	(0xFF)


int
I2CByteWrite_8BitIndex (int Fh, unsigned char Bus, unsigned char Dev, 
			unsigned char Reg, unsigned char Val, int send, 
			int verbose)
{
	sI2CDrvBufInfoType	I;
	int                	IOCTLReturn;
	unsigned char		RxBuffer[100], TxBuffer[100];

	verbose=0;
	memset(&I, 0, sizeof(sI2CDrvBufInfoType));

	I.u8Channel		= Bus;
	I.u8DeviceAddr		= Dev;
	I.pu8MsgRecBuffer	= RxBuffer;
	I.pu8MsgSendBuffer	= TxBuffer;
	TxBuffer[0]		= Reg;
	TxBuffer[1]		= Val;
	I.u8MsgSendDataSize	= send ? 1 : 2;
	I.u8MsgRecDataSize          = 0;

	if (verbose) printf("Byte Write (0x%02X, 0x%02X, 0x%02X, %02X) ---- ",Bus, Dev, Reg, Val);

	if ((IOCTLReturn = ioctl(Fh, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
		printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
		return -1;
	} else {
		if (I.u8ErrorStatus == 0) {
			if (verbose) printf("Success\n");
	  	} else {
			if (verbose) printf("Failed (0x%02X)\n", I.u8ErrorStatus);
			return -1;
		}
	}
	return (0);
}

int I2CByteRead_8BitIndex(int Fh, unsigned char Bus, unsigned char Dev, 
			  unsigned char Reg, unsigned char *data, int verbose)
{
	sI2CDrvBufInfoType I;
	int                IOCTLReturn;
	unsigned char      RxBuffer[100], TxBuffer[100];

	verbose=0;
	memset(&I, 0, sizeof(sI2CDrvBufInfoType));

	I.u8Channel                 = Bus;
	I.u8DeviceAddr              = Dev;
	I.pu8MsgRecBuffer           = RxBuffer;
	I.pu8MsgSendBuffer          = TxBuffer;
	TxBuffer[0]                 = Reg;
	I.u8MsgSendDataSize         = 1;
	I.u8MsgRecDataSize          = 1;

	if ((IOCTLReturn = ioctl(Fh, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
		printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
		return -1;
	} else {
		if (verbose) printf("Byte Read (0x%02X, 0x%02X, 0x%02X) --->  0x%02X ---- ",Bus, Dev, Reg, RxBuffer[0]);
		if (I.u8ErrorStatus == 0) {
			if (verbose) printf("Success\n");
			*data = RxBuffer[0];
			return 0;
		} else {
			printf("Failed (0x%02X)\n", I.u8ErrorStatus);
			return -1;
		}
	}
}

int I2CByteAccess_Variable(int Fh, unsigned char Bus, unsigned char Dev, 
			  unsigned char *wbuf, int ws, unsigned char *rbuf, int rs, int verbose)
{
	sI2CDrvBufInfoType I;
	int                IOCTLReturn;

	memset(&I, 0, sizeof(sI2CDrvBufInfoType));

	I.u8Channel                 = Bus;
	I.u8DeviceAddr              = Dev;
	I.pu8MsgRecBuffer           = rbuf;
	I.pu8MsgSendBuffer          = wbuf;
	I.u8MsgSendDataSize         = ws;
	I.u8MsgRecDataSize          = rs;

	if ((IOCTLReturn = ioctl(Fh, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
		if(verbose) printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
		return -1;
	} else {
		if (verbose) printf("Byte Read (0x%02X, 0x%02X) ",Bus, Dev);
		if (I.u8ErrorStatus == 0) {
			if (verbose) printf("Success\n");
			return 0;
		} else {
			if(verbose) printf("Failed (0x%02X)\n", I.u8ErrorStatus);
			return -1;
		}
	}
}

int I2CByteRead_NoIndex(int Fh, unsigned char Bus, unsigned char Dev, 
			  unsigned char *data, int rs, int verbose)
{
	sI2CDrvBufInfoType I;
	int                IOCTLReturn;
	unsigned char      TxBuffer[100];

	verbose=0;
	memset(&I, 0, sizeof(sI2CDrvBufInfoType));

	I.u8Channel                 = Bus;
	I.u8DeviceAddr              = Dev;
	I.pu8MsgRecBuffer           = data;
	I.pu8MsgSendBuffer          = TxBuffer;
	I.u8MsgSendDataSize         = 0;
	I.u8MsgRecDataSize          = rs;

	if ((IOCTLReturn = ioctl(Fh, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
		printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
		return -1;
	} else {
		if (I.u8ErrorStatus == 0) {
			if (verbose) printf("Success\n");
			return 0;
		} else {
			printf("Failed (0x%02X)\n", I.u8ErrorStatus);
			return -1;
		}
	}
}

int I2CPing(int Fh, unsigned char Bus, unsigned char Dev, int verbose)
{
	unsigned char data;
	unsigned char wbuf[2];
	wbuf[0]=0;
	wbuf[1]=0;
	if( I2CByteAccess_Variable(Fh, Bus, Dev, wbuf, 0, &data, 1, 0) == 0 ) return 0;
	if( I2CByteAccess_Variable(Fh, Bus, Dev, wbuf, 1, &data, 1, 0) == 0 ) return 0;
	if( I2CByteAccess_Variable(Fh, Bus, Dev, wbuf, 2, &data, 1, 0) == 0 ) return 0;
	return -1;
}

int
I2CByteWrite_Block (int Fh, unsigned char Bus, unsigned char Dev, unsigned char *txBuf, 
			int bytes, int verbose)
{
	sI2CDrvBufInfoType I;
	int                i, IOCTLReturn;
	unsigned char      RxBuffer[100], TxBuffer[100];

	memset(&I, 0, sizeof(sI2CDrvBufInfoType));

	I.u8Channel         = Bus;
	I.u8DeviceAddr      = Dev;
	I.pu8MsgRecBuffer   = RxBuffer;
	I.pu8MsgSendBuffer  = TxBuffer;
	I.u8MsgSendDataSize = bytes;
	I.u8MsgRecDataSize  = 0;
  
	for (i=0; i<bytes; i++) {
		TxBuffer[i] = txBuf[i];
		if (verbose) printf(" %2.2x", TxBuffer[i]);
	}
	if (verbose) printf("\nBlock Write (0x%02X, 0x%02X ) ---- ",Bus, Dev );

	if ((IOCTLReturn = ioctl(Fh, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
		if(verbose) printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
		return -1;
	} else {
		if (I.u8ErrorStatus == 0) {
			if (verbose) printf("Success\n");
			return 0;
		} else {
			if (verbose) printf("Failed (0x%02X)\n", I.u8ErrorStatus);
			return -1;
		}
	}
}

// used in pmbus reads
int I2CByteRead_Block ( int Fh, unsigned char Bus, unsigned char Dev, 
			unsigned char Reg, unsigned char *rxBuf, int bytes, int verbose)
{
	sI2CDrvBufInfoType I;
	int                i, IOCTLReturn, nreads = 1, iters;
	unsigned char      RxBuffer[32], TxBuffer[16];

	memset(&I, 0, sizeof(sI2CDrvBufInfoType));
	memset(RxBuffer, 0x00, sizeof(RxBuffer));
	memset(TxBuffer, 0x00, sizeof(TxBuffer));

	I.u8Channel                 = Bus;
	I.u8DeviceAddr              = Dev;
	I.pu8MsgRecBuffer           = RxBuffer;
	I.pu8MsgSendBuffer          = TxBuffer;
	TxBuffer[0]                 = Reg;
	I.u8MsgSendDataSize         = 1;

	nreads = (bytes >> 5) + ((bytes & 0x1F)? 1 : 0);

	for (iters = 0; iters < nreads; iters++) {	
		TxBuffer[0] = Reg + (iters*32);
		I.u8MsgRecDataSize   = (unsigned char)((iters+1)==nreads) ? (bytes & 0x1F) : 0x20;

		if ((IOCTLReturn = ioctl(Fh, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
			if(verbose) printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
			return -1;
		} else {
			if (verbose) 
				printf("Byte Read (0x%02X, 0x%02X, 0x%02X) --->  0x%02X ---- ",
					Bus, Dev, TxBuffer[0], RxBuffer[0]);
			if (I.u8ErrorStatus == 0) {
				if (verbose) printf("Success: ");
				for (i=0; i<I.u8MsgRecDataSize; i++) {
					rxBuf[(iters*32)+i] = RxBuffer[i];
					if (verbose) printf("%2.2x ", RxBuffer[i]);
				}
				if (verbose) printf("\n");
			} else {
				printf("Failed (0x%02X)\n", I.u8ErrorStatus);
				return -1;
			}
		}
	}
	return 0;
}

void I2CWordWrite_8BitIndex( int Fh, unsigned char Bus, unsigned char Dev, 
				unsigned char Reg, unsigned short Val)
{
	printf("I2CWordWrite_8BitIndex --> Not yet implemented!\n");
}  /* end routine */



void
I2CWordRead_8BitIndex(int Fh, unsigned char Bus, unsigned char Dev, unsigned char Reg)
{
	printf("I2CWordRead_8BitIndex --> Not yet implemented!\n");
}  /* end routine */


int platform_i2c_open()
{
	return (open(MY_FILE_NAME, O_RDONLY, 0));
}

void platform_i2c_close(int fd)
{
	close(fd);
}

int ppc_i2c_open(int bus)
{
	return (open(MY_FILE_NAME, O_RDONLY, 0));
}

void ppc_i2c_close(int fd)
{
	close(fd);
}


/* $Id: diag_i2c_lib.c,v 1.2 2016/04/20 11:37:00 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_lib.c - I2C Library
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> 
#include <unistd.h>
#include <fcntl.h>
#include "nvmonvars.h"
#include "diag_i2c_lib.h"

int diag_i2c_ping(unsigned char, unsigned char, int);
int diag_i2c_byte_access_variable(int, unsigned char, unsigned char, 
                                  unsigned char *, int, unsigned char *, int, 
                                  int);
int diag_i2c_read(unsigned int, unsigned int, unsigned int, unsigned int, char *);
int diag_i2c_write(unsigned int, unsigned int, unsigned int, unsigned int, char *);
uchar diag_i2c_byte_read(uchar, uchar, int, uchar *);
uchar diag_i2c_byte_write(uchar, uchar, int, uchar);

static int diag_i2c_read_block(unsigned int, unsigned int, unsigned int, 
                               unsigned int, char *);
static int diag_i2c_byte_read_onebyte(unsigned int, unsigned int, unsigned int, 
                                      unsigned char *) ;
static int diag_i2c_byte_write_onebyte(unsigned int, unsigned int, unsigned int, 
                                       unsigned int, unsigned char); 
static int diag_i2c_write_block(unsigned int, unsigned int, unsigned int, 
                                unsigned char *); 


uchar diag_i2c_byte_write (uchar bus, uchar Dev_Addr, int Offset_Index, uchar data)
{
    unsigned short Reg16;
    int fd;
    uchar RxBuffer[100], TxBuffer[100];

    fd = open(I2C_DRIVER_NAME, O_RDONLY, 0);
    if (fd == -1) {
        printf("%s: Error in opening %s\n", __FUNCTION__, I2C_DRIVER_NAME);
        return (-1);
    }

    sI2CDrvBufInfoType I;

    memset(&I, 0, sizeof(sI2CDrvBufInfoType));
    memset(TxBuffer, 0, 100);
    memset(RxBuffer, 0, 100);

    I.u8Channel                 = bus;
    I.u8DeviceAddr              = Dev_Addr;
    I.pu8MsgRecBuffer           = RxBuffer;
    I.pu8MsgSendBuffer          = TxBuffer;

    Reg16   = Offset_Index;
    TxBuffer[0]                 = (unsigned char)(Reg16 >> 8);
    TxBuffer[1]                 = (unsigned char)(Reg16 >> 0);
    TxBuffer[2]                 = data;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("offset 0x%x data 0x%x \n", TxBuffer[1], TxBuffer[2]);
    }
    I.u8MsgSendDataSize         = 3;
    I.u8MsgRecDataSize          = 0;

    int IOCTLReturn;
     if ((IOCTLReturn = ioctl(fd, AESS_I2CDRV_WR, (ulong)&I)) != 0) {
         if ((NVRAM)->diagflag & D_VERBOSE) {
             printf("%s: IOCTL Fail (0x%08X)\n", __FUNCTION__, IOCTLReturn);
         }
         close(fd);
         return (-1);
     } else {
         if (I.u8ErrorStatus == 0) {
             close(fd);
             return (0);
         } else {
             if ((NVRAM)->diagflag & D_VERBOSE) {
                 printf("Failed (0x%02X)\n", I.u8ErrorStatus);
             }
             close(fd);
             return (-1);
         }
     }

}  /* end routine */


uchar diag_i2c_byte_read (uchar bus, uchar Dev_Addr, int Offset_Index, uchar *data)
{
    int fd;
    uchar RxBuffer[100], TxBuffer[100];
    unsigned short Reg16;

    fd = open(I2C_DRIVER_NAME, O_RDONLY, 0);
    if (fd == -1) {
        printf("%s: Error in opening %s\n", __FUNCTION__, I2C_DRIVER_NAME);
        return (-1);
    }

    sI2CDrvBufInfoType I;

    memset(&I, 0, sizeof(sI2CDrvBufInfoType));
    memset(TxBuffer, 0, 100);
    memset(RxBuffer, 0, 100);

    I.u8Channel                 = bus;
    I.u8DeviceAddr              = Dev_Addr;
    I.pu8MsgRecBuffer           = RxBuffer;
    I.pu8MsgSendBuffer          = TxBuffer;


    Reg16    = Offset_Index;
    TxBuffer[0]                 = (unsigned char)(Reg16 >> 8);
    TxBuffer[1]                 = (unsigned char)(Reg16 >> 0);
    I.u8MsgSendDataSize         = 2;
    I.u8MsgRecDataSize          = 1;

    int IOCTLReturn;
    if ((IOCTLReturn = ioctl(fd, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: IOCTL Fail (0x%08X)\n", __FUNCTION__, IOCTLReturn);
        }
        close(fd);
        return (-1);
    } else {
        if (I.u8ErrorStatus == 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("offset 0x%x%x data 0x%x\n",TxBuffer[0], TxBuffer[1],  RxBuffer[0]);
            }
            *data = RxBuffer[0];
            close(fd);
            return (0);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Failed (0x%02X)\n", I.u8ErrorStatus);
            }
            close(fd);
            return (-1);
        }
    }
}
int diag_i2c_write (unsigned int bus, unsigned int dev, unsigned int offset, 
                    unsigned int size, char *buf)
{
    int retval = 0;

    if (size > 1) {
        if (diag_i2c_write_block(bus, dev, size, buf)) {
            printf("%s: Error in writing I2C block (bus=%d,dev=%02x)\n", __FUNCTION__,
                    bus, dev);
            retval = -1;
        }
        
    } else {
        if (diag_i2c_byte_write_onebyte(bus, dev, offset, 0, buf[0])) {
            printf("%s: Error in writing I2C one byte (bus=%d,dev=%02x)\n", __FUNCTION__,
                    bus, dev);
            retval = -1;
        }
    }
    return (retval);
}


int diag_i2c_read (unsigned int bus, unsigned int dev, unsigned int offset, 
                   unsigned int size, char *buf)
{
    int retval = 0;

    if (size > 1) {
        if (diag_i2c_read_block(bus, dev, offset, size, buf)) {
            printf("%s: Error in reading I2C block (bus=%d,dev=%02x)\n", __FUNCTION__,
                    bus, dev);
            retval = -1;
        }
        
    } else {
        if (diag_i2c_byte_read_onebyte(bus, dev, offset, buf)) {
            printf("%s: Error in reading I2C one byte (bus=%d,dev=%02x)\n", __FUNCTION__,
                    bus, dev);
            retval = -1;
        }
    }
    return (retval);
}

int diag_i2c_ping (unsigned char bus, unsigned char dev, int verbose)
{
    unsigned char data;
    unsigned char wbuf[2];
    int fd;
    
    fd = open(I2C_DRIVER_NAME, O_RDONLY, 0);
    
    if (fd == -1) {
        printf("Error in opening %s\n", I2C_DRIVER_NAME);
        return (-1);
    }
    
    wbuf[0]=0;
    wbuf[1]=0;
    
    if (diag_i2c_byte_access_variable(fd, bus, dev, wbuf, 0, &data, 1, 0) == 0) {
        close(fd);
        return (0);
    }
    if (diag_i2c_byte_access_variable(fd, bus, dev, wbuf, 1, &data, 1, 0) == 0) {
        close(fd);
        return (0);
    }
    if (diag_i2c_byte_access_variable(fd, bus, dev, wbuf, 2, &data, 1, 0) == 0) {
        close(fd);
        return (0);
    }
    
    close(fd);
    return (-1);
}

int diag_i2c_byte_access_variable (int fd, unsigned char bus, unsigned char dev, 
			                       unsigned char *wbuf, int ws, 
			                       unsigned char *rbuf, int rs, int verbose)
{
    sI2CDrvBufInfoType I;
    int                IOCTLReturn;

    memset(&I, 0, sizeof(sI2CDrvBufInfoType));

    I.u8Channel                 = bus;
    I.u8DeviceAddr              = dev;
    I.pu8MsgRecBuffer           = rbuf;
    I.pu8MsgSendBuffer          = wbuf;
    I.u8MsgSendDataSize         = ws;
    I.u8MsgRecDataSize          = rs;

    if ((IOCTLReturn = ioctl(fd, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
        if (verbose) {
            printf("IOCTL Fail (0x%08X)\n", IOCTLReturn);
        }
        return (-1);
    } else {
        if (verbose) {
            printf("Byte Read (0x%02X, 0x%02X) ", bus, dev);
        }
        if (I.u8ErrorStatus == 0) {
            if (verbose) {
                printf("Success\n");
            }    
            return (0);
        } else {
            if (verbose) {
                printf("Failed (0x%02X)\n", I.u8ErrorStatus);
            }
            return (-1);
        }
    }
}

static int diag_i2c_write_block (unsigned int bus, unsigned int dev,
			                          unsigned int size, unsigned char *data)
{
    int ix = 0, offset = 0x0;

    for (ix = 0; ix < size; ix++, offset++) {
        if (diag_i2c_byte_write_onebyte(bus, dev, offset, 0, data[ix])) {
            printf("%s: Error in writing I2C one byte (bus=%d,dev=%02x, offset=%02x)\n",
                    __FUNCTION__, bus, dev, offset);
            return (-1);
        }
    }

    return (0);
}

static int diag_i2c_byte_write_onebyte (unsigned int bus, unsigned int dev, 
			                            unsigned int offset, unsigned int wo_hdr, 
                                        unsigned char data) 
{
    sI2CDrvBufInfoType I;
    int                IOCTLReturn;
    unsigned char      RxBuffer[100], TxBuffer[100];
    int fd;

    fd = open(I2C_DRIVER_NAME, O_RDONLY, 0);
    if (fd == -1) {
        printf("%s: Error in opening %s\n", __FUNCTION__, I2C_DRIVER_NAME);
        return (-1);
    }
    memset(&I, 0, sizeof(sI2CDrvBufInfoType));

    I.u8Channel                 = bus;
    I.u8DeviceAddr              = dev;
    I.pu8MsgRecBuffer           = RxBuffer;
    I.pu8MsgSendBuffer          = TxBuffer;
    TxBuffer[0]                 = offset;
    TxBuffer[1]                 = data;
    I.u8MsgSendDataSize         = wo_hdr ? 1: 2;
    I.u8MsgRecDataSize          = 0;

    if ((IOCTLReturn = ioctl(fd, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: IOCTL Fail (0x%08X)\n", __FUNCTION__, IOCTLReturn);
        }
        close(fd);
        return (-1);
    } else {
        if (I.u8ErrorStatus == 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Data is 0x%x ", TxBuffer[1]);
                printf("Success\n");
            }    
            close(fd);
            return (0);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Failed (0x%02X)\n", I.u8ErrorStatus);
            }
            close(fd);
            return (-1);
        }
    }
}

static int diag_i2c_byte_read_onebyte (unsigned int bus, unsigned int dev, 
			                           unsigned int offset, unsigned char *data) 
{
    sI2CDrvBufInfoType I;
    int                IOCTLReturn;
    unsigned char      RxBuffer[100], TxBuffer[100];
    int fd;

    fd = open(I2C_DRIVER_NAME, O_RDONLY, 0);
    if (fd == -1) {
        printf("%s: Error in opening %s\n", __FUNCTION__, I2C_DRIVER_NAME);
        return (-1);
    }
    memset(&I, 0, sizeof(sI2CDrvBufInfoType));

    I.u8Channel                 = bus;
    I.u8DeviceAddr              = dev;
    I.pu8MsgRecBuffer           = RxBuffer;
    I.pu8MsgSendBuffer          = TxBuffer;
    TxBuffer[0]                 = offset;
    I.u8MsgSendDataSize         = 1;
    I.u8MsgRecDataSize          = 1;

    if ((IOCTLReturn = ioctl(fd, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: IOCTL Fail (0x%08X)\n", __FUNCTION__, IOCTLReturn);
        }
        close(fd);
        return (-1);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s: Byte Read (0x%02X, 0x%02X) ", __FUNCTION__, bus, dev);
        }
        if (I.u8ErrorStatus == 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Data is 0x%x ", RxBuffer[0]);
                printf("Success\n");
            }    
            *data = RxBuffer[0];
            close(fd);
            return (0);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Failed (0x%02X)\n", I.u8ErrorStatus);
            }
            close(fd);
            return (-1);
        }
    }
}


static int diag_i2c_read_block (unsigned int bus, unsigned int dev, unsigned int offset,
                                unsigned int size, char *buf)
{
    int fd, ix, nreads = 1, iters;
    sI2CDrvBufInfoType I;
    int                IOCTLReturn;
    unsigned char      RxBuffer[32], TxBuffer[16];

    memset(&I, 0, sizeof(sI2CDrvBufInfoType));
    memset(RxBuffer, 0x00, sizeof(RxBuffer));
    memset(TxBuffer, 0x00, sizeof(TxBuffer));

    fd = open(I2C_DRIVER_NAME, O_RDONLY, 0);
    if (fd == -1) {
        printf("%s: Error in opening %s\n", __FUNCTION__, I2C_DRIVER_NAME);
        return (-1);
    }

    memset(&I, 0, sizeof(sI2CDrvBufInfoType));

    I.u8Channel                 = bus;
    I.u8DeviceAddr              = dev;
    I.pu8MsgRecBuffer           = RxBuffer;
    I.pu8MsgSendBuffer          = TxBuffer;
    TxBuffer[0]                 = offset;
    I.u8MsgSendDataSize         = 1;

    nreads = (size >> 5) + ((size & 0x1F)? 1 : 0);

    for (iters = 0; iters < nreads; iters++) {
        TxBuffer[0] = offset + (iters * 32);
        I.u8MsgRecDataSize   = (unsigned char)((iters+1)==nreads) ? 
                               (size & 0x1F) : 0x20;

        if ((IOCTLReturn = ioctl(fd, AESS_I2CDRV_WR, (unsigned long)&I)) != 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: IOCTL Fail (0x%08X)\n", __FUNCTION__, IOCTLReturn);
            }
            close(fd);
            return (-1);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Byte Read (0x%02X, 0x%02X, 0x%02X) --->  0x%02X ---- ",
                        bus, dev, TxBuffer[0], RxBuffer[0]);
            }

            if (I.u8ErrorStatus == 0) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("Success\n");
                }    
                for (ix = 0; ix < I.u8MsgRecDataSize; ix++) {
                    buf[(iters * 32) + ix] = RxBuffer[ix];
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("%2.2x ", RxBuffer[ix]);
                    }
                }
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("\n");
                }
                
            } else {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("Failed (0x%02X)\n", I.u8ErrorStatus);
                }
                close(fd);
                return (-1);
            }
        }
    }

    close(fd);
    return (0);
}

/*---------------------------------------------------------------
$Log: diag_i2c_lib.c,v $
Revision 1.2  2016/04/20 11:37:00  benchen2
merge tachi to main trunk

Revision 1.1.2.5  2016/02/05 01:41:24  benchen2
raid card support SBR eeprom program

Revision 1.1.2.4  2015/11/13 07:57:29  tirawan
Add Voltage and Frequency Margin

Revision 1.1.2.3  2015/08/18 02:44:00  meho
Fixed i2c r/w bug of RTC test.

Revision 1.1.2.2  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/

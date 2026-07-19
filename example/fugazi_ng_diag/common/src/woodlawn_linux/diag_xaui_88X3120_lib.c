/* $Id: diag_xaui_88X3120_lib.c,v 1.2 2013/10/08 08:48:29 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_xaui_88X3120_lib.c,v $
 *-----------------------------------------------------------------------------
 * diag_xaui_88X3120_lib.c - Utility Menu and Functions for Woodlawn PHY 88X2120L
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>
#include "cvmx.h"
#include "cvmx-mdio.h"

#include "diag_xaui_88X3120_lib.h"

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
unsigned short SFPhyMdioRead(CTX_PTR_TYPE, unsigned long, unsigned long,
                              unsigned long);
void SFPhyMdioWrite(CTX_PTR_TYPE, unsigned long, unsigned long, unsigned long,
                    unsigned short);
int SFPhyIsSFT9001RevBorLater(CTX_PTR_TYPE, unsigned long);
void SFPhyPutPhyInDownloadMode(CTX_PTR_TYPE, unsigned long);
void SFPhySpecialSoftwareReset(CTX_PTR_TYPE, unsigned long);
int SFPhyMdioRamDownload(CTX_PTR_TYPE,unsigned char data[], unsigned int,
                         unsigned long, unsigned char);
int SFPhyMdioFlashDownload(CTX_PTR_TYPE, unsigned long, unsigned char data[],
                           unsigned int);
unsigned short SFPhyDownLoadFlash(CTX_PTR_TYPE, unsigned long,
                                  unsigned char *,unsigned long,
                                  unsigned char *,unsigned int);
void SFPhyRemovePhyDownloadMode(CTX_PTR_TYPE, unsigned long);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

extern void udelay(unsigned long);

/***********************************************************************
*  Global Variable
************************************************************************/

/***********************************************************************
 *  Functions
 ************************************************************************/

unsigned short SFPhyMdioRead (CTX_PTR_TYPE contextPtr, unsigned long port, unsigned long dev,
                              unsigned long reg)
{
    unsigned int value, pex;

    pex = (*contextPtr);

    value = cvmx_mdio_45_read(pex, port, dev, reg);
#ifdef ORG_PORT_CODE
    rc = ReadXsmiPhy(pex/* PEX num */, port/* Phy address bit 16-20 */, \
        dev/* phyDev bit 21-25 */, reg  /* address  ADDR-REG */, &value);
#endif

    if (value < 0) {
        printf("Read error from device %u address (%lu)\n", pex, reg);
        return (FAILED);
    }

    return value;
}

void SFPhyMdioWrite (CTX_PTR_TYPE contextPtr, unsigned long port, unsigned long dev,
                    unsigned long reg, unsigned short value)
{
    unsigned long rc;
    unsigned int pex;

    pex = (*contextPtr);

    /* XSMI register write */
    rc = cvmx_mdio_45_write(pex, port, dev, reg, value);

    if (rc < 0) {
        printf("Write error to device %lu address (%lu)\n", dev, reg);
    }

#ifdef ORG_PORT_CODE
    rc = WriteXsmiPhy(pex/* PEX num */, port/* Phy address bit 16-20 */, \
        dev/* phyDev bit 21-25 */, reg  /* address  ADDR-REG */, value);
    if (rc != MV_OK)
        printf("Write Xsmi failed\n");
#endif
}

int SFPhyIsSFT9001RevBorLater (CTX_PTR_TYPE contextPtr, unsigned long port)
{
    unsigned short val, val2, val3;

    /*  read the device/package id and look at the manufacturer's model number */
    val = SFPhyMdioRead(contextPtr,port,1,3);
    val >>= 4;
    val &= 0x003F; /*  it's bits 9-4 */

    if (val >= 8) {
        /*  It's an SFT9001 or SFT910X */
        val = SFPhyMdioRead(contextPtr, port,3,53249); /*  read the chip id */
        val &= 0x000F; /*  it's in bits 3-0 */
        if (val <= 2) {
            val2 = SFPhyMdioRead(contextPtr,port,1,49216); // read the new spare LL register
            val3 = SFPhyMdioRead(contextPtr,port,1,49217); // read the new spare LH register
            if (val2 == 0xFFFF && val3 == 0x0000)
            {
                return 1; // It's a SFT910X part
            }
            else
            {
                return 0; // it's an SFT9001 Rev AX
            }
        } else {
            return 1; /*  it's an SFT9001 Rev BX or later */
        }
    } else {
        /*  It's either a SFX7101 or an SFT9001 Rev AX with no firmware running */
        return 0;
    }
}

void SFPhyPutPhyInDownloadMode (CTX_PTR_TYPE contextPtr, unsigned long port)
{

#if SF_EVK_TEST_ENVIRONMENT
    char s[20];
    fprintf(stdout,"Set corresponding DIP switch to put pin F_CFG1 to 1 (on) to put EVK in download mode.");
    fprintf(stdout,"In case of multi port system, set the DIP switch on all ports.\n");
    fprintf(stdout,"Hit <CR> to continue\n");
    gets(s);
    SFPhySpecialSoftwareReset(contextPtr, port);
#endif

    /* Woodlawn trigger the download pin from FPGA FPGA_88X3120_FLASH_CFG_P0 */
#ifdef ORG_PORT_CODE
    printf("SFPhyPutPhyInDownloadMode implemented by jumper, wait for download...\n");
#endif
}

/*******************************************************************
  Resets
  Checking Boot Status
  Download Mode
  Low Power Mode
 *******************************************************************/
/****************************************************************************/
void SFPhySpecialSoftwareReset (CTX_PTR_TYPE contextPtr, unsigned long port)
{
    unsigned short val;
    val = SFPhyMdioRead(contextPtr, port,1,49152);
    /* Writes a self-clearing reset */
    SFPhyMdioWrite(contextPtr, port,1,49152,(val | 1<<15));
}

void SFPhyWait(CTX_PTR_TYPE contextPtr, unsigned x)
{
#if SF_EVK_TEST_ENVIRONMENT
    Sleep(x);
#endif
    udelay(x*1000);
}

/********************************************************************/
/* This function downloads code to RAM in the DSP and then starts the application
 which was downloaded.*/

/* This function downloads code to RAM in the DSP and then starts the application
 which was downloaded. Make sure "size" is an even number (memory can only be written word-wise) */
int SFPhyMdioRamDownload(CTX_PTR_TYPE contextPtr,unsigned char data[],
                         unsigned int size, unsigned long port, unsigned char use_ram_checksum)
{
    unsigned     buffCount;
    unsigned char lowByte, highByte;
    unsigned short tmp;
    unsigned short ram_checksum;
    unsigned short expected_checksum;
#if SF_EVK_TEST_FLASHCODE
    double download_time;
    time_t start_time,stop_time;
#endif

    SFPhyPutPhyInDownloadMode(contextPtr, port);

    /* Reset phy to have change to FLASH_CFG[1] take effect */
    SFPhySpecialSoftwareReset(contextPtr, port);

    /* Allow reset to complete */
    SFPhyWait((CTX_PTR_TYPE)0, 250);

    /* Make sure we can access the DSP
      And it's in the correct mode (waiting for download) */
    if ((tmp=SFPhyMdioRead(contextPtr, port,3,0xD000)) != 0x000A) {
#if SF_EVK_TEST_FLASHCODE
        fprintf(stderr, "DSP is not in waiting on download mode. Expected 0x000A, read 0x%04X\n",
            (unsigned)tmp);
        fprintf(stderr, "Download failed\n");
#endif
        return ERR_PHY_NOT_IN_DOWNLOAD_MODE;
    } else {
#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"Downloading code to EVK RAM, please wait...\n");
#endif
    }

    // For SFT910X REVB or later
    if(use_ram_checksum)    {
        ram_checksum = SFPhyMdioRead(contextPtr, port, RAM_CHECKSUM_REG);   // Read the register to clear it
    }

    SFPhyMdioWrite(contextPtr, port,3,0xD004,0); /* Set starting address in RAM to 0x0000 */
    SFPhyMdioWrite(contextPtr, port,3,0xD005,0);

#if SF_EVK_TEST_FLASHCODE
    time(&start_time);
#endif

    /* Copy the code to the phy's internal RAM */
    buffCount=0;
    expected_checksum = 0;  // Initialize locally calculated checksum value
    while(buffCount < size) {
        lowByte = data[buffCount++];
        highByte = data[buffCount++];
        expected_checksum += (lowByte + highByte);  // This will later be ignored if use_ram_checksum is 0
        SFPhyMdioWrite(contextPtr, port,3,0xD006,(((unsigned short)highByte)<<8)|lowByte);

        /* Let the user know something's going on... */
#if SF_EVK_TEST_FLASHCODE
        amuse_user_ram(buffCount);
#endif
    }

    // For SFT910X REVB or later
    if(use_ram_checksum) {
        // Read the hardware checksum register value
        ram_checksum = SFPhyMdioRead(contextPtr, port, RAM_CHECKSUM_REG);
        // See if it matches with the locally computed checksum value
        if(expected_checksum != ram_checksum) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stdout, "Error downloading code. Expected RAM HW checsum to be %hu but it was %hu", expected_checksum, ram_checksum);
#endif
            return ERR_RAM_HW_CHECKSUM_ERR;
        }
    }

#if SF_EVK_TEST_FLASHCODE
    /* Calculate download time and print out results */
    time(&stop_time);
    download_time = difftime(stop_time,start_time);
    fprintf(stdout,"\nDownload of code to RAM complete. Time = %d seconds\n",(int)download_time);
    fprintf(stdout,"\nDownload complete, starting code.\n");
#endif

    /* Now start code which was downloaded */
    tmp = SFPhyMdioRead(contextPtr, port,3,0xD000);
    SFPhyMdioWrite(contextPtr, port,3,0xD000,(tmp|(1<<6)));

    SFPhyWait(contextPtr, 500); // Give app code time to start

    return 0;
}

/* This handles downloading an image pointed to by data which is size bytes long
   to the phy's flash interfacing with the slave code as a helper program.
   Size must be a power of 2 (the flash can only be written to in words).*/
int SFPhyMdioFlashDownload (CTX_PTR_TYPE contextPtr, unsigned long port,
                            unsigned char data[],unsigned int size)
{
    unsigned short buf_checksum, tmp_checksum, reported_checksum, words_rcvd;
    unsigned tmp, maxBuffSize, numTransfers, lastTransferSize, transferIndex;
    unsigned byteIndex, stopIndex;
#if SF_EVK_TEST_FLASHCODE
    double download_time;
    time_t start_time,stop_time;
#endif

    if (size%2) {
        /* it's an error, size must be an even number of bytes */
#if SF_EVK_TEST_FLASHCODE
        return fatal_error();
#endif
        return ERR_SIZE_NOT_EVEN;
    }

    /* first erase the flash*/
#if SF_EVK_TEST_FLASHCODE
    fprintf(stdout,"Slave will now erase flash. This may take up to 6 seconds.\n");
#endif
    SFPhyMdioWrite(contextPtr, port,COMMAND_REG, ERASE_FLASH_PROGRAM_AREA);

    tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);
    while( tmp==ERASE_FLASH_PROGRAM_AREA || tmp==FLASH_BUSY )
        tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);

    if (tmp == ERROR_CMD) {
#if SF_EVK_TEST_FLASHCODE
        fprintf(stderr,"Slave encountered error while erasing flash. Exiting...\n");
        return fatal_error();
#endif
        printf("Slave encountered error while erasing flash. Exiting... rc=%X\n", tmp);
        return ERR_ERASING_FLASH;

    } else {
        if (tmp == DOWNLOAD_OK) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stdout,"Flash program areas have been erased.\n");
#endif
        } else {
            /* unexpected value read back from download code*/
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"Unexpected response from phy. Exiting...\n");
            return fatal_error();
#endif
            printf("Unexpected response from phy. Exiting... rc=%X\n", tmp);
            return ERR_VALUE_READ_BACK;

        }
    }

    /* read in the max buffer size from the slave*/
    /* this is the maximum size that can be written at any 1 time*/
    maxBuffSize = SFPhyMdioRead(contextPtr, port,MAX_BUFF_SIZE_OUT_REG);
    maxBuffSize *= 2; // now it's in bytes

    numTransfers = size/maxBuffSize;
    lastTransferSize = size%maxBuffSize;

#if SF_EVK_TEST_FLASHCODE
    time(&start_time);
#endif

    /* handle all the full transfers */
    byteIndex=0;
    for(transferIndex=0; transferIndex < numTransfers; transferIndex++) {
        /* Set the flash start address*/
        SFPhyMdioWrite(contextPtr, port,LOW_ADDRESS_REG, (unsigned short)byteIndex);
        SFPhyMdioWrite(contextPtr, port,HIGH_ADDRESS_REG, (unsigned short)(byteIndex>>16));
        /* Set the size of the buffer we're going to send*/
        SFPhyMdioWrite(contextPtr, port,ACTUAL_BUFF_SIZE_IN_REG, maxBuffSize/2);
        /* Tell the slave we've written the start address and size
        / and now we're going to start writing data*/
        SFPhyMdioWrite(contextPtr, port,COMMAND_REG,FILL_BUFFER);

        /* Wait for OK*/
        tmp=SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        while(tmp == FILL_BUFFER) {
            SFPhyWait(contextPtr,1);
            tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        } if (tmp != DOWNLOAD_OK) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"Expected %d from slave got %d. Exiting...\n", OK, tmp);
            return fatal_error();
#endif
            printf("ERR_START_WRITE_DATA and rc = %X\n",tmp);
            return ERR_START_WRITE_DATA;

        }

        /* Write a buffer of data to the slave RAM*/
        stopIndex = byteIndex + maxBuffSize;
        buf_checksum = 0;
        while(byteIndex < stopIndex) {
            unsigned short value;

            value = data[byteIndex++];
            value |= (((unsigned short)data[byteIndex++]) << 8);
            buf_checksum += value;
            SFPhyMdioWrite(contextPtr, port,DATA_REG,value);
        }

#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"."); /* Amuse the user*/
#endif

        /* check and see if we can go on to the write*/
        tmp_checksum = SFPhyMdioRead(contextPtr, port,CHECKSUM_REG);
        words_rcvd = SFPhyMdioRead(contextPtr, port,WORDS_RCVD_REG);
        if (tmp_checksum != buf_checksum || words_rcvd != maxBuffSize/2) {
            /* Host might want to issue a retry here instead failing*/
            /* Note that the flash start address must be reset before resending the buffer*/
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr, "Slave failed to get all the data correctly\n");
            return fatal_error();
#endif
            return ERR_SLAVE_FAIL_TO_GET_DATA;

        }

        /* One full RAM buffer inside DSP is ready to write to flash now*/
        /* Tell the slave to write it*/
#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"\nTelling slave to save %d bytes of buffer %d\n",
            (int)maxBuffSize, (int)transferIndex);
#endif

        SFPhyMdioWrite(contextPtr, port,COMMAND_REG,WRITE_BUFFER);

#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"Waiting for slave to finish programming flash");
#endif
        /* Wait for OK */
        tmp=SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        while(tmp == WRITE_BUFFER || tmp == FLASH_BUSY) {
            /* this can take several 2-3 seconds, don't poll phy too frequently*/
            SFPhyWait(contextPtr, 500);
                       /* since every read causes an interrupt on the phy */
#if SF_EVK_TEST_FLASHCODE
            amuse_user();
#endif

            tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        }

        if (tmp == ERROR_CMD) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"\nSome kind of error occurred on Slave. Exiting\n");
            return fatal_error();
#endif
            return ERR_ON_SLAVE;

        } else {
            if (tmp != DOWNLOAD_OK) {
#if SF_EVK_TEST_FLASHCODE
                fprintf(stderr,"Expected %d from slave got %d. Exiting...\n", OK, tmp);
                return fatal_error();
#endif
                return ERR_ON_SLAVE;

            } else {
                /* readback checksum of what was stored in flash */
                reported_checksum = SFPhyMdioRead(contextPtr, port,CHECKSUM_REG);
                if (reported_checksum != buf_checksum) {
#if SF_EVK_TEST_FLASHCODE
                    fprintf(stderr,"Expected %d checksum but got %d. Exiting...\n",
                        buf_checksum, reported_checksum);
                    return fatal_error();
#endif
                    return ERR_CHECKSUM;

                }
            }
        }

        if (SFPhyMdioRead(contextPtr, port,WORDS_WRITTEN_REG) != (maxBuffSize/2)) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"\nSlave didn't write enough words to flash. Error occurred. Exit\n");
            return fatal_error();
#endif
            return ERR_SLAVE_WRITE_FULL;

        }
#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"\n");
#endif
    }

    /* now handle last transfer */
    if (lastTransferSize) {

        /* Set the flash start address */
        SFPhyMdioWrite(contextPtr, port,LOW_ADDRESS_REG, (unsigned short)byteIndex);
        SFPhyMdioWrite(contextPtr, port,HIGH_ADDRESS_REG, (unsigned short)(byteIndex>>16));
        /* Set the size of the buffer we're going to send */
        SFPhyMdioWrite(contextPtr, port,ACTUAL_BUFF_SIZE_IN_REG, lastTransferSize/2);
        /* Tell the slave we've written the start address and size */
        /* and now we're going to start writing data */
        SFPhyMdioWrite(contextPtr, port,COMMAND_REG,FILL_BUFFER);

        /* Wait for OK */
        tmp=SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        while(tmp == FILL_BUFFER)
            tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);

        if (tmp != DOWNLOAD_OK) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"Expected %d from slave got %d. Exiting...\n", OK, tmp);
            return fatal_error();
#endif
            return ERR_LAST_TRANSFER;

        }

        /* Write a buffer of data to the slave RAM */
        stopIndex = byteIndex + lastTransferSize;
        buf_checksum = 0;
        while(byteIndex < stopIndex) {
            unsigned short value;

            value = data[byteIndex++];
            value |= (((unsigned short)data[byteIndex++]) << 8);
            buf_checksum += value;
            SFPhyMdioWrite(contextPtr, port,DATA_REG,value);
        }

        /* Last buffer is ready to write to flash now
           Tell the slave to write it */
#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"\nTelling slave to save %d bytes of last buffer\n", (int)lastTransferSize);
#endif
        SFPhyMdioWrite(contextPtr, port,COMMAND_REG,WRITE_BUFFER);

        /* Wait until the slave is finished */
#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"Waiting for slave to finish programming last buffer to flash");
#endif

        /* Wait for OK */
        tmp=SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        while(tmp == WRITE_BUFFER || tmp == FLASH_BUSY) {
            SFPhyWait(contextPtr, 500);  /* this can take several 2-3 seconds,
               don't poll phy too frequently since every read causes an interrupt on the phy */
#if SF_EVK_TEST_FLASHCODE
            amuse_user();
#endif
            tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);
        }

        if (tmp == ERROR_CMD) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"\nSome kind of error occurred on Slave. Exiting\n");
            return fatal_error();
#endif
            return ERR_LAST_TRANSFER;

        } else {
            if (tmp != DOWNLOAD_OK) {
#if SF_EVK_TEST_FLASHCODE
                fprintf(stderr,"Expected %d from slave got %d. Exiting...\n", OK, tmp);
                return fatal_error();
#endif
                return ERR_LAST_TRANSFER;

            } else {
                /* readback checksum of what was stored in flash */
                reported_checksum = SFPhyMdioRead(contextPtr, port,CHECKSUM_REG);
                if (reported_checksum != buf_checksum) {
#if SF_EVK_TEST_FLASHCODE
                    fprintf(stderr,"Expected %d checksum but got %d. Exiting...\n",
                        buf_checksum, reported_checksum);
                    return fatal_error();
#endif
                    return ERR_CHECKSUM;

                }
            }
        }

        if (SFPhyMdioRead(contextPtr, port,WORDS_WRITTEN_REG) != (lastTransferSize/2)) {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"\nSlave didn't write enough words to flash. Error occurred. Exit\n");
            return fatal_error();
#endif
            return ERR_SLAVE_WRITE_FULL;

        }

#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"\n");
#endif
    }

#if SF_EVK_TEST_FLASHCODE
    time(&stop_time);
    download_time = difftime(stop_time,start_time);
    fprintf(stdout,"\nTime = %d seconds\n",(int)download_time);
#endif

    return 0;
}

unsigned short SFPhyIsSolarflarePhy(CTX_PTR_TYPE contextPtr, unsigned long port,
                                    unsigned short *modelNo, unsigned short *revNo)
{
    unsigned short reg_1_2, reg_1_3;

    reg_1_3     = SFPhyMdioRead(contextPtr, port, IEEE_PMA_DEVID_REG);
    *modelNo    = (reg_1_3 & (MANUF_MODEL_NUM_MASK << MANUF_MODEL_NUM_BIT_POS)) >> MANUF_MODEL_NUM_BIT_POS;
    *revNo      = SFPhyMdioRead(contextPtr, port, UPC_VER_REG) & CHIP_ID_MASK;

    if(*modelNo == 8 && *revNo < 3) {
        *modelNo = 9;
    }

    reg_1_2 = SFPhyMdioRead(contextPtr, port,1,2);

    /*  Check the OUI bits to see if they match SolarFlare's OUI */
    if(reg_1_2 == 0x0140 &&
       (((reg_1_3 & 0xFC00)>>10) == 0x16)) {
        return 1;
    } else {
        return 0;
    }

}

/* Returns the PHY type, Revision of the connected device */
STRUCT_DEVICE_TYPE SFPhyGetDeviceType(CTX_PTR_TYPE contextPtr, unsigned long port)
{
    STRUCT_DEVICE_TYPE devType;
    unsigned short manuf_model_num;
    unsigned short rev_num;

    if(SFPhyIsSolarflarePhy(contextPtr, port, &manuf_model_num, &rev_num) == 0)
    {
        devType.devID = SFINVALID;
        return devType;
    }

    switch(manuf_model_num)
    {
        case 8: // SFT9001
            devType.devID       = SFT9001;
            devType.devRev      = REVB;
            switch(rev_num)
            {
                case 3: // SFT9001 B0
                    devType.devSubRev   = SUBREV0;
                    break;
                case 4:
                    devType.devSubRev   = SUBREV1;
                    break;
                case 5:
                    devType.devSubRev   = SUBREV1S;
                    break;
                default:
                    devType.devID = SFINVALID;
            }
            break;
        case 9: // SFT910X
            devType.devID       = SFT910X;
            switch(rev_num)
            {
                case 0: // SFT910X A0
                case 1: // SFT910X A1
                case 2: // SFT910X A2
                    devType.devRev      = REVA;
                    devType.devSubRev   = (DEV_SUB_REV) rev_num;
                    break;
                case 4: // SFT910X B1
                case 5: // SFT910X B2
                    devType.devRev      = REVB;
                    devType.devSubRev   = (DEV_SUB_REV) (rev_num - 3);
                    break;
                case 6: // SFT910X C0
                    devType.devRev      = REVC;
                    devType.devSubRev   = (DEV_SUB_REV) (rev_num - 6);
                    break;
                default:
                    devType.devID = SFINVALID;
            }
            break;
        default:
            devType.devID = SFINVALID;
    }

    return devType;
}

/* Returns the memory size available in the connected device for application code */
MEM_SIZE_BYTES SFPhyGetDevMemorySize(CTX_PTR_TYPE contextPtr, unsigned long port)
{
    STRUCT_DEVICE_TYPE devType;
    MEM_SIZE_BYTES ramSize;

    devType = SFPhyGetDeviceType(contextPtr, port);

    switch(devType.devID)
    {
        case SFT9001:                       // SFT9001 all revisions have 192KB of RAM
            ramSize = (192UL*1024UL);
            break;
        case SFT910X:
            switch(devType.devRev)
            {
                case REVA:                      // SFT910X revision A has 192KB of RAM
                    ramSize = (192UL*1024UL);
                    break;
                case REVB:                      // SFT910X revisions B and C have 256KB of RAM
                case REVC:
                default:
                    ramSize = (256UL*1024UL);
                break;
            }
            break;
        case SFINVALID:
        default:
            ramSize = BOOT_RAM_USAGE_BYTES;
    }

    return(ramSize - BOOT_RAM_USAGE_BYTES);
}

unsigned short SFPhyDownLoadFlash (CTX_PTR_TYPE contextPtr, unsigned long port,
                                   unsigned char *appData, unsigned long appSize,
                                   unsigned char *slaveData, unsigned int slaveSize)
{
    unsigned tmp;           /* holds result of an MdioRead() */
    int flash_result;       /* result of writing the image to flash */
    unsigned char use_ram_checksum;     /* tells the RAM download function whether to -
                                        use hardware RAM checksum register or not */
    STRUCT_DEVICE_TYPE devType;     // to contain info regarding the connected device

    /*******************************************************************************
       Check if the code about to be downloaded can fit into the device's memory
    *******************************************************************************/

    if (appSize > (SFPhyGetDevMemorySize(contextPtr, port) + HEADER_SIZE)) {
        // App size cannot be larger than the device memory size. Code download cannot proceed
#if SF_EVK_TEST_FLASHCODE
        fprintf(stderr,"Image is larger than the device memory size!\n");
        return fatal_error();
#endif
        return ERR_IMAGE_TOO_LARGE_TO_DOWNLOAD; /*0xFFF0*/
    }

    /* Determine whether the connected device has a hardware RAM checksum register */
    devType = SFPhyGetDeviceType(contextPtr, port);
    if(devType.devID == SFT910X && devType.devRev != REVA)  // SFT910X rev B and later have a -
    {                                                       // hardware RAM checksum register
        use_ram_checksum = 1;
    }
    else                                                    // None of the other Solarflare/Marvell PHYs -
    {                                                       // have that register
        use_ram_checksum = 0;
    }

    /* Flash is being updated */

    /*******************************************************************************
             Download slave code to phy's RAM and start it
    *******************************************************************************/
    if (SFPhyMdioRamDownload(contextPtr, slaveData,slaveSize, port, use_ram_checksum))
    {
#if SF_EVK_TEST_FLASHCODE
        return fatal_error();
#endif
        return ERR_DOWNLOAD_TO_RAM;         /*0xFFF1*/
    }

    /* make sure the slave code started */
    if (!((tmp=SFPhyMdioRead(contextPtr, port,3,0xD000)) & (1<<4)))
    {
#if SF_EVK_TEST_FLASHCODE
        fprintf(stderr,"Slave code did not start. Expected bit 4 to be 1, read 0x%04X\n",
            (unsigned)tmp);
        fprintf(stderr,"Slave download failed. Exiting...\n");
        return fatal_error();
#endif
        return ERR_SLAVE_CODE_DID_NOT_START;

    }

    /*******************************************************************************
       Write the image to flash with slave's help
    *******************************************************************************/

    if ((flash_result = SFPhyMdioFlashDownload(contextPtr, port, appData, appSize)))
    {
        return flash_result; /* Some kind of error happened */
    }

    /*******************************************************************************
       Let slave verify image
    *******************************************************************************/

    /* Using slave code to verify image.
       This commands slave to read in entire flash image and calculate checksum and make sure
       checksum matches the checksum in the header. A failure means flash was corrupted.

       Another method would be to reset the phy (with FLASH_CFG[1]= 0) and see that the new code
       starts successfully, since a bad checksum will result in the code not being started */

#if SF_EVK_TEST_FLASHCODE
    fprintf(stdout,"Flash programming complete. Verifying image via slave.\n");
#endif

    SFPhyMdioWrite(contextPtr, port,COMMAND_REG,VERIFY_FLASH);

    tmp=SFPhyMdioRead(contextPtr, port,COMMAND_REG);
    while(tmp == VERIFY_FLASH || tmp == FLASH_BUSY)
    {
        SFPhyWait(contextPtr, 500);  /* Don't poll phy too frequently,
                                        every read causes an interrupt */
#if SF_EVK_TEST_FLASHCODE
        amuse_user();
#endif

        tmp = SFPhyMdioRead(contextPtr, port,COMMAND_REG);
    }

    if (tmp == VERIFY_OK)
    {
#if SF_EVK_TEST_FLASHCODE
        fprintf(stdout,"\nFlash image verified. Reset F_CFG1 to 0 and reboot to execute new code\n");
#endif
        return 1;   /*Flash download complete */
    }
    else
    {
        if (tmp == VERIFY_ERR)
        {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"\nFlash verifed FAILED! Flash probably corrupted. Re-try download.\n");
            return fatal_error();
#endif
            return ERR_VERIFY_ERR;      /*0xFFF3*/
        }
        else
        {
#if SF_EVK_TEST_FLASHCODE
            fprintf(stderr,"\nExpected %d from slave got %d. Exiting...\n", OK, tmp);
            return fatal_error();
#endif
            return ERR_UNKNOWN_DOWNLOAD_TO_FLASH_FAIL;
        }
    }
}

void SFPhyRemovePhyDownloadMode(CTX_PTR_TYPE contextPtr, unsigned long port)
{
    char s[20];
    fprintf(stdout,"Download completed\n");
    fprintf(stdout,"Set corresponding DIP switch to put pin F_CFG1 to 0 (off) to run code from flash\n");
    fprintf(stdout,"Hit <CR> to continue\n");
    fgets(s, sizeof(s), stdin);
    SFPhySpecialSoftwareReset(contextPtr, port);
}

/*-------------------------------------------------
 * $Log: diag_xaui_88X3120_lib.c,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:20  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/27 04:49:36  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.7  2012/09/21 11:49:42  kody
 * Fix the 88X3120 FW download issue.
 *
 * Revision 1.6  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/07/05 02:07:27  kody
 * Add Phy 3120 FW download utility.
 *
 * Revision 1.3  2012/04/16 02:41:39  kody
 * Clean up the 88X3120 test code.
 *
 * Revision 1.2  2012/03/26 07:21:44  kody
 * Add stdio.h
 *
 * Revision 1.1  2012/02/10 07:10:46  leslie
 * Add Woodlawn phy 88X3120 lib file.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

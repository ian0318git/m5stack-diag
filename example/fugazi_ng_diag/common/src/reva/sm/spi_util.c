/* $Id: spi_util.c,v 1.3 2017/04/17 07:35:01 umlin Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/reva/sm/spi_util.c,v $
 *
 * spi_util.c - SPI flash diagnostic utilities, referenced in Utilities Menu
 *
 * Umi -- Nov. 2016
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "zynq_qspi.h"

#define QSPI_SIZE             0x1000000
#define READ_BUF_SIZE         0x100
#define HEXDUMP_COLS          16
#define MTD_BLOCK_SIZE        0x10000
#define SPI_100T_MTD          "/dev/mtd0"
#define UPGRADE_FOLDER        "/firmware/"
#define ZYNQ_UPGRADE_IMAGE    "reva_zynq_upgrade.bin"
#define A100T_UPGRADE_IMAGE   "reva_a100t_upgrade.bin"
#define TFTP_SERVERIP         "192.123.123.1"

#define UPGRADE_IMAGE_START_ADDR 0x0
#define GOLDEN_IMAGE_START_ADDR  0x680000

extern int spi_write(uint32_t addr, uint32_t bytes, const unsigned char *wrbuf);
extern int spi_read(uint32_t addr, uint32_t bytes, uchar *rdbuf);
extern int spi_erase(uint32_t addr, int end);

/* firmware image */
extern const unsigned char revasm_100t_ugd_fw[];
extern const int revasm_100t_fw_size;
extern int revasm_100t_firmware_upgrade(void);
extern int revasm_7015_tftp_firmware_upgrade(void);
extern int revasm_100t_tftp_firmware_upgrade(void);
extern int zynq_spi_wrtest(void);
extern int zynq_spi_rdtest(void);
extern int zynq_spi_erstest(void);

/******************************************************************************
 * Function: spi_write
 * Description: Open the MTD device for writing data
 * 
 * Input:    addr - offset addr start to write
 *           bytes - how many bytes to write
 *           wrbuf - data buffer will write to flash
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int spi_write(uint32_t addr, uint32_t bytes, const unsigned char *wrbuf)
{
    int fd, retval;

    /* open the mtd device for reading and
     * writing. Note you want mtd0 not mtdblock0
     * also you probably need to open permissions
     * to the dev
     */
    fd = open(SPI_100T_MTD, O_RDWR);
    if(fd < 0) {
        printf("open fail returm %d\n", fd);
        return FAILED;
    }

    /* Go to the addr block */
    retval = lseek(fd, addr, SEEK_SET);
    if(retval < 0) {
        printf("lseek fail return %d\n", retval);
        return FAILED;
    }

    /* Write buffer to flash */
    retval = write(fd, wrbuf, bytes);
    if(retval < 0) {
        printf("write fail return %d\n", retval);
        return FAILED;
    }

    close(fd);
    return PASSED;
}

/******************************************************************************
 * Function: spi_read
 * Description: Open the MTD device for reading data
 *
 * Input:    addr - offset addr start to read
 *           bytes - how many bytes to read
 *           wrbuf - data buffer will read from flash
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int spi_read(uint32_t addr, uint32_t bytes, uchar *rdbuf)
{
    int i, j;
    int fd, retval;

    /* open the mtd device for reading and
     * writing. Note you want mtd0 not mtdblock0
     * also you probably need to open permissions
     * to the dev
     */
    fd = open(SPI_100T_MTD, O_RDWR);
    if(fd < 0) {
        printf("open fail returm %d\n", fd);
        return FAILED;
    }

    /* Go to the addr block */
    retval = lseek(fd, addr, SEEK_SET);
    if(retval < 0) {
        printf("lseek fail return %d\n", retval);
        return FAILED;
    }

    /* Read flash to buffer */
    retval = read(fd, rdbuf, bytes);
    if(retval < 0) {
        printf("read fail return %d\n", retval);
        return FAILED;
    }

    if(bytes > READ_BUF_SIZE) {
        close(fd);
        return PASSED;
    }

    for(i = 0; i < bytes + ((bytes % HEXDUMP_COLS) ? (HEXDUMP_COLS - bytes % HEXDUMP_COLS) : 0); i++) {
        /* print offset */
        if(i % HEXDUMP_COLS == 0) {
            printf("0x%06x: ", i+addr);
        }

        /* print hex data */
        if(i < bytes) {
            printf("%02x ", 0xFF & ((char*)rdbuf)[i]);
        }
        else { /* end of block, just aligning for ASCII dump */
            printf("   ");
        }

        /* print ASCII dump */
        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if(j >= bytes) { /* end of block, not really printing */
                    putchar(' ');
                }
                else if(isprint(((char*)rdbuf)[j])) { /* printable char */
                    putchar(0xFF & ((char*)rdbuf)[j]);        
                }
                else { /* other char */
                    putchar('.');
                }
            }
            putchar('\n');
        }
    }

    close(fd);
    return PASSED;
}

/******************************************************************************
 * Function: spi_erase
 * Description: Open the MTD device for erasing data
 *
 * Input:    addr - offset addr start to erase
 *           end - offset addr end to erase
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int spi_erase(uint32_t addr, int end)
{
    mtd_info_t mtd_info;
    erase_info_t ei;
    int fd;
    int retval;

    /* open the mtd device for reading and
     * writing. Note you want mtd0 not mtdblock0
     * also you probably need to open permissions
     * to the dev
     */
    fd = open(SPI_100T_MTD, O_RDWR);
    if(fd < 0) {
        printf("open fail returm %d\n", fd);
        return FAILED;
    }

    /* et the device info */
    ioctl(fd, MEMGETINFO, &mtd_info);

    /* go to the first block */
    retval = lseek(fd, 0, SEEK_SET);
    if(retval < 0) {
        printf("lseek fail return %d\n", retval);
        return FAILED;
    }

    /* dump it for a sanity check, should match what's in /proc/mtd */
    if (diagflag_xram & D_TRACE) {
        printf("MTD Type: %x\nMTD total size: %x bytes\n \
                MTD erase size: %x bytes\n",
            mtd_info.type, mtd_info.size, mtd_info.erasesize);
    }

    /* set the erase block size */
    ei.length = mtd_info.erasesize;

    for(ei.start = addr; ei.start <= end; ei.start += ei.length)
    {
        ioctl(fd, MEMUNLOCK, &ei);
        /* show the blocks erasing */
        if (diagflag_xram & D_TRACE) {
            printf("Eraseing Block %#x\n", ei.start);
        }
        ioctl(fd, MEMERASE, &ei);
    }    

    close(fd);
    return PASSED;
}

/******************************************************************************
 * Function: revasm_100t_firmware_upgrade
 * Description: Zynq A100T SPI flash upgrade function
 *
 * Input:    N/A
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int revasm_100t_firmware_upgrade()
{
    char* readdata;
    int i = 0;
    int j = 0;
    int rewrite = 0;

    readdata = (uchar *)malloc(revasm_100t_fw_size);

    printf("**************************\n");
    printf("Start upgrade from 0x0, image size = 0x%x\n", revasm_100t_fw_size);

    for(i=0; i<revasm_100t_fw_size;) {
        printf("0x%x / 0x%x \r", i, revasm_100t_fw_size);
        fflush(stdout);

        if( (i+MTD_BLOCK_SIZE) <= revasm_100t_fw_size) {
            spi_erase(i, i);
            spi_write(i, MTD_BLOCK_SIZE, (revasm_100t_ugd_fw+i) );
            i = i + MTD_BLOCK_SIZE;
        }
        else {
            spi_erase(i, i);
            spi_write(i, (revasm_100t_fw_size-i), (revasm_100t_ugd_fw+i) );
            i = revasm_100t_fw_size;
        }
        printf("0x%x / 0x%x \r", i, revasm_100t_fw_size);
        fflush(stdout);
    }
    printf("\n0x%x / 0x%x \n", i, revasm_100t_fw_size);

    printf("**************************\n");
    spi_read(0x0, revasm_100t_fw_size, readdata);
    printf("-- after --\n");
    for(i=0; i<revasm_100t_fw_size; i++)
    {
        if((readdata[i]&0xff) != (revasm_100t_ugd_fw[i]&0xff))
            break;
    }
    if(i == revasm_100t_fw_size) {
        printf("All data same as 0x%x bytes\n", i);
        free(readdata);
        return PASSED;
    }
    else {
        printf("diff data@%x readdata=%x, revasm_100t_ugd_fw=%x", i, readdata[i], revasm_100t_ugd_fw[i]);
        free(readdata);
        return FAILED;
    }
}

/******************************************************************************
 * Function: zynq_spi_wrtest
 * Description: This function performs a write to SPI flash with user specified
 *              offset address and data.
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_spi_wrtest(void)
{
    uint32_t testaddr;
    uchar testbuf[MTD_BLOCK_SIZE];
    uchar testdata;

    testaddr = (uint32_t)gethex_answer("spi flash offset address to read(0x0000000 - 0x1000000) ", 0, 0, QSPI_SIZE);
    testdata = (uchar)getdec_answer("input data to write (0 - 255) ", 0, 0x00, 0xFF);
    if (testaddr < 0x0 || testaddr > QSPI_SIZE) {
        printf("invalid address. testaddr = 0x%x\n", testaddr);
        return FAILED;
    }

    spi_read((testaddr&0xff0000), MTD_BLOCK_SIZE, testbuf);
    testbuf[testaddr%MTD_BLOCK_SIZE] = testdata;
    spi_erase((testaddr&0xff0000), (testaddr&0xff0000));
    spi_write((testaddr&0xff0000), MTD_BLOCK_SIZE, testbuf);

    return PASSED;
}


/******************************************************************************
 * Function: zynq_spi_rdtest
 * Description: This function performs a read from SPI flash with user specified
 *              offset and address.
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_spi_rdtest(void)
{
    uint32_t testaddr;
    uint32_t testlength;
    uchar testbuf[READ_BUF_SIZE];

    testaddr = (uint32_t)gethex_answer("spi flash offset address to read(0x0000000 - 0x1000000) ", 0, 0, QSPI_SIZE);
    testlength = READ_BUF_SIZE;
    if (testlength > QSPI_SIZE - testaddr) {
        printf("invalid data length. testlength = 0x%x\n", testlength);
        return FAILED;
    }
    if (testaddr < 0x0 || testaddr > QSPI_SIZE) {
        printf("invalid address. testaddr = 0x%x\n", testaddr);
        return FAILED;
    }

    spi_read(testaddr, testlength, testbuf);

    return PASSED;
}

/******************************************************************************
 * Function: zynq_spi_erstest
 * Description: This function erase SPI flash in sectors with user specified
 *              offset address and number of sectors.
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_spi_erstest(void)
{
    uchar start_sector;
    uchar end_sector;

    start_sector = (uchar)getdec_answer("input start sector to erase (0 - 255) ", 0, 0x00, 0xFF); 
    end_sector = (uchar)getdec_answer("input end sector to erase (0 - 255) ", 0, 0x00, 0xFF); 
    if (end_sector < start_sector) {
        printf("invalid number of sectors.\n");
        return FAILED;
    }

    spi_erase(start_sector*MTD_BLOCK_SIZE, end_sector*MTD_BLOCK_SIZE);
    printf("%d - %d sectors have been erased.\n", start_sector, end_sector);

    return PASSED;
}

/******************************************************************************
 * Function: revasm_7015_tftp_firmware_upgrade
 * Description: TFTP 7015 upgrade image ZYNQ_UPGRADE_IMAGE from the platform side.
 *
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int revasm_7015_tftp_firmware_upgrade(void)
{
    char cmd[256];
    FILE *fp;
    unsigned char *fw_data;
    int fw_size, i, fw_addr;

    rmdir(UPGRADE_FOLDER);
    mkdir(UPGRADE_FOLDER, 0777);

    sprintf(cmd, "tftp -g -r %s%s -l %s%s %s", UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE, UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE, TFTP_SERVERIP);
    system(cmd);

    fp = fopen(UPGRADE_FOLDER ZYNQ_UPGRADE_IMAGE,"rb");
    if (fp == NULL) {
        printf("Upgrade image isn't exist!\n");
        printf("Please put the upgrade image into platform side as %s%s manually\n", UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE);
        return FAILED;
    }

    /* seek to end of file */
    fseek(fp, 0, SEEK_END);
    /* get current file pointer */
    fw_size = ftell(fp);
    /* seek back to beginning of file */
    fseek(fp, 0, SEEK_SET);

    if(fw_size <= 0) {
        printf("Upgrade image isn't exist!\n");
        printf("Please put the upgrade image into platform side as %s%s manually\n", UPGRADE_FOLDER, ZYNQ_UPGRADE_IMAGE);
        fclose(fp);
        return FAILED;
    }

    fw_data = (unsigned char *)malloc(fw_size * sizeof(char));
    fread(fw_data, fw_size, 1, fp);
    fclose(fp);

    i = (fw_size - 1) / SECTOR_SIZE + 1;
    if (i < 0) {
        i = 0;
    }
    fw_addr = UPGRADE_IMAGE_START_ADDR;

    printf("Erasing flash...\n");
    if (qspi_erase(fw_addr & 0xff0000, i)) {
        cterr('f', 0, "Failed to erase sector.\n");

        return FAILED;
    }

    printf("Writing flash...\n");
    if (qspi_write(fw_addr, fw_size, fw_data, WRITE_CMD)) {
        cterr('f', 0, "Failed to program image.\n");

        return FAILED;
    }
    printf("sector erase. i = %#x, fw_addr = %#x\n", i, fw_addr);
    printf("size  %d\n", fw_size);
    printf("\nFinish standard firmware image upgrade!\n");

    return PASSED;
}

/******************************************************************************
 * Function: revasm_100t_tftp_firmware_upgrade
 * Description: TFTP 100T upgrade image A100T_UPGRADE_IMAGE from the platform side.
 *
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int revasm_100t_tftp_firmware_upgrade(void)
{
    char cmd[256];
    FILE *fp;
    int fw_size;

    rmdir(UPGRADE_FOLDER);
    mkdir(UPGRADE_FOLDER, 0777);

    sprintf(cmd, "tftp -g -r %s%s -l %s%s %s", UPGRADE_FOLDER, A100T_UPGRADE_IMAGE, UPGRADE_FOLDER, A100T_UPGRADE_IMAGE, TFTP_SERVERIP);
    system(cmd);

    fp = fopen(UPGRADE_FOLDER A100T_UPGRADE_IMAGE,"rb");
    if (fp == NULL) {
        printf("Upgrade image isn't exist!\n");
        printf("Please put the upgrade image into platform side as %s%s manually\n", UPGRADE_FOLDER, A100T_UPGRADE_IMAGE);

        return FAILED;
    }

    /* seek to end of file */
    fseek(fp, 0, SEEK_END);
    /* get current file pointer */
    fw_size = ftell(fp);
    /* seek back to beginning of file */
    fseek(fp, 0, SEEK_SET);

    if(fw_size <= 0) {
        printf("Upgrade image isn't exist!\n");
        printf("Please put the upgrade image into platform side as %s%s manually\n", UPGRADE_FOLDER, A100T_UPGRADE_IMAGE);
        fclose(fp);

        return FAILED;
    }

    fclose(fp);
    sprintf(cmd, "flashcp -v %s%s %s", UPGRADE_FOLDER, A100T_UPGRADE_IMAGE, SPI_100T_MTD);
    system(cmd);

    return PASSED;
}
/******** History ******** 
$Log: spi_util.c,v $
Revision 1.3  2017/04/17 07:35:01  umlin
Reva-SM: TFTP upgrade 7015 and A100T by using image in the firmware

Revision 1.2  2017/03/16 05:20:26  umlin
Reva-SM: Commit Reva-SM module side diag codes to main trunk

Revision 1.1.2.2  2016/12/08 07:31:51  umlin
Reva-SM: Depend on FPGA, this version still need to skip MAC loopback and LED test.

Revision 1.1.2.1  2016/12/02 09:15:00  umlin
Reva-SM: A100T flash upgrade and SPI flash util.


$Endlog$
*/

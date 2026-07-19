/* $Id: qspi_util.c,v 1.6 2018/07/23 07:02:21 easochen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/qspi_util.c,v $
 *
 * qspi_util.c - qspi flash diagnostic utilities, referenced in Utilities Menu
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "zynq_qspi.h"

#define STANDARD_FM_IMAGE_START_ADDR    0x0000000

extern int qspi_write(uint32_t addr, uint32_t bytes, uchar *wrbuf, uchar command);
extern int qspi_read(uint32_t addr, uint32_t bytes, uchar command, uchar *rdbuf);
extern int qspi_erase(uint32_t addr, int count);
extern int qspi_linear_read(uchar *recvbufptr, uint32_t addr, uint32_t bytes);
extern int sector_lock_S25FL129P(int secnum);
extern int sector_lock_N25Q128(int secnum);
extern int qspi_rdidcfi(void);
extern int zynq_clear_sr(void);
extern int qspi_S25FL128S_ASP(int secnum, uint32_t aspaddr);
extern int S25FL128S_write_PPBLock(void);
extern uchar S25FL128S_read_PPBLock(void);
extern int S25FL128S_erase_PPB(void);
extern int S25FL128S_write_PPB(int secnum, uint32_t testaddr);
extern uchar S25FL128S_check_PPB(int secnum, uint32_t testaddr);
extern int S25FL128S_read_PPB(int secnum, uint32_t testaddr);
extern int S25FL128S_write_ASP(uchar ASPR);
extern uchar S25FL128S_read_ASP(void);
int flash_type = NULL;

/* firmware image */
extern const unsigned char prince_xp_ugd_fw[];
extern const int prince_xp_ugd_fw_size;



/******************************************************************************
 * Function: zynq_qspi_wrtest
 * Description: This function performs a write to QSPI flash with user specified
 *              offset address, data and number of bytes.
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_qspi_wrtest(void)
{
    uchar value = 0;
    uchar wrcmd;
    uint8_t ans;
    uint32_t testaddr;
    uint32_t testlength;
    int i;
    int secnum = 0;
    uchar *testdata;
    uchar *readdata;
    int ret;

    flash_type = qspi_rdidcfi();

    testaddr = (uint32_t)gethex_answer("qspi flash offset address to write (0x0000000 - 0x1000000) ", 0, 0, QSPI_SIZE);
    testlength = (uint32_t)getdec_answer("number of bytes to write ", 0, 0, QSPI_SIZE);
    if (testlength > QSPI_SIZE - testaddr) {
        printf("invalid address or data length.\n");
        return FAILED;
    }
    if (((testaddr % PAGE_SIZE) != 0) & (testlength > (PAGE_SIZE - testaddr % PAGE_SIZE))) {
        printf("If offset address is not from start of page, data must be in the same page.\n");
        return FAILED;
    }
    testdata = (uchar *)malloc(testlength);
    readdata = (uchar *)malloc(testlength);
    value = (uchar)getdec_answer("input data to write (0 - 255) ", 0, 0x00, 0xFF);
    memset(testdata, value, testlength);

    secnum = (testlength - 1) / SECTOR_SIZE + 1;
    if (secnum < 0) {
        secnum = 0;
    }
    printf("\nDo you use quad page programme command? (y/n): ");
    ans = getchar();
    getchar();
    if (ans == 'y' || ans == 'Y') {
       wrcmd = QUAD_WRITE_CMD;
    } else {
       wrcmd = WRITE_CMD;
    }
    if (qspi_erase((testaddr & 0xff0000), secnum)) {
        cterr('f', 0, "erase failed.\n");
        free(testdata);
        free(readdata);
        if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
            zynq_clear_sr();
        }
        return FAILED;
    }
    if (qspi_write(testaddr, testlength, testdata, wrcmd)) {
        cterr('f', 0, "write failed.\n");
        free(testdata);
        free(readdata);
        if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
            zynq_clear_sr();
        }
        return FAILED;
    }

    /* Read back to verify */
    ret = qspi_read(testaddr, testlength, QUAD_READ_CMD, readdata);
    if (ret) {
        cterr('f', 0, "Read back failed.\n");
        ret = FAILED;
    } else if (memcmp(readdata, testdata, testlength)){
        cterr('f', 0, "write failed: data not match.\n");
        ret = FAILED;
    } else {
        printf("%d bytes have been written successfully.\n", testlength);
        prpass(testpass, "QSPI flash write test passed\n");
        ret = PASSED;
    }

    if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
        zynq_clear_sr();
    }

    free(testdata);
    free(readdata);
    return ret;

}
/******************************************************************************
 * Function: zynq_qspi_rdtest
 * Description: This function performs a read from QSPI flash with user specified
 *              offset address, number of bytes and read command.
 *              no page limit for data to read
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_qspi_rdtest(void)
{
    uchar rdcmd;
    uint32_t testaddr;
    uint32_t testlength;
    int i;
    int ret;
    uchar *testbuf;

    testaddr = (uint32_t)gethex_answer("qspi flash offset address to read(0x0000000 - 0x1000000) ", 0, 0, QSPI_SIZE);
    testlength = (uint32_t)getdec_answer("number of bytes to read ", 0, 0, QSPI_SIZE);
    if (testlength > QSPI_SIZE - testaddr) {
        printf("invalid address or data length.\n");
        return FAILED;
    }
    testbuf = (uchar *)malloc(testlength);
    memset(testbuf, 0, testlength);

    printf("\n Read Options\n");
    printf("a: io mode normal read\n");
    printf("b: io mode fast read\n");
    printf("c: io mode dual read\n");
    printf("d: io mode quad read\n");
    printf("e: linear mode read\n");
    rdcmd = (uchar)getc_answer("select read command ", "abcde", 'a');

    switch (rdcmd) {
    case 'a':
        ret = qspi_read(testaddr, testlength, READ_CMD, testbuf);
        break;
    case 'b':
        ret = qspi_read(testaddr, testlength, FAST_READ_CMD, testbuf);
        break;
    case 'c':
        ret = qspi_read(testaddr, testlength, DUAL_READ_CMD, testbuf);
        break;
    case 'd':
        ret = qspi_read(testaddr, testlength, QUAD_READ_CMD, testbuf);
        break;
    case 'e':
        ret = qspi_linear_read(testbuf, testaddr, testlength);
        break;
    default:
        ret = FAILED;
        break;
    }
    if (ret) {
        cterr('f', 0, "read failed.\n");
        return FAILED;
    }

    printf("\ndata read:\n");
    for(i = 0; i < testlength; i++) {
        printf("%u  ", testbuf[i]);
    }
    printf("\n");
    prpass(testpass, "QSPI flash read test passed\n");
    free(testbuf);
    return PASSED;
}
/******************************************************************************
 * Function: zynq_qspi_erstest
 * Description: This function erase QSPI flash in sectors with user specified
 *              offset address and number of sectors.
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_qspi_erstest(void)
{
    int sector_num = 0;
    uint32_t testaddr;
    int flash_type = qspi_rdidcfi();

    testaddr = (uint32_t)gethex_answer("qspi flash offset address to erase (0x0000000 - 0x0ffffff) ", 0, 0, 0x0ffffff);
    sector_num = getdec_answer("number of sectors to erase ", 0, 0, NUM_SECTORS);
    if (sector_num > (int)(NUM_SECTORS - testaddr/SECTOR_SIZE)) {
        printf("invalid number of sectors.\n");
        return FAILED;
    }

    if (qspi_erase(testaddr & 0xff0000, sector_num)) {
        cterr('f', 0, "erase failed.\n");
        if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
            zynq_clear_sr();
        }
        return FAILED;
    }

    printf("%d sectors have been erased.\n", sector_num);
    prpass(testpass, "QSPI flash erase test passed\n");
    return PASSED;

}

/******************************************************************************
 * Function: zynq_qspi_lock
 * Description: This function prompts users to lock sectors in QSPI flash
 *
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_qspi_lock(void)
{
    int sector_num = 0;
    uint32_t testaddr;

    flash_type = qspi_rdidcfi();
    if (flash_type == -1) {
        return FAILED;
    } else if (flash_type == S25FL129P) {
        sector_num = getdec_answer("number of sectors to lock (0,8,32,128,256) ", 0, 0, NUM_SECTORS);
        if ((sector_num < 0) || (sector_num > 256)) {
            printf("invalid number of sectors.\n");
            return FAILED;
        }
        sector_lock_S25FL129P(sector_num);
    } else if ((flash_type == S25FL128S) || (flash_type == MT25QL128)){
        testaddr = (uint32_t)gethex_answer("sector offset address of protection start (0x0000000 - 0x0ffffff) ", 0, 0, 0x0ffffff);
        sector_num = getdec_answer("number of sectors to lock (0-256) ", 0, 0, NUM_SECTORS);
        if (sector_num > (int)(NUM_SECTORS - testaddr/SECTOR_SIZE)) {
            printf("invalid number of sectors.\n");
            return FAILED;
        }
        if (qspi_S25FL128S_ASP(sector_num, testaddr & 0xff0000)) {
            cterr('f', 0, "S25FL128S Advanced sector lock failed.\n");
            return FAILED;
        }
    /* N25Q128 flash EOL*/
    } else {
        printf("sector lock aborted.\n");
        return PASSED;
    }

    printf("%d sectors have been locked/unlocked.\n", sector_num);
    prpass(testpass, "QSPI flash sector locked/unlocked.\n");
    return PASSED;

}

/******************************************************************************
 * Function: zynq_qspi_sector_write_check
 * Description: This function checks whether a sector is writable.
                It performs a write and see if it passed.
                The data will be restored after checking.
 * Input:    Sector index.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_qspi_sector_write_check(int secidx)
{
    uchar value = 0;
    uint32_t testaddr;
    uchar olddata[SECTOR_SIZE];
    uchar testdata[SECTOR_SIZE];
    uchar readdata[SECTOR_SIZE];
    int ret = PASSED;

    flash_type = qspi_rdidcfi();

    /* Make sure the sector index is valid */
    assert(secidx >= 0 && secidx < 256);
    testaddr = SECTOR_SIZE * secidx;

    /* Read the contents in the sector in order to restore it later */
    if (qspi_read(testaddr, SECTOR_SIZE, READ_CMD, olddata)) {
        cterr('f', 0, "Read sector failed in sector %d.\n", secidx);
        if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
            zynq_clear_sr();
        }
        return FAILED;
    }

    /* Initialize the test data buffer */
    value = 0xcc;
    memset(testdata, value, SECTOR_SIZE);

    /* Erase the sector and write the test data */
    if (qspi_erase(testaddr, 1)) {
        printf("Erase failed in sector %d.\n", secidx);
        if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
            zynq_clear_sr();
        }
        return FAILED;
    }
    if (qspi_write(testaddr, SECTOR_SIZE, testdata, WRITE_CMD)) {
        printf("Write failed in sector %d.\n", secidx);
        if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
            zynq_clear_sr();
        }
        return FAILED;
    }

    /* Read back to verify */
    if (qspi_read(testaddr, SECTOR_SIZE, QUAD_READ_CMD, readdata)) {
        cterr('f', 0, "Read back failed in sector %d.\n", secidx);
        ret = FAILED;
    } else if (memcmp(readdata, testdata, SECTOR_SIZE)){
        if (flash_type == MT25QL128) {
            printf("Write failed in sector %d\n", secidx);
        } else {
            cterr('f', 0, "Write failed in sector %d: data not match.\n", secidx);
        }
        ret = FAILED;
    } else {
        /* Data match, write the original data back */
        if (qspi_erase(testaddr, 1)) {
            cterr('f', 0, "Erase failed in sector %d.\n", secidx);
            ret = FAILED;
        }
        if (qspi_write(testaddr, SECTOR_SIZE, olddata, WRITE_CMD)) {
            cterr('f', 0, "Write original data back failed in sector %d.\n", secidx);
            ret = FAILED;
        }
    }

    if ((flash_type == S25FL128S) || (flash_type == MT25QL128)) {
        zynq_clear_sr();
    }

    return ret;
}

/***********************************************************************************
 *
 * Function: prince_golden_lock_check()
 *
 * Description: This function checks whether the QSPI sectors for golen images
 *              are locked. The sectors will not be writable if they're locked.
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************************/
int prince_golden_lock_check(void)
{
    int i;
    int sector_index;

#if !defined(PLUG_SER) && !defined(REVA) /* Pluggable Serial and Reva didn't protect 0~3 sectors */
    /* Check Golden Firmware, sector write should fail */
    printf("Check sectors %d to %d\n", 
        QSPI_PROTECT_FW_SEC_START, QSPI_PROTECT_FW_SEC_START + QSPI_PROTECT_FW_SECNUM -1);
    sector_index = QSPI_PROTECT_FW_SEC_START;
    for (i = 0; i < QSPI_PROTECT_FW_SECNUM; i++, sector_index++) {
        if (zynq_qspi_sector_write_check(sector_index) == PASSED) {
            cterr('f', 0, "Sector %d should be locked.\n", sector_index);
            return FAILED;
        }
    }
    printf("Sectors %d to %d are locked.\n\n", 
        QSPI_PROTECT_FW_SEC_START, QSPI_PROTECT_FW_SEC_START + QSPI_PROTECT_FW_SECNUM -1);
#endif

#if defined(PLUG_SER) || defined(REVA)
    /* Pluggable Serial:Check Sector 103 and 204, sector write should pass */
    /* REVA: Check Sector 103 and 202, sector write should pass */
    sector_index = QSPI_PROTECT_GOLDENIMG_SEC_START + QSPI_PROTECT_GOLDENIMG_SECNUM;
#else
    /* Check Sector 4 and 153, sector write should pass */
    sector_index = QSPI_PROTECT_FW_SEC_START + QSPI_PROTECT_FW_SECNUM;
#endif
    printf("Check sector %d\n", sector_index);
    if (zynq_qspi_sector_write_check(sector_index) == FAILED) {
        cterr('f', 0, "Write Test Failed for Sector %d.\n", sector_index);
        return FAILED;
    }
    printf("Sector %d is not locked.\n", sector_index);
    sector_index = QSPI_PROTECT_GOLDENIMG_SEC_START - 1;
    printf("Check sector %d\n", sector_index);
    if (zynq_qspi_sector_write_check(sector_index) == FAILED) {
        cterr('f', 0, "Write Test Failed for Sector %d.\n", sector_index);
        return FAILED;
    }
    printf("Sector %d is not locked.\n", sector_index);

    /* Check Golden Image, sector write should fail */
    printf("\nCheck sector %d and %d\n", 
        QSPI_PROTECT_GOLDENIMG_SEC_START, 
        QSPI_PROTECT_GOLDENIMG_SEC_START + QSPI_PROTECT_GOLDENIMG_SECNUM - 1);
    sector_index = QSPI_PROTECT_GOLDENIMG_SEC_START;
    if (zynq_qspi_sector_write_check(sector_index) == PASSED) {
        cterr('f', 0, "Sector %d should be locked.\n", sector_index);
        return FAILED;
    }

    sector_index = QSPI_PROTECT_GOLDENIMG_SEC_START + QSPI_PROTECT_GOLDENIMG_SECNUM - 1;
    if (zynq_qspi_sector_write_check(sector_index) == PASSED) {
        cterr('f', 0, "Sector %d should be locked.\n", sector_index);
        return FAILED;
    }

    printf("Sector %d and %d are locked.\n\n", 
        QSPI_PROTECT_GOLDENIMG_SEC_START, 
        QSPI_PROTECT_GOLDENIMG_SEC_START + QSPI_PROTECT_GOLDENIMG_SECNUM -1);

    printf("QSPI lock test passed.\n");
    return PASSED;
}

int prince_golden_lock_check_all(void)
{
    int i;
    int sector_index;

#if !defined(PLUG_SER) && !defined(REVA) /* Pluggable Serial and Reva didn't protect 0~3 sectors */
    /* Check Golden Firmware, sector write should fail */
    printf("Check sectors %d to %d\n", 
        QSPI_PROTECT_FW_SEC_START, QSPI_PROTECT_FW_SEC_START + QSPI_PROTECT_FW_SECNUM -1);
    sector_index = QSPI_PROTECT_FW_SEC_START;
    for (i = 0; i < QSPI_PROTECT_FW_SECNUM; i++, sector_index++) {
        if (zynq_qspi_sector_write_check(sector_index) == PASSED) {
            cterr('f', 0, "Sector %d should be locked.\n", sector_index);
            return FAILED;
        }
    }
    printf("Sector %d to %d are locked.\n\n", 
        QSPI_PROTECT_FW_SEC_START, QSPI_PROTECT_FW_SEC_START + QSPI_PROTECT_FW_SECNUM -1);
#endif

    /* Check Sector 4 and 153, sector write should pass */
    sector_index = QSPI_PROTECT_FW_SEC_START + QSPI_PROTECT_FW_SECNUM;
    printf("Check sector %d\n", sector_index);
    if (zynq_qspi_sector_write_check(sector_index) == FAILED) {
        cterr('f', 0, "Write Test Failed for Sector %d.\n", sector_index);
        return FAILED;
    }
    printf("Sector %d is not locked.\n", sector_index);
    sector_index = QSPI_PROTECT_GOLDENIMG_SEC_START - 1;
    printf("Check sector %d\n", sector_index);
    if (zynq_qspi_sector_write_check(sector_index) == FAILED) {
        cterr('f', 0, "Write Test Failed for Sector %d.\n", sector_index);
        return FAILED;
    }
    printf("Sector %d is not locked.\n", sector_index);

    /* Check Golden Image, sector write should fail */
    printf("\nCheck sectors %d to %d\n", 
        QSPI_PROTECT_GOLDENIMG_SEC_START, 
        QSPI_PROTECT_GOLDENIMG_SEC_START + QSPI_PROTECT_GOLDENIMG_SECNUM - 1);
    sector_index = QSPI_PROTECT_GOLDENIMG_SEC_START;
    for (i = 0; i < QSPI_PROTECT_GOLDENIMG_SECNUM; i++, sector_index++) {
        if (zynq_qspi_sector_write_check(sector_index) == PASSED) {
            cterr('f', 0, "Sector %d should be locked.\n", sector_index);
            return FAILED;
        }
    }

    printf("Sectors %d to %d are locked.\n\n", 
        QSPI_PROTECT_GOLDENIMG_SEC_START, 
        QSPI_PROTECT_GOLDENIMG_SEC_START + QSPI_PROTECT_GOLDENIMG_SECNUM -1);

    printf("QSPI lock test passed.\n");
    return PASSED;
}

/***********************************************************************************
 *
 * Function: prince_img_lock()
 *
 * Description: This function protects QSPI sectors where firmware and golden images
 *              are located. Only support S25FL128S with Advanced sector protection
 *
 * Prince: protect 0~3, 154~255 sectors
 * Pluggable Serial: protect 104~203 sectors
 * REVA: protect 104~201 sectors
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************************/
int prince_img_lock(void)
{
    int flash_type;
    uchar reg;
    uchar PPBcheck = 0;

    flash_type = qspi_rdidcfi();

    if ((flash_type != S25FL128S) && (flash_type != MT25QL128)) {
        printf("Flash type isn't supported.\n");
        return FAILED;
    } else {
        /* select Persistent Protection Mode, no password used to unprotect the PPB Lock bit.
         * so the PPB Lock bit can only be set to 1 by a power-cycle or hardware reset. 
         */
        reg = S25FL128S_read_ASP();
        if ((reg & 0x06) != 0x04) {
            printf("Writing ASPR. reg & 0x06 = 0x%x\n", (reg & 0x06));
            reg &= 0xFD;
            S25FL128S_write_ASP(reg);
        }
        /* Program PPB bits of sectors where firmware and golden image are located.
         * Read PPBL bit before programming.
         * If PPB bits array is Protected by PPBL, abort protecting.
         */
        if (S25FL128S_read_PPBLock() == 0x01) {
            printf("Programming PPB bits array.\n");
            S25FL128S_erase_PPB();
#if !defined(PLUG_SER) && !defined(REVA) /* Pluggable Serial and Reva didn't protect 0~3 sectors */
            printf("Lock Firmware Address: 0x%x, number of Sectors Locked: %d.\n", 
                QSPI_PROTECT_FW_ADDR, QSPI_PROTECT_FW_SECNUM);
            S25FL128S_write_PPB(QSPI_PROTECT_FW_SECNUM, QSPI_PROTECT_FW_ADDR);
#endif
            printf("Lock Golden Image Address: 0x%x, number of Sectors Locked: %d.\n",
                 QSPI_PROTECT_GOLDENIMG_ADDR, QSPI_PROTECT_GOLDENIMG_SECNUM);
            S25FL128S_write_PPB(QSPI_PROTECT_GOLDENIMG_SECNUM, QSPI_PROTECT_GOLDENIMG_ADDR);
        } else {
            printf("PPB bits array is protected by PPBL, a POR or hardware reset is needed.\n");
            return PASSED;
        }
        /* Check PPB bits of all sectors.
         * If correct, lock the PPB Lock bit so that we cannot change the protection setting.
         */
        PPBcheck = S25FL128S_check_PPB(NUM_SECTORS, STANDARD_FM_IMAGE_START_ADDR);
        if (PPBcheck == QSPI_PROTECT_SECNUM) {
            S25FL128S_write_PPBLock();
        } else {
            cterr('f', 0, "QSPI PPB bits array programming error.\n");
            return FAILED;
        }
        if (S25FL128S_read_PPBLock() != 0x00) {
            cterr('f', 0, "QSPI PPB Lock bit setting error.\n");
            return FAILED;
        }
    }

    prpass(testpass, "QSPI firmware and golden image protection is done.");

    return PASSED;
}

/*****************************************************************
 *
 * Function: prince_firmware_upgrade()
 *
 * Description: This function upgrades standard Prince image in QSPI flash.
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int prince_firmware_upgrade(void)
{
    uint i, write_size;
    unsigned char *prince_image_fw, *fw_data;
    int prince_image_fw_size;
    uint32_t fw_addr;
    int j;

    prpass(testpass, "Start standard firmware image upgrade! ");

    prince_image_fw = (unsigned char *)prince_xp_ugd_fw;
    prince_image_fw_size = prince_xp_ugd_fw_size;

    fw_addr = STANDARD_FM_IMAGE_START_ADDR;
    fw_data = prince_image_fw;
    i = (prince_image_fw_size - 1) / SECTOR_SIZE + 1;
    if (i < 0) {
        i = 0;
    }

    if (qspi_erase(fw_addr & 0xff0000, i)) {
        cterr('f', 0, "Failed to erase sector.\n");
        return (FAILED);
    }

    if (qspi_write(fw_addr, prince_image_fw_size, fw_data, WRITE_CMD)) {
        cterr('f', 0, "Failed to program image.\n");
        return (FAILED);
    }

    printf("sector erase. i = %#x, fw_addr = %#x\n", i, fw_addr);
    printf("size  %d\n", prince_image_fw_size);
#ifdef QSPI_DEBUG
    for (j = 0; j < 256; j++) {
        printf("0x%x  ", *fw_data);
        fw_data += 1;
    }
#endif
    printf("\nFinish standard firmware image upgrade!\n");

    return (PASSED);
}

/******** History ******** 
$Log: qspi_util.c,v $
Revision 1.6  2018/07/23 07:02:21  easochen
Support golden image protection with Micron flash

Revision 1.5  2013/12/20 09:29:10  xiaoyizh
Add test to check whether sectors for golden image are locked.

Revision 1.4  2013/09/23 07:13:48  liwwang
add S25FL128S_check_PPB function to clean up the printout of S25FL128S_read_PPB

Revision 1.3  2013/09/03 07:10:52  liwwang
add support of Spansion S25FL128S QSPI advanced sector lock and image protection utility

Revision 1.2  2013/07/16 04:33:02  liwwang
add sector lock support, rename firmware image array,

Revision 1.1  2013/04/19 07:17:52  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/

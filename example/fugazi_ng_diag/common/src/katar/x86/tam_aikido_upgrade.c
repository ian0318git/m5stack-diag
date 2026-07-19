/* $Id: tam_aikido_upgrade.c,v 1.3 2021/04/15 01:55:08 peteteng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/tam_aikido_upgrade.c,v $
 *------------------------------------------------------------------
 * 
 * Filename   : 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

/*-----------------------------------------------------------------
   External SPI Flash Memory Map
   
            0x00_0000   
                |       4 KiB           SPI Flash Directory Table
            0x00_1000
                |       2044 KiB        TAM working area
            0x20_0000
                |       64 KiB          Image Signing Keys
            0x21_0000
                |       448 KiB         Golden TAM Firmware
            0x28_0000
                |       512 KiB         Upgrade TAM Firmware
            0x30_0000
                |       4 KiB           Boot Version Select
            0x30_1000
                |       824 KiB         Golden FPGA Bitstream
                        0x3C_F000
                |       824 KiB         Upgrade FPGA Bitstream
            0x49_D000
 *-----------------------------------------------------------------
 */ 
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "tam_library.h"
#include "platform_cookie.h"
#include "tam_act2_api_drv_support.h"
#include "tam_aikido_upgrade.h"
#include "tam_aikido_mailbox.h"
#include "queryflags.h"
#include "platform_aikido.h"

#define ASSERT_RC_OK(x) status = x; \
        if (status != TAM_RC_OK) { \
            printf("\n[ERROR] Expected TAM_RC_OK, Status: 0x%02x (%s:%d)\n", \
                    status, __FUNCTION__, __LINE__); \
            goto out; \
        }

uint16_t bus_size = PLATFORM_BUFF_SIZE;
boolean aikido_act2_flag;
boolean aikido_mailbox_flag;

int program_reggio_spi_prom(void);
int program_spi_update_version(void);
static int tam_aikido_get_file(char *, int);

extern int utility_get_rtc (int show);
extern int is_fpga_i2c_scanned_aikido_addr(int);
extern void tam_lib_platform_smbus(void *,boolean); 
extern int katar_plat_init_smart_eeprom_context (sc_context *, uchar, uchar, uchar *);  

/*
 * Function: program_reggio_spi_prom
 *
 * This function perform the UPGRADE process to Aikido eSPI 
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int program_reggio_spi_prom(void)
{
    tam_lib_status_t status = TAM_RC_OK; 
    int fail_flag = 0, ix = 0;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;
    uint32_t addr_offset = 0x0;
    uint16_t data_len = WR_DATA_LEN, mfg_len = 0;
    uint16_t write_data_length = WR_DATA_LEN;
    uint8_t mfg_data[32]={0};
    uint16_t dev_len=0;
    uint8_t dev_data[32]={0};
    tam_lib_scc_id_t scc_id={0};
    tam_lib_chip_info_t chip_info;
    tam_library_version_t tam_library_version;
    int act2_chip, idx = 0;
    int file_size, file_size2, file_size3;
    uint8_t use_interrupt = 0;
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = MBX_REG_BASE_ADDR;
    FILE *fp = NULL, *fp2 = NULL, *fp3 = NULL;
    uint8_t bitstream_buf[BITSTREAM_BUF_SIZE]={0};
    uint8_t bitstream_buf2[BITSTREAM_BUF_SIZE]={0};
    uint8_t bitstream_buf3[SPI_DIR_TABLE_SIZE]={0};
    uint8_t tmp_buf[WR_DATA_LEN] = {0};
    uint8_t fpga_status, fw_status;
    char fpga_file[64] = {0};
    char aikido_file[64] = {0};
    char spidir_file[64] = {0};
    char cmd[64];
    sc_context *con, cont;
    dev_if_info_t dev_if;

    printf("=============================================================\n"
           "= Please prepare FPGA FW / Aikido FW / SPI Table            =\n"          
           "=============================================================\n");

    /* Query FPGA FW filename */
    printf("Please enter FPGA file name [ex:Katar_Secure_Top_update_REL_10012.spi]\n"
           "(Enter q to quit): ");
    fflush(stdout);

    if (tam_aikido_get_file(fpga_file, sizeof(fpga_file)) == FAILED) {
        return (FAILED);
    }

    sprintf(cmd, "cp %s %s", fpga_file, FPGA_FIRMWARE_NAME);
    system(cmd);
    printf("\ncp %s %s", fpga_file, FPGA_FIRMWARE_NAME);
    printf("\n");

    /* Query Aikido FW filename */
    printf("Please enter Aikido FW file name [ex:EncryptedSPIdata_20180316_v_2_4_0004.bin]\n"
           "(Enter q to quit): ");
    fflush(stdout);

    if (tam_aikido_get_file(aikido_file, sizeof(aikido_file)) == FAILED) {
        return (FAILED);
    }

    sprintf(cmd, "cp %s %s", aikido_file, AIKIDO_FIRMWARE_NAME);
    system(cmd);
    printf("\ncp %s %s", aikido_file, AIKIDO_FIRMWARE_NAME);
    printf("\n");

    /* Query SPI Table filename */
    printf("Please enter SPI Directory file name [ex:Katar_Secure_Top_10012.spidir]\n"
           "(Enter q to quit): ");
    fflush(stdout);

    if (tam_aikido_get_file(spidir_file, sizeof(spidir_file)) == FAILED) {
        return (FAILED);
    }

        printf("Start to write image : ");
        utility_get_rtc(TRUE);

    sprintf(cmd, "cp %s %s", spidir_file, SPI_FLASH_TABLE_NAME);
    system(cmd);
    printf("\ncp %s %s", spidir_file, SPI_FLASH_TABLE_NAME);
    printf("\n");

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n(1-I2C ; 2-Device Bus):", 1, 1, 2);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");

        if (system("i2cdetect -y 0 | grep 77 > /dev/null")) {  // init - FPGA I2C
            if (is_fpga_i2c_scanned_aikido_addr(0) == 0) {
                printf("AIKIDO not found on SMBus or FPGA-I2C!\n");
                return (FAILED);
            }
            printf("Use FPGA-I2C interface\n");
            tam_lib_platform_smbus(platform_opaque_handle, FALSE);

            con = &cont;
            dev_if.parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
            dev_if.parm4 = (uint8_t) MB_I2C_CTRL_ACT2; 
            con->dev_if_p = &dev_if;
            act2_init_cont(con);

            write_data_length = WR_DATA_LEN;
        }
        else {  // init - SMBus
            printf("Use SMBus interface\n");
            tam_lib_platform_smbus(platform_opaque_handle, TRUE);
            write_data_length = SMB_WR_DATA_LEN;
        }
    } else {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO Device Bus (Mailbox)\n");
        write_data_length = WR_DATA_LEN;
    }

    if (aikido_mailbox_flag == TRUE) {
        /* Initialize Mailbox */
        status = tam_lib_device_open_mailbox((void *)con, 
                                             use_interrupt, mbx_msg_size, 
                                             mbx_reg_base_addr, &tam_handle);
        if (status != TAM_RC_OK) {
        /* handle error */
            printf("\n ERROR: Cannot Initialize Mailbox. Status %#x\n", status);
            return (FAILED);
        }
    } else {
        int ret_val;
        
        /* I2C Platform Initialize */
        if (katar_diagact2_lib_initialize(I2CBUS0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
            return (FAILED);
        }

        if (tam_handle == NULL) {
            ret_val = tam_lib_device_open(platform_opaque_handle,
                                          PLATFORM_BUFF_SIZE,
                                          &tam_handle);
            if (ret_val != TAM_RC_OK) {
                printf("\n%s: TAM lib: Cannot open handler: status = 0x%x",
                       __func__, ret_val);
                printf("\n%s: tam_handle = %p ", __func__, tam_handle);
                return (FAILED);
            }
        }
    }
    fflush(stdout);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        tam_lib_platform_debug(platform_opaque_handle, TRUE);
    } else {
        tam_lib_platform_debug(platform_opaque_handle, FALSE);
    }

    status = tam_lib_get_chip_info(tam_handle, &chip_info);
    if (status != TAM_RC_OK) {
        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
        goto out;
    }
    printf(" FW Ver     :");
    for (ix = 0; ix < 3; ix++) {
        printf(" %02x", chip_info.fw_version[ix]);
    }
    printf("\n");
    status = tam_lib_scc_read_id(tam_handle, &scc_id);
    if (status != TAM_RC_OK) {
        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
        goto out;
    }
    printf(" Chip Type  : %02x\n", scc_id.chip_type);
    printf(" Chip Vendor: %02x\n", scc_id.chip_vendor);
    printf(" Chip FW Ver: %02x\n", scc_id.firmware_version);
    printf(" Post Result: %02x\n", scc_id.post_result);
    printf(" Chip Mode  : %s\n",scc_id.bus_mode ? "SIMPLE" : "LEGACY");

    tam_lib_get_library_version(&tam_library_version);
    printf("TAM Library Version: %u.%u.%u \n",
            tam_library_version.major,
            tam_library_version.minor,
            tam_library_version.patch);
    printf("\n");    

    /* Open Upgrade FPGA bitstream, copy to internal buffer */
    fp = fopen(FPGA_FIRMWARE_NAME, "rb");
    if (fp == NULL) {
        printf("%s: FPGA Bitstream file open failed!\n", __func__);
        fail_flag++;
        goto out;
    }

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    printf("Open FPGA File Size = %d Bytes!\n", file_size);
    if (file_size == 0x0) {
        fail_flag++;
        printf("%s: Fail! FPGA Bitstream file size is zero!\n", __func__);
        goto out;
    }
        if(file_size>BITSTREAM_BUF_SIZE)
        {
                printf("File size too large, please increase BITSTREAM_BUF_SIZE\n");
                goto out;
        }
    fread(bitstream_buf, 1, file_size, fp);

#ifdef DEBUG_UPGRADE
    for (jx = 0; jx < file_size; jx++) {
        printf("%02x ", bitstream_buf[jx]);
    }
    printf("\n");
#endif

    /* Open Aikido Upgrade FW bitstream, copy to internal buffer */
    fp2 = fopen(AIKIDO_FIRMWARE_NAME, "rb");
    if (fp2 == NULL) {
        printf("%s: Aikido Bitstream file open ('%s') failed!\n", __func__, 
               AIKIDO_FIRMWARE_NAME); 
        fail_flag++;
        goto out;
    }

    fseek(fp2, 0L, SEEK_END);
    file_size2 = ftell(fp2);
    fseek(fp2, 0L, SEEK_SET);
    printf("Open AIKIDO File Size = %d Bytes!\n", file_size2);
    if (file_size2 == 0x0) {
        fail_flag++;
        printf("%s: Fail! Aikido Bitstream file size is zero\n", __func__);
        goto out;
    }
    if(file_size2>BITSTREAM_BUF_SIZE)
    {
        printf("File size too large, please increase BITSTREAM_BUF_SIZE\n");
        goto out;
    }
    fread(bitstream_buf2, 1, file_size2, fp2);

    /* Open SPI Flash Directory Table bitstream, copy to internal buffer */
    fp3 = fopen(SPI_FLASH_TABLE_NAME,"rb");
    if (fp3 == NULL) {
        printf("%s: SPI Table Bitstream file open ('%s') failed!\n", __func__, 
               SPI_FLASH_TABLE_NAME); 
        fail_flag++;
        goto out;
    }

    fseek(fp3, 0L, SEEK_END);
    file_size3 = ftell(fp3);
    fseek(fp3, 0L, SEEK_SET);
    printf("Open SPI Table File Size = %d Bytes!\n", file_size3);
    if (file_size3 == 0x0) {
        fail_flag++;
        printf("%s: Fail! SPI Table Bitstream file size is zero\n", __func__);
        goto out;
    }
    if(file_size3>SPI_DIR_TABLE_SIZE)
    {
        printf("File size too large, please increase SPI_DIR_TABLE_SIZE\n");
        goto out;
    }
    fread(bitstream_buf3, 1, file_size3, fp3);

    /* Only perform this test for AIkido device */
    if (scc_id.firmware_version >= TAM_ACT2_AIKIDO_FW) {
        printf("\nAIkido Firmware = 0x%x \n", TAM_ACT2_AIKIDO_FW);
        fflush(stdout);

        /* Get MFG ID */
        ASSERT_RC_OK(tam_lib_espi_get_mfg_id(tam_handle, &mfg_len, mfg_data));

        printf("MFG ID = ");
        for (ix = 0; ix < mfg_len; ix++) {
            printf("%02x ", mfg_data[ix]);
        }
        printf("\n");

        /* Get DEV ID */
        ASSERT_RC_OK(tam_lib_espi_get_dev_id(tam_handle, &dev_len, dev_data));

        printf("DEV ID = ");
        for (ix = 0; ix < dev_len; ix++) {
            printf("%02x ", dev_data[ix]);
        }
        printf("\n");

        ASSERT_RC_OK(tam_lib_espi_read(tam_handle, 0x0, 20, tmp_buf));
        printf("\n============== SPI Directory Table =================");
        printf("\n addr 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("      ");
        for (ix = 0; ix < 16; ix++) {
            printf("%02x ", tmp_buf[ix]);
        }
        printf("\n");
        printf("\n");
        printf("  Golden Start Address = 0x%x\n", *(uint32_t *)&tmp_buf[0]);
        printf("  Golden Version       = 0x%04x\n", *(uint16_t *)&tmp_buf[4]);
        printf("  Update Start Address = 0x%x\n", *(uint32_t *)&tmp_buf[6]);
        printf("  Update Version       = 0x%04x\n", *(uint16_t *)&tmp_buf[10]);

#ifdef USE_OLD_API
        /* Erase FPGA upgrade image area, any write should perform erase first */
        printf("\nErase FPGA upgrade image area, any write should perform erase first\n")
                printf("\nErase 0x%x ~ 0x%x\n",UPDATE_FPGA_START,USER_SPI_ADDR_END);;
        fflush(stdout);
        for (ix = UPDATE_FPGA_START; ix < USER_SPI_ADDR_END; ix += SIZE_64K) {
            ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, ix));
        }
#endif
        /* Write FPGA Upgrade bitstream to SPI */
        addr_offset = 0;

        tam_lib_platform_add_delay(platform_opaque_handle,5000);
        while (addr_offset < file_size) {
            if (addr_offset+data_len > file_size) {
                data_len = file_size - addr_offset;
            } else {
                data_len = write_data_length;
            }
            printf("\nWrite data size %d start from 0x%x (%02d %%), ", data_len, UPDATE_FPGA_START+addr_offset,(addr_offset*100/file_size));
#ifdef USE_OLD_API
            ASSERT_RC_OK(tam_lib_espi_write(tam_handle, UPDATE_FPGA_START+addr_offset, data_len, 
                                            &bitstream_buf[idx]));
#else
                        if (diagflag_xram & D_DEBUG_OPTIONS)
                        {
                printf("[%d]tam_lib_espi_bitstream_upgrade offset:%d,length:%d,idx:%d\n",__LINE__,addr_offset,data_len,idx);
                                for (ix = 0; ix < data_len; ix ++) {
                        printf("%02x ", bitstream_buf[ix]);
                }
                    printf("\n");
                        }
            // New Upgrade API after TAM Lib V2.6
            ASSERT_RC_OK(tam_lib_espi_bitstream_upgrade(tam_handle,
                                                        UPGRADE_FPGA,
                                                        data_len,
                                                        &bitstream_buf[idx],
                                                        addr_offset));
#endif

#ifdef DEBUG_UPGRADE
                        if (diagflag_xram & D_DEBUG_OPTIONS)
                                printf("[%d]tam_lib_espi_read offset:%d,length:%d\n",__LINE__,(UPDATE_FPGA_START+addr_offset),data_len);
            ASSERT_RC_OK(tam_lib_espi_read(tam_handle, (UPDATE_FPGA_START+addr_offset), data_len, 
                                           tmp_buf));

            for (ix = 0; ix < data_len; ix ++) {
                printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");

            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf[idx], data_len) != 0) {
                printf("%s(%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }
            printf("Read back and Compare Pass");
#endif
            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("\nCompare FPGA Upgrade write data and read back data : Pass\n");
        fflush(stdout);
#ifdef USE_OLD_API
        /* Erase Aikido upgrade image area, any write should perform erase first */
        printf("\nErase Aikido upgrade image area, any write should perform erase first\n");
                printf("\nErase 0x%x ~ 0x%x\n",UPDATE_FW_START,GOLDEN_FPGA_START);
        fflush(stdout);
                for (ix = UPDATE_FW_START; ix < GOLDEN_FPGA_START; ix += SIZE_64K) {
            ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, ix));
        }
#endif
        /* Write Upgrade bitstream to SPI */
        addr_offset = 0;
        idx = 0;

        tam_lib_platform_add_delay(platform_opaque_handle,5000);
        while (addr_offset < file_size2) {
            if (addr_offset+data_len > file_size2) {
                data_len = file_size2 - addr_offset;
            } else {
                data_len = write_data_length;
            }
            printf("\nWrite data size %d start from 0x%x (%02d %%), ", data_len, UPDATE_FW_START+addr_offset,(addr_offset*100/file_size2));
#ifdef USE_OLD_API
            ASSERT_RC_OK(tam_lib_espi_write(tam_handle, UPDATE_FW_START+addr_offset, data_len, 
                                            &bitstream_buf2[idx]));
#else 
                        if (diagflag_xram & D_DEBUG_OPTIONS)
                        {
                                printf("[%d]tam_lib_espi_bitstream_upgrade offset:%d,length:%d,idx:%d\n",__LINE__,addr_offset,data_len,idx);
                                for (ix = 0; ix < data_len; ix++) {
                        printf("%02x ", bitstream_buf2[ix]);
                }
                    printf("\n");
                        }
            // New Upgrade API after TAM Lib V2.6
            ASSERT_RC_OK(tam_lib_espi_bitstream_upgrade(tam_handle,
                                                        UPGRADE_FW,
                                                        data_len,
                                                        &bitstream_buf2[idx],
                                                        addr_offset));
#endif

#ifdef DEBUG_UPGRADE
                        if (diagflag_xram & D_DEBUG_OPTIONS)
                printf("[%d]tam_lib_espi_read offset:%d,length:%d\n",__LINE__,(UPDATE_FW_START+addr_offset),data_len);
            ASSERT_RC_OK(tam_lib_espi_read(tam_handle, (UPDATE_FW_START+addr_offset), data_len, 
                                           tmp_buf));

            for (ix = 0; ix < data_len; ix++) {
                printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");

            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf2[idx], data_len) != 0) {
                printf("%s (%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }
            printf("Read back and Compare Pass");
#endif
            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("\nCompare FW upgrade write data and read back data : Pass\n");
        fflush(stdout);

                 /* Perform the eSPI authentication for upgrade image */

        printf("\nStart to Authenticate Image...\n");
        tam_lib_platform_add_delay(platform_opaque_handle,5000);
        ASSERT_RC_OK(tam_lib_espi_authenticate(tam_handle, UPGRADE_IMAGE, &fpga_status, &fw_status));
        if (fpga_status != TAM_RC_OK || fw_status != TAM_RC_OK) {
            printf("ERROR: Authentication Failed!! FPGA_STATUS=0x%02x FW_STATUS=0x%02x\n", 
                   fpga_status, fw_status);
            goto out;
        }
        printf("Done!\n");

#ifdef USE_OLD_API
        printf("\n=========== SPI Directory Table to update ==============");
        printf("\n addr 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("      ");
        for (ix = 0; ix < 16; ix++) {
            printf("%02x ", bitstream_buf3[ix]);
        }
        printf("\n");
        printf("\n");
        printf("  Golden Start Address = 0x%x\n", *(uint32_t *)&bitstream_buf3[0]);
        printf("  Golden Version       = 0x%04x\n", *(uint16_t *)&bitstream_buf3[4]);
        printf("  Update Start Address = 0x%x\n", *(uint32_t *)&bitstream_buf3[6]);
        printf("  Update Version       = 0x%04x\n", *(uint16_t *)&bitstream_buf3[10]);

                printf("\nErase SPI Directory Table area, any write should perform erase first\n");
        printf("\nErase 0x%x ~ 0x%x\n",SPI_DIR_TABLE_START,GOLDEN_FW_START);
        fflush(stdout);
        addr_offset = USER_SPI_ADDR_START;

                uint8_t spidir_buf[WR_DATA_LEN * 4] = {0};

        /* Save SPI Flash Directory Table and User Area */
        for (ix = 0; ix < 4; ix++) {
            ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, 
                        WR_DATA_LEN, &spidir_buf[ix * WR_DATA_LEN]));
        }
        /* Erase SPI Directory Table, any write should perform erase first */
        ASSERT_RC_OK(tam_lib_espi_erase_4k(tam_handle, 0x0));

        /* Write SPI Directory Table to SPI */
        idx = 0;
        for (ix = 0; ix < SPI_DIR_TABLE_SIZE; ix++) {
            spidir_buf[ix] = bitstream_buf3[ix];
        }

        while (addr_offset < WR_DATA_LEN * 4) {
            data_len = WR_DATA_LEN;
            printf("\nWrite data size %d start from 0x%x, ", data_len, addr_offset);
            ASSERT_RC_OK(tam_lib_espi_write(tam_handle, addr_offset, data_len, 
                                            &spidir_buf[idx]));
            ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, data_len, 
                                           tmp_buf));
#ifdef DEBUG_UPGRADE
            for (ix = 0; ix < data_len; ix++) {
                printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");
#endif
            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &spidir_buf[idx], data_len) != 0) {
                printf("%s(%d): Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }
            printf("Read back and Compare Pass");
            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
#else
                uint8_t fpga_ver_tbl[SPI_DIR_FPGA_VER_SIZE] = {0};
                uint16_t fpga_ver,read_ver;

                fpga_ver = *(uint16_t *)&bitstream_buf3[10];
                printf("  Update Version       = 0x%04x\n", fpga_ver);

        fpga_ver_tbl[0] = fpga_ver % 256;
        fpga_ver_tbl[1] = fpga_ver / 256;
        
        /* Update FPGA Version to SPI DIR table */
        ASSERT_RC_OK(tam_lib_espi_write(tam_handle, SPI_DIR_FPGA_VER_OFFSET, SPI_DIR_FPGA_VER_SIZE, 
                                        fpga_ver_tbl));

                /* Read SPI Directory Table, size of 12 Bytes */
        ASSERT_RC_OK(tam_lib_espi_read(tam_handle, SPI_DIR_TABLE_START,
                                       SPI_DIR_TABLE_SIZE, tmp_buf));

                read_ver = *(uint16_t *)&tmp_buf[10];
                if(read_ver != fpga_ver)
                {
                        printf("%s(%d): Data compare mismatch\n", __func__, __LINE__);
            fail_flag++;
            goto out;
                }
                printf("Read back and Compare Pass");
#endif
        printf("\nCompare SPI Directory Table write data and read back data : Pass\n");
        fflush(stdout);

        /* Read SPI Directory Table, size of 12 Bytes */
        ASSERT_RC_OK(tam_lib_espi_read(tam_handle, 0x0, 12, tmp_buf));
        printf("\n============== SPI Directory Table =================");
        printf("\n addr 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("      ");
        for (ix = 0; ix < 16; ix++) {
            printf("%02x ", tmp_buf[ix]);
        }
        printf("\n");
        printf("  Golden Start Address = 0x%x\n", *(uint32_t *)&tmp_buf[0]);
        printf("  Golden Version       = 0x%04x\n", *(uint16_t *)&tmp_buf[4]);
        printf("  Update Start Address = 0x%x\n", *(uint32_t *)&tmp_buf[6]);
        printf("  Update Version       = 0x%04x\n", *(uint16_t *)&tmp_buf[10]);

    } else {
        /* Not Aikido FW, skip the test */
        printf("This is not AIkido FW, skip this test.\n");
    }
    printf("\n\n======= Please power cycle for the new FPGA to take effect.=======\n\n");
        utility_get_rtc(TRUE);

out:
    /* closing device */
    status = tam_lib_device_close(&tam_handle);
    if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR close status=0x%0x-%s ",
                 __FUNCTION__, __LINE__,
                 status, tam_lib_rc2string(status));
        printf("%s-%u close status=0x%0x-%s\n", 
                __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
    }

    /* Close File Descriptor */
    if (fp != NULL) {
        fclose(fp);
    }

    if (fp2 != NULL) {
        fclose(fp2);
    }

    if (fp3 != NULL) {
        fclose(fp3);
    }

    if (fail_flag > 0) {
        printf("\n %d FAILED \n", fail_flag);
        return (FAILED);
    } else {
        printf("\nPASSED \n");
        return (PASSED);
    }
}

int program_aikido_dev_key(void)
{
#define DEV_KEY_START         (0x00200000)

    tam_lib_status_t status = TAM_RC_OK; 
    int fail_flag = 0, ix = 0;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;
    uint32_t addr_offset = 0x0;
    uint16_t data_len = WR_DATA_LEN, mfg_len = 0;
	uint16_t write_data_length = WR_DATA_LEN;
    uint8_t mfg_data[32]={0};
    uint16_t dev_len=0;
    uint8_t dev_data[32]={0};
    tam_lib_scc_id_t scc_id={0};
    tam_lib_chip_info_t chip_info;
    tam_library_version_t tam_library_version;
    int act2_chip, idx = 0;
    int file_size3;
    uint8_t use_interrupt = 0;
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = MBX_REG_BASE_ADDR;
    FILE *fp3 = NULL;
    uint8_t bitstream_buf3[BITSTREAM_BUF_SIZE]={0};
    uint8_t tmp_buf[WR_DATA_LEN] = {0};
	char devkey_file[64] = {0};
    sc_context *con, cont;
    dev_if_info_t dev_if;

    printf("=============================================================\n"
           "= Please prepare dev Key                                    =\n"          
           "=============================================================\n");

	testname("Aikido FPGA dev key update");

    /* Query SPI Table filename */
    printf("Please enter dev key file name\n"
           "(Enter q to quit): ");
    fflush(stdout);

    if (tam_aikido_get_file(devkey_file, sizeof(devkey_file)) == FAILED) {
        return (FAILED);
    }

	printf("Start to write image : ");
	utility_get_rtc(TRUE);

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n(1-I2C ; 2-Device Bus):", 1, 1, 2);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");

        if (system("i2cdetect -y 0 | grep 77 > /dev/null")) {  // init - FPGA I2C
            if (is_fpga_i2c_scanned_aikido_addr(0) == 0) {
                printf("AIKIDO not found on SMBus or FPGA-I2C!\n");
                return (FAILED);
            }
            printf("Use FPGA-I2C interface\n");
            tam_lib_platform_smbus(platform_opaque_handle, FALSE);

            con = &cont;
            dev_if.parm2 = (uint8_t) MB_I2C_ADDR_AIKIDO_ACT2;
            dev_if.parm4 = (uint8_t) MB_I2C_CTRL_ACT2;
            con->dev_if_p = &dev_if;
            act2_init_cont(con);

            write_data_length = WR_DATA_LEN;
        }
        else {  // init - SMBus
            printf("Use SMBus interface\n");
            tam_lib_platform_smbus(platform_opaque_handle, TRUE);
            write_data_length = SMB_WR_DATA_LEN;
        }
    } else {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO Device Bus (Mailbox)\n");
    }

    if (aikido_mailbox_flag == TRUE) {
	    /* Initialize Mailbox */
	    status = tam_lib_device_open_mailbox((void *)con, 
                                             use_interrupt, mbx_msg_size, 
                                             mbx_reg_base_addr, &tam_handle);
	    if (status != TAM_RC_OK) {
            /* handle error */
            printf("\n ERROR: Cannot Initialize Mailbox. Status %#x\n", status);
            return (FAILED);
	    }
    } else {
        int ret_val;
        
        /* I2C Platform Initialize */
		if (katar_diagact2_lib_initialize(I2CBUS0, MB_I2C_ADDR_AIKIDO_ACT2) != PASSED) {
            return (FAILED);
        }
		printf("After katar_diagact2_lib_initialize\n");

        if (tam_handle == NULL) {
            ret_val = tam_lib_device_open(platform_opaque_handle,
                                          PLATFORM_BUFF_SIZE,
                                          &tam_handle);
            if (ret_val != TAM_RC_OK) {
                printf("\n%s: TAM lib: Cannot open handler: status = 0x%x",
                       __func__, ret_val);
                printf("\n%s: tam_handle = %p ", __func__, tam_handle);
                return (FAILED);
            }
        }
    }
    fflush(stdout);

    status = tam_lib_get_chip_info(tam_handle, &chip_info);
    if (status != TAM_RC_OK) {
        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
        goto out;
    }
    printf(" FW Ver     :");
    for (ix = 0; ix < 3; ix++) {
        printf(" %02x", chip_info.fw_version[ix]);
    }
    printf("\n");
    status = tam_lib_scc_read_id(tam_handle, &scc_id);
    if (status != TAM_RC_OK) {
        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
        goto out;
    }
    printf(" Chip Type  : %02x\n", scc_id.chip_type);
    printf(" Chip Vendor: %02x\n", scc_id.chip_vendor);
    printf(" Chip FW Ver: %02x\n", scc_id.firmware_version);
    printf(" Post Result: %02x\n", scc_id.post_result);
    printf(" Chip Mode  : %s\n",scc_id.bus_mode ? "SIMPLE" : "LEGACY");

    tam_lib_get_library_version(&tam_library_version);
    printf("TAM Library Version: %u.%u.%u \n",
            tam_library_version.major,
            tam_library_version.minor,
            tam_library_version.patch);
    printf("\n");    


    /* Open bitstream, copy to internal buffer */
    fp3 = fopen(devkey_file,"rb");
    if (fp3 == NULL) {
        printf("%s: SPI Table Bitstream file open ('%s') failed!\n", __func__, 
               devkey_file); 
        fail_flag++;
        goto out;
    }

    fseek(fp3, 0L, SEEK_END);
    file_size3 = ftell(fp3);
    fseek(fp3, 0L, SEEK_SET);
    printf("Open dev key File Size = %d Bytes!\n", file_size3);
    if (file_size3 == 0x0) {
        fail_flag++;
        printf("%s: Fail! dev key Bitstream file size is zero\n", __func__);
        goto out;
    }
    fread(bitstream_buf3, 1, file_size3, fp3);

    /* Only perform this test for AIkido device */
    if (scc_id.firmware_version >= TAM_ACT2_AIKIDO_FW) {
        printf("\nAIkido Firmware = 0x%x \n", TAM_ACT2_AIKIDO_FW);
        fflush(stdout);

        /* Get MFG ID */
		tam_lib_platform_add_delay(platform_opaque_handle,50);
        ASSERT_RC_OK(tam_lib_espi_get_mfg_id(tam_handle, &mfg_len, mfg_data));

        printf("MFG ID = ");
        for (ix = 0; ix < mfg_len; ix++) {
            printf("%02x ", mfg_data[ix]);
        }
        printf("\n");

        /* Get DEV ID */
		tam_lib_platform_add_delay(platform_opaque_handle,50);
        ASSERT_RC_OK(tam_lib_espi_get_dev_id(tam_handle, &dev_len, dev_data));

        printf("DEV ID = ");
        for (ix = 0; ix < dev_len; ix++) {
            printf("%02x ", dev_data[ix]);
        }
        printf("\n");


		printf("Start erase 0x%x\n",DEV_KEY_START);
		ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, DEV_KEY_START));
		
        addr_offset = 0;
        idx = 0;

		tam_lib_platform_add_delay(platform_opaque_handle,5000);
        while (addr_offset < file_size3) {
            if (addr_offset+data_len > file_size3) {
                data_len = file_size3 - addr_offset;
            } else {
                data_len = write_data_length;
            }
            printf("\nWrite data size %d start from 0x%x (%02d %%), ", data_len, DEV_KEY_START+addr_offset,(addr_offset*100/file_size3));

            ASSERT_RC_OK(tam_lib_espi_write(tam_handle, DEV_KEY_START+addr_offset, data_len, 
                                            &bitstream_buf3[idx]));

            ASSERT_RC_OK(tam_lib_espi_read(tam_handle, (DEV_KEY_START+addr_offset), data_len, 
                                           tmp_buf));
            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf3[idx], data_len) != 0) {
                printf("%s (%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }

            printf("Read back and Compare Pass");
            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("\nCompare dev key write data and read back data : Pass\n");
        fflush(stdout);

		printf("Start tam_lib_soft_reset\n");
		status = tam_lib_soft_reset(tam_handle,0x2000, 0);
		if (status != TAM_RC_OK) {
	        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
    	            status, tam_lib_rc2string(status));
	        fail_flag++;
    	    goto out;
	    }else
			printf("tam_lib_soft_reset finished\n");

    } else {
        /* Not Aikido FW, skip the test */
        printf("This is not AIkido FW, skip this test.\n");
    }
    printf("\n\n======= Please power cycle for the new FPGA to take effect.=======\n\n");
	utility_get_rtc(TRUE);

out:
    /* closing device */
    status = tam_lib_device_close(&tam_handle);
    if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR close status=0x%0x-%s ",
                 __FUNCTION__, __LINE__,
                 status, tam_lib_rc2string(status));
        printf("%s-%u close status=0x%0x-%s\n", 
                __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
    }

    /* Close File Descriptor */
    if (fp3 != NULL) {
        fclose(fp3);
    }

    if (fail_flag > 0) {
        printf("\n %d FAILED \n", fail_flag);
        return (FAILED);
    } else {
        printf("\nPASSED \n");
        return (PASSED);
    }
}

/*
 * Function: tam_aikido_get_file
 *
 * This function take user input file and check file exist
 *
 * Inputs: filename - input file name
 *         filesize - the file size
 *
 * Output: PASSED/FAILED
 */
static int tam_aikido_get_file (char *filename, int filesize)
{
    struct stat sts;

    /* Sanity check */
    if (filename == NULL || filesize == 0) {
        printf("%s: Null filename or filesize is zero!\n", __func__);
        return (FAILED);
    }

    get_line(filename, filesize);

    if (strcmp(filename, "q") == 0) {   /* quit */
        return (FAILED);
    } else if (strcmp(filename, "") == 0) {   /* quit */
        return (FAILED);
    }

    if (stat(filename, &sts) == -1) {
                printf("\n%s does not exist \n", filename);
                return (FAILED);
    } else {
        printf("\nFound '%s'\n", filename);
    }

    return (PASSED);
}

/*
 *------------------------------------------------------------------
 * $Log: tam_aikido_upgrade.c,v $
 * Revision 1.3  2021/04/15 01:55:08  peteteng
 * Upgrade to TAM Lib-v3.4.24 based on PRRQ#5091945
 *
 * Revision 1.2  2019/06/14 05:24:52  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.4  2019/03/05 07:29:37  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.3  2019/02/12 08:06:31  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/02/12 01:32:24  mikech2
 * Add program Aikido FPGA DEV keys function
 *
 * Revision 1.1.2.1  2019/01/29 01:54:22  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.6  2018/12/28 09:46:27  peteteng
 * Support Aikido FW upgrade thru UserLogic FPGA I2C
 *
 * Revision 1.1.2.5  2018/12/14 02:06:04  mikech2
 * Fix Akido FPGA SMBus read issue
 *
 * Revision 1.1.2.4  2018/12/12 02:03:39  peteteng
 * Add Aikido FW upgrade through LPC
 *
 * Revision 1.1.2.3  2018/12/06 08:32:25  mikech2
 * Fine-tune Aikido I2C r/w and fix Aikido update FW utility
 *
 * Revision 1.1.2.2  2018/11/30 06:19:19  mikech2
 * Modify Aikido eSPI memory map
 *
 * Revision 1.1.2.1  2018/10/22 08:02:32  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.1  2018/07/10 07:50:31  mikech2
 * Add security FPGA FW update util
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/* $Id: tam_aikido_upgrade.c,v 1.5 2021/04/14 03:22:09 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tam_aikido_upgrade.c,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: tam_aikido_upgrade.c
 *
 * Aug 2016 - Ian Chang porting the code
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*-----------------------------------------------------------------
   External SPI Flash Memory Map �V 4MB
   
   FPGA     0x00_0000   
   2 MB         |       12B     SPI Flash Directory Table 
            0x00_000C
                |       4084 B  User Area
            0x00_1000
                |       508 KB  Golden TAM firmware
            0x08_0000
                |       512 KB  Upgrade TAM firmware
            0x10_0000
                |       512 KB  Golden FPGA bitstream
            0x18_0000
                |       512 KB  Update FPGA bitstream
            0x20_0000
   TAM          |       2 MB    TAM working area
   2 MB     0x40_0000     
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
#include "act2_utils.h"
#include "nmc93c46.h"
#include "smart_cookie.h"

#define ASSERT_RC_OK(x) status = x; \
        if (status != TAM_RC_OK) { \
            printf("\n[ERROR] Expected TAM_RC_OK, Status: 0x%02x (%s:%d)\n", \
                    status, __FUNCTION__, __LINE__); \
            goto out; \
        }

uint16_t bus_size = PLATFORM_BUFF_SIZE;
int program_reggio_spi_prom(void);
int program_spi_update_version(void);
static int tam_aikido_get_file(char *, int);
extern tam_lib_status_t tam_lib_device_open_mailbox(void *, uint8_t, uint16_t, uint32_t, void **);
unsigned int get_smartfusion2_version(void *);
unsigned int get_upgrade_fgpa_bits_offset(void *);
#ifdef AIKIDO_DEV_KEY
int program_aikido_dev_key(void);
#endif

/*---------------------------------------------------------------
 * Function : get_upgrade_fgpa_bits_offset
 * Description: return spi flash upgrade fpga bitstream offset.
 *            
 * INPUT: *handle - TAM library object
 * OUTPUT: offset - upgrade fpga bitstream offset 
 * --------------------------------------------------------------
 */
unsigned int get_upgrade_fgpa_bits_offset (void *handle) 
{
    unsigned int flash_type, offset = 0; 

    flash_type = get_smartfusion2_version(handle); 
    switch (flash_type) { 
    case AIKIDO_M2S005S: 
        offset = AIKIDO_M2S005S_UP_FGPA_BIT_OFFSET;
    break; 
    /* case AIKIDO_M2S010S: */ /* these 2 using the same offset */
    case AIKIDO_M2S010TS:
        offset = AIKIDO_M2S010TS_UP_FGPA_BIT_OFFSET;
    break; 
    case AIKIDO_M2S025TS: 
        offset = AIKIDO_M2S025TS_UP_FGPA_BIT_OFFSET;
    break; 
    case AIKIDO_M2S050TS: 
        offset = AIKIDO_M2S050TS_UP_FGPA_BIT_OFFSET;
    break; 
    case AIKIDO_M2S060TS: 
        offset = AIKIDO_M2S060TS_UP_FGPA_BIT_OFFSET;
    break; 
    case AIKIDO_M2S150TS: 
        offset = AIKIDO_M2S150TS_UP_FGPA_BIT_OFFSET;
    break; 
    default: 
        printf("Unknown AIKIDO Flash type - 0x%x!!!\n",
               flash_type); 
    break;    
    }

    printf("flash type - 0x%x ;flash offset - 0x%x \n",
           flash_type, offset);  

    return (offset); 
}


/*---------------------------------------------------------------
 * Function : get_smartfusion2_version
 * Description: Read @0x2013 for smartfusion2 version. 
 *              This feature only support AIKIDO 3 and later. 
 * INPUT: *handle - TAM library object
 * OUTPUT: flash type
 * --------------------------------------------------------------
 */
unsigned int get_smartfusion2_version (void *handle) 
{
    tam_lib_status_t status;
    uint8_t send_buffer[8], read_buffer[8]; 
    uint16_t bytes_actually_read, ix, type;  

    /* based on SW suggestion, read from @0x2013 for 
     * smartfusion2 number */
    send_buffer[0] = 0x3; 
    send_buffer[1] = 0x0; 
    send_buffer[2] = 0x20; 
    send_buffer[3] = 0x13; 
    send_buffer[4] = 0x0; 
    send_buffer[5] = 0x0; 
    send_buffer[6] = 0x0; 
    send_buffer[7] = 0x0; 


    status = tam_lib_platform_mbx_read(handle, 
                                       8, send_buffer,
                                       8, read_buffer,
                                       &bytes_actually_read); 
    if (status != TAM_RC_OK) {
        printf("\n tam_lib_platform_mbx_read failed with status 0x%x\n", status);
        return (AIKIDO_UNKNOWN_FLASH_TYPE); 

    } else { 
        for (ix = 0; ix <  4; ix++) {
            if (read_buffer[ix] != 0xFF) {
                printf("mbx read check failed \n"); 
                return (AIKIDO_UNKNOWN_FLASH_TYPE); 
            }
        }
        type = read_buffer[4]; 
        return (type); 
    }
}

/*
 * Function: program_reggio_spi_prom
 *
 * This function perform the UPGRADE process to Aikido eSPI 
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int program_reggio_spi_prom (void)
{
    tam_lib_status_t status = TAM_RC_OK; 
    int fail_flag = 0, ix = 0;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;
    uint32_t addr_offset = 0x0;
    uint16_t data_len = WR_DATA_LEN, mfg_len = 0;
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
    uint8_t spidir_buf[WR_DATA_LEN * 4] = {0};
    char fpga_file[64] = {0};
    char aikido_file[64] = {0};
    char spidir_file[64] = {0};
    char cmd[256];
    unsigned int thre_v2p3 = FALSE; 
    uint8_t fpga_status, fw_status;
    uint32_t update_fgpa_start = 0x0, update_fw_start = 0x0;
    uint32_t upgrade_fgpa_bits_offset = 0x0, upgrade_tam_fw_offset = 0x0;
    sc_context *con, cont;
    con = &cont;


    printf("=============================================================\n"
           "= Please prepare FPGA FW / Aikido FW / SPI Table            =\n"          
           "= in TFTP server at the same location with TSN Diag image   =\n"
           "=============================================================\n");

    /* Query FPGA FW filename */
    printf("Please enter FPGA file name [ex:Top_TSN_update_REL_170717_SB.spi]\n"
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
    printf("Please enter Aikido FW file name [ex:EncryptedSPIdata_release_20170713_v2_2_000C.bin]\n"
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
    printf("Please enter SPI Directory file name [ex:Top_TSN_170717_SB.spidir]\n"
           "(Enter q to quit): ");
    fflush(stdout);

    if (tam_aikido_get_file(spidir_file, sizeof(spidir_file)) == FAILED) {
        return (FAILED);
    }

    sprintf(cmd, "cp %s %s", spidir_file, SPI_FLASH_TABLE_NAME);
    system(cmd);
    printf("\ncp %s %s", spidir_file, SPI_FLASH_TABLE_NAME);
    printf("\n");

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n1-I2C, 2-SPI/Device Bus:", 2, 1, 2);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");
    } else {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO SPI/Device Bus (Mailbox)\n");
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
        if (tam_act2_i2c_initialize() == FAILED) {
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
    status = tam_lib_get_chip_info(tam_handle, &chip_info);
    if (status != TAM_RC_OK) {
        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
        goto out;
    }

    /* CHIP_INFO_FW_VER_BYTE0 = 0x23, 
     * if chip info equal or large than 0x23, 
     * it needs using new upgrade procedure.
     *
     * EDCS-1272160 : 
     * Firmware 2.3 and later version has specific TAM Lib API
     * (tam_lib_espi_bitstream_upgrade) for UPGRADE. 
     * Therefore, erase/write/read is not required anymore for UPGRADE
     *
     **/
    if (chip_info.fw_version[0] >= CHIP_INFO_FW_VER_BYTE0) { 
        printf(" FW Ver equal or large than 0x23  \n"); 
        thre_v2p3 = TRUE;
        update_fgpa_start = UPDATE_FPGA_START_V2P3; 
        update_fw_start = UPDATE_FW_START_V2P3; 
        upgrade_fgpa_bits_offset = get_upgrade_fgpa_bits_offset(tam_handle); 
        upgrade_tam_fw_offset = AIKIDO_UP_TAM_FW_OFFSET; 
    } else {
        printf(" FW Ver less than 0x23  \n"); 
        thre_v2p3 = FALSE; 
        update_fgpa_start = UPDATE_FPGA_START;
        update_fw_start = UPDATE_FW_START; 
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

    if(file_size > BITSTREAM_BUF_SIZE) {
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

    if(file_size2 > BITSTREAM_BUF_SIZE) {
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

    if(file_size3 > SPI_DIR_TABLE_SIZE) {
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

        if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
            /* Erase FPGA upgrade image area, any write should perform erase first */
            printf("\nErase FPGA upgrade image area, any write should perform erase first\n");
            printf("\nErase 0x%x ~ 0x%x\n",update_fgpa_start,USER_SPI_ADDR_END);
            fflush(stdout);

            for (ix = update_fgpa_start; ix < USER_SPI_ADDR_END; ix += SIZE_64K) {
                ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, ix));
            }
        }

        /* Write FPGA Upgrade bitstream to SPI */
        printf("\nUpgrade AIKIDO FPGA:\n");
        addr_offset = update_fgpa_start; 

        while (addr_offset < update_fgpa_start + file_size) {
            if (addr_offset + data_len > update_fgpa_start + file_size) {
                data_len = update_fgpa_start + file_size - addr_offset;
            } else {
                data_len = WR_DATA_LEN; /* 0x400 = 1024 bytes */
            }
            printf("\nWrite data size %d start from 0x%x, ", data_len, addr_offset);

            
            if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
                ASSERT_RC_OK(tam_lib_espi_write(tam_handle, addr_offset, data_len, 
                                                &bitstream_buf[idx]));

                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, data_len, 
                                               tmp_buf));
            } else {
                /* New Upgrade API after TAM Lib V2.6 */
                ASSERT_RC_OK(tam_lib_espi_bitstream_upgrade(tam_handle, UPGRADE_FPGA,
                                                        data_len, &bitstream_buf[idx],
                                                        addr_offset));
                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, upgrade_fgpa_bits_offset + addr_offset, 
                                               data_len, tmp_buf));
            }
#ifdef DEBUG_UPGRADE
            for (ix = 0; ix < data_len; ix ++) {
                printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");
#endif
            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf[idx], data_len) != 0) {
                printf("%s(%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }
            printf("Read back and Compare Pass");
            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("Done!\n");

#ifdef DEBUG_UPGRADE
        printf("\nCompare FPGA Upgrade write data and read back data : Pass\n");
        fflush(stdout);
#endif 

        if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
            /* Erase Aikido upgrade image area, any write should perform erase first */
            printf("\nErase Aikido upgrade image area, any write should perform erase first\n");
            printf("\nErase 0x%x ~ 0x%x\n",update_fw_start,GOLDEN_FPGA_START);
            fflush(stdout);

            for (ix = update_fw_start; ix < GOLDEN_FPGA_START; ix += SIZE_64K) {
                ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, ix));
            }
        }

        /* Write FW Upgrade bitstream to SPI */
        printf("\nUpgrade AIKIDO FW:\n");

        addr_offset = update_fw_start;
        idx = 0;

        while (addr_offset < update_fw_start + file_size2) {
            if (addr_offset + data_len > update_fw_start + file_size2) {
                data_len = update_fw_start + file_size2 - addr_offset;
            } else {
                data_len = WR_DATA_LEN;
            }
            printf("\nWrite data size %d start from 0x%x, ", data_len, addr_offset);

            if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
                ASSERT_RC_OK(tam_lib_espi_write(tam_handle, addr_offset, data_len, 
                                                &bitstream_buf2[idx]));

                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, data_len, 
                                               tmp_buf));

            } else {  /* New Upgrade API after TAM Lib V2.6 */
                ASSERT_RC_OK(tam_lib_espi_bitstream_upgrade(tam_handle,
                                                            UPGRADE_FW,
                                                            data_len,
                                                            &bitstream_buf2[idx],
                                                            addr_offset));
                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, upgrade_tam_fw_offset + addr_offset, 
                                               data_len, tmp_buf));
            }
#ifdef DEBUG_UPGRADE
            for (ix = 0; ix < data_len; ix++) {
                printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");
#endif
            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf2[idx], data_len) != 0) {
                printf("%s (%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }
            printf("Read back and Compare Pass");
            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("Done!\n");

        if (thre_v2p3 == TRUE) { /* fw equal or larger than 2.3 */

            /* Perform the eSPI authentication for upgrade image */
            printf("\nStart to Authenticate Image...\n");
            ASSERT_RC_OK(tam_lib_espi_authenticate(tam_handle, UPGRADE_IMAGE, &fpga_status, &fw_status));
            if (fpga_status != TAM_RC_OK || fw_status != TAM_RC_OK) {
                printf("ERROR: Authentication Failed!! FPGA_STATUS=0x%02x FW_STATUS=0x%02x\n", 
                       fpga_status, fw_status);
                goto out;
            }
            printf("Done!\n");

            /* update spi dir */
            uint8_t fpga_ver_tbl[SPI_DIR_FPGA_VER_SIZE] = {0};

            fpga_ver_tbl[0] = bitstream_buf3[10]; 
            fpga_ver_tbl[1] = bitstream_buf3[11]; 
        
            /* Update FPGA Version to SPI DIR table , 10 , 2*/
            printf("\nUpdate FPGA SPI DIR table...\n");
            ASSERT_RC_OK(tam_lib_espi_write(tam_handle, SPI_DIR_FPGA_VER_OFFSET, SPI_DIR_FPGA_VER_SIZE, 
                                            fpga_ver_tbl));
            printf("Done!\n");
            /* fw 2.3 doesn't need to erase SPI dir, jump to the end */
            goto v2p3_later; 
        }

        printf("\nCompare FW upgrade write data and read back data : Pass\n");
        fflush(stdout);

        printf("\nErase SPI Directory Table area, any write should perform erase first\n");
        printf("\nErase 0x%x ~ 0x%x\n",SPI_DIR_TABLE_START,GOLDEN_FW_START);
        fflush(stdout);
        addr_offset = USER_SPI_ADDR_START;
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
        printf("\nCompare SPI Directory Table write data and read back data : Pass\n");
        fflush(stdout);

v2p3_later:
        /* Read SPI Directory Table, size of 12 Bytes */
        ASSERT_RC_OK(tam_lib_espi_read(tam_handle, 0x0, SPI_DIR_TABLE_SIZE, tmp_buf));
        printf("\n============== SPI Directory Table =================");
        printf("\n addr 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("      ");
        for (ix = 0; ix < SPI_DIR_TABLE_SIZE; ix++) {
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

/*
 * Function: program_reggio_spi_prom_with_mailbox
 *
 * This function perform the UPGRADE process to Aikido eSPI 
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int program_reggio_spi_prom_with_mailbox (void)
{
    tam_lib_status_t status = TAM_RC_OK; 
    int fail_flag = 0, ix = 0;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;
    uint32_t addr_offset = 0x0;
    uint16_t data_len = WR_DATA_LEN, mfg_len = 0;
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
    uint8_t spidir_buf[WR_DATA_LEN * 4] = {0};
    char fpga_file[64] = {0};
    char aikido_file[64] = {0};
    char spidir_file[64] = {0};
    char cmd[256];
    unsigned int thre_v2p3 = FALSE; 
    uint8_t fpga_status, fw_status;
    uint32_t update_fgpa_start = 0x0, update_fw_start = 0x0;
    uint32_t upgrade_fgpa_bits_offset = 0x0, upgrade_tam_fw_offset = 0x0;
    sc_context *con, cont;
    con = &cont;


    printf("=============================================================\n"
           "= Please prepare FPGA FW / Aikido FW / SPI Table            =\n"          
           "= in TFTP server at the same location with TSN Diag image   =\n"
           "=============================================================\n");

    /* Query FPGA FW filename */
    printf("Please enter FPGA file name [ex:Top_TSN_update_REL_170717_SB.spi]\n"
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
    printf("Please enter Aikido FW file name [ex:EncryptedSPIdata_release_20170713_v2_2_000C.bin]\n"
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
    printf("Please enter SPI Directory file name [ex:Top_TSN_170717_SB.spidir]\n"
           "(Enter q to quit): ");
    fflush(stdout);

    if (tam_aikido_get_file(spidir_file, sizeof(spidir_file)) == FAILED) {
        return (FAILED);
    }

    sprintf(cmd, "cp %s %s", spidir_file, SPI_FLASH_TABLE_NAME);
    system(cmd);
    printf("\ncp %s %s", spidir_file, SPI_FLASH_TABLE_NAME);
    printf("\n");

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n1-I2C, 2-SPI:", 2, 1, 2);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");
    } else {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO SPI (Mailbox)\n");
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
        if (tam_act2_i2c_initialize() == FAILED) {
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

    status = tam_lib_get_chip_info(tam_handle, &chip_info);
    if (status != TAM_RC_OK) {
        printf("%s-%u ERROR status=%u-%s \n",  __FUNCTION__, __LINE__,
                status, tam_lib_rc2string(status));
        fail_flag++;
        goto out;
    }

    /* CHIP_INFO_FW_VER_BYTE0 = 0x23, 
     * if chip info equal or large than 0x23, 
     * it needs using new upgrade procedure.
     *
     * EDCS-1272160 : 
     * Firmware 2.3 and later version has specific TAM Lib API
     * (tam_lib_espi_bitstream_upgrade) for UPGRADE. 
     * Therefore, erase/write/read is not required anymore for UPGRADE
     *
     **/
    if (chip_info.fw_version[0] >= CHIP_INFO_FW_VER_BYTE0) { 
        printf(" FW Ver equal or large than 0x23  \n"); 
        thre_v2p3 = TRUE;
        update_fgpa_start = UPDATE_FPGA_START_V2P3; 
        update_fw_start = UPDATE_FW_START_V2P3; 
        upgrade_fgpa_bits_offset = get_upgrade_fgpa_bits_offset(tam_handle); 
        upgrade_tam_fw_offset = AIKIDO_UP_TAM_FW_OFFSET; 
    } else {
        printf(" FW Ver less than 0x23  \n"); 
        thre_v2p3 = FALSE; 
        update_fgpa_start = UPDATE_FPGA_START;
        update_fw_start = UPDATE_FW_START; 
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

    if(file_size > BITSTREAM_BUF_SIZE) {
        printf("File size too large, please increase BITSTREAM_BUF_SIZE\n");
        goto out;
    }

    fread(bitstream_buf, 1, file_size, fp);

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

    if(file_size2 > BITSTREAM_BUF_SIZE) {
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

    if(file_size3 > SPI_DIR_TABLE_SIZE) {
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

        if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
            /* Erase FPGA upgrade image area, any write should perform erase first */
            printf("\nErase FPGA upgrade image area, any write should perform erase first\n");
            printf("\nErase 0x%x ~ 0x%x\n",update_fgpa_start,USER_SPI_ADDR_END);
            fflush(stdout);

            for (ix = update_fgpa_start; ix < USER_SPI_ADDR_END; ix += SIZE_64K) {
                ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, ix));
            }
        }

        /* Write FPGA Upgrade bitstream to SPI */
        printf("\nUpgrade AIKIDO FPGA:\n");
        addr_offset = update_fgpa_start; 

        while (addr_offset < update_fgpa_start + file_size) {
            if (addr_offset + data_len > update_fgpa_start + file_size) {
                data_len = update_fgpa_start + file_size - addr_offset;
            } else {
                data_len = WR_DATA_LEN; /* 0x400 = 1024 bytes */
            }
            printf("\nWrite data size %d start from 0x%x, ", data_len, addr_offset);

            
            if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
                ASSERT_RC_OK(tam_lib_espi_write(tam_handle, addr_offset, data_len, 
                                                &bitstream_buf[idx]));

                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, data_len, 
                                               tmp_buf));
            } else {
                /* New Upgrade API after TAM Lib V2.6 */
                ASSERT_RC_OK(tam_lib_espi_bitstream_upgrade(tam_handle, UPGRADE_FPGA,
                                                        data_len, &bitstream_buf[idx],
                                                        addr_offset));
                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, upgrade_fgpa_bits_offset + addr_offset, 
                                               data_len, tmp_buf));
            }
#ifdef DEBUG_UPGRADE
            for (ix = 0; ix < data_len; ix ++) {
                printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");
#endif 
            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf[idx], data_len) != 0) {
                printf("%s(%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            } 

            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("Done!\n");

#ifdef DEBUG_UPGRADE
        printf("\nCompare FPGA Upgrade write data and read back data : Pass\n");
        fflush(stdout);
#endif 

        if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
            /* Erase Aikido upgrade image area, any write should perform erase first */
            printf("\nErase Aikido upgrade image area, any write should perform erase first\n");
            printf("\nErase 0x%x ~ 0x%x\n",update_fw_start,GOLDEN_FPGA_START);
            fflush(stdout);

            for (ix = update_fw_start; ix < GOLDEN_FPGA_START; ix += SIZE_64K) {
                ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, ix));
            }
        }

        /* Write FW Upgrade bitstream to SPI */
        printf("\nUpgrade AIKIDO FW:\n");

        addr_offset = update_fw_start;
        idx = 0;

        while (addr_offset < update_fw_start + file_size2) {
            if (addr_offset + data_len > update_fw_start + file_size2) {
                data_len = update_fw_start + file_size2 - addr_offset;
            } else {
                data_len = WR_DATA_LEN;
            }
            printf("\nWrite data size %d start from 0x%x, ", data_len, addr_offset);

            if (thre_v2p3 == FALSE) {  /* fw less than v2.3 */
                ASSERT_RC_OK(tam_lib_espi_write(tam_handle, addr_offset, data_len, 
                                                &bitstream_buf2[idx]));

                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, data_len, 
                                               tmp_buf));

            } else {  /* New Upgrade API after TAM Lib V2.6 */
                ASSERT_RC_OK(tam_lib_espi_bitstream_upgrade(tam_handle,
                                                            UPGRADE_FW,
                                                            data_len,
                                                            &bitstream_buf2[idx],
                                                            addr_offset));
                ASSERT_RC_OK(tam_lib_espi_read(tam_handle, upgrade_tam_fw_offset + addr_offset, 
                                               data_len, tmp_buf));
            }
#ifdef DEBUG_UPGRADE
            for (ix = 0; ix < data_len; ix++) {
                 printf("%02x ", tmp_buf[ix]);
            }
            printf("\n");
#endif

            /* compare write data and read back data, should be identical */
            if (memcmp(tmp_buf, &bitstream_buf2[idx], data_len) != 0) {
                printf("%s (%d): Fail! Data compare mismatch\n", __func__, __LINE__);
                fail_flag++;
                goto out;
            }

            /* update index */
            addr_offset += data_len;
            idx += data_len;
        }
        printf("Done!\n");

        if (thre_v2p3 == TRUE) { /* fw equal or larger than 2.3 */

            /* Perform the eSPI authentication for upgrade image */
            printf("\nStart to Authenticate Image...\n");
            ASSERT_RC_OK(tam_lib_espi_authenticate(tam_handle, UPGRADE_IMAGE, &fpga_status, &fw_status));
            if (fpga_status != TAM_RC_OK || fw_status != TAM_RC_OK) {
                printf("ERROR: Authentication Failed!! FPGA_STATUS=0x%02x FW_STATUS=0x%02x\n", 
                       fpga_status, fw_status);
                goto out;
            }
            printf("Done!\n");

            /* update spi dir */
            uint8_t fpga_ver_tbl[SPI_DIR_FPGA_VER_SIZE] = {0};

            fpga_ver_tbl[0] = bitstream_buf3[10]; 
            fpga_ver_tbl[1] = bitstream_buf3[11]; 
        
            /* Update FPGA Version to SPI DIR table , 10 , 2*/
            printf("\nUpdate FPGA SPI DIR table...\n");
            ASSERT_RC_OK(tam_lib_espi_write(tam_handle, SPI_DIR_FPGA_VER_OFFSET, SPI_DIR_FPGA_VER_SIZE, 
                                            fpga_ver_tbl));
            printf("Done!\n");
            /* fw 2.3 doesn't need to erase SPI dir, jump to the end */
            goto v2p3_later; 
        }

        printf("\nCompare FW upgrade write data and read back data : Pass\n");
        fflush(stdout);

        printf("\nErase SPI Directory Table area, any write should perform erase first\n");
        printf("\nErase 0x%x ~ 0x%x\n",SPI_DIR_TABLE_START,GOLDEN_FW_START);
        fflush(stdout);
        addr_offset = USER_SPI_ADDR_START; /* 0 */
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
        printf("\nCompare SPI Directory Table write data and read back data : Pass\n");
        fflush(stdout);

v2p3_later:
        /* Read SPI Directory Table, size of 12 Bytes */
        ASSERT_RC_OK(tam_lib_espi_read(tam_handle, 0x0, SPI_DIR_TABLE_SIZE, tmp_buf));
        printf("\n============== SPI Directory Table =================");
        printf("\n addr 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("      ");
        for (ix = 0; ix < SPI_DIR_TABLE_SIZE; ix++) {
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
    char cmd[128];
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
        printf("\n%s does not exist, downloading...\n", filename);
        if (tftp_get(0, filename, 0, filename, 0) != PASSED) {
            sprintf(cmd, "rm -f %s", filename);
            system(cmd);
            fflush(stdout);
            printf("Fails to TFTP download (%s) to local host\n", filename);
            return (FAILED);
        }
        printf("Download '%s' to local host OK!\n", filename);
    } else {
        printf("\nFound '%s'\n", filename);
    }

    return (PASSED);
}


/*
 * Function: program_spi_update_version
 *
 * This function upgrade FPGA Version in SPI Directory Table 
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int program_spi_update_version(void)
{
    tam_lib_status_t status = TAM_RC_OK; 
    int fail_flag = 0, ix = 0;
    void *tam_handle = NULL;
    void *platform_opaque_handle = NULL;
    tam_lib_scc_id_t scc_id={0};
    uint8_t use_interrupt = 0;
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = MBX_REG_BASE_ADDR;
    tam_library_version_t tam_library_version;
    uint8_t tmp_buf[WR_DATA_LEN] = {0};
    uint16_t fpga_upgrade_version = 0x0;
    tam_lib_chip_info_t chip_info;
    uint32_t addr_offset = 0x0;
    uint8_t spidir_buf[WR_DATA_LEN * 4] = {0};
    int  act2_chip;
    sc_context *con, cont;
    con = &cont;

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n(2-Device Bus):", 2, 1, 2);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");
    } else {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO Device Bus (Mailbox)\n");
    }

    if (aikido_mailbox_flag == TRUE) {
	    /* Initialize Mailbox */
	    status = tam_lib_device_open_mailbox((void *)con, use_interrupt,
                                             mbx_msg_size, mbx_reg_base_addr,
                                             &tam_handle);
	    if (status != TAM_RC_OK) {
        /* handle error */
            printf("\n ERROR: Cannot Initialize Mailbox. Status %#x\n", status);
            return (FAILED);
	    }
    } else {
        int ret_val;
        
        /* I2C Platform Initialize */
        if (tam_act2_i2c_initialize() == FAILED) {
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

    addr_offset = USER_SPI_ADDR_START;
    /* Save SPI Flash Directory Table and User Area */
    for (ix = 0; ix < 4; ix++) {
        ASSERT_RC_OK(tam_lib_espi_read(tam_handle, addr_offset, 
                    WR_DATA_LEN, &spidir_buf[ix * WR_DATA_LEN]));
    }
    printf("\n============== SPI Directory Table =================");
    printf("\n addr 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("      ");
    for (ix = 0; ix < 16; ix++) {
        printf("%02x ", spidir_buf[ix]);
    }
    printf("\n");
    printf("  Golden Start Address = 0x%x\n", *(uint32_t *)&spidir_buf[0]);
    printf("  Golden Version       = 0x%04x\n", *(uint16_t *)&spidir_buf[4]);
    printf("  Update Start Address = 0x%x\n", *(uint32_t *)&spidir_buf[6]);
    printf("  Update Version       = 0x%04x\n", *(uint16_t *)&spidir_buf[10]);
    fpga_upgrade_version = gethex_answer("\nInput new FPGA version:", 0x2711 , 0, 0xffff);
    for (ix = 0; ix < 2; ix++) {
        spidir_buf[10 + ix] = (fpga_upgrade_version >> ix * 8) & 0xff;
    }
    for (ix = 0; ix < 16; ix++) {
        printf("%02x ", spidir_buf[ix]);
    }
    printf("\n");

    printf("\nErase SPI Directory Table area, any write should perform erase first\n");
    printf("\nErase 0x%x ~ 0x%x\n", SPI_DIR_TABLE_START, GOLDEN_FW_START);
    fflush(stdout);
    /* Erase SPI Directory Table, any write should perform erase first */
    ASSERT_RC_OK(tam_lib_espi_erase_4k(tam_handle, 0x0));
    ASSERT_RC_OK(tam_lib_espi_write(tam_handle, SPI_DIR_TABLE_START, SPI_DIR_TABLE_SIZE, 
                                    (uint8_t *)&spidir_buf));
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
out:
    /* closing device */
    status = tam_lib_device_close(&tam_handle);
    if (status != TAM_RC_OK) {
        printf("\n%s-%u ERROR close status=0x%0x-%s ",
                 __FUNCTION__, __LINE__,
                 status, tam_lib_rc2string(status));
        fail_flag++;
    }
    if (fail_flag > 0) {
        printf("\n %d FAILED \n", fail_flag);
        return (FAILED);
    } else {
        printf("\nPASSED \n");
        return (PASSED);
    }
}


#ifdef AIKIDO_DEV_KEY
/*
 * Function: program_aikido_dev_key
 *
 * This function perform the UPGRADE device key process to Aikido eSPI 
 *
 * Inputs: None
 *
 * Output: PASSED/FAILED
 */
int program_aikido_dev_key(void)
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
    int file_size3;
    uint8_t use_interrupt = 0;
    uint16_t mbx_msg_size = 0x700;
    uint32_t mbx_reg_base_addr = MBX_REG_BASE_ADDR;
    FILE *fp3 = NULL;
    uint8_t bitstream_buf3[BITSTREAM_BUF_SIZE]={0};
    uint8_t tmp_buf[WR_DATA_LEN] = {0};
    char devkey_file[128] = {0};
    char   ans;
    sc_context *con, cont;
    con = &cont;

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

    printf("\nSelect Aikido Interface:");
    act2_chip = getdec_answer("\n(2-Device Bus):", 2, 1, 2);
    if (act2_chip == 1) {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = FALSE;
        printf("\nSelect AIKIDO I2C\n");
    } else {
        aikido_act2_flag = TRUE;
        aikido_mailbox_flag = TRUE;
        printf("\nSelect AIKIDO Device Bus (Mailbox)\n");
    }
    printf("This process will ERASE sectors and Re-Program device key.\n");
    printf("If you program wrong key. The Aikido FPGA can't work."
           "Do you really want to do it ?\n");
    printf("(Press 'y/Y' to continue or any other key to Quit) ");

    ans = getchar();

    if (!((ans == 'y') || (ans == 'Y'))) {
            printf("\nProgram SPI PROM is Aborted by User !!!\n");
            return (PASSED);
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
        if (tam_act2_i2c_initialize() == FAILED) {
            return (FAILED);
        }

        printf("After tam_act2_i2c_initialize\n");

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


        printf("Start erase 0x%x\n",DEV_KEY_START);
        ASSERT_RC_OK(tam_lib_espi_erase_64k(tam_handle, DEV_KEY_START));

        addr_offset = 0;
        idx = 0;


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
        } else {
            printf("tam_lib_soft_reset finished\n");
        }

    } else {
        /* Not Aikido FW, skip the test */
        printf("This is not AIkido FW, skip this test.\n");
    }
    printf("\n\n======= Please power cycle for the new FPGA to take effect.=======\n\n");

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
#endif



/*
 *------------------------------------------------------------------
 * $Log: tam_aikido_upgrade.c,v $
 * Revision 1.5  2021/04/14 03:22:09  iachang
 * Fixed tam_lib_device_open_mailbox() input NULL pointer issue.
 *
 * Revision 1.4  2019/08/06 06:56:06  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.3  2019/07/11 12:34:40  alicehua
 * Collapse Nutella codes into main trunk
 *
 * Revision 1.2.112.4  2019/07/08 04:52:14  alicehua
 * Added -DAIKIDO_SUPPORT_AIK flag.
 *
 * Revision 1.2.112.3  2019/04/26 09:00:55  harrchan
 * Base on review comments to clean up code
 *
 * Revision 1.2.112.2  2019/04/09 06:56:57  harrchan
 * Add program/verify aikido dev key
 *
 * Revision 1.2.112.1  2019/03/08 05:45:33  harrchan
 * 1.Support diag to access Aikido FPGA with SPI and I2C interface 2.Add utility for Aikido FPGA upgrade
 *
 * Revision 1.2  2017/08/02 14:21:32  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.6.2  2017/07/29 03:40:50  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.4.3  2017/07/21 09:17:34  iachang
 * clean up code
 *
 * Revision 1.1.4.2  2017/07/20 13:37:59  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.2.3.2.1  2017/07/05 03:32:30  iachang
 * Hidden "2-TAM Mailbox" for cookie / ACT-2 utility
 * Hidden "1-I2C" for FPGA firmware upgrade
 *
 * Revision 1.1.2.3  2016/11/25 07:55:38  steja
 * Add device bus select on SPI Table
 *
 * Revision 1.1.2.2  2016/10/21 13:43:40  steja
 * Fixed the Aikido FW upgrade using i2c.
 *
 * Revision 1.1.2.1  2016/09/13 14:35:39  steja
 * Commit Aikido / TAM Mailbox code
 *
 * Revision 1.1.2.1  2016/08/09 09:50:58  iachang
 * Supported FPGA/Aikido firmware upgrade.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

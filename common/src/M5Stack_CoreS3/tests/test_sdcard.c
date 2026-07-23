/*
 * test_sdcard.c — microSD Card Mount Test (DFS §Storage)
 *
 * On-demand test:
 *   1. Mount microSD card via SPI2 (CS=G4) with FAT32
 *   2. Mount failure → warning (card may be absent)
 *   3. On success: read capacity, write 4KB (0xAA pattern), read back & verify
 *   4. Delete test file, unmount
 *
 * SPI bus is shared with LCD (SPI2).  The sdspi driver manages bus
 * lifecycle — if the LCD already initialised SPI2, sdspi reuses it.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Test parameters per DFS:
 *   - Write 4 KB test file with known pattern (0xAA repeated)
 *   - Read back and verify
 *   - Delete test file and unmount
 */
#define SD_MOUNT_POINT   "/sdcard"
#define SD_TEST_FILE     "/sdcard/diag_test.bin"
#define SD_TEST_SIZE     4096       /* 4 KB */

diag_result_t test_sdcard(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "SDCARD", "MB/SPI");

    diag_menu_printf("\r\nSD Card Test (SPI2 CS=G4)\r\n");

    /*------------------------------------------------------------------------*/
    /* Mount configuration                                                    */
    /*------------------------------------------------------------------------*/

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = CONFIG_LCD_SPI_NUM;   /* SPI2 — same bus as LCD */

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = CONFIG_SD_CS_PIN;
    slot_cfg.host_id = host.slot;

    esp_vfs_fat_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 2,
        .allocation_unit_size = 0,
        .disk_status_check_enable = false,
        .use_one_fat = false,
    };

    /*------------------------------------------------------------------------*/
    /* Mount the card                                                         */
    /*------------------------------------------------------------------------*/

    sdmmc_card_t *card = NULL;
    esp_err_t err = esp_vfs_fat_sdspi_mount(
        SD_MOUNT_POINT, &host, &slot_cfg, &mount_cfg, &card);

    if (err == ESP_ERR_NOT_FOUND) {
        diag_menu_printf("  No card detected (ESP_ERR_NOT_FOUND)\r\n");
        diag_menu_printf("SD Card Test: SKIPPED (insert card and re-run)\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "SPI2 CS=G4: no SD card detected");
            diag_err_set_debug(g_diag_err_ctx,
                               "Insert a FAT32-formatted microSD card",
                               "Check card detect and CS=G4 connection");
        }
        return DIAG_SKIPPED;
    }

    if (err != ESP_OK) {
        diag_menu_printf("  Mount failed: %d\r\n", err);
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "SPI2 CS=G4: mount failed (err=%d)", err);
            diag_err_set_debug(g_diag_err_ctx,
                               "Check card format (FAT32 required)",
                               "Check SPI2: MOSI=G37, MISO=G35, SCLK=G36, CS=G4");
        }
        return DIAG_FAILED;
    }

    /*------------------------------------------------------------------------*/
    /* Card info                                                              */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("  Card mounted at %s\r\n", SD_MOUNT_POINT);
    diag_menu_printf("  Capacity: %llu MB\r\n",
                     (unsigned long long)(card->csd.capacity * card->csd.sector_size / (1024 * 1024)));

    /*------------------------------------------------------------------------*/
    /* Write 4 KB test file                                                   */
    /*------------------------------------------------------------------------*/

    uint8_t *write_buf = (uint8_t *)malloc(SD_TEST_SIZE);
    if (!write_buf) {
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
        return DIAG_ERROR;
    }
    memset(write_buf, 0xAA, SD_TEST_SIZE);

    FILE *f = fopen(SD_TEST_FILE, "wb");
    if (!f) {
        diag_menu_printf("  Failed to create test file\r\n");
        free(write_buf);
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
        return DIAG_FAILED;
    }

    size_t written = fwrite(write_buf, 1, SD_TEST_SIZE, f);
    fclose(f);

    if (written != SD_TEST_SIZE) {
        diag_menu_printf("  Write failed: %u of %u bytes\r\n",
                         (unsigned)written, (unsigned)SD_TEST_SIZE);
        free(write_buf);
        unlink(SD_TEST_FILE);
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
        return DIAG_FAILED;
    }

    diag_menu_printf("  Written %u bytes (pattern 0xAA)\r\n", (unsigned)written);

    /*------------------------------------------------------------------------*/
    /* Read back and verify                                                   */
    /*------------------------------------------------------------------------*/

    uint8_t *read_buf = (uint8_t *)malloc(SD_TEST_SIZE);
    if (!read_buf) {
        free(write_buf);
        unlink(SD_TEST_FILE);
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
        return DIAG_ERROR;
    }

    f = fopen(SD_TEST_FILE, "rb");
    if (!f) {
        diag_menu_printf("  Failed to open test file for read\r\n");
        free(write_buf);
        free(read_buf);
        unlink(SD_TEST_FILE);
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
        return DIAG_FAILED;
    }

    size_t read_bytes = fread(read_buf, 1, SD_TEST_SIZE, f);
    fclose(f);

    int match = 1;
    if (read_bytes != SD_TEST_SIZE) {
        match = 0;
    } else {
        for (size_t i = 0; i < SD_TEST_SIZE; i++) {
            if (read_buf[i] != 0xAA) {
                match = 0;
                break;
            }
        }
    }

    free(write_buf);
    free(read_buf);

    if (!match) {
        diag_menu_printf("  Read-back verification FAILED\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "SD card write/read mismatch");
            diag_err_set_debug(g_diag_err_ctx,
                               "Card may be failing — try known-good card",
                               "Check SPI2 MISO=G35 signal integrity");
        }
        unlink(SD_TEST_FILE);
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
        return DIAG_FAILED;
    }

    diag_menu_printf("  Read-back verified: 0xAA × %u\r\n", (unsigned)SD_TEST_SIZE);

    /*------------------------------------------------------------------------*/
    /* Cleanup                                                                */
    /*------------------------------------------------------------------------*/

    unlink(SD_TEST_FILE);
    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);

    diag_menu_printf("SD Card Test: PASSED\r\n");
    return DIAG_PASSED;
}

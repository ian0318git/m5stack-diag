/*
 * test_sdcard.c — microSD Card Mount Test (DFS §Storage)
 *
 * On-demand test via SPI2 CS=G4 (shares bus with LCD).
 * Uses the shared hal_spi2_bus manager to coexist with the LCD test.
 *
 * Steps:
 *   1. Mount microSD card via SPI2 with FAT32
 *   2. Mount failure → warning (card may be absent)
 *   3. On success: read capacity, write 4 KB (0xAA), read back & verify
 *   4. Delete test file, unmount
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "hal_spi2_bus.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SD_MOUNT_POINT   "/sdcard"
#define SD_TEST_FILE     "/sdcard/diag_test.bin"
#define SD_TEST_SIZE     4096

diag_result_t test_sdcard(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "SDCARD", "MB/SPI");

    diag_menu_printf("\r\nSD Card Test (SPI2 CS=G4)\r\n");

    /* Step 1: Acquire shared SPI2 bus (SD mode: G35 = MISO) */
    if (hal_spi2_bus_init(true) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "SPI2 bus init failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check SPI2 pins: MOSI=G37, MISO=G35, SCLK=G36",
                               "Bus may be in use by another test");
        }
        return DIAG_FAILED;
    }

    /* Step 2: Initialise sdspi_host and add SD device */
    sdspi_host_init();
    sdspi_dev_handle_t sd_handle;
    if (hal_spi2_add_sd_device(&sd_handle) != DIAG_PASSED) {
        diag_menu_printf("  SD SPI device add failed\r\n");
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    /* Step 3: Initialise the SD card */
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = (int)sd_handle;

    sdmmc_card_t *card = (sdmmc_card_t *)malloc(sizeof(sdmmc_card_t));
    if (!card) {
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_ERROR;
    }
    memset(card, 0, sizeof(*card));

    esp_err_t err = sdmmc_card_init(&host, card);
    if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_NOT_FOUND) {
        diag_menu_printf("  No card detected\r\n");
        diag_menu_printf("SD Card Test: SKIPPED (insert card and re-run)\r\n");
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "SPI2 CS=G4: no SD card detected");
            diag_err_set_debug(g_diag_err_ctx,
                               "Insert a FAT32-formatted microSD card",
                               "Check card detect and CS=G4 connection");
        }
        return DIAG_SKIPPED;
    }

    if (err != ESP_OK) {
        diag_menu_printf("  SD card init failed: %d\r\n", err);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    diag_menu_printf("  SD card: %llu MB\r\n",
                     (unsigned long long)(card->csd.capacity * card->csd.sector_size / (1024 * 1024)));

    /* Step 4: Mount FATFS */
    FATFS *fs = NULL;
    esp_vfs_fat_conf_t vfs_conf = {
        .base_path = SD_MOUNT_POINT,
        .fat_drive = "0:",
        .max_files = 2,
    };
    err = esp_vfs_fat_register(&vfs_conf, &fs);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        diag_menu_printf("  VFS register failed: %d\r\n", err);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    FRESULT fr = f_mount(fs, "0:", 1);
    if (fr != FR_OK) {
        diag_menu_printf("  FATFS mount failed: %d\r\n", fr);
        if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    /* Step 5: Write 4 KB test file */
    uint8_t *buf = (uint8_t *)malloc(SD_TEST_SIZE);
    if (!buf) {
        f_mount(NULL, "0:", 0);
        if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_ERROR;
    }
    memset(buf, 0xAA, SD_TEST_SIZE);

    FILE *f = fopen(SD_TEST_FILE, "wb");
    if (!f) {
        diag_menu_printf("  Failed to create test file\r\n");
        free(buf);
        f_mount(NULL, "0:", 0);
        if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    size_t written = fwrite(buf, 1, SD_TEST_SIZE, f);
    fclose(f);

    if (written != SD_TEST_SIZE) {
        diag_menu_printf("  Write failed: %u of %u bytes\r\n",
                         (unsigned)written, (unsigned)SD_TEST_SIZE);
        free(buf);
        unlink(SD_TEST_FILE);
        f_mount(NULL, "0:", 0);
        if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    diag_menu_printf("  Written %u bytes (pattern 0xAA)\r\n", (unsigned)written);

    /* Step 6: Read back and verify */
    f = fopen(SD_TEST_FILE, "rb");
    if (!f) {
        diag_menu_printf("  Failed to open test file for read\r\n");
        free(buf);
        unlink(SD_TEST_FILE);
        f_mount(NULL, "0:", 0);
        if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    size_t read_bytes = fread(buf, 1, SD_TEST_SIZE, f);
    fclose(f);

    int match = (read_bytes == SD_TEST_SIZE);
    if (match) {
        for (size_t i = 0; i < SD_TEST_SIZE; i++) {
            if (buf[i] != 0xAA) { match = 0; break; }
        }
    }
    free(buf);

    if (!match) {
        diag_menu_printf("  Read-back verification FAILED\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "SD card write/read mismatch");
            diag_err_set_debug(g_diag_err_ctx,
                               "Card may be failing — try known-good card",
                               "Check SPI2 MISO=G35 signal integrity");
        }
        unlink(SD_TEST_FILE);
        f_mount(NULL, "0:", 0);
        if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
        free(card);
        hal_spi2_remove_sd_device();
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }

    diag_menu_printf("  Read-back verified: 0xAA × %u\r\n", (unsigned)SD_TEST_SIZE);

    /* Step 7: Cleanup */
    unlink(SD_TEST_FILE);
    f_mount(NULL, "0:", 0);
    if (err == ESP_OK) esp_vfs_fat_unregister_path(SD_MOUNT_POINT);
    free(card);
    hal_spi2_remove_sd_device();
    hal_spi2_bus_deinit();

    diag_menu_printf("SD Card Test: PASSED\r\n");
    return DIAG_PASSED;
}

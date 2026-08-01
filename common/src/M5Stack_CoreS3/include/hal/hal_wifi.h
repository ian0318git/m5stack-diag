/*
 * hal_wifi.h - Hardware Abstraction Layer: ESP32-S3 Wi-Fi Station
 *
 * Board adapter for the ESP32-S3 radio.  Provides station-mode
 * connect/disconnect, status (state, IP, RSSI, channel) and the
 * last disconnect reason code for failure classification.
 *
 * Credentials and service configuration are resolved at runtime:
 * NVS overrides (written by `wifi-set`) take priority over the
 * CONFIG_WIFI_DIAG_* menuconfig defaults.
 *
 * Copyright (c) 2026 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef enum {
    HAL_WIFI_DISCONNECTED = 0,
    HAL_WIFI_CONNECTING,
    HAL_WIFI_CONNECTED,
} hal_wifi_state_t;

typedef struct {
    char    ssid[33];      /* BSSID/SSID of the joined AP           */
    uint8_t ip[4];         /* Assigned IP, 0.0.0.0 if none          */
    int8_t  rssi;          /* Signal strength in dBm                */
    uint8_t channel;       /* AP channel                            */
} hal_wifi_info_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the Wi-Fi driver in station mode (lazy-init).
 * @return DIAG_PASSED on success, DIAG_FAILED if the driver fails.
 */
diag_result_t hal_wifi_init(void);

/**
 * @brief Stop the driver and release radio resources.
 *
 * The esp_netif stack and default event loop stay up (they are shared
 * infrastructure and not reversibly torn down).
 */
void hal_wifi_deinit(void);

/*===========================================================================*/
/* Station control                                                           */
/*===========================================================================*/

/**
 * @brief Connect to an access point and wait for DHCP-assigned IP.
 * @param ssid      AP SSID (max 32 chars).
 * @param password  WPA2 password, may be NULL/empty for open APs.
 * @param timeout_ms Maximum wait for IP acquisition.
 * @return DIAG_PASSED on IP acquired; DIAG_FAILED on timeout.  On
 *         failure, inspect hal_wifi_get_disconnect_reason().
 */
diag_result_t hal_wifi_connect(const char *ssid, const char *password,
                               uint32_t timeout_ms);

/**
 * @brief Disconnect from the AP.
 * @return DIAG_PASSED on success.
 */
diag_result_t hal_wifi_disconnect(void);

/**
 * @brief Current station state.
 */
hal_wifi_state_t hal_wifi_get_state(void);

/**
 * @brief Fill info with the joined AP details.
 * @param[out] info  Filled with SSID/IP/RSSI/channel.
 * @return DIAG_PASSED when joined and info is valid.
 */
diag_result_t hal_wifi_get_info(hal_wifi_info_t *info);

/**
 * @brief Last disconnect reason (wifi_err_reason_t, esp_wifi_types.h).
 * @return Reason code, or -1 if never disconnected.
 */
int hal_wifi_get_disconnect_reason(void);

/*===========================================================================*/
/* Credential / service configuration (NVS wins over Kconfig)                 */
/*===========================================================================*/

diag_result_t hal_wifi_cfg_get_ssid(char *buf, size_t len);
diag_result_t hal_wifi_cfg_get_password(char *buf, size_t len);
diag_result_t hal_wifi_cfg_get_ntp(char *buf, size_t len);
diag_result_t hal_wifi_cfg_get_upload_url(char *buf, size_t len);
diag_result_t hal_wifi_cfg_get_mqtt_url(char *buf, size_t len);

diag_result_t hal_wifi_cfg_set_ssid(const char *s);
diag_result_t hal_wifi_cfg_set_password(const char *s);
diag_result_t hal_wifi_cfg_set_ntp(const char *s);
diag_result_t hal_wifi_cfg_set_upload_url(const char *s);
diag_result_t hal_wifi_cfg_set_mqtt_url(const char *s);

/**
 * @brief Erase all NVS overrides (revert to Kconfig defaults).
 */
diag_result_t hal_wifi_cfg_clear(void);

#ifdef __cplusplus
}
#endif

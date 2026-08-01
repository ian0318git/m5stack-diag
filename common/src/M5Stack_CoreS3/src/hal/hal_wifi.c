/*
 * hal_wifi.c - Hardware Abstraction Layer: ESP32-S3 Wi-Fi Station
 *
 * Board adapter for the ESP32-S3 radio (station mode).  Configuration
 * comes from NVS (runtime `wifi-set` overrides) with fallback to the
 * CONFIG_WIFI_DIAG_* menuconfig defaults.
 *
 * Copyright (c) 2026 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>

#include "hal_wifi.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CFG_NS        "diag_wifi"
#define CFG_KEY_SSID  "ssid"
#define CFG_KEY_PASS  "pass"
#define CFG_KEY_NTP   "ntp"
#define CFG_KEY_URL   "url"
#define CFG_KEY_MQTT  "mqtt"

static const char *TAG = "hal_wifi";

static bool             s_init          = false;
static bool             s_wifi_started  = false;
/* Written by the default event-loop task, polled by the caller task:
   must be volatile to avoid stale caching across tasks. */
static volatile bool    s_got_ip        = false;
static volatile int     s_disconnect_reason = -1;   /* -1 = never disconnected */
static esp_netif_t     *s_sta_netif     = NULL;
static esp_event_handler_instance_t s_wifi_evt_inst = NULL;
static esp_event_handler_instance_t s_ip_evt_inst   = NULL;

/*===========================================================================*/
/* NVS-backed configuration (each call opens/closes its handle)               */
/*===========================================================================*/

static diag_result_t cfg_get(const char *key, char *buf, size_t len,
                             const char *fallback)
{
    nvs_handle_t h;
    if (nvs_open(CFG_NS, NVS_READONLY, &h) == ESP_OK) {
        size_t need = len;
        esp_err_t e = nvs_get_str(h, key, buf, &need);
        nvs_close(h);
        if (e == ESP_OK && buf[0] != '\0')
            return DIAG_PASSED;
        if (e != ESP_ERR_NVS_NOT_FOUND)
            ESP_LOGW(TAG, "nvs_get_str(%s) -> %d", key, e);
    }
    snprintf(buf, len, "%s", fallback ? fallback : "");
    return DIAG_PASSED;
}

static diag_result_t cfg_set(const char *key, const char *val)
{
    nvs_handle_t h;
    if (nvs_open(CFG_NS, NVS_READWRITE, &h) != ESP_OK)
        return DIAG_FAILED;

    esp_err_t e = (val && val[0])
        ? nvs_set_str(h, key, val)
        : nvs_erase_key(h, key);
    if (e == ESP_OK)
        e = nvs_commit(h);
    nvs_close(h);
    return (e == ESP_OK) ? DIAG_PASSED : DIAG_FAILED;
}

diag_result_t hal_wifi_cfg_get_ssid(char *buf, size_t len)
{ return cfg_get(CFG_KEY_SSID, buf, len, CONFIG_WIFI_DIAG_SSID); }

diag_result_t hal_wifi_cfg_get_password(char *buf, size_t len)
{ return cfg_get(CFG_KEY_PASS, buf, len, CONFIG_WIFI_DIAG_PASSWORD); }

diag_result_t hal_wifi_cfg_get_ntp(char *buf, size_t len)
{ return cfg_get(CFG_KEY_NTP, buf, len, CONFIG_WIFI_DIAG_NTP_SERVER); }

diag_result_t hal_wifi_cfg_get_upload_url(char *buf, size_t len)
{ return cfg_get(CFG_KEY_URL, buf, len, CONFIG_WIFI_DIAG_UPLOAD_URL); }

diag_result_t hal_wifi_cfg_get_mqtt_url(char *buf, size_t len)
{ return cfg_get(CFG_KEY_MQTT, buf, len, CONFIG_WIFI_DIAG_MQTT_URL); }

/* Reject values that wifi_config_t would silently truncate
   (ssid[32], password[64]) or that cannot fit a NVS string page. */
diag_result_t hal_wifi_cfg_set_ssid(const char *s)
{
    if (s && strlen(s) > 31)
        return DIAG_FAILED;                       /* wifi_config_t.sta.ssid */
    return cfg_set(CFG_KEY_SSID, s);
}

diag_result_t hal_wifi_cfg_set_password(const char *s)
{
    if (s && strlen(s) > 63)
        return DIAG_FAILED;                       /* wifi_config_t.sta.password */
    return cfg_set(CFG_KEY_PASS, s);
}

diag_result_t hal_wifi_cfg_set_ntp(const char *s)
{
    if (s && strlen(s) > 255)
        return DIAG_FAILED;
    return cfg_set(CFG_KEY_NTP, s);
}

diag_result_t hal_wifi_cfg_set_upload_url(const char *s)
{
    if (s && strlen(s) > 255)
        return DIAG_FAILED;
    return cfg_set(CFG_KEY_URL, s);
}

diag_result_t hal_wifi_cfg_set_mqtt_url(const char *s)
{
    if (s && strlen(s) > 255)
        return DIAG_FAILED;
    return cfg_set(CFG_KEY_MQTT, s);
}

diag_result_t hal_wifi_cfg_clear(void)
{
    nvs_handle_t h;
    if (nvs_open(CFG_NS, NVS_READWRITE, &h) != ESP_OK)
        return DIAG_FAILED;
    esp_err_t e = nvs_erase_all(h);
    if (e == ESP_OK)
        e = nvs_commit(h);
    nvs_close(h);
    return (e == ESP_OK) ? DIAG_PASSED : DIAG_FAILED;
}

/*===========================================================================*/
/* Wi-Fi event handling                                                       */
/*===========================================================================*/

static void wifi_evt(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    /* NOTE: connection is driven explicitly by hal_wifi_connect() —
       the STA_START handler must NOT auto-connect, or the manual
       esp_wifi_connect() below double-connects ("sta is connecting"). */
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_got_ip = false;
        s_disconnect_reason =
            (int)((wifi_event_sta_disconnected_t *)data)->reason;
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_got_ip = true;
    }
}

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

diag_result_t hal_wifi_init(void)
{
    if (s_init)
        return DIAG_PASSED;

    esp_netif_init();                             /* idempotent */
    esp_err_t e = esp_event_loop_create_default();/* idempotent */
    if (e != ESP_OK && e != ESP_ERR_INVALID_STATE)
        return DIAG_FAILED;

    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (!s_sta_netif)
        return DIAG_FAILED;

    wifi_init_config_t wic = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&wic) != ESP_OK)
        return DIAG_FAILED;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        wifi_evt, NULL, &s_wifi_evt_inst);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        wifi_evt, NULL, &s_ip_evt_inst);
    s_init = true;
    return DIAG_PASSED;
}

void hal_wifi_deinit(void)
{
    if (!s_init)
        return;
    if (s_wifi_started) {
        esp_wifi_stop();
        s_wifi_started = false;
    }
    esp_wifi_deinit();
    if (s_wifi_evt_inst) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                              s_wifi_evt_inst);
        s_wifi_evt_inst = NULL;
    }
    if (s_ip_evt_inst) {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                              s_ip_evt_inst);
        s_ip_evt_inst = NULL;
    }
    esp_netif_destroy_default_wifi(s_sta_netif);
    s_sta_netif = NULL;
    s_got_ip = false;
    s_init = false;
    /* esp_netif stack + event loop stay up: shared infrastructure */
}

/*===========================================================================*/
/* Station control                                                           */
/*===========================================================================*/

diag_result_t hal_wifi_connect(const char *ssid, const char *password,
                               uint32_t timeout_ms)
{
    if (hal_wifi_init() != DIAG_PASSED)
        return DIAG_FAILED;

    wifi_config_t cfg = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    snprintf((char *)cfg.sta.ssid, sizeof(cfg.sta.ssid), "%s", ssid);
    snprintf((char *)cfg.sta.password, sizeof(cfg.sta.password), "%s",
             password ? password : "");

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &cfg);

    if (!s_wifi_started) {
        if (esp_wifi_start() != ESP_OK)
            return DIAG_FAILED;
        s_wifi_started = true;
    }

    s_got_ip = false;
    s_disconnect_reason = -1;
    esp_wifi_connect();

    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
        if (s_got_ip)
            return DIAG_PASSED;
        vTaskDelay(pdMS_TO_TICKS(100));
        elapsed += 100;
    }
    return DIAG_FAILED;   /* caller inspects hal_wifi_get_disconnect_reason() */
}

diag_result_t hal_wifi_disconnect(void)
{
    if (!s_init)
        return DIAG_PASSED;
    s_got_ip = false;
    return (esp_wifi_disconnect() == ESP_OK) ? DIAG_PASSED : DIAG_FAILED;
}

hal_wifi_state_t hal_wifi_get_state(void)
{
    if (!s_init)
        return HAL_WIFI_DISCONNECTED;
    if (s_got_ip)
        return HAL_WIFI_CONNECTED;
    return HAL_WIFI_CONNECTING;
}

diag_result_t hal_wifi_get_info(hal_wifi_info_t *info)
{
    if (!info || !s_init)
        return DIAG_FAILED;

    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK)
        return DIAG_FAILED;

    memset(info, 0, sizeof(*info));
    memcpy(info->ssid, ap.ssid, sizeof(info->ssid) - 1);
    info->rssi     = ap.rssi;
    info->channel  = ap.primary;

    esp_netif_ip_info_t ip;
    if (s_sta_netif && esp_netif_get_ip_info(s_sta_netif, &ip) == ESP_OK)
        memcpy(info->ip, &ip.ip, 4);
    return DIAG_PASSED;
}

int hal_wifi_get_disconnect_reason(void)
{
    return (int)s_disconnect_reason;
}

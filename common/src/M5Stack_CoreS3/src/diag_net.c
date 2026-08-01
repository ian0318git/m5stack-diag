/*
 * diag_net.c - Application service: NTP sync and result upload
 *
 * NTP: SNTP -> libc time -> BM8563 (RTC untouched on any failure).
 * Upload: JSON report -> HTTP POST or MQTT publish.
 *
 * Copyright (c) 2026 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "diag_net.h"
#include "diag_config.h"
#include "hal_rtc.h"

#include "esp_netif_sntp.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "apps/ping/ping_sock.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <netdb.h>
#include <sys/socket.h>

#define JSON_BUF_SIZE   16384
#define JSON_HEADROOM   1024
#define MQTT_WAIT_MS    10000
#define PING_DEFAULT_COUNT 5
#define PING_INTERVAL_MS   1000
#define PING_TIMEOUT_MS    1000
#define PING_DATA_SIZE     64

/*===========================================================================*/
/* Ping                                                                      */
/*===========================================================================*/

typedef struct {
    SemaphoreHandle_t done;   /* given by ping task on session end */
    uint32_t          sent;
    uint32_t          received;
    uint32_t          min_ms;
    uint32_t          max_ms;
    uint32_t          total_ms;
} ping_ctx_t;

/* Callbacks run on the internal ping task — only update counters,
   never print (diag_menu_printf is called from the caller task). */
static void ping_success_cb(esp_ping_handle_t hdl, void *args)
{
    ping_ctx_t *ctx = (ping_ctx_t *)args;
    uint32_t gap = 0;
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &gap, sizeof(gap));
    if (ctx->received == 0 || gap < ctx->min_ms)
        ctx->min_ms = gap;
    if (gap > ctx->max_ms)
        ctx->max_ms = gap;
    ctx->total_ms += gap;
    ctx->received++;
}

static void ping_end_cb(esp_ping_handle_t hdl, void *args)
{
    (void)hdl;
    ping_ctx_t *ctx = (ping_ctx_t *)args;
    xSemaphoreGive(ctx->done);
}

diag_result_t diag_net_ping(const char *host, uint32_t count,
                            uint32_t *sent, uint32_t *received,
                            uint32_t *min_ms, uint32_t *avg_ms,
                            uint32_t *max_ms)
{
    if (sent)      *sent      = 0;
    if (received)  *received  = 0;
    if (min_ms)    *min_ms    = 0;
    if (avg_ms)    *avg_ms    = 0;
    if (max_ms)    *max_ms    = 0;

    /* Resolve target: IP literal first, then DNS hostname */
    ip_addr_t target = IPADDR4_INIT(0);
    const char *name = (host && host[0]) ? host : "1.1.1.1";
    esp_ip4_addr_t ip4;
    if (esp_netif_str_to_ip4(name, &ip4) == ESP_OK) {
        target.u_addr.ip4.addr = ip4.addr;
    } else {
        struct addrinfo hints = { .ai_family = AF_INET };
        struct addrinfo *res = NULL;
        if (getaddrinfo(name, NULL, &hints, &res) != 0 || !res) {
            return DIAG_FAILED;                 /* unresolvable host */
        }
        struct sockaddr_in *sa = (struct sockaddr_in *)res->ai_addr;
        target.u_addr.ip4.addr = sa->sin_addr.s_addr;
        freeaddrinfo(res);
    }
    target.type = IPADDR_TYPE_V4;

    if (count == 0 || count > 20)
        count = PING_DEFAULT_COUNT;

    ping_ctx_t ctx = { .done = NULL, .sent = 0, .received = 0,
                       .min_ms = UINT32_MAX, .max_ms = 0, .total_ms = 0 };
    ctx.done = xSemaphoreCreateBinary();
    if (!ctx.done)
        return DIAG_ERROR;

    esp_ping_config_t cfg = ESP_PING_DEFAULT_CONFIG();
    cfg.count       = count;
    cfg.interval_ms = PING_INTERVAL_MS;
    cfg.timeout_ms  = PING_TIMEOUT_MS;
    cfg.data_size   = PING_DATA_SIZE;
    cfg.target_addr = target;

    esp_ping_callbacks_t cbs = {
        .cb_args           = &ctx,
        .on_ping_success   = ping_success_cb,
        .on_ping_timeout   = NULL,
        .on_ping_end       = ping_end_cb,
    };

    esp_ping_handle_t ping = NULL;
    if (esp_ping_new_session(&cfg, &cbs, &ping) != ESP_OK) {
        vSemaphoreDelete(ctx.done);
        return DIAG_ERROR;
    }
    if (esp_ping_start(ping) != ESP_OK) {
        esp_ping_delete_session(ping);
        vSemaphoreDelete(ctx.done);
        return DIAG_ERROR;
    }

    /* Wait for the session end (count * interval + slack). */
    uint32_t wait_ms = count * (PING_INTERVAL_MS + PING_TIMEOUT_MS) + 2000;
    xSemaphoreTake(ctx.done, pdMS_TO_TICKS(wait_ms));
    esp_ping_delete_session(ping);
    vSemaphoreDelete(ctx.done);

    if (sent)     *sent     = count;
    if (received) *received = ctx.received;
    if (min_ms && ctx.received) *min_ms = ctx.min_ms;
    if (max_ms && ctx.received) *max_ms = ctx.max_ms;
    if (avg_ms && ctx.received) *avg_ms = ctx.total_ms / ctx.received;

    return (ctx.received > 0) ? DIAG_PASSED : DIAG_FAILED;
}

/*===========================================================================*/
/* NTP sync                                                                  */
/*===========================================================================*/

diag_result_t diag_net_ntp_sync(const char *server, uint32_t timeout_ms)
{
    if (!server || !server[0])
        return DIAG_FAILED;

    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG(server);
    esp_err_t e = esp_netif_sntp_init(&cfg);
    if (e != ESP_OK)
        return DIAG_FAILED;

    bool synced = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeout_ms)) == ESP_OK;
    if (!synced) {
        esp_netif_sntp_deinit();
        return DIAG_FAILED;                       /* RTC untouched */
    }

    time_t now = time(NULL);
    struct tm tm;
    gmtime_r(&now, &tm);          /* UTC per DFS — no TZ handling */
    esp_netif_sntp_deinit();

    /* Plausibility gate: reject pre-2024 / post-2100 (clock never init'd) */
    int year = tm.tm_year + 1900;
    if (year < 2024 || year > 2100)
        return DIAG_FAILED;

    hal_rtc_time_t t = {
        .year    = (uint16_t)year,
        .month   = (uint8_t)(tm.tm_mon + 1),
        .day     = (uint8_t)tm.tm_mday,
        .hour    = (uint8_t)tm.tm_hour,
        .minute  = (uint8_t)tm.tm_min,
        .second  = (uint8_t)tm.tm_sec,
        .weekday = (uint8_t)tm.tm_wday,   /* BM8563: 0=Sunday == tm_wday */
    };
    if (hal_rtc_init() != DIAG_PASSED)
        return DIAG_FAILED;
    diag_result_t r = hal_rtc_set_time(&t);
    hal_rtc_deinit();
    return r;
}

/*===========================================================================*/
/* JSON report builder                                                       */
/*===========================================================================*/

static void json_escape(char *out, size_t outlen, const char *in)
{
    size_t o = 0;
    for (size_t i = 0; in[i] && o + 6 < outlen; i++) {
        switch (in[i]) {
        case '"':  out[o++] = '\\'; out[o++] = '"';  break;
        case '\\': out[o++] = '\\'; out[o++] = '\\'; break;
        case '\n': out[o++] = '\\'; out[o++] = 'n';  break;
        case '\r': out[o++] = '\\'; out[o++] = 'r';  break;
        default:   out[o++] = in[i];
        }
    }
    out[o] = '\0';
}

static size_t json_build_report(char *json, size_t cap,
                                const diag_runner_t *runner,
                                const diag_err_ctx_t *err_ctx,
                                const hal_wifi_info_t *wifi)
{
    size_t used = 0;
    int n;
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    n = snprintf(json, cap,
                 "{\"app\":\"m5s3_diag\",\"idf\":\"%s\","
                 "\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\"",
                 esp_get_idf_version(),
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    used = (n > 0) ? (size_t)n : 0;

    if (wifi && wifi->ip[0]) {
        char esc_ssid[80];
        json_escape(esc_ssid, sizeof(esc_ssid), wifi->ssid);
        n = snprintf(json + used, cap - used,
                     ",\"wifi\":{\"ssid\":\"%s\",\"ip\":\"%u.%u.%u.%u\","
                     "\"rssi\":%d,\"channel\":%u}",
                     esc_ssid, wifi->ip[0], wifi->ip[1],
                     wifi->ip[2], wifi->ip[3], wifi->rssi, wifi->channel);
        used += (size_t)n;
    }

    if (hal_rtc_init() == DIAG_PASSED) {
        hal_rtc_time_t t;
        if (hal_rtc_get_time(&t) == DIAG_PASSED) {
            n = snprintf(json + used, cap - used,
                         ",\"rtc\":\"%04u-%02u-%02u %02u:%02u:%02u\"",
                         t.year, t.month, t.day, t.hour, t.minute, t.second);
            used += (size_t)n;
        }
        hal_rtc_deinit();
    }

    /* tests */
    int total = 0, passed = 0, skipped = 0, failed = 0;
    const diag_test_suite_t *suite = runner ? diag_runner_get_suite(runner)
                                            : NULL;
    n = snprintf(json + used, cap - used, ",\"tests\":[");
    used += (size_t)n;
    if (suite) {
        for (size_t i = 0; i < suite->count; i++) {
            const diag_test_t *t = &suite->tests[i];
            const diag_test_record_t *rec =
                diag_runner_get_record(runner, t->id);
            if (!rec)
                continue;
            total++;
            if (rec->result == DIAG_PASSED)        passed++;
            else if (rec->result == DIAG_SKIPPED)  skipped++;
            else                                   failed++;
            if (used > cap - JSON_HEADROOM)
                break;                             /* budget guard */
            if (total > 1)
                json[used++] = ',';
            char esc_name[2 * DIAG_TEST_NAME_MAX];
            json_escape(esc_name, sizeof(esc_name), t->name);
            n = snprintf(json + used, cap - used,
                         "{\"id\":\"%s\",\"result\":\"%s\","
                         "\"elapsed_ms\":%u,\"message\":\"",
                         esc_name, diag_result_str(rec->result),
                         (unsigned)rec->elapsed_ms);
            used += (size_t)n;
            json_escape(json + used, cap - used - 4, rec->message);
            used += strlen(json + used);
            used += (size_t)snprintf(json + used, cap - used, "\"}");
        }
    }
    n = snprintf(json + used, cap - used, "]");
    used += (size_t)n;

    /* errors */
    n = snprintf(json + used, cap - used, ",\"errors\":[");
    used += (size_t)n;
    if (err_ctx) {
        int shown = 0;
        for (int i = 0; i < err_ctx->num_records; i++) {
            const diag_err_record_t *rec = &err_ctx->records[i];
            if (used > cap - JSON_HEADROOM)
                break;
            if (shown > 0)
                json[used++] = ',';
            char esc_comp[2 * DIAG_ERR_COMP_MAX];
            char esc_loc[2 * DIAG_ERR_LOC_MAX];
            json_escape(esc_comp, sizeof(esc_comp), rec->component);
            json_escape(esc_loc, sizeof(esc_loc), rec->location);
            n = snprintf(json + used, cap - used,
                         "{\"component\":\"%s\",\"location\":\"%s\","
                         "\"message\":\"",
                         esc_comp, esc_loc);
            used += (size_t)n;
            json_escape(json + used, cap - used - 4, rec->message);
            used += strlen(json + used);
            n = snprintf(json + used, cap - used,
                         "\",\"count\":%lu,\"debug1\":\"",
                         (unsigned long)rec->count);
            used += (size_t)n;
            json_escape(json + used, cap - used - 4, rec->debug1);
            used += strlen(json + used);
            n = snprintf(json + used, cap - used, "\",\"debug2\":\"");
            used += (size_t)n;
            json_escape(json + used, cap - used - 4, rec->debug2);
            used += strlen(json + used);
            used += (size_t)snprintf(json + used, cap - used, "\"}");
            shown++;
        }
    }
    n = snprintf(json + used, cap - used,
                 "],\"summary\":{\"total\":%d,\"passed\":%d,"
                 "\"skipped\":%d,\"failed\":%d}}",
                 total, passed, skipped, failed);
    used += (size_t)n;
    if (used >= cap)
        used = cap - 1;                 /* defensive truncation */
    json[used] = '\0';
    return used;
}

/*===========================================================================*/
/* HTTP upload                                                               */
/*===========================================================================*/

diag_result_t diag_net_upload_results(const diag_runner_t *runner,
                                      const diag_err_ctx_t *err_ctx,
                                      const char *url,
                                      const hal_wifi_info_t *wifi,
                                      int *http_status)
{
    if (!url || !url[0] || !http_status)
        return DIAG_FAILED;
    *http_status = 0;

    char *json = malloc(JSON_BUF_SIZE);
    if (!json)
        return DIAG_ERROR;
    size_t used = json_build_report(json, JSON_BUF_SIZE,
                                    runner, err_ctx, wifi);

    esp_http_client_config_t cfg = {
        .url        = url,
        .method     = HTTP_METHOD_POST,
        .timeout_ms = CONFIG_UPLOAD_TIMEOUT_MS,
        .buffer_size = 1024,
    };
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {
        free(json);
        return DIAG_ERROR;
    }
    esp_http_client_set_header(c, "Content-Type", "application/json");
    esp_http_client_set_post_field(c, json, used);
    esp_err_t e = esp_http_client_perform(c);
    *http_status = esp_http_client_get_status_code(c);
    esp_http_client_cleanup(c);
    free(json);

    if (e != ESP_OK)
        return DIAG_FAILED;
    return (*http_status >= 200 && *http_status < 300)
        ? DIAG_PASSED : DIAG_FAILED;
}

/*===========================================================================*/
/* MQTT publish                                                              */
/*===========================================================================*/

static SemaphoreHandle_t s_mqtt_evt;   /* signalled by event handler */
/* Written by the MQTT task, polled by the caller task: volatile. */
static volatile bool     s_mqtt_connected;
static volatile bool     s_mqtt_published;
static volatile bool     s_mqtt_failed;

static void mqtt_evt(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    switch ((esp_mqtt_event_id_t)id) {
    case MQTT_EVENT_CONNECTED:
        s_mqtt_connected = true;
        s_mqtt_failed    = false;
        xSemaphoreGive(s_mqtt_evt);
        break;
    case MQTT_EVENT_PUBLISHED:
        s_mqtt_published = true;
        xSemaphoreGive(s_mqtt_evt);
        break;
    case MQTT_EVENT_DISCONNECTED:
    case MQTT_EVENT_ERROR:
        s_mqtt_failed = true;
        xSemaphoreGive(s_mqtt_evt);
        break;
    default:
        break;
    }
}

diag_result_t diag_net_publish_mqtt(const diag_runner_t *runner,
                                    const diag_err_ctx_t *err_ctx,
                                    const char *broker_url,
                                    const char *topic,
                                    const hal_wifi_info_t *wifi,
                                    bool *pub_ok)
{
    if (!broker_url || !broker_url[0] || !pub_ok)
        return DIAG_FAILED;
    *pub_ok = false;

    char *json = malloc(JSON_BUF_SIZE);
    if (!json)
        return DIAG_ERROR;
    size_t used = json_build_report(json, JSON_BUF_SIZE,
                                    runner, err_ctx, wifi);

    char def_topic[64];
    if (!topic || !topic[0]) {
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        snprintf(def_topic, sizeof(def_topic),
                 "m5s3_diag/%02x%02x%02x%02x%02x%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        topic = def_topic;
    }

    s_mqtt_evt = xSemaphoreCreateBinary();
    if (!s_mqtt_evt) {
        free(json);
        return DIAG_ERROR;
    }
    s_mqtt_connected = false;
    s_mqtt_published = false;
    s_mqtt_failed    = false;

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = broker_url,
        .session.disable_clean_session = false,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);
    if (!client) {
        vSemaphoreDelete(s_mqtt_evt);
        free(json);
        return DIAG_ERROR;
    }
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_evt, NULL);
    if (esp_mqtt_client_start(client) != ESP_OK) {
        esp_mqtt_client_destroy(client);
        vSemaphoreDelete(s_mqtt_evt);
        free(json);
        return DIAG_FAILED;
    }

    diag_result_t result = DIAG_FAILED;

    /* wait for CONNECTED */
    if (xSemaphoreTake(s_mqtt_evt, pdMS_TO_TICKS(MQTT_WAIT_MS)) == pdTRUE &&
        s_mqtt_connected) {
        /* QoS 1: broker PUBACK is required, msg_id > 0 guaranteed.
           (QoS 0 returns msg_id 0 on success — no ack, no success signal.) */
        int msg_id = esp_mqtt_client_publish(client, topic, json, used, 1, 0);
        if (msg_id > 0) {
            /* wait for PUBLISHED (or error) */
            TickType_t waited = 0;
            while (waited < MQTT_WAIT_MS) {
                if (s_mqtt_published) {
                    *pub_ok = true;
                    result  = DIAG_PASSED;
                    break;
                }
                if (s_mqtt_failed)
                    break;
                vTaskDelay(pdMS_TO_TICKS(100));
                waited += 100;
            }
        }
    }

    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
    vSemaphoreDelete(s_mqtt_evt);
    free(json);
    return result;
}

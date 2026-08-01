/*
 * diag_net.h - Application service: NTP sync and result upload
 *
 * INTERFACE ADAPTER layer — network services built on top of the
 * Wi-Fi HAL.  Callers must ensure Wi-Fi is connected first.
 *
 * Copyright (c) 2026 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "diag_core.h"
#include "diag_runner.h"
#include "diag_error.h"
#include "hal_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Ping                                                                       */
/*===========================================================================*/

/**
 * @brief ICMP echo test (Wi-Fi must already be connected).
 *
 * @param host      Target: IP literal ("8.8.8.8") or hostname (resolved
 *                  via DNS; NULL/empty defaults to "1.1.1.1").
 * @param count     Number of pings (1..20, 0 = default 5).
 * @param[out] sent     Packets sent.
 * @param[out] received Packets replied.
 * @param[out] min_ms / avg_ms / max_ms  RTT statistics (0 when no reply).
 * @return DIAG_PASSED when at least one reply was received.
 */
diag_result_t diag_net_ping(const char *host, uint32_t count,
                            uint32_t *sent, uint32_t *received,
                            uint32_t *min_ms, uint32_t *avg_ms,
                            uint32_t *max_ms);

/*===========================================================================*/
/* NTP                                                                        */
/*===========================================================================*/

/**
 * @brief Sync the BM8563 RTC from an NTP server (SNTP over UDP/123).
 *
 * Wi-Fi must already be connected.  On success the RTC is written with
 * the UTC time.  On failure (sync timeout, implausible time) the RTC
 * is left untouched and DIAG_FAILED is returned.
 *
 * @param server     NTP server hostname (e.g. "pool.ntp.org").
 * @param timeout_ms Maximum wait for the SNTP sync.
 * @return DIAG_PASSED when RTC was updated.
 */
diag_result_t diag_net_ntp_sync(const char *server, uint32_t timeout_ms);

/*===========================================================================*/
/* Upload                                                                     */
/*===========================================================================*/

/**
 * @brief Build a JSON report from runner records + error context and
 *        POST it to the given URL (plain http://).
 *
 * Wi-Fi must already be connected.
 *
 * @param runner      Runner holding test records.
 * @param err_ctx     Error context (may be NULL).
 * @param url         HTTP endpoint.
 * @param wifi        Joined-AP info block (may be NULL).
 * @param[out] http_status  Server response code (0 if unreachable).
 * @return DIAG_PASSED when the server replied 2xx.
 */
diag_result_t diag_net_upload_results(const diag_runner_t *runner,
                                      const diag_err_ctx_t *err_ctx,
                                      const char *url,
                                      const hal_wifi_info_t *wifi,
                                      int *http_status);

/**
 * @brief Build the JSON report and publish it to an MQTT broker.
 *
 * Wi-Fi must already be connected.  The report is published to the
 * given topic (default "m5s3_diag/<mac>" when topic is NULL/empty).
 *
 * @param runner      Runner holding test records.
 * @param err_ctx     Error context (may be NULL).
 * @param broker_url  Broker URI, e.g. "mqtt://192.168.1.10:1883".
 * @param topic       MQTT topic (NULL = default).
 * @param wifi        Joined-AP info block (may be NULL).
 * @param[out] pub_ok Non-zero when the broker acknowledged the publish.
 * @return DIAG_PASSED on publish acknowledgement.
 */
diag_result_t diag_net_publish_mqtt(const diag_runner_t *runner,
                                    const diag_err_ctx_t *err_ctx,
                                    const char *broker_url,
                                    const char *topic,
                                    const hal_wifi_info_t *wifi,
                                    bool *pub_ok);

#ifdef __cplusplus
}
#endif

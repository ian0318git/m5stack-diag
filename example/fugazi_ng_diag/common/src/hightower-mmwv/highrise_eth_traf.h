/* $Id: highrise_eth_traf.h,v 1.1 2020/08/19 09:50:04 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/highrise_eth_traf.h,v $
 *------------------------------------------------------------------
 *
 * highrise_eth_traf.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 * < A copy of switzer_traf.h >
 *
 *------------------------------------------------------------------
 */

#ifndef __HIGHRISE_ETH_TRAF__
#define __HIGHRISE_ETH_TRAF__

struct highrise_eth_traf;
struct highrise_eth_task;

struct highrise_eth_traf_tx_task_settings {
    struct highrise_eth_traf *traf;
    enum {
        HIGHRISE_ETH_TRAF_TX_MODE_FIXED,
        HIGHRISE_ETH_TRAF_TX_MODE_RADOM,
    } mode;
    enum {
        HIGHRISE_ETH_TRAF_TX_CHECK_BIT_ADD_YES,
        HIGHRISE_ETH_TRAF_TX_CHECK_BIT_ADD_NO,
    } check;
    unsigned int len;
    unsigned int burst;
    unsigned int interval;
};

struct highrise_eth_traf_rx_task_settings {
    struct highrise_eth_traf *traf;
    enum {
        HIGHRISE_ETH_TRAF_RX_MODE_CHECK_BIT,
        HIGHRISE_ETH_TRAF_RX_MODE_CHECK_LEN,
    } chk_mode;
};

struct highrise_eth_traf_statistic_info {
    uint64_t tx_cnt;
    uint64_t tx_size;
    uint64_t rx_cnt;
    uint64_t rx_size;
    uint64_t rx_errcnt;
};

struct highrise_eth_traf *highrise_eth_traf_init(const char *iface, size_t sq_len);
void highrise_eth_traf_exit(struct highrise_eth_traf *traf);

int highrise_eth_traf_tx_poll(struct highrise_eth_traf *traf, time_t timeout);
int highrise_eth_traf_tx_pkt(struct highrise_eth_traf *traf, const char *pkt, size_t len);
int highrise_eth_traf_rx_poll(struct highrise_eth_traf *traf, time_t timeout);
int highrise_eth_traf_rx_pkt(struct highrise_eth_traf *traf, char *pkt, size_t len);

struct highrise_eth_task *highrise_eth_traf_tx_task_start(struct highrise_eth_traf_tx_task_settings *settings);
struct highrise_eth_task * highrise_eth_traf_tx_task_start_using_source_mac(struct highrise_eth_traf_tx_task_settings *settings);
void highrise_eth_traf_tx_stop(struct highrise_eth_task *task);

struct highrise_eth_task *highrise_eth_traf_rx_task_start(struct highrise_eth_traf_rx_task_settings *settings);
void highrise_eth_traf_rx_stop(struct highrise_eth_task *task);

void highrise_eth_traf_statistic(struct highrise_eth_traf *traf, struct highrise_eth_traf_statistic_info *info, int verbose);

int highrise_eth_traf_util_test(const char *tx_iface, const char *rx_iface,
                    struct highrise_eth_traf_tx_task_settings *tx_settings,
                    struct highrise_eth_traf_rx_task_settings *rx_settings,
                    time_t timeout);

int highrise_eth_traf_util_test_using_source_mac(const char *tx_iface, const char *rx_iface,
                    struct highrise_eth_traf_tx_task_settings *tx_settings,
                    struct highrise_eth_traf_rx_task_settings *rx_settings,
                    time_t timeout);

int highrise_eth_traf_pktlen_gradient_increase_util_test(const char *tx_iface, const char *rx_iface,
                          unsigned int grad_class, unsigned int count, int if_using_source_mac);

#endif

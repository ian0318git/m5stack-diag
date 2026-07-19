/* $Id: eth_traf.h,v 1.1 2020/01/09 01:01:48 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/eth_traf.h,v $
 *------------------------------------------------------------------
 *
 * eth_traf.h
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 * < A copy of switzer_traf.h >
 *
 *------------------------------------------------------------------
 */

#ifndef __ETH_TRAF__
#define __ETH_TRAF__

struct eth_traf;
struct eth_task;

struct eth_traf_tx_task_settings {
    struct eth_traf *traf;
    enum {
        ETH_TRAF_TX_MODE_FIXED,
        ETH_TRAF_TX_MODE_RADOM,
    } mode;
    enum {
        ETH_TRAF_TX_CHECK_BIT_ADD_YES,
        ETH_TRAF_TX_CHECK_BIT_ADD_NO,
    } check;
    unsigned int len;
    unsigned int burst;
    unsigned int interval;
};

struct eth_traf_rx_task_settings {
    struct eth_traf *traf;
    enum {
        ETH_TRAF_RX_MODE_CHECK_BIT,
        ETH_TRAF_RX_MODE_CHECK_LEN,
    } chk_mode;
};

struct eth_traf_statistic_info {
    uint64_t tx_cnt;
    uint64_t tx_size;
    uint64_t rx_cnt;
    uint64_t rx_size;
    uint64_t rx_errcnt;
};

struct eth_traf *eth_traf_init(const char *iface, size_t sq_len);
void eth_traf_exit(struct eth_traf *traf);

int eth_traf_tx_poll(struct eth_traf *traf, time_t timeout);
int eth_traf_tx_pkt(struct eth_traf *traf, const char *pkt, size_t len);
int eth_traf_rx_poll(struct eth_traf *traf, time_t timeout);
int eth_traf_rx_pkt(struct eth_traf *traf, char *pkt, size_t len);

struct eth_task *eth_traf_tx_task_start(struct eth_traf_tx_task_settings *settings);
struct eth_task * eth_traf_tx_task_start_using_source_mac(struct eth_traf_tx_task_settings *settings);
void eth_traf_tx_stop(struct eth_task *task);

struct eth_task *eth_traf_rx_task_start(struct eth_traf_rx_task_settings *settings);
void eth_traf_rx_stop(struct eth_task *task);

void eth_traf_statistic(struct eth_traf *traf, struct eth_traf_statistic_info *info, int verbose);

int eth_traf_util_test(const char *tx_iface, const char *rx_iface,
                    struct eth_traf_tx_task_settings *tx_settings,
                    struct eth_traf_rx_task_settings *rx_settings,
                    time_t timeout);

int eth_traf_util_test_using_source_mac(const char *tx_iface, const char *rx_iface,
                    struct eth_traf_tx_task_settings *tx_settings,
                    struct eth_traf_rx_task_settings *rx_settings,
                    time_t timeout);

int eth_traf_pktlen_gradient_increase_util_test(const char *tx_iface, const char *rx_iface,
                          unsigned int grad_class, unsigned int count, int if_using_source_mac);

#endif

/*
 *-----------------------------------------------------------------------------
$Log: eth_traf.h,v $
Revision 1.1  2020/01/09 01:01:48  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

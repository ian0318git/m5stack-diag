/* $Id:
 * $Source:
 *------------------------------------------------------------------
 *
 * eth_raf.c
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 * < A copy of switzer_traf.c >
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <poll.h>

#include "common.h"
#include "eth_traf.h"

#define POLL_TIMEOUT_RX_TASK 500
#define ETH_HD_LEN 21
#define PKT_MIN_LEN 60
#define PKT_MAX_LEN 1500
#define PKT_TYPE 0x0800
#define PKT_TAIL 0xbeaf
#define MAX_PKT_STORE_COUNT 64

pthread_mutex_t rx_mutex;

struct pkt_store {
    int pkt_len;
    unsigned char pkt[PKT_MAX_LEN];
    struct pkt_store *next;
};

struct eth_traf_info {
    uint64_t count;
    uint64_t size;
    uint64_t errcnt;
    uint32_t store_count;
    struct pkt_store *pkt_store;
    uint32_t sq[0];
};

struct eth_task {
    int sock;
    char iface[64];
    int done;
    unsigned int pkt_len;
    unsigned int interval;
    unsigned int burst;
    int tx_mode;
    int tx_chk_bit;
    int rx_mode;
    int mac_sa_flag;
    pthread_t tid;
    size_t sq_len;
    uint32_t *sq;
    struct eth_traf_info *info;
    struct eth_task *next;
    struct eth_task *prev;
};


struct eth_traf {
    int sock;
    char iface[64];
    int tx_task_count;
    int rx_task_count;
    size_t sq_len;
    struct eth_traf_info *tx_info;
    struct eth_traf_info *rx_info;
    struct eth_task *tx_task;
    struct eth_task *rx_task;
};

struct packet_setting {
    uint8_t mac_da[6];
    uint8_t mac_sa[6];
    uint16_t pkt_type;
    uint8_t chksum;
    uint16_t pkt_len;
    uint32_t sq_id;
    uint8_t data[0];
}__attribute__((__packed__));

struct eth_gradient_task {
    struct eth_traf *traf;
    unsigned int grad_class;
    unsigned int count;
    int total_count;
    int mac_sa_flag;
};

static int get_mac_sa(const char *if_name, uint8_t *mac_sa)
{
    int raw;
    struct ifreq ifr;

    bzero((void *)&ifr, sizeof(ifr));
    sprintf(ifr.ifr_name, if_name);
    /* Create the raw socket */
    if ((raw = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))== -1) {
        printf("Error creating raw socket\n");
        return(-1);
    }

    /* get hdware addr */
    if((ioctl(raw, SIOCGIFHWADDR, &ifr)) == -1) {
        printf("Error getting HDADDR index !\n");
        return(-1);
    }
    memcpy(mac_sa, ifr.ifr_hwaddr.sa_data, 6);
    close(raw);

    return 0;
}

static void gen_socket_pkt (struct packet_setting *pkt, const char *iface, int pkt_len, int mac_sa_flag)
{
    int i;
    int data_len;

    uint8_t mac_sa[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t mac_da[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (mac_sa_flag) {
        get_mac_sa(iface, mac_sa);
    }
    memcpy(pkt->mac_da, mac_da, 6);
    memcpy(pkt->mac_sa, mac_sa, 6);

    pkt_len = pkt_len > PKT_MAX_LEN ? PKT_MAX_LEN : pkt_len;
    pkt_len = pkt_len < PKT_MIN_LEN ? PKT_MIN_LEN : pkt_len;
    data_len = pkt_len - ETH_HD_LEN;
    pkt->pkt_len = pkt_len;
    pkt->pkt_type = PKT_TYPE;

    for (i = 0; i < data_len - 2; i++) {
        pkt->data[i] = (uint8_t)rand() % 256;
    }
    pkt->data[i] = (uint8_t)(PKT_TAIL >> 8);
    pkt->data[i+1] = (uint8_t)(PKT_TAIL & 0xff);
}

static void display_pkt(unsigned char *b_ptr, int pktlen)
{
    int i, len;
    len = pktlen;
    printf("%s- packet: showing %d bytes\n", __FUNCTION__, len);
    for (i=0; i < len; i++) {
        if ((i>0) && ((i % 16) == 0)) {
	        printf("\n");
	    }
	    printf("%02x ", *b_ptr++);
    }
    printf("\n");
}

static void store_pkt (struct eth_traf_info *traf_info, unsigned char *pkt, int pkt_len) {
    struct pkt_store *store;
    if (traf_info->pkt_store != NULL) {
        if (traf_info->store_count > MAX_PKT_STORE_COUNT) {
            printf("warn: store pkt is too much\n");
            return;
        }
    }
    store = (struct pkt_store *)malloc(sizeof(struct pkt_store));
    if (store == NULL) {
        printf("malloc failed, store pkt err\n");
    }
    memcpy(&store->pkt[0], pkt, pkt_len);
    store->pkt_len = pkt_len;
    store->next = traf_info->pkt_store;
    traf_info->pkt_store = store;
    traf_info->store_count += 1;
}

static void display_store_pkt (struct pkt_store *pkt_store) {
    struct pkt_store *store = pkt_store;
    while (store != NULL) {
        display_pkt(&store->pkt[0], store->pkt_len);
        store = store->next;
    }
}

static void store_destroy (struct pkt_store *pkt_store) {
    struct pkt_store *store = pkt_store;
    while (store != NULL) {
        pkt_store = store->next;
        free(store);
        store = pkt_store;
    }
}

static uint8_t chk_sum(struct packet_setting *pkt)
{
    int i;
    uint8_t sum = 0;
    uint8_t *data = (void *)pkt;
    int len = pkt->pkt_len;

    for (i=0; i< len; i++) {
        sum += data[i];
    }

    sum -= pkt->chksum;
    return sum;
}

static int chk_pkt(struct packet_setting *pkt)
{
    uint8_t chksum = pkt->chksum;
    if (chksum != chk_sum(pkt)) {
        printf("pkt_data does not match\n");
        return(-1);
    }
    return 0;
}

static void cleanup_eth_dev (char *if_name, int sock)
{
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* Set the network card in promiscuous mode */
    sprintf(ifr.ifr_name, if_name);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr) == -1) {
        printf("ioctl: SIOCSIFFLAGS get interface index\n");
    }

    ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr) == -1) {
        printf("ioctl: SIOCSIFFLAGS to clear promiscous mode\n");
    }

    close(sock);
}

static int set_promisc (const char *device, int sock)
{
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* Set the network card in promiscuos mode */
    sprintf(ifr.ifr_name, device);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        printf("ioctl: SIOCSIFFLAGS get index interface\n");
        close(sock);
        return(-1);
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        printf("ioctl: SIOCSIFFLAGS to set promiscous mode\n");
        close(sock);
        return(-1);
    }

    return 0;
}


static int bind_socket (const char *device, int rawsock, int protocol)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;

    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));

    /* First Get the Interface Index  */
    sprintf((char *)ifr.ifr_name, device);
    if ((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        printf("Error getting Interface index !\n");
        return(-1);
    }

    /* Bind our raw socket to this interface */
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll))) == -1) {
        printf("Error binding raw socket to interface\n");
        return(-1);
    }

    return 0;
}

static int setup_socket (const char *eth_name, int *sock)
{
    int raw;
    /* Create the raw socket */
    if ((raw = socket(PF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(ETH_P_ALL)))== -1) {
        printf("Error creating raw socket\n");
        return -1;
    }

    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        printf("bind socket failed\n");
        close(raw);
        return -1;
    }

    if (set_promisc(eth_name, raw) == -1) {
        printf("set promisc failed\n");
        close(raw);
        return -1;
    }

    sleep(1);

    *sock = raw;
    return 0;
}

static int eth_pkt_tx (struct eth_task *tx_task)
{
    unsigned int burst = tx_task->burst;
    uint64_t sq_id = 0, sen_size = 0;
    uint64_t sen_cnt = 0, sq_cnt = 0;
    uint32_t *tx_sq = tx_task->sq;
    unsigned int interval;
    uint16_t pkt_type, pkt_len = tx_task->pkt_len;
    int i, rc, tx_skt = tx_task->sock;
    uint8_t txpkt_buf[PKT_MAX_LEN];
    struct packet_setting *tx_pkt = (void *)txpkt_buf;

    memset(txpkt_buf, 0, PKT_MAX_LEN);

    if (tx_task->tx_mode == ETH_TRAF_TX_MODE_FIXED){
        gen_socket_pkt(tx_pkt, tx_task->iface, pkt_len, tx_task->mac_sa_flag);
        pkt_type = tx_pkt->pkt_type;
        pkt_len = tx_pkt->pkt_len;
    }
    interval = tx_task->interval;

    while (!tx_task->done) {
        if (tx_task->tx_mode == ETH_TRAF_TX_MODE_RADOM){
            pkt_len = rand() % (PKT_MAX_LEN - PKT_MIN_LEN + 1) + PKT_MIN_LEN;
            gen_socket_pkt(tx_pkt, tx_task->iface, pkt_len, tx_task->mac_sa_flag);
            pkt_type = tx_pkt->pkt_type;
            pkt_len = tx_pkt->pkt_len;
        }

        tx_pkt->sq_id = sq_cnt;
        tx_pkt->pkt_len = pkt_len;
        tx_pkt->pkt_type = pkt_type;
        if (tx_task->tx_chk_bit == ETH_TRAF_TX_CHECK_BIT_ADD_YES) {
            tx_pkt->chksum = chk_sum(tx_pkt);
        }

        tx_pkt->sq_id = htonl(tx_pkt->sq_id);
        tx_pkt->pkt_type = htons(tx_pkt->pkt_type);
        tx_pkt->pkt_len = htons(tx_pkt->pkt_len);

        sq_id = sq_cnt;
        sq_cnt++;
        sq_cnt = sq_cnt % tx_task->sq_len;
        rc = write(tx_skt, tx_pkt, pkt_len);
        if (rc <= 0 ) {
            if (errno == EAGAIN || !rc)
                continue;
            else {
                printf("sock write err\n");
                return -1;
            }
        }
        sen_cnt++;
        sen_size += pkt_len;
        tx_sq[sq_id]++;
        if (burst > 0 && interval > 0) {
            if (sen_cnt % burst == 0) {
                usleep(interval);
            }
        }
    }

    tx_task->info->count += sen_cnt;
    tx_task->info->size += sen_size;
    for (i = 0; i < tx_task->sq_len; i++) {
        tx_task->info->sq[i] += tx_sq[i];
    }

    return 0;
}

static int drop_pkt(struct packet_setting *pkt, int size)
{
    if (pkt->pkt_type != PKT_TYPE)
        return -1;
    if (size < PKT_MIN_LEN || size > PKT_MAX_LEN)
        return -1;
    if (pkt->data[size - ETH_HD_LEN - 2] != PKT_TAIL >> 8)
        return -1;
    if (pkt->data[size - ETH_HD_LEN - 1] != (PKT_TAIL & 0xff))
        return -1;
    return 0;
}

static int eth_pkt_rx(struct eth_task *rx_task)
{
    int i, rc, err_flag;
    uint64_t rv_cnt = 0, err_cnt = 0, rv_size = 0;
    uint32_t *rx_sq = rx_task->sq;
    int rx_skt = rx_task->sock;
    uint8_t rxpkt_buf[PKT_MAX_LEN];
    struct packet_setting *rx_pkt = (void *)rxpkt_buf;
    int rvbuf_size = 208 * 1024;

    setsockopt(rx_skt, SOL_SOCKET, SO_RCVBUF, (const char *)&rvbuf_size, sizeof(int));
    memset(rxpkt_buf, 0, PKT_MAX_LEN);

    while (!rx_task->done) {
        err_flag = 0;
        rc = read(rx_skt, rxpkt_buf, sizeof(rxpkt_buf));
        if (rc <= 0) {
            if (errno == EAGAIN || !rc){
                struct pollfd fd = {
                    .fd = rx_skt,
                    .events = POLLIN,
                };
                if (poll(&fd, 1, POLL_TIMEOUT_RX_TASK) < 0)
                    break;
                if (!(fd.revents & POLLIN))
                    continue;
            } else {
                printf("sock read err\n");
                return -1;
            }
            continue;
        }

        rx_pkt->pkt_type = ntohs(rx_pkt->pkt_type);
        rx_pkt->pkt_len = ntohs(rx_pkt->pkt_len);
        rx_pkt->sq_id = ntohl(rx_pkt->sq_id);

        if (drop_pkt(rx_pkt, rc)) {
            pthread_mutex_lock(&rx_mutex);
            store_pkt(rx_task->info, rxpkt_buf, rc);
            pthread_mutex_unlock(&rx_mutex);
            continue;
        }
        if (rx_task->rx_mode == ETH_TRAF_RX_MODE_CHECK_BIT) {
            if (rx_pkt->pkt_len != rc) {
                err_flag = 1;
            } else if (chk_pkt(rx_pkt)) {
                err_flag = 1;
            }
        }

        if (rx_pkt->sq_id >=0 && rx_pkt->sq_id <= rx_task->sq_len)
            rx_sq[rx_pkt->sq_id]++;
        else
            err_flag = 1;

        if (rx_task->rx_mode == ETH_TRAF_RX_MODE_CHECK_LEN) {
            if (rx_pkt->pkt_len != rc) {
                err_flag = 1;
            }
        }

        if (err_flag) {
            err_cnt++;
            display_pkt(rxpkt_buf,rc);
            continue;
        }
        rv_cnt++;
        rv_size += rc;
    }

    rx_task->info->count += rv_cnt;
    rx_task->info->errcnt += err_cnt;
    rx_task->info->size += rv_size;
    for (i = 0; i < rx_task->sq_len; i++) {
        rx_task->info->sq[i] += rx_sq[i];
    }

    return 0;
}

struct eth_traf *eth_traf_init(const char *iface, size_t sq_len)
{
    int raw, len;
    struct eth_traf *traf;

    srand((int)time(NULL));

    if (sq_len <= 0) {
        printf("sq_len err\n");
        return NULL;
    }

    traf = (struct eth_traf *)malloc(sizeof(struct eth_traf));
    if (traf == NULL) {
        printf("eth_traf init err\n");
        return NULL;
    }
    traf->tx_task_count = 0;
    traf->rx_task_count = 0;
    traf->sq_len = sq_len;

    traf->tx_task = (struct eth_task *)malloc(sizeof(struct eth_task));
    if (traf->tx_task == NULL) {
        printf("eth_traf init err\n");
        goto err1;
    }

    traf->rx_task = (struct eth_task *)malloc(sizeof(struct eth_task));
    if (traf->rx_task == NULL) {
        printf("eth_ traf init err\n");
        goto err2;
    }

    traf->tx_task->next = NULL;
    traf->tx_task->prev = NULL;
    traf->rx_task->next = NULL;
    traf->rx_task->prev = NULL;

    len = sizeof(uint32_t) * sq_len + sizeof(struct eth_traf_info);
    traf->tx_info = (struct eth_traf_info *)malloc(len);
    if (traf->tx_info == NULL) {
        printf("tx_info create err\n");
        goto err3;
    }

    memset(traf->tx_info, 0, len);
    traf->tx_info->pkt_store = NULL;
    traf->tx_info->store_count = 0;

    traf->rx_info = (struct eth_traf_info *)malloc(len);
    if (traf->rx_info == NULL) {
        printf("rx_info create err\n");
        goto err4;
    }

    memset(traf->rx_info, 0, len);
    traf->rx_info->pkt_store = NULL;
    traf->rx_info->store_count = 0;

    if (setup_socket (iface, &raw) == -1){
        printf("socket setup err\n");
        goto err5;
    }
    traf->sock = raw;
    strcpy(traf->iface, iface);
    return traf;

err5:
    free(traf->rx_info);
err4:
    free(traf->tx_info);
err3:
    free(traf->rx_task);
err2:
    free(traf->tx_task);
err1:
    free(traf);
    return NULL;
}

static struct eth_task * eth_traf_tx_task(struct eth_traf_tx_task_settings *settings, int if_using_source_mac)
{
    int rc, len;
    struct eth_task *tx_task = NULL;

    tx_task = (struct eth_task *)malloc(sizeof(struct eth_task));
    if (tx_task == NULL) {
        printf("tx_task malloc err\n");
        return NULL;
    }

    tx_task->mac_sa_flag = if_using_source_mac;
    tx_task->sock = settings->traf->sock;
    tx_task->pkt_len = settings->len;
    tx_task->info = settings->traf->tx_info;
    tx_task->sq_len = settings->traf->sq_len;
    tx_task->interval = settings->interval;
    tx_task->burst = settings->burst;
    tx_task->tx_mode = settings->mode;
    tx_task->tx_chk_bit = settings->check;
    tx_task->done = 0;

    len = sizeof(uint32_t) * tx_task->sq_len;
    tx_task->sq = (uint32_t *)malloc(len);
    if (tx_task == NULL) {
        printf("tx_task sq create err\n");
        goto err1;
    }
    memset(tx_task->sq, 0, len);

    strcpy(tx_task->iface, settings->traf->iface);
    tx_task->next = NULL;
    tx_task->prev = NULL;

    rc = pthread_create(&tx_task->tid, NULL, (void *)eth_pkt_tx, tx_task);
    if (rc) {
        printf("pthread create err\n");
        goto err2;
    }

    settings->traf->tx_task_count++;
    tx_task->next = settings->traf->tx_task->next;
    tx_task->prev = settings->traf->tx_task;
    if (settings->traf->tx_task->next != NULL)
        settings->traf->tx_task->next->prev = tx_task;
    settings->traf->tx_task->next = tx_task;
    return tx_task;

err2:
    free(tx_task->sq);
err1:
    free(tx_task);
    return NULL;
}

struct eth_task * eth_traf_tx_task_start(struct eth_traf_tx_task_settings *settings)
{
    return eth_traf_tx_task(settings, 0);
}

struct eth_task * eth_traf_tx_task_start_using_source_mac(struct eth_traf_tx_task_settings *settings)
{
    return eth_traf_tx_task(settings, 1);
}

void eth_traf_tx_stop(struct eth_task *task)
{
    task->done = 1;
    if (pthread_join(task->tid, NULL)) {
        printf("tx pthread join err\n");
    }
    task->prev->next = task->next;
    if (task->next != NULL) {
        task->next->prev = task->prev;
    }
    free(task->sq);
    free(task);
}

struct eth_task *eth_traf_rx_task_start(struct eth_traf_rx_task_settings *settings)
{
    int rc, len;
    struct eth_task *rx_task = NULL;

    rx_task = (struct eth_task *)malloc(sizeof(struct eth_task));
    if (rx_task == NULL) {
        printf("rx_task malloc err\n");
        return NULL;
    }
    rx_task->sock = settings->traf->sock;
    rx_task->info = settings->traf->rx_info;
    rx_task->sq_len = settings->traf->sq_len;
    rx_task->rx_mode = settings->chk_mode;
    rx_task->done = 0;

    len = sizeof(uint32_t) * rx_task->sq_len;

    rx_task->sq = (uint32_t *)malloc(len);
    if (rx_task->sq == NULL) {
        printf("rx_task sq create err\n");
        goto err1;
    }
    memset(rx_task->sq, 0, len);
    strcpy(rx_task->iface, settings->traf->iface);
    rx_task->next = NULL;

    if (pthread_mutex_init(&rx_mutex, NULL) != 0) {
        printf("%d, %s mutex init failed\n", __LINE__, __func__);
        goto err1;
    }

    rc = pthread_create(&rx_task->tid, NULL, (void *)eth_pkt_rx, rx_task);
    if (rc) {
        printf("pthread create err\n");
        goto err2;
    }

    settings->traf->rx_task_count++;

    rx_task->next = settings->traf->rx_task->next;
    rx_task->prev = settings->traf->rx_task;
    if (settings->traf->rx_task->next != NULL)
        settings->traf->rx_task->next->prev = rx_task;
    settings->traf->rx_task->next = rx_task;

    return rx_task;

err2:
    free(rx_task->sq);
err1:
    free(rx_task);
    return NULL;
}

void eth_traf_rx_stop(struct eth_task *task)
{
    task->done = 1;
    if (pthread_join(task->tid, NULL)) {
        printf("rx pthread join err\n");
    }
    pthread_mutex_destroy(&rx_mutex);
    task->prev->next = task->next;
    if (task->next != NULL)
        task->next->prev = task->prev;
    free(task->sq);
    free(task);
}

void eth_traf_statistic(struct eth_traf *traf, struct eth_traf_statistic_info *info, int verbose)
{
    int i, if_len = 10;
    int tx_clen, tx_slen, rx_clen, rx_slen, rx_elen, total_len;
    char len[64];
    info->tx_cnt = traf->tx_info->count;
    info->tx_size = traf->tx_info->size;
    info->rx_cnt = traf->rx_info->count;
    info->rx_size = traf->rx_info->size;
    info->rx_errcnt = traf->rx_info->errcnt;

    sprintf(len, "%lu", info->tx_cnt);
    tx_clen = strlen(len) < 10 ? 10 : strlen(len) + 3;
    sprintf(len, "%lu", info->tx_size);
    tx_slen = strlen(len) < 10 ? 10 : strlen(len) + 3;
    sprintf(len, "%lu", info->rx_cnt);
    rx_clen = strlen(len) < 10 ? 10 : strlen(len) + 3;
    sprintf(len, "%lu", info->rx_size);
    rx_slen = strlen(len) < 10 ? 10 : strlen(len) + 3;
    sprintf(len, "%lu", info->rx_errcnt);
    rx_elen = strlen(len) < 10 ? 10 : strlen(len) + 3;

    total_len = if_len + tx_clen + tx_slen + rx_clen + rx_slen + rx_elen;
    if (verbose > 0) {
        char fmt[64];
        char symbol[256];
        memset(symbol, '-', sizeof(symbol));
        sprintf(fmt, "\n   %%-%ds%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds\n",
                      if_len, tx_clen, tx_slen, rx_clen, rx_slen, rx_elen);
        printf(fmt, "IF","Transmit","TxBytes","Receive","RxBytes","RxErr");
        printf("%.*s\n", total_len, symbol);
        sprintf(fmt, "\n   %%-%ds%%-%dlu%%-%dlu%%-%dlu%%-%dlu%%-%dlu\n",
                      if_len, tx_clen, tx_slen, rx_clen, rx_slen, rx_elen);
        printf(fmt, traf->iface, info->tx_cnt, info->tx_size,
                    info->rx_cnt, info->rx_size, info->rx_errcnt);
        printf("%.*s\n\n", total_len, symbol);
        if (verbose > 1) {
            for (i = 0; i < traf->sq_len; i++) {
                if (traf->tx_info->sq[i] != traf->rx_info->sq[i]) {
                    printf("%s tx_sq[%d] %d, rx_sq[%d] %d\n",traf->iface, i, traf->tx_info->sq[i], i, traf->rx_info->sq[i]);
                }
            }
            if (traf->rx_info->pkt_store != NULL) {
                printf("%s dropped pkt, total dropped: %u\n", traf->iface, traf->rx_info->store_count);
                display_store_pkt(traf->rx_info->pkt_store);
            }
            if (verbose > 2) {
                for (i = 0; i < traf->sq_len; i++) {
                    printf("%s tx_sq[%d] %d, rx_sq[%d] %d\n",traf->iface, i, traf->tx_info->sq[i], i, traf->rx_info->sq[i]);
                }
            }
        }
    }
}

void eth_traf_exit(struct eth_traf *traf)
{
    struct eth_task *tx_task = NULL;
    struct eth_task *rx_task = NULL;

    tx_task = traf->tx_task;
    rx_task = traf->rx_task;
    while (tx_task->next != NULL) {
        eth_traf_tx_stop(tx_task->next);
    }
    free(tx_task);
    while (rx_task->next != NULL) {
        eth_traf_rx_stop(rx_task->next);
    }
    free(rx_task);
    store_destroy(traf->tx_info->pkt_store);
    store_destroy(traf->rx_info->pkt_store);
    free(traf->tx_info);
    free(traf->rx_info);
    cleanup_eth_dev (traf->iface, traf->sock);

    free(traf);
}

int eth_traf_tx_poll(struct eth_traf *traf, time_t timeout)
{
    struct pollfd fd = {
        .fd = traf->sock,
        .events = POLLOUT,
    };
    if (poll(&fd, 1, timeout) < 0) {
        printf("%s poll err\n", traf->iface);
        return -1;
    }
    if (!(fd.revents & POLLOUT)) {
        printf("%s write timeout\n", traf->iface);
        return -1;
    }
    return 0;
}
int eth_traf_tx_pkt(struct eth_traf *traf, const char *pkt, size_t len)
{
    int rc;
    int sock = traf->sock;
    uint8_t txpkt_buf[PKT_MAX_LEN];

    if (len > PKT_MAX_LEN) {
        printf("%s the len of input pkt err\n", traf->iface);
        return -1;
    }

    memcpy(txpkt_buf, pkt, len);
    rc = write(sock, txpkt_buf, len);
    if (rc < 0 ) {
        printf("%s sock write err\n", traf->iface);
        return -1;
    }
    return rc;
}

int eth_traf_rx_poll(struct eth_traf *traf, time_t timeout)
{
    struct pollfd fd = {
        .fd = traf->sock,
        .events = POLLIN,
    };
    if (poll(&fd, 1, timeout) < 0) {
        printf("%s poll err\n", traf->iface);
        return -1;
    }
    if (!(fd.revents & POLLIN)) {
        printf("%s read timeout\n", traf->iface);
        return -1;
    }
    return 0;
}

int eth_traf_rx_pkt(struct eth_traf *traf, char *pkt, size_t len)
{
    int rc, sock = traf->sock;
    uint8_t rxpkt_buf[PKT_MAX_LEN];

    if (len > PKT_MAX_LEN) {
        printf("%s the len of input pkt err\n", traf->iface);
        return -1;
    }

    rc = read(sock, rxpkt_buf, sizeof(rxpkt_buf));
    if (rc < 0) {
        printf("%s sock read err\n", traf->iface);
        return -1;
    }
    memcpy(pkt, rxpkt_buf, rc);
    return rc;
}

static int eth_traf_util(const char *tx_iface, const char *rx_iface,
                    struct eth_traf_tx_task_settings *tx_settings,
                    struct eth_traf_rx_task_settings *rx_settings,
                    time_t timeout, int mac_sa_flag)
{
    int flag;
    struct eth_traf *tx_traf = NULL;
    struct eth_traf *rx_traf = NULL;
    struct eth_task *tx_task = NULL;
    struct eth_task *rx_task = NULL;
    struct eth_traf_statistic_info tx_info;
    struct eth_traf_statistic_info rx_info;

    flag = strcmp(tx_iface, rx_iface);
    if (flag) {
        tx_traf = eth_traf_init(tx_iface, 256);
        rx_traf = eth_traf_init(rx_iface, 256);
    } else {
        tx_traf = eth_traf_init(tx_iface, 256);
        rx_traf = tx_traf;
    }

    tx_settings->traf = tx_traf;
    rx_settings->traf = rx_traf;
    rx_task = eth_traf_rx_task_start(rx_settings);
    usleep(5000);
    if (mac_sa_flag) {
        tx_task = eth_traf_tx_task_start_using_source_mac(tx_settings);
    } else {
        tx_task = eth_traf_tx_task_start(tx_settings);
    }

    sleep(timeout);

    eth_traf_tx_stop(tx_task);
    usleep(5000);
    eth_traf_rx_stop(rx_task);


    if (flag) {
        eth_traf_statistic(tx_traf, &tx_info, 2);
        eth_traf_statistic(rx_traf, &rx_info, 2);
        eth_traf_exit(tx_traf);
        eth_traf_exit(rx_traf);
    } else {
        eth_traf_statistic(tx_traf, &tx_info, 2);
        rx_info = tx_info;
        eth_traf_exit(tx_traf);
    }

    if (tx_info.tx_cnt != rx_info.rx_cnt || !rx_info.rx_cnt)
        return -1;
    return 0;
}

int eth_traf_util_test(const char *tx_iface, const char *rx_iface,
                    struct eth_traf_tx_task_settings *tx_settings,
                    struct eth_traf_rx_task_settings *rx_settings,
                    time_t timeout)
{
    return eth_traf_util(tx_iface, rx_iface, tx_settings, rx_settings, timeout, 0);
}

int eth_traf_util_test_using_source_mac(const char *tx_iface, const char *rx_iface,
                    struct eth_traf_tx_task_settings *tx_settings,
                    struct eth_traf_rx_task_settings *rx_settings,
                    time_t timeout)
{
    return eth_traf_util(tx_iface, rx_iface, tx_settings, rx_settings, timeout, 1);
}

static int eth_traf_gradient_tx_task(struct eth_gradient_task *grad_task)
{
    unsigned int count = grad_task->count;
    unsigned int grad_class = grad_task->grad_class;
    int grad_len;
    struct eth_traf *traf = grad_task->traf;
    int i, rc, total_count = 0, pkt_len = PKT_MIN_LEN;
    uint8_t txpkt_buf[PKT_MAX_LEN];
    struct packet_setting *tx_pkt = (void *)txpkt_buf;

    grad_class = grad_class > 1 ? grad_class : 2;
    grad_len = (PKT_MAX_LEN - PKT_MIN_LEN) / (grad_class - 1);
    if (grad_len == 0) {
        printf("grad_class is too big\n");
        return -1;
    }

    while (pkt_len <= PKT_MAX_LEN) {
        for (i = 0; i < count; i++) {
            gen_socket_pkt(tx_pkt, grad_task->traf->iface, pkt_len, grad_task->mac_sa_flag);
            tx_pkt->sq_id = 1;
            tx_pkt->chksum = chk_sum(tx_pkt);
            tx_pkt->sq_id = htonl(tx_pkt->sq_id);
            tx_pkt->pkt_type = htons(tx_pkt->pkt_type);
            tx_pkt->pkt_len = htons(tx_pkt->pkt_len);
            if (eth_traf_tx_poll(traf, 500)) {
                continue;
            }
            rc = eth_traf_tx_pkt(traf, (const char *)tx_pkt, pkt_len);
            if (rc <= 0) {
                return -1;
            }
            total_count++;
            usleep(100);
        }
        pkt_len += grad_len;
    }
    grad_task->total_count = total_count;
    return 0;
}

static int eth_traf_gradient_rx_task(struct eth_gradient_task *grad_task)
{
    int rc, total_count = 0;
    struct eth_traf *rx_traf = grad_task->traf;
    uint8_t rxpkt_buf[PKT_MAX_LEN];
    struct packet_setting *rx_pkt = (void *)rxpkt_buf;

    while (1) {
        if (eth_traf_rx_poll(rx_traf, 1000)) {
            break;
        }
        rc = eth_traf_rx_pkt(rx_traf, (char *)rxpkt_buf, sizeof(rxpkt_buf));
        if (rc <= 0)
            return -1;
        rx_pkt->pkt_type = ntohs(rx_pkt->pkt_type);
        rx_pkt->pkt_len = ntohs(rx_pkt->pkt_len);
        rx_pkt->sq_id = ntohl(rx_pkt->sq_id);

        if (drop_pkt(rx_pkt, rc)) {
            continue;
        }
        if (rx_pkt->pkt_len != rc) {
            continue;
        }
        if (chk_pkt(rx_pkt)) {
            display_pkt(rxpkt_buf,rc);
            continue;
        }
        total_count++;
    }
    grad_task->total_count = total_count;
    return 0;
}

int eth_traf_pktlen_gradient_increase_util_test(const char *tx_iface, const char *rx_iface,
                          unsigned int grad_class, unsigned int count, int if_using_source_mac)
{
    int flag = 0;
    struct eth_gradient_task grad_tx_task;
    struct eth_gradient_task grad_rx_task;
    struct eth_traf *tx_traf = NULL;
    struct eth_traf *rx_traf = NULL;
    pthread_t tx_pid, rx_pid;

    grad_tx_task.grad_class = grad_class;
    grad_tx_task.count = count;
    grad_tx_task.total_count = 0;
    grad_tx_task.mac_sa_flag = if_using_source_mac;
    flag = strcmp(tx_iface, rx_iface);
    if (flag) {
        tx_traf = eth_traf_init(tx_iface, 256);
        rx_traf = eth_traf_init(rx_iface, 256);
    } else {
        tx_traf = eth_traf_init(tx_iface, 256);
        rx_traf = tx_traf;
    }
    grad_tx_task.traf = tx_traf;
    grad_rx_task.traf = rx_traf;

    pthread_create(&tx_pid, NULL, (void *)eth_traf_gradient_tx_task, &grad_tx_task);
    pthread_create(&rx_pid, NULL, (void *)eth_traf_gradient_rx_task, &grad_rx_task);

    pthread_join(tx_pid, NULL);
    pthread_join(rx_pid, NULL);

    if (flag) {
        eth_traf_exit(tx_traf);
        eth_traf_exit(rx_traf);
    } else {
        eth_traf_exit(tx_traf);
    }

    if (grad_tx_task.total_count != 0) {
        if (grad_tx_task.total_count == grad_rx_task.total_count) {
            return 0;
        }
    }
    return -1;
}

/*
 *-----------------------------------------------------------------------------
$Log: eth_traf.c,v $
Revision 1.4  2021/10/20 06:10:21  iachang
Migrate ISR Platform common code From RHEL7 to RHEL8

Revision 1.3  2020/05/22 01:38:07  jiajliu
CSCvn24010-62: refine the filter for unrelated packets

Revision 1.2  2020/04/20 02:59:19  jiajliu
CSCvt88319: Work around a PROMISC mode bug in BCM57412's FW 216.1.102.6

Revision 1.1  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

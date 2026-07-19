/* $Id: switzer_common.h,v 1.3 2021/04/12 13:37:34 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_common.h,v $
 *------------------------------------------------------------------
 *
 * switzer_common.h - Switzer Common interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_COMMON_H__
#define __SWITZER_COMMON_H__

#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "linux_pci.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
 * !!!Do not use 'break' like statement('goto', ...),!!!
 * !!!instead use 'P=NULL' to stop the loop.         !!!
 * p    - char *    the iter ptr
 * str  - char *    the string buffer
 * ch   - char      the dilimiter char
 * t    - char *    the tmp ptr
 */
#define forpart(P, str, ch, t)          \
for(                                    \
    ({                                  \
        (t) = NULL;                     \
        (P) = (str);                    \
        if ((P)) {                      \
            (t) = strchr((P), ch);      \
            if (t) *(t) = 0;            \
        }                               \
    });                                 \
                                        \
    (P);                                \
                                        \
    ({                                  \
        if (t) *(t) = ch;               \
        else (P) = NULL;                \
        if ((P)) {                      \
            if (t) {                    \
                (P) = t + 1;            \
                t = strchr((P), ch);    \
                if (t) *(t) = 0;        \
            }                           \
        }                               \
    })                                  \
)

//delimi by string 'sub'
#define forpart_ext(P, str, sub, t)     \
for(                                    \
    ({                                  \
        (P) = (str);                    \
        if ((P)) {                      \
            (t) = strstr((P), sub);     \
            if (t) *(t) = 0;            \
        }                               \
    });                                 \
                                        \
    (P) && (sub);                       \
                                        \
    ({                                  \
        if (t) *(t) = (sub)[0];         \
        else (P) = NULL;                \
        if ((P)) {                      \
            if (t) {                    \
                (P) = t + strlen(sub);  \
                t = strstr((P), sub);   \
                if (t) *(t) = 0;        \
            }                           \
        }                               \
    })                                  \
)

/*
 * !!! see comment of 'forpart()'
 */
#define forline(L, str, t) forpart(L, str, '\n', t)

#define __DEF_FOR_EACH_T(T)                                 \
    static inline int for_each_##T(T *out_v, int i, int n, ...) \
    {                                                           \
        va_list ap;                                             \
        int _i  = 0;                                            \
        int ret = -1;                                           \
                                                                \
        va_start(ap, n);                                        \
        for(_i = 0; _i < n; _i++) {                             \
            if (_i == i) {                                      \
                *out_v = va_arg(ap, T);                         \
                ret = 0;                                        \
                break;                                          \
            }                                                   \
            va_arg(ap, T);                                      \
        }                                                       \
        va_end(ap);                                             \
        return ret;                                             \
    }

/* TODO:How about 'long long' */
__DEF_FOR_EACH_T(int)
__DEF_FOR_EACH_T(long)
__DEF_FOR_EACH_T(float)
__DEF_FOR_EACH_T(double)
__DEF_FOR_EACH_T(char)

#define for_each(I, V, T, N, ...) \
for((I) = 0; (I) < (N) && for_each_##T(&(V), I, N, ##__VA_ARGS__) == 0; (I)++)

/* time & delay interfaces */

#define switzer_ticks   (switzer_getticks())

long switzer_getticks(void);
void switzer_udelay(unsigned long n);
void switzer_mdelay(unsigned long n);

#define PROMPT_DELAY_MS(INTV, TO) do {                  \
    static int waited = 0;                              \
    for (waited = 0; waited < (TO); waited += (INTV)) { \
        if ((waited / (INTV)) % 50 == 0) {              \
            putchar('\n');                              \
        }                                               \
        putchar('.');                                   \
        fflush(stdout);                                 \
        switzer_mdelay(INTV);                           \
    }                                                   \
    putchar('\n');                                      \
} while(0)

#define PROMPT_WAIT_COND_MS(COND, INTV, TO) do {        \
    static int waited = 0;                              \
    for (waited = 0; waited < (TO); waited += (INTV)) { \
        if (COND)                                       \
            break;                                      \
        if ((waited / (INTV)) % 50 == 0) {              \
            putchar('\n');                              \
        }                                               \
        putchar('.');                                   \
        fflush(stdout);                                 \
        switzer_mdelay(INTV);                           \
    }                                                   \
    printf("%s\n", waited < (TO) ? "OK" : "Timeout");   \
} while(0)

/* log interfaces */

enum {
    SWITZER_LOG_LEVEL_CONSOLE,  /* log to console without formating */
    SWITZER_LOG_LEVEL_FATAL,    /* fatal conditions */
    SWITZER_LOG_LEVEL_ERR,      /* error conditions */
    SWITZER_LOG_LEVEL_WARN,     /* warning conditions */
    SWITZER_LOG_LEVEL_INFO,     /* informational */
    SWITZER_LOG_LEVEL_DBG,      /* debug-level messages */
    SWITZER_LOG_LEVEL_VDBG,     /* verbose debug-level messages */

    MAX_NR_SWITZER_LOG_LEVEL
};

struct switzer_trace {
    const char *func;
    const char *file;
    unsigned line;
};

extern int switzer_log_level;
int switzer_log_vformat(struct switzer_trace *t, int lvl,
                        char *buf, size_t size, const char *fmt, va_list ap);
int switzer_log_vtrace(struct switzer_trace *t, int lvl,
                       const char *fmt, va_list ap);
int switzer_log_trace(struct switzer_trace *t,
                      int lvl, const char *fmt, ...);

#define log_trace(lvl, fmt, ...)                            \
    ({                                                      \
        static struct switzer_trace __t = {                 \
            .func = __func__,                               \
            .file = __FILE__,                               \
            .line = __LINE__,                               \
        };                                                  \
        switzer_log_trace(&__t, lvl, fmt, ## __VA_ARGS__);  \
    })

#define log_fatal(fmt, ...)                                 \
    log_trace(SWITZER_LOG_LEVEL_FATAL, fmt, ## __VA_ARGS__)
#define log_err(fmt, ...)                                   \
    log_trace(SWITZER_LOG_LEVEL_ERR, fmt, ## __VA_ARGS__)
#define log_warn(fmt, ...)                                  \
    log_trace(SWITZER_LOG_LEVEL_WARN, fmt, ## __VA_ARGS__)
#define log_info(fmt, ...)                                  \
    log_trace(SWITZER_LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)
#define log_dbg(fmt, ...)                                   \
    log_trace(SWITZER_LOG_LEVEL_DBG, fmt, ## __VA_ARGS__)
#define log_vdbg(fmt, ...)                                  \
    log_trace(SWITZER_LOG_LEVEL_VDBG, fmt, ## __VA_ARGS__)

#define prt(fmt, ...)                                           \
    log_trace(SWITZER_LOG_LEVEL_CONSOLE, fmt, ## __VA_ARGS__)
#define log(fmt, ...)   log_info(fmt, ## __VA_ARGS__)


/* helpful wrappers to (log + return) */

#define __FILE_SHORT__ ({const char *p = strrchr(__FILE__, '/'); p ? p + 1 : __FILE__;})

#define URET(VAR___, VAL___, FMT___, ...) do {   \
    if ((FMT___)[0])  prt(FMT___, ##__VA_ARGS__);\
    VAR___ = VAL___;                             \
    goto _EXIT_POINT;                            \
}while(0)

#define EURET(VAR___, VAL___, FMT___, ...) do { \
    if ((FMT___)[0]) cterr('f', 0, "%s:%s:%d:"FMT___, __FILE_SHORT__, __func__, __LINE__, ##__VA_ARGS__); \
    VAR___ = VAL___;                            \
    goto _EXIT_POINT;                           \
}while(0)

#define URET_COND(COND___, VAR___, VAL___, FMT___, ...)  \
    do {if (COND___) URET(VAR___, VAL___, FMT___, ##__VA_ARGS__);} while(0)

#define EURET_COND(COND___, VAR___, VAL___, FMT___, ...)  \
    do {if (COND___) EURET(VAR___, VAL___, FMT___, ##__VA_ARGS__);} while(0)

#define RET_COND(COND___, VAL___, FMT___, ...) \
    do {if (COND___) {if ((FMT___)[0])  prt(FMT___, ##__VA_ARGS__); return VAL___;}}while(0)

#define ERET_COND(COND___, VAL___, FMT___, ...) do { \
    if (COND___) {      \
        if ((FMT___)[0]) cterr('f', 0, "%s:%s:%d:"FMT___, __FILE_SHORT__, __func__, __LINE__, ##__VA_ARGS__);\
        return VAL___;  \
    }                   \
}while(0)

/* file mmap interfaces */

#define SWITZER_MMAP_READ    1
#define SWITZER_MMAP_WRITE   2
#define SWITZER_MMAP_EXEC    4

struct switzer_mmap {
    void *paddr;                /* physical address */
    void *vaddr;                /* virtual address */
    size_t length;
};

int switzer_file_mmap(const char *path,
                      struct switzer_mmap *map, unsigned long flags);
void switzer_file_munmap(struct switzer_mmap *map);

/* pci interfaces */

struct pci_dev *switzer_pci_dev_get(uint16_t domain, uint8_t bus,
                                    uint8_t dev, uint8_t func);

/* hex interfaces */

void switzer_hex_dump(const void *buf, size_t size, off_t offset);
int switzer_hex_to_bin(const char *hex, void *buf, size_t size);

#endif /* __SWITZER_COMMON_H__ */

/* $Id: curie2ru_common.h,v 1.1 2020/01/09 01:01:57 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_common.h,v $
 *------------------------------------------------------------------
 *
 * curie2ru_common.h - Curie2ru Common interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __CURIE2RU_COMMON_H__
#define __CURIE2RU_COMMON_H__

#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

#include "linux_pci.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* time & delay interfaces */

#define curie2ru_ticks   (curie2ru_getticks())

long curie2ru_getticks(void);
void curie2ru_udelay(unsigned long n);
void curie2ru_mdelay(unsigned long n);

/* log interfaces */

enum {
    CURIE2RU_LOG_LEVEL_CONSOLE,  /* log to console without formating */
    CURIE2RU_LOG_LEVEL_FATAL,    /* fatal conditions */
    CURIE2RU_LOG_LEVEL_ERR,      /* error conditions */
    CURIE2RU_LOG_LEVEL_WARN,     /* warning conditions */
    CURIE2RU_LOG_LEVEL_INFO,     /* informational */
    CURIE2RU_LOG_LEVEL_DBG,      /* debug-level messages */
    CURIE2RU_LOG_LEVEL_VDBG,     /* verbose debug-level messages */

    MAX_NR_CURIE2RU_LOG_LEVEL
};

struct curie2ru_trace {
    const char *func;
    const char *file;
    unsigned line;
};

extern int curie2ru_log_level;
int curie2ru_log_vformat(struct curie2ru_trace *t, int lvl,
                         char *buf, size_t size, const char *fmt, va_list ap);
int curie2ru_log_vtrace(struct curie2ru_trace *t, int lvl,
                        const char *fmt, va_list ap);
int curie2ru_log_trace(struct curie2ru_trace *t,
                       int lvl, const char *fmt, ...);

#define log_trace(lvl, fmt, ...)                            \
    ({                                                      \
        static struct curie2ru_trace __t = {                \
            .func = __func__,                               \
            .file = __FILE__,                               \
            .line = __LINE__,                               \
        };                                                  \
        curie2ru_log_trace(&__t, lvl, fmt, ## __VA_ARGS__); \
    })

#define log_fatal(fmt, ...)                                     \
    log_trace(CURIE2RU_LOG_LEVEL_FATAL, fmt, ## __VA_ARGS__)
#define log_err(fmt, ...)                                   \
    log_trace(CURIE2RU_LOG_LEVEL_ERR, fmt, ## __VA_ARGS__)
#define log_warn(fmt, ...)                                  \
    log_trace(CURIE2RU_LOG_LEVEL_WARN, fmt, ## __VA_ARGS__)
#define log_info(fmt, ...)                                  \
    log_trace(CURIE2RU_LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)
#define log_dbg(fmt, ...)                                   \
    log_trace(CURIE2RU_LOG_LEVEL_DBG, fmt, ## __VA_ARGS__)
#define log_vdbg(fmt, ...)                                  \
    log_trace(CURIE2RU_LOG_LEVEL_VDBG, fmt, ## __VA_ARGS__)

#define prt(fmt, ...)                                           \
    log_trace(CURIE2RU_LOG_LEVEL_CONSOLE, fmt, ## __VA_ARGS__)
#define log(fmt, ...)   log_info(fmt, ## __VA_ARGS__)

/* file mmap interfaces */

#define CURIE2RU_MMAP_READ    1
#define CURIE2RU_MMAP_WRITE   2
#define CURIE2RU_MMAP_EXEC    4

    struct curie2ru_mmap {
        void *paddr;                /* physical address */
        void *vaddr;                /* virtual address */
        size_t length;
    };

int curie2ru_file_mmap(const char *path,
                       struct curie2ru_mmap *map, unsigned long flags);
void curie2ru_file_munmap(struct curie2ru_mmap *map);

/* pci interfaces */

struct pci_dev *curie2ru_pci_dev_get(uint16_t domain, uint8_t bus,
                                     uint8_t dev, uint8_t func);

/* hex interfaces */

void curie2ru_hex_dump(const void *buf, size_t size, off_t offset);
int curie2ru_hex_to_bin(const char *hex, void *buf, size_t size);

#endif /* __CURIE2RU_COMMON_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_common.h,v $
Revision 1.1  2020/01/09 01:01:57  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

/* $Id: diag_common.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_common.h,v $
 *------------------------------------------------------------------
 *
 * diag_common.h - Fugazi Common interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FUGAZI_COMMON_H__
#define __FUGAZI_COMMON_H__

#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

#include "linux_pci.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* time & delay interfaces */

#define fugazi_ticks   (fugazi_getticks())

long fugazi_getticks(void);
void fugazi_udelay(unsigned long n);
void fugazi_mdelay(unsigned long n);

/* log interfaces */

enum {
    FUGAZI_LOG_LEVEL_CONSOLE,  /* log to console without formating */
    FUGAZI_LOG_LEVEL_FATAL,    /* fatal conditions */
    FUGAZI_LOG_LEVEL_ERR,      /* error conditions */
    FUGAZI_LOG_LEVEL_WARN,     /* warning conditions */
    FUGAZI_LOG_LEVEL_INFO,     /* informational */
    FUGAZI_LOG_LEVEL_DBG,      /* debug-level messages */
    FUGAZI_LOG_LEVEL_VDBG,     /* verbose debug-level messages */

    MAX_NR_FUGAZI_LOG_LEVEL
};

struct fugazi_trace {
    const char *func;
    const char *file;
    unsigned line;
};

extern int fugazi_log_level;
int fugazi_log_vformat(struct fugazi_trace *t, int lvl,
                         char *buf, size_t size, const char *fmt, va_list ap);
int fugazi_log_vtrace(struct fugazi_trace *t, int lvl,
                        const char *fmt, va_list ap);
int fugazi_log_trace(struct fugazi_trace *t,
                       int lvl, const char *fmt, ...);

#define log_trace(lvl, fmt, ...)                            \
    ({                                                      \
        static struct fugazi_trace __t = {                \
            .func = __func__,                               \
            .file = __FILE__,                               \
            .line = __LINE__,                               \
        };                                                  \
        fugazi_log_trace(&__t, lvl, fmt, ## __VA_ARGS__); \
    })

#define log_fatal(fmt, ...)                                     \
    log_trace(FUGAZI_LOG_LEVEL_FATAL, fmt, ## __VA_ARGS__)
#define log_err(fmt, ...)                                   \
    log_trace(FUGAZI_LOG_LEVEL_ERR, fmt, ## __VA_ARGS__)
#define log_warn(fmt, ...)                                  \
    log_trace(FUGAZI_LOG_LEVEL_WARN, fmt, ## __VA_ARGS__)
#define log_info(fmt, ...)                                  \
    log_trace(FUGAZI_LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)
#define log_dbg(fmt, ...)                                   \
    log_trace(FUGAZI_LOG_LEVEL_DBG, fmt, ## __VA_ARGS__)
#define log_vdbg(fmt, ...)                                  \
    log_trace(FUGAZI_LOG_LEVEL_VDBG, fmt, ## __VA_ARGS__)

#define prt(fmt, ...)                                           \
    log_trace(FUGAZI_LOG_LEVEL_CONSOLE, fmt, ## __VA_ARGS__)
#define log(fmt, ...)   log_info(fmt, ## __VA_ARGS__)

/* file mmap interfaces */

#define FUGAZI_MMAP_READ    1
#define FUGAZI_MMAP_WRITE   2
#define FUGAZI_MMAP_EXEC    4

    struct fugazi_mmap {
        void *paddr;                /* physical address */
        void *vaddr;                /* virtual address */
        size_t length;
    };

extern int fugazi_file_mmap(const char *path,struct fugazi_mmap *map, 
                            unsigned long flags);
void fugazi_file_munmap(struct fugazi_mmap *map);

/* pci interfaces */

struct pci_dev *fugazi_pci_dev_get(uint16_t domain, uint8_t bus,
                                     uint8_t dev, uint8_t func);

/* hex interfaces */

void fugazi_hex_dump(const void *buf, size_t size, off_t offset);
int fugazi_hex_to_bin(const char *hex, void *buf, size_t size);

#endif /* __FUGAZI_COMMON_H__ */


/*-------------------------------------------------
 * $Log: diag_common.h,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:25  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */


/* $Id: diag_common.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_common.c,v $
 *------------------------------------------------------------------
 *
 * diag_common.c - Fugazi common interfaces
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <ctype.h>

#include "diag_common.h"

/* time & delay interfaces */
long fugazi_getticks(void)
{
    static long era;
    long now;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    now = (long)(tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
    if (!era)
        era = now;
    return now - era;
}

void fugazi_udelay(unsigned long n)
{
    struct timespec rmtp, rqtp = {
        .tv_sec = n / 1000000,
        .tv_nsec = (n % 1000000) * 1000,
    };
    while (nanosleep(&rqtp, &rmtp) == -1 && errno == EINTR)
        rqtp = rmtp;
}

void fugazi_mdelay(unsigned long n)
{
    struct timespec rmtp, rqtp = {
        .tv_sec = n / 1000,
        .tv_nsec = (n % 1000) * 1000000,
    };
    while (nanosleep(&rqtp, &rmtp) == -1 && errno == EINTR)
        rqtp = rmtp;
}
/* log interfaces */

#define BUFFER_SIZE 4096

#define COLOR_END                   "\e[0m"
#define COLOR_BLACK                 "\e[1;30m"
#define COLOR_RED                   "\e[1;31m"
#define COLOR_RED_UNDERLINE         "\e[1;4;31m"
#define COLOR_RED_BLINK             "\e[1;5;7;31m"
#define COLOR_GREEN                 "\e[1;32m"
#define COLOR_YELLOW                "\e[1;33m"
#define COLOR_BLUE                  "\e[1;34m"
#define COLOR_PURPLE                "\e[1;35m"
#define COLOR_CYAN                  "\e[1;36m"
#define COLOR_WHITE                 "\e[1;37m"
#define COLOR_RED_BACKGROUND        "\e[1;41m"

#define COLOR(text, color)          COLOR_##color text COLOR_END

#define LOG_LEVEL_CHECK(lvl)                                \
    (lvl < 0 ? 0 : (lvl >= MAX_NR_FUGAZI_LOG_LEVEL ?      \
                    MAX_NR_FUGAZI_LOG_LEVEL - 1 : lvl))

static const char *log_prefix[] = {
    [FUGAZI_LOG_LEVEL_FATAL] = COLOR("F ", RED_BLINK),
    [FUGAZI_LOG_LEVEL_ERR]   = COLOR("E ", RED_BACKGROUND),
    [FUGAZI_LOG_LEVEL_WARN]  = COLOR("W ", RED),
    [FUGAZI_LOG_LEVEL_INFO]  = COLOR("I ", BLUE),
    [FUGAZI_LOG_LEVEL_DBG]   = COLOR("D ", GREEN),
    [FUGAZI_LOG_LEVEL_VDBG]  = COLOR("V ", BLACK),
};

int fugazi_log_level = FUGAZI_LOG_LEVEL_INFO;

int fugazi_log_vformat(struct fugazi_trace *t, int lvl,
                         char *buf, size_t size, const char *fmt, va_list ap)
{
    const char *slash, *file;
    char *p = buf;
    int len;

    p = buf;
    do {
        lvl = LOG_LEVEL_CHECK(lvl);
        /* level formating */
        if (lvl) {
            if ((len = snprintf(p, size, "%s", log_prefix[lvl])) > size)
                len = size;
            p += len;
            if (!(size -= len))
                break;

            if ((len = snprintf(p, size, "%lu ", fugazi_ticks)) > size)
                len = size;
            p += len;
            if (!(size -= len))
                break;

            slash = strrchr(t->file, '/');
            file = slash ? slash + 1 : t->file;
            if ((len = snprintf(p, size, "%s:%d:%s() ",
                                file, t->line, t->func)) > size)
                len = size;
            p += len;
            if (!(size -= len))
                break;
        }

        /* log content */
        if ((len = vsnprintf(p, size, fmt, ap)) > size)
            len = size;
        p += len;
        va_end(ap);
    } while (0);

    return (int)(p - buf);      /* total size */
}

int fugazi_log_vtrace(struct fugazi_trace *t, int lvl,
                        const char *fmt, va_list ap)
{
    char buf[BUFFER_SIZE];
    int len = 0;

    lvl = LOG_LEVEL_CHECK(lvl);
    if (lvl <= fugazi_log_level) {
        len = fugazi_log_vformat(t, lvl, buf, sizeof(buf), fmt, ap);
        len = fwrite(buf, 1, len, stdout);
    }

    return len;
}

int fugazi_log_trace(struct fugazi_trace *t,
                       int lvl, const char *fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = fugazi_log_vtrace(t, lvl, fmt, ap);
    va_end(ap);
    return len;
}
/* mmap interfaces */

#define PAGE_SIZE   0x1000      /* 4KB */
#define PAGE_MASK   ~(PAGE_SIZE - 1)

int fugazi_file_mmap(const char *path,
                     struct fugazi_mmap *map, unsigned long mflags)
{
    int fd;
    int flags;
    off_t size;
    struct stat st;
    void *paddr, *vaddr;
    size_t length;

    if (path == NULL)
        path = "/dev/mem";

    vaddr = MAP_FAILED;

    if (mflags & FUGAZI_MMAP_WRITE)
        flags = O_RDWR | O_CREAT;
    else
        flags = O_RDONLY;
    if ((fd = open(path, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
        return fd;

    if (fstat(fd, &st) < 0)
        goto err;
    if (map->length == 0)
        map->length = st.st_size - (size_t)map->paddr;
    size = (size_t)map->paddr + map->length;
    if (S_ISREG(st.st_mode) && st.st_size < size && ftruncate(fd, size) < 0)
        goto err;

    flags = PROT_READ;
    if (mflags & FUGAZI_MMAP_WRITE)
        flags |= PROT_WRITE;
    if (mflags & FUGAZI_MMAP_EXEC)
        flags |= PROT_EXEC;

    paddr = (void *)((size_t)map->paddr & PAGE_MASK);
    length = (((size_t)map->paddr + map->length +
               PAGE_SIZE - 1) & PAGE_MASK) - (size_t)paddr;
    vaddr = mmap(NULL, length, flags, MAP_SHARED, fd, (off_t)paddr);
    if (vaddr != MAP_FAILED)
        map->vaddr = vaddr - paddr + map->paddr;

err:
    close(fd);
    return vaddr == MAP_FAILED ? -1 : 0;
}

void fugazi_file_munmap(struct fugazi_mmap *map)
{
    void *vaddr;
    size_t length;

    vaddr = (void *)((size_t)map->vaddr & PAGE_MASK);
    length = (((size_t)map->vaddr + map->length +
               PAGE_SIZE - 1) & PAGE_MASK) - (size_t)vaddr;
    munmap(vaddr, length);
}

/* pci interfaces */

#define PCI_LINK_TIMEOUT    1000 /* ms */

struct fugazi_pci_dev_get_data {
    uint16_t domain;
    uint8_t bus;
    uint8_t secondary;
    uint8_t subordinate;
    struct pci_dev *pci;
};

static int fugazi_pci_dev_get_match(struct pci_dev *dev, void *_data)
{
    struct fugazi_pci_dev_get_data *data = _data;

    if (dev->domain != data->domain ||
        dev->secondary > data->bus ||
        dev->subordinate < data->bus ||
        dev->secondary < data->secondary ||
        dev->subordinate > data->subordinate)
        return 0;

    if (data->pci)
        pci_dev_put(data->pci);
    data->pci = pci_dev_get(dev);

    data->secondary = dev->secondary;
    data->subordinate = dev->subordinate;
    return 0;
}

struct pci_dev *fugazi_pci_dev_get(uint16_t domain, uint8_t bus,
                                     uint8_t dev, uint8_t func)
{
    struct fugazi_pci_dev_get_data data = {
        .domain = domain,
        .bus = bus,
        .secondary = 0,
        .subordinate = 0xff,
        .pci = NULL,
    };
    struct pci_dev *pci;

    if ((pci = pci_dev_get_by_path(domain, bus, dev, func)))
        return pci;
    pci_dev_find(&data, fugazi_pci_dev_get_match);
    if (!(pci = data.pci))
        return NULL;
    if (pci_bus_link_status(pci, PCI_LINK_TIMEOUT))
        pci_bus_rescan(pci);
    pci_dev_put(pci);

    return pci_dev_get_by_path(domain, bus, dev, func);
}

/* hex interfaces */

static int fugazi_hex_dump_width(size_t len)
{
    int width;

    for (width = 0, len--; len; width++)
        len >>= 4;
    return width;
}

static void __fugazi_hex_dump(const void *buf, size_t len, int rowsize,
                                char *linebuf, size_t linebuflen)
{
    int i;
    char c;

    linebuf[0] = '\0';
    if (linebuflen < rowsize * 3 + 1 + rowsize + 1)
        return;

    for (i = 0; i < rowsize; i++) {
        if (i < len) {
            c = *(char *)(buf + i);
            sprintf(linebuf + i * 3, "%.2x ", (unsigned char)c);
            if (!(isascii(c) && isprint(c)))
                c = '.';
            *(char *)(linebuf + rowsize * 3 + 1 + i) = c;
        } else {
            strcpy(linebuf + i * 3, "   ");
        }
    }
    *(char *)(linebuf + rowsize * 3) = ' ';
    *(char *)(linebuf + rowsize * 3 + 1 + len) = '\0';
}

void fugazi_hex_dump(const void *buf, size_t size, off_t offset)
{
    int i, linelen, remaining = size, rowsize = 16;
    char linebuf[16 * 3 + 1 + 16 + 1];
    char fmt[32];

    snprintf(fmt, sizeof(fmt), "%%.%dlx(%%.%dlx): %%s\n",
             fugazi_hex_dump_width(offset + size),
             fugazi_hex_dump_width(size));

    for (i = 0; i < size; i += rowsize) {
        linelen = rowsize;
        if (linelen > remaining)
            linelen = remaining;
        remaining -= rowsize;
        __fugazi_hex_dump(buf + i, linelen, rowsize, linebuf, sizeof(linebuf));
        prt(fmt, offset + i, i, linebuf);
    }
}

static int __fugazi_hex_to_bin(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    return -1;
}

int fugazi_hex_to_bin(const char *hex, void *buf, size_t size)
{
    int hi, lo;

    while (size--) {
        if ((hi = __fugazi_hex_to_bin(*hex++)) < 0)
            return -1;
        if ((lo = __fugazi_hex_to_bin(*hex++)) < 0)
            return -1;
        *(uint8_t *)buf++ = (hi << 4) | lo;
    }
    return 0;
}


/*-------------------------------------------------
 * $Log: diag_common.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 * */

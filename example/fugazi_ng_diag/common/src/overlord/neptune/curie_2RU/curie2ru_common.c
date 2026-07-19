/* $Id: curie2ru_common.c,v 1.1 2020/01/09 01:01:56 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_common.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru_common.c - Curie2ru common interfaces
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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

#include "curie2ru_common.h"


/* time & delay interfaces */

long curie2ru_getticks(void)
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

void curie2ru_udelay(unsigned long n)
{
    struct timespec rmtp, rqtp = {
        .tv_sec = n / 1000000,
        .tv_nsec = (n % 1000000) * 1000,
    };
    while (nanosleep(&rqtp, &rmtp) == -1 && errno == EINTR)
        rqtp = rmtp;
}

void curie2ru_mdelay(unsigned long n)
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
    (lvl < 0 ? 0 : (lvl >= MAX_NR_CURIE2RU_LOG_LEVEL ?      \
                    MAX_NR_CURIE2RU_LOG_LEVEL - 1 : lvl))

static const char *log_prefix[] = {
    [CURIE2RU_LOG_LEVEL_FATAL] = COLOR("F ", RED_BLINK),
    [CURIE2RU_LOG_LEVEL_ERR]   = COLOR("E ", RED_BACKGROUND),
    [CURIE2RU_LOG_LEVEL_WARN]  = COLOR("W ", RED),
    [CURIE2RU_LOG_LEVEL_INFO]  = COLOR("I ", BLUE),
    [CURIE2RU_LOG_LEVEL_DBG]   = COLOR("D ", GREEN),
    [CURIE2RU_LOG_LEVEL_VDBG]  = COLOR("V ", BLACK),
};

int curie2ru_log_level = CURIE2RU_LOG_LEVEL_INFO;

int curie2ru_log_vformat(struct curie2ru_trace *t, int lvl,
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

            if ((len = snprintf(p, size, "%lu ", curie2ru_ticks)) > size)
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

int curie2ru_log_vtrace(struct curie2ru_trace *t, int lvl,
                        const char *fmt, va_list ap)
{
    char buf[BUFFER_SIZE];
    int len = 0;

    lvl = LOG_LEVEL_CHECK(lvl);
    if (lvl <= curie2ru_log_level) {
        len = curie2ru_log_vformat(t, lvl, buf, sizeof(buf), fmt, ap);
        len = fwrite(buf, 1, len, stdout);
    }

    return len;
}

int curie2ru_log_trace(struct curie2ru_trace *t,
                       int lvl, const char *fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = curie2ru_log_vtrace(t, lvl, fmt, ap);
    va_end(ap);
    return len;
}

/* mmap interfaces */

#define PAGE_SIZE   0x1000      /* 4KB */
#define PAGE_MASK   ~(PAGE_SIZE - 1)

int curie2ru_file_mmap(const char *path,
                       struct curie2ru_mmap *map, unsigned long mflags)
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

    if (mflags & CURIE2RU_MMAP_WRITE)
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
    if (mflags & CURIE2RU_MMAP_WRITE)
        flags |= PROT_WRITE;
    if (mflags & CURIE2RU_MMAP_EXEC)
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

void curie2ru_file_munmap(struct curie2ru_mmap *map)
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

struct curie2ru_pci_dev_get_data {
    uint16_t domain;
    uint8_t bus;
    uint8_t secondary;
    uint8_t subordinate;
    struct pci_dev *pci;
};

static int curie2ru_pci_dev_get_match(struct pci_dev *dev, void *_data)
{
    struct curie2ru_pci_dev_get_data *data = _data;

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

struct pci_dev *curie2ru_pci_dev_get(uint16_t domain, uint8_t bus,
                                     uint8_t dev, uint8_t func)
{
    struct curie2ru_pci_dev_get_data data = {
        .domain = domain,
        .bus = bus,
        .secondary = 0,
        .subordinate = 0xff,
        .pci = NULL,
    };
    struct pci_dev *pci;

    if ((pci = pci_dev_get_by_path(domain, bus, dev, func)))
        return pci;
    pci_dev_find(&data, curie2ru_pci_dev_get_match);
    if (!(pci = data.pci))
        return NULL;
    if (pci_bus_link_status(pci, PCI_LINK_TIMEOUT))
        pci_bus_rescan(pci);
    pci_dev_put(pci);

    return pci_dev_get_by_path(domain, bus, dev, func);
}

/* hex interfaces */

static int curie2ru_hex_dump_width(size_t len)
{
    int width;

    for (width = 0, len--; len; width++)
        len >>= 4;
    return width;
}

static void __curie2ru_hex_dump(const void *buf, size_t len, int rowsize,
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

void curie2ru_hex_dump(const void *buf, size_t size, off_t offset)
{
    int i, linelen, remaining = size, rowsize = 16;
    char linebuf[16 * 3 + 1 + 16 + 1];
    char fmt[32];

    snprintf(fmt, sizeof(fmt), "%%.%dlx(%%.%dlx): %%s\n",
             curie2ru_hex_dump_width(offset + size),
             curie2ru_hex_dump_width(size));

    for (i = 0; i < size; i += rowsize) {
        linelen = rowsize;
        if (linelen > remaining)
            linelen = remaining;
        remaining -= rowsize;
        __curie2ru_hex_dump(buf + i, linelen, rowsize, linebuf, sizeof(linebuf));
        prt(fmt, offset + i, i, linebuf);
    }
}

static int __curie2ru_hex_to_bin(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    return -1;
}

int curie2ru_hex_to_bin(const char *hex, void *buf, size_t size)
{
    int hi, lo;

    while (size--) {
        if ((hi = __curie2ru_hex_to_bin(*hex++)) < 0)
            return -1;
        if ((lo = __curie2ru_hex_to_bin(*hex++)) < 0)
            return -1;
        *(uint8_t *)buf++ = (hi << 4) | lo;
    }
    return 0;
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_common.c,v $
Revision 1.1  2020/01/09 01:01:56  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

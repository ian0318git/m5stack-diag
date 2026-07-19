/* $Id: linux_pci.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/linux_pci.c,v $
 *------------------------------------------------------------------
 *
 * pci.c - linux pci implementation, derived from starfleet project
 *
 * May 2016, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2016-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * For more details of PCIE configuration, please refer to section
 * 7.5, PCI Express Base Specification Revision 2.1.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <linux/pci.h>

#include <linux_list.h>
#include <linux_pci.h>
#include "common.h"


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* log interfaces */
#define prt(fmt, ...)           printf(fmt, ## __VA_ARGS__)
#define log_prt(lvl, fmt, ...)  prt(fmt, ## __VA_ARGS__)
#define log_fatal(fmt, ...)     log_prt(LOG_LEVEL_FATAL, fmt, ## __VA_ARGS__)
#define log_err(fmt, ...)       log_prt(LOG_LEVEL_ERR, fmt, ## __VA_ARGS__)
#define log_warn(fmt, ...)      log_prt(LOG_LEVEL_WARN, fmt, ## __VA_ARGS__)
#define log_info(fmt, ...)      log_prt(LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)
#define log_dbg(fmt, ...)       log_prt(LOG_LEVEL_DBG, fmt, ## __VA_ARGS__)
#define log_vdbg(fmt, ...)      log_prt(LOG_LEVEL_VDBG, fmt, ## __VA_ARGS__)

/* atomic interfaces, simplify implemented via pthread, because of
 * missing atomic functions in C89 */
typedef struct {
    pthread_mutex_t mtx;
    int counter;
} atomic_t;

#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))

static inline int atomic_add_return(int i, atomic_t *v)
{
    int rc;

    pthread_mutex_lock(&v->mtx);
    v->counter += i;
    rc = v->counter;
    pthread_mutex_unlock(&v->mtx);
	return rc;
}

static inline int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
}

static inline void atomic_set(atomic_t *v, int i)
{
	v->counter = i;
}

static inline int atomic_read(atomic_t *v)
{
	return v->counter;
}

/* reference interfaces */
struct ref {
    atomic_t refcount;
};

static inline void ref_init(struct ref *ref)
{
    atomic_set(&ref->refcount, 1);
}

static inline void ref_get(struct ref *ref)
{
    if (atomic_inc_return(&ref->refcount) < 2) {
        log_fatal("reference error %d\n", atomic_read(&ref->refcount));
    }
}

static inline int ref_sub(struct ref *ref, unsigned int count,
                          void (*release)(struct ref *ref))
{
    int refcount = atomic_sub_return((int) count, &ref->refcount);
    if (refcount == 0) {
        release(ref);
        return 1;
    } else if (refcount < 0) {
        log_fatal("reference error %d\n", refcount);
    }
    return 0;
}

static inline int ref_put(struct ref *ref, void (*release)(struct ref *ref))
{
    return ref_sub(ref, 1, release);
}

/* os_file interfaces */
static ssize_t file_rw(const char *path, int rw,
                       void *buf, size_t count, off_t offset)
{
	int fd;
	size_t size = -1;

	if ((fd = open(path, rw ? O_RDONLY : O_WRONLY | O_CREAT,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
		return -1;
	if (offset && lseek(fd, offset, SEEK_SET) < 0)
		goto err;
	if (rw)
		size = read(fd, buf, count);
	else
		size = write(fd, buf, count);
err:
	close(fd);
	return size;
}

static ssize_t file_read(const char *path, void *buf, size_t count, off_t offset)
{
    return file_rw(path, 1, buf, count, offset);
}

static ssize_t file_write(const char *path, void *buf, size_t count, off_t offset)
{
    return file_rw(path, 0, buf, count, offset);
}

/* time & delay interfaces */
#define ticks   (getticks())

static time_t getticks(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (time_t)(tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}

static void mdelay(time_t n)
{
    struct timespec rmtp, rqtp = {
        .tv_sec = n / 1000,
        .tv_nsec = (n % 1000000) * 1000000,
    };
    while (nanosleep(&rqtp, &rmtp) == -1 && errno == EINTR)
        rqtp = rmtp;
}

/* endianess interfaces */
#if (__BYTE_ORDER == __BIG_ENDIAN)
static inline uint16_t cpu_to_le16(const uint16_t x)
{
	return ((((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |
            (((uint16_t)(x) & (uint16_t)0xff00U) >> 8));
}
static inline uint32_t cpu_to_le32(const uint32_t x)
{
    return ((((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |
            (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |
            (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |
            (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24));
}
#define le16_to_cpu(x)  cpu_to_le16(x)
#define le32_to_cpu(x)  cpu_to_le32(x)
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
#define cpu_to_le16(x)  x
#define cpu_to_le32(x)  x
#define le16_to_cpu(x)  x
#define le32_to_cpu(x)  x
#endif

/* pci interfaces */
#define PCI_LOG(fmt, ...)       prt(fmt, ##__VA_ARGS__)
#define PCI_LOG_ERR(fmt, ...)   log_err(fmt, ##__VA_ARGS__)
#define PCI_LOG_WARN(fmt, ...)  log_warn(fmt, ##__VA_ARGS__)
#define PCI_LOG_DBG(fmt, ...)   log_dbg(fmt, ##__VA_ARGS__)

#define PCI_SYS_DIR         "/sys/bus/pci/devices"
#define PCI_SYSFS_RESCAN    "/sys/bus/pci/rescan"
#define PCI_PATH_MAX        64
#define PCI_PATH_BUF        (PCI_PATH_MAX + 16)  /* room for /config etc. suffix */

#define to_private(_dev)    container_of(_dev, struct pci_private, dev)

struct pci_private {
    struct list_head list;
    struct pci_dev dev;
    struct ref ref;
    struct ref cref;          /* config fd ref */
    char path[PCI_PATH_MAX];  /* /sys/bus/pci/devices/xxxx:xx:xx.xx */
    int config;               /* fd of the pci config */
    pthread_mutex_t mtx;
    unsigned int nr_bars;
};

/* Although all the PCI info we need could be gotten from the pci
   config space, we parse this file for offloading our work */
struct pci_resource {
    char *resource[32];         /* point to resource line */
    /* content of /sys/bus/pci/devices/xxxx:xx:xx.xx/resource */
    char resource_data[4096];
};

struct pci_snapshot {
    struct pci_dev *dev;
    uint16_t command;
    uint32_t bar[MAX_NR_PCI_BARS];
    uint8_t irq_line;
    uint8_t config[PCI_CFG_SPACE_EXP_SIZE];
};

#define PCI_LOCK()      pthread_mutex_lock(&pci_lock)
#define PCI_UNLOCK()    pthread_mutex_unlock(&pci_lock)
static pthread_mutex_t pci_lock = PTHREAD_MUTEX_INITIALIZER;
static LIST_HEAD(pci_list);

static void pci_private_release(struct ref *ref)
{
    struct pci_private *priv = container_of(ref, typeof(*priv), ref);

    pthread_mutex_destroy(&priv->mtx);
    free(priv);
}
static inline void pci_private_put(struct pci_private *priv)
{
    ref_put(&priv->ref, pci_private_release);
}
static inline void pci_private_get(struct pci_private *priv)
{
    ref_get(&priv->ref);
}

static void pci_dev_release(struct ref *ref)
{
    struct pci_private *priv = container_of(ref, typeof(*priv), cref);

    close(priv->config);
    priv->config = -1;
    pci_private_put(priv);
}

void pci_dev_put(struct pci_dev *dev)
{
    if (dev)
        ref_put(&to_private(dev)->cref, pci_dev_release);
}

static int pci_dev_open(struct pci_private *priv)
{
    int fd;
    char path[PCI_PATH_BUF];

    snprintf(path, sizeof(path), "%s/config", priv->path);
    if ((fd = open(path, O_RDWR)) < 0 &&
        (fd = open(path, O_RDONLY)) < 0) {
        PCI_LOG_ERR("open %s failed!\n", path);
        return -1;
    }

    pthread_mutex_lock(&priv->mtx);
    if (priv->config >= 0)
        close(priv->config);
    priv->config = fd;
    pthread_mutex_unlock(&priv->mtx);

    return 0;
}

static int __pci_dev_get(struct pci_private *priv)
{
    pci_private_get(priv);
    return pci_dev_open(priv);
}

struct pci_dev *pci_dev_get(struct pci_dev *dev)
{
    struct pci_private *priv;

    if (!dev)
        return dev;
    priv = to_private(dev);
    if (atomic_inc_return(&priv->cref.refcount) < 2) {
        if (__pci_dev_get(priv)) {
            pci_dev_put(dev);
            return NULL;
        }
    }
    return dev;
}

struct pci_dev_find_match {
    enum {
        PCI_DEV_FIND_BY_ID,
        PCI_DEV_FIND_BY_PATH,
    } match;
    uint8_t secondary;
    uint8_t subordinate;
    uint16_t vendor;
    uint16_t device;
    int inst;
    uint16_t domain;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    int count;
};

static int pci_dev_get_match(struct pci_dev *dev, void *data)
{
    struct pci_dev_find_match *match = data;

    if (match->match == PCI_DEV_FIND_BY_PATH) {
        if (match->domain == dev->domain &&
            match->bus == dev->bus &&
            match->dev == dev->dev &&
            match->func == dev->func)
            return true;
    } else {
        if ((match->secondary || match->subordinate) &&
            (match->domain != dev->domain ||
             match->secondary > dev->bus ||
             match->subordinate < dev->bus))
            return false;
        if ((match->vendor == dev->vendor &&
             match->device == dev->device) &&
            ++match->count == match->inst)
            return true;
    }
    return false;
}

struct pci_dev *__pci_dev_get_by_id(uint16_t vendor, uint16_t device, int inst,
                                    struct pci_dev *bus)
{
    struct pci_dev_find_match match = {
        .match  = PCI_DEV_FIND_BY_ID,
        .vendor = vendor,
        .device = device,
        .inst   = inst,
        .count  = 0,
    };
    if (bus) {
        match.domain = bus->domain;
        match.secondary = bus->secondary;
        match.subordinate = bus->subordinate;
    }
    return pci_dev_find(&match, pci_dev_get_match);
}

struct pci_dev *pci_dev_get_by_id(uint16_t vendor, uint16_t device, int inst)
{
    return __pci_dev_get_by_id(vendor, device, inst, NULL);
}

struct pci_dev *pci_dev_get_by_path(uint16_t domain, uint8_t bus, uint8_t dev, uint8_t func)
{
    struct pci_dev_find_match match = {
        .match  = PCI_DEV_FIND_BY_PATH,
        .domain = domain,
        .bus    = bus,
        .dev    = dev,
        .func   = func,
    };
    return pci_dev_find(&match, pci_dev_get_match);
}

static int pci_dev_get_bus_match(struct pci_dev *dev, void *data)
{
    struct pci_dev *subdev = data;

    return dev->domain == subdev->domain && dev->secondary == subdev->bus;
}

struct pci_dev *pci_dev_get_bus(struct pci_dev *dev)
{
    return pci_dev_find(dev, pci_dev_get_bus_match);
}

struct pci_dev *pci_dev_find(void *data,
                             int (*match)(struct pci_dev *dev, void *data))
{
    struct pci_dev *dev = NULL;
    struct pci_private *priv, *next;

    PCI_LOCK();
    list_for_each_entry_safe(priv, next, &pci_list, list) {
        if (match(&priv->dev, data)) {
            dev = pci_dev_get(&priv->dev);
            break;
        }
    }
    PCI_UNLOCK();
    return dev;
}

static int enable_pci_resource = true;

static void line_break(char *data, char *line[], size_t size)
{
    int i;
    char *p = data;

    for (i = 0; i < size; i++) {
        line[i] = p;
        if (!(p = strchr(p, '\n')))
            break;
        *p++ = '\0';
    }
}

static int pci_resource_init(struct pci_resource *resource, const char *path)
{
    ssize_t size = sizeof(resource->resource_data);
    char *data = resource->resource_data;

    if ((size = file_read(path, data, size - 1, 0)) < 0) {
        PCI_LOG_ERR("pci load resource(%s) failed!\n", path);
        return -1;
    }
    data[size] = '\0';
    line_break(data, resource->resource, sizeof(resource->resource));
    return 0;
}

static void pci_resource_exit(struct pci_resource *resource)
{
    return;
}

static int pci_parse_resource(struct pci_dev *dev,
                              struct pci_resource *resource)
{
    struct pci_private *priv = to_private(dev);
    int i;
    unsigned long long start, end, flags;

    /*
     * get the size from the /sys/bus/pci/devices/xxxx:xx:xx.xx/resource
     * file, as we could not directly get the resource size from pci
     * configuration space. We also get the PCI kernel flags by the way,
     * to simplify the configuration parsing.
     */
    for (i = 0; i < priv->nr_bars; i++) {
        unsigned long address;
        unsigned long size;
        struct pci_bar *bar;

        bar = &dev->bar[i];
        /* parse from resource */
        if (sscanf((char *)resource->resource[i], "0x%016llx 0x%016llx 0x%016llx",
                   &start, &end, &flags) != 3) {
            PCI_LOG_ERR("get resource error!\n\t%s\n", resource->resource[i]);
            return -1;
        }
        /* save the address and size which parsed from hardware */
        address = bar->address;
        size = bar->size;
        /* overwrite the bar content with the value from kernel resource */
        bar->address = start;
        if (start)
            bar->size = end - start + 1;
        else
            bar->size = 0;
        bar->flags = flags;
        /* set virtual flags */
        if (address == 0 && start != 0)
            bar->virtual = 1;
#if 0
        if (!bar->virtual) {
            /* verify */
            if (address != bar->address) {
                PCI_LOG_ERR("Warning: bar[%d] incompatible address: "
                            "0x%016llx != 0x%016llx\n",
                            i, (uint64_t)bar->address, (uint64_t)address);
            }
            if (size != bar->size) {
                PCI_LOG_ERR("Warning: bar[%d] incompatible size:    "
                            "0x%016llx != 0x%016llx\n",
                            i, (uint64_t)bar->size, (uint64_t)size);
            }
        }
#else
        (void)size;
#endif
    }

    return 0;
}

static int pci_find_cap_start(struct pci_dev *dev, uint8_t hdr_type)
{
    uint16_t status;

    pci_read_config_word(dev, PCI_STATUS, &status);
    if (!(status & PCI_STATUS_CAP_LIST))
        return 0;

    switch (hdr_type) {
    case PCI_HEADER_TYPE_NORMAL:
    case PCI_HEADER_TYPE_BRIDGE:
        return PCI_CAPABILITY_LIST;
    case PCI_HEADER_TYPE_CARDBUS:
        return PCI_CB_CAPABILITY_LIST;
    }

    return 0;
}

#define PCI_FIND_CAP_TTL    48

static int pci_find_next_cap_ttl(struct pci_dev *dev,
                                 uint8_t pos, int cap, int *ttl)
{
    uint8_t id;
    uint16_t ent;

    pci_read_config_byte(dev, pos, &pos);

    while ((*ttl)--) {
        if (pos < 0x40)
            break;
        pos &= ~3;
        pci_read_config_word(dev, pos, &ent);

        id = ent & 0xff;
        if (id == 0xff)
            break;
        if (id == cap)
            return pos;
        pos = (ent >> 8);
    }
    return 0;
}

static int pci_find_next_cap(struct pci_dev *dev, uint8_t pos, int cap)
{
    int ttl = PCI_FIND_CAP_TTL;

    return pci_find_next_cap_ttl(dev, pos, cap, &ttl);
}

static int pci_find_capability(struct pci_dev *dev, int cap)
{
    int pos;

    pos = pci_find_cap_start(dev, dev->hdr_type);
    if (pos)
        pos = pci_find_next_cap(dev, pos, cap);

    return pos;
}

static int pci_find_next_ext_cap(struct pci_dev *dev, int start, int cap)
{
    uint32_t header;
    int ttl;
    int pos = PCI_CFG_SPACE_SIZE;

    /* minimum 8 bytes per capability */
    ttl = (PCI_CFG_SPACE_EXP_SIZE - PCI_CFG_SPACE_SIZE) / 8;

    if (start)
        pos = start;

    if (pci_read_config_dword(dev, pos, &header) != 0)
        return 0;

    /*
     * If we have no capabilities, this is indicated by cap ID,
     * cap version and next pointer all being 0.
     */
    if (header == 0)
        return 0;

    while (ttl-- > 0) {
        if (PCI_EXT_CAP_ID(header) == cap && pos != start)
            return pos;

        pos = PCI_EXT_CAP_NEXT(header);
        if (pos < PCI_CFG_SPACE_SIZE)
            break;

        if (pci_read_config_dword(dev, pos, &header) != 0)
            break;
    }

    return 0;
}

static int pci_find_ext_capability(struct pci_dev *dev, int cap)
{
    return pci_find_next_ext_cap(dev, 0, cap);
}

static int pci_parse_common_config(struct pci_dev *dev)
{
    uint8_t hdr_type;

    /* device id */
    if (pci_read_config_word(dev, PCI_VENDOR_ID, &dev->vendor) < 0 ||
        pci_read_config_word(dev, PCI_DEVICE_ID, &dev->device) < 0 ||
        pci_read_config_word(dev, PCI_CLASS_DEVICE, &dev->class) < 0)
        return -1;

    /* header type */
    if (pci_read_config_byte(dev, PCI_HEADER_TYPE, &hdr_type) < 0)
        return -1;
    dev->hdr_type = hdr_type & 0x7f;
    dev->multifunction = !!(hdr_type & 0x80);

    return 0;
}

static void pci_parse_pcie_config(struct pci_dev *dev)
{
    int pos;
    uint16_t reg16;

    pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
    if (!pos)
        return;
    dev->pcie_cap = pos;
    pci_read_config_word(dev, pos + PCI_EXP_FLAGS, &reg16);
    dev->pcie_flags = reg16;
    dev->pcie_type = (reg16 & PCI_EXP_FLAGS_TYPE) >> 4;

    dev->msi_cap = pci_find_capability(dev, PCI_CAP_ID_MSI);
    dev->msix_cap = pci_find_capability(dev, PCI_CAP_ID_MSIX);
}

static int pci_parse_normal_config(struct pci_dev *dev)
{
    struct pci_private *priv = to_private(dev);
    int i;

    /* base address register */
    for (i = 0; i < priv->nr_bars; i++) {
        unsigned long address;
        unsigned long size;
        unsigned long flags = 0;
        uint32_t data32, tmp;
        struct pci_bar *bar;
        unsigned int pos;

        bar = &dev->bar[i];
        pos = PCI_BASE_ADDRESS_0 + (i << 2);
        /* get bar address */
        pci_read_config_dword(dev, pos, &data32);
        address = data32;
        /* get flags */
        flags = data32 & ~(PCI_BASE_ADDRESS_MEM_MASK &
                           PCI_BASE_ADDRESS_IO_MASK);

        if (!enable_pci_resource) {
            /* get bar size */
            tmp = data32;
            pci_write_config_dword(dev, pos, (uint32_t)~0);
            pci_read_config_dword(dev, pos, &data32);
            pci_write_config_dword(dev, pos, tmp);
            size = data32;

            /**
             * All bits set in size means the device isn't working properly.
             * If the BAR isn't implemented, all bits must be 0.
             */
            if (size == 0 || size == (uint32_t)~0)
                continue;
        } else {
            /**
             * Do not touch the bar if enable_pci_resource is true,
             * the bar size will be retrieved from pci resource
             */
            size = 0;
        }

        if ((flags & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
            address &= PCI_BASE_ADDRESS_IO_MASK;
            size &= PCI_BASE_ADDRESS_IO_MASK;
            flags &= ~PCI_BASE_ADDRESS_IO_MASK;
        } else {
            /* For 64-bit */
            if ((flags & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                PCI_BASE_ADDRESS_MEM_TYPE_64) {
                i++;
                pos = PCI_BASE_ADDRESS_0 + (i << 2);

                pci_read_config_dword(dev, pos, &data32);
                address |= ((uint64_t)data32 << 32);

                if (!enable_pci_resource) {
                    tmp = data32;
                    pci_write_config_dword(dev, pos, (uint32_t)~0);
                    pci_read_config_dword(dev, pos, &data32);
                    pci_write_config_dword(dev, pos, tmp);
                    size |= ((uint64_t)data32 << 32);
                }
            }
            address &= PCI_BASE_ADDRESS_MEM_MASK;
            size &= PCI_BASE_ADDRESS_MEM_MASK;
            flags &= ~PCI_BASE_ADDRESS_MEM_MASK;
        }
        /* Get the lowest bit */
        size &= ~(size - 1);

        bar->address = address;
        bar->size = size;
        bar->flags = flags;
    }

    pci_parse_pcie_config(dev);

    return 0;
}

static int pci_parse_bridge_config(struct pci_dev *dev)
{
    if (pci_parse_normal_config(dev))
        return -1;

    pci_read_config_byte(dev, PCI_PRIMARY_BUS, &dev->primary);
    pci_read_config_byte(dev, PCI_SECONDARY_BUS, &dev->secondary);
    pci_read_config_byte(dev, PCI_SUBORDINATE_BUS, &dev->subordinate);

    /*
     * TODO:
     * pci_read_bridge_io(dev);
     * pci_read_bridge_mmio(dev);
     * pci_read_bridge_mmio_pref(dev);
     */

    return 0;
}

static int pci_parse_cardbus_config(struct pci_dev *dev)
{
    /* TODO: need to implement */
    return 0;
}

static int pci_private_init(struct pci_private *priv)
{
    const char *path;
    uint16_t domain;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    struct pci_dev *pci = &priv->dev;

    /* fill the domain, bus, dev and func fields */
    path = strrchr(priv->path, '/') + 1;
    if (sscanf(path, "%hx:%hhx:%hhx.%hhd",
               &domain, &bus, &dev, &func) != 4) {
        PCI_LOG_ERR("invalid PCI device %s!\n", path);
        return -1;
    }
    pci->domain = domain;
    pci->bus = bus;
    pci->dev = dev;
    pci->func = func;

    if (pci_parse_common_config(pci) < 0) {
        PCI_LOG_ERR("pci parse common config(%s) failed!\n", path);
        return -1;
    }

    switch (pci->hdr_type) {
    case PCI_HEADER_TYPE_NORMAL:
        priv->nr_bars = 6;
        if (pci_parse_normal_config(pci) < 0) {
            PCI_LOG_ERR("pci parse normal config(%s) failed!\n", path);
            return -1;
        }
        break;
    case PCI_HEADER_TYPE_BRIDGE:
        priv->nr_bars = 2;
        if (pci_parse_bridge_config(pci) < 0) {
            PCI_LOG_ERR("pci parse bridge config(%s) failed!\n", path);
            return -1;
        }
        break;
    case PCI_HEADER_TYPE_CARDBUS:
        priv->nr_bars = 1;
        if (pci_parse_cardbus_config(pci) < 0) {
            PCI_LOG_ERR("pci parse cardbus config(%s) failed!\n", path);
            return -1;
        }
        break;
    default:
        PCI_LOG_ERR("PCI Unkown header type %d!\n", pci->hdr_type);
        return -1;
    }

    return 0;
}

static void pci_private_exit(struct pci_private *priv)
{
    return;
}

static int pci_private_check(struct pci_private *priv)
{
    struct pci_resource resource;
    char path[PCI_PATH_BUF];
    int rc;

    if (!enable_pci_resource)
        return 0;

    memset(&resource, 0, sizeof(resource));
    snprintf(path, sizeof(path), "%s/resource", priv->path);
    if (!(rc = pci_resource_init(&resource, path))) {
        rc = pci_parse_resource(&priv->dev, &resource);
        pci_resource_exit(&resource);
    }
    return rc;
}

static int __pci_dev_remove(const char *path);

static struct pci_private *pci_private_create(const char *path)
{
    struct pci_dev *dev;
    struct pci_private *priv;

    if (!(priv = calloc(1, sizeof(*priv))))
        return NULL;

    priv->config = -1;
    snprintf(priv->path, sizeof(priv->path), "%s/%s", PCI_SYS_DIR, path);
    ref_init(&priv->ref);
    atomic_set(&priv->cref.refcount, 0);

    if (pthread_mutex_init(&priv->mtx, NULL))
        goto fail;

    if (!(dev = pci_dev_get(&priv->dev)))
        goto fail1;

    if (pci_private_init(priv) < 0)
        goto fail1;

    if (pci_private_check(priv) < 0)
        goto fail2;

    pci_dev_put(dev);
    return priv;
fail2:
    pci_private_exit(priv);
fail1:
    pci_dev_put(dev);
    __pci_dev_remove(priv->path);
fail:
    free(priv);
    return NULL;
}

void pci_dev_show(struct pci_dev *dev)
{
    struct pci_private *priv = to_private(dev);
    int i;
    char *hdr_type_str = "";

    if (!dev)
        return;

    switch (dev->hdr_type) {
    case PCI_HEADER_TYPE_NORMAL:
        hdr_type_str = "Normal";
        break;
    case PCI_HEADER_TYPE_BRIDGE:
        hdr_type_str = "Bridge";
        break;
    case PCI_HEADER_TYPE_CARDBUS:
        hdr_type_str = "Cardbus";
        break;
    default:
        PCI_LOG_ERR("PCI Unkown header type %d!\n", dev->hdr_type);
        break;
    }
    PCI_LOG("PCI %s %04x:%02x:%02x.%01x\n", hdr_type_str,
            dev->domain, dev->bus, dev->dev, dev->func);

    PCI_LOG("vendor:                0x%04x\n", dev->vendor);
    PCI_LOG("device:                0x%04x\n", dev->device);
    PCI_LOG("class:                 0x%04x\n", dev->class);
    PCI_LOG("multifunction:         %d\n", dev->multifunction);

    for (i = 0; i < priv->nr_bars; i++) {
        struct pci_bar *bar = &dev->bar[i];
        unsigned long flags = bar->flags;
        char bar_dev[128] = "";

        if (bar->size == 0)
            continue;
        if (flags & LINUX_IORESOURCE_IO)
            strcat(bar_dev, " I/O");
        if (flags & LINUX_IORESOURCE_MEM)
            strcat(bar_dev, " MEM");
        if (flags & PCI_BASE_ADDRESS_MEM_TYPE_64)
            strcat(bar_dev, "64");
        if (flags & LINUX_IORESOURCE_IRQ)
            strcat(bar_dev, " IRQ");
        if (flags & LINUX_IORESOURCE_DMA)
            strcat(bar_dev, " DMA");
        if (flags & LINUX_IORESOURCE_BUS)
            strcat(bar_dev, " BUS");
        if (bar->virtual)
            strcat(bar_dev, " virtual");

        PCI_LOG("bar[%d]:\n"
                "\taddress: 0x%08lx\n"
                "\tsize:    0x%08lx\n"
                "\tflags:   0x%08lx [%s ]\n",
                i,
                bar->address,
                bar->size,
                bar->flags, bar_dev);
    }

    PCI_LOG("\n");
}

static int __pci_dev_enable(struct pci_dev *dev, bool enable)
{
    uint16_t old_cmd, cmd;

    pci_read_config_word(dev, PCI_COMMAND, &old_cmd);
    if (enable)
        cmd = old_cmd | (PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
    else
        cmd = old_cmd & ~(PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
    if (cmd != old_cmd)
        pci_write_config_word(dev, PCI_COMMAND, cmd);
    return 0;
}

int pci_dev_enable(struct pci_dev *dev, bool enable)
{
    struct pci_private *priv = to_private(dev);
    char _enable[32];
    char fsysfs[PCI_PATH_BUF];

    sprintf(fsysfs, "%s/enable", priv->path);
    if (file_read(fsysfs, _enable, sizeof(_enable), 0) < 0)
        return -1;
    if ((enable && (_enable[0] != '0')) || (!enable && (_enable[0] == '0')))
        return 0;
    sprintf(_enable, "%d", enable);
    if (file_write(fsysfs, _enable, strlen(_enable) + 1, 0) < 0)
        return __pci_dev_enable(dev, enable);
    return 0;
}

int pci_dev_enable_master(struct pci_dev *dev, bool enable)
{
    uint16_t old_cmd, cmd;

    pci_read_config_word(dev, PCI_COMMAND, &old_cmd);
    if (enable)
        cmd = old_cmd | PCI_COMMAND_MASTER;
    else
        cmd = old_cmd & ~PCI_COMMAND_MASTER;
    if (cmd != old_cmd)
        pci_write_config_word(dev, PCI_COMMAND, cmd);
    return 0;
}

int pci_dev_enable_msi(struct pci_dev *dev, bool enable)
{
    uint16_t old_flags, flags;

    if (!dev->msi_cap)
        return -1;
    pci_read_config_word(dev, dev->msi_cap + PCI_MSI_FLAGS, &old_flags);
    if (enable)
        flags = old_flags | PCI_MSI_FLAGS_ENABLE;
    else
        flags = old_flags & ~PCI_MSI_FLAGS_ENABLE;
    if (flags != old_flags)
        pci_write_config_word(dev, dev->msi_cap + PCI_MSI_FLAGS, flags);
    return 0;
}

int pci_dev_enable_msix(struct pci_dev *dev, bool enable)
{
    uint16_t old_flags, flags;

    if (!dev->msix_cap)
        return -1;
    pci_read_config_word(dev, dev->msix_cap + PCI_MSIX_FLAGS, &old_flags);
    if (enable)
        flags = old_flags | PCI_MSIX_FLAGS_ENABLE;
    else
        flags = old_flags & ~PCI_MSIX_FLAGS_ENABLE;
    if (flags != old_flags)
        pci_write_config_word(dev, dev->msix_cap + PCI_MSIX_FLAGS, flags);
    return 0;
}

int pci_dev_enable_aer(struct pci_dev *dev, bool enable)
{
#define PCI_EXP_AER_FLAGS (PCI_EXP_DEVCTL_CERE | PCI_EXP_DEVCTL_NFERE | \
                           PCI_EXP_DEVCTL_FERE | PCI_EXP_DEVCTL_URRE)
    uint16_t old_ctrl, ctrl;

    if (!pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR))
        return -1;

    pci_read_config_word(dev, dev->pcie_cap + PCI_EXP_DEVCTL, &old_ctrl);
    if (enable)
        ctrl = old_ctrl | PCI_EXP_AER_FLAGS;
    else
        ctrl = old_ctrl & ~PCI_EXP_AER_FLAGS;
    if (ctrl != old_ctrl)
        pci_write_config_word(dev, dev->pcie_cap + PCI_EXP_DEVCTL, ctrl);
    return 0;
}

static int __pci_dev_remove(const char *path)
{
    char fsysfs[PCI_PATH_BUF];

    sprintf(fsysfs, "%s/remove", path);
    if (file_write(fsysfs, "1", 2, 0) < 0)
        return -1;
    return 0;
}

static void pci_private_remove(struct pci_private *priv);

int pci_dev_remove(struct pci_dev *dev)
{
    int rc;
    struct pci_private *priv = to_private(dev);

    rc = __pci_dev_remove(priv->path);
    pci_private_remove(priv);
    return rc;
}

static bool __pci_dev_link_status(struct pci_dev *dev)
{
    uint32_t id;                     /* vendor id and device id */

    if (pci_read_config_dword(dev, PCI_VENDOR_ID, &id))
        return false;
    return !(id == 0 || id == 0xffffffff);
}

static bool __pci_bus_link_status(struct pci_dev *dev)
{
    uint16_t linksta;

    if (!dev->pcie_cap)
        return false;

    if (pci_read_config_word(dev, dev->pcie_cap + PCI_EXP_LNKSTA, &linksta))
        return false;
    return !!(linksta & PCI_EXP_LNKSTA_DLLLA);
}

#define PCI_LINK_TIME           100 /* 100 milliseconds */
#define PCI_LINK_COUNT          3

bool pci_dev_link_status(struct pci_dev *dev, time_t timeout)
{
    bool status;
    time_t expires = ticks + timeout;
    unsigned int count = 0;

    while (1) {
        if ((status = __pci_dev_link_status(dev)))
            count++;
        else
            count = 0;
        if (count > PCI_LINK_COUNT)
            break;
        if (expires - ticks < 0)
            break;
        mdelay(PCI_LINK_TIME);
    }
    return status;
}

bool pci_bus_link_status(struct pci_dev *dev, time_t timeout)
{
    bool status;
    time_t expires = ticks + timeout;
    unsigned int count = 0;

    while (1) {
        if ((status = __pci_bus_link_status(dev)))
            count++;
        else
            count = 0;
        if (count > PCI_LINK_COUNT)
            break;
        if (expires - ticks < 0)
            break;
        mdelay(PCI_LINK_TIME);
    }
    return status;
}

static void __pci_private_remove(struct pci_private *priv)
{
    list_del(&priv->list);
    pci_private_put(priv);
}

static void pci_private_remove(struct pci_private *priv)
{
    PCI_LOCK();
    __pci_private_remove(priv);
    PCI_UNLOCK();
}

static void pci_clear(void)
{
    struct pci_private *priv, *next;

    PCI_LOCK();
    list_for_each_entry_safe(priv, next, &pci_list, list) {
        __pci_private_remove(priv);
    }
    PCI_UNLOCK();
}

static void pci_clear_invalid(void)
{
    struct pci_private *priv, *next;

    PCI_LOCK();
    list_for_each_entry_safe(priv, next, &pci_list, list) {
        if (!access(priv->path, F_OK))
            continue;
        __pci_private_remove(priv);
    }
    PCI_UNLOCK();
}

static void pci_private_insert(struct pci_private *priv)
{
    struct pci_private *next, *n;

    PCI_LOCK();
    list_for_each_entry_safe(next, n, &pci_list, list) {
        int i, diff, diffs[] = {
            next->dev.domain - priv->dev.domain,
            next->dev.bus - priv->dev.bus,
            next->dev.dev - priv->dev.dev,
            next->dev.func - priv->dev.func,
        };
        for (i = 0; i < ARRAY_SIZE(diffs); i++) {
            if ((diff = diffs[i]))
                break;
        }
        if (diff < 0)
            continue;
        if (diff == 0) {
            if (memcmp(&next->dev, &priv->dev, sizeof(struct pci_dev))) {
                __pci_private_remove(next);
                next = n;
            } else {
                pci_private_put(priv);
                priv = NULL;
            }
        }
        break;
    }
    if (priv)
        list_add_tail(&priv->list, &next->list);
    PCI_UNLOCK();
}

int __pci_rescan(void)
{
    DIR *dir;
    struct dirent *dp;
    const char *path;
    struct pci_private *priv;

    if ((dir = opendir(PCI_SYS_DIR)) == NULL) {
        PCI_LOG_ERR("open system directory: %s failed!\n", PCI_SYS_DIR);
        return -1;
    }

    pci_clear_invalid();
    while ((dp = readdir(dir)) != NULL) {
        path = dp->d_name;
        if (path[0] == '.')
            continue;
        if (!(priv = pci_private_create(path))) {
            PCI_LOG_WARN("scan pci device %s failed\n", path);
            continue;
        }
        pci_private_insert(priv);
    }

    closedir(dir);
    return 0;
}

int pci_bus_rescan(struct pci_dev *dev)
{
    struct pci_private *priv = to_private(dev);
    char fsysfs[PCI_PATH_BUF];

    sprintf(fsysfs, "%s/rescan", priv->path);
    if (file_write(fsysfs, "1", 2, 0) < 0)
        return -1;
    return __pci_rescan();
}

int pci_rescan(void)
{
    if (file_write(PCI_SYSFS_RESCAN, "1", 2, 0) < 0)
        return -1;
    return __pci_rescan();
}

static int pci_read_config(struct pci_private *priv,
                           off_t offset, void *buf, size_t size)
{
    if (pread(priv->config, buf, size, offset) == size)
        return 0;
    if (pci_dev_open(priv))
        return -1;
    if (pread(priv->config, buf, size, offset) != size)
        return -1;
    return 0;
}

static int pci_write_config(struct pci_private *priv,
                            off_t offset, const void *buf, size_t size)
{
    if (pwrite(priv->config, buf, size, offset) == size)
        return 0;
    if (pci_dev_open(priv))
        return -1;
    if (pwrite(priv->config, buf, size, offset) != size)
        return -1;
    return 0;
}

int pci_read_config_byte(struct pci_dev *dev, int where, uint8_t *val)
{
    struct pci_private *priv = to_private(dev);
    uint8_t data;

    if (pci_read_config(priv, where, &data, sizeof(data))) {
        PCI_LOG_ERR("pci read config byte error!\n");
        return -1;
    }
    *val = (typeof(*val))data;
    return 0;
}

int pci_read_config_word(struct pci_dev *dev, int where, uint16_t *val)
{
    struct pci_private *priv = to_private(dev);
    uint16_t data;

    if (pci_read_config(priv, where, &data, sizeof(data))) {
        PCI_LOG_ERR("pci read config word error!\n");
        return -1;
    }
    *val = le16_to_cpu(data);
    return 0;
}

int pci_read_config_dword(struct pci_dev *dev, int where, uint32_t *val)
{
    struct pci_private *priv = to_private(dev);
    uint32_t data;

    if (pci_read_config(priv, where, &data, sizeof(data))) {
        PCI_LOG_ERR("pci read config dword error!\n");
        return -1;
    }
    *val = le32_to_cpu(data);
    return 0;
}

int pci_write_config_byte(struct pci_dev *dev, int where, uint8_t val)
{
    struct pci_private *priv = to_private(dev);
    uint8_t data;

    data = val;
    if (pci_write_config(priv, where, &data, sizeof(data))) {
        PCI_LOG_ERR("pci write config byte error!\n");
        return -1;
    }
    return 0;
}

int pci_write_config_word(struct pci_dev *dev, int where, uint16_t val)
{
    struct pci_private *priv = to_private(dev);
    uint16_t data;

    data = cpu_to_le16(val);
    if (pci_write_config(priv, where, &data, sizeof(data))) {
        PCI_LOG_ERR("pci write config word error!\n");
        return -1;
    }
    return 0;
}

int pci_write_config_dword(struct pci_dev *dev, int where, uint32_t val)
{
    struct pci_private *priv = to_private(dev);
    uint32_t data;

    data = cpu_to_le32(val);
    if (pci_write_config(priv, where, &data, sizeof(data))) {
        PCI_LOG_ERR("pci write config dword error!\n");
        return -1;
    }
    return 0;
}

static int do_snapshot(struct pci_snapshot *snapshot)
{
    struct pci_dev *dev = snapshot->dev;
    struct pci_private *priv = to_private(dev);
    int i;
    uint32_t *config;

    /* command */
    pci_read_config_word(dev, PCI_COMMAND, &snapshot->command);

    /* PCI BARs */
    for (i = 0; i < priv->nr_bars; i++)
        pci_read_config_dword(dev, PCI_BASE_ADDRESS_0 + 4 * i, &snapshot->bar[i]);

    /* IRQ */
    pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &snapshot->irq_line);

    /* config */
    for (i = 0; i < PCI_CFG_SPACE_EXP_SIZE; i += sizeof(uint32_t)) {
        config = (uint32_t *)&snapshot->config[i];
        if (pread(priv->config, config, sizeof(uint32_t), i) != sizeof(uint32_t))
            break;
    }

    return 0;
}

struct pci_snapshot *pci_snapshot_create(struct pci_dev *dev)
{
    struct pci_snapshot *snapshot;

    if ((snapshot = malloc(sizeof(*snapshot))) == NULL)
        return NULL;

    snapshot->dev = dev;
    if (do_snapshot(snapshot) < 0)
        goto fail;
    pci_dev_get(dev);
    return snapshot;
fail:
    free(snapshot);
    return NULL;
}

int pci_snapshot_capture(struct pci_snapshot *snapshot)
{
    return do_snapshot(snapshot);
}

int pci_snapshot_restore(struct pci_snapshot *snapshot)
{
    struct pci_dev *dev = snapshot->dev;
    struct pci_private *priv = to_private(dev);
    int i;
    uint32_t data, *config;

    /* command */
    pci_write_config_word(dev, PCI_COMMAND, snapshot->command);

    /* PCI BARs */
    for (i = 0; i < priv->nr_bars; i++) {
        if (snapshot->bar[i] == 0xffffffff ||
            snapshot->bar[i] == 0x0)
            continue;
        pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + 4 * i, snapshot->bar[i]);
    }

    /* IRQ */
    pci_write_config_byte(dev, PCI_INTERRUPT_LINE, snapshot->irq_line);

    /* config */
    for (i = 0; i < PCI_CFG_SPACE_EXP_SIZE; i += sizeof(uint32_t)) {
        config = (uint32_t *)&snapshot->config[i];
        if (pread(priv->config, &data, sizeof(uint32_t), i) != sizeof(uint32_t))
            break;
        if (data == *config)
            continue;
        pwrite(priv->config, config, sizeof(uint32_t), i);
    }

    return 0;
}

void pci_snapshot_destroy(struct pci_snapshot *snapshot)
{
    pci_dev_put(snapshot->dev);
    free(snapshot);
}

struct pci_dev *pci_snapshot_dev(struct pci_snapshot *snapshot)
{
    return snapshot->dev;
}

static int __attribute__((constructor)) pci_initcall(void)
{
    if (getuid() == 0 && __pci_rescan() < 0) /* root */
        return -1;
    return 0;
}

static void __attribute__((destructor)) pci_exitcall(void)
{
    pci_clear();
}


/*-------------------------------------------------
 * $Log: linux_pci.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.3  2020/10/22 01:59:40  iachang
 * Fixed compiler warning
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/31 09:52:09  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

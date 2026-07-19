/* $Id: intel_gpio.c,v 1.1 2020/01/09 01:02:00 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/intel_gpio.c,v $
 *------------------------------------------------------------------
 *
 * intel_p2sb.c - Intel P2SB GPIO interface
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "intel_gpio.h"
#include "curie2ru_common.h"

#define P2SB_GPIO_SPACE_SIZE    0x10000

/* Get from Intel C620 Chipset Datasheet */
static int p2sb_gpio_port_ids[] = {
    0xAF,   /* GPIO Community 0 */
    0xAE,   /* GPIO Community 1 */
    0xAD,   /* GPIO Community 2 */
    0xAC,   /* GPIO Community 3 */
    0xAB,   /* GPIO Community 4 */
    0x11    /* GPIO Community 5 */
};

#define P2SB_GPIO_NR_COMMUNITY  \
    (sizeof(p2sb_gpio_port_ids) / sizeof(p2sb_gpio_port_ids[0]))

#define GPIORXDIS   0x200
#define GPIOTXDIS   0x100
#define GPIORXSTATE 0x02
#define GPIOTXSTATE 0x01

#define LBG_PAD_OWN     0x020
#define LBG_PADCFGLOCK  0x060
#define LBG_HOSTSW_OWN  0x080
#define LBG_GPI_IE      0x110

#define BIT(i)  (1 << (i))

/* Offset from regs */
#define REVID                           0x000
#define REVID_SHIFT                     16
#define REVID_MASK                      0xffff0000

#define PADBAR                          0x00c
#define GPI_IS                          0x100
#define GPI_GPE_STS                     0x140
#define GPI_GPE_EN                      0x160

#define PADOWN_BITS                     4
#define PADOWN_SHIFT(p)                 ((p) % 8 * PADOWN_BITS)
#define PADOWN_MASK(p)                  (0xf << PADOWN_SHIFT(p))
#define PADOWN_GPP(p)                   ((p) / 8)

/* Offset from pad_regs */
#define PADCFG0                         0x000
#define PADCFG0_RXEVCFG_SHIFT           25
#define PADCFG0_RXEVCFG_MASK            (3 << PADCFG0_RXEVCFG_SHIFT)
#define PADCFG0_RXEVCFG_LEVEL           0
#define PADCFG0_RXEVCFG_EDGE            1
#define PADCFG0_RXEVCFG_DISABLED        2
#define PADCFG0_RXEVCFG_EDGE_BOTH       3
#define PADCFG0_PREGFRXSEL              BIT(24)
#define PADCFG0_RXINV                   BIT(23)
#define PADCFG0_GPIROUTIOXAPIC          BIT(20)
#define PADCFG0_GPIROUTSCI              BIT(19)
#define PADCFG0_GPIROUTSMI              BIT(18)
#define PADCFG0_GPIROUTNMI              BIT(17)
#define PADCFG0_PMODE_SHIFT             10
#define PADCFG0_PMODE_MASK              (0xf << PADCFG0_PMODE_SHIFT)
#define PADCFG0_GPIORXDIS               BIT(9)
#define PADCFG0_GPIOTXDIS               BIT(8)
#define PADCFG0_GPIORXSTATE             BIT(1)
#define PADCFG0_GPIOTXSTATE             BIT(0)

#define PADCFG1                         0x004
#define PADCFG1_TERM_UP                 BIT(13)
#define PADCFG1_TERM_SHIFT              10
#define PADCFG1_TERM_MASK               (7 << PADCFG1_TERM_SHIFT)
#define PADCFG1_TERM_20K                4
#define PADCFG1_TERM_2K                 3
#define PADCFG1_TERM_5K                 2
#define PADCFG1_TERM_1K                 1

/* get the virual memory of P2SB controller */
void *intel_gpio_ioremap(unsigned int community)
{
    uint64_t bar;
    off_t base;
    uint32_t padoff;
    struct curie2ru_mmap map;

    if (community >= P2SB_GPIO_NR_COMMUNITY)
        return NULL;

    if (intel_p2sb_get_bar(&bar) < 0)
        return NULL;

    /* TODO: remove after bringing up */
    printf("P2SB BARREG %#lx\n", bar);

    base = bar + (p2sb_gpio_port_ids[community] << 16);

    map.paddr = (void *)(unsigned long)base;
    map.length = P2SB_GPIO_SPACE_SIZE;
    if (curie2ru_file_mmap(NULL, &map,
                           CURIE2RU_MMAP_READ | CURIE2RU_MMAP_WRITE)) {
        log_err("CPU GPIO: failed to remap COM%d\n", community);
        return NULL;
    }

    padoff = *(uint32_t *)(map.vaddr + 0xc);
    if (padoff != 0x400) {
        log_warn("CPU GPIO: warning: PADBAR %#x is not 0x400\n", padoff);
    }

    return map.vaddr;
}

void intel_gpio_iounmap(void *addr)
{
    struct curie2ru_mmap map;

    map.paddr = 0;
    map.vaddr = addr;
    map.length = P2SB_GPIO_SPACE_SIZE;
    curie2ru_file_munmap(&map);
}

/* check if the pin is owned by host */
static int gpio_owned_by_host(void *community, int offset)
{
    uint32_t *pad_own;

    pad_own = community + LBG_PAD_OWN;
    pad_own += PADOWN_GPP(offset);

    if (*pad_own & PADOWN_MASK(offset)) {
        log_err("GPIO%d is not owned by host\n", offset);
        return 0;
    }

    return 1;
}

/* check if the pin is locked */
static int gpio_locked(void *community, int offset)
{
    uint32_t *cfglock;
    uint32_t mask = 1 << (offset % 24);

    cfglock = community + LBG_PADCFGLOCK;
    cfglock += offset / 24 * 2;

    if (cfglock[0] & mask) {
        log_err("GPIO%d is locked\n", offset);
        return 1;
    }

    if (cfglock[1] & mask) {
        log_err("GPIO%d TX is locked\n", offset);
        return 1;
    }

    return 0;
}

#define PADCFG0ADDR(com, offset) ((uint32_t *)(com + 0x400 + offset * 8))

/* enable/disable CPU GPIO PIN */
int intel_gpio_request_enable(void *community, int offset)
{
    uint32_t *padcfg0, value;

    if (!gpio_owned_by_host(community, offset) ||
         gpio_locked(community, offset))
        return 0;

    padcfg0 = PADCFG0ADDR(community, offset);
    value = *padcfg0;

    if (value & PADCFG0_PMODE_MASK) {
        printf("GPIO%d enable GPIO mode\n", offset);
        /* Put the pad into GPIO mode */
        value &= ~PADCFG0_PMODE_MASK;
        /* Disable SCI/SMI/NMI generation */
        value &= ~(PADCFG0_GPIROUTIOXAPIC | PADCFG0_GPIROUTSCI);
        value &= ~(PADCFG0_GPIROUTSMI | PADCFG0_GPIROUTNMI);
        /* set input mode */
        value &= ~PADCFG0_GPIORXDIS;
        value |= PADCFG0_GPIOTXDIS;

        *padcfg0 = value;
    }

    return 1;
}

/* set CPU GPIO pin direction */
void intel_gpio_set_direction(void *community, int offset, int input)
{
    uint32_t *padcfg0, value;

    padcfg0 = PADCFG0ADDR(community, offset);
    value = *padcfg0;

    if (input) {
        value &= ~PADCFG0_GPIORXDIS;
        value |= PADCFG0_GPIOTXDIS;
    } else {
        value &= ~PADCFG0_GPIOTXDIS;
        value |= PADCFG0_GPIORXDIS;
    }

    *padcfg0 = value;
}

/* configure CPU GPIO pin value */
void intel_gpio_set(void *community, int offset, uint32_t value)
{
    uint32_t *padcfg0, cfg0;

    padcfg0 = PADCFG0ADDR(community, offset);
    cfg0 = *padcfg0;

    if (value)
        cfg0 |= PADCFG0_GPIOTXSTATE;
    else
        cfg0 &= ~PADCFG0_GPIOTXSTATE;

    *padcfg0 = cfg0;
}

/* get CPU GPIO pin value */
int intel_gpio_get(void *community, int offset)
{
    uint32_t *padcfg0, cfg0;

    padcfg0 = PADCFG0ADDR(community, offset);
    cfg0 = *padcfg0;

    if (!(cfg0 & PADCFG0_GPIOTXDIS)) {
        return !!(cfg0 & PADCFG0_GPIOTXSTATE);
    } else {
        return !!(cfg0 & PADCFG0_GPIORXSTATE);
    }
}

/* show GPIO pin status */
void intel_gpio_dbg_show(void *community, int offset)
{
    uint32_t *padcfg0, cfg0, cfg1;
    int owned, locked;

    padcfg0 = PADCFG0ADDR(community, offset);
    cfg0 = padcfg0[0];
    cfg1 = padcfg0[1];

    owned = gpio_owned_by_host(community, offset);
    locked = gpio_locked(community, offset);
    printf("Owned by host: %d, Locked: %d, PADCFG0 %#x, PADCFG1 %#x\n",
            owned, locked, cfg0, cfg1);
}

/*
 *-----------------------------------------------------------------------------
$Log: intel_gpio.c,v $
Revision 1.1  2020/01/09 01:02:00  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

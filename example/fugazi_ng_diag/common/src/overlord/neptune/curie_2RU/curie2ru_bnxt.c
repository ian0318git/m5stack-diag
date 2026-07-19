/*------------------------------------------------------------------
 *
 * curie2ru_bnxt.c - Curie2ru broadcom bnxt interfaces.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "curie2ru_common.h"
#include "curie2ru_bnxt.h"

#define IF_SYS_DIR "/sys/bus/pci/devices/%04x:%02x:%02x.%01d/net"
#define IF_SYS_PATH_MAX 128

#ifndef MDIO_PHY_ID_C45
#define MDIO_PHY_ID_C45			0x8000
#endif

int curie2ru_bnxt_init(struct curie2ru_bnxt *bnxt,
                       struct curie2ru_bnxt_settings *settings)
{
    DIR *dir;
    struct dirent *dp;
    char path[IF_SYS_PATH_MAX];

    snprintf(path, sizeof(path), IF_SYS_DIR,
             settings->pci_domain, settings->pci_bus,
             settings->pci_dev, settings->pci_func);
    if ((dir = opendir(path)) == NULL) {
        char dbgcmd[128], bus[64];

        log_err("open system directory: %s failed!\n", path);
        snprintf(bus, sizeof(bus), "%04x:%02x:%02x", settings->pci_domain,
                 settings->pci_bus, settings->pci_dev);
        snprintf(dbgcmd, sizeof(dbgcmd), "lspci -s %s", bus);
        system(dbgcmd);
        snprintf(dbgcmd, sizeof(dbgcmd), "dmesg | grep \"bnxt_en %s\"", bus);
        system(dbgcmd);

        return -1;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        strncpy(bnxt->ifname, dp->d_name, BNXT_IFNAME_SIZE);
    }
    closedir(dir);

    if ((bnxt->skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        log_err("open socket failed!\n");
        return -1;
    }

    memcpy(&bnxt->settings, settings, sizeof(*settings));
    return 0;
}

void curie2ru_bnxt_exit(struct curie2ru_bnxt *bnxt)
{
    close(bnxt->skfd);
}

static inline struct mii_ioctl_data *
curie2ru_bnxt_mdio_ifr(struct curie2ru_bnxt *bnxt, struct ifreq *ifr)
{
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr->ifr_data;

    memset(ifr, 0, sizeof(*ifr));
    strncpy(ifr->ifr_name, bnxt->ifname, IFNAMSIZ);
    return mdio;
}

int curie2ru_bnxt_mdio_read(struct curie2ru_bnxt *bnxt,
                            uint8_t prtad, uint8_t devad,
                            uint16_t addr, uint16_t *data)
{
    struct ifreq ifr;
    struct mii_ioctl_data *mdio = curie2ru_bnxt_mdio_ifr(bnxt, &ifr);

    mdio->phy_id = (((prtad & 0x1f) << 5) | (devad & 0x1f));
    mdio->phy_id |= MDIO_PHY_ID_C45;
    mdio->reg_num = addr;
    if (ioctl(bnxt->skfd, SIOCGMIIREG, &ifr) < 0) {
        log_err("mdio read failed: %x.%x:%x!\n", prtad, devad, addr);
        return -1;
    }
    *data = mdio->val_out;
    return 0;
}

int curie2ru_bnxt_mdio_write(struct curie2ru_bnxt *bnxt,
                             uint8_t prtad, uint8_t devad,
                             uint16_t addr, uint16_t data)
{
    struct ifreq ifr;
    struct mii_ioctl_data *mdio = curie2ru_bnxt_mdio_ifr(bnxt, &ifr);

    mdio->phy_id = (((prtad & 0x1f) << 5) | (devad & 0x1f));
    mdio->phy_id |= MDIO_PHY_ID_C45;
    mdio->reg_num = addr;
    mdio->val_in = data;
    if (ioctl(bnxt->skfd, SIOCSMIIREG, &ifr) < 0) {
        log_err("mdio write failed: %x.%x:%x!\n", prtad, devad, addr);
        return -1;
    }
    return 0;
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_bnxt.c,v $
Revision 1.2  2020/03/11 17:46:59  jiajliu
Refine code for bcm utlity and test

Revision 1.1  2020/01/09 01:01:56  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

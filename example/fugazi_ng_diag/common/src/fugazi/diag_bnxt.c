/* $Id: diag_bnxt.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bnxt.c,v $
 *------------------------------------------------------------------
 *
 * diag_bnxt.c - Fugazi broadcom bnxt interfaces function
 *               Port from Curie-2RU curie2ru_bnxt.c
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "common.h"
#include "diag_common.h"
#include "diag_bnxt.h"
#include "nvsysvars.h"
#include "diag_bcm_lib.h"


int fugazi_bnxt_init (struct fugazi_bnxt *bnxt,
                      struct fugazi_bnxt_settings *settings)
{
    DIR *dir;
    struct dirent *dp;
    char path[IF_SYS_PATH_MAX];

    snprintf(path, sizeof(path), IF_SYS_DIR,
             settings->pci_domain, settings->pci_bus,
             settings->pci_dev, settings->pci_func);
    if ((dir = opendir(path)) == NULL) {
        log_err("open system directory: %s failed!\n", path);
        return -1;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.') {
            continue;
        }
        strncpy(bnxt->ifname, dp->d_name, BNXT_IFNAME_SIZE);
    }
    closedir(dir);

    if ((bnxt->skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        log_err("open socket failed!\n");
        return -1;
    }

    memcpy(&bnxt->settings, settings, sizeof(*settings));
    return (PASSED);
}

void fugazi_bnxt_exit(struct fugazi_bnxt *bnxt)
{
    close(bnxt->skfd);
}

static inline struct mii_ioctl_data *
fugazi_bnxt_mdio_ifr (struct fugazi_bnxt *bnxt, struct ifreq *ifr)
{
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr->ifr_data;

    memset(ifr, 0, sizeof(*ifr));
    strncpy(ifr->ifr_name, bnxt->ifname, IFNAMSIZ);
    return mdio;
}

int fugazi_bnxt_mdio_read (struct fugazi_bnxt *bnxt,
                           uint8_t prtad, uint16_t devad,
                           uint16_t addr, uint16_t *data)
{
    struct ifreq ifr;
    struct mii_ioctl_data *mdio = fugazi_bnxt_mdio_ifr(bnxt, &ifr);

    if (devad != 0)
    {
        mdio->phy_id = (((prtad & 0x1f) << 5) | (devad & 0x1f) | MDIO_PHY_ID_C45);
        mdio->reg_num = addr;
    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            log_info("DBG 2: prtad=%x,devad=%x,mdio->phy_id=%x\n", prtad, devad, 
                      mdio->phy_id);
        }
        if (ioctl(bnxt->skfd, SIOCGMIIREG, &ifr) < 0) {
            log_err("mdio read failed: %x.%x:%x!\n", prtad, devad, addr);
            return -1;
        }
        *data = mdio->val_out;
    } else {
        mdio->phy_id = prtad & 0x1f;
        mdio->reg_num = addr;
    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            log_info("DBG 2: prtad=%x,devad=%x,mdio->phy_id=%x\n", prtad, devad, 
                      mdio->phy_id);
        }
        if (ioctl(bnxt->skfd, SIOCGMIIREG, &ifr) < 0) {
            log_err("mdio read failed: %x.%x:%x!\n", prtad, devad, addr);
            return -1;
        }
        *data = mdio->val_out;
    }

    return (PASSED);
}

int fugazi_bnxt_mdio_write(struct fugazi_bnxt *bnxt,
                           uint8_t prtad, uint16_t devad,
                           uint16_t addr, uint16_t data)
{
    struct ifreq ifr;
    struct mii_ioctl_data *mdio = fugazi_bnxt_mdio_ifr(bnxt, &ifr);

    if (devad != 0)
    {
        mdio->phy_id = (((prtad & 0x1f) << 5) | (devad & 0x1f) | MDIO_PHY_ID_C45);
        mdio->reg_num = addr;
        mdio->val_in = data;
        if (ioctl(bnxt->skfd, SIOCSMIIREG, &ifr) < 0) {
            log_err("mdio write failed: %x.%x:%x!\n", prtad, devad, addr);
            return -1;
        }
    } else {
        mdio->phy_id = (prtad & 0x1f);
        mdio->reg_num = addr;
        mdio->val_in = data;
        if (ioctl(bnxt->skfd, SIOCSMIIREG, &ifr) < 0) {
            log_err("mdio write failed: %x.%x:%x!\n", prtad, devad, addr);
            return -1;
        }
    }

    return (PASSED);
}



/*-------------------------------------------------
 * $Log: diag_bnxt.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.9  2020/08/07 02:54:29  iachang
 * code clean up.
 *
 * Revision 1.1.6.8  2020/08/05 08:33:23  iachang
 * Code clean up.
 *
 * Revision 1.1.6.7  2020/07/31 09:52:08  iachang
 * Code clean up.
 *
 * Revision 1.1.6.6  2019/05/28 06:04:45  iachang
 * Separated BCM57412 FW/Cfg program at two items.
 *
 * Revision 1.1.6.5  2019/04/25 23:25:27  letsai
 * 1. Remove eUSB test.
 * 2. Fixed bnxt_mdio r/w function to support both 1G and 10G phy.
 *
 * Revision 1.1.6.4  2019/04/06 01:07:50  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.3  2019/03/14 21:46:47  iachang
 * Bring up BCM82757 first PHY.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 *
 *
 * $Endlog$
 * */



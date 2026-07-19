/* $Id: bcm57412_lib.c,v 1.4 2020/11/03 06:16:16 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm57412_lib.c,v $
 *------------------------------------------------------------------
 *
 * bcm57412_lib.c - Functions of BCM57412 MDIO.
 *
 * Feb. 2019, Leslie Chen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"

#include "common_utils.h"
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include "platform_eth.h"
#include "bcm57412_lib.h"

#include<errno.h>
#include<ifaddrs.h> /* for using getifaddrs */

struct curie_bcm82752 curie2ru, *curie = &curie2ru;

#define IF_SYS_DIR "/sys/bus/pci/devices/%04x:%02x:%02x.%01d/net"
#define IF_SYS_PATH_MAX 128

/*
 * To support newer version of bnxt_en driver,
 * otherwise BCM82752 firmware download will fail.
 */
#ifndef MDIO_PHY_ID_C45
#define MDIO_PHY_ID_C45                 0x8000
#endif

int curie_bnxt_init(struct curie_bnxt *bnxt,
                       struct curie_bnxt_settings *settings)
{
    DIR *dir;
    struct dirent *dp;
    char path[IF_SYS_PATH_MAX];

    snprintf(path, sizeof(path), IF_SYS_DIR,
             settings->pci_domain, settings->pci_bus,
             settings->pci_dev, settings->pci_func);
    if ((dir = opendir(path)) == NULL) {
        printf("open system directory: %s failed!\n", path);
        return -1;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        strncpy(bnxt->ifname, dp->d_name, BNXT_IFNAME_SIZE);
    }
    closedir(dir);

    if ((bnxt->skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("open socket failed!\n");
        return -1;
    }

    memcpy(&bnxt->settings, settings, sizeof(*settings));
    return 0;
}

void curie_bnxt_exit(struct curie_bnxt *bnxt)
{
    close(bnxt->skfd);
}

static inline struct mii_ioctl_data *
curie_bnxt_mdio_ifr(struct curie_bnxt *bnxt, struct ifreq *ifr)
{
    struct mii_ioctl_data *mdio = (struct mii_ioctl_data *)&ifr->ifr_data;

    memset(ifr, 0, sizeof(*ifr));
    strncpy(ifr->ifr_name, bnxt->ifname, IFNAMSIZ);
    return mdio;
}

int curie_bnxt_mdio_read(struct curie_bnxt *bnxt,
                            uint8_t prtad, uint8_t devad,
                            uint16_t addr, uint16_t *data)
{
    struct ifreq ifr;
    struct mii_ioctl_data *mdio = curie_bnxt_mdio_ifr(bnxt, &ifr);

    mdio->phy_id = (((prtad & 0x1f) << 5) | (devad & 0x1f));
    /* To support MDIO_45 */
    mdio->phy_id |= MDIO_PHY_ID_C45;
    mdio->reg_num = addr;
    if (ioctl(bnxt->skfd, SIOCGMIIREG, &ifr) < 0) {
        printf("mdio read failed: %x.%x:%x!\n", prtad, devad, addr);
        return -1;
    }
    *data = mdio->val_out;
    return 0;
}

int curie_bnxt_mdio_write(struct curie_bnxt *bnxt,
                             uint8_t prtad, uint8_t devad,
                             uint16_t addr, uint16_t data)
{
    struct ifreq ifr;
    struct mii_ioctl_data *mdio = curie_bnxt_mdio_ifr(bnxt, &ifr);

    mdio->phy_id = (((prtad & 0x1f) << 5) | (devad & 0x1f));
    /* To support MDIO_45 */
    mdio->phy_id |= MDIO_PHY_ID_C45;
    mdio->reg_num = addr;
    mdio->val_in = data;
    if (ioctl(bnxt->skfd, SIOCSMIIREG, &ifr) < 0) {
        printf("mdio write failed: %x.%x:%x!\n", prtad, devad, addr);
        return -1;
    }
    return 0;
}

static void curie_bcm82752_exit(struct curie_bcm82752 *curie)
{
    struct curie_quadra28 *q28 = curie->quadra28;

    curie_quadra28_exit(q28);
}

static void __curie_bcm57412_exit(struct curie_bnxt *bnxt)
{
    curie_bnxt_exit(bnxt);
}

static void curie_bcm57412_exit(struct curie_bcm82752 *curie)
{
    __curie_bcm57412_exit(&curie->bnxt[1]);
    __curie_bcm57412_exit(&curie->bnxt[0]);
}

void curie_exit(void)
{
    curie_bcm57412_exit(curie);

    curie_bcm82752_exit(curie);
}

static int __curie_bcm57412_init(struct curie_bnxt *bnxt,
                                    struct curie_bnxt_settings *settings)
{
    int rc;

    if ((rc = curie_bnxt_init(bnxt, settings))) {
        printf("curie_bnxt_init failed\n");
        return rc;
    }

    return 0;
}

static int curie_bcm57412_init(struct curie_bcm82752 *curie)
{
    struct curie_bnxt_settings settings[NR_CURIE_BNXT] = {
        {
            .pci_domain = 0,
            .pci_bus = 0x1,
            .pci_dev = 0,
            .pci_func = 0,
        },
        {
            .pci_domain = 0,
            .pci_bus = 0x1,
            .pci_dev = 0,
            .pci_func = 0,
        },
    };
    int rc;

    if ((rc = __curie_bcm57412_init(&curie->bnxt[0], &settings[0])))
        return rc;
    if ((rc = __curie_bcm57412_init(&curie->bnxt[1], &settings[1]))) {
        __curie_bcm57412_exit(&curie->bnxt[0]);
        return rc;
    }

    return 0;
}

static int curie_bcm82752_mdio_read(void *ctx, uint8_t mdio, uint8_t devad,
                                       uint16_t addr, uint16_t *data)
{
    return curie_bnxt_mdio_read(ctx, mdio, devad, addr, data);
}

static int curie_bcm82752_mdio_write(void *ctx, uint8_t mdio, uint8_t devad,
                                        uint16_t addr, uint16_t data)
{
    return curie_bnxt_mdio_write(ctx, mdio, devad, addr, data);
}

int curie_bcm82752_init(struct curie_bcm82752 *curie)
{
    struct curie_quadra28_settings settings[] = {
        {
            .type  = "quadra28",
            .ctx   = &curie->bnxt[0],
            .read  = curie_bcm82752_mdio_read,
            .write = curie_bcm82752_mdio_write,
        },
        {
            .type  = "quadra28",
            .ctx   = &curie->bnxt[0],
            .read  = curie_bcm82752_mdio_read,
            .write = curie_bcm82752_mdio_write,
        }
    };
    struct curie_quadra28 *q28 = curie->quadra28;
    int rc;

    /* quadra28 sdk init */
    if ((rc = curie_quadra28_init(q28, settings))) {
        printf("curie_quadra28_init failed\n");
        return rc;
    }

    return 0;
}

int curie_init(void)
{
    int rc;

    memset(curie, 0, sizeof(*curie));

    if ((rc = curie_bcm57412_init(curie))) {
        printf("curie_bcm57412_init failed\n");
        return rc;
    }

     if ((rc = curie_bcm82752_init(curie))) {
            printf("curie_bcm82752_init failed\n");
            return rc;
     }

    return 0;
}

long bcm82752_reg_read(void)
{
    int port_max = 1;
    int port_min = 0;
    int rc;
    uint32_t devaddr, regaddr;
    uint16_t data;
    uint phy_addr, portnum;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE_MIURA_DEV_PMA_PMD,
                            CURIE_MIURA_DEV_PCS,
                            CURIE_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);

    phy_addr = te_port_mapping_phy_addr[portnum];
    rc = curie_bnxt_mdio_read(&curie->bnxt[0], phy_addr, devaddr, regaddr, &data);

    if (rc < 0) {
        printf("BCM82752 read error\n");
        return (FAILED);
    }

    printf("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);

    return (PASSED);
}

long bcm82752_reg_write(void)
{
    int port_max = 1;
    int port_min = 0;
    int rc;
    uint32_t devaddr, regaddr;
    uint16_t data;
    uint phy_addr, portnum;

    portnum = gethex_answer("\nEnter TE port num(0x0 - 0x1) ", port_min, port_min, port_max);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE_MIURA_DEV_PMA_PMD,
                            CURIE_MIURA_DEV_PCS,
                            CURIE_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    phy_addr = te_port_mapping_phy_addr[portnum];
    rc = curie_bnxt_mdio_write(&curie->bnxt[0], phy_addr, devaddr, regaddr, data);

    if (rc < 0) {
        printf("BCM82752 write error\n");
        return (FAILED);
    }

    printf("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: curie1ru_port_is_linkup
 *   Check link up status from Linux information.
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int curie1ru_port_is_linkup(int port) {
    int timeout_counter = 1200, is_link = FALSE;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];
    
    sprintf(pname,"eth%d", port);  
    
    while(1) {
        /* Get the interface information */
        if (getifaddrs(&if_list) < 0) {
            printf("Failed to get interface information: %s.\n",
            strerror(errno));
            return(FAILED);
        }

        if (if_list == NULL) {
            printf("No network interfaces were found.\n");
            return(FAILED);
        }

        for (if_info = if_list; if_info; if_info = if_info->ifa_next) {
            /* parse the port name */
            if (strncmp(if_info->ifa_name, pname, strlen(pname)) != 0) {
            	continue;
            }
            	 
            flags = if_info->ifa_flags;
            if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
                /* Link up */
                fflush(stdout);
                is_link = TRUE;
                break;
            } else {
                /* Link down */
                msleep(10);
                timeout_counter--;
                if (timeout_counter == 0) {
                    freeifaddrs(if_list);
                    printf("Checking eth%x from Linux side - Timeout\n", port); 
                    return(FAILED);
                }
            }

	    fflush(stdout);
        } /*for*/
        
        freeifaddrs(if_list);

        if (is_link == TRUE) {
            printf("Checking eth%x from Linux side - Link up\n", port); 
            break;        
        }
    } /*while*/

    return (PASSED);
}

/*-------------------------------------------------
$Log: bcm57412_lib.c,v $
Revision 1.4  2020/11/03 06:16:16  leschen
To support MDIO_45 for 3rd RDT new bnxt_en driver.

Revision 1.3  2020/01/21 05:49:01  leschen
Fixing Ethernet port 4(10G) high temperature lbpk issue.

Revision 1.2  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/03/12 07:59:05  leschen
Initial check in to support BCM82752


$Endlog$
*/

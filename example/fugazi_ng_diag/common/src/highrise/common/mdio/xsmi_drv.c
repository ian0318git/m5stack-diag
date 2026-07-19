#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "xsmi_drv.h"
#include "hr_commn_util.h"
#include "highrise.h"

struct ioctl_data {
    // if 4.14 kernel, dev addr integrated with 'phy'
    // if 4.4  kernel, dev addr integrated with 'reg'
    union {
        uint16_t     phy;
        struct {
            uint16_t devad:5;
            uint16_t phyid:5;
        };
    };

    union {
        uint32_t     reg;
        struct {
            uint16_t off;
            uint16_t dev;
        };
    };
    uint16_t    val_in;
    uint16_t    val_out;
};

#define SIOCGMIIREG	0x8948
#define SIOCSMIIREG	0x8949

static int xsmi_read(struct xsmi_dev *xsmi, uint8_t phy, uint8_t dev, uint16_t reg, uint16_t *data, uint16_t cnt)
{
    struct ifreq ifr = {
        .ifr_ifrn  = {
            .ifrn_name = "eth0",
        },
    };
    struct ioctl_data *cdata = (struct ioctl_data *)&ifr.ifr_ifru;

    cdata->phy = 0;
    cdata->reg = 0;
    switch(highrise_kernel_ver()) {
    case HR_KERNEL_VER_4_4:
        cdata->phy = phy;
        cdata->dev = dev;
        cdata->off = reg;
        break;
    case HR_KERNEL_VER_4_14:
        cdata->phyid = phy;
        cdata->devad = dev;
        cdata->phy  |= 0x8000;
        cdata->reg   = reg;
        break;
    default:
        return -(__LINE__);
    }

    cdata->val_in = 0;
    cdata->val_out= 0;

    ERR_RET_COND(0 > ioctl(xsmi->fd, SIOCGMIIREG, &ifr), -(__LINE__), "Read failed.\n");
    *data = cdata->val_out;
    return 0;
}

static int xsmi_write(struct xsmi_dev *xsmi, uint8_t phy, uint8_t dev, uint16_t reg, uint16_t *data)
{
    struct ifreq ifr = {
        .ifr_ifrn  = {
            .ifrn_name = "eth0",
        },
    };
    struct ioctl_data *cdata = (struct ioctl_data *)&ifr.ifr_ifru;

    cdata->phy = 0;
    cdata->reg = 0;
    switch(highrise_kernel_ver()) {
    case HR_KERNEL_VER_4_4:
        cdata->phy = phy;
        cdata->dev = dev;
        cdata->off = reg;
        break;
    case HR_KERNEL_VER_4_14:
        cdata->phyid = phy;
        cdata->devad = dev;
        cdata->phy  |= 0x8000;
        cdata->reg   = reg;
        break;
    default:
        return -(__LINE__);
    }
    cdata->val_in = *data;
    cdata->val_out= 0;

    ERR_RET_COND(0 > ioctl(xsmi->fd, SIOCSMIIREG, &ifr), -(__LINE__), "Read failed.\n");
    return 0;
}

static int xsmi_sock_open(struct xsmi_dev *xsmi)
{
    xsmi->fd = socket(AF_UNIX,SOCK_DGRAM, 0);
    ERR_RET_COND(xsmi->fd < 0, -(__LINE__), "Failed create socket.\n");
    return 0;
}

static int xsmi_sock_release(struct xsmi_dev *xsmi)
{
    if (xsmi->fd > 0) {
        close(xsmi->fd);
        xsmi->fd = -1;
    }
    return 0;
}

static struct xsmi_drv _driver = {
    .ctrl  = NULL,
    .read  = xsmi_read,
    .write = xsmi_write,
    .lock  = NULL,
};

/*
 * Desc:
 *  This create XSMI controller device with driver associated.
 * Params:
 *  xsmi    - IN & OUT - A poionter to take back the xsmi dev.
 *            When passed in, xsmi != NULL, *xsmi = NULL.
 *  paddr   - IN       - The xsmi's register physical addr within CPU space.
 *  size    - IN       - The xsmi's register space size.
 *  name    - IN       - The sim's name.
 * Return:
 *  =0      - OK
 *  <0      - Failed
 *  >0      - Undefined
 */
int xsmi_dev_create(struct xsmi_dev **xsmi, uint64_t paddr, uint64_t size, char *name)
{
    int uret = 0;

    LOG_DBG("Enter '%s'\n", __func__);
    ERR_RET_COND((!xsmi || *xsmi || paddr == 0 || size != XSMI_REG_SPACE_SIZE || !name),
                -(__LINE__), "Invalid argument.\n");

    *xsmi = (struct xsmi_dev *)malloc(sizeof(struct xsmi_dev));
    ERR_RET_COND(!*xsmi, -(__LINE__), "Malloc failed\n");
    memset(*xsmi, 0, sizeof(struct xsmi_dev));

    strncpy((*xsmi)->name, name, sizeof((*xsmi)->name) - 1);
    (*xsmi)->paddr = paddr;
    (*xsmi)->size  = size;
    (*xsmi)->fd    = -1;
    (*xsmi)->drv   = &_driver;

    (*xsmi)->vaddr = MAP_FAILED;
    ERR_URET_COND(0 > xsmi_sock_open(*xsmi), -(__LINE__), "Failed.\n");

    uret = 0;
_EXIT_POINT:
    if (uret < 0) {
        LOG_LOG("%d:'%s()' failed\n", __LINE__, __FILE__);
        if (*xsmi) {
            xsmi_sock_release(*xsmi);
            free(*xsmi);
            *xsmi = NULL;
        }
    }
    return uret;
}

/*
 * Desc:
 *  Free the xsmi controller dev allocated by 'xsmi_dev_create()'
 * Params:
 *  xsmi     - IN - xsmi dev allocated by 'xsmi_dev_create()'
 * Return:
 *  =0      - OK
 *  <0      - Failed
 *  >0      - Undefined
 */
int xsmi_dev_free(struct xsmi_dev *xsmi)
{
    LOG_DBG("Enter '%s()'\n", __func__);
    ERR_RET_COND(!xsmi, -(__LINE__), "Invalid argument.\n");
    xsmi_sock_release(xsmi);
    free(xsmi);
    return 0;
}

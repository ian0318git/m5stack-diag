#ifndef __XXX_YYY_XSMI_DRV_H__
#define __XXX_YYY_XSMI_DRV_H__

#include <stdint.h>
#include <pthread.h>

struct xsmi_dev;
struct xsmi_ctrl_data;

struct xsmi_drv {
    int(*ctrl )(struct xsmi_dev *xsmi, uint32_t cmd, struct xsmi_ctrl_data *ctrl);
    int(*read )(struct xsmi_dev *xsmi, uint8_t phy, uint8_t dev, uint16_t reg, uint16_t *data, uint16_t cnt);
    int(*write)(struct xsmi_dev *xsmi, uint8_t phy, uint8_t dev, uint16_t reg, uint16_t *data);
    #define XSMI_DEV_OP_UNLOCK  0
    #define XSMI_DEV_OP_LOCK    1
    int(*lock )(struct xsmi_dev *xsmi, int op);
};

struct xsmi_dev {
    char      name[16];
    uint64_t  paddr;
    volatile void *vaddr;
    uint64_t  size;
    pthread_mutex_t mtx;

    int       fd;  // xmdio read by netlink sock 
    struct xsmi_drv *drv;
};

#define XSMI_MNGM_REG_OFF       0x0
#define XSMI_ADDR_REG_OFF       0x8
#define XSMI_CONF_REG_OFF       0xC

#define XSMI_REG_SPACE_ADDR     0xf212a600
#define XSMI_REG_SPACE_SIZE     0x1C

#define XSMI_CTRL_ALIGN_FALL_EDGE   (1<<0)
#define XSMI_CTRL_PHY_SEL           (1<<1)
#define XSMI_CTRL_TIMING_CFG        (1<<2)
#define XSMI_CTRL_MDC_DIVISION      (1<<3)
#define XSMI_CTRL_CMD_GET_CFG       (1<<31)
struct xsmi_ctrl_data {
    #define XSMI_MDC_DIVISION_256   0
    #define XSMI_MDC_DIVISION_64    1
    #define XSMI_MDC_DIVISION_32    2
    #define XSMI_MDC_DIVISION_8     3
    uint32_t mdc_division       :2;
    uint32_t mdio_timing        :2;
    uint32_t phy_sel            :1;
    uint32_t align_to_fall_edge :1;
    uint32_t                    :26;
};


#define XSMI_MDIO_TIMEOUT 100000

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
int xsmi_dev_create(struct xsmi_dev **xsmi, uint64_t paddr, uint64_t size, char *name);

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
int xsmi_dev_free(struct xsmi_dev *xsmi);
#endif

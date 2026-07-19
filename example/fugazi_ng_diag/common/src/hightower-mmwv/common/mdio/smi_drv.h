#ifndef __XXX_YYY_SMI_DRV_H__
#define __XXX_YYY_SMI_DRV_H__

#include <stdint.h>
#include <pthread.h>

struct smi_dev;

struct smi_drv {
    int(*ctrl  )(struct smi_dev *smi, uint32_t cmd, void *data);
    int(*read  )(struct smi_dev *smi, uint8_t phy, uint8_t reg, uint16_t *data);
    int(*write )(struct smi_dev *smi, uint8_t phy, uint8_t reg, uint16_t *data);
};

struct smi_dev {
    char      name[16];
    uint64_t  paddr;
    void     *vaddr;
    uint64_t  size ;
    pthread_mutex_t mtx;
    struct smi_drv *drv;
};

#define SMI_MNG_REG_OFF         0x0
#define SMI_CFG_REG_OFF         0x4
#define PHY_AUTO_NEGO_CFG_OFF   0x8
#define PHY_ADDR_REG0_OFF       0xC

#define SMI_REG_SPACE_ADDR      0xf212a200
#define SMI_REG_SPACE_SIZE      0x4C

#define SMI_CTRL_CMD_ACCEL_EN       (1<<0)
#define SMI_CTRL_CMD_FAST_MDC       (1<<1)
#define SMI_CTRL_CMD_FAST_MDC_DIV   (1<<2)
#define SMI_CTRL_CMD_OUT_LATENCY    (1<<3)
#define SMI_CTRL_CMD_AUTO_POLL_NUM  (1<<4)
#define SMI_CTRL_CMD_POLL_EN        (1<<5)
#define SMI_CTRL_CMD_INVERT_MDC     (1<<6)

#define SMI_CTRL_CMD_GET_CFG        (1<<31)
struct smi_ctrl_data {
    struct {
        uint32_t accel_en     :1;
        uint32_t fast_mdc     :1;
        uint32_t fast_mdc_div :2;
        uint32_t out_latency  :1;
        uint32_t auto_poll_num:5;
        uint32_t poll_en      :1;
        uint32_t invert_mdc   :1;
        uint32_t              :20;
    }smi_cfg;
};


#define SMI_MDIO_TIMEOUT 100000

/*
 * Desc:
 *  This create SMI controller device with driver associated.
 * Params:
 *  smi     - IN & OUT - A poionter to take back the smi dev.
 *            When passed in, smi != NULL, *smi = NULL.
 *  paddr   - IN       - The smi's register physical addr within CPU space.
 *  size    - IN       - The smi's register space size.
 *  name    - IN       - The sim's name.
 * Return:
 *  =0      - OK
 *  <0      - Failed
 *  >0      - Undefined
 */
int smi_dev_create(struct smi_dev **smi, uint64_t paddr, uint64_t size, char *name);

/*
 * Desc:
 *  Free the smi controller dev allocated by 'smi_dev_create()'
 * Params:
 *  smi     - IN - smi dev allocated by 'smi_dev_create()'
 * Return:
 *  =0      - OK
 *  <0      - Failed
 *  >0      - Undefined
 */
int smi_dev_free(struct smi_dev *smi);
#endif

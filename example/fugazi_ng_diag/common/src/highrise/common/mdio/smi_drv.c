#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include "smi_drv.h"
#include "hr_commn_util.h"

#define SMI_REG_RD32(P_SMI__, ROFF__, P_DATA) \
    *((uint32_t *)(P_DATA)) = *((uint32_t *)((P_SMI__)->vaddr + ROFF__))

#define SMI_REG_WR32(P_SMI__, ROFF__, P_DATA) \
    *((uint32_t *)((P_SMI__)->vaddr + ROFF__)) = *((uint32_t *)(P_DATA))

static int smi_ctrl (struct smi_dev *smi, uint32_t cmd, void *data)
{
    int flag = 0;
    struct smi_ctrl_data *ctrl = (struct smi_ctrl_data *)data;
    typeof(((struct smi_ctrl_data *)0)->smi_cfg) smi_cfg;

    LOG_DBG("Enter '%s'\n", __func__);
    ERR_RET_COND(!smi || !data, -(__LINE__), "Invalid params.\n");


    if (cmd & SMI_CTRL_CMD_GET_CFG) {
        SMI_REG_RD32(smi, SMI_CFG_REG_OFF, &ctrl->smi_cfg);
        return 0;
    }

    /* pthread_mutex_lock(&smi->mtx); */
    SMI_REG_RD32(smi, SMI_CFG_REG_OFF, &smi_cfg);

    #define SET_CFG(CMD_, FLG_, OV_, NV_) do {\
        if (((CMD_) & (FLG_)) && (OV_) != (NV_)) { \
            (OV_) = (NV_); \
            flag += 1; \
        } \
    }while(0)

    SET_CFG(cmd, SMI_CTRL_CMD_ACCEL_EN     , smi_cfg.accel_en     , ctrl->smi_cfg.accel_en     );
    SET_CFG(cmd, SMI_CTRL_CMD_FAST_MDC     , smi_cfg.fast_mdc     , ctrl->smi_cfg.fast_mdc     );
    SET_CFG(cmd, SMI_CTRL_CMD_FAST_MDC_DIV , smi_cfg.fast_mdc_div , ctrl->smi_cfg.fast_mdc_div );
    SET_CFG(cmd, SMI_CTRL_CMD_OUT_LATENCY  , smi_cfg.out_latency  , ctrl->smi_cfg.out_latency  );
    SET_CFG(cmd, SMI_CTRL_CMD_AUTO_POLL_NUM, smi_cfg.auto_poll_num, ctrl->smi_cfg.auto_poll_num);
    SET_CFG(cmd, SMI_CTRL_CMD_POLL_EN      , smi_cfg.poll_en      , ctrl->smi_cfg.poll_en      );
    SET_CFG(cmd, SMI_CTRL_CMD_INVERT_MDC   , smi_cfg.invert_mdc   , ctrl->smi_cfg.invert_mdc   );
    if (flag)
        SMI_REG_WR32(smi, SMI_CFG_REG_OFF, &smi_cfg);

    /* pthread_mutex_unlock(&smi->mtx); */
    return 0;
}


union _mr{
    uint32_t reg;
    struct {
        uint32_t data     :16;
        uint32_t phy_addr :5;
        uint32_t reg_addr :5;
        uint32_t op_code  :1;
        uint32_t rd_valid :1;
        uint32_t busy     :1;
    };
};

static const int POLL_BUSY = 0;
static const int POLL_VALID= 1;
static int smi_poll(struct smi_dev *smi, const int poll, const int lock)
{
    int timeout = SMI_MDIO_TIMEOUT;
    union _mr mr;

    while(timeout > 0) {
        mr.reg = 0;
        SMI_REG_RD32(smi, SMI_MNG_REG_OFF, &mr.reg);
        if ((poll == POLL_BUSY  && mr.busy) ||
            (poll == POLL_VALID && mr.rd_valid == 0)) {
          /*if (lock)
                pthread_mutex_unlock(&smi->mtx); */
            usleep(100);
            timeout -= 100;
          /*if (lock)
                pthread_mutex_lock(&smi->mtx); */
            continue;
        }
        break;
    }
    return timeout < 0 ? -1 : 0;
}

static const int OP_RD = 0;
static const int OP_WR = 1;
static int smi_rdwr(struct smi_dev *smi, const int op, uint8_t phy, uint8_t reg, uint16_t *data)
{
    int uret = 0;
    union _mr mr;

    LOG_DBG("Enter '%s'\n", __func__);

 /* pthread_mutex_lock(&smi->mtx); */
    ERR_URET_COND(smi_poll(smi, POLL_BUSY, 1) < 0, -(__LINE__), "Wait for smi not busy timeout.\n");

    mr.reg      = 0;
    mr.data     = *data;
    mr.phy_addr = phy;
    mr.reg_addr = reg;
    mr.op_code  = op == OP_RD ? 1 : 0;

    SMI_REG_WR32(smi, SMI_MNG_REG_OFF, &mr.reg);
    if (op == OP_WR)
        goto _EXIT_POINT;

    ERR_URET_COND(smi_poll(smi, POLL_VALID, 0) < 0, -(__LINE__), "Wait for smi read valid timeout.\n");

    SMI_REG_RD32(smi, SMI_MNG_REG_OFF, &mr.reg);

    *data = mr.data;

    uret = 0;
_EXIT_POINT:
 /* pthread_mutex_unlock(&smi->mtx); */
    return uret;
}

static int smi_read(struct smi_dev *smi, uint8_t phy, uint8_t reg, uint16_t *data)
{
    return smi_rdwr(smi, OP_RD, phy, reg, data);
}

static int smi_write(struct smi_dev *smi, uint8_t phy, uint8_t reg, uint16_t *data)
{
    return smi_rdwr(smi, OP_WR, phy, reg, data);
}

static struct smi_drv _driver = {
    .ctrl  = smi_ctrl,
    .read  = smi_read,
    .write = smi_write,
};

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
int smi_dev_create(struct smi_dev **smi, uint64_t paddr, uint64_t size, char *name)
{
    int uret = 0;

    LOG_DBG("Enter '%s'\n", __func__);
    ERR_RET_COND((!smi || *smi || paddr == 0 || size != SMI_REG_SPACE_SIZE || !name),
                -(__LINE__), "Invalid argument.\n");

    *smi = (struct smi_dev *)malloc(sizeof(struct smi_dev));
    ERR_RET_COND(!*smi, -(__LINE__), "Malloc failed\n");
    memset(*smi, 0, sizeof(struct smi_dev));

    (*smi)->paddr = paddr;
    (*smi)->size  = size;
    strncpy((*smi)->name, name, sizeof((*smi)->name) - 1);

    ERR_URET_COND(0 != pthread_mutex_init(&(*smi)->mtx, 0), -(__LINE__), "Mutex init failed.\n");
    (*smi)->vaddr = mmap_reg_space(paddr, size);
    ERR_URET_COND(MAP_FAILED == (*smi)->vaddr, -(__LINE__), "Map reg space failed.\n");

    (*smi)->drv = &_driver;

    uret = 0;
_EXIT_POINT:
    if (uret < 0) {
        LOG_DBG("%d:'%s()' failed\n", __LINE__, __FILE__);
        if (*smi) {
            pthread_mutex_destroy(&(*smi)->mtx);

            if ((*smi)->vaddr != MAP_FAILED)
                munmap_reg_space((*smi)->vaddr, paddr, size);

            free(*smi);
            *smi = NULL;
        }
    }
    return uret;
}

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
int smi_dev_free(struct smi_dev *smi)
{
    ERR_RET_COND(!smi || smi->vaddr == MAP_FAILED, -(__LINE__), "Invalid argument.\n");

    LOG_DBG("Enter '%s'\n", __func__);
    pthread_mutex_destroy(&smi->mtx);

    munmap_reg_space(smi->vaddr, smi->paddr, smi->size);

    free(smi);
    return 0;
}

#ifdef UNIT_TEST
static void usage(char *name)
{
    printf("%s read22  phyaddr reg\n", name);
    printf("%s write22 phyaddr reg value\n", name);
    printf("%s read45  phyaddr dev reg\n", name);
    printf("%s write45 phyaddr dev reg value\n", name);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    struct smi_dev *smi = NULL;

    uint8_t phy;
    uint8_t dev;
    uint8_t reg;
    uint16_t tmp;
    uint16_t data;
    unsigned int val = 0;

    hr_commn_util_log_lvl_set();

    ret = smi_dev_create(&smi, 0xf212a200, SMI_REG_SPACE_SIZE, "smi-a");
    ERR_RET_COND(ret < 0, -(__LINE__), "Create smi dev failed:%d\n", ret);

    if (argc >= 2) {
        if (strcmp(argv[1], "read22") == 0 && argc == 4) { //read
            sscanf(argv[2], "%u", &val);
            phy = val;
            sscanf(argv[3], "%u", &val);
            reg = val;

            smi->drv->read(smi, phy, reg, &data);
            printf("phy-0x%-2x reg-%-4u:%u(%x)\n", phy, reg, data, data);
        } else if (strcmp(argv[1], "write22") == 0 && argc == 5) { //write
            sscanf(argv[2], "%u", &val);
            phy = val;
            sscanf(argv[3], "%u", &val);
            reg = val;
            sscanf(argv[4], "%u", &val);
            data= val;
            smi->drv->write(smi, phy, reg, &data);
        } else if (strcmp(argv[1], "read45") == 0 && argc == 5) {
            sscanf(argv[2], "%u", &val);
            phy = val;
            sscanf(argv[3], "%u", &val);
            dev = val;
            sscanf(argv[4], "%u", &val);
            reg = val;

            tmp = dev;
            smi->drv->write(smi, phy, 13, &tmp);
            tmp = reg;
            smi->drv->write(smi, phy, 14, &tmp);
            tmp = (dev & (~(3 << 15))) | (1 << 15);
            smi->drv->write(smi, phy, 13, &tmp);
            smi->drv->read (smi, phy, 14, &data);
            printf("phy-0x%-2x devi-%-4u reg-%-4u:%u(%x)\n", phy, dev, reg, data, data);
        } else if (strcmp(argv[1], "write45") == 0 && argc == 6) {
            sscanf(argv[2], "%u", &val);
            phy = val;
            sscanf(argv[3], "%u", &val);
            dev = val;
            sscanf(argv[4], "%u", &val);
            reg = val;
            sscanf(argv[5], "%u", &val);
            data = val;

            tmp = dev;
            smi->drv->write(smi, phy, 13, &tmp);
            tmp = reg;
            smi->drv->write(smi, phy, 14, &tmp);
            tmp = (dev & (~(3 << 15))) | (1 << 15);
            smi->drv->write(smi, phy, 13, &tmp);
            tmp = data;
            smi->drv->write(smi, phy, 14, &tmp);
        } else if (strcmp(argv[1], "ctrl") == 0) {
            typeof(((struct smi_ctrl_data *)0)->smi_cfg) smi_cfg;
            if (argc == 2) {//get
                smi->drv->ctrl(smi, SMI_CTRL_CMD_GET_CFG, &smi_cfg);
                printf("accel_en     :%u\n", smi_cfg.accel_en     );
                printf("fast_mdc     :%u\n", smi_cfg.fast_mdc     );
                printf("fast_mdc_div :%u\n", smi_cfg.fast_mdc_div );
                printf("out_latency  :%u\n", smi_cfg.out_latency  );
                printf("auto_poll_num:%u\n", smi_cfg.auto_poll_num);
                printf("poll_en      :%u\n", smi_cfg.poll_en      );
                printf("invert_mdc   :%u\n", smi_cfg.invert_mdc   );
            } else if (argc == 3) {//set
                sscanf(argv[2], "%u", (uint32_t*)&smi_cfg);
                smi->drv->ctrl(smi, ~(SMI_CTRL_CMD_GET_CFG), &smi_cfg);
            } else {
                printf("%d:Invalid argument.\n", __LINE__);
                usage(argv[0]);
            }
        } else {
            printf("%d:Invalid argument.\n", __LINE__);
            usage(argv[0]);
        }
    } else {
        usage(argv[0]);
    }

    ret = smi_dev_free(smi);
    ERR_RET_COND(ret < 0, -(__LINE__), "Free smi dev failed:%d\n", ret);
    return 0;
}
#endif

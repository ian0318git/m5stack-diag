/* $Id: switzer_fpga.c,v 1.4 2021/04/12 13:37:34 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_fpga.c,v $
 *------------------------------------------------------------------
 *
 * switzer_fpga.c - Switzer FPGA driver interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "byteswap.h"
#include "switzer_priv.h"
#include "switzer_common.h"
#include "switzer_fpga.h"
#include "csrs/nim_te_csrs_top.h"
#include "csrs/sm_csrs_top.h"

/* ****************************** MDIO ****************************** */

#define SWITZER_MDIO_TIMEOUT        100 /* ms */

#define SWITZER_MDIO_OP_ADDRESS     0
#define SWITZER_MDIO_OP_WRITE       1
#define SWITZER_MDIO_OP_READ        3
#define SWITZER_MDIO_OP_READ_INC    2 /* post read increment address */

#define SWITZER_MDIO_ERR_UNKNOWN    1
#define SWITZER_MDIO_ERR_TIMEOUT    2

struct switzer_mdio {
    struct phy_csrs *csrs;
};

struct switzer_mdio *switzer_mdio_probe(struct switzer_mdio_settings *settings)
{
    struct switzer_mdio *mdio;

    if (!(mdio = malloc(sizeof(*mdio))))
        return NULL;

    mdio->csrs = settings->base;
    /* use lower clock for MIURA 1.8 used which has an compatibility problem
     * - BCM82757 Merlin status poll timeout caused by high MDIO clock
     * during firmware download */
    PHY_CSRS__PHY_MDC_REG__CLK_DIVIDE__MODIFY(mdio->csrs->phy_mdc, 1); /* 3.9 MHz */
    return mdio;
}

void switzer_mdio_remove(struct switzer_mdio *mdio)
{
    free(mdio);
}

static int switzer_mdio_xfer(struct switzer_mdio *mdio, uint8_t op,
                             uint8_t prtad, uint8_t devad, uint16_t *data)
{
    struct phy_csrs *csrs = mdio->csrs;
    uint32_t reg = 0;
    int rw = ((op == SWITZER_MDIO_OP_ADDRESS) ||
              (op == SWITZER_MDIO_OP_WRITE));
    int err;
    long expires;

    PHY_CSRS__PHY_MDIO_REG__REG_ADDR__MODIFY(reg, devad);
    PHY_CSRS__PHY_MDIO_REG__PHY_ADDR__MODIFY(reg, prtad);
    PHY_CSRS__PHY_MDIO_REG__OP__MODIFY(reg, op);
    PHY_CSRS__PHY_MDIO_REG__GO__SET(reg);

    if (rw)                     /* TX */
        PHY_CSRS__PHY_MDIO_REG__DATA__MODIFY(reg, *data);

    /* go */
    csrs->phy_mdio = reg;

    /* check until timeout */
    expires = switzer_ticks + SWITZER_MDIO_TIMEOUT;
    while (1) {
        reg = csrs->phy_mdio;
        if (!PHY_CSRS__PHY_MDIO_REG__GO__READ(reg))
            break;
        if (expires - switzer_ticks < 0)
            break;
    }

    err = PHY_CSRS__PHY_MDIO_REG__ERR__READ(reg);
    if (err == SWITZER_MDIO_ERR_UNKNOWN) {
        log_err("switzer mdio error during transaction\n");
        return -EIO;
    } else if (err == SWITZER_MDIO_ERR_TIMEOUT) {
        log_err("switzer mdio timeout error\n");
        return -EIO;
    } else if (PHY_CSRS__PHY_MDIO_REG__GO__READ(reg)) {
        log_err("switzer mdio transaction timeout\n");
        return -EBUSY;
    }

    if (!rw)                    /* RX */
        *data = PHY_CSRS__PHY_MDIO_REG__DATA__READ(reg);

    return 0;
}

int switzer_mdio_read(struct switzer_mdio *mdio,
                      uint8_t prtad, uint8_t devad,
                      uint16_t addr, uint16_t *data)
{
    uint8_t op;
    int rc;

    op = SWITZER_MDIO_OP_ADDRESS;
    if ((rc = switzer_mdio_xfer(mdio, op, prtad, devad, &addr)))
        return rc;
    op = SWITZER_MDIO_OP_READ;
    return switzer_mdio_xfer(mdio, op, prtad, devad, data);
}

int switzer_mdio_write(struct switzer_mdio *mdio,
                       uint8_t prtad, uint8_t devad,
                       uint16_t addr, uint16_t data)
{
    uint8_t op;
    int rc;

    op = SWITZER_MDIO_OP_ADDRESS;
    if ((rc = switzer_mdio_xfer(mdio, op, prtad, devad, &addr)))
        return rc;
    op = SWITZER_MDIO_OP_WRITE;
    return switzer_mdio_xfer(mdio, op, prtad, devad, &data);
}

/* ****************************** I2C ****************************** */

#define SWITZER_I2C_TIMEOUT     100 /* ms */
#define SWITZER_I2C_FIFO_SIZE   512

struct switzer_i2c_master {
    struct sfp_i2c_csrs *csrs;
};

struct switzer_i2c_slave {
    struct switzer_i2c_master *master;
    uint8_t addr;
    uint8_t port;
    uint8_t freq;
    switzer_i2c_xfer_t xfer;
};

struct switzer_i2c_master *
switzer_i2c_master_probe(struct switzer_i2c_master_settings *settings)
{
    struct switzer_i2c_master *master;

    if (!(master = malloc(sizeof(*master))))
        return NULL;

    master->csrs = settings->base;
    return master;
}

void switzer_i2c_master_remove(struct switzer_i2c_master *master)
{
    free(master);
}

struct switzer_i2c_slave *
switzer_i2c_slave_probe(struct switzer_i2c_slave_settings *settings)
{
    struct switzer_i2c_slave *slave;

    if (!(slave = malloc(sizeof(*slave))))
        return NULL;

    slave->master = settings->master;
    slave->addr = settings->addr;
    slave->port = settings->port;

    if (!settings->freq)
        settings->freq = 100;   /* KHz */
    slave->freq = 62500 / (4 * settings->freq) - 1;

    if (settings->xfer) {
        slave->xfer = settings->xfer;
    } else {
        slave->xfer = SWITZER_I2C_XFER_BYTES;
    }

    return slave;
}

void switzer_i2c_slave_remove(struct switzer_i2c_slave *slave)
{
    free(slave);
}

static int switzer_i2c_slave_pre_xfer(struct switzer_i2c_slave *slave,
                                      switzer_i2c_xfer_t xfer)
{
    struct sfp_i2c_csrs *csrs = slave->master->csrs;
    uint32_t ctrl = csrs->sfp_i2c_ctrl;
    uint32_t cmd = csrs->sfp_i2c_cmd;

    switch (xfer) {
    case SWITZER_I2C_XFER_BYTES:
    case SWITZER_I2C_XFER_BLOCK_BYTES:
        break;
    default:
        log_err("switzer i2c unsupported xfer %d\n", xfer);
        return -1;
    }

    if (!(cmd & SFP_I2C_CSRS__SFP_I2C_CMD_REG__DONE__MASK)) {
        log_err("switzer i2c busy\n");
        return -1;
    }

    if (!(ctrl & SFP_I2C_CSRS__SFP_I2C_CTRL_REG__RD_FIFO_EMPTY__MASK) ||
        !(ctrl & SFP_I2C_CSRS__SFP_I2C_CTRL_REG__WR_FIFO_EMPTY__MASK)) {
        log_warn("switzer i2c fifo not empty %08x\n", ctrl);
        SFP_I2C_CSRS__SFP_I2C_CTRL_REG__FIFO_FLUSH__CLR(csrs->sfp_i2c_ctrl);
    }

    return 0;
}

static int switzer_i2c_slave_post_xfer(struct switzer_i2c_slave *slave, int rw)
{
    struct sfp_i2c_csrs *csrs = slave->master->csrs;
    const char *action = rw ? "write" : "read";
    uint32_t cmd;
    long expires;

    /* check until timeout */
    expires = switzer_ticks + SWITZER_MDIO_TIMEOUT;
    while (1) {
        cmd = csrs->sfp_i2c_cmd;
        if ((cmd & SFP_I2C_CSRS__SFP_I2C_CMD_REG__NACK__MASK) ||
            (cmd & SFP_I2C_CSRS__SFP_I2C_CMD_REG__DONE__MASK))
            break;
        if (expires - switzer_ticks < 0)
            break;
        switzer_udelay(100);
    }

    if (cmd & SFP_I2C_CSRS__SFP_I2C_CMD_REG__NACK__MASK) {
        log_err("switzer i2c %s nack error\n", action);
        return -ENXIO;
    } else if (cmd & SFP_I2C_CSRS__SFP_I2C_CMD_REG__DONE__MASK)
        return 0;
    log_err("switzer i2c %s timeout\n", action);
    return -EBUSY;
}

static ssize_t switzer_i2c_slave_xfer(struct switzer_i2c_slave *slave, int rw,
                                      switzer_i2c_xfer_t xfer, uint8_t addr,
                                      uint8_t subaddr, void *buf, size_t count)
{
    struct sfp_i2c_csrs *csrs = slave->master->csrs;
    uint32_t ctrl = 0, cmd = 0;
    ssize_t sz, fifo_size, block_size;
    int i, block = (xfer == SWITZER_I2C_XFER_BLOCK_BYTES);;

    if ((sz = switzer_i2c_slave_pre_xfer(slave, xfer)) < 0)
        return sz;

    fifo_size = SWITZER_I2C_FIFO_SIZE;
    if (block)
        fifo_size--;
    if (count > fifo_size)
        count = fifo_size;

    sz = count;
    if (block)
        sz++;
    if (rw) {                   /* TX */
        if (block)
            csrs->sfp_i2c_dataLo = (sz & 0xff);
        for (i = 0; i < count; i++)
            csrs->sfp_i2c_dataLo = *(uint8_t *)(buf + i);
    }
    SFP_I2C_CSRS__SFP_I2C_CTRL_REG__FIFO_BYTE_LEN__MODIFY(ctrl, sz - 1);
    SFP_I2C_CSRS__SFP_I2C_CTRL_REG__PORT_SELECT__MODIFY(ctrl, slave->port);
    SFP_I2C_CSRS__SFP_I2C_CTRL_REG__FIFO_CTL_MODE__SET(ctrl);
    csrs->sfp_i2c_ctrl = ctrl;

    /* trigger */
    SFP_I2C_CSRS__SFP_I2C_CMD_REG__RD_WR_N__MODIFY(cmd, rw ? 0 : 1);
    SFP_I2C_CSRS__SFP_I2C_CMD_REG__DEVICE_ADDR__MODIFY(cmd, addr);
    SFP_I2C_CSRS__SFP_I2C_CMD_REG__SUB_ADDR__MODIFY(cmd, subaddr);
    SFP_I2C_CSRS__SFP_I2C_CMD_REG__MODE__MODIFY(cmd, 1);
    SFP_I2C_CSRS__SFP_I2C_CMD_REG__FREQ_SEL__MODIFY(cmd, slave->freq);
    csrs->sfp_i2c_cmd = cmd;

    if ((sz = switzer_i2c_slave_post_xfer(slave, rw)) < 0)
        return sz;

    sz = count;
    if (!rw) {                  /* RX */
        if (block) {
            block_size = (csrs->sfp_i2c_dataLo & 0xff);
            if (sz > block_size)
                sz = block_size;
        }
        for (i = 0; i < sz; i++) {
            if (csrs->sfp_i2c_ctrl &
                SFP_I2C_CSRS__SFP_I2C_CTRL_REG__RD_FIFO_EMPTY__MASK)
                break;
            *(uint8_t *)(buf + i) = (csrs->sfp_i2c_dataLo & 0xff);
        }
        sz = i;
    }

    return sz;
}

ssize_t __switzer_i2c_slave_read(struct switzer_i2c_slave *slave,
                                 switzer_i2c_xfer_t xfer, uint8_t addr,
                                 uint8_t cmd, void *buf, size_t count)
{
    return switzer_i2c_slave_xfer(slave, 0, xfer, addr,
                                  cmd, buf, count);
}

ssize_t __switzer_i2c_slave_write(struct switzer_i2c_slave *slave,
                                  switzer_i2c_xfer_t xfer, uint8_t addr,
                                  uint8_t cmd, const void *buf, size_t count)
{
    return switzer_i2c_slave_xfer(slave, 1, xfer, addr,
                                  cmd, (void *)buf, count);
}

ssize_t switzer_i2c_slave_read(struct switzer_i2c_slave *slave,
                               uint8_t cmd, void *buf, size_t count)
{
    return switzer_i2c_slave_xfer(slave, 0, slave->xfer, slave->addr,
                                  cmd, buf, count);
}

ssize_t switzer_i2c_slave_write(struct switzer_i2c_slave *slave,
                                uint8_t cmd, const void *buf, size_t count)
{
    return switzer_i2c_slave_xfer(slave, 1, slave->xfer, slave->addr,
                                  cmd, (void *)buf, count);
}

uint8_t switzer_i2c_slave_addr(struct switzer_i2c_slave *slave)
{
    return slave->addr;
}

/* ****************************** DASH I2C ******************************
   ***********************Use DASH FPGAI2C control*********************** */

struct switzer_dash_i2c_master {
    struct switzer_i2c_ctrl_t *csrs;
};

struct switzer_dash_i2c_slave {
    struct switzer_dash_i2c_master *master;
    uint32_t      addr;
    uint32_t      sub_addr_len;   /* I2C Sub address lenth (0-3) */
    uint32_t      sub_addr;       /* I2C Sub address */
    uint8_t       mux;            /* Mux number that I2C device connected to */
    uint8_t       i2c_speed;
    uint8_t       addr_extended;  /* Addr extended mode 1 - 10 bits addr, 0 - 7 bits addr */
};

#define SCL_DRIVE_TIMES   100

struct switzer_dash_i2c_master *
switzer_dash_i2c_master_probe(struct switzer_dash_i2c_master_settings *settings)
{
    struct switzer_dash_i2c_master *master;

    if (!(master = malloc(sizeof(*master))))
        return NULL;

    master->csrs = settings->base;
    return master;
}

void switzer_dash_i2c_master_remove(struct switzer_dash_i2c_master *master)
{
    free(master);
}

struct switzer_dash_i2c_slave *
switzer_dash_i2c_slave_probe(struct switzer_dash_i2c_slave_settings *settings)
{
    struct switzer_dash_i2c_slave *slave;

    if (!(slave = malloc(sizeof(*slave))))
        return NULL;

    slave->master        = settings->master;
    slave->addr          = settings->addr;
    slave->sub_addr_len  = settings->sub_addr_len;
    slave->sub_addr      = settings->sub_addr;
    slave->mux           = settings->mux;
    slave->addr_extended = settings->addr_extended;

    if (settings->i2c_speed > SWITZER_DASH_I2C_400K)
        slave->i2c_speed = SWITZER_DASH_I2C_400K;   /* 400KHz */
    else
        slave->i2c_speed = settings->i2c_speed;

    return slave;
}

void switzer_dash_i2c_slave_remove(struct switzer_dash_i2c_slave *slave)
{
    free(slave);
}

/**********************************************************************
 *
 * Function: switzer_dash_i2c_reset
 *
 * Description: Reset an I2C master module
 *
 * Input: i2c - pointer to switzer_dash_i2c_slave
 *
 * Output: PASSED or FAILED
 */
static void switzer_dash_i2c_reset(struct switzer_dash_i2c_slave *slave)
{
    struct switzer_i2c_ctrl_t *csrs = slave->master->csrs;
    int ctr = 0;

    csrs->ctrl |= SWITZER_DASH_I2C_CTRL_SOFT_RESET;
    switzer_udelay(1000);

    /* goes into bitbang mode */
    csrs->ctrl |= SWITZER_DASH_I2C_CTRL_BITBANG;

    /* drives the SDA lines low */
    csrs->bit_bang &= ~(SWITZER_DASH_I2C_BITBANG_SDA_DRIVER);

    /* keeps driving SCL until it recovers */
    for (ctr = 0; ctr < SCL_DRIVE_TIMES; ctr++) {
       csrs->bit_bang &= ~(SWITZER_DASH_I2C_BITBANG_SCL_DRIVER);
       switzer_mdelay(1);
       csrs->bit_bang |= SWITZER_DASH_I2C_BITBANG_SCL_DRIVER;
       switzer_mdelay(1);
    }

    /* drives the SDA lines High */
    csrs->bit_bang |= SWITZER_DASH_I2C_BITBANG_SDA_DRIVER;

    /* leave bitbang mode */
    csrs->ctrl &= ~(SWITZER_DASH_I2C_CTRL_BITBANG);
}

/**********************************************************************
 *
 * Function: switzer_dash_i2c_slave_pre_xfer
 *
 * Description: Check if the i2c master is idle
 *
 * Input: i2c - pointer to switzer_dash_i2c_slave
 *
 * Output: PASSED or FAILED
 *
 */
static int switzer_dash_i2c_slave_pre_xfer(struct switzer_dash_i2c_slave *slave)
{
    struct switzer_i2c_ctrl_t *csrs = slave->master->csrs;
    uint32_t i, timeout_val, retry;

    timeout_val = 30;
    for (retry = 0; retry < 5; retry++) {
        for (i=0; i < timeout_val; i++) {
            if (csrs->stat & SWITZER_DASH_I2C_STAT_NOT_ACTIVE) {
                return 0;
            }
            switzer_mdelay(10);
        }
        switzer_dash_i2c_reset(slave);
    }
    log_err("switzer i2c busy\n");
    return -EBUSY;
}

static int switzer_dash_i2c_slave_post_xfer(struct switzer_dash_i2c_slave *slave, int rw,
                                            uint8_t mux, uint32_t addr, uint32_t sub_addr_sz,
                                            uint32_t subaddr, size_t count)
{
    struct switzer_i2c_ctrl_t *csrs = slave->master->csrs;
    uint32_t reg_val, i, timeout_val;
    const char *action = rw ? "write" : "read";

    if (mux >= 4) {
        log_err("mu3;.cxx has to be less than 4\n");
        return -1;
    }

    reg_val = csrs->ctrl;
    reg_val |= (count << SWITZER_DASH_I2C_L_SHFT_CTRL_BYTE_LEN);

    if (rw == SWITZER_I2C_WRITE)
        reg_val |= SWITZER_DASH_I2C_CTRL_WR_MODE;
    else
        reg_val |= SWITZER_DASH_I2C_CTRL_RD_MODE;

    csrs->sla_addr = addr;
    if (sub_addr_sz != 0) {
        csrs->sla_sub_addr = subaddr;
    }
    csrs->ctrl = (reg_val | SWITZER_DASH_I2C_CTRL_NORMAL |
                 ((sub_addr_sz) << SWITZER_DASH_I2C_L_SHFT_CTRL_SUB_ADDR_EN));

    /* give time for device to send acknowlegement..especiall when talking to quack */
    /* wait = (data_len * 10); */ /* defined but not used, removed. */
    /*10 byte address @ 100Khz */
    switzer_mdelay(3);

    /* Monitor the done bit in status register. Add 10 satety bytes for
     * wait time calculation due to I2C protocol is slow and have gaps
     */
    timeout_val = 500;

    /* Wait one byte time to let the i2c op to start before polling status */
    /* if no delay we might miss the no ack */
    for (i=0; i <= timeout_val; i++) {
        reg_val = csrs->stat;
        if (reg_val & SWITZER_DASH_I2C_STAT_NO_SLV) { /*check bit 2*/
            log_err("switzer i2c %s nack error\n", action);
            return -ENXIO;
        }

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & SWITZER_DASH_I2C_STAT_STD_DONE) != 0) {
	        break;
	    }
        switzer_mdelay(8);
    }
    if (i > timeout_val) {
        log_err("switzer i2c %s timeout\n", action);
        return -EBUSY;
    }

    return 0;
}

static ssize_t switzer_dash_i2c_slave_xfer(struct switzer_dash_i2c_slave *slave,
                                           int rw, uint8_t mux,
                                           uint32_t addr, uint8_t addr_extended,
                                           uint32_t sub_addr_sz, uint32_t sub_addr,
                                           uint8_t i2c_speed, void *cmd, uint8_t cmd_len,
                                           void *buf, size_t count)
{
    struct switzer_i2c_ctrl_t *csrs = slave->master->csrs;
    ssize_t sz;
    size_t i, dword_count;
    uint32_t *tmp_buf, reg_val;
    char cbuf[BUF_SIZE];
    tmp_buf = (uint32_t *)cbuf;
    static const ssize_t int32_sz = sizeof(uint32_t);

    if ((sz = switzer_dash_i2c_slave_pre_xfer(slave)) < 0) {
        return sz;
    }

    /* Set up control register */
    reg_val = SWITZER_DASH_I2C_CTRL_CLK_50 |
              (i2c_speed << SWITZER_DASH_I2C_L_SHFT_CTRL_SPEED) |
              (mux << SWITZER_DASH_I2C_L_SHFT_CTRL_MUX);
    reg_val = (addr_extended ? SWITZER_DASH_I2C_CTRL_SLV_ADDR_10 :
                               SWITZER_DASH_I2C_CTRL_SLV_ADDR_7);
    csrs->ctrl = reg_val;

    /* I2C control in DASH FPGA use 32bits Data FIFO Register, */
    /* so we need to convert byte's count to dword's, and */
    /* convert the endianness from host to little endian */
    dword_count = count / int32_sz;
    dword_count += (count % int32_sz) ? 1 : 0;

    if (rw == SWITZER_I2C_WRITE) {
        /* Write */
        sz = 0;
        if (cmd_len > 0) {
            memcpy(cbuf, (char *)cmd, cmd_len);
            sz = cmd_len;
            dword_count = (count + sz) / int32_sz;
            dword_count += ((count + sz) % int32_sz) ? 1 : 0;
        }
        memcpy(cbuf + sz, buf, count);
        for (i = 0; i < dword_count; i++) {
            csrs->data_fifo = dswap4(*(tmp_buf + i));
        }

        if ((sz = switzer_dash_i2c_slave_post_xfer(slave, rw, mux,
                                                   addr, sub_addr_sz,
                                                   sub_addr, count + sz)) < 0) {
            return sz;
        }
        sz = count;
    } else {
        /* Read */
        if (cmd_len > 0) {
            /* Send command first */
            memcpy(cbuf, (char *)cmd, cmd_len);
            sz = cmd_len / int32_sz + (cmd_len % int32_sz) ? 1 : 0;
            for (i = 0; i < sz; i++) {
                csrs->data_fifo = dswap4(*(tmp_buf + i));
            }

            if ((sz = switzer_dash_i2c_slave_post_xfer(slave, SWITZER_I2C_WRITE, mux, addr,
                                                       sub_addr_sz, sub_addr, cmd_len)) < 0)
                return sz;
        }

        if ((sz = switzer_dash_i2c_slave_post_xfer(slave, rw, mux, addr,
                                                   sub_addr_sz, sub_addr, count)) < 0)
            return sz;

        for (i = 0; i < dword_count; i++) {
            if (csrs->stat & SWITZER_DASH_I2C_STAT_FIFO_UNDER)
                break;
            reg_val = csrs->data_fifo;
            *(tmp_buf + i) = dswap4(reg_val);
        }
        i *= int32_sz;
        sz = count < i ? count : i;
        memcpy(buf, tmp_buf, sz);
    }

    return sz;
}

ssize_t __switzer_dash_i2c_slave_read(struct switzer_dash_i2c_slave *slave,
                                      uint8_t mux, uint32_t addr, uint8_t addr_extended,
                                      uint32_t sub_addr_sz, uint32_t sub_addr,
                                      uint8_t i2c_speed, void *cmd, uint8_t cmd_len,
                                      void *buf, size_t count)
{
    return switzer_dash_i2c_slave_xfer(slave, SWITZER_I2C_READ, mux, addr,
                                       addr_extended, sub_addr_sz, sub_addr,
                                       i2c_speed, cmd, cmd_len, buf, count);
}

ssize_t __switzer_dash_i2c_slave_write(struct switzer_dash_i2c_slave *slave,
                                       uint8_t mux, uint32_t addr, uint8_t addr_extended,
                                       uint32_t sub_addr_sz, uint32_t sub_addr,
                                       uint8_t i2c_speed, void *cmd, uint8_t cmd_len,
                                       const void *buf, size_t count)
{
    return switzer_dash_i2c_slave_xfer(slave, SWITZER_I2C_WRITE, mux, addr,
                                       addr_extended, sub_addr_sz, sub_addr,
                                       i2c_speed, cmd, cmd_len, (void *)buf, count);
}

ssize_t switzer_dash_i2c_slave_read(struct switzer_dash_i2c_slave *slave,
                                    uint8_t cmd, void *buf, size_t count)
{
    return switzer_dash_i2c_slave_xfer(slave, SWITZER_I2C_READ, slave->mux,
                                       slave->addr, SWITZER_DASH_I2C_NOMAL_ADDR,
                                       slave->sub_addr_len, slave->sub_addr, slave->i2c_speed,
                                       (void *)&cmd, sizeof(cmd), buf, count);
}

ssize_t switzer_dash_i2c_slave_write(struct switzer_dash_i2c_slave *slave,
                                     uint8_t cmd, const void *buf, size_t count)
{
    return switzer_dash_i2c_slave_xfer(slave, SWITZER_I2C_WRITE, slave->mux,
                                       slave->addr, SWITZER_DASH_I2C_NOMAL_ADDR,
                                       slave->sub_addr_len, slave->sub_addr, slave->i2c_speed,
                                       (void *)&cmd, sizeof(cmd), (void *)buf, count);
}

uint32_t switzer_dash_i2c_slave_addr(struct switzer_dash_i2c_slave *slave)
{
    return slave->addr;
}

uint8_t switzer_dash_i2c_slave_mux(struct switzer_dash_i2c_slave *slave)
{
    return slave->mux;
}

uint8_t switzer_dash_i2c_slave_sleep(struct switzer_dash_i2c_slave *slave)
{
    return slave->i2c_speed;
}

uint32_t switzer_dash_i2c_slave_sub_addr(struct switzer_dash_i2c_slave *slave)
{
    return slave->sub_addr;
}

uint32_t switzer_dash_i2c_slave_sub_addr_len(struct switzer_dash_i2c_slave *slave)
{
    return slave->sub_addr_len;
}

uint8_t switzer_dash_i2c_slave_addr_extended(struct switzer_dash_i2c_slave *slave)
{
    return slave->addr_extended;
}

/* ****************************** SPI Prom ****************************** */

#define SWITZER_SPI_PROM_TIMEOUT        500 /* ms */
#define SWITZER_SPI_PROM_FIFO_SIZE      256

#define SWITZER_SPI_PROM_PAGE_SIZE      0x100   /* 256 */
#define SWITZER_SPI_PROM_SECTOR_SIZE    0x10000 /* 64K */

#define SWITZER_SPI_PROM_OP_WRSR        0x01 /* write status register */
#define SWITZER_SPI_PROM_OP_PP          0x02 /* page program */
#define SWITZER_SPI_PROM_OP_READ        0x03 /* read data */
#define SWITZER_SPI_PROM_OP_WRDI        0x04 /* write disable */
#define SWITZER_SPI_PROM_OP_RDSR        0x05 /* read status register */
#define SWITZER_SPI_PROM_OP_WREN        0x06 /* write enable */
#define SWITZER_SPI_PROM_OP_SE          0x20 /* 4K sector erase */
#define SWITZER_SPI_PROM_OP_RDID        0x9F /* read identification */
#define SWITZER_SPI_PROM_OP_BE          0xD8 /* 64K sector erase */

#define SWITZER_SPI_PROM_OP_RDSR_WIP    1 /* write in progress */

struct switzer_spi_prom {
    struct spi_core_csrs *csrs;
};

struct switzer_spi_prom *
switzer_spi_prom_probe(struct switzer_spi_prom_settings *settings)
{
    struct switzer_spi_prom *prom;

    if (!(prom = malloc(sizeof(*prom))))
        return NULL;

    prom->csrs = settings->base;
    return prom;
}

void switzer_spi_prom_remove(struct switzer_spi_prom *prom)
{
    free(prom);
}

static int switzer_spi_prom_pre_xfer(struct switzer_spi_prom *prom)
{
    struct spi_core_csrs *csrs = prom->csrs;
    uint32_t status = csrs->spi_status;

    if (status & SPI_CORE_CSRS__SPI_STATUS_REG__BUSY__MASK) {
        log_err("switzer spi prom busy\n");
        return -1;
    }
    csrs->spi_status = status;  /* WOCLR */
    return 0;
}

static int switzer_spi_prom_post_xfer(struct switzer_spi_prom *prom)
{
    struct spi_core_csrs *csrs = prom->csrs;
    uint32_t status;
    long expires;

    /* check until timeout */
    expires = switzer_ticks + SWITZER_SPI_PROM_TIMEOUT;
    while (1) {
        status = csrs->spi_status;
        if (!(status & SPI_CORE_CSRS__SPI_STATUS_REG__BUSY__MASK) &&
            (status & SPI_CORE_CSRS__SPI_STATUS_REG__DONE__MASK))
            break;
        if (expires - switzer_ticks < 0)
            break;
        switzer_udelay(100);
    }

    if (status & SPI_CORE_CSRS__SPI_STATUS_REG__BUSY__MASK) {
        log_err("switzer spi prom busy\n");
        return -EBUSY;
    } else if (status & SPI_CORE_CSRS__SPI_STATUS_REG__DONE__MASK)
        return 0;
    log_err("switzer spi prom timeout\n");
    return -EBUSY;
}

static void switzer_spi_prom_xfer_trigger(struct switzer_spi_prom *prom,
                                          uint8_t op, uint32_t addr)
{
    static const struct {
        uint8_t opcode:1;
        uint8_t dummy:1;
        uint8_t dir:1;
        uint8_t addr:5;
    } maps[256] = {
        [SWITZER_SPI_PROM_OP_WRSR] = {1, 0, 1, 0},
        [SWITZER_SPI_PROM_OP_PP]   = {1, 0, 1, 3},
        [SWITZER_SPI_PROM_OP_READ] = {1, 0, 0, 3},
        [SWITZER_SPI_PROM_OP_WRDI] = {1, 0, 1, 0},
        [SWITZER_SPI_PROM_OP_RDSR] = {1, 0, 0, 0},
        [SWITZER_SPI_PROM_OP_WREN] = {1, 0, 1, 0},
        [SWITZER_SPI_PROM_OP_SE]   = {1, 0, 1, 3},
        [SWITZER_SPI_PROM_OP_RDID] = {1, 0, 0, 0},
        [SWITZER_SPI_PROM_OP_BE]   = {1, 0, 1, 3},
    };
    struct spi_core_csrs *csrs = prom->csrs;
    uint32_t control, addr_op = 0;

    SPI_CORE_CSRS__SPI_ADDR_OP_REG__ADDRESS__MODIFY(addr_op, addr);
    SPI_CORE_CSRS__SPI_ADDR_OP_REG__OPCODE__MODIFY(addr_op, op);
    csrs->spi_addr_op = addr_op;

    control = csrs->spi_control;
    if (maps[op].addr) {
        SPI_CORE_CSRS__SPI_CONTROL_REG__USE_ADDR__SET(control);
        SPI_CORE_CSRS__SPI_CONTROL_REG__ADDR_SIZE__MODIFY(control,
                                                          maps[op].addr - 1);
    } else {
        SPI_CORE_CSRS__SPI_CONTROL_REG__USE_ADDR__CLR(control);
    }
    SPI_CORE_CSRS__SPI_CONTROL_REG__DATA_DIR__MODIFY(control, maps[op].dir);
    if (maps[op].dummy)
        SPI_CORE_CSRS__SPI_CONTROL_REG__USE_DUMMY__SET(control);
    else
        SPI_CORE_CSRS__SPI_CONTROL_REG__USE_DUMMY__CLR(control);
    if (maps[op].opcode)
        SPI_CORE_CSRS__SPI_CONTROL_REG__USE_OPCODE__SET(control);
    else
        SPI_CORE_CSRS__SPI_CONTROL_REG__USE_OPCODE__CLR(control);
    csrs->spi_control = control;
}

static int switzer_spi_prom_write_enable(struct switzer_spi_prom *prom)
{
    int rc;

    if ((rc = switzer_spi_prom_pre_xfer(prom)) < 0)
        return rc;
    switzer_spi_prom_xfer_trigger(prom, SWITZER_SPI_PROM_OP_WREN, 0);
    if ((rc = switzer_spi_prom_post_xfer(prom)) < 0)
        return rc;
    return rc;
}

int switzer_spi_prom_read_status(struct switzer_spi_prom *prom, uint8_t *status)
{
    struct spi_core_csrs *csrs = prom->csrs;
    int rc;

    if ((rc = switzer_spi_prom_pre_xfer(prom)) < 0)
        return rc;
    switzer_spi_prom_xfer_trigger(prom, SWITZER_SPI_PROM_OP_RDSR, 0);
    if ((rc = switzer_spi_prom_post_xfer(prom)) < 0)
        return rc;
    *status = csrs->spi_data;
    return rc;
}

int switzer_spi_prom_write_status(struct switzer_spi_prom *prom, uint8_t status)
{
    struct spi_core_csrs *csrs = prom->csrs;
    int rc;

    if ((rc = switzer_spi_prom_write_enable(prom)) < 0)
        return rc;

    if ((rc = switzer_spi_prom_pre_xfer(prom)) < 0)
        return rc;
    csrs->spi_data = status;
    switzer_spi_prom_xfer_trigger(prom, SWITZER_SPI_PROM_OP_WRSR, 0);
    if ((rc = switzer_spi_prom_post_xfer(prom)) < 0)
        return rc;
    return rc;
}

static ssize_t __switzer_spi_prom_read(struct switzer_spi_prom *prom,
                                       uint32_t addr, void *buf, size_t count)
{
    struct spi_core_csrs *csrs = prom->csrs;
    ssize_t sz;

    if ((sz = switzer_spi_prom_pre_xfer(prom)) < 0)
        return sz;

    SPI_CORE_CSRS__SPI_RDSIZE_REG__RDSIZE__MODIFY(csrs->spi_rdsize, count - 1);

    switzer_spi_prom_xfer_trigger(prom, SWITZER_SPI_PROM_OP_READ, addr);

    if ((sz = switzer_spi_prom_post_xfer(prom)) < 0)
        return sz;

    for (sz = 0; sz < count; sz++) {
        if (SPI_CORE_CSRS__SPI_STATUS_REG__RD_FIFO_EMPTY__READ(csrs->spi_status))
            break;
        *(uint8_t *)(buf + sz) = (csrs->spi_data & 0xff);
    }
    return sz;
}

ssize_t switzer_spi_prom_read(struct switzer_spi_prom *prom,
                              uint32_t addr, void *buf, size_t count)
{
    ssize_t sz, size;

    for (size = 0; size < count; size += sz) {
        sz = count - size;
        if (sz > SWITZER_SPI_PROM_FIFO_SIZE)
            sz = SWITZER_SPI_PROM_FIFO_SIZE;
        if ((sz = __switzer_spi_prom_read(prom, addr + size,
                                          buf + size, sz)) < 0)
            return sz;
    }
    return size;
}

static int switzer_spi_prom_wip_wait(struct switzer_spi_prom *prom)
{
    int rc;
    long expires;
    uint8_t status;

    expires = switzer_ticks + SWITZER_SPI_PROM_TIMEOUT;
    while (1) {
        if ((rc = switzer_spi_prom_read_status(prom, &status)) < 0)
            return rc;
        if (!(status & SWITZER_SPI_PROM_OP_RDSR_WIP))
            break;
        if (expires - switzer_ticks < 0)
            return -ENXIO;
        switzer_udelay(100);
    }
    return 0;
}

static ssize_t __switzer_spi_prom_write(struct switzer_spi_prom *prom,
                                        uint32_t addr, const void *buf, size_t count)
{
    struct spi_core_csrs *csrs = prom->csrs;
    int rc;
    ssize_t sz;

    if ((rc = switzer_spi_prom_write_enable(prom)) < 0)
        return rc;

    if ((rc = switzer_spi_prom_pre_xfer(prom)) < 0)
        return rc;

    for (sz = 0; sz < count; sz++) {
        if (SPI_CORE_CSRS__SPI_STATUS_REG__WR_FIFO_FULL__READ(csrs->spi_status))
            break;
        csrs->spi_data = *(uint8_t *)(buf + sz);
    }

    switzer_spi_prom_xfer_trigger(prom, SWITZER_SPI_PROM_OP_PP, addr);

    if ((rc = switzer_spi_prom_post_xfer(prom)) < 0)
        return rc;

    if ((rc = switzer_spi_prom_wip_wait(prom)) < 0)
        return rc;
    return sz;
}

ssize_t switzer_spi_prom_write(struct switzer_spi_prom *prom,
                               uint32_t addr, const void *buf, size_t count)
{
    ssize_t sz, size;

    if (addr & (SWITZER_SPI_PROM_PAGE_SIZE - 1))
        log_warn("address %x is not page aligned\n", addr);

    for (size = 0; size < count; size += sz) {
        sz = count - size;
        if (sz > SWITZER_SPI_PROM_FIFO_SIZE)
            sz = SWITZER_SPI_PROM_FIFO_SIZE;
        if ((sz = __switzer_spi_prom_write(prom, addr + size,
                                           buf + size, sz)) < 0)
            return sz;
    }
    return size;
}

ssize_t switzer_spi_prom_erase(struct switzer_spi_prom *prom,
                               uint32_t addr, size_t count)
{
    int rc;
    ssize_t sz;

    if (addr & (SWITZER_SPI_PROM_SECTOR_SIZE - 1))
        log_warn("address %x is not sector aligned\n", addr);

    for (sz = 0; sz < count; sz += SWITZER_SPI_PROM_SECTOR_SIZE) {
        if ((rc = switzer_spi_prom_write_enable(prom)) < 0)
            return rc;
        if ((rc = switzer_spi_prom_pre_xfer(prom)) < 0)
            return rc;
        switzer_spi_prom_xfer_trigger(prom, SWITZER_SPI_PROM_OP_BE, addr + sz);
        if ((rc = switzer_spi_prom_post_xfer(prom)) < 0)
            return rc;
        if ((rc = switzer_spi_prom_wip_wait(prom)) < 0)
            return rc;
    }
    return sz;
}

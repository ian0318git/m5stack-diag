/* $Id: switzer_test.c,v 1.6 2021/04/12 13:37:35 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_test.c,v $
 *------------------------------------------------------------------
 *
 * switzer_test.c - Switzer NGWIC.
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

#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "platform_slot.h"
#include "dash_fpga.h"
#include "cookie_4.h"
#include "platform_fru.h"
#include "nmc93c46.h"
#include "smart_cookie.h"

#include "linux_pci.h"
#include "switzer_common.h"
#include "switzer_priv.h"

#define SWITZER_SENSE_VOLTAGE_DATA          151

#define SWITZER_ADPT_SENSE_RESISTOR         330
#define SWITZER_1TE_SENSE_RESISTOR          150

/* 8mOhm = 8000uOhm == 80(100uOhm) */
#define SWITZER_MANHATTAN_SENSE_RESISTOR    80

static struct {
    struct ngio_intf_t *ngio;

    int fru_orig;
    char pid[FRU_SIZE], *pid_orig;
    char location[FRU_SIZE], *location_orig;

    n2g_i2c_if_t oir;
    n2g_i2c_if_t pca;
    void *i2c_master;

    uint8_t is_carrier_on;

    struct {
        struct ngio_intf_t *ngio;
        n2g_i2c_if_t oir;
        n2g_i2c_if_t pca;

        uint8_t is_sub_on;
        void *i2c_master;
    } sub_module;
} platform;

extern unsigned long get_platform_i2c_addr(int ctrl);
int switzer_carrier_test(struct ngio_intf_t *ngio)
  __attribute__((weak, alias("__switzer_carrier_test")));
int __switzer_carrier_test(struct ngio_intf_t *ngio)
{
  return (TRUE);
}

static void switzer_cterr_init(struct ngio_intf_t *ngio)
{
    int i;
    char *pid = platform.pid;
    char *loc = platform.location;

    if (platform.is_carrier_on)
        return;

    get_pid(ngio->cookie, pid);

    switch (ngio->mod_type) {
    default:
    case WIC_MODULE:
        i = WIC0 + ngio->slot - 1;
        sprintf(loc, "MB/WIC%x", ngio->slot);
        break;
    case SM_MODULE:
        i = SM0 + ngio->slot - 1;
        sprintf(loc, "MB/SM%x", ngio->slot);
        break;
    case SM_DAUGHTER_CARD:
        i = SM0_WIC + ngio->slot - 1;
        sprintf(loc, "MB/SM Carrier Card Slot%x", ngio->slot);
        break;
    }

    fru_table_offset = i;
    platform.fru_orig = i;
    platform.pid_orig = (char *)platform_fru_table[i].pid_string;
    platform.location_orig = (char *)platform_fru_table[i].location_string;
    platform_fru_table[i].pid_string = (unsigned char *)pid;
    platform_fru_table[i].location_string = (unsigned char *)loc;
}

static void switzer_cterr_exit(void)
{
    int i = platform.fru_orig;

    if (platform.is_carrier_on)
        return;

    platform_fru_table[i].pid_string = (unsigned char *)platform.pid_orig;
    platform_fru_table[i].location_string = (unsigned char *)platform.location_orig;
}

static void switzer_ngio_i2c_init(struct ngio_intf_t *ngio)
{
    if (platform.is_carrier_on) {
        memcpy(&platform.sub_module.oir, ngio->oir, sizeof(platform.sub_module.oir));
        memcpy(&platform.sub_module.pca, ngio->pca, sizeof(platform.sub_module.pca));

        platform.sub_module.i2c_master = (void *)platform.sub_module.pca.i2c_base;
    } else {
        memcpy(&platform.oir, ngio->oir, sizeof(platform.oir));
        memcpy(&platform.pca, ngio->pca, sizeof(platform.pca));
        platform.i2c_master = (void *)get_platform_i2c_addr(ngio->i2c_ctrl);
    }
}

static void switzer_ngio_i2c_exit(void)
{
}

struct ngio_intf_t *switzer_ngio(void)
{
    struct ngio_intf_t *switzer_ngio;

    if (platform.sub_module.is_sub_on)
        switzer_ngio = platform.sub_module.ngio;
    else
        switzer_ngio = platform.ngio;

    return switzer_ngio;
}

struct n2g_i2c_iface *switzer_ngio_oir(void)
{
    struct n2g_i2c_iface *switzer_oir;

    if (platform.sub_module.is_sub_on)
        switzer_oir = &platform.sub_module.oir;
    else
        switzer_oir = &platform.oir;

    return switzer_oir;
}

struct n2g_i2c_iface *switzer_ngio_pca(void)
{
    struct n2g_i2c_iface *switzer_pca;

    if (platform.sub_module.is_sub_on)
        switzer_pca = &platform.sub_module.pca;
    else
        switzer_pca = &platform.pca;

    return switzer_pca;
}

void *switzer_ngio_i2c_master(void)
{
    return platform.sub_module.is_sub_on ? platform.sub_module.i2c_master :
                                           platform.i2c_master;
}

int switzer_test(void *_ngio)
{
    struct ngio_intf_t *ngio = _ngio;
    int rc = FAILED;

    if (platform.is_carrier_on) {
        platform.sub_module.ngio = ngio;
        platform.sub_module.is_sub_on = 1;
    } else {
        platform.ngio = ngio;
    }
    switzer_ngio_i2c_init(ngio);
    switzer_cterr_init(ngio);

    prt("Switzer %s(%#x) %s\n", ngio->name, ngio->id, platform.location);
    testname("Switzer %s(%#x) %s", ngio->name, ngio->id, platform.location);

    switch (ngio->id) {
    case C_NIM_1X:
        rc = switzer_10g_test(ngio);
        break;
    case C_SM_NIM_ADPT:
        platform.is_carrier_on = 1;
        rc = switzer_carrier_test(ngio);
        platform.is_carrier_on = 0;
        break;
    case C_NIM_2M:
        rc = switzer_manhattan_2m_test(ngio);
        break;
    case C_NIM_4T:
        rc = switzer_manhattan_4t_test(ngio);
        break;
    case C_NIM_1M:
        rc = switzer_manhattan_1m_test(ngio);
        break;
    case C_NIM_2T:
        rc = switzer_manhattan_2t_test(ngio);
        break;
    default:
        break;
    }

    switzer_cterr_exit();
    switzer_ngio_i2c_exit();

    if (platform.sub_module.is_sub_on) {
        platform.sub_module.is_sub_on = 0;
        platform.sub_module.ngio = NULL;
    } else {
        platform.ngio = NULL;
    }
    return rc;
}

long switzer_ltc4215_reg_test(void)
{
    prpass(testpass, "LTC4215 OIR Register test ");
    cterr_add_component("LTC4215 on Switzer",
                        "I2C controller at router");
    cterr_add_debug("Check LTC4215 on Switzer",
                    "Check the I2C controller at the router");

    return oir_ltc4215_register_test(switzer_ngio_oir());
}

long switzer_ltc4215_reg_read(void)
{
    return util_oir_ltc4215_reg_read(switzer_ngio_oir());
}

long switzer_ltc4215_reg_write(void)
{
    return util_oir_ltc4215_reg_write(switzer_ngio_oir());
}

long switzer_ltc4215_power_info(void)
{
    struct ngio_intf_t *ngio = switzer_ngio();
    n2g_i2c_if_t *oir = switzer_ngio_oir();
    uint32_t voltage, current, power;
    uint8_t data;

    if (oir_ltc4215_reg_read(oir, LTC4215_SOURCE_REG, &data))
        return FAILED;
    voltage = ((data + 1) * SINGLE_SM_VOL) / 100; //unit 10mV

    if (oir_ltc4215_reg_read(oir, LTC4215_SENSE_REG, &data))
        return FAILED;
    current = data * SWITZER_SENSE_VOLTAGE_DATA; //unit 1uV (tmp voltage)

    /* Project switzer has different sense resistors in different board, and we use */
    /* Switzer-1TE's resistor(150) as default. */
    /* Please add case if the new board has other resistor. */
    switch (ngio->id) {
    case C_NIM_1X:
        current /= SWITZER_1TE_SENSE_RESISTOR;
        break;
    case C_SM_NIM_ADPT:
        current /= SWITZER_ADPT_SENSE_RESISTOR;
        break;
    case C_NIM_2M:
    case C_NIM_4T:
    case C_NIM_1M:
    case C_NIM_2T:
        current /= SWITZER_MANHATTAN_SENSE_RESISTOR; //unit 10mA = (1uV / 100uOHM)
        break;
    default:
        current /= SWITZER_1TE_SENSE_RESISTOR;
        break;
    }

    power = voltage * current;

    prt("Voltage = %3d.%02d V\n", (voltage / 100   ), (voltage % 100));
    prt("Current = %3d.%02d A\n", (current / 100   ), (current % 100));
    prt("Power   = %3d.%02d W\n", (power   / 10000 ), ((power % 10000) / 100));

    return PASSED;
}

long switzer_ltc4215_power_off(void)
{
    struct ngio_intf_t *ngio = switzer_ngio();

    n2g_i2c_if_t *oir = switzer_ngio_oir();
    uint8_t data;

    /* turn off the light */
    if (util_oir_ltc4215_led(oir, OIR_LED_OFF))
        return FAILED;

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data))
        return FAILED;
    prt("%d ltc4215 ctrl reg:0x%02x\n", __LINE__, data);

    /* power off module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data))
        return FAILED;

    if (ngio->reset(ngio) < 0) {
        cterr('f', 0, "Unable to reset on module\n");
        return FAILED;
    }

    return PASSED;
}

long switzer_ltc4215_power_on(void)
{
    struct ngio_intf_t *ngio = switzer_ngio();

    n2g_i2c_if_t *oir = switzer_ngio_oir();
    uint8_t data;

    /* turn on board power and take I2C out of reset */
    if ((ngio->on(ngio)) < 0) {
        cterr('f', 0, "Unable to power module\n");
        return FAILED;
    }

    /* power on NGWIC module */
    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data))
        return FAILED;
    prt("%d ltc4215 ctrl reg:0x%02x\n", __LINE__, data);

    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir, LTC4215_CONTROL_REG, &data))
        return FAILED;

    /* turn on the amber light */
    if (util_oir_ltc4215_led(oir, OIR_LED_AMBER_ONLY))
        log_warn("util_oir_ltc4215_led failed.\n");

    mdelay(200);                /* arbitrary */

    if (oir_ltc4215_reg_read(oir, LTC4215_CONTROL_REG, &data))
        return FAILED;
    prt("%d ltc4215 ctrl reg:0x%02x\n", __LINE__, data);

    if (ngio->i2c_unreset(ngio) < 0) {
        cterr('f', 0, "Unable to unreset i2c on module\n");
        return FAILED;
    }

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir, LTC4215_STATUS_REG, &data))
        return FAILED;
    prt("%d ltc4215 status reg:0x%02x\n", __LINE__, data);

    if (!(data & LTC4215_FET_ON_STATUS)) {
        cterr('f', 0, "FET CANNOT be Turned On.\n");
        return FAILED;
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        cterr('f', 0, "Power CANNOT be Turned On.\n");
        return FAILED;
    }

    ngio->unreset(ngio);

    return PASSED;
}

long switzer_power_led_util(uint8_t led_color)
{
    n2g_i2c_if_t *oir = switzer_ngio_oir();
    if (util_oir_ltc4215_led(oir, led_color)) {
        log_warn("util_oir_ltc4215_led failed.\n");
        return FAILED;
    }
    return PASSED;
}

long switzer_pca_reg_read(void)
{
    uint32_t offset;
    uchar data;

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    if (io_port_8bit_i2c_read(&platform.pca,
                              offset, &data, TRUE) == FAILED) {
        cterr_add_component("PCA IO expander on Switzer",
                            "I2C controller at router");
        cterr_add_debug("Check PCA IO expander on Switzer",
                        "Check the I2C controller at the router");
        cterr('f',0,"Unable to read PCA IO expander register @ %#x\n", offset);
        return FAILED;
    }
    prt("Register @ %#x = %#x\n", offset, data);
    return PASSED;
}

long switzer_pca_reg_write(void)
{
    uint32_t offset;
    uchar data = 0;

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(&platform.pca, offset, &data) == FAILED) {
        cterr_add_component("PCA IO expander on Switzer",
                            "I2C controller at router");
        cterr_add_debug("Check PCA IO expander on Switzer",
                        "Check the I2C controller at the router");
        cterr('f',0,"Unable to write PCA IO expander register @ %#x\n", offset);
        return FAILED;
    }
    return PASSED;
}

long switzer_utils_i2c_reg_read(struct switzer_i2c_slave *i2c)
{
    int rc;
    uint8_t cmd;
    size_t count;
    switzer_i2c_xfer_t xfer;
    char buf[BUF_SIZE];

    rc = gethex_answer("I2C XFER(normal:0, quick:1, block:2)", 0, 0, 2);
    switch (rc) {
    default:
    case 0:
        xfer = SWITZER_I2C_XFER_BYTES;
        break;
    case 1:
        xfer = SWITZER_I2C_XFER_QUICK_BYTES;
        break;
    case 2:
        xfer = SWITZER_I2C_XFER_BLOCK_BYTES;
        break;
    }

    cmd = gethex_answer("I2C command", 0, 0, 255);
    count = gethex_answer("I2C command bytes", 2, 0, sizeof(buf));

    if ((rc = __switzer_i2c_slave_read(
             i2c, xfer, switzer_i2c_slave_addr(i2c), cmd, buf, count)) < 0) {
        cterr('f',0,"I2C read failed @ %#x\n", cmd);
        return FAILED;
    }
    switzer_hex_dump(buf, rc, cmd);
    return PASSED;
}

long switzer_utils_i2c_reg_write(struct switzer_i2c_slave *i2c)
{
    int rc;
    uint8_t cmd;
    size_t count;
    switzer_i2c_xfer_t xfer;
    char hex[BUF_SIZE], buf[BUF_SIZE/2];

    rc = gethex_answer("I2C XFER(normal:0, quick:1, block:2)", 0, 0, 2);
    switch (rc) {
    default:
    case 0:
        xfer = SWITZER_I2C_XFER_BYTES;
        break;
    case 1:
        xfer = SWITZER_I2C_XFER_QUICK_BYTES;
        break;
    case 2:
        xfer = SWITZER_I2C_XFER_BLOCK_BYTES;
        break;
    }

    cmd = gethex_answer("I2C command", 0, 0, 255);
    hex[0] = '\0';
    prt("I2C data hexadecimal bytes:  ");
    get_line(hex, sizeof(hex));

    count = strlen(hex) / 2;
    if (switzer_hex_to_bin(hex, buf, count) < 0) {
        cterr('f', 0, "Hexadecimal string format error: %s\n", hex);
        return FAILED;
    }

    if ((rc = __switzer_i2c_slave_write(
             i2c, xfer, switzer_i2c_slave_addr(i2c), cmd, buf, count)) < 0) {
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    return PASSED;
}

long switzer_utils_dash_i2c_reg_read(struct switzer_dash_i2c_slave *i2c)
{
    int rc;
    uint8_t cmd;
    size_t count;
    char buf[BUF_SIZE];

    cmd = gethex_answer("I2C command", 0, 0, 255);
    count = gethex_answer("I2C command bytes", 1, 0, sizeof(buf));

    if ((rc = switzer_dash_i2c_slave_read(i2c, cmd, buf, count)) < 0) {
        cterr('f',0,"I2C read failed @ %#x\n", cmd);
        return FAILED;
    }
    switzer_hex_dump(buf, rc, cmd);
    return PASSED;
}

long switzer_utils_dash_i2c_reg_write(struct switzer_dash_i2c_slave *i2c)
{
    int rc;
    uint8_t cmd;
    size_t count;
    char hex[BUF_SIZE], buf[BUF_SIZE/2];

    cmd = gethex_answer("I2C command", 0, 0, 255);
    hex[0] = '\0';
    prt("I2C data hexadecimal bytes:  ");
    get_line(hex, sizeof(hex));

    if ((count = strlen(hex)) > 1)
        count = strlen(hex) / 2;

    if (switzer_hex_to_bin(hex, buf, count) < 0) {
        cterr('f', 0, "Hexadecimal string format error: %s\n", hex);
        return FAILED;
    }

    if ((rc = switzer_dash_i2c_slave_write(i2c, cmd, buf, count)) < 0) {
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    return PASSED;
}

long switzer_utils_spi_prom_read_status(struct switzer_spi_prom *prom)
{
    int rc;
    uint8_t status;

    rc = switzer_spi_prom_read_status(prom, &status);
    if (rc < 0) {
        cterr_add_component("SPI flash",
                            "SPI controller within the FPGA");
        cterr_add_debug("Check SPI flash",
                        "Check SPI controller within the FPGA");
        cterr('f', 0, "SPI prom read status error");
        return FAILED;
    }
    prt("SPI PROM Status: %#x\n", status);
    return PASSED;
}

long switzer_utils_spi_prom_write_status(struct switzer_spi_prom *prom)
{
    int rc;
    uint8_t status;

    /* set BP3:BP0[5:2]=b1010 to protect golden sectors */
    status = gethex_answer("Enter status: ", 0x28, 0, 0xff);
    rc = switzer_spi_prom_write_status(prom, status);
    if (rc < 0) {
        cterr_add_component("SPI flash",
                            "SPI controller within the FPGA");
        cterr_add_debug("Check SPI flash",
                        "Check SPI controller within the FPGA");
        cterr('f', 0, "SPI prom read status error");
        return FAILED;
    }
    return PASSED;
}

long switzer_utils_spi_prom_read(struct switzer_spi_prom *prom)
{
    int rc;
    uint32_t addr;
    size_t count;
    char buf[BUF_SIZE];

    addr = gethex_answer("Enter address", 0, 0, 0xffffff);
    count = gethex_answer("Enter size", 256, 0, sizeof(buf));

    rc = switzer_spi_prom_read(prom, addr, buf, count);
    if (rc < 0) {
        cterr_add_component("SPI flash",
                            "SPI controller within the FPGA");
        cterr_add_debug("Check SPI flash",
                        "Check SPI controller within the FPGA");
        cterr('f', 0, "SPI prom read error");
        return FAILED;
    }
    switzer_hex_dump(buf, count, addr);
    return PASSED;
}

long switzer_utils_spi_prom_write(struct switzer_spi_prom *prom, int sane)
{
    int rc, ans, i;
    uint32_t addr;
    ssize_t sz;
    size_t size, count;
    const char *path = "spi_prom.bin";
    struct switzer_mmap map = {NULL, NULL, 0};
    char buf[BUF_SIZE];

    cterr_add_component("SPI flash",
                        "SPI controller within the FPGA");
    cterr_add_debug("Check SPI flash",
                    "Check SPI controller within the FPGA");

    addr = gethex_answer("Enter address", 0, 0, 0xffffff);
    count = gethex_answer("Enter size", 0, 0, 0x1000000);
    prt("Enter firmware path [%s]:  ", path);
    if (get_line(buf, sizeof(buf)) > 0)
        path = buf;

    if ((rc = switzer_file_mmap(path, &map, SWITZER_MMAP_READ)) < 0) {
        cterr('f', 0, "Open FPGA firmware path %s failed", path);
        return FAILED;
    }

    if (count > map.length)
        count = map.length;
    else if (!count)
        count = map.length;

    prt("This process will %sPROGRAM%s the SPI flash %#x@%#x with '%s'.\n"
        "Do you really want to do it ?\n"
        "(Press 'y/Y' to continue or any other key to Quit) ",
        sane ? "ERASE, " : "", sane ? " and VERIFY" : "", count, addr, path);
    ans = getchar();
    if ((ans != 'y') && (ans != 'Y'))
        goto out;

    rc = FAILED;

    if (sane) {
        prt("Erasing SPI PROM from %#x to %#x...\n", addr, addr + count - 1);
        sz = switzer_spi_prom_erase(prom, addr, count);
        if (sz < 0) {
            cterr('f', 0, "SPI prom erase error");
            goto out;
        }
    }

    prt("Programming SPI PROM from %#x to %#x...\n", addr, addr + count - 1);
    sz = switzer_spi_prom_write(prom, addr, map.vaddr, count);
    if (sz < 0) {
        cterr('f', 0, "SPI prom program error");
        goto out;
    }

    if (sane) {
        prt("Verifying SPI PROM from %#x to %#x...\n", addr, addr + count - 1);
        for (size = 0; size < count; size += sizeof(buf)) {
            sz = sizeof(buf);
            if (sz > count - size)
                sz = count - size;
            sz = switzer_spi_prom_read(prom, addr + size, buf, sz);
            if (sz < 0) {
                cterr('f', 0, "SPI prom read error");
                goto out;
            }
            for (i = 0; i < sz; i++) {
                if (buf[i] != ((char *)map.vaddr)[size + i]) {
                    cterr('f', 0, "SPI prom verify error %#x", size + i);
                    goto out;
                }
            }
        }
    }

    rc = PASSED;
out:
    switzer_file_munmap(&map);
    return rc;
}

long switzer_utils_spi_prom_erase(struct switzer_spi_prom *prom)
{
    int rc, ans;
    uint32_t addr;
    size_t count;

    addr = gethex_answer("Enter address: ", 0, 0, 0xffffff);
    count = gethex_answer("Enter size: ", 0, 0, 0x1000000);

    prt("This process will ERASE the SPI flash %#x@%#x.\n"
        "Do you really want to do it ?\n"
        "(Press 'y/Y' to continue or any other key to Quit) ",
        count, addr);
    ans = getchar();
    if ((ans != 'y') && (ans != 'Y'))
        return PASSED;

    rc = switzer_spi_prom_erase(prom, addr, count);
    if (rc < 0) {
        cterr_add_component("SPI flash",
                            "SPI controller within the FPGA");
        cterr_add_debug("Check SPI flash",
                        "Check SPI controller within the FPGA");
        cterr('f', 0, "SPI prom erase error");
        return FAILED;
    }
    return PASSED;
}

long switzer_utils_spi_prom_test(struct switzer_spi_prom *prom)
{
    uint32_t addr = 0;
    size_t count = BUF_SIZE;
    char buf[BUF_SIZE];

    prpass(testpass, "SPI Flash Test ");

    if (switzer_spi_prom_read(prom, addr, buf, count) < 0) {
        cterr_add_component("SPI flash",
                            "SPI controller within the FPGA");
        cterr_add_debug("Check SPI flash",
                        "Check SPI controller within the FPGA");
        cterr('f', 0, "SPI prom read error");
        return FAILED;
    }
    return PASSED;
}

long switzer_os_shell(void)
{
    const char *env;

    prt("\nEscaping to Shell from Switzer %s(%#x) %s Menu,\n",
        platform.ngio->name, platform.ngio->id, platform.location);
    prt("To back to Menu, please type exit from Shell.\n\n");

    if (!(env = getenv("SHELL")))
        env = "/bin/bash";
    system(env);
    return PASSED;
}


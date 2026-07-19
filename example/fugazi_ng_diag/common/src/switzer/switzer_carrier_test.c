/* $Id: switzer_carrier_test.c,v 1.7 2021/08/05 08:08:21 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_carrier_test.c,v $
 *------------------------------------------------------------------
 *
 * switzer_carrier_test.c - Switzer-carrier SM.
 *
 * Mar. 2019, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>

#include "byteswap.h"
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
#include "queryflags.h"
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
#include "adapter_fpga.h"

#include "pericom_pcie_switch.h"
#include "switzer_common.h"
#include "switzer_carrier.h"

#define SWITZER_CARRIER_SLOT_WIC1       NGSM_WIC1_SLOT
#define SWITZER_CARRIER_SLOT_WIC2       NGSM_WIC2_SLOT

#define SWITZER_CARRIER_SLOT_UART_BASE  20


#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)

#define SWITZER_CARRIER_NIM0_SYNC 0xc0100
#define SWITZER_CARRIER_NIM1_SYNC 0x30200

const static uint8_t switzer_wic_i2c_ctrl[] = {SWITZER_CARRIER_SLOT_WIC1,
                                               SWITZER_CARRIER_SLOT_WIC2};
static n2g_i2c_if_t carrier_wic_i2c_act2;
extern int slot_get_info(struct ngio_intf_t *ngio, char *type);
extern int print_ngio_slots (struct ngio_intf_t *ngio, char *mod_type);
extern int print_dc_slots (struct ngio_intf_t *ngio, char *ngio_str);
static struct switzer_carrier __mod, *mod = &__mod;

static int switzer_carrier_slot_info_init(struct switzer_carrier *mod);
static void switzer_carrier_slot_info_exit(struct switzer_carrier *mod);
static unsigned long switzer_carrier_set_sync_sgn(int slot);
uint8_t switzer_carrier_get_wic_i2c_ctrl(int slot);

static long utils_fpga_reg_read(void)
{
    uint32_t offset, data;

    offset = gethex_answer("FPGA offset", 0, 0, 0xffff);

    data = *(volatile uint32_t *)(mod->fpga.map.vaddr + offset);

    prt("%#.4x --> %#.8x\n", offset, data);
    return PASSED;
}

static long utils_fpga_reg_write(void)
{
    uint32_t offset, data;

    offset = gethex_answer("FPGA offset", 0, 0, 0xffff);
    data = gethex_answer("Data to write", 0, 0, 0xffffffff);

    *(volatile uint32_t *)(mod->fpga.map.vaddr + offset) = data;

    prt("%#.4x <-- %#.8x\n", offset, data);
    return PASSED;
}

static long ds4424_reg_read(void)
{
    uint8_t cmd;
    cterr_add_component("DS4424 on Switzer",
                        "I2C controller in the FPGA");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C controller in the FPGA");

    cmd = gethex_answer("DS4424 number(i2c slave: 0x60 - 0, 0x20 -1)", 0, 0, 1);
    return switzer_utils_dash_i2c_reg_read(mod->pm[cmd].i2c);
}

static long ds4424_reg_write(void)
{
    uint8_t cmd;
    cterr_add_component("DS4424 on Switzer",
                        "I2C controller in the FPGA");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C controller in the FPGA");

    cmd = gethex_answer("DS4424 number(i2c slave: 0x60 - 0, 0x20 -1)", 0, 0, 1);
    return switzer_utils_dash_i2c_reg_write(mod->pm[cmd].i2c);
}

static long __pclk_reg_read_byte(struct switzer_carrier *mod, uint8_t cmd, uint8_t *val)
{
    int rc;
    char buf[BUF_SIZE];

    if ((rc = switzer_dash_i2c_slave_read(mod->clk.i2c, cmd, buf, 2)) < 0)
        return FAILED;

    *val = buf[1];
    return PASSED;
}

static long __pclk_reg_write_byte(struct switzer_carrier *mod, uint8_t cmd, const uint8_t *val)
{
    int rc;
    uint8_t buf[BUF_SIZE];

    /* Write bytes count before bytes value */
    buf[0] = 1;
    buf[1] = *val;

    if ((rc = switzer_dash_i2c_slave_write(mod->clk.i2c, cmd, buf, 2)) < 0)
        return FAILED;

    return PASSED;
}

static long pclk_reg_read(void)
{
    int rc;
    uint8_t cmd, val;

    /* Multiple bytes Read/Write failed sometime. */
    /* Per HW's suggestion, Read/Write one byte once */
    cmd = gethex_answer("I2C command", 0, 0, 255);

    /* The PCIE Clock in Switzer Carrier always return 0x08(byte length) as  */
    /* the first byte. So Read two bytes and return the second byte */
    if ((rc = __pclk_reg_read_byte(mod, cmd, &val)) < 0) {
        cterr_add_component("PCIE clock on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("PCIE clock on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f',0,"I2C read failed @ %#x\n", cmd);
        return FAILED;
    }
    switzer_hex_dump(&val, 1, cmd);

    return PASSED;
}

static long pclk_reg_write(void)
{
    int rc;
    uint8_t cmd, val;

    /* Multiple bytes Read/Write failed sometime. */
    /* Per HW's suggestion, Read/Write one byte once */
    cmd = gethex_answer("I2C command", 0, 0, 255);
    val = gethex_answer("I2C data byte", 0, 0, 255);

    if ((rc = __pclk_reg_write_byte(mod, cmd, &val)) < 0) {
        cterr_add_component("PCIE clock on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    return PASSED;
}

#define PCIE_CLOCK_OUTPUT_ENABLE_REGSTER 0x00

#define PCIE_CLOCK_OUTPUT_ENABLE_NIM0    0x01
#define PCIE_CLOCK_OUTPUT_ENABLE_NIM1    0x02

static long switzer_pclk_reg_test(void)
{
    int rc;
    uint8_t cmd, data, val, pre_val;

    prpass(testpass, "PCIe Switch Clock Test ");
    cterr_add_component("PCI clock on Switzer",
                        "I2C controller in the FPGA");
    cterr_add_debug("PCI clock on Switzer",
                    "I2C controller in the FPGA");

    cmd = PCIE_CLOCK_OUTPUT_ENABLE_REGSTER;

    /* Read configuration */
    if ((rc = __pclk_reg_read_byte(mod, cmd, &data)) < 0) {
        cterr('f',0,"I2C read failed @ %#x\n", cmd);
        return FAILED;
    }

    pre_val = data;

    /* Disable clock output for NIM0 and NIM1 */
    val = data & ~(PCIE_CLOCK_OUTPUT_ENABLE_NIM0 | PCIE_CLOCK_OUTPUT_ENABLE_NIM1);
    if ((rc = __pclk_reg_write_byte(mod, cmd, &val)) < 0) {
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    if ((rc = __pclk_reg_read_byte(mod, cmd, &data)) < 0) {
        cterr('f',0,"I2C read failed @ %#x\n", cmd);
        return FAILED;
    }

    if (data != val) {
        cterr_add_component("PCI clock on Switzer",
                            "Check PCI clock 9DBV0841 on Switzer");
        cterr_add_debug("PCI clock on Switzer",
                        "Check PCI clock 9DBV0841 on Switzer");
        cterr('f',0,"Disable sub NIM0/NIM1 clock output failed.\n");
        return FAILED;
    }

    /* Enable clock output for NIM0 and NIM1 */
    val = data | PCIE_CLOCK_OUTPUT_ENABLE_NIM0 | PCIE_CLOCK_OUTPUT_ENABLE_NIM1;
    if ((rc = __pclk_reg_write_byte(mod, cmd, &val)) < 0) {
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    if ((rc = __pclk_reg_read_byte(mod, cmd, &data)) < 0) {
        cterr('f',0,"I2C read failed @ %#x\n", cmd);
        return FAILED;
    }

    if (data != val) {
        cterr('f',0,"Enable sub NIM0/NIM1 clock output failed.\n");
        return FAILED;
    }

    /* Restore configurationg */
    if ((rc = __pclk_reg_write_byte(mod, cmd, &pre_val)) < 0) {
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    return PASSED;
}

static size_t switzer_carrier_mutliple_cmd_read(struct switzer_dash_i2c_slave *slave,
                                                void *cmd, uint8_t cmd_len,
                                                void *buf, size_t count)
{
    return __switzer_dash_i2c_slave_read(slave, switzer_dash_i2c_slave_mux(slave),
                                         switzer_dash_i2c_slave_addr(slave),
                                         switzer_dash_i2c_slave_addr_extended(slave),
                                         switzer_dash_i2c_slave_sub_addr_len(slave),
                                         switzer_dash_i2c_slave_sub_addr(slave),
                                         switzer_dash_i2c_slave_sleep(slave),
                                         cmd, cmd_len, buf, count);
}

static size_t switzer_carrier_mutliple_cmd_write(struct switzer_dash_i2c_slave *slave,
                                                 void *cmd, uint8_t cmd_len,
                                                 const void *buf, size_t count)
{
    return __switzer_dash_i2c_slave_write(slave, switzer_dash_i2c_slave_mux(slave),
                                          switzer_dash_i2c_slave_addr(slave),
                                          switzer_dash_i2c_slave_addr_extended(slave),
                                          switzer_dash_i2c_slave_sub_addr_len(slave),
                                          switzer_dash_i2c_slave_sub_addr(slave),
                                          switzer_dash_i2c_slave_sleep(slave),
                                          cmd, cmd_len, buf, count);
}

static long __pcie_switch_i2c_reg_xfer(struct switzer_carrier *mod, uint8_t rw,
                                       uint8_t port, uint32_t addr, uint32_t *val)
{
    int rc;
    uint8_t cmd[4];
    uint32_t data;
    struct switzer_dash_i2c_slave *i2c = mod->pci_switch.i2c;

    cmd[1] = (port >> 1) & 0x0F;
    cmd[2] = ((port & 0x01) << 7) | 0x3C | (uint8_t)((addr >> 10) & 0x03);
    cmd[3] = (uint8_t)((addr >> 2) & 0xFF); /* address bits[0:1] are fixed to 0 */

    if (rw) {
        cmd[0] = 0x03; /* Write commad */
        data = dswap4(*val);
        if ((rc = switzer_carrier_mutliple_cmd_write(i2c, cmd,
                                                     4, (const void *)&data, 4)) < 0) {
            cterr('f',0,"I2C write failed @ %#x\n", cmd);
        }
    } else {
        cmd[0] = 0x04; /* Read commad */
        if ((rc = switzer_carrier_mutliple_cmd_read(i2c, cmd, 4, (void *)&data, 4)) < 0) {
            cterr('f',0,"I2C read failed @ %#x\n", cmd);
        }
        *val = dswap4(data);
    }

    return rc;
}

static long __pcie_switch_i2c_reg_write(struct switzer_carrier *mod,
                                        uint8_t port, uint32_t addr, const uint32_t *val)
{
    return __pcie_switch_i2c_reg_xfer(mod, 1, port, addr, (uint32_t *)val);
}

static long __pcie_switch_i2c_reg_read(struct switzer_carrier *mod,
                                       uint8_t port, uint32_t addr, uint32_t *val)
{
    return __pcie_switch_i2c_reg_xfer(mod, 0, port, addr, val);
}

static size_t __pcie_eeprom_read(struct switzer_carrier *mod, uint32_t addr, uint8_t *buff, size_t count)
{
    int rc;
    uint32_t data, value;

    ssize_t size;

    for (size = 0; size < count; size = size + 2) {
        data = ADR_DATA_REG_ADR((addr + size));

        if ((rc =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_ADR_DATA_REG, &data)) < 0) {
            prt("enable eeprom read set address failed\n");
            return rc;
        }

        data = EEPROM_CONTROL_READ | EEPROM_CONTROL_START;

        if ((rc =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_CONTROL_REG, &data)) < 0) {
            prt("enable eeprom read command failed\n");
            return rc;
        }

        if ((rc =__pcie_switch_i2c_reg_read(mod, 0, EEPROM_ADR_DATA_REG, &value)) < 0){
            return rc;
        }

        buff[size] = (uint8_t)((value >> 16) & 0xff);

        // Read two bytes, except if beyoud read count
        if (size + 1 < count){
            buff[size+1] = (uint8_t)((value >> 24) & 0xff);
        }
    }

    return PASSED;
}

static ssize_t __pcie_eeprom_write(struct switzer_carrier *mod, uint32_t addr,
                                   const void *buf, size_t count)
{
    const ssize_t byte_len = 2;
    ssize_t sz, size;
    uint32_t data, value;
    uint16_t *buff = (uint16_t *)buf;
    uint8_t read_buff;

    // Adjust for odd number of bytes
    for (size = 0; size < (count - count%2); size += byte_len) {
        prt("Address 0x%04x <---- 0x%04x\n", (uint32_t)(addr + size), buff[size/2]);
        data = EEPROM_CONTROL_ENABLE_WRITE | EEPROM_CONTROL_START;

        if ((sz =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_CONTROL_REG, &data)) < 0) {
            prt("enable eeprom write failed\n");
            return sz;
        }

        value = ADR_DATA_REG_ADR((addr + size)) | ADR_DATA_REG_DATA(buff[size/2]);
        if ((sz =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_ADR_DATA_REG, &value)) < 0)
            return sz;

        data = EEPROM_CONTROL_WRITE | EEPROM_CONTROL_START | EEPROM_CONTROL_DISABLE_AUTOLOAD;
        if ((sz =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_CONTROL_REG, &data)) < 0) {
            prt("enable eeprom write failed\n");
            return sz;
        }

        usleep(20000);
    }

    //Write last byte if odd number of bytes
    if (count % 2){
        data = EEPROM_CONTROL_ENABLE_WRITE | EEPROM_CONTROL_START;

        if ((sz =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_CONTROL_REG, &data)) < 0) {
            prt("enable eeprom write failed\n");
            return sz;
        }

        __pcie_eeprom_read(mod, addr + count, &read_buff, 1);

        //Perform casts first to avoid shifting out of data
        value = (uint32_t)read_buff;
        data = (uint32_t)buff[count/2];

        value = (data & 0xff) | (read_buff << 8);
        value = ADR_DATA_REG_ADR((addr + count - 1)) | ADR_DATA_REG_DATA(value);

        prt("Address 0x%04x <---- 0x%04x\n", (uint32_t)(addr + count -1), value >> 16);

        if ((sz =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_ADR_DATA_REG, &value)) < 0)
            return sz;

        data = EEPROM_CONTROL_WRITE | EEPROM_CONTROL_START | EEPROM_CONTROL_DISABLE_AUTOLOAD;
        if ((sz =__pcie_switch_i2c_reg_write(mod, 0, EEPROM_CONTROL_REG, &data)) < 0) {
            prt("enable eeprom write failed\n");
            return sz;
        }
    }

    return size;
}

static ssize_t switzer_carrier_pcie_eeprom_read(void)
{
    int i;

    uint32_t addr;
    uint8_t *buff;
    size_t count;

    addr = gethex_answer("Enter address", 0, 0, 0xffffff);
    count = gethex_answer("Enter size", 0, 0, 0x1000000);

    buff = (uint8_t *)malloc(count);

    if(__pcie_eeprom_read(mod, addr, buff, count)){
        return FAILED;
    }

    for(i = 0; i<count; i++){
        if(i % 10 == 0){
            prt("\n0x%04x:\t", addr + i);
        }

        prt("%02x ", buff[i]);
    }

    free(buff);

    return PASSED;
}

static ssize_t switzer_carrier_pcie_eeprom_program(void)
{
    uint32_t addr;
    size_t size;
    int count, i;
    char prompt[64];

    uint16_t *value;
    uint8_t *value_bytes;

    addr = gethex_answer("Enter offset", 0, 0, 0xffffff);
    count = gethex_answer("Enter number of bytes", 0, 0, 10);

    value_bytes = (uint8_t *)malloc(count + (count % 2));

    for(i = 0; i < count; i++){
        sprintf(prompt, "Enter value to program for byte %04x", addr + i);
        value_bytes[i] = (uint8_t)gethex_answer(prompt, 0, 0, 0xff);
    }

    value = (uint16_t *)value_bytes;

    size = __pcie_eeprom_write(mod, addr, value, count);

    free(value);

    if (size < 0) {
        cterr('f', 0, "PCIe prom program error");
        return FAILED;
    }

    return PASSED;
}

static ssize_t __pcie_eeprom_erase(struct switzer_carrier *mod, uint16_t start, uint16_t end)
{
    uint16_t *eraser;
    ssize_t output;

    eraser = (uint16_t *)malloc((end - start));
    memset(eraser, 0xff, end-start);

    output = __pcie_eeprom_write(mod, start, eraser, end - start);

    free(eraser);

    return output;
}

static ssize_t switzer_carrier_pcie_eeprom_erase(void)
{
    uint16_t start;
    uint16_t end;

    start = gethex_answer("Enter erase starting address:", 0, 0, 0xffff);
    end = gethex_answer("Enter erase ending address", 0, 0, 0xffff);

    return __pcie_eeprom_erase(mod, start, end);
}

static long switzer_carrier_pcie_eeprom_write(void)

{
    int rc;
    uint32_t addr;
    ssize_t sz;
    size_t count;
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

    rc = FAILED;


    prt("Programming EEPROM from %#x to %#x...\n", addr, addr + count - 1);
    sz = __pcie_eeprom_write(mod, addr, map.vaddr, count);
    if (sz < 0) {
        cterr('f', 0, "SPI prom program error");
        goto out;
    }

    rc = PASSED;
out:
    switzer_file_munmap(&map);
    return rc;
}

static long switzer_carrier_pcie_switch_i2c_write(void)
{
    int rc;
    uint32_t addr, data;
    uint8_t port;

    port = gethex_answer("port", 0, 0, 0x0F);
    addr = gethex_answer("register address", 0, 0, 0xFFF);
    data = gethex_answer("register value", 0, 0, 0xFFFFFFFF);

    if ((rc = __pcie_switch_i2c_reg_write(mod, port, addr, &data)) < 0) {
        cterr_add_component("PCIe Register on Switzer Carrier",
                            "I2C controller at router");
        cterr_add_debug("Check PCIe Register on Switzer Carrier",
                        "Check the I2C controller at the router");
        cterr('f',0,"I2C write failed @ %#x\n", addr);
        return FAILED;
    }

    prt("%#x.%#.8x <-- %#.8x\n", port, addr, data);
    return PASSED;
}

static long switzer_carrier_pcie_switch_i2c_read(void)
{
    int rc;
    uint32_t data;
    uint32_t addr;
    uint8_t port;

    port = gethex_answer("port", 0, 0, 0x0F);
    addr = gethex_answer("register address", 0, 0, 0xFFF);

    if ((rc = __pcie_switch_i2c_reg_read(mod, port, addr, &data)) < 0) {
        cterr_add_component("PCIe Register on Switzer Carrier",
                            "I2C controller at router");
        cterr_add_debug("Check PCIe Register on Switzer Carrier",
                        "Check the I2C controller at the router");
        cterr('f',0,"I2C read failed @ %#x\n", addr);
        return FAILED;
    }

    prt("%#x.%#.8x --> %#.8x\n", port, addr, data);
    return PASSED;
}

/* I2C Utils submenu items */
static submenu_xtable_t i2c_utils_submenu_table[] = {
    {"LTC4215 Register Read", switzer_ltc4215_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"LTC4215 Register Write", switzer_ltc4215_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"DS4424 Register Read", ds4424_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"DS4424 Register Write", ds4424_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Clock Register Read", pclk_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Clock Register Write", pclk_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Switch I2C Read", switzer_carrier_pcie_switch_i2c_read, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Switch I2C Write", switzer_carrier_pcie_switch_i2c_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Switch EEPROM Read", switzer_carrier_pcie_eeprom_read, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Switch EEPROM Write", switzer_carrier_pcie_eeprom_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Switch EEPROM Program", switzer_carrier_pcie_eeprom_program, 0,
     0, NULL, 0, NULL, 0},
    {"PCIe Switch EEPROM Erase", switzer_carrier_pcie_eeprom_erase, 0,
     0, NULL, 0, NULL, 0},
};

#define I2C_UTILS_SUBMENU_TABLE_SZ (sizeof(i2c_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t i2c_utils_submenu_primary_items[I2C_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t i2c_utils_submenu_secondary_items[I2C_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char i2c_utils_submenu_title[] = "Switzer Carrier I2C Utilities Menu";

static menuinfo_t i2c_utils_submenu = {
    i2c_utils_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    i2c_utils_submenu_primary_items,
};

static menuinfo_t *i2c_utils_submenup = &i2c_utils_submenu;

static long i2c_utils(void)
{
    build_primary_submenu(i2c_utils_submenu_table, I2C_UTILS_SUBMENU_TABLE_SZ,
                          i2c_utils_submenu_title, &i2c_utils_submenup);
    build_secondary_submenu(i2c_utils_submenu_table, I2C_UTILS_SUBMENU_TABLE_SZ,
                            i2c_utils_submenu_secondary_items);
    menu(i2c_utils_submenup, i2c_utils_submenu_secondary_items, '\0');
    return PASSED;
}

static const reg_info_t ds4424_reg_tbl[] = {
    {"OUT0", 0xf8, READ_ONLY, {1}, 0xF, 0x0},
    {"OUT2", 0xf9, READ_ONLY, {1}, 0xF, 0x0},
    {"OUT3", 0xfa, READ_ONLY, {1}, 0xF, 0x0},
    {"OUT4", 0xfb, READ_ONLY, {1}, 0xF, 0x0},
    {"End of Register", 0, 0, {0}, 0, 0},
};

static long ds4424_reg_test(void)
{
    const reg_info_t *reg;
    uint8_t data;
    int i;

    prpass(testpass, "DS4424 Register Test ");

    for (i = 0; i < 2; i++) {
        for (reg = ds4424_reg_tbl; reg->size.size; reg++) {
            if (switzer_dash_i2c_slave_read(mod->pm[i].i2c,
                                            reg->offset, &data, sizeof(data)) < 0) {
                cterr_add_component("DS4424 on Switzer",
                                    "I2C controller in the FPGA");
                cterr_add_debug("DS4424 on Switzer",
                                "Check the I2C controller in the FPGA");
                cterr('f', 0, "Register Test on DS4424 failed");
                return FAILED;
            }
        }
    }
    return PASSED;
}

static long utils_spi_prom_read_status(void)
{
    return switzer_utils_spi_prom_read_status(mod->spi.prom);
}

static long utils_spi_prom_write_status(void)
{
    return switzer_utils_spi_prom_write_status(mod->spi.prom);
}

static long utils_spi_prom_read(void)
{
    return switzer_utils_spi_prom_read(mod->spi.prom);
}

static long utils_spi_prom_write(void)
{
    return switzer_utils_spi_prom_write(mod->spi.prom, TRUE);
}

static long utils_spi_prom_program(void)
{
    return switzer_utils_spi_prom_write(mod->spi.prom, FALSE);
}

static long utils_spi_prom_erase(void)
{
    return switzer_utils_spi_prom_erase(mod->spi.prom);
}

/* Utils submenu items */
static submenu_xtable_t utils_submenu_table[] = {
    {"I2C Utilites", i2c_utils, 0,
     0, NULL, 0, NULL, 0},
    {"FPGA Register Read", utils_fpga_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"FPGA Register Write", utils_fpga_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Read Status", utils_spi_prom_read_status, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Write Status", utils_spi_prom_write_status, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Read", utils_spi_prom_read, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Write", utils_spi_prom_write, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Program", utils_spi_prom_program, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Erase", utils_spi_prom_erase, 0,
     0, NULL, 0, NULL, 0},
    {"Escape to OS Shell (debugging only)", switzer_os_shell, 0,
     0, NULL, 0, NULL, 0},
};

#define UTILS_SUBMENU_TABLE_SZ (sizeof(utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t utils_submenu_primary_items[UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t utils_submenu_secondary_items[UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char utils_submenu_title[] = "Switzer Carrier Utilities Menu";

static menuinfo_t utils_submenu = {
    utils_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    utils_submenu_primary_items,
};

static menuinfo_t *utils_submenup = &utils_submenu;

static long io_utils(void)
{
    build_primary_submenu(utils_submenu_table, UTILS_SUBMENU_TABLE_SZ,
                          utils_submenu_title, &utils_submenup);
    build_secondary_submenu(utils_submenu_table, UTILS_SUBMENU_TABLE_SZ,
                            utils_submenu_secondary_items);
    menu(utils_submenup, utils_submenu_secondary_items, '\0');
    return PASSED;
}

static int switzer_carrier_device_init(struct switzer_carrier *mod);
static void switzer_carrier_device_exit(struct switzer_carrier *mod);

static long power_on(void)
{
    prt("\nPower On the module.\n");
    if (switzer_carrier_device_init(mod) < 0)
        return FAILED;
    return PASSED;
}

static long power_off(void)
{
    uint8_t ans;

    prt("\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    prt("\n");
    if (ans != 'y' && ans != 'Y') {
        prt("\nPower Off ABORT!\n");
        return PASSED;
    }
    switzer_carrier_device_exit(mod);
    return PASSED;
}

#define VM_OUT_3_3_ADDR 0xF8
#define VM_OUT_2_5_ADDR 0xF9
#define VM_OUT_1_2_ADDR 0xFA
#define VM_OUT_1_0_ADDR 0xFB

#define VM_OUT_1_8_ADDR 0xFB

#define VM_DATA_STEP 1
#define VM_DATA_MAX 30
#define VM_DATA_MASK 0x7F

#define VM_MAX_PERCENT 3

#define VM_SOURCE_FLAG 0x00
#define VM_SINK_FLAG (0x01 << 7)

#define VM_CALCULATE_COEFFICIENT 10

struct switzer_vpara {
    uint8_t devnum;
    uint8_t addr;
};

static const struct switzer_vpara switzer_vpara_table[] = {{0, VM_OUT_1_0_ADDR},
                                                           {0, VM_OUT_1_2_ADDR},
                                                           {1, VM_OUT_1_8_ADDR},
                                                           {0, VM_OUT_2_5_ADDR},
                                                           {0, VM_OUT_3_3_ADDR}};

#define SWITZER_VPARA_TABLE_SZ (sizeof(switzer_vpara_table) / sizeof(struct switzer_vpara))

static double DS4424_data_to_voltage(uint32_t basevoltage, uint8_t vmdata)
{
    double vl;

    if ((vmdata & ~VM_DATA_MASK) == VM_SOURCE_FLAG) {
        vl = basevoltage * (100.0 + (double)(vmdata & VM_DATA_MASK) * VM_MAX_PERCENT / VM_DATA_MAX) / 1000.0;
    } else {
        vl = basevoltage * (100.0 - (double)(vmdata & VM_DATA_MASK) * VM_MAX_PERCENT / VM_DATA_MAX) / 1000.0;
    }

    return vl;
}

static int DS4424_data_to_margin_percent(uint8_t vmdata, int *pmarg)
{
    if ((vmdata & VM_DATA_MASK) > VM_DATA_MAX) {
        /* invalid data */
        return FAILED;
    }

    if ((vmdata & ~VM_DATA_MASK) == VM_SOURCE_FLAG) {
        *pmarg = (vmdata & VM_DATA_MASK) * VM_MAX_PERCENT / VM_DATA_MAX;
    } else {
        *pmarg = 0 - ((vmdata & VM_DATA_MASK) * VM_MAX_PERCENT / VM_DATA_MAX);
    }

    return PASSED;
}

static uint32_t DS4424_get_base_voltage(uint8_t devnum, uint8_t addr)
{
    uint32_t basevoltage;

    if (devnum == 0) {
        switch (addr) {
        case VM_OUT_1_0_ADDR:
            basevoltage = 10;
            break;
        case VM_OUT_1_2_ADDR:
            basevoltage = 12;
            break;
        case VM_OUT_2_5_ADDR:
            basevoltage = 25;
            break;
        case VM_OUT_3_3_ADDR:
            basevoltage = 33;
            break;
        default:
            basevoltage = 0;
            break;
        }
    } else {
        switch (addr) {
        case VM_OUT_1_8_ADDR:
            basevoltage = 18;
            break;
        default:
            basevoltage = 0;
            break;
        }
    }

    return basevoltage;
}

static long display_voltage_marging(struct switzer_carrier *mod, uint8_t devnum, uint8_t addr)
{
    uint8_t predata;
    uint32_t basevoltage;
    int per;
    struct switzer_dash_i2c_slave *slave;
    slave = mod->pm[devnum].i2c;
    basevoltage = DS4424_get_base_voltage(devnum, addr);

    if (switzer_dash_i2c_slave_read(slave, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }

    if (predata == 0) {
        prt("%0.1fV: not margined\n", DS4424_data_to_voltage(basevoltage, 0));
    } else {
        if (DS4424_data_to_margin_percent(predata, &per) == FAILED) {
            prt("%0.1fV: margined out of range\n", DS4424_data_to_voltage(basevoltage, 0));
            return FAILED;
        } else {
            prt("%0.1fV: margined %+d%%\n", DS4424_data_to_voltage(basevoltage, 0), per);
        }
    }

    return PASSED;
}

static long display_all_voltage_margin(void)
{
    int i;
    const struct switzer_vpara *vpara_t = &switzer_vpara_table[0];

    for (i = 0; i < SWITZER_VPARA_TABLE_SZ; i++) {
        display_voltage_marging(mod, vpara_t->devnum, vpara_t->addr);
        vpara_t++;
    }

    return PASSED;
}

static long __power_set_vmarg(struct switzer_dash_i2c_slave *slave, uint8_t devnum,
                              uint8_t addr, uint8_t predata, uint8_t tagdata)
{
    uint8_t predataflag, tagdataflag;                      //bit7,source or sink flag

    if ((tagdata & VM_DATA_MASK) > VM_DATA_MAX) {
        cterr('f', 0, "vmdata out of range");
        return FAILED;
    }

    predataflag = predata & ~VM_DATA_MASK;
    tagdataflag = tagdata & ~VM_DATA_MASK;

    if (predataflag == tagdataflag) {
        do {
            /* Set voltage from predata to tagdata using step VM_DATA_STEP */
            if (predata < tagdata - VM_DATA_STEP)
                predata += VM_DATA_STEP;
            else if (predata > tagdata + VM_DATA_STEP)
                predata -= VM_DATA_STEP;
            else
                predata = tagdata;

            if (switzer_dash_i2c_slave_write(slave, addr,
                                             &predata, sizeof(predata)) < 0) {
                cterr_add_component("DS4424 on Switzer",
                                    "I2C controller in the FPGA");
                cterr_add_debug("DS4424 on Switzer",
                                "Check the I2C controller in the FPGA");
                cterr('f', 0, "Set DS4424 voltage failed, data = %d", predata);
                return FAILED;
            }
        } while (predata != tagdata);
    } else {
        //First set predata to 0, then to tagdata
        if (__power_set_vmarg(slave, devnum, addr, predata, 0 | predataflag) == FAILED)
            return FAILED;

        if (__power_set_vmarg(slave, devnum, addr, 0 | tagdataflag, tagdata) == FAILED)
            return FAILED;
    }

    return PASSED;
}

static long power_set_per_vmarg(struct switzer_dash_i2c_slave *slave, uint8_t devnum,
                                uint8_t addr, uint32_t per, uint8_t ssflag)
{
    uint8_t predata, tagdata;
    uint32_t tmpdata, basevoltage;
    int rc;

    if (per > VM_MAX_PERCENT) {
        cterr('f', 0, "Set percent out of range");
        return FAILED;
    }

    basevoltage = DS4424_get_base_voltage(devnum, addr);
    if (basevoltage == 0) {
        cterr('f', 0, "wrong addr param %#x", addr);
        return FAILED;
    }

    if (switzer_dash_i2c_slave_read(slave, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }

    tmpdata = per * VM_DATA_MAX * VM_CALCULATE_COEFFICIENT / VM_MAX_PERCENT;
    if (tmpdata % VM_CALCULATE_COEFFICIENT >= 5) {
        tagdata = (uint8_t)(tmpdata / VM_CALCULATE_COEFFICIENT + 1);
    } else {
        tagdata = (uint8_t)(tmpdata / VM_CALCULATE_COEFFICIENT);
    }

    tagdata |= ssflag;

    if ((rc = __power_set_vmarg(slave, devnum, addr, predata, tagdata)) == PASSED) {
        prt("change voltage from %0.4fV to %0.4fV\n",
                            DS4424_data_to_voltage(basevoltage, predata),
                            DS4424_data_to_voltage(basevoltage, tagdata));
    } else {
        cterr('f', 0, "change voltage failed");
    }

    return rc;
}

static long power_auto_set_vmarg(struct switzer_carrier *mod, uint8_t devnum,
                                 uint8_t addr, switzer_vmarg_t vmarg)
{
    uint32_t per;
    uint8_t ssflag;
    struct switzer_dash_i2c_slave *slave;

    switch (vmarg) {
    case SWITZER_VMARG_NORMAL:
        per = 0;
        ssflag = VM_SOURCE_FLAG;
        break;
    case SWITZER_VMARG_LOW:
        per = VM_MAX_PERCENT;
        ssflag = VM_SINK_FLAG;
        break;
    case SWITZER_VMARG_HIGH:
        per = VM_MAX_PERCENT;
        ssflag = VM_SOURCE_FLAG;
        break;
    default:
        return FAILED;
    }

    slave = mod->pm[devnum].i2c;

    return power_set_per_vmarg(slave, devnum, addr, per, ssflag);
}

static long power_user_set_vmarg(void)
{
    uint8_t predata, addr, ssflag;
    uint32_t basevoltage, per, anr;
    struct switzer_dash_i2c_slave *slave;
    uint8_t devnum;

    anr = getdec_answer("Choose a current source "
                        "(0 - 1.0V, 1 - 1.2V, 2 - 1.8, 3 - 2.5, 4 - 3.3V)", 0, 0, 4);
    switch (anr) {
    case 0:
        addr = VM_OUT_1_0_ADDR;
        devnum = 0;
        break;
    case 1:
        addr = VM_OUT_1_2_ADDR;
        devnum = 0;
        break;
    case 2:
        addr = VM_OUT_1_8_ADDR;
        devnum = 1;
        break;
    case 3:
        addr = VM_OUT_2_5_ADDR;
        devnum = 0;
        break;
    case 4:
        addr = VM_OUT_3_3_ADDR;
        devnum = 0;
        break;
    default:
        return FAILED;
    }

    slave = mod->pm[devnum].i2c;
    basevoltage = DS4424_get_base_voltage(devnum, addr);
    if (switzer_dash_i2c_slave_read(slave, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }
    prt("\nPresent voltage is %0.3fV", DS4424_data_to_voltage(basevoltage, predata));

    anr = getdec_answer("Source or Sink? (0 for sink, 1 for source)", 0, 0, 1);
    switch (anr) {
    case 0:
        ssflag = VM_SINK_FLAG;
        break;
    case 1:
        ssflag = VM_SOURCE_FLAG;
        break;
    default:
        return FAILED;
    }

    per = getdec_answer("Set percent(0 to 3)", 0, 0, 3);

    return power_set_per_vmarg(slave, devnum, addr, per, ssflag);
}

static long power_set_1_0_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(mod, 0, VM_OUT_1_0_ADDR, vmarg);
}

static long power_set_1_2_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(mod, 0, VM_OUT_1_2_ADDR, vmarg);
}

static long power_set_1_8_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(mod, 1, VM_OUT_1_8_ADDR, vmarg);
}

static long power_set_2_5_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(mod, 0, VM_OUT_2_5_ADDR, vmarg);
}

static long power_set_3_3_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(mod, 0, VM_OUT_3_3_ADDR, vmarg);
}

static long power_set_all_vmarg(switzer_vmarg_t vmarg)
{
    int i;
    const struct switzer_vpara *vpara_t = &switzer_vpara_table[0];;

    for (i = 0; i < SWITZER_VPARA_TABLE_SZ; i++) {
        power_auto_set_vmarg(mod, vpara_t->devnum, vpara_t->addr, vmarg);
        vpara_t++;
    }

    return PASSED;
}

static submenu_xtable_t power_submenu_table[] = {
    {"Power Status", switzer_ltc4215_power_info, 0, 0, NULL, 0, NULL, 0},
    {"Power Off", power_off, 0, 0, NULL, 0, NULL, 0},
    {"Power On", power_on, 0, 0, NULL, 0, NULL, 0},
    {"Set 1.0V to Normal",	(PFT)power_set_1_0_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 1.0V to Margin High",	(PFT)power_set_1_0_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 1.0V to Margin Low", (PFT)power_set_1_0_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Set 1.2V to Normal",	(PFT)power_set_1_2_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 1.2V to Margin High",	(PFT)power_set_1_2_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 1.2V to Margin Low", (PFT)power_set_1_2_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Set 1.8V to Normal",	(PFT)power_set_1_8_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 1.8V to Margin High",	(PFT)power_set_1_8_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 1.8V to Margin Low", (PFT)power_set_1_8_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Set 2.5V to Normal",	(PFT)power_set_2_5_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 2.5V to Margin High",	(PFT)power_set_2_5_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 2.5V to Margin Low", (PFT)power_set_2_5_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Set 3.3V to Normal",	(PFT)power_set_3_3_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 3.3V to Margin High",	(PFT)power_set_3_3_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 3.3V to Margin Low", (PFT)power_set_3_3_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Display Voltage Margins", (PFT)display_all_voltage_margin, 0, 0, NULL, 0, NULL, 0},
    {"Set 1.0V/1.2V/1.8V/2.5V/3.3V to Normal",	(PFT)power_set_all_vmarg,
     SWITZER_VMARG_NORMAL, 0, NULL, 0, NULL, 0},
    {"Set 1.0V/1.2V/1.8V/2.5V/3.3V to High",	(PFT)power_set_all_vmarg,
     SWITZER_VMARG_HIGH, 0, NULL, 0, NULL, 0},
    {"Set 1.0V/1.2V/1.8V/2.5V/3.3V to Low",	(PFT)power_set_all_vmarg,
     SWITZER_VMARG_LOW, 0, NULL, 0, NULL, 0},
    {"User Set", (PFT)power_user_set_vmarg, 0, 0, NULL, 0, NULL, 0},
};

#define POWER_SUBMENU_TABLE_SZ (sizeof(power_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t power_submenu_primary_items[POWER_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t power_submenu_secondary_items[POWER_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char power_submenu_title[] = "Switzer Carrier Power Utilities Menu";

static menuinfo_t power_submenu = {
    power_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    power_submenu_primary_items,
};

static menuinfo_t *power_submenup = &power_submenu;

static long power_utils(void)
{
    build_primary_submenu(power_submenu_table, POWER_SUBMENU_TABLE_SZ,
                          power_submenu_title, &power_submenup);
    build_secondary_submenu(power_submenu_table, POWER_SUBMENU_TABLE_SZ,
                            power_submenu_secondary_items);
    menu(power_submenup, power_submenu_secondary_items, '\0');
    return PASSED;
}

static int switzer_carrier_slot_test(struct ngio_intf_t *ngio, int real_slot,
                                     int slot, int test_type, char *mod_str)
{
    int test_err, fru_orig;

    if (real_slot != slot) /* User opted for submenus */
	    ngio->menu_display = TRUE;
    else
        ngio->menu_display = FALSE;

    switzer_carrier_set_sync_sgn(real_slot);
    /* Store carrier card test's cterr info setup before each slot test */
    fru_orig = fru_table_offset;

    testname("Switzer Carrier %s Slot %1d", mod_str,  real_slot);
    prpass(testpass, " "); /* Zero out the teatpass buffer */

    if (slot_get_info(ngio, mod_str) == FAILED) {
        prcomplete(testpass, errcount, 0);
        return (FAILED);
    }

    /* invoke the diagnostics */
    ngio->test_type = test_type;

    if (test_type == FULL_TEST) {
        test_err = ngio->diag((void *)ngio);
    } else {
        if (ngio->intf_diag)
            test_err = ngio->intf_diag((void *)ngio);
        else {
            test_err = PASSED;
            cterr('w', 0, "No interface test available.");
        }
    }

    if (slot != real_slot) {
        printf("\n%s Subtest Menu accumulated errors = %ld",
               ngio->name, err_accum);
    }

    if (slot == real_slot) {
	    if (test_err == PASSED) { /* only run Authenticate if the test PASSED */
	        if (diagflag_xram & D_XEC_AUTH) { /* For MFG */
                smartchip_authenticate_retest(ngio->mod_type, real_slot);
            }
	    }
        prcomplete(testpass, errcount, 0);
    }

    if (fru_orig != fru_table_offset) {
        /* clear the Sub-NIM card test's cterr info setup */
        platform_fru_table[fru_table_offset].pid_string = (unsigned char *)"";
        platform_fru_table[fru_table_offset].location_string = (unsigned char *)"";
        fru_table_offset = fru_orig;
    }

    if (test_err == FAILED) {
        return test_err;
    }

    if (!((NVRAM)->diagflag & D_POWER_ON)) {
        if (ngio->off)
            ngio->off(ngio);
    }

    return test_err;
}

static long switzer_carrier_nim_test(int slot)
{
    int real_slot = (slot < SWITZER_CARRIER_MAX_SLOT) ?
                     slot : slot - SWITZER_CARRIER_MAX_SLOT;
    struct ngio_intf_t *wic = mod->wic[real_slot - SWITZER_CARRIER_SLOT_WIC1].ngio;

    return switzer_carrier_slot_test(wic, real_slot, slot, FULL_TEST, "Sub NIM");
}

#define SWITZER_PCI_EXP_BASE_ADDR        0x68
#define SWITZER_PCI_EXP_LNKSTA           0x12   /* Link Status */

#define SWITZER_PCI_EXP_LNKSTA_CLS_2_5GB 0x0001 /* Current Link Speed 2.5GT/s */
#define SWITZER_PCI_EXP_LNKSTA_CLS_5_0GB 0x0002 /* Current Link Speed 5.0GT/s */
#define SWITZER_PCI_EXP_LNKSTA_CLS_MASK  \
        (SWITZER_PCI_EXP_LNKSTA_CLS_2_5GB | SWITZER_PCI_EXP_LNKSTA_CLS_5_0GB)

#define SWITZER_PCI_EXP_LNKSTA_NLW_X1    0x0010 /* Current Link Width x1 */
#define SWITZER_PCI_EXP_LNKSTA_NLW_X2    0x0020 /* Current Link Width x2 */
#define SWITZER_PCI_EXP_LNKSTA_NLW_MASK  \
        (SWITZER_PCI_EXP_LNKSTA_NLW_X1 | SWITZER_PCI_EXP_LNKSTA_NLW_X2)

static long switzer_pcie_switch_test(void)
{
    /* Confirm PCIe link speed 5GT/s, width x2 */
    uint32_t offset;
    uint16_t data, speed, width;
    char cmd[256];
    int ret = PASSED;
    struct switzer_carrier_pcisw *pci_switch = &mod->pci_switch;

    prpass(testpass, "PCIe Switch Test ");

    offset = SWITZER_PCI_EXP_BASE_ADDR + SWITZER_PCI_EXP_LNKSTA;

    data = pci_config_read(pci_switch->pci_bus,
                           pci_switch->pci_dev, pci_switch->pci_func, offset);
    speed = data & SWITZER_PCI_EXP_LNKSTA_CLS_MASK;
    width = data & SWITZER_PCI_EXP_LNKSTA_NLW_MASK;

    switch (speed) {
    case SWITZER_PCI_EXP_LNKSTA_CLS_2_5GB:
        sprintf(cmd, "\nPCIe link speed: 2.5 Gb/s, ");
        ret = FAILED;
        break;
    case SWITZER_PCI_EXP_LNKSTA_CLS_5_0GB:
        sprintf(cmd, "\nPCIe link speed: 5.0 Gb/s, ");
        break;
    default:
        sprintf(cmd, "\nPCIe link speed: unkown(%#x), ", speed);
        ret = FAILED;
        break;
    }

    switch (width) {
    case SWITZER_PCI_EXP_LNKSTA_NLW_X1:
        sprintf(cmd + strlen(cmd), "link width: x1");
        ret = FAILED;
        break;
    case SWITZER_PCI_EXP_LNKSTA_NLW_X2:
        sprintf(cmd + strlen(cmd), "link width: x2");
        break;
    default:
        sprintf(cmd + strlen(cmd), "link width: unkown(%#x)", width);
        ret = FAILED;
        break;
    }

    if (ret == PASSED) {
        prt(cmd);
    } else {
        cterr_add_component("PCIe Switch",
                            "PCIe Switch link status");
        cterr_add_debug("Check PCIe Switch",
                        "Check PCIe Switch link status");
        cterr('f', 0, cmd);
    }

    return ret;
}

static int fpga_reg_read(unsigned long addr, int size,
                         unsigned long *buf, void *param)
{
    *(uint32_t *)buf = *(volatile uint32_t *)(mod->fpga.map.vaddr + addr);
    return PASSED;
}

static int fpga_reg_write(unsigned long addr, int size,
                          unsigned long data, void *param)
{
    *(volatile uint32_t *)(mod->fpga.map.vaddr + addr) = (uint32_t)data;
    return PASSED;
}

static reg_info_t_ext fpga_reg_ext = {4, fpga_reg_read, fpga_reg_write, 0};

static reg_info_t fpga_reg_tbl[] =
{
    /* Register name, Offset, Type, Size, Mask, Reset Value */
    {"GENERAL_SCRATCH_PAD", 0x8,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&fpga_reg_ext},
     0xffffffff, 0},
};

static long fpga_reg_test(void)
{
    prpass(testpass, "FPGA Register Test ");
    cterr_add_component("FPGA",
                        "PCIe bus");
    cterr_add_debug("Check FPGA",
                    "Check PCIe link status");
    return register_tests(0, fpga_reg_tbl);
}

static long spi_flash_test(void)
{
    return switzer_utils_spi_prom_test(mod->spi.prom);
}

/* Main menu items */
static submenu_xtable_t main_menu_table[] = {
    {"IO Utilities", io_utils, 0,
     0, NULL, 0, io_utils, 0},
    {"Power Utilities", power_utils, 0,
     0, NULL, 0, power_utils, 0},
    {"FPGA Register Test", fpga_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"PCIe Switch Test", switzer_pcie_switch_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"PCIe Switch Clock Test", switzer_pclk_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"LTC4215 Register Test", switzer_ltc4215_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"DS4424 Register Test", ds4424_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"SPI flash Test", spi_flash_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Sub NIM0 Test", (PFT)switzer_carrier_nim_test, SWITZER_CARRIER_SLOT_WIC1,
     F_ALL_E, NULL, 0,
     (PFT)switzer_carrier_nim_test, SWITZER_CARRIER_SLOT_WIC1 + SWITZER_CARRIER_MAX_SLOT},
    {"Sub NIM1 Test", (PFT)switzer_carrier_nim_test, SWITZER_CARRIER_SLOT_WIC2,
     F_ALL_E, NULL, 0,
     (PFT)switzer_carrier_nim_test, SWITZER_CARRIER_SLOT_WIC2 + SWITZER_CARRIER_MAX_SLOT},
};

#define MAIN_MENU_TABLE_SIZE                                \
    (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static menuinfo_t maindiag = {
    "Switzer Carrier Main Menu",	/* title */
    0,                              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,          /* shows major flags */
    0,                              /* generic prompt */
    0,                              /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static menuinfo_t *maindiagp = &maindiag;

static long sub_iface_test(void)
{
    struct ngio_intf_t *wic;
    int rc = PASSED;
    int slot = SWITZER_CARRIER_SLOT_WIC1;

    for (; slot < SWITZER_CARRIER_MAX_SLOT; slot++) {
        wic = mod->wic[slot - SWITZER_CARRIER_SLOT_WIC1].ngio;
        rc += switzer_carrier_slot_test(wic, slot, slot, IFACE_TEST, "Sub NIM");
    }

    return rc;
}

static int switzer_carrier_iface_test(struct ngio_intf_t *ngio)
{
    if (switzer_pcie_switch_test())
        return FAILED;

    if (sub_iface_test())
        return FAILED;

    return PASSED;
}

static int switzer_carrier_fpga_uart_reset(struct switzer_carrier *mod, int port)
{
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    struct switzer_uart_t *uart;

    if (port < SWITZER_CARRIER_MAX_SLOT) {
        uart = &fpga->csrs->uart[port];

        uart->fcr = SWITZER_UART_FCR_R_TRIG_11 |
                    SWITZER_UART_FCR_CLEAR_RCVR |
                    SWITZER_UART_FCR_CLEAR_XMIT;   /* tx rx reset */
        uart->mcr &= ~SWITZER_UART_MCR_LOOP; /* turn off loopback mode */
    } else {
        log_err("switzer carrier uart port err, port slot is %d\n", port);
        return FAILED;
    }
    return PASSED;
}

static int switzer_carrier_fpga_uart_tx(struct switzer_carrier *mod, int port,
                                        int baud, char* tx_str, int tx_sz, int is_int_lpbk)
{
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    struct switzer_uart_t *uart;
    unsigned int idx, quot;
    char dll, dlm; // division latch least significant and most significant

    if (port < SWITZER_CARRIER_MAX_SLOT) {
        uart = &fpga->csrs->uart[port];
    } else {
        log_err("switzer carrier uart port err, port slot is %d\n", port);
        return FAILED;
    }

    quot = SWITZER_CARRIER_FPGA_FREQ / baud;
    dll = quot & 0xFF;
    dlm = (quot & 0xFF00) >> 8;

    uart->fcr = SWITZER_UART_FCR_R_TRIG_11 |
                SWITZER_UART_FCR_CLEAR_RCVR |
                SWITZER_UART_FCR_CLEAR_XMIT;   /* tx rx reset */

    /* setup baud rate */
    uart->lcr = SWITZER_UART_LCR_DLAB | SWITZER_UART_LCR_WLEN8;   /* 0xc */
    uart->dll = dll;
    uart->dlm = dlm;

    uart->lcr = SWITZER_UART_LCR_WLEN8;
    uart->fcr = SWITZER_UART_FCR_ENABLE_FIFO; /*enable FIFO and 1 byte trigger level */

    if (!tx_sz)
        return PASSED;

    if (is_int_lpbk) {
        uart->mcr = SWITZER_UART_MCR_LOOP; /* turn on loopback mode */
    } else {
        uart->mcr &= ~SWITZER_UART_MCR_LOOP; /* turn off loopback mode */
    }
    for (idx = 0; idx < tx_sz; idx++) {
        uart->dll = (tx_str[idx] & 0xFF);
        switzer_udelay(1000);
    }

    return PASSED;
}

static int switzer_carrier_fpga_uart_rx(struct switzer_carrier *mod,
                                        int port, int *rx_sz, char* rx_str)
{
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    struct switzer_uart_t *uart;
    int cnt = 0;
    char* c;

    if (port < SWITZER_CARRIER_MAX_SLOT) {
        uart = &fpga->csrs->uart[port];
    } else {
        log_err("switzer carrier uart port err, port slot is %d\n", port);
        return FAILED;
    }

    cnt = 0;
    c = rx_str;
    while (uart->lsr & 1) {
        c[cnt] = uart->dll;
        cnt++;
        if (*rx_sz > 0) {
            if (cnt >= *rx_sz)
                return PASSED;
        }
        switzer_udelay(2000); /*delay is important: works for baud 9600 */
    }
    *rx_sz = cnt;
    return PASSED;
}

static int switzer_carrier_fpga_uart_lpbk_txrx(struct switzer_carrier *mod, int port,
                                               char* test_str, int test_sz, char* rx_str,
                                               int *rx_sz, int baud, int is_int_lpbk)
{
    int rc;
    if ((rc = switzer_carrier_fpga_uart_reset(mod, port)) == FAILED) {
        log_err("switzer_carrier_fpga_uart_reset failed.\n");
        return rc;
    }

    if ((rc = switzer_carrier_fpga_uart_tx(mod, port, baud,
                                           test_str, test_sz, is_int_lpbk)) == FAILED) {
        log_err("switzer_carrier_fpga_uart_tx failed.\n");
        return rc;
    }

    if ((rc = switzer_carrier_fpga_uart_rx(mod, port, rx_sz, rx_str)) == FAILED) {
        log_err("switzer_carrier_fpga_uart_rx failed.\n");
        return rc;
    }

    switzer_carrier_fpga_uart_reset(mod, port);

    return PASSED;
}

static void switzer_carrier_uart_reset(int port)
{
    port -= SWITZER_CARRIER_SLOT_UART_BASE;
    switzer_carrier_fpga_uart_reset(mod, port);
}

static int switzer_carrier_uart_tx(int port, int baud,
                                   char* tx_str, int tx_sz, int is_int_lpbk)
{
    port -= SWITZER_CARRIER_SLOT_UART_BASE;
    return switzer_carrier_fpga_uart_tx(mod, port, baud, tx_str, tx_sz, is_int_lpbk);
}

static int switzer_carrier_uart_rx(int port, int *rx_sz, char* rx_str)
{
    port -= SWITZER_CARRIER_SLOT_UART_BASE;
    return switzer_carrier_fpga_uart_rx(mod, port, rx_sz, rx_str);
}

static int switzer_carrier_uart_lpbk_txrx(int port, char* test_str,
                                          int test_sz, char* rx_str,
                                          int *rx_sz, int baud, int is_int_lpbk)
{
    port -= SWITZER_CARRIER_SLOT_UART_BASE;
    if (switzer_carrier_fpga_uart_lpbk_txrx(mod, port, test_str, test_sz,
                                            rx_str, rx_sz, baud, is_int_lpbk)) {
        return FAILED;
    }
    return PASSED;
}

int adapter_uart_lpbk_txrx(int port, char* test_str,
                                                 int test_sz, char* rx_str,
                                                 int *rx_sz, int baud, int is_int_lpbk)
{
    return switzer_carrier_uart_lpbk_txrx(port,test_str,
                                          test_sz, rx_str,
                                          rx_sz, baud, is_int_lpbk);
}

static int switzer_carrier_platform_init(struct switzer_carrier *mod)
{
    if (switzer_carrier_slot_info_init(mod))
        return FAILED;

    if (adapter_uart_init(DAUGHTER_CARD,
                          switzer_carrier_uart_reset,
                          switzer_carrier_uart_tx,
                          switzer_carrier_uart_rx,
                          switzer_carrier_uart_lpbk_txrx)) {
        cterr('f',0,"adapter_uart_init failed\n");
        goto err;
    }

    return PASSED;

err:
    adapter_uart_exit();
    return FAILED;
}

static void switzer_carrier_platform_exit(struct switzer_carrier *mod)
{
    adapter_uart_exit();
    switzer_carrier_slot_info_exit(mod);
}

static int switzer_carrier_device_init(struct switzer_carrier *mod)
{
    struct ngio_intf_t *ngio = switzer_ngio();
    struct switzer_settings settings = {
        .pci_domain = 0,
        .pci_bus = get_ngio_pcie_dev_bus_num(ngio->mod_type, ngio->slot),
        .pci_dev = 0,
        .pci_func = 0,
    };

    if (switzer_ltc4215_power_on())
        log_warn("ltc4215 power on failed\n");

    /* pci ready */
    ngio->pci_rdy(ngio, 1);

    /* Remove PCI kernel modules if it's available. Otherwise system */
    /* will be crashed when we remove PCI node in "switzer_carrier_init". */
    system("modprobe -r switzer_uart");
    system("modprobe -r uio_switzer_fpga");

    switzer_mdelay(8000); /* PCIe switch init needs more than 6s */
    if (switzer_carrier_init(mod, &settings))
        return -1;

    /* turn on the green light */
    if (util_oir_ltc4215_led(ngio->oir, OIR_LED_GREEN_ONLY))
        log_warn("util_oir_ltc4215_led failed.\n");

    system("modprobe uio_switzer_fpga");
    system("echo \"1137 01e9\" > /sys/bus/pci/drivers/uio_switzer_fpga/new_id");
    system("modprobe switzer_uart");

    return 0;
}

static void switzer_carrier_device_exit(struct switzer_carrier *mod)
{
    system("modprobe -r switzer_uart");
    system("modprobe -r uio_switzer_fpga");
    switzer_carrier_exit(mod);
    switzer_ltc4215_power_off();
    mod->ngio = NULL;
}


int switzer_carrier_test(struct ngio_intf_t *ngio)
{
    int rc = FAILED;
    mod->ngio = ngio;

    if (switzer_carrier_device_init(mod))
        return FAILED;

    if (switzer_carrier_platform_init(mod))
        goto err;

    rc = PASSED;
    /* Display and interact with user until <ESC><RET> back to main menu */
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    if (ngio->test_type == IFACE_TEST) {
        rc = switzer_carrier_iface_test(ngio);
    } else if (ngio->menu_display == FALSE) {
        do_all_menu_items(maindiagp);
    } else {
        menu(maindiagp, main_menu_secondary_items, '\0');
    }

    switzer_carrier_platform_exit(mod);
err:
    switzer_carrier_device_exit(mod);
    return rc;
}

static unsigned long switzer_get_i2c_addr(struct switzer_carrier *mod, int slot)
{
    /* This function should be invoked after finishing FPGA initialization. */
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    unsigned long addr = 0;

    if (slot <= SWITZER_CARRIER_SLOT_WIC2) {
        addr = (unsigned long)&fpga->csrs->wic_i2c[slot - SWITZER_CARRIER_SLOT_WIC1];
    }

    return addr;
}

static unsigned long switzer_carrier_get_uart_ctrl_addr(struct switzer_carrier *mod)
{
    /* This function should be invoked after finishing FPGA initialization. */
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    unsigned long addr = 0;

    addr = (unsigned long)&fpga->csrs->uart_ctrl;

    return addr;
}

static unsigned long switzer_carrier_set_sync_sgn(int slot)
{
    struct switzer_uart_ctrl_t *uart_ctrl;

    uart_ctrl = (struct switzer_uart_ctrl_t *)switzer_carrier_get_uart_ctrl_addr(mod);
    if (slot == SWITZER_CARRIER_SLOT_WIC1)
        uart_ctrl->ctrl = SWITZER_CARRIER_NIM0_SYNC;
    else
        uart_ctrl->ctrl = SWITZER_CARRIER_NIM1_SYNC;
    return(PASSED);
}

uint8_t get_adapter_pcie_sub_bus_num(int slot)
{
    uint8_t bus_no;

    /* When Switzer-Carrier didn't be power on with
     * enter into Switzer-Carrier menu or run Switzer-Carrier interface test,
     * the mod->ngio will be NULL */
    if (mod->ngio == NULL) {
        return 0;
    }

    switch (slot) {
    case SWITZER_CARRIER_SLOT_WIC1:
        bus_no = switzer_carrier_get_pci_dev_bus(mod, SWITZER_CARRIER_PCIE_FUNC_NIM0);
        break;
    case SWITZER_CARRIER_SLOT_WIC2:
        bus_no = switzer_carrier_get_pci_dev_bus(mod, SWITZER_CARRIER_PCIE_FUNC_NIM1);
        break;
    default:
        bus_no = 0;
        break;
    }

    return bus_no;
}

static int __switzer_carrier_ngiowic_present(struct switzer_carrier *mod,
                                             int slot)
{
    struct switzer_ng_t *wic;
    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];
    return switzer_ngiowic_present(wic);
}

static void __switzer_carrier_ngiowic_disable(struct switzer_carrier *mod,
                                              int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    switzer_ngiowic_disable(wic);
}

static int __switzer_carrier_ngiowic_i2c_unreset(struct switzer_carrier *mod,
                                                 int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_i2c_unreset(wic);
}

static int __switzer_carrier_ngiowic_unreset(struct switzer_carrier *mod,
                                             int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_unreset(wic);
}

static int __switzer_carrier_ngiowic_i2c_reset(struct switzer_carrier *mod,
                                               int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_i2c_reset(wic);
}

static int __switzer_carrier_ngiowic_reset(struct switzer_carrier *mod,
                                           int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_reset(wic);
}

static int __switzer_carrier_ngiowic_enable_uart(struct switzer_carrier *mod,
                                                 int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_enable_uart(wic);
}

static int __switzer_carrier_ngiowic_disable_uart(struct switzer_carrier *mod,
                                                  int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_disable_uart(wic);
}

static int __switzer_carrier_ngiowic_enable(struct switzer_carrier *mod,
                                            int slot)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return FAILED;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];

    return switzer_ngiowic_enable(wic);
}

static void __switzer_carrier_ngiowic_pci_rdy(struct switzer_carrier *mod,
                                              int slot, int on)
{
    struct switzer_ng_t *wic;

    if ((mod->testing_slot = slot) >= SWITZER_CARRIER_MAX_SLOT)
        return;

    wic = (struct switzer_ng_t *)&mod->fpga.csrs->wic[mod->testing_slot -
                                                      SWITZER_CARRIER_SLOT_WIC1];
    switzer_ngiowic_pci_rdy(wic, on);

    return;
}

static int switzer_carrier_ngiowic_present(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_present(mod, intf->slot);
}

static int switzer_carrier_ngiowic_enable(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_enable(mod, intf->slot);
}

static void switzer_carrier_ngiowic_disable(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    __switzer_carrier_ngiowic_disable(mod, intf->slot);
}

static int switzer_carrier_ngiowic_i2c_reset(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_i2c_reset(mod, intf->slot);
}

static int switzer_carrier_ngiowic_i2c_unreset(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_i2c_unreset(mod, intf->slot);
}

static int switzer_carrier_ngiowic_reset(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_reset(mod, intf->slot);
}

static int switzer_carrier_ngiowic_unreset(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_unreset(mod, intf->slot);
}

static int switzer_carrier_ngiowic_enable_uart(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_enable_uart(mod, intf->slot);
}

static int switzer_carrier_ngiowic_disable_uart(void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    return __switzer_carrier_ngiowic_disable_uart(mod, intf->slot);
}

static void switzer_carrier_ngiowic_pci_rdy(void *p, int on)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    __switzer_carrier_ngiowic_pci_rdy(mod, intf->slot, on);
}

static int switzer_carrier_wic_oir_init(struct switzer_carrier *mod,
                                        struct ngio_intf_t *p, int slot)
{
    n2g_i2c_if_t *oir;
    if (p->oir != NULL)
        return PASSED;

    oir = (n2g_i2c_if_t *)malloc(sizeof(n2g_i2c_if_t));
    if (oir == NULL)
        return FAILED;

    memset(oir, 0, sizeof(n2g_i2c_if_t));
    oir->dev_name = "OIR";
    oir->i2c_bus_type = MOD_IOFPGA_I2C;
    oir->size    = sizeof(uint16_t);
    oir->sub_addr_len = 1;
    oir->mux = I2C_MUX_ZERO;
    oir->buf = (char *)malloc(256);
    oir->i2c_base = switzer_get_i2c_addr(mod, slot);
    if (oir->buf == NULL) {
        free(oir);
        return FAILED;
    }

    oir->i2c_ctrl = switzer_carrier_get_wic_i2c_ctrl(slot);
    oir->i2c_dev = NGIOWIC_I2C_ADDR_OIR;

    p->oir = oir;

    return PASSED;
}

static void switzer_carrier_wic_oir_exit(struct switzer_carrier *mod,
                                         struct ngio_intf_t *p, int slot)
{
    n2g_i2c_if_t *oir;
    if (p->oir != NULL) {
        oir = (n2g_i2c_if_t *)p->oir;
        if (oir->buf != NULL)
            free(oir->buf);

        free(p->oir);
        p->oir = NULL;
    }
}

static uint8_t switzer_carrier_get_wic_uart_ctrl(struct switzer_carrier *mod, int slot)
{
    /* The diag code decides that only one Carrier card can be powered on */
    /* at a time. So we use ttyDASH20 and ttyDASH21 for both SM1 and SM2 */
    /* ttyDASH20 - slot0, ttyDASH21 - slot1*/
    return SWITZER_CARRIER_SLOT_UART_BASE + slot - SWITZER_CARRIER_SLOT_WIC1;
}

#define SWITZER_CARRIER_QUACK_RETRY 8

/**************************************************************************
 *
 * Name: i2c_act2_reset
 *
 * Description: This function implementes a reset to Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
static void switzer_carrier_i2c_act2_reset(sc_context *con_p)
{
    unsigned int slot;
    struct ngio_intf_t *ngio;

    slot = con_p->slot;

    prt("Resetting NIM ACT2...");
    ngio = mod->wic[slot - SWITZER_CARRIER_SLOT_WIC1].ngio;
    ngio->i2c_reset(ngio);
    msleep(500);
    ngio->i2c_unreset(ngio);
    msleep(5000);
    prt("Done\n");
}


/**************************************************************************
 *
 * Name: i2c_act2_read_bytes
 *
 * Description: Read bytes from the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         read_buffer - buffer to hold the data
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static int switzer_carrier_i2c_act2_read_bytes(sc_context *con_p, char *rx_buffer)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int ix;

    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;
    n2g_i2c_if_p->i2c_base = switzer_get_i2c_addr(mod, con_p->slot);

    n2g_i2c_if_p->size = 4; /* default of read 2 byte routine */
    n2g_i2c_if_p->buf = rx_buffer;
    n2g_i2c_if_p->offset = -1;

    for (ix = 0; ix < SWITZER_CARRIER_QUACK_RETRY; ix++) {
        if ((ret_status = n2g_i2c_read(n2g_i2c_if_p)) != PASSED) {
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
}

/**************************************************************************
 *
 * Name: i2c_act2_write_bytes
 *
 * Description: Write bytes to the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         tx_buffer - pointer to the command to be sent
 *         tx_size - size of the command
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static int switzer_carrier_i2c_act2_write_bytes(sc_context *con_p, char *tx_buffer, int tx_size)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int ix;

    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;

    n2g_i2c_if_p->size = tx_size;
    n2g_i2c_if_p->buf = tx_buffer;
    n2g_i2c_if_p->offset = -1;
    n2g_i2c_if_p->i2c_base = switzer_get_i2c_addr(mod, con_p->slot);

    for (ix = 0; ix < SWITZER_CARRIER_QUACK_RETRY; ix++) {
        if ((ret_status = n2g_i2c_write(n2g_i2c_if_p)) != PASSED) {
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
}

/**********************************************************************
 *
 * Function: plat_init_smart_eeprom_context
 *
 * Description:
 *           intializes sc_context.
 * Input:  con_p   - pointer to sc_context
 *         type    - type of module (ie, aim, mb, wic, etc)
 *         slot    - slot
 *         cookie_p- pointer to eeprom data
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
static int switzer_carrier_init_smart_eeprom_context(sc_context *con_p, uint8_t type,
                                                     uint8_t slot, uint8_t *cookie_p,
                                                     char *smc_buf)
{
    con_p->info_string = smc_buf;
    sprintf((char *)smc_buf, "Sub WIC MODULE %d", slot);

    con_p->type = type;
    con_p->slot = slot;
    con_p->cookie_contents = cookie_p;
    con_p->quack_read_2bytes = (PFT)switzer_carrier_i2c_act2_read_bytes;
    con_p->quack_write_2bytes = (PFT)switzer_carrier_i2c_act2_write_bytes;
    con_p->quack_reset = (PFT)switzer_carrier_i2c_act2_reset;

    con_p->dev_if_p->parm1 = (uint8_t)MOD_IOFPGA_I2C;
    if (type == DAUGHTER_CARD) {
        con_p->dev_if_p->parm2 = (uint8_t)NGIOWIC_I2C_ADDR_ACT2;
    } else {
        con_p->dev_if_p->parm2 = (uint8_t)NGIOVM_I2C_ADDR_ACT2;
    }
    con_p->dev_if_p->parm3 = (uint8_t)NGIO_I2C_MUX_ACT2;
    con_p->dev_if_p->parm4 = (uint8_t)switzer_carrier_get_wic_i2c_ctrl(slot);
    con_p->dev_if_p->interface = SCC_I2C_IF;

    return PASSED;
}

static uint16_t switzer_carrier_get_cookie_id(int slot, int type, uint8_t *eeprom_data,
                                              uint16_t *id, char *err)
{
    uint8_t num_byte, *data_ptr;
    sc_context *con, cont;
    dev_if_info_t dev_if;
    char smc_buf[80];

    *id = INVALID_ID;
    con = &cont;
    con->dev_if_p = &dev_if;
    con->dev_if_p->cookie_size = COOKIE_SIZE_512;

    switzer_carrier_init_smart_eeprom_context(con, type, slot, (uint8_t *)eeprom_data, smc_buf);

    /* Use platform function to read cookie so that we don't need to change
       when the cookie construction is changed */
    if (smart_cookie_read(con) == PASSED) {
        if ((data_ptr = (uchar *)search_type_ret_addr_of_first_data
             ((uchar *)eeprom_data, (uchar) CONTROLLER_TYPE,
              &num_byte, FALSE)) == (uchar *) NULL) {
            /*Search CONTROLLER_TYPE failed. */
            *id = INVALID_ID;
        } else {
            *id = *data_ptr++;             /* get board ID */
            *id = *id << 8 | *data_ptr;
        }
        return PASSED;
    }
    return FAILED;
}

static unsigned short switzer_carrier_slot_get_id(void *io, char *err)
{
    struct ngio_intf_t *ngio = (struct ngio_intf_t *)io;
    uint16_t id;
    uint16_t rc = FAILED;
    int slot;

    slot = ngio->mod_type == WIC_DAUGHTER_CARD ? ngio->pc->slot : ngio->slot;

    /* Add support for NIM Daughter Card */
    rc = switzer_carrier_get_cookie_id(slot, ngio->mod_type, ngio->cookie, &id, err);
    if (rc == PASSED) {
        ngio->id = id;
    }

    return rc;
}

static int __switzer_carrier_slot_pca_init(struct switzer_carrier *mod,
                                           struct ngio_intf_t *p, int slot)
{
    n2g_i2c_if_t *pca;

    pca = (n2g_i2c_if_t *)malloc(sizeof(n2g_i2c_if_t));
    if (pca == NULL)
        return FAILED;

    memset(pca, 0, sizeof(n2g_i2c_if_t));

    pca_init_i2c((void *)pca);
    pca->i2c_ctrl = p->i2c_ctrl;
    pca->i2c_dev = NGWIC_I2C_ADDR_IO_PORT;
    pca->i2c_bus_type = MOD_IOFPGA_I2C;
    pca->i2c_base = switzer_get_i2c_addr(mod, slot);

    pca->buf = (void *)malloc(COOKIE_SIZE_512);
    if (pca->buf == NULL)
        goto pca_err;

    memset(pca->buf, 0, COOKIE_SIZE_512);
    p->pca = (void *)pca;

    return PASSED;

pca_err:
    free(pca);
    return FAILED;
}

static void __switzer_carrier_slot_pca_exit(struct switzer_carrier *mod,
                                           struct ngio_intf_t *p, int slot)
{
    n2g_i2c_if_t *pca;
    if (p->pca != NULL) {
        pca = (n2g_i2c_if_t *)p->pca;
        if (pca->buf != NULL) {
            free(pca->buf);
            pca->buf = NULL;
        }
        free(p->pca);
        p->pca = NULL;
    }
}

static int switzer_carrier_slot_dc_dummy(void *p)
{
    return PASSED;
}

static int __switzer_carrier_slot_dc_init(struct ngio_intf_t *p)
{
    struct ngio_intf_t *dc;

    if (p->dc != NULL)
        return PASSED;

    dc = (struct ngio_intf_t *)malloc(sizeof(struct ngio_intf_t));
    if (dc == NULL)
        return FAILED;
    memset(dc, 0, sizeof(struct ngio_intf_t));

    dc->pc = p;
    dc->dc = NULL;

    dc->i2c_ctrl = p->i2c_ctrl;
    dc->uart_ctrl = p->uart_ctrl;
    dc->mod_type = WIC_DAUGHTER_CARD;
    dc->slot = FIRST_SLOT;
    dc->get_id = switzer_carrier_slot_get_id;
    dc->on  = switzer_carrier_slot_dc_dummy;
    dc->off = ngiodc_disable;
    dc->i2c_unreset  = switzer_carrier_slot_dc_dummy;

    /* module code need to override these */
    dc->is_present = ngiodc_present;
    dc->reset = ngiodc_reset;
    dc->unreset = ngiodc_unreset;
    dc->uart_on = ngiodc_enable_uart;
    dc->uart_off = ngiodc_disable_uart;
    dc->pci_rdy = NULL;

    p->dc = dc;

    return PASSED;
}

static void __switzer_carrier_slot_dc_exit(struct ngio_intf_t *p)
{
    if (p->dc) {
        free(p->dc);
        p->dc = NULL;
    }
}

static int __switzer_carrier_slot_init(struct switzer_carrier *mod, int slot)
{
    struct ngio_intf_t *pngio;
    struct switzer_carrier_wic *wic = &mod->wic[slot - SWITZER_CARRIER_SLOT_WIC1];
    char slot_str[64] = {0,};

    if (wic->ngio != NULL)
        return PASSED;

    pngio = (struct ngio_intf_t *)malloc(sizeof(struct ngio_intf_t));

    if (pngio == NULL)
        return FAILED;

    memset(pngio, 0, sizeof(struct ngio_intf_t));
    pngio->slot = slot;
    pngio->is_present = switzer_carrier_ngiowic_present;
    pngio->on = switzer_carrier_ngiowic_enable;
    pngio->off = switzer_carrier_ngiowic_disable;
    pngio->i2c_reset = switzer_carrier_ngiowic_i2c_reset;
    pngio->i2c_unreset = switzer_carrier_ngiowic_i2c_unreset;
    pngio->reset = switzer_carrier_ngiowic_reset;
    pngio->unreset = switzer_carrier_ngiowic_unreset;
    pngio->uart_on = switzer_carrier_ngiowic_enable_uart;
    pngio->uart_off = switzer_carrier_ngiowic_disable_uart;
    pngio->i2c_ctrl = switzer_carrier_get_wic_i2c_ctrl(slot);
    pngio->get_id = switzer_carrier_slot_get_id;
    pngio->pci_rdy = switzer_carrier_ngiowic_pci_rdy;
    pngio->uart_ctrl = switzer_carrier_get_wic_uart_ctrl(mod, slot);
    pngio->mod_type = DAUGHTER_CARD;
    pngio->pc = mod->ngio;
    pngio->dc = NULL;

    if (switzer_carrier_wic_oir_init(mod, pngio, slot))
        goto slot_err1;
    if (__switzer_carrier_slot_pca_init(mod, pngio, slot))
        goto slot_err2;
    if (__switzer_carrier_slot_dc_init(pngio))
        goto slot_err3;

    wic->ngio = pngio;

    if (pngio->is_present(pngio)) {
        snprintf(slot_str, sizeof(slot_str), "Sub-NIM%d", slot);
        if (slot_get_info(pngio, slot_str) != FAILED) {
            if (pngio->id != SLOT_VACCODE) {
                print_ngio_slots(pngio, slot_str);
            } else {
                prt("%s:(slot vacant)\n", slot_str);
            }
        } else {
            prt("Read %s failed\n", slot_str);
        }
        // print dc info
        sprintf(slot_str, "WIC%d Daughtercard", pngio->slot);
        print_dc_slots(pngio->dc, slot_str);
    } else {
        prt("%s:(slot vacant)\n", slot_str);
    }

    return PASSED;

slot_err3:
    __switzer_carrier_slot_pca_exit(mod, pngio, slot);
slot_err2:
    switzer_carrier_wic_oir_exit(mod, pngio, slot);
slot_err1:
    free(pngio);

    return FAILED;
}

static void __switzer_carrier_slot_exit(struct switzer_carrier *mod, int slot)
{
    struct switzer_carrier_wic *wic = &mod->wic[slot - SWITZER_CARRIER_SLOT_WIC1];

    if (wic->ngio == NULL)
        return;

    __switzer_carrier_slot_dc_exit(wic->ngio);
    __switzer_carrier_slot_pca_exit(mod, wic->ngio, slot);
    switzer_carrier_wic_oir_exit(mod, wic->ngio, slot);

    free(wic->ngio);
    wic->ngio = NULL;
}

static int switzer_carrier_slot_info_init(struct switzer_carrier *mod)
{
    int i;

    for (i = SWITZER_CARRIER_SLOT_WIC1; i < SWITZER_CARRIER_MAX_SLOT; i++) {
        if (__switzer_carrier_slot_init(mod, i))
            goto slot_err;
    }
    return PASSED;

slot_err:
    while (i-- > 0) {
        __switzer_carrier_slot_exit(mod, i);
    }
    return FAILED;
}

static void switzer_carrier_slot_info_exit(struct switzer_carrier *mod)
{
    int i;

    for (i = SWITZER_CARRIER_SLOT_WIC1; i < SWITZER_CARRIER_MAX_SLOT; i++) {
        __switzer_carrier_slot_exit(mod, i);
    }
}

uint8_t switzer_carrier_get_wic_i2c_ctrl(int slot)
{
    if (slot > SWITZER_CARRIER_SLOT_WIC2 || slot < SWITZER_CARRIER_SLOT_WIC1) {
        log_err("switzer_carrier wic slot %d is uncorrect\n", slot);
        return 0;
    }
    /* Switzer-carrier doesn't use i2c_ctrl */
    return switzer_wic_i2c_ctrl[slot - SWITZER_CARRIER_SLOT_WIC1];
}

unsigned long switzer_carrier_get_wic_i2c_base(int wic_i2c_ctrl)
{
    /* This function should be invoked after finishing FPGA initialization. */
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    unsigned long addr = 0;
    int slot = wic_i2c_ctrl;

    if (slot > SWITZER_CARRIER_SLOT_WIC2 || slot < SWITZER_CARRIER_SLOT_WIC1) {
        log_err("switzer_carrier wic slot %d is uncorrect\n", slot);
        return 0;
    }

    addr = (unsigned long)&fpga->csrs->wic_i2c[slot - SWITZER_CARRIER_SLOT_WIC1];
    return addr;
}

unsigned long switzer_carrier_get_wic_ngio(int slot)
{
    if (slot > SWITZER_CARRIER_SLOT_WIC2 || slot < SWITZER_CARRIER_SLOT_WIC1) {
        log_err("switzer_carrier wic slot %d is uncorrect\n", slot);
        return 0;
    }
    return (unsigned long)mod->wic[slot - SWITZER_CARRIER_SLOT_WIC1].ngio;
}

void *switzer_carrier_get_wic_i2c_quack(int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    carrier_wic_i2c_act2.dev_name = "ACT2";
    carrier_wic_i2c_act2.offset = -1;
    carrier_wic_i2c_act2.sub_addr_len = 0;
    carrier_wic_i2c_act2.size = sizeof(uint16_t);
    carrier_wic_i2c_act2.mux = I2C_MUX_ZERO;
    carrier_wic_i2c_act2.buf = NULL;
    carrier_wic_i2c_act2.i2c_ctrl = switzer_carrier_get_wic_i2c_ctrl(slot);
    carrier_wic_i2c_act2.i2c_dev = NGIOWIC_I2C_ADDR_ACT2;
    carrier_wic_i2c_act2.i2c_bus_type = MOD_IOFPGA_I2C,
    carrier_wic_i2c_act2.i2c_base = switzer_carrier_get_wic_i2c_base(slot);
    return (void *)&carrier_wic_i2c_act2;
}

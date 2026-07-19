/*------------------------------------------------------------------
 *
 * m2_testcard.c - M.2 testcard wrappers.
 *
 * Jan. 2021, Xiaolan Yang <xiaolaya@cisco.com>
 *
 * Copyright (c) 2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "queryflags.h"
#include "linux_pciutils.h"
#include "linux_usb_test.h"
#include "goofy_i2c.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "dev_pca9557.h"
#include "m2_testcard_host_impl.h"
#include "cli_cmd.h"

/* M2 testcard flag defines */
#define MF_1	(MF_CONTINUOUS | MF_DOGRP)
#define MF_2	(MF_1 | MF_DOALL)
#define MF_3	(MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4	(MF_1 | MF_SHOW_ERRCOUNT)

#define DEV_NAME_LEN 64
#define EEPROM_SIZE  256
#define MOTHER_BOARD   0

#define M2_TC_I2C_ADDR_COOKIE   (0xA0 >> 1)    /* 0x50 (after shifted) */
#define M2_TC_I2C_ADDR_EEPROM   (0xA2 >> 1)    /* 0x51 (after shifted) */
#define M2_TC_I2C_ADDR_IO_EXP   (0x30 >> 1)    /* 0x18 (after shifted) */

#define EEPROM_I2C    0
#define GPIO_EXP_I2C  1
#define EEPROM_COOKIE 2

#define OPT_READ      0
#define OPT_WRITE     1

#define M2_SSD_VENDOR_ID     0x1344
#define M2_SSD_DEVICE_ID     0x6001

#define M2_PCA9557_OUTPUT_REG             1
#define M2_PCA9557_POLAR_REG              2
#define M2_PCA9557_CONFIG_REG             3

#define M2_PCA9557_OUTPUT_REG_INIT        0x01
#define M2_PCA9557_INVER_REG_NO_INVERSE   0x00
#define M2_PCA9557_CONFIG_REG_OUTPUT      0xfe

#define LED_ON                            0xfe
#define LED_OFF                           0x01

#define M2_POWER_DOWN  0
#define M2_POWER_UP    1

extern uint32_t pci_config_read(uint32_t bus, uint16_t device,
                                uint32_t fn, int offset);
extern int get_pcie_cap_struct_ptr(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_cap(uint32_t, uint16_t, int, uint);
extern int access_device_test(char *);
extern int cookie_4_processor_x (uchar *contents, int board_type,
                                int cookie_type, int cookie_size,
                                cli_cookie_cmd *);

static n2g_i2c_if_t m2_tc_i2c_dev[] = {

    /* I2C Device for FPGA M.2 test card*/
    {
     .dev_name = "M.2 Test Card eeprom",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = M2_TC_I2C_ADDR_EEPROM,
     .i2c_speed = N2G_I2C_100KHZ,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .buf = NULL,
    },
    {
     .dev_name = "M.2 Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = M2_TC_I2C_ADDR_IO_EXP,
     .i2c_speed = N2G_I2C_100KHZ,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .buf = NULL,
    },
    {
     .dev_name = "M.2 Test Card eeprom cookie",
     .offset = 0,
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = M2_TC_I2C_ADDR_COOKIE,
     .i2c_speed = N2G_I2C_100KHZ,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .buf = NULL,
    },
};
static int m2_testcard_nvme_test (void);
static int m2_testcard_eusb_test (void);
static int m2_testcard_eeprom_test (void);
static int m2_testcard_utility (void);
static int m2_tc_eeprom_utility(void);
static int m2_testcard_cookie_utility(void);
static int m2_tc_gpio_exp_utility(void);
static int m2_tc_eeprom_cookie(void);
static int m2_tc_cookie_program(void);
static uint32_t m2_tc_i2c_gpio_exp_write (uint32_t, char *);
static uint32_t m2_tc_i2c_gpio_exp_read (uint32_t, char *);
static int m2_testcard_eusb_fw_upgrade (void);
static int m2_testcard_power_down (void);
static int m2_testcard_power_up (void);
static int m2_pca9557_init (void);
static int m2_tc_led_control (uint8_t);

/******************************************************************************
 *
 * Sub Menu used for M.2 Test Card test.
 *
 ******************************************************************************
 */
submenu_xtable_t m2_testcard_submenu_table[] = {
    {"M.2 NVME SSD test",           (PFT)m2_testcard_nvme_test,
        0, MF_3, (type_t(*)())0, 0, (PFT)0,  0},

    {"M.2 USB eMMC test",           (PFT)m2_testcard_eusb_test,
        0, MF_3, (type_t(*)())0, 0, (PFT)0,  0},

    {"M.2 I2C eeprom test",         (PFT)m2_testcard_eeprom_test,
        0, MF_3, (type_t(*)())0, 0, (PFT)0,  0},

    {"M.2 utility",                 (PFT)m2_testcard_utility,
        0, 0,    (type_t(*)())0, 0, (PFT)0,  0},
    {"M.2 I2C cookie utility",      (PFT)m2_testcard_cookie_utility,
        0, 0,    (type_t(*)())0, 0, (PFT)0,  0},
};

#define M2_TESTCARD_SUBMENU_TABLE_SIZE (sizeof(m2_testcard_submenu_table) / \
                                        sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t m2_testcard_primary_items[M2_TESTCARD_SUBMENU_TABLE_SIZE +
						                 MAX_BASE_ITEMS];
static mitem_t m2_testcard_secondary_items[M2_TESTCARD_SUBMENU_TABLE_SIZE +
						                   MAX_BASE_ITEMS];

menuinfo_t m2_testcard_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    m2_testcard_primary_items,
};
menuinfo_t *m2_testcard_submenup = &m2_testcard_subtest_menu;

int m2_testcard_test (boolean test_executed)
{
    unsigned long save_err;

    build_primary_submenu(m2_testcard_submenu_table, M2_TESTCARD_SUBMENU_TABLE_SIZE,
			              "M2 testcard", &m2_testcard_submenup);
    build_secondary_submenu(m2_testcard_submenu_table,
			                M2_TESTCARD_SUBMENU_TABLE_SIZE,
			                m2_testcard_secondary_items);

    m2_pca9557_init();

    if (test_executed) {
        save_err = err_accum;
        do_all_menu_items(&m2_testcard_subtest_menu);
        if ((err_accum - save_err) == 0)
            m2_tc_led_control(LED_ON);
        else
            m2_tc_led_control(LED_OFF);
    } else {
        menu(&m2_testcard_subtest_menu, m2_testcard_secondary_items, '\0');
    }
    return PASSED;
}

/******************************************************************************
 *
 * Sub Menu used for M.2 Test Card i2c utility.
 *
 ******************************************************************************
 */
submenu_xtable_t m2_tc_i2c_submenu_table[] = {
    {"M.2 I2C EEPROM read/write",           (PFT)m2_tc_eeprom_utility,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},

    {"M.2 I2C GPIO Expander read/write",    (PFT)m2_tc_gpio_exp_utility,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},
    {"M.2 USB FW upgrade",                  (PFT)m2_testcard_eusb_fw_upgrade,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},
    {"M.2 Moudle Power up",                 (PFT)m2_testcard_power_up,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},
    {"M.2 Moudle Power Down",               (PFT)m2_testcard_power_down,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},
};

#define M2_TC_I2C_SUBMENU_TABLE_SIZE (sizeof(m2_tc_i2c_submenu_table) / \
                                      sizeof(submenu_xtable_t))
/* Cookie transaction */
static uchar eeprom_cookie_contents[EEPROM_SIZE]=
{
    0x04,0xff,0x40,0x11,0x51,0x41,0x02,0x00,0xe2,0x46,
    0x00,0x49,0x01,0x91,0x1f,0x01,0x42,0x31,0x31,0xc0,
    0x46,0x03,0x20,0x01,0xab,0x6e,0x01,0xc1,0x8b,0xc1,
    0xc1,0xc1,0xc1,0xc1,0xc1,0xc1,0xc1,0xc1,0xc1,0xc1,
    0x88,0x00,0x00,0x00,0x00,0x02,0x01,0xcb,0x8d,0x4d,
    0x2e,0x32,0x2d,0x50,0x43,0x49,0x45,0x2d,0x54,0x45,
    0x53,0x54,0x89,0x56,0x30,0x31,0x00,0x43,0x00,0x02,
    0xcf,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,
    0x81,0x00,0x00,0x00,0x00,0x04,0x00,0xc6,0x8a,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xf3,
    0x00,0x06,0x40,0x00,0x00,0x43,0x00,0x06,0xc9,0x0b,
    0x02,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x01,0x00,
    0x05,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff
};

submenu_xtable_t m2_tc_i2c_submenu__cookie_table[] = {
    {"M.2 I2C EEPROM cookie load default",      (PFT)m2_tc_eeprom_cookie,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},

    {"M.2 I2C EEPROM cookie program",           (PFT)m2_tc_cookie_program,
        0, 0, (type_t(*)())0, 0, (PFT)0,  0},
};

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t m2_tc_i2c_primary_items[M2_TC_I2C_SUBMENU_TABLE_SIZE +
						            MAX_BASE_ITEMS];
static mitem_t m2_tc_i2c_secondary_items[M2_TC_I2C_SUBMENU_TABLE_SIZE +
						              MAX_BASE_ITEMS];

menuinfo_t m2_tc_i2c_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    m2_tc_i2c_primary_items,
};
menuinfo_t *m2_tc_i2c_submenup = &m2_tc_i2c_subtest_menu;

static int m2_testcard_utility (void)
{
    build_primary_submenu(m2_tc_i2c_submenu_table, M2_TC_I2C_SUBMENU_TABLE_SIZE,
			              "M.2 TC i2c utility", &m2_tc_i2c_submenup);
    build_secondary_submenu(m2_tc_i2c_submenu_table,
			                M2_TC_I2C_SUBMENU_TABLE_SIZE,
			                m2_tc_i2c_secondary_items);

    menu(&m2_tc_i2c_subtest_menu, m2_tc_i2c_secondary_items, '\0');
    return PASSED;
}

#define M2_TC_I2C_SUBMENU_COOKIE_TABLE_SIZE (sizeof(m2_tc_i2c_submenu__cookie_table) / \
                                       sizeof(submenu_xtable_t))

static mitem_t m2_tc_i2c_primary_cookie_items[M2_TC_I2C_SUBMENU_COOKIE_TABLE_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t m2_tc_i2c_secondary_cookie_items[M2_TC_I2C_SUBMENU_COOKIE_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

menuinfo_t m2_tc_i2c_subtest_cookie_menu = {
     "%s Subtest Cookie Menu",
     0,                                /* mtparam added by init_empty_menu */
     (PFT)show_endnote,                /* notes missing WICs in combos */
     0,                                /* use generic prompt */
     0,                                /* size (bumped by add_menu_item() */
     m2_tc_i2c_primary_cookie_items,
};
menuinfo_t *m2_tc_i2c_cookie_submenup = &m2_tc_i2c_subtest_cookie_menu;

static int m2_testcard_cookie_utility (void)
{
     build_primary_submenu(m2_tc_i2c_submenu__cookie_table, M2_TC_I2C_SUBMENU_COOKIE_TABLE_SIZE,
                           "M.2 TC i2c cookie utility", &m2_tc_i2c_cookie_submenup);
     build_secondary_submenu(m2_tc_i2c_submenu__cookie_table,
                             M2_TC_I2C_SUBMENU_COOKIE_TABLE_SIZE,
                             m2_tc_i2c_secondary_cookie_items);

     menu(&m2_tc_i2c_subtest_cookie_menu, m2_tc_i2c_secondary_cookie_items, '\0');
     return PASSED;
}


/******************************************************************************
 *
 * M.2 Test Card API.
 *
 ******************************************************************************
 */

static int m2_pca9557_init(void)
{
    char chr;

    // set to output
    chr = M2_PCA9557_CONFIG_REG_OUTPUT;     //new data is 0xfe
    m2_tc_i2c_gpio_exp_write(M2_PCA9557_CONFIG_REG, &chr);

    // clear inversion: no inversion
    chr = M2_PCA9557_INVER_REG_NO_INVERSE;	//new data is 0x00
    m2_tc_i2c_gpio_exp_write(M2_PCA9557_POLAR_REG, &chr);

    // initial bit0
    m2_tc_i2c_gpio_exp_read(M2_PCA9557_OUTPUT_REG, &chr);
    chr = chr | M2_PCA9557_OUTPUT_REG_INIT; //set bit0 is '1'
    m2_tc_i2c_gpio_exp_write(M2_PCA9557_OUTPUT_REG, &chr);

    return PASSED;

}
static int m2_tc_led_control(uint8_t flag)
{
    uchar chr ;

    m2_tc_i2c_gpio_exp_read(M2_PCA9557_OUTPUT_REG, (char*)&chr);
    if (flag == LED_ON)
        chr = chr & LED_ON;
    else
        chr = chr | LED_OFF;
    m2_tc_i2c_gpio_exp_write(M2_PCA9557_OUTPUT_REG, (char*)&chr);

    return PASSED;
}

static int m2_pcie_lane_scan(int bus, int dev, int fn, int link_speed, int link_width)
{
    /* Description : scan m2 pcie link speed and width
     * Inputs: bus, dev, fn - m2 pcie [bus]:[dev].[func]
     *         link_speed   - m2 pcie working speed
     *         link_width   - m2 pcie working width
     *         M2 PCIE working speed and width may be restricted by rommon
     * Output: PASSED or FAILED
     */
    uint32_t reg_val, cap_val, sta_val;
    uint32_t cap_s, sta_s, sta_w;
    uint32_t val, device_id, vendor_id;

    printf("\nTesting SSD vendor ID: 0x%02x, device ID: 0x%02x\n",
           M2_SSD_VENDOR_ID, M2_SSD_DEVICE_ID);

    /* Read Vendor ID and Device ID */
    val = pci_config_read(bus, dev, fn, 0);
    vendor_id =  val & 0x0000FFFF;
    device_id = (val & 0xFFFF0000) >> 16;
    if ((vendor_id != M2_SSD_VENDOR_ID) || (device_id != M2_SSD_DEVICE_ID)) {
        cterr('f',0, "Vendor_id : Device_id(%04x:%04x) is not correct.",
              vendor_id, device_id);
        return (FAILED);
    }

    reg_val = get_pcie_cap_struct_ptr(bus, dev, fn, PCI_CAP_PTR_OFFSET);
    if (reg_val == FAILED) {
        cterr('f',0, "Can't get PCI cap pointer.");
        return (FAILED);
    }

    cap_val = get_pcie_link_cap(bus, dev, fn, reg_val);
    sta_val = get_pcie_link_status(bus, dev, fn, reg_val);

    /* Speed - bit 0~3 */
    cap_s = cap_val & PCI_EXP_LINK_STA_SPD_MASK;
    sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
    /* Width - bit 4~9 */
    sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

    if ((cap_s == PCI_EXP_LINK_STA_SPD_2DOT5) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
        printf("Capability speed is 2.5G, Link speed is 2.5G\n");
    } else if ((cap_s == PCI_EXP_LINK_STA_SPD_5GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
        printf("Capability speed is 5G, Link speed is 5G\n");
    } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_8GT)) {
        printf("Capability speed is 8G, Link speed is 8G\n");
    } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
        printf("Capability speed is 8G, Link speed is 2.5G\n");
    } else {
        cterr('f',0, "Link speed is not 2.5G, 5G or 8G, vendor id 0x%x device id 0x%x"\
                     "SSD capability speed is %x status speed is %x.",
                     M2_SSD_VENDOR_ID, M2_SSD_DEVICE_ID, cap_s, sta_s);
        return (FAILED);
    }
    if (sta_s != link_speed) {
        cterr('f',0, "Link speed is not correct, vendor id 0x%x device id 0x%x.",
                     M2_SSD_VENDOR_ID, M2_SSD_DEVICE_ID);
        return (FAILED);
    }

    /* NVMe SSD Width */
    if (sta_w == link_width) {
        if (sta_w == PCI_EXP_LINK_STA_WID_1) {
            printf("Link width is x1\n");
        } else if (sta_w == PCI_EXP_LINK_STA_WID_2) {
            printf("Link width is x2\n");
        } else if (sta_w == PCI_EXP_LINK_STA_WID_4) {
            printf("Link width is x4\n");
        } else if (sta_w == PCI_EXP_LINK_STA_WID_8) {
            printf("Link width is x8\n");
        } else {
            cterr('f',0, "Link width is not x1 x2 x4 and x8, vendor id 0x%x device id 0x%x, SSD"\
                         "status width = %x.", M2_SSD_VENDOR_ID, M2_SSD_DEVICE_ID, sta_w);
	        return (FAILED);
        }
    } else {
        cterr('f',0, "Link width %x is not correct, vendor id 0x%x device id 0x%x.",
                     sta_w, M2_SSD_VENDOR_ID, M2_SSD_DEVICE_ID);
        return (FAILED);
    }

    printf("PCIe lane scan passed.\n");
    return (PASSED);
}

static int m2_testcard_nvme_test (void)
{
    char nvme_dev[32] = {0};
    char sys_cmd[256] = {0};
    char *temp_file="temp_file";
    FILE *fp;
    int word_count;
    int bus, dev, fn, speed, width;

    testname("M.2 Test Card PCIE NVMe SSD Read/Write");
    prpass(testpass, (char *)NULL);
    printf("\n");

    /* pcie lane scan */
    if (m2_tc_host_get_m2_pcie_config(&bus, &dev, &fn, &speed, &width) == FAILED) {
        cterr('f',0, "Can't get m2 PCIE link speed and width.");
        return (FAILED);
    }
    if (m2_pcie_lane_scan(bus, dev, fn, speed, width) == FAILED) {
        cterr('f',0, "Can't get m2 PCIE width.");
        return (FAILED);
    }

    if (m2_tc_host_get_nvme_dev(nvme_dev,sizeof(nvme_dev)) == FAILED) {
        cterr('f',0, "Can't get PCIE nvme dev.");
        return (FAILED);
    }
    /* Check if NVMe module exists*/
    sprintf(sys_cmd, "rm -f %s; ls /dev | grep %s | wc -w > %s;",
            temp_file, &nvme_dev[5], temp_file);
    system(sys_cmd);

    fp = fopen(temp_file, "r");
    if (fp == NULL) {
        cterr('f',0, "Failed to open %s", temp_file);
        return (FAILED);
    }
    word_count = 0;
    fscanf(fp, "%d", &word_count); /* Scan in the value */
    fclose(fp);

    /* SSD read/write test */
    if (word_count != 0) {
        printf("SSD read/write verify\n");
        if (sata_tests((uchar *)nvme_dev) != PASSED) {
            cterr('f',0, "SSD read/write test failed.");
            return (FAILED);
        }
    } else {
        cterr('f',0, "SSD read/write test failed.");
        return (FAILED);
    }
    return (PASSED);
}

static int m2_testcard_eusb_test (void)
{
    int rc = FAILED;
    char eusb_dev[32] = {0};
    testname("M.2 Test Card USB eMMC Read/Write");
    prpass(testpass, (char *)NULL);
    printf("\n");

    rc = m2_tc_host_get_eusb_dev(eusb_dev,sizeof(eusb_dev));
    if (rc == FAILED) {
        cterr('f',0, "Can't get USB eMMC dev.");
        return (rc);
    }
    rc = access_device_test(eusb_dev);
    if (rc == FAILED) {
        cterr('f', 0, "M.2 Test Card USB eMMC test failed.");
        return(rc);
    }
    return (PASSED);
}

static uint32_t m2_tc_i2c_read(int i2c_id, uint32_t offset, char *data)
{
    uint32_t rc;
    n2g_i2c_if_t i2c_if;
    uint8_t i2c_ctrl, i2c_mux;

    memcpy(&i2c_if, &m2_tc_i2c_dev[i2c_id], sizeof(i2c_if));
    m2_tc_host_get_i2c_dev(&i2c_ctrl, &i2c_mux);
    i2c_if.i2c_ctrl = i2c_ctrl;
    i2c_if.mux = i2c_mux;
    i2c_if.buf = (char *)data;
    i2c_if.offset = offset;
    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "unable to read from eeprom.");
        return FAILED;
    }
    return(PASSED);
}

static uint32_t m2_tc_i2c_write(int i2c_id, uint32_t offset, unsigned char *data)
{
    uint32_t rc;
    n2g_i2c_if_t i2c_if;
    uint8_t i2c_ctrl, i2c_mux;

    memcpy(&i2c_if, &m2_tc_i2c_dev[i2c_id], sizeof(i2c_if));
    m2_tc_host_get_i2c_dev(&i2c_ctrl, &i2c_mux);
    i2c_if.i2c_ctrl = i2c_ctrl;
    i2c_if.mux = i2c_mux;
    i2c_if.buf = (char *)data;
    i2c_if.offset = offset;
    rc = n2g_i2c_write(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "Unable to write to eeprom.");
        return FAILED;
    }
    return(PASSED);
}

static uint32_t m2_tc_i2c_eeprom_read(uint32_t offset, char *data)
{
    return m2_tc_i2c_read(EEPROM_I2C, offset, data);
}

static uint32_t m2_tc_i2c_eeprom_write(uint32_t offset, unsigned char *data)
{
    return m2_tc_i2c_write(EEPROM_I2C, offset, data);
}
static uint32_t m2_tc_i2c_eeprom_cookie_read(uint32_t offset, char *data)
{
    return m2_tc_i2c_read(EEPROM_COOKIE, offset, data);
}

static uint32_t m2_tc_i2c_eeprom_cookie_write(uint32_t offset, unsigned char *data)
{
    return m2_tc_i2c_write(EEPROM_COOKIE, offset, data);
}

static uint32_t m2_tc_i2c_gpio_exp_read(uint32_t offset, char *data)
{
    return m2_tc_i2c_read(GPIO_EXP_I2C, offset, data);
}

static uint32_t m2_tc_i2c_gpio_exp_write(uint32_t offset, char *data)
{
    return m2_tc_i2c_write(GPIO_EXP_I2C, offset, (unsigned char*)data);
}

static int m2_tc_eeprom_test (void)
{
    uint32_t rc, i;
    unsigned char sav_data[EEPROM_SIZE+1];
    unsigned char new_data[EEPROM_SIZE+1];

    memset(sav_data, 0, sizeof(sav_data));
    memset(new_data, 0, sizeof(new_data));
    /*save orginal eeprom data */
    for (i = 0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_read(i, (char *)&sav_data[i]);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read eeprom.", __FUNCTION__);
            return (FAILED);
        }
    }

    /*write new data into eeprom byte by byte*/
    for (i = 0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_write(i, (unsigned char*)&i);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to write new data to eeprom.",
                          __FUNCTION__);
            return (FAILED);
        }
        /* wait at 5ms for each write transaction */
        usleep(5000);
    }

    /*read back what we just wrote to eeprom */
    for (i = 0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_read(i, (char*)&new_data[i]);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read eeprom.", __FUNCTION__);
            return (FAILED);
        }
    }

    /* compare data */
    for (i = 0; i < EEPROM_SIZE; i++) {
        if (new_data[i] != i) {
            cterr('f', 0, "%s: wrong data: @%#x=%#x; expecting %#x.",
                          __FUNCTION__, i, new_data[i], i);
            break;
        }
    }

    fflush(stdout);
    /*write save data back to eeprom byte by byte*/
    for (i = 0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_write(i, &sav_data[i]);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to write save data to eeprom.",
                          __FUNCTION__);
            return (FAILED);
        }
        /* wait at 5ms for each write transaction */
        usleep(5000);
    }
    return (PASSED);
}

static int m2_testcard_eeprom_test (void)
{
    int rc = FAILED;

    testname("M.2 Test Card I2C EEPROM Read/Write");
    prpass(testpass, (char *)NULL);
    printf("\n");

    rc = m2_tc_eeprom_test();
    if (rc == FAILED) {
        cterr('f', 0, "I2C EEPROM test failed.");
    }
    return(rc);
}

static int m2_tc_eeprom_utility(void)
{
    int rc, opt;
    unsigned char data;
    uint32_t offset;

    printf("EEPROM Read/Write Utility\n");

    opt = getdec_answer("Read/Write EEPROM? (0-Read, 1-Write):",
                        OPT_READ, OPT_READ, OPT_WRITE);

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);

    if (opt == OPT_READ) {
        rc = m2_tc_i2c_eeprom_read(offset, (char*)&data);
    } else {
        data = gethex_answer("Enter data", 0, 0, 0xFF);
        rc = m2_tc_i2c_eeprom_write(offset, &data);
    }
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "%s: Unable to access eeprom.", __FUNCTION__);
        return (FAILED);
    }
    printf("offset 0x%2x: 0x%2x\n", offset, data);

    return(PASSED);
}

static int m2_tc_gpio_exp_dev_create (dev_pca9557_object_t *gpio_obj)
{
    dev_object_t *dev = (dev_object_t *)gpio_obj;

    /* Create common device object */
    pca9557_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    if (gpio_obj->base.dev_object_fvt->dev_attach) {
        gpio_obj->base.dev_object_fvt->dev_attach(dev);
    } else {
        printf("%s: Something is wrong. Attach function is NULL\n", __func__);
        return (FAILED);
    }
    /* Setup call-out function vectors */
    gpio_obj->callout_fvt->rd = m2_tc_i2c_gpio_exp_read;
    gpio_obj->callout_fvt->wr = m2_tc_i2c_gpio_exp_write;

    return (PASSED);
}

static int m2_tc_gpio_exp_utility (void)
{
    dev_pca9557_object_t pca_data;
    dev_pca9557_object_t *pca_obj = &pca_data;
    int opt;

    m2_tc_gpio_exp_dev_create(pca_obj);

    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object.", __func__);
        return (FAILED);
    }

    printf("GPIO Expander Register Read/Write Utility\n");
    opt = getdec_answer("Read/Write Register? (0-Read, 1-Write):",
                        OPT_READ, OPT_READ, OPT_WRITE);

    if (opt == OPT_READ) {
        pca_obj->callin_fvt->dump_register((dev_object_t *)pca_obj);
    } else {
        pca_obj->callin_fvt->alter_register((dev_object_t *)pca_obj);
    }
    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (PASSED);
}

static int m2_tc_eeprom_cookie_write(void)
{
    uint32_t rc,i;
    /* write cookie data into eeprom  by byte */
    for (i=0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_cookie_write(i, &eeprom_cookie_contents[i]);

        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to access eeprom.", __FUNCTION__);
            return (FAILED);
        }
    /* wait at 5ms for each write transaction */
    usleep(5000);
    }

    return (PASSED);
}

static int m2_tc_eeprom_cookie(void)
{
    int rc = FAILED;
    printf("\n");

    rc = m2_tc_eeprom_cookie_write();
    if (rc == FAILED) {
        cterr('f', 0, "I2C EEPROM cookie failed.");
    }
    printf("M.2 I2C EEPROM cookie load default is OK !!!\n");
    return(rc);
}

static int m2_tc_cookie_program (void)
{
    int rc = FAILED,i;
    unsigned char cookie_data[EEPROM_SIZE+1];
    memset(cookie_data, 0, sizeof(cookie_data));

    /*read eeprom cookie data */
    for (i = 0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_cookie_read(i, (char*)&cookie_data[i]);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to read eeprom.", __FUNCTION__);
            return (FAILED);
        }
    }
    rc = cookie_4_processor_x(cookie_data, MOTHER_BOARD, 0, EEPROM_SIZE, NULL);

    /*  if cookie is altered  */
    if(rc){
    /*write new cookie data into eeprom by byte*/
    for (i = 0; i < EEPROM_SIZE; i++) {
        rc = m2_tc_i2c_eeprom_cookie_write(i, &cookie_data[i]);
        if (rc != RC_I2C_OP_OK) {
             cterr('f', 0, "%s: Unable to write new data to eeprom.",
                           __FUNCTION__);
             return (FAILED);
        }
        /* wait at 5ms for each write transaction */
        usleep(5000);
        }
    }

    return(PASSED);
}

static int m2_tc_user_confirmation()
{
    char ans;
    printf("Do you really want to do it ?\n");
    printf("(Press 'y/Y' to continue or any other key to Quit) ");

    ans = getchar();
    if (!((ans == 'y') || (ans == 'Y'))) {
        printf("\n The user did not continue!!!\n");
        return (FAILED);
    }

    return PASSED;
}

static int m2_testcard_eusb_fw_upgrade(void)
{
    char sys_cmd[256] = {0};
    char eusb_dev[32] = {0};

    /* Show warning and get User confirmation. */
    printf("This process will upgrade USB Firmware  \n");
    if (m2_tc_user_confirmation() == FAILED) {
    /* If user choose not to continue, return */
        return PASSED;
    }

    if (m2_tc_host_get_eusb_dev(eusb_dev, sizeof(eusb_dev)) == FAILED) {
        cterr('f',0, "Can't get USB eMMC dev.");
        return (FAILED);
    }

    sprintf(sys_cmd, "set_dfu_mode -vvv %s; sleep 5; dfu-util -R -D /firmware/Cisco-emmc-v211.dfu", eusb_dev);
    printf(" %s\n ", sys_cmd);
    system(sys_cmd);

    return PASSED;
}

static int m2_testcard_power_up(void)
{
    printf("M2 Testcard Power Control \n");
    /* Show warning and get User confirmation. */
    printf("This process will Enable power to module  \n");
    if (m2_tc_user_confirmation() == FAILED) {
    /* If user choose not to continue, return */
        return PASSED;
    }

    m2_tc_power_control(M2_POWER_UP);

    return PASSED;
}

static int m2_testcard_power_down(void)
{
    printf("M2 Testcard Power Control \n");
    /* Show warning and get User confirmation. */
    printf("This process will Disable power to module  \n");
    if (m2_tc_user_confirmation() == FAILED) {
    /* If user choose not to continue, return */
        return PASSED;
    }

    m2_tc_power_control(M2_POWER_DOWN);

    return PASSED;
}

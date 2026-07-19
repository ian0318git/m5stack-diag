 /* $Id: mb_tests.c,v 1.2 2019/12/11 10:10:32 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "dash_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "i2c_api.h"
#include "dash_fpga.h"
#include "diag_ddr4_lib.h"
#include "diag_i2c_lib.h"
#include "diag_usb_lib.h"
#include "diag_usb_test.h"
#include "diag_temp_snsr_test.h"
#include "diag_gephy_1543_test.h"
#include "diag_led_test.h"
#include "diag_rtc_test.h"
#include "platform_cpu.h"
#include "diag_m2_test.h"
#include "common_utils.h"
#include "diag_esw_test.h"
#include "diag_async_test.h"

extern boolean has_daughter_card(int);
extern uint32 cterr_db_print(char *fmtptr, ...);
static int aux_loopback_test(int dummy);
static int dev_port_read(int dummy);
static int dev_port_write(int dummy);
static int aux_port_test(void);

/*
 * Global variables
 */
fru_table_t platform_fru_table[];


/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar mb_phy_id[] = "Marvell 1548/1340";
uchar mb_sfp_id[] = "SFP-ID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";
uchar psu_pid[] = "PSU-PID";
uchar rps_pid[] = "RPS-PID";
uchar pvdm_pid[] = "PVDM-PID";
uchar backplane_pid[] = "Backplane-PID";
uchar risercard_pid[] = "RiserCard-PID";
uchar sm_pid[] = "SM-PID";
uchar wic_pid[] = "WIC-PID";
uchar dc_pid[] = "DC-PID";
uchar mb_loc[] = "MB";
uchar mb_xaui_loc[] = "MB_XAUI";
uchar mb_aux_loc[] = "MB_AUX";
uchar mb_i2c_loc[] = "MB/FPGA/I2C & MB/Rangeley/I2C";
uchar mb_fpga_reg_loc[] = "MB/FPGA-REG";
uchar mb_emmc_loc[] = "MB/eMMC";
uchar mb_eusb_loc[] = "MB/eUSB";
uchar mb_usb0_loc[] = "MB/USB0";
uchar mb_msata_loc[] = "MB/mSATA";
uchar mb_phy_loc[] = "MB/PHY";
uchar mb_sfp_loc[] = "MB/SFP";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar sfp0_loc[] = "MB/SFP0";
uchar sfp1_loc[] = "MB/SFP1";
uchar sfp2_loc[] = "MB/SFP2";
uchar sfp3_loc[] = "MB/SFP3";
uchar psu0_loc[] = "MB/PSU0";
uchar psu1_loc[] = "MB/PSU1";
uchar rps_loc[] = "MB/RPS";
uchar pvdm0_loc[] = "MB/PVDM0";
uchar backplane_loc[] = "MB/Backplane";
uchar risercard_loc[] = "MB/RiserCard";
uchar sm0_loc[] = "MB/SM0";
uchar sm1_loc[] = "MB/SM1";
uchar wic0_loc[] = "MB/WIC0";
uchar wic1_loc[] = "MB/WIC1";
uchar wic2_loc[] = "MB/WIC2";
uchar sm0wic_loc[] = "SM0/WIC";
uchar sm1wic_loc[] = "SM1/WIC";
uchar sm0pvdm0_loc[] = "SM0/PVDM0";
uchar sm0pvdm1_loc[] = "SM0/PVDM1";
uchar sm0pvdm2_loc[] = "SM0/PVDM2";
uchar sm1pvdm0_loc[] = "SM1/PVDM0";
uchar sm1pvdm1_loc[] = "SM1/PVDM1";
uchar sm1pvdm2_loc[] = "SM1/PVDM2";
uchar sm0wic0dc_loc[] = "SM0/WIC0/DC";
uchar sm1wic0dc_loc[] = "SM1/WIC0/DC";
uchar sm0wic1dc_loc[] = "SM0/WIC1/DC";
uchar sm1wic1dc_loc[] = "SM1/WIC1/DC";
uchar sm0dc_loc[] = "SM0/DC";
uchar sm1dc_loc[] = "SM1/DC";

fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { mb_pid,        mb_xaui_loc },
    { mb_pid,        mb_aux_loc },
    { mb_pid,        mb_i2c_loc},
    { mb_pid,        mb_fpga_reg_loc},
    { mb_pid,        mb_emmc_loc},
    { mb_pid,        mb_eusb_loc},
    { mb_pid,        mb_usb0_loc},
    { mb_pid,        mb_msata_loc},
    { mb_phy_id,     mb_phy_loc },
    { mb_sfp_id,     mb_sfp_loc },
    { dimm_pid,      dimm0_loc },
    { dimm_pid,      dimm1_loc },
    { sfp_pid,       sfp0_loc },
    { sfp_pid,       sfp1_loc },
    { sfp_pid,       sfp2_loc },
    { sfp_pid,       sfp3_loc },
    { psu_pid,       psu0_loc },
    { psu_pid,       psu1_loc },
    { rps_pid,       rps_loc },
    { pvdm_pid,      pvdm0_loc },
    { backplane_pid, backplane_loc },
    { risercard_pid, risercard_loc },
    { sm_pid,        sm0_loc },
    { sm_pid,        sm1_loc },
    { wic_pid,       wic0_loc },
    { wic_pid,       wic1_loc },
    { wic_pid,       wic2_loc },
    { wic_pid,       sm0wic_loc },
    { wic_pid,       sm1wic_loc },
    { pvdm_pid,      sm0pvdm0_loc },
    { pvdm_pid,      sm0pvdm1_loc },
    { pvdm_pid,      sm0pvdm2_loc },
    { pvdm_pid,      sm1pvdm0_loc },
    { pvdm_pid,      sm1pvdm1_loc },
    { pvdm_pid,      sm1pvdm2_loc },
    { dc_pid,        sm0wic0dc_loc },
    { dc_pid,        sm1wic0dc_loc },
    { dc_pid,        sm0wic1dc_loc },
    { dc_pid,        sm1wic1dc_loc },
    { dc_pid,        sm0dc_loc },
    { dc_pid,        sm1dc_loc },
};


/*
 * Sub Menu used for "Main menu -> motherboard test"
 */
submenu_xtable_t mb_tests_submenu_table[] = {
    {"CPU core test",
    (PFT) cpu_core_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_cpu_test_menu, FALSE},

    {"Main Memory test",
    (PFT) linux_memory_tester, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, (PFT) linux_memory_tester, TRUE},

    {"Boot flash test",
    (PFT) build_boot_flash_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL| MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, 
    (PFT) build_boot_flash_menu, TRUE},

    {"I2C scan test",
    (PFT) build_i2c_scan_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (type_t(*)()) build_i2c_scan_menu, TRUE},

    {"eMMC test",
    (PFT) build_emmc_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_emmc_test_menu, TRUE},

    {"FPGA test",
    (PFT) dash_rd_wr_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) dash_rd_wr_test, TRUE},

    {"Thermal Sensor test",
    (PFT) build_snsr_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_snsr_menu, TRUE},
    
    {"Pressure Sensor test",
    (PFT) build_ps_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_ps_menu, TRUE},

    {"External USB test",
    (PFT) diag_ext_usb_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) diag_ext_usb_test, TRUE},

    {"GE PHY 1543 Test",
    (PFT) build_gephy_1543_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0, 
    (PFT) build_gephy_1543_test_menu, TRUE},

    {"Ethernet Switch Test",
    (PFT) build_esw_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())is_nanook_plus, 0, 
    (PFT) build_esw_test_menu, TRUE},   
 
    {"M.2 Device test",
    (PFT) build_m2_test_menu, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_m2_test_menu, TRUE},

    {"LED Test",
    (PFT) diag_led_test , TRUE,
    MF_CONTINUOUS | MF_DOGRP  | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,(PFT) diag_led_test, FALSE},

    {"RTC test",
    (PFT) build_rtc_menu, TRUE,
    MF_CONTINUOUS | MF_DOGRP | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_rtc_menu, FALSE},

    {"AUX loopback test",
    (PFT)aux_loopback_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP  | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0,   0, 
    (PFT)0,   0},

    /* require 3 wire loopback connector to test modem
       control signals (RTS/CTS & DTS/DTR). */
    {"AUX RTS/CTS & DTS/DTR connectivity test",
    (PFT)aux_port_test, 0,
    MF_CONTINUOUS | MF_DOGRP  | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0,   0, 
    (PFT)0,   0},

    {"Main board ASYNC Test",
    (PFT) build_mb_async_test_menu , FALSE,
    MF_CONTINUOUS | MF_DOGRP  | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,(PFT) build_mb_async_test_menu, TRUE},

    {"Daughter card ASYNC Test",
    (PFT) build_db_async_test_menu , FALSE,
    MF_CONTINUOUS | MF_DOGRP  | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())has_daughter_card, 0,(PFT) build_db_async_test_menu, TRUE},
};

#define MB_TESTS_SUBMENU_TABLE_SIZE (sizeof(mb_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "Main menu -> motherboard test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mb_tests_primary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t mb_tests_secondary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t mb_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    mb_tests_primary_items,
};

menuinfo_t *mb_submenup = &mb_subtest_menu;


/*-------------------------------------------------------------------
 *
 * Function: mb_tests()
 *
 * First build the primary & secondary submenus for the motherboard
 * diags based on the _xtable_ mb_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int mb_tests (boolean mb_test_items_executed)
{
    int rc = FAILED;

    build_primary_submenu(mb_tests_submenu_table,
                          MB_TESTS_SUBMENU_TABLE_SIZE, "Motherboard",
                          &mb_submenup);

    build_secondary_submenu(mb_tests_submenu_table,
                            MB_TESTS_SUBMENU_TABLE_SIZE,
                            mb_tests_secondary_items);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        add_menu_item(&mb_subtest_menu, "/dev/port read",
                      (PFT)dev_port_read, (type_t *)&zero,
                      0);
        add_menu_item(&mb_subtest_menu, "/dev/port write",
                      (PFT)dev_port_write, (type_t *)&zero,
                      0);
    }

    if (mb_test_items_executed) {
        do_all_menu_items(&mb_subtest_menu);
    } else {
        menu(&mb_subtest_menu, mb_tests_secondary_items, '\0');
    }

    return (rc);
}

static void
display_uart_reg (void)
{
    /* cterr_db_print("Display UART regs:\n"); */
    display_uart_regs(0);
}

static void
add_aux_port_test_err_report (void)
{    
    fru_table_offset = MB_AUX;
    platform_fru_table[MB_AUX].pid_string = mb_pid;
    platform_fru_table[MB_AUX].location_string = mb_aux_loc;
    cterr_add_component("MB", "UART", "AUX");
    cterr_add_reg_dump((PFV)display_uart_reg);
    cterr_add_debug("Please check whether aux connector is plugged correctly",
            "Please try to replace the aux plug with another 3-wires loopback plug",
            "Please try to use uart utility to make sure uart is working");
}

static void
add_aux_loopback_err_report (void)
{
    fru_table_offset = MB_AUX;
    platform_fru_table[MB_AUX].pid_string = mb_pid;
    platform_fru_table[MB_AUX].location_string = mb_aux_loc;
    cterr_add_component("MB", "UART", "AUX");
    cterr_add_reg_dump((PFV)display_uart_reg);
    cterr_add_debug("Please check whether aux connector is plugged correctly",
            "Please try to replace the aux plug with another one",
            "Please try to use uart utility to make sure uart is working");
}

/*
 * Function:    aux_uart_write
 * Description:    write to aux uart register
 * Inputs:    offset: uart register offset and val: value to be written 
 * Output:    PASS
 */
static int
aux_uart_write(off_t offset, char val)
{   
    int fdio;
    char buf = val;
    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }
    
    assert(fdio);

    if (lseek(fdio, (offset + AUX_UART_UIO_BASE), SEEK_SET) < 0) {
        perror("can't lseek offset");
        close(fdio);
        return FAILED;
    }
    write(fdio, &buf, 1);
    usleep(100);
    close(fdio);
    return PASSED;
}

static int
_dev_port_write(off_t offset, char val)
{   
    int fdio;
    char buf = val;
    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }
    
    assert(fdio);

    if (lseek(fdio, offset, SEEK_SET) < 0) {
        perror("can't lseek offset");
        close(fdio);
        return FAILED;
    }
    write(fdio, &buf, 1);
    usleep(100);
    close(fdio);
    return PASSED;
}

static int
_dev_port_read(off_t offset, char *buf)
{   
    int fdio;
    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }

    assert(fdio);

    lseek(fdio, (offset), SEEK_SET);
    read(fdio, buf, 1);
    close(fdio);
    usleep(5000);
    return PASSED;
}

/*
 * Function:    aux_uart_read
 * Description:    read from aux uart register
 * Inputs:    offset: uart register offset and buf: value is returned to this buffer
 * Output:    PASS
 */
static int
aux_uart_read(off_t offset, char *buf)
{   
    int fdio;
    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }

    assert(fdio);

    lseek(fdio, (offset + AUX_UART_UIO_BASE), SEEK_SET);
    read(fdio, buf, 1);
    close(fdio);
    usleep(5000);
    return PASSED;
}   

/*
 * Function:    aux_loopback
 * Description:    do aux internal loopback test 
 * Inputs:    baud: set the uart baud rate
 *              int_lpbk; 1 if internal loopback is on, 0 otherwise.
 * Output:    PASS, or FAIL
 */
int 
aux_loopback (int baud, int int_lpbk)
{
    char test_str[] = "1234567890ABCDE";
    int i, ret = PASS;
    char read_buf;
    unsigned int quot;
    char recv_str[256] = {0};

    if (baud <= 0)
        return FAILED;

    quot = 50000000 / baud;

    /* aux uart config */
    aux_uart_write(UART_FCR, 0xc6); /* reset fifo */

    /* Wait for the serial port to be ready */ 
    aux_uart_read(UART_LSR, &read_buf);
    while ((read_buf & 0x60) != 0x60) {
        aux_uart_read(UART_LSR, &read_buf);
        if (i++ > 50000) {
            printf("wait port ready timeout\n");
        }
    }

    aux_uart_write(UART_LCR, 0x93); /* turn on divisor latch access bit */
    aux_uart_write(UART_DLL, (quot & 0xFF)); /* rate 9600: 0x1458. DLL: 0x58 */
    aux_uart_write(UART_DLM, (quot & 0xFF00 >> 8)); /* 9600: 0x1458 DLM: 0x14 */
    aux_uart_write(UART_LCR, 0x13); /* turn off divisor latch access bit */
    aux_uart_write(UART_FCR, 0xc1); /* enable fifo and 1 byte trigger level */

    if (int_lpbk) {
        aux_uart_write(UART_MCR, 0x10); /* turn on loopback mode */
    } else {
        aux_uart_write(UART_MCR, 0x00);
    }

    msleep(100);
    
    for (i=0; i<strlen(test_str); i++) {
        /* feed data */
        aux_uart_write(UART_TX, test_str[i]);
        msleep(10);
        /* read data */
        aux_uart_read(UART_LSR, &read_buf);
        if (read_buf & 0x1) {
            aux_uart_read(UART_RX, &read_buf);
            recv_str[i] = read_buf; 
            if (read_buf != test_str[i]) {
                ret = FAIL;
                goto test_done;
            }
        } else { 
            /* should have some more but found no more data in buf*/
            ret = FAIL;
            goto test_done;
        }
    }
    
test_done:
    aux_uart_write(UART_MCR, 0); /* turn off loopback mode */
    aux_uart_write(UART_FCR, 0xc6); /* reset fifo */
    if (ret == FAIL) {
        cterr_db_print("rx/tx string differ [rx = %s] [tx = %s].", recv_str, test_str);
    }
    return ret;
}

/*
 * Function:    aux_port_test
 * Description:    do aux port write/read test
 * Inputs: -
 * Output: PASS
 */
static int
aux_port_test (void)
{
    off_t wr_off = 0x2FC; /* write offset */
    off_t rd_off = 0x2FE; /* read offset  */
    char wr_comp, rd_comp;
    char wr_buf, rd_buf, data;
    int ix, ret = FAILED;
    int rd_shift_bit, wr_shift_bit;
    int fdio;

    char *tname = "AUX port test";

    if (get_enhance_err_flag()) {
        add_aux_port_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off or force skip usb is enabled. ");
        return(PASSED);
    }

    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }

    assert(fdio);

    /* 
     * Note: delay after read/write is required for the value to be updated
     */
    for (ix = 0; ix < 0x2; ix++) { /* test value: 00 and 11 */
        wr_buf = 0;
        rd_buf = 0;

        lseek(fdio, rd_off, SEEK_SET);
        read(fdio, &rd_buf, 1);
        usleep(5000);
        /*printf("rd buf before write: 0x%X\n", rd_buf);*/
        
        lseek(fdio, wr_off, SEEK_SET);
        read(fdio, &wr_buf, 1);
        usleep(5000);
        /* printf("wr buf before write: 0x%X\n", wr_buf); */
    
        /* aux port test only modify bit#0 and/or bit#1 */
        data = 0x03; /* modify both bit#0 and bit#1 */
        wr_buf  = (ix == 0) ?  wr_buf & ~(data) : wr_buf | data; 
        /* printf("wr buf to be written: 0x%X\n", wr_buf); */

        lseek(fdio, wr_off, SEEK_SET);
        write(fdio, &wr_buf, 1);
        usleep(5000);

        lseek(fdio, rd_off, SEEK_SET);
        read(fdio, &rd_buf, 1);
        usleep(5000);
        /* printf("rd buf after write: 0x%X\n", rd_buf); */
    
        /* 
         * wr_shift_bit:
         * if bit#0 and bit#1 are written it should be shifted by 0,
         * if only bit#0 is written, it should be shifted by 0,
         * if only bit#1 is written, it should be shifted by 1.
         */
        wr_shift_bit = 0;
        wr_comp = (wr_buf & (data << wr_shift_bit)) >> wr_shift_bit;
        /* 
         * rd_shift_bit:
         * if bit#0 and bit#1 are written it should be shifted by 4,
         * if only bit#0 is written, it should be shifted by 5,
         * if only bit#1 is written, it should be shifted by 4.
         */
        rd_shift_bit = 4;
        rd_comp = (rd_buf & (data << rd_shift_bit)) >> rd_shift_bit;
        /* printf(" wr %d, rd %d\n", wr_comp, rd_comp); */

        if (wr_comp == rd_comp) {
            ret = PASSED;
        } else {
            ret = FAILED;
            cterr('f', 0, "Read data not match [wr byte= 0x%02X, rd byte = 0x%02X, wr_comp = 0x%02X, , rd_comp = 0x%02X]. Test failed! Is 3-wires loopback connector installed?\n", wr_buf, rd_buf, wr_comp, rd_comp);
            goto fun_end;
        }

    }

fun_end:
    close(fdio);

    if (ret == PASSED) {
        cterr_db_print("Test passed!");
    } else {
        cterr('f', 0, "Read data not match [wr byte= 0x%02X, rd byte = 0x%02X]. Test failed! Is 3-wires loopback connector installed?\n", wr_buf, rd_buf);
    }
    return ret;
}

/*
 * Function: aux_loopback_test
 *
 * Description : Aux loopback test. support external only . no  internal
 *               lpbbk.
 * Inputs: dummy - not used
 *
 * Output: PASSED/FAILED
 */
int
aux_loopback_test (int dummy)
{
    char *tname = "AUX port loopback";

    if (get_enhance_err_flag()) {
        add_aux_loopback_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    
    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        if (aux_loopback(9600, 1) != PASSED) {
            cterr('f', 0, "AUX internal loopback failed.");
            return(FAILED);
        }
        return(PASSED);
    }

    if (aux_loopback(9600, 0) != PASSED) {
        cterr('f', 0, "AUX external failed. Is loopback connector installed?");
        return(FAILED);
    }
    
#ifdef UART_INTF_TEST
    if (uart_intf_test("/dev/ttyS1", NULL, B9600) != PASSED) {
        cterr('f', 0, "AUX loopback failed. Is loopback connector installed?");
        return(FAILED);
    }
#endif
    return(PASSED);
}

int
dev_port_read (int dummy)
{
    unsigned int addr;
    char read_buf;
    
    
    addr = gethex_answer("Enter Address", 0x0, 1, 0xFFFF);
    _dev_port_read(addr, &read_buf);
    printf("Read  %x : %x\n", addr, read_buf);

    return(PASSED);
}

int
dev_port_write (int dummy)
{
    unsigned int addr, val;    
    
    addr = gethex_answer("Enter Address", 0x0, 1, 0xFFFF);
    val = gethex_answer("Enter value", 0x0, 0, 0xFF);
    _dev_port_write(addr, val);
    printf("Write %x : %x\n", addr, val);

    return(PASSED);
}

/*-------------------------------------------------
 * $Log: mb_tests.c,v $
 * Revision 1.2  2019/12/11 10:10:32  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

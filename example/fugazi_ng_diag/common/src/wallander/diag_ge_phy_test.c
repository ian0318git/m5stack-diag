/* $Id: diag_ge_phy_test.c,v 1.3 2015/07/14 08:12:37 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_ge_phy_test.c,v $ 
 *-----------------------------------------------------------------------------
 * diag_ge_phy_test.c - Wallander GE PHY Test Menu
 *
 * Apr 2014, Xiaoying Zhang
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>        /* for bzero*/
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <features.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>  /* struct ethtool */
#include <linux/sockios.h>  /* SIOCETHTOOL */
#include <sys/types.h>      /* getpid */
#include <unistd.h>         /* getpid */
#include <netinet/in.h>     /* for including the linux_eth.h */
#include <ifaddrs.h>        /* for using getifaddrs */
#include <net/if.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "nvsysvars.h"
// #include "nvmonvars.h"
#include "proto.h"
#include "error.h"
#include "monitor.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "router_if.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "cvmx.h"
#include "cvmx-mdio.h"
#include "cvmx-pko.h"
#include "platform_eth.h"
#include "diag_fpga_lib.h"
#include "diag_vtss_phy.h"
#include "diag_ge_phy.h"
#include "diag_common_drv.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)


static int phy_reg_test_all_pages(void);
static int phy_id_check(void);

extern vsc_phy_regs_t vsc8584_phy_reg_tbl[6];
extern vsc_phy_regs_t vsc8552_phy_reg_tbl[5];

static char *buf_p;
static char err_msg[500];

static int mode_internal = 0;
static int mode_ext_sfp = 0;
static int mode_ext_cu = 0;

/* packet buffer */
unsigned char tx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet_sec[ETH_PKT_MAX_LEN];
unsigned char global_pkt_array[ETH_PKT_MAX_LEN*2+1];

/* Packets to be used in xaui port loopback tests
 * we leaave 12 byte for put mac address into the packet
 */
static pktdata_info_t pktdata[] = {
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 5},
  {0xa7, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 5},
  {0xa5, ((ETH_UDP_DATA_MAX_LEN - 1) - 12), H_INCFILL, 5},
  {0xa3, (ETH_UDP_DATA_MAX_LEN - 12), H_INCFILL, 5},
};

/* PTP packets to be used in ge port loopback tests
 * we leave 12 byte for put mac address into the packet
 */
static pktdata_info_t ptp_pktdata[] = {
  {0xa0, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
  {0xa7, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
  {0xa5, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
  {0xa3, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
};

sem_t rx_ready, rx_finish, tx_cmp;

mac_addr_t mac_da = {0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
mac_addr_t mac_sa = {0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

uchar ip_da[4][4] = {
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 0, 1 },
};

/* Sub Menu used for GE PHY tests.*/
static submenu_xtable_t ge_phy_tests_submenu_table[] = {
    {"PHY Register Test", (type_t(*)()) phy_id_check,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY Internal Loopback Test", (type_t(*)()) phy_int_lpbk_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY SFP External Loopback Test", (type_t(*)()) phy_sfp_ext_lpbk_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY Copper External Loopback Test", (type_t(*)()) phy_cu_ext_lpbk_test,   0,
        MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Backplane Loopback Test", (type_t(*)()) phy_bp_ext_lpbk_test,   0,
        MF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"1588 Loopback Test", (type_t(*)()) phy_ptp_ext_lpbk_test,   0,
        MF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY Packet Loopback Test", (type_t(*)()) phy_lpbk_test,   0,
        MF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PHY Loopback Config", (type_t(*)()) phy_lpbk_config,   0,
        MF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GE_PHY_FPGA_TESTS_SUBMENU_TABLE_SIZE (sizeof(ge_phy_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ge_phy_tests_primary_items[GE_PHY_FPGA_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t ge_phy_tests_secondary_items[GE_PHY_FPGA_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

menuinfo_t ge_phy_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ge_phy_tests_primary_items,
};
menuinfo_t *ge_phy_submenup = &ge_phy_subtest_menu;

/*
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 */

int ge_phy_test (int show_menu)
{
    build_primary_submenu(ge_phy_tests_submenu_table,
                          GE_PHY_FPGA_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY", &ge_phy_submenup);
    build_secondary_submenu(ge_phy_tests_submenu_table,
                            GE_PHY_FPGA_TESTS_SUBMENU_TABLE_SIZE,
                            ge_phy_tests_secondary_items);

    if (show_menu) {
        menu(ge_phy_submenup, ge_phy_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(ge_phy_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : ge_phy_do_all_wrapper
 * Description : Wrapper for PHY do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int ge_phy_do_all_wrapper (void)
{
    if (phy_int_lpbk_test()) {
        return (FAILED);
    }

    if (phy_sfp_ext_lpbk_test()) {
        return (FAILED);
    }

    if (phy_cu_ext_lpbk_test()) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: phy_register_tests
 *
 * For each register from reg_ptr, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : interface structure pointer, info for all registers
 *
 * Output: PASS/FAIL
 *
 *******************************************************************************
 */
static int phy_register_tests (int port,
                               ushort page,
                               reg_info_t *reg_ptr )
{
    uint32_t i;
    int retval = PASSED;
    int retval2 =  PASSED;
    ushort save_val, readval;
    ushort data, temp;
    ushort tst_offset;
    uint bus_no = SMI_BUS_0;

    readval = 0;

    while (reg_ptr->size.size != 0) {
        retval = wallander_phy_reg_page_rd(bus_no, port, page, reg_ptr->offset, &save_val);
        if (retval == FAILED) {
            buf_p += sprintf(buf_p, "%s(): Error reading %s register "
                                    "offset %#x", __FUNCTION__, 
                                     (char *)reg_ptr->name, reg_ptr->offset);
            return (FAILED);
        }

        if (reg_ptr->type == READ_WRITE) {
            tst_offset = reg_ptr->offset;

            /* 
             * ripple 1 test
             */
            for (i = 0; i < (reg_ptr->size.size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }

                /* Write to register under test */
                retval = wallander_phy_reg_page_wr(bus_no, port, page, tst_offset, temp);
                /* Read back */
                readval = 0;
                if (retval == PASSED) {
                    retval2 = wallander_phy_reg_page_rd(bus_no, port, page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (retval2 == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Ripple one test "
                                            "failed when accessing %s "
                                            "Register offset %#x, "
                                            " Expect %#x, Read %#x",
                                             __FUNCTION__, (char *)reg_ptr->name,
                                             tst_offset, temp, 
                                             readval);
                    return (FAILED);
                }
            }

            /* 
             * ripple 0 test
             */
            for (i = 0; i < (reg_ptr->size.size * 8); i++) {
                temp = (1 << i) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                temp = (~(1 << i)) & reg_ptr->mask;
                /* Write to register under test */
                readval = 0xFFFF;
                retval = wallander_phy_reg_page_wr(bus_no, port, page, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    retval2 = wallander_phy_reg_page_rd(bus_no, port, page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (retval2 == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Ripple one test "
                                            "failed when accessing %s "
                                            "Register offset %#x, "
                                            "Expect %#x, Read %#x",
                                             __FUNCTION__, (char *)reg_ptr->name,
                                             tst_offset, temp,
                                             readval);
                    return (retval);
                }
            }

            /*
             * pattern test
             */
            data = (ushort)PATTERN;
            for (i = 0; i < 2; i++) {
                temp = data &reg_ptr->mask;
                /* Write to register under test */
                retval = wallander_phy_reg_page_wr(bus_no, port, page, tst_offset, temp);
                if (retval == PASSED) {
                    /* Read back */
                    retval2 = wallander_phy_reg_page_rd(bus_no, port, page, tst_offset, &readval);
                }
                if (((readval & reg_ptr->mask) != temp) ||
                    (retval == FAILED) || (retval2 == FAILED)) {
                    buf_p += sprintf(buf_p, "%s(): Pattern test failed "
                                            "when accessing %s Register "
                                            "offset %#x Expect %#x, "
                                            "Read %#x", __FUNCTION__,
                                             (char *)reg_ptr->name, tst_offset, 
                                             temp, readval);
                    return (retval);
                }

                data = (ushort)~PATTERN; /* complement data pattern */
            }

            /*
             * restore original value
             */
            retval = wallander_phy_reg_page_wr(bus_no, port,page, tst_offset, save_val);
            if (retval == FAILED) {
                buf_p += sprintf(buf_p, "%s(): Error restoring %s register "
                                        "offset %#x\n", __FUNCTION__, 
                                        (char *)reg_ptr->name, reg_ptr->offset);
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_register_test
 *
 * Description: This function performs the GE PHY register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int phy_reg_test_all_pages (void)
{
    int port_num;
    uint page_num;
    uint page_cur;
    reg_info_t *reg_ptr;
    vsc_phy_regs_t *page_reg_ptr = NULL;
    int i, j;

    testname("GE PHY Register");

    port_num = get_num_ports();

    buf_p = err_msg;
    for (i = 0; i < port_num; i++) {
        printf ("\n Testing Port %d\n", i);
        if (port_num == 4) {
            page_reg_ptr = &vsc8584_phy_reg_tbl[0];
            page_num = 6;
        } else {
            page_reg_ptr = &vsc8552_phy_reg_tbl[0];
            page_num = 5;
        }

        for (j = 0; j < page_num; j++) {
            printf(" - Testing %s\n", page_reg_ptr->pagename);
            page_cur = page_reg_ptr->pagenum;
            reg_ptr = page_reg_ptr->pageregs;

            if (phy_register_tests(i, page_cur, reg_ptr) != PASSED) {
                printf("%s\n", err_msg);
                cterr('f', 0, "PHY register test failed on page %d.", page_cur);
                return (FAILED);
            }
            page_reg_ptr++;
        }
    }

    return (PASSED);
}


int get_num_ports ( void )
{
    if (get_board_id() == 1)
        return 4;
    else
        return 2;
}

/******************************************************************************
 *
 * Function: phy_id_check
 *
 * Description: This function checks the PHY ID.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int phy_id_check (void)
{
    int port_num;
    uint bus_no = SMI_BUS_0;
    ushort reg_data;
    ulong phy_id;
    ulong phy_id_rd;
    int i;

    testname("GE PHY ID Check");

    if (get_board_id() == 1) {
        /* 2-port SKU uses VSC8584 */
        phy_id = VTSS_PHY_ID_VSC8584;
    } else {
        /* 1-port SKU uses VSC8552 */
        phy_id = VTSS_PHY_ID_VSC8552;
    }

    port_num = get_num_ports();

    for (i = 0; i < port_num; i++) {
        int page = VSC85XX_PHY_EXT_REG_PAGE_0;
        int offset = VSC85XX_1G_PHY_ID_1_REGISTER;
        if (wallander_phy_reg_page_rd(bus_no, i, page, offset, &reg_data)) {
            cterr('f', 0, "Failed to read PHY reg in Port %d Page %d Offset %d.",
                    i, page, offset);
            return (FAILED);
        } else {
//             printf("\n Register value %#x ", (unsigned int)reg_data);
        }

        phy_id_rd = reg_data << 16;

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = VSC85XX_1G_PHY_ID_2_REGISTER;
        if (wallander_phy_reg_page_rd(bus_no, i, page, offset, &reg_data)) {
            cterr('f', 0, "Failed to read PHY reg in Port %d Page %d Offset %d.",
                    i, page, offset);
            return (FAILED);
        } else {
//             printf("\n Register value %#x ", (unsigned int)reg_data);
        }

        phy_id_rd |= reg_data;

        if ((phy_id_rd & VTSS_PHY_ID_MASK) != phy_id) {
            cterr('f', 0, "%s(): Phy ID check failed, Expected %#x, Read %#x\n",
                  __FUNCTION__, phy_id, phy_id_rd);
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_reg_port_page_dump
 *
 * Description: Dump PHY registers of certain page.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_reg_port_page_dump (int port_no, ushort page)
{
    int j;
    ushort data;
    ushort data2;
    ushort cur_page;
    int port_num;
    uint page_num;
    uint bus_no = SMI_BUS_0;
    reg_info_t *reg_ptr;
    vsc_phy_regs_t *page_reg_ptr = NULL;
    vsc_phy_regs_t *page_reg_ptr_start = NULL;

    port_num = get_num_ports();

    if (port_num == 4) {
        page_reg_ptr_start = &vsc8584_phy_reg_tbl[0];
        page_num = 6;
    } else {
        page_reg_ptr_start = &vsc8552_phy_reg_tbl[0];
        page_num = 5;
    }

    assert(page < page_num);

    page_reg_ptr = page_reg_ptr_start + page;
    reg_ptr = page_reg_ptr->pageregs;

    printf("\n%s\n", page_reg_ptr->pagename);
    while (reg_ptr->size.size != 0) {
        if (page == 0) {
            /* For Page 0, read the register twice to flush any latch-on-event values */
            if ( wallander_phy_reg_page_rd(bus_no, port_no, page, reg_ptr->offset, &data)) {
                cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                    page, reg_ptr->offset);
                return (FAILED);
            }
            if ( wallander_phy_reg_page_rd(bus_no, port_no, page, reg_ptr->offset, &data2)) {
                cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                    page, reg_ptr->offset);
                return (FAILED);
            }
            printf("%-32s reg %.2d = %#.4x\t %#.4x\n", reg_ptr->name, 
                                            reg_ptr->offset, 
                                            data, data2);
        } else {
            if ( wallander_phy_reg_page_rd(bus_no, port_no, page, reg_ptr->offset, &data)) {
                cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                    page, reg_ptr->offset);
                return (FAILED);
            }
            printf("%-32s reg %.2d = %#.4x\n", reg_ptr->name, 
                                            reg_ptr->offset, 
                                            data);
        }
        reg_ptr++;
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_reg_rd_util
 *
 * Description: Utility for users to read PHY registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_reg_rd_util()
{
    ushort offset;
    ushort page;
    ushort reg_data;
    uint bus_no = SMI_BUS_0;
    uint port_no = 0;

    if (get_num_ports() == 4) {
        port_no = getdec_answer("\nEnter port number[0 to 3]:",
                0, 0, 3);
    } else {
        port_no = getdec_answer("\nEnter port number[0 to 1]:",
                0, 0, 1);
    }

    page = getdec_answer("\nEnter register page[0 to 16]:",
               0, 0, 16);
    offset = getdec_answer("\nEnter register offset[0 to 31]:",
               0, 0, 31);

    if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
        printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                port_no, page, offset);
        return (FAILED);
    } else {
        printf("\n Register value %#x ", (unsigned int)reg_data);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: phy_reg_wr_util
 *
 * Description: Utility for users to write PHY registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_reg_wr_util()
{
    ushort page;
    ushort offset;
    ushort reg_data;
    uint bus_no = SMI_BUS_0;
    uint port_no;

    if (get_num_ports() == 4) {
        port_no = getdec_answer("\nEnter port number[0 to 3]:",
                0, 0, 3);
    } else {
        port_no = getdec_answer("\nEnter port number[0 to 1]:",
                0, 0, 1);
    }

    page = getdec_answer("\nEnter register page[0 to 16]:",
           0, 0, 16);
    offset = getdec_answer("\nEnter register offset[0 to 31]:",
             0, 0, 31);

    if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
        printf("Failed to read PHY reg in Port %d Page %d Offset %d.\n", 
                port_no, page, offset);
        return (FAILED);
    } else {
        printf("\n Original Register value of Port %d Page %d Offset %d is %#x\n", 
                port_no, page, offset, reg_data);
    }
    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFF]:",
               0, 0, 0xffff);

    if (wallander_phy_reg_page_wr(bus_no, port_no, page, offset, reg_data)) {
        printf("Failed to write PHY reg in Port %d Page %d Offset %d.", 
                port_no, page, offset);
        return (FAILED);
    } else {
        printf("\n Write Register value %#x to Port %d Page %d Offset %d", 
                reg_data, port_no, page, offset);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: phy_reg_rd_util
 *
 * Description: Utility for users to dump PHY registers of certain port.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_reg_dp_util()
{
    int j;
    int port_num;
    uint page_num;
    int port_no;

    port_num = get_num_ports();

    if (port_num == 4) {
        page_num = 6;
        port_no = getdec_answer("\nEnter port number[0 to 3]:",
                0, 0, 3);
    } else {
        page_num = 5;
        port_no = getdec_answer("\nEnter port number[0 to 1]:",
                0, 0, 1);
    }

    printf("PHY Register Dump\n");
    printf ("\n Port %d\n", port_no);
    for (j = 0; j < page_num; j++) {
        phy_reg_port_page_dump(port_no, j);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: show_cpu_port_ctrl_stat
 *
 * Description: Display CPU PCS control and status registers.
 *
 * Inputs      : port index
 * Outputs     : None.
 *
 *****************************************************************************/
void show_cpu_port_ctrl_stat(int port)
{
    printf("CVMX_GMXX_PRTX_CFG:\t%#llx\n", 
        (long long unsigned int)cvmx_read_csr(CVMX_GMXX_PRTX_CFG(port, 1)));
    printf("PCSX_MRX_CONTROL_REG:\t%#llx\n", 
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(port, 1)));
    printf("PCSX_MRX_STATUS_REG: \t%#llx\t", 
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(port, 1)));
    printf("PCSX_MRX_STATUS_REG: \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(port, 1)));
    printf("PCSX_RXX_SYNC_REG: \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_RXX_SYNC_REG(port, 1)));
    printf("CVMX_PCSX_RXX_STATES_REG: \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_RXX_STATES_REG(port, 1)));
    printf("CVMX_PCSX_TXX_STATES_REG: \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_TXX_STATES_REG(port, 1)));
    printf("PCSX_INTX_EN_REG: \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_INTX_EN_REG(port, 1)));
    printf("PCSX_INTX_REG:    \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_INTX_REG(port, 1)));

    // Clear PCS interrupt - write 1 to clear
    cvmx_write_csr(CVMX_PCSX_INTX_REG(port, 1), cvmx_read_csr(CVMX_PCSX_INTX_REG(port, 1)));
    printf("PCSX_INTX_REG:    \t%#llx\n",
        (long long unsigned int)cvmx_read_csr(CVMX_PCSX_INTX_REG(port, 1)));
}

/******************************************************************************
 *
 * Function: show_port_ctrl_stat_util
 *
 * Description: Utility to display frequently used control and status registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int show_port_ctrl_stat_util()
{
    int i;
    ushort data;
    ushort page;
    ushort offset;
    int port_num;
    uint bus_no = SMI_BUS_0;

    port_num = get_num_ports();

    for (i = 0; i < port_num; i++) {
        printf ("\n Port %d\n", i);
        show_cpu_port_ctrl_stat(i);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 0;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mode Control            @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 1;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mode Status             @ page%d reg%d = %#.4x\n", page, offset, data);
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mode Status             @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 23;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Extended PHY Ctrl 1     @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_1;
        offset = 18;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Cu Media CRC Good Cnt   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_1;
        offset = 19;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Extended Mode Ctrl      @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 17;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mac SerDes PCS Status   @ page%d reg%d = %#.4x\n", page, offset, data);
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mac SerDes PCS Status   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 20;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mac Serdes Status       @ page%d reg%d = %#.4x\n", page, offset, data);
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Mac Serdes Status       @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 24;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Media SerDes PCS Status @ page%d reg%d = %#.4x\n", page, offset, data);
        offset = 24;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Media SerDes PCS Status @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 21;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Media/MAC Tx Good Cnt   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 28;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("Media/MAC Rx Good Cnt   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_16;
        offset = 14;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
        printf("GPIO Ctrl 2             @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 1;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: save_port_ctrl_stat
 *
 * Description: Save frequently used control and status registers dump to file.
 *
 * Inputs      : fp - File pointer
                 desc - brief description on current state
 * Outputs     : None.
 *
 *****************************************************************************/
int save_port_ctrl_stat(FILE *fp, char *desc)
{
    int i;
    ushort data;
    ushort page;
    ushort offset;
    int port_num;
    uint bus_no = SMI_BUS_0;

    port_num = get_num_ports();

    fprintf(fp, "\n==== %s\n", desc);


    for (i = 0; i < port_num; i++) {
        fprintf(fp, "\n Port %d\n", i);
        fprintf(fp, "CVMX_GMXX_PRTX_CFG:\t%#llx\n", 
            (long long unsigned int)cvmx_read_csr(CVMX_GMXX_PRTX_CFG(i, 1)));
        fprintf(fp, "PCSX_MRX_CONTROL_REG:\t%#llx\n", 
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(i, 1)));
        fprintf(fp, "PCSX_MRX_STATUS_REG: \t%#llx\t", 
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(i, 1)));
        fprintf(fp, "PCSX_MRX_STATUS_REG: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(i, 1)));
        fprintf(fp, "PCSX_RXX_STATES_REG: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_RXX_STATES_REG(i, 1)));
        fprintf(fp, "PCSX_TXX_STATES_REG: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_TXX_STATES_REG(i, 1)));
        fprintf(fp, "PCSX_RXX_SYNC_REG: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_RXX_SYNC_REG(i, 1)));
        fprintf(fp, "GMXX_RXX_STATS_PKTS: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_GMXX_RXX_STATS_PKTS_CTL(i, 1)));
        fprintf(fp, "GMXX_TXX_STAT3: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_GMXX_TXX_STAT3(i, 1)));
        fprintf(fp, "PCSX_INTX_EN_REG: \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_INTX_EN_REG(i, 1)));
        fprintf(fp, "PCSX_INTX_REG:    \t%#llx\t",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_INTX_REG(i, 1)));

        // Clear PCS interrupt - write 1 to clear
        cvmx_write_csr(CVMX_PCSX_INTX_REG(i, 1), cvmx_read_csr(CVMX_PCSX_INTX_REG(i, 1)));
        fprintf(fp, "PCSX_INTX_REG:    \t%#llx\n",
            (long long unsigned int)cvmx_read_csr(CVMX_PCSX_INTX_REG(i, 1)));
#if 1
        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 0;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Mode Control            @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 1;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Mode Status             @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 23;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Extended PHY Ctrl 1     @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_1;
        offset = 18;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Cu Media CRC Good Cnt   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_1;
        offset = 19;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Extended Mode Ctrl      @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 17;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Mac SerDes PCS Status   @ page%d reg%d = %#.4x\n", page, offset, data);
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Mac SerDes PCS Status   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 20;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Mac Serdes Status       @ page%d reg%d = %#.4x\n", page, offset, data);
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Mac Serdes Status       @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 24;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Media SerDes PCS Status @ page%d reg%d = %#.4x\n", page, offset, data);
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Media SerDes PCS Status @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 21;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Media/MAC Tx Good Cnt   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_3;
        offset = 28;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "Media/MAC Rx Good Cnt   @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_16;
        offset = 14;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
        fprintf(fp, "GPIO Ctrl 2             @ page%d reg%d = %#.4x\n", page, offset, data);

        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = 1;
        if ( wallander_phy_reg_page_rd(bus_no, i, page, offset, &data)) {
/*            cterr('f', 0, "Failed to read PHY reg in Page %d Offset %d.", 
                page, offset);*/
            return (FAILED);
        }
#endif
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: smi_ctrl_reg_dp_util
 *
 * Description: Utility to displays Cavium SMI register settings
 *
 * Inputs      : None
 * Outputs     : PASSED.
 *
 *****************************************************************************/
int smi_ctrl_reg_dp_util (void)
{
    int bus_id;
    cvmx_smix_rd_dat_t smi_rd;
    cvmx_smix_clk_t smi_clk;
    cvmx_smix_cmd_t smi_cmd;
    cvmx_smix_wr_dat_t smi_wr;
    cvmx_smix_en_t smi_en;

    printf("SMI Register Display:");

    for (bus_id = 0; bus_id < 1; bus_id++) {
        printf("\nSMI Bus-%d:\n", bus_id);
        smi_rd.u64 = cvmx_read_csr(CVMX_SMIX_RD_DAT(bus_id));
        smi_wr.u64 = cvmx_read_csr(CVMX_SMIX_WR_DAT(bus_id));
        smi_clk.u64 = cvmx_read_csr(CVMX_SMIX_CLK(bus_id));
        smi_en.u64 = cvmx_read_csr(CVMX_SMIX_EN(bus_id));
        smi_cmd.u64 = cvmx_read_csr(CVMX_SMIX_CMD(bus_id));

        printf(" Enable       : %#lx\n", smi_en.u64);
        printf(" Clock        : %#lx\n", smi_clk.u64);
        printf(" Command      : %#lx\n", smi_cmd.u64);
        printf(" Write        : %#lx\n", smi_wr.u64);
        printf(" Read         : %#lx\n", smi_rd.u64);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: show_phy_test_log_util
 *
 * Description: Utility to show the status log saved during the loopback test.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int show_phy_test_log_util()
{
    int mode = 0;
    mode = getdec_answer("\nTest(0 - Internal; 1 - SFP External; " \
                         "2 - Copper External; 3 - Backplane; 4 - 1588):",
                         0, 0, 4);

    switch (mode) {
    case 0:
        system("cat /tmp/int_lpbk_reg_dump");
        break;
    case 1:
        system("cat /tmp/sfp_lpbk_reg_dump");
        break;
    case 2:
        system("cat /tmp/cu_lpbk_reg_dump");
        break;
    case 3:
        system("cat /tmp/bp_lpbk_reg_dump");
        break;
    case 4:
        system("cat /tmp/ptp_lpbk_reg_dump");
        break;
    default:
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: show_1588_blocks
 *
 * Description: Dump 1588 blocks.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
void show_1588_blocks()
{
    printf(     "0 - Analyzer 0 Ingress\n"
            "\t\t1 - Analyzer 0 Egress\n"
            "\t\t2 - Analyzer 1 Ingress\n"
            "\t\t3 - Analyzer 1 Egress\n"
            "\t\t4 - Analyzer 2 Ingress\n"
            "\t\t5 - Analyzer 2 Egress\n"
            "\t\t6 - Processor 0\n"
            "\t\t7 - Processor 1 ");
}

int phy_1588_reg_rd_util()
{
    u32 offset;
    u32 block;
    u32 reg_data;
    u32 port_no;

    if (get_board_id() != 1) {
        printf("This utility is for NIM-2GE-CU-SFP only.\n");
        return (PASSED);
    }

    port_no = getdec_answer("\nEnter port number[0 to 3]:",
            0, 0, 3);

    show_1588_blocks();
    block = getdec_answer("\nEnter Block ID:",
               0, 0, 7);
    offset = getdec_answer("\nEnter register offset[0 to 31]:",
               0, 0, 31);

    if (vsc_1588_reg_access(port_no, 0, block, offset, &reg_data)) {
        printf("Failed to read PHY 1588 reg in Port %d Block %d Offset %d.", 
                port_no, block, offset);
        return (FAILED);
    } else {
        printf("\n Register value %#x ", reg_data);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: phy_1588_reg_wr_util
 *
 * Description: Utility to write 1588 registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_1588_reg_wr_util()
{
    u32 offset;
    u32 block;
    u32 reg_data;
    u32 port_no;

    if (get_board_id() != 1) {
        printf("This utility is for NIM-2GE-CU-SFP only.\n");
        return (PASSED);
    }

    port_no = getdec_answer("\nEnter port number[0 to 3]:",
            0, 0, 3);

    show_1588_blocks();
    block = getdec_answer("\nEnter Block ID:",
               0, 0, 7);
    offset = getdec_answer("\nEnter register offset[0 to 31]:",
               0, 0, 31);

    if (vsc_1588_reg_access(port_no, 1, block, offset, &reg_data)) {
        printf("Failed to read PHY 1588 reg in Port %d Block %d Offset %d.", 
                port_no, block, offset);
        return (FAILED);
    } else {
        printf("\n Write Register value %#x to Port %d Block %d Offset %d", 
                reg_data, port_no, block, offset);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: phy_1588_reg_dp_util
 *
 * Description: Utility to dump 1588 registers.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_1588_reg_dp_util()
{
    int i;
    int port_num;

    if (get_board_id() != 1) {
        printf("This utility is for NIM-2GE-CU-SFP only.\n");
        return (PASSED);
    }

    port_num = get_num_ports();

    printf("PHY 1588 Register Dump\n");

    for (i = 0; i < port_num; i++) {
        printf ("\n Port %d\n", i);
        if (vsc_1588_reg_dump(i)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_1588_tod_util
 *
 * Description: Utility to read/write 1588 Tod.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_1588_tod_util (void)
{
    int port_no;
    int mode;
    u16 sec_hi = 0;
    u32 sec_lo = 0;
    u32 sec_ns = 0;

    if (get_board_id() != 1) {
        printf("This utility is for NIM-2GE-CU-SFP only.\n");
        return (PASSED);
    }

    port_no = getdec_answer("\nEnter port number[0 to 3]:",
            0, 0, 3);

    mode = getdec_answer("\nEnter mode: 0 - Read; 1 - Write; 2 - Arm; 3 - Done:",
            0, 0, 3);

    if (mode == 1) {
        sec_hi = gethex_answer("\nTOD Hi: ",
               0, 0, 0xffff);
        sec_lo = gethex_answer("\nTOD Lo: ",
               0, 0, 0xffffffff);
        sec_ns = gethex_answer("\nTOD nsec: ",
               0, 0, 0xffffffff);
    }

    return vsc_1588_tod_access(port_no, mode, sec_hi, sec_lo, sec_ns);
}

/******************************************************************************
 *
 * Function: phy_1588_stat_util
 *
 * Description: Utility to show 1588 status.
 *
 * Inputs      : None
 * Outputs     : None.
 *
 *****************************************************************************/
int phy_1588_stat_util (void)
{
    int i;
    int port_num;

    if (get_board_id() != 1) {
        printf("This utility is for NIM-2GE-CU-SFP only.\n");
        return (PASSED);
    }
    port_num = get_num_ports();

    printf(" ingr      egr port  FIFO      FIFO     ingr       egr        ingr     egr  \n\r" );
    printf(" mod cnt   mod cnt   tx        drop     shrink err shrink err FCS err  FCS err \n\r" );

    for (i = 0; i < port_num; i++) {
        if (vsc_phy_1588_stats(i)) {
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *  
 * Function    : setup_phy_test_mode_util
 *
 * Description : Utility to set up PHY GE test mode
 *
 * Inputs      : None
 *
 * Outputs     : PASSED / FAILED
 *
 **********************************************************************************/
int setup_phy_test_mode_util (void)
{
    ushort page = VSC85XX_PHY_EXT_REG_PAGE_0;
    ushort offset = VSC85XX_TEST_MODE_REG;
    ushort reg_data, test_mode;
    uint bus_no = SMI_BUS_0;
    uint port_no = 0;
    int mode;

    printf("Enter phy test modes.\n");
    printf("Enter 0 - Normal Mode\n");
    printf("Enter 1 - Transmit Waveform Test\n");
    printf("Enter 2 - Transmit Jitter Test Master Mode\n");
    printf("Enter 3 - Transmit Jitter Test Slave Mode\n");
    printf("Enter 4 - Transmit Distortion Test\n");

    /* For 1-port SKU, the port index could only be 0;
     * For 2-port SKU, users can choose port 0 or 2.  */
    if (get_num_ports() == 4) {
        port_no = getdec_answer("\nEnter port number[0/2]:",
                0, 0, 2);
    }

    /* Only applies to front panel ports */
    if ((port_no != 0) && (port_no != 2)) {
        printf("Invalid port index. Only applies to front panel ports.\n");
        return (FAILED);
    }

    mode = gethex_answer("Select Test Modes[0]: ", 0, 0, 0x4);
    switch (mode) {
        case 0:
            printf("Normal Mode\n");
            test_mode = VSC85XX_NORMAL_MODE;
            break;
        case 1:
            printf("Test Mode 1 - Transmit Waveform Test\n");
            test_mode = VSC85XX_TEST_MODE_1;
            break;
        case 2:
            printf("Test Mode 2 - Transmit Jitter Test(Master Mode)\n");
            test_mode = VSC85XX_TEST_MODE_2;
            break;
        case 3:
            printf("Test Mode 3 - Transmit Jitter Test(Slave Mode)\n");
            test_mode = VSC85XX_TEST_MODE_3;
            break;
        case 4:
            printf("Test Mode 4 - Transmit Distortion Test\n");
            test_mode = VSC85XX_TEST_MODE_4;
            break;
        default :
            printf("Not support this test mode\n");
            return (FAILED);
    }

    /* If current mode is not Copper, configure the PHY */
    if (mode_ext_cu == 0) {
        if (phy_cu_ext_lpbk_config()){
            cterr('f', 0, "PHY Copper External Loopback Initialize Failed.");
            return (FAILED);
        }
    }
    /* Now the PHY should be in Copper mode */
    mode_ext_cu = 1;
    mode_ext_sfp = 0;
    mode_internal = 0;

    /* Switch Mode */
    if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
        cterr('f', 0, "Failed to read PHY reg in Port %d Page %d Offset %d.",
                port_no, page, offset);
        return (FAILED);
    }

    reg_data &= ~(VSC85XX_TEST_MODE_MASK);
    reg_data |= test_mode;
    if (wallander_phy_reg_page_wr(bus_no, port_no, page, offset, reg_data)) {
        cterr('f', 0, "Failed to write PHY reg in Port %d Page %d Offset %d.",
                port_no, page, offset);
        return (FAILED);
    }

    /* Read back */
    if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
        cterr('f', 0, "Failed to read PHY reg in Port %d Page %d Offset %d.",
                port_no, page, offset);
        return (FAILED);
    }
    reg_data &= (VSC85XX_TEST_MODE_MASK);
    if (reg_data == test_mode) {
        printf("Test mode has been switched to mode %d succesfully.\n", mode);
        return (PASSED);
    } else {
        printf("Failed to switch test mode. Read back mode reg #%x, expect #%x.\n", reg_data, test_mode);
        return (FAILED);
    }
}

/******************************************************************************
 *
 * Function: create_raw_socket
 *
 *     Create the raw socket with specific protocol.
 *
 * Input:  protocol - seclect protocol
 *
 * Output: rawsock - return created socket num.
 *
 *****************************************************************************/
int create_raw_socket(int protocol)
{ 
    int rawsock;
    if((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol)))== -1) {
        perror("Error creating raw socket\n");
        exit(-1);
    }
    return rawsock;
}

/******************************************************************************
 *
 * Function: bind_socket
 *     Bind raw socket to interface 
 *
 * Input:  device - current port
 *         rawsock - socket
 *         protocol - select protocol
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int bind_socket(char *device, int rawsock, int protocol) {
    struct sockaddr_ll sll;
    struct ifreq ifr;
 
    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */
 
    strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
    if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        perror("Error getting Interface index !\n");
        return FAILED;
    }
 
    /* Bind our raw socket to this interface */
 
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol); 

#if DEBUG
    printf("%s interface is %d\n", __FUNCTION__, ifr.ifr_ifindex);
#endif

    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1) {
        perror("Error binding raw socket to interface\n");
        return FAILED;
    }


    return PASSED;
}


/******************************************************************************
 *
 * Function: set_promisc
 *    set promisc mode.
 *    when program exit, this interface will still be promisc mode.
 *    we should disable promisc mode when we exit (ie, use atexit)
 *
 * Input:  
 *
 * Output: PASSED/FAILED
 *
 * Note: if the set_promisc is failed, the rx will get the haft of 
 *       packet from tx.
 *
 *****************************************************************************/
int set_promisc(char *device, int sock)
{
    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* First Get the Interface Index  */
    /* Set the network card in promiscuos mode */
    strncpy(ifr.ifr_name, device, IFNAMSIZ);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS");
        close(sock);
        return FAILED;
    }

    ifr.ifr_flags|=IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to set promiscous mode");
        close(sock);
        return FAILED;
    }

    return PASSED;
}

/***********************************************************************
 *
 * Function:    setup_eth_port()
 *
 * Description: Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:   sgmii_port - host system sgmii port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:  PASS/FAIL
 *
 ************************************************************************
 */
int setup_eth_port (int sgmii_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf(eth_name, "eth%d", sgmii_port);

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (set_promisc(eth_name, raw) == -1) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        return(FAIL);
    }

    *socket = raw;

    return PASS;
}

/***********************************************************************
 *
 * Function:    setup_xaui_port()
 *
 * Description: Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:   xaui_port - host system xaui port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:  PASS/FAIL
 *
 ************************************************************************
 */
int setup_xaui_port (int xaui_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf(eth_name, "xaui%d", xaui_port);

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (set_promisc(eth_name, raw) == -1) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        return(FAIL);
    }

    *socket = raw;

    return PASS;
}

/******************************************************************************
 *
 * Function: send_raw_packet
 *    set packet via socket.
 *
 * Input:  rawsock - socket
 *         pkt - tx buffer
 *         pkt_len - size of packet
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int send_raw_packet(int rawsock, unsigned char *pkt, int pkt_len)
{
    int sent= 0;

    /* A simple write on the socket ..thats all it takes ! */
    if((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
        return PASSED;
    }

    return FAILED;

}

/******************************************************************************
 *
 * Function: pkt_cmp
 *    compare the packet contents in two buffers.
 *
 * Input:  bufa - first buffer
 *         bufb - second buffer
 *         count - buffer length
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int pkt_cmp (unsigned char *bufa, unsigned char *bufb, int count)
{
    int ib = 0, rc = 0;
    unsigned char *p1 = bufa;
    unsigned char *p2 = bufb;

    for (ib = 0; ib < count; ib++, p1++, p2++) {
      if (*p1 != *p2) {
            if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
                printf("failed on byte %d, first data = %02x, second data = %02x\n",
                       (ib+1), *p1, *p2);
                printf("print byte %d, first data = %02x, second data = %02x\n",
                       (ib+2), *(p1+1), *(p2+1));
                printf("print byte %d, first data = %02x, second data = %02x\n",
                       (ib+3), *(p1+2), *(p2+2));
                printf("print byte %d, first data = %02x, second data = %02x\n",
                       (ib+4), *(p1+3), *(p2+3));
            }

            rc = FAILED;
            break;
        }
    }
      return rc;
}

/******************************************************************************
 *
 * Function: phy_cavium_is_linkup
 *
 *   Check linux up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_cavium_is_linkup (char *type, int port)
{

    int timeout_counter = 100, is_link = FALSE;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];

    sprintf(pname,"%s%d", type, port);

    while(1) {
        /* Get the interface information */
        if (getifaddrs(&if_list) < 0) {
            printf("%s(): %s Failed to get interface information: %s.\n",
                   __FUNCTION__, pname, strerror(errno));
            return (FAILED);
        }
        if (if_list == NULL) {
            printf("%s(): %s No network interfaces were found.\n",
                    __FUNCTION__, pname);
            return (FAILED);
        }

        for (if_info = if_list; if_info; if_info = if_info->ifa_next) {
            /* parse the port name */
            if (strncmp(if_info->ifa_name, pname, IFNAMSIZ) != 0) {
                continue;
            }

             /* printf("%s ", if_info->ifa_name); */

             flags = if_info->ifa_flags;
             if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
               /* printf("up\n"); */
                 fflush(stdout);
                 is_link = TRUE;
                 break;
             } else {
                 /* printf("down\n");  */
                 msleep(10);
                 timeout_counter--;
                 if (timeout_counter == 0) {
                     return (FAILED);
                 }
             }
             fflush(stdout);
        } /*for*/

        freeifaddrs(if_list);

        if (is_link == TRUE) {
            break;
        }
    } /*while */

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_media_is_linkup
 *
 *   Check PHY media link status.
 *
 * Input: port - port index.
 *        lpbk_mode - INT/EXT
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_media_is_linkup (int port)
{
    int timeout = 600;
    ushort data;
    ushort page;
    ushort offset;
    uint bus_no = SMI_BUS_0;

    page = VSC85XX_PHY_EXT_REG_PAGE_0;
    offset = 1;
    while (timeout--) {
        if (wallander_phy_reg_page_rd(bus_no, port, page, offset, &data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                    port, page, offset);
            return (FAILED);
        }
        if (wallander_phy_reg_page_rd(bus_no, port, page, offset, &data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                    port, page, offset);
            return (FAILED);
        }
        if (data & 0x0004) {
            /* Link is up */
            return (PASSED);
        }
        msleep(10);
    }

    printf("Port %d media link down.\n", port);
    return (FAILED);
}

static void
check_bitlock(void)
{
    uint64_t data = cvmx_read_csr(CVMX_PCSX_RXX_SYNC_REG(1, 1));
    int i;
    cvmx_gserx_dlmx_phy_reset_t reset_reg;

    for (i = 0; i < 1; i++) {
        if (data != 0) {
            break;
        }

        reset_reg.u64 = cvmx_read_csr(CVMX_GSERX_DLMX_PHY_RESET(0, 0));
        reset_reg.cn70xx.phy_reset = 1;
        cvmx_write_csr(CVMX_GSERX_DLMX_PHY_RESET(0, 0), reset_reg.u64);
        cvmx_wait_usec(1000);
        reset_reg.cn70xx.phy_reset = 0;
        cvmx_write_csr(CVMX_GSERX_DLMX_PHY_RESET(0, 0), reset_reg.u64);

        cvmx_wait_usec(100000 * 5);

        data = cvmx_read_csr(CVMX_PCSX_RXX_SYNC_REG(1, 1));
    }

    if (data == 0) {
        printf("reset DLM failed\n");
    } else if (i != 0) {
        printf("retry %d times\n", i);
    }

    return;
}

/******************************************************************************
 *
 * Function: phy_restart_autoneg
 *  Restart ANEG.
 *
 * Input:  Port index.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_restart_aneg(int port)
{
    int timeout = 300;
    ushort data;
    ushort page;
    ushort offset;
    uint bus_no = SMI_BUS_0;

    page = VSC85XX_PHY_EXT_REG_PAGE_0;
    offset = 0;
    if (wallander_phy_reg_page_rd(bus_no, port, page, offset, &data)) {
        printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                port, page, offset);
        return (FAILED);
    }
    data |= 0x200;
    if (wallander_phy_reg_page_wr(bus_no, port, page, offset, data)) {
        printf("Failed to write PHY reg in Port %d Page %d Offset %d.", 
                port, page, offset);
        return (FAILED);
    }
    while (timeout--) {
        if (wallander_phy_reg_page_rd(bus_no, port, page, offset, &data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                    port, page, offset);
            return (FAILED);
        }
        if (data & 0x200) {
            /* Should be self-cleared */
            msleep(10);
            continue;
        }
        return (PASSED);
    }
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_port_check_sync
 *  Check whether PHY sync is OK.
 *
 * Input:  Port index.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_port_check_sync(int port_no)
{
    int timeout = 100;
    ushort page = VSC85XX_PHY_EXT_REG_PAGE_3;
    ushort offset = VSC85XX_PHY_MAC_SERDES_STATUS;
    ushort reg_data;
    uint bus_no = SMI_BUS_0;

    while (timeout--) {
        if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.\n", 
                    port_no, page, offset);
            return (FAILED);
        }

        if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.\n", 
                    port_no, page, offset);
            return (FAILED);
        }

        if (reg_data & VSC85XX_PHY_MAC_SERDES_SYNC) {
            return (PASSED);
        } else {
            msleep(10);
        }
    }

    printf("PHY reg value in Port %d Page %d Offset %d: %#x.\n", 
        port_no, page, offset, reg_data);
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_check_sync
 *  Check whether PHY sync is OK.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_check_sync(void)
{
    if (phy_port_check_sync(0)) {
        check_bitlock();
        if (phy_port_check_sync(0)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_port_check_mac_pcs_link
 *  Check whether PHY MAC PCS link is up.
 *
 * Input:  Port index.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_port_check_mac_pcs_link(int port_no)
{
    ushort page = VSC85XX_PHY_EXT_REG_PAGE_3;
    ushort offset = VSC85XX_PHY_PCS_STATUS;
    ushort reg_data;
    uint bus_no = SMI_BUS_0;
    int timeout = 100;

    while (timeout--) {
        if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.\n", 
                    port_no, page, offset);
            return (FAILED);
        }
        if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.\n", 
                    port_no, page, offset);
            return (FAILED);
        }

        if (reg_data & VSC85XX_PHY_PCS_STATUS_LINK) {
            return (PASSED);
        }
        msleep(10);
    }
    printf("PHY reg value in Port %d Page %d Offset %d: %#x.\n", 
        port_no, page, offset, reg_data);
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_check_mac_pcs_link
 *  Check whether PHY MAC PCS link is up for all ports.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_check_mac_pcs_link(void)
{
    int i;
    int max_phy_ports = get_num_ports();

    for (i = 0; i < max_phy_ports; i++) {
        if (phy_port_check_mac_pcs_link(i)) {
            check_bitlock();
            if (phy_port_check_mac_pcs_link(i)) {
                return (FAILED);
            }
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_check_media_link
 *  Check whether PHY Media link is up for all ports.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_check_media_link(void)
{
    int i;
    int max_phy_ports = get_num_ports();

    for (i = 0; i < max_phy_ports; i++) {
        if (phy_media_is_linkup(i)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: cpu_port_check_pcs_link
 *  Check whether CPU PCS link is up.
 *
 * Input:  Port index.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int cpu_port_check_pcs_link(int port_no)
{
    uint64_t pcs_status;
    int timeout = 100;

    while (timeout--) {
        pcs_status = cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(port_no, 1));
        pcs_status = cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(port_no, 1));

        if (pcs_status & VSC85XX_PHY_PCS_STATUS_LINK) {
            return (PASSED);
        }
        msleep(10);
    }
    printf("CVMX_PCSX_MRX_STATUS_REG(%d, 1): %#llx\n", 
        port_no, (long long unsigned int)pcs_status);
    return (FAILED);
}


/******************************************************************************
 *
 * Function: cpu_check_pcs_link
 *  Check whether PHY MAC PCS link is up for all ports.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int cpu_check_pcs_link(void)
{
    int i;
    int max_phy_ports = get_num_ports();

    for (i = 0; i < max_phy_ports; i++) {
        if (cpu_port_check_pcs_link(i)) {
            check_bitlock();
            if (cpu_port_check_pcs_link(i)) {
                return (FAILED);
            }
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: check_pkt
 *   Compared the packet between the buffer of tx and rx.
 *
 * Input:  NONE
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int check_pkt (int pkt_len)
{
    if ((pkt_cmp(tx_packet, rx_packet, pkt_len)) == PASSED) {
#if DEBUG
        printf("%s() Rx packet matched\n", __func__);
#endif
        return PASSED;
    } else {
        printf("%s() Rx packet mismatched\n", __func__);

        return (FAILED);
    }

}

/******************************************************************************
 *
 * Function: ptp_check_pkt
 *   Compared the ptp packet between the buffer of tx and rx.
 *
 * Input:  NONE
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int ptp_check_pkt (int pkt_len)
{
    int ix, timestamp_count = 0;
    int tx_mismatch_buf[10] = {0};
    int rx_mismatch_buf[10] = {0};
    unsigned char *p1 = tx_packet;
    unsigned char *p2 = rx_packet;

    /* Check first 12 bytes, byte 1~ byte 12, tx must equal to rx */
    for (ix = PTP_PKT_CMP_START_LEN_1; 
        ix <= /*PTP_PKT_CMP_END_LEN_1*/PTP_PKT_CMP_START_LEN_2 - 1; ix++) {
        if (p1[ix] != p2[ix]) {
            printf("Packet mismatched on byte %d, tx_packet data = %02x, rx_packet data = %02x",
                   ix, p1[ix], p2[ix]);

            return (FAILED);
        }
    }

    /* Check timestamps 10 bytes, byte 49~byte 58, will give timestamps init val,
        after update the timestamps the value will be changed, but it's possible the value 
        will same with the original val */
    for (ix = PTP_PKT_CMP_START_LEN_2; ix <= PTP_PKT_CMP_END_LEN_2; ix++) {
        if (p1[ix] == p2[ix]) {
            tx_mismatch_buf[timestamp_count] = p1[ix];
            rx_mismatch_buf[timestamp_count] = p2[ix];
            timestamp_count ++;
        }
    }

    /* If 10 bytes timestamps values keep the same, then timestamp update fail */
    if (timestamp_count == 10) {
        timestamp_count =0;
        printf("Timestamp is not updated. Content:\n");
        for (ix = PTP_PKT_CMP_START_LEN_2; ix <= PTP_PKT_CMP_END_LEN_2 ; ix++) {
            printf("Timestamp-tx[%d]=%02x, Timestamp-rx[%d]=%02x\n",
                   ix, tx_mismatch_buf[timestamp_count], 
                   ix, rx_mismatch_buf[timestamp_count]);
            timestamp_count ++;
        }
        printf("\n");

        return (FAILED);
    }
    if (timestamp_count < 10) {
        printf("Timestamp is updated. Content:\n");
        for (ix = PTP_PKT_CMP_START_LEN_2; ix <= PTP_PKT_CMP_END_LEN_2 ; ix++) {
            printf("Timestamp-tx[%d]=%02x, Timestamp-rx[%d]=%02x\n",
                   ix, p1[ix], 
                   ix, p2[ix]);
        }
        printf("\n");
    }
    return (PASSED);
}

int chk_macaddr (uchar *macaddr1, uchar *macaddr2)
{
    return (pkt_cmp(macaddr1, macaddr2,6));
}

int send_packets(int *socket, int len, char val, int port, int speed)
{
    int raw, rc = 0;
    uint mac_size, fil_len;
    unsigned char volatile *cptr;
    char iface_type[32];

    raw = *socket;

    /* clean up the rx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);

    cptr = (unsigned char *)tx_packet;
    mac_size = sizeof(mac_addr_t);

    /* put in the destination/source mac address */
    memcpy((char *)cptr, (char *)mac_da, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy((char *)cptr, (char *)mac_sa, sizeof(mac_addr_t));
    cptr += mac_size;

    /* fill the packet. the len is include the size of mac address 
     * we need to minus the size of mac address on len for filbyte
     */
    fil_len = (len - (2*mac_size)); 
    filbyte((uchar *)cptr, fil_len, val);

    sprintf(iface_type, SEL_PORT_ETH);

    rc = phy_media_is_linkup(port);
    if (rc == FAILED) {
        cterr('f',0, "port media link up time out");
        return (FAILED);
    }

    rc = phy_cavium_is_linkup(iface_type, port);
    if (rc == FAILED) {
        cterr('f',0, "port link up time out");
        return (FAILED);
    }

    rc = phy_port_check_mac_pcs_link(port);
    if (rc == FAILED) {
        cterr('f',0, "port mac pcs link up time out");
        return (FAILED);
    }

    rc = cpu_port_check_pcs_link(port);
    if (rc == FAILED) {
        cterr('f',0, "cpu pcs link up time out");
        return (FAILED);
    }

   if(!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        cterr('f',0, "error on sending packet");
        return (FAILED);
    }

    return (PASSED);

}

/******************************************************************************
 *
 * Function: ptp_set_packet
 *   Set up PTP packet for tx and rx using..
 *
 * Input:  val - content of packet
 *         port - port index
 *
 * Output: None
 *
 *****************************************************************************/
int ptp_set_packet(char *pkt, int port)
{
    int r = 0;
    struct ethhdr *tx_eth;
    struct iphdr *tx_ip;
    struct udphdr *tx_udp;
    struct ptphdr *tx_ptp;
    struct ptpdata *tx_ptp_data;

    tx_ptp_data = (struct ptpdata *)
                  (pkt + sizeof(struct ethhdr) + sizeof(struct iphdr) +
                         sizeof(struct udphdr) + sizeof(struct ptphdr));
    tx_ptp_data->second_mSB = htons(0x01);
    tx_ptp_data->second_lSB = htonl(0x02);
    tx_ptp_data->nanosecond = htonl(0x03);

    //prepare the Timestamp management packet header   
    tx_ptp = (struct ptphdr *)
             (pkt + sizeof(struct ethhdr) + sizeof(struct iphdr) + 
                    sizeof(struct udphdr)); 

    tx_ptp->msgtype = PTP_MSG_DELAY_REQ;
    tx_ptp->version = PTP_CLK_VERSION_NO;
    tx_ptp->length = htons(PTP_DELAY_REQ_LENGTH);
    tx_ptp->domain_number = 0;
    tx_ptp->reserved0 = 0;
    tx_ptp->flag = 0;
    tx_ptp->correction = 0L;
    /* add 0xBEEFDEAD at offset 16 from start of payload. This is where
       we expect 4 bytes of nanoseconds from Time stamp bytes will be 
       stuffed in, in the Rx Ingress packet */
    tx_ptp->reserved1 = htonl(0xbeefdead);
    tx_ptp->source_port_id = 0L;
    tx_ptp->port_number = 0;
    tx_ptp->sequence_id = 0;
    tx_ptp->control = 0x01;
    tx_ptp->log_msgInterval = 0x7f;

    /*
     * UDP header
     */
    tx_udp = (struct udphdr *)
             (pkt + sizeof(struct ethhdr) + sizeof(struct iphdr));

    tx_udp->source = htons((u_short)319); // PTP Event
    tx_udp->dest = htons((u_short)319); // PTP Event
    tx_udp->len  = htons( (u_short)(sizeof(struct udphdr) + PTP_DELAY_REQ_LENGTH + 6 )); // 6 bytes pay load
    tx_udp->check   = 0x00;

    /* 
     * IP header
     */
    tx_ip =  (struct iphdr *)(pkt + sizeof(struct ethhdr));

    tx_ip->version  = 0x4;  // ipv4
    tx_ip->ihl = 0x5;
    tx_ip->tos = 0x00;  // type of service

    memcpy( (char *)&tx_ip->saddr, (char*)ip_da[port], 4);
    memcpy( (char *)&tx_ip->daddr, (char*)ip_da[port], 4);

    // calculate size of ip packet
    tx_ip->tot_len  =  htons((u_short)(sizeof(struct iphdr) + tx_udp->len));

    tx_ip->id  = 0;
    tx_ip->frag_off = 0;
    tx_ip->ttl = 0x40;
    tx_ip->protocol   = IPPROTO_UDP;
    tx_ip->check = 0;

    // FIXME: hard-coded here
    tx_ip->check = htons(0x7a9e);

    /*
     * Ethernet header
     */
    tx_eth =  (struct ethhdr *)(u_long)pkt;

    memcpy( (char *)&tx_eth->h_dest, (char*)&mac_da[0], 6);
    memcpy( (char *)&tx_eth->h_source, (char*)&mac_sa[0], 6);
    tx_eth->h_proto = ETH_P_IP;

    return r;
}

/*------------------------------------------------------------------
 *
 * Function: ptp_send_packets
 *   for 1588 tx send packet to rx. if number of packet is too much,
 *   then the delay is needed.
 *
 * Input:  len - packet length
 *         val - content of packet
 *
 * Output: PASSED/FAILED
 * 
 *------------------------------------------------------------------
 */

int ptp_send_packets(int *socket, int len, char val, int port, int speed)
{
    int raw, rc = 0;
/*    uint fil_len;
    uint mac_size;
    unsigned char volatile *cptr;*/
    char iface_type[32];
    char test_packet[96];

    memset(test_packet, 0, sizeof(test_packet));
    ptp_set_packet(test_packet, port);
    memcpy((char *)tx_packet, (char *)test_packet, sizeof(test_packet));

    raw = *socket;

    sprintf(iface_type, SEL_PORT_ETH);

    rc = phy_cavium_is_linkup(iface_type, port);
    if (rc == FAILED) {
        printf("port link up time out\n");
        return (FAILED);
    }

    if (!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        cterr('f',0, "error on sending packet");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: ptp_check_timestamp
 *  Check whether timestamp content is reasonable.
 *
 * Input:  port - port index.
 *
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int ptp_check_timestamp(int port) 
{
    char ts_buf[FPGA_TS_DATA_LENGTH];
    int i;
    uint16 exp_data;
    uint16 data_hi;
    int rc = PASSED;

    printf("Check Timestamp.\n");
    if (fpga_check_ts_intr()) {
        printf("Timestamp interrupt not triggered.\n");
        return (FAILED);
    }
    if (fpga_check_ts_ready()) {
        printf("Timestamp not ready.\n");
        return (FAILED);
    }

    for (i = 0; i < FPGA_TS_DATA_LENGTH; i++) {
        char val = 0;
        if (fpga_reg_read(FPGA_TS_DATA_START + i, &val)) {
            return (FAILED);
        }
        ts_buf[i] = val;
    }

    /* Check port index */
    /* hard coded number at 1588 insert */
    exp_data = 0x1110 + port;
    data_hi  = ( ts_buf[0x11] << 8 ) | ts_buf[0x12];

    if ( data_hi != exp_data ) {
        printf("1588 SPI packet timestamp info data_hi=0x%x exp=0x%x, port=%d\n", 
                data_hi, exp_data, port );
        rc = FAILED;
    }

    if (( ts_buf[0x17] == 0 ) && 
        ( ts_buf[0x18] == 0 ) &&
        ( ts_buf[0x19] == 0 ) && 
        ( ts_buf[0x1A] == 0 )) {
        printf("1588 SPI packet timestamp info data_nsec=0, port=%d, offset [0x17]", port );
        rc = FAILED;
    }

    return (rc);
}


/******************************************************************************
 *
 * Function: receive_packets
 *  Create socket, setup port speed for receiving packet.
 *
 * Input:  argument - pass arg for pthread, now is amount of packet.
 *
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
void *receive_packets (diag_info_pthread_t *get_info)
{
    int rx= 0, rc = 0;
    struct timespec ts;
    uint ii, pkt_cnt = 0;
    uchar *rx_pkt_buf;
    struct timeval tv;
    int otherpkt_cnt = 0;

#if DEBUG
    int yy;

    printf(" name %s ", get_info->name);
    printf(" speed %d ", get_info->speed);
    printf(" pkt_num %d ", get_info->pkt_num);
    printf(" pkt_len %d ", get_info->pkt_len);
    printf(" socket %d ", get_info->socket);
#endif

    rx_pkt_buf = (uchar *)rx_packet;

    /* Set up tv for socket time out.
     * Get the second and microsecond portion of wait time
     * to set the socket time out.
     */
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = TX_RX_SYNC_TIME;   /* sec portion of wait time */
    tv.tv_usec = 0;  /* microsec portion of wait time */
    if (setsockopt(get_info->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
        pthread_exit ((void *)FAILED);
    }

    for (ii = 0; ii < get_info->pkt_num; ii++) {
        pkt_cnt = 0;

        /* printf tx for rx is ready to receive packet. */
        if (sem_post(&rx_ready) != PASSED) {
            if (errno == EINVAL)
                printf("The sem(rx_ready) does not refer to a valid semaphore \n");
            else
                printf("The function sem_post() is not supported by this implementation\n");
            pthread_exit ((void *)FAILED);
        }

        do {
            /* clear the rx_packet buffer */
            memset((char *)rx_packet, 0, ETH_PKT_MAX_LEN);

            rx = read(get_info->socket, (unsigned char *)rx_pkt_buf, get_info->pkt_len);

            if (rx < 0) {
                printf("\n%s rx= %d socket %d read timeout. loop(ii)= %d, pkt_cnt = %d otherpkt_cnt= %d\n",
                       __FUNCTION__, rx, get_info->socket, ii, pkt_cnt, otherpkt_cnt);
                break; /* exit do loop */
            }

            /* drop invalid packet*/
            if (chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) {
                otherpkt_cnt++;

                if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
                    printf("\n detected non diag packet at pkt %d. Ignore.\n", ii);
                    printf("\n\n>>>> %s %s %d bytes received: !!! pkt_cnt= %d\n",
                           __FUNCTION__, get_info->name, rx, pkt_cnt);

                    printf("Destination MAC address: "
                           "%02x:%02x:%02x:%02x:%02x:%02x\n",
                           rx_pkt_buf[0],rx_pkt_buf[1],rx_pkt_buf[2],
                           rx_pkt_buf[3],rx_pkt_buf[4],rx_pkt_buf[5]);
                    printf("Source MAC address: "
                           "%02x:%02x:%02x:%02x:%02x:%02x\n",
                           rx_pkt_buf[6],rx_pkt_buf[7],rx_pkt_buf[8],
                           rx_pkt_buf[9],rx_pkt_buf[10],rx_pkt_buf[11]);
                }

#if DEBUG
                for (yy=0; yy < rx; yy++) {
                    if ((yy > 0) && (yy % 16) == 0) {
                        printf("\n");
                    }
                    printf("%02x ", rx_pkt_buf[yy]);
                }
                printf("end of pkt print\n");
#endif

                continue;
            }
            /* valid packet, increase the packet count */
            pkt_cnt++;

#if DEBUG
            printf("%d bytes received: !!! , pkt_cnt = %d \n", rx, pkt_cnt);
#endif
        } while (pkt_cnt < 2);  
        /* loopback should get double of packets, one from original path,
         * another one from driver. 
         */

        /* the receive packets pkt_cnt are double of pkt_num
         * one from hw, another from driver. Inform tx for
         * rx read packet is finish.
         */
        if (sem_post(&rx_finish)) {
            if (errno == EINVAL) {
                printf("The sem(rx_finish) does not refer to a valid semaphore \n");
            } else {
                printf("The function sem_post() is not supported by this implementation\n");
            }
            pthread_exit ((void *)FAILED);
        }

        if (rx < 0) {
            pthread_exit ((void *)FAILED);
        }

        /* Add some more time to wait for sem tx_cmp to be unlocked
         */
        /* init timeout value. */
        rc = clock_gettime(CLOCK_REALTIME, &ts);
        if (rc != PASSED) {
            printf("clock gettime failed..\n");
            pthread_exit ((void *)FAILED);
        }
        ts.tv_sec += TX_RX_SYNC_TIME; /* TX_RX_SYNC_TIME 10*/

        /* wait for tx compare the tx_packet and rx_packet.  */
        rc = sem_timedwait(&tx_cmp, &ts);
        if (rc != PASSED) {
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on tx_cmp timeout. \n");
            } else {
                printf("semaphore wait on tx_cmp failed. \n");
            }
            pthread_exit ((void *)FAILED);
        }
    }  /* for*/

    pthread_exit((void *)PASSED);
}

/******************************************************************************
 *
 * Function: show_buf_content
 *  Show tx & rx buffer content.
 *
 * Input:  show_pkt_len - packet length.
 *
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
void show_buf_content(int show_pkt_len) 
{
    uint ii;
    unsigned char volatile *tptr, *rptr;

    tptr = tx_packet;
    rptr = rx_packet;

    printf("\nstart of pkt print.\n");
    for (ii=0; ii < show_pkt_len; ii++) {
        if ((ii > 0) && (ii % 8) == 0) {
            printf("\n");
        }
        printf("tx:%02x rx:%02x  ", tptr[ii], rptr[ii]);
    }
    printf("\nend of pkt print.\n");
}

/******************************************************************************
 *
 * Function: tx_rx_diag
 *  Using Pthread to create another thread for rx.
 *  tx should wait for rx build. After tx send packet to rx
 *  tx also need to wait for rx get all the packet.
 *  the waiting mechanism is using semaphore. 
 *  the timeout value is set to 10.
 *
 * Input:  p_type - port type
 *         eth_port - port number
 *         speed - test speed
 *         signal - test signal fiber or copper
 *         pkt_cnt - test packet count
 *         value - contain of speed
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int tx_rx_diag(char* p_type, int eth_port, int speed, int pkt_cnt, int pkt_len, int value) 
{
    pthread_t threads;
    struct timespec ts;
    diag_info_pthread_t rx_info;
    char pname[10];
    int ii;
    int tx_skt, rx_skt;
    int rc;
    void  *pthr_rv;

    int is_ptp_flag;
    if (pkt_len & PTP_PKT_LEN_BIT_31_MASK) {
        is_ptp_flag = TRUE;
        pkt_len &= (~PTP_PKT_LEN_BIT_31_MASK);
        printf ("PTP Flag is TRUE.\n");
    } else {
        is_ptp_flag = FALSE;
    } 

    /* init the semaphore. */
    rc = sem_init(&rx_ready, 0, 0 );
    if (rc != PASSED) {
        printf("eth_port %d sem_init on rx_ready failed.\n", eth_port);
        return (FAILED);
    }

    rc = sem_init(&rx_finish, 0, 0 );
    if (rc != PASSED) {
        printf("eth_port %d sem_init on rx_finish failed.\n", eth_port);
        return (FAILED);
    }

    rc = sem_init(&tx_cmp, 0, 0 );
    if (rc != PASSED) {
        printf("eth_port %d sem_init on tx_cmp failed.\n", eth_port);
        return (FAILED);
    }

    sprintf(pname,"%s%d", p_type, eth_port);

    /* setup ETH tx and rx socket */
    if (setup_eth_port(eth_port, &tx_skt) == FAIL) {
        return(FAILED);
    }

    if (setup_eth_port(eth_port, &rx_skt) == FAIL) {
        return(FAILED);
    }


    /* extend the space for putting the dest/src mac address */
    pkt_len += (2*sizeof(mac_addr_t));

    /* set up global value for both rx and tx on struct*/
    strncpy(rx_info.name, pname,IFNAMSIZ);
    rx_info.speed = speed;
    rx_info.pkt_num = pkt_cnt;
    rx_info.pkt_len = pkt_len;
    rx_info.socket = rx_skt;

    /* build another thread for rx, and pass rx_info to rx */
    if(pthread_create(&threads, NULL, (void *)receive_packets, (diag_info_pthread_t *) &rx_info)) {
        cterr('f',0, "pthread_create failed");
        return (FAILED);
    }

    for (ii = 0; ii < pkt_cnt; ii++) {
        /* init timeout value. */
        rc = clock_gettime(CLOCK_REALTIME, &ts);
        if (rc != PASSED) {
            printf("clock gettime failed..\n");
            goto exit_tx_rx_diag;
        }
        ts.tv_sec += TX_RX_SYNC_TIME;

        /* wait for the setting of rx side */
        rc = sem_timedwait(&rx_ready, &ts);
        if (rc != PASSED) {
              show_buf_content(rx_info.pkt_len);
              if (errno == ETIMEDOUT) {
                  printf("sem_timedwait on rx_ready timeout.\n");
              } else {
                  printf("semaphore wait on rx ready failed.\n");
              }
            goto exit_tx_rx_diag;
        }

        msleep(1); /* ensure rx read is ready before tx */

        /* the main thread prepare to sending packet. */
        if (is_ptp_flag == TRUE) {
            rc = ptp_send_packets(&tx_skt, pkt_len, value, eth_port, speed);

            if (rc == FAILED) {
                printf("ptp_send_packets failed\n");
                goto exit_tx_rx_diag;
            }
        } else {
            rc = send_packets(&tx_skt, pkt_len, value, eth_port, speed);

            if (rc == FAILED) {
                printf("send_packets failed\n");
                goto exit_tx_rx_diag;
            }  
        } 

        /* Add some more time to wait for sem rx_finish to be unlocked
         */
        ts.tv_sec += TX_RX_SYNC_TIME;
        /* use semaphore to detect timeout on rx side */
        rc = sem_timedwait(&rx_finish, &ts);
        if (rc != PASSED) {
            show_buf_content(rx_info.pkt_len);
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on rx_finish timeout. (Packet-%d)\n", ii);
            } else {
                printf("semaphore wait on rx finish failed. (Packet-%d)\n", ii);
            }
            goto exit_tx_rx_diag;
        }

        /* compare the packet on rx_packet and tx_packet */
        if (is_ptp_flag == TRUE) {
            rc = ptp_check_pkt(pkt_len);
            if (rc == PASSED) {
                /* For debug only */
//                 show_buf_content(rx_info.pkt_len);
                /* if match, clean up rx buffer for next packet. */
                memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
            } else {
                printf("%s: mismatch\n", __FUNCTION__);
                show_buf_content(rx_info.pkt_len);
                goto exit_tx_rx_diag;
            }
        } else {
            rc = check_pkt(pkt_len);
            if (rc == PASSED) {
                /* if match, clean up rx buffer for next packet. */
                memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
            } else {
                printf("%s: mismatch\n", __FUNCTION__);
                show_buf_content(rx_info.pkt_len);
                goto exit_tx_rx_diag;
            }
        }

        /* printf rx for read next packet. */
        if (sem_post(&tx_cmp)){
            if (errno == EINVAL){
                printf("The sem(tx_cmp) does not refer to a valid semaphore\n");
            } else {
                printf("The function sem_post() is not supported by this implementation\n");
            }
            return (FAILED);
        }

    }  /* for */

exit_tx_rx_diag:

    /* if failed, cancel the thread */
    if(rc != PASSED)
        pthread_cancel(threads);

    /* Sync the tx and rx in here and check the rx is pass or fail */
    pthread_join(threads, (void **)&pthr_rv);
    if (pthr_rv != PASSED) {
        printf("tx_rx_diag receive failed\n");
        rc = FAILED;
    } else {
        rc = PASSED;
    }

    /* close socket. */
    close(tx_skt);
    close(rx_skt);
    sem_destroy(&rx_ready);
    sem_destroy(&rx_finish);
    sem_destroy(&tx_cmp);

    return (rc);
}

/******************************************************************************
 *
 * Function: wallander_packet_lpbk_test
 *  Packet loopback test.
 *
 * Input:  port  - port index.
 *         speed - port speed
 *         is_ptp_flag - ptp flag
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int wallander_packet_lpbk_test(int port, int speed, int is_ptp_flag)
{
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0;
    uchar orig_hkpflag = hkeepflags;

    if (is_ptp_flag) {
        pkt_type = sizeof(ptp_pktdata)/sizeof(pktdata_info_t);
    } else {
        pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);
    }

    hkeepflags = orig_hkpflag;

    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        if (is_ptp_flag) {
            pkt_cnt = ptp_pktdata[typ_curr].send_count;
            pkt_len = ptp_pktdata[typ_curr].len;
            pkt_val = ptp_pktdata[typ_curr].val;
            hkeepflags |= ptp_pktdata[typ_curr].hkpflags;
        } else {
            pkt_cnt = pktdata[typ_curr].send_count;
            pkt_len = pktdata[typ_curr].len;
            pkt_val = pktdata[typ_curr].val;
            hkeepflags |= pktdata[typ_curr].hkpflags;
        }

        prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                                    port, speed, pkt_cnt, pkt_len, pkt_val);
        fflush(stdout);
        if (is_ptp_flag) {
            /* Write value 1 to pkt_len bit 31 for distinguish ptp lpbk packet
               or normal lpbk packet */
            pkt_len |= PTP_PKT_LEN_BIT_31_MASK;
        }

        /* prepare to send packet */
        rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
        if (rc == FAILED) {
//              cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d",
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            return (FAILED);
        }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", port, speed);
    fflush(stdout);
    hkeepflags = orig_hkpflag;
    return (rc);
}


/******************************************************************************
 *
 * Function: phy_get_speed
 *  Get the speed of the port
 *
 * Input:  port  - Port index.
 *         speed - Pointer to speed
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_get_speed(int port, int *speed)
{
    ushort page = VSC85XX_PHY_EXT_REG_PAGE_0;
    ushort offset = 28;
    ushort reg_data;
    uint bus_no = SMI_BUS_0;

    if (wallander_phy_reg_page_rd(bus_no, port, page, offset, &reg_data)) {
        printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                port, page, offset);
        return (FAILED);
    }

    switch (reg_data & 0x0018) {
        case 0x0010:
            *speed = SPD_1000MBPS;
            break;
        case 0x0008:
            *speed = SPD_100MBPS;
            break;    
        default:
            *speed = SPD_10MBPS;
            break;
        }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: hw_reset_init_phy
 *  Reset and initialize Vitesse PHY
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int hw_reset_init_phy(boolean time_stamp_input, int mode)
{
    /* Enable PHY Coma Mode FPGA output */
    if (fpga_enable_phy_coma_mode_output()) {
        cterr('f', 0, "Enable PHY Coma Mode FPGA output Failed.");
    }

    /* Turn on PHY Coma Mode */
    if (fpga_turn_on_phy_coma_mode()) {
        cterr('f', 0, "Turn on PHY Coma Mode Failed.");
    }

    /* Reset Vitesse PHY via FPGA */
    if (fpga_reset_phy()) {
        cterr('f', 0, "PHY Reset Failed.");
        return (FAILED);
    }
    sleep(ETH_DRIVER_DELAY);

    if (wallander_init_all_phy_ports(time_stamp_input, TRUE, mode)) {
        return (FAILED);
    }

    /* Turn off PHY Coma Mode */
    if (fpga_turn_off_phy_coma_mode()) {
        cterr('f', 0, "Turn off PHY Coma Mode Failed.");
    }
    sleep(ETH_DRIVER_DELAY);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: eth_port_config
 *  Configure eth ports:
 *      front panel port(s) - autoneg on
 *      backplane port(s)   - autoneg off
 *
 * Input:  None.
 *
 * Output: None.
 *
 *****************************************************************************/
void eth_port_config(void)
{
    char cmd_str[32];
    char pname[10];
    int port_num = get_num_ports();
    int i;

    for (i = 0; i < port_num; i++) {
        /* ifconfig up */
        sprintf(pname,"eth%d", i);
        sprintf(cmd_str, "ifconfig %s up", pname);
        printf("\n%s\n", cmd_str);
        system(cmd_str);

        /* use ethtool to config the eth driver */
        if (i & 0x1) {
            sprintf(cmd_str, "ethtool -s %s autoneg off speed 1000 duplex full", pname);
        } else {
            sprintf(cmd_str, "ethtool -s %s autoneg on speed 1000 duplex full", pname);
        }
        printf("%s\n", cmd_str);
        system(cmd_str);
    }
    sprintf(cmd_str, "ifconfig eth1 %s", DIAG_IP_ADDR);
    printf("%s\n", cmd_str);
    system(cmd_str);

    sleep(ETH_DRIVER_DELAY * 3);
}

/******************************************************************************
 *
 * Function: phy_default_config
 *  Initialize and configure Vitesse PHY
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_default_config(boolean time_stamp_input, int mode)
{
    int retries = 4;

    while (retries--) {
        if (hw_reset_init_phy(time_stamp_input, mode)) {
            printf("PHY Reset Failed.\n");
            continue;
        }
        sleep(1);

        printf("Check PHY sync status.\n");
        if (phy_check_sync()) {
            printf("PHY no sync.\n");
            continue;
        }
        printf("Check MAC PCS link.\n");
        if (phy_check_mac_pcs_link()) {
            printf("MAC PCS link down.\n");
            continue;
        }
        eth_port_config();
        if (phy_media_is_linkup(1)) {
            printf("BP Media link down.\n");
            continue;
        }

        printf("PHY initialized with %d retries.\n", (3 - retries));
        sleep(2);

        return (PASSED);
    }
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_fp_cu_cfg
 *  Configure PHY Frontplane port to Copper mode.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_fp_cu_cfg(int port_no, int lpbk_mode)
{
    ushort offset;
    ushort page;
    ushort reg_data;
    uint bus_no = SMI_BUS_0;

    /* Only apply to front panel ports */
    if (port_no & 1) {
        return (PASSED);
    }

    page = 0;
    offset = 23;
    if (wallander_phy_reg_page_wr(bus_no, port_no, page, offset, 0x804)) {
        printf("Failed to write PHY reg in Port %d Page %d Offset %d.", 
                port_no, page, offset);
        return (FAILED);
    }

    /* Reset PHY */
    page = 0;
    offset = 0;
    if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
        printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                port_no, page, offset);
        return (FAILED);
    }
    reg_data |= VSC85XX_SW_RESET;
    if (wallander_phy_reg_page_wr(bus_no, port_no, page, offset, reg_data)) {
        printf("Failed to write PHY reg in Port %d Page %d Offset %d.", 
                port_no, page, offset);
        return (FAILED);
    }

    msleep(500);

    if (lpbk_mode == 0) {

        /* Turn off Auto-Neg and set speed to 1000Mbps */
        page = VSC85XX_PHY_EXT_REG_PAGE_0;
        offset = VSC85XX_MODE_CONTROL;
        if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                    port_no, page, offset);
            return (FAILED);
        }
        reg_data &= ~(0x1000);
        reg_data &= ~(0x2000);
        reg_data |= (0x4140);
        if (wallander_phy_reg_page_wr(bus_no, port_no, page, offset, reg_data)) {
            printf("Failed to write PHY reg in Port %d Page %d Offset %d.", 
                    port_no, page, offset);
            return (FAILED);
        }
        if (wallander_phy_reg_page_rd(bus_no, port_no, page, offset, &reg_data)) {
            printf("Failed to read PHY reg in Port %d Page %d Offset %d.", 
                    port_no, page, offset);
            return (FAILED);
        }
    }
    sleep(ETH_DRIVER_DELAY * 3);
    return (PASSED);
}

/******************************************************************************
 *
 * Function: phy_int_lpbk_config
 *  Init and configure PHY for internal loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_int_lpbk_config()
{
    int rc = PASSED;
    int i;
    int max_phy_ports = get_num_ports();
    FILE *fp;
    char status_file[32];
    char cmd_str[100];
    char pname[10];
    int speed = SPD_1000MBPS;
    int retries = 4;

    sprintf(status_file, "/tmp/int_lpbk_reg_dump");

    fp = fopen(status_file, "w+");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    while (retries--) {
        for (i = 0; i < max_phy_ports; i++) {
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s down", pname);
            printf("%s\n", cmd_str);
            system(cmd_str);
        }

        if (hw_reset_init_phy(FALSE, COPPER_MODE)) {
            printf("PHY Initialize Failed.\n");
            continue;
        }
        save_port_ctrl_stat(fp, "After Init PHY");

        for (i = 0; i < max_phy_ports; i++) {
            /* ifconfig up */
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s up", pname);
            printf("\n%s\n", cmd_str);
            system(cmd_str);

            /* use ethtool to config the eth driver */
            sprintf(cmd_str, "ethtool -s %s autoneg off speed %d duplex full", pname, speed);
            printf("%s\n", cmd_str);
            system(cmd_str);
            sleep(ETH_DRIVER_DELAY * 3);
        }
        save_port_ctrl_stat(fp, "After ifconfig and ethtool");

        for (i = 0; i < max_phy_ports; i++) {
            phy_fp_cu_cfg(i, 0);
            sleep (ETH_DRIVER_DELAY * 3);
            /* Set all phy to internel loopback mode */
            /* System Lpbk -- TRUE; Line Lpbk -- FALSE */
            rc = wallander_set_phy_loopback(i, TRUE, FALSE);
            if (rc != PASSED) {
                fclose(fp);
                show_port_ctrl_stat_util();
                cterr('f', 0, "Set PHY Loopback Failed.");
                return rc;
            }
        }
        save_port_ctrl_stat(fp, "Setting Near-end loopback");

        printf("Check PHY media link.\n");
        if (phy_check_media_link()) {
            printf("PHY media link down.\n");
            continue;
        }

        printf("Check PHY sync status.\n");
        if (phy_check_sync()) {
            printf("PHY no sync.\n");
            continue;
        }
        printf("Check MAC PCS link.\n");
        if (phy_check_mac_pcs_link()) {
            printf("MAC PCS link down.\n");
            continue;
        }
        printf("Check CPU PCS link.\n");
        if (cpu_check_pcs_link()) {
            printf("CPU PCS link down.\n");
            continue;
        }
        printf("PHY initialized with %d retries.\n", (3 - retries));
        sleep(ETH_DRIVER_DELAY * 2);
        save_port_ctrl_stat(fp, "After config");
        fclose(fp);

        return (PASSED);
    }
    fclose(fp);
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_int_lpbk_test
 *  PHY ports internal loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_int_lpbk_test()
{
    int rc = PASSED;
    int i;
    int max_phy_ports = get_num_ports();
    int speed = SPD_1000MBPS;

    testname("PHY Internal Loopback");

    if (phy_int_lpbk_config()){
        cterr('f', 0, "PHY Internal Loopback Initialize Failed.");
        return (FAILED);
    }

    mode_internal = 1;
    mode_ext_sfp = 0;
    mode_ext_cu = 0;

    for (i = 0; i < max_phy_ports; i++) {
        /* Start loopback test */
        rc = wallander_packet_lpbk_test(i, speed, 0);
        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "PHY Internal Loopback Test Failed.");
            return rc;
        }

        /* Clear internel loopback */
        /* System Lpbk -- FALSE; Line Lpbk -- FALSE */
        rc = wallander_set_phy_loopback(i, FALSE, FALSE);
        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "Set PHY Loopback Failed.");
            return rc;
        }
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: phy_sfp_ext_lpbk_config
 *  Init and configure PHY for SFP external loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_sfp_ext_lpbk_config()
{
    int i;
    int max_front_ports = get_num_ports();
    FILE *fp;
    char status_file[32];
    char cmd_str[100];
    char pname[10];
    int speed = SPD_1000MBPS;
    int retries = 4;

    sprintf(status_file, "/tmp/sfp_lpbk_reg_dump");

    fp = fopen(status_file, "w+");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    while (retries--) {
        for (i = 0; i < max_front_ports; i++, i++) {
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s down", pname);
            printf("%s\n", cmd_str);
            system(cmd_str);
        }

        if (hw_reset_init_phy(FALSE, FIBER_MODE)) {
            printf("PHY Initialize Failed.\n");
            continue;
        }
        save_port_ctrl_stat(fp, "After Init PHY");

        for (i = 0; i < max_front_ports; i++, i++) {
            printf("Enable SFP Tx for port %d\n", i);
            if (enable_sfp_tx(i/2)) {
                fclose(fp);
                show_port_ctrl_stat_util();
                cterr('f', 0, "Failed to enable SFP Tx.");
                return (FAILED);
            }
            /* ifconfig up */
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s up", pname);
            printf("\n%s\n", cmd_str);
            system(cmd_str);

            /* use ethtool to config the eth driver */
            sprintf(cmd_str, "ethtool -s %s autoneg on speed %d", pname, speed);
            printf("%s\n", cmd_str);
            system(cmd_str);
            sleep(ETH_DRIVER_DELAY * 3);
        }
        save_port_ctrl_stat(fp, "After ifconfig and ethtool");

        printf("Check PHY media link.\n");
        if (phy_check_media_link()) {
            printf("PHY media link down.\n");
            continue;
        }
        printf("Check PHY sync status.\n");
        if (phy_check_sync()) {
            printf("PHY no sync.\n");
            continue;
        }
        printf("Check MAC PCS link.\n");
        if (phy_check_mac_pcs_link()) {
            printf("MAC PCS link down.\n");
            continue;
        }
        printf("Check CPU PCS link.\n");
        if (cpu_check_pcs_link()) {
            printf("CPU PCS link down.\n");
            continue;
        }
        printf("PHY initialized with %d retries.\n", (3 - retries));
        sleep(ETH_DRIVER_DELAY * 2);
        save_port_ctrl_stat(fp, "After config");
        fclose(fp);

        return (PASSED);
    }
    fclose(fp);
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_sfp_ext_lpbk_test
 *  PHY ports SFP external loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_sfp_ext_lpbk_test()
{
    int rc = PASSED;
    int i;
    int max_front_ports = get_num_ports();
    int speed = SPD_1000MBPS;

    testname("PHY SFP External Loopback");

    /* Skip the test if EXT_LPBK flag is off */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("Ext. loopback is off, skip the test\n");
        return (PASSED);
    }

    /* If current mode is not SFP, configure the PHY */
    if (mode_ext_sfp == 0) {
        if (phy_sfp_ext_lpbk_config()){
            cterr('f', 0, "PHY SFP External Loopback Initialize Failed.");
            return (FAILED);
        }
    }
    /* Now the PHY should be in SFP mode */
    mode_ext_sfp = 1;
    mode_ext_cu = 0;
    mode_internal = 0;

    for (i = 0; i < max_front_ports; i++, i++) {
        /* Start loopback test */
        rc = wallander_packet_lpbk_test(i, speed, 0);
        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "PHY Internal Loopback Test Failed.");
            return rc;
        }
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: phy_cu_ext_lpbk_config
 *  Init and configure PHY for Copper external loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_cu_ext_lpbk_config()
{
    int i;
    int max_front_ports = get_num_ports();
    FILE *fp;
    char status_file[32];
    char cmd_str[100];
    char pname[10];
    int speed = SPD_1000MBPS;
    int retries = 4;

    sprintf(status_file, "/tmp/cu_lpbk_reg_dump");

    fp = fopen(status_file, "w+");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    while (retries--) {
        for (i = 0; i < max_front_ports; i++, i++) {
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s down", pname);
            printf("%s\n", cmd_str);
            system(cmd_str);
        }

        if (hw_reset_init_phy(FALSE, COPPER_MODE)) {
            printf("PHY Initialize Failed.\n");
            continue;
        }
        save_port_ctrl_stat(fp, "After Init PHY");

        for (i = 0; i < max_front_ports; i++, i++) {
            /* ifconfig up */
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s up", pname);
            printf("\n%s\n", cmd_str);
            system(cmd_str);

            /* use ethtool to config the eth driver */
            sprintf(cmd_str, "ethtool -s %s autoneg on speed %d", pname, speed);
            printf("%s\n", cmd_str);
            system(cmd_str);
            sleep(ETH_DRIVER_DELAY * 3);
        }
        save_port_ctrl_stat(fp, "After ifconfig and ethtool");

        printf("Check PHY sync status.\n");
        if (phy_check_sync()) {
            printf("PHY no sync.\n");
            continue;
        }
        printf("Check MAC PCS link.\n");
        if (phy_check_mac_pcs_link()) {
            printf("MAC PCS link down.\n");
            continue;
        }
        printf("Check CPU PCS link.\n");
        if (cpu_check_pcs_link()) {
            printf("CPU PCS link down.\n");
            continue;
        }
        printf("PHY initialized with %d retries.\n", (3 - retries));
        sleep(ETH_DRIVER_DELAY * 2);
        save_port_ctrl_stat(fp, "After config");
        fclose(fp);

        return (PASSED);
    }
    fclose(fp);
    return (FAILED);
}

/******************************************************************************
 *
 * Function: phy_cu_ext_lpbk_test
 *  PHY Frontplane Copper port external loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_cu_ext_lpbk_test()
{
    int rc = PASSED;
    int i;
    int max_front_ports = get_num_ports();
    int speed = SPD_1000MBPS;

    testname("PHY Copper External Loopback");

    /* Skip the test if EXT_LPBK flag is off */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("Ext. loopback is off, skip the test\n");
        return (PASSED);
    }

    /* If current mode is not Copper, configure the PHY */
    if (mode_ext_cu == 0) {
        if (phy_cu_ext_lpbk_config()){
            cterr('f', 0, "PHY Copper External Loopback Initialize Failed.");
            return (FAILED);
        }
    }
    /* Now the PHY should be in Copper mode */
    mode_ext_cu = 1;
    mode_ext_sfp = 0;
    mode_internal = 0;

    for (i = 0; i < max_front_ports; i++, i++) {
        rc = wallander_set_phy_loopback(i, FALSE, FALSE);
        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "Set PHY Loopback Failed.");
            return rc;
        }
        /* For 2-port SKU, set another front panel port as line loopback */
        if (get_board_id() == 1) {
            int another_port = i ^ 2;
            rc = wallander_set_phy_loopback(another_port, FALSE, TRUE);
            if (rc != PASSED) {
                show_port_ctrl_stat_util();
                cterr('f', 0, "Set PHY Loopback Failed.");
                return rc;
            }
        }
        sleep(ETH_DRIVER_DELAY * 3);

        rc = phy_media_is_linkup(i);
        if (rc != PASSED) {
            /* Dump regsters in Page 0 & 1 for debug */
            phy_reg_port_page_dump(i, 0);
            phy_reg_port_page_dump(i, 1);

            /* Restart auto-neg on the port */
            rc = phy_restart_aneg(i);
            if (rc != PASSED) {
                phy_reg_port_page_dump(i, 0);
                phy_reg_port_page_dump(i, 1);
                cterr('f', 0, "Restart Auto-Neg Failed.");
                return rc;
            }

            /* Check link again */
            rc = phy_media_is_linkup(i);
            if (rc != PASSED) {
                show_port_ctrl_stat_util();
                cterr('f', 0, "PHY Media link is down.");
                return rc;
            }
        }

        /* Start loopback test */
        rc = wallander_packet_lpbk_test(i, speed, 0);
        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "PHY Copper External Loopback Test Failed.");
            return rc;
        }

        /* For 2-port SKU, restore another front panel port to normal mode */
        if (get_board_id() == 1) {
            int another_port = i ^ 2;
            rc = wallander_set_phy_loopback(another_port, FALSE, FALSE);
            if (rc != PASSED) {
                show_port_ctrl_stat_util();
                cterr('f', 0, "Set PHY Loopback Failed.");
                return rc;
            }
        }

    }

    return (rc);
}

/******************************************************************************
 *
 * Function: phy_bp_ext_lpbk_test
 *  PHY Backplan port external loopback test, need to set line loopback in Host.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_bp_ext_lpbk_test()
{
    int rc = PASSED;
    int i;
    int max_phy_ports = get_num_ports();
    char cmd_str[100];
    char pname[10];

    FILE *fp;
    char status_file[32];

    sprintf(status_file, "/tmp/bp_lpbk_reg_dump");

    testname("PHY Backplane External Loopback");
    if (getc_answer("Please enable Backplane line loopback first. Enabled? (y/n)", 
        "yn", 'y') == 'n') {
        return rc;
    }

    /* If current mode is Internal, configure the PHY as copper mode*/
    if (mode_internal == 1) {
        if (phy_cu_ext_lpbk_config()){
            cterr('f', 0, "PHY Copper External Loopback Initialize Failed.");
            return (FAILED);
        }
    }
    /* Now the PHY should be in Copper mode */
    mode_ext_cu = 1;
    mode_ext_sfp = 0;
    mode_internal = 0;

    /* Skip the test if EXT_LPBK flag is off */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("Ext. loopback is off, skip the test\n");
        return (PASSED);
    }

    fp = fopen(status_file, "w+");
    if (fp == NULL) {
        show_port_ctrl_stat_util();
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    for (i = /*1*/3; i < max_phy_ports; i++, i++) {
        sprintf(pname,"eth%d", i);
        sprintf(cmd_str, "ifconfig %s down", pname);
        printf("%s\n", cmd_str);
        system(cmd_str);
    }
    for (i = /*1*/3; i < max_phy_ports; i++, i++) {
        sprintf(pname,"eth%d", i);
        sprintf(cmd_str, "ifconfig %s up", pname);
        printf("%s\n", cmd_str);
        system(cmd_str);
        sprintf(cmd_str, "ethtool -s %s autoneg off speed 1000  duplex full", pname);
        printf("%s\n", cmd_str);
        system(cmd_str);
        sleep(ETH_DRIVER_DELAY * 3);

        rc = phy_media_is_linkup(i);
        if (rc != PASSED) {
            fclose(fp);
            show_port_ctrl_stat_util();
            cterr('f', 0, "PHY Media link is down.");
            return rc;
        }
        save_port_ctrl_stat(fp, "Checking link");

        /* Start loopback test */
        rc = wallander_packet_lpbk_test(i, SPD_1000MBPS, 0);
        save_port_ctrl_stat(fp, "After Test");

        if (rc != PASSED) {
            fclose(fp);
            printf("\n==== Previous Register Dump ====\n");
            system("cat /tmp/bp_lpbk_reg_dump");
            cterr('f', 0, "PHY Backplane External Loopback Test Failed.");
            return rc;
        }
    }

    fclose(fp);
    return (rc);
}

/******************************************************************************
 *
 * Function: phy_ptp_ext_lpbk_config
 *  Init and configure PHY for PTP external loopback test.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_ptp_ext_lpbk_config()
{
    int i;
    int max_front_ports = get_num_ports();
    FILE *fp;
    char status_file[32];
    char cmd_str[100];
    char pname[10];
    int speed = SPD_1000MBPS;
    int retries = 4;

    sprintf(status_file, "/tmp/ptp_lpbk_reg_dump");

    fp = fopen(status_file, "w+");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    while (retries--) {
        for (i = 0; i < max_front_ports; i++, i++) {
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s down", pname);
            printf("%s\n", cmd_str);
            system(cmd_str);
        }

        if (hw_reset_init_phy(TRUE, FIBER_MODE)) {
            printf("PHY Initialize Failed.\n");
            continue;
        }
        save_port_ctrl_stat(fp, "After Init PHY");

        for (i = 0; i < max_front_ports; i++, i++) {
            printf("Enable SFP Tx for port %d\n", i);
            if (enable_sfp_tx(i/2)) {
                fclose(fp);
                show_port_ctrl_stat_util();
                cterr('f', 0, "Failed to enable SFP Tx.");
                return (FAILED);
            }
            /* ifconfig up */
            sprintf(pname,"eth%d", i);
            sprintf(cmd_str, "ifconfig %s up", pname);
            printf("\n%s\n", cmd_str);
            system(cmd_str);

            /* use ethtool to config the eth driver */
            sprintf(cmd_str, "ethtool -s %s autoneg on speed %d", pname, speed);
            printf("%s\n", cmd_str);
            system(cmd_str);
            sleep(ETH_DRIVER_DELAY * 3);
        }
        save_port_ctrl_stat(fp, "After ifconfig and ethtool");

        printf("Check PHY media link.\n");
        if (phy_check_media_link()) {
            printf("PHY media link down.\n");
            continue;
        }
        printf("Check PHY sync status.\n");
        if (phy_check_sync()) {
            printf("PHY no sync.\n");
            continue;
        }
        printf("Check MAC PCS link.\n");
        if (phy_check_mac_pcs_link()) {
            printf("MAC PCS link down.\n");
            continue;
        }
        printf("Check CPU PCS link.\n");
        if (cpu_check_pcs_link()) {
            printf("CPU PCS link down.\n");
            continue;
        }
        printf("PHY initialized with %d retries.\n", (3 - retries));
        sleep(ETH_DRIVER_DELAY * 2);
        save_port_ctrl_stat(fp, "After config");
        fclose(fp);

        return (PASSED);
    }
    fclose(fp);
    return (FAILED);
}

int phy_ptp_ext_lpbk_test()
{
    int rc = PASSED;
    int i;
    int max_front_ports = get_num_ports();
    int speed = SPD_1000MBPS;

    testname("PHY PTP External Loopback");

    /* Skip the test for 1-port SKU */
    if (get_board_id() != 1) {
        printf("This utility is for NIM-2GE-CU-SFP only.\n");
        return (PASSED);
    }
    /* Skip the test if EXT_LPBK flag is off */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("Ext. loopback is off, skip the test\n");
        return (PASSED);
    }

    /* Enable FPGA Timestamp function */
    if (fpga_enable_ts()) {
        cterr('f', 0, "Failed to enable FPGA Timestamp function.");
        return (FAILED);
    }
    if (fpga_enable_ts_intr()) {
        cterr('f', 0, "Failed to enable FPGA Timestamp interrupt.");
        return (FAILED);
    }


    /* Initialize the PHY with Timestamp enabled */
    if (phy_ptp_ext_lpbk_config()) {
        cterr('f', 0, "PHY with Timestamp enabled initialize Failed.");
        return (FAILED);
    }

    /* Load timestamp to PHY */
    if ( vsc_load_timestamp()) {
        cterr('f', 0, "Failed to load timestamp to PHY.");
        return (FAILED);
    }


    for (i = 0; i < max_front_ports; i++, i++) {
        /* Start loopback test */
        rc = wallander_packet_lpbk_test(i, speed, 1);

        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "PHY PTP External Loopback Test Failed.");
            return rc;
        }

        if (ptp_check_timestamp(i)) {
            cterr('f', 0, "PHY PTP External Loopback Test Failed.");
            return rc;
        }
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: phy_lpbk_test
 *  Perform loopback test on certain port, don't change any configuration of it.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_lpbk_test()
{
    int rc = PASSED;
    int i;
    int port_no;
    int passes;

    testname("PHY Loopback");

    if (get_num_ports() == 4) {
        port_no = getdec_answer("\nEnter port number[0 to 3]:",
                0, 0, 3);
    } else {
        port_no = getdec_answer("\nEnter port number[0 to 1]:",
                0, 0, 1);
    }

    passes = getdec_answer("\nRepeat times:",
            1, 1, 100000);

    /* Start loopback test */
    for (i = 0; i < passes; i++) {
        rc = wallander_packet_lpbk_test(port_no, SPD_1000MBPS, 0);
        if (rc != PASSED) {
            show_port_ctrl_stat_util();
            cterr('f', 0, "PHY Internal Loopback Test Failed in passes %d.", i);
            return rc;
        }
    }

    return (rc);
}

/******************************************************************************
 *
 * Function: phy_lpbk_config
 *  Utility to configure the loopback mode on certain port.
 *
 * Input:  None.
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
int phy_lpbk_config()
{
    int rc = PASSED;
    int port_no;
    int mode;

    testname("PHY Loopback Config");

    if (get_num_ports() == 4) {
        port_no = getdec_answer("\nEnter port number[0 to 3]:",
                0, 0, 3);
    } else {
        port_no = getdec_answer("\nEnter port number[0 to 1]:",
                0, 0, 1);
    }

    mode = getdec_answer("\nLoopback mode(0 - None; 1 - Near-end; 2 - Far-end):",
            0, 0, 2);

    /* Start loopback test */
    if (mode == 0) {
        /* System Lpbk -- FALSE; Line Lpbk -- FALSE */
        rc = wallander_set_phy_loopback(port_no, FALSE, FALSE);
        if (rc != PASSED) {
            cterr('f', 0, "Set PHY Loopback Failed.");
            return rc;
        }
    } else if (mode == 1) {
        /* System Lpbk -- TRUE; Line Lpbk -- FALSE */
        rc = wallander_set_phy_loopback(port_no, TRUE, FALSE);
        if (rc != PASSED) {
            cterr('f', 0, "Set PHY Near-end Loopback Failed.");
            return rc;
        }
    } else {
        /* System Lpbk -- TRUE; Line Lpbk -- TRUE */
        rc = wallander_set_phy_loopback(port_no, FALSE, TRUE);
        if (rc != PASSED) {
            cterr('f', 0, "Set PHY Far-end Loopback Failed.");
            return rc;
        }
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_ge_phy_test.c,v $
 * Revision 1.3  2015/07/14 08:12:37  xiaoyizh
 * Fix register page dump issue.
 *
 * Revision 1.2  2015/03/13 07:35:26  xiaoyizh
 * Clean up and move check_bitlock() into phy link/sync check.
 *
 * Revision 1.1  2015/02/26 07:18:29  xiaoyizh
 * Initial check in for Wallander.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

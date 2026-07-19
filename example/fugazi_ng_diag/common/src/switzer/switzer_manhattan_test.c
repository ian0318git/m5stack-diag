/* $Id: switzer_manhattan_test.c,v 1.3 2021/08/04 04:48:24 xiaolaya Exp $Port0 Link
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_manhattan_test.c,v $
 *------------------------------------------------------------------
 *
 * switzer_manhattan_test.c - Switzer-Manhattan NIM.
 *
 * Mar. 2020, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>

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

#ifdef TABEIL
#include "dnv_eth_lib.h"
#else
#include "platform_eth_pkt_txrx.h"
#endif

#include "switzer_traf.h"
#include "switzer_miura_reg.h"
#include "switzer_common.h"
#include "switzer_manhattan.h"
#include "switzer_manhattan_upoe.h"
#include "switzer_tps23881_sram_parity.h"

#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)

extern long manhattan_bcm54194_test(int show_menu);
extern int manhattan_bcm54194_led_test(int);
extern int manhattan_bcm54194_led_util(int, int);
extern int bcm54194_sfp_reg_dump(int arg);
extern int bcm54194_sfp_reg_rdwr(int rdwr);
int switzer_manhattan_sock_test(char *eth_port_s, char *eth_port_r);
static long utils_intnl_phy_rst(int arg);
static void switzer_manhattan_platform_exit(struct switzer_manhattan *mod);
static int switzer_manhattan_platform_init(struct switzer_manhattan *mod);

static struct switzer_manhattan __mod, *mod = &__mod;
static const int pca_i2c_addr[] = {SWITZER_MANHATTAN_I2C_ADDR_PCA1,
                                   SWITZER_MANHATTAN_I2C_ADDR_PCA2};

static long is_manhattan_2m(void)
{
    return is_switzer_manhattan_2m(mod);
}

static long is_manhattan_4t(void)
{
    return is_switzer_manhattan_4t(mod);
}

static long is_manhattan_1m(void)
{
    return is_switzer_manhattan_1m(mod);
}

static long is_manhattan_2t(void)
{
    return is_switzer_manhattan_2t(mod);
}

static char *__manhattan_board_name(void)
{
    return is_manhattan_2m() ? "C-NIM-2M (Switzer-Manhattan-2M)" :
           is_manhattan_4t() ? "C-NIM-4T (Switzer-Manhattan-4T)" :
           is_manhattan_1m() ? "C-NIM-1M (Switzer-Manhattan-1M)" :
           is_manhattan_2t() ? "C-NIM-2T (Switzer-Manhattan-2T)" : "Unknown";
}

static long __is_poe_available(void)
{
    if (is_manhattan_1m() || is_manhattan_2m() || is_manhattan_4t()) {
        /*TODO: check POE power supply */
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * This serves as a switch to enable BCM82757 and its subsidiaries.
 * This maybe removed later denpending on the final choose of BCM82757 and BCM54194.
 */
static long init_actual_4t_check(void)
{
    #define ENV_ACTUAL_C_NIM_4T_CHK "ACTUAL_C_NIM_4T_CHK"
    char *p = NULL;
    if ((p = getenv(ENV_ACTUAL_C_NIM_4T_CHK))) {
        mod->chk_4t = (strcasecmp(p, "y") == 0 || 0 == strcasecmp(p, "yes") || 0 == strcmp(p, "1"));
    } else {
        mod->chk_4t = 0;
    }
    prt("Env ACTUAL_C_NIM_4T_CHK=%s\n", p ? p : "");
    prt("Do actual 4T check:%s\n", mod->chk_4t ? "Yes" : "No");
    return 0;
}

static long init_mdio_dbg_flag_check(void)
{
    #define ENV_MDIO_DBG_FLAG "ENV_MDIO_DBG_FLAG"
    char *p = NULL;
    if ((p = getenv(ENV_MDIO_DBG_FLAG))) {
        mod->mdio_dbg = (strcasecmp(p, "y") == 0 || 0 == strcasecmp(p, "yes") || 0 == strcmp(p, "1"));
    } else {
        mod->mdio_dbg = 0;
    }
    prt("Env ENV_MDIO_DBG_FLAG=%s\n", p ? p : "");
    prt("mdio_dbg:%s\n", mod->mdio_dbg ? "Yes" : "No");
    return 0;
}

static long is_manhattan_init_done(int negtive)
{
    return negtive ?
           !(switzer_manhattan_stage_get(mod) >= SWITZER_MANHATTAN_STAGE_INIT_DONE)
            :
            (switzer_manhattan_stage_get(mod) >= SWITZER_MANHATTAN_STAGE_INIT_DONE);
}

static long is_manhattan_init_okay(void) {
    return is_manhattan_init_done(0);
}

static long is_manhattan_init_fail(void) {
    return is_manhattan_init_done(1);
}

static long is_interactive(int upd, int intact)
{
    static int _intact = 0;
    if (upd)
        _intact = !!intact;
    return _intact;
}

static long interactive_flag_set(void)
{
    int val = is_interactive(0, 0);
    prt("If interactive is true, some tests will be interactive.\n");
    val = getdec_answer("Select interactive test flag(0/1)", val, 0, 1);
    is_interactive(1, val);
    return PASSED;
}

static long mdio_dbg_flag(void)
{
    int val = 0;
    prt("Current status '%s'\n", mod->mdio_dbg ? "Enabled" : "Disabled");
    val = getdec_answer("0-disable, 1-enable", mod->mdio_dbg, 0, 1);
    mod->mdio_dbg = val;
    prt("New     status '%s'\n", mod->mdio_dbg ? "Enabled" : "Disabled");
    return PASSED;
}

static long switzer_manhattan_debug_flag(void)
{
    int val = 0;
    val = getdec_answer("Select flag:\n"
                        "  0: Interactive flag(some test will be interactive)\n"
                        "  1: Mdio rd/wr param dump flag\n"
                        "> ", 0, 0, 1);
    switch(val) {
    case 0:
        return interactive_flag_set();
    case 1:
        return mdio_dbg_flag();
    }
    return PASSED;
}

enum {
    MANHATTAN_LINK_UP=0,
    MANHATTAN_LINK_DOWN,
};
static int _iflink_wait(const char *ifname, int state, time_t timeout)
{
#define LINK_CHECK_INTERVAL 200
#define LINK_CHECK_COUNT 3
    struct ifreq ifr;
    struct ethtool_value evalue;
    int sockfd;
    int rc, i, n, c;

    printf("Wait for %s link up\n", ifname);

    evalue.cmd = ETHTOOL_GLINK;
    evalue.data = 0;
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_data = (char *)&evalue;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("error: socket: %s\n", strerror(errno));
        return -1;
    }

    rc = -1;
    i = 0;
    c = 0;
    n = timeout / LINK_CHECK_INTERVAL;
    while (1) {
        if (ioctl(sockfd, SIOCETHTOOL, &ifr) == -1) {
            printf("error: SIOCETHTOOL: %s\n", strerror(errno));
            break;
        }
        if (state == MANHATTAN_LINK_UP)
            rc = evalue.data ? 0 : -1;
        else
            rc = evalue.data ? -1 : 0;
        if (rc)
            c = 0;
        else if (++c >= LINK_CHECK_COUNT)
            break;
        if (n > 0 && ++i > n)
            break;
        switzer_mdelay(LINK_CHECK_INTERVAL);
        printf(".");
    }
    printf("%s\n", !rc ? "OK" : "Failed");

    close(sockfd);
    return rc;
#undef LINK_CHECK_INTERVAL
#undef LINK_CHECK_COUNT
}

static long debugfs_mnt_pnt(char *cmnt, const char *mnt)
{
    FILE *fp = NULL;
    char *ptr= NULL;
    char  buf[1024] = {0,};
    const char *cmd = "mount 2>/dev/null |grep 'type\\s\\+debugfs'";

    ERET_COND(!(fp = popen(cmd, "r")), -1, "Failed to run cmd '%s'\n", cmd);
    while ((fgets(buf, sizeof(buf), fp))) {
        /* 'nodev on /sys/kernel/debug type debugfs (rw,relatime)' */
        ptr = strstr(buf, "type debugfs");
        if (ptr) {
            for(ptr -= 1; ptr > buf &&  isspace(*ptr); ptr--);
            *(ptr + 1) = 0;
            for(        ; ptr > buf && !isspace(*ptr); ptr--);
            if (ptr > buf) {
                ptr += 1;
                if (mnt) {
                    strcpy(cmnt, ptr);
                }
                pclose(fp);
                return 0;
            }
            continue;
        }
    }
    pclose(fp);
    if (mnt) {
        snprintf(buf, sizeof(buf), "[ ! -d %s ] && mkdir -p %s; mount -t debugfs nodev %s", mnt, mnt, mnt);
        ERET_COND(0 != system(buf), -1, "Run '%s' failed.\n", buf);
        strcpy(cmnt, mnt);
        /* prt("debugfs is mounted at '%s'\n", mnt); */
        return 0;
    }
    return -1;
}

static long utils_phy_reg_read(void)
{
    int rc;
    switzer_lane_t lane;
    uint32_t data, regaddr, devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);

    rc = switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 read error");
        return FAILED;
    }
    prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);

    return PASSED;
}

static long utils_phy_reg_write(void)
{
    int rc;
    switzer_lane_t lane;
    uint32_t data, regaddr, devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 write error");
        return FAILED;
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

static long phy_registers_dump(void)
{
    int rc;
    switzer_lane_t lane;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_manhattan_registers_dump(mod, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 registers dump error");
        return FAILED;
    }
    return PASSED;
}

static long utils_intnl_phy_reg_rdwr(const int op)
{
    int      rc      = PASSED;
    uint32_t data    = 0;
    uint32_t port    = 0;
    uint32_t devaddr = 0;
    uint32_t regaddr = 0;

    if (is_manhattan_1m()) {
        port = 0;
    } else {
        port = getdec_answer("Enter port num(0/1):", 0, 0, 1);
    }

    devaddr = getdec_answer("Enter dev addr(1, 3, 7, 30):", 0, 0, 30);
    switch(devaddr) {
        case 1: case 3: case 7: case 30: break;
        default:
            cterr('f', 0, "Invalid dev addr:%d\n", devaddr);
            rc = FAILED; goto _EXIT_POINT;
    }

    regaddr = gethex_answer("Enter reg addr(0 - 0xffff):", 0, 0, 0xffff);
    if (regaddr > 0xffff) {
        cterr('f', 0, "Invalid reg addr:0x%x\n", regaddr);
        rc = FAILED; goto _EXIT_POINT;
    }

    if (op == 0) { /* read */
        rc = switzer_manhattan_iphy_read(mod, (port << 16) | devaddr, regaddr, &data);
        if (rc != PASSED) {
            rc = FAILED; goto _EXIT_POINT;
        }
        prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
        goto _EXIT_POINT;
    }

    /* write */
    data = gethex_answer("Enter data to write(0 - 0xffff):", 0, 0, 0xffff);
    if (data > 0xffff) {
        cterr('f', 0, "Invalid reg data:0x%x\n", data);
        rc = FAILED; goto _EXIT_POINT;
    }

    rc = switzer_manhattan_iphy_write(mod, devaddr, regaddr, data);
    if (rc != PASSED) {
        rc = FAILED; goto _EXIT_POINT;
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);


    rc = PASSED;
_EXIT_POINT:
    if (rc == FAILED) {
        cterr_add_component("X710 Internal Phy",
                            "MDIO controller within V710");
        cterr_add_debug("X710 Internal Phy",
                        "Check MDIO controller within V710");
        cterr('f', 0, "X710 Internal phy %s error", op == 0? "read" : "write");
    }
    return rc;
}

static long utils_intnl_phy_reg_read(void)
{
    return utils_intnl_phy_reg_rdwr(0);
}

static long utils_intnl_phy_reg_write(void)
{
    return utils_intnl_phy_reg_rdwr(1);
}

static long x710_aq_cmd_send(const int port, int indirect, const char *cmd, char *result, const int size)
{
    int bus  = 0;
    char mnt[512 ] = {0,};
    char cmd_buf[1024] = {0,};

    ERET_COND(0 > (bus = get_ngio_pcie_dev_bus_num(mod->ngio->mod_type, mod->ngio->slot)),
        FAILED, "Invalid bus number %d\n", bus);

    ERET_COND(0 != debugfs_mnt_pnt(mnt, "/debugfs_mnt"), FAILED, "Failed to get debugfs mount point.\n");

    /* echo send aq_cmd $CMD > /sys/kernel/debug/i40e/0000:31:00.0/command"; */
    snprintf(cmd_buf, sizeof(cmd_buf), "echo send%saq_cmd %s > %s/i40e/0000:%02x:00.%x/command",
            indirect ? " indirect " : " ", cmd, mnt, bus, port);
    ERET_COND(0 != system(cmd_buf), FAILED, "Failed to run '%s'\n", cmd_buf);
    sleep(1);

    if (result && size > 0) {
        int pos  = 0;
        int len  = 0;
        FILE *fp = NULL;
        memset(result, 0, size);
        memset(cmd_buf, 0, sizeof(cmd_buf));
        snprintf(cmd_buf, sizeof(cmd_buf), "cat %s/i40e/0000:%02x:00.%x/command", mnt, bus, port);
        ERET_COND(!(fp = popen(cmd_buf, "r")), FAILED, "Failed to run '%s'\n", cmd_buf);

        while(1) {
            len = fread(result + pos, 1, size - pos - 1, fp);
            if (len > 0)
                pos += len;
            if (feof(fp) || pos >= size - 1) {
                break;
            }
            if (ferror(fp)) {
                pos = -1;
                break;
            }
        }
        fclose(fp);
        if (pos <= 0)
            return FAILED;
    }

    return PASSED;
}

enum {
    X710_INTL_PHY_100M = 0,
    X710_INTL_PHY_1G      ,
    X710_INTL_PHY_2P5G    ,
    X710_INTL_PHY_RE_AN   ,
};
static long utils_intnl_phy_set_speed(const int speed)
{
    /* TODO: Export aq_command interface instead of using debugfs */
    int port = 0;
    int bus  = 0;
    const char *aq_cmd[] = {
        "0 0x601 0 0 0 0 0x600c3 0x60902  0 0x4000 ",   //100M
        "0 0x601 0 0 0 0 0x40000 0x80904  0 0x601  ",   //1G
        "0 0x601 0 0 0 0 0x600ab 0x3e0907 0 0xc000 ",   //2.5G
        "0 0x605 0 0 0 0 0x3     0        0 0      ",   //re-auto neg
        NULL,
    };

    if (is_manhattan_1m()) {
        port = 0;
    } else {
        port = getdec_answer("Enter RJ45 port num(0/1):", 0, 0, 1);
    }
    ERET_COND(0 > (bus = get_ngio_pcie_dev_bus_num(mod->ngio->mod_type, mod->ngio->slot)),
        FAILED, "Invalid bus number %d\n", bus);

    switch(speed) {
    case X710_INTL_PHY_100M :
    case X710_INTL_PHY_1G   :
    case X710_INTL_PHY_2P5G :
        ERET_COND(PASSED != x710_aq_cmd_send(port, 0, aq_cmd[speed], NULL, 0), FAILED, "Failed.\n");
        break;
    default                 :
        ERET_COND(1, FAILED, "Invalid speed:%d\n", speed); break;
    }
    /* Trigger auto negotiation */
    ERET_COND(PASSED != x710_aq_cmd_send(port, 0, aq_cmd[X710_INTL_PHY_RE_AN], NULL, 0), FAILED, "Failed.\n");

    return PASSED;
}

static long utils_intnl_phy_test_mode(int arg)
{
    int      deva = 7;
    int      rega = 0xFFE9;
    int      port = 0;
    int      tstm = 0;
    int      i    = 0;
    uint32_t regv = 0;

    const struct {
        int   val;
        char *desc;
    } tst_mode[] = {
        {4, "Test mode 4, Transmitter distortion test."},
        {3, "Test mode 3, Slave transmit jitter test."},
        {2, "Test mode 2, Master transmit jitter test."},
        {1, "Test mode 1, Transmit waveform test."},
        {0, "Normal operation"},
        {-1, NULL}
    };

    if (is_manhattan_1m()) {
        port = 0;
    } else {
        port = getdec_answer("Port 0/1", 0, 0, 1);
    }

    printf("Choose test mode:\n");
    for(i = 0; tst_mode[i].desc; i++) {
        printf("  %d: %s\n", i, tst_mode[i].desc);
    }
    i = getdec_answer(">", 4, 0, 4);
    tstm = tst_mode[i].val;

    utils_intnl_phy_rst((port << 4) | 1); //soft reset

    // Force 1000BASE-T
    regv = 0x140;
    rega = 0xFFE0;
    ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
            FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);

    // Set Test mode
    rega = 0xFFE9;
    ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
            FAILED, "Internal PHY read port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
    regv &= ~(7 << 13);
    regv |= tstm << 13;

    ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
            FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);

    return 0;
}

static long utils_intnl_phy_mdix_mode(int arg)
{
    int       port = 0;
    int       mode = 0; //0-auto, 1-forced no-swap-RX, 2-forced swap-RX
    int       speed= 0;
    int       rega = 0;
    const int deva = 7;
    uint32_t  regv = 0;

    if (is_manhattan_1m()) {
        port = 0;
    } else {
        port = getdec_answer("Port 0/1", 0, 0, 1);
    }

    mode = getdec_answer("MDIX Mode:\n"
                         "  0: Auto\n"
                         "  1: Forced no-swap-RX\n"
                         "  2: Forced swap-RX\n"
                         "> ", 0, 0, 2);
    speed = 0; //100Mbps

    switch (mode) {
    case 0:
        //0, clear force bit
        rega = 0xFFF8;
        ////0.0, select Misc Control Reg.
        regv = (0x7 << 12) | 0x7;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        ////0.1, read Misc Control Reg.
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
                FAILED, "Internal PHY read  port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("RD port-%d dev-0x%04x reg-0x%04x (Misc ctrl): 0x%04x\n", port, deva, rega, regv);
        ////0.2, clear force MDX
        regv &= ~(1 << 9);
        regv |= 0x7; //select misc ctrl reg
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (Misc ctrl): 0x%04x\n", port, deva, rega, regv);

        //1, clear swap rx bit
        rega = 0xFFF8;
        ////1.0, select Misc Test Reg.
        regv = (0x4 << 12) | 0x7;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        ////1.1, read Misc Test Reg.
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
                FAILED, "Internal PHY read  port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("RD port-%d dev-0x%04x reg-0x%04x (Misc test): 0x%04x\n", port, deva, rega, regv);
        ////1.2, clear swap RX MDI
        regv &= ~(1 << 4);
        regv &= ~0x7;
        regv |= 0x4;    //select misc test reg
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (Misc test): 0x%04x\n", port, deva, rega, regv);

        //3, enable auto nego
        rega = 0xFFE0;
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
                FAILED, "Internal PHY read  port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("RD port-%d dev-0x%04x reg-0x%04x (MII ctrl) : 0x%04x\n", port, deva, rega, regv);
        regv |= 1 << 12;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (MII ctrl) : 0x%04x\n", port, deva, rega, regv);
        break;
    case 1:
    case 2:
        //0, disable auto nego and select speed
        rega = 0xFFE0;
        regv &= ~(1 << 12);
        regv |= 1 << 8; // full duplex
        if (speed == 0) {
            regv &= ~(1 << 6);
            regv |= 1 << 13;
        } else {
            regv &= 1 << 6;
            regv &= ~(1 << 13);
        }
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (MII ctrl) : 0x%04x\n", port, deva, rega, regv);

        //1, auto advert config
        rega = 0xFFE4;
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
                FAILED, "Internal PHY read  port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("RD port-%d dev-0x%04x reg-0x%04x (Auto-neg Advert) : 0x%04x\n", port, deva, rega, regv);
        regv |= (3 << 7) | 1;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (Auto-neg Advert) : 0x%04x\n", port, deva, rega, regv);

        //2, Set/clear swap rx bit
        rega = 0xFFF8;
        ////1.0, select Misc Test Reg.
        regv = (0x4 << 12) | 0x7;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        ////1.1, read Misc Test Reg.
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
                FAILED, "Internal PHY read  port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("RD port-%d dev-0x%04x reg-0x%04x (Misc test): 0x%04x\n", port, deva, rega, regv);
        ////1.2, Set/clear swap RX MDI
        if (mode == 2) {
            regv |= (1 << 4);
        } else {
            regv &= ~(1 << 4);
        }
        regv &= ~0x7;
        regv |= 0x4;    //select misc test reg
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (Misc test): 0x%04x\n", port, deva, rega, regv);

        //3, set force bit
        rega = 0xFFF8;
        ////0.0, select Misc Control Reg.
        regv = (0x7 << 12) | 0x7;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        ////0.1, read Misc Control Reg.
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &regv),
                FAILED, "Internal PHY read  port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("RD port-%d dev-0x%04x reg-0x%04x (Misc ctrl): 0x%04x\n", port, deva, rega, regv);
        ////0.2, clear force MDX
        regv |= 1 << 9;
        regv |= 0x7; //select misc ctrl reg
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, regv),
                FAILED, "Internal PHY write port-%d dev-0x%04x reg-0x%04x failed.\n", port, deva, rega);
        printf("WR port-%d dev-0x%04x reg-0x%04x (Misc ctrl): 0x%04x\n", port, deva, rega, regv);

        break;
    default:
        ERET_COND(1, FAILED, "Something must be wrong.\n");
    }
    printf("Done.\n");
    return PASSED;
}

static long utils_intnl_phy_rst(int arg)
{
    uint32_t data = 0;
    int      port = 0;
    int      act  = 0;

    if (arg < 0) {
        act = getdec_answer("0:Hard reset\n"
                            "1:Soft reset\n"
                            "> ", 0, 0, 1);
        if (is_manhattan_1m()) {
            port = 0;
        } else {
            port = getdec_answer("Port 0/1", 0, 0, 1);
        }
    } else {
        act =  arg & 0xf;
        port= (arg & 0xf0) >> 4;
    }

    if (act == 0) {
        printf("Reset...\n");
        ERET_COND(0 != switzer_manhattan_x710_gpio_set(mod, 0, SWITZER_MANHATTAN_INTPHY_RST, 0), FAILED, "Failed.\n");
        switzer_mdelay(1000);
        ERET_COND(0 != switzer_manhattan_x710_gpio_set(mod, 0, SWITZER_MANHATTAN_INTPHY_RST, 1), FAILED, "Failed.\n");
        printf("Un-reset...\n");
        switzer_mdelay(1000);
        printf("Done.\n");
    } else {
        /* 1. Write DEVAD 0x1E register address 0x4191 = 0x0001.
        ** 2. Wait for 200us.
        ** 3. Write DEVAD =1/3 0x0000 bit 15 = 1.
        ** 4. Check DEVAD = 1/3 for 0x2040 value by firmware to acknowledge of completion of soft reset operation
        */

        /* soft reset notify */
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | 0x1E, 0x4191, 0x1), FAILED, "Failed mdio write.\n");
        switzer_mdelay(1);
        /* reset pmd */
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | 0x1, 0x0, &data), FAILED, "Failed mdio read.\n");
        data |= 1 << 15;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | 0x1, 0x0, data), FAILED, "Failed mdio write.\n");
        /* reset pcs */
        ERET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | 0x3, 0x0, &data), FAILED, "Failed mdio read.\n");
        data |= 1 << 15;
        ERET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | 0x3, 0x0, data), FAILED, "Failed mdio write.\n");
        switzer_mdelay(20);
    }
    return 0;
}

static long utils_intnl_phy_cable_diag(int arg)
{
    uint32_t deva = 0;
    uint32_t rega = 0;
    uint32_t data = 0;
    uint32_t lenv = 0;
    int      port = 0;
    int      unit = 0; /* Cable lenght unit */
    int      cable= 0; /* Cable type */
    int      i    = 0;
    int      uret = 0;

    if (is_manhattan_1m()) {
        port = 0;
    } else {
        port = getdec_answer("Port 0/1", 0, 0, 1);
    }

    unit = getdec_answer("Select Cable Lenght Unit:\n"
                         "  0: Centimeters\n"
                         "  1: Meters\n"
                         "> ", 1, 0, 1);
    cable= getdec_answer("Select Cable Type:\n"
                         "  0: Cat 6a cable with 10G/5G/2.5G PHY link partner.\n"
                         "  1: Cat 5e cable with 10G/5G/2.5G PHY link partner.\n"
                         "  2: Cat 6  cable with 10G/5G/2.5G PHY link partner.\n"
                         "  3: Cat 6a or cat 6 cable with   GPHY link partner.\n"
                         "  4: Cat 5e cable with            GPHY link partner.\n"
                         "> ", 0, 0, 4);

    utils_intnl_phy_rst((port << 4) | 1);

    deva = 0x1e;
    rega = 0x4006;
    data = 0x8000 | (unit << 10) | (cable << 7);
    EURET_COND(0 != switzer_manhattan_iphy_write(mod, (port << 16) | deva, rega, data), uret, FAILED,
            "Failed to write MDIO port-%d dev-0x%04x reg-0x%04x.\n", port, deva, rega);
    switzer_mdelay(1);
    printf("Cable diagnostic running\n");
    PROMPT_WAIT_COND_MS(0 == switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &data) && !(data & 0x0800), 100, 10000);
    EURET_COND(data & 0x0800, uret, FAILED, "Wait for cable diag done timeout.\n");

    deva = 0x1;
    rega = 0xA896;
    EURET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &data), uret, FAILED,
            "Failed to read  MDIO port-%d dev-0x%04x reg-0x%04x.\n", port, deva, rega);
    for(i = 0; i < 4; i++) {
        printf("Pair-%c:", 'A' + i);
        rega = 0xA897 + i;
        EURET_COND(0 != switzer_manhattan_iphy_read(mod, (port << 16) | deva, rega, &lenv), uret, FAILED,
                "Failed to read  MDIO port-%d dev-0x%04x reg-0x%04x.\n", port, deva, rega);
        switch((data & (0xf << (i * 4))) >> (i * 4)) {
        case 0:
            printf("Fault, Cable diagnostics routine was unable to identify the condition of this pair.\n"
                   "       Might be identified on a subsequent cable diagnostics attempt.\n");
            break;
        case 1:
            printf("OK, No fault detected\n");
            printf("       Length is %u.%u%s ", lenv / 100, lenv % 100, unit == 0 ? "Centimeters" : "Meters");
            break;
        case 2:
            printf("Fault, Pair open or Rt > 115 OHM\n");
            break;
        case 3:
            printf("Fault, Intra-pair-short or Rt < 85 OHM\n");
            break;
        case 4:
            printf("Fault, Inter-pair-short or Rt < 85 OHM\n");
            break;
        case 9:
            printf("Fault, Pair busy, link partner in 100BASE-TX forced mode, 1000BASE-T test mode, or pair condition unidentified.\n");
            break;
        default:
            printf("Fault, unknown Diagnostics Code  0x%x\n", (data & (0xf << (i * 4))) >> (i * 4));
            break;
        }
        printf("       (Length reg value:0x%04x)\n", lenv);
    }

    uret = PASSED;
_EXIT_POINT:
    utils_intnl_phy_rst((port << 4) | 1);
    return uret;
}


static long utils_x710_reg_rdwr(const int op)
{
    int      rc      = 0;
    uint32_t data    = 0;
    uint32_t regaddr = 0;

    regaddr = gethex_answer("Enter reg addr(0 - 0xffffff):", 0, 0, 0xffffff);
    if (regaddr > 0x1000000) {
        cterr('f', 0, "Invalid reg addr:0x%x\n", regaddr);
        rc = FAILED; goto _EXIT_POINT;
    }

    if (op == 0) { /* read */
        if (0 != (rc = switzer_manhattan_x710_read(mod, 0, regaddr, &data))) {
            rc = FAILED; goto _EXIT_POINT;
        }
        prt("%#.8x --> %#.8x\n", regaddr, data);
    } else {
        data = gethex_answer("Enter data to write(0 - 0xffffffff):", 0, 0, 0xffffffff);
        if (0 != (rc = switzer_manhattan_x710_write(mod, 0, regaddr, data))) {
            rc = FAILED; goto _EXIT_POINT;
        }
        prt("%#.8x <-- %#.8x\n", regaddr, data);
    }

    rc = PASSED;
_EXIT_POINT:
    if (rc == FAILED) {
        cterr_add_component("X710 Register",
                            "X710 Register Rd/Wr");
        cterr_add_component("X710 Register",
                            "Check Nim Power and PCIE Status");
        cterr('f', 0, "%s X710 register error", op == 0? "read" : "write");
    }
    return rc;
}
static long utils_x710_reg_read(void)
{
    return utils_x710_reg_rdwr(0);
}

static long utils_x710_reg_write(void)
{
    return utils_x710_reg_rdwr(1);
}

static long utils_x710_gpio_set(void)
{
    int rc  = PASSED;
    int idx = 0;
    int bit = 0;

    idx = getdec_answer("Enter Gpio index(0 - 29):", 0, 0, I40E_GPIO_IDX_MAX);
    bit = getdec_answer("Enter Gpio value(0/1):", 0, 0, 1);
    rc = switzer_manhattan_x710_gpio_set(mod, 0, idx, bit);
    EURET_COND(rc < 0, rc, FAILED, "Failed to set Gpio-%d\n", idx);
    prt("OK\n");

    rc = PASSED;
_EXIT_POINT:
    return rc;
}

struct x710_gpio_ctl_fld {
    char *sname;
    char *lname;
    int  shift;
    int  mask;
    int  *desc;
};

#define X710_GPIO_CTRL_FLD(___fld, ___fname, ___desc) \
    #___fname, \
    #___fld,   \
    I40E_GLGEN_GPIO_CTL_##___fld##_SHIFT, \
    I40E_GLGEN_GPIO_CTL_##___fld##_MASK , \
    ___desc

static struct x710_gpio_ctl_fld gpio_ctrl_flds [] = {
    {X710_GPIO_CTRL_FLD(PRT_NUM     , PRT      , NULL)},
    {X710_GPIO_CTRL_FLD(PRT_NUM_NA  , PRT_NA   , NULL)},
    {X710_GPIO_CTRL_FLD(PIN_DIR     , DIR      , NULL)},
    {X710_GPIO_CTRL_FLD(TRI_CTL     , TRI_CTL  , NULL)},
    {X710_GPIO_CTRL_FLD(OUT_CTL     , OUT_CTL  , NULL)},
    {X710_GPIO_CTRL_FLD(PIN_FUNC    , FUN      , NULL)},
    {X710_GPIO_CTRL_FLD(LED_INVRT   , LED_INVT , NULL)},
    {X710_GPIO_CTRL_FLD(LED_BLINK   , LED_BLNK , NULL)},
    {X710_GPIO_CTRL_FLD(LED_MODE    , LED_M    , NULL)},
    {X710_GPIO_CTRL_FLD(INT_MODE    , INT_M    , NULL)},
    {X710_GPIO_CTRL_FLD(OUT_DEFAULT , OUT_DEF  , NULL)},
    {X710_GPIO_CTRL_FLD(PHY_PIN_NAME, PHY_PIN  , NULL)},
    {X710_GPIO_CTRL_FLD(PRT_BIT_MAP , PRT_BMAP , NULL)},
    {NULL, NULL, 0, 0, NULL},
};

static long __x710_gpio_ctrl_parse(int idx, uint32_t ctrl, int title)
{
    int i   = 0;
    int len = 0;
    int ebit= 0;
    char fmt[64];
    char str[64];

    #define _SBIT32(_MSK)   \
        ({int i_ = 0; for(i_ = 0; (_MSK) && i_ < 32 && ((_MSK) & (1 << i_)) == 0; i_++); (_MSK) ? i_ : -1; })
    #define _EBIT32(_MSK)   \
        ({int i_ = 0; for(i_ = 31;(_MSK) && i_ >= 0 && ((_MSK) & (1 << i_)) == 0; i_--); (_MSK) ? i_ : -1; })

    if (title) {
        prt("    ");
        for(i = 0; gpio_ctrl_flds[i].sname; i++) {
            prt("%s ", gpio_ctrl_flds[i].sname);
        }
        prt("\n");

        prt("    ");
        for(i = 0; gpio_ctrl_flds[i].sname; i++) {
            len = strlen(gpio_ctrl_flds[i].sname);
            memset(fmt, 0, sizeof(fmt));
            memset(str, 0, sizeof(str));
            snprintf(fmt, sizeof(fmt), "%%-%ds ", len);
            ebit = _EBIT32(gpio_ctrl_flds[i].mask);
            if (gpio_ctrl_flds[i].shift == ebit) {
                snprintf(str, sizeof(str), "%d", gpio_ctrl_flds[i].shift);
            } else {
                snprintf(str, sizeof(str), "%d:%d", gpio_ctrl_flds[i].shift, ebit);
            }
            prt(fmt, str);
        }
        prt("\n");
    }

    prt("%2d: ", idx);
    for(i = 0; gpio_ctrl_flds[i].sname; i++) {
        len = strlen(gpio_ctrl_flds[i].sname);
        memset(fmt, 0, sizeof(fmt));
        memset(str, 0, sizeof(str));
        if (gpio_ctrl_flds[i + 1].sname) {
            snprintf(fmt, sizeof(fmt), "%%-%dx ", len);
        } else {
            /* last one */
            snprintf(fmt, sizeof(fmt), "%%-4x");
        }
        prt(fmt, (ctrl & gpio_ctrl_flds[i].mask) >> gpio_ctrl_flds[i].shift);
    }
    prt("  (0x%08x) %s\n", ctrl, I40E_GLGEN_GPIO_PIN_NAME(idx));

    return 0;
}

static inline int __prt_bin32_str(uint32_t val, int shift, int len, char *idnt)
{
    int idx = 0;
    prt("%s", idnt ? idnt : "");
    for(idx = shift + len - 1; idx >= shift; idx--) {
        if (idx % 8 == 0)
            prt("%-2d", idx);
        else
            prt("%2s", " ");

        if (idx % 8 == 0)
            prt(" ");
        if (idx % 16 == 0)
            prt(" ");
    }

    prt("\n%s", idnt ? idnt : "");
    for(idx = shift + len - 1; idx >= shift; idx--) {
        prt("%d ", val & (1 << idx) ? 1 : 0);
        if (idx % 8 == 0)
            prt(" ");
        if (idx % 16 == 0)
            prt(" ");
    }
    prt("\n");

    return 0;
}

static long utils_x710_gpio_ctrl(void)
{
    int      rc  = PASSED;
    int      idx = 0;
    int      bit = 0;
    int      i   = 0;
    uint32_t reg = 0;
    uint32_t fld = 0;
    char    *p   = NULL;
    char     buf[64];

    idx = getdec_answer("Enter Gpio index(0 - 29):", 0, 0, I40E_GPIO_IDX_MAX);

    bit = switzer_manhattan_x710_gpio_ctrl_get(mod, 0, idx, &reg);
    EURET_COND(bit < 0, rc, FAILED, "Failed to read Gpio-%d\n", idx);
    __x710_gpio_ctrl_parse(idx, reg, 1);

    prt("Input 'q' to abort\n");
    prt("Input nothing to use current value\n");
    for(i = 0; gpio_ctrl_flds[i].sname; i++) {
        prt("%-12s(%2xh):", gpio_ctrl_flds[i].sname,
            ((gpio_ctrl_flds[i].mask & reg) >> gpio_ctrl_flds[i].shift));
        fflush(stdout);

        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), stdin);
        for(p = buf; *p && isspace(*p) && *p != '\n'; p++); /* Skip leading space(s) */

        URET_COND(*p == 'q', rc, PASSED, "Abort\n");

        if (*p == 0 || *p == '\n') {
            continue;
        }

        EURET_COND((1 != sscanf(p, "%x", &fld)), rc, FAILED, "Invalid value '%s'\n", p);

        reg &= ~gpio_ctrl_flds[i].mask;
        reg |= (fld << gpio_ctrl_flds[i].shift) & gpio_ctrl_flds[i].mask;
    }
    prt("New ctrl value:0x%08x\n", reg);
    rc = switzer_manhattan_x710_gpio_ctrl_set(mod, 0, idx, reg);

    prt("OK\n");
    rc = PASSED;
_EXIT_POINT:
    return rc;
}

static long utils_x710_gpio_dump(void)
{
    int rc = 0;
    int idx= 0;
    int bit= 0;
    uint32_t reg  = 0;
    uint32_t ctrl = 0;

    bit = switzer_manhattan_x710_gpio_get(mod, 0, idx, &reg);
    EURET_COND(bit < 0, rc, FAILED, "Failed to read Gpio-%d\n", idx);

    prt("All Gpio Status:\n");
    __prt_bin32_str(reg, 0, I40E_GPIO_IDX_MAX, "    ");
    prt("\n");
    prt("All Gpio Control:\n");
    for(idx = 0; idx <= I40E_GPIO_IDX_MAX; idx++) {
        rc = switzer_manhattan_x710_gpio_ctrl_get(mod, 0, idx, &ctrl);
        EURET_COND(rc < 0, rc, FAILED, "Failed to read Gpio-%d Ctrl\n", idx);

        __x710_gpio_ctrl_parse(idx, ctrl, idx == 0);
    }
    prt("OK\n");

    rc = PASSED;
_EXIT_POINT:
    return rc;
}


static long phy_autoneg_remote_ability_get(void)
{
    int rc;
    switzer_lane_t lane;
    unsigned short fec_ability, pause_ability;
    bcm_plp_an_config_t an_config;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_manhattan_ephy_autoneg_remote_ability_get(mod, lane, if_side,
                                                          &fec_ability, &pause_ability, &an_config);
    if (rc) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 autoneg_remote_ability_get error");

        return FAILED;
    }
    prt("fec_ability: %u, pause_ability: %u\n"
        "an_config: master_lane %u, cl72_en %u, tech_ability %u\n",
        fec_ability, pause_ability, an_config.master_lane, an_config.cl72_en, an_config.tech_ability);
    return PASSED;
}

static long switzer_manhattan_nvm_upgrade(void)
{
    struct ngio_intf_t *ngio = mod->ngio;
    const char *path = "X710-NVM-XXXX.bin";
    char cmd[1024], buf[1024];
    struct stat sb;
    int rc;
    char ans;
    int bus = 0;

    prt("Enter firmware path [%s]: ", path);
    if (get_line(buf, sizeof(buf)) > 0)
        path = buf;

    if ((rc = stat(path, &sb)) < 0) {
        log_err("Open NVM firmware path %s failed\n", path);
        return FAILED;
    }

    if (sb.st_size == 0) {
        log_err("NVM firmware path %s size is 0\n", path);
        return FAILED;
    }

    prt("\nThis will upgrade the firmware of X710-%s with\n    %s\n"
        "Continue?(y/N) ",
        is_manhattan_4t() ? "TM4" : "AT2", path);
    ans = getchar();
    if ((ans != 'y') && (ans != 'Y'))
        return PASSED;

    if (0 > (bus = get_ngio_pcie_dev_bus_num(ngio->mod_type, ngio->slot))) {
        log_err("Invalid bus number %d\n", bus);
        return FAILED;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "eeupdate64e /BUS=%d /DEV=%d /FUN=%d /DATA %s", bus, 0, 0, path);
    printf("Run %s ..\n", cmd);
    system(cmd);

    prt("\nPlease power cycle the board to make the new NVM take effect.\n");

    return PASSED;
}

/* Not all 0s, not broadcast addr. */
static inline int _is_mac_valid(uint8_t *addr)
{
    const char zaddr[6] = { 0, };

    return !(addr[0] & 1) && memcmp(addr, zaddr, 6);
}

static long switzer_manhattan_mac_program(void)
{
    char    cmd[1024]  = {[0 ... sizeof(cmd) - 1] = 0};
    char    buf[1024]  = {[0 ... sizeof(buf) - 1] = 0};
    FILE   *fp         = NULL;
    char   *ptr        = NULL;
    char    ans        = 0   ;
    int     port       = 0   ;
    int     bus        = 0   ;
    int     idx        = 0   ;
    int     mac[6]     = {0,};
    uint8_t new_mac[6] = {0,};
    const char mac_desc[]    = "LAN MAC Address is";
    struct ngio_intf_t *ngio = mod->ngio;

    ERET_COND(0 > (bus = get_ngio_pcie_dev_bus_num(ngio->mod_type, ngio->slot)),
              FAILED, "Got invalid pci bus number %d\n", bus);

    prt("Current MAC address(es):\n");
    for(port = 0; port < 4; port++) {
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "eeupdate64e /BUS=%d /DEV=%d /FUN=%d /MAC_DUMP", bus, 0, port);

        ERET_COND(!(fp = popen(cmd, "r")), FAILED,
                  "Get MAC address failed, bus:%x, dev:%x, func:%x", bus, 0, port);

        while ((fgets(buf, sizeof(buf), fp))) {
            ptr = strstr(buf, mac_desc);
            if (ptr) {
                if (6 != sscanf(ptr + sizeof(mac_desc), "%02x%02x%02x%02x%02x%02x",
                               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])) {
                    ERET_COND(1, FAILED, "Failed to current MAC address of port-%d\n", port);
                }
                break;
            }
            memset(buf, 0, sizeof(buf));
        }
        pclose(fp);
        fp = NULL;

        prt("Port#%d MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n",
            port, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    port = getdec_answer("Port(0-3)", 0, 0, 3);
    prt("New MAC address (Format as HH:HH:HH:HH:HH:HH): ");
    memset(buf, 0, sizeof(buf));
    if ((fgets(buf, sizeof(buf), stdin)) != NULL) {
        sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x" ,
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        for(idx = 0; idx < 6; idx++)
            new_mac[idx] = mac[idx] & 0xff;

        prt("Will program port#%d MAC address to %02x:%02x:%02x:%02x:%02x:%02x\n",
            port, new_mac[0], new_mac[1], new_mac[2], new_mac[3], new_mac[4], new_mac[5]);

        ERET_COND(!_is_mac_valid(&new_mac[0]), FAILED,
            "Invalid new MAC address\n"
            "Should not be 0s or broadcast address.\n");

        ans = getc_answer("Contiue ?(y/N):", "yYnN", 'n');
        RET_COND((ans != 'y') && (ans != 'Y'), PASSED, "Abort.\n");

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "eeupdate64e /BUS=%d /DEV=%d /FUN=%d /MAC=%02x%02x%02x%02x%02x%02x",
                 bus, 0, port, new_mac[0], new_mac[1], new_mac[2], new_mac[3], new_mac[4], new_mac[5]);
        prt("Run %s ...\n", cmd);
        system(cmd);
        prt("\nPlease power cycle the board to make the new MAC take effect.\n");
    }
    return PASSED;
}

static long pse_reg_read (void)
{
    uchar reg = 0;
    uint8_t port_num = 0;

    port_num = (uint8_t)gethex_answer("Enter the port number(0x0-0x03) ", 0, 0, 0x3);
    reg = (uint8_t)gethex_answer("Enter the register offset ", 0, 0, 0x63);
    if (switzer_pse_register_read(mod, port_num, reg) < 0) {
        log_warn("Register read failed\n");
        return FAILED;
    }
    return PASSED;
}

static long pse_reg_write (void)
{
    uchar reg, data = 0;
    uint8_t port_num = 0;

    port_num = (uint8_t)gethex_answer("Enter the port number(0x0-0x03) ", 0, 0, 0x3);
    reg = (uint8_t)gethex_answer("Enter the register offset ", 0, 0, 0x63);
    data = (uint8_t)gethex_answer("Enter the data ", 0, 0, 0xFF);
    if (switzer_pse_register_write(mod, port_num, reg, data) < 0) {
        log_warn("Register read failed\n");
        return FAILED;
    }
    return PASSED;
}

static int switzer_upoe_init(void)
{
    uint8_t mode, pwr_mode;
    pwr_allocation_t pwr;

    mode = (uint8_t)gethex_answer("Enter mode(manual 0x01, semi-auto 0x02, auto 0x03)", 0x03, 0, 0x03);
    pwr_mode = (uint8_t)gethex_answer("Enter power(4P_15W 0x01, 4P_30W 0x02, 4P_60W 0x03, 4P_90W 0x04)", 0x04, 0, 0x04);
    switch (pwr_mode) {
        case 0x1: pwr = _4P_15W;
            break;
        case 0x2: pwr = _4P_30W;
            break;
        case 0x3: pwr = _4P_60W;
            break;
        case 0x4: pwr = _4P_90W;
            break;
        default: pwr = _4P_90W;
            break;
    }
    switzer_upoe_pse_init(mod, mode, pwr);
    return 0;
}

static int switzer_safe_mode_load(void)
{
    prt("read register 0x41, value equals to 0x41 means in safe mode\n\r");
    safe_mode_load_code (mod);
    return 0;
}

static int switzer_display_upoe_parameters(void)
{
    uint8_t firmwarerev, port_num = 0;
    classStatus_t  classStatus;
    detStatus_t    detectStatus;
    unsigned long voltage, current, temperature, detectresistance;
    static struct {
        char *str;
        int  cal_value;
    } detects[] = {
        [UNKNOWN_DETECTION] = {"UNKNOWN", 0},
        [SHORT_CIRCUIT_DETECTION] = {"SHORT-CIRCUIT", 0},
        [TOO_LOW_DETECTION] = {"TOO_LOW_DETECTION", 1},
        [VALID_DETECTION] = {"VALID_DETECTION", 1},
        [TOO_HIGH_DETECTION] = {"TOO_HIGH_DETECTION", 1},
        [OPEN_CIRCUIT_DETECTION] = {"OPEN_CIRCUIT_DETECTION", 0},
        [MOSFET_FAULT_DETECTION] = {"MOSFET_FAULT_DETECTION", 0},
    };
    static char *class_strs[] = {
        [CLASS_UNKNOWN] = "Unknown",
        [CLASS_1] = "Class 1",
        [CLASS_2] = "Class 2",
        [CLASS_3] = "Class 3",
        [CLASS_4] = "Class 4",
        [CLASS_0] = "Class 0",
        [CLASS_OVERCURRENT] = "Overcurrent",
        [CLASS_5] = "Class 5, 4 Pair Single Signature",
        [CLASS_6] = "Class 6, 4 Pair Single Signature",
        [CLASS_7] = "Class 7, 4 Pair Single Signature",
        [CLASS_8] = "Class 8, 4 Pair Single Signature",
        [CLASS_4_PLUS] = "Class 4+, Type 1 Limited",
        [CLASS_5_DUAL] = "Class 5, 4 Pair Dual Signature",
        [CLASS_MISMATCH] = "Class Mismatch",
    };

    /* get intput voltage */
    switzer_upoe_get_input_voltage (mod, port_num, &voltage);
    voltage *= 3662;
    prt("   ***Display environment variables***\n");
    prt("      Input voltage : %d.%d mV\n", voltage/1000, voltage%1000);
    /*get temperature*/
    switzer_upoe_get_temperature(mod, port_num, &temperature);
    temperature = temperature * 652 - 20000;
    prt("      Temperature : %d.%d degree C\n", temperature/1000, temperature%1000);
    /* get firmware version */
    switzer_upoe_get_firmware_version(mod, &firmwarerev);
    prt("      Firmware revision : %x\n", firmwarerev);
    for (port_num = 0; port_num < UPOE_PORTS; port_num++) {
        if (switzer_upoe_get_port_pwr_good_status(mod, port_num) & PD_PORT_ON) {
            switzer_upoe_get_port_volt_current_measurements(mod, port_num, &voltage, &current);
            /* calculate values, in mv and mA */
            current *= 89500 ;
            voltage *= 3662;
            prt("\nPort %d ON, Voltage: %d.%d v,  Current: %d.%d mA \n", port_num,
                                voltage/1000000, voltage%1000000, current/1000000, current%1000000);
        } else {
            prt("\nPort %d, OFF\n", port_num);
        }
        switzer_upoe_get_classification_detection_status(mod, port_num, &classStatus, &detectStatus);
        if(detectStatus == MOSFET_FAULT_DETECTION) continue;
        prt("  Detection Status: %s  ", detects[detectStatus]);
        if(detects[detectStatus].cal_value) {
            switzer_upoe_get_port_detect_resistance(mod, port_num, &detectresistance);
            detectresistance *= 1953125 ;
            prt(" Detection Resistance is %d.%d OHM\n", detectresistance/10000, detectresistance%10000);
        }

        if(classStatus <= CLASS_MISMATCH)
            prt("  Classification Status: %s\n", class_strs[classStatus]);
    }
    return PASSED;
}

/* SEMI-AUTO GUIDANCE - according to tps23881 spec
 * 1. Perform load detection
 * 2. Performs classification for type-1 through type-4 loads
 * 3. Enables power on with protective foldback current limiting, and Port Power policing Pcut valus
 * 4. Shuts down in the event of fault loads and shorts
 * 5. Performs Maintain Power Signature function to insure removal of power if loads is disconnected
 * 6. Undervoltage lock out occurs if VPWR falss velow Vpuv_f (typical 26.5V)
*/

static int switzer_upoe_semi_auto_mode_test(void)
{
    uint8_t port_num, intr, test_value, test_pcut, test_ilim, test_4pcut;
    uint8_t pwr_enable_event, pwr_good_status_event, disconnect_event, pwr_cut_event;
    uint8_t detect_event, cls_event, inrush_event, ilim_event, supply_event;
    classStatus_t  classStatus;
    detStatus_t    detectStatus;
    //uint8_t value = 0;

    //switzer_upoe_get_general_mask(mod, 0, &value);
    //prt("the general mask is 0x%x\n", value);
    //in our case we only apply i2c_0
    switzer_upoe_get_interrupt_mask(mod, 0, &intr);
    switzer_upoe_get_interrupt_events(mod, 0, &pwr_enable_event, &pwr_good_status_event,
            &disconnect_event, &pwr_cut_event, &detect_event, &cls_event,
            &inrush_event, &ilim_event, &supply_event);

    switzer_upoe_pse_init (mod, SEMI_AUTO_MODE, _4P_90W);
    mdelay(2000);

#if 0
    prt("Print event registers: \n\r");
    prt(" power good port events: 0x%x\n\r", pwr_good_status_event);
    prt(" disconnect event: 0x%x\n\r", disconnect_event);
    prt(" pwr_cut_event: 0x%x\n\r", pwr_cut_event);
    prt(" Inrush event: 0x%x\n\r", inrush_event);
    prt(" Ilim event: 0x%x\n\r", ilim_event);
    prt(" supply event: 0x%x\n\r", supply_event);
#endif
    //switzer_upoe_get_general_mask(mod, 0, &value);
    //prt("!!!after sys init the general mask is 0x%x\n", value);

    //prt(" supply event: 0x%x\n", supply_event);
    if (intr & SUPPLY_EVENT_FAULT) {
        test_value = supply_event;
        if (test_value & SRAM_FAULT_OCCUR) {
            prt("SRAM fault occured \n");
        } else if (test_value & OSS_EVENT_OCCUR) {
            prt("An OSS event occured \n");
        } else if (test_value & VPWR_VOLTAGE_OCCUR) {
            prt("A VPWR undervoltage has occured \n");
        } else if (test_value & VDD_UNDERVOLTAGE_WARNING) {
            prt("VDD has fallen under the UVLO warning threshold \n");
        } else if (test_value & VDD_UVLO) {
            prt("VDD UVLO occur \n");
        } else if (test_value & THERMAL_SHUTDOWN) {
            prt("Thermal shut down has occured \n");
        }
    }
    if (intr & INRUSH_FAULT) {
        test_value = inrush_event;
        for (port_num = 0; port_num < UPOE_PORTS; port_num ++) {
            if (test_value & MASKED_4P_VALUE) {
                prt("Inrush falt has veen detected\n");
            }
            test_value >> SHIFT_4P_PORT;
        }
    }
    if (intr & IFAULT_EVENT) {
        test_pcut = pwr_cut_event;
        test_ilim = ilim_event;
        test_4pcut = (supply_event & UPOE_DATA_MASK) >> 2;

        if (test_4pcut & 0x01) {
            prt("4-pair summed pcut fault occured on channlels 1 and 2\n");
        }
        if (test_4pcut & 0x02) {
            prt("4-pair summed pcut fault occured on channlels 3 and 4\n");
        }

        for (port_num = 0; port_num < UPOE_PORTS; port_num ++) {
            if (test_pcut & MASKED_4P_VALUE) {
                prt("4P Pcut fault channel\n");
            }
            if (test_ilim & MASKED_4P_VALUE) {
                prt("ilim fault port\n");
            }
        }
        test_pcut >>= SHIFT_4P_PORT;
        test_ilim >>= SHIFT_4P_PORT;
    }
    if (intr & CLASSIFICATION_CYCYLE) {
        // classification valid
        test_value = cls_event;
        for (port_num = 0; port_num < UPOE_PORTS; port_num++) {
            if (test_value & MASKED_4P_VALUE) {
                switzer_upoe_get_classification_detection_status(mod, port_num, &classStatus, &detectStatus);
                if((classStatus != CLASS_OVERCURRENT) &&
                        (classStatus != CLASS_UNKNOWN) &&
                        (classStatus != CLASS_MISMATCH))
                {
                    if (switzer_upoe_get_port_pwr_enable_status(mod, port_num) == PD_PORT_OFF){
                        switzer_upoe_set_port_power (mod, port_num, PD_PORT_ON);
                        switzer_mdelay(100);
                    }
                }
            }
            test_value >>= SHIFT_4P_PORT;
        }
    }

    if (intr & DISCONNECTION_EVENT) {
        test_value = disconnect_event;
        for (port_num = 0; port_num < UPOE_PORTS; port_num ++) {
            if (test_value & MASKED_4P_VALUE) {
                if (switzer_upoe_get_port_pwr_enable_status(mod, port_num) && PD_PORT_ON){
                    switzer_upoe_set_port_power (mod, port_num, PD_PORT_OFF);
                    switzer_mdelay(100);
                }
            }
            test_value >>= SHIFT_4P_PORT;
        }
    }
    switzer_display_upoe_parameters();
    return PASSED;
}


static int switzer_upoe_test (void)
{
    uint8_t port_num, flip = 0;
    uint8_t ports, start_port = 0;
    classStatus_t  classStatus;
    detStatus_t    detectStatus;
    static char *detect_strs[] = {
        [UNKNOWN_DETECTION] = "UNKNOWN",
        [SHORT_CIRCUIT_DETECTION] = "SHORT-CIRCUIT",
        [TOO_LOW_DETECTION] = "TOO_LOW_DETECTION",
        [VALID_DETECTION] = "VALID_DETECTION",
        [TOO_HIGH_DETECTION] = "TOO_HIGH_DETECTION",
        [OPEN_CIRCUIT_DETECTION] = "OPEN_CIRCUIT_DETECTION",
        [MOSFET_FAULT_DETECTION] = "MOSFET_FAULT_DETECTION",
    };
    static char *class_strs[] = {
        [CLASS_UNKNOWN] = "Unknown",
        [CLASS_1] = "Class 1",
        [CLASS_2] = "Class 2",
        [CLASS_3] = "Class 3",
        [CLASS_4] = "Class 4",
        [CLASS_0] = "Class 0",
        [CLASS_OVERCURRENT] = "Overcurrent",
        [CLASS_5] = "Class 5, 4 Pair Single Signature",
        [CLASS_6] = "Class 6, 4 Pair Single Signature",
        [CLASS_7] = "Class 7, 4 Pair Single Signature",
        [CLASS_8] = "Class 8, 4 Pair Single Signature",
        [CLASS_4_PLUS] = "Class 4+, Type 1 Limited",
        [CLASS_5_DUAL] = "Class 5, 4 Pair Dual Signature",
        [CLASS_MISMATCH] = "Class Mismatch",
    };

    ports = (uint8_t)gethex_answer("Enter number of ports(1,2)", 2, 0, 0x2);
    start_port = (uint8_t)gethex_answer("Enter start number of ports(0,1)", 0, 0, 0x1);
    switzer_upoe_pse_init (mod, AUTO_MODE, _4P_90W);
    mdelay(2000);
    for (port_num = start_port; port_num < ports; port_num++) {
        if (switzer_upoe_get_port_pwr_good_status(mod, port_num) && PD_PORT_ON) {
            switzer_upoe_get_classification_detection_status(mod, port_num, &classStatus, &detectStatus);
            prt("\n Port %x, ON\n", port_num);
            prt("Detection Status: %s\n", detect_strs[detectStatus]);
            prt("Classification Status: %s\n", class_strs[classStatus]);
        } else {
            flip = 1;
            prt("\n Port %x, OFF\n", port_num);
            prt("Either detection or classification failed ");
        }
    }

    if(flip == 1) {
        cterr('f', 0, "UPOE test failed");
        return FAILED;
    }
    return PASSED;
}

static long ds4424_reg_read(void)
{
    uint8_t cmd;
    cterr_add_component("DS4424 on Switzer",
                        "I2C Bus to DS4424");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C Bus to DS4424");

    cmd = gethex_answer("DS4424 number(i2c slave: 0x60 - 0, 0x20 -1)", 0, 0, 1);
    return switzer_utils_dash_i2c_reg_read(mod->pm[cmd].i2c);
}

static long ds4424_reg_write(void)
{
    uint8_t cmd;
    cterr_add_component("DS4424 on Switzer",
                        "I2C Bus to DS4424");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C Bus to DS4424");

    cmd = gethex_answer("DS4424 number(i2c slave: 0x60 - 0, 0x20 -1)", 0, 0, 1);
    return switzer_utils_dash_i2c_reg_write(mod->pm[cmd].i2c);
}

static long switzer_manhattan_i2c_mux_reg_read(void)
{
    cterr_add_component("DS4424 on Switzer",
                        "I2C Bus to PCA9554");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C Bus to PCA9554");

    return switzer_utils_dash_i2c_reg_read(mod->i2c_mux.i2c);
}

static long switzer_manhattan_i2c_mux_reg_write(void)
{
    cterr_add_component("PCA9545 on Switzer",
                        "I2C Bus to PCA9554");
    cterr_add_debug("PCA9545 on Switzer",
                    "Check the I2C Bus to PCA9554");

    return switzer_utils_dash_i2c_reg_write(mod->i2c_mux.i2c);
}

static long switzer_manhattan_lm75_reg_read(void)
{
    cterr_add_component("DS4424 on Switzer",
                        "PCA9554 I2C Mux status");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the PCA9554 I2C Mux status");

    return switzer_utils_dash_i2c_reg_read(mod->lm75.i2c);
}

static long switzer_manhattan_lm75_reg_write(void)
{
    cterr_add_component("PCA9545 on Switzer",
                        "PCA9554 I2C Mux status");
    cterr_add_debug("PCA9545 on Switzer",
                    "Check the PCA9554 I2C Mux status");

    return switzer_utils_dash_i2c_reg_write(mod->lm75.i2c);
}

static long switzer_manhattan_poe_reg_read(void)
{
    cterr_add_component("DS4424 on Switzer",
                        "PCA9554 I2C Mux status");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the PCA9554 I2C Mux status");

    return switzer_utils_dash_i2c_reg_read(mod->poe[0].i2c);
}

static long switzer_manhattan_poe_reg_write(void)
{
    cterr_add_component("PCA9545 on Switzer",
                        "PCA9554 I2C Mux status");
    cterr_add_debug("PCA9545 on Switzer",
                    "Check the PCA9554 I2C Mux status");

    return switzer_utils_dash_i2c_reg_write(mod->poe[0].i2c);
}

static submenu_xtable_t upoe_submenu_table[] = {
    {"Read  Pse chip Register"           , (PFT)pse_reg_read                    , 0         , 0, NULL, 0, NULL, 0},
    {"Write Pse chip Register"           , (PFT)pse_reg_write                   , 0         , 0, NULL, 0, NULL, 0},
    {"PSE Init"                          , (PFT)switzer_upoe_init               , AUTO_MODE , 0, NULL, 0, NULL, 0},
    {"PSE safe mode load"                , (PFT)switzer_safe_mode_load          , 0         , 0, NULL, 0, NULL, 0},
    {"Display port environment variables", (PFT)switzer_display_upoe_parameters , 0         , 0, NULL, 0, NULL, 0},
    {"Display Upoe Port mapping"         , (PFT)switzer_display_port_mapping    , 0         , 0, NULL, 0, NULL, 0},
    {"Upoe Test"                         , (PFT)switzer_upoe_test               , UPOE_PORTS, 0, NULL, 0, NULL, 0},
    {"Semi Auto Mode Test"               , (PFT)switzer_upoe_semi_auto_mode_test, 0         , 0, NULL, 0, NULL, 0},
};

#define UPOE_SUBMENU_TABLE_SZ (sizeof(upoe_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t upoe_submenu_primary_items[UPOE_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t upoe_submenu_secondary_items[UPOE_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];


static char upoe_submenu_title[] = "Switzer Manhattan UPOE Utilities Menu";

static menuinfo_t upoe_submenu = {
    upoe_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    upoe_submenu_primary_items,
};

static menuinfo_t *upoe_submenup = &upoe_submenu;

static long upoe_test(int show_menu)
{
    build_primary_submenu(upoe_submenu_table, UPOE_SUBMENU_TABLE_SZ,
                          upoe_submenu_title, &upoe_submenup);
    build_secondary_submenu(upoe_submenu_table, UPOE_SUBMENU_TABLE_SZ,
                            upoe_submenu_secondary_items);
    if (show_menu)
        menu(upoe_submenup, upoe_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(upoe_submenup);
    return PASSED;
}

static long utils_sfp_reg_read(void)
{
    int rc;
    uint8_t addr, cmd;
    size_t count;
    char buf[BUF_SIZE];
    switzer_lane_t lane;

    if (is_manhattan_4t()) {
        lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
        cmd = gethex_answer("SFP offset", 0, 0, 0xff);
        count = gethex_answer("SFP read size", 128, 0, sizeof(buf));
        addr = gethex_answer("SFP address", SWITZER_MANHATTAN_I2C_ADDR_SFP, 0, 0xff);

        rc = switzer_manhattan_sfp_read(mod, lane, addr, cmd, buf, count);
        if (rc < 0) {
            cterr_add_component("SFP",
                                "I2C controller within the BCM82757");
            cterr_add_debug("Check SFP",
                            "Check I2C controller within the BCM82757");
            cterr('f', 0, "SFP read error");
            return FAILED;
        }
        switzer_hex_dump(buf, count, addr);
        return PASSED;
    }

    if (is_manhattan_2t()) {
        bcm54194_sfp_reg_dump(0);
    }
    return PASSED;
}

static long utils_sfp_reg_write(void)
{
    int rc;
    uint8_t addr, cmd;
    size_t count;
    char hex[BUF_SIZE], buf[BUF_SIZE/2];
    switzer_lane_t lane;

    if (is_manhattan_4t()) {
        lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
        cmd = gethex_answer("SFP offset", 0, 0, 0xff);
        hex[0] = '\0';
        prt("SFP write data hexadecimal bytes:  ");
        get_line(hex, sizeof(hex));

        count = strlen(hex) / 2;
        if (switzer_hex_to_bin(hex, buf, count) < 0) {
            cterr('f', 0, "Hexadecimal string format error: %s\n", hex);
            return FAILED;
        }

        addr = gethex_answer("SFP address", SWITZER_MANHATTAN_I2C_ADDR_SFP, 0, 0xff);

        rc = switzer_manhattan_sfp_write(mod, lane, addr, cmd, buf, count);
        if (rc < 0) {
            cterr_add_component("SFP",
                                "I2C controller within the BCM82757");
            cterr_add_debug("Check SFP",
                            "Check I2C controller within the BCM82757");
            cterr('f', 0, "SFP write error");
            return FAILED;
        }
    }

    if (is_manhattan_2m()) {
        bcm54194_sfp_reg_rdwr(1);
    }
    return PASSED;
}

static long switzer_manhattan_pca_read()
{
    uint8_t cmd;
    n2g_i2c_if_t *pca = switzer_ngio_pca();

    cmd = gethex_answer("PCA Nuber(PCA9557-1@1C: 0, PCA9557-2@1D: 1)", 0, 0, 1);
    pca->i2c_dev = pca_i2c_addr[cmd];

    return switzer_pca_reg_read();
}

static long switzer_manhattan_pca_reg_write(n2g_i2c_if_t *pca)
{
    uint32_t offset;
    uchar data = 0;

    offset = gethex_answer("Reg offset to write: ", 0, 0, 0x3);
    data   = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr_add_component("PCA IO expander on Switzer",
                            "I2C controller at router");
        cterr_add_debug("Check PCA IO expander on Switzer",
                        "Check the I2C controller at the router");
        cterr('f',0,"Unable to write PCA IO expander register @ %#x\n", offset);
        return FAILED;
    }
    return PASSED;
}

static long switzer_manhattan_pca_write()
{

    uint8_t cmd;
    n2g_i2c_if_t *pca = switzer_ngio_pca();

    cmd = gethex_answer("PCA Nuber(PCA9557-1@1C: 0, PCA9557-2@1D: 1)", 0, 0, 1);
    pca->i2c_dev = pca_i2c_addr[cmd];

    return switzer_manhattan_pca_reg_write(pca);
}

/* Utils submenu items */
static long __show_sfp_reg_rdwr(void)
{
    return is_manhattan_4t() || is_manhattan_2t();
}
static long __show_poe_util(void)
{
    return is_manhattan_2m() || is_manhattan_4t() || is_manhattan_1m();
}
static submenu_xtable_t utils_submenu_table[] = {
    {"X710 NVM Upgrade", switzer_manhattan_nvm_upgrade, 0,
     0, NULL, 0, NULL, 0},
    {"X710 MAC Program", switzer_manhattan_mac_program, 0,
     0, NULL, 0, NULL, 0},
    {"X710 Register Read",  utils_x710_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"X710 Register Write", utils_x710_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"X710 Gpio Set"     , utils_x710_gpio_set , 0,
     0, NULL, 0, NULL, 0},
    {"X710 Gpio Ctrl"    , utils_x710_gpio_ctrl, 0,
     0, NULL, 0, NULL, 0},
    {"X710 Gpio Dump"    , utils_x710_gpio_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Internal PHY Regster Read",  utils_intnl_phy_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"Internal PHY Regster Write", utils_intnl_phy_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Register Read", utils_phy_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Register Write", utils_phy_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"SFP Register Read", utils_sfp_reg_read, 0,
     0, __show_sfp_reg_rdwr, 0, NULL, 0},
    {"SFP Register Write", utils_sfp_reg_write, 0,
     0, __show_sfp_reg_rdwr, 0, NULL, 0},
    {"LTC4215 Register Read", switzer_ltc4215_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"LTC4215 Register Write", switzer_ltc4215_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCA9557 Register Read", switzer_manhattan_pca_read, 0,
     0, NULL, 0, NULL, 0},
    {"PCA9557 Register Write", switzer_manhattan_pca_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCA9545(I2C MUX) Register Read", switzer_manhattan_i2c_mux_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"PCA9545(I2C MUX) Register Write", switzer_manhattan_i2c_mux_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"DS4424 Register Read", ds4424_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"DS4424 Register Write", ds4424_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"LM75 Register Read", switzer_manhattan_lm75_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"LM75 Register Write", switzer_manhattan_lm75_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"POE Register Read", switzer_manhattan_poe_reg_read, 0,
     0, __show_poe_util, 0, NULL, 0},
    {"POE Register Write", switzer_manhattan_poe_reg_write, 0,
     0, __show_poe_util, 0, NULL, 0},
    {"Debug Flags Toggling", switzer_manhattan_debug_flag, 0,
     0, NULL, 0, NULL, 0},
};

#define UTILS_SUBMENU_TABLE_SZ (sizeof(utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t utils_submenu_primary_items[UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t utils_submenu_secondary_items[UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char utils_submenu_title[] = "Switzer Manhattan Utilities Menu";

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

/* Power submenu items */

static int switzer_manhattan_device_init(struct switzer_manhattan *mod);
static void switzer_manhattan_device_exit(struct switzer_manhattan *mod);

static long power_on(void)
{
    prt("\nPower On the module.\n");

    if (switzer_ltc4215_power_on()) {
        log_warn("ltc4215 power on failed\n");
        return FAILED;
    }

    if (switzer_manhattan_platform_init(mod) < 0)
        return FAILED;

    if (switzer_manhattan_device_init(mod) < 0)
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
    switzer_manhattan_device_exit(mod);
    switzer_manhattan_platform_exit(mod);
    switzer_ltc4215_power_off();
    return PASSED;
}

#define DS4424_OUT0_ADDR    0xF8
#define DS4424_OUT1_ADDR    0xF9
#define DS4424_OUT2_ADDR    0xFA
#define DS4424_OUT3_ADDR    0xFB

/* on chip 1 */
#define VM_OUT_3_30_ADDR    DS4424_OUT3_ADDR
#define VM_OUT_1_88_ADDR    DS4424_OUT2_ADDR
#define VM_OUT_1_00_ADDR    DS4424_OUT1_ADDR
#define VM_OUT_0_92_ADDR    DS4424_OUT0_ADDR

/* on chip 2 */
#define VM_OUT_1_00A_ADDR   DS4424_OUT3_ADDR
#define VM_OUT_1_00B_ADDR   DS4424_OUT2_ADDR

#define VM_DATA_STEP      1
#define VM_DATA_MAX       30
#define VM_DATA_MASK      0x7F

#define VM_MAX_PERCENT    3

#define VM_SOURCE_FLAG 0x00
#define VM_SINK_FLAG (0x01 << 7)

#define VM_CALCULATE_COEFFICIENT 10

struct switzer_vpara {
    uint8_t devnum;
    uint8_t addr;
    char   *suffix;
};

static const struct switzer_vpara switzer_vpara_table[] = {{0, VM_OUT_0_92_ADDR,  "  "},
                                                           {0, VM_OUT_1_00_ADDR,  "  "},
                                                           {0, VM_OUT_1_88_ADDR,  "  "},
                                                           {0, VM_OUT_3_30_ADDR,  "  "},
                                                           {1, VM_OUT_1_00A_ADDR, "_A"},
                                                           {1, VM_OUT_1_00B_ADDR, "_B"}};

#define SWITZER_VPARA_TABLE_SZ (sizeof(switzer_vpara_table) / sizeof(struct switzer_vpara))

static double DS4424_data_to_voltage(double basevoltage, uint8_t vmdata)
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

static double DS4424_get_base_voltage(uint8_t devnum, uint8_t addr)
{
    uint32_t basevoltage;

    if (devnum == 0) {
        switch (addr) {
        case VM_OUT_0_92_ADDR:
            basevoltage = 9.2;
            break;
        case VM_OUT_1_00_ADDR:
            basevoltage = 10.0;
            break;
        case VM_OUT_1_88_ADDR:
            basevoltage = 18.8;
            break;
        case VM_OUT_3_30_ADDR:
            basevoltage = 33.0;
            break;
        default:
            basevoltage = 0.0;
            break;
        }
    } else {
        switch (addr) {
        case VM_OUT_1_00A_ADDR:
            basevoltage = 10.0;
            break;
        case VM_OUT_1_00B_ADDR:
            basevoltage = 10.0;
            break;
        default:
            basevoltage = 0.0;
            break;
        }
    }

    return basevoltage;
}

static long display_voltage_marging(struct switzer_manhattan *mod, uint8_t devnum, uint8_t addr, char *tag)
{
    uint8_t predata;
    uint32_t basevoltage;
    double   basevoltage_f;
    int per;
    struct switzer_dash_i2c_slave *slave;
    slave = mod->pm[devnum].i2c;
    basevoltage   = DS4424_get_base_voltage(devnum, addr);
    basevoltage_f = DS4424_data_to_voltage(basevoltage, 0);

    if (switzer_dash_i2c_slave_read(slave, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C Mux PCA9554");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C Mux PCA9554");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }

    if (predata == 0) {
        prt("%0.1fV%s: not margined\n", basevoltage_f, tag ? tag :"");
    } else {
        if (DS4424_data_to_margin_percent(predata, &per) == FAILED) {
            prt("%0.1fV%s: margined out of range\n", basevoltage_f, tag ? tag : "");
            return FAILED;
        } else {
            prt("%0.1fV%s: margined %+d%%\n", basevoltage_f, tag ? tag : "", per);
        }
    }

    return PASSED;
}

static long display_all_voltage_margin(void)
{
    int i;
    const struct switzer_vpara *vpara_t = &switzer_vpara_table[0];

    for (i = 0; i < SWITZER_VPARA_TABLE_SZ; i++) {
        display_voltage_marging(mod, vpara_t->devnum, vpara_t->addr, vpara_t->suffix);
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
                                    "I2C Mux PCA9554");
                cterr_add_debug("DS4424 on Switzer",
                                "Check the I2C Mux PCA9554");
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
    if (fabs(basevoltage - 0) < 1E-6) {
        cterr('f', 0, "wrong addr param %#x", addr);
        return FAILED;
    }

    if (switzer_dash_i2c_slave_read(slave, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C Mux PCA9554");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C Mux PCA9554");
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

static long power_auto_set_vmarg(struct switzer_manhattan *mod, uint8_t devnum,
                                 uint8_t addr, switzer_vmarg_t vmarg)
{
    uint32_t per;
    uint8_t ssflag;
    struct switzer_dash_i2c_slave *slave;

    switch (vmarg) {
    case SWITZER_VMARG_NORMAL:
        per    = 0;
        ssflag = VM_SOURCE_FLAG;
        break;
    case SWITZER_VMARG_LOW:
        per    = VM_MAX_PERCENT;
        ssflag = VM_SINK_FLAG;
        break;
    case SWITZER_VMARG_HIGH:
        per    = VM_MAX_PERCENT;
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

    anr = getdec_answer("Choose a current source\n"
                        "0 - 0.92V\n"
                        "1 - 1.00V\n"
                        "2 - 1.00V_A\n"
                        "3 - 1.00V_B\n"
                        "4 - 1.88V\n"
                        "5 - 3.30V", 0, 0, 4);
    switch (anr) {
    case 0:
        addr   = VM_OUT_0_92_ADDR;
        devnum = 0;
        break;
    case 1:
        addr   = VM_OUT_1_00_ADDR;
        devnum = 0;
        break;
    case 2:
        addr   = VM_OUT_1_00A_ADDR;
        devnum = 1;
        break;
    case 3:
        addr   = VM_OUT_1_00B_ADDR;
        devnum = 1;
        break;
    case 4:
        addr   = VM_OUT_1_88_ADDR;
        devnum = 0;
        break;
    case 5:
        addr   = VM_OUT_3_30_ADDR;
        devnum = 0;
        break;
    default:
        return FAILED;
    }

    slave = mod->pm[devnum].i2c;
    basevoltage = DS4424_get_base_voltage(devnum, addr);
    if (switzer_dash_i2c_slave_read(slave, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C Mux PCA9554");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C Mux PCA9554");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }
    prt("\nCurrent voltage is %0.3fV\n", DS4424_data_to_voltage(basevoltage, predata));

    anr = getdec_answer("Source or Sink? (0-sink, 1-source)", 0, 0, 1);
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


#define _POWER_SET_VMARG_FUNC(VOLT)                                  \
static long power_set_##VOLT##_vmarg(switzer_vmarg_t vmarg)          \
{                                                                    \
    return power_auto_set_vmarg(mod, 0, VM_OUT_##VOLT##_ADDR, vmarg);\
}
_POWER_SET_VMARG_FUNC(0_92)
_POWER_SET_VMARG_FUNC(1_00)
_POWER_SET_VMARG_FUNC(1_00A)
_POWER_SET_VMARG_FUNC(1_00B)
_POWER_SET_VMARG_FUNC(1_88)
_POWER_SET_VMARG_FUNC(3_30)

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

#define _POWER_VOLT_MARGIN_ITEM(VOLT, SVOLT)                                                  \
    {"Set " SVOLT " to Normal"     , (PFT)power_set_##VOLT##_vmarg, SWITZER_VMARG_NORMAL, 0,  \
     NULL                          , 0                            , NULL                , 0}, \
                                                                                              \
    {"Set " SVOLT " to Margin High", (PFT)power_set_##VOLT##_vmarg, SWITZER_VMARG_HIGH  , 0,  \
     NULL                          , 0                            , NULL                , 0}, \
                                                                                              \
    {"Set " SVOLT " to Margin Low" , (PFT)power_set_##VOLT##_vmarg, SWITZER_VMARG_LOW   , 0,  \
     NULL                          , 0                            , NULL                , 0}

static submenu_xtable_t power_submenu_table[] = {
    {"Power Status", switzer_ltc4215_power_info, 0, 0, NULL, 0, NULL, 0},
    {"Power Off"   , power_off                 , 0, 0, NULL, 0, NULL, 0},
    {"Power On"    , power_on                  , 0, 0, NULL, 0, NULL, 0},

    _POWER_VOLT_MARGIN_ITEM(0_92 , "0.92V   "),
    _POWER_VOLT_MARGIN_ITEM(1_00 , "1.00V   "),
    _POWER_VOLT_MARGIN_ITEM(1_00A, "1.00V_A "),
    _POWER_VOLT_MARGIN_ITEM(1_00B, "1.00V_B "),
    _POWER_VOLT_MARGIN_ITEM(1_88 , "1.88V   "),
    _POWER_VOLT_MARGIN_ITEM(3_30 , "3.30V   "),
    _POWER_VOLT_MARGIN_ITEM(all  , "0.92V/1.00V/1.00V_A/1.00V_B/1.88V/3.30V"),

    {"Display Voltage Margins", (PFT)display_all_voltage_margin, 0, 0, NULL, 0, NULL, 0},
    {"User Set"               , (PFT)power_user_set_vmarg      , 0, 0, NULL, 0, NULL, 0},
};

#define POWER_SUBMENU_TABLE_SZ (sizeof(power_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t power_submenu_primary_items[POWER_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t power_submenu_secondary_items[POWER_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char power_submenu_title[] = "Switzer Manhattan Power Utilities Menu";

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

static long power_led_util(void)
{
    int led_color;
    led_color = getdec_answer("led color:\n"
            "0 : led off\n"
            "1 : led amber\n"
            "2 : led green\n"
            "3 : led amber only\n"
            "4 : led green only\n"
            "5 : led amber off\n"
            "6 : led green off", 0, 0, 6);
    if (switzer_power_led_util(led_color)) {
        cterr('f', 0, "power led util fialed");
        return FAILED;
    }
    return PASSED;
}

static long sfp_led_util(void)
{
    int port     = 0;
    int type     = 0; /* Led for 'enable' or 'link' */
    int on       = 0;
    int color    = 0;
    int gpio     = 0;
    int gpio_yel = 0;
    int gpio_grn = 0;
    int vyel     = 0;
    int vgrn     = 0;

    port = getdec_answer("sfp port(2/3):", 2, 2, 3);
    type = getdec_answer("led type :\n"
            "0 : led for enable status\n"
            "1 : led for link   status\n", 0, 0, 1);
    if (type == 0) {
        color= getdec_answer("led color:\n"
                "0 : led off\n"
                "1 : led amber only\n"
                "2 : led green only\n"
                "3 : led amber & green\n", 0, 0, 3);

        switch(color) {
        /* Low active; -1 means don't change it */
        case 0 : vyel =  1; vgrn =  1; break;
        case 1 : vyel =  0; vgrn =  1; break;
        case 2 : vyel =  1; vgrn =  0; break;
        case 3 : vyel =  0; vgrn =  0; break;
        default: ERET_COND(1, FAILED, "Something must be error.\n");
        }

        gpio_yel = (port == 2 ? SWITZER_MANHATTAN_LED_PORT2_ENB_YEL : SWITZER_MANHATTAN_LED_PORT3_ENB_YEL);
        gpio_grn = (port == 2 ? SWITZER_MANHATTAN_LED_PORT2_ENB_GRN : SWITZER_MANHATTAN_LED_PORT3_ENB_GRN);

        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio_yel, vyel),
                  FAILED, "Failed to set amber led(Gpio-%d) to %d\n", gpio_yel, vyel);

        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio_grn, vgrn),
                  FAILED, "Failed to set green led(Gpio-%d) to %d\n", gpio_grn, vgrn);
    } else {
        on   = getdec_answer("off(0)/on(1):", 0, 0, 1);
        gpio = port == 2 ? SWITZER_MANHATTAN_LED_PORT2_LNK_GRN : SWITZER_MANHATTAN_LED_PORT3_LNK_GRN;
        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio, !on), /*active low ?*/
                  FAILED, "Failed to set green led(Gpio-%d) to %d\n", gpio, on);
    }
    prt("OK\n");
    return PASSED;
}

static int manhanttan_sfp_link_led_util(int led)
{
    int gpio_grn = 0;
    int gpio_yel = 0;
    int choices  = 0;
    int i        = 0;

    struct pwr_led_color {
        int   val ;
        char *desc;
        int  green_on;     // 0:off , 1:on , -1:no change
        int  amber_on;
    } colors [] = {
        { 0, "OFF"        ,  0,  0},
        { 1, "AMBER"      , -1,  1},
        { 2, "GREEN"      ,  1, -1},
        { 3, "AMBER ONLY" ,  0,  1},
        { 4, "GREEN ONLY" ,  1,  0},
        { 5, "AMBER OFF"  , -1,  0},
        { 6, "GREEN OFF"  ,  0, -1},
        {-1, NULL         , -1, -1},
    };

    for (; colors[i].desc; i++){
        printf("  %d: %s\n", colors[i].val, colors[i].desc); 
    }
    choices = getdec_answer("> ", 0, 0, i - 1);
    gpio_grn = (led == 2 ? SWITZER_MANHATTAN_LED_PORT2_ENB_GRN : SWITZER_MANHATTAN_LED_PORT3_ENB_GRN);
    gpio_yel = (led == 2 ? SWITZER_MANHATTAN_LED_PORT2_ENB_YEL : SWITZER_MANHATTAN_LED_PORT3_ENB_YEL);

    if(colors[choices].green_on != -1)
        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio_grn, colors[choices].green_on ? 0 : 1),
                      FAILED, "Failed to set green led");

    if(colors[choices].amber_on != -1)
        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio_yel, colors[choices].amber_on ? 0 : 1),
                      FAILED, "Failed to set green led");

    return PASSED;
}

static int manhattan_active_led_util(long port, int onoff)
{
    int      i     = 0;
    int      uret  = 0;
    uint32_t ctrl;
    struct {
        int   idx;
        char *name;
    } gpio[] = {
        {SWITZER_MANHATTAN_LED_PORT0_ACTIVE_GRN, "LED_PORT0_ACTIVE_GRN"}, /* single color */
        {SWITZER_MANHATTAN_LED_PORT1_ACTIVE_GRN, "LED_PORT1_ACTIVE_GRN"}, /* single color */
    };

    //1M: by x710 : PORT0
    //2T: by x710 : PORT0 and PORT1
    i = port;
    ERET_COND(0 > switzer_manhattan_x710_gpio_ctrl_get(mod, 0, gpio[i].idx, &ctrl),
        FAILED, "Failed to get %s gpio_ctrl value\n", gpio[i].name);
    if (onoff)
        onoff = 0xF;
    else
        onoff = 0x0;

    I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl, LED_BLINK);
    I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl, LED_MODE);
    I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0     , LED_BLINK);
    I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, onoff , LED_MODE);
    EURET_COND(0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, gpio[i].idx, ctrl),
        uret, FAILED, "Failed to set LED pin %s %s\n", gpio[i].name, onoff & 0xF ? "On" : "Off");
    uret = PASSED;

_EXIT_POINT:
    return uret;
}

static char *sfp_link_leds_name[] = {
    "LED_SFP Port-0 Link LED(Green/Amber)",
    "LED_SFP Port-1 Link LED(Green/Amber)",
    NULL
};

static char *_bcm54194_leds_name[] = {
    "LED_RJ45 Port-0 Link LED(Green)",
    "LED_RJ45 Port-1 Link LED(Green)",
    NULL
};

static char *port_active_led_name[] = {
    "LED PORT0_ACTIVE Port-0 Active LED(Green)",
    "LED PORT1_ACTIVE Port-1 Active LED(Green)",
    NULL
};

static int manhattan_2t_led_util(int arg)
{
    int i   = 0;
    int j   = 0;
    int k   = 0;
    int on  = 0;
    int led = 0;

    printf("Choose LED:\n");
    for(i = 0; _bcm54194_leds_name[i]; i++) {
        printf("  %d: %s\n", i, _bcm54194_leds_name[i]);
    }

    for(j = 0; sfp_link_leds_name[j]; j++, i++){
        printf("  %d: %s\n", i, sfp_link_leds_name[j]);
    }

    for(k = 0; port_active_led_name[k]; k++, i++){
        printf("  %d: %s\n", i, port_active_led_name[k]);
    }

    led = getdec_answer("> ", 0, 0, i - 1);
    if (led < 2 || led > 3)
    on  = getdec_answer("On-1, off-0", 0, 0, 1);

    if (led < 2)
        manhattan_bcm54194_led_util(led, on);
    if (led > 1 && led < 4)
        manhanttan_sfp_link_led_util(led);
    if (led > 3)
        manhattan_active_led_util(led - 4, on);
    return PASSED;
}

#define DUMP_PCA9557_REGS(PCA, TAG, ...) do {                                                      \
    if (TAG)prt(TAG, ##__VA_ARGS__);                                                               \
    ERET_COND(FAILED == io_port_8bit_i2c_read(PCA, SWITZER_MANHATTAN_PCA9557_IN_REG , &chr, TRUE), \
              FAILED, "Failed to read pca9557-%02X IN_REG \n", (PCA)->i2c_dev);                    \
    prt("..pca9557-%02X IN_REG :0x%02x\n", (PCA)->i2c_dev, chr);                                   \
    ERET_COND(FAILED == io_port_8bit_i2c_read(PCA, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr, TRUE), \
              FAILED, "Failed to read pca9557-%02X OUT_REG\n", (PCA)->i2c_dev);                    \
    prt("..pca9557-%02X OUT_REG:0x%02x\n", (PCA)->i2c_dev, chr);                                   \
    ERET_COND(FAILED == io_port_8bit_i2c_read(PCA, SWITZER_MANHATTAN_PCA9557_POL_REG, &chr, TRUE), \
              FAILED, "Failed to read pca9557-%02X POL_REG\n", (PCA)->i2c_dev);                    \
    prt("..pca9557-%02X POL_REG:0x%02x\n", (PCA)->i2c_dev, chr);                                   \
    ERET_COND(FAILED == io_port_8bit_i2c_read(PCA, SWITZER_MANHATTAN_PCA9557_CTL_REG, &chr, TRUE), \
              FAILED, "Failed to read pca9557-%02X CTL_REG\n", (PCA)->i2c_dev);                    \
    prt("..pca9557-%02X CTL_REG:0x%02x\n", (PCA)->i2c_dev, chr);                                   \
}while(0)

static long rj45_led_util(void)
{
    int port = 0;
    int type = 0;
    int color= 0;
    int on   = 0;
    int gpio = 0;
    int uret = 0;
    uint8_t chr = 0;
    uint32_t ctrl;
    struct {
        struct {
            int   gpio;
            char *name;
        } leds[2];
    } ports [] = {
        {{
            {SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P, "LED_PORT0_POE_YEL"}, /* bicolor led */
            {SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P, "LED_PORT0_POE_GRN"}  /* bicolor led */
        }},
        {{
            {SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P, "LED_PORT1_POE_YEL"}, /* bicolor led */
            {SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P, "LED_PORT1_POE_GRN"}  /* bicolor led */
        }},
    };
    n2g_i2c_if_t *pca = switzer_ngio_pca();

    port = getdec_answer("rj45 port(0/1):", 0, 0, 1);
    type = getdec_answer("0:link-status, 1:poe-status, 2:active-status", 0, 0, 2);
    if (type == 0) {
        on   = getdec_answer("0:off, 1:on", 0, 0, 1);
        gpio = port == 0 ? SWITZER_MANHATTAN_LED_PORT0_LNK_GRN : SWITZER_MANHATTAN_LED_PORT1_LNK_GRN;
        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio, !on), /* active low ? */
                  FAILED, "Failed to set green led(Gpio-%d) to %d\n", gpio, on);
    } else if (type == 1) {
        color = getdec_answer("color(0:off, 1:amber, 2:green):", 0, 0, 2);

        pca->i2c_dev = SWITZER_MANHATTAN_I2C_ADDR_PCA2;
        DUMP_PCA9557_REGS(pca, "PCA-2 regs before write:\n");

        ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr, TRUE),
                FAILED, "Failed to read pca9557 reg-1\n");

        /* 00-off, 01-amber, 10-green, 11-off */
        if (color != 0) {
            chr |= (1 << ports[port].leds[0].gpio);
            chr |= (1 << ports[port].leds[1].gpio);
            chr &= ~(1 << ports[port].leds[color - 1].gpio); /* active low */

            ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr),
                      FAILED, "Failed to write pca9557 reg-1\n");
        } else {
            chr = 0;
            ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr),
                      FAILED, "Failed to write pca9557 reg-1\n");
        }
        DUMP_PCA9557_REGS(pca, "PCA-2 regs after write:\n");
    } else {
        on   = getdec_answer("0:off, 1:on", 0, 0, 1);
        gpio = SWITZER_MANHATTAN_LED_PORT0_ACTIVE_GRN;
        ERET_COND(0 > switzer_manhattan_x710_gpio_ctrl_get(mod, 0, gpio, &ctrl),
            FAILED, "Failed to get gpio_ctrl value\n");
        if (on)
            on = 0xF;
        else
            on = 0x0;
    
        I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl, LED_BLINK);
        I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl, LED_MODE);
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0     , LED_BLINK);
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, on , LED_MODE);
        EURET_COND(0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, gpio, ctrl),
            uret, FAILED, "Failed to set LED pin %s %s\n");
        uret = PASSED;
    
    _EXIT_POINT:
        return uret;
        }

    prt("OK\n");
    return PASSED;
}

static long show_intnl_rj45_led_util(void)
{
    return is_manhattan_4t() || is_manhattan_2m() || is_manhattan_1m();
}
static submenu_xtable_t led_util_submenu_table[] = {
    {"Power Led Util", (type_t(*)())power_led_util             , 0, 0, NULL                    , 0, NULL, 0},
    {"SFP   Led Util", (type_t(*)())sfp_led_util               , 0, 0, is_manhattan_4t         , 0, NULL, 0},
    {"RJ45  Led Util", (type_t(*)())rj45_led_util              , 0, 0, show_intnl_rj45_led_util, 0, NULL, 0},
    {"Port  Led Util", (type_t(*)())manhattan_2t_led_util, 0, 0, is_manhattan_2t         , 0, NULL, 0},
};

#define LED_UTIL_SUBMENU_TABLE_SZ (sizeof(led_util_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t led_util_submenu_primary_items[LED_UTIL_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t led_util_submenu_secondary_items[LED_UTIL_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];

static char led_util_submenu_title[] = "Switzer Manhattan Led Utilities Menu";

static menuinfo_t led_util_submenu = {
    led_util_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_util_submenu_primary_items,
};
static menuinfo_t *led_util_submenup = &led_util_submenu;

static long led_utils(void)
{
    build_primary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SZ,
                          led_util_submenu_title, &led_util_submenup);
    build_secondary_submenu(led_util_submenu_table, LED_UTIL_SUBMENU_TABLE_SZ,
                            led_util_submenu_secondary_items);
    menu(led_util_submenup, led_util_submenu_secondary_items, '\0');
    return PASSED;
}

static long phy_status_dump(void)
{
    int rc;
    switzer_lane_t lane;
    switzer_if_side_t if_side;
    unsigned int flags;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    flags = gethex_answer("Enter Dump Flags", 0, 0, 0xffffffff);

    mod->miura.info.flags = flags;
    rc = switzer_manhattan_ephy_dump(mod, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 dump error");
        return FAILED;
    }
    return PASSED;
}

static long phy_mac_diagnostic_dump(void)
{
    int rc;
    switzer_lane_t lane;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_manhattan_ephy_mac_dump(mod, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 dump error");
        return FAILED;
    }
    return PASSED;
}

static long phy_link_status(void)
{
    int rc;
    switzer_lane_t lane;
    unsigned int link_status;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_manhattan_ephy_link_status(mod, lane, if_side, &link_status);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 link status get error");
        return FAILED;
    }
    prt("phy link status: %d\n",link_status);
    return PASSED;
}

#define PHY_LINK_UP_DELAY 2000

static int get_phy_link(switzer_if_side_t if_side,
                        switzer_lane_t lane, unsigned int *link_status)
{
    int i, rc;

    for (i = 0; i < 12; i++) {
        rc = switzer_manhattan_ephy_link_status(mod, lane, if_side, link_status);
        if (rc < 0) {
            cterr_add_component("BCM82757",
                            "MDIO controller within X710");
            cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
            cterr('f', 0, "BCM82757 link status get error");
            return FAILED;
        }
        if (*link_status) {
            break;
        }
        mdelay(PHY_LINK_UP_DELAY);
    }
    return PASSED;
}

static long phy_firmware_download(void)
{
    struct switzer_miura *miura = &mod->miura;

    switzer_miura_reset(miura);

    if (switzer_miura_fw_download(miura)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_loopback(void)
{
    switzer_lane_t lane;
    switzer_if_side_t if_side;
    unsigned int lb_mode, enable;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    lb_mode = gethex_answer("Enter Loopback mode", 1, 0, 10);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (switzer_manhattan_miura_loopback_set(mod, lane, if_side, lb_mode, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_prbs(void)
{
    switzer_lane_t lane;
    switzer_if_side_t if_side;
    unsigned int action, poly, enable = 0;
    switzer_prbs_t prbs = SWITZER_PRBS_31;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (if_side == SWITZER_IF_SIDE_SYS) {
        cterr_add_component("BCM82757",
                            "Serdes link on BCM82757 system side");
        cterr_add_debug("Check BCM82757",
                        "Check Serdes link on BCM82757 system side");
    } else {
        cterr_add_component("BCM82757",
                            "Serdes link on BCM82757 line side");
        cterr_add_debug("Check BCM82757",
                        "Check Serdes link on BCM82757 line side");
    }
    action = gethex_answer("Enter Action(Check:0, Enable:1, Disable:2)",
                           0, 0, 2);

    switch (action) {
    default:
    case 0:
        if (switzer_manhattan_miura_prbs_check(mod, lane, if_side)) {
            cterr('f', 0, "BCM82757 PRBS check failed");
            return FAILED;
        }
        break;

    case 1:
        poly = getdec_answer("PRBS Polynomial(7, 9, 11, 15, 23, 31)", 31, 0, 31);
        switch (poly) {
        case 7:
            prbs = SWITZER_PRBS_7;
            break;
        case 9:
            prbs = SWITZER_PRBS_9;
            break;
        case 11:
            prbs = SWITZER_PRBS_11;
            break;
        case 15:
            prbs = SWITZER_PRBS_15;
            break;
        case 23:
            prbs = SWITZER_PRBS_23;
            break;
        default:
        case 31:
            prbs = SWITZER_PRBS_31;
            break;
        }
        enable = 1;
    case 2:
        if (switzer_manhattan_miura_prbs_set(mod, lane, if_side, prbs, enable)) {
            cterr('f', 0, "BCM82757 PRBS set failed");
            return FAILED;
        }
        break;
    }

    return PASSED;
}

static long phy_firmware_lane_config_set(void)
{
    switzer_lane_t lane;
    switzer_if_side_t if_side;
    bcm_plp_pm_firmware_lane_config_t firmware_lane_config;

    cterr_add_component("BCM82757",
                        "MDIO controller within X710");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within X710");

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);
    if (switzer_manhattan_miura_firmware_lane_get(mod, lane,
                                                  if_side, &firmware_lane_config)) {
        cterr('f', 0, "switzer firmware lane get failed");
        return FAILED;
    }
    firmware_lane_config.firmware_mode = gethex_answer(
                           "0 : default\n"
                           "1 : dfe mode\n"
                           "2 : osdfe mode\n"
                           "3 : baud rate dfe mode\n"
                           "4 : low power dfe mode\n"
                           "5 : media type sfp dac\n"
                           "6 : media type xlaui\n"
                           "7 : media type optical sr4\n", 0, 0, 7);
    firmware_lane_config.ena_dis = gethex_answer(
                           "ena_dis(Disable:0, Enable:1)", 1, 0, 1);
    firmware_lane_config.AnEnabled = gethex_answer(
                           "Autonego( Disable:0, Enable:1)", 1, 0, 1);
    firmware_lane_config.MediaType = gethex_answer(
                           "MediaType(PcbBackTrace:0, Copper:1, Optics:2)", 1, 0, 2);
    firmware_lane_config.DbLoss = gethex_answer(
                           "DbLossValue(0 - 100)", 1, 0, 2);

    if (switzer_manhattan_miura_firmware_lane_set(mod, lane,
                                                  if_side, &firmware_lane_config)) {
        cterr('f', 0, "switzer firmware lane set failed");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_cl73(void)
{
    switzer_lane_t lane;
    switzer_if_side_t if_side;
    unsigned int enable;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (switzer_manhattan_miura_cl73_set(mod, lane, if_side, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_macsec_bypass(void)
{
    switzer_lane_t lane;
    int speed_flag, speed;
    bcm_pm_interface_t sys_inf, line_inf;

    lane = getdec_answer("Enter Lane(0-3)", 0, 0, 3);
    speed_flag = getdec_answer("Speed(10G:0, 1G:1)", 0, 0, 1);
    sys_inf = getdec_answer("system side interface:\n"
                            "bcm_pm_InterfaceSR    - 1\n"
                            "bcm_pm_InterfaceKX    - 3\n"
                            "bcm_pm_InterfaceKR    - 5 (defualt)\n"
                            "bcm_pm_InterfaceXFI   - 15\n"
                            "bcm_pm_InterfaceSFI   - 16\n"
                            "bcm_pm_Interface1000X - 19\n"
                            "bcm_pm_InterfaceSGMII - 20\n"
                            "bcm_pm_InterfaceXAUI  - 21", bcm_pm_InterfaceKR, 0, bcm_pm_InterfaceCount);

    line_inf = getdec_answer("line side interface:\n"
                             "bcm_pm_InterfaceSR    - 1\n"
                             "bcm_pm_InterfaceKX    - 3\n"
                             "bcm_pm_InterfaceKR    - 5\n"
                             "bcm_pm_InterfaceXFI   - 15\n"
                             "bcm_pm_InterfaceSFI   - 16 (default)\n"
                             "bcm_pm_Interface1000X - 19\n"
                             "bcm_pm_InterfaceSGMII - 20\n"
                             "bcm_pm_InterfaceXAUI  - 21", bcm_pm_InterfaceSFI, 0, bcm_pm_InterfaceCount);

    if (speed_flag) {
        speed = SWITZER_PORT_SPEED_1G;
    } else {
        speed = SWITZER_PORT_SPEED_10G;
    }

    if (switzer_manhattan_miura_config_interface_macsec_bypass(mod, lane, sys_inf, line_inf, speed)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        return FAILED;
    }

    return PASSED;
}

static long phy_display_eye_scan(void)
{
    switzer_lane_t lane;
    int rc;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_manhattan_display_eye_scan(mod, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        cterr('f', 0, "BCM82757 eye scan failed");
        return FAILED;
    }
    return PASSED;
}

static const reg_info_t phy_miura_reg_tbl[] = {
    {"General Purpose Register 1C", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Cr,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1D", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Dr,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1E", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Er,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1F", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Fr,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"End of General Purpose Register", 0, 0, {0}, 0, 0},
};

static int __phy_register_test(const reg_info_t *regs,
                               switzer_lane_t lane, switzer_if_side_t if_side)
{
    uint32_t i;
    uint32_t regaddr, devaddr;
    uint32_t data, data_orig, data_test;

    while (regs->size.size != 0) {
        regaddr = regs->offset;
        devaddr = regaddr >> 16;
        if (switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, regaddr, &data_orig) < 0) {
            cterr('f', 0, "Error reading %s register offset %d.%#x",
                  regs->name, devaddr, regaddr);
            return FAILED;
        }

        if (regs->type == READ_WRITE) {
            /* ripple 1 test */
            for (i = 0; i < regs->size.size * 8; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                /* Write to register under test */
                if (switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, regaddr, data_test) < 0 ||
                    switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Ripple one test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
            }

            /* ripple 0 test */
            for (i = 0; i < regs->size.size * 8; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                data_test = (~(1 << i)) & regs->mask;
                /* Write to register under test */
                if (switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, regaddr, data_test) < 0 ||
                    switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Ripple zero test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
            }

            /* pattern test */
            data = 0x5adb;
            for (i = 0; i < 2; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                /* Write to register under test */
                if (switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, regaddr, data_test) < 0 ||
                    switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Pattern test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
                data = ~data;   /* complement data pattern */
            }

            /* restore original value */
            if (switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, regaddr, data_test) < 0) {
                cterr('f', 0, "Error restoring %s register offset %d.%#x",
                      regs->name, devaddr, regaddr);
                return FAILED;
            }
        }
        regs++;
    }

    return PASSED;
}

static long phy_register_test(void)
{
    switzer_lane_t lane = SWITZER_LANE_0;
    switzer_if_side_t if_side = SWITZER_IF_SIDE_SYS;

    prpass(testpass, "BCM82757 Register Test ");

    if (__phy_register_test(phy_miura_reg_tbl, lane, if_side) == FAILED) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        cterr('f', 0, "Register Test on BCM82757 failed");
        return FAILED;
    }
    return PASSED;
}

static long phy_intr_test(void)
{
    int      i    = 0;
    int      j    = 0;
    int      uret = 0;
    uint32_t data = 0;
    uint32_t gpio = 0;
    uint32_t regv = 0;
    uint32_t rega = 0;

    const switzer_if_side_t if_side = SWITZER_IF_SIDE_SYS;
    const uint32_t devaddr  = SWITZER_MIURA_DEV_PMA_PMD;
    const uint16_t val_high = 0x0a60; //0000 1010 0110 0000
    const uint16_t val_low  = 0x0260; //0000 0010 0110 0000
  //const uint16_t val_reset= 0x8460; //1000 0100 0110 0000

    struct {
        char     *name;
        int       x710_gpio;
        uint32_t  phy_lsi;
        uint32_t  save;
        uint16_t  phy_lsi_seq[2];
        char      *phy_lsi_seq_name[2];
    } test_seq[2] = {
        {
            "SWITZER_MANHATTAN_P2_LASI0_INT",
            SWITZER_MANHATTAN_P2_LASI0_INT,
            BCMI_MIURA_DIRECT_PAD_CNTRL_LASI_0_CONTROLr,
            0,
            {val_high, val_low},
            {"HIGH"  , "LOW"  }
        },

        {
            "SWITZER_MANHATTAN_P3_LASI1_INT",
            SWITZER_MANHATTAN_P3_LASI1_INT,
            BCMI_MIURA_DIRECT_PAD_CNTRL_LASI_1_CONTROLr,
            0,
            {val_high, val_low},
            {"HIGH"  , "LOW"  }
        },
    };

    prt("Save phy reg values.\n");
    for(i = 0; i < 2; i++) {
        rega = test_seq[i].phy_lsi;
        ERET_COND(0 != switzer_manhattan_ephy_read(mod, 0, if_side, devaddr, rega, &test_seq[i].save),
            FAILED, "Failed to read BCM82757 register\n");
        prt("  @0x%04x:0x%04x\n", rega, test_seq[i].save);
    }

    for(i = 0; i < 2; i++) {
        prt("Test %s\n", test_seq[i].name);

        rega = test_seq[i].phy_lsi;
        for(j = 0; j < 2; j++) {
            data = test_seq[i].phy_lsi_seq[j];
            prt("  Set to %s(@0x%04x:0x%04x)\n", test_seq[i].phy_lsi_seq_name[i], rega, data);

            EURET_COND(0 != switzer_manhattan_ephy_write(mod, 0, if_side, devaddr, rega, data),
                uret, FAILED, "Failed to write BCM82757 register\n");

            prt("  Check x710 gpio\n");
            EURET_COND(0 > (gpio = switzer_manhattan_x710_gpio_get(mod, 0, test_seq[i].x710_gpio, &regv)),
                uret, FAILED, "Failed to read x710 gpio.\n");
            prt("  Gpio-%d, regv:0x%08x\n", test_seq[i].x710_gpio, regv);

            EURET_COND((gpio != (j == 0 ? 1 : 0)), uret, FAILED, "GPIO (%d) value(0x%08x) is not as expected.\n",
                test_seq[i].x710_gpio, regv);

            prt("  OK\n");
        }
    }

    uret = PASSED;
_EXIT_POINT:
    prt("Restore phy reg values\n");
    for(i = 0; i < 2; i++) {
        rega = test_seq[i].phy_lsi;
        ERET_COND(0 != switzer_manhattan_ephy_write(mod, 0, if_side, devaddr, rega, test_seq[i].save),
            FAILED, "Failed to read BCM82757 register\n");
        prt("  @0x%04x:0x%04x\n", rega, test_seq[i].save);
    }
    return uret;
}

static long rj45_external_lpbk_test(int port_s, int port_r)
{
    FILE      *fp       = NULL;
    char       cmd[256] = {0,};
    char       buf[512] = {0,};
    int        bus      = 0;
    int        try_idx  = 0;
    const int  try_max  = 5;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the test\n");
        return PASSED;
    }
    if (is_manhattan_2t()) {
        if (port_s == port_r) {
            prpass(testpass, "Port%d External Loopback Test ", port_s);
        } else {
            prpass(testpass, "Port%d -> Port%d Loopback Test ", port_s, port_r);
        }

        if (switzer_manhattan_sock_test(mod->eth_port.intnl_port[port_s], mod->eth_port.intnl_port[port_r])) {
            log_err("switzer socket test failed");
            return FAILED;
        }
        return PASSED;
    }

    /* 2M, 4T and 1M, RJ45 are internal port of 710 */
    /* TODO:Currently, use Intel's tool 'celo64e' to test external loopback */
    ERET_COND(port_s != port_r, FAILED, "Something must be error.\n");

    ERET_COND(0 > (bus = get_ngio_pcie_dev_bus_num(mod->ngio->mod_type, mod->ngio->slot)), FAILED, "Failed to get PCI bus numb.\n");

    snprintf(cmd, sizeof(cmd),
            "rm -f CELO.LOG CELODBG.LOG; celo64e /CARD %u 0 %u /EXTLB /TIMEOUT 30 /DEBUGLOG=0xffffffff", bus, port_s);
    for (try_idx = 0; try_idx < try_max; try_idx++) {
        prt("\nRun external loopback test by Intel tool 'celo64e', trace is saved in CELO.LOG and CELODBG.LOG.\n\n");
        prt("Try %02d ------\n", try_idx);
        ERET_COND(!(fp = popen(cmd, "r")), -1, "Failed to run cmd '%s'\n", cmd);
        while ((fgets(buf, sizeof(buf), fp))) {
            if (!strstr(buf, "hit <ESC> to abort"))
                prt("%s", buf);
            if (strstr(buf, "FAILED")) {
                try_idx = try_max + 999; /* Stop test, complete while() to print all trace then stop by for() loop */
            }
            memset(buf, 0, sizeof(buf));
        }
        pclose(fp);
        switzer_mdelay(1000);
    }
    if (try_idx > try_max) {
        log_err("Failed external loopback test on RJ45 port-%d\n", port_s);
        return FAILED;
    }
    return PASSED;
}

static long rj45_lpbk_test(int port)
{
    if (rj45_external_lpbk_test(port, port)) {
        cterr('f', 0, "Port%d external loopback test failed", port);
        return FAILED;
    }

    return PASSED;
}

static long phy_internal_lpbk_test(switzer_lane_t lane, int speed)
{
    unsigned int sys_link;
    int slot = mod->ngio->slot;
    unsigned int module_type = mod->ngio->mod_type;
    ngio_eth_speed_t new_speed, old_speed;

    cterr_add_component("BCM82757",
                        "MDIO controller within V710");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within V710");

    prpass(testpass, "External PHY Internal Loopback Test, lane: %d ", lane);

    if (switzer_manhattan_miura_config_macsec_bypass(mod, lane, speed)) {
        log_err("set macsec 10g bypass failed");
        return FAILED;
    }

    if (switzer_manhattan_miura_loopback_set(mod, lane, SWITZER_IF_SIDE_LINE, 1, 1)) {
        log_err("set loopback failed");
        return FAILED;
    }

    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_manhattan_miura_cl73_set(mod, lane, SWITZER_IF_SIDE_SYS, 1)) {
            log_err("set cl73 failed");
            return FAILED;
        }
    } else {
        /* need force ngio eth to 1G in 1G lpbk test */
        if (is_curie_2ru()) {
            new_speed = NGIO_ETH_SPEED_1G;
            ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);
        }
    }

    if (get_phy_link(SWITZER_IF_SIDE_SYS, lane, &sys_link)) {
        log_err("get phy link failed");
        return FAILED;
    }
    if (!sys_link) {
        prt("sys_link %d\n", sys_link);
        log_err("switzer link err");
        return FAILED;
    }
    mdelay(PHY_LINK_UP_DELAY);

    if (switzer_manhattan_sock_test(mod->eth_port.extnl_port[lane], mod->eth_port.extnl_port[lane])) {
        log_err("switzer socket test failed");
        return FAILED;
    }

    if (switzer_manhattan_miura_loopback_set(mod, lane, SWITZER_IF_SIDE_LINE, 1, 0)) {
        log_err("set loopback failed");
        return FAILED;
    }

    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_manhattan_miura_cl73_set(mod, lane, SWITZER_IF_SIDE_SYS, 0)) {
            log_err("set cl73 failed");
            return FAILED;
        }
    } else {
        if (is_curie_2ru()) {
            ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);
        }
    }

    switzer_manhattan_miura_config_macsec_cleanup(mod);

    return PASSED;
}

static long phy_external_lpbk_test(switzer_lane_t lane, int speed)
{
    unsigned int sys_link, line_link;
    int slot = mod->ngio->slot;
    unsigned int module_type = mod->ngio->mod_type;
    ngio_eth_speed_t new_speed, old_speed;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM82757 external loopback test\n");
        return PASSED;
    }

    prpass(testpass, "External PHY External Loopback Test, lane: %d ", lane);

    cterr_add_component("BCM82757",
                        "MDIO controller within V710");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within V710");

    if (switzer_manhattan_miura_config_macsec_bypass(mod, lane, speed)) {
        log_err("set macsec 10g bypass failed");
        return FAILED;
    }
    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_manhattan_miura_cl73_set(mod, lane, SWITZER_IF_SIDE_SYS, 1)) {
            log_err("set cl73 failed");
            return FAILED;
        }
    } else {
        /* need force ngio eth to 1G in 1G lpbk test */
        if (is_curie_2ru()) {
            new_speed = NGIO_ETH_SPEED_1G;
            ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);
        }
    }

    if (get_phy_link(SWITZER_IF_SIDE_LINE, lane, &line_link)) {
        cterr('f', 0, "get phy link failed");
        return FAILED;
    }
    if (get_phy_link(SWITZER_IF_SIDE_SYS, lane, &sys_link)) {
        cterr('f', 0, "get phy link failed");
        return FAILED;
    }
    if (!sys_link || !line_link) {
        prt("sys_link %d, line_link %d\n", sys_link, line_link);
        cterr('f', 0, "switzer link err");
        return FAILED;
    }
    mdelay(PHY_LINK_UP_DELAY);

    if (switzer_manhattan_sock_test(mod->eth_port.extnl_port[lane], mod->eth_port.extnl_port[lane])) {
        cterr('f', 0, "switzer socket test failed");
        return FAILED;
    }

    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_manhattan_miura_cl73_set(mod, lane, SWITZER_IF_SIDE_SYS, 0)) {
            cterr('f', 0, "set cl73 failed");
            return FAILED;
        }
    } else {
        if (is_curie_2ru()) {
            ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);
        }
    }
    switzer_manhattan_miura_config_macsec_cleanup(mod);
    return PASSED;
}

static long ext_phy_lpbk_test(switzer_lane_t lane)
{
    if (is_manhattan_4t()) {
        if (phy_internal_lpbk_test(lane, SWITZER_PORT_SPEED_1G)) {
            cterr('f', 0, "external PHY lane %d speed 1G internal loopback test failed", lane);
            return FAILED;
        }

        if (phy_external_lpbk_test(lane, SWITZER_PORT_SPEED_1G)) {
            cterr('f', 0, "external PHY lane %d speed 1G external loopback test failed", lane);
            return FAILED;
        }
        return PASSED;
    }

    return PASSED;
}

#define PRBS_TEST_DELAY 1

static long __phy_prbs_test(switzer_if_side_t if_side, switzer_lane_t lane,
                            switzer_prbs_t prbs, uint32_t delay_sec)
{
    uint32_t enable = 1;
    const int delay_a = 3000; /* ms */
    const int delay_b = 5000; /* ms */

    /* enable prbs */
    prt("Enable prbs.\n");
    if (switzer_manhattan_miura_prbs_set(mod, lane, if_side, prbs, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        cterr('f', 0, "BCM82757 PRBS set enable failed");
        return FAILED;
    }

    /* wait for prbs lock */
    prt("Delay %dms to lock prbs...\n", delay_sec * delay_a);
    switzer_mdelay(delay_sec * delay_a);
    /* clear prbs rx stat */
    if (switzer_manhattan_prbs_clear_rx_stat(mod, lane, if_side)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller with X710");
        cterr('f', 0, "BCM82757 PRBS clear rx stat failed");
    }

    /* check prbs */
    prt("Delay %dms to check prbs...\n", delay_sec * delay_b);
    switzer_mdelay(delay_sec * delay_b);
    if (switzer_manhattan_miura_prbs_check(mod, lane, if_side)) {
        cterr_add_component("BCM82757",
                            "SFP plugin link status");
        cterr_add_debug("Check BCM82757",
                        "Check the SFP plugin link status");
        cterr('f', 0, "BCM82757 PRBS check failed");
        return FAILED;
    }

    /* disable prbs */
    enable = 0;
    prt("Disable prbs.\n");
    if (switzer_manhattan_miura_prbs_set(mod, lane, if_side, prbs, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within X710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within X710");
        cterr('f', 0, "BCM82757 PRBS set disable failed");
        return FAILED;
    }

    return PASSED;
}

static long phy_prbs_test(switzer_lane_t lane, switzer_if_side_t if_side)
{
    switzer_prbs_t prbs = SWITZER_PRBS_7;

    if (if_side == SWITZER_IF_SIDE_SYS) {
        prpass(testpass, "PHY SYS PRBS Test ");
    } else {
        if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            printf("External loopback flag is off, skip the line side prbs test\n");
            return PASSED;
        }

        prpass(testpass, "PHY LINE PRBS Test ");
    }

    if (switzer_manhattan_miura_config_macsec_bypass(mod, lane, SWITZER_PORT_SPEED_10G)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }

    while (prbs <= SWITZER_PRBS_31) {
        prt("\nPrbs pattern-%d\n", prbs);
        if (__phy_prbs_test(if_side, lane, prbs, PRBS_TEST_DELAY) == FAILED)
            return FAILED;
        prbs++;
    }

    switzer_manhattan_miura_config_macsec_cleanup(mod);

    return PASSED;
}

static long phy_line_prbs_test(switzer_lane_t lane)
{
    if (phy_prbs_test(lane, SWITZER_IF_SIDE_LINE))
        return FAILED;

    return PASSED;
}

static void phy_show_tx_param(switzer_if_side_t if_side, bcm_plp_tx_t *tx_param) {
    if (if_side == SWITZER_IF_SIDE_SYS) {
        printf("\nsys side tx param\n"
               "pretap  : %d\n"
               "maintap : %d\n"
               "post    : %d\n"
               "post2   : %d\n"
               "post3   : %d\n"
               "amp     : %d\n\n",
               tx_param->pre, tx_param->main, tx_param->post, tx_param->post2,
               tx_param->post3, tx_param->amp);
    } else {
        printf("\nline side tx param\n"
               "pretap  : %d\n"
               "maintap : %d\n"
               "post    : %d\n"
               "post2   : %d\n\n",
               tx_param->pre, tx_param->main, tx_param->post, tx_param->post2);
    }
}

static long phy_tx_get()
{
    switzer_lane_t lane;
    bcm_plp_tx_t tx_param;
    switzer_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);

    if (switzer_manhattan_miura_tx_get(mod, lane, if_side, &tx_param)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        return FAILED;
    }

    phy_show_tx_param(if_side, &tx_param);

    return PASSED;
}

static long phy_tx_set()
{
    switzer_lane_t lane;
    bcm_plp_tx_t tx_param;
    switzer_if_side_t if_side;
    int answ = 1;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);

    if (switzer_manhattan_miura_tx_get(mod, lane, if_side, &tx_param)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within V710");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within V710");
        return FAILED;
    }

    phy_show_tx_param(if_side, &tx_param);

    while (answ) {
        if (if_side == SWITZER_IF_SIDE_SYS) {
            answ = gethex_answer("Enter Tap(pre:1, main:2, post:3, "
                                 "post2:4, post3:5, amp:6, exit:0)", 0, 0, 6);
        } else {
            answ = gethex_answer("Enter Tap(pre:1, main:2, post:3, "
                                 "post2:4, exit:0)", 0, 0, 4);
        }

        switch (answ) {
        case 1:
            tx_param.pre = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 2:
            tx_param.main = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 3:
            tx_param.post = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 4:
            tx_param.post2 = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 5:
            tx_param.post3 = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 6:
            tx_param.amp = getdec_answer("Enter Value", 0, 0, 255);
            break;
        default:
        continue;
            break;
        }

        if (switzer_manhattan_miura_tx_set(mod, lane, if_side, &tx_param)) {
            cterr_add_component("BCM82757",
                                "MDIO controller within V710");
            cterr_add_debug("Check BCM82757",
                            "Check MDIO controller within V710");
            return FAILED;
        }
        phy_show_tx_param(if_side, &tx_param);
    }
    return PASSED;
}

static long phy_config_tx_disable()
{
    switzer_lane_t lane;
    int en;

    lane = gethex_answer("Enter Lane(0-3)", 0, 0, 3);
    en = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (switzer_manhattan_ephy_config_tx_disable(mod, lane, en)) {
        log_warn("switzer_manhattan_ephy_config_tx_disable failed\n");
        return -1;
    }

    return PASSED;
}

/* PHY Utils submenu items */
static submenu_xtable_t phy_utils_submenu_table[] = {
    {"Broadcom PHY Register Read", utils_phy_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Register Write", utils_phy_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Autoneg Remote Ability Get", phy_autoneg_remote_ability_get, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Registers dump", phy_registers_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Status dump", phy_status_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY MAC Diagnostic Dump", phy_mac_diagnostic_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY link status get", phy_link_status, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Firmware Download", phy_firmware_download, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Display Eye Scan", phy_display_eye_scan, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Loopback", phy_config_loopback, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config PRBS", phy_config_prbs, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Firmware Lane", phy_firmware_lane_config_set, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Clause 73", phy_config_cl73, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Macsec Bypass", phy_config_macsec_bypass, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config TX Disable", phy_config_tx_disable, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY TX Config Get", phy_tx_get, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY TX Config Set", phy_tx_set, 0,
     0, NULL, 0, NULL, 0},
};

#define PHY_UTILS_SUBMENU_TABLE_SZ (sizeof(phy_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t phy_utils_submenu_primary_items[PHY_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t phy_utils_submenu_secondary_items[PHY_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char phy_utils_submenu_title[] = "Switzer Manhattan External PHY Utilities Menu";

static menuinfo_t phy_utils_submenu = {
    phy_utils_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    phy_utils_submenu_primary_items,
};

static menuinfo_t *phy_utils_submenup = &phy_utils_submenu;

static long phy_utils(int show_menu)
{
    build_primary_submenu(phy_utils_submenu_table, PHY_UTILS_SUBMENU_TABLE_SZ,
                          phy_utils_submenu_title, &phy_utils_submenup);
    build_secondary_submenu(phy_utils_submenu_table, PHY_UTILS_SUBMENU_TABLE_SZ,
                            phy_utils_submenu_secondary_items);
    menu(phy_utils_submenup, phy_utils_submenu_secondary_items, '\0');
    return PASSED;
}

/* X710 Internal PHY Utils submenu items */
static submenu_xtable_t internal_phy_utils_submenu_table[] = {
    {"Internal PHY Register Read" , utils_intnl_phy_reg_read , 0,
     0                            , NULL                     , 0, NULL, 0},
    {"Internal PHY Register Write", utils_intnl_phy_reg_write, 0,
     0                            , NULL                     , 0, NULL, 0},

    {"Set Port Speed 100Mbps"     , utils_intnl_phy_set_speed, X710_INTL_PHY_100M,
     0                            , NULL                     , 0, NULL, 0},

    {"Set Port Speed 1Gbps"       , utils_intnl_phy_set_speed, X710_INTL_PHY_1G,
     0                            , NULL                     , 0, NULL, 0},

    {"Set Port Speed 2.5Gbps"     , utils_intnl_phy_set_speed, X710_INTL_PHY_2P5G,
     0                            , NULL                     , 0, NULL, 0},

    {"1000Base-T Test Mode"       , utils_intnl_phy_test_mode, 0,
     0                            , NULL                     , 0, NULL, 0},

    {"100Base-T MDI Crossover"    , utils_intnl_phy_mdix_mode, 0,
     0                            , NULL                     , 0, NULL, 0},

    {"Cable Diagnostic Utility"   , utils_intnl_phy_cable_diag, 0,
     0                            , NULL                     , 0, NULL, 0},

    {"Internal PHY Reset"         , utils_intnl_phy_rst      , -1,
     0                            , NULL                     , 0, NULL, 0},
};

#define INTERNAL_PHY_UTILS_SUBMENU_TABLE_SZ (sizeof(internal_phy_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t internal_phy_utils_submenu_primary_items[INTERNAL_PHY_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t internal_phy_utils_submenu_secondary_items[INTERNAL_PHY_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char internal_phy_utils_submenu_title[] = "Switzer Manhattan Internal Phy Utils";

static menuinfo_t internal_phy_utils_submenu = {
    internal_phy_utils_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    internal_phy_utils_submenu_primary_items,
};

static menuinfo_t *internal_phy_utils_submenup = &internal_phy_utils_submenu;
static long internal_phy_utils(int show_menu)
{
    build_primary_submenu(internal_phy_utils_submenu_table, INTERNAL_PHY_UTILS_SUBMENU_TABLE_SZ,
                          internal_phy_utils_submenu_title, &internal_phy_utils_submenup);
    build_secondary_submenu(internal_phy_utils_submenu_table, INTERNAL_PHY_UTILS_SUBMENU_TABLE_SZ,
                            internal_phy_utils_submenu_secondary_items);
    menu(internal_phy_utils_submenup, internal_phy_utils_submenu_secondary_items, '\0');
    return PASSED;
}

static long __show_intnl_phy_util(void)
{
    return (is_manhattan_2m() || is_manhattan_4t() || is_manhattan_1m());
}
/* PHY submenu items */
static submenu_xtable_t phy_submenu_table[] = {
    {"Internal PHY Utilities"                , internal_phy_utils   , 0             ,
     0                                       , __show_intnl_phy_util, 0             , internal_phy_utils  , 0},
    {"Broadcom PHY Utilities"                , phy_utils            , 0             ,
     0                                       , is_manhattan_4t      , 0             , phy_utils           , 0},
    {"Broadcom PHY Register Test"            , phy_register_test    , 0             ,
     F_ALL_E                                 , is_manhattan_4t      , 0             , NULL                , 0},
    {"Broadcom PHY Interrupt Test"           , phy_intr_test        , 0             ,
     F_ALL_E                                 , is_manhattan_4t      , 0             , NULL                , 0},
    {"Broadcom PHY Lane0 Line Side PRBS Test", phy_line_prbs_test   , SWITZER_LANE_0,
     F_ALL_E                                 , is_manhattan_4t      , 0             , NULL                , 0},

    {"Broadcom PHY Lane1 Line Side PRBS Test", phy_line_prbs_test   , SWITZER_LANE_1,
     F_ALL_E                                 , is_manhattan_4t      , 0             , NULL                , 0},

    {"BCM541XX Test"                         , manhattan_bcm54194_test, 0             ,
     F_ALL_E                                 , is_manhattan_2t      , 0             , manhattan_bcm54194_test, 1},
};

#define PHY_SUBMENU_TABLE_SZ (sizeof(phy_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t phy_submenu_primary_items[PHY_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t phy_submenu_secondary_items[PHY_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char phy_submenu_title[] = "Switzer Manhattan External PHY Subtest Menu";

static menuinfo_t phy_submenu = {
    phy_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    phy_submenu_primary_items,
};

static menuinfo_t *phy_submenup = &phy_submenu;

static long phy_test(int show_menu)
{
    build_primary_submenu(phy_submenu_table, PHY_SUBMENU_TABLE_SZ,
                          phy_submenu_title, &phy_submenup);
    build_secondary_submenu(phy_submenu_table, PHY_SUBMENU_TABLE_SZ,
                            phy_submenu_secondary_items);
    if (show_menu)
        menu(phy_submenup, phy_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(phy_submenup);
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
                                    "Line to DS4424");
                cterr_add_debug("DS4424 on Switzer",
                                "Check the line to DS4424");
                cterr('f', 0, "Register Test on DS4424 failed");
                return FAILED;
            }
        }
    }
    return PASSED;
}

#define MANHATTAN_LED_TEST_WAIT(MSEC) do { \
    if (is_interactive(0, 0)) {            \
        prt("Press ENTER to continue..."); \
        while(getchar() != '\n');          \
    } else {                               \
        switzer_mdelay(MSEC);              \
    }                                      \
}while(0)

static long manhattan_pwr_led_tst(void)
{
    int i = 0;
    struct pwr_led_color {
        int   val ;
        char *desc;
    } colors [] = {
        { 0, "OFF"        },
        { 1, "AMBER"      },
        { 2, "GREEN"      },
        { 3, "AMBER ONLY" },
        { 4, "GREEN ONLY" },
        { 5, "AMBER OFF"  },
        { 6, "GREEN OFF"  },
        {-1, NULL         },
    };

    prt("\nTest power LED\n");
    for (i = 0; colors[i].desc; i++) {
        ERET_COND(0 != switzer_power_led_util(colors[i].val),
            FAILED, "Failed to set power LED to %d(%s).\n", colors[i].val, colors[i].desc);
        prt("..%s\n", colors[i].desc);
        MANHATTAN_LED_TEST_WAIT(2000);
    }
    return PASSED;
}

static long manhattan_sfp_pt_enb_led_tst(long port)
{
    int      i     = 0;
    int      uret  = 0;
    uint32_t onoff = 0;

    struct {
        int   idx;
        int   save;
        char *name;
    } gpio[] = {
        {SWITZER_MANHATTAN_LED_PORT2_ENB_YEL, 0, "LED_PORT2_ENB_YEL"}, /* bicolor led */
        {SWITZER_MANHATTAN_LED_PORT2_ENB_GRN, 0, "LED_PORT2_ENB_GRN"}, /* bicolor led */
        {SWITZER_MANHATTAN_LED_PORT3_ENB_YEL, 0, "LED_PORT3_ENB_YEL"}, /* bicolor led */
        {SWITZER_MANHATTAN_LED_PORT3_ENB_GRN, 0, "LED_PORT3_ENB_GRN"}, /* bicolor led */
    };

    i = port - 2;
    //for(i = 0; is_manhattan_4t() && i < 2; i += 1)
    {
        prt("\nTest enabling LEDs of front SFP port-%d\n", is_manhattan_4t() ? 2 + i : i);

        ERET_COND(0 > (gpio[i * 2].save = switzer_manhattan_x710_gpio_get(mod, 0, gpio[i * 2].idx, NULL)),
            FAILED, "Failed to get %s\n", gpio[i * 2].name);

        ERET_COND(0 > (gpio[i * 2 + 1].save = switzer_manhattan_x710_gpio_get(mod, 0, gpio[i * 2 + 1].idx, NULL)),
            FAILED, "Failed to get %s\n", gpio[i *2 + 1].name);
        prt("\nSave original status:LED pin %s %s and %s %s\n",
            gpio[i * 2].name, gpio[i * 2].save ? "Off" : "On ",
            gpio[i *2 + 1].name, gpio[i * 2 + 1].save ? "Off" : "On ");

        prt("\n");
        for(onoff = 0; onoff < 4; onoff++) {/* bit-0:amber, bit-2:grn */
            EURET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio[i * 2].idx, !(onoff & 0x1)), /*active low*/
                uret, FAILED, "Failed to set %s %s\n", gpio[i * 2].name, onoff & 0x1 ? "On " : "Off");

            EURET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio[i * 2 + 1].idx, !(onoff & 0x2)), /*active low*/
                uret, FAILED, "Failed to set %s %s\n", gpio[i *2 + 1].name, onoff & 0x2 ? "On " : "Off");
            prt("..%s %s and %s %s\n",
                gpio[i * 2].name, onoff & 0x1 ? "On " : "Off",
                gpio[i * 2 + 1].name, onoff & 0x2 ? "On " : "Off");
            MANHATTAN_LED_TEST_WAIT(2000);
        }
    }

    uret = PASSED;
_EXIT_POINT:
        //restore
        ERET_COND(0 > (switzer_manhattan_x710_gpio_set(mod, 0, gpio[i * 2].idx, gpio[i * 2].save)),
            FAILED, "Failed to set %s\n", gpio[i * 2].name);

        ERET_COND(0 > (switzer_manhattan_x710_gpio_set(mod, 0, gpio[i * 2 + 1].idx, gpio[i * 2 + 1].save)),
            FAILED, "Failed to set %s\n", gpio[i *2 + 1].name);
        prt("\nRestore original status:LED pin %s %s and %s %s\n",
            gpio[i * 2].name, gpio[i * 2].save ? "Off" : "On ",
            gpio[i *2 + 1].name, gpio[i * 2 + 1].save ? "Off" : "On ");
    return uret;
}


static long manhattan_pt_lnk_led_tst(long port)
{
    int      i     = 0;
    int      j     = 0;
    int      uret  = 0;
    int      bitv  = 0;
    int      rstf  = 0;
    int      onoff = 0;
    struct {
        int   idx;
        char *name;
    } gpio[] = {
        {SWITZER_MANHATTAN_LED_PORT0_LNK_GRN, "LED_PORT0_LNK_GRN"}, /* single color */
        {SWITZER_MANHATTAN_LED_PORT1_LNK_GRN, "LED_PORT1_LNK_GRN"}, /* single color */
        {SWITZER_MANHATTAN_LED_PORT2_LNK_GRN, "LED_PORT2_LNK_GRN"}, /* single color */
        {SWITZER_MANHATTAN_LED_PORT3_LNK_GRN, "LED_PORT3_LNK_GRN"}, /* single color */
    };

    //2M: by x710 : PORT0, PORT1
    //4T: by x710 : PORT0, PORT1, PORT2, PORT3
    //1M: by x710 : PORT0
    //2T: by 54194: LED_PORT0B_LNK_GRN_L, LED_PORT1_LNK_GRN_L
    if (is_manhattan_2m() || is_manhattan_4t() || is_manhattan_1m()) {
        i = port;
        prt("\nTest link status LEDs of port-%d\n", is_manhattan_2t() ? port - 2 : i);

        ERET_COND(0 > (bitv = switzer_manhattan_x710_gpio_get(mod, 0, gpio[i].idx, NULL)),
            FAILED, "Failed to get %s\n", gpio[i].name);
        rstf = 1;

        for_each(j, onoff, int, 3, (int)0, (int)1, (int)0) { //off, on, off
            EURET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio[i].idx, !(onoff & 0x1)), /*active low*/
                uret, FAILED, "Failed to set LED pin %s %s\n", gpio[i].name, onoff & 0x1 ? "On" : "Off");
            prt("..%s %s\n", gpio[i].name, onoff & 0x1 ? "On" : "Off");
            MANHATTAN_LED_TEST_WAIT(2000);
        }
    } else if (is_manhattan_2t()) {
        ERET_COND(0 != manhattan_bcm54194_led_test(port), FAILED, "Failed.\n");
    }

    uret = PASSED;
_EXIT_POINT:
    //restore
    if (rstf) {
        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, gpio[i].idx, bitv),
            FAILED, "Failed to set %s to %d\n", gpio[i].name, bitv);
    }
    return uret;
}

static long manhattan_active_led_tst(long port)
{
    int      i     = 0;
    int      j     = 0;
    int      uret  = 0;
    int      rstf  = 0;
    int      onoff = 0;
    uint32_t save_ctrl, ctrl;
    struct {
        int   idx;
        char *name;
    } gpio[] = {
        {SWITZER_MANHATTAN_LED_PORT0_ACTIVE_GRN, "LED_PORT0_ACTIVE_GRN"}, /* single color */
        {SWITZER_MANHATTAN_LED_PORT1_ACTIVE_GRN, "LED_PORT1_ACTIVE_GRN"}, /* single color */
    };

    //1M: by x710 : PORT0
    //2T: by x710 : PORT0 and PORT1
    i = port;
    prt("\nTest Active status LEDs of port-%d\n", i);
    ERET_COND(0 > switzer_manhattan_x710_gpio_ctrl_get(mod, 0, gpio[i].idx, &save_ctrl),
        FAILED, "Failed to get %s gpio_ctrl value\n", gpio[i].name);
    rstf = 1;
    ctrl = save_ctrl;
    for_each(j, onoff, int, 3, (int)0x0, (int)0xF, (int)0x0) { //off, on, off
        I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl, LED_BLINK);
        I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl, LED_MODE);
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0     , LED_BLINK);
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, onoff , LED_MODE);
        EURET_COND(0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, gpio[i].idx, ctrl),
            uret, FAILED, "Failed to set LED pin %s %s\n", gpio[i].name, onoff & 0xF ? "On" : "Off");
        prt("..%s %s\n", gpio[i].name, onoff & 0xF ? "On" : "Off");
        MANHATTAN_LED_TEST_WAIT(2000);
    }

    uret = PASSED;
_EXIT_POINT:
    //restore
    if (rstf) {
        ERET_COND(0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, gpio[i].idx, save_ctrl),
            FAILED, "Failed to set back %s gpio_ctrl value\n", gpio[i].name);
    }
    return uret;
}

static long manhattan_rj45_poe_led_tst(long port)
{
    int      i     = 0;
    int      uret  = 0;
    uint32_t onoff = 0;
    uint8_t  chr   = 0;
    uint8_t  save  = 0;
    n2g_i2c_if_t *pca = switzer_ngio_pca();

    struct {
        int   idx;
        char *name;
    } gpio[] = {
        {SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P, "LED_PORT0_POE_YEL"}, /* Single color led */
        {SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P, "LED_PORT1_POE_YEL"}, /* Single color led */
    };

    pca->i2c_dev = SWITZER_MANHATTAN_I2C_ADDR_PCA2;
    DUMP_PCA9557_REGS(pca, "PCA-2 regs before POE LEDs test:\n");

    //for (i = 0; i < 2; i++)
    i = port;
    {
        prt("\nTest POE status LEDs of port-%d\n", i);

        ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &save, TRUE),
            FAILED, "Failed to read pca9557 reg-1\n");

        /* 00-off, 01-amber, 10-off */
        for(onoff = 0; onoff < 3; onoff++) {
            EURET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr, TRUE),
                uret, FAILED, "Failed to read pca9557 reg-1\n");

            chr &= ~(1 << gpio[i].idx);

            chr |= (!(onoff & 0x1) << gpio[i].idx);

            EURET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr),
                uret, FAILED, "Failed to write pca9557 reg-1\n");

            prt("..Port-%d POE LED %s\n", i,
                onoff == 0 ? "Off"   :
                onoff == 1 ? "Amber" :
                onoff == 2 ? "Off"   : "Unknown");
            MANHATTAN_LED_TEST_WAIT(2000);
        }
    }

    uret = PASSED;
_EXIT_POINT:
    EURET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &save),
        uret, FAILED, "Failed to write pca9557 reg-1\n");
    DUMP_PCA9557_REGS(pca, "PCA-2 regs after  POE LEDs test:\n");
    return uret;
}

static long __show_lnk_led_tst_p0(void) {
    return (is_manhattan_2m() || is_manhattan_4t() || is_manhattan_1m());
}

static long __show_lnk_led_tst_p1(void) {
    return (is_manhattan_2m() || is_manhattan_4t());
}

static long __show_poe_led_tst_p0(void) {
    return (is_manhattan_4t() || is_manhattan_2m() || is_manhattan_1m());
}

static long __show_poe_led_tst_p1(void) {
    return (is_manhattan_4t() || is_manhattan_2m());
}

static long __show_active_led_tst_p0(void) {
    return (is_manhattan_2t() || is_manhattan_1m());
}

static long __show_active_led_tst_p1(void) {
    return (is_manhattan_2t());
}

// static long __show_enb_led_tst_p2(void) {
//     return (is_manhattan_4t() || is_manhattan_2t()); //TODO: schema of 2T has this, confirm with HW ??
// }

// static long __show_enb_led_tst_p3(void) {
//     return (is_manhattan_4t() || is_manhattan_2t()); //TODO: schema of 2T has this, confirm with HW ??
// }
/* LED submenu items */
static submenu_xtable_t led_submenu_table[] = {
    {"Power        LED Test"         , manhattan_pwr_led_tst       , 0, F_ALL_E, NULL                 , 0, NULL, 0},
    {"Port0 Link   LED Test"         , manhattan_pt_lnk_led_tst    , 0, F_ALL_E, __show_lnk_led_tst_p0, 0, NULL, 0},
    {"Port1 Link   LED Test"         , manhattan_pt_lnk_led_tst    , 1, F_ALL_E, __show_lnk_led_tst_p1, 0, NULL, 0},
    {"Port2 Link   LED Test"         , manhattan_pt_lnk_led_tst    , 2, F_ALL_E, is_manhattan_4t      , 0, NULL, 0},
    {"Port3 Link   LED Test"         , manhattan_pt_lnk_led_tst    , 3, F_ALL_E, is_manhattan_4t      , 0, NULL, 0},
    {"Port2 Enable LED Test"         , manhattan_sfp_pt_enb_led_tst, 2, F_ALL_E, is_manhattan_4t      , 0, NULL, 0},
    {"Port3 Enable LED Test"         , manhattan_sfp_pt_enb_led_tst, 3, F_ALL_E, is_manhattan_4t      , 0, NULL, 0},

    {"RJ45 Port0 Link LED Test"      , manhattan_pt_lnk_led_tst    , 0, F_ALL_E, is_manhattan_2t      , 0, NULL, 0},
    {"RJ45 Port1 Link LED Test"      , manhattan_pt_lnk_led_tst    , 1, F_ALL_E, is_manhattan_2t      , 0, NULL, 0},
    {"SFP  Port0 Link LED Test"      , manhattan_sfp_pt_enb_led_tst, 2, F_ALL_E, is_manhattan_2t      , 0, NULL, 0},
    {"SFP  Port1 Link LED Test"      , manhattan_sfp_pt_enb_led_tst, 3, F_ALL_E, is_manhattan_2t      , 0, NULL, 0},

    {"Port0 POE    LED Test"         , manhattan_rj45_poe_led_tst  , 0, F_ALL_E, __show_poe_led_tst_p0, 0, NULL, 0},
    {"Port1 POE    LED Test"         , manhattan_rj45_poe_led_tst  , 1, F_ALL_E, __show_poe_led_tst_p1, 0, NULL, 0},

    {"Port0 Actice LED Test"         , manhattan_active_led_tst    , 0, F_ALL_E, __show_active_led_tst_p0, 0, NULL, 0},
    {"Port1 Actice LED Test"         , manhattan_active_led_tst    , 1, F_ALL_E, __show_active_led_tst_p1, 0, NULL, 0},
};

#define LED_SUBMENU_TABLE_SZ (sizeof(led_submenu_table) / sizeof(submenu_xtable_t))
static mitem_t led_submenu_primary_items[LED_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t led_submenu_secondary_items[LED_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char led_submenu_title[] = "Switzer Manhattan LED Subset Menu";

static menuinfo_t led_submenu = {
    led_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_submenu_primary_items,
};

static menuinfo_t *led_submenup = &led_submenu;

static long leds_test(int show_menu)
{
    build_primary_submenu(led_submenu_table, LED_SUBMENU_TABLE_SZ,
                          led_submenu_title, &led_submenup);
    build_secondary_submenu(led_submenu_table, LED_SUBMENU_TABLE_SZ,
                            led_submenu_secondary_items);
    if (show_menu)
        menu(led_submenup, led_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(led_submenup);
    return PASSED;
}


static long _gpio_tst_pri_if_rdy(void)
{
    int i = 0;
    unsigned char chr = 0;
    n2g_i2c_if_t *pca = switzer_ngio_pca();

    prt("\nTest GPIO PRI_IF_RDY\n");

    pca->i2c_dev = SWITZER_MANHATTAN_I2C_ADDR_PCA1;
    for(i = 0; i < 2; i++) {
        prt("Set GPIO PRI_IF_RDY to %d\n", i);
        ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, SWITZER_MANHATTAN_PRI_IF_RDY, i),
            FAILED, "Failed to set x710 gpio %d\n", SWITZER_MANHATTAN_PRI_IF_RDY);
        switzer_mdelay(50);

        ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_IN_REG, &chr, TRUE),
            FAILED, "Failed to read pca9557 reg-0\n");

        ERET_COND(!!(chr & (1 << SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_P)) != i,
            FAILED, "Failed test x710 gpio %d\n", SWITZER_MANHATTAN_PRI_IF_RDY);
    }
    prt("..OK\n");
    return PASSED;
}

static long _gpio_tst_extphy_gpio(void)
{
    int i = 0;
    int j = 0;
    uint32_t data = 0;
    const uint32_t if_side = SWITZER_IF_SIDE_LINE;
    const uint32_t devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    const uint32_t lane    = 0;
    struct reg_pair {
        uint32_t x710_gpio;
        char     *name;
        uint32_t bcm_ctl;
        uint32_t bcm_st;
        uint32_t bcm_ctl_save;
    } pair[] = {
        {
            SWITZER_MANHATTAN_EXTPHY_GPIO0_0,
            "EXTPHY_GPIO0_0",
            BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_0_CONTROLr,
            BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_0_STATUSr,
            0
        },
        {
            SWITZER_MANHATTAN_EXTPHY_GPIO0_1,
            "EXTPHY_GPIO0_1",
            BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_1_CONTROLr,
            BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_1_STATUSr,
            0
        },
    };

    if (is_manhattan_4t()) {
        for(i = 0; i < sizeof(pair)/sizeof(struct reg_pair); i++) {
            prt("\nTest GPIO %s\n", pair[i].name);
            ERET_COND(0 != switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, pair[i].bcm_ctl, &data),
                FAILED, "Faild to read BCM82757 reg-%d\n", pair[i].bcm_ctl);
            pair[i].bcm_ctl_save = data;

            data = 0x40e5; // Input
            ERET_COND(0 != switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, pair[i].bcm_ctl, data),
                FAILED, "Faild to write BCM82757 reg-%d\n", pair[i].bcm_ctl);

            for(j = 0; j < 2; j++) {
                ERET_COND(0 > switzer_manhattan_x710_gpio_set(mod, 0, pair[i].x710_gpio, j),
                    FAILED, "Failed to set x710 gpio %d\n", pair[i].x710_gpio);
                switzer_mdelay(5);

                ERET_COND(0 != switzer_manhattan_ephy_read(mod, lane, if_side, devaddr, pair[i].bcm_st, &data),
                    FAILED, "Faild to read BCM82757 reg-%d\n", pair[i].bcm_st);

                ERET_COND(!!(0x4 & data) != j, FAILED, "Failed to test x710 gpio %d\n", pair[i].x710_gpio);
            }

            data = pair[i].bcm_ctl_save; //restore
            ERET_COND(0 != switzer_manhattan_ephy_write(mod, lane, if_side, devaddr, pair[i].bcm_ctl, data),
                FAILED, "Faild to write BCM82757 reg-%d\n", pair[i].bcm_ctl);
            prt("..OK\n");
        }
    }
    return PASSED;
}

static long _gpio_tst_sfp_p3_dsl_dygp(void)
{
    //ontly 4T, not 2M, 1M and 2T
    prt("\nTest GPIO SFP_P3_DSL_DYGP\n");
    prt("..TODO\n");
    return PASSED;
}

static long _gpio_tst_pse_intb(void)
{
    prt("\nTest GPIO PSE_INTB\n");
    prt("..TODO\n");
    return PASSED;
}

static long gpios_test(void)
{
    char *test_name = "GPIOs Test";

    //SWITZER_MANHATTAN_PRI_IF_RDY
    ERET_COND(PASSED != _gpio_tst_pri_if_rdy(), FAILED, "%s FAILED\n", test_name);

    //SWITZER_MANHATTAN_SFP_P3_DSL_DYGP
    ERET_COND(PASSED != _gpio_tst_sfp_p3_dsl_dygp(), FAILED, "%s FAILED\n", test_name);

    //SWITZER_MANHATTAN_PSE_INTB
    ERET_COND(PASSED != _gpio_tst_pse_intb(), FAILED, "%s FAILED\n", test_name);

    //SWITZER_MANHATTAN_EXTPHY_GPIO0_0
    //SWITZER_MANHATTAN_EXTPHY_GPIO0_1
    ERET_COND(PASSED != _gpio_tst_extphy_gpio(), FAILED, "%s FAILED\n", test_name);

    prpass(testpass, "GPIOs Test ");

    return PASSED;
}

static long __show_rj45_lpbk_p0(void)
{
    return (is_manhattan_2m() || is_manhattan_4t() || is_manhattan_1m());
}
static long __show_rj45_lpbk_p1(void)
{
    return (is_manhattan_2m() || is_manhattan_4t());
}
/* PHY submenu items */
static submenu_xtable_t loopback_submenu_table[] = {
    /* Internal PHY port */
    {"Port0 Loopback Test"            , rj45_lpbk_test     , 0             ,
     F_ALL_E                          , __show_rj45_lpbk_p0, 0             , NULL, 0},

    /* Internal PHY port */
    {"Port1 Loopback Test"            , rj45_lpbk_test     , 1             ,
     F_ALL_E                          , __show_rj45_lpbk_p1, 0             , NULL, 0},

    /* External PHY(82757) SFP port */
    {"Port2 Loopback Test"            , ext_phy_lpbk_test  , SWITZER_LANE_0,
     F_ALL_E                          , is_manhattan_4t    , 0             , NULL, 0},

    /* External PHY(82757) SFP port */
    {"Port3 Loopback Test"            , ext_phy_lpbk_test  , SWITZER_LANE_1,
     F_ALL_E                          , is_manhattan_4t    , 0             , NULL, 0},
};

#define LOOPBACK_SUBMENU_TABLE_SZ (sizeof(loopback_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t loopback_submenu_primary_items[LOOPBACK_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t loopback_submenu_secondary_items[LOOPBACK_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char loopback_submenu_title[] = "Switzer Manhattan Loopback Subtest Menu";

static menuinfo_t loopback_submenu = {
    loopback_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    loopback_submenu_primary_items,
};

static menuinfo_t *loopback_submenup = &loopback_submenu;

static long loopback_test(int show_menu)
{
    build_primary_submenu(loopback_submenu_table, LOOPBACK_SUBMENU_TABLE_SZ,
                          loopback_submenu_title, &loopback_submenup);
    build_secondary_submenu(loopback_submenu_table, LOOPBACK_SUBMENU_TABLE_SZ,
                            loopback_submenu_secondary_items);
    if (show_menu)
        menu(loopback_submenup, loopback_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(loopback_submenup);
    return PASSED;
}

/*
 * User must make sure buf of params are sufficient to accomodate the strings
 */
static long misc_info_710(int verbose, char *drv_name, char *drv_ver, char *fw_ver)
{
    FILE *fp = NULL;
    char  buf[256]= {0,};
    char *p  = NULL;

    snprintf(buf, sizeof(buf), "ethtool -i %s", mod->eth_port.intnl_port[0]);
    ERET_COND(!(fp = popen(buf, "r")), -1, "Failed to run cmd '%s'\n", buf);

    #define _GET_KEY_VAL(KEY, KEY_BUF)                                   \
        if (strncmp(buf, KEY, sizeof(KEY) - 1) == 0) {                   \
            if (KEY_BUF) {                                               \
                for (p = &buf[sizeof(KEY) - 1]; *p && isspace(*p); p++); \
                strcpy(KEY_BUF, p);                                      \
            }                                                            \
            if (verbose)                                                 \
                prt("%s\n", buf);                                        \
            memset(buf, 0, sizeof(buf));                                 \
            continue;                                                    \
        }

    while ((fgets(buf, sizeof(buf), fp))) {
        buf[strlen(buf) - 1] = 0;
        _GET_KEY_VAL("driver:", drv_name);
        _GET_KEY_VAL("version:", drv_ver);
        _GET_KEY_VAL("firmware-version:", fw_ver);
        memset(buf, 0, sizeof(buf));
    }
    pclose(fp);
    return PASSED;
    #undef _GET_KEY_VAL
}

static long misc_info_710_temp(int verbose, double *temp)
{
    const char  *cmd  = "0x0 0x721 0x0 0x0 0x0 0x0 0x2 0x0 0x0 0x0 0x0";
    const char rkey[] = "AQ desc WB";
    char    buf[1024] = {0,};
    char        *p    = NULL;
    unsigned int ret  = 0;
    unsigned int code = 0;
    unsigned int val  = 0;

    ERET_COND(PASSED != x710_aq_cmd_send(0, 0, cmd, buf, sizeof(buf)), FAILED, "Failed.\n");

    /* eno3: AQ desc WB 0x0003 0x0721 0x0000 0x0000 0x00000000 0x00000000 0x0000003e 0x00000000 0x00000000 0x00000000 */
    if ((p = strstr(buf, rkey))) {
        sscanf(p + sizeof(rkey), "%x %x %*x %*x %*x %*x %x", &ret, &code, &val);
    }
    ERET_COND((ret != 0x3 || code != 0x721), FAILED, "Failed to read X710 temperature, response:\n%s\n", buf);

    if (verbose)
        prt("X710  Temperature :%d Celsius Degree\n", val);

    if (temp)
        *temp = val;

    return PASSED;
}

static long misc_info_extphy_temp(int verbose, double *temp)
{
    uint32_t val = 0;
    double _temp = 0.0;

    /* Broadcom case number: CS00011030348
     * Temp = 413.35 - (o_ADC_data * 0.49055)
     * o_ADC_data = Bits 9:0 of 0x8126
     */
    switzer_manhattan_ephy_read(mod, 0, 1, SWITZER_MIURA_DEV_PMA_PMD, 0x8126, &val);
    _temp = 413.35 - 0.49055 * (val & ((1 << 10) - 1));
    if (verbose)
        prt("82757 Temperature:%.2f Celsius Degree\n", _temp);
    if (temp)
        *temp = _temp;
    return 0;
}

static long misc_info_sfp_temp(int verbose, int port, double *temp)
{
    /*  EDCS-275976:
        Transceiver temperature: Temperature, T (deg-C), is given by
        T = T1 * TAD + T0
        Where TAD is 16-bit signed 2's complement A/D value at bytes 96-97, T1 is unsigned
        fixed-point value at bytes 84-85 and T0 is signed 2's complement value with LSB equal
        to 1/256 deg-C at bytes 86-87. The result, T, is 16-bit signed 2's complement value with
        LSB equal to 1/256 deg-C.
    */

    int      ext_algo = 0;
    uint8_t  buf[128] = {0,};
    uint16_t tad = 0;
    uint16_t t0  = 0;

    int      TAD = 0;
    int16_t  T0  = 0;
    uint16_t T1  = 0;
    int      T   = 0;
    double   Tf  = 0.0;

    //TODO: check sfp compilance and presences

    //1, check if use external algorithm
    ERET_COND(0 > switzer_manhattan_sfp_read(mod, port, SWITZER_MANHATTAN_I2C_ADDR_SFP, 0, buf, sizeof(buf)),
        FAILED, "Read SPF-%d@%#X Failed.\n", port, SWITZER_MANHATTAN_I2C_ADDR_SFP);
    if (buf[92] & 0x10)
        ext_algo = 1;

    //2, calc temperature
    ERET_COND(0 > switzer_manhattan_sfp_read(mod, port, SWITZER_MANHATTAN_I2C_ADDR_SFP_MEASURES, 0, buf, sizeof(buf)),
        FAILED, "Read SFP-%d@%#X Failed.\n", port, SWITZER_MANHATTAN_I2C_ADDR_SFP_MEASURES);

    //2.1, check if module power up
    ERET_COND(buf[110] & 0x1, FAILED, "Module is not power ready.\n");

    //2.2, calc
    if (ext_algo == 0) {
        T = buf[96];
        if (T & 0x8000) {
            T = 0x8000 | (~(T & (~0x8000)) + 1);
        } else {
            T = T;
        }
        Tf = T + (1.0 * buf[97]) / 256;
    }
    else {
        tad = (buf[96] << 8) | buf[97];
        T1  = (buf[84] << 8) | buf[85];
        t0  = (buf[86] << 8) | buf[87];

        if (t0 & 0x8000) {
            T0 = 0x8000 | (~(t0 & (~0x8000)) + 1);
        } else {
            T0 = t0;
        }

        if (tad & 0x8000) {
            TAD = 0x8000 | (~(tad & (~0x8000)) + 1);
        } else {
            TAD = tad;
        }

        Tf = 1.0 * (T1 * TAD + (T0 * 1.0) / 256) / 256;
    }

    if (verbose)
        prt("SFP-%d:%.2f Celcius Degree\n", port, Tf);

    if (*temp)
        *temp = Tf;

    return 0;
}

static long misc_info(void)
{
    double temp      = 0;
    char drv[64]     = {0,};
    char drv_ver[64] = {0,};
    char fw_ver[64]  = {0,};

    prt("%-20s : %s\n", "NIM ID", __manhattan_board_name());

    misc_info_710(0, drv, drv_ver, fw_ver);
    prt("%-20s : %s %s\n"
        "%-20s : %s\n",
        "X710 Driver", drv, drv_ver,
        "X710 FW Ver", fw_ver);

    ERET_COND(misc_info_710_temp(0, &temp), FAILED, "Failed\n");
    prt("%-20s : %.2f Celsius Degree\n", "X710 Temperature", temp);

    if (is_manhattan_4t()) {
        ERET_COND(misc_info_extphy_temp(0, &temp), FAILED, "Failed.\n");
        prt("%-20s : %.2f Celsius Degree\n", "82757 Temperature", temp);
        ERET_COND(misc_info_sfp_temp(0, 0, &temp), FAILED, "Failed.\n");
        prt("%-20s : %.2f Celsius Degree\n", "SFP0  Temperature", temp);
        ERET_COND(misc_info_sfp_temp(0, 1, &temp), FAILED, "Failed.\n");
        prt("%-20s : %.2f Celsius Degree\n", "SFP1  Temperature", temp);
    }

    if (is_manhattan_2t()) {
        prt("TODO\n");
    }

    return 0;
}

static long __show_poe_test(void)
{
    return is_manhattan_init_okay() && __is_poe_available();
}

static long __show_ds4424_test(void)
{
    //return is_manhattan_init_okay() && is_manhattan_1m();
    return FALSE;
}

/* Main menu items */
static submenu_xtable_t main_menu_table[] = {
    {"IO Utilities"         , io_utils                     , 0,
     0                      , is_manhattan_init_okay       , 0, io_utils     , 0},
    {"Power Utilities"      , power_utils                  , 0,
     0                      , is_manhattan_init_okay       , 0, power_utils  , 0},
    {"LED Utilities"        , led_utils                    , 0,
     0                      , is_manhattan_init_okay       , 0, led_utils    , 0},
    {"PHY Test"             , phy_test                     , 0,
     F_ALL_E                , is_manhattan_init_okay       , 0, phy_test     , 1},
    {"LTC4215 Register Test", switzer_ltc4215_reg_test     , 0,
     F_ALL_E                , is_manhattan_init_okay       , 0, NULL         , 0},
    {"DS4424 Register Test" , ds4424_reg_test              , 0,
     F_ALL_E                , __show_ds4424_test           , 0, NULL         , 0},
    {"LED Test"             , leds_test                    , 0,
     F_ALL_E                , is_manhattan_init_okay       , 0, leds_test    , 1},
    {"GPIO Test"            , gpios_test                   , 0,
     F_ALL_E                , is_manhattan_init_okay       , 0, NULL         , 0},
    {"Loopback Test"        , loopback_test                , 0,
     F_ALL_E                , is_manhattan_init_okay       , 0, loopback_test, 1},

    {"X710 NVM Upgrade"     , switzer_manhattan_nvm_upgrade, 0,
     0                      , is_manhattan_init_fail       , 1, NULL         , 0},

    {"X710 MAC Program"     , switzer_manhattan_mac_program, 0,
     0                      , is_manhattan_init_fail       , 1, NULL         , 0},
    {"UPOE Test"            , upoe_test                    , 0,
     0                      , __show_poe_test              , 0, upoe_test    , 1},

    {"Misc Info Utility"    , misc_info                    , 0,
     0                      , is_manhattan_init_okay       , 0, NULL         , 0},
};

#define MAIN_MENU_TABLE_SIZE                                \
    (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static menuinfo_t maindiag = {
    "Switzer Manhattan Main Menu",	/* title */
    0,                              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,          /* shows major flags */
    0,                              /* generic prompt */
    0,                              /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static menuinfo_t *maindiagp = &maindiag;

static int switzer_manhattan_iface_test(struct ngio_intf_t *ngio)
{
    if (switzer_ltc4215_reg_test())
        return FAILED;

   // if (is_manhattan_1m()) {
   //     if (ds4424_reg_test())
   //         return FAILED;
   // }

    if (gpios_test())
        return FAILED;

    if (phy_test(0))
        return FAILED;

    if (loopback_test(0))
        return FAILED;

    return PASSED;
}

static int switzer_manhattan_open_i2c_mux(struct switzer_manhattan *mod, int onoff)
{
    uint8_t cmd   = 0x01;
    size_t  count = 1;
    char    hex   = onoff ? 1 : 0;

    if (switzer_dash_i2c_slave_write(mod->i2c_mux.i2c, cmd, &hex, count) < 0) {
        cterr('f',0,"I2C write failed @ %#x\n", cmd);
        return FAILED;
    }

    return PASSED;
}


static int _pca9557_1_init(struct switzer_manhattan *mod, n2g_i2c_if_t *pca)
{
    uint8_t chr = 0;

    pca->i2c_dev = SWITZER_MANHATTAN_I2C_ADDR_PCA1;

    // clear inversion: no inversion
    ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_POL_REG, &chr, TRUE),
        FAILED, "Failed to read pca9557 reg-2\n");
    chr = (chr & ~(1 << SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_P));
    ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_POL_REG, &chr),
        FAILED, "Failed to write pca9557 reg-2\n");

    // set to input
    ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_CTL_REG, &chr, TRUE),
        FAILED, "Failed to read pca9557 reg-3\n");
    chr = (chr & (~(1 << SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_P))) |
          ((!SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_D) << SWITZER_MANHATTAN_PCA1_PRI_IF_RDY_P);
    ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_CTL_REG, &chr),
        FAILED, "Failed to write pca9557 reg-3\n");
    return PASSED;
}

static int _pca9557_2_init(struct switzer_manhattan *mod, n2g_i2c_if_t *pca)
{
    uint8_t chr = 0;

    pca->i2c_dev = SWITZER_MANHATTAN_I2C_ADDR_PCA2;

    DUMP_PCA9557_REGS(pca, "PCA-2 Regs before init(%s):\n", __func__);

    // SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P
    // SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P
    // SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P
    // SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P
    // SWITZER_MANHATTAN_PCA2_PSE_RST_P
    // SWITZER_MANHATTAN_PCA2_PSE_OSS_P

    //1, clear inversion: no inversion
    ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_POL_REG, &chr, TRUE),
        FAILED, "Failed to read pca9557 reg-2\n");

    chr = (chr & ~(((1 << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PSE_RST_P))           | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PSE_OSS_P))));

    ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_POL_REG, &chr),
        FAILED, "Failed to write pca9557 reg-2\n");

    //2, set default value( before changing the direction to output in case output impacts )
    ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr, TRUE),
        FAILED, "Failed to read pca9557 reg-2\n");
    chr = (chr | (((1 << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P)) | \
                  ((1 << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P)) | \
                  ((1 << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P)) | \
                  ((1 << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P)) | \
                  ((1 << SWITZER_MANHATTAN_PCA2_PSE_RST_P          )) | \
                  ((1 << SWITZER_MANHATTAN_PCA2_PSE_OSS_P))));
    ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_OUT_REG, &chr),
        FAILED, "Failed to write pca9557 reg-3\n");

    //3, set to input/output respectively
    ERET_COND(FAILED == io_port_8bit_i2c_read(pca, SWITZER_MANHATTAN_PCA9557_CTL_REG, &chr, TRUE),
        FAILED, "Failed to read pca9557 reg-3\n");
    chr = ~0; //set to input default to all
    chr = (chr & ~(((1 << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P)) | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PSE_RST_P))           | \
                   ((1 << SWITZER_MANHATTAN_PCA2_PSE_OSS_P))));

    chr = (chr | (((!SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_D) << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_YEL_P) |\
                  ((!SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_D) << SWITZER_MANHATTAN_PCA2_PORT0_LED_POE_GRN_P) |\
                  ((!SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_D) << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_YEL_P) |\
                  ((!SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_D) << SWITZER_MANHATTAN_PCA2_PORT1_LED_POE_GRN_P) |\
                  ((!SWITZER_MANHATTAN_PCA2_PSE_RST_D          ) << SWITZER_MANHATTAN_PCA2_PSE_RST_P          ) |\
                  ((!SWITZER_MANHATTAN_PCA2_PSE_OSS_D          ) << SWITZER_MANHATTAN_PCA2_PSE_OSS_P          )));

    ERET_COND(FAILED == io_port_8bit_i2c_write(pca, SWITZER_MANHATTAN_PCA9557_CTL_REG, &chr),
        FAILED, "Failed to write pca9557 reg-3\n");

    DUMP_PCA9557_REGS(pca, "PCA-2 Regs after init(%s):\n", __func__);

    return PASSED;
}

static int switzer_manhattan_pca9557_init(struct switzer_manhattan *mod)
{
    n2g_i2c_if_t *pca = switzer_ngio_pca();
    pca->dev_name = "PCA9557";

    ERET_COND(PASSED != _pca9557_1_init(mod, pca), FAILED, "Failed");
    /* No PoE chip on 2T */
    if (is_manhattan_1m() || is_manhattan_2m())
        ERET_COND(PASSED != _pca9557_2_init(mod, pca), FAILED, "Failed");

    return PASSED;
}

static int switzer_manhattan_platform_init(struct switzer_manhattan *mod)
{
    switzer_manhattan_stage_set(mod, SWITZER_MANHATTAN_STAGE_INIT_0);
    init_mdio_dbg_flag_check();

    ERET_COND(0 != switzer_manhattan_pca9557_init(mod), FAILED, "Failed to init pca9557.\n");
    return 0;
}

static void switzer_manhattan_eth_up(char *port)
{
    char cmd[64] = {0,};
    sprintf(cmd, "ifconfig %s promisc up", port);
    system(cmd);
}

static void switzer_manhattan_eth_down(char *port)
{
    char cmd[64] = {0,};
    sprintf(cmd, "ifconfig %s promisc down", port);
    system(cmd);
}

static int switzer_manhattan_eth_init(struct switzer_manhattan *mod)
{
    int i;
    int port_num = 0;

    port_num = is_manhattan_2m() ? SWITZER_MANHATTAN_IPORT_NUM     :
               is_manhattan_1m() ? SWITZER_MANHATTAN_IPORT_NUM - 1 :
               is_manhattan_2t() ? 0                               :
               is_manhattan_4t() ? SWITZER_MANHATTAN_IPORT_NUM     : 0;
    for (i = 0; i < port_num; i++) {
        switzer_manhattan_eth_up(mod->eth_port.intnl_port[i]);
    }

    port_num = is_manhattan_2m() ? 0                           :
               is_manhattan_1m() ? 0                           :
               is_manhattan_2t() ? SWITZER_MANHATTAN_EPORT_NUM :
               is_manhattan_4t() ? SWITZER_MANHATTAN_EPORT_NUM : 0;
    for (i = 0; i < SWITZER_MAX_PORTS_OF_EACH_PAIR; i++) {
        switzer_manhattan_eth_up(mod->eth_port.extnl_port[i]);
    }

    return PASSED;
}

static void switzer_manhattan_platform_exit(struct switzer_manhattan *mod)
{
}

static int switzer_manhattan_device_init(struct switzer_manhattan *mod)
{
    struct ngio_intf_t *ngio = switzer_ngio();
    struct switzer_settings settings = {
        .pci_domain = 0,
        .pci_bus = get_ngio_pcie_dev_bus_num(ngio->mod_type, ngio->slot),
        .pci_dev = 0,
        .pci_func = 0,
    };

    /* Workaround for Switzer-Manhattan RDT eeprom check failed and pcie init failure:
     * V710 need to add more delay between ngio_pwr_en and ngio_unreset.
     */
    switzer_mdelay(70000); /* Arbitrary */
    printf("Add 70s delay before ngio unreset for Switzer-Manhattan\n");

    ngio->unreset(ngio);

    /* PCI ready is used to trigger a hotplug event, it should be invoked */
    /* before any PCI operation */
    ngio->pci_rdy(ngio, 1);

    system("modprobe -r i40e");
    switzer_mdelay(5000); /* Arbitrary */

    system("depmod -a; sleep 1; modprobe i40e");
    prt("Sleep 10s to probe i40e.ko...\n");
    switzer_mdelay(10000); /* Arbitrary */

    if (switzer_manhattan_init(mod, &settings))
        return -1;

    switzer_manhattan_eth_init(mod);

    mod->ngio = ngio;

    if (is_manhattan_1m()) {
        if (switzer_manhattan_open_i2c_mux(mod, 1))
            log_warn("switzer_manhattan_open_i2c_mux failed.\n");
    }

    /* turn on the green light */
    if (util_oir_ltc4215_led(ngio->oir, OIR_LED_GREEN_ONLY))
        log_warn("util_oir_ltc4215_led failed.\n");

    /* check if POE is available and update the flag */
    __is_poe_available();

    return 0;
}

static int switzer_manhattan_device_unbind_rmv(uint16_t dom, uint8_t bus, uint8_t dev, uint8_t func)
{
    //return system("modprobe -r i40e");
    return 0;
}

static void switzer_manhattan_device_exit(struct switzer_manhattan *mod)
{
    switzer_manhattan_exit(mod);
    if (mod->ngio) {
        switzer_manhattan_device_unbind_rmv(0,
            get_ngio_pcie_dev_bus_num(mod->ngio->mod_type, mod->ngio->slot), 0, 0);
    }
    mod->ngio = NULL;
}

static int switzer_manhattan_test(struct ngio_intf_t *ngio)
{
    int rc = FAILED;

    if (switzer_manhattan_platform_init(mod))
        goto err0;

    if (switzer_manhattan_device_init(mod)) {
        mod->ngio = ngio;
        build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr, &maindiagp);
        build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, main_menu_secondary_items);
        menu(maindiagp, main_menu_secondary_items, '\0');
        rc = PASSED; //for upperlayer to turn off NIM
        goto err1;
    }

    switzer_manhattan_stage_set(mod, SWITZER_MANHATTAN_STAGE_INIT_DONE);

    rc = PASSED;
    /* Display and interact with user until <ESC><RET> back to main menu */
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    if (ngio->test_type == IFACE_TEST) {
        rc = switzer_manhattan_iface_test(ngio);
    } else if (ngio->menu_display == FALSE) {
        do_all_menu_items(maindiagp);
    } else {
        menu(maindiagp, main_menu_secondary_items, '\0');
    }

    switzer_manhattan_device_exit(mod);
err1:
    switzer_manhattan_platform_exit(mod);
err0:
    return rc;
}

int switzer_manhattan_2m_test(struct ngio_intf_t *ngio)
{
    memset(mod, 0, sizeof(struct switzer_manhattan));
    mod->manhattan_type = SWITZER_MANHATTAN_2M;
    return switzer_manhattan_test(ngio);
}

int switzer_manhattan_4t_test(struct ngio_intf_t *ngio)
{
    memset(mod, 0, sizeof(struct switzer_manhattan));
    mod->manhattan_type = SWITZER_MANHATTAN_4T;
    init_actual_4t_check();
    return switzer_manhattan_test(ngio);
}

int switzer_manhattan_1m_test(struct ngio_intf_t *ngio)
{
    memset(mod, 0, sizeof(struct switzer_manhattan));
    mod->manhattan_type = SWITZER_MANHATTAN_1M;
    return switzer_manhattan_test(ngio);
}

int switzer_manhattan_2t_test(struct ngio_intf_t *ngio)
{
    memset(mod, 0, sizeof(struct switzer_manhattan));
    mod->manhattan_type = SWITZER_MANHATTAN_2T;
    return switzer_manhattan_test(ngio);
}

int switzer_manhattan_sock_test(char *eth_port_s, char *eth_port_r)
{
    struct switzer_eth_traf_tx_task_settings tx_settings;
    struct switzer_eth_traf_rx_task_settings rx_settings;
    tx_settings.mode = SWITZER_ETH_TRAF_TX_MODE_RADOM_VIRTUAL_MAC;
    tx_settings.check = SWITZER_ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_settings.len = 150;
    tx_settings.burst = 1;
    tx_settings.duration = 500;
    rx_settings.chk_mode = SWITZER_ETH_TRAF_RX_MODE_CHECK_BIT;

    switzer_manhattan_eth_down(eth_port_s);
    msleep(1000);
    switzer_manhattan_eth_up(eth_port_s);
    if (0 != _iflink_wait(eth_port_s, MANHATTAN_LINK_UP  , 10000)) {
        log_err("Wait %s link up timeout\n", eth_port_s);
        return FAILED;
    }

    if (strcmp(eth_port_s, eth_port_r) != 0) {
        switzer_manhattan_eth_down(eth_port_r);
        msleep(1000);
        switzer_manhattan_eth_up(eth_port_r);
        if (0 != _iflink_wait(eth_port_r, MANHATTAN_LINK_UP  , 10000)) {
            log_err("Wait %s link up timeout\n", eth_port_r);
            return FAILED;
        }
    }

    printf("Start traffic\n");
    if (switzer_eth_traf_util_test(eth_port_s, eth_port_r, &tx_settings, &rx_settings, 5)) {
        return FAILED;
    }
    return PASSED;
}

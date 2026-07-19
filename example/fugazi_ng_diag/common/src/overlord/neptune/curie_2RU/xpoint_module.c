/* $Id: xpoint_module.c,v 1.1 2020/01/09 01:02:07 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/xpoint_module.c,v $
 *------------------------------------------------------------------
 *
 * xpoint_module.c - Crosspoint SM module side test
 *
 * Dec. 2018, Xin Jin <xinjin2@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdarg.h>
#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "error.h"
#include "menu.h"
#include "slot.h"
#include <assert.h>
#include "i2c_api.h"
#include "platform_i2c.h"
#include "slot.h"
#include "xpoint_module.h"
#include "ngio_testcard.h"
#include "testcard_tlk_10232.h"
#include "queryflags.h"
#include "cookie_4.h"

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

#define MIN_PRBS_DURATION     3
#define MAX_PRBS_DURATION     86400
#define DEFAULT_PRBS_DURATION 10

#define DEFAULT_MODE TLK10232_MODE_10GKR_NO_AN
#define DEFAULT_PRBS TLK10232_PRBS_PATTERN_31

typedef enum _MODULE_TYPE {
    MODULE_WIC,
    MODULE_SM,
} MODULE_TYPE_E;

typedef enum _CARD_TYPE {
    TEST_CARD,
    UNKNOWN_CARD
} CARD_TYPE_E;

typedef struct _module_info {
    char *name;
    int slot;
    MODULE_TYPE_E t;
    CARD_TYPE_E card_type;
    int tlk_channel_addr;
    ngio_if *ngio;
    n2g_i2c_if_t i2c_if;
    TLK10232_PRBS_PATTERN_TYPE_E prbs_pattern;
    char *prbs_name;
} module_info_t;

typedef struct _xpoint_module_info {
    TLK10232_MODE_TYPE_E mode_type;
    char *mode_name;
    int prbs_duration;
    module_info_t module_info[2];
} xpoint_module_info_t;

extern int slot_get_info(struct ngio_intf_t *ngio, char*);

static char *s_mode_name[TLK10232_MODE_NUMBER] = {"1G-KX", "10G-KR-NO-AN", "10G-KR"};
static char *s_prbs_name[TLK10232_PRBS_PATTERN_NUMBER] = {"PRBS-31", "PRBS-23", "PRBS-7"};

static xpoint_module_info_t xpoint_module_info = {
    .mode_type = DEFAULT_MODE,
    .mode_name = NULL,
    .prbs_duration = DEFAULT_PRBS_DURATION,
    .module_info = {
        {"Unknown Card",
         1, MODULE_SM,
         UNKNOWN_CARD, TC_TLK10232_CHB_ADDR, NULL,
         {0}, DEFAULT_PRBS, NULL},
        {"Unknown Card",
         2, MODULE_SM,
         UNKNOWN_CARD, TC_TLK10232_CHB_ADDR, NULL,
         {0}, DEFAULT_PRBS, NULL},
    }
};

static int init_i2c(n2g_i2c_if_t *i2c_if, MODULE_TYPE_E module_type, int slot)
{
    int rc;

    switch (module_type) {
    case MODULE_WIC:
        i2c_if->dev_name = "NGWIC Card";
        i2c_if->i2c_ctrl = get_wic_i2c_ctrl(slot);
        rc = PASSED;
        break;
    case MODULE_SM:
        i2c_if->dev_name = "NGSM Card";
        i2c_if->i2c_ctrl = get_sm_i2c_ctrl(slot);
        rc = PASSED;
        break;
    default:
        rc = FAILED;
        break;
    }

    if (rc != FAILED) {
        i2c_if->offset = 0;
        i2c_if->i2c_bus_type = IOFPGA_I2C;
        i2c_if->sub_addr_len = 0;
        i2c_if->size = sizeof(uint16_t);
        i2c_if->mux = I2C_MUX_ZERO;
        i2c_if->buf = NULL;
        i2c_if->i2c_dev = 0;
    }

    return rc;
}

static ngio_if* get_ngio(int slot, MODULE_TYPE_E t)
{
    ngio_if *ngio = NULL;

    switch (t) {
    case MODULE_WIC:
        ngio = slot_get_ngiowic(slot);
        break;
    case MODULE_SM:
        ngio = slot_get_ngiosm(slot);
        break;
    default:
        break;
    }

    return ngio;
}

static int module_init(module_info_t *info)
{
    int rc = FAILED;

    assert(info != NULL);

    info->ngio = get_ngio(info->slot, info->t);
    if (info->ngio == NULL ) {
        cterr('f', 0, "get ngio failed [slot:%d name:%s]",
              info->slot,
              info->name);
        return rc;
    }

    rc = init_i2c(&info->i2c_if, info->t, info->slot);
    if(rc == FAILED) {
        cterr('f', 0, "init i2c failed [slot:%d name:%s]",
              info->slot,
              info->name);
        return rc;
    }

    info->ngio->id = SLOT_ILLCODE;
    rc = slot_get_info(info->ngio, info->name);
    if(rc == FAILED) {
        if (info->ngio->id != SLOT_VACCODE) {
            cterr('f', 0, "slot get info failed [slot:%d name:%s]",
                  info->slot,
                  info->name);
        }
        return rc;
    } else {
        info->name = info->ngio->name;
        printf("%s on slot %d\n",
              info->name,
              info->ngio->slot);
        if (info->ngio->id == SM_10GKR_TESTCARD
            || info->ngio->id == SM_BCM57412_TESTCARD) {
            info->card_type = TEST_CARD;
        } else {
            info->card_type = UNKNOWN_CARD;
        }
    }

    ngio_ge_cfg(info->ngio);
    msleep(250);
    info->ngio->uart_on(info->ngio);

    return rc;
}

static void module_rw_tlk10232_register(module_info_t *info, TLK10232_REGISTER_TEST_TYPE_E type)
{
    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        tc_tlk10232_reg_rw(type);
        break;
    default:
        printf("Not support card type %d\n", info->card_type);
        break;
    }
}

static int module_set_mode(module_info_t *info)
{
    int rc;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_set_mode_type(info->tlk_channel_addr, xinfo->mode_type);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static void module_clear_latched_registers(module_info_t *info)
{
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        tc_tlk10232_clear_latched_registers(info->tlk_channel_addr, xinfo->mode_type);
        break;
    default:
        printf("Not support card type %d\n", info->card_type);
        break;
    }
}

static int module_set_prbs(module_info_t *info, TLK10232_PRBS_PATTERN_TYPE_E prbs_pattern)
{
    int rc;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_set_prbs(info->tlk_channel_addr, prbs_pattern);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static int module_clear_error_counters(module_info_t *info)
{
    int rc;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_clear_error_counters(info->tlk_channel_addr);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static int module_prbs_generate(module_info_t *info, int enable)
{
    int rc;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_prbs_generate(info->tlk_channel_addr, enable);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static int module_prbs_verify(module_info_t *info, int enable)
{
    int rc;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_prbs_verify(info->tlk_channel_addr, enable);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static int module_check_error_counters(module_info_t *info)
{
    int rc;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_check_error_counters(info->tlk_channel_addr);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static int module_check_status(module_info_t *info, TLK10232_STATUS_TYPE_E type)
{
    int rc;

    assert(info != NULL);

    switch (info->card_type) {
    case TEST_CARD:
        set_tc_i2c_struct(&info->i2c_if);
        rc = tc_tlk10232_check_status(info->tlk_channel_addr, type);
        break;
    default:
        rc = FAILED;
        printf("Not support card type %d\n", info->card_type);
        break;
    }

    return rc;
}

static void module_deinit(module_info_t *info)
{
    ngio_if *ngio = info->ngio;

    if (ngio != NULL) {
        if(ngio->id == SLOT_VACCODE || ngio->id == SLOT_ILLCODE) {
            return;
        }

        if (ngio->off != NULL) {
            ngio->off(ngio);
        }
        ngio->id = SLOT_ILLCODE;

        info->ngio = NULL;
    }
}

static void xpoint_module_select_mode(void)
{
    xpoint_module_info_t *xinfo = &xpoint_module_info;
    TLK10232_MODE_TYPE_E mode;

    mode = getdec_answer("Select Mode :\n0 1G-KX\n1 10G-KR-NO-AN\n2 10G-KR\n",
            xinfo->mode_type,
            TLK10232_MODE_1GKX,
            TLK10232_MODE_10GKR);

    xinfo->mode_type = mode;
    xinfo->mode_name = s_mode_name[mode];
    printf("mode is [%s]\n", xinfo->mode_name);
}

static void xpoint_module_select_prbs(void)
{
    xpoint_module_info_t *xinfo = &xpoint_module_info;
    TLK10232_PRBS_PATTERN_TYPE_E prbs;
    int i;

    for (i = 0; i < 2; i++){
        printf("[%s %d]\n",
                xinfo->module_info[i].name,
                xinfo->module_info[i].slot);
        prbs = getdec_answer("Select PRBS pattern:\n0 PRBS31\n1 PRBS23\n2 PRBS7\n",
                xinfo->module_info[i].prbs_pattern,
                TLK10232_PRBS_PATTERN_31,
                TLK10232_PRBS_PATTERN_7);

        xinfo->module_info[i].prbs_pattern = prbs;
        xinfo->module_info[i].prbs_name = s_prbs_name[prbs];
        printf("[%s %d] PRBS pattern is [%s]\n",
                xinfo->module_info[i].name,
                xinfo->module_info[i].slot,
                xinfo->module_info[i].prbs_name);
    }
}

static void xpoint_module_set_prbs_duration(void)
{
    xpoint_module_info_t *xinfo = &xpoint_module_info;
    int duration;

    duration = getdec_answer("Set PRBS duration (secs):\n",
                xinfo->prbs_duration,
                MIN_PRBS_DURATION,
                MAX_PRBS_DURATION);

    xinfo->prbs_duration = duration;
    printf("PRBS duration is [%d] secs\n", xinfo->prbs_duration);
}

static int xpoint_module_set_mode(void)
{
    int i;
    int rc;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    for (i = 0; i < 2; i++) {
        rc = module_set_mode(&xinfo->module_info[i]);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d set %s mode failed", xinfo->module_info[i].name, xinfo->module_info[i].slot, xinfo->mode_name);
            break;
        } else {
            printf("%s %d set %s mode ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot, xinfo->mode_name);
        }
    }

    return rc;
}

static void xpoint_module_clear_latched_registers(void)
{
    int i;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    for (i = 0; i < 2; i++) {
        module_clear_latched_registers(&xinfo->module_info[i]);
    }
}

static int xpoint_module_set_prbs(void)
{
    int i;
    int rc;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    for (i = 0; i < 2; i++) {
        rc = module_set_prbs(&xinfo->module_info[i], xinfo->module_info[i].prbs_pattern);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d set prbs pattern %s failed",
                    xinfo->module_info[i].name,
                    xinfo->module_info[i].slot,
                    xinfo->module_info[i].prbs_name);
            break;
        } else {
            printf("%s %d set prbs pattern %s ok.\n",
                    xinfo->module_info[i].name,
                    xinfo->module_info[i].slot,
                    xinfo->module_info[i].prbs_name);
        }
    }

    return rc;
}

static int xpoint_module_check_link_status(void)
{
    int i;
    int rc;
    int times;
    int interval = 500;
    int max_times = 10;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    for (i = 0; i < 2; i++) {
        times = 0;
        while (1) {
            rc = module_check_status(&xinfo->module_info[i], TLK10232_HS_AZ_DONE);
            rc |= module_check_status(&xinfo->module_info[i], TLK10232_AGC_LOCKED);
            rc |= module_check_status(&xinfo->module_info[i], TLK10232_PLL_STATUS_LOCKED);
            switch (xinfo->mode_type) {
            case TLK10232_MODE_10GKR:
                rc |= module_check_status(&xinfo->module_info[i], TLK10232_AUTO_NEGOTIATION);
                rc |= module_check_status(&xinfo->module_info[i], TLK10232_LINK_TRAINING);
                rc |= module_check_status(&xinfo->module_info[i], TLK10232_KR_MODE);
                break;
            case TLK10232_MODE_10GKR_NO_AN:
                rc |= module_check_status(&xinfo->module_info[i], TLK10232_KR_PCS_RX_LINK_STATUS);
                rc |= module_check_status(&xinfo->module_info[i], TLK10232_KR_PCS_BLOCK_LOCK);
                break;
            default:
                break;
            }
            if(rc != PASSED) {
                if (times < max_times) {
                    times++;
                    printf("%s %d check status failed %d times!\nTry again...\n", xinfo->module_info[i].name, xinfo->module_info[i].slot, times);
                    msleep(interval);
                    continue;
                } else {
                    cterr('f', 0, "%s %d check status failed,",
                            xinfo->module_info[i].name, xinfo->module_info[i].slot);
                    break;
                }
            } else {
                printf("%s %d check status ok!\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
                break;
            }
        }
    }

    return rc;
}

static void clear_return_print(char *fmt, ...)
{
    va_list ap;

    printf( "\033[2K\r");

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    fflush(stdout);
}

static int xpoint_module_prbs(void)
{
    int rc = PASSED;
    int i;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    rc = xpoint_module_set_mode();
    if(rc != PASSED) {
        goto ERROR_FLAG;
    }

    rc = xpoint_module_check_link_status();
    if(rc != PASSED) {
        goto ERROR_FLAG;
    }

    xpoint_module_clear_latched_registers();

    rc = xpoint_module_set_prbs();
    if(rc != PASSED) {
        goto ERROR_FLAG;
    }

    for (i = 0; i < 2; i++) {
        rc = module_prbs_verify(&xinfo->module_info[i], 1);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d enable prbs verify failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            goto ERROR_FLAG;
        } else {
            printf("%s %d enable prbs verify ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        }
    }

    for (i = 0; i < 2; i++) {
        rc = module_prbs_generate(&xinfo->module_info[i], 1);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d enable prbs generate failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            goto ERROR_FLAG;
        } else {
            printf("%s %d enable prbs generate ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        }
    }

    for (i = 0; i < 2; i++) {
        rc = module_clear_error_counters(&xinfo->module_info[i]);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d clear error counters failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            goto ERROR_FLAG;
        } else {
            printf("%s %d clear error counters ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        }
    }

    printf("\nThe PRBS duration is [%d] secs\n", xinfo->prbs_duration);
    for (i = 0; i < xinfo->prbs_duration; i++) {
        clear_return_print("There are [%d] secs left.", xinfo->prbs_duration - i);
        sleep(1);
    }
    clear_return_print("\n");

    for (i = 0; i < 2; i++) {
        rc = module_check_error_counters(&xinfo->module_info[i]);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d check error counters failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        } else {
            printf("%s %d check error counters ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        }
    }

    for (i = 0; i < 2; i++) {
        rc = module_prbs_verify(&xinfo->module_info[i], 0);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d disable prbs verify failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            goto ERROR_FLAG;
        } else {
            printf("%s %d disable prbs verify ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        }
    }

    for (i = 0; i < 2; i++) {
        rc = module_prbs_generate(&xinfo->module_info[i], 0);
        if(rc != PASSED) {
            cterr('f', 0, "%s %d disable prbs generate failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            goto ERROR_FLAG;
        } else {
            printf("%s %d disable prbs generate ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
        }
    }

ERROR_FLAG:

    return rc;
}

static int xpoint_module_access_tlk10232_register(int slot)
{
    int i;
    xpoint_module_info_t *xinfo = &xpoint_module_info;
    int rc = FAILED;

    for (i = 0; i < 2; i++) {
        if (xinfo->module_info[i].slot == slot){
            printf("%s %d access tlk10232 register.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            module_rw_tlk10232_register(&xinfo->module_info[i], TLK10232_REGISTER_READ_WRITE_TEST);
            rc = PASSED;
            break;
        }
    }

    return rc;
}

static int xpoint_module_dump_tlk10232_register(int slot)
{
    int i;
    xpoint_module_info_t *xinfo = &xpoint_module_info;
    int rc = FAILED;

    for (i = 0; i < 2; i++) {
        if (xinfo->module_info[i].slot == slot){
            printf("%s %d dump tlk10232 register.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            module_rw_tlk10232_register(&xinfo->module_info[i], TLK10232_REGISTER_DUMP_TEST);
            rc = PASSED;
            break;
        }
    }

    return rc;
}

submenu_xtable_t xpoint_module_tests_submenu_table[] = {
    {"Access SM1 TLK10232 Register",
     (PFT)xpoint_module_access_tlk10232_register, 1, 0,
     NULL, 0, NULL, 0},
    {"Access SM2 TLK10232 Register",
     (PFT)xpoint_module_access_tlk10232_register, 2, 0,
     NULL, 0, NULL, 0},
    {"Dump SM1 TLK10232 Register",
     (PFT)xpoint_module_dump_tlk10232_register, 1, 0,
     NULL, 0, NULL, 0},
    {"Dump SM2 TLK10232 Register",
     (PFT)xpoint_module_dump_tlk10232_register, 2, 0,
     NULL, 0, NULL, 0},
    {"Set mode",
     (PFT)xpoint_module_set_mode, 0, 0,
     NULL, 0, NULL, 0},
    {"Check link status",
     (PFT)xpoint_module_check_link_status, 0, 0,
     NULL, 0, NULL, 0},
    {"Set PRBS test pattern",
     (PFT)xpoint_module_set_prbs, 1, 0,
     NULL, 0, NULL, 0},
    {"PRBS test",
     (PFT)xpoint_module_prbs, 0,
     MF_3, NULL, 0, NULL, 0},
};

#define XPOINT_MODULE_TESTS_SUBMENU_TABLE_SIZE \
    (sizeof(xpoint_module_tests_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t xpoint_module_tests_primary_items[XPOINT_MODULE_TESTS_SUBMENU_TABLE_SIZE +
                                          MAX_BASE_ITEMS];
static mitem_t xpoint_module_tests_secondary_items[XPOINT_MODULE_TESTS_SUBMENU_TABLE_SIZE +
                                            MAX_BASE_ITEMS];

menuinfo_t xpoint_module_subtest_menu = {
    "Crosspoint Module Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    xpoint_module_tests_primary_items,
};

menuinfo_t *xpoint_module_submenup = &xpoint_module_subtest_menu;

extern int check_skip_test (char *item);

/* crosspoint entry for module side using 2 SM test cards */
int xpoint_module_prbs_test(boolean test_items_executed)
{
    int i;
    int rc;
    xpoint_module_info_t *xinfo = &xpoint_module_info;

    testname("Crosspoint module side loopback");
    prpass(testpass, "Crosspoint module, ");

    /* TODO: add skip method before test add module status check */
    if (check_skip_test("XPOINT_MODULE") == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    xinfo->mode_type = DEFAULT_MODE;
    xinfo->mode_name = s_mode_name[DEFAULT_MODE];
    for (i = 0; i < 2; i++) {
        rc = module_init(&xinfo->module_info[i]);
        if(rc != PASSED) {
            if (xinfo->module_info[i].ngio->id != SLOT_VACCODE) {
                printf("%s %d init failed", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            }
            goto DEINIT;
        } else {
            xinfo->module_info[i].prbs_pattern = DEFAULT_PRBS;
            xinfo->module_info[i].prbs_name = s_prbs_name[DEFAULT_PRBS];
            if (xinfo->module_info[i].card_type == UNKNOWN_CARD) {
                printf("not support %s %d.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
                goto DEINIT;
            } else {
                printf("%s %d init ok.\n", xinfo->module_info[i].name, xinfo->module_info[i].slot);
            }
        }
    }

    build_primary_submenu(xpoint_module_tests_submenu_table, XPOINT_MODULE_TESTS_SUBMENU_TABLE_SIZE,
                          "Crosspoint Module", &xpoint_module_submenup);
    build_secondary_submenu(xpoint_module_tests_submenu_table, XPOINT_MODULE_TESTS_SUBMENU_TABLE_SIZE,
                            xpoint_module_tests_secondary_items);

    if (!test_items_executed) {
        xpoint_module_select_mode();
        xpoint_module_select_prbs();
        xpoint_module_set_prbs_duration();
        menu(&xpoint_module_subtest_menu, xpoint_module_tests_secondary_items, '\0');
    }
    else
        exec_doall_menu_items(&xpoint_module_subtest_menu);

DEINIT:
    for (i = 0; i < 2; i++) {
        module_deinit(&xinfo->module_info[i]);
    }

    return PASSED;
}

/*
 *-----------------------------------------------------------------------------
$Log: xpoint_module.c,v $
Revision 1.1  2020/01/09 01:02:07  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

/* $Id: highrise_cpld_diag.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/highrise_cpld_diag.c,v $
 *------------------------------------------------------------------
 * Filename: highrise_cpld_diag.c
 *
 * Description: The Highrise CPLD main source code
 * Author: Mingchun Ding
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "gpio.h"
#include "hightower_sub6.h"
#include "highrise_cpld_diag.h"
#include "highrise_cpld_lib.h"
#include "hr_commn_util.h"


/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
long ht_cpld_led_test(void);
static long hr_cpld_reg_test(void);
static long hr_cpld_wdog_test(void);
static long hr_cpld_cpu_reset_test(void);
static long hr_cpld_emmc_reset_test(void);
static long hr_cpld_dyinggasp_test(void);
static long hr_cpld_pwr_st_intr_test(void);
static long hr_cpld_reg_alter(void);
static long hr_cpld_reg_dump(void);
static long hr_cpld_fw_prog(void);
long hr_cpld_wdog_sys_reset(void);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long build_hr_cpld_menu(int);
long hr_cpld_utility_submenu(int);

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern int do_all_menu_items(struct menuinfo *);
#ifdef PCA9557_ENABLED
extern long timingcard_pca9557_init (void);
extern long timingcard_pca9557_power_cycle_cpld (void);
#endif
extern void force_user_do_power_cycle(char *prompts[]);

/***********************************************************************
 *  Global Variable
 ************************************************************************/
static submenu_xtable_t hr_cpld_submenu_tbl[] = {
    { "CPLD utility", (type_t(*)())hr_cpld_utility_submenu, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "CPLD Register Test", (type_t(*)())hr_cpld_reg_test, 0,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },

    { "CPLD LED Test", (type_t(*)())ht_cpld_led_test, FALSE,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_HIDDEN_EXE, (type_t(*)())0, 0, (type_t(*)())0, 0 },

    { "CPLD WatchDog Test", (type_t(*)())hr_cpld_wdog_test, FALSE,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_HIDDEN_EXE, (type_t(*)())0, 0, (type_t(*)())0, 0 },

    { "CPLD CPU Reset Test", (type_t(*)())hr_cpld_cpu_reset_test, FALSE,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_HIDDEN_EXE, (type_t(*)())0, 0, (type_t(*)())0, 0 },

    { "CPLD EMMC Reset Test", (type_t(*)())hr_cpld_emmc_reset_test, FALSE,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_HIDDEN_EXE, (type_t(*)())0, 0, (type_t(*)())0, 0 },

    { "CPLD Dying-gasp Test", (type_t(*)())hr_cpld_dyinggasp_test, FALSE,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_HIDDEN_EXE, (type_t(*)())0, 0, (type_t(*)())0, 0 },

    { "CPLD Power Status Intr Test", (type_t(*)())hr_cpld_pwr_st_intr_test, FALSE,
     MF_SHOW_ERRCOUNT | MF_CONTINUOUS | MF_HIDDEN_EXE, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define HR_CPLD_SUBMENU_TABLE_SZ \
                (sizeof(hr_cpld_submenu_tbl)/sizeof(submenu_xtable_t))

/***********************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 ************************************************************************/
static mitem_t hr_cpld_primary_items[HR_CPLD_SUBMENU_TABLE_SZ +
                                             MAX_BASE_ITEMS];
static mitem_t hr_cpld_secondary_items[HR_CPLD_SUBMENU_TABLE_SZ +
                                               MAX_BASE_ITEMS];

static menuinfo_t hr_cpld_main_menu = {
    "CPLD Menu",
    0,                        /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,    /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    hr_cpld_primary_items,
};
static menuinfo_t *hr_cpld_menup = &hr_cpld_main_menu;

/***********************************************************************
 * utilities menu
 ************************************************************************/
static mitem_t hr_cpld_util_submenu_table[] = {
    { "Dump CPLD Registers", 0, 0, hr_cpld_reg_dump,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Alter CPLD Register", 0, 0, hr_cpld_reg_alter,
      (long *)&zero, 0, (type_t(*)())0, 0 },

    { "Program CPLD Firmware", 0, 0, hr_cpld_fw_prog,
      (long *)&zero, 0, (type_t(*)())0, 0 },

    { "WatchDog System Reset", 0, 0, hr_cpld_wdog_sys_reset,
      (long *)&zero, 0, (type_t(*)())0, 0 },
};

#define HR_CPLD_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(hr_cpld_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t hr_cpld_util_subtest_menu = {
    "Utilities Menu",
    0,                                  /* title param */
    0,                                  /* show diag flags */
    0,
    HR_CPLD_UTIL_SUBMENU_TABLE_SZ,
    hr_cpld_util_submenu_table,
};

static menuinfo_t *hr_cpld_util_submenup = &hr_cpld_util_subtest_menu;

extern unsigned char pof_cpld_fw_array[];
int upgrade_interface = UPGRADE_FROM_CPLD;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_hr_cpld_menu
 *
 * Description: Build Timing Card CPLD tests and utilities menu.
 *
 * Inputs:  show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_hr_cpld_menu (int show_menu)
{

    build_primary_submenu(hr_cpld_submenu_tbl, HR_CPLD_SUBMENU_TABLE_SZ,
                          "CPLD Main Menu", &hr_cpld_menup);
    build_secondary_submenu(hr_cpld_submenu_tbl, HR_CPLD_SUBMENU_TABLE_SZ,
                            hr_cpld_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(hr_cpld_menup, hr_cpld_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(hr_cpld_menup);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: hr_cpld_utility_submenu().
 *
 * This function implements the CPLD test/menu
 *
 * Input: menu_option - show menu option
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
long hr_cpld_utility_submenu (int menu_option)
{
    char *tname = "CPLD Utilities";

    testname("%s", tname);

    menu(hr_cpld_util_submenup, hr_cpld_util_submenu_table, '\0');

    return (PASSED);
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_test
 *
 * Wrapper for CPLD Register test.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long hr_cpld_reg_test (void)
{
    char *tname = "CPLD Register";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (hr_cpld_reg_test_lib() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


#define LED_TST_LED_OFF         0x0
#define LED_TST_LED_ON          0x1
#define LED_TST_LED_BLINK       0x2

#define LED_TST_LED_4G_R_MSK    0x00003000
#define LED_TST_LED_4G_G_MSK    0x00000C00
#define LED_TST_LED_4G_B_MSK    0x00000300
#define LED_TST_LED_4G_Y_MSK    (LED_TST_LED_4G_R_MSK | LED_TST_LED_4G_G_MSK)
#define LED_TST_LED_5G_R_MSK    0x000000C0
#define LED_TST_LED_5G_G_MSK    0x00000030
#define LED_TST_LED_5G_B_MSK    0x0000000C
#define LED_TST_LED_5G_Y_MSK    (LED_TST_LED_5G_R_MSK | LED_TST_LED_5G_G_MSK)
#define LED_TST_SW_CTRL_MSK     0x00000001

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
/**********************************************************************
 *
 * Function: ht_cpld_led_test
 *
 * Wrapper for CPLD LED test.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long ht_cpld_led_test (void)
{
    struct _led_state {
        unsigned int val;
        char *name;
    } led_state[] = {
        {LED_TST_LED_ON   , "On"   },
        {LED_TST_LED_BLINK, "Blink"},
    };

    struct _led_ctrl {
        unsigned int msk;
        char *name;
    } led_ctrl[] = {
        {LED_TST_LED_4G_R_MSK, "4G Red"   },
        {LED_TST_LED_4G_G_MSK, "4G Green" },
        {LED_TST_LED_4G_B_MSK, "4G Blue"  },
        {LED_TST_LED_4G_Y_MSK, "4G Yellow"},
        {LED_TST_LED_5G_R_MSK, "5G Red"   },
        {LED_TST_LED_5G_G_MSK, "5G Green" },
        {LED_TST_LED_5G_B_MSK, "5G Blue"  },
        {LED_TST_LED_5G_Y_MSK, "5G Yellow"},
    };
    int  idx = 0;
    int  jdx = 0;
    int  ret = PASSED;
    char chr = 0;
    unsigned int val = 0;
    const char *tname = "Led Test";
    char board_name[128] = {[0 ... sizeof(board_name)-1] = 0};
    uint8_t board_id     = 0;

    testname("%s", tname);

    hr_cpld_get_boardid(&board_id, board_name);

    printf("\n!!!Visual test, please check LED status conforming to prompt.!!!\n\n");
    printf("Board: %s(ID: %u)\n", board_name, board_id);

    _DRAIN_STDIN();
    for (idx=0; idx<ARRAY_SIZE(led_ctrl) && !ret; idx++) {
        for (jdx=0; jdx<ARRAY_SIZE(led_state) && !ret; jdx++) {
            val = 0;

            #define _COMPOSE_VAL(MSK__) \
                if (MSK__ & led_ctrl[idx].msk) val |= (led_state[jdx].val << BIT_START(MSK__, 0))

            if ((board_id == HR_CPLD_BOARD_HIGHRISE_CAT18) &&
                ((led_ctrl[idx].msk & LED_TST_LED_5G_R_MSK) ||
                 (led_ctrl[idx].msk & LED_TST_LED_5G_G_MSK) ||
                 (led_ctrl[idx].msk & LED_TST_LED_5G_B_MSK)))
                    continue;

            _COMPOSE_VAL(LED_TST_LED_5G_R_MSK);
            _COMPOSE_VAL(LED_TST_LED_5G_G_MSK);
            _COMPOSE_VAL(LED_TST_LED_5G_B_MSK);
            _COMPOSE_VAL(LED_TST_LED_4G_R_MSK);
            _COMPOSE_VAL(LED_TST_LED_4G_G_MSK);
            _COMPOSE_VAL(LED_TST_LED_4G_B_MSK);

            val |= LED_TST_SW_CTRL_MSK;

            hr_cpld_reg_write_32(HR_CPLD_LED_CTRL, val);

            printf("(0x%08x) LED %-9s %-6s? (Y/N):", val, led_ctrl[idx].name, led_state[jdx].name);
            fflush(stdout);

            chr = getchar();
            _DRAIN_STDIN();
            if (chr == 'y' || chr == 'Y') {
                continue;
            }

            cterr('f', 0, "%s failed.", tname);
            return FAILED;
        }
    }

    printf("PASSED\n");
    return PASSED;
}

static long hr_cpld_wdog_supported(void)
{
    uint16_t ver      = 0;
    uint16_t ver_date = 0;

    ERR_RET_COND(0 > hr_cpld_get_version(&ver, &ver_date), -(__LINE__), "Failed to get CPLD version");

    printf("CPLD Version:%04x, %04x\n", ver, ver_date);

    RET_COND(ver < 0xa002, 0, "info", "This CPLD Version does not support WatchDog\n");
    return 1;
}

static long hr_cpld_wdog_enb(int enb)
{
    unsigned long ret = 0;
    unsigned long reg = 0;

    TERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_RESET_CTRL, &reg), FAILED,
                  "Failed to read reg-%02x", HR_CPLD_RESET_CTRL);

    if (enb)
        reg |=  HR_CPLD_UNRESET_WDOG;
    else
        ret &= ~HR_CPLD_UNRESET_WDOG;

    TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL, reg), FAILED,
                  "Failed to write reg-%02x", HR_CPLD_RESET_CTRL);

    TERR_RET_COND(0 > gpio_write(CPU_TO_CPLD_STATUS_0, 0), FAILED, "Failed to write gpio-%d\n", CPU_TO_CPLD_STATUS_0);
    if (enb)
        TERR_RET_COND(0 > gpio_write(CPU_TO_CPLD_STATUS_0, 1), FAILED, "Failed to write gpio-%d\n", CPU_TO_CPLD_STATUS_0);

    return 0;
}

static long hr_cpld_wdog_feed(void)
{
    int val = 0;
    TERR_RET_COND(0 > gpio_read(CPU_TO_CPLD_STATUS_0, &val), FAILED, "Failed to read gpio-%d\n", CPU_TO_CPLD_STATUS_0);
    val = (~val) & 0x1;
    TERR_RET_COND(0 > gpio_write(CPU_TO_CPLD_STATUS_0, val), FAILED, "Failed to write gpio-%d\n", CPU_TO_CPLD_STATUS_0);
    return 0;
}

static long hr_cpld_wdog_test (void)
{
    int      ret      = 0;
    int      idx      = 0;
    int      jdx      = 0;

    const char *tname = "CPLD WatchDog Test";

    TERR_RET_COND(0 > (ret = hr_cpld_wdog_supported()), -(__LINE__), "Failed.\n");
    RET_COND(!ret, 0, "", "");

    system("sync"); /* in case of data lost */

    TERR_RET_COND(0 > hr_cpld_wdog_enb(1), -(__LINE__), "Enable WatchDog failed.\n");

    printf("System should NOT reset within 20s:\n");
    for(idx = 0; idx < 10; idx++) {
        TERR_RET_COND(0 > hr_cpld_wdog_feed(), -(__LINE__), "Feed dog failed.\n");
        for(jdx = 0; jdx < 3; jdx++) {
            putchar('.');
            fflush(stdout);
            sleep(1);
        }
        putchar(' ');
        fflush(stdout);
    };
    putchar('\n');

    /* system would reset during this loop if watchdog works properly */
    printf("System should     reset within 20s:\n");
    for(idx = 0; idx < 10 ; idx++) {
        for(jdx = 0; jdx < 3; jdx++) {
            putchar('-');
            fflush(stdout);
            sleep(1);
        }
        putchar(' ');
        fflush(stdout);
    }

    /* If we are here, watchdog does not take effect */
    TERR_RET_COND(1, FAILED, "%s failed.\n", tname);
}

long hr_cpld_wdog_sys_reset(void)
{
    int idx = 0;
    int jdx = 0;

    system("sync"); /* sync cache to storage in case of date lost */

    /* enable watchdog without feeding it */
    ERR_RET_COND(0 > hr_cpld_wdog_enb(1), -(__LINE__), "Enable WatchDog failed.\n");

    /* system would reset during this loop if watchdog works properly */
    printf("System will reset within 20s:\n");
    for(idx = 0; idx < 10 ; idx++) {
        for(jdx = 0; jdx < 3; jdx++) {
            putchar('-');
            fflush(stdout);
            sleep(1);
        }
        putchar(' ');
        fflush(stdout);
    }

    /* If we are here, watchdog does not take effect */
    ERR_RET_COND(1, FAILED, "Failed to reset system by watchdog.\n");
}

static long hr_cpld_cpu_reset_test(void)
{
    unsigned long reg = 0;
    char          chr = 0;

    TERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_RESET_PROTECT, &reg), FAILED,
                  "Failed to read reg-%02x", HR_CPLD_RESET_PROTECT);

    _DRAIN_STDIN();
    printf("Enable CPU reset protect(y/N):");
    fflush(stdout);
    chr = getchar();
    _DRAIN_STDIN();
    switch(chr) {
    case 'y': case 'Y':
        reg |=  HR_CPLD_RESET_LOCK_CPU;
        break;
    case 'n': case 'N': case '\n' :case '\r':
        reg &= ~HR_CPLD_RESET_LOCK_CPU;
        break;
    default:
        TERR_RET_COND(1, -(__LINE__), "Invalid choice, abort.\n");
    }
    TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_PROTECT, reg), FAILED,
                  "Failed to write reg-%02x", HR_CPLD_RESET_PROTECT);

    printf("CPU reset protect is %s\n", reg & HR_CPLD_RESET_LOCK_CPU ? "enabled" : "disabled");

    TERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_RESET_CTRL, &reg), FAILED,
                  "Failed to read reg-%02x", HR_CPLD_RESET_CTRL);

    reg |= HR_CPLD_CPU_REST_ENB;

    TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL, reg), FAILED,
                  "Failed to write reg-%02x", HR_CPLD_RESET_CTRL);

    TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_CPU_RESET_TRIGGER, 1), FAILED,
                  "Failed to write reg-%02x", HR_CPLD_CPU_RESET_TRIGGER);

    sleep(3);
    TERR_RET_COND(chr == 'n' || chr == 'N', -(__LINE__), "Should not be here, CPU should reset.\n");

    printf("Passed.\n");
    return 0;
}

extern int show_emmc_info (void);
extern int emmc_hwrst_enb_dis(int new, int *post);
static long hr_cpld_emmc_reset_test(void)
{
    unsigned long reg = 0;
    int    mmc_rst_enb= 0;
    const char *tname = "CPLD Emmc reset test";
    int       protect = 0;
    int           ret = FAILED;
    char *prmpts[] = {
        "Emmc is in hardware reset, power-cycle the board please.",
        NULL
    };

    emmc_hwrst_enb_dis(0, &mmc_rst_enb);
    if (mmc_rst_enb == 0) {
        emmc_hwrst_enb_dis(1, &mmc_rst_enb);
    }
    TERR_RET_COND(mmc_rst_enb != 1, -(__LINE__), "Cannot enable Emmc hw-reset function.\n");

    for(protect = 1; protect >= 0; protect--) {
        TERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_RESET_PROTECT, &reg), FAILED,
            "Failed to read reg-%02x", HR_CPLD_RESET_PROTECT);

        if (protect == 0) {
            reg &= ~HR_CPLD_RESET_LOCK_EMMC;
        } else {
            reg |=  HR_CPLD_RESET_LOCK_EMMC;
        }

        TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_PROTECT, reg), FAILED,
                      "Failed to write reg-%02x", HR_CPLD_RESET_PROTECT);

        printf("\nEmmc reset protect is %s\n", reg & HR_CPLD_RESET_LOCK_EMMC? "enabled" : "disabled");

        system("sync"); /* sync cache to emmc storage in case of data lost */

        TERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_RESET_CTRL, &reg), FAILED,
                      "Failed to read reg-%02x", HR_CPLD_RESET_CTRL);

        /* As observed, this sequence works:
           write 1 --> write 0 --> write 1 */
        reg |= HR_CPLD_UNRESET_EMMC;
        TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL, reg), FAILED,
                      "Failed to write reg-%02x", HR_CPLD_RESET_CTRL);

        reg &= ~HR_CPLD_UNRESET_EMMC;
        TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL, reg), FAILED,
                      "Failed to write reg-%02x", HR_CPLD_RESET_CTRL);

        reg |= HR_CPLD_UNRESET_EMMC;
        TERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL, reg), FAILED,
                      "Failed to write reg-%02x", HR_CPLD_RESET_CTRL);

        sleep(2);
        if (protect == 0) {
            /* Emmc should NOT be accessible now. */
            printf("\nExpected: Emmc should NOT be accessible now\n");
            if (PASSED != show_emmc_info()) {
                printf("\nObserved: Emmc is NOT accessible now\n");
                printf("\nPASSED\n");
                printf("\n\nTo use EMMC again, please power cycle the board.");
                force_user_do_power_cycle(prmpts);
            }
            else {
                printf("\nObserved: Emmc is still accessible now\n");
                ret = FAILED;
            }
        } else {
            /* Emmc should     be accessible now. */
            printf("Expected: Emmc should     be accessible still\n");
            if (PASSED != show_emmc_info()) {
                printf("\nObserved: Emmc is NOT accessible now\n");
                printf("%s FAILED\n", tname);
                printf("\n\nTo use EMMC again, please power cycle the board.");
                force_user_do_power_cycle(prmpts);
            }
            else {
                printf("\nObserved: Emmc is still accessible now\n");
                ret = PASSED;
            }
        }
        printf("\n\n");
    }

    if (ret == PASSED)
        printf("%s PASSED\n", tname);
    else
        printf("%s FAILED\n", tname);
    return ret;
}

static long hr_cpld_dyinggasp_test(void)
{
/*
    const char *stage_file = "/mmcp1/.highrise_cpld_dyinggasp_test_stage";
    const char *stage_file_rmv = "rm -f /mmcp1/.highrise_cpld_dyinggasp_test_stage";
*/
    const char *stage_file = "/dygsp/highrise_cpld_dyinggasp_test_stage";
    const char *stage_file_rmv = "rm -f /dygsp/highrise_cpld_dyinggasp_test_stage";

    int fd = -1;
    char stage = 0;
    char chr = 0;
    unsigned int long val = 0;
    char *tname = "CPLD Dying-Gasp Test";
    char *prmpts[] = {
        "Please power-cycple the board.",
        "After power-cycle, run the diage immediatly.",
        "And check the test status.",
        NULL
    };

    system("mkdir /dygsp");
    system("mount /dev/mmcblk0p1 /dygsp");

    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PWR_STATUS, &val), FAILED, "Read cpld failed.\n");
    printf("\nCurrent dying-gas interrupt status:%s\n\n", val & HR_CPLD_PWR_INT_GASP  ? "Set" : "Cleared");

    if (access(stage_file, F_OK) == 0) {
        fd = open(stage_file, O_RDWR);
        if (fd < 0) {
            printf("Cannot open '%s', FAILED.\n", stage_file);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }
        if (1 != read(fd, &stage, 1)) {
            printf("Cannot read '%s'. FAILED.\n", stage_file);
            close(fd);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }
        lseek(fd, 0, SEEK_SET);
    } else {
        fd = open(stage_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            printf("Cannot open '%s', FAILED.\n", stage_file);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }
        stage = 'a';
    }

    printf("\nTest stage:%c\n", stage - 32);
    switch(stage) {
    case 'a':
        {
            _DRAIN_STDIN();
            printf("\n\nPlease STRICTLY following the prompts to run this test, continue(y/N):");
            fflush(stdout);
            chr = getchar();
            _DRAIN_STDIN();
            if (chr != 'y' && chr != 'Y') {
                printf("Abort by user.\n");
                close(fd);
                system(stage_file_rmv);
                sync();
                return PASSED;
            }
        }

        /*clear and requeset power-cycle */
        val |= HR_CPLD_PWR_INT_GASP;
        ERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_PWR_STATUS, val), FAILED, "Write cpld failed.\n");
        msleep(300);
        val = 0;
        ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PWR_STATUS, &val), FAILED, "Read cpld failed.\n");
        if (val & HR_CPLD_PWR_INT_GASP) {
            printf("\nClear dying-gasp status failed. FAILED\n");
            close(fd);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }

        stage = 'b';
        if (1 != write(fd, &stage, 1)) {
            printf("Write '%s' failed. FAILED.\n", stage_file);
            close(fd);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }
        close(fd);
        sync();
        force_user_do_power_cycle(prmpts);
        break;
    case 'b':
        if (!(val & HR_CPLD_PWR_INT_GASP)) {
            printf("\nDying-gasp intr status should be set but not, FAILED.\n");
            close(fd);
            system(stage_file_rmv);
            system("sync");
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }

        /*clear and reboot */
        val |= HR_CPLD_PWR_INT_GASP;
        ERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_PWR_STATUS, val), FAILED, "Write cpld failed.\n");
        msleep(300);
        val = 0;
        ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PWR_STATUS, &val), FAILED, "Read cpld failed.\n");
        if ((val & HR_CPLD_PWR_INT_GASP)) {
            printf("Clear dying-gasp status failed. FAILED\n");
            close(fd);
            system(stage_file_rmv);
            system("sync");
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }

        stage = 'c';
        if (1 != write(fd, &stage, 1)) {
            printf("Write '%s' failed. FAILED.\n", stage_file);
            close(fd);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }
        close(fd);
        sync();

        printf("\nNow system will reboot.\n");
        printf("After reboot, please run this test again immediatly.\n");
        printf("And check the test status.\n\n");
        sleep(8);
        system("reboot");
        while(1);
        break;
    case 'c':
        if (val & HR_CPLD_PWR_INT_GASP) {
            printf("\nDying-gasp intr status should NOT be set but set, FAILED.\n");
            close(fd);
            system(stage_file_rmv);
            sync();
            cterr('f', 0, "%s FAILED.", tname);
            return FAILED;
        }
        break;
    default:
        printf("\nUnknown test stage '%c'\n", stage);
        close(fd);
        system(stage_file_rmv);
        sync();
        printf("FAILED\n");
        cterr('f', 0, "%s FAILED.", tname);
        return FAILED;
    }
    close(fd);
    system(stage_file_rmv);
    sync();
    printf("%s PASSED\n", tname);
    return PASSED;
}

static long hr_cpld_pwr_st_intr_test(void)
{
    int uret = PASSED;
    int enb  = 0;
    unsigned int long val = 0;
    char *tname = "CPLD Power Status Intr Test";
    char buf[4096] = {[0 ... sizeof(buf) - 1] = 0};
    char *p = NULL;
    char *q = NULL;

    testname("%s", tname);

    #define _PWR_INTR_SST_MSK_ALL (HR_CPLD_INT_STA_CPU_ERR | \
                                   HR_CPLD_INT_STA_PWR_ERR)

    #define _PWR_INTR_DST_MSK_ALL (HR_CPLD_INT_STA_CPU_ERR | \
                                   HR_CPLD_PWR_INT_3300S   | \
                                   HR_CPLD_PWR_INT_1800S   | \
                                   HR_CPLD_PWR_INT_3300    | \
                                   HR_CPLD_PWR_INT_2500    | \
                                   HR_CPLD_PWR_INT_1800    | \
                                   HR_CPLD_PWR_INT_1500    | \
                                   HR_CPLD_PWR_INT_1200    | \
                                   HR_CPLD_PWR_INT_0900    | \
                                   HR_CPLD_PWR_INT_0800    | \
                                   HR_CPLD_PWR_INT_GASP)

    #define _PWR_INTR_TRIG_MSK_ALL (HR_CPLD_PWR_TST_CPU_DECC | \
                                    HR_CPLD_PWR_TST_3300S    | \
                                    HR_CPLD_PWR_TST_1800S    | \
                                    HR_CPLD_PWR_TST_3300     | \
                                    HR_CPLD_PWR_TST_2500     | \
                                    HR_CPLD_PWR_TST_1800     | \
                                    HR_CPLD_PWR_TST_1500     | \
                                    HR_CPLD_PWR_TST_1200     | \
                                    HR_CPLD_PWR_TST_0900     | \
                                    HR_CPLD_PWR_TST_0800     | \
                                    HR_CPLD_PWR_TST_GASP)

    #define _PWR_INTR_REG_DUMP(REG) do {                \
        if (PASSED == hr_cpld_reg_read_32(REG, &val)) { \
            printf("%s val:%#lx\n", #REG, val);         \
        }                                               \
    } while(0)

    #define _PWR_INTR_CLR_ALL(TAG) do {                                        \
        printf("%s\n", TAG);                                                   \
        val = 0;  /* wzclr */                                                  \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_ERROR_TEST, val), \
            FAILED, "Write cpld failed.\n");                                   \
        val = ~0; /* woclr */                                                  \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_PWR_STATUS, val), \
            FAILED, "Read cpld failed.\n");                                    \
        val = 0;  /* wzclr */                                                  \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_INT_ENABLE, val), \
            FAILED, "Write cpld failed.\n");                                   \
        val = ~0; /* woclr */                                                  \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_INT_STATUS, val), \
            FAILED, "Write cpld failed.\n");                                   \
    }while(0)

    #define _PWR_INTR_ST_SHOW() do {                                           \
        ERR_URET_COND(PASSED != hr_cpld_get_sys_info(buf, sizeof(buf) - 1),    \
            FAILED, "Get current powr status failed.\n");                      \
        if ((p = strstr(buf, "Power Status")) && (q = strstr(p, "\n\n"))) {    \
            *q = 0;                                                            \
            printf("Current %s\n", p);                                         \
        }                                                                      \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_ERROR_TEST, &val), \
            FAILED, "Read cpld failed.\n");                                    \
        _PWR_INTR_REG_DUMP(HR_CPLD_INT_ENABLE);                                \
        _PWR_INTR_REG_DUMP(HR_CPLD_INT_STATUS);                                \
        _PWR_INTR_REG_DUMP(HR_CPLD_PWR_STATUS);                                \
        _PWR_INTR_REG_DUMP(HR_CPLD_ERROR_TEST);                                \
    } while(0)


    #define _PWR_INTR_ENB(ENB, REG, MSK) do {                                     \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##REG, &val), FAILED, \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#REG);                            \
        val = ENB ? (val | HR_CPLD_##MSK) : (val & ~HR_CPLD_##MSK);               \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_##REG, val), FAILED, \
            "write Reg-%s failed, val:%lx.\n", "HR_CPLD_"#REG, val);              \
    }while(0)

    /* Enable only the bit by MSK, disbale all others */
    #define _PWR_INTR_ENB_ONLY(ENB, REG, MSK) do {                                \
        val = ENB ? HR_CPLD_##MSK : 0;                                            \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_##REG, val), FAILED, \
            "write Reg-%s failed, val:%lx.\n", "HR_CPLD_"#REG, val);              \
    }while(0)


    /* Clear, Trigger and Check (CTC)
     * ENB  - Intr en state
     * IMSK - The bit to enbale
     * TREG - Trigger register
     * TMSK - Trigger bit
     * SREG - Summary status register
     * SMSK - Summary status bit
     * DREG - Detail status register
     * DMSK - Detail status bit
     */
    #define _PWR_INTR_CTC(ENB, IMSK, TREG, TMSK, SREG, SMSK, DREG, DMSK) do {       \
        printf("Power interrupt %s\n", ENB? "enabled" : "disabled");                \
        _PWR_INTR_ENB_ONLY(ENB, INT_ENABLE, INT_EN_##IMSK##_ERR);                   \
        printf("Trigger %s ...\n", "HR_CPLD_"#TMSK);                                \
    /* a, clear trig      */                                                        \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##TREG, &val), FAILED,  \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#TREG);                             \
        val = (val & ~_PWR_INTR_TRIG_MSK_ALL); /* wzclr */                          \
        printf("Write reg %s val %lx\n", "HR_CPLD_"#TREG, val);                     \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_##TREG, val), FAILED,  \
            "write Reg-%s failed, val:%lx.\n", "HR_CPLD_"#TREG, val);               \
                                                                                    \
    /* b, clear st sum    */                                                        \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##SREG, &val), FAILED,  \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#SREG);                             \
        val = (~val | _PWR_INTR_SST_MSK_ALL); /* woclr */                           \
        printf("Write reg %s val %lx\n", "HR_CPLD_"#SREG, val);                     \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_##SREG, val), FAILED,  \
            "write Reg-%s failed, val:%lx.\n", "HR_CPLD_"#SREG, val);               \
                                                                                    \
    /* c, clear st detail */                                                        \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##DREG, &val), FAILED,  \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#DREG);                             \
        val = (~val | _PWR_INTR_DST_MSK_ALL); /* woclr */                           \
        printf("Write reg %s val %lx\n", "HR_CPLD_"#DREG, val);                     \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_##DREG, val), FAILED,  \
            "write Reg-%s failed, val:%lx.\n", "HR_CPLD_"#DREG, val);               \
                                                                                    \
    /* d, trig */                                                                   \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##TREG, &val), FAILED,  \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#TREG);                             \
        val = (val & ~_PWR_INTR_TRIG_MSK_ALL) | HR_CPLD_##TMSK; /* wzclr */         \
        printf("Write reg %s val %lx\n", "HR_CPLD_"#TREG, val);                     \
        ERR_URET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_##TREG, val), FAILED,  \
            "write Reg-%s failed, val:%lx.\n", "HR_CPLD_"#TREG, val);               \
                                                                                    \
    /* e, check st summary*/                                                        \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##SREG, &val), FAILED,  \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#SREG);                             \
        ERR_URET_COND(!(val & (HR_CPLD_##SMSK)), FAILED,                            \
            "Intr %s failed to be triggered, reg val:%#lx\n",                       \
            "HR_CPLD_"#SMSK, val);                                                  \
                                                                                    \
        ERR_URET_COND(((val & _PWR_INTR_SST_MSK_ALL) & (~(HR_CPLD_##SMSK))), FAILED,\
            "Intr other than %s occured when it/they should not, reg val:%#lx\n",   \
            "HR_CPLD_"#SMSK, val);                                                  \
    /* f, check st detail */                                                        \
        ERR_URET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_##DREG, &val), FAILED,  \
            "Read  Reg-%s failed.\n", "HR_CPLD_"#DREG);                             \
        ERR_URET_COND(!(val & (HR_CPLD_##DMSK)), FAILED,                            \
            "Intr %s failed to be triggered, reg val:%#lx\n",                       \
            "HR_CPLD_"#DMSK, val);                                                  \
                                                                                    \
        ERR_URET_COND(((val & _PWR_INTR_DST_MSK_ALL) & (~(HR_CPLD_##DMSK))), FAILED,\
            "Intr other than %s occured when it/they should not, reg val:%#lx\n",   \
            "HR_CPLD_"#DMSK, val);                                                  \
                                                                                    \
        _PWR_INTR_ST_SHOW();                                                        \
                                                                                    \
    /* g, check CPU side intr line status(MPP0) */                                  \
        ERR_URET_COND(0 != gpio_read(CPLD_CPU_INT_L, (int *)&val), FAILED,          \
            "Failed to read CPLD_CPU_INT_L(MPP0).\n");                              \
        printf("CPLD_CPU_INT_L(MPP0):%ld\n", val);                                  \
                                                                                    \
        val = !val; /*NOTE:Low-active??, need to confirm with HW */                 \
        ERR_URET_COND(!!(ENB) != !!val, FAILED,                                     \
            "CPLD_CPU_INT_L(MPP0) is %s while it should %sbe.\n",                   \
            val ? "set" : "NOT set", ENB ? "" : "NOT ");                            \
                                                                                    \
        printf("Check %s and %s OK\n\n", "HR_CPLD_"#SMSK, "HR_CPLD_"#DMSK);         \
    }while(0)

    /*1, Show current status*/
    printf("\n");
    _PWR_INTR_ST_SHOW();
    printf("\n");


    /*2, Clear status, test trigger and enable regs */
    printf("\n");
    _PWR_INTR_CLR_ALL("Clear Power Status and Trigger...\n");
    _PWR_INTR_ST_SHOW();
    printf("\n");


    /*3, Trigger each test bit and check status && MPP0 */
    printf("\n");
    printf("Trigger Power Status Intrs...\n");

    for(enb = 0; enb <= 1; enb++) {
        /* Reserved now, not writable
        _PWR_INTR_CTC(enb, CPU, ERROR_TEST, PWR_TST_CPU_DECC, INT_STATUS, INT_STA_CPU_ERR, INT_STATUS, INT_STA_CPU_ERR);
        */
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_3300S, INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_3300S);
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_1800S, INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_1800S);
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_3300 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_3300 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_2500 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_2500 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_1800 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_1800 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_1500 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_1500 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_1200 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_1200 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_0900 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_0900 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_0800 , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_0800 );
        _PWR_INTR_CTC(enb, PWR, ERROR_TEST, PWR_TST_GASP , INT_STATUS, INT_STA_PWR_ERR, PWR_STATUS, PWR_INT_GASP );
    }

    /*4, Clear status, test trigger and enable regs */
    printf("\n");
    _PWR_INTR_CLR_ALL("Clear Power Status & Trigger before exit...\n");
    _PWR_INTR_ST_SHOW();
    printf("\n");

    uret = PASSED;
_EXIT_POINT:
    _PWR_INTR_REG_DUMP(HR_CPLD_INT_ENABLE);
    _PWR_INTR_REG_DUMP(HR_CPLD_INT_STATUS);
    _PWR_INTR_REG_DUMP(HR_CPLD_PWR_STATUS);
    _PWR_INTR_REG_DUMP(HR_CPLD_ERROR_TEST);
    if (uret != PASSED) {
        cterr('f', 0, "%s FAILED.\n", tname);
    } else {
        printf("%s PASSED.\n", tname);
    }
    return uret;
#undef _PWR_INTR_CLR_ALL
#undef _PWR_INTR_REG_DUMP
#undef _PWR_INTR_ST_SHOW
#undef _PWR_INTR_SST_MSK_ALL
#undef _PWR_INTR_DST_MSK_ALL
#undef _PWR_INTR_TRIG_MSK_ALL
#undef _PWR_INTR_ENB
#undef _PWR_INTR_ENB_ONLY
#undef _PWR_INTR_CTC
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_alter
 *
 * Wrapper for CPLD Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long hr_cpld_reg_alter (void)
{
    return util_oir_cpld_reg_write();
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_dump
 *
 * Wrapper for CPLD Register Read utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long hr_cpld_reg_dump (void)
{
    return util_oir_cpld_reg_read();
}

extern int jbi_main(int, char**);
static long hr_cpld_fw_prog (void)
{
    /*Available options:
     *    -h          : show help message
     *    -v          : show verbose messages
     *    -i          : show file info only - does not execute any action
     *    -a<action>  : specify an action name (Jam STAPL)
     *    -d<var=val> : initialize variable to specified value (Jam 1.1)
     *    -d<proc=1>  : enable optional procedure (Jam STAPL)
     *    -d<proc=0>  : disable recommended procedure (Jam STAPL)
     *    -s<port>    : serial port name (for BitBlaster)
     *    -r          : don't reset JTAG TAP after use
     * PROGRAM
     * BLANKCHECK
     * VERIFY
     * ERASE
     * READ_USERCODE
     * CHECK_IDCODE
     */

    const char *act_lst[]  = {
        "PROGRAM",
        "BLANKCHECK",
        "VERIFY",
        "ERASE",
        "READ_USERCODE",
        "CHECK_IDCODE",
        NULL,
    };
    static char def_jbc[64] = "/highrise.jbc";

    int  idx = 0;
    int  ret = 0;
    char chr = 0;
    char buf[64] = {[0 ... sizeof(buf) - 1] = 0};
    char jbc[64] = {[0 ... sizeof(buf) - 1] = 0};
    char act[64] = {[0 ... sizeof(buf) - 1] = 0};
    char *arg[8] = {[0 ... sizeof(arg)/sizeof(char *) - 1] = NULL};

    _DRAIN_STDIN();

    /* 0, ask for .jbc file */
    printf("Choose the .jbc file(Default:%s):", def_jbc);
    fgets(buf, sizeof(buf) - 1, stdin);
    if (strlen(buf) > 1) {
        memset(jbc, 0, sizeof(jbc));
        sscanf(buf, "%63s", jbc);
    } else {
        strncpy(jbc, def_jbc, strlen(def_jbc));
    }
    ERR_RET_COND(0 != access(jbc, R_OK), -(__LINE__), "Cannot access file '%s'\n", jbc);
    /* save for possible next run */
    memset(def_jbc, 0, sizeof(def_jbc));
    memcpy(def_jbc, jbc, sizeof(jbc));


    /* 1, ask for action */
    for(idx = 0; act_lst[idx]; idx++) {
        printf("    %d  %s\n", idx, act_lst[idx]);
    }
    printf("Choose the action:");
    fflush(stdout);
    chr = getchar();
    _DRAIN_STDIN();
    ERR_RET_COND(!(chr >= '0' && chr < '0' + idx), -(__LINE__), "Invalid choice '%c'\n", chr);
    snprintf(act, sizeof(act) - 1, "-a%s", act_lst[chr - '0']);

    /* 2, compose argv */
    idx = 0;
    arg[idx++] = "-v";
    arg[idx++] = act;
    arg[idx++] = jbc;

    /* 3, double confirm */
    for(idx = 0; arg[idx]; idx++) {
        printf("    argv[%d]:%s\n", idx, arg[idx]);
    }
    printf("Double confirm to do %s(y/N):", act + 2);
    fflush(stdout);
    chr = getchar();
    _DRAIN_STDIN();
    if (chr != 'y' && chr != 'Y' ) {
        printf("Abort.\n");
        return 0;
    }

    ret = jbi_main(idx, arg);
    if (ret != 0) {
        fprintf(stderr, "Failed to program CPLD\n");
    }

    /* per HW suggestion, delay 10s after program */
    sleep(10); 
    printf("***Warning: User need to power off machine for 60sec manually\n");
    return 0; 
}

/**********************************************************************
 *
 * Function: cpld_upgrade_from_cpld
 *
 * This function upgrades the NGVM Timing Card CPLD firmware from CPLD.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
#ifdef SLOW_UPGRADE
static long cpld_upgrade_from_cpld (void)
{
    return cpld_upgrade_firmware(UPGRADE_FROM_CPLD);
}
#endif


/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: highrise_cpld_diag.c,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.4  2020/12/09 07:29:50  alpeng
 * add function prologue; remove redundant header; adding ifdef for header files;
 *
 * Revision 1.1.4.3  2020/12/09 01:52:02  alpeng
 * use C comment 
 *
 * Revision 1.1.4.2  2020/11/25 07:35:59  alpeng
 *  clean up #if 0
 *
 * Revision 1.1.4.1  2020/09/10 06:03:16  alpeng
 * add board id check before launch diag
 *
 * Revision 1.1  2020/08/19 09:50:52  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.8  2014/04/22 06:06:02  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.7  2014/03/31 02:09:43  kodko
 * Check the CPLD firmware to see if it can support the speed up upgrade.
 *
 * Revision 1.1.2.6  2014/03/19 07:13:50  kodko
 * Speed up the CPLD firmware upgrade time under 2 minutes.
 *
 * Revision 1.1.2.5  2014/03/10 08:00:10  kodko
 * Remove redundant code.
 *
 * Revision 1.1.2.4  2014/03/07 07:39:58  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.3  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:05  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

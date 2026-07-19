/* $Id: platform_cpu.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/platform_cpu.c,v $
 *------------------------------------------------------------------
 *
 * platform_cpu.c
 * leverage from TSN linux_main.c and platform_cpu.c
 *
 * Copyright (c) 2019- by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cpu.h"
#include "hr_commn_util.h"


/* Extern functions or variable */
extern int quiet_launch;
extern int do_all_menu_items(struct menuinfo *);
extern int linux_cpu_core_test (int expected_cpu_core_number);
#ifndef HIGHRISE_TODO
extern int highrise_display_temp(void);
#endif
extern int highrise_mem_read32 (uint offset, uint *buf);
extern int highrise_mem_write32 (uint offset, uint wr_data);
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

/* Declare local functions */
int cpu_core_test (int do_more_test);
int build_cpu_test_menu (int db_test_items_executed);

/* Local variable */

static char *highrise_cpu_info[] = {
    "Processor",
    "BogoMIPS",
    "Features",
    "CPU implementer",
    "CPU architecture",
    "CPU revision",
    "Hardware",
};
static const uint size_of_highrise_cpu_info =
    sizeof(highrise_cpu_info) / sizeof(uchar *);




static submenu_xtable_t cpu_test_table[] = {
    {"CPU core test",
    (PFT) cpu_core_test, FALSE,
    MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
    (type_t(*)())0, 0,
    (PFT) build_cpu_test_menu, FALSE},

};

#define CPU_TEST_TABLE_SZ \
        (sizeof(cpu_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t cpu_pri_test_items[CPU_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t cpu_sec_test_items[CPU_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t cpu_test_menu = {
    "CPU Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    cpu_pri_test_items,
};
static menuinfo_t *cpu_test_menup = &cpu_test_menu;

/**********************************************************************
 *
 * Function: build_cpu_test_menu
 *
 * Description: Build CPU test menu.
 *
 * Inputs: db_test_items_executed - TRUE for do all of tests. FALSE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int build_cpu_test_menu (int db_test_items_executed)
{
    int rc = FAILED;
    char *tname = "CPU";

    testname(tname);

    build_primary_submenu(cpu_test_table,
                          CPU_TEST_TABLE_SZ, "CPU",
                          &cpu_test_menup);

    build_secondary_submenu(cpu_test_table,
                            CPU_TEST_TABLE_SZ,
                            cpu_sec_test_items);

    if (db_test_items_executed) {
        do_all_menu_items(&cpu_test_menu);
    } else {
        menu(&cpu_test_menu, cpu_sec_test_items, '\0');
    }

    return (rc);
}


/**************************************************************
 * Enhance Error Function
 * 1. Subtests of the test function will reuse all variables
 * 2. All variables will be cleared automatically when
 *    entering and leaving each menu item.
 * Segment 1: PID | Unique_string : slot_info
 *      fru_table_offset should be set, otherwise, it will not
 *      go to enhanced error message format in cterr()
 *      set fru_table_offset to get the predefine value
 *      or change mb_pid & mb_loc
 * Segment 2: Test step captured from prpass
 * Segment 3: Failure message captured from cterr
 * Segment 4: Components used
 * Segment 5: register and memory dump
 * Segment 6: Platform Environment initialized here
 * Segment 7: Top 3 Debugging Steps
 **************************************************************/
static void add_cpu_core_err_report(void)
{
    fru_table_offset = MB;
    platform_fru_table[fru_table_offset].pid_string = mb_pid;
    platform_fru_table[fru_table_offset].location_string = mb_loc;

    cterr_add_component("Marvell Armada 7040", "ARM Cores", "L2 Cache");
#ifndef HIGHRISE_TODO
    cterr_add_env_dump((PFV)highrise_display_temp);
#else
    printf("[%s]:%d, TODO\n", __FUNCTION__, __LINE__);
#endif
    cterr_add_debug("Reference the kernel messages and contact the CPU vendor.");
}

/*******************************************************************************
 * Function: cpu_core_test
 *
 * Description : Test for stress of cpu core & memory.
 * Default command: "stress -c 400 -m 2 -t 1".
 *
 * Inputs: test option - auto or user assign
 *
 * Output: PASSED/FAILED
 *******************************************************************************
 */
int cpu_core_test (int do_more_test)
{
    if (get_enhance_err_flag()) {
        add_cpu_core_err_report();
    }

    char cmd[100], line[100], *ptr;
    unsigned int cpu_count = 400, mem_count = 2, sec = 1;
    int rc = FAILED;
    char *tname = "CPU core";
    FILE *fp;

    memset( line, 0x0, sizeof(line));

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (linux_cpu_core_test(HIGHRISE_CPU_NUM) != PASSED) {
        cterr('f', 0, "%s test failed.", tname);
        return (FAILED);
    }


    if (!quiet_launch) {
        prpass(testpass, "%s stress, ", tname);
    }
    if (do_more_test) {
        cpu_count =
            getdec_answer("\nEnter CPU-bound processes:", 400, 1, 400);
        mem_count =
            getdec_answer("Enter memory allocator process:", 2, 1, 6);
        sec = getdec_answer("Enter test time (seconds):", 1, 1, 60);
    }
    if (!quiet_launch) {
        prpass(testpass, "Starting stress test, ");
        prpass(testpass, "Stress testing (times: %d, process: %d, duration: %d sec.), ",
            cpu_count, mem_count, sec);
    }
    sprintf(cmd, "stress -c %d -m %d -t %d > %s 2>&1",
            cpu_count, mem_count, sec, CPU_STRESS_LOG);
    system(cmd);

    sprintf(cmd, "cat %s | grep run > %s", CPU_STRESS_LOG, CPU_STRESS_RLT);
    system(cmd);

    fp = fopen(CPU_STRESS_RLT, "r");
    if (fp != NULL) {
        fgets(line, sizeof(line), fp);
        if (do_more_test) {
            printf("\n%s \n", line);
        }
        if (line[0] == '\0') {
            printf("Line[0] == 0\n");
            rc = FAILED;
        }
        ptr = strstr(line, "successful");
        if (ptr != NULL) {
            rc = PASSED;
        }
        fclose(fp);
    } else {
        cterr('f', 0, "%s test failed.", tname);
        rc = FAILED;
    }

    unlink(CPU_STRESS_LOG);
    unlink(CPU_STRESS_RLT);

    if ((rc == PASSED) && (quiet_launch != 1)) {
        prpass(testpass, "%s test passed, ", tname);
    }

    return (rc);

}

/*******************************************************************************
 *
 * Function    : highrise_get_cpu_ondie_temp
 * Description : Function to get CPU on-die temperature in degree C.
 * Inputs      :
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

int highrise_get_cpu_ondie_temp(const char *ap_cp, float *temp_c)
{
    unsigned int rega  = 0;
    uint32_t     regc0 = 0;
    uint32_t     regc1 = 0;
    uint32_t     regst = 0;

    if (strcmp(ap_cp, "ap") == 0) {
        rega = CPU_AP_ONDIE_TEMP_REG_CTL0;
    }
    else if (strcmp(ap_cp, "cp") == 0) {
        rega = CPU_CP_ONDIE_TEMP_REG_CTL0;
    }
    else {
        return -1;
    }
    ERR_RET_COND(PASSED != highrise_mem_read32(rega, &regc0), -(__LINE__), "Failed read reg:0x%08x\n", rega);
    set_bits32(&regc0, 1, CPU_ONDIE_TEMP_TSN_OSR_OFF  , CPU_ONDIE_TEMP_TSN_OSR_WIDTH  );
    set_bits32(&regc0, 1, CPU_ONDIE_TEMP_TSN_ENB_OFF  , CPU_ONDIE_TEMP_TSN_ENB_WIDTH  );
    set_bits32(&regc0, 0, CPU_ONDIE_TEMP_TSN_RST_OFF  , CPU_ONDIE_TEMP_TSN_RST_WIDTH  );
    set_bits32(&regc0, 0, CPU_ONDIE_TEMP_TSN_START_OFF, CPU_ONDIE_TEMP_TSN_START_WIDTH);     /*stop it first */
    ERR_RET_COND(PASSED != highrise_mem_write32(rega, regc0), -(__LINE__), "Failed write reg:0x%08x\n", rega);

    ERR_RET_COND(PASSED != highrise_mem_read32(rega + 4, &regc1), -(__LINE__), "Failed read reg:0x%08x\n", rega + 4);
    set_bits32(&regc1, 6, CPU_ONDIE_TEMP_TSN_SEN_SEL_OFF, CPU_ONDIE_TEMP_TSN_SEN_SEL_OFF);   /*select the max temp */
    ERR_RET_COND(PASSED != highrise_mem_write32(rega, regc1), -(__LINE__), "Failed write reg:0x%08x\n", rega + 4);

    set_bits32(&regc0, 1, CPU_ONDIE_TEMP_TSN_START_OFF, CPU_ONDIE_TEMP_TSN_START_WIDTH);     /*start it */
    ERR_RET_COND(PASSED != highrise_mem_write32(rega, regc0), -(__LINE__), "Failed write reg:0x%08x\n", rega);

    usleep(100000); 

    ERR_RET_COND(PASSED != highrise_mem_read32(rega + 8, &regst), -(__LINE__), "Failed read reg:0x%08x\n", rega + 8);
    regst >>= CPU_ONDIE_TEMP_TSN_READOUT_OFF;
    regst  &= (1 << CPU_ONDIE_TEMP_TSN_READOUT_WIDTH) - 1;
    if (regst & (1 << (CPU_ONDIE_TEMP_TSN_READOUT_WIDTH - 1))) { /* if negative, extend the sign bit */
        *temp_c = (-1.0) * (((~regst) + 1) & ((1 << (CPU_ONDIE_TEMP_TSN_READOUT_WIDTH - 1)) - 1));
    } else {
        *temp_c = 1.0 * regst;
    }

    *temp_c = (*temp_c) * 0.423 + 150;
    return 0;
}


/*******************************************************************************
 *
 * Function    : highrise_cpu_ondie_temp
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */

static int show_thermal(const char *file, const float *temp, const char *tag)
{
    char buf[32] = {[0 ... sizeof(buf)-1] = 0};
    int fd = 0;
    float therm = 0.0;

    if (file) {
        ERR_RET_COND(0 > (fd = open(file, O_RDONLY)), FAILED, "Open %s failed.\n", file);
        ERR_RET_COND(0 >= read(fd, buf, sizeof(buf) - 1) && (1 | close(fd)), FAILED, "Read %s failed.\n", file);

        therm  = atoi(&buf[0]);
        therm /= 1000;
        close(fd);
    }
    else if (temp) {
        therm = *temp;
    }
    printf("%-16s: %.3f Celcius\n", tag, therm);

    return PASSED;
}

int highrise_cpu_ondie_temp (int opt)
{
    float temp = 0.0;
    if (access("/sys/class/thermal/thermal_zone0/temp", R_OK) == 0 &&
        access("/sys/class/thermal/thermal_zone1/temp", R_OK) == 0) {
        ERR_RET_COND(PASSED != show_thermal("/sys/class/thermal/thermal_zone0/temp", NULL, "CPU AP Temp"), FAILED, "Failed.\n");
        ERR_RET_COND(PASSED != show_thermal("/sys/class/thermal/thermal_zone1/temp", NULL, "CPU SB Temp"), FAILED, "Failed.\n");
        return PASSED;
    }

    ERR_RET_COND(highrise_get_cpu_ondie_temp("ap", &temp) < 0, FAILED, "Failed.\n");
    ERR_RET_COND(PASSED != show_thermal(NULL, &temp, "CPU AP Temp"), FAILED, "Failed.\n");
    return PASSED;
}




/*****************************************************************************
 *
 * Function   : highrise_get_info
 * Description: Get TSN system Info by reading related linux file.
 * Inputs     : info_file - related Linux file to get info
 *              info_item - the item to collect
 *              info_item_size - size of items that needs to collect
 *              file_name - file to put the collected info
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int highrise_get_info(char *info_file, char **info_item,
                         int info_item_size, char *file_name)
{
    int index = 0;
    char sys_cmd[256];

    /*
     * Read out the needed Info from related Linux info_file
     */
    for (index = 0; index < info_item_size; index++) {
        sprintf(sys_cmd, "cat %s | grep -m 1 '%s' >> %s",
                info_file, info_item[index], file_name);
        system(sys_cmd);
    }
    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : highrise_get_cpucore
 * Description: To show CPU core number by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int highrise_get_cpucore( char *file_name )
{
    char sys_cmd[256];
    char sys_cpucore[3];

    if ((ExecuteCmdbyPopen ("cat /proc/cpuinfo | grep processor | wc -l",
        sys_cpucore, HIGHRISE_INFO_BUF_SIZE)) == 0) {
        cterr('f',0,"get CPU core failed!!\n");
        return (FAILED);
    }

    sprintf(sys_cmd, "echo \"CPU cores\t: %d\" >> %s", atoi(sys_cpucore), file_name);
    system(sys_cmd);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : highrise_show_cpuinfo
 * Description: To show TSN CPU Info by reading file "/proc/cpuinfo".
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
int highrise_show_cpuinfo (void)
{
    int rc = FAILED;
    char cpu_info[HIGHRISE_INFO_BUF_SIZE];
    FILE *fp;
    uint reg_offset = 0, reg_val = 0;
    reg_offset = (uint)(CPU_AP_REG_BASE + CPU_SAR_REG);

    fp = fopen(HIGHRISE_CPU_INFO_FILE, "r");
    if (fp == NULL) {
        rc = highrise_get_info("/proc/cpuinfo", highrise_cpu_info,
                           size_of_highrise_cpu_info, HIGHRISE_CPU_INFO_FILE);
        if (rc != PASSED) {
            printf("Failed to get CPU information. \n");
            return (rc);
        } else {
            fp = fopen(HIGHRISE_CPU_INFO_FILE, "r");
            if (fp == NULL) {
                printf("Failed to open %s. \n", HIGHRISE_CPU_INFO_FILE);
                return (FAILED);
            }
        }
    }
    highrise_get_cpucore(HIGHRISE_CPU_INFO_FILE);

    while (fgets(cpu_info, HIGHRISE_INFO_BUF_SIZE, fp) != NULL) {
        printf("%s", cpu_info);
    }

    fclose(fp);
    unlink(HIGHRISE_CPU_INFO_FILE);

    /* Show currently CPU on Die temperature */
    if (highrise_cpu_ondie_temp(0) != PASSED) {
        return (FAILED);
    }

    /* Read CPU clock frequencies for SAR register */
    if (highrise_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val = 0x%08X.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* get clock frequencies from register value */
    reg_val = (uint)(reg_val & CPU_SAR_RST2_FREQ_MASK);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val after mask = 0x%08X.\n",
               __FUNCTION__, __LINE__, reg_val);
    }

    switch (reg_val) {
        case CPU_1200_DDR_800_RCLK_800:
            printf("%-16s: 0x%08x(%s)\n", "CPU Freq", reg_val, "CPU_1200_DDR_800_RCLK_800");
            break;
        case CPU_1400_DDR_800_RCLK_800:
            printf("%-16s: 0x%08x(%s)\n", "CPU Freq", reg_val, "CPU_1400_DDR_800_RCLK_800");
            break;
        case CPU_600_DDR_800_RCLK_800:
            printf("%-16s: 0x%08x(%s)\n", "CPU Freq", reg_val, "CPU_600_DDR_800_RCLK_800");
            break;
        case CPU_800_DDR_800_RCLK_800:
            printf("%-16s: 0x%08x(%s)\n", "CPU Freq", reg_val, "CPU_800_DDR_800_RCLK_800");
            break;
        case CPU_1000_DDR_800_RCLK_800:
            printf("%-16s: 0x%08x(%s)\n", "CPU Freq", reg_val, "CPU_1000_DDR_800_RCLK_800");
            break;
        default:
            printf("\n *** Err! CPU freg incorrect, reg_val = 0x%08X\n", reg_val);
            return (FAILED);
            break;
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: platform_cpu.c,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.3  2020/12/09 06:35:02  alpeng
 * add cvs log field
 *
 *
 */

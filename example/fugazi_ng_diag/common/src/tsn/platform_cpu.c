/* $Id: platform_cpu.c,v 1.9 2019/01/18 05:54:46 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_cpu.c,v $
 *------------------------------------------------------------------
 *
 * platform_cpu.c
 *
 * Copyright (c) 2014-2019 by cisco Systems, Inc.
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
#include "tsn_comm.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_fpga.h"
#include "proto.h"
#include "platform_cpu.h"
#include "plug_host_fpga_lib.h"
#include "plat_defs.h"
#include "linux_coretest.h"


#define CPU_STRESS_LOG   "/tmp/cpu_core_log"
#define CPU_STRESS_RLT   "/tmp/cpu_core_rlt"
#define ENHANCE_ERROR_MSG_RDY 1

#define POLLING_INTRVL 100 
#define MAX_POLLING_COUNTS 100

/*
 * Declare local function
 */
int tsn_get_cpu_ondie_temp(int *);
int tsn_confirm_devbus_config(int);
int tsn_get_devbus_baseaddr(int, uint *);

/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);
extern int build_cpu_test_menu (int);
extern int cpu_core_test(int);
extern int tsn_display_temp_errormsg(void);

/*
 * Global variables
 */
extern int quiet_launch;
uint tsn_fpga_reg_baseaddr = 0;
uint tsn_aikido_reg_baseaddr = 0;

/* Table of device bus configs */
static devbus_conf_t tsn_devbus_conf_tbl[] = {
    {"devbus CS0(to FPGA)",        TSN_DEVBUS_0,
     STAR_DEVBUS0_RD_PARAM,         TSN_DEVBUS0_WR_PARAM,
     TSN_DEVBUS_ACT_LOW,           TSN_DEVBUS_READY_IGNORED},
    {"devbus CS1(to Aikido FPGA)", TSN_DEVBUS_1,
     TSN_DEVBUS1_RD_PARAM,         TSN_DEVBUS1_WR_PARAM,
     TSN_DEVBUS_ACT_LOW,           TSN_DEVBUS_READY_IGNORED},
};

/*
 * CPU Test Menu
 */

static submenu_xtable_t cpu_test_table[] = {
    {"CPU core test", (type_t(*)())cpu_core_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
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

/*******************************************************************************
 *
 * Function    : tsn_cpu_ondie_temp
 * Description : Function to get CPU on die temperature in degree C.
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int tsn_cpu_ondie_temp (int opt)
{
    int cpu_temp = 0;

    if (tsn_get_cpu_ondie_temp(&cpu_temp) != PASSED) {
        printf("%s: Failed to read CPU on-die temperature.\n", __FUNCTION__);
        return (FAILED);
    }
    printf("Current CPU on-die Temp. = %d degree C.\n", cpu_temp);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_get_cpu_ondie_temp
 * Description : Function to get CPU on-die temperature in degree C.
 * Inputs      : 
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int tsn_get_cpu_ondie_temp (int *temp_c)
{
    uint reg_offset = 0, reg_val = 0;
    int  temp = 0;

    reg_offset = (uint)(CPU_AP_REG_BASE + CPU_ONDIE_TEMP_REG);
 
    /* Read CPU thermal sensor register */
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, reg_offset);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val = 0x%08X.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* Count temperature from register value */
    reg_val = (uint)((reg_val & CPU_THERM_TEMP_MASK) >> CPU_THERM_TEMP_OFFSET);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val after mask = 0x%08X.\n",
               __FUNCTION__, __LINE__, reg_val);
    }

    if (reg_val >= CPU_THERM_OUTPUT_MSB) {
        reg_val -= CPU_THERM_OUTPUT_COMP;
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val after check = 0x%08X.\n",
               __FUNCTION__, __LINE__, reg_val);
    }

    temp = ((((int)reg_val * CPU_THERM_GAIN) + CPU_THERM_OFFSET) / CPU_THERM_DIV);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d temp = %d degree C.\n", __FUNCTION__, __LINE__, temp);
    }
    *temp_c = temp;

    return (PASSED);
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
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "ARM Cores", "L2 Cache");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Reference the kernel messages and contact the CPU vendor.");
#endif

    char cmd[100], line[100], *ptr;
    unsigned int cpu_count = 400, mem_count = 2, sec = 1;
    int rc = FAILED;
    char *tname = "CPU core";
    FILE *fp;

    memset( line, 0x0, sizeof(line));

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (linux_cpu_core_test(TSN_CPU_NUM) != PASSED) {
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
        cterr('f', 0, "%s test failed.");
        rc = FAILED; 
    }

    unlink(CPU_STRESS_LOG);
    unlink(CPU_STRESS_RLT);

    if ((rc == PASSED) && (quiet_launch != 1)) {  
        prpass(testpass, "%s test passed, ", tname);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

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

/*******************************************************************************
 *
 * Function    : tsn_devbus_init
 * Description : Function to init TSN CPU device bus.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_devbus_init (int opt)
{
    int  curr_bus = 0;
    int  start_num = TSN_DEVBUS_0, end_num = TSN_DEVBUS_1;
    uint base_addr[2] = {0, 0};
    devbus_conf_t *devbus_conf_p = 0;

    for (curr_bus = start_num; curr_bus <= end_num; curr_bus++) {
        /* Confirm device bus read and write parameters config. */   
        if (tsn_confirm_devbus_config(curr_bus) != PASSED) {
            printf("%s(%d): DevBus_CS%d configuration is incorrect.\n",
                   __FUNCTION__, __LINE__, curr_bus);
            return (FAILED);
        }

        /* Get base addr. of device bus. */
        if (tsn_get_devbus_baseaddr(curr_bus, &base_addr[curr_bus]) != PASSED) {
            printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
                   __FUNCTION__, __LINE__, curr_bus);
            return (FAILED);
        }
    }

    /* Set device bus window base addr. info */
    tsn_fpga_reg_baseaddr = base_addr[TSN_FPGA_DEVBUS_NUM];
    tsn_aikido_reg_baseaddr = base_addr[TSN_AIKIDO_DEVBUS_NUM];

    if (this_is_star()) {
        printf("Device bus init with Star FPGA timing parameter.\n");
    } else if (this_is_supernova()) {
        printf("Device bus init with Supernova FPGA timing parameter.\n");
    } else {
        devbus_conf_p = &tsn_devbus_conf_tbl[0];
        devbus_conf_p->rd_param = TSN_DEVBUS0_RD_PARAM; 
        for (curr_bus = start_num; curr_bus <= end_num; curr_bus++) {
            /* Confirm device bus read and write parameters config. */   
            if (tsn_confirm_devbus_config(curr_bus) != PASSED) {
                printf("%s(%d): DevBus_CS%d configuration is incorrect.\n",
                       __FUNCTION__, __LINE__, curr_bus);
                return (FAILED);
            }

            /* Get base addr. of device bus. */
            if (tsn_get_devbus_baseaddr(curr_bus, &base_addr[curr_bus]) != PASSED) {
                printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
                       __FUNCTION__, __LINE__, curr_bus);
                return (FAILED);
            }
        }

        /* Set device bus window base addr. info */
        tsn_fpga_reg_baseaddr = base_addr[TSN_FPGA_DEVBUS_NUM];
        tsn_aikido_reg_baseaddr = base_addr[TSN_AIKIDO_DEVBUS_NUM];
        printf("Device bus init with TSN FPGA timing parameter.\n");
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_confirm_devbus_config
 * Description : Function to confirm TSN CPU device bus configs.
 * Inputs      : bus_num - device bus number
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_confirm_devbus_config (int bus_num)
{
    uint          reg_offset = 0, reg_val = 0, cmp_data = 0;
    uint          wr_in = 0, conf_msk = 0;
    devbus_conf_t *devbus_conf_p = 0;
    int           tsn_used_devbus = 0;

    tsn_used_devbus = (sizeof(tsn_devbus_conf_tbl) / sizeof(devbus_conf_t));
    if (bus_num >= tsn_used_devbus) {
        printf("%s(%d): Invalid device bus number(%d) for TSN.\n",
               __FUNCTION__, __LINE__, bus_num);
        return (FAILED);
    }

    devbus_conf_p = &tsn_devbus_conf_tbl[bus_num];

    /* Config DEV_CS[x] Read Parameters Reg. */
    reg_offset = (uint)DEVBUS_RD_PARAMS_REG_ADDR(bus_num);
    cmp_data = (uint)(devbus_conf_p->rd_param);
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d Read parameters(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    if (reg_val != cmp_data) {
        wr_in = cmp_data;
        if (tsn_mem_write32(reg_offset, wr_in) != PASSED) {
            printf("%s(%d): Failed to write DevBus_CS%d Read parameters"
                   "(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }

        msleep(TSN_DEVBUS_CONF_TIME);

        reg_val = 0;
        if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read DevBus_CS%d "
                   "Read parameters(@0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset);
            return (FAILED);
        }

        if (reg_val != cmp_data) {
            printf("%s(%d): Failed to set DevBus_CS%d "
                   "Read parameter(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }
    }

    /* Config DEV_CS[x] Write Parameters Reg. */
    reg_offset = (uint)DEVBUS_WR_PARAMS_REG_ADDR(bus_num);
    cmp_data = (uint)(devbus_conf_p->wr_param);
    reg_val = 0;
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d Write parameters(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    if (reg_val != cmp_data) {
        wr_in = cmp_data;
        if (tsn_mem_write32(reg_offset, wr_in) != PASSED) {
            printf("%s(%d): Failed to write DevBus_CS%d Write parameters"
                   "(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }

        msleep(TSN_DEVBUS_CONF_TIME);

        reg_val = 0;
        if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read DevBus_CS%d "
                   "Write parameters(@0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset);
            return (FAILED);
        }

        if (reg_val != cmp_data) {
            printf("%s(%d): Failed to set DevBus_CS%d "
                   "Write parameter(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }
    }

    /* Config Device Bus Sync Control Reg. */
    reg_offset = (uint)DEVBUS_SYNC_CTRL_REG_ADDR;
    cmp_data = (uint)((devbus_conf_p->polarity << DBSCR_POLAR_SHIFT(bus_num)) |
                      (devbus_conf_p->ignore << DBSCR_IGNORE_SHIFT(bus_num)));
    conf_msk = (uint)(DBSCR_POLAR_MSK << DBSCR_POLAR_SHIFT(bus_num) |
                      DBSCR_IGNORE_MSK << DBSCR_IGNORE_SHIFT(bus_num));
    reg_val = 0;
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d Sync Control(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    if ((reg_val & conf_msk) != cmp_data) {
        wr_in = (uint)((reg_val & (uint)(~conf_msk)) | cmp_data);
        if (tsn_mem_write32(reg_offset, wr_in) != PASSED) {
            printf("%s(%d): Failed to write DevBus_CS%d"
                   " Sync Control(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }

        msleep(TSN_DEVBUS_CONF_TIME);

        reg_val = 0;
        if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read DevBus_CS%d Sync Control(@0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset);
            return (FAILED);
        }

        if (reg_val != wr_in) {
            printf("%s(%d): Failed to set DevBus_CS%d "
                   "Sync Control(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
                return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_get_devbus_baseaddr
 * Description : Function to get TSN device bus base address.
 * Inputs      : bus_num    - device bus number
 *               *base_addr - buffer to put the read back base addr.
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_get_devbus_baseaddr (int bus_num, uint *base_addr)
{
    uint reg_offset = 0, reg_val = 0;

    reg_offset = (uint)BRIDGE_WIN_BASE_REG_ADDR(bus_num);
    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d window base(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    *base_addr = (uint)(reg_val & (uint)DEVBUS_WINBASE_MSK); 

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_devbus_info
 * Description : Function to show TSN CPU device bus info.
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
void tsn_show_devbus_info (void)
{
    printf("TSN/STAR/SUPERNOVA FPGA(devbus_CS%d) base addr.: 0x%08X\n",
           TSN_FPGA_DEVBUS_NUM, tsn_fpga_reg_baseaddr);
    printf("TSN/STAR/SUPERNOVA Aikido FPGA(devbus_CS%d) base addr.: 0x%08X\n",
           TSN_AIKIDO_DEVBUS_NUM, tsn_aikido_reg_baseaddr);
}

/*******************************************************************************
 *
 * Function   : tsn_cpu_mac_check_linkstat
 * Description: Function to confirm CPU MAC Link state.
 *              By confirm CPU Port Status Register0 bit 0: UP(1) / DOWN(0).
 * Inputs     : mac_num - CPU GEMAC port number(port0 to 3)
 *              link_opt - to confirm link up(1) / down(0)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_cpu_mac_check_linkstat (int mac_num, boolean link_opt)
{
    uint reg_addr = CPU_PORT_STATUS_REG0(mac_num);
    uint reg_val = 0, chk_val = CPU_PSR0_LINKUP;
    int ctr = 0;
    int polling_result = FAILED;

    if (link_opt == CPUMAC_LINKDOWN) {
        chk_val = 0;
    }

    for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
        reg_val = 0;
        if (tsn_mem_read32(reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read CPU register 0x%08X.\n",
                   __func__, reg_addr);
            return (FAILED);
        }

        if ((reg_val & CPU_PSR0_LINKUP) == chk_val) {
            polling_result = PASSED;
            break;
        }
        msleep(POLLING_INTRVL);
    }

    if (polling_result != PASSED) {
        printf("%s: TIMEMOUT! But CPU MAC%d link is still %s.\n",
               __func__, mac_num, ((link_opt == CPUMAC_LINKUP) ? "down" : "up"));
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: CPU MAC%d link is %s.\n",
               __func__, mac_num, ((link_opt == CPUMAC_LINKUP) ? "up" : "down"));
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_cpu_mac_config
 * Description: Function to config CPU GEMAC port.
 * Inputs     : mac_num - CPU GEMAC port number(port0 to 3)
 *              reg_val - Configure value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_cpu_mac_config (int mac_num, uint reg_val)
{
    uint reg_addr = CPU_PORT_AN_CONF_REG(mac_num);
    uint read_back = 0;
    uint  msk_val = (uint)(~(CPU_PACR_F_LINKUP | CPU_PACR_F_LINKDOWN));
    int ctr = 0, polling_result = FAILED;

    /* Force CPU GEMAC port link down for configure. */
    if (tsn_mem_write32(reg_addr, (uint)PANCR_FORCE_LINK_DOWN) != PASSED) {
        printf("%s(%d): Failed to write CPU GEMAC%d reg. 0x%08X.\n",
               __func__, __LINE__, mac_num, reg_addr);
        return (FAILED);
    }

    /* Confirm CPU GEMAC is link down */
    if (tsn_cpu_mac_check_linkstat(mac_num, CPUMAC_LINKDOWN) != PASSED) {
        printf("%s: Failed to force CPU GEMAC%d Link down.\n",
               __func__, mac_num);
        return (FAILED);
    }

    msleep(200);

    /* Configure CPU GEMAC port. */
    if (tsn_mem_write32(reg_addr, reg_val) != PASSED) {
        printf("%s(%d): Failed to write CPU GEMAC%d reg. 0x%08X",
               __func__, __LINE__, mac_num, reg_addr);
        return (FAILED);
    }

    msleep(200);

    /* Confirm the setup by read it back */
    for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
        read_back = 0;
        if (tsn_mem_read32(reg_addr, &read_back) != PASSED) {
            printf("%s: Failed to read CPU register 0x%08X.\n",
                   __func__, reg_addr);
            return (FAILED);
        }

        if ((read_back & msk_val) == (reg_val & msk_val)) {
            polling_result = PASSED;
            break;
        }
        msleep(POLLING_INTRVL);
    }

    if (polling_result != PASSED) {
        printf("%s: Failed to set CPU GEMAC%d as 0x%08X.\n",
               __func__, mac_num, reg_val);
        return (FAILED);
    }

    msleep(200);

    return (PASSED);
}


/*-------------------------------------------------
$Log: platform_cpu.c,v $
Revision 1.9  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.8  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.7.16.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.7  2018/07/10 00:28:08  lucywang
Enhanced CPU core test to  make sure each CPU core is activated

Revision 1.6  2018/04/15 22:03:30  palin2
Merged Vulcan back to maintrunk.

Revision 1.5  2018/02/27 07:21:04  hondwang
Fix Star platform ondie temp in low temperature check issue

Revision 1.4  2018/02/12 09:13:46  hondwang
merge Star CPU frequency check into main trunk

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.4  2018/02/12 08:39:01  hondwang
Add CPU frequency check

Revision 1.2.4.3  2017/12/11 08:45:21  hondwang
Remove tsn_confirm_devbus_config hard code for Star

Revision 1.2.4.2  2017/11/22 09:45:46  hondwang
Fix demo SKU and menu show

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/25 08:31:55  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.5.6.2  2017/07/07 10:20:50  hondwang
Fix Makefile and device bus config issue

Revision 1.1.4.5.6.1  2017/06/23 02:20:18  tirawan
Upload Star Second FPGA read parameter if this platform is Star with Pluggable module and correct LTE reset initialization

Revision 1.1.4.5.2.1  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.5  2016/11/29 02:54:39  palin2
Dynamically getting device bus window base from CPU register.

Revision 1.1.4.4  2016/11/01 07:29:20  petteng
Add enhanced error message

Revision 1.1.4.3  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.4  2016/06/12 18:02:39  palin2
Updated CPU on-die temperature sensor read function.

Revision 1.1.2.3  2016/06/03 01:00:46  palin2
Added function to show CPU on die temperature.

Revision 1.1.2.2  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.1  2016/03/25 08:05:40  steja
Add CPU Stress tools



*/

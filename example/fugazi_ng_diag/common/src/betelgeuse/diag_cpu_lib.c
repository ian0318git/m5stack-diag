/* $Id: diag_cpu_lib.c,v 1.2 2019/01/10 06:36:21 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_cpu_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_cpu_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_moka_fpga_lib.h"
#include "proto.h"
#include "diag_cpu_lib.h"
#include "linux_coretest.h"
#include "diag_temp_sensor_util.h"
#include "diag_cpu_lib.h"

/*
 * Declare local function
 */
int plat_confirm_devbus_config(int);
int plat_get_devbus_baseaddr(int, uint *);

/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);
extern int build_cpu_test_menu (int);
extern int show_plat_curr_temps(void);

/*
 * Global variables
 */
extern int quiet_launch;
uint plat_fpga_reg_baseaddr = 0;
uint plat_aikido_reg_baseaddr = 0;

/* Table of device bus configs */
static devbus_conf_t plat_devbus_conf_tbl[] = {
    {"devbus CS0(to FPGA)",        PLAT_DEVBUS_0,
     PLAT_DEVBUS0_RD_PARAM,         PLAT_DEVBUS0_WR_PARAM,
     PLAT_DEVBUS_ACT_LOW,           PLAT_DEVBUS_READY_IGNORED},
    {"devbus CS1(to Aikido FPGA)", PLAT_DEVBUS_1,
     PLAT_DEVBUS1_RD_PARAM,         PLAT_DEVBUS1_WR_PARAM,
     PLAT_DEVBUS_ACT_LOW,           PLAT_DEVBUS_READY_IGNORED},
};

/*******************************************************************************
 *
 * Function    : plat_get_cpu_ondie_temp
 * Description : Function to get CPU on-die temperature in degree C.
 * Inputs      : 
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int plat_get_cpu_ondie_temp (int *temp_c)
{
    uint reg_offset = 0, reg_val = 0;
    int  temp = 0;

    reg_offset = (uint)(CPU_AP_REG_BASE + CPU_ONDIE_TEMP_REG);
 
    /* Read CPU thermal sensor register */
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
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
 *
 * Function    : show_cpu_ddr_freq
 * Description : Function to show CPU and DDR frequency.
 * Inputs      : 
 * Outputs     : PASSED/FAILED
 *
 ******************************************************************************* 
 */
int show_cpu_ddr_freq (void) {
    uint reg_offset = 0, reg_val = 0;
    reg_offset = (uint)(CPU_AP_REG_BASE + CPU_SAR_REG);

    /* Read CPU clock frequencies for SAR register */
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
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

    /*
     *         CPU Freq.(MHz) | DDR Freq.(MHz)
     *   0x04: 1600           | 800
     *   0x1a: 1400           | 800
     *   0x19: 1200           | 800
     *   0x1d: 1000           | 800
     *   0x1c: 800            | 800
     *   0x1b: 600            | 800
     *
     */
    switch (reg_val) {
        case CPU_FREQ_1600_RAM_FREQ_800:
            printf("\nCPU Freq.: 1600 MHz\n");
            printf("SDRAM Freq.: 800 MHz\n");
            break;
        case CPU_FREQ_1400_RAM_FREQ_800:
            printf("\nCPU Freq.: 1400 MHz\n");
            printf("SDRAM Freq.: 800 MHz\n");
            break;
        case CPU_FREQ_1200_RAM_FREQ_800:
            printf("\nCPU Freq.: 1200 MHz\n");
            printf("SDRAM Freq.: 800 MHz\n");
            break;
        case CPU_FREQ_1000_RAM_FREQ_800:
            printf("\nCPU Freq.: 1000 MHz\n");
            printf("SDRAM Freq.: 800 MHz\n");
            break;
        case CPU_FREQ_800_RAM_FREQ_800:
            printf("\nCPU Freq.: 800 MHz\n");
            printf("SDRAM Freq.: 800 MHz\n");
            break;
        case CPU_FREQ_600_RAM_FREQ_800:
            printf("\nCPU Freq.: 600 MHz\n");
            printf("SDRAM Freq.: 800 MHz\n");
            break;
        default:
            printf("\nCPU Freq.: unknown\n");
            printf("SDRAM Freq.: unknown\n");
            printf("%s:%d: unknown reg_val = 0x%x\n", __FUNCTION__, __LINE__, reg_val);
            break;
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_devbus_init
 * Description : Function to init CPU device bus.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_devbus_init (int opt)
{
    int  curr_bus = 0;
    int  start_num = PLAT_DEVBUS_0, end_num = PLAT_DEVBUS_1;
    uint base_addr[2] = {0, 0};

    for (curr_bus = start_num; curr_bus <= end_num; curr_bus++) {
        /* Confirm device bus read and write parameters config. */   
        if (plat_confirm_devbus_config(curr_bus) != PASSED) {
            printf("%s(%d): DevBus_CS%d configuration is incorrect.\n",
                   __FUNCTION__, __LINE__, curr_bus);
            return (FAILED);
        }

        /* Get base addr. of device bus. */
        if (plat_get_devbus_baseaddr(curr_bus, &base_addr[curr_bus]) != PASSED) {
            printf("%s(%d): Failed to get DevBus_CS%d base addr.\n",
                   __FUNCTION__, __LINE__, curr_bus);
            return (FAILED);
        }
    }

    /* Set device bus window base addr. info */
    plat_fpga_reg_baseaddr = base_addr[PLAT_FPGA_DEVBUS_NUM];
    plat_aikido_reg_baseaddr = base_addr[PLAT_AIKIDO_DEVBUS_NUM];

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_confirm_devbus_config
 * Description : Function to confirm CPU device bus configs.
 * Inputs      : bus_num - device bus number
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_confirm_devbus_config (int bus_num)
{
    uint          reg_offset = 0, reg_val = 0, cmp_data = 0;
    uint          wr_in = 0, conf_msk = 0;
    devbus_conf_t *devbus_conf_p = 0;
    int           plat_used_devbus = 0;

    plat_used_devbus = (sizeof(plat_devbus_conf_tbl) / sizeof(devbus_conf_t));
    if (bus_num >= plat_used_devbus) {
        printf("%s(%d): Invalid device bus number(%d).\n",
               __FUNCTION__, __LINE__, bus_num);
        return (FAILED);
    }

    devbus_conf_p = &plat_devbus_conf_tbl[bus_num];

    /* Config DEV_CS[x] Read Parameters Reg. */
    reg_offset = (uint)DEVBUS_RD_PARAMS_REG_ADDR(bus_num);
    cmp_data = (uint)(devbus_conf_p->rd_param);
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d Read parameters(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    if (reg_val != cmp_data) {
        wr_in = cmp_data;
        if (plat_mem_write32(reg_offset, wr_in) != PASSED) {
            printf("%s(%d): Failed to write DevBus_CS%d Read parameters"
                   "(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }

        msleep(PLAT_DEVBUS_CONF_TIME);

        reg_val = 0;
        if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
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
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d Write parameters(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    if (reg_val != cmp_data) {
        wr_in = cmp_data;
        if (plat_mem_write32(reg_offset, wr_in) != PASSED) {
            printf("%s(%d): Failed to write DevBus_CS%d Write parameters"
                   "(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }

        msleep(PLAT_DEVBUS_CONF_TIME);

        reg_val = 0;
        if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
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
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d Sync Control(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    if ((reg_val & conf_msk) != cmp_data) {
        wr_in = (uint)((reg_val & (uint)(~conf_msk)) | cmp_data);
        if (plat_mem_write32(reg_offset, wr_in) != PASSED) {
            printf("%s(%d): Failed to write DevBus_CS%d"
                   " Sync Control(@0x%08X: 0x%08X).\n",
                   __FUNCTION__, __LINE__, bus_num, reg_offset, wr_in);
            return (FAILED);
        }

        msleep(PLAT_DEVBUS_CONF_TIME);

        reg_val = 0;
        if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
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
 * Function    : plat_get_devbus_baseaddr
 * Description : Function to get device bus base address.
 * Inputs      : bus_num    - device bus number
 *               *base_addr - buffer to put the read back base addr.
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_get_devbus_baseaddr (int bus_num, uint *base_addr)
{
    uint reg_offset = 0, reg_val = 0;

    reg_offset = (uint)BRIDGE_WIN_BASE_REG_ADDR(bus_num);
    if (plat_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read DevBus_CS%d window base(@0x%08X).\n",
               __FUNCTION__, __LINE__, bus_num, reg_offset);
        return (FAILED);
    }

    *base_addr = (uint)(reg_val & (uint)DEVBUS_WINBASE_MSK); 

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_cpu_mac_check_linkstat
 * Description: Function to confirm CPU MAC Link state.
 *              By confirm CPU Port Status Register0 bit 0: UP(1) / DOWN(0).
 * Inputs     : mac_num - CPU GEMAC port number(port0 to 3)
 *              link_opt - to confirm link up(1) / down(0)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_cpu_mac_check_linkstat (int mac_num, boolean link_opt)
{
    uint reg_addr = CPU_PORT_STATUS_REG0(mac_num);
    uint reg_val = 0, chk_val = CPU_PSR0_LINKUP;
    int ctr = 0;
    int polling_result = FAILED;

    if (link_opt == CPUMAC_LINKDOWN) {
        chk_val = 0;
    }

    /* read CPU offset: 0xF2130E10 + (m*0x1000), Bit[0] */
    for (ctr = 0; ctr < MAX_POLLING_COUNTS; ctr++) {
        reg_val = 0;
        if (plat_mem_read32(reg_addr, &reg_val) != PASSED) {
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
 * Function    : plat_mem_read32
 * Description : Function to read memory by byte.
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_mem_read32 (uint offset, uint *buf)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;
 
    fd = open("/dev/mem", (O_RDONLY | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *buf = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plat_mem_write32
 * Description : Function performs write memory by byte.
 * Inputs      : offset  - offset
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_mem_write32 (uint offset, uint wr_data)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;
 
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_cpu_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:21  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

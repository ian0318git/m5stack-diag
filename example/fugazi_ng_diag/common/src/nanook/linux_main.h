 /* $Id: linux_main.h,v 1.2 2019/12/11 10:10:32 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/linux_main.h,v $
 *------------------------------------------------------------------
 *
 * linux_main.h
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _LINUX_MAIN_H_
#define _LINUX_MAIN_H_


/*
 * Externs function
 */
extern void diag_menu(int, char *argv[]);
extern int ExecuteCmdbyPopen(char *, char *, int);
extern int fpga_reset_32_api(uint, uint, uint, uint);
extern int tam_act2_reset(int);
extern void init_eth2(void);
extern int nanook_show_glory_fpga_ver(int);
extern int display_nanook_sku_info(void);
extern uint nanook_open_module(int *, const char *);
extern int dash_set_map(int);

#define NANOOK_I2C_0                   "i2c-0"
#define NANOOK_I2C_1                   "i2c-1"
#define NANOOK_INFO_BUF_SIZE           256
#define NANOOK_CPU_INFO_FILE           "/sky_cpuinfo.txt"
#define TURN_OFF_OVERCOMMIT_MEM       "echo 2 > /proc/sys/vm/overcommit_memory"
#define TURN_OFF_OVERCOMMIT_RATIO     "echo 100 > /proc/sys/vm/overcommit_ratio"
#define NANOOK_SHOW_MEMORY_SIZE        "lshw -quiet -short | grep \"System Memory\" | awk '{print \"Total Memory: \" $3,$4,$5}'"

#define FPGA_EMMC_RESET             (1 << 23)
#define FPGA_GEWAN1_RESET           (1 << 22)
#define FPGA_GEWAN0_RESET           (1 << 21)
#define EXT_DSL_CHIP_RESET          (1 << 20)
#define EXT_PRI_LTE_RESET           (1 << 6)
#define EXT_CPU_SYS_RESET           (1 << 3)
#define ACT2_RESET                  (1 << 2)
#define EXT_ESW_RESET               (1 << 1)
#define INT_I2C_RESET               (1 << 0)
#define GPIO_RX_ENABLE              (1 << 9)
#define GPIO_TX_DISABLE             (1 << 8)
#define FPGA_INT_DEV_RST_REG         0x808
#define WAITTIME_20_MS          20
#define WAITTIME_150_MS         150
#define WAITTIME_3500_MS        3500
#define WAITTIME_5000_MS        5000
#define FPGA_MASTER_REV_REG          0x884

static char *nanook_cpu_info[] = {
    "Processor",
    "vendor_id",
    "model name",
    "stepping",
    "microcode",
    "cpu MHz",
    "cache size",
    "bogomips",
};

static const uint size_of_nanook_cpu_info =
    sizeof(nanook_cpu_info) / sizeof(uchar *);

extern void init_slot_info(void);

#endif                          /* MB_TESTS_H__ */



/*-------------------------------------------------
 * $Log: linux_main.h,v $
 * Revision 1.2  2019/12/11 10:10:32  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

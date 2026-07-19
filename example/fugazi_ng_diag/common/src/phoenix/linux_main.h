/* $Id: linux_main.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/linux_main.h,v $
 *------------------------------------------------------------------
 *
 * linux_main.h
 *
 *
 * Copyright (c) 2008-2019 by cisco Systems, Inc.
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
extern int phoenix_show_fpga_ver(int);
extern int phoenix_show_sku_dbx_info(void);
extern uint phoenix_open_module(int *, const char *);

#define PHOENIX_I2C_0                   "i2c-0"
#define PHOENIX_I2C_1                   "i2c-1"
#define PHOENIX_INFO_BUF_SIZE           256
#define PHOENIX_CPU_INFO_FILE           "/sky_cpuinfo.txt"
#define TURN_OFF_OVERCOMMIT_MEM       "echo 2 > /proc/sys/vm/overcommit_memory"
#define TURN_OFF_OVERCOMMIT_RATIO     "echo 100 > /proc/sys/vm/overcommit_ratio"
#define PHOENIX_SHOW_MEMORY_SIZE        "lshw -quiet -short | grep \"System Memory\" | awk '{print \"Total Memory: \" $3,$4,$5}'"


static char *phoenix_cpu_info[] = {
    "Processor",
    "vendor_id",
    "model name",
    "stepping",
    "microcode",
    "cpu MHz",
    "cache size",
    "bogomips",
};

static const uint size_of_phoenix_cpu_info =
    sizeof(phoenix_cpu_info) / sizeof(uchar *);

extern void init_slot_info(void);

#endif                          /* MB_TESTS_H__ */




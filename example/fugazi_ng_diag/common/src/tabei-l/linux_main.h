 /* $Id: linux_main.h,v 1.2 2019/10/17 02:16:25 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/linux_main.h,v $
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
extern int tabei_show_fpga_ver(int);
extern uint tabei_open_module(int *, const char *);

#define TABEI_I2C_0                   "i2c-0"
#define TABEI_I2C_1                   "i2c-1"
#define TABEI_INFO_BUF_SIZE           256
#define TABEI_CPU_INFO_FILE           "/sky_cpuinfo.txt"
#define TURN_OFF_OVERCOMMIT_MEM       "echo 2 > /proc/sys/vm/overcommit_memory"
#define TURN_OFF_OVERCOMMIT_RATIO     "echo 100 > /proc/sys/vm/overcommit_ratio"
#define TABEI_SHOW_MEMORY_SIZE        "lshw -quiet -short | grep \"System Memory\" | awk '{print \"Total Memory: \" $3,$4,$5}'"


static char *tabei_cpu_info[] = {
    "Processor",
    "vendor_id",
    "model name",
    "stepping",
    "microcode",
    "cpu MHz",
    "cache size",
    "bogomips",
};

static const uint size_of_tabei_cpu_info =
    sizeof(tabei_cpu_info) / sizeof(uchar *);

extern void init_slot_info(void);

#endif                          /* MB_TESTS_H__ */



/*-------------------------------------------------
 * $Log: linux_main.h,v $
 * Revision 1.2  2019/10/17 02:16:25  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.7  2019/07/16 07:48:31  olin2
 * Remove unused function
 *
 * Revision 1.1.4.6  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.4.5  2018/10/19 01:44:19  harrchan
 * I2C scan test
 *
 * Revision 1.1.4.4  2018/10/15 12:30:12  kodko
 * Add CPLD register read/write function.
 *
 * Revision 1.1.4.3  2018/10/09 09:22:05  olin2
 * Initial commit for NIM test
 *
 * Revision 1.1.4.2  2018/10/02 01:50:02  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */

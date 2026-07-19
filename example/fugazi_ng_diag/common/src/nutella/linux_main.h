/* $Id: linux_main.h,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/linux_main.h,v $
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
extern int fpga_reset_32_api (uint, uint, uint, uint);
extern int tam_act2_reset(int);
extern void init_eth2(void);
extern int nutella_show_fpga_ver(int);

#define NUTELLA_I2C_0                   "i2c-0"
#define NUTELLA_I2C_1                   "i2c-1"
#define NUTELLA_INFO_BUF_SIZE           256
#define NUTELLA_CPU_INFO_FILE           "/sky_cpuinfo.txt"
#define TURN_OFF_OVERCOMMIT_MEM       "echo 2 > /proc/sys/vm/overcommit_memory"
#define TURN_OFF_OVERCOMMIT_RATIO     "echo 100 > /proc/sys/vm/overcommit_ratio"
#define NUTELLA_SHOW_MEMORY_SIZE        "lshw -quiet -short | grep \"System Memory\" | awk '{print \"Total Memory: \" $3,$4,$5}'"


static char *nutella_cpu_info[] = {
    "Processor",
    "vendor_id",
    "model name",
    "stepping",
    "microcode",
    "cpu MHz",
    "cache size",
    "bogomips",
};

static const uint size_of_nutella_cpu_info =
    sizeof(nutella_cpu_info) / sizeof(uchar *);


#endif                          /* MB_TESTS_H__ */



/*-------------------------------------------------
$Log: linux_main.h,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/

 /* $Id: linux_main.h,v 1.3 2018/08/31 03:59:30 chieyang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/linux_main.h,v $
 *------------------------------------------------------------------
 *
 * linux_main.h
 *
 *
 * Copyright (c) 2008-2018 by cisco Systems, Inc.
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
extern int viper_show_fpga_ver(int);
extern int display_viper_sku_info(void);


#define VIPER_I2C_0                   "i2c-0"
#define VIPER_I2C_1                   "i2c-1"
#define VIPER_INFO_BUF_SIZE           256
#define VIPER_CPU_INFO_FILE           "/sky_cpuinfo.txt"
#define TURN_OFF_OVERCOMMIT_MEM       "echo 2 > /proc/sys/vm/overcommit_memory"
#define TURN_OFF_OVERCOMMIT_RATIO     "echo 100 > /proc/sys/vm/overcommit_ratio"
#define VIPER_SHOW_MEMORY_SIZE        "lshw -quiet -short | grep \"System Memory\" | awk '{print \"Total Memory: \" $3,$4,$5}'"


static char *viper_cpu_info[] = {
    "Processor",
    "vendor_id",
    "model name",
    "stepping",
    "microcode",
    "cpu MHz",
    "cache size",
    "bogomips",
};

static const uint size_of_viper_cpu_info =
    sizeof(viper_cpu_info) / sizeof(uchar *);


#endif                          /* MB_TESTS_H__ */



/*-------------------------------------------------
 * $Log: linux_main.h,v $
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.2  2018/07/06 02:54:08  harrchan
 * Add enhance error message
 *
 * Revision 1.1.2.1  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * $Endlog$
 *-------------------------------------------------
 */

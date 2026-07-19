/* $Id: vm_timingcard_pca9557_lib.h,v 1.2 2015/02/14 12:48:43 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_pca9557_lib.h,v $
 *******************************************************************************
 * File Name: vm_timingcard_pca9557_lib.h
 *
 * Description: Timing Card PCA9557 header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2014 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_TIMINGCARD_PCA9557_LIB_H_
#define VM_TIMINGCARD_PCA9557_LIB_H_

#define TIMING_CARD_PCA9557_I2C_ADDR        0x18

#define PCA9557_TDI_GPIO2_OUTPUT            (0x1 << 2)
#define PCA9557_TDI_GPIO4_OUTPUT            (0x1 << 4)
#define PCA9557_TDO_GPIO5_INPUT             (0x1 << 5)
#define PCA9557_TCK_GPIO6_OUTPUT            (0x1 << 6)
#define PCA9557_TMS_GPIO7_OUTPUT            (0x1 << 7)

/* Define PCA9557 register offset */
typedef enum {
    PCA9557_NGIO_EXPANDER_INPUT = 0,
    PCA9557_NGIO_EXPANDER_OUTPUT,
    PCA9557_POLARITY_INVERSION,
    PCA9557_CONFIGURATION,
} pca9557_offset_t;

extern long timingcard_pca9557_init(void);
extern long timingcard_pca9557_program_cpld(unsigned char *);
extern long util_oir_pca9557_reg_write(void);
extern long util_oir_pca9557_reg_read(void);
extern long timingcard_pca9557_reg_test_lib(void);
extern long timingcard_pca9557_i2c_w(uint, uchar);
extern long timingcard_pca9557_i2c_r(uint, uchar *);
extern long timingcard_pca9557_power_cycle_cpld(void);

#endif /* VM_TIMINGCARD_PCA9557_LIB_H_ */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_pca9557_lib.h,v $
 * Revision 1.2  2015/02/14 12:48:43  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.2  2014/03/07 07:39:59  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.1  2014/02/24 09:02:44  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */

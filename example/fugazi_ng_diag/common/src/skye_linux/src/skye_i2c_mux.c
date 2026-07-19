/* $Id: skye_i2c_mux.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_i2c_mux.c,v $
 *------------------------------------------------------------------------------
 * 
 * skye_i2c_mux.c: File for Skye I2C Mux test and utilities.
 *
 * Oct. 8, 2013 - palin2 created for ShrinkRay.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <gxio/common.h>
#include <gxio/gpio.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "types.h" 
#include "queryflags.h" 
#include "nvmonvars.h"
#include "skye_i2c.h"

/*******************************************************************************
 *                           Function Prototypes
 *******************************************************************************
 */
int skye_i2c_mux_ctrl_reg_rd(uchar *);
int skye_i2c_mux_ctrl_reg_wr(uchar *);

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */


/*******************************************************************************
 *
 * Function   : show_skye_i2c_mux_status
 * Description: Utility to show Skye I2C Mux(PCA9546a) channel status
 *              by reading Control Register of PCA9546a.
 * Inputs     : opt - reserved for future use.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
show_skye_i2c_mux_status (int opt)
{
    int   reserved = 0;
    uchar r_data = 0;

    reserved = opt;

    if (skye_i2c_mux_ctrl_reg_rd(&r_data) != PASSED) {
        printf("\n%s: Failed to read Skye I2C Mux control register.\n",
               __FUNCTION__);
        return (FAILED);
    }

    printf("\nSkye I2C Mux control register: 0x%02X.\n", r_data); 
    printf(" -Channel 0(for Power Sequencer & Clock Buffer) is %s.\n",
           (r_data & PCA9546A_I2C_CH0) ? "Enabled" : "Disabled");
    printf(" -Channel 1(for Thermal sensor)                 is %s.\n",
           (r_data & PCA9546A_I2C_CH1) ? "Enabled" : "Disabled");
    printf(" -Channel 2(for Szalinski)                      is %s.\n",
           (r_data & PCA9546A_I2C_CH2) ? "Enabled" : "Disabled");
    printf(" -Channel 3(for Core current sensor)            is %s.\n",
           (r_data & PCA9546A_I2C_CH3) ? "Enabled" : "Disabled");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_i2c_mux_ctrl_reg_rd
 * Description: Function to read Skye I2C Mux(PCA9546a) Control Register.
 * Inputs     : buf - buffer to put the read back data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_mux_ctrl_reg_rd (uchar *buf)
{
    int        fd = -1;
    char       devname[32];

    memset(devname, 0, sizeof(devname));
    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    if (skye_i2c_read(fd, SR_I2C_MUX_ADDR, SR_I2C_MUX_ADDR_SZ,
                           PCA9546A_CTRL_REG, ONE_B_REG, buf) != PASSED) {
        printf("%s: Failed to read data from I2C Mux Control Reg."
               " (I2C%d, Addr = 0x%02X).\n",
               __FUNCTION__, SR_CPU_I2CM2, SR_I2C_MUX_ADDR);
        close(fd);
        return (FAILED);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_i2c_mux_ctrl_reg_wr
 * Description: Function to write Skye I2C Mux(PCA9546a) Control Register.
 * Inputs     : wdata - data want to write in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_mux_ctrl_reg_wr (uchar *wdata)
{
    int        fd = -1;
    char       devname[32];

    memset(devname, 0, sizeof(devname));
    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    if (skye_i2c_write(fd, SR_I2C_MUX_ADDR, SR_I2C_MUX_ADDR_SZ,
                            PCA9546A_CTRL_REG, ONE_B_REG, wdata) != PASSED) {
        printf("%s: Failed to write data(0x%02X) to I2C Mux Control Reg."
               " (I2C%d, Addr = 0x%02X).\n",
               __FUNCTION__, *wdata, SR_CPU_I2CM2, SR_I2C_MUX_ADDR);
        close(fd);
        return (FAILED);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_i2c_mux_setup
 * Description: Wrapped utility to Enable/Disable Skye I2C Mux(PCA9546a)
 *              channels.
 * Inputs     : opt - Enalbe/Disable
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_mux_setup (int opt)
{
    uchar data = 0, c_data = 0;
    int   choice = 0;
    char  string[256];

    memset(string, 0, sizeof(string));
    snprintf(string, sizeof(string), "Enter what channel(s) you want to %s: ",
                                     (opt == ENABLE) ? "Enable" : "Disable");

    /* Skye using PCA9546a as I2C Mux, it's a 4-channel I2C switch.
     * And based on its datasheet, it supports several channels be enabled
     * at the same time. So added "All channels" item to enable all channels
     * at the same tiem for Bring-up.
     */
    printf("\nSkye I2C Mux(PCA9546a) is a 4-Channel I2C Switch:\n");
    printf("1. Channel 0.\n");
    printf("2. Channel 1.\n");
    printf("3. Channel 2.\n");
    printf("4. Channel 3.\n");
    printf("5. All Channels.\n");
    printf("6. Exit.\n");
    choice = getdec_answer(string, 6, 1, 6);

    switch (choice) {
    case 1:
        c_data = PCA9546A_I2C_CH0;
    break;
    case 2:
        c_data = PCA9546A_I2C_CH1;
    break;
    case 3:
        c_data = PCA9546A_I2C_CH2;
    break;
    case 4:
        c_data = PCA9546A_I2C_CH3;
    break;
    case 5:
        c_data = PCA9546A_I2C_ALL_CH;
    break;
    case 6:
        printf("\nUser aborted.\n");
        return (PASSED);
    default:
        printf("\n%s: Unspported !!!(%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }

    if (skye_i2c_mux_ctrl_reg_rd(&data) != PASSED) {
        printf("\n%s: Failed to read Skye I2C Mux control register.\n",
               __FUNCTION__);
        return (FAILED);
    }

    printf("data1 = 0x%02X.\n", data);

    if (opt == ENABLE) {
        data |= c_data;
    } else {
        data &= ((uchar)(~c_data));
    }

    printf("data2 = 0x%02X.\n", data);

    if (skye_i2c_mux_ctrl_reg_wr(&data) != PASSED) {
        printf("\n%s: Failed to write Skye I2C Mux control register.\n",
               __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*----------------------------------------------------
$Log: skye_i2c_mux.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:36  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:55  palin2
Initial check-in Skye module side Diag code.

------------------------------------------------------
skye_i2c_mux.c:
Revision 1.2  2014/02/27 15:01:46  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.1  2013/10/07 21:35:18  palin2
Add ShrinkRay I2C Mux related access function and utility support.

------------------------------------------------------
$Endlog$
*/


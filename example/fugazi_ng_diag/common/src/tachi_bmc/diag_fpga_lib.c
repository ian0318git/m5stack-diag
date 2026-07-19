/* $Id: diag_fpga_lib.c,v 1.5 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_lib.c - FPGA Library
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015 - 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "diag_fpga_lib.h"
#include "patriot_linux/apps/common_utils.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include "diag_fpga_util.h"

int diag_fpga_reg_bitops(int, int, int);
int diag_fpga_reg_read(int, int *);
int diag_fpga_reg_write(int, int);
int diag_fpga_reg_or(int, int);
int diag_fpga_reg_nand(int, int);
int diag_margin_fpga_reg_nand(int, int);
int diag_fpga_ext_reset(int);
int diag_fpga_ext_unreset(int);
void diag_fpga_start_int_test(void);
void diag_fpga_stop_int_test(void);
void diag_fpga_get_intr_test_result(int *);

static int diag_fpga_spi_ioctl(int, fpga_req *);

int diag_fpga_ext_reset (int bit)
{
    return (diag_fpga_reg_or(FPGA_EXT_RESET_REG, bit));
}

int diag_fpga_ext_unreset (int bit)
{
    return (diag_fpga_reg_nand(FPGA_EXT_RESET_REG, bit));
}

void diag_fpga_start_int_test (void)
{
    fpga_req data;

    diag_fpga_spi_ioctl(SPI_IOCTL_START_INTR_TEST, &data);
}

void diag_fpga_stop_int_test (void)
{
    fpga_req data;

    diag_fpga_spi_ioctl(SPI_IOCTL_STOP_INTR_TEST, &data);
}

void diag_fpga_get_intr_test_result (int *data_in)
{
    fpga_req data;

    diag_fpga_spi_ioctl(SPI_IOCTL_GET_INTR_RESULT, &data);

    *data_in = data.data;
}

int diag_fpga_reg_read (int offset, int *data_in)
{
    fpga_req data;

    data.offset = offset;
 
    if (diag_fpga_spi_ioctl(SPI_IOCTL_RD_CMD, &data) == FAILED) {
        return (FAILED);
    }

    *data_in = data.data;
    
    return (PASSED);
}

int diag_fpga_reg_write (int offset, int data_out)
{
    fpga_req data;

    data.offset = offset;
    data.data   = data_out;
    
    /* Let Lewis reset bit out off reset so write offset 0x4 with the mask,if the lewis_reset_mask_flag is on,
       the mask will not be up */    
    if ((offset == FPGA_EXT_RESET_REG ) && (lewis_reset_mask_flag == 0 )) {
        data.data &=~ FPGA_CETUS_RESET;
    }

    /* Set lewis_reset_mask_flag back to 0 */
    lewis_reset_mask_flag = 0;
    return (diag_fpga_spi_ioctl(SPI_IOCTL_WR_CMD, &data));
}

int diag_fpga_reg_or (int offset, int bit)
{
    int data_in;
    
    if (diag_fpga_reg_read(offset, &data_in) == FAILED) {
        return (FAILED);
    }
    data_in |= bit;

    return (diag_fpga_reg_write(offset, data_in));
}

int diag_fpga_reg_nand (int offset, int bit)
{
    int data_in;

    if (diag_fpga_reg_read(offset, &data_in) == FAILED) {
        return (FAILED);
    }
    data_in &= ~(bit);
    return (diag_fpga_reg_write(offset, data_in));
}

int diag_margin_fpga_reg_nand (int offset, int bit)
{
    int data_in;

    if (diag_fpga_reg_read(offset, &data_in) == FAILED) {
        return (FAILED);
    }
    data_in &= ~(bit);
    
    return (diag_fpga_reg_write(offset,(VOLT_MARG_TUNE | data_in)));
}

int diag_fpga_reg_bitops (int ops, int offset, int bit)
{
    int data;
    
    if (diag_fpga_reg_read(offset, &data) != PASSED) {
        return (FAILED);
    }
    
    switch (ops) {
    case FPGA_BIT_OPS_ON:
        data |= bit;
        break;
    case FPGA_BIT_OPS_OFF:
        data &= ~(bit);
        break;
    default:
        printf("Not recognized bit ops (%d)\n", ops);
        return (FAILED);
    }
    
    return (PASSED);
}

int
mb_board_type (void)
{
    return (BDTYPE_TACHI_ENTRY);
}

static int diag_fpga_spi_ioctl (int cmd, fpga_req *drv_data)
{
    int fd;
    int ret;
    int retval = PASSED;

    fd = open(FPGA_SPI_CHAR_DEV, O_RDWR);
    if (fd < 0) {
        printf("%s: Open %s failed\n", __FUNCTION__, FPGA_SPI_CHAR_DEV);
        return (FAILED);
    }

    ret = ioctl(fd, cmd, (fpga_req *)drv_data);
    if (ret) {
        printf("%s: ioctl (%d) return failed (%d)\n", __FUNCTION__, cmd, ret);
        retval = FAILED;
    }

    close(fd);

    return (retval);
}

/*-------------------------------------------------------------------
 *
 * Function: set_nios_mode
 *  to set nios mode to normal mode, disable mode, or diagnostic mode
 * 
 *
 * Input: mode: NIOS mode, NIOS_DISABLE_MODE (0), 
 *              NIOS_NORMAL_MODE (0x1), NIOS_DIAG_MODE (0x3)
 *
 * Output: PASSED/FAILED
 *
 *-------------------------------------------------------------------
 */
int 
set_nios_mode (int mode)
{
    int msg;
    int msg_status;
    uint16_t msg_value, target_msg_status;
    int count = 0, is_valid_mode;

    diag_fpga_reg_read(NIOS_MODE_REG, &msg);
    diag_fpga_reg_read(NIOS_STATUS_REG, &msg_status);
    
    is_valid_mode = (mode == NIOS_DISABLE_MODE || mode == NIOS_NORMAL_MODE ||
                     mode == NIOS_DIAG_MODE);
    if (!is_valid_mode) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Unknown NIOS mode (0x%X)\n", mode);
        }
        return (FAILED);
    }

    /* set mode and check value */
    msg_value = mode;
    target_msg_status = (mode == NIOS_NORMAL_MODE) ? NIOS_NORMAL_CHECK : mode;

    for (count = 0; count < NIOS_CHECK_RETRY; count++) {
        diag_fpga_reg_read(NIOS_STATUS_REG, &msg_status);
        msg_status &= 0xFFFF;
        /* check disable status */
        if (msg_status == target_msg_status) {
            break;
        }
        /* enable/disable nios */
        diag_fpga_reg_write(NIOS_MODE_REG, msg_value);
        usleep(300000);
    }
    if ((msg_status != target_msg_status)) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("NIOS mode (%x) ignored.\n", mode);
        }
        printf("NIOS mode (%x) ignored.\n", mode);
        printf("MSG Status is %#x, target_msg_status is %#x\n", msg_status, target_msg_status);
        return (FAILED);
    }

    return (PASSED);
}

/*---------------------------------------------------------------
$Log: diag_fpga_lib.c,v $
Revision 1.5  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.4  2016/10/27 03:24:46  iachang
Fixed Lewis issue with FPGA ver1.4

Revision 1.3  2016/05/05 01:01:33  benchen2
fix margin issue

Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.11  2016/04/20 00:37:59  huanngo
Remove the code for Tachi-H

Revision 1.1.2.10  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.9  2015/11/13 00:50:32  tirawan
Remove FPGA SGPIO and add FPGA Interrupt

Revision 1.1.2.8  2015/10/26 12:41:51  tirawan
Correct set NIOS mode

Revision 1.1.2.7  2015/10/15 06:23:22  benchen2
add set_nios_mode

Revision 1.1.2.6  2015/08/01 01:37:40  tirawan
Update FPGA SPI Read/Write function

Revision 1.1.2.5  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.4  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.3  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.2  2015/07/12 06:52:45  tirawan
Add Console Switch Utility, SPI driver and FPGA programming

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/


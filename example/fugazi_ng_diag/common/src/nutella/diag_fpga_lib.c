/* $Id: diag_fpga_lib.c,v 1.5 2020/02/04 08:49:42 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_fpga_lib.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_fpga_lib.c
 * Description: FPGA Library.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "defs.h"
#include "error.h"
#include "common.h"
#include "types.h"
#include "nvsysvars.h"
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>
#include "queryflags.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "proto.h"
#include "diag_fpga_lib.h"
#include "diag_fpga.h"


/*
 * Global variables
 */
unsigned long  fpga_ptr;

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int fpga_read_reg(uint, uint *);
int fpga_write_reg(uint, uint);
int fpga_reset_api(uint, uint, uint, uint);
int open_ioperm(void);
int close_ioperm(void);
int fpga_register_operation(uint, uint, int);
int fpga_version_is_more_than_2p0(void);
int fpga_version_is_more_than_3p0(void);
int lpc_irq_force_test(int);
int eobc_or_packet_interrupt_test(int);
int enable_irq_mask(int, int, uint);
int setup_reset_ctl_reg(int, int);
int check_int_irq_stat(int);
int fpga_check_serirq(int, int);
int toggle_driver_irq_flag(int);
int fpga_clear_cpu_serirq_status(void);
int fpga_restore_reg_to_default(uint, uint);
int fpga_test_pattern_test(uint, uint*, int, uint);
int fpga_bit_to_check(uint, int, int);
static int enable_irq_mask_test(int, int);

/*******************************************************************************
 *
 * Function    : fpga_read_reg
 * Description : Function to read FPGA register.
 * Inputs      : reg_offset - register offset
 *               *buf       - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_read_reg (uint reg_offset, uint32_t *buf)
{
    *buf = *((unsigned int *)((long)fpga_ptr + reg_offset));
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_write_reg
 * Description : Function performs FPGA register write.
 * Inputs      : reg_offset - register offset
 *               wr_data    - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_write_reg (uint reg_offset, uint wr_data)
{
    *((unsigned int *)((long)fpga_ptr + reg_offset)) = wr_data;
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reset_api
 * Description : Function of FPGA to reset/unreset interface.
 * Inputs      : r_offset  - register offset
 *               r_bit     - reset bit of register
 *               r_opt     - reset(TRUE)/un-reset(FALSE)
 *               r_time_ms - the reset time interval(millisecond)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reset_api (uint r_offset, uint r_bit, uint r_opt, uint r_time_ms)
{
    uint reg_val = 0;

    /* Read FPGA interface reset register. */
    if (fpga_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set the Reset bit. */
        reg_val |= r_bit;
    } else if (r_opt == FALSE) {
        /* Clear the reset bit. */
        reg_val &= (uint)(~r_bit);
    } else {
        printf("%s: Invalid Reset option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }

    /* Write the reset/un-reset into the corresponding register bit. */
    if (fpga_write_reg(r_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    /* Delay milliseconds after reset/un-reset */
    msleep(r_time_ms);

    /* Confirm the change to FPGA interface reset register. */
    reg_val = 0;
    if (fpga_read_reg(r_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (((r_opt == TRUE) && ((reg_val & r_bit) != r_bit)) ||
        ((r_opt == FALSE) && ((reg_val & r_bit) != 0))) {
        printf("%s: Failed to %s reset bit in FPGA reg.(0x%04X).\n",
               __FUNCTION__, (r_opt == TRUE) ? "set" : "clear", r_offset);
        return (FAILED);
    }


     return (PASSED);
}

/*******************************************************************************
 *
 * Function   : has_lte_sku
 * Description: Function to distinguish sku feature with Nutella
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_lte_sku (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }
    
    btype = (btype & FPGA_BTYPE_LTE_SKU_BIT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_LTE_SKU_BIT) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_sfp_sku
 * Description: Function to distinguish sku feature with Nutella 
 * Inputs     : None 
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_sfp_sku (void)
{
    uint reg_addr = (uint)FPGA_BOARD_TYPE_REG;
    uint btype = 0;

    if (fpga_read_reg(reg_addr, &btype) != PASSED) {
        printf("%s: Failed to read FPGA Borad Type Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }
    
    btype = (btype & FPGA_BTYPE_SFP_SKU_BIT);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: FPGA board type %08X.\n", __FUNCTION__, btype);
    }

    if (btype == FPGA_BTYPE_SFP_SKU_BIT) {
        return (TRUE);
    }
    return (FALSE);
}

/*-------------------------------------------------------------------
 *
 * Function: get_platform_aikido_addr
 * Description: get aikido address
 * 0x31A00 FPGA configuration SPI PROM programming register
 * Input: NONE
 *
 * Output: address
 *
 *-------------------------------------------------------------------
 */
unsigned long get_platform_aikido_addr (void)
{
    assert(fpga_ptr);
    return ((unsigned long)fpga_ptr + FPGA_AIKIDO_SPI_MASTER_OFFSET);
}

/*******************************************************************************
 *
 * Function    : fpga_version_is_more_than_3p0
 * Description : This function will determine whether FPGA version is more than
 *               3.0.0
 * Inputs      : none 
 * Outputs     : TRUE/FALSE
 *
 *******************************************************************************
 */
int fpga_version_is_more_than_3p0 (void)
{
    uint master_revision_data = 0;
    uint master_rev_major_data = 0;

    fpga_read_reg((uint)FPGA_MASTER_REV_REG, &master_revision_data);

    master_rev_major_data = ((master_revision_data & MASTER_FPGA_MAJOR_REV_MASK) >> 
                              MASTER_FPGA_MAJOR_REV_SHIFT);

    if (master_rev_major_data >= CEDGE_FPGA_MAJ_VER_3) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function   : fpga_register_operation
 * Description: Function to control FPGA register
 * Inputs     : reg_offset - FPGA register address
 *              set_val    - set FPGA register to value
 *              mode       - OR / AND
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int fpga_register_operation (uint reg_offset, uint set_val, int mode)
{
    uint32_t reg_val = 0;
    
    /* Access FPGA Register */
    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    if (mode == FPGA_REGISTER_OPERATION_MODE){
        /* OR operation */
        reg_val &= DEFAULT_TO_ZERO;
        reg_val |= set_val; 
        if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }
        msleep(DELAY_FOR_OPERATION);

    } else {
        /* AND operation */
        reg_val &= set_val;
        if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }
    }

    return(PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_version_is_more_than_2p0
 * Description : This function will determine whether FPGA version is more than
 *               2.0.0
 * Inputs      : none 
 * Outputs     : TRUE/FALSE
 *
 *******************************************************************************
 */
int fpga_version_is_more_than_2p0 (void)
{
    uint master_revision_data = 0;
    uint master_rev_major_data = 0;

    fpga_read_reg((uint)FPGA_MASTER_REV_REG, &master_revision_data);

    master_rev_major_data = ((master_revision_data & MASTER_FPGA_MAJOR_REV_MASK) >> 
                              MASTER_FPGA_MAJOR_REV_SHIFT);

    if (master_rev_major_data >= CEDGE_FPGA_MAJ_VER_2) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*******************************************************************************
 *
 * Function    : lpc_irq_force_test 
 * Description : Library function for LPC irq test
 * Inputs      : Test IRQ number 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int lpc_irq_force_test (int test_irq)
{
    uint trig_irq, reg_val;

    /* Read register(0x054) to check default value is 0x0 */
    if (fpga_read_reg(PHASE2_FPGA_LPC_CHASSIS_TEST_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_CHASSIS_TEST_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_VALUE_IS_ZERO) {
        cterr('f', 0, "%s : FPGA LPC Chassis Test Register "
              "is not 0x%08X, value equals to 0x%08X.\n", __FUNCTION__,
              DEFAULT_VALUE_IS_ZERO, reg_val);
        return (FAILED);
    }

    /* Clear CPU SERIRQ status. */
    if (fpga_clear_cpu_serirq_status() != PASSED) {
        cterr('f', 0, "%s : Failed to clear CPU SERIRQ status.\n",
              __FUNCTION__);
        return (FAILED);
    }

    /* Force interrupt */
    if (test_irq == LPC_IRQ0) {
        /* Write 0xCA040000 into Chassis Test Register to trigger the 
         * IRQ 0 interrupt */
        trig_irq = TRIGGER_IRQ0_INTERRUPT;
    } else {
        /* Write 0xCA100000 into Chassis Test Register to trigger the 
         * IRQ 6 interrupt */
        trig_irq = TRIGGER_IRQ6_INTERRUPT;
    }
    if (fpga_write_reg(PHASE2_FPGA_LPC_CHASSIS_TEST_REG, 
                       trig_irq) != PASSED) {
        cterr('f', 0, "%s : Failed to trigger IRQ %d interrupt\n",
              __FUNCTION__, test_irq);
        return (FAILED);
    }
    
    /* Check SERIRQ is asserted. */
    if (fpga_check_serirq(test_irq, TRUE) != PASSED) {
        cterr('f', 0, "%s : Failed to receive IRQ %d interrupt, "
              "SERIRQ is not asserted.\n", __FUNCTION__, test_irq);
        return (FAILED);
    }
    
    /* Clearing CPU SERIRQ status. */
    if (fpga_clear_cpu_serirq_status() != PASSED) {
        cterr('f', 0, "%s : Failed to clear CPU SERIRQ status.\n",
              __FUNCTION__);
        return (FAILED);
    }
    
    /* Clear PHASE2_FPGA_LPC_CHASSIS_TEST_REG 0xFED4_0054 to 0xCA000000, 
     * and read back to make sure it's 0xCA000000 */
    if (fpga_write_reg(PHASE2_FPGA_LPC_CHASSIS_TEST_REG, CLEAR_LPC_CHASSIS_INTR) 
                      != PASSED) {
        cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
              __FUNCTION__, PHASE2_FPGA_LPC_CHASSIS_TEST_REG);
        return (FAILED);
    }
    if (fpga_read_reg(PHASE2_FPGA_LPC_CHASSIS_TEST_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_LPC_CHASSIS_TEST_REG);
        return (FAILED);
    }
    if (reg_val != CLEAR_LPC_CHASSIS_INTR) {
        cterr('f', 0, "%s : FPGA LPC Chassis Test Register "
              "is not 0x%08X, value equals to 0x%08X.\n", __FUNCTION__,
              CLEAR_LPC_CHASSIS_INTR, reg_val);
        return (FAILED);
    }
    
   /* Check SERIRQ is disasserted. */
    if (fpga_check_serirq(test_irq, FALSE) != PASSED) {
        cterr('f', 0, "%s : Failed to receive IRQ %d interrupt, "
              "SERIRQ is not asserted.\n", __FUNCTION__, test_irq);
        return (FAILED);
    }

    if (fpga_write_reg(PHASE2_FPGA_LPC_CHASSIS_TEST_REG, 
                       DEFAULT_VALUE_IS_ZERO) != PASSED) {
        cterr('f', 0, "%s : Failed to set FPGA LPC Chassis Test Register "
              "to default value.\n",
              __FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : enable_irq_mask
 * Description : Library function for enable IRQ6 mask
 * Inputs      : mode - ENABLE or DISABLE the mask
 *               reset_num - Test CC/FP CP or FP Packet number 
 *               mask_val - IRQ6 mask value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int enable_irq_mask (int mode, int reset_num, uint mask_val)
{
    uint32_t irq_mask = 0, read_val = 0;

    if ((reset_num == CCCP_RESET) ||
        (reset_num == FPCP_RESET)){
        irq_mask = EOBC_READY_CHANGE_ON;
    } else {
        irq_mask = PACKET_READY_CHANGE_ON;
    }

    if (mode == MASK_ENABLE) {
        mask_val &= ~(irq_mask); 
    
    } else {
        mask_val |= irq_mask;
    }

    /* Set IRQ6 Mask register(0x038) bit 1.
     * 0 means ENABLE, 1 means DISABLE.*/
    if (fpga_write_reg(PHASE2_FPGA_INTR_IRQ6_MASK_REG, mask_val) != PASSED) {
        cterr('f', 0, "%s : Failed to write FPGA LPC IRQ6 register 0x%04X.\n",
              __FUNCTION__, PHASE2_FPGA_INTR_IRQ6_MASK_REG);
        return (FAILED);
    }

    /* Check whether enable or disable IRQ6 mask is successful. */
    if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ6_MASK_REG, &read_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_INTR_IRQ6_MASK_REG);
        return (FAILED);
    }
    if ((read_val & mask_val) != mask_val) {
        cterr('f', 0, "%s : Set IRQ6 Mask failed.\n", __FUNCTION__);
        return (FAILED);
    }
   
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : setup_reset_ctl_reg
 * Description : Library function for setup reset control register
 * Inputs      : mode - ready for CP or FP communication or not
 *               reset_num - Test CC/FP CP or FP Packet number 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int setup_reset_ctl_reg (int mode, int reset_num)
{
    uint32_t ctl_reg = 0, ctl_val = 0, expected_val, read_val = 0;

    if (reset_num == CCCP_RESET) {
        ctl_reg = PHASE2_FPGA_CCCP_RST_CTL_REG;
        expected_val = CP_READY_OUTPUT_CTL;
    } else if (reset_num == FPCP_RESET){
        ctl_reg = PHASE2_FPGA_FPCP_RST_CTL_REG;
        expected_val = CP_READY_OUTPUT_CTL;
    } else {
        ctl_reg = PHASE2_FPGA_FP_RST_CTL_REG;
        expected_val = PKT_READY_OUTPUT_CTL;
    }

    if (fpga_read_reg(ctl_reg, &ctl_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", ctl_reg);
        return (FAILED);
    }
    if (mode == READY) {
        if (ctl_val != DEFAULT_VALUE_IS_ZERO) {
            cterr('f', 0, "%s : FPGA Register(0x%04X) is not 0x%08X, "
                  "value equals to 0x%08X.\n", __FUNCTION__,
                  ctl_reg, DEFAULT_VALUE_IS_ZERO, ctl_val);
            return (FAILED);
        }
        ctl_val |= expected_val; 
    } else {
        ctl_val &= ~(expected_val); 
    }
    
    /* Set CC CP Reset Control Register(0x404) or 
     * FP CP Reset Control Register(0x604) or
     * FP Reset Control Register(0x610) bit 0 to 1/0.
     * 1 means CP or FP is ready for communication,
     * 0 means CP or FP is not ready for communication. */
    if (fpga_write_reg(ctl_reg, ctl_val) != PASSED) {
            cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
                  __FUNCTION__, ctl_reg);
            return (FAILED);
    }
    
    if (fpga_read_reg(ctl_reg, &read_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", ctl_reg);
        return (FAILED);
    }
    if ((read_val & ctl_val) != ctl_val) {
        cterr('f', 0, "%s : Set FPGA Reset Control Register"
              "(0x%04X) failed, value is not 0 or 0x%08X, it equals to 0x%08X.\n",
              __FUNCTION__, ctl_reg, expected_val, read_val);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : check_int_irq_stat
 * Description : Library function for LPC irq test
 * Inputs      : Test CC/FP CP or FP Packet number 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int check_int_irq_stat (int reset_num)
{
    uint32_t ctl_reg, ctl_val, stat_val, expected_val;
    uint32_t change_detect, time_status;

    if (reset_num == CCCP_RESET) {
        ctl_reg = PHASE2_FPGA_CCCP_RST_CTL_REG;
        change_detect = EOBC_READY_CHANGE_DETECT;
        time_status = CC_REAL_TIME_STATUS;
    } else if (reset_num == FPCP_RESET){
        ctl_reg = PHASE2_FPGA_FPCP_RST_CTL_REG;
        change_detect = EOBC_READY_CHANGE_DETECT;
        time_status = FP_REAL_TIME_STATUS;
    } else {
        ctl_reg = PHASE2_FPGA_FP_RST_CTL_REG;
        change_detect = PACKET_READY_CHANGE_DETECT;
        time_status = PKT_REAL_TIME_STATUS;
    }

    /* bit 18/20/26 in IRQ6 status register(0x34) will track the value of
     * output control bit in IRQ6 reset control register(0x404/0x604/0x610),
     * so when the register(0x404/0x604/0x610) value is not 0,
     * expected_val should be ORed with time_status.
     * */
    if (fpga_read_reg(ctl_reg, &ctl_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", ctl_reg);
        return (FAILED);
    }
    expected_val = (DEFAULT_IRQ6_STAT_REG | change_detect);
    if (ctl_val != DEFAULT_VALUE_IS_ZERO) {
        expected_val |= time_status;
    }
    
    /* Read FPGA Interrupt IRQ6 Status Register(0x34)
     * (1) Ready situation:
     *     If it is testing CC CP Reset, bit 20 and 1 will be 1;
     *     if it is testing FP CP Reset, bit 18 and 1 will be 1;
     *     if it is testing FP Reset, bit 26 and 2 will be 1;
     * (2) Not Ready situation:
     *     If it is testing CC CP Reset, bit 20 will be 0 and 1 will be 1;
     *     if it is testing FP CP Reset, bit 18 will be 0 and 1 will be 1;
     *     if it is testing FP Reset, bit 26 will be 0 and 2 will be 1;
     * */
    if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ6_STAT_REG, &stat_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", PHASE2_FPGA_INTR_IRQ6_STAT_REG);
        return (FAILED);
    }
    if (stat_val != expected_val) {
        printf("%s : FPGA CP or FP Reset Register triggered interrupt failed,"
               " FPGA Interrupt IRQ6 Status Register(0x%04X) "
               "value equals to 0x%08X.\n", __FUNCTION__,
               PHASE2_FPGA_INTR_IRQ6_STAT_REG, stat_val);
        return (FAILED);
    }
    
    /* Write 1 to bit 1 to clear(W1C) CP reset or write 1 to bit 2 to
     * clear(W1C) FP reset, and read back to check. */
    stat_val |= change_detect; 
    if (fpga_write_reg(PHASE2_FPGA_INTR_IRQ6_STAT_REG, stat_val) != PASSED) {
            cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
                  __FUNCTION__, PHASE2_FPGA_INTR_IRQ6_STAT_REG);
            return (FAILED);
    }
    if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ6_STAT_REG, &stat_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", PHASE2_FPGA_INTR_IRQ6_STAT_REG);
        return (FAILED);
    }
    if ((stat_val & change_detect) != 0) {
        cterr('f', 0, "%s : FPGA Interrupt IRQ6 Status Register(0x%04X) "
              "W1C is failed, value equals to 0x%08X.\n", __FUNCTION__,
              PHASE2_FPGA_INTR_IRQ6_STAT_REG, stat_val);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : enable_irq_mask_test
 * Description : Library function for testing IRQ6 interrupt by enable/disable mask
 * Inputs      : reset_num - Test CC/FP CP or FP Packet number 
 *               check_mode -  check CPU SERIRQ is asserted or disasserted
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int enable_irq_mask_test (int reset_num, int check_mode)
{
    int ready_mode;

    for (ready_mode = 0; ready_mode <= NOT_READY; ready_mode++) {

        /* Set CP or FP Reset Control Register to ready/not_ready situation. */
        if (setup_reset_ctl_reg(ready_mode, reset_num) != PASSED) {
            cterr('f', 0, "%s : Failed to setup IRQ6 Reset Control "
                  "Register.\n ", __FUNCTION__);
            return (FAILED);
        }
        
        /* Check SERIRQ is asserted or disasserted.
         * TURE means check SERIRQ is asserted,
         * FALSE means check SERIRQ is disasserted. */
        if (fpga_check_serirq(LPC_IRQ6, check_mode) != PASSED) {
            if (check_mode == TRUE) {
                cterr('f', 0, "%s : Failed to receive IRQ %d interrupt, "
                      "SERIRQ is not asserted.\n", __FUNCTION__, LPC_IRQ6);
                return (FAILED);
            } else {
                cterr('f', 0, "%s : IRQ %d interrupt failed, "
                      "SERIRQ is asserted.\n", __FUNCTION__, LPC_IRQ6);
                return (FAILED);
            }
        }

        /* Check whether IRQ6 interrupt is successful or not by reading FPGA
         * Interrupt IRQ6 Status Register(0x34). */
        if (check_int_irq_stat(reset_num) != PASSED) {
            cterr('f', 0, "%s : FPGA CP or FP Reset Register triggered "
                  "interrupt failed.\n", __FUNCTION__);
            return (FAILED);
        }

        /* Clearing CPU SERIRQ status. */
        if (fpga_clear_cpu_serirq_status() != PASSED) {
            cterr('f', 0, "%s : Failed to clear CPU SERIRQ status.\n",
                  __FUNCTION__);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : eobc_or_packet_interrupt_test 
 * Description : Library function for LPC IRQ6 test
 * Inputs      : reset_num - Test CC/FP CP or FP Packet number 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int eobc_or_packet_interrupt_test (int reset_num)
{
    uint32_t mask_val = 0;
    /* Check IRQ6 Mask register(0x038) is default value(0x6) */
    if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ6_MASK_REG, &mask_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_INTR_IRQ6_MASK_REG);
        return (FAILED);
    }
    if (mask_val != DEFAULT_IRQ6_MASK_REG) {
        cterr('f', 0, "%s : FPGA LPC IRQ6 Mask Register is not 0x%08X, "
              "value equals to 0x%08X.\n", __FUNCTION__,
              DEFAULT_IRQ6_MASK_REG, mask_val);
        return (FAILED);
    }

    /* Enable IRQ6 mask */
    if (enable_irq_mask(MASK_ENABLE, reset_num, mask_val) != PASSED) {
        cterr('f', 0, "%s : Failed to enable IRQ6 mask.\n ", __FUNCTION__);
        return (FAILED);
    }

    /* Do the IRQ6 interrupt test when mask is enable */
    if (enable_irq_mask_test(reset_num, TRUE) != PASSED) {
        cterr('f', 0, "%s : IRQ6 Interrupt Test failed when the mask "
              "is enable.\n ", __FUNCTION__);
        return (FAILED);
    }

    /* Disable IRQ6 mask */
    if (fpga_read_reg(PHASE2_FPGA_INTR_IRQ6_MASK_REG, &mask_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n",
               PHASE2_FPGA_INTR_IRQ6_MASK_REG);
        return (FAILED);
    }
    if (enable_irq_mask(MASK_DISABLE, reset_num, mask_val) != PASSED) {
        cterr('f', 0, "%s : Failed to disable IRQ6 mask.\n ", __FUNCTION__);
        return (FAILED);
    }

    /* Do the IRQ6 interrupt test when mask is disable */
    if (enable_irq_mask_test(reset_num, FALSE) != PASSED) {
        cterr('f', 0, "%s : IRQ6 Interrupt Test failed when the mask "
              "is disable.\n ", __FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_check_serirq
 * Description : check SERIRQ is asserted or not by using scratchpad register
 * Inputs      : test_irq - test IRQ number
 *               check_type - asserted/deasserted(TRUE/FALSE)
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_check_serirq (int test_irq, int check_type)
{
    uint32_t reg_val = 0;
    uint32_t expected_val = 0;
    int ix;
   
    /* Check if interrupt is received in the driver by checking 
     * SCRATCHPAD_REG value, because interrupt handler will set
     * SCRATCHPAD_REG to 0xABCDEF00 if it's testing IRQ0. 
     * Otherwise, the SCRATCHPAD_REG is 0xABCDEF01 for IRQ6. */
    if (check_type == TRUE) {
        if (test_irq == LPC_IRQ0) {
            expected_val = IRQ0_INTERRUPT_MAGIC_VAL;
        } else {
            expected_val = IRQ6_INTERRUPT_MAGIC_VAL;
        }
    } else {
        /* Check if SERIRQ is dis-asserted in the driver by reading
         * SCRATCHPAD_REG(0x800), it should be 0.*/
        expected_val = DEFAULT_VALUE_IS_ZERO;
    }

    for (ix = 0; ix <= IRQ_MAX_POLLING; ix++) {
        if (fpga_read_reg(PHASE2_FPGA_SCRATCHPAD_REG, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", PHASE2_FPGA_SCRATCHPAD_REG);
            return (FAILED);
        }
        if (reg_val == expected_val) {
            return (PASSED);
        } else if (ix == IRQ_MAX_POLLING) {
            return (FAILED);
        } else {
            msleep(WAIT_FOR_IRQ);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : toggle_driver_irq_flag
 * Description : Toggle driver IRQ flag
 * Inputs      : opt - enable/disable irq flag
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int toggle_driver_irq_flag (int opt)
{
    int fd, write_data = 0, read_data = 0;

    fd = open(FPGA_DEVICE, O_RDWR);

    if (opt == IRQ_DISABLE) {
        /* Write 0 to disable SERIRQ FLAG*/
        write_data = IRQ_DISABLE;
        if (ioctl(fd, IOCTL_WR_CMD, &write_data) == -1) {
            printf("fpga ioctl write error\n");
            close(fd);
            return (FAILED);
        }
    } else {
        /* Write 1 to enable SERIRQ FLAG*/
        write_data = IRQ_ENABLE;
        if (ioctl(fd, IOCTL_WR_CMD, &write_data) == -1) {
            printf("fpga ioctl write error\n");
            close(fd);
            return (FAILED);
        }
    }

    if (ioctl(fd, IOCTL_RD_CMD, &read_data) == -1 ) {
        printf("fpga ioctl read error\n");
        close(fd);
        return (FAILED);
    }

    if (read_data == IRQ_ENABLE) {
        printf("\nIRQ now is enable\n");
    } else {
        printf("\nIRQ now is disable\n");
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_clear_cpu_serirq_status
 * Description : Clear SCRATCHPAD_REG(0x800) to zero because we use it to check
 *               whether IRQ0/IRQ6 is asserted or dis-asserted.
 * Inputs      : NONE 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_clear_cpu_serirq_status (void)
{
    uint32_t reg_val = 0;
    
    /* Clear SCRATCHPAD_REG 0xFED4_0800 to 0, and read back to make sure
     * it's 0 */
    if (fpga_write_reg(PHASE2_FPGA_SCRATCHPAD_REG, DEFAULT_VALUE_IS_ZERO) 
                      != PASSED) {
        cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
              __FUNCTION__, PHASE2_FPGA_SCRATCHPAD_REG);
        return (FAILED);
    }
    
    if (fpga_read_reg(PHASE2_FPGA_SCRATCHPAD_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", PHASE2_FPGA_SCRATCHPAD_REG);
        return (FAILED);
    }
    if (reg_val != DEFAULT_VALUE_IS_ZERO) {
        cterr('f', 0, "%s : FPGA Scratchpad Register "
              "is not 0x%08X, value equals to 0x%08X.\n", __FUNCTION__,
              DEFAULT_VALUE_IS_ZERO, reg_val);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_test_pattern_test
 * Description : write register value to test pattern, and read back to check 
 * Inputs      : reg_offset - register address
 *               expected - expected value
 *               result_compare - whether result is same or unique value
 *               default_val - for restore register to default value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_test_pattern_test (uint reg_offset, uint* expected,
                            int result_compare, uint default_val)
{
    int ix;
    uint reg_val = default_val;
    uint expected_val[TEST_PATTERN_SIZE];
    uint test_pattern[] = {SCRATCHPAD_REG_TEST_PATTERN_1,
                           SCRATCHPAD_REG_TEST_PATTERN_2,
                           SCRATCHPAD_REG_TEST_PATTERN_3,
                           SCRATCHPAD_REG_TEST_PATTERN_4,
                           SCRATCHPAD_REG_TEST_PATTERN_5};
    
    /* Write register to test pattern, and read back to check */
    for (ix = 0; ix < (sizeof(test_pattern) / sizeof(test_pattern[0])); ix++) {
        if (result_compare == LAST_BYTE) {
            expected_val[ix] = (test_pattern[ix] & 0xFF);
        } else if (result_compare == LAST_FOUR_BIT) {
            expected_val[ix] = (test_pattern[ix] & 0xF);
        } else if (result_compare == SAME) {
            expected_val[ix] = test_pattern[ix];
        } else {
            expected_val[ix] = expected[ix];
        }

        reg_val = test_pattern[ix];
        if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
            cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
                  __FUNCTION__, reg_offset);
            return (FAILED);
        }

        if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }
        if (reg_val != expected_val[ix]) {
            cterr('f', 0, "%s : After writing FPGA Register "
                  "and read it again is not test value 0x%08X, "
                  "value equals to 0x%08X.\n", __FUNCTION__,
                  expected_val[ix], reg_val);
            return (FAILED);
        }
    }
    
    /* Restore register to default value */ 
    reg_val = default_val;
    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        cterr('f', 0, "%s : Failed to write FPGA register 0x%04X.\n",
              __FUNCTION__, reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_bit_to_check 
 * Description : check the FPGA register bit is zero or one
 * Inputs      : reg_offset - register address
 *               check_bit - which bit
 *               expected_val - zero or one
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_bit_to_check (uint reg_offset, int check_bit, int expected_val)
{
    uint32_t reg_val = 0;

	/* Read FPGA Register */
    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    if (expected_val == FALSE) {
        /* Check the bit equals 0 */
	    if ((reg_val & (TRUE << check_bit)) != FALSE) {
            printf("FPGA register 0x%04X bit %d is not 0.\n", reg_offset, check_bit);
            return (FAILED);
        }
    } else {
        /* Check the bit equals 1 */
	    if ((reg_val & (TRUE << check_bit)) == FALSE) {
            printf("FPGA register 0x%04X bit %d is not 1.\n", reg_offset, check_bit);
            return (FAILED);
        }
    
    }
    
    return (PASSED);
}
/*-------------------------------------------------
$Log: diag_fpga_lib.c,v $
Revision 1.5  2020/02/04 08:49:42  alicehua
CSCvs68364: Add and modify codes for FPGA Phase2.

Revision 1.4  2019/07/11 12:31:27  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/

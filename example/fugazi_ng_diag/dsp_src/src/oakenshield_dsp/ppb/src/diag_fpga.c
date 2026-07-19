/* $Id: diag_fpga.c,v 1.4 2021/04/15 00:53:07 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/diag_fpga.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga.c - This file contains functions for Oakenshield FPGA
 *
 * Owen Lin -2016
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "types.h"
#include "menu.h"
#include "diag_ppb.h"
#include "common.h"
#include "error.h"
#include "debug_console.h"
#include "uart.h"
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"
#include "libuart.h"
#include "libgpio.h"
#include "diag_fpga.h"
#include "ssp.h"
#include "tdmsw16_fpga.h"
#include "fpga_con_memory.h"
#include "fxs_test.h"
#include "tstcodec_si3050.h"
#include "si3xxx_utils.h"
#include "jamport.h"

void fpga_unreset_tdm_pll(void);
int fpga_spi_indirect_read(uint16_t , int , uint32_t*);
int fpga_spi_indirect_write(uint16_t , int , uint32_t);
int tdm_cleanup(void);
uchar get_oak_id(void);
void oak_tdm_xc_setup(uchar board_id, int connect);
int tdm_rate[4] = {TDM_RATE_2M, TDM_RATE_8M, TDM_RATE_16M, TDM_RATE_32M};
extern uint8_t fpga_image[];
extern uint32_t fpga_image_size;
int phoenix_only_test_dbx_flag = FALSE;
int phoenix_not_test_db1_flag = FALSE;
int phoenix_not_test_db2_flag = FALSE;
int phoenix_not_test_db3_flag = FALSE;
static int phoenix_hw_brd_type_flag = 0;

/**********************************************************************
 *
 * Function: fpga_spi_read_utility
 *
 * description: This is the utility of fpga spi read. 
 *
 * Input : fpga_addr - offset of register to be read.
 *	       size - Number of bytes to be read. TDMSW registers areu 4 bytes
 *	       buf  - points to the data buffer to hold read data.
 *	       param - Pointer to parameter
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int fpga_spi_read_utility (void)
{
    uint16_t fpga_addr;
    int size;
    uint32_t data;

    fpga_addr = gethex_answer("\nEnter FPGA register ",
                 0, 0, 0xFFFF);

    size = gethex_answer("\nEnter register size", 0, 0, 4);

    fpga_spi_direct_read(fpga_addr, size, &data);
    
    bsp_debug_printf("FPGA register :%x, Data: %x\n", fpga_addr, data);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fpga_spi_write_utility
 *
 * description: This is the utility of fpga spi write. 
 *
 * Input : none
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int fpga_spi_write_utility (void)
{
    uint16_t fpga_addr;
    int size;
    uint32_t data;

    fpga_addr = gethex_answer("\nEnter FPGA register ",
                 0, 0, 0xFFFF);

    size = gethex_answer("\nEnter register size", 0, 0, 4);

    data = gethex_answer("\nEnter data", 0, 0, 0xffffffff);

    fpga_spi_direct_write(fpga_addr, size, data);
    
    bsp_debug_printf("FPGA register :%x, Data: %x\n", fpga_addr, data);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fpga_spi_direct_read
 *
 * FPGA registers can  be accessed directly with spi.
 *
 * Input : addr - offset of register to be read.
 *	   size - Number of bytes to be read. TDMSW registers are 4 bytes
 *	   buf  - points to the data buffer to hold read data.
 *	   param - Pointer to parameter
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int fpga_spi_direct_read (uint16_t addr, int size, uint32_t *rd_data)
{
    int ix;
    uint8_t *val_data;

    memset(rd_data, 0, sizeof(*rd_data));
    
    val_data = (uint8_t *)rd_data;
    for (ix = 0; ix < size; ix++) {
        spi_read_fpga(addr+ix, 2, val_data, 1);
        val_data++;
    }


    return (PASSED);
}

/**********************************************************************
 *
 * Function: fpga_spi_direct_write
 *
 * FPGA registers can  be accessed directly  with spi.
 *
 * Input : addr - offset of register to be writed
 *	   size - Number of bytes to be read. TDMSW registers are 4 bytes
 *	   buf  - points to the data buffer to hold read data.
 *	   param - Pointer to parameter
 *
 * Output: PASSED
 *
 **********************************************************************
 */
int fpga_spi_direct_write (uint16_t addr, int size, uint32_t wr_data)
{
    int ix;
    uint8_t *val_data;
    
    val_data = (uint8_t *)&wr_data;
    for (ix = 0; ix < size; ix++) {
        spi_write_fpga(addr + ix, 2, (val_data + ix), 1);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: fpga_spi_indirect_read
 *
 * TDMSW registers can only be accessed in 4 bytes width. And the SP270x
 * local bus access to FPGA is 1 byte per times. We need to use indirect access
 * to read/write TDMSW registers.
 *
 * Input : addr - offset of register to be read.
 *	   size - Number of bytes to be read. TDMSW registers are 4 bytes
 *	   buf  - points to the data buffer to hold read data.
 *	   param - Pointer to parameter
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_spi_indirect_read (uint16_t addr, int size, uint32_t *rd_data)
{
    int ix;
    uint8_t *val_addr, data, tdmsw_cmd_status, *val_data;


    memset(rd_data, 0, sizeof(*rd_data));
    
    /* make sure TDMSW is available to access */
    for (ix = 0; ix < TDMSW_CMD_WAIT; ix++) {
        spi_read_fpga(FPGA_GENERAL_TDMSW_CMD_STATUS, 2, &data, 1);
        if (data & TDMSW_BUSY) {
            msleep(10);
        } else {
            break;
        }
    }

    if (ix == TDMSW_CMD_WAIT) {
        cterr('f', 0, "TDMSW is busy! Can not access.");
        return (FAILED);
    }

    val_addr = (uint8_t *)&addr;

    spi_write_fpga(FPGA_GENERAL_TDMSW_ADR_LO, 2, val_addr, 1);
    spi_write_fpga(FPGA_GENERAL_TDMSW_ADR_HI, 2, (val_addr + 1), 1);

    /* send read command */
    data = (TDMSW_CMD_READ|TDMSW_CMD_GO);
    spi_write_fpga(FPGA_GENERAL_TDMSW_CMD_STATUS, 2, &data, 1);

    /* wait for read command to complete - 50ms */
    for (ix = 0; ix < TDMSW_CMD_WAIT; ix++) {
        spi_read_fpga(FPGA_GENERAL_TDMSW_CMD_STATUS, 2, &tdmsw_cmd_status, 1);
        if (tdmsw_cmd_status & TDMSW_BUSY) {
            msleep(10);
        } else {
            break;
        }
    }
    
    if (ix == TDMSW_CMD_WAIT) {
        cterr('f', 0, "TDMSW register@%#x read timeout!", addr);
        return (FAILED);
    }

    val_data = (uint8_t *)rd_data;
    for (ix = 0; ix < size; ix++) {
        spi_read_fpga(FPGA_GENERAL_TDMSW_DATA_0 + ix, 2, val_data, 1);
        val_data++;
    }


    return (PASSED);
}


/**********************************************************************
 *
 * Function: fpga_spi_indirect_write
 *
 * TDMSW registers can only be accessed in 4 bytes width. And the SP270x
 * local bus access to FPGA is 1 byte per times. We need to use indirect access
 * to read/write TDMSW registers.
 *
 * Input : addr - offset of register to be written.
 *	   size - Number of bytes to write. TDMSW registers are 4 bytes
 *	   data - Write data.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_spi_indirect_write (uint16_t addr, int size, uint32_t wr_data)
{
    int ix;
    uint8_t *val_addr, tdmsw_cmd_status, *val_data;
    
    /* make sure TDMSW is available to access */
    for (ix = 0; ix < TDMSW_CMD_WAIT; ix++) {
        spi_read_fpga(FPGA_GENERAL_TDMSW_CMD_STATUS, 2, &tdmsw_cmd_status, 1);
        if (tdmsw_cmd_status & TDMSW_BUSY) {
            msleep(10);
        } else {
            break;
        }
    }

    val_addr = (uint8_t *)&addr;
    spi_write_fpga(FPGA_GENERAL_TDMSW_ADR_LO, 2, val_addr, 1);
    spi_write_fpga(FPGA_GENERAL_TDMSW_ADR_HI, 2, (val_addr + 1), 1);

    /* Write data to Data register */
    val_data = (uint8_t *)&wr_data;
    for (ix = 0; ix < size; ix++) {
        spi_write_fpga(FPGA_GENERAL_TDMSW_DATA_0 + ix, 2, (val_data + ix), 1);
    }

    /* send read command */
    tdmsw_cmd_status = (TDMSW_CMD_WRITE|TDMSW_CMD_GO);
    spi_write_fpga(FPGA_GENERAL_TDMSW_CMD_STATUS, 2, &tdmsw_cmd_status, 1);

    /* wait for write command to complete - 50ms */
    for (ix = 0; ix < TDMSW_CMD_WAIT; ix++) {
        spi_read_fpga(FPGA_GENERAL_TDMSW_CMD_STATUS, 2, &tdmsw_cmd_status, 1);
        if (tdmsw_cmd_status & TDMSW_BUSY) {
            msleep(10);
        } else {
            break;
        }
    }
    
    if (ix == TDMSW_CMD_WAIT) {
        cterr('f', 0, "TDMSW register@%#x read timeout!", addr);
        return (FAILED);
    }


    return (PASSED);

}




/**********************************************************************
 *
 * Function: fpga_general_reg_test
 *
 * This function will test FPGA general registers.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_general_reg_test (void)
{
    int ix;
    uchar wval;
    uint32_t test_data, save_data;
    uint16_t test_reg = FPGA_GENERAL_TDMSW_ADR_LO;
    char *name = "FPGA_GENERAL_TDMSW_ADR_LO";

    bsp_debug_printf("\n\r FPGA Register Test\n"); 
    /* Save register under test */
    if (fpga_spi_direct_read(test_reg, 1, &save_data)) {
        sprintf((char *)&(hd_if->errmsg), "\nFail to read reg:%#x.\n"
                        ,test_reg);
        return (FAILED);
    }

    /* ripple 1 test */
    for (ix = 0; ix < 8; ix++) {
        wval = 1 << ix;
        if (fpga_spi_direct_write(test_reg, 1, wval)) {
            sprintf((char *)&(hd_if->errmsg), "\nFail to write reg:%#x.\n"
                        ,test_reg);
            return (FAILED);
        }
        if (fpga_spi_direct_read(test_reg, 1, &test_data)) {
            sprintf((char *)&(hd_if->errmsg), "\nFail to read reg:%#x.\n"
                        ,test_reg);
            return (FAILED);
        }

        if (test_data != wval) { 
            cterr ('f', 0, "Ripple one test failed when accessing %s "
                           "register. Expect: %#x, Read: %#x.",
                           name, wval, test_data);

            sprintf((char *)&(hd_if->errmsg), "\nRipple one test failed when accessing %s\
                           register. Expect: %x, Read: %lx\n", name, wval, test_data);
            return (FAILED);
        }
    } 

    /* ripple 0 test */
    for (ix = 0; ix < 8; ix++) {
        wval = ~(1 << ix);
        if (fpga_spi_direct_write(test_reg, 1, wval)) {
            return (FAILED);
        }
        if (fpga_spi_direct_read(test_reg, 1, &test_data)) {
            return (FAILED);
        }

        if (test_data != wval) {


            cterr ('f', 0, "Ripple zero test failed when accessing %s "
                           "register. Expect: %#x, Read: %#x.",
                           name, wval, test_data);
                           
            sprintf((char *)&(hd_if->errmsg), "\nRipple zero test failed when accessing %s\
                           register. Expect: %x, Read: %lx\n", name, wval, test_data);
            return (FAILED);
        }
    } 

    /* Restore register under test */
    if (fpga_spi_direct_write(test_reg, 1, save_data)) {
        return (FAILED);
    }


    return (PASSED);
}


/**********************************************************************
 *
 * Function: fpga_reg_test
 *
 * wrapper function for FPGA registers test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_reg_test (void)
{
    bsp_debug_printf("FPGA general registers test.\n");

    if (fpga_general_reg_test() == FAILED) {
        cterr('f', 0, "FPGA register test failed.");
        return (FAILED);
    }

    return (PASSED);

}


/**********************************************************************
 *
 * Function: fpga_intr_test
 *
 * This function will test INT1 from FPGA to NPU this Intrrupt  
 * process is follow the Hardware Function Spec.
 *
 * Input : None
 *
 * Output: PASSED/FAILED 
 *
 **********************************************************************
 */
int fpga_intr_test (void)
{
    int ix, gpio_data;
    uint32_t reg_data;
   
    bsp_debug_printf("FPGA interrupt test\n");

    /* clear all the pending interrupt bits first */
    fpga_spi_direct_write(FPGA_GENERAL_FPGA_INT_EVENT, 1, 0xff);
    
    /* enable TDMSW_FSYNC_MISS_ERR interrupt for diag test */
    fpga_spi_direct_write(FPGA_GENERAL_FPGA_INT_EVENT_ENA, 1, TDMSW_FSYNC_MISS_ERR);
  
    /* trigger INT1 from FPGA to NPU */
    fpga_spi_direct_write(FPGA_GENERAL_FPGA_INT_DIAG_TEST, 1, TDMSW_FSYNC_MISS_ERR);

    gpio_data = sp_GetGPIOData(0x4);
    if (!(gpio_data & 0x4)) {
        cterr('f', 0, "Interrupt GPIO PIN Failed, data: %x\n", gpio_data);
        sprintf((char *)&(hd_if->errmsg), "Interrupt GPIO PIN Failed, data: %x\n", gpio_data);
        return (FAILED);
    }

    /* wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (ix = 0; ix < 5000; ix++) {
        fpga_spi_direct_read(FPGA_GENERAL_FPGA_INT_DIAG_TEST, 1, &reg_data);
        if (reg_data == 0) {
            /* Clear the event */
            fpga_spi_direct_write(FPGA_GENERAL_FPGA_INT_EVENT, 1, TDMSW_FSYNC_MISS_ERR);
            break;
        } else {
            msleep(100);
        }
    }

    /* disable TDMSW_FSYNC_MISS_ERR interrupt after the test */
    fpga_spi_direct_read(FPGA_GENERAL_FPGA_INT_EVENT_ENA, 1, &reg_data);
    fpga_spi_direct_write(FPGA_GENERAL_FPGA_INT_EVENT_ENA, 1, reg_data & (~TDMSW_FSYNC_MISS_ERR));

    if (ix == 5000) {
        fpga_spi_direct_read(FPGA_GENERAL_FPGA_INT_DIAG_TEST, 1, &reg_data);
        bsp_debug_printf("Timeout waiting for interrupt to be cleared. fpga_int_diag_test = %#x\n", 
	                      reg_data);
        sprintf((char *)&(hd_if->errmsg), "Timeout waiting for interrupt to be cleared.\
                          fpga_int_diag_test = %#x\n", gpio_data);
        return (FAILED);
    } else {
        return (PASSED);
    }

}

/**********************************************************************
 *
 * Function: fpga_get_rev
 *
 * This function will get FPGA revision.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void fpga_get_rev (void)
{
    uint32_t register_2b_data = 0;
    uint32_t *reg_4b_data = &register_2b_data;
    uint8_t *reg_1b_data;


    fpga_spi_direct_read(FPGA_GENERAL_FPGA_DATA, 4, reg_4b_data);


    reg_1b_data = (uint8_t*)reg_4b_data;
    /* fpga_rd_dat[3] for hour, fpga_rd_data[2] for day, 
       fpga_rd_dat[1] for month,  fpga_rd_data[0] for year. */

    bsp_debug_printf("Oakenshield FPGA was built at %x/%x/%x at %x o'clock\n", 
                      *(reg_1b_data+2), *(reg_1b_data+1),
                      *(reg_1b_data+3), *(reg_1b_data));


    fpga_spi_direct_read(FPGA_GENERAL_FPGA_REV, 4, reg_4b_data);

    bsp_debug_printf("Oakenshield FPGA Image Revision: %x.%x.%x (%s)\n", 
                      *(reg_1b_data+2), *(reg_1b_data+1),
                      *(reg_1b_data),
	   ((*(reg_1b_data+3))&0x01)?"Debug Image":"Official Release Image");

}

/**********************************************************************
 *
 * Function: fpga_reset_tdm_pll
 *
 * This function will reset TDM PLL from FPGA.
 *
 * Input : None
 * Output : None
 *
 **********************************************************************
 */
void fpga_reset_tdm_pll (void)
{
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;
    uint32_t wr_data;

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, reg_data);

    wr_data = *reg_data | TDM_PLL_RST;

    fpga_spi_direct_write(FPGA_GENERAL_MISC_CONTROL, 1, wr_data);

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, reg_data);
    bsp_debug_printf("\n\r RESET:ADDR & contents of MISC_CTRL reg %#x %#x\n",
                     FPGA_GENERAL_MISC_CONTROL, *reg_data);

}

/**********************************************************************
 *
 * Function: fpga_unreset_tdm_pll
 *
 * This function will take TDM PLL out of reset from FPGA.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void fpga_unreset_tdm_pll(void)
{
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;
    uint32_t wr_data;

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, reg_data);

    wr_data = *(reg_data) & (~TDM_PLL_RST);

    fpga_spi_direct_write(FPGA_GENERAL_MISC_CONTROL, 1, wr_data);

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, reg_data);
    bsp_debug_printf("\n\r UNRESET:ADDR & contents of MISC_CTRL reg %#x %#x\n",
                     FPGA_GENERAL_MISC_CONTROL, *reg_data);

}

/**********************************************************************
 *
 * Function: fpga_config_tdm_pll
 *
 * This function will configure TDM PLL to use primary input clock from host.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void fpga_config_tdm_pll (void)
{
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;
    uint32_t wr_data;

    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 1, reg_data);
    
    wr_data =  *reg_data | (TDMPLL_PRI_ENA | TDMPLL_PRI_SEL);
    
    fpga_spi_direct_write(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 1, wr_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 1, reg_data);
    bsp_debug_printf("\n\r CONFIG TDM PLL:ADDR & contents of MISC_CTRL reg %#x %#x\n",
                     FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, *reg_data);

}


/**********************************************************************
 *
 * Function: fpga_check_tdm_pll
 *
 * This function will check TDM PLL lock and reference failed status 
 * if the framer is configured in slave mode.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_check_tdm_pll (void)
{
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;

    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1D, 1, reg_data);

    if (*reg_data & TDMPLL_REF_FAIL) {
        bsp_debug_printf("\n\r Detect TDM PLL reference clock failed.");
        return (FAILED);
    }

    if (!(*reg_data & TDMPLL_LOCK)) {
        bsp_debug_printf("\n\r TDM PLL is not in lock status.");
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: fpga_reset_tdmsw
 *
 * This function will reset TDMSW from FPGA.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_reset_tdmsw (void)
{
    uint16_t reg_addr;
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;
    uint32_t wr_data;

    reg_addr = TDMSW16_REG_BASE + TDMSW16_CTL_OFFSET;

    fpga_spi_indirect_read(reg_addr, 1, reg_data);

    wr_data = *reg_data | TDMSW_RST;

    fpga_spi_indirect_write(reg_addr, 1, wr_data);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: fpga_unreset_tdmsw
 *
 * This function will take TDMSW out of reset from FPGA.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_unreset_tdmsw (void)
{
    uint16_t reg_addr;
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;
    uint32_t wr_data;

    reg_addr = TDMSW16_REG_BASE + TDMSW16_CTL_OFFSET;

    fpga_spi_indirect_read(reg_addr, 1, reg_data);


    wr_data = *reg_data & (~TDMSW_RST);

    fpga_spi_indirect_write(reg_addr, 1, wr_data);

    return (PASSED);
}


/**********************************************************************
 *
 * Function: set_tdmsw_lpbk
 *
 * This function will configure the specific stream in TDMSW in lpbk or not.
 *
 * Input : stream - the stream number (0-15) to configure.
 *                  If stream = TDMSW16_NUM_TDM_STREAM, 
 *                  configure all the streams.
 *         lpbk - TRUE for lpbk, FALSE for no lpbk
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int set_tdmsw_lpbk (int stream, int lpbk)
{
    uint32_t register_data = 0;
    uint32_t *reg_data = &register_data;
    uint32_t wr_data;
    uint16_t lpbk_addr;
    int str;

    if (stream == TDMSW16_NUM_TDM_STREAM) {
        if (lpbk == TRUE) {
            wr_data = SET_TDM_LPBK_TRUE;
        } else {
            wr_data = 0;
        }

        lpbk_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_LPBK31_00_OFFSET;
        fpga_spi_indirect_write(lpbk_addr, 4, wr_data);

    } else { 
        lpbk_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_LPBK31_00_OFFSET;
        str = stream;

        fpga_spi_indirect_read(lpbk_addr, 4, reg_data);

        if (lpbk == TRUE) {
            *reg_data |= (1 << str);
        } else {
            *reg_data &= ~(1 << str);
        }

        fpga_spi_indirect_write(lpbk_addr, 4, *reg_data);
    }


    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_tdmsw_stream_rate
 *
 * This function will configure the rate for the specific stream in TDMSW.
 *
 * Input : stream - the stream number (0-15) to configure, if stream = 16, 
 *                  then configure all 16 streams.
 *         rate - TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int set_tdmsw_stream_rate (int stream, stream_rate rate)
{
    uint32_t reg_data;
    uint16_t rate_addr;
    int ix, str;


    if (stream == TDMSW16_NUM_TDM_STREAM) {
        reg_data = rate;
        for (ix = 0; ix < 16; ix++) {
            reg_data |= (rate << (ix*2));
        }

        rate_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_RATE15_00_OFFSET;
        fpga_spi_indirect_write(rate_addr, 4, reg_data);
    } else {
        /* 0 <= stream <= 15 */
        rate_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_RATE15_00_OFFSET;
        str = stream;

        fpga_spi_indirect_read(rate_addr, 4, &reg_data);

        reg_data &= ~(0x11 << (str*2));
        reg_data |= (rate << (str*2));

        fpga_spi_indirect_write(rate_addr, 4, reg_data);
    }

    return (PASSED);
}

/*******************************************************************
 *
 * Function: get_tdmsw_stream_rate
 *
 * Description:
 *      This function return the TDM rate of the given stream.
 *
 * Input :
 *      stream - stream number to get the rate
 *
 * Output: TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M
 *          or -1 if FAILED.
 *
 *******************************************************************
 */
int get_tdmsw_stream_rate (int stream)
{
    uint16_t rate_addr;
    uint32_t reg_data;
    int str;

    /* 0 <= stream <= 15 */
    rate_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_RATE15_00_OFFSET;
    str = stream;

    fpga_spi_indirect_read(rate_addr, 4, &reg_data);

    return ((reg_data >> (str*2)) & TDM_STREAM_MESK);
}

/**********************************************************************
 *
 * Function: set_tdmsw_stream_ena
 *
 * This function will enable/disable the specific stream in TDMSW.
 *
 * Input : stream - the stream number (0-15) to configure
 *         If stream = TDMSW16_NUM_TDM_STREAM, 
 *         configure all the streams.
 *         ena - TRUE for enable, FALSE for disable
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int set_tdmsw_stream_ena (int stream, int ena)
{
    uint16_t enbl_addr;
    uint32_t reg_data;
    int str;

    if (stream == TDMSW16_NUM_TDM_STREAM) {
        if (ena == TRUE) {
            reg_data = 0x00ff;
        } else {
            reg_data = 0;
        }
        enbl_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_ENBL31_00_OFFSET;
        fpga_spi_indirect_write(enbl_addr, 4, reg_data);
    } else {
        enbl_addr = TDMSW16_REG_BASE + TDMSW16_STREAM_ENBL31_00_OFFSET;
        str = stream;

        fpga_spi_indirect_read(enbl_addr, 4, &reg_data);

        if (ena == TRUE) {
            reg_data |= (1 << str);
        } else {
            reg_data &= ~(1 << str);
        }

        fpga_spi_indirect_write(enbl_addr, 4, reg_data);
    }
    return (PASSED);
}

/*******************************************************************
 *
 * Function: show_tdmsw_regs
 *
 * Description: This function will display the contents of the
 *              Oakenshield FPGA TDMSW registers
 *
 * Input  : None
 *
 * Output : Always return PASSED to avoid compilation warning
 *
 *******************************************************************
 */
int show_tdmsw_regs (void)
{
    uint32_t reg_data;


    fpga_spi_indirect_read(TDMSW16_REG_BASE+TDMSW16_STREAM_ENBL31_00_OFFSET, 4, &reg_data);
    bsp_debug_printf("\n\rTDM switch ENBL Register(31-00)  @%x = %x",
                        TDMSW16_REG_BASE+TDMSW16_STREAM_ENBL31_00_OFFSET,
                        reg_data);

    fpga_spi_indirect_read(TDMSW16_REG_BASE+TDMSW16_STREAM_RATE15_00_OFFSET, 4, &reg_data);
    bsp_debug_printf("\n\rTDM stream Rate Register(15-00) @%x = %x",
                        TDMSW16_REG_BASE+TDMSW16_STREAM_RATE15_00_OFFSET,
                        reg_data);

    fpga_spi_indirect_read(TDMSW16_REG_BASE+TDMSW16_STREAM_LPBK31_00_OFFSET, 4, &reg_data);
    bsp_debug_printf("\n\rTDM stream Lpbk Register(31-00) @%x = %x",
                        TDMSW16_REG_BASE+TDMSW16_STREAM_LPBK31_00_OFFSET,
                         reg_data);

    fpga_spi_indirect_read(TDMSW16_REG_BASE+TDMSW16_CTL_OFFSET, 4, &reg_data);
    bsp_debug_printf("\n\rTDM Control Register     @%x = %x",
                        TDMSW16_REG_BASE+TDMSW16_CTL_OFFSET,
                         reg_data);

    return(PASSED);

}

/*****************************************************************
 *
 * Function: tdmsw_peek_reg
 *
 * Description: This function performs a read to NPU memory-mapped
 *              TDMSW registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int tdmsw_peek_reg (void)
{
    uint16_t base_addr;
    ushort offset;
    uint32_t reg_data;

    base_addr = TDMSW16_REG_BASE;

    offset = gethex_answer("\nEnter TDMSW register offset[0x00 to 0x30]:",
               0, 0, 0x30);


    /* all the TDMSW registers are 4 bytes aligned */
    offset &= 0xfc;

    fpga_spi_indirect_read((base_addr + offset), 4, &reg_data);
    bsp_debug_printf("\n register value @ offset %x = %x ",
                         (base_addr+offset), reg_data);
    return (PASSED);

}

/*****************************************************************
 *
 * Function: tdmsw_poke_reg
 *
 * Description: This function performs a write to NPU memory-mapped
 *              TDMSW registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int tdmsw_poke_reg (void)
{
    uint16_t base_addr;
    uint16_t offset;
    uint32_t reg_data;

    base_addr = TDMSW16_REG_BASE;


    offset = gethex_answer("\nEnter TDMSW register offset[0x00 to 0x30]:",
               0, 0, 0x30);

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:",
                 0, 0, 0xffffffff);

    /* all the TDMSW registers are 4 bytes aligned */
    offset &= 0xfc;
    bsp_debug_printf("\n\r tdmsw_poke_reg: OFFSET= %#x\n", offset);

    fpga_spi_indirect_write((base_addr + offset), 4, reg_data);
    fpga_spi_indirect_read((base_addr + offset), 4, &reg_data);
    bsp_debug_printf("\n\r register value @ offset %#x = %#.8x ",
                        (base_addr+offset), reg_data);


    return (PASSED);
}


/*****************************************************************
 *
 * Function: tdmsw_peek_conn_mem
 *
 * Description: This function performs a read to NPU memory-mapped
 *              TDMSW connection memory.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int tdmsw_peek_conn_mem (void)
{
    uint16_t offset;
    uint32_t mem_data;

    offset = gethex_answer("\nEnter TDMSW memory offset[0x0 to 0x7FFF]:",
               0, 0, 0x7fff);

    /* all the TDMSW connection memory locations are 4 bytes aligned */
    offset &= 0x7fff;

    fpga_spi_indirect_read(offset, 4, &mem_data);
    bsp_debug_printf("\n\r connection memory @ offset %x = %x ", offset, mem_data);

    return (PASSED);

}

/*****************************************************************
 *
 * Function: tdmsw_poke_conn_mem
 *
 * Description: This function performs a write to NPU memory-mapped
 *              TDMSW connection memory.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int tdmsw_poke_conn_mem (void)
{
    uint16_t offset;
    uint32_t mem_data; 

    offset = gethex_answer("\nEnter TDMSW memory offset[0x0 to 0x7FFF]:",
               0, 0, 0x7fff);

    mem_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:",
                 0, 0, 0xffffffff);


    /* all the TDMSW registers are 4 bytes aligned */
    offset &= 0x7fff;

    fpga_spi_indirect_write(offset, 4, mem_data);

    return (PASSED);
}


/*****************************************************************
 *
 * Function: fpga_peek_reg
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA general registers.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_peek_reg (void)
{
    uint32_t reg_data;
    uint16_t offset;
    uint16_t reg_p;

    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x206]:",
               0, 0, 0x206);

    reg_p = FPGA_GENERAL_REG_BASE + offset;
    fpga_spi_direct_read(reg_p, 1, &reg_data);
    bsp_debug_printf("\n\r register value @%#x = %#x \n", reg_p, reg_data);


    return (PASSED);
}

/*****************************************************************
 *
 * Function: read_fpga_dir_reg
 *
 * Description: This function performs a read  FPGA general registers
 *
 * and transmit to platform.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int read_fpga_dir_reg (void)
{
    int size;
    uint32_t addr, offset;
    uint32_t reg_data;

    offset = hd_if->param1;
    size = hd_if->param2;
    addr = FPGA_GENERAL_REG_BASE + offset;

    fpga_spi_direct_read(addr, size, &reg_data);
    bsp_debug_printf("\n\r register value @%#lx = %#lx \n", addr, reg_data);
    sprintf((char *)&(hd_if->bufmsg), "Read Address:%#lx ,Size:%d, Value: %#lx.\n", addr, size, reg_data);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: write_fpga_dir_reg
 *
 * Description: This function performs a write  FPGA general registers
 *
 * and transmit to platform.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int write_fpga_dir_reg (void)
{
    int size;   
    uint32_t addr, offset, parameter;
    uint32_t value, reg_data;

    parameter = hd_if->param1;
    value = hd_if->param2;
    offset = (parameter & 0xfff0) >> 4;
    size = parameter & 0xf;
    addr = FPGA_GENERAL_REG_BASE + offset;

    fpga_spi_direct_write(addr, size, value);
    bsp_debug_printf("\n\r register write = @%#lx value = %#lx \n", addr, value);
    fpga_spi_direct_read(addr, size,&reg_data);

    sprintf((char *)&(hd_if->bufmsg), "Write Address:%#lx ,Size:%d, Value: %#lx.\n", addr, size, reg_data);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: read_fpga_indir_reg
 *
 * Description: This function performs a read  FPGA general registers
 *
 * and transmit to platform.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int read_fpga_indir_reg (void)
{
    uint32_t addr;
    uint32_t reg_data;

    addr = hd_if->param1;

    fpga_spi_indirect_read(addr, 4, &reg_data);
    bsp_debug_printf("\n\r register value @%#lx = %#lx \n", addr, reg_data);
    sprintf((char *)&(hd_if->bufmsg), "Read Address:%#lx ,Size:4, Value: %#lx.\n", addr, reg_data);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: write_fpga_indir_reg
 *
 * Description: This function performs a write  FPGA general registers
 *
 *              and transmit to platform.
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int write_fpga_indir_reg (void)
{
    uint32_t addr;
    uint32_t value, reg_data;

    addr = hd_if->param1;
    value = hd_if->param2;


    fpga_spi_indirect_write(addr, 4, value);
    bsp_debug_printf("\n\r register write = @%#lx value = %#lx \n", addr, value);
    fpga_spi_indirect_read(addr, 4,&reg_data);

    sprintf((char *)&(hd_if->bufmsg), "Write Address:%#lx ,Size:4, Value: %#lx.\n", addr, reg_data);

    return (PASSED);
}
/*****************************************************************
 *
 * Function: fpga_poke_reg
 *
 * Description: This function performs a write to NPU memory-mapped
 *              FPGA general registers.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_poke_reg (void)
{
    uint32_t reg_data;
    uint16_t reg_p, offset;

    offset = gethex_answer("\nEnter FPGA general register offset[0x00 to 0x206]:",
               0, 0, 0x206);
    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFF]:",
                 0, 0, 0xff);

    reg_p = FPGA_GENERAL_REG_BASE + offset;
    fpga_spi_direct_write(reg_p, 1, reg_data);
    bsp_debug_printf("\n\r register value @%#x = %#x \n", reg_p, reg_data);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: fpga_peek_dump_mem
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA DS0 dump memory location.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_peek_dump_mem (void)
{
    uint16_t offset, mem_p;
    uint32_t mem_data;

    offset = gethex_answer("\nEnter FPGA dump memory offset[0x00 to 0x1FFF]:",
               0, 0, 0x1fff);
    mem_p = DS0_DUMP_BUFFER_BASE + offset;
    fpga_spi_direct_read(mem_p, 4, &mem_data);
    bsp_debug_printf("\n\r DS0 dump memory value @%#x = %#x ", mem_p, mem_data);

    return (PASSED);

}


/*****************************************************************
 *
 * Function: rd_verify_wr_conn_mem
 *
 * Description:
 *      This function will perform a read-verify-write test
 *      through memory using word accesses.
 *	Algorithm:
 *	Read the contents of 'addr' and compare against 'expval'.
 *	If compare OK, write with 'wrval'.  Cycle through memory
 *	for 'size' int in the direction, and using the address
 *	offset, specified in 'dir_val' (direction is a function
 *	of whether the offset value is positive or negative).
 *
 * Input :
 *	size - number of bytes
 *	rd_mask - read mask
 *	wrval - write value
 *	expval - expected value
 *	dir_val - direction to go through memory and by how much
 *	     1 = increment through memory, one word at a time
 *	    -1 = decrement through memory, one word at a time
 *	caddr - pointer to start address of memory test
 *
 * Output:
 *	1 - FAILED, 0 - PASSED
 *
 *****************************************************************
 */
int rd_verify_wr_conn_mem (int size, uint32_t rd_mask, uint32_t wrval, uint32_t expval,
                       int direction, uint32_t caddr)
{
    int count;
    uint32_t offset, rdval, mem_addr;

    offset = 0;
    for (count = 0; count < size/4; count++) {
        if (direction == MEM_INCREMENT_1) {
            mem_addr = caddr + offset;
        } else {  /* MEM_DECREMENT_1 */
            mem_addr = caddr - offset;
        }
        fpga_spi_indirect_read(mem_addr, 4, &rdval);
        rdval &= rd_mask;
        if (rdval != expval) {
            bsp_debug_printf( "%s conn mem err @ %x, read %x, expect %x",
                              direction == MEM_INCREMENT_1 ? "inc" : "dec", mem_addr,
                              rdval, expval);
            sprintf((char *)&(hd_if->errmsg), "%s conn mem err @ %lx, read %lx, expect %lx",
                              direction == MEM_INCREMENT_1 ? "inc" : "dec", mem_addr,
                              rdval, expval);
            return(FAILED);
        }

        fpga_spi_indirect_write(mem_addr, 4, wrval);

        offset += 4;
    }

    return(PASSED);

}


/*****************************************************************
 *
 * Function: fpga_conn_mem_test
 *
 * Description:
 *	This function will test the tdm connection memory.
 *	The connection memory maps input stream/timeslot to
 *	output stream/timeslot.  The connection memory is
 *	addressed using the output stream/timeslot number.
 *	the content of the output stream/timeslot address
 *	includes the input stream/timeslot number as well
 *	as other control bits.
 *
 *	Will test using 8Mbps speed.
 *
 *	for 2Mbps stream:
 *	    64 streams, 32 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) + (o_timeslot * 4 * 4)
 *
 *	for 8Mbps stream:
 *	    64 streams, 128 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) + (o_timeslot * 4)
 *
 *	for 16Mbps stream:
 *	    64 streams, 256 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) 
 *                      + ((o_timeslot%2) * 128 * 4) + ((o_timeslot>>1) * 4
 *
 *	for 32Mbps stream:
 *	    64 streams, 512 timeslots per stream
 *	    cmem_addr = base + (o_stream * 128 * 4) 
 *                      + ((o_timeslot%4) * 128 * 4) + ((o_timeslot>>2) * 4
 *
 *	connection memory map is as follows:
 *	    stream  0, timeslot 0 - 127, addr offset 0x0000
 *	    stream  1, timeslot 0 - 127, addr offset 0x0200
 *		. 	.	.
 *	    stream 14, timeslot 0 - 127, addr offset 0x1c00
 *	    stream 15, timeslot 0 - 127, addr offset 0x1e00
 *
 *	the connection memory must be accessed as an int.
 *
 * Input : None
 *
 * Output:
 *	1 - FAILED, 0 - PASSED
 *
 *****************************************************************
 */
int fpga_conn_mem_test (void)
{
    uint32_t bkgnd, forgnd, cid, rdval;
    int retval = PASSED;
    uint count, ix, stream, timeslot, conn_mem_size, tdm_num;
    int buf[5] = {0x00005aa5, 0x00003cc3, 0x00006996, 0x00000f0f, 0x00000000};
    uint16_t caddr, offset;

    bsp_debug_printf("\n\r FPGA Connection Memory test\r");

    offset = 0;

    conn_mem_size = TDMSW16_CONN_MEM_SIZE;
    tdm_num = TDMSW16_NUM_TDM_STREAM;

    /* disable all output enables */
    if (set_tdmsw_stream_ena(tdm_num, FALSE) == FAILED) {
        sprintf((char *)&(hd_if->errmsg), "\nFPGA memory test failed.Failed to disable 64 TDM streams");
        bsp_debug_printf("Failed to disable 64 TDM streams");
        return (FAILED);
    }

    /*
     * initialize connection memory
     * increment through memory
     */
    forgnd = TDMSW16_CM_PASSWORD | TDMSW16_CM_FORCEBYTE |
             TDMSW16_CM_ODRV | TDMSW16_CM_FORCELSB;
    for (ix = 0; ix < conn_mem_size/4; ix++) {
        fpga_spi_indirect_write(offset, 4, forgnd);
        offset += 4;
    }

   /*
    * connection memory test
    * a modified March C memory test
    */
    for (count = 0; count < 5; count++) {
        bsp_debug_printf("\n\r TDM connection memory march test %d \n", count);
        bkgnd = forgnd & TDMSW16_CONN_MEM_DATA_MASK;
        forgnd = buf[count] | TDMSW16_CM_PASSWORD;
        caddr = 0;
        /* increment through memory */
        if (rd_verify_wr_conn_mem(conn_mem_size,
                  TDMSW16_CONN_MEM_DATA_MASK,
                  forgnd, bkgnd, MEM_INCREMENT_1, caddr)) {
            retval = FAILED;
            break;
        }
        bkgnd = forgnd & TDMSW16_CONN_MEM_DATA_MASK;
        forgnd = (~forgnd & TDMSW16_CONN_MEM_PW_MASK) | TDMSW16_CM_PASSWORD;
        caddr = conn_mem_size - 4;
        if (rd_verify_wr_conn_mem(conn_mem_size,
                      TDMSW16_CONN_MEM_DATA_MASK,
                      forgnd, bkgnd, MEM_DECREMENT_1, caddr)) {
            retval = FAILED;
            break;
        }
    }
    if (retval == PASSED) {
        /*
         * connection memory address test
         * addr (output stream/timeslot) = input stream/timeslot
         * increment through memory (size * 4 because each
         * stream/timeslot is accessed as an int)
         */
        bsp_debug_printf( "\n\r TDM connection memory address test \n");
        offset = 0;
        for (stream = 0; stream < 8; stream++) {
            for (timeslot = 0; timeslot < NUM_16M_TIMESLOTS; timeslot++) {
                cid = (stream * 128) + (timeslot%2)*128 + (timeslot>>1);
                fpga_spi_indirect_write(offset, 4, cid | TDMSW16_CM_PASSWORD);
                offset += 4;
            }
        }

        offset = 0;
        for (stream = 0; stream < 8; stream++) {
            for (timeslot = 0; timeslot < NUM_16M_TIMESLOTS; timeslot++) {
                bkgnd = (stream * 128) + (timeslot%2)*128 + (timeslot>>1);
                fpga_spi_indirect_read(offset, 4, &rdval);
                offset += 4;
                rdval &= TDMSW16_CONN_MEM_DATA_MASK;
                if (rdval != (bkgnd | TDMSW16_CM_PASSWORD)) {
                    sprintf((char *)&(hd_if->errmsg), "\n conn mem addr err @0x%x \
                                     stream %d, tslot %d, read 0x%lx, expect 0x%lx",\
                                     offset, stream, timeslot, rdval, bkgnd);
                    
                    bsp_debug_printf( "conn mem addr err @0x%x, "
                    "stream %d, tslot %d, read 0x%x, expect 0x%x",
                    offset, stream, timeslot, rdval, bkgnd);
                   return (FAILED);
                }
            }
        }
    }
    return (retval);

}


/**********************************************************************
 *
 * Function: fpga_mem_test
 *
 * wrapper function for FPGA connection memory and DS0 dump memory test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int fpga_mem_test (void)
{
    bsp_debug_printf("\r\n FPGA connection memory test\n");
    if (fpga_conn_mem_test() == PASSED) {
       tdm_cleanup();
       return (PASSED);
    }
    tdm_cleanup();
    cterr('f', 0, "FPGA memory test failed.");
    return (FAILED);
}

/*******************************************************************
 *
 * Function:
 *      tdm_get_mem_offset
 *
 * Description:
 *      This function will return the tdm connection memory
 *      address offset for the given output stream, tslot, rate
 *	The equations are specified in the Oakenshield FPGA
 *	Hardware Functional Specification, EDCS-1106187. 
 *
 * Input  :
 *      stream   - Stream number to get connection memory offset
 *      timeslot - DS0 number of the stream
 *      rate     - TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M
 *
 * Output :
 *      cm_offset or -1 if FAILED
 *
 *******************************************************************
 */
int tdm_get_mem_offset (uint stream, uint tslot, stream_rate rate)
{
    int cm_offset;

    cm_offset = -1;
    if (stream >= TDMSW16_NUM_TDM_STREAM) {
       return (-1);
    }

    switch (rate) {
    case TDM_STREAM_2M:
        if (tslot < NUM_2M_TIMESLOTS) {
            cm_offset = ((stream * 128) + (tslot * 4)) * 4;
        }
        break;

    case TDM_STREAM_8M:
        if (tslot < NUM_8M_TIMESLOTS) {
            cm_offset = ((stream * 128) + tslot) * 4;
        }
        break;

    case TDM_STREAM_16M:
        if (tslot < NUM_16M_TIMESLOTS) {
            cm_offset = ((stream*128) + (tslot%2)*128 + (tslot>>1)) * 4;
        }
        break;

    case TDM_STREAM_32M:
        if (tslot < NUM_32M_TIMESLOTS) {
            cm_offset = ((stream*128) + (tslot%4)*128 + (tslot>>2)) * 4;
        }
        break;

    default:
        break;
    }

    return(cm_offset);


}

/*******************************************************************
 *
 * Function:
 *      tdm_get_input_cid
 *
 * Description:
 *      This function will return the input CID for the given input
 *      stream, input time slot and rate.
 *
 * Input  :
 *      stream   - Input Stream number
 *      timeslot - DS0 number of the input stream
 *      rate     - TDM_STREAM_2M, TDM_STREAM_8M, TDM_STREAM_16M,TDM_STREAM_32M
 *
 * Output :
 *      input CID on success or -1 if FAILED
 *
 *******************************************************************
 */
int tdm_get_input_cid (uint stream, uint tslot, stream_rate rate)
{
    int cid;

    cid = -1;
    if (stream >= TDMSW16_NUM_TDM_STREAM) {
        return (-1);
    }

    switch (rate) {
    case TDM_STREAM_2M:
        if (tslot < NUM_2M_TIMESLOTS) {
            cid = (stream * 128) + (tslot * 4) + 3;
        }
        break;

    case TDM_STREAM_8M:
        if (tslot < NUM_8M_TIMESLOTS) {
            cid = (stream * 128) + tslot;
        } 
        break;

    case TDM_STREAM_16M:
        if (tslot < NUM_16M_TIMESLOTS) {
            cid = (stream * 128) + (tslot%2)*128 + (tslot>>1);
        }
        break;

    case TDM_STREAM_32M:
        if (tslot < NUM_32M_TIMESLOTS) {
            cid = (stream * 128) + (tslot%4)*128 + (tslot>>2);
        }
        break;

    default:
        break;
    }

    return (cid);
}

/*******************************************************************
 *
 * Function:
 *	tdm_stream_connect
 *
 * Description:
 *	This function will make the output stream to input
 *	stream connection for the specified tdm stream.  The
 *	output stream/timeslot indexes into the connection
 *	memory.  The input stream/timeslot value is contained
 *	in the connection memory location indexed to by the
 *	output stream/timeslot.
 * Notes:
 *	The rate of input and output streams are determined dynamically
 *	by reading the rate register, so make sure the streams' rates are
 *	configured correctly before calling this function since that
 *	defines the maximum number of timeslots which can be connected.
 *
 * Input  : 
 *      istrm  - input stream
 *      itslot - input timeslot
 *      ostrm  - output stream
 *      otslot - output timeslot
 *      connect_sel - mode select: TDMSW16_CM_FORCEBYTE, TDMSW16_CM_FORCELSB,
 *				TDMSW16_CM_ODRV
 *
 * Output : PASSED of connect successfully, FAILED otherwise
 *
 *******************************************************************
 */
int tdm_stream_connect (int istrm, int itslot, int ostrm, int otslot, int connect_sel)
{
    int offset, orate, irate, input_cid;

    /* Determine output stream rate and connection memory offset */
    orate = get_tdmsw_stream_rate(ostrm);
    if (orate == -1) {
        bsp_debug_printf("Failed to set rate for stream %d", ostrm);
        return (FAILED);
    }
    offset = tdm_get_mem_offset(ostrm, otslot, orate);
    if (offset == -1) {
        bsp_debug_printf( "Failed to get connection memory offset. "
                          "output stream %d/tslot %d/rate %dMbps",
                          ostrm, otslot, tdm_rate[orate]);
        return (FAILED);
    }

    irate = get_tdmsw_stream_rate(istrm);
    if (irate == -1) {
        bsp_debug_printf( "Failed to set rate for stream %d", istrm);
        return (FAILED);
    }

    input_cid = tdm_get_input_cid (istrm, itslot, irate);
    if (input_cid == -1) {
        bsp_debug_printf( "Failed to get input cid. "
                          "input stream %d/tslot %d/rate %dMbps",
                           istrm, itslot, tdm_rate[irate]);
        return (FAILED);
    }


    /* Write input CID to the connection memory */
    fpga_spi_indirect_write(offset, 4, TDMSW16_CM_PASSWORD|connect_sel|input_cid);


    return (PASSED);

}

/*******************************************************************
 *
 * Function:
 *	tdm_stream_disconnect
 *
 * Description:
 *	This function will disable one output
 *	stream/timeslot connection of the tdm
 *	
 *
 * Input  : 
 *	output stream
 *	output timeslot
 *
 * Output :
 *	1 - FAILED, 0 - PASSED
 *
 *******************************************************************
 */
int tdm_stream_disconnect (int ostrm, int otslot)
{
    int offset, orate;

    /* Determine output stream rate and connection memory offset */
    orate = get_tdmsw_stream_rate(ostrm);
    if (orate == -1) {
        bsp_debug_printf( "Failed to set rate for stream %d", ostrm);
        return (FAILED);
    }
    offset = tdm_get_mem_offset(ostrm, otslot, orate);
    if (offset == -1) {
        bsp_debug_printf( "Failed to get connection memory offset. "
                          "output stream %d/tslot %d/rate %dMbps",
                          ostrm, otslot, tdm_rate[orate]);
        return (FAILED);
    }
    /* Write to the connection memory */
    fpga_spi_indirect_write(offset, 4, TDMSW16_CM_PASSWORD);
    msleep(30);


    return (PASSED);
}

/*******************************************************************
 *
 * Function:
 *      tdm_disconnect
 *
 * Description:
 *      This function will disable multiple output stream
 *      connections of the tdm, for the specified number of timeslots.
 *
 * Input  :
 *      start_stream
 *      num_streams
 *	start_timeslot
 *	num_timeslots
 *
 * Output :
 *      1 - FAILED, 0 - PASSED
 *
 *******************************************************************
 */
int tdm_disconnect (int stream_from, int num_streams,
                    int tslot_from, int num_tslots)
{
    int i, conn_mem_size;
    uint32_t offset = 0;

    conn_mem_size = TDMSW16_CONN_MEM_SIZE;

    for (i = 0; i < conn_mem_size/4; i++) {
        fpga_spi_indirect_write(offset, 4, TDMSW16_CM_PASSWORD);
        offset += 4;
    }

    return (PASSED);
}


/*******************************************************************
 *
 * Function:
 *      tdm_cleanup
 *
 * Description:
 *      This function will put the tdm in normal operation
 *      mode and disable the outputs
 *
 * Input  :
 *
 * Output : PASSED/FAILED
 *
 *******************************************************************
 */
int tdm_cleanup (void)
{
    uint tdm_num;
    uchar board_id;
    int ix;

    tdm_num = TDMSW16_NUM_TDM_STREAM;

    /* disable tdm stream outputs, do not drive the timeslots */
    if (tdm_disconnect(0, tdm_num, 0, NUM_16M_TIMESLOTS) == FAILED) {
        bsp_debug_printf("\n%s: Failed to disconnect all TDM streams.\n", __FUNCTION__);
        return (FAILED);
    }

    /* disable output enables */
    if (set_tdmsw_stream_ena(tdm_num, FALSE) == FAILED) {
        bsp_debug_printf("\n%s: Failed to disable TDM streams.\n", __FUNCTION__);
        return (FAILED);
    }

    /* put tdm into normal operation mode */
    if (set_tdmsw_lpbk(tdm_num, FALSE) == FAILED) {
        bsp_debug_printf("\n%s: Failed to disable lpbk for TDM streams\n", __FUNCTION__);
        return (FAILED);
    }

   /* set TDM streams in default rate 
       For TDMSW16:
       stream 0 - 3: 2Mbps
       stream 6 - 7: 2Mbps
       stream 4: 16Mbps
       stream 8: 16Mbps
       stream 13: 8Mbps
    */

    for (ix = 0; ix < 8; ix++) {
        if (ix == 4 || ix == 5) {
            continue; 
        }
        if (set_tdmsw_stream_rate(ix, TDM_STREAM_2M) == FAILED) {
            bsp_debug_printf("\n%s: Failed to configure rate for TDM stream %d\n",
                              __FUNCTION__, ix);
            return (FAILED);
        } else {
            if (set_tdmsw_stream_ena(ix, TRUE) == FAILED) {
                bsp_debug_printf("\n%s: Failed to enable TDM stream %d\n", __FUNCTION__, ix);
                return (FAILED);
            }
        }
    }

    if (set_tdmsw_stream_rate(4, TDM_STREAM_16M) == FAILED) {
        bsp_debug_printf("\n%s: Failed to configure rate for TDM stream 4\n",
                         __FUNCTION__);
        return (FAILED);
    } else {
        if (set_tdmsw_stream_ena(4, TRUE) == FAILED) {
            bsp_debug_printf("\n%s: Failed to enable TDM stream 4\n", __FUNCTION__);
            return (FAILED);
        }
    }

    if (set_tdmsw_stream_rate(8, TDM_STREAM_16M) == FAILED) {
        bsp_debug_printf("\n%s: Failed to configure rate for TDM stream 8\n",
                         __FUNCTION__);
        return (FAILED);
    } else {
        if (set_tdmsw_stream_ena(8, TRUE) == FAILED) {
            bsp_debug_printf("\n%s: Failed to enable TDM stream 8\n", __FUNCTION__);
            return (FAILED);
        }
    }

    if (set_tdmsw_stream_rate(13, TDM_STREAM_8M) == FAILED) {
        bsp_debug_printf("\n%s: Failed to configure rate for TDM stream 13\n",
                         __FUNCTION__);
        return (FAILED);
    } else {
        if (set_tdmsw_stream_ena(13, TRUE) == FAILED) {
            bsp_debug_printf("\n%s: Failed to enable TDM stream 13\n", __FUNCTION__);
            return (FAILED);
        }
    }

    /* allow tdm switch to free run */
    if (fpga_reset_tdmsw() == FAILED) {
        bsp_debug_printf("Failed to reset TDMSW");
        return (FAILED);
    }

    if (fpga_unreset_tdmsw() == FAILED) {
        bsp_debug_printf("Failed to take TDMSW out of reset");
        return (FAILED);
    }

    /* allow tdm time to stabilize */
    msleep(500);

    /* connect TDM switch */
    board_id = get_oak_id();
    oak_tdm_xc_setup(board_id, TRUE);


    return (PASSED);
}


/**********************************************************************
 *
 * Function: set_tdmsw_lpbk_test
 *
 * This function will do all the neccessary setup within TDMSW for 
 * either TDM loopback test or codec loopback tests.
 * Test will be performed at 2Mbps, each timeslot
 * tested individually, then all timeslots at the same time.
 *
 * Input : 
 *	       stream
 *         lpbk - TRUE: TDM loopback
 *                FALSE: TDM not in loopback mode
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int set_tdmsw_lpbk_test(int stream, int lpbk)
{
    int ts_cnt, num_ts, ts_start;
    char errstr[128];

    ts_start = 0;
    /* TDMSW loopback only supports E1 mode. BP clock 2.048M. */
    num_ts = NUM_16M_TIMESLOTS;

    /*
     * map connection memory from stream to stream.
     * test each 2 Mbps timeslots individually.
     */
    for (ts_cnt = ts_start; ts_cnt < num_ts; ts_cnt++) {
        if (lpbk == TRUE) {
            if (tdm_stream_connect(stream, ts_cnt, stream, ts_cnt,
                                   TDMSW16_CM_ODRV) == FAILED) {
                bsp_debug_printf("Failed to connect input stream %d with output "
                                 "stream %d", stream, stream);

                sprintf(errstr, "\nFailed to connect input stream %d with output stream %d"
                                  , stream, stream);
                strcat((char *)&(hd_if->errmsg), errstr);
                return (FAILED);
            }
        } else {
            if (tdm_stream_disconnect(stream, ts_cnt) == FAILED) {
                bsp_debug_printf("Failed to disconnect stream %d, timeslot %d\n",
                                  stream, ts_cnt);
                sprintf(errstr, "\nFailed to disconnect input stream %d with output stream %d"
                                  , stream, stream);
                strcat((char *)&(hd_if->errmsg), errstr);
                return (FAILED);
            }
        }
    }


    return (PASSED);
}

int get_cid (int rate, int strm, int ds_num)
{
    if (rate == TDM_STREAM_2M) {
        return ((strm * 128) + (ds_num * 4) + 3);
    } else if (rate == TDM_STREAM_16M) {
        return ((strm * 128) + ((ds_num % 2)* 128) + (ds_num >> 1));
    } else {
        bsp_debug_printf("%s(): Stream rate %d not supported", __FUNCTION__, rate);
        return (FAILED);
    }
}

int get_addr (int rate, int strm, int ds_num)
{
    if (rate == TDM_STREAM_2M) {
        return ((strm * 128) + (ds_num * 4)) * 4;
    } else if (rate == TDM_STREAM_16M) {
        return (((strm * 128) + ((ds_num % 2)* 128) + (ds_num >> 1))*4);
    } else {
        bsp_debug_printf("%s(): Stream rate %d not supported", __FUNCTION__, rate);
        return (FAILED);
    }
}


/*****************************************************************
 *
 * Function: tdmsw_force_byte_test
 *
 * Description:
 *	This function will test the force byte feature of the
 *	tdm connection memory as well as the DS0 dump feature.
 *	This test uses the LS 8 bits of connection memory
 *	for data generation and reads the contents of DS0 dump
 *	buffer for data verification.
 *	TDMSW16_CM_FORCEBYTE, when set, forces the LS 8 bits of
 *	connection memory to be output to stream out instead
 *	of the data coming in from the input stream.
 *	Since force byte affects the output, we must connect
 *	the test tdm stream with the DS0 dump stream (#61)
 *      so that we can see the actual data that is being sent out.  
 *
 *	Algorithm:
 *	Initialize and verify contents of DS0 dump buffer.
 *	Then setup connection memory.  Then enable tdm out.
 *	Now read the contents of
 *	DS0 dump buffer and verify that it is modified by the
 *	contents of connection memory.  The test is performed
 *	on stream 0, all 32 timeslots to stream 61, all 128 timeslot.
 *
 * Input :
 *	None
 *
 * Output:
 *	1 - FAILED, 0 - PASSED
 *
 *****************************************************************
 */
int tdmsw_force_byte_test (void)
{
    int istrm, itslot, ostrm, otslot, offset, ix;
    uint32_t reg_data, rd_data;
    uint16_t data_ptr;
    uint16_t fpga_reg;
    uchar data_pattern = 0xaa;

    bsp_debug_printf( "TDMSW force byte test ");

    istrm = 0;
    itslot = 0;
    otslot = 0;

    ostrm = TDM16_DS0_DUMP_STREAM;

    if (set_tdmsw_stream_rate(istrm, TDM_STREAM_8M) == FAILED) {
        cterr('f', 0, "Failed to configure rate for TDM stream %d",
              istrm);
        sprintf((char *)&(hd_if->errmsg), "Failed to configure rate for TDM stream %d",istrm);
        return (FAILED);
    }

    if (set_tdmsw_stream_rate(ostrm, TDM_STREAM_8M) == FAILED) {
        cterr('f', 0, "Failed to configure rate for TDM stream %d",
              ostrm);
        sprintf((char *)&(hd_if->errmsg), "Failed to configure rate for TDM stream %d",ostrm);
        return (FAILED);
    }

    /* connect istrm/itslot with ostrm/otslot */
    offset = ((ostrm * 128) + otslot) * 4;
    fpga_spi_indirect_write(offset, 4, TDMSW16_CM_PASSWORD | TDMSW16_CM_FORCEBYTE |
           TDMSW16_CM_ODRV | data_pattern);
    msleep(500);

    /* enable output stream */
    if (set_tdmsw_stream_ena(istrm, TRUE) == FAILED) {
        cterr('f', 0, "Failed to enable stream %d", istrm);
        sprintf((char *)&(hd_if->errmsg), "Failed to enable stream %d", istrm);
        return (FAILED);
    }

    if (set_tdmsw_stream_ena(ostrm, TRUE) == FAILED) {
        cterr('f', 0, "Failed to enable stream %d", ostrm);
        sprintf((char *)&(hd_if->errmsg), "Failed to enable stream %d", ostrm);
        return (FAILED);
    }

    /* start DS0 dump */
    fpga_reg = FPGA_GENERAL_DS0_DUMP_CNTL;
    fpga_spi_direct_write(fpga_reg, 1, (DS0_DUMP_GO | DS0_DUMP_TOP_RDY | DS0_DUMP_BOT_RDY));

    /* read from DS0 dump buffer to verify the data */
    for (ix = 0; ix < 1000; ix++) {
        fpga_spi_direct_read(FPGA_GENERAL_DS0_DUMP_CNTL, 1, &reg_data);
        if ((reg_data & DS0_DUMP_TOP_RDY) &&
            (reg_data & DS0_DUMP_BOT_RDY)) {
            break;
        }

        msleep(500);
    }

    if (ix == 1000) {
        cterr('f', 0, "Timeout waiting for DS0 dump buffer ready, ds0_dump_ctrl=%#x",
              reg_data);
        sprintf((char *)&(hd_if->errmsg), "Timeout waiting for DS0 dump buffer ready,\
                        ds0_dump_ctrl=%#lx", reg_data);
        return (FAILED);
    }


    data_ptr = DS0_DUMP_BUFFER_BASE;
    for (ix = 0; ix < DS0_DUMP_BUFFER_SIZE; ix++) {
        fpga_spi_direct_read(data_ptr, 1, &rd_data);
        if (rd_data != data_pattern) {
            cterr('f', 0, "DS0 dump buffer @%#x contains mismatch data. "
                  "expect = %#x, read = %#x", data_ptr,
                  data_pattern, rd_data);
            sprintf((char *)&(hd_if->errmsg),"DS0 dump buffer @%#x contains mismatch data. "
                  "expect = %#x, read i= %#lx", data_ptr,
                  data_pattern, rd_data);
            return (FAILED);
        }
        data_ptr++;
    }

    /* reset RDY bit and stop dump */
    fpga_spi_direct_write(fpga_reg, 1, DS0_DUMP_TOP_RDY | DS0_DUMP_BOT_RDY);

    /* Clear the DS0 dump and ready bits in the FPGA_INT_EVENT */
    fpga_spi_direct_write(fpga_reg, 1, DS0_DUMP_RDY | DS0_DUMP_ERR);

    bsp_debug_printf("\nTest passes! Doing cleanup!\n");

    return (tdm_cleanup());

}


/**********************************************************************
 *
 * Function: spi_flash_read 
 *
 * This function will do the spi flash read. 
 *
 * Input : 
 *	      opcode- read/write
 *         addr - the addr want to read
 *         size - size with read data
 *        rd_data- 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int 
spi_flash_read (int opcode, uint32_t addr, int size, uint8_t *rd_data)
{
    uint32_t spi_data;
    uint32_t result;
    int ix;

    for (ix = 0; ix < 2000; ix++) {
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 1, &result);
        if (result & FPGA_SPI_RD_FIFO_EMPTY) {
            break;
        }
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_DATA, 1, &spi_data);
        msleep(2);
    }
    if (ix >= 2000) {
        bsp_debug_printf("\n\r Time out waiting for read FIFO empty.");
        return(-1);
    }
    /* clear fpga_spi_stat register and wait for read FIFO empty. */
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 1, &result);
    if (result & FPGA_SPI_WR_FIFO_ORUN) {
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_FIFO, 1, FPGA_SPI_WR_FIFO_ORUN);
    }
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_DONE, 1, &result);
    if (result & FPGA_SPI_DONE) {
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_DONE, 1, FPGA_SPI_DONE);
    }
    /* set read data size */
    fpga_spi_direct_write(FPGA_SPI_REG_BASE_RD_SIZE, 1, size-1);

    /* set read address and opcode */
    result = opcode << 24 | addr;
    fpga_spi_direct_write(FPGA_SPI_REG_BASE_ADDR, 4, result);

    /* set bit 0 and bit 2 of fpga_spi_ctrl register per
       requirement of the opcode used. */
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_CTRL, 1, &spi_data);

    if (opcode == RD_DATA_BYTES) {
        spi_data |= FPGA_SPI_ADDR_EN;
    } else {
        spi_data &= ~FPGA_SPI_ADDR_EN;
    }

    /* Set for READ */
    spi_data &= ~(FPGA_SPI_WRITE | FPGA_SPI_DUMMY_BYTE_EN);
    fpga_spi_direct_write(FPGA_SPI_REG_BASE_CTRL, 1, spi_data);

    /* wait for spi flash read to complete by polling DONE bit
       in fpga_spi_stat register */
    for (ix = 0; ix < 2000; ix++) {
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_DONE, 1, &result);
        if (result & FPGA_SPI_DONE) {
            break;
        }
        msleep(2);
    }

    if (ix >= 2000) {
        bsp_debug_printf("\n\r Time out waiting for read to complete.");
        return (-1);
    }
    /* read data */
    for (ix = 0; ix < size; ix++) {
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_DATA, 1, &result);
        *rd_data++ = (uint8_t)result;
    }
    for (ix = 0; ix < 2000; ix++) {
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 1, &result);
        if (result & FPGA_SPI_RD_FIFO_EMPTY) {
            break;
        }
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_DATA, 1, &spi_data);
        msleep(2);
    }

    fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_DONE, 1, FPGA_SPI_DONE);

    return (0);
}

static int 
spi_flash_write (int opcode, uint32_t addr, int size, uint8_t *wr_data)
{
    uint8_t status_reg = 0;
    int ix;
    uint32_t ctrl, result;

    /* clear fpga_spi_stat register. */
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 1, &result);
    if (result & FPGA_SPI_WR_FIFO_ORUN) {
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_FIFO, 1, FPGA_SPI_WR_FIFO_ORUN);
    }
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_DONE, 1, &result);
    if (result & FPGA_SPI_DONE) {
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_DONE, 1, FPGA_SPI_DONE);
    }

    /* poll "Write In Progress" bit by reading SPI flash status 
       register and wait bit 0 to be 0. */
    for (ix = 0; ix < 3000; ix++) {
        if (spi_flash_read(RD_STATUS, 0, 1, &status_reg) < 0) {
            bsp_debug_printf("\n\r Failed to read SPI flash status register.");
            return (-1);
        }
        if (!(status_reg & RDSR_WIP)) {
            break;
        }
        msleep(2);
    }

    if (ix >= 3000) {
        bsp_debug_printf("\n\r Time out waiting for SPI flash ready.");
        return (-1);
    }

    /* check for write enable bit in SPI flash status register */
    if (opcode != WRITE_ENABLE) {
        if (!(status_reg & RDSR_WEL)) {
            bsp_debug_printf("\n\r SPI flash write is not enabled. %x", status_reg);
            return (-1);
        }
    }

    /* set write address and opcode */
    fpga_spi_direct_write(FPGA_SPI_REG_BASE_ADDR_OPC, 1, opcode);
    if ((opcode == SECTOR_ERASE) || (opcode == PAGE_PROGRAM)) {
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_ADDR_LO, 1, addr & 0xFF);
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_ADDR_ME, 1, (addr >> 8) & 0xFF);
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_ADDR_HI, 1, (addr >> 16) & 0xFF);
    }

    if ((opcode == PAGE_PROGRAM) || (opcode == WRITE_STATUS)) {
        /* write the data to the write FIFO */
        for (ix = 0; ix < size; ix++) {
            result = (uint32_t)*wr_data++;
            fpga_spi_direct_write(FPGA_SPI_REG_BASE_DATA, 1, result);
        }
    }

    fpga_spi_direct_read(FPGA_SPI_REG_BASE_CTRL, 1, &ctrl);
    /* set bit 1 of fpga_spi_ctrl register to 1 for write operation 
       and set bit 0 and bit 2 per requirement of the opcode used. */
    if ((opcode == SECTOR_ERASE) || (opcode == PAGE_PROGRAM)) {
        ctrl |= FPGA_SPI_ADDR_EN;
    } else {
        ctrl &= (~FPGA_SPI_ADDR_EN);
    }   

    fpga_spi_direct_write(FPGA_SPI_REG_BASE_CTRL, 1, (ctrl | FPGA_SPI_WRITE));

    /* wait for spi flash write to complete by polling DONE bit
       in fpga_spi_stat register */
    for (ix = 0; ix < 6000; ix++) {
        fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_DONE, 1, &result);
        if ((result & FPGA_SPI_DONE) == FPGA_SPI_DONE) {
            fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_DONE, 1, FPGA_SPI_DONE);
            break;
        }
        msleep(2);
    }

    if (ix >= 6000) {
        bsp_debug_printf("\n\r Time out waiting for write to complete.");
        return (-1);
    }

    /* check bit 2 of fpga_spi_stat register to make sure 
       write FIFO is empty. If not, error has occured. */
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 1, &result);
    if (!(result & FPGA_SPI_WR_FIFO_EMPTY)) {
        bsp_debug_printf("\n\r Error: SPI flash write FIFO is not empty.");
        return (-1);
    }

    /* clear fpga_spi_stat register. */
    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 1, &result);
    if (result & FPGA_SPI_WR_FIFO_ORUN) {
        fpga_spi_direct_write(FPGA_SPI_REG_BASE_STAT_FIFO, 1, FPGA_SPI_WR_FIFO_ORUN);
    }

    return (0);  
}

/**********************************************************************
 *
 * Function: erase_upgrade_sectors 
 *
 * This function will erase the fpga boot flash sectors. 
 *
 * Input : 
 *	      start_addr - 
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int erase_upgrade_sectors (uint32_t start_addr)
{
    int ix;
    uint32_t fpga_sector_addr;

    bsp_debug_printf("\n\r Erasing upgrade sectors...");
    if (is_phoenix()) {
         bsp_debug_printf("\n\r Erasing 4MB upgrade sectors...");
        /* erase SPI flash upgrade region first */
        for (ix = (PHOENIX_SPI_FPGA_UPGRADE_SECTOR_SIZE - 1); ix >= 0; ix--) {
            /* erase all the 64 * 64kb sectors(4MB) for SPI flash in upgrade section */
            /* enable SPI flash write first*/
            bsp_debug_printf(" %d", ix+16);
            if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL)) {
                bsp_debug_printf("\n\r Failed to enable SPI flash write in erase.");
                return (-1);
            }

            /* erase sector (64KB) from top to bottom, Upgrade Image Header removal first*/
            fpga_sector_addr = ix * SPI_ERASE_SECTOR_SIZE + start_addr;
            if (spi_flash_write(SECTOR_ERASE, fpga_sector_addr, 0, NULL)) {
                bsp_debug_printf("\n\r Failed to erase sector.");
                return (-1);
            }
        }
    } else {
        /* erase SPI flash upgrade region first */
        for (ix = (SPI_FPGA_UPGRADE_SECTOR_SIZE - 1); ix >= 0; ix--) {
            /* erase all the 16 64kb sectors(1MB) for SPI flash in upgrade section */
            /* enable SPI flash write first*/
            bsp_debug_printf(" %d", ix+16);
            if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL)) {
                bsp_debug_printf("\n\r Failed to enable SPI flash write in erase.");
                return (-1);
            }

            /* erase sector (64KB) from top to bottom, Upgrade Image Header removal first*/
            fpga_sector_addr = ix * SPI_ERASE_SECTOR_SIZE + start_addr;
            if (spi_flash_write(SECTOR_ERASE, fpga_sector_addr, 0, NULL)) {
                bsp_debug_printf("\n\r Failed to erase sector.");
                return (-1);
            }
        }
    }
    bsp_debug_printf("\n");
    return (0);
} 

/*****************************************************************
 *
 * Function: fpga_upgrade
 *
 * Description: This function performs FPGA upgrade
 *
 * Input: start_addr
 *
 * Output: PASSED
 *
 *****************************************************************/
static
int fpga_upgrade(uint32_t start_addr)
{
    int ix;
    uint32_t fpga_image_date, fpga_dest_addr;
    char img_date[FPGA_DATE_LEN], buffer[10];
    uint32_t mb_ctrl;
    uint8_t flash_id[4], *dbuf;
    uint32_t current_date;
    uint8_t  value;

    if (is_phoenix()) {
        fpga_image_size = PHOENIX_SPI_FLASH_UPGRADE_SIZE;
        start_addr = PHOENIX_SECONDARY_FPGA_IMAGE_START_ADDR;
    } else {
        fpga_image_size = SPI_FLASH_UPGRADE_SIZE;
    }
    bsp_debug_printf("\n\r fpga_image_size=%x",fpga_image_size);
    if (start_addr == GOLDEN_FPGA_IMAGE_START_ADDR) {
        bsp_debug_printf("\n\r Upgrading GOLDEN FPGA now....");
    } else if ((start_addr == SECONDARY_FPGA_IMAGE_START_ADDR) ||
               (start_addr == PHOENIX_SECONDARY_FPGA_IMAGE_START_ADDR)){
        bsp_debug_printf("\n\r Upgrading SECONDARY FPGA now....");
    } else {
        bsp_debug_printf("\n\r Wrong Address 0x%x, aborting....", start_addr);
        return (FAILED);
    }
    if (spi_flash_read(RD_IDENTIFICATION, 0, 3, flash_id) < 0) {
        bsp_debug_printf("\n\r FPGA Flash read ID failed");
    }
    bsp_debug_printf("\n\r FPGA Flash ID: %x %x %x", flash_id[0], flash_id[1], flash_id[2]);
    fpga_spi_direct_read(FPGA_GENERAL_FPGA_DATA, 4, &current_date);

    if (is_phoenix()) {
        for (ix = 0; ix < FPGA_DATE_LEN; ++ix) {
            img_date[ix] = fpga_image[PHOENIX_UP_FILE_FPGA_REV_OFFSET+4+ix];
        }
    } else {
        for (ix = 0; ix < FPGA_DATE_LEN; ++ix) {
            img_date[ix] = fpga_image[FPGA_REV_OFFSET+4+ix];
        }
    }
    fpga_image_date = (uint32_t)img_date[3] << 24 |
                      (uint32_t)img_date[2] << 16 |
                      (uint32_t)img_date[1] << 8 |
                      (uint32_t)img_date[0];

    /* FPGA upgrade is in progress, use printf instead of bsp_debug_printf
     * there is no need to log into buffer since module will be reset after done
     * unless there is error during upgrading
     */
    bsp_debug_printf("\n\r Start FPGA upgrade from current revision %08X to bundled revision %08X", 
                     current_date, fpga_image_date);

    bsp_debug_printf("\r\n Do you want to Continue?");  
    bsp_debug_printf(">");  
    debug_console_gets(buffer, sizeof(buffer));
    if (buffer[0] != 'Y' && buffer[0] != 'y') {
        return (PASSED);
    }


    fpga_spi_indirect_read(MB_CTRL, 4, &mb_ctrl);
    /* Clear the cache version of FPGA header */
    /* This will force FPGA to refresh MB header */
    mb_ctrl &= 0xffffffe;
    fpga_spi_indirect_write(MB_CTRL, 4, mb_ctrl);

    /* Unprotect the SPI flash all sectors */
    /* enable SPI flash write */
    bsp_debug_printf("\n\r Unprotect the SPI flash for all sectors...");
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL)) {
        bsp_debug_printf("\n\r unprotect::Failed to enable SPI flash write.");
        return (-1);
    }
    /* Before programming, set status register BP2:BP0=000 to make 
       all secotrs unprotected. */
    value = UNPROTECT_ALL_SECTORS;
    if (spi_flash_write(WRITE_STATUS, 0, 1, &value)) {
        bsp_debug_printf("\n\r Failed to write to unprotect cmd");
        return (-1);
    }
    /* erase SPI flash upgrade region first */
    if (erase_upgrade_sectors(start_addr)) {
        bsp_debug_printf("\n\r upgrade sector erase failure");
        return (-1);
    }

    bsp_debug_printf("\n\r Sector erase successfully, programming upgrade region now...\n\r");

    /* program to the SPI flash upgrade region */
    fpga_dest_addr = start_addr;
    dbuf = fpga_image;

    if ((fpga_image_size != SPI_FLASH_UPGRADE_SIZE) && (!is_phoenix())){
        bsp_debug_printf("\n\r The bundled upgrade FPGA image size is not correct! Expected 0x%x, actual 0x%x",
                             SPI_FLASH_UPGRADE_SIZE, fpga_image_size);
        return (-1);
    }

    for (ix = 0; ix < fpga_image_size; ix += FPGA_SPI_PAGE_SIZE) {
        if (!(ix&1023)) {
            bsp_debug_printf("!");
        }

        /* enable SPI flash write */
        if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL)) {
            bsp_debug_printf("\n\r Write: Failed to enable SPI flash write.");
            return (-1);
        }
        /* Program header will store at TDM FPGA upgrade file offset 0x1ff000
           Need program header info to 0x7FF000 */
        if ((is_phoenix()) && (ix == PHOENIX_UP_FILE_FPGA_REV_OFFSET)) {
            bsp_debug_printf("\n\r Write header info to SPI flash 0x7FF000.");
            if (spi_flash_write(PAGE_PROGRAM, PHOENIX_FPGA_REV_OFFSET, FPGA_SPI_PAGE_SIZE, dbuf)) {
                bsp_debug_printf("\n\r Write: Failed to write to SPI flash.");
                erase_upgrade_sectors(start_addr);
                return (-1);
            }
        } else {
            if (spi_flash_write(PAGE_PROGRAM, fpga_dest_addr, FPGA_SPI_PAGE_SIZE, dbuf)) {
                bsp_debug_printf("\n\r Write: Failed to write to SPI flash.");
                erase_upgrade_sectors(start_addr);
                return (-1);
            }
        }        

        fpga_dest_addr += FPGA_SPI_PAGE_SIZE;
        dbuf += FPGA_SPI_PAGE_SIZE;
    }
    bsp_debug_printf("\n\r FPGA upgrade done, powercycle system to take effect!");

    return (PASSED);
}

/*****************************************************************
 *
 * Function: oak_fpga_upgrade_secondary
 *
 * Description: This function upgrades secondary Oakenshield PFGA image in
  *             the SPI flash.  
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int oak_fpga_upgrade_secondary (void)
{
    uint32_t image_size = SPI_FLASH_UPGRADE_SIZE;

    /* Phoenix flash image size change */
    if (is_phoenix()) {
        image_size = PHOENIX_SPI_FLASH_UPGRADE_SIZE;
    } 

    if (fpga_image_download(image_size) == FAILED) {
        return (FAILED);
    } 
    
    return (fpga_upgrade(SECONDARY_FPGA_IMAGE_START_ADDR));
}


/*******************************************************************
 *
 * Function: peek_spi_flash
 *
 * Description: This function will peak SPI flash
 *
 * Input  : none
 *
 * Output : PASSED/FAILED
 *
 *******************************************************************
 */
int peek_spi_flash (void)
{
    int opcode, size, ix;
    uint32_t addr;
    uint8_t data[4];

    opcode = gethex_answer("\nEnter opcode to access SPI flash[03, 05, 9F]:",
               0x03, 0x03, 0x9f);

    if ((opcode != RD_IDENTIFICATION) && (opcode != RD_STATUS)
        && (opcode != RD_DATA_BYTES)) {
        bsp_debug_printf("\nWrong opcode to access SPI flash!\n");
        return (FAILED);
    }

    if (opcode == RD_IDENTIFICATION) {
        size = 4;
    } else {
        size = 1;
    }

    if ((opcode == RD_DATA_BYTES) || (opcode == RD_DATA_BYTES_HIGH_SPEED)) {
        addr = gethex_answer("\nEnter address to access SPI flash"
                             "[0x100000 - 0x200000]:",
                             0x100000, 0x100000, 0x200000);
    } else {
        addr = 0;
    }

    if (spi_flash_read(opcode, addr, size, data) == FAILED) {
        bsp_debug_printf("\nFailed to read from SPI flash.\n");
        return (FAILED);
    }

    for (ix = 0; ix < size; ix++) {
        bsp_debug_printf("\nData = %#x", data[ix]);
    }

    return (PASSED);
}

/*******************************************************************
 *
 * Function: poke_spi_flash
 *
 * Description: This function will poke SPI Flash
 *
 * Input  :  None
 *
 * Output : PASSED/FAILED
 *
 *******************************************************************
 */
int poke_spi_flash (void)
{
    uint32_t addr;
    uint8_t data;
    unsigned char wr_status;

    addr = gethex_answer("\nEnter address to access SPI flash"
                         "[0x100000 - 0x1fffff]:",
                         0x100000, 0x100000, 0x1fffff);

    data = gethex_answer("\nEnter data to write to SPI flash[0 - 0xff]:",
                         0, 0, 0xff);

    /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        bsp_debug_printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    /* Before poking SPI flash, set status register BP2:BP0=000 to make 
       all secotrs unprotected. */
    wr_status = 0x00;
    if (spi_flash_write(WRITE_STATUS, 0, 1, &wr_status) == FAILED) {
        bsp_debug_printf("Failed to write to SPI flash status register.\n");
        return (FAILED);
    }

    /* erase sector (64KB) */
    if (spi_flash_write(SECTOR_ERASE, addr & 0x1f0000, 0, NULL) == FAILED) {
        bsp_debug_printf("Failed to erase sector.\n");
        return (FAILED);
    }

     /* enable SPI flash write */
    if (spi_flash_write(WRITE_ENABLE, 0, 0, NULL) == FAILED) {
        bsp_debug_printf("Failed to enable SPI flash write.\n");
        return (FAILED);
    }

    if (spi_flash_write(PAGE_PROGRAM, addr, 1, &data) == FAILED) {
        bsp_debug_printf("Failed to write to SPI flash.\n");
        return (FAILED);
    }
    return (PASSED);

}

/*******************************************************************
 *
 * Function: show_mb_regs
 *
 * Description: This function will display the contents of the
 *              Oakenshiled FPGA Multiboot registers
 *
 * Input  : None
 *
 * Output : Always return PASSED to avoid compilation warning
 *
 *******************************************************************
 */
int show_mb_regs (void)
{
    uint32_t reg_data;

    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_CTRL_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot Control Register  @%x = %x",
                         FPGA_MB_REG_BASE+MB_CTRL_OFFSET, reg_data);
    }
    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_STAT_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot Status Register  @%x = %x",
                         FPGA_MB_REG_BASE+MB_STAT_OFFSET, reg_data);
    }
    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_HDR_ID_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot Header ID Register @%x = %x",
                         FPGA_MB_REG_BASE+MB_HDR_ID_OFFSET, reg_data);
    }
    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_HDR_DATE_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot Header Date Register @%x = %x",
                         FPGA_MB_REG_BASE+MB_HDR_DATE_OFFSET, reg_data);
    }
    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_HDR_FLAG_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot Header Flag Register @%x = %x",
                         FPGA_MB_REG_BASE+MB_HDR_FLAG_OFFSET, reg_data);
    }
    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_HDR_MAGIC_NUM_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot Header Magic Num Register @%x = %x",
                         FPGA_MB_REG_BASE+MB_HDR_MAGIC_NUM_OFFSET, reg_data);
    }
    if (fpga_spi_indirect_read(FPGA_MB_REG_BASE+MB_HISTORY_OFFSET, 4,
           &reg_data) == PASSED) {
        bsp_debug_printf("\n\rMultiboot State History Register @%x = %x",
                         FPGA_MB_REG_BASE+MB_HISTORY_OFFSET, reg_data);
    }

    return (PASSED);
}

/*****************************************************************
 *
 * Function: mb_peek_reg
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA Multiboot registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int mb_peek_reg (void)
{
    uint32_t base_addr;
    ushort offset;
    uint32_t reg_data;

    base_addr = FPGA_MB_REG_BASE;

    offset = gethex_answer("\nEnter Multiboot register offset[0x00 to 0x18]:",
               0, 0, 0x18);

    /* all the Multiboot registers are 4 bytes aligned */
    offset &= 0xfc;

    if (fpga_spi_indirect_read((base_addr + offset), 4, &reg_data) == PASSED) {
        bsp_debug_printf("\n\r register value @ offset %#x = %#.8x ",
                        (base_addr+offset), reg_data);
        return (PASSED);
    } else {
        return (FAILED);
    }

}

/*****************************************************************
 *
 * Function: mb_poke_reg
 *
 * Description: This function performs a write to NPU memory-mapped
 *              FPGA Multiboot registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int mb_poke_reg (void)
{
    uint32_t base_addr;
    ushort offset;
    uint32_t reg_data;

    base_addr = FPGA_MB_REG_BASE;
    offset = gethex_answer("\nEnter Multiboot register offset[0x00 to 0x18]:",
               0, 0, 0x18);

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:",
                 0, 0, 0xffffffff);


    /* all the Multiboot registers are 4 bytes aligned */
    offset &= 0xfc;

    if (fpga_spi_indirect_write((base_addr + offset), 4, reg_data)== PASSED) {
        if (fpga_spi_indirect_read((base_addr + offset), 4, &reg_data) == PASSED) {
            bsp_debug_printf("\n register value @ offset %#x = %#.8x ",
                            (base_addr+offset), reg_data);
            return (PASSED);
        }
    }
    return (FAILED);


}

/*****************************************************************
 *
 * Function: spi_peek_reg
 *
 * Description: This function performs a read to NPU memory-mapped
 *              FPGA SPI flash registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int spi_peek_reg (void)
{
    uint16_t base_addr, offset;
    uint32_t reg_data;

    base_addr = FPGA_SPI_REG_BASE;
    offset = gethex_answer("\nEnter SPI flash register offset[0x00 to 0x13]:",
               0, 0, 0x13);


    fpga_spi_direct_read(base_addr + offset, 1, &reg_data);

    bsp_debug_printf("\n\r register value @%#x = %#x ", (base_addr+offset), reg_data);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: spi_poke_reg
 *
 * Description: This function performs a write to NPU memory-mapped
 *              FPGA SPI flash registers.  
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************/
int spi_poke_reg (void)
{
    uint16_t base_addr, offset;
    uint32_t reg_data;

    base_addr = FPGA_SPI_REG_BASE;

    offset = gethex_answer("\nEnter SPI flash register offset[0x00 to 0x13]:",
               0, 0, 0x13);

    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFF]:",
                 0, 0, 0xff);

    fpga_spi_direct_write(base_addr + offset, 1, reg_data);
    fpga_spi_direct_read(base_addr + offset, 1, &reg_data);

    bsp_debug_printf("\n\r register value @%#x = %#x ", (base_addr+offset), reg_data);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: si32261_reset
 *
 * Description: This function performs reset/unreset to MB FXS/DID CODEC.
 *
 * Input: TRUE - reset, FALSE - unreset
 *
 * Output: None
 *
 *****************************************************************/
void si32261_reset (boolean reset)
{
    uchar board_id = get_oak_id();
    if (board_id == BOARD_72FXS) {
        if (reset == TRUE) {
            fpga_spi_direct_write(FPGA_TDM_0_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_1_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_2_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_3_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_4_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_5_RESET, 1, 0xFF); 
	        msleep(500);
        } else {
            fpga_spi_direct_write(FPGA_TDM_0_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_1_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_2_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_3_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_4_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_5_RESET, 1, 0x0); 
	        msleep(500);
        }
    } else if (is_phoenix()) {
        if (reset == TRUE) {
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_0_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_1_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_2_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_3_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_4_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_5_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_6_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_7_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_8_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_9_RESET, 1, 0xFF); 
	        msleep(500);
        } else {
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_0_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_1_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_2_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_3_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_4_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_5_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_6_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_7_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_8_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_9_RESET, 1, 0x0); 
	        msleep(500);
        }    
    } else {
        if (reset == TRUE) {
            fpga_spi_direct_write(FPGA_TDM_0_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_2_RESET, 1, 0xFF); 
	        msleep(500);
        } else {
            fpga_spi_direct_write(FPGA_TDM_0_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_2_RESET, 1, 0x0); 
	        msleep(500);
        }
    }

}

/*****************************************************************
 *
 * Function: si3050_reset
 *
 * Description: This function performs reset/unreset to MB FXO CODEC.
 *
 * Input: TRUE - reset, FALSE - unreset
 *
 * Output: None
 *
 *****************************************************************/
void si3050_reset (boolean reset)
{
    /*
     * Phoenix FXO only work on TDM3
     */
    if (is_phoenix()) {
        if (reset == TRUE) {
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_3_RESET, 1, 0xFF); 
	        msleep(20);
        } else {
            fpga_spi_direct_write(FPGA_PHOENIX_TDM_3_RESET, 1, 0x0); 
	        msleep(20);
        }
    } else {
        if (reset == TRUE) {
            fpga_spi_direct_write(FPGA_TDM_1_RESET, 1, 0xFF); 
            fpga_spi_direct_write(FPGA_TDM_3_RESET, 1, 0xFF); 
	        msleep(20);
        } else {
            fpga_spi_direct_write(FPGA_TDM_1_RESET, 1, 0x0); 
            fpga_spi_direct_write(FPGA_TDM_3_RESET, 1, 0x0); 
	        msleep(20);
        }
    }

}


/*****************************************************************
 *
 * Function: get_oak_id
 *
 * Descrition: This function returns the Oakenshield ID.
 *
 * Input: None
 *
 * Output: Board ID, or FF if not set
 *
 *****************************************************************
 */
uchar get_oak_id (void)
{
    uint16_t fpga_reg;
    uint32_t reg_data;

    /* base on HW board type report SKU if it exist */
    switch (phoenix_hw_brd_type_flag) {
        case PHOENIX_144FXS_HW_BRD_TYPE:
            return(PHOENIX_144FXS);
        case PHOENIX_132FXS_6FXO_HW_BRD_TYPE:
            return(PHOENIX_132FXS_6FXO);
        case PHOENIX_84FXS_6FXO_HW_BRD_TYPE:
            return(PHOENIX_84FXS_6FXO);
        default:
            break;
    }

    /* No match HW board type will check SKU by TDM FPGA */
    fpga_reg = FPGA_GENERAL_BOARD_ID;
    fpga_spi_direct_read(fpga_reg, 1, &reg_data);
   
    return(reg_data & BOARD_ID_MASK);
}


/*******************************************************************
 *
 * Function: show_gen_regs
 *
 * Description: This function will display the contents of the
 *              Oakenshield FPGA General registers
 *
 * Input  : None
 *
 * Output : Always return PASSED to avoid compilation warning
 *
 *******************************************************************
 */
int show_gen_regs (void)
{
    uint32_t reg_data;

    fpga_spi_direct_read(FPGA_GENERAL_FPGA_DATA, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA build date information Register @%x = %x",
              FPGA_GENERAL_FPGA_DATA, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_MISC_CONTROL, 1, &reg_data);
    bsp_debug_printf("\n\rMisc controls Register  @%x = %x",
              FPGA_GENERAL_MISC_CONTROL, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_BOARD_ID, 1, &reg_data);
    bsp_debug_printf("\n\rBoard Identification Register  @%x = %x",
              FPGA_GENERAL_BOARD_ID, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_FPGA_INT_EVENT, 1, &reg_data);
    bsp_debug_printf("\n\rFPGA interrupt sources Register @%x = %x",
              FPGA_GENERAL_FPGA_INT_EVENT, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_FPGA_INT_DIAG_TEST, 1, &reg_data);
    bsp_debug_printf("\n\rFPGA interrupt diagnostic test Register @%x = %x",
              FPGA_GENERAL_FPGA_INT_DIAG_TEST, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_FPGA_INT_EVENT_ENA, 1, &reg_data);
    bsp_debug_printf("\n\rFPGA interrupt event mask Register @%x = %x",
              FPGA_GENERAL_FPGA_INT_DIAG_TEST, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDMSW_CMD_STATUS, 1, &reg_data);
    bsp_debug_printf("\n\rTDMSW indirect access command status Register @%x = %x",
              FPGA_GENERAL_TDMSW_CMD_STATUS, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDMSW_ADR_LO, 2, &reg_data);
    bsp_debug_printf("\n\rTDMSW indirect access address Register @%x = %x",
              FPGA_GENERAL_TDMSW_ADR_LO, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDMSW_DATA_0, 4, &reg_data);
    bsp_debug_printf("\n\rTDMSW indirect access read/write data Register @%x = %x",
              FPGA_GENERAL_TDMSW_DATA_0, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, 2, &reg_data);
    bsp_debug_printf("\n\rTDM PLL control and status Register @%x = %x",
              FPGA_GENERAL_TDM_PLL_CTRL_STAT_1C, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_FPGA_REV, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA revision information Register @%x = %x",
              FPGA_GENERAL_FPGA_REV, reg_data);

    fpga_spi_direct_read(FPGA_GENERAL_DS0_DUMP_CNTL, 1, &reg_data);
    bsp_debug_printf("\n\rDS0 Dump control Register @%x = %x",
              FPGA_GENERAL_DS0_DUMP_CNTL, reg_data);

    return (PASSED);

}

/*******************************************************************
 *
 * Function: show_spi_regs
 *
 * Description: This function will display the contents of the
 *              Oakenshield FPGA SPI Flash registers
 *
 * Input  : None
 *
 * Output : Always return PASSED to avoid compilation warning
 *
 *******************************************************************
 */
int show_spi_regs (void)
{
    uint32_t reg_data;

    fpga_spi_direct_read(FPGA_SPI_REG_BASE_CTRL, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA SPI Control Register @%x = %x",
              FPGA_SPI_REG_BASE_CTRL, reg_data);

    fpga_spi_direct_read(FPGA_SPI_REG_BASE_STAT_FIFO, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA SPI Status Register @%x = %x",
              FPGA_SPI_REG_BASE_STAT_FIFO, reg_data);

    fpga_spi_direct_read(FPGA_SPI_REG_BASE_RD_SIZE, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA SPI Read Size Register @%x = %x",
              FPGA_SPI_REG_BASE_RD_SIZE, reg_data);

    fpga_spi_direct_read(FPGA_SPI_REG_BASE_DATA, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA SPI Data Register @%x = %x",
              FPGA_SPI_REG_BASE_DATA, reg_data);

    fpga_spi_direct_read(FPGA_SPI_REG_BASE_ADDR, 4, &reg_data);
    bsp_debug_printf("\n\rFPGA SPI Address and Opcode Register @%x = %x",
              FPGA_SPI_REG_BASE_ADDR, reg_data);


    return (PASSED);

}



/*******************************************************************
 *
 * Function: oak_tdm_xc_setup
 *
 * Description: This function will setup the TDM IOM2/McBSP TS Mapping as
 *              described in the Oakenshield FPGA HFS (EDCS-1259128), and
 *		        Oakenshield SDS (EDCS-1280942)
 *
 * Input  : Board ID, connect/disconnect
 *
 * Output : None
 *
 *******************************************************************
 */
void oak_tdm_xc_setup(uchar board_id, int connect)
{
    int ix = 0;

    switch (board_id) {
    case BOARD_16FXS_2FXO:
        bsp_debug_printf("\n\r BOARD_16FXS_2FXO\n");
        break;
    case BOARD_24FXS_4FXO:
        bsp_debug_printf("\n\r BOARD_24FXS_4FXO\n");
        break;
    case BOARD_8FXS_12FXO:
        bsp_debug_printf("\n\r BOARD_8FXS_12FXO\n");
        break;
    case BOARD_72FXS:
        bsp_debug_printf("\n\r BOARD_72FXS\n");
        break;
    case VG400_2FXS_2FXO:
        bsp_debug_printf("\n\r VG400_2FXS_2FXO\n");
        break;
    case VG400_4FXS_4FXO:
        bsp_debug_printf("\n\r VG400_4FXS_4FXO\n");
        break;
    case VG400_6FXS_6FXO:
        bsp_debug_printf("\n\r VG400_6FXS_6FXO\n");
        break;
    case VG400_8FXS:
        bsp_debug_printf("\n\r VG400_8FXS\n");
        break;
    case PHOENIX_144FXS:
        bsp_debug_printf("\n\r PHOENIX_144FXS\n");
        break;
    case PHOENIX_132FXS_6FXO:
        bsp_debug_printf("\n\r PHOENIX_132FXS_6FXO\n");
        break;
    case PHOENIX_84FXS_6FXO:
        bsp_debug_printf("\n\r PHOENIX_84FXS_6FXO\n");
        break;
    default:
        cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
        return;
        break;
    }
    if(is_phoenix()) {
        while (1) {
            if (phoenix_fpga_con_memory[ix].offset == 0xFFFF) {
                bsp_debug_printf("\r\n Connection Memory Setup Done!\r\n");
                break; 
            }
            fpga_spi_indirect_write(phoenix_fpga_con_memory[ix].offset, 4, 
                    phoenix_fpga_con_memory[ix].cid);
            ix++;
        }
    } else {
        while (1) {
            if (fpga_con_memory[ix].offset == 0xFFFF) {
                bsp_debug_printf("\r\n Connection Memory Setup Done!\r\n");
                break; 
            }
            fpga_spi_indirect_write(fpga_con_memory[ix].offset, 4, fpga_con_memory[ix].cid);
            ix++;
        }
    }

}


/*******************************************************************
 *
 * Function: oak_diag_codec_reset
 *
 * Description: This function will reset or unreset the Codec chip(s)
 *              of the Oakenshield card.
 *
 * Input  : reset if TRUE, unreset if FALSE
 *
 * Output : None
 *
 *******************************************************************
 */
void oak_diag_codec_reset(int reset)
{
    uchar board_id = get_oak_id();

    /*
    BOARD_16FXS_2FXO              0x00
        MB_TDM0 MB_TDM1 DC_TDM2
    BOARD_24FXS_4FXO              0x01
        MB_TDM0 MB_TDM1 DC_TDM2
    BOARD_8FXS_12FXO              0x10
        MB_TDM0 MB_TDM1 DC_TDM3
    VG400_2FXS_2FXO               0x100
        MB_TDM0 MB_TDM1 DC_TDM2
    VG400_4FXS_4FXO               0x101
        MB_TDM0 MB_TDM1 DC_TDM2
    VG400_6FXS_6FXO               0x110
        MB_TDM0 MB_TDM1 DC_TDM2
    PHOENIX_144FXS                0x8
        MB_TDM0~1 DB1_TDM2~3 DB2_TDM4~6 DB3_TDM7~9
    PHOENIX_132FXS_6FXO           0x9
        MB_TDM0~1 DB1_TDM2~3 DB2_TDM4~6 DB3_TDM7~9
    PHOENIX_84FXS_6FXO            0xA 
        MB_TDM0~1 DB1_TDM2~3 DB2_TDM4~6
    */

    switch(board_id) {
    case BOARD_16FXS_2FXO: 
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    case BOARD_24FXS_4FXO:
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    case BOARD_8FXS_12FXO:
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    case BOARD_72FXS:
        si32261_reset(reset);
        break;
    case VG400_2FXS_2FXO:
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    case VG400_4FXS_4FXO:
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    case VG400_6FXS_6FXO:
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    case VG400_8FXS:
        si32261_reset(reset);
        break;
    case PHOENIX_144FXS:
        si32261_reset(reset);
        break;
    case PHOENIX_132FXS_6FXO:
    case PHOENIX_84FXS_6FXO:
        si32261_reset(reset);
        si3050_reset(reset);
        break;
    default:
        cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
        return;
        break;
    }

}



/**********************************************************************
 *
 * Function: test_tdmsw_reset
 *
 * This function will setup reset tdmsw
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void test_tdmsw_reset (void)
{
    bsp_debug_printf("\n\r Resetting TDMSW \n");
    oak_module_tdm_init();
    bsp_debug_printf("\n\r TDMSW reset and re-initialization completed !!!\n");
}



/**********************************************************************
 *
 * Function: fpga_setup
 *
 * This function will setup FPGA
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void fpga_setup (void)
{

    uchar board_id = get_oak_id();

    /* reset tdm */
    test_tdmsw_reset();

    /* connect TDM switch */
    oak_tdm_xc_setup(board_id, TRUE);

}

/**********************************************************************
 *
 * Function: set_fail_over_port
 *
 * This function will set fail over port
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
int set_fail_over_port (void)
{
    uchar board_id;
    uint32_t fail_over_port_mask;
    uint16_t fpga_addr = 0x9320;

    board_id = get_oak_id();

    switch(board_id) {
    case BOARD_16FXS_2FXO:
        fail_over_port_mask = 0x3;
        break;
    case BOARD_24FXS_4FXO:
        fail_over_port_mask = 0xf;
        break;
    case BOARD_8FXS_12FXO:
        fail_over_port_mask = 0xff;
        break;
    case VG400_2FXS_2FXO:
        fail_over_port_mask = 0x30;
        break;
    case VG400_4FXS_4FXO:
        fail_over_port_mask = 0xf;
        break;
    case VG400_6FXS_6FXO:
        fail_over_port_mask = 0x3f;
        break;
    case PHOENIX_132FXS_6FXO:
    case PHOENIX_84FXS_6FXO:
        fail_over_port_mask = 0x3f;
        break;
    default:
        cterr('f', 0, "%s Unknown board. ID = %#x", __FUNCTION__, board_id);
        return (FAILED);
        break;
    }

    if (hd_if->param1 == TRUE) {
        bsp_debug_printf("\n\r Enable \n");
        fpga_spi_direct_write(fpga_addr, 1, 0);    
    } else {
        bsp_debug_printf("\n\r Disable \n");
        fpga_spi_direct_write(fpga_addr, 1, fail_over_port_mask);    
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: is_vg400
 *
 * This function return vg400 board type
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
int is_vg400(void) {
    return ((get_oak_id() == VG400_2FXS_2FXO) ||
            (get_oak_id() == VG400_4FXS_4FXO) || 
            (get_oak_id() == VG400_6FXS_6FXO) ||
            (get_oak_id() == VG400_8FXS));
}


/**********************************************************************
 *
 * Function: is_phoenix
 *
 * This function use to check is phoenix project or not
 *
 * Input : None
 *
 * Output: TRUE = is Phoenix project
 *         FALSE = not Phoenix project
 *
 **********************************************************************
 */
int is_phoenix(void) {
    return ((get_oak_id() == PHOENIX_144FXS) ||
            (get_oak_id() == PHOENIX_132FXS_6FXO) || 
            (get_oak_id() == PHOENIX_84FXS_6FXO));
}

/**********************************************************************
 *
 * Function: phoenix_has_dbx
 *
 * This function return phoenix db1 present or not
 *
 * Input : dbx = want check daughter board
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
int phoenix_has_dbx (int dbx) {
    uint32_t reg_data;

    switch (dbx) {
    case BOARD_DB1_TEST:
        /* Only care present pin when user want test with DB1 */
        if (phoenix_not_test_db1_flag == FALSE) {
            fpga_spi_direct_read(FPGA_GENERAL_BOARD_ID, 1, &reg_data);
            return(!(reg_data & BOARD_DB1_MASK));
        }
        break;
    case BOARD_DB2_TEST:
        /* Only care present pin when user want test with DB2 */
        if (phoenix_not_test_db2_flag == FALSE) {
            fpga_spi_direct_read(FPGA_GENERAL_BOARD_ID, 1, &reg_data);
            return(!(reg_data & BOARD_DB2_MASK));
        }
        break;
    case BOARD_DB3_TEST:
        /* Only care present pin when user want test with DB3 */
        if (phoenix_not_test_db3_flag == FALSE) {
            fpga_spi_direct_read(FPGA_GENERAL_BOARD_ID, 1, &reg_data);
            return(!(reg_data & BOARD_DB3_MASK));
        }
        break;
    default:
        break;
    }
    return(FALSE);
}

/**********************************************************************
 *
 * Function: phoenix_db1_only_fxs
 *
 * This function return phoenix db1 only has fxs (no fxo)
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
int phoenix_db1_only_fxs (void) {
    if (!is_phoenix()) {
        return (FALSE);
    } else {
        return ((get_oak_id() == PHOENIX_144FXS) ? TRUE : FALSE);
    }
}

/**********************************************************************
 *
 * Function: set_host_hw_brd_type_flag
 *
 * This function will setup Host hardware board type flag
 *
 * Input : Setup flag value
 *
 * Output: None
 *
 **********************************************************************
 */
void set_host_hw_brd_type_flag (uint32_t flag) {

    phoenix_hw_brd_type_flag = PHOENIX_HW_BRD_TYPE(flag);
    switch (flag) {
        case PHOENIX_144FXS_HW_BRD_TYPE:
            bsp_debug_printf("\n\rSet phoenix_hw_brd_type_flag with 144FXS SKU\n");
            break;
        case PHOENIX_132FXS_6FXO_HW_BRD_TYPE:
            bsp_debug_printf("\n\rSet phoenix_hw_brd_type_flag with 132FXS+6FXO SKU\n");
            break;
        case PHOENIX_84FXS_6FXO_HW_BRD_TYPE:
            bsp_debug_printf("\n\rSet phoenix_hw_brd_type_flag with 84FXS+6FXO SKU\n");
            break;
        default:
            bsp_debug_printf("\n\rInvalide hardware board type set phoenix_hw_brd_type_flag 0\n");
            phoenix_hw_brd_type_flag = 0;
    }
}

/**********************************************************************
 *
 * Function: toggle_sep_test_dbx_flag
 *
 * This function will setup MB or DBx to separate testing by flag
 *
 * Input : Setup value
 *
 * Output: None
 *
 **********************************************************************
 */
void toggle_sep_test_dbx_flag (uint32_t value) {
    
    /* Check to test with MB or not*/
    if (value & PHOENIX_ONLY_TEST_MB_MASK) {
        phoenix_only_test_dbx_flag = FALSE;
        bsp_debug_printf("\n\rphoenix_only_test_dbx_flag = FALSE\n");
    } else {
        phoenix_only_test_dbx_flag = TRUE;
        bsp_debug_printf("\n\rphoenix_only_test_dbx_flag = TRUE\n");
    }
    
    /* Check to test with DB1 or not*/
    if (value & PHOENIX_ONLY_TEST_DB1_MASK) {
        phoenix_not_test_db1_flag = FALSE;
        bsp_debug_printf("\n\rphoenix_not_test_db1_flag = FALSE\n");
    } else {
        phoenix_not_test_db1_flag = TRUE;
        bsp_debug_printf("\n\rphoenix_not_test_db1_flag = TRUE\n");
    }

    /* Check to test with DB2 or not*/
    if (value & PHOENIX_ONLY_TEST_DB2_MASK) {
        phoenix_not_test_db2_flag = FALSE;
        bsp_debug_printf("\n\rphoenix_not_test_db2_flag = FALSE\n");
    } else {
        phoenix_not_test_db2_flag = TRUE;
        bsp_debug_printf("\n\rphoenix_not_test_db2_flag = TRUE\n");
    }

    /* Check to test with DB3 or not*/
    if (value & PHOENIX_ONLY_TEST_DB3_MASK) {
        phoenix_not_test_db3_flag = FALSE;
        bsp_debug_printf("\n\rphoenix_not_test_db3_flag = FALSE\n");
    } else {
        phoenix_not_test_db3_flag = TRUE;
        bsp_debug_printf("\n\rphoenix_not_test_db3_flag = TRUE\n");
    }

}
/**********************************************************************
 *
 * Function: phoenix_fpga_dsp_spi_controlsw
 *
 * This function will switch Phoenix FPGA SPI control bus between DSP0 and DSP1
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
void phoenix_fpga_dsp_spi_controlsw (void)
{
    uint32_t reg_data, ctrl_id;

    fpga_spi_direct_read(PHOENIX_FPGA_DSP_SPI_CTRL, 4, &reg_data);
    ctrl_id = reg_data >> PHOENIX_FPGA_DSP_SPI_ID_SHIFT;

    /* Only Phoenix DSP1 need switch DSP control path */
    if (ctrl_id == PHOENIX_FPGA_DSP1_SPI_CTRL_ID) {
        /* Enable switch SPI control path and set indicator to DSP1 */
        reg_data = reg_data | (PHOENIX_FPGA_DSP_SPI_CTRL_INDICATOR_MASK) 
                   | PHOENIX_FPGA_DSP_SPI_CTRL_KEY;
        /* Write twice by TDM FPGA engineer suggest */
        bsp_debug_printf("Phoenix switch FPGA SPI Control bus.\n");
        fpga_spi_direct_write(PHOENIX_FPGA_DSP_SPI_CTRL, 4, reg_data);
        fpga_spi_direct_write(PHOENIX_FPGA_DSP_SPI_CTRL, 4, reg_data);
    }

}


static codec_rst_port_tbl_t codec_rst_port_table[] = {
    {
        PHOENIX_144FXS,              /* Board ID */
        {
            FXS_PORT0, FXS_PORT23,   /* MB */
            FXS_PORT24, FXS_PORT47,  /* DB1 */
            FXS_PORT48, FXS_PORT95,  /* DB2 */
            FXS_PORT96, FXS_PORT143  /* DB3 */
        },
        {
            -1, -1,                  /* MB */
            -1, -1,                  /* DB1 */
            -1, -1,                  /* DB2 */
            -1, -1,                  /* DB3 */
        }
    },

    {
        PHOENIX_132FXS_6FXO,
        {
            FXS_PORT0, FXS_PORT23,
            FXS_PORT24, FXS_PORT35,
            FXS_PORT36, FXS_PORT83,
            FXS_PORT84, FXS_PORT131
        },
        {
            -1, -1,
            FXO_PORT0, FXO_PORT5,
            -1, -1,
            -1, -1,
        }
    },

    {
        PHOENIX_84FXS_6FXO,
        {
            FXS_PORT0, FXS_PORT23,
            FXS_PORT24, FXS_PORT35,
            FXS_PORT36, FXS_PORT83,
            -1, -1
        },
        {
            -1, -1,
            FXO_PORT0, FXO_PORT5,
            -1, -1,
            -1, -1,
        }
    },

    { BOARD_RESERVED,
      { -1, -1, -1, -1, -1, -1, -1, -1},
      { -1, -1, -1, -1, -1, -1, -1, -1}
    }
};

/**********************************************************************
 *
 * Function: get_codec_rst_test_table
 *
 * This function will return codec reset port table.
 *
 * Input : board_id
 *         -- PHOENIX_144FXS
 *         -- PHOENIX_132FXS_6FXO
 *         -- PHOENIX_84FXS_6FXO
 *
 * Output: None
 *
 **********************************************************************
 */
static codec_rst_port_tbl_t* get_codec_rst_test_table(int board_id)
{
    int ix = 0;

    for (ix = 0; ix <= BOARD_RESERVED; ix++) {
        if (board_id == codec_rst_port_table[ix].board) {
            return (&codec_rst_port_table[ix]);
        }
        if (BOARD_RESERVED == codec_rst_port_table[ix].board) {
            return (NULL);
        }
    }

    return (NULL);
}

/**********************************************************************
 *
 * Function: codec_rev_test
 *
 * This function will test FXS and FXO Rev function.
 *
 * Input : tbl -- codec_rst_port_tbl_t
 *         mode -- DIAG_CODEC_NORMAL_MODE
 *                 DIAG_CODEC_RESET_MODE
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int codec_rev_test(codec_rst_port_tbl_t* tbl, int mode)
{
    int brd_ix = 0;
    int fxs_s, fxs_e, fxo_s, fxo_e;
    int port, fxo_rev;
    int ret = PASSED, ret_code;
    uint rev_code;
    char errstr[128];

    for (brd_ix = 0; brd_ix < BRD_NUM_MAX; brd_ix++) {
        fxs_s = -1;
        fxo_s = -1;
        switch (brd_ix) {
            case BOARD_MB_TEST: /* MB */
                if (phoenix_only_test_dbx_flag != TRUE) {
                    fxs_s = tbl->fxs_test_port[MB_INDEX];
                    fxs_e = tbl->fxs_test_port[MB_INDEX+1];
                    fxo_s = tbl->fxo_test_port[MB_INDEX];
                    fxo_e = tbl->fxo_test_port[MB_INDEX+1];
                }
                break;

            case BOARD_DB1_TEST: /* DB1 */
                if (phoenix_has_dbx(BOARD_DB1_TEST)) {
                    fxs_s = tbl->fxs_test_port[DB1_INDEX];
                    fxs_e = tbl->fxs_test_port[DB1_INDEX+1];
                    fxo_s = tbl->fxo_test_port[DB1_INDEX];
                    fxo_e = tbl->fxo_test_port[DB1_INDEX+1];
                }
                break;

            case BOARD_DB2_TEST: /* DB2 */
                if (phoenix_has_dbx(BOARD_DB2_TEST)) {
                    fxs_s = tbl->fxs_test_port[DB2_INDEX];
                    fxs_e = tbl->fxs_test_port[DB2_INDEX+1];
                    fxo_s = tbl->fxo_test_port[DB2_INDEX];
                    fxo_e = tbl->fxo_test_port[DB2_INDEX+1];
                }
                break;

            case 3: /* DB3 */
                if (phoenix_has_dbx(BOARD_DB3_TEST)) {
                    fxs_s = tbl->fxs_test_port[DB3_INDEX];
                    fxs_e = tbl->fxs_test_port[DB3_INDEX+1];
                    fxo_s = tbl->fxo_test_port[DB3_INDEX];
                    fxo_e = tbl->fxo_test_port[DB3_INDEX+1];
                }
                break;

            default:
                bsp_debug_printf("\n%s: Invalid board = %d ...\n",
                                 __func__, brd_ix);
                sprintf(errstr, "\n%s: Invalid board = %d ...\n",
                        __func__, brd_ix);
                strcat((char *)&(hd_if->errmsg), errstr);
                return (FAILED);
        }

        /* FXS Rev testing */
        if (fxs_s >= 0 && fxs_e >= fxs_s) {
            for (port = fxs_s; port < fxs_e; port += FXS_PORT_NUM_PER_CODEC) {
                /* chip ID register address is 0*/
                ret_code = si_read_reg_32261(port, 0, &rev_code);
                if (DIAG_CODEC_RESET_MODE == mode) {
                    ret_code = !ret_code;
                }

                if (ret_code != PASSED) {
                    cterr('f', 0, "port %d SI32261 Read Codec Rev failed.",
                          port);
                    sprintf(errstr, "\nport %d SI32261 Read Codec Rev failed.\n", port);
                    strcat((char *)&(hd_if->errmsg), errstr);
                    ret = FAILED;
                }
            }
        }

        /* FXO Rev tesing */
        if (fxo_s >= 0 && fxo_e >= fxo_s) {
            for (port = fxo_s; port < fxo_e; port += FXO_PORT_NUM_PER_CODEC) {
                ret_code = si3050_reg_read(port, SYSTEM_SIDE_CHIP_REV_REG, &fxo_rev);
                if (DIAG_CODEC_RESET_MODE == mode) {
                    ret_code = !ret_code;
                }

                if (ret_code != PASSED) {
                    cterr('f', 0, "port %d SI3050 Read Codec Rev failed.",
                          port);
                    sprintf(errstr, "\nport %d SI3050 Read Codec Rev failed.\n", port);
                    strcat((char *)&(hd_if->errmsg), errstr);
                    ret = FAILED;
                }
            }
        }
    }

    return (ret);
}

/**********************************************************************
 *
 * Function: tdm_codec_reset_test
 *
 * This function will test TDM FPGA reset function.
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
int tdm_codec_reset_test(void)
{
    int board_id;
    char errstr[128];
    codec_rst_port_tbl_t *tbl = NULL;

    /* Check board ID */
    board_id = get_oak_id();

    tbl = get_codec_rst_test_table(board_id);
    if (tbl == NULL) {
        cterr('f', 0, "%s Unknown board. ID = %#x", __func__, board_id);
        sprintf(errstr, "\n%s: Unknown board. ID = %#x\n", __func__, board_id);
        strcat((char *)&(hd_if->errmsg), errstr);
        return (FAILED);
    }

    /* 1. Make sure FXS and FXO Rev test is passed. */
    reset_fxs_codec();
    reset_si3050();
    if (FAILED == codec_rev_test(tbl, DIAG_CODEC_NORMAL_MODE)) {
        return (FAILED);
    }

    /* 2. Set to reset mode and do Codec Rev test again.
     *    Codec Rev test should be failed in reset mode.
     */
    si32261_reset(1);
    si3050_reset(1);
    if (FAILED == codec_rev_test(tbl, DIAG_CODEC_RESET_MODE)) {
        return (FAILED);
    }

    /* 3. Set to normal mode.
     *    Codec Rev test should be passed in normal mode.
     */
    si32261_reset(0);
    si3050_reset(0);
    if (FAILED == codec_rev_test(tbl, DIAG_CODEC_NORMAL_MODE)) {
        return (FAILED);
    }

    PRINT_STR("\r\r");
    PRINT_STR("TDM Codec Reset test is PASSED.");
    PRINT_STR("\r\r");

    return (PASSED);
}

/**********************************************************************
 *
 *  Function: setup_hw_brd_type_flag
 *
 *  Description: This function will force set hardware board type flag
 *               at DSP side (phoenix_hw_brd_type_flag)
 *
 *  Input: none
 *
 *  Returns: none
 *
 **********************************************************************
 */
void setup_hw_brd_type_flag (void)
{
    uint32_t flag;

    bsp_debug_printf("Force setup HW board type flag for SKU check \n"
           "\n\rExample: "
           "\n\r0x7, HW board type strapping to PHOENIX_144FXS,");
    bsp_debug_printf("\n\r0x6, HW board type strapping to PHOENIX_132FXS_6FXO, "
           "\n\r0x5, HW board type strapping to PHOENIX_84FXS_6FXO, "
           "\n\r0x0, NO HW board type setting, check SKU by TDM FPGA.\n");

    flag = gethex_answer("\nEnter hardware board type flag[0x0 to 0x7]:", 0x0, 0x0, 0x7);

    set_host_hw_brd_type_flag(flag);
}

/**********************************************************************
 *
 *  Function: setup_sep_test_dbx_flag
 *
 *  Description: This function will set phoenix_sep_test_dbx_flag
 *               The flag use to separate MB or DBx testing.
 *
 *  Input: none
 *
 *  Returns: none
 *
 **********************************************************************
 */
void setup_sep_test_dbx_flag (void)
{
    uint32_t flag;

    bsp_debug_printf("Set flag to run separate MB or DBx testing \n"
           "\n\rExample: "
           "\n\r0xF, binary 1111 run MB, DB1, DB2 and DB3 ");
    bsp_debug_printf("\n\r0x8, binary 1000 run MB only, "
           "\n\r0x4, binary 0100 run DB1 only, "
           "\n\r0x2, binary 0010 run DB2 only, ");
    bsp_debug_printf("\n\r0x1, binary 0001 run DB3 only, "
           "\n\r0xC, binary 1100 run MB, DB1 only, "
           "\n\r0x7, binary 0111 run DB1, DB2, DB3 only.\n");

    flag = gethex_answer("\nEnter separate test MB or DBx flag[0x1 to 0xF]:", 0xf, 0x1, 0xf);

    toggle_sep_test_dbx_flag(flag);
}

/*****************************************************************
 *
 * Function: fpga_upgrade_cpld
 *
 * Description: This function upgrades cpld firmware
 *
 * Input: None
 *
 * Output: PASSED
 *
 *****************************************************************/
int fpga_upgrade_cpld (void)
{
    char *cpld_image;

    /* Download image from host to memory */
    if (fpga_image_download(PHOENIX_CPLD_UPGRADE_SIZE) == FAILED) {
        return (FAILED);
    }

    cpld_image = (char *)fpga_image;

    jam_upgrade_cpld(cpld_image);
    return 0;
}

/******** History ********
$Log: diag_fpga.c,v $
Revision 1.4  2021/04/15 00:53:07  achiu2
[PRRQ:CSCvx56970-2]Phoenix code review for ER

Revision 1.3  2018/08/30 06:39:42  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.2.28.1  2018/01/26 09:42:04  haohsu
*** empty log message ***

Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:38  harrchan
Initial commit code for Oakenshield

Revision 1.1.2.7  2017/03/30 10:25:50  harrchan
Add fpga upgrade utility

Revision 1.1.2.6  2017/03/09 07:23:34  harrchan
Support oakenshield double wide case

Revision 1.1.2.5  2017/02/09 06:41:05  olin2
Support voltage margin and fail over port utility

Revision 1.1.2.4  2017/01/17 05:07:06  olin2
Clean up debug code

Revision 1.1.2.3  2017/01/05 06:06:33  olin2
Support FXS Ring and Calibration

Revision 1.1.2.2  2016/12/23 06:56:04  olin2
Support FXS/FXO loopback test

Revision 1.1.2.1  2016/12/14 04:57:38  olin2
Initial commit code for Oakenshield



$Endlog$
*/

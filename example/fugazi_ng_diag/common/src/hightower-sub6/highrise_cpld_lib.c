/* $Id: highrise_cpld_lib.c,v 1.3 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/highrise_cpld_lib.c,v $
 *******************************************************************************
 * File Name: highrise_cpld_lib.c
 *
 * Description: Highrise CPLD library source file
 *
 * Author: Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "cookie_4.h"
#include "proto.h"
#include "string.h"
#include "sm_slot.h"
#include "menu.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cross_platform.h"
#include "ngio.h"
#include "plat_defs.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "highrise_cpld_lib.h"
#include "highrise_cpld_diag.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/
#define CPLD_RONLY    (READ_ONLY | REG_ACCESS)
#define CPLD_RW       (READ_WRITE | REG_ACCESS)

#define BYTE_SWAP32(x)  ((((x)>>24)&0xFF)|(((x)&0xFF)<<24)|(((x)&0xFF00)<<8)|(((x)>>8)&0xFF00))

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static int hr_cpld_read_fn(unsigned long, int, unsigned long *, void *);
static int hr_cpld_write_fn(unsigned long, int, unsigned long, void *);
static void cpld_reset_2_run(void);
static unsigned int cpld_shift(unsigned int, unsigned char, unsigned char,
                               boolean);
#ifdef ORIGINAL_SPEEDUP_CPLD_PROGRAM
static void cpld_reset_jtag(void);
static void cpld_run_2_ir(void);
static void cpld_ir_2_run(void);
static void cpld_dr_2_run(void);
static void cpld_ir_2_dr(void);
static void cpld_run_2_dr(void);
#endif
static void cpld_run_test_delay(int);
static long speed_up_cpld_jtag_io(uchar, uchar, boolean);
static void speedup_cpld_reset_jtag(void);
static void speedup_cpld_run_2_ir(void);
static void speedup_cpld_ir_2_run(void);
static void speedup_cpld_dr_2_run(void);
static void speedup_cpld_ir_2_dr(void);
static void speedup_cpld_run_2_dr(void);
static void erase_all(int);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long hr_cpld_reg_test_lib(void);
long util_oir_cpld_reg_read(void);
long util_oir_cpld_reg_write(void);
long hr_cpld_reg_read_lib(uchar, uchar *);
long hr_cpld_reg_write_lib(uchar, uchar);
long hr_cpld_jtag_ctrl(boolean);
long display_embedded_cpld_fw_version(uchar *);
long cpld_jtag_io(int, int, int);
long max2_cpld_program(void);
long hr_simply_program_cpld(unsigned char *);

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern unsigned char cpld_fw_array[];
extern unsigned long cpld_fw_size;
extern int upgrade_interface;
extern int getdec_answer(char *msgstr, uint, uint, uint);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

#define I2C_MUX_ZERO        0
#define I2C_CTRL_FIFTEEN    15
static n2g_i2c_if_t cpld =
{
    .dev_name = "Highrise CPLD",
    .offset = 0,
    .i2c_bus_type = 0, 
    .size = 0x4,
    .mux = I2C_MUX_ZERO,
    .buf = NULL,
    /* Dash FPGA I2C Controller 15 is used for NGVM, refer to O2 HFS. */
    .i2c_ctrl = 0,
    /* CPLD I2C 7-bits address */
    .i2c_dev = HIGHRISE_CPLD_I2C_ADDR,
};

static int hr_cpld_reg_tst_show = 0;
static int hr_cpld_reg_tst_rd(unsigned long addr, int size, unsigned long *buf, void *param)
{
    int ret = hr_cpld_read_fn(addr, size, buf, param);
    if (hr_cpld_reg_tst_show)
        printf("CPLD Rd 0x%02lX : 0x%08lX\n", addr, *buf);
    return ret;
}

static int hr_cpld_reg_tst_wr(unsigned long addr, int size, unsigned long data, void *param)
{
    if (hr_cpld_reg_tst_show)
        printf("CPLD Wr 0x%02lX : 0x%08lX\n", addr, data);
    return hr_cpld_write_fn(addr, size, data, param);
}

static reg_info_t_ext cpld_reg_ext = {4, hr_cpld_reg_tst_rd,
                                      hr_cpld_reg_tst_wr, &cpld};

#define HR_CPLD_REG(REG,TYPE,MASK,DFLT) {#REG, HR_CPLD_##REG, CPLD_##TYPE | SAVE_RESTORE, {.ext=&cpld_reg_ext}, MASK, DFLT}
static reg_info_t cpld_test_regs[] = {
    HR_CPLD_REG(VERSION,            RONLY,  0x0000FFFF, 0x00000000),
    HR_CPLD_REG(VERSION_DATE,       RONLY,  0x0000FFFF, 0x00000000),
    HR_CPLD_REG(BOARD_ID,           RONLY,  0x0000000F, 0x00000000),
    HR_CPLD_REG(PWR_STATUS,         RONLY,  0x000003FF, 0x00000000),
    HR_CPLD_REG(MODEM_STATUS,       RONLY,  0x00000007, 0x00000000),
    HR_CPLD_REG(ERROR_TEST,         RW,     0x000003FF, 0x00000000),
    HR_CPLD_REG(RESET_CTRL,         RONLY,  0x000000FF, 0x00000000),
    HR_CPLD_REG(RESET_PROTECT,      RW,     0x00000039, 0x00000000),
    HR_CPLD_REG(CPU_RESET_TRIGGER,  RONLY,  0xFFFFFFFF, 0x00000000), 
    HR_CPLD_REG(CPU_BOOT_STATUS,    RONLY,  0x0000000F, 0x00000000),
    HR_CPLD_REG(MODEM_CTRL,         RONLY,  0x00000003, 0x00000000),
    HR_CPLD_REG(LED_CTRL,           RW,     0x00003FFD, 0x00000000),
    HR_CPLD_REG(INT_ENABLE,         RW,     0x00000006, 0x00000000),
    HR_CPLD_REG(INT_STATUS,         RONLY,  0x00000007, 0x00000000),
    HR_CPLD_REG(SCRATCHPAD,         RW,     0xFFFFFFFF, 0x00000000),
    HR_CPLD_REG(PHY_STATUS,         RONLY,  0x000003FF, 0x00000000),
    HR_CPLD_REG(MAC0_ADDR,          RW,     0xFFFF0000, 0x00000000),
    HR_CPLD_REG(MAC1_ADDR,          RW,     0xFFFFFFFF, 0x00000000),
    HR_CPLD_REG(PWR_CYCLE,          RONLY,  0xFFFFFFFF, 0x00000022),
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},
};

static int count = 0;

/**********************************************************************
 *
 * Function: util_oir_cpld_reg_read
 *
 * Wrapper for CPLD Register Display utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_cpld_reg_read (void)
{
    hr_cpld_reg_tst_show = 0;
    if (register_display(0, &cpld_test_regs[0]) == FAILED) {
        cterr('f', 0, "CPLD Register Display Failed");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: util_oir_cpld_reg_write
 *
 * Wrapper for CPLD Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long util_oir_cpld_reg_write (void)
{
    uint32_t value;
    uint32_t addr;

    printf("\n\nCPLD Register Write\n\n");

    addr = gethex_answer("Reg offset to write", 0, 0, 0xff);
    
    value = gethex_answer("Enter new reg value", 0x0, 0, 0xffffffff);
    
    return hr_cpld_reg_write_32(addr, value);
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_read_32
 *
 * This function provides the library for cpld register reads.
 *
 * Input : offset - register offset
 *         *buffer - pointer of the buffer address
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reg_read_32 (unsigned long offset, unsigned long *data)
{
    int rc;

    rc = hr_cpld_read_fn((uchar)offset, 4, data, &cpld);

    return (rc);
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_write_32
 *
 * This function provides the library for cpld register writes.
 *
 * Input : offset - register offset
 *         buffer - write buffer value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reg_write_32 (unsigned long offset, unsigned long data)
{
    return hr_cpld_write_fn ((uchar)offset, 4, data, &cpld);
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_read_lib
 *
 * This function provides the library for cpld register reads.
 *
 * Input : offset - register offset
 *         *buffer - pointer of the buffer address
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reg_read_lib (uchar offset, uchar *buffer)
{
    unsigned long data;

    int rc;

    rc = hr_cpld_read_fn(offset, 1, &data, &cpld);
    if (PASSED == rc)
        *buffer = (uchar)data;

    return (rc);
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_write_lib
 *
 * This function provides the library for cpld register writes.
 *
 * Input : offset - register offset
 *         buffer - write buffer value
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reg_write_lib (uchar offset, uchar buffer)
{
    unsigned long data = buffer;

    return hr_cpld_write_fn (offset, 1, data, &cpld);
}

/**********************************************************************
 *
 * Function: hr_cpld_reg_test_lib
 *
 * This function perform the CPLD register test.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_reg_test_lib (void)
{
    printf("\n");
    hr_cpld_reg_tst_show = 0;
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        hr_cpld_reg_tst_show = 1;
    } 
    if (register_tests(0, &cpld_test_regs[0]) == FAILED) {
        cterr('f', 0, "CPLD Register Test Failed");
        return (FAILED);
    }
    printf("CPLD Register Test Passed\n");

    return (PASSED);
}

/**********************************************************************
 *
 * Function: hr_cpld_jtag_ctrl
 *
 * Function to turn on jtag
 *
 * Input : jtag_on_off - turn on/off the jtag on
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_cpld_jtag_ctrl (boolean jtag_on_off)
{
    uchar cpld_buf;

    if (upgrade_interface == UPGRADE_FROM_CPLD) {
        /* JTAG Control register JTAG_ON bit as 1 */
        if (hr_cpld_reg_read_lib(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
            return (FAILED);
        }

        if (jtag_on_off == TRUE) {
            /* Make JTAG_ON bit as 1 */
            cpld_buf |= (CPLD_JTAG_ON);
        } else {
            /* Make JTAG_ON bit as 0 */
            cpld_buf &= ~(CPLD_JTAG_ON);
        }

        /* Make JTAG_TCK bit as 0 */
        cpld_buf &= ~(CPLD_JTAG_TCK_ST);

        /* Alter the register */
        if (hr_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : tms - jtag tms control
 *         tdi - jtag tdi control
 *         read_tdo - jtag read_tdo control
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long cpld_jtag_io (int tms, int tdi, int read_tdo)
{
    int tdo = 0;
    uchar cpld_buf;
    uchar io_expander_buf;

    /* Print out the status. */
    count++;
    if ((count % 10) == 0) {
        printf(".");
        count = 0;
    }

    if (upgrade_interface == UPGRADE_FROM_CPLD) {
        /* Upgrade the firmware from CPLD
         * Write data (TMS, TDI, TCK written low)
         * Read TDO
         * Write data (TCK written high)
         * Write data (TCK written low)
         */
        cpld_buf = CPLD_JTAG_ON;

        /* Prepare: Write data (TMS) */
        if (tms) {
            cpld_buf |= CPLD_JTAG_TMS_ST;
        } else {
            cpld_buf &= ~CPLD_JTAG_TMS_ST;
        }

        /* Prepare: Write data (TDI) */
        if (tdi) {
            cpld_buf |= CPLD_TDI_ST;
        } else {
            cpld_buf &= ~CPLD_TDI_ST;
        }

        /* Prepare: TCK written low */
        cpld_buf &= ~CPLD_JTAG_TCK_ST;

        /* step 1: Write data (TMS, TDI, TCK written low) */
        if (hr_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }

        /* step 2: Read TDO */
        if (read_tdo) {
            if (hr_cpld_reg_read_lib(CPLD_JTAG_CTL, &cpld_buf) == FAILED) {
                return (FAILED);
            }

            tdo = (cpld_buf & CPLD_TDO_ST) ? 1 : 0;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%d ", tdo);
            }
        }

        /* Step 3: Write data (TCK written high) */
        cpld_buf |= CPLD_JTAG_TCK_ST;
        if (hr_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }

        /* Step 4: Write data (TCK written low) */
        cpld_buf &= ~CPLD_JTAG_TCK_ST;
        if (hr_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
            return (FAILED);
        }
    } else {
        /* Upgrade the firmware from IO Expander interface.
         * Write data (TMS, TDI, TCK written low)
         * Read TDO
         * Write data (TCK written high)
         * Write data (TCK written low)
         */
        io_expander_buf = 0;

        /* Prepare: Write data (TMS) */
        if (tms) {
            io_expander_buf |= PCA9557_TMS_GPIO7_OUTPUT;
        } else {
            io_expander_buf &= ~PCA9557_TMS_GPIO7_OUTPUT;
        }

        /* Prepare: Write data (TDI) */
        if (tdi) {
            io_expander_buf |= PCA9557_TDI_GPIO4_OUTPUT;
        } else {
            io_expander_buf &= ~PCA9557_TDI_GPIO4_OUTPUT;
        }

        /* Prepare: TCK written low */
        io_expander_buf &= ~PCA9557_TCK_GPIO6_OUTPUT;

        /* step 1: Write data (TMS, TDI, TCK written low) */
        if (hr_cpld_reg_write_lib(PCA9557_NGIO_EXPANDER_OUTPUT, io_expander_buf)
            == FAILED) {
            return (FAILED);
        }

        /* step 2: Read TDO */
        if (read_tdo) {
            if (hr_cpld_reg_read_lib(PCA9557_NGIO_EXPANDER_INPUT, &io_expander_buf)
                == FAILED) {
                return (FAILED);
            }

            tdo = (io_expander_buf & PCA9557_TDO_GPIO5_INPUT) ? 1 : 0;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%d ", tdo);
            }
        }

        /* Step 3: Write data (TCK written high) */
        io_expander_buf |= PCA9557_TCK_GPIO6_OUTPUT;
        if (hr_cpld_reg_write_lib(PCA9557_NGIO_EXPANDER_OUTPUT, io_expander_buf)
            == FAILED) {
            return (FAILED);
        }

        /* Step 4: Write data (TCK written low) */
        io_expander_buf &= ~PCA9557_TCK_GPIO6_OUTPUT;
        if (hr_cpld_reg_write_lib(PCA9557_NGIO_EXPANDER_OUTPUT, io_expander_buf)
            == FAILED) {
            return (FAILED);
        }
    }

    return (tdo);
}

/**********************************************************************
 *
 * Function: speed_up_cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : reg_offset - cpld register offset
 *         cpld_w_buf - cpld write data value
 *         tdo_read - cpld read data value flag
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long speed_up_cpld_jtag_io (uchar reg_offset, uchar cpld_w_buf,
                                   boolean tdo_read)
{
    uchar cpld_r_buf = 0, rd_data;

    if (tdo_read == FALSE) {
        /* Write data value */
        if (hr_cpld_reg_write_lib(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }
    }

    /* Read data value */
    if (tdo_read == TRUE) {
        if (hr_cpld_reg_read_lib(reg_offset, &rd_data) == FAILED) {
            return (FAILED);
        }

        cpld_r_buf = rd_data;
    }

    return (cpld_r_buf);
}


/**********************************************************************
 *
 * Function: hr_cpld_read_fn
 *
 * Read CPLD Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         *buf - pointer to read buffer
 *         *param - pointer to param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int hr_cpld_read_fn (unsigned long addr, int size,
                                    unsigned long *buf, void *param)
{
    n2g_i2c_if_t *i2c_if = (n2g_i2c_if_t *)param;
    int rc = PASSED;

    i2c_if->offset = (unsigned int)addr;
    i2c_if->buf = (char *)buf;
    i2c_if->size = size;

    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "Unable to read i2c, ret_code = %#x", rc);
        rc = FAILED;
        return (rc);
    }

    *buf = BYTE_SWAP32(*buf);

    return (rc);
}

/**********************************************************************
 *
 * Function: hr_cpld_write_fn
 *
 * Write CPLD Register.
 *
 * Input : addr - Register offset
 *         size - Register size
 *         data - data for write
 *         *param - pointer to param
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int hr_cpld_write_fn (unsigned long addr, int size,
                                     unsigned long data, void *param)
{
    n2g_i2c_if_t *i2c_if = (n2g_i2c_if_t *)param;
    int rc = PASSED;

    data = BYTE_SWAP32(data);
    i2c_if->buf = (char *)&data;
    i2c_if->offset = (unsigned int)addr;
    i2c_if->size = size;
   
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("CPLD W [%#04lx] = %#010lx on i2c bus addr%#04x\n",
                addr, data, HIGHRISE_CPLD_I2C_ADDR);
    }

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        printf("unable to write i2c.\n");
        rc = FAILED;
        return (rc);
    }
    usleep(5000);

    return (rc);
}


/**********************************************************************
 *
 * Function: speedup_cpld_reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_reset_jtag(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0xFF, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_reset_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_reset_2_run(void)
{
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: erase_all
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void erase_all(int erase_addr)
{
    unsigned short w_data;

    printf("\nErase start address %#x.\n", erase_addr);
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    cpld_shift(erase_addr, 13, 1, 0);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_ERASE */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x013d here. */
    w_data = CPLD_ERASE_COMMAND;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    msleep(600);
}

/**********************************************************************
 *
 * Function: speedup_cpld_run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_run_2_ir(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0xC, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_shift
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static unsigned int cpld_shift(unsigned int data, unsigned char cnt,
                               unsigned char msb, boolean read_tdo)
{
    unsigned char ix;
    unsigned char tms;
    unsigned char tdi;
    unsigned char tdo;
    unsigned int rddata;


    rddata = 0;
    for(ix = 0; ix < cnt; ix++) {
        if(ix == (cnt - 1)) {
            tms = 1;
        } else {
            tms = 0;
        }

        if(msb == 1) {
            tdi = (data & (1 << (cnt - ix - 1))) > 0 ? 1:0;
        } else {
            tdi = (data & (1 << ix))> 0 ? 1:0;
        }

        /* Write TMS, TDI and read TDO */
        tdo = cpld_jtag_io(tms, tdi, read_tdo);

        /* read tdo */
        if(msb == 1) {
            rddata = (tdo != 0) ? (rddata | (1 << (cnt - ix - 1))) : rddata;
        } else {
            rddata = (tdo != 0) ? (rddata | ( 1 << ix)) : rddata;
        }
    }

    return (rddata);
}

/**********************************************************************
 *
 * Function: speedup_cpld_ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_ir_2_run(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x80, FALSE);
}

/**********************************************************************
 *
 * Function: cpld_run_test_delay
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_test_delay(int delay_cycles)
{
    int ix;

    for (ix = 0; ix < delay_cycles; ix++) {
        cpld_jtag_io(0, 0, 0);
    }
}

/**********************************************************************
 *
 * Function: speedup_cpld_dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_dr_2_run(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x80, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_ir_2_dr(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x1C, FALSE);
}

/**********************************************************************
 *
 * Function: speedup_cpld_run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void speedup_cpld_run_2_dr(void)
{
    speed_up_cpld_jtag_io(CPLD_TWELVE, 0x4, FALSE);
}

/**********************************************************************
 *
 * Function: hr_simply_program_cpld
 *
 * This function programs the cpld firmware via pca9557
 *
 * Input : *pof_start_address - pointer to the program firmware.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_simply_program_cpld (unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;

    memset((void *)silicon_id, 0, sizeof(silicon_id));

    /* reset */
    speedup_cpld_reset_jtag();

    /* reset to ir */
    cpld_reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("\nSample Preload.\n");

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0280 here. */
    w_data = CPLD_SAMPLE_PRELOAD;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 8 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 8 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("\nRT ISP Enable.\n");

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0266 here. */
    w_data = CPLD_ISP_ENABLE;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /**************************** read  silicon_id ****************************/
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();

    /* Speed up write data. */
    speed_up_cpld_jtag_io(CPLD_TEN, 0x12, FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, 0x20, FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        speedup_cpld_run_2_dr();
        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        silicon_id[ix] = r_data;
        speedup_cpld_dr_2_run();
    }

#ifdef DEBUG
    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");
#endif

    /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0000 for 5M40Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0100 for 5M80Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0200 for 5M160Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0300 for 5M240Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0400 for 5M570Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0500 for 5M1270Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0600 for 5M2210Z */

    /* Erase 0 */
    erase_all(0x0);
    /* Erase 1 */
    erase_all(0x1000);
    /* Erase 2 */
    erase_all(0x1100);

    /******************************** Program *********************************/
    printf("\nProgram.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_PROG */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x00bd here. */
    w_data = CPLD_ISC_PROG;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Start to program from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    /* 3328 */
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* write */
        speedup_cpld_run_2_dr();

        /* Speed up write data. */
        speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
        speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

        speedup_cpld_dr_2_run();

        /* Delay clock cycle which refers from LA waves of website sample code. */
        speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, FALSE);
    }

    /******************************** Verify *********************************/
    printf("\nVerify.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* w_data=0;
         * w_data=((*file_pt)<<8);
         * file_pt++;
         * w_data=w_data|(*file_pt);
         * file_pt++;
         */

        /* write */
        speedup_cpld_run_2_dr();;

        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        speedup_cpld_dr_2_run();

        if(r_data != w_data) {
            cpld_jtag_io(0, 0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
            return (FAILED);
        }

    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /***************************** Program Done ******************************/
    printf("\nSending Program Done.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_dr_2_run();

    /* LOAD ISC_PROG */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x00bd here. */
    w_data = CPLD_ISC_PROG;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();
    speedup_cpld_run_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0xffdf here. */
    w_data = CPLD_ISC_PROG_DONE_1;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);
    speedup_cpld_dr_2_run();

    usleep(100);

    /* set add 0 */
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0301 here. */
    w_data = CPLD_ADDRESS_SHIFT;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* LOAD Address in DR */
    speedup_cpld_ir_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0 here. */
    w_data = 0x0;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_dr_2_run();

    /* LOAD ISC_READ */
    speedup_cpld_run_2_ir();
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x0281 here. */
    w_data = CPLD_ISP_READ;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    speedup_cpld_run_2_dr();
    /* Speed up write data, MSB = 1, so write from MSB to LSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0xFFFF here. */
    w_data = CPLD_ISC_PROG_DONE_2;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    speedup_cpld_dr_2_run();

    /**************************** RT ISP Disable *****************************/
    printf("\nRT ISP Disable.\n");
    /* run to ir */
    speedup_cpld_run_2_ir();

    /* ISC_DISABLE */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x019a here. */
    w_data = CPLD_ISP_DISABLE;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /* run to ir */
    speedup_cpld_run_2_ir();

    /* BYPASS */
    /* Speed up write data, MSB = 0, so write from LSB to MSB.
     * And add 0 at LSB to make it as 16 bits value.
     * So write 0x03ff here. */
    w_data = CPLD_BYPASS;
    speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

    /* run */
    speedup_cpld_ir_2_run();

    cpld_run_test_delay(7);

    speedup_cpld_reset_jtag();

    printf("\nProgram Done.\n");

    return (PASSED);
}

/* Following codes is the original source code from Altera. */
#ifdef ORIGINAL_SPEEDUP_CPLD_PROGRAM
/**********************************************************************
 *
 * Function: cpld_reset_jtag
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_reset_jtag(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_run_2_ir
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_2_ir(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_ir_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_ir_2_run(void)
{
    /* Enter Update-IR from Exit2-IR */
    cpld_jtag_io(1, 0, 0);
    /* Enter Run from Update-IR */
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_ir_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_ir_2_dr(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_dr_2_run
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_dr_2_run(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: cpld_run_2_dr
 *
 * Input : None
 *
 * Output: None
 *
 **********************************************************************
 */
static void cpld_run_2_dr(void)
{
    cpld_jtag_io(1, 0, 0);
    cpld_jtag_io(0, 0, 0);
    cpld_jtag_io(0, 0, 0);
}

/**********************************************************************
 *
 * Function: speed_up_cpld_jtag_io
 *
 * JTAG I/O routine
 *
 * Input : reg_offset - cpld register offset
 *         cpld_w_buf - cpld write data value
 *         tdo_read - cpld read data value flag
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long speed_up_cpld_jtag_io (uchar reg_offset, uchar cpld_w_buf,
                                   boolean tdo_read)
{
    int ix;
    uchar cpld_r_buf = 0, rd_data;

    if (tdo_read == FALSE) {
        /* Write data value */
        if (hr_cpld_reg_write_lib(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }
    }

    /* Read data value */
    if (tdo_read == TRUE) {
        /* Write data value */
        if (hr_cpld_reg_write_lib(reg_offset, cpld_w_buf) == FAILED) {
            return (FAILED);
        }

        if (hr_cpld_reg_read_lib(CPLD_SPEED_UP, &rd_data) == FAILED) {
            return (FAILED);
        }

        /* Do MSB LSB bit swap. */
        for (ix = 0; ix < 8; ix++) {
            cpld_r_buf = ((rd_data & (0x1 << ix)) != 0) ? (cpld_r_buf | (1 << (7 - ix))) : cpld_r_buf;
        }
    }

    return (cpld_r_buf);
}

/**********************************************************************
 *
 * Function: hr_simply_program_cpld
 *
 * This function programs the cpld firmware via pca9557
 *
 * Input : *pof_start_address - pointer to the program firmware.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
long hr_simply_program_cpld (unsigned char *pof_start_address)
{
    unsigned short silicon_id[5];
    unsigned char *file_pt;
    unsigned int ix;
    unsigned int r_data;
    unsigned short w_data;

    /* reset */
    cpld_reset_jtag();

    /* reset to ir */
    cpld_reset_2_run();

    /***************************** Sample Preload ******************************/
    printf("\nSample Preload.\n");

    /* run to ir */
    cpld_run_2_ir();

    /* ISC_DISABLE */
    cpld_shift(0x5, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x0, 1, 1, 0);

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x0, 1, 1, 0);
    cpld_dr_2_run();

    /* Delay 2 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(2);

    /***************************** RT ISP Enable ******************************/
    printf("\nRT ISP Enable.\n");

    /* run to ir */
    cpld_run_2_ir();

    /* ISC_DISABLE */
    cpld_shift(0x199, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    /**************************** read  silicon_id ****************************/
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();

    /* Speed up write data. */
    speed_up_cpld_jtag_io(CPLD_TEN, 0x12, FALSE);
    speed_up_cpld_jtag_io(CPLD_ELEVEN, 0x20, FALSE);

    /* run */
    cpld_ir_2_run();

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x205, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 4 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(4);

    for(ix = 0; ix < 5; ix++) {
        cpld_run_2_dr();
        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        silicon_id[ix] = r_data;
        cpld_dr_2_run();
    }

#ifdef DEBUG
    printf("\nsilicon_id is: ");
    for(ix = 0; ix < 5; ix++) {
        printf("%#x, ", silicon_id[ix]);
    }
    printf("\n");
#endif

    /* silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0000 for 5M40Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0100 for 5M80Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0200 for 5M160Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0300 for 5M240Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0400 for 5M570Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0500 for 5M1270Z
     * silicon_id = 0x4c41,0x4554,0x4152,0x3031,0x0600 for 5M2210Z */

    /* Erase 0 */
    erase_all(0x0);
    /* Erase 1 */
    erase_all(0x1000);
    /* Erase 2 */
    erase_all(0x1100);

    /******************************** Program *********************************/
    printf("\nProgram.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_ir_2_run();

    /* LOAD ISC_PROG */
    cpld_run_2_ir();
    cpld_shift(0x2F4, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Start to program from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    /* 3328 */
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* write */
        cpld_run_2_dr();

        /* Speed up write data. */
        speed_up_cpld_jtag_io(CPLD_TEN, (w_data >> 8), FALSE);
        speed_up_cpld_jtag_io(CPLD_ELEVEN, (w_data & 0xFF), FALSE);

        cpld_dr_2_run();

        /* Delay clock cycle which refers from LA waves of website sample code. */
        speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, FALSE);
    }

    /******************************** Verify *********************************/
    printf("\nVerify.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_ir_2_run();

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x205, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Start to verify from firmware offset 171 which refers from LA waves of
     * website sample code. */
    file_pt = pof_start_address + 171;
    for(ix = 0; ix < 3328; ix++) {

        /* Show the status bar */
        if ((ix % 50) == 0) {
            printf(".");
        }

        w_data = 0;
        w_data = *file_pt;
        file_pt++;
        w_data = w_data | ((*file_pt) << 8);
        file_pt++;

        /* w_data=0;
         * w_data=((*file_pt)<<8);
         * file_pt++;
         * w_data=w_data|(*file_pt);
         * file_pt++;
         */

        /* write */
        cpld_run_2_dr();

        /* Speed up read data. */
        r_data = speed_up_cpld_jtag_io(CPLD_TEN, 0xFF, TRUE);
        r_data = ((r_data << 8) | speed_up_cpld_jtag_io(CPLD_ELEVEN, 0xFF, TRUE));
        cpld_dr_2_run();

        if(r_data != w_data) {
            cpld_jtag_io(0, 0, 0);
            printf("Verify address %d fail r_data is %#x w_data is %#x\n",
                   ix, r_data, w_data);
            return (FAILED);
        }

    }

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /***************************** Program Done ******************************/
    printf("\nSending Program Done.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10 ,0 , 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_dr_2_run();

    /* LOAD ISC_PROG */
    cpld_run_2_ir();
    cpld_shift(0x2F4, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    cpld_run_2_dr();
    w_data = 0xFFDF;
    r_data = cpld_shift(w_data, 16, 1, 0);           

    cpld_dr_2_run();

    usleep(100);

    /* set add 0 */
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_ADDRESS_SHIFT */
    cpld_shift(0x203, 10, 0, 0);

    /* LOAD Address in DR */
    cpld_ir_2_dr();
    cpld_shift(0x0, 13, 1, 0);

    /* run */
    cpld_dr_2_run();

    /* LOAD ISC_READ */
    cpld_run_2_ir();
    cpld_shift(0x205, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    cpld_run_2_dr();
    r_data = cpld_shift(0xFFFF, 16, 1, 0);

    cpld_dr_2_run();

    /**************************** RT ISP Disable *****************************/
    printf("\nRT ISP Disable.\n");
    /* run to ir */
    cpld_run_2_ir();

    /* ISC_DISABLE */
    cpld_shift(0x166, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    /* Delay 7 clock cycle which refers from LA waves of website sample code. */
    cpld_run_test_delay(7);

    /* run to ir */
    cpld_run_2_ir();

    /* BYPASS */
    cpld_shift(0x3FF, 10, 0, 0);

    /* run */
    cpld_ir_2_run();

    cpld_run_test_delay(7);

    cpld_reset_jtag();

    printf("\nProgram Done.\n");

    return (PASSED);
}
#endif

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: highrise_cpld_lib.c,v $
 * Revision 1.3  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.2  2021/01/25 09:22:29  markzha
 * Sync RDT issues fixing and optimize compiling for Highrise
 *
 * Revision 1.1  2020/08/19 09:50:53  markzha
 * *** empty log message ***
 *
 * Revision 1.3  2017/07/14 02:51:39  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.9  2014/04/22 06:06:03  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.8  2014/03/19 07:13:50  kodko
 * Speed up the CPLD firmware upgrade time under 2 minutes.
 *
 * Revision 1.1.2.7  2014/03/11 03:56:11  leschen
 * Miss argument for setting sync trig out function.
 *
 * Revision 1.1.2.6  2014/03/11 03:14:37  leschen
 * Fix setting timingcard clk/trig function.
 *
 * Revision 1.1.2.5  2014/03/10 08:00:10  kodko
 * Remove redundant code.
 *
 * Revision 1.1.2.4  2014/03/07 07:39:58  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.3  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------------------
 */


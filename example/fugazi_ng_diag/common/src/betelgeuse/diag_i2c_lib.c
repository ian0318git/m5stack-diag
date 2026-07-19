/* $Id: diag_i2c_lib.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_i2c_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "types.h"
#include "proto.h"
#include "free.h"
#include "defs.h"
#include "error.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "dev_print.h"
#include "dev_object.h"
#include "common.h"
#include "common_utils.h"
#include "goofy_i2c.h"
#include "byteswap.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "diag_sirius_fpga_lib.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "cross_platform.h"
#include "dev_at24c0n.h"        /* 256-byte EEPROM special handling */
#include "diag_i2c_lib.h"
#include "i2c_dev.h"

/* Extern */
extern int get_i2c_fd(int);
extern unsigned char i2c_debug;

static n2g_i2c_states_t *get_n2g_i2c_states_table(uint8_t i2c_bus,
                                                  uint8_t i2c_dev);
static uint32_t i2c_dev_read(n2g_i2c_dev_t *, ulong, uint8_t, char *);
static uint32_t i2c_dev_write(n2g_i2c_dev_t *, ulong, uint8_t, char *);

static char *i2c_err[] = {
    "OK",
    "BUSY",
    "time out",
    "RC_I2C_DMA_ADDR_NOT_64ALIGN",
    "no slave device ack",
    "no slave sub_addr device ack",
    "RC_I2C_BUS_ERR",
    "unknown error",
    "\0",
};

/*********************************************************************
 *        I2C devices characteristics tables.
 *********************************************************************
 */
/* CPU I2C controller 0 devices */
static n2g_i2c_dev_t n2g_i2c0_eeprom = { CPU_I2C0, MB_I2C_ADDR_EEPROM, 1, 1, 0 };       /* EEPROM 2k bit */
static n2g_i2c_dev_t n2g_i2c0_quack = { CPU_I2C0, MB_I2C_ADDR_ACT2, 0, 0, 0 };  /* Secure Chip */
static n2g_i2c_dev_t n2g_i2c0_aikido_quack = { CPU_I2C0, MB_I2C_ADDR_AIKIDO_ACT2, 0, 0, 0 };  /* Secure Chip */

/* CPU I2C controller 1 devices */
static n2g_i2c_dev_t n2g_i2c1_sfp0 = { CPU_I2C1, MB_I2C_ADDR_SFP0, 1, 1, 0 };  /* 88E1548L SFP */
static n2g_i2c_dev_t n2g_i2c1_mb_temp = { CPU_I2C1, MB_I2C_ADDR_MB_TEMP, 1, 1, 0 }; /* MB Temp Sensor (MAX31730) */
static n2g_i2c_dev_t n2g_i2c1_sfp_int_reg = { CPU_I2C1, MB_I2C_ADDR_SFP0_INT_REG, 1, 1, 0 };  /* 88E1548L SFP */

/* CPU I2C controller 2 devices */
static n2g_i2c_dev_t n2g_i2c2_rtc = { CPU_I2C2, MB_I2C_ADDR_RTC, 1, 1, 0 };     /* RTC DS1337S+ */
static n2g_i2c_dev_t n2g_i2c2_poe = { CPU_I2C2, MB_I2C_ADDR_POE_30W_CTRLER, 1, 1, 0 };  /* 30W POE */
static n2g_i2c_dev_t n2g_i2c2_mcu = { CPU_I2C2, MB_I2C2_MCU, 1, 1, 0 };  /* MCU */
static n2g_i2c_dev_t n2g_i2c2_mcu_bl = { CPU_I2C2, MB_I2C2_MCU_BOOTLOADER, 1, 1, 0 };  /* MCU Bootloader */
static n2g_i2c_dev_t n2g_i2c2_poe_eeprom = { CPU_I2C2, MB_I2C_ADDR_POE_EEPROM, 1, 1, 0 };  /* POE EEPROM */
static n2g_i2c_dev_t n2g_i2c2_wifi_act2 = { CPU_I2C2, WIFI_I2C_ADDR_ACT2, 1, 1, 0 };  /* Wifi ACT2 */
static n2g_i2c_dev_t n2g_i2c2_wifi_temp = { CPU_I2C2, WIFI_I2C_ADDR_TEMP, 1, 1, 0 };  /* Wifi Temp Sensor (MAX31730) */
static n2g_i2c_dev_t n2g_i2c2_wifi_plat_temp = { CPU_I2C2, WIFI_I2C_PLAT_ADDR_TEMP, 1, 1, 0 };  /* Wifi Temp Sensor (MAX31730) */

/* Test Card I2C controller 0 devices */     
static n2g_i2c_dev_t n2g_i2c0_plug_tc_temp = { PLUG_FPGA, PLUG_I2C_ADDR_TEMP, 1, 1, 0 };       /* Pluggable Temp Sensor (LM75BDP) */
static n2g_i2c_dev_t n2g_i2c0_plug_tc_act2 = { PLUG_FPGA, PLUG_I2C_ADDR_ACT2, 0, 0, 0 };  /* Pluggable FPGA Secure Chip */
static n2g_i2c_dev_t n2g_i2c0_plug_tc_gpio_exp = { PLUG_FPGA, PLUG_TC_I2C_ADDR_GPIO_EXP, 0, 0, 0 };  /* Pluggable Test Card GPIO Expander */

/*********************************************************************
 *        I2C device state tables.
 *********************************************************************
 */
static n2g_i2c_states_t i2c_mb0_state[MB_I2C_0_INVALID] = {
    {0, &n2g_i2c0_eeprom, N2G_I2C_IDLE},        /* EEPROM 2K bit */
    {0, &n2g_i2c0_quack, N2G_I2C_IDLE}, /* Secure Chip */
    {0, &n2g_i2c0_aikido_quack, N2G_I2C_IDLE}, /* Aikido Secure Chip */
};

static n2g_i2c_states_t i2c_mb1_state[MB_I2C_1_INVALID] = {
    {0, &n2g_i2c1_mb_temp, N2G_I2C_IDLE},       /* Mother Board Temp Sensor */
    {0, &n2g_i2c1_sfp0, N2G_I2C_IDLE},		/* 88E1112 SFP */
    {0, &n2g_i2c1_sfp_int_reg, N2G_I2C_IDLE},		/* 88E1112 SFP */
};

static n2g_i2c_states_t i2c_mb2_state[MB_I2C_2_INVALID] = {
    {0, &n2g_i2c2_rtc, N2G_I2C_IDLE},   /* RTC DS1337 */
    {0, &n2g_i2c2_poe, N2G_I2C_IDLE},   /* 30W POE CTRL */
    {0, &n2g_i2c2_mcu, N2G_I2C_IDLE},   /* MCU */
    {0, &n2g_i2c2_mcu_bl, N2G_I2C_IDLE},   /* MCU bootloader */
    {0, &n2g_i2c2_poe_eeprom, N2G_I2C_IDLE},   /* POE EEPROM */
    {0, &n2g_i2c2_wifi_act2, N2G_I2C_IDLE},   /* Wifi ACT2 */
    {0, &n2g_i2c2_wifi_temp, N2G_I2C_IDLE},   /* Wifi Temp */
    {0, &n2g_i2c2_wifi_plat_temp, N2G_I2C_IDLE},   /* Platform Wifi Temp */
};

static n2g_i2c_states_t i2c_plug0_state[PLUG_FPGA_INVALID] = {
    {0, &n2g_i2c0_plug_tc_temp, N2G_I2C_IDLE},       /* Pluggable Test Card Temp Sensor */
    {0, &n2g_i2c0_plug_tc_act2, N2G_I2C_IDLE},       /* Pluggable Test Card FPGA Secure Chip */
    {0, &n2g_i2c0_plug_tc_gpio_exp, N2G_I2C_IDLE},       /* Pluggable Test Card GPIO Expander */
};

char *i2c_err_str(int num)
{
    if (num < RC_I2C_UNKNOWN)
        return i2c_err[num];
    return i2c_err[RC_I2C_UNKNOWN];
}
static int err_no = 0;
static int i2c_status = 0;

static void plug_fpga_wr_i2c_data_fifo(int, uint32_t, uchar *);

static void plug_fpga_i2c_reset(int);
static int plug_fpga_i2c_normal_op(int, uint8_t, uint32_t, uint32_t,
                                   uint32_t, uint32_t, uint32_t);
static int plug_fpga_i2c_send_reg_offset (int , uint32_t, uint32_t,
                                          uint32_t, uint32_t);
static int i2c_flush_fifo (int, int);

int plug_fpga_i2c_rd(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);
int plug_fpga_i2c_wr(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);

static uint32_t i2c_dswap4(int x)
{
     return dswap4(x);
}

/**********************************************************************
 *
 * Function: plug_fpga_chk_i2c_idle_temp
 *
 * Description: Check if the i2c master is idle
 *
 * Input: none
 *
 * Output: TRUE or FALSE
 *
 **********************************************************************/
boolean plug_fpga_chk_i2c_idle_temp (int i2c_addr)
{
    uint32_t ix, timeout_val, retry;
    volatile uint32_t i2c_status;

    timeout_val = PLUG_FPGA_I2C_IDLE_TIMEOUT;//GFY_I2C_XFER_BIT_COUNT(2);
    for (retry = 0 ;  retry < 5; retry++) {
        for (ix = 0; ix < timeout_val; ix++) {
            plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, (uint *)&i2c_status);
            if (i2c_status & MSK_PLUG_I2C_STAT_NOT_ACTIVE) {
                return (TRUE);
            }
            msleep(10);
        }
        plug_fpga_i2c_reset(i2c_addr);
    }
    printf("i2c failure: data is still being trasferred. too long to complete.\n");
    print_offset_val("", base_plug_fpga, (ulong)i2c_status, __LINE__, 0);
    return (FALSE);
}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_reset
 *
 * Description: Reset an I2C master module
 *
 * Input: none
 *
 * Output: PASSED or FAILED
 *
 *********************************************************************/
void plug_fpga_i2c_reset (int i2c_addr)
{
    int ctr = 0;
    int i2c_ctrl, i2c_bb;

    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    i2c_ctrl |= PLUG_I2C_CTRL_SOFT_RESET;
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, i2c_ctrl);
    usleep(PLUG_FPGA_REG_WRITE_DELAY);

    /* goes into bitbang mode */
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    i2c_ctrl |= PLUG_I2C_CTRL_BITBANG;
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, i2c_ctrl);

    /* drives the SDA lines low */
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
    i2c_bb &= ~(PLUG_I2C_BITBANG_SDA_DRIVER);
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_BITBANG_OFFSET, i2c_bb);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        print_offset_val("", base_plug_fpga, (ulong)i2c_bb, __LINE__, 0);
    }

    /* keeps driving SCL until it recovers */
    for (ctr = 0; ctr < SCL_DRIVE_TIMES; ctr++) {
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
        i2c_bb &= ~(PLUG_I2C_BITBANG_SCL_DRIVER);
        plug_fpga_reg_write(i2c_addr + PLUG_I2C_BITBANG_OFFSET, i2c_bb);
        msleep(1);
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
        i2c_bb |= PLUG_I2C_BITBANG_SCL_DRIVER;
        plug_fpga_reg_write(i2c_addr + PLUG_I2C_BITBANG_OFFSET, i2c_bb);
        msleep(1);
    }

    /* drives the SDA lines High */
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_BITBANG_OFFSET, (uint *)&i2c_bb);
    i2c_bb |= PLUG_I2C_BITBANG_SDA_DRIVER;

    /* leave bitbang mode */
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    i2c_ctrl &= ~(PLUG_I2C_CTRL_BITBANG);
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, i2c_ctrl);

}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_send_reg_offset
 *
 * Description: send register offset to plug FPGA i2c bus 
 *
 * Input: i2c - point to pluggable fpga i2c master
 *        slv_addr - point to pluggable i2c slave device
 *        reg_addr - register address
 *         
 * Output: return OK
 *
 **********************************************************************/
static int plug_fpga_i2c_send_reg_offset (int i2c_addr, uint32_t mux,
                                          uint32_t slv_addr,uint32_t reg_addr,
                                          uint32_t sub_addr_sz)
{
    uint32_t rc;
    uint32_t addr_size = 1;

    /* write 1 byte reg offset into data fifo */
    plug_fpga_wr_i2c_data_fifo(i2c_addr, addr_size, (unsigned char *)
                               &reg_addr);

    /* initializte write transaction to send off set onto the bus */
    rc = (plug_fpga_i2c_normal_op(i2c_addr, mux, slv_addr, addr_size,
                                  sub_addr_sz, reg_addr,
                                  GFY_I2C_CTRL_WR_MODE));

    if (rc != RC_I2C_OP_OK) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c_drv.c sending offfset failed %d; line %d\n", rc, __LINE__);
        }
    }
    return rc;
}

/**********************************************************************
 *
 * Function: i2c_flush_fifo
 *
 * Description: flush out data inside fifo
 *
 * Input: i2c - pointer to pluggable FPGA i2c master
 *        byte - number of byte to flush
 * Output: RC_I2C_OP_OK
 *
 **********************************************************************/
static int i2c_flush_fifo (int i2c_addr, int byte)
{
    unsigned int tmp;

    while (byte--) {
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&tmp);
        tmp++;
    }
    /* do it a couple of more times just to be safe */
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&tmp);
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&tmp);

    return (RC_I2C_OP_OK);

}

/**********************************************************************
 *
 * Function: plug_fpga_display_i2c_reg
 *
 * Description: Show the i2c registers
 *
 * Input: i2c - pointer to pluggable FPGA i2c master
 *
 * Output: void
 *
 **********************************************************************/
int plug_fpga_display_i2c_reg (int i2c_addr)
{
    int i2c_ctrl, i2c_scratch, i2c_status, i2c_status_mask;
    int i2c_slave_addr, i2c_slave_sub_addr;

    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, (uint *)&i2c_ctrl);
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_SCEACH_PAD_OFFSET, (uint *)&i2c_scratch);
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, (uint *)&i2c_status);
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_MASK_OFFSET, (uint *)&i2c_status_mask);
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_SLAVE_ADDR_OFFSET, (uint *)&i2c_slave_addr);
    plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_SUBSL_ADDR_OFFSET, (uint *)&i2c_slave_sub_addr);
    
    printf("i2c_control = %#.8x\n", i2c_ctrl);
    printf("i2c_scratch = %#.8x\n", i2c_scratch);
    printf("i2c_status = %#.8x\n", i2c_status);
    printf("i2c_status_mask = %#.8x\n", i2c_status_mask);
    printf("i2c_slave_addr = %#.8x\n", i2c_slave_addr);
    printf("i2c_slave_sub_addr = %#.8x\n", i2c_slave_sub_addr);

    return (PASSED);
}

/**********************************************************************
 *
 * Function: plug_fpga_rd_i2c_data_fifo_temp
 *
 * Description: Read data bytes from the i2c data fifo
 *
 * Input: i2c_addr - pointer to pluggable FPGA i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *
 ***********************************************************************/
void plug_fpga_rd_i2c_data_fifo_temp (int i2c_addr, uint32_t data_len,
                                 uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;
    int data_in, data_in_tmp;

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (ix = 0; ix < word_count; ix++) {
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&data_in);
        data_in_tmp = data_in; 
        dword = i2c_dswap4(data_in);
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
            *data_p++ = *byte_p++;
        }
    }

    if (byte_count > 0) {
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET, (uint *)&data_in);
        dword = i2c_dswap4(data_in);
        byte_p = (uchar *)&dword;
        for (ix = 0; ix < byte_count; ix++) {
            *data_p++ = *byte_p++;
        }
    }

    /* flush again just to be safe */
    i2c_flush_fifo(i2c_addr, data_len);

}

/**********************************************************************
 *
 * Function: plug_fpga_wr_i2c_data_fifo
 *
 * Description: Write data bytes to the i2c data fifo
 *
 * Input: i2c - pointer to pluggable FPGA i2c master
 *        data_len - The number of bytes to xfer
 *        data_p - pointer to the buffer holding the data being xfer
 *
 * Output: void
 *
 ***********************************************************************/
void plug_fpga_wr_i2c_data_fifo (int i2c_addr, uint32_t data_len,
                                 uchar *data_p)
{
    uint32_t ix, jx, word_count, byte_count;
    uint32_t dword;
    uchar *byte_p;

    /* do i need to read until i get an underflow condition to
       make sure fifo is empty before writing to fifo? */

    word_count = data_len / 4;
    byte_count = data_len % 4;

    for (ix = 0; ix < word_count; ix++) {
        byte_p = (uchar *)&dword;
        for (jx = 0; jx < 4; jx++) {
            *byte_p++ = *data_p++;
        }
        plug_fpga_reg_write(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET,
                            (uint )i2c_dswap4(dword));
    }

    if (byte_count) {
        dword = 0;
        byte_p = (uchar *)&dword;
        for (ix = 0; ix < byte_count; ix++) {
            *byte_p++ = *data_p++;
        }
        plug_fpga_reg_write(i2c_addr + PLUG_I2C_DATA_FIFO_OFFSET,
                            (uint )i2c_dswap4(dword));
    }
}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_normal_op
 *
 * Description: Perform standard I2C read or write operation on the
 *              I2C slave device
 *
 * Input: i2c - Pluggable FPGA i2c reg address
 *        mux - mux port
 *        slv_addr - The i2c slave's address on the i2c bus
 *        rd_wr_mode - Read or write
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 **********************************************************************/
int plug_fpga_i2c_normal_op (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                             uint32_t data_len, uint32_t sub_addr_sz,
                             uint32_t reg_addr, uint32_t rd_wr_mode)
{
    uint32_t reg_val, ix, timeout_val, temp_val, wait;
    err_no = 0;
    i2c_status = 0;

    /* Set up control register and the slave address register */
    reg_val = PLUG_I2C_CTRL_CLK_50 | PLUG_I2C_CTRL_CLK_50 |
              PLUG_I2C_CTRL_SPEED_NORMAL_100 | rd_wr_mode |
             (data_len << L_SHFT_PLUG_I2C_CTRL_BYTE_LEN) |
             (mux << L_SHFT_PLUG_I2C_CTRL_MUX);

    if (mux >= 4) {
        assert("mux has to be less than 4");
    }

    /* write mode end */
    /* Enable the normal operation */
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SLAVE_ADDR_OFFSET, slv_addr);
    if (sub_addr_sz != 0) {
        plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SUBSL_ADDR_OFFSET, reg_addr);
    }

    reg_val |= (PLUG_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, reg_val);

    /* give time for device to send acknowlegement..especiall when talking to quack */
    wait = (data_len * 10); /*10 byte address @ 100Khz */
    msleep(PLUG_FPGA_I2C_OP_DELAY);

    /* Monitor the done bit in status register. Add 10 satety bytes for
     * wait time calculation due to I2C protocol is slow and have gaps
     */
    timeout_val = PLUG_FPGA_I2C_OP_TOUT;

    /* Wait one byte time to let the i2c op to start before polling status */
    /* if no delay we might miss the no ack */
    for (ix = 0; ix <= timeout_val; ix++) {
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, &reg_val);
        temp_val = reg_val;
        if (temp_val & MSK_PLUG_I2C_STAT_NO_SLV) { /* check bit 2 */
            err_no = (RC_I2C_SLV_NACK);
            printf("\n\n");
            printf("%s:%d:device shown below is not acknowledging; is it installed? "
                   " [i2c status @%#x=%#x %d]\n",
                   __FUNCTION__,__LINE__,
                   i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, reg_val, err_no);
            return (RC_I2C_SLV_NACK);
        } 

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & MSK_PLUG_I2C_STAT_STD_DONE) != 0) {
            break;
        }
        msleep(8);
    }
    if (ix > timeout_val) {
        err_no = (RC_I2C_TIMEOUT);
        printf("%s:%d:device shown below is not acknowledging; is it installed? "
               " [i2c status @%#x=%#x %d]\n",
               __FUNCTION__,__LINE__,
               i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, reg_val, err_no);
        return (RC_I2C_TIMEOUT);
    }

    return (RC_I2C_OP_OK);
}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_rd
 *
 * Description: Read data from the I2C slave device to the data buffer
 *              in normal I2C mode
 *
 * Input: i2c_addr - Offset to I2C Controller Address
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 ************************************************************************/
int plug_fpga_i2c_rd (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                      int32_t reg_addr, uint32_t sub_addr_sz,
                      uint32_t data_len, uchar *data_buf)
{
    int rc;

    /* Unmasking I2C Master Status Register */
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_STS_MASK_OFFSET, 0);

    if (plug_fpga_chk_i2c_idle_temp(i2c_addr) == FALSE) {
        return (RC_I2C_BUSY);
    }
    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_sz == 0)) {
        rc = plug_fpga_i2c_send_reg_offset(i2c_addr, mux, slv_addr,
                                           reg_addr, sub_addr_sz);
        if (rc != RC_I2C_OP_OK) {
            return rc;
        }
    }

    /* send read request to the slave */
    rc = (plug_fpga_i2c_normal_op(i2c_addr, mux, slv_addr, data_len, sub_addr_sz,
                                  reg_addr, PLUG_I2C_CTRL_RD_MODE));
    if (rc != RC_I2C_OP_OK) {
        return rc;
    }

    /* read data from fifo and flush */
    plug_fpga_rd_i2c_data_fifo_temp(i2c_addr, data_len, data_buf);

    return rc;

}

/**********************************************************************
 *
 * Function: plug_fpga_i2c_wr
 *
 * Description: send i2c reg offset and data into data fifo, then
 *              write to pluggable FPGA control register to flush data on to the bus
 *              the first byte on the bus will be reg offset.
 *
 * Input: i2c_addr - Pluggable FPGA i2c master address
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_size - byte size of the reg_addr (0 to 3 bytes)
 *        reg_offset - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = RC_I2C_OP_OK, RC_I2C_BUSY, RC_I2C_SLV_NACK or RC_I2C_TIMEOUT.
 *
 ************************************************************************/
/* plug_fpga_i2c_wr: send i2c reg offset and data into data fifo, then
   write to pluggable FPGA control register to flush data on to the bus
   the first byte on the bus will be reg offset.
*/
int plug_fpga_i2c_wr (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                      int32_t reg_offset, uint32_t sub_addr_size,
                      uint32_t data_len, uchar *data_buf)
{
    unsigned int rc;
    unsigned char *buf = NULL;

    if (plug_fpga_chk_i2c_idle_temp(i2c_addr) == FALSE) {
        return (RC_I2C_BUSY);
    }
    buf = malloc(data_len+sizeof(uint32_t));  /* we should allocate at least 4 bytes */
    memset(buf, 0, data_len+sizeof(uint32_t));
    if (sub_addr_size != 0) {
        plug_fpga_wr_i2c_data_fifo(i2c_addr, data_len, data_buf);
    }
    else {
        /* if required, send reg offset to slave device and flush fifo */
        if (reg_offset >= 0) {
            buf[0] = (unsigned char )reg_offset & 0xFF;
            data_len++;
            /*
            if (reg_offset & 0xFF00) {
                buf[1] = (unsigned char )((reg_offset & 0xFF00) >> 8);
                data_len++;
                memcpy(&buf[2], data_buf, data_len);
            } else {
                memcpy(&buf[1], data_buf, data_len);
            }
            */
            memcpy(&buf[1], data_buf, data_len);

        } else {
            /*  for smart devices (ie ACT2) that dont' want address to be sent */
            /* here we send data only ...no address */
            memcpy(&buf[0], data_buf, data_len);
        }
        plug_fpga_wr_i2c_data_fifo(i2c_addr, data_len, buf);
    }

    free(buf);

    /* write data to fifo ...(is it safe to flush fifo here, or do it
       later after we send out data to slave?) */

    /* initialiate write transaction */
    rc = (plug_fpga_i2c_normal_op(i2c_addr, mux, slv_addr, data_len,
                                  sub_addr_size, reg_offset,
                                  PLUG_I2C_CTRL_WR_MODE));

    return rc;
}

/**********************************************************************
 *
 * Function: i2c_err_no
 *
 * Description: return the error number of I2C device
 *
 * Input: status - point to the I2C status code
 *
 * Output: error number
 *
 **********************************************************************/
int i2c_err_no(uint32_t * status)
{
    *status = i2c_status;
    return err_no;
}


/**********************************************************************
 *
 * Function: plug_fpga_i2c_ack_check
 *
 * Description: Check Plug I2C device have ack or not
 *
 * Input: i2c_addr - Offset to I2C Controller Address
 *        slv_addr - The i2c slave's address on the i2c bus
 *        sub_addr_sz - byte size of the reg_addr (0 to 3 bytes)
 *        reg_addr - device register address
 *        data_len - The number of bytes to be xfer
 *        data_buf - The buffer to store the data being xfer
 *
 * Output: rc = PASSED have ACK
 *            = FAILED no ACK
 *
 **********************************************************************/
int plug_fpga_i2c_ack_check (int i2c_addr, uint8_t mux, uint32_t slv_addr,
                      int32_t reg_addr, uint32_t sub_addr_sz,
                      uint32_t data_len, uchar *data_buf)
{
    uint32_t reg_val, ix, timeout_val, temp_val, wait; 
    err_no = 0;
    i2c_status = 0;
    int rc;

    /* Unmasking I2C Master Status Register */
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_STS_MASK_OFFSET, 0);

    if (plug_fpga_chk_i2c_idle_temp(i2c_addr) == FALSE) {
        return (FAILED);
    }
    /* if required, send reg offset to slave and flush fifo */
    if ((reg_addr >= 0) && (sub_addr_sz == 0)) {
        rc = plug_fpga_i2c_send_reg_offset(i2c_addr, mux, slv_addr,
                                           reg_addr, sub_addr_sz);
        if (rc != RC_I2C_OP_OK) {
            return (FAILED);
        }
    }
    
    /* Set up control register and the slave address register */
    reg_val = PLUG_I2C_CTRL_CLK_50 | PLUG_I2C_CTRL_CLK_50 |
              PLUG_I2C_CTRL_SPEED_NORMAL_100 | PLUG_I2C_CTRL_RD_MODE |
             (data_len << L_SHFT_PLUG_I2C_CTRL_BYTE_LEN) |
             (mux << L_SHFT_PLUG_I2C_CTRL_MUX);

    if (mux >= 4) {
        assert("mux has to be less than 4");
    }

    /* write mode end */
    /* Enable the normal operation */
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SLAVE_ADDR_OFFSET, slv_addr);
    if (sub_addr_sz != 0) {
        plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_SUBSL_ADDR_OFFSET, reg_addr);
    }

    reg_val |= (PLUG_I2C_CTRL_NORMAL | (sub_addr_sz) << 24);
    plug_fpga_reg_write(i2c_addr + PLUG_I2C_MSTR_CTRL_OFFSET, reg_val);

    /* give time for device to send acknowlegement..especiall when talking to quack */
    wait = (data_len * 10); /*10 byte address @ 100Khz */
    msleep(PLUG_FPGA_I2C_OP_DELAY);

    /* Monitor the done bit in status register. Add 10 satety bytes for
     * wait time calculation due to I2C protocol is slow and have gaps
     */
    timeout_val = PLUG_FPGA_I2C_OP_TOUT;

    /* Wait one byte time to let the i2c op to start before polling status */
    /* if no delay we might miss the no ack */
    for (ix = 0; ix <= timeout_val; ix++) {
        plug_fpga_reg_read(i2c_addr + PLUG_I2C_MSTR_STS_OFFSET, &reg_val);
        temp_val = reg_val;
        if (temp_val & MSK_PLUG_I2C_STAT_NO_SLV) { /* check bit 2 */
            err_no = (RC_I2C_SLV_NACK);
            return (FAILED);
        } 

        /* if slave device does not answer, return busy status bit 4A */
        if ((reg_val & MSK_PLUG_I2C_STAT_STD_DONE) != 0) {
            break;
        }
        msleep(8);
    }
    if (ix > timeout_val) {
        err_no = (RC_I2C_TIMEOUT);
        return (FAILED);
    }

    /* read data from fifo and flush */
    plug_fpga_rd_i2c_data_fifo_temp(i2c_addr, data_len, data_buf);

    return (PASSED);
}

/*********************************************************************
 *
 * Function:    n2g_i2c_open
 *
 * Description:    legacy code. not used.
 *           
 *********************************************************************
 */
uint32_t n2g_i2c_open(n2g_i2c_if_t * i2c_p)
{
    return (PASSED);

}

/********************************************************************
 *
 * Function:    n2g_i2c_close
 *
 * Description:    legacy code. not used.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_close(n2g_i2c_if_t * i2c_p)
{
    return (PASSED);
}

/*********************************************************************
 *
 * Function:    n2g_i2c_read
 *
 * Description:    N2G Generic I2C Read API.
 *
 * Inputs:    i2c_p    - Pointer to the N2G I2C API interface struct. Fields
 *              needed in the struct are:
 *              i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:    PASSED - No errors encounterd.
 *        E_I2C_INV_DEV - Invalid device address.
 *        E_I2C_NOT_LOCKED - Device not locked by any process.
 *        E_I2C_LOCKED - Device is locked by another process.
 *        E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *        Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_read(n2g_i2c_if_t * i2c_p)
{
    uint32_t rc = 0;
    unsigned long addr = 0;
    n2g_i2c_states_t *state_p;  /* pointer to the state struct */

    /*
     * Call the lower device driver 
     */
    switch (i2c_p->i2c_bus_type) {
    case CPU_I2C0:
    case CPU_I2C1:
    case CPU_I2C2:
        state_p =
            get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);
        if (state_p != NULL) {

            rc = i2c_dev_read(state_p->i2c_dev,
                              i2c_p->offset, i2c_p->size, i2c_p->buf);

        } else {
            printf("I2C%d is not support this device!\n",
                   i2c_p->i2c_bus_type);
            rc = FAIL;
        }
        break;
    case PLUG_FPGA:
        addr = get_plug_fpga_i2c_addr(i2c_p->i2c_ctrl);

        rc = plug_fpga_i2c_rd(addr, i2c_p->mux, i2c_p->i2c_dev,
                              i2c_p->offset,
                              i2c_p->sub_addr_len,
                              i2c_p->size,
                              (unsigned char *) i2c_p->buf);
        if (i2c_debug) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->sub_addr_len %d\n", i2c_p->sub_addr_len);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->buf 0x%X\n", *i2c_p->buf);
        }
        break;

    default:
        printf("not suported i2c_api.c %d line %d\n", i2c_p->i2c_bus_type,
               __LINE__);
        assert(0);
        break;
    }                           /* endof switch */

    return (rc);
}

/*********************************************************************
 *
 * Function:    i2c_dev_read
 *
 * Description:    Motherboard I2C Read API.
 *
 * Inputs:    dev_p    - Pointer to device characteristics table.
 *        offset    - I2C device offset.
 *        size    - Number of bytes to read.
 *        *buf    - Read buffer pointer.
 *
 * Outputs:    PASSED - No errors encounterd.
 *        E_I2C_INV_P   - Invalid slave address.
 *        E_I2C_CTL_ERR - I2C controller error.
 *        E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
i2c_dev_read(n2g_i2c_dev_t * dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    int fd_i2c1 = get_i2c_fd(1);
    int fd_i2c2 = get_i2c_fd(2);

    if (dev_p->bus_no == CPU_I2C0) {
        if (fd_i2c0 > 0) {
            if ((rc = ioctl(fd_i2c0, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                       __FUNCTION__, __LINE__, dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c0;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if (fd_i2c1 > 0) {
            if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                       __FUNCTION__, __LINE__, dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c1;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C2) {
        if (fd_i2c2 > 0) {
            if ((rc = ioctl(fd_i2c2, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                       __FUNCTION__, __LINE__, dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c2;
            }
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("%s:%d:; dev_p->fp: %d\n", __FUNCTION__, __LINE__, dev_p->fp);
        printf("dev_addr: %#x, rd_hd_size %d \n\n", dev_p->dev_addr,
               dev_p->rd_hd_size);
    }
    rc = api_mb_i2c_read(dev_p, offset, size, buf);

    return (rc);
}

/*********************************************************************
 *
 * Function:    n2g_i2c_write
 *
 * Description:    N2G Generic I2C Write API.
 *
 * Inputs:    i2c_p    - Pointer to the N2G I2C API interface struct. Fields
 *              needed in the struct are:
 *              i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:    PASSED - No errors encounterd.
 *        E_I2C_INV_DEV - Invalid device address.
 *        E_I2C_NOT_LOCKED - Device not locked by any process.
 *        E_I2C_LOCKED - Device is locked by another process.
 *        E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *        Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_write(n2g_i2c_if_t * i2c_p)
{

    unsigned long addr = 0;
    n2g_i2c_states_t *state_p;  /* pointer to the state struct */
    uint rc;

    /*
     * Call the lower device driver 
     */
    switch (i2c_p->i2c_bus_type) {
    case CPU_I2C0:
    case CPU_I2C1:
    case CPU_I2C2:
        state_p =
            get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);
        /*
         * Southbridge 
         */
        rc = i2c_dev_write(state_p->i2c_dev, i2c_p->offset,
                           i2c_p->size, i2c_p->buf);
        break;
    case PLUG_FPGA:
        addr = get_plug_fpga_i2c_addr(i2c_p->i2c_ctrl);
        rc = plug_fpga_i2c_wr(addr, i2c_p->mux, i2c_p->i2c_dev,
                             i2c_p->offset,
                             i2c_p->sub_addr_len,
                             i2c_p->size,
                             (unsigned char *)i2c_p->buf);
        if (i2c_debug) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->sub_addr_len %d\n", i2c_p->sub_addr_len);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->buf 0x%X\n\n", *i2c_p->buf);
        }
    break;

    default:
        printf("not suported i2c_api.c n2g_i2c_write: %d line %d\n",
               i2c_p->i2c_bus_type, __LINE__);
        assert(0);
        break;

    }                           /* endof switch */

    return (rc);
}

/*********************************************************************
 *
 * Function:    i2c_dev_write
 *
 * Description:    Motherboard I2C Write API.
 *
 * Inputs:    dev_p  - Pointer to device characteristics table.
 *        offset - I2C device offset.
 *        size   - Number of bytes to write.
 *        *buf   - Write buffer pointer.
 *
 * Outputs:    PASSED - No errors encounterd.
 *        E_I2C_INV_P   - Invalid slave address.
 *        E_I2C_CTL_ERR - I2C write error.
 *        E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
i2c_dev_write(n2g_i2c_dev_t * dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    int fd_i2c1 = get_i2c_fd(1);
    int fd_i2c2 = get_i2c_fd(2);

    if (dev_p->bus_no == CPU_I2C0) {
        if (fd_i2c0 > 0) {
            if ((rc = ioctl(fd_i2c0, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                       __FUNCTION__, __LINE__, dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c0;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if (fd_i2c1 > 0) {
            if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                       __FUNCTION__, __LINE__, dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c1;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C2) {
        if (fd_i2c2 > 0) {
            if ((rc = ioctl(fd_i2c2, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                printf("%s:%d:Unable to connect to device %#x. rc = %#x\n", 
                       __FUNCTION__, __LINE__, dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c2;
            }
        }
    }

    rc = api_mb_i2c_write(dev_p, offset, size, buf);

    return (rc);
}

/*********************************************************************
 *
 * Function:    get_n2g_i2c_states_table
 *
 * Description:    Get N2G I2C device table pointer.
 *
 * Inputs:    i2c_bus - N2G_I2C_BUS in n2g_i2c.h
 *        i2c_dev - MB_I2C_DEVICE in n2g_i2c.h.
 *
 * Outputs:    Pointer to the N2G I2C table of requested device.
 *        NULL if not a valid device.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static n2g_i2c_states_t *get_n2g_i2c_states_table(uint8_t i2c_bus,
                                                  uint8_t i2c_dev)
{
    int i = 0;
    switch (i2c_bus) {
    case CPU_I2C0:
        for (i = 0; i < MB_I2C_0_INVALID; i++) {
            if (i2c_mb0_state[i].i2c_dev->dev_addr == i2c_dev) {
                return (&i2c_mb0_state[i]);
            }
        }
        return (NULL);
        break;
    case CPU_I2C1:
        for (i = 0; i < MB_I2C_1_INVALID; i++) {
            if (i2c_mb1_state[i].i2c_dev->dev_addr == i2c_dev) {
                return (&i2c_mb1_state[i]);
            }
        }
        return (NULL);
        break;

    case CPU_I2C2:
        for (i = 0; i < MB_I2C_2_INVALID; i++) {
            if (i2c_mb2_state[i].i2c_dev->dev_addr == i2c_dev) {
                return (&i2c_mb2_state[i]);
            }
        }
        return (NULL);
        break;
    case PLUG_FPGA:
        if (i2c_dev >= PLUG_FPGA_INVALID) {
            /* Invalid I2C device */
            return (NULL);
        } else {
            return (&i2c_plug0_state[i2c_dev]);
        }
        break;
    default:
        /*
         * Invalid I2C bus number requested 
         */
        assert(!"i2c_api.c : states table is null\n");
        return (NULL);
        break;
    }                           /* endof i2c_bus */

    return (NULL);
}

/*-------------------------------------------------
 * $Log: diag_i2c_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

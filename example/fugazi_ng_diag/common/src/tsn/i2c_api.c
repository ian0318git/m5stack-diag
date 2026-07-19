/* $Id: i2c_api.c,v 1.5 2018/11/23 08:49:51 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/i2c_api.c,v $
 *------------------------------------------------------------------
 * Filename: i2c_api.c
 *
 * Description: Transformers (CPU) I2C API supports.
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "common.h"
#include "error.h"
#include "types.h"
#include "proto.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "dev_object.h"
#include "goofy_i2c.h"
#include "dev_at24c0n.h"        /* 256-byte EEPROM special handling */
#include "i2c_address.h"
#include "i2c_dev.h"
#include "nvmonvars.h"
#include "plug_host_fpga_lib.h"

/* Extern */
extern int get_i2c_fd(int);
extern unsigned char i2c_debug;

/*
 * Functional prototype
 */
static n2g_i2c_states_t *get_n2g_i2c_states_table(uint8_t i2c_bus,
                                                  uint8_t i2c_dev);

static uint32_t i2c_dev_read(n2g_i2c_dev_t *, ulong, uint8_t, char *);
static uint32_t i2c_dev_write(n2g_i2c_dev_t *, ulong, uint8_t, char *);
/*********************************************************************
 *              Global variables
 *********************************************************************
 */
/*
 * Holding shadow values of the 2 1:4 mux
 */

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
static n2g_i2c_dev_t n2g_i2c2_wifi_star_temp = { CPU_I2C2, WIFI_I2C_STAR_ADDR_TEMP, 1, 1, 0 };  /* Wifi Temp Sensor (MAX31730) */

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
    {0, &n2g_i2c2_wifi_star_temp, N2G_I2C_IDLE},   /* STAR Wifi Temp */
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
        if(i2c_debug) {
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
                cterr('f', 0,
                      "[%s] at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                      rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c0;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if (fd_i2c1 > 0) {
            if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                cterr('f', 0,
                      "[%s] at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                      rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c1;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C2) {
        if (fd_i2c2 > 0) {
            if ((rc = ioctl(fd_i2c2, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                cterr('f', 0,
                      "[%s] at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                      rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c2;
            }
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("i2c_api.c [%s]: line %d; dev_p->fp: %d\n", __FUNCTION__,
               __LINE__, dev_p->fp);
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
        if(i2c_debug) {
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
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__,
                      dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c0;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if (fd_i2c1 > 0) {
            if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__,
                      dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                dev_p->fp = fd_i2c1;
            }
        }
    } else if (dev_p->bus_no == CPU_I2C2) {
        if (fd_i2c2 > 0) {
            if ((rc = ioctl(fd_i2c2, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__,
                      dev_p->dev_addr, rc);
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
            return(NULL);
        } else {
            return(&i2c_plug0_state[i2c_dev]);
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

/*------------------------------------------------------------------
$Log: i2c_api.c,v $
Revision 1.5  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.4.36.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.4  2018/05/24 09:47:10  steja
CSCvj57981-Enhance SFP GLC-GE-100FX Support

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:03  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:05  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.4.6.6  2017/07/10 06:56:45  hondwang
add mcu upgrade function

Revision 1.1.4.4.6.5  2017/06/30 13:37:55  hondwang
Fix Star platform I2c scan issue and add this_is_star function

Revision 1.1.4.4.6.4  2017/06/20 03:35:15  shjung
Added pluggable LTE tests, utility and lib for GPIO expander

Revision 1.1.4.4.6.3  2017/06/16 06:52:37  tirawan
Foxconn Pluggable FPGA I2C Read/Write function correction during the bring up

Revision 1.1.4.4.6.2  2017/06/14 12:34:40  shjung
Create Pluggable LTE tests and utilities menu

Revision 1.1.4.4.6.1  2017/06/13 06:54:14  shjung
Add pluggable FPGA I2C read/write function

Revision 1.1.4.4.2.2  2017/07/18 06:10:36  steja
Code cleanup

Revision 1.1.4.4.2.1  2017/07/11 13:46:07  steja
1. Add Check Motherboard Aikido cookie ID
2. Add Check Aikido I2C Scan
2. Add Check SFP present I2C Scan

Revision 1.1.4.4  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.3  2016/07/14 12:56:59  steja
Add POE cookie eeprom programming

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.7  2016/06/21 04:36:33  palin2
Added voltage margin utility and MCU register R/W utilities.

Revision 1.1.2.6  2016/06/17 10:37:40  steja
Fix I2C scan

Revision 1.1.2.5  2016/05/24 01:18:11  palin2
Updated Thermal sensor and ACT2 chip I2C bus number based on P1A HW changes

Revision 1.1.2.4  2016/05/09 08:06:55  steja
Fixed POE i2c address R/W

Revision 1.1.2.3  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.2  2016/04/11 14:12:27  steja
Update code i2c utility for bringup

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility



$Endlog$
*/

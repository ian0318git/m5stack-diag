 /* $Id: i2c_api.c,v 1.2 2018/08/06 02:31:52 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/i2c_api.c,v $
 *------------------------------------------------------------------
 * Filename: i2c_api.c
 *
 * Description: I2C API supports.
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
#include "diag_i2c_lib.h"
#include "n2g_api_rc.h"
#include "cross_platform.h"
#include "dev_object.h"
#include "diag_i2c_addr.h"
#include "i2c_dev.h"
#include "nvmonvars.h"
#include "diag_fpga_i2c_lib.h"

/* Extern */
extern int get_i2c_fd(int);

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
/* DDR4 SPD */
static n2g_i2c_dev_t n2g_i2c0_ddr_spd = { CPU_I2C0, MB_I2C_ADDR_DIMM0, 1, 1, 0 };
/* Temp snsr */
static n2g_i2c_dev_t n2g_i2c0_temp_snsr = { CPU_I2C0, MB_I2C_ADDR_MB_TEMP_LM75, 1, 1, 0 };

/*********************************************************************
 *        I2C device state tables.
 *********************************************************************
 */
static n2g_i2c_states_t i2c_mb0_state[MB_I2C_0_INVALID] = {
    {0, &n2g_i2c0_ddr_spd, N2G_I2C_IDLE},        /* EEPROM 2K bit */
    {0, &n2g_i2c0_temp_snsr, N2G_I2C_IDLE},      /* Temp.Sensor */
};

/*********************************************************************
 *
 * Function:    i2c_err_str
 *
 * Description: function to display what kind of error of i2c
 *
 * Inputs:    num - number of i2c error index
 *
 * Outputs:   string of error message
 *
 * Assumptions:
 *
 *********************************************************************
 */
char *i2c_err_str(int num)
{
    if (num < RC_I2C_UNKNOWN)
        return i2c_err[num];
    return i2c_err[RC_I2C_UNKNOWN];
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
 *             E_I2C_INV_DEV - Invalid device address.
 *             E_I2C_NOT_LOCKED - Device not locked by any process.
 *             E_I2C_LOCKED - Device is locked by another process.
 *             E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *             Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_read(n2g_i2c_if_t * i2c_p)
{
    fpga_i2c_t i2c;
    uint32_t rc = 0;
    n2g_i2c_states_t *state_p;  /* pointer to the state struct */

    /*
     * Call the lower device driver 
     */
    switch (i2c_p->i2c_bus_type) {
    case CPU_I2C0:
    case CPU_I2C1:
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
    case IOFPGA_I2C:
        /* Call FPGA I2C Read */
        rc = fpga_i2c_rd(&i2c, i2c_p->mux, i2c_p->i2c_dev,
                         i2c_p->offset,
                         i2c_p->sub_addr_len,
                         i2c_p->size,
                         (unsigned char *) i2c_p->buf);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
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
 * Function:    n2g_i2c_write
 *
 * Description:    N2G Generic I2C Write API.
 *
 * Inputs:    i2c_p    - Pointer to the N2G I2C API interface struct. Fields
 *              needed in the struct are:
 *              i2c_bus_type, i2c_dev, offset, size, *buf.
 *
 * Outputs:    PASSED - No errors encounterd.
 *             E_I2C_INV_DEV - Invalid device address.
 *             E_I2C_NOT_LOCKED - Device not locked by any process.
 *             E_I2C_LOCKED - Device is locked by another process.
 *             E_I2C_MUX_BUSY - Other device on 1:4 Mux is busy.
 *             Other return codes are provided by the lower device driver.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t n2g_i2c_write(n2g_i2c_if_t * i2c_p)
{
    fpga_i2c_t i2c;
    n2g_i2c_states_t *state_p;  /* pointer to the state struct */
    uint rc;

    /*
     * Call the lower device driver 
     */
    switch (i2c_p->i2c_bus_type) {
    case CPU_I2C0:
    case CPU_I2C1:
        state_p =
            get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);

        rc = i2c_dev_write(state_p->i2c_dev, i2c_p->offset,
                           i2c_p->size, i2c_p->buf);
        break;
    case IOFPGA_I2C:
        /* Call FPGA I2C write */
        rc = fpga_i2c_wr(&i2c, i2c_p->mux, i2c_p->i2c_dev,
                         i2c_p->offset,
                         i2c_p->sub_addr_len,
                         i2c_p->size,
                         (unsigned char *)i2c_p->buf);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->i2c_siz 0x%X\n", i2c_p->size);
            printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
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
 * Function:    i2c_dev_read
 *
 * Description:    Motherboard I2C Read API.
 *
 * Inputs:    dev_p    - Pointer to device characteristics table.
 *            offset    - I2C device offset.
 *            size    - Number of bytes to read.
 *            *buf    - Read buffer pointer.
 *
 * Outputs:  PASSED - No errors encounterd.
 *           E_I2C_INV_P   - Invalid slave address.
 *           E_I2C_CTL_ERR - I2C controller error.
 *           E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t i2c_dev_read(n2g_i2c_dev_t * dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    int fd_i2c1 = get_i2c_fd(1);

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
 * Function:    i2c_dev_write
 *
 * Description:    Motherboard I2C Write API.
 *
 * Inputs:    dev_p  - Pointer to device characteristics table.
 *            offset - I2C device offset.
 *            size   - Number of bytes to write.
 *            *buf   - Write buffer pointer.
 *
 * Outputs:    PASSED - No errors encounterd.
 *             E_I2C_INV_P   - Invalid slave address.
 *             E_I2C_CTL_ERR - I2C write error.
 *             E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t i2c_dev_write(n2g_i2c_dev_t * dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    int fd_i2c1 = get_i2c_fd(1);

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
 *            i2c_dev - MB_I2C_DEVICE in n2g_i2c.h.
 *
 * Outputs:    Pointer to the N2G I2C table of requested device.
 *             NULL if not a valid device.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static n2g_i2c_states_t *get_n2g_i2c_states_table(uint8_t i2c_bus,
                                                  uint8_t i2c_dev)
{
    int ix;
    switch (i2c_bus) {
    case CPU_I2C0:
    case CPU_I2C1:
        for (ix = 0; ix < MB_I2C_0_INVALID; ix++) {
            if (i2c_mb0_state[ix].i2c_dev->dev_addr == i2c_dev) {
                return (&i2c_mb0_state[ix]);
            }
        }
        return (NULL);
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
Revision 1.2  2018/08/06 02:31:52  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.2  2018/03/28 07:55:52  lucywang
Changed Thermal sersor to LM75B, TBD : bug fix

Revision 1.1.2.1  2018/02/27 08:06:49  harrchan
Initial viper application code base




$Endlog$
*/

/* $Id: i2c_api.c
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/i2c_api.c,v $
 *------------------------------------------------------------------
 * Filename: i2c_api.c
 *
 * Description: (CPU) I2C API supports.
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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
#include "i2c_dev.h"
#include "nvmonvars.h"

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
//static n2g_i2c_dev_t n2g_i2c2_m2 = { CPU_I2C0, MB_I2C_ADDR_M2, 1, 1, 0 };     /* M2 connector */

/* CPU I2C controller 1 devices */
static n2g_i2c_dev_t n2g_i2c_sfp =  {MB_I2C_BUS_SFP, MB_I2C_ADDR_SFP, 0, 0, 0 };  /* SFP */
static n2g_i2c_dev_t n2g_i2c_act2 =  {MB_I2C_BUS_ACT2, MB_I2C_ADDR_ACT2, 0, 0, 0 };  /* Secure Chip */
static n2g_i2c_dev_t n2g_i2c_rtc =  {MB_I2C_BUS_RTC, MB_I2C_ADDR_RTC, 1, 1, 0 }; /* RTC DS1337S+ */
static n2g_i2c_dev_t n2g_i2c_tmp75 =  {MB_I2C_BUS_TMP75, MB_I2C_ADDR_TMP75, 1, 1, 0 }; /* MB Temp Sensor TMP76 */

/* CPU I2C controller 2 devices */
static n2g_i2c_dev_t n2g_i2c_cpld =  {MB_I2C_BUS_CPLD, MB_I2C_ADDR_CPLD, 1, 1, 0 }; /* CPLD */

/*********************************************************************
 *        I2C device state tables.
 *********************************************************************
 */
static n2g_i2c_states_t i2c_mb0_state[MB_I2C_0_INVALID] = {
    //{0, &n2g_i2c2_m2, N2G_I2C_IDLE},   /* M2 Connector */
};

static n2g_i2c_states_t i2c_mb1_state[MB_I2C_1_INVALID] = {
    {0, &n2g_i2c_sfp, N2G_I2C_IDLE},		/* SFP */ 
    {0, &n2g_i2c_act2, N2G_I2C_IDLE},		/* MB ACT2*/
    {0, &n2g_i2c_rtc, N2G_I2C_IDLE},		/* MB RTC DS1337 */
    {0, &n2g_i2c_tmp75, N2G_I2C_IDLE},     /* MB Temp Sensor */
};

static n2g_i2c_states_t i2c_mb2_state[MB_I2C_2_INVALID] = {
    {0, &n2g_i2c_cpld, N2G_I2C_IDLE},
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
    n2g_i2c_states_t *state_p;  /* pointer to the state struct */

    /*
     * Call the lower device driver 
     */
    if(i2c_debug) {
        printf("rc : %d\n", rc);

        printf("i2c_if_p->offset: 0x%X\n", i2c_p->offset);
        printf("i2c_if_p->i2c_bus_type: %d\n", i2c_p->i2c_bus_type);
        printf("i2c_if_p->i2c_dev: 0x%X\n", i2c_p->i2c_dev);
        printf("i2c_if_p->i2c_ctrl: %d\n", i2c_p->i2c_ctrl);
        printf("i2c_if_p->sub_addr_len: %d\n", i2c_p->sub_addr_len);
        printf("i2c_if_p->size: %d\n", i2c_p->size);
        printf("i2c_if_p->rd_hd_size: %d\n", i2c_p->rd_hd_size);
        printf("i2c_if_p->wr_hd_size: %d\n", i2c_p->wr_hd_size);
        printf("i2c_if_p->mux: %d\n", i2c_p->mux);
        printf("i2c_if_p->err_no: %d\n", i2c_p->err_no);
        printf("i2c_if_p->i2c_speed: %d\n", i2c_p->i2c_speed);
        printf("i2c_if_p->buf 0x%x\n", *(i2c_p->buf));
        printf("i2c_if_p->i2c_base 0x%lx\n", i2c_p->i2c_base);
    }

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

    if(i2c_debug) {
        printf("rc : %d\n", rc);

        printf("n2g_i2c_dev_t->bus_no: %d\n", (state_p->i2c_dev)->bus_no);
        printf("n2g_i2c_dev_t->dev_addr: 0x%x\n", (state_p->i2c_dev)->dev_addr);
        printf("n2g_i2c_dev_t->size: %d\n", (state_p->i2c_dev)->wr_hd_size);
        printf("n2g_i2c_dev_t->size: %d\n", (state_p->i2c_dev)->rd_hd_size);
        printf("n2g_i2c_dev_t->fp: %d\n", (state_p->i2c_dev)->fp);
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
    int fd_i2c0, fd_i2c1, fd_i2c2;
    if (dev_p->bus_no == CPU_I2C0) {
        fd_i2c0 = get_i2c_fd(0);
        if ((fd_i2c0 < 0)) {
            cterr('f', 0, "[%s]:%d: error fd_i2c0:%d\n" , __FUNCTION__, __LINE__, fd_i2c0);
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        fd_i2c1 = get_i2c_fd(1);
        if ((fd_i2c1 < 0)) {
            cterr('f', 0, "[%s]:%d: error fd_i2c1:%d\n" , __FUNCTION__, __LINE__, fd_i2c1);
        }
    } else if (dev_p->bus_no == CPU_I2C2) {
        fd_i2c2 = get_i2c_fd(2);
        if ((fd_i2c2 < 0)) {
            cterr('f', 0, "[%s]:%d: error fd_i2c2:%d\n" , __FUNCTION__, __LINE__, fd_i2c2);
        }
    } else {
        cterr('f', 0, "Not support this bus %d" , dev_p->bus_no);
    }
  
    if (dev_p->bus_no == CPU_I2C0) {
        if ((rc = ioctl(fd_i2c0, I2C_SLAVE, dev_p->dev_addr)) < 0) {
            cterr('f', 0,
                    "[%s] at %s: unable to connect to device %#x. "
                    "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                    rc);
            return (FAILED);
        } else {
            dev_p->fp = fd_i2c0;
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
            cterr('f', 0,
                    "[%s] at %s: unable to connect to device %#x. "
                    "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                    rc);
            return (FAILED);
        } else {
            dev_p->fp = fd_i2c1;
        }
    } else if (dev_p->bus_no == CPU_I2C2) {
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
    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("i2c_api.c [%s]: line %d; dev_p->fp: %d\n",
                __FUNCTION__, __LINE__, dev_p->fp);
        printf("bus_no: %d, dev_addr: %#x, rd_hd_size %d, size:%d \n\n", 
                dev_p->bus_no, dev_p->dev_addr, dev_p->rd_hd_size, size);
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
        if(i2c_debug) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->sub_addr_len %d\n", i2c_p->sub_addr_len);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->buf 0x%X\n\n", *(i2c_p->buf));
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

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("i2c_api.c [%s]: line %d; dev_p->fp: %d\n",
                __FUNCTION__, __LINE__, dev_p->fp);
        printf("bus_no: %d, dev_addr: %#x, rd_hd_size %d \n\n", 
                dev_p->bus_no, dev_p->dev_addr, dev_p->rd_hd_size);
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
Revision 1.2  2021/06/02 02:56:21  alpeng
merge sears into trunk

Revision 1.1.4.1  2021/03/04 09:46:23  leschen
Remove unused I2c node /dev/i2c-0

Revision 1.1  2020/08/19 09:50:05  markzha
*** empty log message ***


$Endlog$
*/

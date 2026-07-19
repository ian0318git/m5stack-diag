/* $Id: platform_i2c_api.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_i2c_api.c,v $
 *------------------------------------------------------------------
 * Filename: katar_i2c_api.c
 *
 * Description: Transformers (CPU) I2C API supports.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "platform_aikido.h"
#include "diag_fpga_i2c_lib.h"

/*
 * Functional prototype
 */
static n2g_i2c_states_t *katar_get_n2g_i2c_states_table(uint8_t i2c_bus, uint8_t i2c_dev);
static uint32_t katar_i2c_dev_read(n2g_i2c_dev_t *, ulong, uint8_t, char *);
static uint32_t katar_i2c_dev_write(n2g_i2c_dev_t *, ulong, uint8_t, char *buf);

/*********************************************************************
 *        I2C devices characteristics tables.
 *********************************************************************
 */
/* CPU I2C controller 0 devices */
static n2g_i2c_dev_t n2g_i2c0_dimm1 = { CPU_I2C0, MB_I2C_ADDR_DIMM1, 1, 1, 0 };  /* DIMM1 */
static n2g_i2c_dev_t n2g_i2c0_dimm2 = { CPU_I2C0, MB_I2C_ADDR_DIMM2, 1, 1, 0 };  /* DIMM2 */
static n2g_i2c_dev_t n2g_i2c0_sfp_switch = { CPU_I2C0, MB_I2C_ADDR_SFP_SWITCH, 1, 1, 0 };  /* SFP SWITCH */
static n2g_i2c_dev_t n2g_i2c0_sfp_spd = { CPU_I2C0, MB_I2C_ADDR_SFP_SPD, 1, 1, 0 };        /* SFP SPD */
static n2g_i2c_dev_t n2g_i2c0_temp1 = { CPU_I2C0, MB_I2C_ADDR_TEMP1, 1, 1, 0 };            /* Temp Sensor #1 */
static n2g_i2c_dev_t n2g_i2c0_temp2 = { CPU_I2C0, MB_I2C_ADDR_TEMP2, 1, 1, 0 };            /* Temp Sensor #2 */
static n2g_i2c_dev_t n2g_i2c0_poe = { CPU_I2C0, MB_I2C_ADDR_POE, 1, 1, 0 };                /* POE */

/* CPU I2C controller 1 devices */
//static n2g_i2c_dev_t n2g_i2c1_sfp_switch = { CPU_I2C1, MB_I2C_ADDR_SFP_SWITCH, 1, 1, 0 };  /* SFP SWITCH */
//static n2g_i2c_dev_t n2g_i2c1_sfp_spd = { CPU_I2C1, MB_I2C_ADDR_SFP_SPD, 1, 1, 0 };        /* SFP SPD */
//static n2g_i2c_dev_t n2g_i2c1_temp1 = { CPU_I2C1, MB_I2C_ADDR_TEMP1, 1, 1, 0 };            /* Temp Sensor #1 */
//static n2g_i2c_dev_t n2g_i2c1_temp2 = { CPU_I2C1, MB_I2C_ADDR_TEMP2, 1, 1, 0 };            /* Temp Sensor #2 */
//static n2g_i2c_dev_t n2g_i2c1_poe = { CPU_I2C1, MB_I2C_ADDR_POE, 1, 1, 0 };                /* POE */



extern int fpga_i2c_rd(fpga_i2c_t *, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);
extern int fpga_i2c_wr(fpga_i2c_t *, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);



/*********************************************************************
 *        I2C device state tables.
 *********************************************************************
 */
static n2g_i2c_states_t i2c_mb0_state[MB_I2C_0_INVALID] = { 
    {0, &n2g_i2c0_dimm1, N2G_I2C_IDLE},         /* DIMM1 */
    {0, &n2g_i2c0_dimm2, N2G_I2C_IDLE},         /* DIMM2 */
    {0, &n2g_i2c0_sfp_switch, N2G_I2C_IDLE},    /* SFP Switch */
    {0, &n2g_i2c0_sfp_spd, N2G_I2C_IDLE},       /* SFP SPD */
    {0, &n2g_i2c0_temp1, N2G_I2C_IDLE},         /* Temp Sensor #1 */
    {0, &n2g_i2c0_temp2, N2G_I2C_IDLE},         /* Temp Sensor #2 */
    {0, &n2g_i2c0_poe, N2G_I2C_IDLE},           /* POE */
};

static n2g_i2c_states_t i2c_mb1_state[MB_I2C_1_INVALID] = {
//    {0, &n2g_i2c1_sfp_switch, N2G_I2C_IDLE},    /* SFP Switch */
//    {0, &n2g_i2c1_sfp_spd, N2G_I2C_IDLE},       /* SFP SPD */
//    {0, &n2g_i2c1_temp1, N2G_I2C_IDLE},         /* Temp Sensor #1 */
//    {0, &n2g_i2c1_temp2, N2G_I2C_IDLE},         /* Temp Sensor #2 */
//    {0, &n2g_i2c1_poe, N2G_I2C_IDLE},           /* POE */
};



/*********************************************************************
 *
 * Function:    katar_n2g_i2c_read
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
uint32_t katar_n2g_i2c_read(n2g_i2c_if_t * i2c_p)
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
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
        } 
        state_p = katar_get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
        } 
        if (state_p != NULL) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            } 
            rc = katar_i2c_dev_read(state_p->i2c_dev,
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
 * Function:    katar_get_n2g_i2c_states_table
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
static n2g_i2c_states_t *katar_get_n2g_i2c_states_table(uint8_t i2c_bus,
                                                  uint8_t i2c_dev)
{
    int i = 0;
    switch (i2c_bus) {
    case CPU_I2C0:
        for (i = 0; i < MB_I2C_0_INVALID; i++) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            } 
            if (i2c_mb0_state[i].i2c_dev->dev_addr == i2c_dev) {
                return (&i2c_mb0_state[i]);
            }
        }
        return (NULL);
        break;  
    case CPU_I2C1:
        for (i = 0; i < MB_I2C_1_INVALID; i++) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            } 
            if (i2c_mb1_state[i].i2c_dev->dev_addr == i2c_dev) {
                return (&i2c_mb1_state[i]);
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


/*********************************************************************
 *
 * Function:    katar_i2c_dev_read
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
extern int get_i2c_fd(int);
uint32_t katar_i2c_dev_read(n2g_i2c_dev_t * dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    int fd_i2c1 = get_i2c_fd(1);

    if (dev_p->bus_no == CPU_I2C0) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
        } 
        if (fd_i2c0 > 0) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            } 
            if ((rc = ioctl(fd_i2c0, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                } 
                cterr('f', 0,
                      "[%s] at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                      rc);
                return (FAILED);
            } else {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                } 
                dev_p->fp = fd_i2c0;
            }
        }
        else {
            printf("No %s found.", I2CBUS0);
            return (FAILED);
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
        } 
        if (fd_i2c1 > 0) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            } 
            if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                } 
                cterr('f', 0,
                      "[%s] at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__, dev_p->dev_addr,
                      rc);
                return (FAILED);
            } else {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                } 
                dev_p->fp = fd_i2c1;
            }
        }
        else {
            printf("No %s found.", I2CBUS1);
            return (FAILED);
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("i2c_api.c [%s]: line %d; dev_p->fp: %d\n", __FUNCTION__,
               __LINE__, dev_p->fp);
        printf("dev_addr: %#x, rd_hd_size %d \n\n", dev_p->dev_addr,
               dev_p->rd_hd_size);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
    } 
    rc = api_mb_i2c_read(dev_p, offset, size, buf);

    return (rc);
}


/*********************************************************************
 *
 * Function:    katar_n2g_i2c_write
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
uint32_t katar_n2g_i2c_write(n2g_i2c_if_t * i2c_p)
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
            katar_get_n2g_i2c_states_table(i2c_p->i2c_bus_type, i2c_p->i2c_dev);
        /*  
         * Southbridge 
         */
        rc = katar_i2c_dev_write(state_p->i2c_dev, i2c_p->offset,
                           i2c_p->size, i2c_p->buf);
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->sub_addr_len %d\n", i2c_p->sub_addr_len);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->buf 0x%X\n\n", *i2c_p->buf);
        }
        break;
    case IOFPGA_I2C:
        /* Call FPGA I2C write */
        rc = fpga_i2c_wr(&i2c, i2c_p->mux, i2c_p->i2c_dev,
                         i2c_p->offset,
                         i2c_p->sub_addr_len,
                         i2c_p->size,
                         (unsigned char *)i2c_p->buf);

        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("i2c_if_p->i2c_ctrl %d (%s())\n", i2c_p->i2c_ctrl, __FUNCTION__);
            printf("i2c_if_p->mux %d\n", i2c_p->mux);
            printf("i2c_if_p->i2c_dev 0x%X\n", i2c_p->i2c_dev);
            printf("i2c_if_p->i2c_offset 0x%X\n", i2c_p->offset);
            printf("i2c_if_p->i2c_siz 0x%X\n", i2c_p->size);
            printf("i2c_if_p->buf 0x%X (%s())\n", *i2c_p->buf, __FUNCTION__);
        }   
        break;
    default:
        printf("not suported katar_i2c_api.c katar_n2g_i2c_write: %d line %d\n",
               i2c_p->i2c_bus_type, __LINE__);
        assert(0);
        break;

    }                           /* endof switch */

    return (rc);
}


/*********************************************************************
 *
 * Function:    katar_i2c_dev_write
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
uint32_t katar_i2c_dev_write(n2g_i2c_dev_t * dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    int fd_i2c1 = get_i2c_fd(1);

    if (dev_p->bus_no == CPU_I2C0) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
        }   
        if (fd_i2c0 > 0) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            }
            if ((rc = ioctl(fd_i2c0, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                }
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__,
                      dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                }
                dev_p->fp = fd_i2c0;
            }
        }
        else {
            printf("No %s found.", I2CBUS0);
            return (FAILED);
        }
    } else if (dev_p->bus_no == CPU_I2C1) {
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
        }   
        if (fd_i2c1 > 0) {
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
            }   
            if ((rc = ioctl(fd_i2c1, I2C_SLAVE, dev_p->dev_addr)) < 0) {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                }   
                cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                      "rc = %#x", __FUNCTION__, __FILE__,
                      dev_p->dev_addr, rc);
                return (FAILED);
            } else {
                if (diagflag_xram & D_DEBUG_OPTIONS) {
                    printf("===== %s %d =====\n",__FUNCTION__,__LINE__);
                }   
                dev_p->fp = fd_i2c1;
            }
        }
        else {
            printf("No %s found.", I2CBUS1);
            return (FAILED);
        }
    }

    rc = api_mb_i2c_write(dev_p, offset, size, buf);

    return (rc);
}




/*
 *------------------------------------------------------------------
 * $Log: platform_i2c_api.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.3  2019/03/13 03:34:15  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.2  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.4  2018/12/27 00:42:24  peteteng
 * Support Aikido thru UserLogic FPGA I2C
 *
 * Revision 1.1.2.3  2018/12/20 09:10:57  peteteng
 * Add FPGA I2C read/write/scan/reset util
 *
 * Revision 1.1.2.2  2018/11/08 02:21:52  peteteng
 * Remove names in comment
 *
 * Revision 1.1.2.1  2018/10/22 08:02:25  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.4  2018/09/28 03:09:18  peteteng
 * Fix SFP SPD issue
 *
 * Revision 1.1.2.3  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.2  2018/08/03 01:33:24  peteteng
 * Add SFP cookie - SFP select
 *
 * Revision 1.1.2.1  2018/07/24 09:54:12  peteteng
 * Add SFP cookie - read
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


/* $Id: skye_i2c_api.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_i2c_api.c,v $
 *------------------------------------------------------------------------------
 * skye_i2c_api.c - Main file of Skye I2C related APIs.
 *
 * July 09 2013, Paul Lin(palin2) created for ShrinkRay.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "common.h"
#include "common_utils.h"
#include "queryflags.h"
#include "types.h"
#include "skye_i2c.h"
#include "nvmonvars.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
int skye_i2c_read(int, uint16_t, int, uint16_t, uint16_t, uchar*);
int skye_i2c_write(int, uint16_t, int, uint16_t, uint16_t, uchar*);
int skye_dimm_spd_read(int, uint16_t, uint16_t, uchar*);
int skye_dimm_spd_write(int, uint16_t, uint16_t, uchar*);
int skye_fpga_i2c_read(uint16_t, uint16_t, uchar*);
int skye_fpga_i2c_write(uint16_t, uint16_t, uchar*);
int skye_bib_rd_util(void);
int skye_bib_dump_util(void);


/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int skye_i2c_mux_ctrl_reg_wr(uchar *);
extern boolean cpu_id;
extern void * malloc(unsigned long nbytes);
extern void free(void *buf);
extern boolean check_cpu(int);


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/


/*******************************************************************************
 *
 * Function   : skye_i2c_read
 * Description:	Function to read data from I2C devices via Tilera I2C interface.
 * Inputs     : fd - File Descriptor of I2C controller
 *              dev_addr - Target I2C device address
 *              addr_size - Address size of target I2C device
 *              offset - Register offset that wants to read
 *              nbytes - Total size that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_read (int fd, uint16_t dev_addr, int addr_size,
                    uint16_t offset, uint16_t nbytes, uchar* buf)
{
    while (nbytes) {
        unsigned long bytesthispass = nbytes;
        uchar addr[2];

        if (addr_size == 1) {
            addr[0] = offset;
        } else if (addr_size == 2) {
            addr[0] = offset >> 8;
            addr[1] = offset & 0xFF;
        }

        struct i2c_msg msgs[2] = {
            {
                .addr = dev_addr,
                .flags = 0,
                .len = addr_size,
                .buf = addr,
            },
            {
                .addr = dev_addr,
                .flags = I2C_M_RD,
                .len = bytesthispass,
                .buf = buf,
            },
        };

        struct i2c_rdwr_ioctl_data io = {
            .msgs = (addr_size == 0) ? &msgs[1] : msgs,
            .nmsgs = (addr_size == 0) ? 1 : 2,
        };

        if (ioctl(fd, I2C_RDWR, &io) < 0) {
            printf("%s: Failed to do I2C IOCTL read.\n", __FUNCTION__);
            return (FAILED);
        }
        nbytes -= bytesthispass;
        offset += bytesthispass;
        addr_size = 0;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_i2c_write
 * Description:	Function to write data to I2C devices via Tilera I2C interface.
 * Inputs     : fd - File Descriptor of I2C controller
 *              dev_addr - Target I2C device address
 *              addr_size - Address size of target I2C device
 *              offset - Register offset that wants to read
 *              nbytes - Total size that wants to read
 *              wdata - Buffer to put the data want to write-in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_write (int fd, uint16_t dev_addr, int addr_size,
                     uint16_t offset, uint16_t nbytes, uchar* wdata)
{
    uchar buf[1026];

    memset(buf, 0, sizeof(buf));

    while (nbytes) {
        unsigned long bytesthispass = nbytes;

        if (bytesthispass > sizeof (buf) - addr_size) {
            bytesthispass = sizeof (buf) - addr_size;
        }

        strcpy((char *)&buf[addr_size], (char *)wdata);

        if (addr_size == 1) {
            buf[0] = offset;
        } else if (addr_size == 2) {
            buf[0] = offset >> 8;
            buf[1] = offset & 0xFF;
        } else {
            printf("%s: Unsupported Addr. Size(%d).\n",
                   __FUNCTION__, addr_size);
            return (FAILED);
        }

        struct i2c_msg msgs[2] = {
            {
                .addr = dev_addr,
                .flags = 0,
                .len = bytesthispass + addr_size,
                .buf = buf,
            },
        };

        struct i2c_rdwr_ioctl_data io = {
            .msgs = msgs,
            .nmsgs = 1,
        };

        if (ioctl(fd, I2C_RDWR, &io) < 0) {
            printf("%s: Failed to do I2C IOCTL write.\n", __FUNCTION__);
            return (FAILED);
        }

        nbytes -= bytesthispass;
        offset += bytesthispass;
        addr_size = 0;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_dimm_spd_read
 * Description:	Wrapper function to dump DDR DIMM SPD info.
 * Inputs     :	ch - number of DDR DIMM channel
 *              offset - Register offset that wants to read
 *              len - Total size(in bytes) that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_dimm_spd_read (int ch, uint16_t offset, uint16_t len, uchar* buf)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];

    memset(devname, 0, sizeof(devname));

    /* Get I2C address of DIMM SPD by their channel */
    switch (ch) {
    case 0:
        dev_addr = SR_DIMM0_SPD_I2C_ADDR;
    break;
    case 1:
        dev_addr = SR_DIMM1_SPD_I2C_ADDR;
    break;
    default:
        printf("Unknown DIMM channel number(%d).\n", ch);
        return (FAILED);
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM1);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM1);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, SR_DIMM_ADDR_SZ,
                           offset, len, buf) != PASSED) {
        printf("%s: Failed to read data from I2C%d-DIMM%d SPD(Addr: 0x%02X).\n",
               __FUNCTION__, SR_CPU_I2CM1, ch, dev_addr);
        close(fd);
        return (FAILED);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_dimm_spd_write
 * Description:	Wrapper function to write DDR DIMM SPD info.
 * Inputs     :	ch - number of DDR DIMM channel
 *              offset - Register offset that wants to read
 *              len - Total size(in bytes) that wants to read
 *              wdata - Buffer to put data for write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_dimm_spd_write (int ch, uint16_t offset, uint16_t len, uchar* wdata)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];

    memset(devname, 0, sizeof(devname));

    /* Get I2C address of DIMM SPD by their channel */
    switch (ch) {
    case 0:
        dev_addr = SR_DIMM0_SPD_I2C_ADDR;
    break;
    case 1:
        dev_addr = SR_DIMM1_SPD_I2C_ADDR;
    break;
    default:
        printf("Unknown DIMM channel number(%d).\n", ch);
        return (FAILED);
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM1);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM1);
        return (FAILED);
    }

    if (skye_i2c_write(fd, dev_addr, SR_DIMM_ADDR_SZ,
                            offset, len, wdata) != PASSED) {
        printf("%s: Failed to read data from I2C%d-DIMM%d SPD(Addr: 0x%02X).\n",
               __FUNCTION__, SR_CPU_I2CM1, ch, dev_addr);
        close(fd);
        return (FAILED);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_dimm_thermal_rd
 * Description:	Wrapper function to dump DDR DIMM SPD info.
 * Inputs     :	ch - number of DDR DIMM channel
 *              offset - Register offset that wants to read
 *              *buf - buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_dimm_thermal_rd (int ch, uint16_t offset, uint16_t *buf)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      rd_buf[2];

    memset(devname, 0, sizeof(devname));
    memset(rd_buf, 0, sizeof(rd_buf));

    /* Get I2C address of DIMM thermal sensor by channel */
    switch (ch) {
    case 0:
        dev_addr = SR_DIMM0_TS_I2C_ADDR;
    break;
    case 1:
        dev_addr = SR_DIMM1_TS_I2C_ADDR;
    break;
    default:
        printf("Unknown DIMM channel number(%d).\n", ch);
        return (FAILED);
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM1);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM1);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, SR_THERMAL_ADDR_SZ,
                           offset, SR_DIMM_TS_REG_SZ, (uchar *)rd_buf) != PASSED) {
        printf("%s: Failed to read data from I2C%d-DIMM%d Thermal Sensor"
               "(Addr: 0x%02X).\n", __FUNCTION__, SR_CPU_I2CM1, ch, dev_addr);

        close(fd);
        return (FAILED);
    }

    /* Swap Data */
    *buf = (uint16_t)(((uint16_t)(rd_buf[0] << 8)) | rd_buf[1]);

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_dimm_thermal_wr
 * Description:	Wrapper function to write DDR DIMM thermal sensor register.
 * Inputs     :	ch - number of DDR DIMM channel
 *              offset - Register offset that wants to write
 *              wdata - Buffer to put data for write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_dimm_thermal_wr (int ch, uint16_t offset, uint16_t *wdata)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      buf[2];

    memset(devname, 0, sizeof(devname));
    memset(buf, 0, sizeof(buf));

    /* Get I2C address of DIMM thermal sensor by channel */
    switch (ch) {
    case 0:
        dev_addr = SR_DIMM0_TS_I2C_ADDR;
    break;
    case 1:
        dev_addr = SR_DIMM1_TS_I2C_ADDR;
    break;
    default:
        printf("Unknown DIMM channel number(%d).\n", ch);
        return (FAILED);
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM1);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM1);
        return (FAILED);
    }

    /* Swap Data */
    buf[0] = (uchar)((uint16_t)(*wdata & 0xFF00) >> 8); 
    buf[1] = (uchar)(*wdata & 0x00FF);

    if (skye_i2c_write(fd, dev_addr, SR_THERMAL_ADDR_SZ,
                            offset, SR_DIMM_TS_REG_SZ, buf) != PASSED) {
        printf("%s: Failed to read data from I2C%d-DIMM%d Thermal Sensor"
               "(Addr: 0x%02X).\n", __FUNCTION__, SR_CPU_I2CM1, ch, dev_addr);

        close(fd);
        return (FAILED);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_on_board_thermal_rd
 * Description:	Wrapper function to dump on board thermal info.
 * Inputs     : offset - Register offset that wants to read
 *              *buf - buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_on_board_thermal_rd (uint16_t offset, uchar *buf)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      rd_buf[2];
    uchar      mux_data = PCA9546A_I2C_CH1;

    memset(devname, 0, sizeof(devname));
    memset(rd_buf, 0, sizeof(rd_buf));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 1 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }

    dev_addr = SR_THERMAL_I2C_ADDR;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, SR_THERMAL_ADDR_SZ,
                           offset, ONE_B_REG, (uchar *)rd_buf) != PASSED) {
        printf("%s: Failed to read data from I2C%d On Board Thermal Sensor"
               "(Addr: 0x%02X).\n", __FUNCTION__, SR_CPU_I2CM1, dev_addr);

        close(fd);
        return (FAILED);
    }

    /* Swap Data */
    *buf = rd_buf[0];

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_on_board_thermal_wr
 * Description:	Wrapper function to write on board thermal sensor register.
 * Inputs     : offset - Register offset that wants to write
 *              *w_data - Buffer to put data for write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_on_board_thermal_wr (uint16_t offset, uchar *w_data)
{
    int        fd = -1, bus_no = 0, addr_size = 0;
    uint16_t   wlen = 0, dev_addr = 0;
    char       devname[32], msg[256];
    uchar      wdata[256];
    uchar      mux_data = PCA9546A_I2C_CH1;

    memset(devname, 0, sizeof(devname));
    memset(wdata, 0, sizeof(wdata));
    memset(msg, 0, sizeof(msg));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 1 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }

    bus_no = SR_CPU_I2CM2;
    dev_addr = SR_THERMAL_I2C_ADDR;
    addr_size = SR_THERMAL_ADDR_SZ;
    wlen = ONE_B_REG;
    wdata[0] = *w_data;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    if (skye_i2c_write(fd, dev_addr, addr_size,
                            offset, wlen, wdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_current_rd
 * Description:	Wrapper function to dump current sensor info.
 * Inputs     : offset - Register offset that wants to read
 *              *buf - buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_current_rd (uint16_t offset, uint16_t *buf)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      rd_buf[2];
    uchar      mux_data = PCA9546A_I2C_CH3;

    memset(devname, 0, sizeof(devname));
    memset(rd_buf, 0, sizeof(rd_buf));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 1 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }
    dev_addr = SR_CUR_SENSOR_I2C_ADDR;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, SR_CURRENT_ADDR_SZ,
                           offset, TWO_B_REG, (uchar *)rd_buf) != PASSED) {
        printf("%s: Failed to read data from I2C%d On Board Thermal Sensor"
               "(Addr: 0x%02X).\n", __FUNCTION__, SR_CPU_I2CM1, dev_addr);

        close(fd);
        return (FAILED);
    }
    /* Swap Data */
    *buf = (uint16_t)(((uint16_t)(rd_buf[0] << 8)) | rd_buf[1]);

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_current_wr
 * Description:	Wrapper function to write current sensor register.
 * Inputs     : offset - Register offset that wants to write
 *              *w_data - Buffer to put data for write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_current_wr (uint16_t offset, uint16_t *w_data)
{
    int        fd = -1, bus_no = 0, addr_size = 0;
    uint16_t   wlen = 0, dev_addr = 0;
    char       devname[32], msg[256];
    uchar      wdata[256];
    uchar      mux_data = PCA9546A_I2C_CH3;

    memset(devname, 0, sizeof(devname));
    memset(wdata, 0, sizeof(wdata));
    memset(msg, 0, sizeof(msg));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 1 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }
    bus_no = SR_CPU_I2CM2;
    dev_addr = SR_CUR_SENSOR_I2C_ADDR;
    addr_size = SR_CURRENT_ADDR_SZ;
    wlen = TWO_B_REG;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    wdata[0] = (uchar)((uint16_t)(*w_data & 0xFF00) >> 8); 
    wdata[1] = (uchar)(*w_data & 0x00FF);
    if (skye_i2c_write(fd, dev_addr, addr_size,
                            offset, wlen, wdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_fpga_i2c_read
 * Description:	Wrapper function to Read Skye FPGA(Szalinski) register
 *              through Tilera CPU I2CM2.
 * Inputs     :	offset - Register offset that wants to read
 *              len - Total size(in bytes) that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_fpga_i2c_read (uint16_t offset, uint16_t len, uchar* buf)
{
    int        fd = -1, ctr = 0;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      rd_data = 0, mux_data = PCA9546A_I2C_CH2;

    memset(devname, 0, sizeof(devname));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 2 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }

    if (cpu_id == MASTER_CPU) {
        dev_addr = SR_CPU0_FPGA_I2C_ADDR;
    } else {
        dev_addr = SR_CPU1_FPGA_I2C_ADDR;
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    for (ctr = 0; ctr < len; ctr++, buf++) {
        if (skye_i2c_read(fd, dev_addr, SR_FPGA_ADDR_SZ,
                               (uint16_t)(offset + ctr), ONE_B_REG,
                               (uchar *)&rd_data) != PASSED) {
            printf("%s: Failed to read data from FPGA Szalinski"
                   " (I2C%d, Addr = 0x%02X).\n",
                   __FUNCTION__, SR_CPU_I2CM2, dev_addr);
            close(fd);
            return (FAILED);
        }
        *buf = rd_data;
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_fpga_i2c_write
 * Description:	Wrapper function to Write Skye FPGA(Szalinski) register
 *              through Tilera CPU I2CM2.
 * Inputs     :	offset - Register offset that wants to write
 *              len - Total size(in bytes) that wants to write
 *              wdata - Data want to write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_fpga_i2c_write (uint16_t offset, uint16_t len, uchar* wdata)
{
    int        fd = -1;
    uint16_t   dev_addr = 0, ctr = 0;
    char       devname[32];
    uchar      w_data = 0, mux_data = PCA9546A_I2C_CH2;

    memset(devname, 0, sizeof(devname));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 2 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }

    if (cpu_id == MASTER_CPU) {
        dev_addr = SR_CPU0_FPGA_I2C_ADDR;
    } else {
        dev_addr = SR_CPU1_FPGA_I2C_ADDR;
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    for (ctr = 0; ctr < len; ctr++, wdata++) {
        w_data = *wdata;
        if (skye_i2c_write(fd, dev_addr, SR_FPGA_ADDR_SZ,
                                (uint16_t)(offset + ctr), ONE_B_REG,
                                (uchar *)&w_data) != PASSED) {
            printf("%s: Failed to write data to FPGA Szalinski"
                   " (I2C%d, Addr = 0x%02X).\n",
                   __FUNCTION__, SR_CPU_I2CM2, dev_addr);
            close(fd);
            return (FAILED);
        }
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_ps_i2c_rd
 * Description:	Wrapper function to Read Power Sequencer register
 *              through Tilera CPU I2CM2.
 * Inputs     :	s_offset - Starting register offset that wants to read
 *              len - Total size(in bytes) that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
plat_ps_i2c_rd (uint16_t s_offset, uint16_t len, uchar* buf)
{
    int        fd = -1, addr_sz = 0, dev_i2c_bus = 0;
    uint16_t   dev_addr = 0, reg_offset = 0, ctr = 0;
    char       devname[32];
    uchar      reg_val = 0;

    /* Setup common parameters to access I2C. */
    dev_addr    = (uint16_t)SR_PWR_SEQ_I2C_ADDR;
    addr_sz     = (int)SR_PWR_SEQ_ADDR_SZ;
    dev_i2c_bus = (int)SR_CPU_I2CM2;
    reg_offset  = s_offset;

    memset(devname, 0, sizeof(devname));
    snprintf(devname, sizeof(devname), "/dev/i2c-%d", dev_i2c_bus);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    for (ctr = 0; ctr < len; ctr++, reg_offset++, buf++) {
        reg_val = 0;

        if (skye_i2c_read(fd, dev_addr, addr_sz, reg_offset,
                               (uint16_t)ONE_B_REG, &reg_val) != PASSED) {
            printf("%s: Failed to read register 0x%02X from Power Sequencer"
                   " (I2C%d, Addr = 0x%02X).\n",
                   __FUNCTION__, reg_offset, dev_i2c_bus, dev_addr);
            close(fd);
            return (FAILED);
        }

        *buf = reg_val;
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	plat_ps_i2c_wr
 * Description:	Wrapper function to Write Power Sequencer register(s)
 *              through Tilera CPU I2CM2.
 * Inputs     :	s_offset - Starting register offset that wants to write
 *              len - Total size(in bytes) that wants to write
 *              wdata - Data want to write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
plat_ps_i2c_wr (uint16_t s_offset, uint16_t len, uchar* wdata)
{
    int        fd = -1, addr_sz = 0, dev_i2c_bus = 0;
    uint16_t   dev_addr = 0, ctr = 0, reg_offset = 0;
    char       devname[32];
    uchar      w_val = 0;

    dev_addr    = (uint16_t)SR_PWR_SEQ_I2C_ADDR;
    addr_sz     = (int)SR_PWR_SEQ_ADDR_SZ;
    dev_i2c_bus = (int)SR_CPU_I2CM2;
    reg_offset  = s_offset;

    memset(devname, 0, sizeof(devname));
    snprintf(devname, sizeof(devname), "/dev/i2c-%d", dev_i2c_bus);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    for (ctr = 0; ctr < len; ctr++, reg_offset++, wdata++) {
        w_val = *wdata;

        if (skye_i2c_write(fd, dev_addr, addr_sz, reg_offset,
                                (uint16_t)ONE_B_REG, &w_val) != PASSED) {
            printf("%s: Failed to write 0x%02X to Reg. 0x%02X of "
                   "Power Sequencer (I2C%d, Addr = 0x%02X).\n",
                   __FUNCTION__, w_val, reg_offset, dev_i2c_bus, dev_addr);
            close(fd);
            return (FAILED);
        }
    }
    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : crc
 * Description: crc routine for BIB content before write to SROM
 * Inputs     : ptr - pointer to the buffer content
 *              len - length of the buffer content
 * Outputs    : CRC content
 *
 *******************************************************************************
 */
uint32_t
crc (uint8_t* ptr, int len)
{
    uint32_t crc = ~0;

    while (len--)
    {
        uint8_t input = *ptr++;

        for (int i = 0; i < 8; i++)
        {
            crc = (crc >> 1) ^ (((input ^ crc) & 1) ? 0xEDB88320 : 0);
            input >>= 1;
        }
    }
    return ~crc;
}


/*******************************************************************************
 *
 * Function   : skye_bib_change_mac_util
 * Description: functionality to change BIB MAC address
 * Inputs     : none
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int
skye_bib_change_mac_util (void)
{
    unsigned long bus_no = 0;
    unsigned long dev_addr = SR_BIB_I2C_ADDR;
    unsigned long addr_size = 2;
    long offset = -1024;
    unsigned long nbytes = 1024;
    int tbytes = nbytes;
    int twords;
    int ix,skip_bytes;
    boolean found_magic = FALSE;
    int n=0;
    unsigned long uoffset = offset &0xffff;
    char devname[32];
    uint8_t *buf,*buf2;
    uint32_t crc_dat,data_bytes;
    int macN = 1;

    snprintf(devname, sizeof (devname), "/dev/i2c-%d", (int) bus_no);
#ifdef DEBUG
    printf ("%s\n",devname);
#endif
    int fd = open(devname, O_RDWR);

    if (fd < 0)
    {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, (int) bus_no);
        close(fd);
        return (FAILED);
    }

    buf = (uint8_t *) malloc (sizeof(uint8_t)*(nbytes+addr_size));
    buf2 = buf+2;
    if (buf == NULL) {
        printf("%s: cannot allocate %i bytes.\n", __FUNCTION__, (int) (nbytes + addr_size));
        free(buf);
        close(fd);
        return (FAILED);
    }

    if (addr_size == 1)
        buf[0] = uoffset;
    else if (addr_size == 2)
    {
        buf[0] = offset >> 8;
        buf[1] = offset & 0xFF;
    }

    struct i2c_msg msgs[3] = {
        {
            .addr = dev_addr,
            .flags = 0,
            .len = addr_size,
            .buf = buf,
        },
        {
            .addr = dev_addr,
            .flags = I2C_M_RD,
            .len = 0,
            .buf = buf2,
        },
        { // write
            .addr = dev_addr,
            .flags = 0,
            .len = nbytes,
            .buf = buf,
        }
    };

    while (nbytes > 0)
    {
        unsigned long bytesthispass = nbytes;
        unsigned long my_bytesthispass;

        msgs[1].len = bytesthispass;

        struct i2c_rdwr_ioctl_data io =
        {
            .msgs = (addr_size == 0) ? &msgs[1] : msgs,
            .nmsgs = (addr_size == 0) ? 1 : 2,
        };

        if (ioctl(fd, I2C_RDWR, &io) < 0)
        {
            printf("%s: i2cat: can't read", __FUNCTION__);
            free(buf);
            close(fd);
            return (FAILED);
        }
        my_bytesthispass = bytesthispass;

        /*  Scan for PHY CFG */
        while(my_bytesthispass > 0)
        {
#ifdef DEBUG
            printf("%02x%02x%02x%02x\n", buf2[n+3],buf2[n+2],buf2[n+1],buf2[n]);
#endif
            if (buf2[n+3] == 0x00 && buf2[n+2] == 0x37 && buf2[n+1] == 00 && buf2[n]==0x07) {
#ifdef DEBUG
                buf2[n+14] = mac_array[macN][0];
                buf2[n+15] = mac_array[macN][1];
                buf2[n+16] = mac_array[macN][2];
                buf2[n+17] = mac_array[macN][3];
                buf2[n+18] = mac_array[macN][4];
                buf2[n+19] = mac_array[macN][5];
                macN++;
#endif
                if (check_cpu(0) && (macN > 2)) {
                    printf("gbe%d\n", macN);
                } else
                if (check_cpu(1) && (macN > 1)) {
                    printf("gbe%d\n", macN);
                }else {
                    printf("xgbe%d\n", macN);
                }
                for (ix = 14; ix < 20; ix++) {
                    /* User input */
                    buf2[n+ix]= gethex_answer("Enter New Address", buf2[n+ix], 0, 0xFF);
                }
                macN++;
            } else if (buf2[n+3] == 0x30 && buf2[n+2] == 0x42 && buf2[n+1] == 0x69 && buf2[n]==0x42) {
                found_magic=TRUE;
                skip_bytes=n;
                /* Just in case there are more than 255 words */
                twords = buf2[n+8] + (buf2[n+9] << 8) + (buf2[n+10] << 16) + (buf2[n+11] << 24);
#ifdef DEBUG
                printf ("%02x%02x%02x%02x\n",buf2[n+11],buf2[n+10],buf2[n+9],buf2[n+8]);
                printf ("skip_lines = %i\n",skip_bytes);
#endif

            }
            my_bytesthispass-=4; n+=4;
         }
         nbytes -= bytesthispass;
         offset += bytesthispass;
         addr_size = 0;
         macN = 1;
    }

#ifdef DEBUG
    printf("nbytes = %d\n", (int)nbytes);
#endif
    /* 2 bytes of address plus 12 bytes of header */
    if (! found_magic) {
      printf("%s: BIB magic header not found.\n", __FUNCTION__);
      free(buf);
      close(fd);
      return (FAILED);
    }

    /* Convert the number of lines to */
    data_bytes = twords<<2;
    crc_dat = crc(buf2+(skip_bytes)+12,data_bytes);
#ifdef DEBUG
    printf ("twords = %02x CRC=%02x\n",(uint32_t) twords, crc_dat);
    printf ("old CRC = %02x%02x%02x%02x\n",buf2[n-9],buf2[n-10],buf2[n-11],buf2[n-12]);
    printf("tbytes = %d\n", tbytes);
#endif
    /* Insert new CRC */
    buf2[tbytes-12] = crc_dat & 0xff;
    buf2[tbytes-11] = (crc_dat >>  8) & 0xff;
    buf2[tbytes-10] = (crc_dat >> 16) & 0xff;
    buf2[tbytes- 9] = (crc_dat >> 24) & 0xff;

    /* Write to I2C ROM */
    struct i2c_rdwr_ioctl_data io =
    {
        .msgs = &msgs[2],
        .nmsgs = 1,
    };

    if (ioctl(fd, I2C_RDWR, &io) < 0)
    {
        printf("%s: i2cwr: can't write\n", __FUNCTION__);
        free(buf);
        close(fd);
        return (FAILED);
    }
    printf ("Wrote %d bytes to I2C ROM.\n",msgs[2].len);

    if (skye_bib_rd_util() != PASSED)
    {
        printf("%s: bib read failed\n", __FUNCTION__);
        free(buf);
        close(fd);
        return (FAILED);
    }

    free(buf);
    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_bib_wr_util
 * Description: functionality to change BIB MAC address (NEED Investigation)
 * Inputs     : none
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int
skye_bib_wr_util (void)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   wlen = 0, dev_addr = 0, shifted_dev_addr = 0, offset = 0;
    uint16_t   rlen = 0;
    char       devname[32];
    uchar      wdata[1024];
    uchar      rdata[1024];
    uchar      mux_data = PCA9546A_I2C_ALL_CH;
    int macN = 1, ix = 0;
    uint32_t   crc_dat,data_bytes;
    int tbytes = sizeof(wdata);
    int twords;
    int skip_bytes;
    boolean found_magic = FALSE;

    memset(devname, 0, sizeof(devname));
    memset(rdata, 0, sizeof(rdata));
    memset(wdata, 0, sizeof(wdata));

    bus_no = 0;
    dev_addr = 0xA8;
    addr_size = 2;
    offset = BIB_START_OFFSET;
    wlen = BIB_END_OFFSET-offset;
    rlen = wlen;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    shifted_dev_addr = (dev_addr >> I2C_DEV_ADDR_SHIFT);

    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
        }
    }
    /* Read first then make changes */
    if (skye_i2c_read(fd, shifted_dev_addr, addr_size,
                           offset, wlen, wdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    for (ctr = 0; ctr < wlen; ctr+=4) {
        if (wdata[ctr+3] == 0x00 && wdata[ctr+2] == 0x37 && wdata[ctr+1] == 00 && wdata[ctr]==0x07) {
            printf("Before changes:\n");
            /* Printing Ethernet Definition for CPU 0(Master) & 1(Slave)*/
            if (check_cpu(0) && (macN > 2)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,wdata[ctr+14],wdata[ctr+15],wdata[ctr+16],wdata[ctr+17], wdata[ctr+18], wdata[ctr+19]);
            } else
            if (check_cpu(1) && (macN > 1)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,wdata[ctr+14],wdata[ctr+15],wdata[ctr+16],wdata[ctr+17], wdata[ctr+18], wdata[ctr+19]);
            }else {
                printf("xgbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,wdata[ctr+14],wdata[ctr+15],wdata[ctr+16],wdata[ctr+17], wdata[ctr+18], wdata[ctr+19]);
            }

            for (ix = 14; ix < 20; ix++) {
                wdata[ctr+ix]=gethex_answer("Enter Register offset", wdata[ctr+ix], 0, 0xFF);
            }

            printf("After changes:\n");
            /* Printing Ethernet Definition for CPU 0(Master) & 1(Slave)*/
            if (check_cpu(0) && (macN > 2)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,wdata[ctr+14],wdata[ctr+15],wdata[ctr+16],wdata[ctr+17], wdata[ctr+18], wdata[ctr+19]);
            } else
            if (check_cpu(1) && (macN > 1)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,wdata[ctr+14],wdata[ctr+15],wdata[ctr+16],wdata[ctr+17], wdata[ctr+18], wdata[ctr+19]);
            }else {
                printf("xgbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,wdata[ctr+14],wdata[ctr+15],wdata[ctr+16],wdata[ctr+17], wdata[ctr+18], wdata[ctr+19]);
            }
            macN++;

            offset = 0xf000 + ctr + 14;
	        break;
        } else if (wdata[ctr+3] == 0x30 && wdata[ctr+2] == 0x42 && wdata[ctr+1] == 0x69 && wdata[ctr]==0x42) {
            found_magic=TRUE;
            skip_bytes=ctr;
            /* Just in case there are more than 255 words */
            twords = wdata[ctr+8] + (wdata[ctr+9] << 8) + (wdata[ctr+10] << 16) + (wdata[ctr+11] << 24);
            if ((NVRAM)->diagflag & D_VERBOSE) {
              printf ("%02x%02x%02x%02x\n",wdata[ctr+11],wdata[ctr+10],wdata[ctr+9],wdata[ctr+8]);
              printf ("skip_lines = %i\n",skip_bytes);
            }
        }

    }

     /* 2 bytes of address plus 12 bytes of header */
     if (!found_magic) {
         printf("\n%s: BIB magic header not found.\n", __FUNCTION__);
         close(fd);
         return (FAILED);
     }

     /* Update CRC */
     /* Convert the number of lines to */
     data_bytes = twords<<2;
     crc_dat = crc(wdata+(skip_bytes)+12,data_bytes);
     if ((NVRAM)->diagflag & D_VERBOSE) {
       printf ("twords = %02x CRC=%02x\n",(uint32_t) twords, crc_dat);
       printf ("old CRC = %02x%02x%02x%02x\n",wdata[ctr-9],wdata[ctr-10],wdata[ctr-11],wdata[ctr-12]);
     }
     /* Insert new CRC */
     wdata[tbytes-12] = crc_dat & 0xff;
     wdata[tbytes-11] = (crc_dat >>  8) & 0xff;
     wdata[tbytes-10] = (crc_dat >> 16) & 0xff;
     wdata[tbytes- 9] = (crc_dat >> 24) & 0xff;
     if ((NVRAM)->diagflag & D_VERBOSE) {
       printf ("new CRC = %02x%02x%02x%02x\n",wdata[tbytes-9],wdata[tbytes-10],wdata[tbytes-11],wdata[tbytes-12]);
     }
    /* End of update CRC */

    if (skye_i2c_read(fd, shifted_dev_addr, addr_size,
                           offset, wlen, wdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    for (ix = 0; ix < wlen; ix+=4) {
        printf("%02x%02x%02x%02x\n", wdata[ix+3],wdata[ix+2],wdata[ix+1],wdata[ix]);
    }
    printf("offset %x, wlen %x , addr_size %d\n",offset, wlen, addr_size);

    /* After make changes then save it */
    if (skye_i2c_write(fd, shifted_dev_addr, addr_size,
                            offset, wlen, (unsigned char *)wdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    printf("\nDone writing (total %d bytes) to I2C%d, Addr. 0x%02X,"
           " offset = 0x%02X.\n",
           wlen, bus_no, dev_addr, offset);
    printf("offset %x, rlen %x , addr_size %d\n",offset, rlen, addr_size);
    if (skye_i2c_read(fd, shifted_dev_addr, addr_size,
                           offset, rlen, rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    /* Read back after save changes*/
    printf("After save changes:\n");
    macN=1;
    for (ctr = 0; ctr < rlen; ctr+=4) {
        printf("%02x%02x%02x%02x\n", rdata[ctr+3],rdata[ctr+2],rdata[ctr+1],rdata[ctr]);
        if (rdata[ctr+3] == 0x00 && rdata[ctr+2] == 0x37 && rdata[ctr+1] == 00 && rdata[ctr]==0x07) {
            if (check_cpu(0) && (macN > 2)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,rdata[ctr+14],rdata[ctr+15],rdata[ctr+16],rdata[ctr+17], rdata[ctr+18], rdata[ctr+19]);
            } else
            if (check_cpu(1) && (macN > 1)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,rdata[ctr+14],rdata[ctr+15],rdata[ctr+16],rdata[ctr+17], rdata[ctr+18], rdata[ctr+19]);
            }else {
                printf("xgbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,rdata[ctr+14],rdata[ctr+15],rdata[ctr+16],rdata[ctr+17], rdata[ctr+18], rdata[ctr+19]);
            }
            macN++;
        }
    }

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_bib_dump_util
 * Description: Utility to dump BIB in raw
 * Inputs     : none
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int
skye_bib_dump_util (void)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   rlen = 0, dev_addr = 0, shifted_dev_addr = 0, offset = 0;
    char       devname[32];
    uchar      buf[1024];
    uchar      mux_data = PCA9546A_I2C_ALL_CH;

    memset(devname, 0, sizeof(devname));
    memset(buf, 0, sizeof(buf));

    bus_no = 0;
    dev_addr = 0xA8;
    addr_size = 2;
    offset = BIB_DUMP_START_OFF;
    rlen = BIB_END_OFFSET-offset;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    shifted_dev_addr = (dev_addr >> I2C_DEV_ADDR_SHIFT);

    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
        }
    }

    if (skye_i2c_read(fd, shifted_dev_addr, addr_size,
                           offset, rlen, buf) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    printf("\nTotal %d bytes Data read from I2C%d, Addr. 0x%02X, "
           "from offset = 0x%02X:",
           rlen, bus_no, dev_addr, offset);
            printf("\n");

    for (ctr = 0; ctr < rlen; ctr++) {
        if ((ctr % 16) == 0) {
            printf("\n");
        }
        printf("%02x ", buf[ctr]);
    }
    printf("\n");

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_bib_rd_util
 * Description: functionality to read BIB MAC address
 * Inputs     : none
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int
skye_bib_rd_util (void)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   rlen = 0, dev_addr = 0, shifted_dev_addr = 0, offset = 0;
    char       devname[32];
    uchar      buf[1024];
    uchar      mux_data = PCA9546A_I2C_ALL_CH;
    int macN = 1;

    memset(devname, 0, sizeof(devname));
    memset(buf, 0, sizeof(buf));

    bus_no = 0;
    dev_addr = 0xA8;
    addr_size = 2;
    offset = BIB_START_OFFSET;
    rlen = BIB_END_OFFSET-offset;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    shifted_dev_addr = (dev_addr >> I2C_DEV_ADDR_SHIFT);

    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
        }
    }

    if (skye_i2c_read(fd, shifted_dev_addr, addr_size,
                           offset, rlen, buf) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    printf("\nTotal %d bytes Data read from I2C%d, Addr. 0x%02X, "
           "from offset = 0x%02X:",
           rlen, bus_no, dev_addr, offset);
            printf("\n");
    for (ctr = 0; ctr < rlen; ctr+=4) {
        if (buf[ctr+3] == 0x00 && buf[ctr+2] == 0x37 && buf[ctr+1] == 00 && buf[ctr]==0x07) {
            /* Printing Ethernet Definition for CPU 0(Master) & 1(Slave)*/
            if (check_cpu(0) && (macN > 2)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,buf[ctr+14],buf[ctr+15],buf[ctr+16],buf[ctr+17], buf[ctr+18], buf[ctr+19]);
            } else
            if (check_cpu(1) && (macN > 1)) {
                printf("gbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,buf[ctr+14],buf[ctr+15],buf[ctr+16],buf[ctr+17], buf[ctr+18], buf[ctr+19]);
            }else {
                printf("xgbe%d MAC =%02x:%02x:%02x:%02x:%02x:%02x\n", macN ,buf[ctr+14],buf[ctr+15],buf[ctr+16],buf[ctr+17], buf[ctr+18], buf[ctr+19]);
            }
            macN++;
        }
    }
    printf("\n");

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_i2c_rd_util
 * Description:	Wrapper utility to do Skye I2C read.
 * Inputs     : None
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_rd_util (void)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   rlen = 0, dev_addr = 0, shifted_dev_addr = 0, offset = 0;
    char       devname[32];
    uchar      buf[256];
    uchar      mux_data = PCA9546A_I2C_ALL_CH;

    memset(devname, 0, sizeof(devname));
    memset(buf, 0, sizeof(buf));

    bus_no = getdec_answer("Enter I2C bus number", 0, 0, 2);
    dev_addr = gethex_answer("Enter Device I2C address", 0, 0, 0xFF);
    addr_size = getdec_answer("Enter I2C device address size", 1, 1, 2);
    offset = gethex_answer("Enter Register offset", 0, 0, 0xFFFF);
    rlen = getdec_answer("Enter bytes number you want to read", 1, 1, 256);

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    shifted_dev_addr = (dev_addr >> I2C_DEV_ADDR_SHIFT);

    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
        }
    }

    if (skye_i2c_read(fd, shifted_dev_addr, addr_size,
                           offset, rlen, buf) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    printf("\nTotal %d bytes Data read from I2C%d, Addr. 0x%02X, "
           "from offset = 0x%02X:",
           rlen, bus_no, dev_addr, offset);
    for (ctr = 0; ctr < rlen; ctr++) {
        if ((ctr % 16) == 0) {
            printf("\n");
        }
        printf("0x%02X ", buf[ctr]);
    }
    printf("\n");

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_i2c_wr_util
 * Description:	Wrapper utility to do Skye I2C write.
 * Inputs     : None
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_i2c_wr_util (void)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   wlen = 0, dev_addr = 0, shifted_dev_addr = 0, offset = 0;
    char       devname[32], msg[256];
    uchar      wdata[256];
    uchar      mux_data = PCA9546A_I2C_ALL_CH;

    memset(devname, 0, sizeof(devname));
    memset(wdata, 0, sizeof(wdata));
    memset(msg, 0, sizeof(msg));

    bus_no = getdec_answer("Enter I2C bus number", 0, 0, 2);
    dev_addr = gethex_answer("Enter Device I2C address", 0, 0, 0xFF);
    addr_size = getdec_answer("Enter I2C device address size", 1, 1, 2);
    offset = gethex_answer("Enter Register offset", 0, 0, 0xFFFF);
    wlen = getdec_answer("Enter bytes number you want to write", 1, 1, 256);
    for (ctr = 0; ctr < wlen; ctr++) {
        snprintf(msg, sizeof(msg), "Enter data that you want to"
                                   " write into 0x%02X.",
                                   (offset + ctr));
        wdata[ctr] = gethex_answer(msg, 0, 0, 0xFF);
    }

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    shifted_dev_addr = (dev_addr >> I2C_DEV_ADDR_SHIFT);

    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
        }
    }

    if (skye_i2c_write(fd, shifted_dev_addr, addr_size,
                            offset, wlen, wdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    printf("\nDone writing (total %d bytes) to I2C%d, Addr. 0x%02X,"
           " offset = 0x%02X.\n",
           wlen, bus_no, dev_addr, offset);
    for (ctr = 0; ctr < wlen; ctr++) {
        printf("Offset:0x%02X = 0x%02X\n", (offset + ctr), wdata[ctr]);
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_thermal_i2c_read
 * Description:	Wrapper function to Read Skye thermal sensor register
 *              through Tilera CPU I2CM2.
 * Inputs     :	offset - Register offset that wants to read
 *              len - Total size(in bytes) that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_thermal_i2c_read (uint16_t offset, uint16_t len, uchar* buf)
{
    int        fd = -1, ctr = 0;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      rd_data = 0, mux_data = PCA9546A_I2C_CH1;

    memset(devname, 0, sizeof(devname));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 2 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }

    dev_addr = SR_THERMAL_I2C_ADDR;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    for (ctr = 0; ctr < len; ctr++, buf++) {
        if (skye_i2c_read(fd, dev_addr, SR_FPGA_ADDR_SZ,
                               (uint16_t)(offset + ctr), ONE_B_REG,
                               (uchar *)&rd_data) != PASSED) {
            printf("%s: Failed to read data from FPGA Szalinski"
                   " (I2C%d, Addr = 0x%02X).\n",
                   __FUNCTION__, SR_CPU_I2CM2, dev_addr);
            close(fd);
            return (FAILED);
        }
        *buf = rd_data;
    }

    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_thermal_i2c_write
 * Description:	Wrapper function to Write Skye thermal sensor register
 *              through Tilera CPU I2CM2.
 * Inputs     :	offset - Register offset that wants to write
 *              len - Total size(in bytes) that wants to write
 *              wdata - Data want to write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_thermal_i2c_write (uint16_t offset, uint16_t len, uchar* wdata)
{
    int        fd = -1;
    uint16_t   dev_addr = 0, ctr = 0;
    char       devname[32];
    uchar      w_data = 0, mux_data = PCA9546A_I2C_CH1;

    memset(devname, 0, sizeof(devname));

    /* Set-up I2C MUX */
    if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel 2 for Szalinski.\n",
               __FUNCTION__);
        return (FAILED);
    }

    dev_addr = SR_THERMAL_I2C_ADDR;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    for (ctr = 0; ctr < len; ctr++, wdata++) {
        w_data = *wdata;
        if (skye_i2c_write(fd, dev_addr, SR_FPGA_ADDR_SZ,
                                (uint16_t)(offset + ctr), ONE_B_REG,
                                (uchar *)&w_data) != PASSED) {
            printf("%s: Failed to write data to FPGA Szalinski"
                   " (I2C%d, Addr = 0x%02X).\n",
                   __FUNCTION__, SR_CPU_I2CM2, dev_addr);
            close(fd);
            return (FAILED);
        }
    }

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	skye_clk_buf_i2c_read
 * Description:	Wrapper function to Read Skye clock buffer register
 *              through Tilera CPU I2CM2.
 * Inputs     :	offset - Register offset that wants to read
 *              len - Total size(in bytes) that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_clk_buf_i2c_read (uint16_t offset, uint16_t len, uchar* buf)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];
    uchar      rd_data = 0;

    memset(devname, 0, sizeof(devname));

    dev_addr = SR_CLK_BUF_I2C_ADDR;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, 2,
            (uint16_t)(offset), ONE_B_REG,
            (uchar *)&rd_data) != PASSED) {
        printf("%s: Failed to read data from Clock Buffer"
               " (I2C%d, Addr = 0x%02X, offset = 0x%04X)).\n",
               __FUNCTION__, SR_CPU_I2CM2, dev_addr, (uint16_t)offset);
            close(fd);
            return (FAILED);
    }
    *buf = rd_data;

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_clk_buf_i2c_read_x
 * Description: Wrapper function to Read Skye clock buffer register
 *              through Tilera CPU I2CM2.
 * Inputs     : offset - Register offset that wants to read
 *              buf - Buffer to put the read back data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_clk_buf_i2c_read_x (int offset, uint* buf)
{
    return (skye_clk_buf_i2c_read((uint16_t)offset, 1,(uchar *)buf));
}

/*******************************************************************************
 *
 * Function   :	skye_clk_buf_i2c_write
 * Description:	Wrapper function to Write Skye clock buffer register
 *              through Tilera CPU I2CM2.
 * Inputs     :	offset - Register offset that wants to write
 *              len - Total size(in bytes) that wants to write
 *              wdata - Data want to write-in
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_clk_buf_i2c_write (uint16_t offset, uint16_t len, uchar* wdata)
{
    int        fd = -1;
    uint16_t   dev_addr = 0;
    char       devname[32];

    memset(devname, 0, sizeof(devname));

    dev_addr = SR_CLK_BUF_I2C_ADDR;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", SR_CPU_I2CM2);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, SR_CPU_I2CM2);
        return (FAILED);
    }

    if (skye_i2c_write(fd, dev_addr, 2,
            (uint16_t)(offset), len, (uchar *)wdata) != PASSED) {
            printf("%s: Failed to write data to Clock Buffer"
                   " (I2C%d, Addr = 0x%02X, offset = 0x%04X).\n",
            __FUNCTION__, SR_CPU_I2CM2, dev_addr, (uint16_t)(offset));
        close(fd);
        return (FAILED);
    }

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_clk_buf_i2c_write_x
 * Description: Wrapper function to Write Skye clock buffer register
 *              through Tilera CPU I2CM2.
 * Inputs     : offset - Register offset that wants to write
 *              wdata - Data want to write-in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_clk_buf_i2c_write_x (int offset, int wdata)
{
    uchar* data = NULL;
    data = (uchar *)&wdata;
    return (skye_clk_buf_i2c_write(offset, 1, data));
}


/*******************************************************************************
 *
 * Function   : util_clock_buffer_reg_rd
 * Description: Wrapped uility to read specific register of
 *              Skye clock buffer content.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
util_clock_buffer_reg_rd (void)
{
    uint16_t     off = 0;
    uchar        rdata = 0;

    off = (uint16_t)gethex_answer("Enter offset you want to read ",
                                  0x0000, 0x0000, 0x0316);
    if (skye_clk_buf_i2c_read(off, 0, (uchar *)&rdata) != PASSED) {
        printf("%s: Failed to read clock buffer content"
               "(offset = 0x%04X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    printf("(0x%04X): 0x%02X.\n", off, rdata);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_buffer_reg_wr
 * Description: Wrapped uility to write specific register of
 *              Skye clock buffer content.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
util_clock_buffer_reg_wr (void)
{
    uchar      rdata = 0, cdata = 0, wdata = 0;
    uint16_t   off = 0;

    off = (uint16_t)gethex_answer("Enter offset you want to write ",
                                  0x00, 0x00, 0x316);

    if (skye_clk_buf_i2c_read(off, 0, (uchar *)&rdata) != PASSED) {
        printf("%s: Failed to read clock buffer register"
               "(offset = 0x%04X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    wdata = (uint16_t)gethex_answer("Enter Data you want to write-in ",
                                    rdata, 0x00, 0xFF);

    if (skye_clk_buf_i2c_write(off, 0, (uchar *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%04X to clock buffer register"
               "(offset = 0x%04X).\n", __FUNCTION__, wdata, off);
        return (FAILED);
    }

    if (skye_clk_buf_i2c_read(off, 0, (uchar *)&cdata) != PASSED) {
        printf("%s: Failed to read clock buffer register"
               "(offset = 0x%04X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    printf("The original value of 0x%04X is 0x%02X.\n", off, rdata);
    printf("Now the value of 0x%04X is 0x%02X.\n\n", off, cdata);

    return (PASSED);
}


/*-------------------------------------------------
$Log: skye_i2c_api.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.4  2015/05/11 13:45:46  steja
Code clean up <CSCuu14285>

Revision 1.1.4.3  2015/04/30 08:33:53  steja
Clean up code

Revision 1.1.4.2  2015/04/29 11:36:36  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.5  2014/11/27 09:19:30  palin2
Added utility to dump all BIB value in raw.

Revision 1.1.2.4  2014/11/10 09:48:02  steja
Update Clock buffer utility

Revision 1.1.2.3  2014/10/07 13:53:29  steja
Update read and write clock buffer function

Revision 1.1.2.2  2014/08/08 03:45:04  steja
Add Clock Buffer Register Test

Revision 1.1.2.1  2014/07/21 01:56:55  palin2
Initial check-in Skye module side Diag code.

---------------------------------------------------
skye_i2c_api.c:
Revision 1.2.8.3  2014/07/09 02:21:08  palin2
Support I2C scan test for Shrinkray.

Revision 1.2.8.2  2014/05/26 15:24:12  palin2
Updated error message of power sequencer i2c write function.

Revision 1.2.8.1  2014/05/20 17:54:51  palin2
Add power sequencer utilities and update power sequencer I2C r/w function.

Revision 1.2  2014/02/27 15:01:46  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.15  2014/02/07 18:31:32  steja
code clean up

Revision 1.1.4.14  2014/01/16 03:44:42  steja
Update BIB R/W MAC utility

Revision 1.1.4.13  2014/01/13 17:19:41  steja
Add Clock Buffer utility Read and Write to Basic Utilities

Revision 1.1.4.12  2013/12/25 09:11:21  steja
Fix the clock buffer programming issue

Revision 1.1.4.11  2013/12/18 05:03:11  steja
1. support PSE2 backplane loopback test
2. support BIB change MAC address utility

Revision 1.1.4.10  2013/12/16 08:34:35  iachang
Support current sensor
Modify on-board thermal sensor

Revision 1.1.4.9  2013/12/06 09:39:44  iachang
Move DIMM Thermal sensor to skye_thermal.c
Support on-board Thermal sensor
Convert the measure to actual temperature

Revision 1.1.4.8  2013/11/29 07:08:55  steja
1. Fix the full data path TLK working.
2. add USB test
3. add read BIB MAC utility

Revision 1.1.4.7  2013/11/19 14:36:47  steja
Provide TLK utility for debugging
Update the BTK TLK into coded

Revision 1.1.4.6  2013/11/18 11:00:18  iachang
Support CPU1 Szalinski FPGA I2C access.

Revision 1.1.4.5  2013/11/13 08:18:53  palin2
Update DIMM Thermal sensor I2C write function.

Revision 1.1.4.4  2013/11/13 01:38:05  palin2
1. Add utilities to read & dump ShrinkRay DIMM thermal sensor register.
2. Also add specific I2C read/write function for ShrinkRay Thermal sensor.

Revision 1.1.4.3  2013/10/28 06:09:39  palin2
1. Add setup I2C Mux suport in ShrinkRay I2C read/write utilities.
2. Add utilities to read & dump Szalinski registers.
3. Update Szalinski show version utility.

Revision 1.1.4.2  2013/09/13 07:00:09  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.5  2013/08/27 02:57:19  palin2
Add ShinkRay I2C access uilities wrap.

Revision 1.1.2.4  2013/08/19 07:11:51  palin2
Add Voltage Margin utility.

Revision 1.1.2.3  2013/07/15 21:55:01  palin2
Initial check-in for ShrinkRay FPGA(Szalinski) Diag test and utility.

Revision 1.1.2.2  2013/07/14 22:03:12  palin2
Added ShrinkRay I2C write support and DDR DIMM SPD write utility.

Revision 1.1.2.1  2013/07/09 07:24:25  palin2
Create "skye_i2c_api.c" for ShrinkRay I2C APIs,
and move related I2C read/write function to it.

---------------------------------------------------
$Endlog$
*/


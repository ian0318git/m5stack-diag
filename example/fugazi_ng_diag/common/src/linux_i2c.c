/* $Id: linux_i2c.c,v 1.11 2020/08/19 09:49:17 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_i2c.c,v $
 *------------------------------------------------------------------
 * Filename:    linux_i2c.c
 *
 * Description: Linux User Mode I2C wrapper
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <types.h>
#include <stdint.h>
#include <string.h>
#include <linux/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "linux_api.h"
#include "common.h"
#include "error.h"
#include "types.h"
#include "proto.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "n2g_api_rc.h"
#include "i2c_dev.h"
#include <assert.h>
#include "byteswap.h"
#include <errno.h>
/*********************************************************************
 *
 * Function:    show_i2c0
 *
 * Description: Display Motherboard TWSI0 registers.
 *
 * Inputs:      None.
 *
 * Outputs:     FAILED - DiagLinux does not support show register.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int show_i2c0(void)
{
    testname("Display TWSI 0 Registers");
    cterr('f', 0, "%s %s Not supported", __FILE__, __FUNCTION__);
    return(FAILED);
}

/*********************************************************************
 *
 * Function:    show_i2c1
 *
 * Description: Display Motherboard TWSI1 registers.
 *
 * Inputs:      None.
 *
 * Outputs:     FAILED - DiagLinux does not support show register.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int show_i2c1(void)
{
    testname("Display TWSI 1 Registers");
    cterr('f', 0, "%s %s Not supported", __FILE__, __FUNCTION__);
    return(FAILED);
}

/*********************************************************************
 *
 * Function:    api_mb_i2c_reset
 *
 * Description: API for Motherboard I2C reset. For Cavium I2C, write to TWSI_RST
 *              (TWSI Software Reset Register) will soft reset the I2C
 *              controller (TWSI core) back to idle (TWSI_STAT = 0xF8),
 *              and sets TWSI_CLT[STA, STP, IFLG) to 0's. "After this
 *              register is written, the TWSI controller is not available for
 *              any communication with the TWSI bus for three TCLK cycles
 *              (for the default value of the TWSI_CLK, it derives to 315
 *              core-clock cycles)." N2G I2C devices on the motherboard runs
 *              at 100 KHz (10 microseconds), 3 TCLK cycles will require
 *              30 microseconds delay. Core is running at GHz range (ns),
 *              315 ns may not be sufficient.
 *              The I2C controller state is not checked, so that if the I2C
 *              controller is busy with other device, it will be yanked out
 *              of the busy state, and enters idle state.
 *
 * Inputs:      i2c_ctl - N2G_I2C_BUS in n2g_i2c.h.
 *
 * Outputs:     PASSED - No errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t api_mb_i2c_reset(uint8_t i2c_ctl)
{
    printf("****%s %s Diaglinux does not support I2C reset\n",
            __FILE__, __FUNCTION__);
    return(PASSED);
}

/*********************************************************************
 *
 * Function:    api_mb_i2c_init
 *
 * Description: Motherboard I2C API for init.
 *
 * Inputs:      i2c_ctl - points to the I2C host adapter struct.
 *              i2c_speed - I2C Bus speed.
 *
 * Outputs:     PASSED - No errors encounterd.
 *		E_I2C_INV_ACK or others - Unable to open the host adapter.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t api_mb_i2c_init(i2c_host_t *i2c_ctl, char i2c_speed)
{
#ifdef I2C_INIT_DEBUG
    printf("\n **** %s CP 1 i2c_ctl %p \n", __FUNCTION__, i2c_ctl);
#endif /* I2C_INIT_DEBUG syy tmp */
    if ((i2c_ctl->fp = open((char *)i2c_ctl->dev_name, O_RDWR)) < 0) {
#ifdef I2C_INIT_DEBUG
    printf("\n **** %s CP 2 ### \n", __FUNCTION__);
#endif /* I2C_INIT_DEBUG  syy tmp */

	if (i2c_ctl->fp == -ENODEV) {
	    /* No device found */
#ifdef I2C_INIT_DEBUG
            printf("\n **** %s CP 2 ### \n", __FUNCTION__);
#endif /* I2C_INIT_DEBUG syy tmp */

	    return(E_I2C_INV_ACK);
	} else {
#ifdef I2C_INIT_DEBUG
            printf("\n **** %s CP 5 ### \n", __FUNCTION__);
#endif /* I2C_INIT_DEBUG syy tmp */

            cterr('f', 0, "%s %s Unable to open the host adapter %s in kernel. "
			  "%#x", __FILE__, __FUNCTION__, i2c_ctl->dev_name,
			  i2c_ctl->fp);
            return(i2c_ctl->fp);
	}
    } else {
#ifdef I2C_INIT_DEBUG
        printf("\n **** %s CP 10 ### \n", __FUNCTION__);
#endif /* I2C_INIT_DEBUG syy tmp */

        return(PASSED);
    }
}

/*********************************************************************
 *
 * Function:    api_mb_i2c_open
 *
 * Description: Motherboard I2C API for open to the host adapter.
 *
 * Inputs:      i2c_ctl - points to the I2C host adapter struct.
 *              dev_p   - Pointer to device characteristics table.
 *
 * Outputs:     PASSED - No errors encounterd.
 *              E_I2C_INV_ACK or others - Unable to open the host adapter.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t api_mb_i2c_open(i2c_host_t *i2c_ctl, n2g_i2c_dev_t *dev_p)
{
    int rc;
/*#define LINUX_I2C_OPEN_DEBUG  * syy tmp */

#ifdef LINUX_I2C_OPEN_DEBUG
    printf("%s dev %#x %s\n",
           __FUNCTION__, dev_p->dev_addr, i2c_ctl->dev_name);
#endif /* LINUX_I2C_OPEN_DEBUG */

    if ((i2c_ctl->fp = open((char *)i2c_ctl->dev_name, O_RDWR)) < 0) {
	/* Unable to open to the host adapter */
	if (i2c_ctl->fp == -ENODEV) {
	    /* No device found */
	    return(E_I2C_INV_ACK);
	} else {
            cterr('f', 0, "%s %s Unable to open the host adapter %s in kernel. "
			  "%#x", __FILE__, __FUNCTION__, i2c_ctl->dev_name,
			  i2c_ctl->fp);
 	    return(i2c_ctl->fp);
	}
    } else {
	/* Connect to the slave */
        if ((rc = ioctl(i2c_ctl->fp, I2C_SLAVE, dev_p->dev_addr)) < 0) {
	    /* Unable to connect to the device */
	    if (i2c_ctl->fp == -ENODEV) {
		/* No device found */
		 return(E_I2C_INV_ACK);
	     } else {
		cterr('f', 0, "%s %s %s unable to connect to device %#x. "
			      "rc = %#x", __FILE__, __FUNCTION__,
			      i2c_ctl->dev_name, dev_p->dev_addr, rc);
		return(i2c_ctl->fp);
	    }
        } else {
	    dev_p->fp = i2c_ctl->fp;
#ifdef LINUX_I2C_OPEN_DEBUG
    printf("%s dev %#x opened sucdessfully, fp = %#x\n",
           __FUNCTION__, dev_p->dev_addr, i2c_ctl->fp);
#endif /* LINUX_I2C_OPEN_DEBUG */
            return(PASSED);
        }
    }
#undef LINUX_I2C_OPEN_DEBUG /* syy tmp */
}

/*********************************************************************
 *
 * Function:    api_mb_i2c_close
 *
 * Description: Motherboard I2C API for closing the host adapter.
 *
 * Inputs:      i2c_ctl - points to the I2C host adapter struct.
 *
 * Outputs:     PASSED - No errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t api_mb_i2c_close(i2c_host_t *i2c_ctl)
{
/*#define LINUX_I2C_CLOSE_DEBUG  * syy tmp */

#ifdef LINUX_I2C_CLOSE_DEBUG
    printf("%s dev %#s \n",
           __FUNCTION__, i2c_ctl->dev_name);
#endif /* LINUX_I2C_CLOSE_DEBUG */

    close(i2c_ctl->fp);
    return (PASSED);
#undef LINUX_I2C_CLOSE_DEBUG /* syy tmp */
}

/*-------------------------------------------------------------------
 *
 * Function : is_cterr_print_on
 * Description: Return FALSE if need cterr print off 
 *              This function returns TRUE by default, and if not define
 *              is_cterr_on declare this function in platform
 *              code
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_cterr_print_on (void)
        __attribute__((weak, alias("__is_cterr_print_on")));
        int __is_cterr_print_on (void)
{
        return (TRUE);
}

/*********************************************************************
 *
 * Function:    api_mb_i2c_read
 *
 * Description: Motherboard I2C Read API.
 *
 * Inputs:      dev_p   - Pointer to device characteristics table.
 *              offset  - I2C device offset.
 *              size    - Number of bytes to read.
 *              *buf    - Read buffer pointer.
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Failed to read.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t api_mb_i2c_read(n2g_i2c_dev_t *dev_p, uint32_t offset,
                uint8_t size, char *buf)
{
    struct i2c_msg rd_msg;
    uint32_t rc = PASSED;
    __s32 res;
    boolean i2c_cterr_print = is_cterr_print_on(); 
/*#define LINUX_I2C_READ_DEBUG   * */

#ifdef LINUX_I2C_READ_DEBUG 
    printf("api_mb_i2c_read %#x offset %#x;  %d bytes \n",
           dev_p->dev_addr, offset, size);
#endif

    switch (dev_p->rd_hd_size) {
    case 0:
	switch (size) {
	case 0:
	    /* Nothing to read */
	    assert(!"I2C Read Programming error");
	    res = -1;
	    break;
	case 1:
	    /* Read a byte with SMBus */
	    res = i2c_smbus_read_byte(dev_p->fp, (__u8 *)buf);
	    break;
	default:
           res = i2c_smbus_read_i2c_block_data(dev_p->fp, (uint8_t)offset,
	                                       (uint8_t)size, (__u8 *)buf);
	    break;
	}
        if (res < 0) {
	    /* read failed */
            if (i2c_cterr_print == FALSE) {
                rc = FAILED;
            } else {
                cterr('f', 0, "%s %d Device %#x %d byte SMBus read failed %#x, errno(%d)",
                      __FILE__, __LINE__, dev_p->dev_addr,
                      size, res, errno);
                rc = FAILED;
            }
	}
	break; 
    case 1:
        /* Use SMBus */
        switch (size) {
        case 1:
            /* Read one data byte */
	    res = i2c_smbus_read_byte_data(dev_p->fp, (uint8_t)offset, (__u8 *)buf);
	    break;
	case 2:
	    /* Read 2 data bytes (word in SMBus term) */
	    res = i2c_smbus_read_word_data(dev_p->fp, (uint8_t)offset,
					   (uint16_t *)buf);
	    break;
        default:
	    /* More than 2 bytes */
	    res = i2c_smbus_read_i2c_block_data(dev_p->fp, (uint8_t)offset,
						(uint8_t)size, (__u8 *)buf);
	    break;
        }
        if (res < 0) {
            /* read failed */
            if (i2c_cterr_print == FALSE) {
                rc = FAILED;
            } else {
                cterr('f', 0, "%s %s Device %#x offset %#x %d bytes. "
                      "SMBus returns %#x, errno(%d)", __FILE__, __FUNCTION__, 
                      dev_p->dev_addr, offset, size, res, errno);
                rc = FAILED;
            }
        } else {
	    
        }
        break;
    default:
        /* Need a write first */
	switch (dev_p->rd_hd_size) {
	case 2:
	    /* 2 bytes of offset/command */
	    res = i2c_smbus_write_byte_data(dev_p->fp, (uint8_t)(offset >> 8),
					    (uint8_t)offset);
	    break;
	case 3:
	    /* 3 bytes of offset/command */
	    res = i2c_smbus_write_word_data(dev_p->fp, (uint8_t)(offset >> 16),
					    (uint16_t)offset);
	    break;
	case 4:
           res = i2c_smbus_read_i2c_block_data(dev_p->fp, (uint8_t)offset,
	                                       (uint8_t)size, (__u8 *)buf);
	    break;
	default:
            assert(!"I2C Read Programming error");
	    res = -1;
	    break;
	} /* endof switch */

        if (res >= 0) {
            /* Ready for the read. */
            rd_msg.addr = (uint16_t)dev_p->dev_addr;
            rd_msg.flags = I2C_M_RD;
            rd_msg.len = (uint16_t)size;
            rd_msg.buf = buf;
            res = i2c_access(dev_p->fp, &rd_msg);
        } /* endof if rc */

        if (res < 0) {
            /* read failed */
            if (i2c_cterr_print == FALSE) {
                rc = FAILED;
            } else {
                cterr('f', 0, "%s %s Device %#x %d byte SMBus read failed %#x",
                              __FILE__, __FUNCTION__, dev_p->dev_addr,
                              size, res);
                rc = FAILED;
            }
        }

	break;
    }

    return(rc);
}


/*-------------------------------------------------------------------
 *
 * Function : is_need_dswap
 * Description: Return TRUE if need dswap 
 *              This function returns FALSE by default, and if not define
 *              is_need_dswap on declare this function in platform
 *              code
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
boolean is_need_dswap (void)
        __attribute__((weak, alias("__is_need_dswap")));
        boolean __is_need_dswap (void)
{
        return (FALSE);
}

/*********************************************************************
 *
 * Function:    api_mb_i2c_write
 *
 * Description: Motherboard I2C Write API.
 *
 * Inputs:      dev_p   - Pointer to device characteristics table.
 *              offset  - I2C device offset.
 *              size    - Number of bytes to write.
 *              *buf    - Write buffer pointer.
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Failed to write.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t api_mb_i2c_write(n2g_i2c_dev_t *dev_p, uint32_t offset,
                 uint8_t size, char *buf)
{
#if OVERLORD
#else
    struct i2c_msg wr_msg;
#endif
    uint32_t rc = PASSED;
    __s32 res;
    uint16_t wr_word;
    unsigned char wr_buf[260];
    uint8_t wr_length = dev_p->wr_hd_size + size;
    int buf_i = 0;
    boolean buf_need_swap = is_need_dswap(); 
    uint16_t wr_tmp = 0;
/*#define LINUX_I2C_WRITE_DEBUG  * syy tmp */
    
#ifdef LINUX_I2C_WRITE_DEBUG
    printf("%s dev %#x offset %#x %d bytes. fp = %#x\n",
           __FUNCTION__, dev_p->dev_addr, offset, size, dev_p->fp);
#endif /* LINUX_I2C_WRITE_DEBUG */

    if (wr_length == 0) {
	cterr('f', 0, "%s %s size of 0. Size = %d. Head size %d. \n"
                      "Check the program", __FILE__, __FUNCTION__,
		      dev_p->wr_hd_size, size);
	return FAILED;
    }

    memset(wr_buf, 0, sizeof(wr_buf));

    if (dev_p->wr_hd_size) {
        /* Need to send out the offset or command */
        for (; buf_i < dev_p->wr_hd_size; buf_i++ ) {
            wr_buf[buf_i] = (offset >> ((dev_p->wr_hd_size - buf_i - 1) * 8)) & 
                            0xFF;
        }
    }

	memcpy(&wr_buf[buf_i],buf,size);

    if (buf_need_swap == TRUE) {
        wr_length = dev_p->wr_hd_size + size;
    } 
    switch (wr_length) {
    case 1:
	/* SMBus Send Byte */
	res = i2c_smbus_write_byte(dev_p->fp, wr_buf[0]);
	break;
    case 2:
	/* SMBus Write Byte */
	res = i2c_smbus_write_byte_data(dev_p->fp, wr_buf[0], wr_buf[1]);
	break;
    case 3:
	/* SMBus Write Word */
    wr_word = (uint16_t)wr_buf[1] << 8;
    if (buf_need_swap == TRUE) {
        /* wr_buf[2] need to swap, then shift back to the right 8 bit */
        wr_tmp = (uint16_t)dswap2(wr_buf[2]);
        wr_tmp = (uint16_t)wr_tmp >> 8;
        wr_word |= (uint16_t)wr_tmp;
    } else {	
        wr_word |= (uint16_t)wr_buf[2];
    }	
    res = i2c_smbus_write_word_data(dev_p->fp, wr_buf[0], wr_word);
	break;
    default:
#if OVERLORD
	/* SMBus Write Block */
        res = i2c_smbus_write_i2c_block_data(dev_p->fp, wr_buf[0],
                                             (wr_length - 1), (__u8 *)&wr_buf[1]);
#else
	/* Cannot use SMBus. Pure I2C access */
        wr_msg.addr = (uint16_t)dev_p->dev_addr;
        wr_msg.flags = 0;	/* write */
        wr_msg.len = wr_length;
        wr_msg.buf = (char *)&wr_buf[0];

	res = i2c_access(dev_p->fp, &wr_msg);
#endif /* OVERLORD */
	break;
    } /* endof switch */

    if (res < 0) {
        /* read failed */
        cterr('f', 0, "%s %s Device %#x %d byte I2C write failed rc = %#x. "
		      "wr_length = %#x",
                      __FILE__, __FUNCTION__, dev_p->dev_addr,
                      size, res, wr_length);
        rc = FAILED;
    }

    return (rc);
}


/*------------------------------------------------------------------
$Log: linux_i2c.c,v $
Revision 1.11  2020/08/19 09:49:17  markzha
*** empty log message ***

Revision 1.10  2019/06/14 03:58:52  mikech2
Collapse katar-branch00 to Main Trunk

Revision 1.9.22.4  2019/06/12 01:00:02  mikech2
Fix TSN compile warning

Revision 1.9.22.3  2019/05/02 03:00:50  mikech2
Update linux_i2c base on PRRQ#4685761 Comment#4

Revision 1.9.22.2  2019/04/22 02:52:48  mikech2
Update linux_i2c base on PRRQ#4685761 Comment#4

Revision 1.9.22.1  2018/09/07 03:06:30  mikech2
Fix api_mb_i2c_write negative value issue

Revision 1.9  2018/02/09 09:11:18  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.8.16.1  2018/02/01 12:58:17  steja
Add platform function for common code

Revision 1.8  2017/09/06 12:14:17  steja
1.Fix TSN WIFI ACT2 i2c scan test failed at first time after power on (CSCvf83218)
2. Remove Discrete ACT2 utility and I2C Scan for Discrete ACT2 only for Development phase(CSCvf81035)

Revision 1.7  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.6  2014/01/10 21:30:58  mcharon
remove header files that no longer exist for new gnu

Revision 1.5  2013/10/08 08:48:26  tirawan
Woodlawn collapsed to main trunk

Revision 1.4  2012/06/06 09:57:09  iachang
Clean up complier warnings.

Revision 1.3  2012/05/31 14:24:24  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/

/* $Id: ich9_i2c.c,v 1.2 2013/11/26 08:40:35 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/ich9_i2c.c,v $
 *------------------------------------------------------------------
 * Filename:	ich9_i2c.c
 *
 * Description:	Informers (ICH9 - SouthBridge) I2C API supports.
 *		ICH9 SMBus registers can be accessed via I/O or Memory mapped
 *		mechanism. Informers Diag I2C will use memory mapped. The
 *		Informers PCIe diag or the BIOS may configure both the I/O
 *		and the Memory, but this module will use memory mapped since
 *		the memory access is faster and easier than I/O access.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "endians.h"
#include "common.h"
#include "error.h"
#include "types.h"
#include "proto.h"
#include <assert.h>
#include <sys/mman.h>
#include "i2c_api.h"
#include "i2c_dev.h"
#include "platform_i2c.h"
#include "n2g_api_rc.h"
#include "platform_pci.h"
#include "ich9_i2c_regs.h"
#include "dash_fpga.h"


//#define ICH9_SMB_DEBUG 1

/* #define SIMULATE_RW   * */
/* #define ICH9_IO_ACCESS   * */
/* #define ICH9_REG_RD_DEBUG  * */
/* #define ICH9_REG_WR_DEBUG  * */


/*********************************************************************
 *		Functions prototype
 *********************************************************************
 */
static uchar ich9_smb_rd_b(ulong, ulong);
static void  ich9_smb_wr_b(ulong, ulong, uchar);
static uint16_t ich9_smb_rd_w(ulong, ulong);
/* ICH9 only has one 2 bytes register. We only read it. */
static int ich9_smb_pci_wr_b(ulong, uint8_t, uint);
/* static int wait_for_busy_off(ich9_smb_reg_t *, int); */
static ich9_smb_reg_t * get_ich9_smb_base(void);
uint32_t ich_i2c_write(n2g_i2c_dev_t *, ulong, uint8_t, char *);
/* static uint32_t ich9_i2c_clear_stat(ulong, int);
 * static uint32_t ich9_i2c_get_block_data(ulong, uint, char *);
 */

extern uint32_t api_mb_i2c_read(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);
extern uint32_t api_mb_i2c_write(n2g_i2c_dev_t *, uint32_t, uint8_t, char *);
extern int      fd_i2c0;
extern int      fd_fpga;

/*********************************************************************
 *		Global variables
 *********************************************************************
 */


/*********************************************************************
 *
 * Function:	show_ich_i2c
 *
 * Description:	Display Motherboard SouthBridge ICH9 I2C registers.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Unknown VID/DID.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int
show_ich_i2c(void)
{
    ich9_smb_reg_t * i2c_p;
    ulong offset;
    uint32_t reg;
    uint16_t dw;
    uint8_t bus, dev, func;
    uint8_t db;

    bus = PCI_SMBUS_BUS;
    dev = PCI_SMBUS_DEV;
    func = PCI_SMBUS_FUNC;

    testname("Display ICH9 SMBus Registers");

    printf("\nDisplay ICH9 SMBus Controller PCIe Registers:\n");

    reg = pci_config_read(bus, dev, func, PCI_VENDOR_ID_OFFSET);
    printf("Vendor Identification          = 0x%04X\n", reg);
    printf("Device Identification          = 0x%04X\n", reg >> 16);

    reg = pci_config_read(bus, dev, func, PCI_COMMAND_REG_OFFSET);
    printf("PCI Command                    = 0x%04X\n", reg);
    printf("PCI Status                     = 0x%04X\n", reg >> 16);

    reg = pci_config_read(bus, dev, func, PCI_REVISION_ID_OFFSET);
    printf("Revision Identification        = 0x%02X\n", reg);
    printf("Programming Interface          = 0x%02X\n", reg >> 8);
    printf("Sub Class Code                 = 0x%02X\n", reg >> 16);
    printf("Base Class Code                = 0x%02X\n", reg >> 24);

    reg = pci_config_read(bus, dev, func, PCI_MEM_BAR0_OFFSET);
    printf("Memory Base Address Register 0 = 0x%08X\n", reg);

    reg = pci_config_read(bus, dev, func, PCI_MEM_BAR1_OFFSET);
    printf("Memory Base Address Register 1 = 0x%08X\n", reg);

    reg = pci_config_read(bus, dev, func, PCI_MEM_BAR4_OFFSET);
    printf("SMBus Base Address             = 0x%08X\n", reg);

    reg = pci_config_read(bus, dev, func, ICH9_SMB_SVID);
    printf("Subsystem Vendor Identification= 0x%04X\n", reg);
    printf("Subsystem Identification       = 0x%04X\n", reg >> 16);

    reg = pci_config_read(bus, dev, func, ICH9_SMB_INT_LN);
    printf("Interrupt Line                 = 0x%02X\n", reg);
    printf("Interrupt Pin                  = 0x%02X\n", reg >> 8);

    reg = pci_config_read(bus, dev, func, ICH9_SMB_HOSTC);
    printf("Host Configuration             = 0x%02X\n", reg);

    printf("\nDisplay ICH9 SMBus Controller Registers:\n");

    i2c_p = get_ich9_smb_base();

    if (i2c_p == NULL) {
#ifdef ICH9_I2C_SHOW_DEBUG
	cterr('f', 0, "show_ich_i2c() Unable to get ICH9 SMBus Base");
#endif /* ICH9_I2C_SHOW_DEBUG */
	return(FAILED);
    }

    offset = (ulong)i2c_p + ICH9_HST_STS;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_HST_STS);
    printf("Host Status            = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_HST_CNT;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_HST_CNT);
    printf("Host Control           = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_HST_CMD;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_HST_CMD);
    printf("Host Command           = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_XMIT_SLVA;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_XMIT_SLVA);
    printf("Transmit Slave Address = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_HST_D0;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_HST_D0);
    printf("Host Data 0            = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_HST_D1;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_HST_D1);
    printf("Host Data 1            = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_HOST_BLOCK_DB;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_HOST_BLOCK_DB);
    printf("Host Block Data Byte   = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_PEC;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_PEC);
    printf("Packet Error Check     = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_RCV_SLVA;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_RCV_SLVA);
    printf("Receive Slave Address  = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_SLV_DATA;
    dw = ich9_smb_rd_w((ulong) i2c_p, ICH9_SLV_DATA);
    printf("Receive Slave Data     = 0x%04x @ %#lx\n", dw, offset);

    offset = (ulong)i2c_p + ICH9_AUX_STS;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_AUX_STS);
    printf("Auxiliary Status       = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_AUX_CTL;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_AUX_CTL);
    printf("Auxiliary Control      = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_SMLINK_PIN_CTL;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_SMLINK_PIN_CTL);
    printf("SMLink Pin Control     = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_SMBUS_PIN_CTL;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_SMBUS_PIN_CTL);
    printf("SMBus Pin Control      = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_SLV_STS;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_SLV_STS);
    printf("Slave Status           = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_SLV_CMD;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_SLV_CMD);
    printf("Slave Command          = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_NOTIFY_DADDR;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_NOTIFY_DADDR);
    printf("Notify Device Address  = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_NOTIFY_DLOW;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_NOTIFY_DLOW);
    printf("Notify Data Low Byte   = 0x%02x @ %#lx\n", db, offset);

    offset = (ulong)i2c_p + ICH9_NOTIFY_DHIGH;
    db = ich9_smb_rd_b((ulong) i2c_p, ICH9_NOTIFY_DHIGH);
    printf("Notify Data High Byte  = 0x%02x @ %#lx\n", db, offset);

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	ich_i2c_reset
 *
 * Description:	API for Motherboard ICH9 I2C reset.
 *		ICH9 has a Soft SMBus Reset bit in the PCI Config register.
 *		IOS cleas the Host Status bits to reset the SMBus. We will
 *		reset ICH9, the clear the status.
 *
 * Inputs:	i2c_ctl - Not used.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_CTL_ERR - Unable to get the base address, or unable to 
 *				clear the error status bits.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
ich_i2c_reset(uint8_t i2c_ctl)
{
    ich9_smb_reg_t * i2c_p;
    uint8_t status;
/* #define ICH9_I2C_RESET_DEBUG  * */

    i2c_p = get_ich9_smb_base();

    if (i2c_p == NULL) {
#ifdef ICH9_I2C_RESET_DEBUG
	cterr('f', 0, "ich_i2c_reset() Unable to get ICH9 SMBus Base");
#endif /* ICH9_I2C_RESET_DEBUG */
	/* Note - Host status reset failure will not have Byte Done status bit
	 * been set. We use it here to differentiate it from the host status
	 * reset failure.
	 */
	assert(!"ich_i2c_reset() Cannot get Base address");
	return(E_I2C_INV_P);
    }

    /* Soft SMBus Reset */
    status = ICH9_SMB_HOSTC_SSRESET | ICH9_SMB_HOSTC_HST_EN;
    (void)ich9_smb_pci_wr_b(ICH9_SMB_HOSTC, status, TRUE);
    msleep(I2C_RESET_TIME);
    status = pci_config_read_byte(PCI_SMBUS_BUS, PCI_SMBUS_DEV, PCI_SMBUS_FUNC,
				  ICH9_SMB_HOSTC);

    if (status & ICH9_SMB_HOSTC_SSRESET) {
#ifdef ICH9_I2C_RESET_DEBUG
	cterr('f', 0, "ich_i2c_reset() SoftReset failed. HOSTC = %#x", status);
#endif /* ICH9_I2C_RESET_DEBUG */
	return(E_I2C_CTL_ERR);
    }

    /* Set to I2C type. Default to enable Host controller. */
    status = ICH9_SMB_HOSTC_HST_EN;
    (void)ich9_smb_pci_wr_b(ICH9_SMB_HOSTC, status, TRUE);

    /* Clear the error Status bits */
    status = ich9_smb_rd_b((ulong)i2c_p, ICH9_HST_STS);
    if (status) {
	/* Write back the status to clear */
	ich9_smb_wr_b((ulong)i2c_p, ICH9_HST_STS, status);
	/* Read it back and check error bits */
	status = ich9_smb_rd_b((ulong)i2c_p, ICH9_HST_STS) &
			      ICH9_HST_STS_RESET_MASK;

	if (status) {
#ifdef ICH9_I2C_RESET_DEBUG
	    cterr('f', 0, "ich_i2c_reset() Unable to reset ICH9 SMBus. Status "
			  "= %#x", status);
#endif /* ICH9_I2C_RESET_DEBUG */
	    return(E_I2C_CTL_ERR | ((uint32_t)status & 0xFF));
	} /* endof if 2nd status */
    } /* endof if 1st status */

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	ich_i2c_init
 *
 * Description:	Motherboard ICH9 I2C API for init.
 *
 * Inputs:	i2c_ctl - N2G_I2C_BUS in n2g_i2c.h. Not used.
 *		i2c_speed - Only 100 KHz supported.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_P - Invalid speed passed.
 *		E_I2C_CTL_ERR - Unable to reset the bus.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
ich_i2c_init(uint8_t i2c_ctl, char i2c_speed)
{
    ich9_smb_reg_t * i2c_p;
    uint8_t reg;

/* #define ICH9_I2C_INIT_DEBUG  * */

    switch(i2c_speed) {
    case N2G_I2C_100KHZ:
        break;
    default:
	/* Unsupported speed */
#ifdef ICH9_I2C_INIT_DEBUG
	cterr('f', 0, "ich_i2c_init() Unsupported I2C speed %#x", i2c_speed);
#endif /* ICH9_I2C_INIT_DEBUG */
	return(E_I2C_INV_P);
	break;
    } /* endof switch i2c_speed */


    /* Host status register is cleared in ich_i2c_reset(). Host Configuration
     * Register is also initialized at this point. Ready to initialize the
     * other registers.
     */
    i2c_p = get_ich9_smb_base();

    if (i2c_p == NULL) {
#ifdef ICH9_I2C_INIT_DEBUG
	cterr('f', 0, "ich_i2c_init() Unable to get ICH9 SMBus Base");
#endif /* ICH9_I2C_INIT_DEBUG */
	/* If ich_i2c_reset() can get the base, we should be able to do the
	 * same here. If not, this is programming error.
	 */
	assert(!"ich_i2c_init() Cannot get Base address");
	return(E_I2C_INV_P);
    }

    /* PEC disabled, interupt disabled */
    ich9_smb_wr_b((ulong)i2c_p, ICH9_HST_CNT, 0);

    /* Slave Address setup */
    ich9_smb_wr_b((ulong)i2c_p, ICH9_RCV_SLVA, ICH9_RCV_SLVA_DEFAULT);

    /* Clear Auxiliary Status */
    reg = ich9_smb_rd_b((ulong)i2c_p, ICH9_AUX_STS) & ICH9_AUX_STS_CRCE;
    ich9_smb_wr_b((ulong)i2c_p, ICH9_AUX_STS, reg);

    /* The following registers may cause the SMBus to DEV_ERR.  */
#ifdef ICH9_SPECIAL_INIT_SET
    /* Set Auxiliary Control to Enable 32-byte buffer, and disable automatic
     * append CRC
     */
    ich9_smb_wr_b((ulong)i2c_p, ICH9_AUX_CTL, ICH9_AUX_CTL_E32B);

    /* Set SMLink Pin Control to not overdrive low SMLINK0 */
    ich9_smb_wr_b((ulong)i2c_p, ICH9_SMLINK_PIN_CTL, ICH9_SMLINK_CLK_CTL);

    /* Set SMBus Pin Control to have ICH9 to drive SMBCLK pin low, since
     * ICH9 is the master.
     */
    ich9_smb_wr_b((ulong)i2c_p, ICH9_SMBUS_PIN_CTL, 0);

    /* Set Slave Command Register to allow SMBAlert interrupt generation,
     * Host Notify interrupts, and wakeup calls. Should not receive any.
     * But for Error Reporting, this will be useful.
     */
    ich9_smb_wr_b((ulong)i2c_p, ICH9_SLV_CMD, ICH9_HOST_NOTIFY_WKEN |
					     ICH9_HOST_NOTIFY_INTREN);
#endif /* ICH9_SPECIAL_INIT_SET */

    return(PASSED);
}
/*********************************************************************
 *
 * Function:	ich_i2c_read
 *
 * Description:	Motherboard ICH9 I2C Read API.
 *
 * Inputs:	dev_p	- Pointer to device characteristics table.
 *		offset	- I2C device offset.
 *		size	- Number of bytes to read.
 *		*buf	- Read buffer pointer.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_P   - Invalid slave address.
 *		E_I2C_CTL_ERR - I2C controller error.
 *		E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
ich_i2c_read (n2g_i2c_dev_t *dev_p, ulong offset, uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);
    
    if (fd_i2c0 > 0) {
        //        printf("setign address ich_i2c.c %d\n", __LINE__);
        if ((rc = ioctl(fd_i2c0, I2C_SLAVE, dev_p->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
			  dev_p->dev_addr, rc);
            return (FAILED);
        } else {
            dev_p->fp = fd_i2c0;
        }
    }

    rc = api_mb_i2c_read(dev_p, offset, size, buf);

    return (rc);
}

int32_t
i2c_dev_rd (void * p)
{
    n2g_i2c_if_t *i2c = (n2g_i2c_if_t *)p;
    
    int rc = FAILED;
    //   n2g_i2c_dev_t *dev_p, ulong offset, uint8_t size, char *buf)
    int fd_i2c0 = get_i2c_fd(0);
    //
    if (fd_i2c0 > 0) {
        if ((rc = ioctl(fd_i2c0, I2C_SLAVE, i2c->i2c_dev)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
			  i2c->i2c_dev, rc);
            return (FAILED);
        } 

    }

    switch (i2c->rd_hd_size) {
    case I2C_SMBUS_BLOCK_DATA:
        rc = i2c_smbus_read_block_data(fd_i2c0, (uint8_t)i2c->offset,
                                       (uchar *)i2c->buf);
            break;
    default:
        assert(!"invalid read type");
    }

    return (rc);
}

/*********************************************************************
 *
 * Function:	ich_i2c_write
 *
 * Description:	Motherboard ICH9 I2C Write API.
 *
 * Inputs:	dev_p  - Pointer to device characteristics table.
 *		offset - I2C device offset.
 *		size   - Number of bytes to write.
 *		*buf   - Write buffer pointer.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_P   - Invalid slave address.
 *		E_I2C_CTL_ERR - I2C write error.
 *		E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
ich_i2c_write (n2g_i2c_dev_t *dev_p, ulong offset,
                 uint8_t size, char *buf)
{
    int rc = FAILED;
    int fd_i2c0 = get_i2c_fd(0);

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
    rc = api_mb_i2c_write(dev_p, offset, size, buf);

    return (rc);
}

int32_t
i2c_dev_wr (void * p, uint8_t wr_size)
{
    n2g_i2c_if_t *i2c = (n2g_i2c_if_t *)p;

    int rc = FAILED;

    int fd_i2c0 = get_i2c_fd(0);
    if (fd_i2c0 > 0) {
        if ((rc = ioctl(fd_i2c0, I2C_SLAVE, i2c->i2c_dev)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c->i2c_dev, rc);
            return (FAILED);
        }

    }

    switch (i2c->wr_hd_size) {
    case I2C_SMBUS_BLOCK_DATA:
        rc = i2c_smbus_write_block_data(fd_i2c0, (uint8_t)i2c->offset,
                                        wr_size, (uchar *)i2c->buf);
        break;
    default:
        assert(!"invalid read type");
    }

    return (rc);
}


#if 0
/*********************************************************************
 *
 * Function:	wait_for_busy_off
 *
 * Description:	Wait for the I2C to be available
 *
 * Inputs:	i2c_p - Points to the base address of SMBus.
 *		timeout - Timeout counter.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
wait_for_busy_off(ich9_smb_reg_t *i2c_p, int timeout)
{
#ifdef SIMULATE_RW
    printf("wait_for_busy_off()\n");
    return(PASSED);
#else
    uint8_t stat;
    int i;
/* #define ICH9_I2C_WAIT_DEBUG   * */

    for (i = 0; i < timeout; i++) {
	stat = ich9_smb_rd_b((ulong)i2c_p, ICH9_HST_STS);
	if ((stat & ICH9_HST_STS_HOST_BUSY) == 0) {
#ifdef ICH9_I2C_WAIT_DEBUG
	    printf("wait_for_busy_off() count %d\n", i);
#endif /* ICH9_I2C_WAIT_DEBUG */
	    return(PASSED);
	}
	wastetime(N2G_I2C_BIT_DELAY);
    }

#ifdef ICH9_I2C_WAIT_DEBUG  /* */
    printf("wait_for_busy_off() Host Status = %#x after %d try\n", stat, i);
    for (i = 0; i < 1000; i++) {
	stat = ich9_smb_rd_b((ulong)i2c_p, ICH9_HST_STS);
	if ((stat & ICH9_HST_STS_HOST_BUSY) == 0) {
	    printf("wait_for_busy_off() count %d ms\n", i * N2G_I2C_BIT_DELAY);
	    return (PASSED);
	}
	wastetime(N2G_I2C_BIT_DELAY);
    }
    printf("wait_for_busy_off() Host Status = %#x after %d try\n", stat, i);

#endif /* ICH9_I2C_WAIT_DEBUG */

    return(FAILED);
#endif /* SIMULATE_RW */
}
#endif


/*********************************************************************
 *
 * Function:	get_ich9_smb_base
 *
 * Description:	Return the memory mapped base address of ICH9 SMBus
 *
 * Inputs:	None
 *
 * Outputs:	Base address of ICH9 SMBus. NULL if unable to get the address.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static ich9_smb_reg_t *
get_ich9_smb_base(void)
{
    uint32_t reg;
    ich9_smb_reg_t * ptr;
    int fd_i2c0 = get_i2c_fd(0);

    if (fd_i2c0 < 1) {
        assert(!"fd to fpga driver is not valid. fpga driver installed?");
    }

#ifdef ICH9_SMB_DEBUG
    printf("\n\nPCI_SMBUS_BUS = %d, PCI_SMBUS_DEV = %d,"
           " PCI_SMBUS_FUNC = %d, PCI_VENDOR_ID_OFFSET = %d.\n\n",
           PCI_SMBUS_BUS, PCI_SMBUS_DEV, PCI_SMBUS_FUNC, PCI_VENDOR_ID_OFFSET);
#endif /* ICH9_SMB_DEBUG */

    reg = pci_config_read(PCI_SMBUS_BUS, PCI_SMBUS_DEV, PCI_SMBUS_FUNC,
			  PCI_VENDOR_ID_OFFSET);

    if (reg != ICH9_SMB_ID) {
#ifdef ICH9_SMB_DEBUG
	cterr('f', 0, "get_ich9_smb_base() Invalid VID/DID. Expect %#x, "
		      "Read %#x", ICH9_SMB_ID, reg);
#endif /* ICH9_SMB_DEBUG */
	return((ich9_smb_reg_t *)NULL);
    }

#ifndef ICH9_IO_ACCESS
    ptr = (ich9_smb_reg_t *)(pci_config_read(PCI_SMBUS_BUS, PCI_SMBUS_DEV,
                                             PCI_SMBUS_FUNC, PCI_MEM_BAR0_OFFSET) &
                                             ICH9_SMB_SMBMBAR0_BA_M);
#else /* I/O access */
    ptr = (ich9_smb_reg_t *)(pci_config_read(PCI_SMBUS_BUS, PCI_SMBUS_DEV,
					PCI_SMBUS_FUNC, PCI_MEM_BAR4_OFFSET) &
					ICH9_SMB_SMB_BASE_BA_M);
#endif /* ICH9_IO_ACCESS */

#ifdef ICH9_SMB_DEBUG
    printf("\nICH9 SMB Base @ %#x\n", ptr);
    if (ptr == NULL) {
	cterr('f', 0, "%s: ICH9 Not Configured yet", __FUNCTION__);
    }
#endif /* ICH9_SMB_DEBUG */

    if (fd_i2c0 > 0) {
        ptr = (void *)mmap(NULL, 0x3000, (PROT_READ | PROT_WRITE),
                           MAP_SHARED, fd_i2c0, (2*getpagesize()));
    }
    if (ptr == MAP_FAILED) {
        close(fd_i2c0);
        perror("Error mmapping the file");
        exit(0);
    }

    return(ptr);
}

/*********************************************************************
 *
 * Function:	ich9_smb_rd_b
 *
 * Description:	Read a byte from SMB controller register.
 *
 * Inputs:	base - Base address.
 *		offset - Register offset.
 *
 * Outputs:	Data read.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static uchar
ich9_smb_rd_b(ulong base, ulong offset)
{
#ifdef ICH9_IO_ACCESS
    return(INB(base + offset);
#else /* Memory */
#ifdef ICH9_REG_RD_DEBUG
    printf("read byte from %#x\n", offset);
#ifdef BEAGLE_DEBUG
    while (getc_answer("(y/n) ? ", "yn", 'y') != 'y');
#endif /* BEAGLE DEBUG */
#endif /* ICH9_REG_RD_DEBUG */
    return(*(volatile uchar *)(base + offset));
#endif /* ICH9_IO_ACCESS */

}

/*********************************************************************
 *
 * Function:	ich9_smb_wr_b
 *
 * Description:	Write byte to SMB controller register.
 *
 * Inputs:	base - Base address.
 *		offset - Register offset.
 *		data - Data to be written.
 *
 * Outputs:	None
 *
 * Assumptions:
 *
 *********************************************************************
 */
static void
ich9_smb_wr_b (ulong base, ulong offset, uchar data)
{
#ifdef ICH9_IO_ACCESS
    OUTB(base + offset, data);
#else /* Memory */
#ifdef ICH9_REG_WR_DEBUG
    printf("write byte %#x to %#x\n", data, offset);
#ifdef BEAGLE_DEBUG
    while (getc_answer("(y/n) ? ", "yn", 'y') != 'y');
#endif /* BEAGLE DEBUG */
#endif /* ICH9_REG_WR_DEBUG */
    *(volatile uchar *)(base + offset) = data;
    flush_io_wb();
#endif /* ICH9_IO_ACCESS */

}

/*********************************************************************
 *
 * Function:	ich9_smb_rd_w
 *
 * Description:	Read 2 bytes from SMB controller register.
 *
 * Inputs:	base - Base address.
 *		offset - Register offset.
 *
 * Outputs:	Data read.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static uint16_t
ich9_smb_rd_w(ulong base, ulong offset)
{
#ifdef ICH9_IO_ACCESS
    return(INW(base + offset);
#else /* Memory */
#ifdef ICH9_REG_RD_DEBUG
    printf("read word from %#x\n", offset);
#endif /* ICH9_REG_RD_DEBUG */
    return(*(volatile uint16_t *)(base + offset));
#endif /* ICH9_IO_ACCESS */

}

/*********************************************************************
 *
 * Function:	ich9_smb_pci_wr_b
 *
 * Description:	PCIe config write of one bytes.
 *
 * Inputs:	offset - PCIe config offset.
 *		data - Data to be written.
 *		check - Read verify.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
ich9_smb_pci_wr_b(ulong offset, uint8_t data, uint check)
{
    uint8_t rd_data;

    pci_config_write_byte(PCI_SMBUS_BUS, PCI_SMBUS_DEV, PCI_SMBUS_FUNC,
			  offset, data);

    if (check == TRUE) {
	rd_data = pci_config_read_byte(PCI_SMBUS_BUS, PCI_SMBUS_DEV,
				       PCI_SMBUS_FUNC, offset);
	if (rd_data != data) {
	    return(FAILED);
	}
    }

    return(PASSED);
}


#if 0
/*********************************************************************
 *
 * Function:	ich9_i2c_clear_stat
 *
 * Description:	Flush out pending I2C I/O by read/write the status register.
 *
 * Input:	base - Base address.
 *		retry - Max retry count to prevent hang.
 *
 * Output:	PASSED/E_I2C_CTL_ERR
 *
 *********************************************************************
 */
static uint32_t
ich9_i2c_clear_stat(ulong base, int retry)
{
    int i;
    uint8_t stat;
/* #define ICH9_I2C_CLRSTAT_DEBUG  * */

    for (i = 0; i < retry; i++) {
	stat = ich9_smb_rd_b(base, ICH9_HST_STS);
	if (stat & (~ICH9_HST_STS_INUSE)) {
#ifdef ICH9_I2C_CLRSTAT_DEBUG   /* */
	    printf("%d'th byte stat not cleared. stat = %#x\n", i, stat);
#endif /* ICH9_I2C_CLRSTAT_DEBUG */
	    ich9_smb_wr_b(base, ICH9_HST_STS, stat);
	} else {
	    /* Cleared */
	    return(PASSED);
	} /* endof if stat */
    } /* endof for */

    /* If out of the retry count, exit with failed status */
#ifdef ICH9_I2C_CLRSTAT_DEBUG /* */
    printf("%s Cannot clear in use status after %d retries. status = %#x\n",
	   __FUNCTION__, i, stat);
#endif /* ICH9_I2C_CLRSTAT_DEBUG */
    return(E_I2C_CTL_ERR | ((uint32_t)stat & 0xFF));
}

/*********************************************************************
 *
 * Function:	ich9_i2c_get_block_data
 *
 * Description:	Read a block of data.
 *
 * Input:	base - Base address.
 *		size - Number of bytes in the block.
 *		*buf - Points to the data buffer.
 *
 * Output:	PASSED/E_I2C_STAT_TO
 *
 *********************************************************************
 */
static uint32_t
ich9_i2c_get_block_data(ulong base, uint size, char * buf)
{
    uint i, cnt;
    uint8_t hst_bd, stat, hst_cnt;

#ifdef ICH9_I2C_READ_DEBUG
    printf("Ready to read %d bytes\n", size);
#endif /* ICH9_I2C_READ_DEBUG */
    for (i = 0; i < size; i++) {
	for (cnt = 0; cnt < I2C_DATA_BYTE_XMIT_TIME; cnt++) {
	    stat = ich9_smb_rd_b(base, ICH9_HST_STS);
	    if (stat & ICH9_HST_STS_DS) {
		/* Byte Done */
		break;
	    }
	    wastetime(N2G_I2C_BIT_DELAY); /* @100 KHz - 10 us per bit */
	} /* endof for cnt */

	if (cnt == I2C_DATA_BYTE_XMIT_TIME) {
/* #define ICH9_I2C_READ_DEBUG   * */
#ifdef ICH9_I2C_READ_DEBUG_ERROR
	    cterr('f', 0, "Block read timed-out. stat = %#x", stat);
#endif /* ICH9_I2C_READ_DEBUG_ERROR */
	    return(E_I2C_STAT_TO);
	}

	hst_bd = ich9_smb_rd_b(base, ICH9_HOST_BLOCK_DB);
	*buf++ = hst_bd;

	/* Clear status to get next byte */
	/* Refer to HST_STS Byte Done Status (DS) "Note: ... Software must
	 * clear the DS bit before it can clear the BUSY bit."
	 */
	ich9_smb_wr_b(base, ICH9_HST_STS, ICH9_HST_STS_DS);
	ich9_smb_wr_b(base, ICH9_HST_STS, ICH9_HST_STS_HOST_BUSY);

	ich9_smb_wr_b(base, ICH9_HST_STS, (stat & (~(ICH9_HST_STS_DS |
						     ICH9_HST_STS_HOST_BUSY))));

	/* Check for last byte */
	if (i >= (size - 2)) {
	    /* Last byte */
	    hst_cnt = ich9_smb_rd_b(base, ICH9_HST_CNT);
	    hst_cnt |= ICH9_HST_CNT_LAST_BYTE;
	    ich9_smb_wr_b(base, ICH9_HST_CNT, hst_cnt);
	} /* endof if i */
#ifdef ICH9_I2C_READ_DEBUG
	printf("%d'th data = %#x. stat = %#x\n", i, hst_bd, stat);
#endif /* ICH9_I2C_READ_DEBUG */
	wastetime(I2C_DATA_BYTE_XMIT_TIME - ONE); /* Wait for the byte xfer */
    } /* endof for */

    return(PASSED);
}
#endif


/*********************************************************************
 *
 * Function:	retry_ich_i2c_read
 *
 * Description:	Motherboard ICH9 I2C Read API with retry.
 *
 * Inputs:	dev_p  - Pointer to device characteristics table.
 *		offset - I2C device offset.
 *		size   - Number of bytes to write.
 *		*buf   - Write buffer pointer.
 *              retry_max  - number of retry
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_P   - Invalid slave address.
 *		E_I2C_CTL_ERR - I2C write error.
 *		E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
retry_ich_i2c_read (n2g_i2c_dev_t *dev_p, ulong offset, uint8_t size, 
	     char *buf, uint8_t retry_max)
{
    uint32_t rc = FAIL;
    uint8_t retry = 0;

    rc = ich_i2c_read(dev_p, offset, size, buf);
    while((rc != PASS) && (retry < retry_max)) {
        ich_i2c_init(CPU_I2C1, N2G_I2C_100KHZ);
	msleep(1000);
	rc = ich_i2c_read(dev_p, offset, size, buf);
	retry++;
    }
    return(rc);
}

/*********************************************************************
 *
 * Function:	retry_ich_i2c_write
 *
 * Description:	Motherboard ICH9 I2C Write API with retry.
 *
 * Inputs:	dev_p  - Pointer to device characteristics table.
 *		offset - I2C device offset.
 *		size   - Number of bytes to write.
 *		*buf   - Write buffer pointer.
 *              retry_max  - number of retry
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_INV_P   - Invalid slave address.
 *		E_I2C_CTL_ERR - I2C write error.
 *		E_I2C_STAT_TO - Status read timeout.
 *
 * Assumptions:
 *
 *********************************************************************
 */
uint32_t
retry_ich_i2c_write(n2g_i2c_dev_t *dev_p, ulong offset,
                 uint8_t size, char *buf, uint8_t retry_max)
{
    uint32_t rc = FAIL;
    uint8_t retry = 0;

    rc = ich_i2c_write(dev_p, offset, size, buf);
    while((rc != PASS) && (retry < retry_max)) {
        ich_i2c_init(CPU_I2C1, N2G_I2C_100KHZ);
	msleep(1000);
	rc = ich_i2c_write(dev_p, offset, size, buf);
	retry++;
    }
    return(rc);
}


/*------------------------------------------------------------------
$Log: ich9_i2c.c,v $
Revision 1.2  2013/11/26 08:40:35  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.4  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/

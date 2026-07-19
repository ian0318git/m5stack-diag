/* $Id: p1021_i2c.c,v 1.1 2014/03/25 02:12:33 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: p1021_i2c.c
 *
 * Description: I2C drivers for p1021
 *
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "patriot_main.h"
#include "p1021_immap.h"

uint32_t p1021_i2c_read_fpga_byte(uint32_t, volatile uchar *);
uint32_t p1021_i2c_write_fpga_byte(uint32_t, uchar);
static uint32_t send_i2c_stop(ccsr_i2c_t *i2c_ptr);

/*********************************************************************
 *
 * Function:	p1021_i2c_read
 *
 * Description:	Read from I2C register.
 *
 * Inputs:	*reg - Points to the I2C register.
 *
 * Outputs:	Register read.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static inline uint8_t
p1021_i2c_read(volatile uint8_t *reg)
{
    uint8_t value;

    value = *reg;
    asm volatile ("isync");
    asm volatile ("msync");
    return(value);
}

/*********************************************************************
 *
 * Function:	p1021_i2c_write
 *
 * Description:	Write to I2C register.
 *
 * Inputs:	value - data to be written.
 *		*reg - Points to I2C register to be written.
 *
 * Outputs:	None.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static inline void
p1021_i2c_write(uint8_t *reg, uint8_t value)
{

    /* Write the register. */
    *reg = value;
    asm volatile ("isync");
    asm volatile ("msync");

}


/*********************************************************************
 *
 * Function:	get_i2c_ctl_addr
 *
 * Description:	Get the I2C controller address based on the I2C bus number.
 *
 * Inputs:	i2c_ctl - I2C bust number
 *
 * Outputs:	I2C controller address.
 *
 * Assumptions:	Device type (i2c_ctl) must be a valid type.
 *
 *********************************************************************
 */
static ccsr_i2c_t *
get_i2c_ctl_addr(uint8_t i2c_ctl)
{
    /* Decide which I2C based on the I2C bus number */
    if (i2c_ctl == CPU_I2C0) {
        /* I2C0 */
        return(&REGB->im_i2c1);
    } else {
        /* I2C1 */
        return(&REGB->im_i2c2);
    }

}



/**************************************************************************
 *
 * Name: p1021_i2c_init
 *
 * Description: Initializes I2C controller
 *
 * Inputs: i2c_ctl - i2c Bus Number
 *
 * Output: none
 *
 *************************************************************************/
void
p1021_i2c_init (uint8_t i2c_ctl)
{
    ccsr_i2c_t *i2c_ptr; /* pointer to I2C controller */
    uint8_t i2ccr, ccr, stat, buf;
    uint32_t offset = 0;
    
    /* Decide which I2C based on the I2C bus number */
    i2c_ptr = get_i2c_ctl_addr(i2c_ctl);
#ifdef DEBUG
    printf("\ni2c ctl addr = 0x%08x\n", i2c_ptr);
#endif
    /* Reset I2C first */
    i2ccr = p1021_i2c_read(&i2c_ptr->i2ccr);	/* Read I2CCR */
    i2ccr &= ~(MPC8500_I2CCR_MEN + MPC8500_I2CCR_MIEN + MPC8500_I2CCR_BCST);
    
    /* Write the Control register to reset I2C. */
    p1021_i2c_write(&i2c_ptr->i2ccr, i2ccr);
    usleep(100);

    /* Set the Slave Adress */
    p1021_i2c_write(&i2c_ptr->i2cadr, MB_I2C_ADDR_CTRL);
    
    /* Setup the I2C clock frequency at 100KHz*/
    p1021_i2c_write(&i2c_ptr->i2cfdr, 0x2D);

    /* Setup the sampling rate */
    p1021_i2c_write(&i2c_ptr->i2cdfsrr, PQ_I2CDFSRR);

    /* Enable the I2C controller */
    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);		/* Read I2CCR */
    p1021_i2c_write(&i2c_ptr->i2ccr, ccr | MPC8500_I2CCR_MEN);

    /* Perform a dummy read to clear out I2C bus */
    p1021_i2c_read_fpga_byte(offset, &buf);
    
    return;

}


/**********************************************************************
 *
 * Function:    platform_cpu_i2c_init
 *
 * Description: Initialize CPU I2C bus controllers.
 *
 * Inputs:      None.
 *
 * Outputs:     None.
 *
 **********************************************************************
 */
void platform_cpu_i2c_init(void)
{
    int i;

    for (i = CPU_I2C0; i <= CPU_I2C1; i++) {
        /* CPU I2C controller uses 100 KHz */
        p1021_i2c_init(i);
    }
    return;
}



/*********************************************************************
 *
 * Function:	is_i2c_busy
 *
 * Description:	Check if CPU I2C controller is busy.
 *
 * Inputs:	*i2c_ptr - Points to I2C controller base.
 *
 * Outputs:	FALSE - Not busy.
 *		None zero - Busy with the status returned.
 *
 *
 *********************************************************************
 */
static uint32_t
is_i2c_busy(ccsr_i2c_t *i2c_ptr)
{
    uint8_t stat;

    /* Read the status register. */
    stat = p1021_i2c_read(&i2c_ptr->i2csr);
#ifdef DEBUG
    printf("\nFunction: %s[#%d], stat = 0x%02x\n", __FUNCTION__, __LINE__, stat);
#endif
    if (stat & MPC8500_I2CSR_MBB) {
        /* I2C bus is busy, or byte transfer is in progress */
        return((uint32_t)stat);
    } else {
        /* I2C bus is not busy and byte transfer is complete */
        return(FALSE);
    }
}


/*********************************************************************
 *
 * Function:	send_i2c_start
 *
 * Description:	Send I2C Start bit.
 *
 * Inputs:	*i2c_ptr - Points to PowerQUICC I2C controller base.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_CTL_ERR - Start already done.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static uint32_t
send_i2c_start(ccsr_i2c_t *i2c_ptr)
{
    uint8_t ccr, csr;

    /* Clear MAL and MIF */
    csr = p1021_i2c_read(&i2c_ptr->i2csr);
#ifdef DEBUG
    printf("\nFunction: %s[#%d], csr before write = 0x%02x \n", __FUNCTION__,
	   __LINE__, csr);
#endif
    p1021_i2c_write(&i2c_ptr->i2csr, csr & (~(MPC8500_I2CSR_MAL |
                    MPC8500_I2CSR_MIF)));
#ifdef DEBUG
    csr = p1021_i2c_read(&i2c_ptr->i2csr);
    printf("\nFunction: %s[#%d], csr after write = 0x%02x \n", __FUNCTION__,
	   __LINE__, csr);
#endif
    /* Get the Control Register */
    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);

    if (ccr & MPC8500_I2CCR_MSTA) {
        /* Start already set */
        return(ccr);
    }

    ccr &= (~MPC8500_I2CCR_TXAK); /* Clear TXAK */

    /* Send out the Start bit */
    p1021_i2c_write(&i2c_ptr->i2ccr, ccr | MPC8500_I2CCR_MSTA);
#ifdef DEBUG
    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);
    printf("\nFunction: %s[#%d], ccr after write = 0x%02x \n", __FUNCTION__,
	   __LINE__, ccr);
#endif
    return(PASSED);
}

/*********************************************************************
 *
 * Function:	set_txrx_mode
 *
 * Description:	Set I2C Control register to transmit or receive.
 *
 * Inputs:	*i2c_ptr - Points to PowerQUICC I2C controller base.
 *              mode     - read or write.
 *
 * Outputs:	None.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static void
set_txrx_mode(ccsr_i2c_t *i2c_ptr, uint8_t mode)
{
    uint8_t ccr;

    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);	/* Get the control register */
#ifdef DEBUG
    printf("\nFunction: %s[#%d], ccr before write = 0x%02x \n", __FUNCTION__,
	   __LINE__, ccr);
#endif
    if (mode == I2C_READ_COMMAND) {
        /* Receive mode */
        p1021_i2c_write(&i2c_ptr->i2ccr, ccr & (~MPC8500_I2CCR_MTX));
    } else {
        /* Transmit mode */
        p1021_i2c_write(&i2c_ptr->i2ccr, ccr | MPC8500_I2CCR_MTX);
    }
#ifdef DEBUG
    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);	/* Get the control register */
    printf("\nFunction: %s[#%d], ccr after write = 0x%02x \n", __FUNCTION__,
	   __LINE__, ccr);
#endif

}



/*********************************************************************
 *
 * Function:	poll_i2c_rx_stat
 *
 * Description:	Check if a byte is received by the I2C controller.
 *
 * Inputs:    *i2c_ptr - Points to PowerQUICC I2C controller base.
 *
 * Outputs: PASSED        - No errors encounterd.
 *          E_I2C_CTL_ERR - I2C read error.
 *          stat          - return status
 *
 * Assumptions: PowerQUICC I2C controller is the only master. If not,
 *              then we need to check MBB in the status register.
 *
 *********************************************************************
 */
static uint32_t
poll_i2c_rx_stat(ccsr_i2c_t *i2c_ptr)
{
    uint8_t stat;
    uint i;

    /* Poll the status bits */
    for (i = 0; i < I2C_POLL_DATA_TIMEOUT; i++) {
        /* read the status */
        stat = p1021_i2c_read(&i2c_ptr->i2csr);
        if (stat & MPC8500_I2CSR_MAL) {
            /* Arbitration is lost */
        } else {
            /* still under arbitration */
            if (stat & MPC8500_I2CSR_MIF) {
                /* Receive complete. Get the data */
                /* Clear MIF */
                p1021_i2c_write(&i2c_ptr->i2csr, stat & (~MPC8500_I2CSR_MIF));
                return (PASSED);
            } else {
                /* Byte transfer in progress */
		wastetime (I2C_STAT_WAIT_TIME);
                continue;
            } /* MCF */
        } /* MAL */
        /* Error status found */
        return (stat);
    }

    return (E_I2C_TIMEDOUT);
}


/*********************************************************************
 *
 * Function:	get_i2c_data_byte
 *
 * Description:	Receive one I2C data byte.
 *
 * Inputs: *i2c_ptr - Points to PowerQUICC I2C controller base.
 *         *buf     - Points to data buffer for the received data.
 *
 * Outputs: PASSED        - No errors encounterd.
 *          E_I2C_CTL_ERR - I2C read error.
 *
 * Assumptions: PowerQUICC I2C controller is the only master. If not,
 *              then we need to check MBB in the status register.
 *
 *********************************************************************
 */
static uint32_t
get_i2c_data_byte(ccsr_i2c_t *i2c_ptr, volatile uchar *buf)
{
    uint32_t rc;

    rc = poll_i2c_rx_stat(i2c_ptr);

    if (rc == PASSED) {
        /* Got the data */
        *buf = p1021_i2c_read(&i2c_ptr->i2cdr);
    }

    return (rc);
}


/*********************************************************************
 *
 * Function:	send_i2c_data_byte
 *
 * Description:	Transmit one I2C data byte.
 *
 * Inputs: *i2c_ptr - Points to PowerQUICC I2C controller base.
 *         data     - Data byte to be transmitted.
 *         start    - First byte after Start condition.
 *         mif      - Check for MIF bit.
 *
 * Outputs: PASSED        - No errors encounterd.
 *          E_I2C_CTL_ERR - I2C write error.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static uint32_t
send_i2c_data_byte(ccsr_i2c_t *i2c_ptr, uchar data,
                   boolean start, boolean mif)
{
    uchar stat, exp_stat, i;
#ifdef DEBUG
    printf("\nFunction: %s[#%d], data:0x%02x, start = %d \n", __FUNCTION__,
	   __LINE__, data, start);
#endif
    /* Check the status of the I2C */
    stat = p1021_i2c_read(&i2c_ptr->i2csr);
#ifdef DEBUG
    printf("\nFunction: %s[#%d], stat(i2csr) =0x%02x \n", __FUNCTION__, __LINE__,
	   stat);
#endif    
    if (mif) {
        exp_stat = MPC8500_I2CSR_MCF | MPC8500_I2CSR_MIF;
    } else {
        exp_stat = MPC8500_I2CSR_MCF;
    }
#ifdef DEBUG
    printf("\nexp_stat = 0x%02x", exp_stat);
#endif

    if (start != TRUE) {	
        /* Not first byte after Start bit. The I2C bus should be busy */
        if (!(stat & MPC8500_I2CSR_MBB)) {
            /* I2C bus not busy. */
            printf("\nFunction : %s(#%d), I2C Bus Not Busy.\n", __FUNCTION__,
		   __LINE__);
            return (stat);
        } /* MBB */
    } /* start */

    if ((start == TRUE) || ((stat & exp_stat) == exp_stat)) {
        /* Byte transfer is complete */
        if (stat & MPC8500_I2CSR_MAL) {
            /* Arbitration is lost */
            printf("\nFunction : %s(#%d), Arbitration is lost.\n", __FUNCTION__,
		   __LINE__);
            return (stat);
        } else {
            /* Ready to transmit */
            p1021_i2c_write(&i2c_ptr->i2cdr, data);
        }
    } else {
        /* Byte transfer in progress or Stop condition */
        printf("\nFunction:%s(#%d),Byte transfer in progress or Stop condition\n",
	       __FUNCTION__, __LINE__);
        return (stat);
    }

    wastetime (I2C_DATA_BYTE_XMIT_TIME);
    return (PASSED);

}

/*********************************************************************
 *
 * Function:  check_i2c_xmit_status
 *
 * Description: Check I2C status after transmit a byte.
 *
 * Inputs: *i2c_ptr - Points to PowerQUICC I2C controller base.
 *         ack      - ACK or NACK or either.
 *
 * Outputs: PASSED         - No errors encounterd.
 *          E_I2C_CTL_ERR  - I2C status.
 *          E_I2C_TIMEDOUT - time out.
 *
 * Assumptions: PowerQUICC I2C controller is the only master. If not,
 *              then we need to check MBB in the status register.
 *
 *********************************************************************
 */
static uint32_t
check_i2c_xmit_status(ccsr_i2c_t *i2c_ptr, uint8_t ack)
{
    uint8_t stat;
    uint i;

    /* Poll the status bits */
    for (i = 0; i < I2C_POLL_STAT_TIMEOUT; i++) {
        /* read the status */
        stat = p1021_i2c_read(&i2c_ptr->i2csr);
        if (stat & MPC8500_I2CSR_MAL) {
            /* Arbitration is lost */
        } else {
            /* Still under arbitration */
            if ((stat & (MPC8500_I2CSR_MCF | MPC8500_I2CSR_MIF)) ==
                (MPC8500_I2CSR_MCF | MPC8500_I2CSR_MIF)) {
                /* Transmit complete */
                /* Check for ACK or NACK */
                if ((ack & I2C_NACK) && (stat & MPC8500_I2CSR_RXAK)) {
                    /* Expecting NACK and got NACK */
                    return (PASSED);
                } else {
                    if ((ack & I2C_ACK) && (!(stat & MPC8500_I2CSR_RXAK))) {
                        /* Expecting ACK and got ACK */
                        return (PASSED);
                    } else {
                        /* Unexpected ACK or NACK */
                        return (E_I2C_INV_ACK);
                    } /* I2C_ACK */
                } /* I2C_NACK */
            } else {
                /* Byte transfer in progress */
		wastetime (I2C_STAT_WAIT_TIME);
                continue;
            } /* MCF */
        } /* MAL */
        /* Error status found */
        return (stat);
    } /* for */

    return (E_I2C_TIMEDOUT);
}


/*********************************************************************
 *
 * Function: send_i2c_slave_addr
 *
 * Description: Send I2C slave address.
 *
 * Inputs: *i2c_ptr - Points to PowerQUICC I2C controller base.
 *         dev_addr - I2C slave device address.
 *         command - read/write operation.
 *         start - First byte after Start condition.
 *
 * Outputs: PASSED    - No errors encounterd.
 *          None zero - Not PASSED with status returned.
 *
 * Assumptions: Assume I2C is not busy.
 *
 *********************************************************************
 */
static uint32_t
send_i2c_slave_addr(ccsr_i2c_t *i2c_ptr, uint8_t dev_addr, uint8_t command,
                    boolean start)
{
    uint32_t rc;
    boolean mif;
    uchar stat;
#ifdef DEBUG
    printf("\n%s command = %d, start = %d", __FUNCTION__, command, start);
#endif
    /* Set to write mode */
    set_txrx_mode(i2c_ptr, I2C_WRITE_COMMAND);

    if (start == TRUE) {
#ifdef DEBUG
        printf("\nStart, mif = FALSE \n");
#endif
        /* If first byte after start, then don't expect MIF */
        mif = FALSE;
    } else {
#ifdef DEBUG
        printf("\nNon Start, mif = TRUE \n");
#endif
        /* Expect MIF if not first byte */
        mif = TRUE;
    }
    
#ifdef DEBUG
    printf("\nDev_addr = %#x", dev_addr);
    stat = p1021_i2c_read(&i2c_ptr->i2csr);
    printf("\nFunction: %s[#%d], stat(i2csr)=0x%02x, start=%d, command=0x%02x \n",
	   __FUNCTION__, __LINE__, stat, start, command);
#endif
    /* Send the address and command bit */
    rc = send_i2c_data_byte(i2c_ptr, dev_addr << I2C_SLAVE_ADDR_SHIFT |
                            command, start, mif);

    if (rc != PASSED) {
        /* Unable to send slave address */
        printf("\nFunction : %s(#%d), send i2c data byte failed rc = 0x%02x\n",
	       __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    /* Return the transmit status */
    return (check_i2c_xmit_status(i2c_ptr, I2C_ACK));

}


/*********************************************************************
 *
 * Function: send_i2c_offset
 *
 * Description: Motherboard I2C (TWSI) Read/Write with HLC enabled.
 *
 * Inputs: *i2c_ptr - Points to PowerQUICC I2C controller base.
 *         offset   - I2C device offset, or command.
 *         ack      - Expecting ACK or NACK or either.
 *
 * Outputs: PASSED    - No errors encounterd.
 *          None zero - Not PASSED with status returned.
 *
 * Assumptions: Assume the I2C is "free" - not busy with other operations.
 *
 *********************************************************************
 */
static uint32_t
send_i2c_offset(ccsr_i2c_t *i2c_ptr, uint32_t offset,
                uint8_t ack)
{
    uint32_t rc;
    uint8_t off_byte;
    uint i;

    /* Offset is on 32 bits boundary. Adjust it to point to the first byte */
    offset <<= ((sizeof(uint32_t) - 1) * 8);

    off_byte = (uint8_t) (offset >> 24);
    /* Send the offset/command byte */
    rc = send_i2c_data_byte(i2c_ptr, off_byte, FALSE, TRUE);
    if (rc == PASSED) {
        /* Check if sent successfully */
        rc = check_i2c_xmit_status(i2c_ptr, ack);
    }
    if (rc != PASSED) {
        /* transmission failed */
        return (rc);
    }

    return (PASSED);
}


/*********************************************************************
 *
 * Function:	send_i2c_restart
 *
 * Description:	Send I2C Repeat Start bit.
 *
 * Inputs:	*i2c_ptr - Points to PowerQUICC I2C controller base.
 *
 * Outputs:	PASSED - No errors encounterd.
 *		E_I2C_CTL_ERR - Start bit not sent yet, cannot repeat.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static uint32_t
send_i2c_restart(ccsr_i2c_t *i2c_ptr)
{
    uint8_t ccr;

    /* Get the Control Register */
    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);

    if ((ccr & MPC8500_I2CCR_MSTA) != MPC8500_I2CCR_MSTA) {
        /* Start not set yet */
        return (ccr);
    }

    /* Send out the repeat Start bit */
    p1021_i2c_write(&i2c_ptr->i2ccr, ccr | MPC8500_I2CCR_RSTA);

    wastetime (N2G_I2C_BIT_DELAY);
    return (PASSED);
}

/*********************************************************************
 *
 * Function: set_rx_ack
 *
 * Description: Set ACK or NACK for master receive.
 *
 * Inputs:  *i2c_ptr - Points to PowerQUICC I2C controller base.
 *          ack      - ACK or NACK.
 *
 * Outputs: None.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static void
set_rx_ack(ccsr_i2c_t *i2c_ptr, uint8_t ack)
{
    uint8_t ccr;

    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);	/* Get the control register */

    if (ack == I2C_ACK) {
        /* ACK */
        p1021_i2c_write(&i2c_ptr->i2ccr, ccr & (~MPC8500_I2CCR_TXAK));
    } else {
        /* NACK */
        p1021_i2c_write(&i2c_ptr->i2ccr, ccr | MPC8500_I2CCR_TXAK);
    }
}

/*********************************************************************
 *
 * Function: send_i2c_stop
 *
 * Description: Send I2C Stop bit.
 *
 * Inputs: *i2c_ptr - Points to PowerQUICC I2C controller base.
 *
 * Outputs: PASSED        - No errors encounterd.
 *          E_I2C_CTL_ERR - Start bit not sent yet. cannot stop.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static uint32_t
send_i2c_stop(ccsr_i2c_t *i2c_ptr)
{
    uint8_t ccr;

    /* Get the Control Register */
    ccr = p1021_i2c_read(&i2c_ptr->i2ccr);

    if ((ccr & MPC8500_I2CCR_MSTA) != MPC8500_I2CCR_MSTA) {
        /* Start not set yet */
        return (ccr);
    }

    /* Clear Start bit to send Stop bit */
    ccr &= ~(MPC8500_I2CCR_MSTA);
    p1021_i2c_write(&i2c_ptr->i2ccr, ccr);

    /* Add a bit (10 microseconds for 100 KHz bus) delay on the I2C bus.
     * Without the delay, the controller disable and re-enable will prevent
     * the Stop bit been observed on the bus. This can cause some I2C devices
     * (eg. Environmental control unit or Power sequencer in Xformers) I2C
     * state machine in the wrong state.
     */

    wastetime (I2C_POST_STOP_DELAY);
    return (PASSED);
}


/**************************************************************************
 *
 * Name: p1021_i2c_read_bytes
 *
 * Description: Reads number of bytes from an I2C device
 *
 * Inputs: dev_addr    - i2c device address
 *         offset - i2c device offset
 *         size - number of bytes to read
 *         buf - pointer to buffer to read to
 *         bus_no - bus number
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************/
int
p1021_i2c_read_bytes (uchar dev_addr, uint32_t offset, uint8_t size,
                       volatile uchar *buf, uchar bus_no)
{

    ccsr_i2c_t *i2c_ptr; /* pointer to I2C controller */
    uint32_t rc = FAILED;
    uint i;
    volatile uchar *obuf;
    boolean first = TRUE;   /* First byte after Start */
    uchar stat;
    
    
    /* Get the pointer to the I2C controller */
    i2c_ptr = get_i2c_ctl_addr(bus_no);
   
#ifdef DEBUG
    printf("\nFunction: %s[#%d], i2c_ptr :0x%08x \n", __FUNCTION__, __LINE__,
	   i2c_ptr);
#endif
    /* Check if the controller is busy */
    if ((rc = is_i2c_busy(i2c_ptr)) != FALSE) {
        printf("\nI2C Controller is busy\n");
        return (rc);
    }

    /* Send out Start condition */
    rc = send_i2c_start(i2c_ptr);

    if (rc != PASSED) {
        /* Unable to send out start */
        printf("\nSend out start failed(#%d)\n", __LINE__);
        return (rc);
    }

#if DEBUG    
    /* Check if the controller is busy */
    if ((rc = is_i2c_busy(i2c_ptr)) != FALSE) {
        printf("\nI2C Controller is busy\n");
        return (rc);
    }
    printf("\n%s %d: rc = 0x%02x\n", __FUNCTION__, __LINE__, rc);
#endif

    /* Send out the address.
     * If the device does not have register or command,
     * send out the slave address with read.
     * Otherwise, send out the slave address with write
     */
    
    rc = send_i2c_slave_addr(i2c_ptr, dev_addr, I2C_WRITE_COMMAND, first);
#ifdef DEBUG    
    printf("\nd %d: rc = 0x%02x", __LINE__, rc);
#endif    
    first = FALSE;       /* first byte after Start is sent */
    if (rc == PASSED) {
        /* Send out the register/command */
        rc = send_i2c_offset(i2c_ptr, offset,
                             I2C_ACK);
        if (rc == PASSED) {
            /* Send out repeat start */
            rc = send_i2c_restart(i2c_ptr);
        } else {
	    printf("\nFailed to send out the register/command, rc = 0x%02x\n",
		   rc);
	}
    } else {
        if (rc == E_I2C_INV_ACK) {
            /* NACK. Device not present */
            printf("\nDevice not present(#%d)\n", __LINE__);
        } else {
	    printf("\nFailed to send slave addr, rc = 0x%02x\n",
		   rc);
	}
    }
#ifdef DEBUG    
    printf("\nd %d: rc = 0x%02x", __LINE__, rc);
#endif

    if (rc == PASSED) {
        /* Read command */
        rc = send_i2c_slave_addr(i2c_ptr, dev_addr, I2C_READ_COMMAND, first);

        if (rc == E_I2C_INV_ACK) {
            /* NACK. Slave device not present */
            printf("\nSlave device not present(#%d)\n", __LINE__);
        } else {
            /* Refer to 7572 User Manual under "Generation of STOP", second
             * paragraph -
             * "For 1-byte transfer, a dummy read should be performed by the
             * interrupt service routine ...".
             */
            if (size == 1) {
                /* Set TXAK for the dummy to indicate next to be the last */
                set_rx_ack(i2c_ptr, I2C_NACK);
            } else {
                set_rx_ack(i2c_ptr, I2C_ACK);
            }

            /* Set to read mode */
            set_txrx_mode(i2c_ptr, I2C_READ_COMMAND);

            /* Dummy read */
            get_i2c_data_byte(i2c_ptr, buf);
        } /* enof if rc */
    } else {
	printf("\n%s: %d, Failed to send slave addr rc = 0x%08x\n",
	       __FUNCTION__, __LINE__, rc);
    }

    obuf = buf;
#ifdef DEBUG    
    printf("\nd %d: rc = 0x%02x", __LINE__, rc);
#endif    
    if (rc == PASSED) {
        /* Read the data */
        for (i = 0; i < size; i++, buf++) {

            if (i == (size - 1)) {
                /* Last byte. Check if data is in */
                rc = poll_i2c_rx_stat(i2c_ptr);
                if (rc != PASSED) {
                    break;
                }

                /* Send out STOP */
                rc = send_i2c_stop(i2c_ptr);
                /* Get the data after the Stop */
                *buf = p1021_i2c_read(&i2c_ptr->i2cdr);

                if (rc != PASSED) {
                    break;
                }
            } else {
                /* check if data ready */
                rc = get_i2c_data_byte(i2c_ptr, buf);
                if (rc != PASSED) {
                    /* read failed */
                    break;
                }
                if (i == (size - 2)) {
                    /* Last byte. send out NACK */
                    set_rx_ack(i2c_ptr, I2C_NACK);
                }
            }
        }
    } else {
        /* Error condition. Send Stop to free the bus */
        send_i2c_stop(i2c_ptr);
	printf("\nError condition. Send Stop to free the bus, rc = 0x%02x\n", rc);
    }
#ifdef DEBUG    
    printf("\nd %d: rc = 0x%02x", __LINE__, rc);
#endif    
    /* Restore it back to ACK for next read */
    set_rx_ack(i2c_ptr, I2C_ACK);

    wastetime (N2G_I2C_BIT_DELAY);
    return (rc);

}

/*********************************************************************
 *
 * Function: p1021_i2c_read_fpga_byte
 *
 * Description: This function read 1 byte from FPGA
 *
 * Inputs: offset - offset to FPGA register.
 *         buf    - pointer to buffer for reading
 *
 * Outputs:	PASSED/FAILED
 *
 *********************************************************************
 */
uint32_t
p1021_i2c_read_fpga_byte(uint32_t offset, volatile uchar *buf)
{

    volatile uchar *rd_val = buf;

    if (p1021_i2c_read_bytes (MB_I2C_ADDR_FPGA, offset, 1, rd_val,
                               CPU_I2C0)) {
        return (FAILED);
    }

    return (PASSED);


}


/*********************************************************************
 *
 * Function:	p1021_i2c_write_bytes
 *
 * Description:	Motherboard I2C Write API.
 *
 * Inputs:	dev_p   - Pointer to device characteristics table.
 *		offset	- I2C device offset.
 *		size	- Number of bytes to write.
 *		*buf	- Write buffer pointer.
 *
 * Outputs:	PASSED/FAILED
 *
 *********************************************************************
 */
uint32_t
p1021_i2c_write_bytes(uchar dev_addr, uint32_t offset,
                      uint8_t size, uchar *buf, uchar bus_no)
{
    ccsr_i2c_t *i2c_ptr;	/* pointer to I2C Base */
    uint32_t rc, rc_stop = FAILED;	/* Return codde */
    uint i;

    /* Get the pointer to the I2C controller */
    i2c_ptr = get_i2c_ctl_addr(bus_no);

    /* Check if the controller is busy */
    if ((rc = is_i2c_busy(i2c_ptr)) != FALSE) {
        return (rc);
    }

    /* Send out Start condition */
    rc = send_i2c_start(i2c_ptr);
    if (rc != PASSED) {
        /* Unable to send out start */
        return (rc);
    }

    /* Send out the address and write */
    rc = send_i2c_slave_addr(i2c_ptr, dev_addr, I2C_WRITE_COMMAND, TRUE);
    if (rc == PASSED) {
        /* Check for write address */
        /* Send out the register/command */
        rc = send_i2c_offset(i2c_ptr, offset, I2C_ACK);
    }

    if (rc == PASSED) {
        /* Write the data */
        for (i = 0; i < size; i++, buf++) {
            /* write the data */
            rc = send_i2c_data_byte(i2c_ptr, *buf, FALSE, TRUE);
            if (rc == PASSED) {
                /* Check if sent successfully */
                rc = check_i2c_xmit_status(i2c_ptr, I2C_ACK);
            }
            if (rc != PASSED) {
                /* transmission failed */
                break;
            }
        }
    }

    /* send out stop */
    rc_stop = send_i2c_stop(i2c_ptr);

    /* rc has the Return code from transactions other than Stop.
     * rc_stop has the Stop return code.
     * return the first failed return code
     */
    if (rc == PASSED) {
        rc = rc_stop;
    }

    return (rc);

}


/*********************************************************************
 *
 * Function: p1021_i2c_write_fpga_byte
 *
 * Description: This function write 1 byte to FPGA
 *
 * Inputs: offset  - offset to FPGA register.
 *         value   - value to write
 *
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
uint32_t
p1021_i2c_write_fpga_byte(uint32_t offset, uchar value)
{
    uchar wr_val = value;

    if (p1021_i2c_write_bytes (MB_I2C_ADDR_FPGA, offset, 1, &wr_val,
                                CPU_I2C0)) {
        return (FAILED);
    }
    return (PASSED);
}


/*------------------------------------------------------------------------------
 * $Log: p1021_i2c.c,v $
 * Revision 1.1  2014/03/25 02:12:33  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.3  2012/08/21 01:16:09  huanngo
 * Remove the code to check the I2C bus is busy right after START bit is sent
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.9  2012/04/12 18:37:02  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.4.8  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.7  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.6  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.5  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.4  2011/08/24 16:02:33  steja
 * Update I2C code addd #ifdef DEBUG
 *
 * Revision 1.1.4.3  2011/08/24 00:53:15  huanngo
 * Fix the problem of I2C read/write on Eval board I2C EEPROM
 *
 * Revision 1.1.4.2  2011/08/18 19:43:24  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.7  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.6  2011/07/08 10:38:18  steja
 * Clean up code
 *
 * Revision 1.1.2.5  2011/07/08 00:08:48  huanngo
 * Clean up code
 *
 * Revision 1.1.2.4  2011/07/01 22:13:02  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.3  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.2  2011/06/08 17:32:23  huanngo
 * Change the i2c write function to p1021_i2c_write_bytes(
 *
 * Revision 1.1.2.1  2011/05/21 01:01:29  huanngo
 * Support memory test, I2C interface
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

/* $Id: zynq_i2c.c,v 1.1 2013/04/19 07:17:53 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/zynq_i2c.c,v $
 *
 * zynq_i2c.c - zynq i2c drivers
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <common.h>
#include <asm/errno.h>
#include <types.h>
#include <assert.h>
#include "error.h"
#include "zynq_i2c.h"

int zynq_i2c_init(void);
void zynq_i2c_exit(void);
int zynq_i2c_reset(void);
int zynq_i2c_regtest(void);
int zynq_i2c_read(uchar dev_addr, volatile uchar *buf, uint length);
int zynq_i2c_write(uchar dev_addr, uchar *buf, uint length);
int zynq_i2c_read_byte(uchar offset, volatile uchar *buf);
int zynq_i2c_write_byte(uchar offset, uchar value);

static void i2c_debug_status(void);
static int i2c_setup_master(int Role);

extern int fd_prc;
static ccsr_zynq_i2c *zynq_i2c = NULL;
static uint *zc702_mux = NULL;

static inline void Xout_le32(uint *addr, uint value)
{
    *(volatile uint *)addr = value;
    SYNCHRONIZE_IO;
}

static inline uint Xin_le32(uint *addr)
{
    volatile uint temp = *(volatile uint *)addr; 
    SYNCHRONIZE_IO;
    return temp;
}

static inline void Xclrbits_le32(uint *addr, uint clear)
{
    Xout_le32((addr), (Xin_le32(addr) & ~(clear)));
}

static inline void Xsetbits_le32(uint *addr, uint set)
{
    Xout_le32((addr), (Xin_le32(addr) | (set)));
}

/*********************************************************************************
 * Function:	zync_i2c_reset
 *
 * Description: disable interrupts, clear the FIFOs, clear interrupts status reg.
 *              reset all registers to reset value
 *
 **********************************************************************************
 */
int zynq_i2c_reset(void)
{
    uint32_t IntrStatusReg;

    assert(zynq_i2c);

    /* Disable interrupts */
    Xout_le32(&zynq_i2c->interrupt_disable, ZYNQ_I2C_INTERRUPT_MASK);
    /* Clear the FIFOs. */
    Xout_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_CLR_FIFO);
    /* Clear interrupts status reg. */
    //IntrStatusReg = Xin_le32(&zynq_i2c->interrupt_status);
    Xout_le32(&zynq_i2c->interrupt_status, ZYNQ_I2C_INTERRUPT_MASK);
    /* Reset any values so the software state matches the hardware device.*/
    Xout_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_RESET);
    Xout_le32(&zynq_i2c->slave_mon_pause, ZYNQ_I2C_SLAVEMON_RESET);
     /* All the IIC registers should be in their default state right now. */
    if ((ZYNQ_I2C_CONTROL_RESET != Xin_le32(&zynq_i2c->control)) ||
        (ZYNQ_I2C_TRANSIZE_RESET != Xin_le32(&zynq_i2c->transfer_size)) ||
        (ZYNQ_I2C_INTERRSTATUS_RESET != Xin_le32(&zynq_i2c->interrupt_status)) ||
        (ZYNQ_I2C_TO_RESET != Xin_le32(&zynq_i2c->time_out)) ||
        (ZYNQ_I2C_SLAVEMON_RESET != Xin_le32(&zynq_i2c->slave_mon_pause)) ||
        (ZYNQ_I2C_DATA_RESET != Xin_le32(&zynq_i2c->data))) {
        printf("i2c registers are not reset.\n");
        return (FAILED);
    }

    printf("i2c registers are reset\n");
    return (PASSED);
}

/***********************************************
 * Function:	zynq_i2c_init
 *
 * Description: mmap, reset registers
 *
 * Outputs:     return PASSED/FAILED.
 ***********************************************
 */
int zynq_i2c_init(void)
{
    void *i2c_base_ptr = NULL;
    void *i2c_mux_ptr = NULL;

    if (fd_prc <= 0) {
        return FAILED;
    }

    i2c_base_ptr = (void *)mmap(NULL, I2C0_MMAP_LEN, (PROT_READ | PROT_WRITE),
			        MAP_SHARED, fd_prc, ZYNQ_I2C0_BASE);
    if (i2c_base_ptr == MAP_FAILED) {
	cterr('f', 0, "Error mmapping i2c0 device");
	return (FAILED);
    }
    zynq_i2c = (ccsr_zynq_i2c*)i2c_base_ptr;

#ifdef ZC702
    i2c_mux_ptr = (void *)mmap(NULL, ZC702_MUX_LEN,  (PROT_READ | PROT_WRITE),
			        MAP_SHARED, fd_prc, ZC702_MUX_BASE);
    if (i2c_mux_ptr == MAP_FAILED) {
	cterr('f', 0, "Error mmapping i2c mux");
	return (FAILED);
    }
    zc702_mux = (uint*)i2c_mux_ptr;
#endif

    if (zynq_i2c_reset()) {
        return FAILED;
    }
    return PASSED;
}

/************************************************************
 * Function:	zynq_i2c_exit
 *
 * Description: reset registers, munmap
 *
 * Outputs:     return PASSED/FAILED.
 ************************************************************
 */
void zynq_i2c_exit(void)
{
    if (!zynq_i2c) {
        printf("no i2c device initated.\n");
        return;
    }
    zynq_i2c_reset();
    munmap(zynq_i2c, I2C0_MMAP_LEN);
#ifdef ZC702
    munmap(zc702_mux, ZC702_MUX_LEN);
#endif

}

static void i2c_debug_status(void)
{
    int int_status;
    int status;
    int_status = Xin_le32(&zynq_i2c->interrupt_status);
    status = Xin_le32(&zynq_i2c->status);
    printf("control reg: %x\n ",Xin_le32(&zynq_i2c->control));
    printf("transfer size: %d\n ",Xin_le32(&zynq_i2c->transfer_size));
    if (int_status || status) {
        printf("Status: ");
	if (int_status & ZYNQ_I2C_INTERRUPT_COMP) printf("COMP ");
        if (int_status & ZYNQ_I2C_INTERRUPT_DATA) printf("DATA ");
	if (int_status & ZYNQ_I2C_INTERRUPT_NACK) printf("NACK ");
	if (int_status & ZYNQ_I2C_INTERRUPT_TO) printf("TO ");
	if (int_status & ZYNQ_I2C_INTERRUPT_SLVRDY) printf("SLVRDY ");
	if (int_status & ZYNQ_I2C_INTERRUPT_RXOVF) printf("RXOVF ");
	if (int_status & ZYNQ_I2C_INTERRUPT_TXOVF) printf("TXOVF ");
	if (int_status & ZYNQ_I2C_INTERRUPT_RXUNF) printf("RXUNF ");
	if (int_status & ZYNQ_I2C_INTERRUPT_ARBLOST) printf("ARBLOST ");
	if (status & ZYNQ_I2C_STATUS_RXDV) printf("RXDV ");
	if (status & ZYNQ_I2C_STATUS_TXDV) printf("TXDV ");
	if (status & ZYNQ_I2C_STATUS_RXOVF) printf("RXOVF ");
	if (status & ZYNQ_I2C_STATUS_BA) printf("BA ");
	printf("\n");
    }
}

/*******************************************************************************
 * Function:	i2c_setup_master
 *
 * Description: set control register to prepare the transfer
 *
 * Inputs:      Role - write/read
 * Outputs:     return PASSED/FAILED
 *
 ********************************************************************************
 */

static int i2c_setup_master(int Role)
{
    uint32_t ControlReg;

    ControlReg = Xin_le32(&zynq_i2c->control);

    /* Only check if bus is busy when repeated start option is not set. */
    if ((ControlReg & ZYNQ_I2C_CONTROL_HOLD) == 0) {
	if (Xin_le32(&zynq_i2c->status) & ZYNQ_I2C_STATUS_BA) {
	    return FAILED;
	}
    }

    /* Mask the Divisors to set sclk 0x5700*/
    ControlReg &= ~(ZYNQ_I2C_CONTROL_DIV_A_MASK | ZYNQ_I2C_CONTROL_DIV_B_MASK);
    ControlReg |= (ZYNQ_I2C_SCL_DIVA << ZYNQ_I2C_CONTROL_DIV_A_SHIFT) |
                  (ZYNQ_I2C_SCL_DIVB << ZYNQ_I2C_CONTROL_DIV_B_SHIFT);
    /* Set up master, AckEn, nea,hold and also clear fifo. */
    ControlReg |= ZYNQ_I2C_CONTROL_ACKEN | ZYNQ_I2C_CONTROL_CLR_FIFO |
                  ZYNQ_I2C_CONTROL_NEA | ZYNQ_I2C_CONTROL_MS | ZYNQ_I2C_CONTROL_HOLD;
    /* Set R/W */
    if (Role == READ_ROLE) {
        ControlReg |= ZYNQ_I2C_CONTROL_RW;   /* read: 1  0x575f */
    } else {
	ControlReg &= ~ZYNQ_I2C_CONTROL_RW;  /* 0x575e */
    }

    Xout_le32(&zynq_i2c->control, ControlReg);
    /* Disable All Interrupts */
    Xout_le32(&zynq_i2c->interrupt_disable, ZYNQ_I2C_INTERRUPT_MASK);

    return PASSED;
}

/***************************************************************************************
 * Function:  zync_i2c_regtest
 * Description: print control/transfersize registers and status/interrupt_status.
 *              read some registers and compare with the reset values.
 *              write some registers then read their values. 
 *              prompt users to w/r i2c registers manually
 * Outputs: return PASSED/FAILED
 ***************************************************************************************
 */
int zynq_i2c_regtest(void)
{
    uint32_t TimeoutReg;
    uint32_t InterrReg;

    uint32_t Regaddr;
    uint32_t Regdata;
    uint8_t ans;

    i2c_debug_status();

    /* proofread */
    TimeoutReg = Xin_le32(&zynq_i2c->time_out);
    if (TimeoutReg != ZYNQ_I2C_TO_RESET) {
        printf("I2C Timeout Register test failed. I2C TimeoutReg = 0x%lx\n", TimeoutReg);
    } else {
        printf("Timeout Register test passed.\n");
    }
    InterrReg = Xin_le32(&zynq_i2c->interrupt_mask);
    if (InterrReg != ZYNQ_I2C_INTERRUPT_MASK) {
        printf("I2C Interrupt mask Register test failed. I2C InterrReg = 0x%lx\n", InterrReg);
    } else {
        printf("Interrupt mask Register test passed.\n");
    }
    /* Write, Read a register. */
    Xout_le32(&zynq_i2c->slave_mon_pause, SLAVE_REG_TEST_VALUE);
    Xout_le32(&zynq_i2c->control, CONTROL_REG_TEST_VALUE);
    if (SLAVE_REG_TEST_VALUE != Xin_le32(&zynq_i2c->slave_mon_pause)) {
        printf("I2C Slave Monitor Pause Register read/write error.\n");
    } else {
        printf("Slave Monitor Pause Register test passed.\n");
    }
    if (CONTROL_REG_TEST_VALUE != Xin_le32(&zynq_i2c->control)) {
        printf("I2C Control Register read/write error. I2C Control Register: 0x%lx\n", Xin_le32(&zynq_i2c->control));
    } else {
        printf("Control Register test passed.\n");
    }

    while (1) {
        printf("\nRead or Write i2c register? (r/w) The other key to quit: ");
        ans = getchar();
        getchar();
        if (ans == 'r' || ans == 'R') {
            Regaddr = (uint32_t)gethex_answer("i2c register offset address to read (0x00 - 0x28): ", 0, 0, 0x28);
            Regdata = Xin_le32((uint*)((uchar*)&zynq_i2c->control + Regaddr));
            printf("Register 0x%x : 0x%lx\n", Regaddr, Regdata);
        } else if (ans == 'w' || ans == 'W') {
            Regaddr = (uint32_t)gethex_answer("i2c register offset address to write (0x00 - 0x28): ", 0, 0, 0x28);
            Regdata = (uint32_t)gethex_answer("input data to write (0x00000000 - 0xFFFFFFFF): ", 0, 0x00000000, 0xFFFFFFFF);
            Xout_le32((uint*)((uchar*)&zynq_i2c->control + Regaddr), Regdata);
        } else {
            break;
        }
    }

    zynq_i2c_reset();
    prpass(testpass, "I2C register test passed\n");

    return (PASSED);

}

/**************************************************************************************
 * Function:	zynq_i2c_read
 *
 * Description: Begin write, send address byte(s), begin read, receive data bytes, end
 *              used for both memory device and register device
 * Inputs:      dev_addr - I2C slave addr
 *              buf - pointer to memory
 *              length - size of transfer data
 * Outputs:     return PASSED/FAILED
 *************************************************************************************
 */
int zynq_i2c_read(uchar dev_addr, volatile uchar *buf, uint length)
{
    uint intrstatus;
    uint status;
    uint Intrs;
    uint transsize;
    uint bytestoread = length;
    uint bytestobuf;
    uint bytestmp;

    /* Intrs keeps all the error-related interrupts.*/
    Intrs = ZYNQ_I2C_INTERRUPT_ARBLOST | ZYNQ_I2C_INTERRUPT_RXOVF|
            ZYNQ_I2C_INTERRUPT_TO | ZYNQ_I2C_INTERRUPT_NACK | ZYNQ_I2C_INTERRUPT_RXUNF;

    if(i2c_setup_master(READ_ROLE)) {
        return FAILED;
    }
    /* Clear the interrupt status register(wtc) before use it to monitor. */
    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
    Xout_le32(&zynq_i2c->interrupt_status, intrstatus);

    /* write slave addr,this initiates the I2C transfer.*/
    Xout_le32(&zynq_i2c->address, dev_addr);

    /* Set up the transfer size register */
    if (length > ZYNQ_I2C_FIFO_DEPTH) {
        Xout_le32(&zynq_i2c->transfer_size,ZYNQ_I2C_FIFO_DEPTH);
    }else {
	Xout_le32(&zynq_i2c->transfer_size,length);
    }

    /* Poll the interrupt status register to find the errors.*/
    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
    while ((bytestoread > 0) && ((intrstatus & Intrs) == 0)) {
	/*
	 * If there is no data in the FIFO, check the interrupt
         * status register for error, and continue.
	 */
        status = Xin_le32(&zynq_i2c->status);
	if ((status & ZYNQ_I2C_STATUS_RXDV) == 0) {
	    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
	    continue;
	}
	transsize = Xin_le32(&zynq_i2c->transfer_size);
        /*
	 * If length is greater than FIFO size,the master needs to wait for
         * data comes in and set transfer size register again and send more.
         */
	if (bytestoread > ZYNQ_I2C_FIFO_DEPTH) {
            /* transfer size reg decreases every byte transfer */
	    while ((transsize > 2) && ((intrstatus & Intrs) == 0)) {
	        transsize = Xin_le32(&zynq_i2c->transfer_size);
                intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
	    }
            /* If timeout happened, it is an error. */
            if (intrstatus & ZYNQ_I2C_INTERRUPT_TO) {
                cterr('f', 0, "Recv data timeout.\n");
                return FAILED;
            }

            transsize = Xin_le32(&zynq_i2c->transfer_size);
            /* how many bytes to save to buffer */
            bytestobuf = ZYNQ_I2C_FIFO_DEPTH - transsize;
            /* determin bytes to read next transfer, tell slave to send more */
            bytestmp = bytestoread - bytestobuf;
            if (bytestmp > ZYNQ_I2C_FIFO_DEPTH) {
                Xout_le32(&zynq_i2c->transfer_size, ZYNQ_I2C_FIFO_DEPTH);
            } else{
		Xout_le32(&zynq_i2c->transfer_size, bytestmp);
            }
        } else {
            bytestobuf = bytestoread;
        }

	intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
        while ((bytestobuf > 0) && ((intrstatus & Intrs) == 0)) {
            status = Xin_le32(&zynq_i2c->status);
            intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
            if ((status & ZYNQ_I2C_STATUS_RXDV) == 0) {
                /* No data in FIFO */
                printf("%d bytes to read to buffer\n", bytestoread);
                continue;
            }
            *(buf++) = (uchar)Xin_le32(&zynq_i2c->data);
            bytestobuf --;
            bytestoread --;
	}
    }
    if (intrstatus & Intrs) {
        #ifdef I2C_DEBUG
        i2c_debug_status();
        #endif
        cterr('f', 0, "I2C read error.\n");
        return FAILED;
    }

    /* All done... release the bus */
    Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);

    usleep(200000);
    return PASSED;

}
/*********************************************************************
 *
 * Function: zynq_i2c_read_byte
 *
 * Description: This function read 1 byte from I2C slave dev speicified
 *              used for register device
 *
 * Inputs: offset  - offset within I2C dev
 *         buf     - buffer to save data read
 *
 * Outputs: return PASSED/FAILED
 *
 *********************************************************************
 */
int zynq_i2c_read_byte(uchar offset, volatile uchar *buf)
{
    uchar wr_val = offset;
    volatile uchar *rd_val = buf;
    if (zynq_i2c_write(ZYNQ_I2C_ADDR_DS4424, &wr_val, 1)) {
        return (FAILED);
    }
    if (zynq_i2c_read(ZYNQ_I2C_ADDR_DS4424, rd_val, 1)) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function:	zynq_i2c_write
 *
 * Description: Begin write, send address byte(s), send data bytes, end.
 *              used for both memory device and register device
 * Inputs:      dev_addr - I2C slave addr
 *              buf - pointer to memory
 *              length - size of transfer data
 * Outputs:     return PASSED/FAILED
 *
 ********************************************************************************
 */
int zynq_i2c_write(uchar dev_addr, uchar *buf, uint length)
{
    uint intrstatus;
    uint status;
    uint Intrs;
    uint availsize;
    uint bytestowrite = length ;
    uint fifobytes;
    uint i;

    assert(zynq_i2c);

    if(i2c_setup_master(WRITE_ROLE)) {
        return FAILED;
    }
    /* write slave addr,this initiates the I2C transfer.*/
    Xout_le32(&zynq_i2c->address, dev_addr);

    /* Intrs keeps all the error-related interrupts.*/
    Intrs = ZYNQ_I2C_INTERRUPT_ARBLOST | ZYNQ_I2C_INTERRUPT_RXOVF|
        ZYNQ_I2C_INTERRUPT_TO | ZYNQ_I2C_INTERRUPT_NACK;

    /* Clear the interrupt status register(wtc) before use it to monitor. */
    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
    Xout_le32(&zynq_i2c->interrupt_status, intrstatus);

    /*
     *Transmit first FIFO full of data.
     *Determine number of bytes to write to FIFO.
     */
    if (bytestowrite > ZYNQ_I2C_FIFO_DEPTH) {
        fifobytes = ZYNQ_I2C_FIFO_DEPTH;
    } else {
        fifobytes  = bytestowrite;
    }

    /* Fill FIFO with amount determined above. */
    for (i = 0; i < fifobytes; i++) {
        Xout_le32(&zynq_i2c->data, *(buf++));
        bytestowrite --;
    }
    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
    /* Continue sending as long as there is more data and no errors. */
    while ((bytestowrite > 0) && ((intrstatus & Intrs) == 0)) {
        /* Wait until transmit FIFO is empty.*/
        status = Xin_le32(&zynq_i2c->status);
        if ((status & ZYNQ_I2C_STATUS_TXDV) != 0) {
	    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
	    continue;
        }
        /* transfer size reg is bytes in fifo - 1 */
        availsize = ZYNQ_I2C_FIFO_DEPTH - Xin_le32(&zynq_i2c->transfer_size) - 1;
        /* Send more data out through transmit FIFO. */
	if (bytestowrite > availsize) {
            fifobytes = availsize;
        } else {
            fifobytes  = bytestowrite;
        }
        for (i = 0; i < fifobytes; i++) {
            Xout_le32(&zynq_i2c->data, *(buf++));
            bytestowrite --;
        }
    }
    /* Check for completion of transfer. */
    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
    while ((intrstatus & ZYNQ_I2C_INTERRUPT_COMP) != ZYNQ_I2C_INTERRUPT_COMP) {
         if (intrstatus & Intrs) {
                #ifdef I2C_DEBUG
                printf("%d bytes to write\n", bytestowrite);
                i2c_debug_status();
                cterr('f', 0, "I2C write error.\n");
                #endif
             return FAILED;
         }
    intrstatus = Xin_le32(&zynq_i2c->interrupt_status);
    }
    /* All done... release the bus */
    Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
    usleep(200000);
    return PASSED;
}

/******************************************************************************
 *
 * Function: zynq_i2c_write_byte
 *
 * Description: This function write 1 byte to I2C slave device specified offset
 *              used for register device
 * Inputs: offset  - offset within I2C dev
 *         value   - value to write
 *
 * Outputs: return PASSED/FAILED
 *
 ******************************************************************************
 */
int zynq_i2c_write_byte(uchar offset, uchar value)
{
    uchar wr_val[2];
    wr_val[0] = offset;
    wr_val[1] = value;

    if (zynq_i2c_write(ZYNQ_I2C_ADDR_DS4424, wr_val, 2)) {
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 * Description: This function only used to test r/w when bring-up
 ********************************************************************
 */
int zc702_test(void)
{
    uchar wrval = 0;
    uchar rdval = 0;
    uchar wrbuf[65];
    uchar rdbuf[64];
    int i;

    /* GPIO Code to pull MUX out of reset. */
    Xout_le32((zc702_mux + 51), 0x2000);
    printf("mux %lx\n", Xin_le32(zc702_mux + 51));
    Xout_le32((zc702_mux + 52), 0x2000);
    printf("mux %lx\n", Xin_le32(zc702_mux + 52));
    Xout_le32((zc702_mux + 10), 0x2000);
    printf("mux %lx\n", Xin_le32(zc702_mux + 10));
    /* MUX init */
    wrval = 0x04;
    if (zynq_i2c_write(ZC702_I2C_MUX_ADDR, &wrval, 1)) {
        printf("mux write failed\n");
        return (FAILED);
    }
    if (zynq_i2c_read(ZC702_I2C_MUX_ADDR, &rdval, 1)) {
        printf("mux read failed\n");
        return (FAILED);
    }
    /* write */
    wrbuf[0] = ZC702_EEPROM_START_ADDR;
    for (i = 0; i < 64; i++) {
        wrbuf[i+1] = i;
        rdbuf[i] = 0;
        }
    if (zynq_i2c_write(ZC702_I2C_ADDR, wrbuf, 65)) {
        printf("i2c write failed\n");
        return (FAILED);
    }
    /* read */
    if (zynq_i2c_write(ZC702_I2C_ADDR, wrbuf, 1)) {
        printf("i2c read failed when sending offset addr\n");
        return (FAILED);
    }
    if (zynq_i2c_read(ZC702_I2C_ADDR, rdbuf, 64)) {
        printf("i2c read failed\n");
        return (FAILED);
    }
    printf("read eeprom data:\n");
    for (i = 0; i < 64; i++) {
        printf("%d  ", rdbuf[i]);
    }
    return (PASSED);
}

/******** History ******** 
$Log: zynq_i2c.c,v $
Revision 1.1  2013/04/19 07:17:53  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/

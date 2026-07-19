/* $Id: zynq_qspi.c,v 1.6 2018/07/23 07:02:21 easochen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/zynq_qspi.c,v $
 *
 * zynq_qspi.c - zynq qspi flash drivers
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <common.h>
#include <asm/errno.h>
#include <types.h>
#include <assert.h>
#include "nvmonvars.h"
#include "error.h"
#include "zynq_qspi.h"

int zynq_qspi_reset(void);
int zynq_qspi_regtest(void);
int zynq_qspi_init(void);
int zynq_clear_sr(void);
void zynq_qspi_exit(void);
int zynq_qspi_showinfo(void);
uchar read_confreg_S25FL129P(void);

static void qspi_getReadData(uint32_t data, uchar size);
static int qspi_getWriteData(uint32_t *data, uchar size);
static int qspi_polledTransfer(uchar *sendbuf, uchar *recvbuf, unsigned bytes, int instr);
int qspi_write(uint32_t addr, uint32_t bytes, uchar *wrbuf, uchar command);
int qspi_read(uint32_t addr, uint32_t bytes, uchar command, uchar *rdbuf);
int qspi_erase(uint32_t addr, int count);
int qspi_linear_read(uchar *recvbufptr, uint32_t addr, uint32_t bytes);
int qspi_rdidcfi(void);
int sector_lock_S25FL129P(int secnum);
int sector_lock_N25Q128(int secnum);
int sector_lock_N25Q128_SPM1(int secnum, uchar value);
int qspi_S25FL128S_ASP(int secnum, uint32_t aspaddr);
int S25FL128S_write_PPBLock(void);
uchar S25FL128S_read_PPBLock(void);
int S25FL128S_erase_PPB(void);
int S25FL128S_write_PPB(int secnum, uint32_t testaddr);
uchar S25FL128S_check_PPB(int secnum, uint32_t testaddr);
int S25FL128S_read_PPB(int secnum, uint32_t testaddr);
int S25FL128S_write_ASP(uchar ASPR);
uchar S25FL128S_read_ASP(void);

extern int fd_prc;
extern int flash_type;
void *qspi_base_ptr = NULL;
void *qspi_phy_ptr = NULL;
uchar WriteBuffer[PAGE_SIZE + DATA_OFFSET + DUMMY_SIZE + OVERHEAD_SIZE];
uchar ReadBuffer[PAGE_SIZE + DATA_OFFSET + DUMMY_SIZE + OVERHEAD_SIZE];
static XQspi *qptr;

static inline void Xout_le32(uint32_t *addr, uint32_t value);
static inline uint32_t Xin_le32(uint32_t *addr);

#define qspi_readreg(RegOffset)    Xin_le32(qspi_base_ptr + RegOffset)
#define qspi_writereg(RegOffset, RegValue)    Xout_le32((qspi_base_ptr + RegOffset), (RegValue))

static inline void Xout_le32(uint32_t *addr, uint32_t value)
{
    *(volatile uint32_t *)addr = value;
    SYNCHRONIZE_IO;
}

static inline uint32_t Xin_le32(uint32_t *addr)
{
    volatile uint32_t temp = *(volatile uint32_t *)addr; 
    SYNCHRONIZE_IO;
    return temp;
}

/****************************************************************************
 * Function: zynq_qspi_reset
 * Description: initial QSPI dev struct, reset necessary registers for transfer
 *              initial WriteBuffer and ReadBuffer
 *
 *****************************************************************************/
int zynq_qspi_reset(void)
{
    uint32_t ConfigReg;
    uint32_t DelayReg;
    uint32_t StatusReg;
    uint32_t InterruptMASK;
    uint32_t LconfigReg;
    uint32_t EnableReg;
    uint32_t Register;

    assert(qptr);
    assert(qspi_base_ptr);

    qptr->IsBusy = FALSE;
    qptr->SendBuffer = NULL;
    qptr->RecvBuffer = NULL;
    qptr->RequestedBytes = 0;
    qptr->RemainingBytes = 0;
    qptr->IsReady = TRUE;
    qptr->SlaveSelect = (((~(0x0001 << QSPI0_SELECT)) << QSPI_CONFIG_SSCTRL_SHIFT) &
                                       QSPI_CONFIG_SSCTRL_MASK);

    ConfigReg = QSPI_CONFIG_RESET_STATE;
    /* Select the slaves */
    ConfigReg &= ~QSPI_CONFIG_SSCTRL_MASK;
    ConfigReg |= qptr->SlaveSelect;

    qspi_writereg(QSPI_CONFIG_OFFSET, ConfigReg);

    qspi_writereg(QSPI_DELAY_OFFSET, QSPI_DR_NODELAY_MASK);

    qspi_writereg(QSPI_STATUS_OFFSET, QSPI_ISR_RESET);

    qspi_writereg(QSPI_IDIS_OFFSET, QSPI_IDR_DIS);

    qspi_writereg(QSPI_LINEAR_CFG_OFFSET, QSPI_LQSPI_CR_RST_STATE);

    qspi_writereg(QSPI_ENABLE_OFFSET, QSPI_ENABLE_RESET);

    /* All the QSPI registers should be in their default state right now.*/
    Register = qspi_readreg(QSPI_CONFIG_OFFSET);
    if (Register != ConfigReg) {
        cterr('f', 0, "QSPI Config register is not reset.\n");
        return FAILED;
    }
    DelayReg = qspi_readreg(QSPI_DELAY_OFFSET);
    if (DelayReg != QSPI_DR_NODELAY_MASK) {
        cterr('f', 0, "QSPI delay register is not reset.\n");
	return FAILED;
    }
    StatusReg = qspi_readreg(QSPI_STATUS_OFFSET);
    if (StatusReg != QSPI_ISR_RESET_STATE) {
        cterr('f', 0, "QSPI ISR register is not reset.\n");
        printf("QSPI ISR register: 0x%lx\n", StatusReg);
	return FAILED;
    }
    InterruptMASK = qspi_readreg(QSPI_IMASK_OFFSET);
    if (InterruptMASK != QSPI_IXR_DIS_MASK) {
        cterr('f', 0, "QSPI Interrupt disable register is not reset.\n");
        printf("QSPI Interrupt disable register: 0x%lx\n", InterruptMASK);
	return FAILED;
    }
    LconfigReg = qspi_readreg(QSPI_LINEAR_CFG_OFFSET);
    if (LconfigReg != QSPI_LQSPI_CR_RST_STATE) {
        cterr('f', 0, "LQSPI Config register is not reset.\n");
	return FAILED;
    }
    EnableReg = qspi_readreg(QSPI_ENABLE_OFFSET);
    if (EnableReg != QSPI_ENABLE_RESET) {
        cterr('f', 0, "QSPI Enable register is not reset.\n");
	return FAILED;
    }

    memset(WriteBuffer, 0, PAGE_SIZE + DATA_OFFSET + DUMMY_SIZE + OVERHEAD_SIZE);
    memset(ReadBuffer, 0, PAGE_SIZE + DATA_OFFSET + DUMMY_SIZE + OVERHEAD_SIZE);
#ifdef QSPI_DEBUG
    printf("QSPI is reset\n");
#endif

    return (PASSED);
}
/****************************************************************************
 * Function: qspi_regtest
 * Description: read some registers and compare with the reset values.
 *              write some registers then read their values.
 *              prompt users to w/r qspi registers manually
 *
 *****************************************************************************/
int zynq_qspi_regtest(void)
{
    uint32_t ModIDReg;
    uint32_t LPBKDlyReg;
    uint32_t SlaveIdleCntReg;
    uint32_t DelayReg;
    uint32_t ConfigReg;
    uint32_t InterruptMASK;

    uint32_t Regaddr;
    uint32_t Regdata;
    uint8_t ans;

    assert(qspi_base_ptr);

    ModIDReg = qspi_readreg(QSPI_MOD_ID_OFFSET);
    if (ModIDReg != QSPI_MOD_ID_RESET) {
        printf("qspi Mod ID Register test failed. qspi ModIDReg = 0x%lx\n", ModIDReg);
    } else {
        printf("Mod ID Register test passed.\n");
    }
    LPBKDlyReg = qspi_readreg(QSPI_LPBK_DLYADJ_OFFSET);
    if (LPBKDlyReg != QSPI_LPBK_DLYADJ_RESET) {
        printf("qspi LPBK Delay Adj Register test failed. qspi LPBKDlyReg = 0x%lx\n", LPBKDlyReg);
    } else {
        printf("LPBK Delay Adj Register test passed.\n");
    }
    SlaveIdleCntReg = qspi_readreg(QSPI_SIC_OFFSET);
    if (SlaveIdleCntReg != QSPI_SICR_RESET) {
        printf("qspi SlaveIdleCnt Register test failed. qspi SlaveIdleCntReg = 0x%lx\n", SlaveIdleCntReg);
    } else {
        printf("SlaveIdleCnt Register test passed.\n");
    }
    qspi_writereg(QSPI_DELAY_OFFSET, QSPI_DR_TEST);
    DelayReg = qspi_readreg(QSPI_DELAY_OFFSET);
    if (DelayReg != QSPI_DR_TEST) {
        printf("qspi Delay Register test failed. qspi DelayReg = 0x%lx\n", DelayReg);
    } else {
        printf("Delay Register test passed.\n");
    }
    qspi_writereg(QSPI_CONFIG_OFFSET, QSPI_CONFIG_TEST);
    ConfigReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    if (ConfigReg != QSPI_CONFIG_TEST) {
        printf("qspi Config Register test failed.cqspi ConfigReg = 0x%lx\n", ConfigReg);
    } else {
        printf("Config Register test passed.\n");
    }
    qspi_writereg(QSPI_IEN_OFFSET, QSPI_IER_EN);
    InterruptMASK = qspi_readreg(QSPI_IMASK_OFFSET);
    if (InterruptMASK != QSPI_IXR_EN_MASK) {
        printf("qspi Interrupt enable Register test failed. qspi InterruptMASK = 0x%lx\n", InterruptMASK);
    } else {
        printf("Interrupt enable Register test passed.\n");
    }

    while (1) {
        printf("\nRead or Write qspi register? (r/w) The other key to quit: ");
        ans = getchar();
        getchar();
        if (ans == 'r' || ans == 'R') {
            Regaddr = (uint32_t)gethex_answer("qspi register offset address to read (0x00 - 0xFC): ", 0, 0, 0xFC);
            Regdata = qspi_readreg(Regaddr);
            printf("Register 0x%x : 0x%lx\n", Regaddr, Regdata);
        } else if (ans == 'w' || ans == 'W') {
            Regaddr = (uint32_t)gethex_answer("qspi register offset address to write (0x00 - 0xFC): ", 0, 0, 0xFC);
            Regdata = (uint32_t)gethex_answer("input data to write (0x00000000 - 0xFFFFFFFF): ", 0, 0x00000000, 0xFFFFFFFF);
            qspi_writereg(Regaddr, Regdata);
        } else {
            break;
        }
    }

    prpass(testpass, "QSPI Register test passed\n");
    if (zynq_qspi_reset()) {
        return FAILED;
    }

    return (PASSED);
}
/****************************************************************************
 * Function: qspi_init
 * Description: mmap,initial qspi dev struct, reset all rigisters.
 * Output: PASSED/FAILED
 *****************************************************************************/
int zynq_qspi_init(void)
{
    int Status;
    uint32_t Register;

    if (fd_prc == -1) {
        return FAILED;
    }
    qspi_base_ptr = (uchar *)mmap(NULL, QSPI_MMAP_LEN, (PROT_READ | PROT_WRITE),
                                  MAP_SHARED, fd_prc, ZYNQ_QSPI0_BASE);
    if (qspi_base_ptr == MAP_FAILED) {
	cterr('f', 0, "Error mmapping Qspi device registers");
	return (FAILED);
    }

    qptr = (XQspi *)malloc(sizeof(XQspi));
    if (zynq_qspi_reset()) {
        return (FAILED);
    }
    return (PASSED);
}
/****************************************************************************
 * Function: qspi_exit
 * Description: ummap,free qspi dev struct
 * Output: NULL
 *****************************************************************************/
void zynq_qspi_exit(void)
{
    int Status;
    uint32_t Register;

    if (!qspi_base_ptr) {
        return;
    }

    free(qptr);
    munmap(qspi_base_ptr, QSPI_MMAP_LEN);

}
/****************************************************************************
 *
 * Function:     qspi_getReadData
 * Description:  Copy data from RX FIFO to Receive buffer.
 * Input:        data - received from RX FIFO
 *               size - number of bytes to be copied to the Receive buffer
 * Output:       None.
 *
 ******************************************************************************/
static void qspi_getReadData(uint32_t data, uchar size)
{
    uchar byte3;

    assert(qptr);
#ifdef QSPI_DEBUG
    printf("Receive 0x%lx\n", data);
#endif
    if (qptr->RecvBuffer) {
        switch (size) {
	case 1:
	    *((uchar *)qptr->RecvBuffer) = (data >> 24);
            #ifdef QSPI_DEBUG
            printf("RecvBuffer: 0x%lx\n", *((uchar *)qptr->RecvBuffer));
            #endif
	    qptr->RecvBuffer += 1;
	    break;
	case 2:
	    *((unsigned short *)qptr->RecvBuffer) = (data >> 16);
            #ifdef QSPI_DEBUG
            printf("RecvBuffer: 0x%lx\n", *((unsigned short *)qptr->RecvBuffer));
            #endif
	    qptr->RecvBuffer += 2;
	    break;
	case 3:
	    *((unsigned short *)qptr->RecvBuffer) = (data >> 8);
	    qptr->RecvBuffer += 2;
	    byte3 = (uchar)(data >> 24);
	    *((uchar *)qptr->RecvBuffer) = byte3;
            #ifdef QSPI_DEBUG
            printf("RecvBuffer: 0x%lx\n", *((uchar *)qptr->RecvBuffer));
            #endif
	    qptr->RecvBuffer += 1;
	    break;
	case 4:
	    *((uint32_t *)qptr->RecvBuffer) = data;
	    qptr->RecvBuffer += 4;
	    break;
	default:
	    break;
	}
    }
    qptr->RequestedBytes -= size;
    if (qptr->RequestedBytes < 0) {
	qptr->RequestedBytes = 0;
    }
}

/****************************************************************************
 *
 * Function: qspi_getWriteData
 * Description: Copy data from send buffer. QSPI supports only 32-bit transfers
 *
 * Input:    data - pointer that data to be copied to.
 *           size - number of bytes to be copied from the send buffer.
 *
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
static int qspi_getWriteData(uint32_t *data, uchar size)
{
    assert(qptr);
    if (!qptr->SendBuffer) {
        cterr('f', 0, "Unable to get data to send\n");
        return FAILED;
    }
    switch (size) {
    case 1:
        *data = *((uchar *)qptr->SendBuffer);
        qptr->SendBuffer += 1;
        *data |= 0xFFFFFF00;
        break;
    case 2:
	*data = *((unsigned short *)qptr->SendBuffer);
	qptr->SendBuffer += 2;
	*data |= 0xFFFF0000;
	break;
    case 3:
        *data = *((unsigned short *)qptr->SendBuffer);
	qptr->SendBuffer += 2;
        *data |= (*((uchar *)qptr->SendBuffer) << 16);
	qptr->SendBuffer += 1;
        *data |= 0xFF000000;
	break;
    case 4:
	*data = *((uint32_t *)qptr->SendBuffer);
	qptr->SendBuffer += 4;
	break;
    default:
        break;
    }
#ifdef QSPI_DEBUG
    printf("write 0x%lx\n", *data);
#endif
    qptr->RemainingBytes -= size;
    if (qptr->RemainingBytes < 0) {
        qptr->RemainingBytes = 0;
    }
    return PASSED;
}

/****************************************************************************
 * Function:     qspi_polledTransfer
 * Description:  transfers specified in polled mode.
 *               The caller has the option of providing two or one buffer for send and receive,
 *               or no buffer for receive.
 *               In manual mode, transfer size can be more than FIFO size(256 bytes)
 *               In auto mode, transfer size is limited by the FIFO depth(256 bytes)
 * Input:       sendbuf - pointer to buffer for data to be sent
 *              recvbuf - pointer to buffer for received data.
 *              bytes -   number of bytes to send/recv.
 *                        number of bytes received always equal bytes sent
 *              instr -  whether the first byte(s) in send buffer is a serial flash instruction.
 * Output:      PASSED/FAILED
 *****************************************************************************/
static int
qspi_polledTransfer(uchar *sendbuf, uchar *recvbuf, unsigned bytes, int instr)
{
    uint32_t StatusReg = 0;
    uint32_t ControlReg = 0;
    uchar Instruction = 0;
    uint32_t Data = 0;
    unsigned int Index = 0;
    XQspiInstFormat *CurrInst;

    assert(qptr);
    assert(sendbuf);
    assert(recvbuf);

    if (qptr->IsReady != TRUE || (instr > QSPI_IS_INST) || qptr->IsBusy == TRUE) {
        cterr('f', 0, "Unable to transfer.\n");
        return (FAILED);
    }

    /* Set the busy flag, which is cleared when transfer is entirely done. */
    qptr->IsBusy = TRUE;
    /* Set up buffer pointers.*/
    qptr->SendBuffer = sendbuf;
    qptr->RecvBuffer = recvbuf;
    qptr->RequestedBytes = bytes;
    qptr->RemainingBytes = bytes;

    /* If the slave select lines are "Forced" (under manual control),
     * set the slave selects now, before beginning the transfer.
     */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    if (0 != (ControlReg & QSPI_CONFIG_SSFORCE_MASK)) {
        ControlReg &= ~QSPI_CONFIG_SSCTRL_MASK;
	ControlReg |= qptr->SlaveSelect;
        qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);
    }

    /* Enable the device */
    qspi_writereg(QSPI_ENABLE_OFFSET, QSPI_ENABLE_MASK);

    if (instr) {
        Instruction = *(qptr->SendBuffer);
        for (Index = 0 ; Index < QspiInstSize; Index++) {
	    if (Instruction == QspiInst[Index].OpCode) {
	        break;
            }
	}
        if (Index == QspiInstSize) {
            cterr('f', 0, "Instruction %x not supported. Index = %d\n", Instruction, Index);
	    return FAILED;
        }

        CurrInst = &QspiInst[Index];
        if (CurrInst->OpCode == QSPI_OPCODE_WRSR2) {
            *((uchar *)qptr->SendBuffer) = QSPI_OPCODE_WRSR;
            printf("sendbuf: 0x%x\n", *((unsigned short *)qptr->SendBuffer));
        }
	/* Get the complete command (flash inst + address/data) */
	if (qspi_getWriteData(&Data, CurrInst->InstSize)){
            return FAILED;
        }
        qspi_writereg(CurrInst->TxOffset, Data);
    }

    while((qptr->RemainingBytes > 0) || (qptr->RequestedBytes > 0)) {
        /* Fill the DTR/FIFO */
        while (qptr->RemainingBytes > 0) {
            if ((qspi_readreg(QSPI_STATUS_OFFSET) & QSPI_IXR_TXFULL_MASK) != 0) {
                break;
            }
            if (qptr->RemainingBytes < 4) {
		qspi_getWriteData(&Data, qptr->RemainingBytes);
	    } else {
		qspi_getWriteData(&Data, 4);
	    }

	    qspi_writereg(QSPI_TXD_00_OFFSET, Data);
	}

	/* If in Manual Start mode, start the transfer */
	if ((ControlReg & (QSPI_CONFIG_MSTREN_MASK | QSPI_CONFIG_MANSTRTEN_MASK)) ==
            (QSPI_CONFIG_MSTREN_MASK | QSPI_CONFIG_MANSTRTEN_MASK)) {
	    ControlReg |= QSPI_CONFIG_MANSTRT_MASK;
	    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);
	}

	/* Wait for transfer start, only wait when TX FIFO is full and RX FIFO is empty  */
	do {
	    StatusReg = qspi_readreg(QSPI_STATUS_OFFSET);
            if ((StatusReg & QSPI_IXR_RXNEMPTY_MASK) != 0) {
                break;
            }
	} while ((StatusReg & QSPI_IXR_TXNFULL_MASK) == 0);

	/* When RX is not empty, process received data until RX FIFO is empty. */
	while ((StatusReg & QSPI_IXR_RXNEMPTY_MASK) != 0) {
        #ifdef QSPI_DEBUG
            printf("@qptr->RequestedBytes: %d\n", qptr->RequestedBytes);
        #endif
	    Data = qspi_readreg(QSPI_RXD_OFFSET );
            if ((qptr->RequestedBytes > 0) && (qptr->RequestedBytes < 4 )) {
	        qspi_getReadData(Data, qptr->RequestedBytes);
	    } else if (qptr->RequestedBytes >= 4) {
		qspi_getReadData(Data, 4);
	    } else {
                printf("Cleaning dirty data in FIFO.\n");
            }
	    StatusReg = qspi_readreg(QSPI_STATUS_OFFSET);
	}
    }

    /* If the Slave select lines are being manually controlled, disable them */
    if (0 != (ControlReg & QSPI_CONFIG_SSFORCE_MASK)) {
	ControlReg |= QSPI_CONFIG_SSCTRL_MASK;
	qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);
    }

    qptr->IsBusy = FALSE;

    /* Disable the device.*/
    qspi_writereg(QSPI_ENABLE_OFFSET, 0);

    return PASSED;
}

/******************************************************************************
 *
 * Function: qspi_write
 *
 * Description: This function writes to the  serial FLASH connected to the QSPI interface.
 *              The FLASH contains a 256 byte write buffer which can be filled and then a
 *              write is automatically performed by the device.  All the data put into the
 *              buffer must be in the same page of the device.
 *
 * Input:  addr  - offset address to store data within QSPI FLASH
 *                 must be less than QSPI_SIZE(0x1000000)
 *         bytes - number of bytes to write.
 *         wrbuf - pointer to data to be written
 *
 * Output: FAILED/PASSED
 *
 *****************************************************************************/
int qspi_write(uint32_t addr, uint32_t bytes, uchar *wrbuf, uchar command)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    uchar *pbuf = wrbuf;
    int i;
    int timeout;
    int page = 0;
    int bytecount = bytes;
    int size = 0;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK | QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    while (bytecount > 0) {
        /* Send write enable command. send as a seperate transfer before the write */
        if (qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), 
            QSPI_IS_INST)) {
            cterr('f', 0, "sending write enable command error.\n");
            return FAILED;
        }

        /* Setup the write command with the specified address and data */
        WriteBuffer[COMMAND_OFFSET] = command;
        WriteBuffer[ADDRESS_1_OFFSET] = (uchar)((addr & 0xFF0000) >> 16);
        WriteBuffer[ADDRESS_2_OFFSET] = (uchar)((addr & 0xFF00) >> 8);
        WriteBuffer[ADDRESS_3_OFFSET] = (uchar)(addr & 0xFF);

        /* fill WriteBuffer with data*/
        size = ((bytecount > PAGE_SIZE) ? PAGE_SIZE : bytecount);
        for (i = 0; i < size; i++) {
            WriteBuffer[DATA_OFFSET + i] = *pbuf;
            pbuf++;
            bytecount--;
        }

        /* Send the write command, address and data*/
        if (qspi_polledTransfer(WriteBuffer, ReadBuffer, size + OVERHEAD_SIZE, QSPI_IS_INST)) {
            cterr('f', 0, "sending data error.\n");
            return FAILED;
        }

        for (timeout = 0; timeout < 10000; timeout++) {
            /* Poll the status register to determine when transfer completes,
             * by sending a read status command and receiving the status byte
	     */
            qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd),  
                                QSPI_IS_INST);
            if ((FlashStatus[1] & 0x01) == 0) {
	        break;
	    }
            usleep(1000);
        }
        if (timeout == 10000) {
#ifdef QSPI_DEBUG
            printf("QSPI Read Status Register Time out.\n");
#endif
            return FAILED;
        }
        addr += size;
    }

    return (PASSED);
}

/******************************************************************************
 * Function: qspi_rdidcfi
 * Description: This function read ID-CFI bytes to determin whether it's
 *              S25FL129P, S25FL128S or MT25QL128A
 * Input:     None
 * Output:    0 - unknown tpye
 *            1 - S25FL129P
 *            2 - S25FL128S
 *            3 - MT25QL128A
 *
 ******************************************************************************/
int qspi_rdidcfi(void)
{
    uint32_t ControlReg;

    zynq_qspi_reset();

    WriteBuffer[COMMAND_OFFSET]   = READ_ID;
    WriteBuffer[ADDRESS_1_OFFSET] = 0x23;    /* 3 dummy bytes */
    WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
    WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

    assert(qspi_base_ptr);
    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_4 & QSPI_CONFIG_PRESC_MAXIMUM) << QSPI_CONFIG_PRESC_SHIFT;
    /* Read ID in Auto mode. */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK;
    ControlReg &= ~QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);
    if (qspi_polledTransfer(WriteBuffer, ReadBuffer, RD_CFI_SIZE, QSPI_IS_INST)) {
        cterr('f', 0, "read id-cfi error.\n");
        return (-1);
    }
#ifdef QSPI_DEBUG
    printf("QSPI FlashID = 0x%x%x%x\n\r", ReadBuffer[1], ReadBuffer[2], ReadBuffer[3]);

    printf("Byte 45h of ID-CFI is = 0x%x\n\r", ReadBuffer[S25FL_DIFF_OFFSET]);
#endif
    if( ReadBuffer[1] == 0x01 && ReadBuffer[2] == 0x20 && ReadBuffer[3] == 0x18) {
        if (ReadBuffer[S25FL_DIFF_OFFSET] == 0x15) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Flash is S25FL129P\n");
            }
            return S25FL129P;
        } else if (ReadBuffer[S25FL_DIFF_OFFSET] == 0x21) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Flash is S25FL128S\n");
            }
            return S25FL128S;
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Flash type is unknown\n");
            }
            return -1;
        }
    } else if (ReadBuffer[1] == 0x20 && ReadBuffer[2] == 0xba && ReadBuffer[3] == 0x18){
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("Flash is Micron MT25QL128\n");
            }
        return MT25QL128;
    } else {
        printf("Flash type is unknown\n");
        return -1;
    }

}

/******************************************************************************
 * Function: zynq_qspi_showinfo
 * Description: This function displays QSPI FLASH size and ID information.
 * Input:    none
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_qspi_showinfo(void)
{
    uint32_t ControlReg;

    zynq_qspi_reset();

    /* Read ID in Auto mode. */
    WriteBuffer[COMMAND_OFFSET]   = READ_ID;
    WriteBuffer[ADDRESS_1_OFFSET] = 0x23;    /* 3 dummy bytes */
    WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
    WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

    assert(qspi_base_ptr);

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_4 & QSPI_CONFIG_PRESC_MAXIMUM) << QSPI_CONFIG_PRESC_SHIFT;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    if(qspi_polledTransfer(WriteBuffer, ReadBuffer, RD_ID_SIZE, QSPI_IS_INST)) {
        cterr('f', 0, "read flash id error.\n");
        return FAILED;
    }
    printf("\nQSPI0 FLASH is connected.\n");
    printf("Total Size : %d Bytes (0xFC000000 - 0xFCFFFFFF)\n", QSPI_SIZE);
    printf("Sector size: %d Bytes, Page size: %d Bytes\n", SECTOR_SIZE, PAGE_SIZE);
    printf("QSPI FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[1], ReadBuffer[2], ReadBuffer[3]);

    return PASSED;
}

/******************************************************************************
 * Function: zynq_clear_sr
 * Description: This function clears the WIP, P_ERR, and E_ERR bits of SR.
 * Input:    None.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int zynq_clear_sr(void)
{
    uint32_t ControlReg;
    zynq_qspi_reset();
    assert(qspi_base_ptr);

    /* Clear SR in Auto mode. */
    if (flash_type == MT25QL128) {
        WriteBuffer[COMMAND_OFFSET] = CLEAR_MT25QL128_FSR_CMD;
    } else {
        WriteBuffer[COMMAND_OFFSET] = CLEAR_STATUS_CMD;
    }
    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    if (qspi_polledTransfer(WriteBuffer, ReadBuffer, 1, QSPI_IS_INST)) {
        cterr('f', 0, "clear status register error.\n");
        return FAILED;
    }
#ifdef QSPI_DEBUG
    printf("Status register WIP bit is cleared.\n");
#endif
    return PASSED;

}
/******************************************************************************
 * Function: qspi_read
 * Description: This function reads from the  QSPI FLASH.
 * Input:    addr  - offset address to store data within QSPI FLASH
 *                   must be less than QSPI_SIZE(0x1000000)
 *           bytes - number of bytes to read.
 *           command - QSPI device supports Read, Fast Read, Dual Read and
 *                     Quad Read commands to read data from the flash.
 * Output:   PASSED/FAILED
 *
 ******************************************************************************/
int qspi_read(uint32_t addr, uint32_t bytes, uchar command, uchar *rdbuf)
{
    uint32_t ControlReg;
    uint32_t dummy = 0;
    uint32_t size = 0;
    uchar *pbuf = rdbuf;
    int i;
    int page;
    uint32_t bytecount = bytes;
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];


    assert(qspi_base_ptr);
    zynq_qspi_reset();

    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK | QSPI_CONFIG_MANSTRTEN_MASK;
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;

    /* Setup clk according to read command */
    if (command == READ_CMD) {
        ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_4 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                                 QSPI_CONFIG_PRESC_SHIFT;
        dummy = 0;
    } else {
        ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                                 QSPI_CONFIG_PRESC_SHIFT;
        dummy = DUMMY_SIZE;
    }

    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);
    /* Setup the read command with the specified address and data for the FLASH */
    while (bytecount > 0) {
        WriteBuffer[COMMAND_OFFSET] = command;
        WriteBuffer[ADDRESS_1_OFFSET] = (uchar)((addr & 0xFF0000) >> 16);
        WriteBuffer[ADDRESS_2_OFFSET] = (uchar)((addr & 0xFF00) >> 8);
        WriteBuffer[ADDRESS_3_OFFSET] = (uchar)(addr & 0xFF);

        /* send the read command and address and receive the specified number
           of bytes of data in the data buffer */
        memset(ReadBuffer, 0, sizeof(ReadBuffer));
        size = dummy + ((bytecount > READ_SIZE) ? READ_SIZE : bytecount);
        if (qspi_polledTransfer(WriteBuffer, ReadBuffer, size + OVERHEAD_SIZE + OVERHEAD_SIZE, QSPI_IS_INST)) {
            cterr('f', 0, "reading data error.\n");
            return FAILED;
        }

        for(i = DATA_OFFSET + dummy; i < (size + OVERHEAD_SIZE); i++) {
            *pbuf = ReadBuffer[i];
            pbuf++;
            bytecount--;
        }
        addr += size - dummy;
    }
    return (PASSED);
}

/******************************************************************************
 * Function:    qspi_erase
 * Description: This function erases the sectors in the QSPI FLASH
 * Input:       addr - the address of the first sector which needs to be erased.
 *              count - number of sectors to be erased.
 * Output:      PASSED/FAILED
 *
 *****************************************************************************/
int qspi_erase(uint32_t addr, int count)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    /* If erase size is equal to flash size, use bulk erase command */
    if (count == NUM_SECTORS) {
        /* send write enable command, needs to be sent as a seperate transfer */
        qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
        /* Setup the bulk erase command */
        WriteBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;
        qspi_polledTransfer(WriteBuffer, ReadBuffer, BULK_ERASE_SIZE, QSPI_IS_INST);
        for (timeout = 0; timeout < 10000; timeout++) {
            /* Poll the status register of the device to determine when it completes,
             * by sending a read status command and receiving the status byte
             */
            qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd),
                                QSPI_IS_INST);
            if ((FlashStatus[1] & 0x01) == 0) {
                break;
            }
            usleep(1000);
        }
        if (timeout == 10000) {
#ifdef QSPI_DEBUG
            printf("QSPI Read Status Register Time out.\n");
#endif
            return FAILED;
        }
        printf("\nBULK Erase completed.\n");
        return PASSED;
    }

    for (i = 0; i < count; i++) {
        /* send write enable command, needs to be sent as a seperate transfer */
        qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
        /* Setup the sector erase command */
        WriteBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
        WriteBuffer[ADDRESS_1_OFFSET] = (uchar)(addr >> 16);
        WriteBuffer[ADDRESS_2_OFFSET] = (uchar)(addr >> 8);
        WriteBuffer[ADDRESS_3_OFFSET] = (uchar)(addr & 0xFF);

        qspi_polledTransfer(WriteBuffer, ReadBuffer, SEC_ERASE_SIZE, QSPI_IS_INST);

        for (timeout = 0; timeout < 10000; timeout++) {
            qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
            /* If the status indicates the write is done, then stop waiting,
             * if this loop never exits, the device slave select is possibly 
             * incorrect so that the device status is not being read
             */
            if ((FlashStatus[1] & 0x01) == 0) {
                break;
            }
            usleep(1000);
        }
        if (timeout == 10000) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("QSPI Read Status Register Time out.\n");
            }
            return FAILED;
        }
        addr += SECTOR_SIZE;
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nSECTOR ERASE completed. FlashStatus[1]  = %x, timeout = %d\n", FlashStatus[1],timeout );
    }
    return PASSED;
}
/****************************************************************************
 * Function: qspi_Lread
 * Description: Read the flash in Linear QSPI mode
 * Input:	recvbufptr - pointer to a buffer for received data.
 *              addr - starting address within the flash from where 
 *                     data needs to be read.
 *              bytes - number of bytes to receive.
 * Output:      PASSED/FAILED
 ******************************************************************************/
int qspi_linear_read(uchar *recvbufptr, uint32_t addr, uint32_t bytes)
{
    uint32_t ControlReg;

    assert(qspi_base_ptr);

    if (fd_prc == -1) {
        return FAILED;
    }

    qspi_phy_ptr = (uchar *)mmap(NULL, SECTOR_SIZE, (PROT_READ | PROT_WRITE),
                                 MAP_SHARED, fd_prc, QSPI_LINEAR_BASEADDR + addr);
    if (qspi_phy_ptr == MAP_FAILED) {
	cterr('f', 0, "Error mmapping Qspi device");
	return (FAILED);
    }

    zynq_qspi_reset();

    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    /* LQSPI mode */
    qspi_writereg(QSPI_LINEAR_CFG_OFFSET, QSPI_LQSPI_CR_MASK);

    qspi_writereg(QSPI_ENABLE_OFFSET, QSPI_ENABLE_MASK);

    printf("starting linear read...\n");
    memcpy(recvbufptr, (const uchar*)qspi_phy_ptr, bytes);
    printf("copy completed.\n");
    qspi_writereg(QSPI_ENABLE_OFFSET, 0);

    munmap(qspi_phy_ptr, SECTOR_SIZE);
    return PASSED;
}

/***********************************************************************************
 * Function:    sector_lock_S25FL129P
 * Description: This function locks consecutive sectors in QSPI flash of S25FL129P type
 *              start from top of flash array
 * Input:       secnum - number of sectors to lock, can be 8, 32, 128, 256
 * Output:      PASSED/FAILED
 *
 ***********************************************************************************/
int sector_lock_S25FL129P(int secnum)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar WriteStatusCmd[3];  /* send 3 bytes */
    uchar WriteStatus[3];
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;
    uchar ans;

    printf("\nChoose protect mode (a,b,c, other keys to quit:\n");
    printf("a: lock sectors but unlock FREEZE bit\n");
    printf("b: lock sectors as well as FREEZE bit\n");
    printf("c: unlock all sectors.\n\n");
    printf("enter protect mode: ");
    ans = getchar();
    getchar();

    /* Read Configuration Register */
    WriteStatusCmd[2] = read_confreg_S25FL129P();
    /* Write Status Register */
    WriteStatusCmd[0] = WRITE_STATUS_CMD;
    /* [1] is SR  */
    switch(secnum) {
    case 8 :
        WriteStatusCmd[1] = 0x0B;
        break;
    case 32 :
        WriteStatusCmd[1] = 0x13;
        break;
    case 128 :
        WriteStatusCmd[1] = 0x1B;
        break;
    case 256 :
        WriteStatusCmd[1] = 0x1F;
        break;
    default:
        WriteStatusCmd[1] = 0x03;
        break;
    }
    /* [2] is CR */
    switch(ans) {
    case 'a':    /* bit[1] is quad bit */
        WriteStatusCmd[2] &= 0xFE;
        break;
    case 'b':    /* bit[0] is FREEZE bit */
        WriteStatusCmd[2] |= 0x01;
        break;
    case 'c':
        WriteStatusCmd[1] &= 0xE3;
        break;
    default:
        return PASSED;
    }

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);

    qspi_polledTransfer(WriteStatusCmd, WriteStatus, sizeof(WriteStatusCmd), QSPI_IS_INST);

    for (timeout = 0; timeout < 10000; timeout++) {
        qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
        usleep(1000);
    }
    if (timeout == 10000) {
        printf("QSPI Read Status Register Time out.\n");
    }

    return PASSED;
}

/******************************************************************************
 * Function:    read_confreg_S25FL129P
 *
 * Input:       None
 * Output:      Configuration register value
 *
 *****************************************************************************/
uchar read_confreg_S25FL129P(void)
{
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar ReadConfCmd[] = { READ_CONF_CMD, 0 };  /* must send 2 bytes */
    uchar Flashconf[2] = { 0, 0 };
    uchar FlashStatus[2] = { 0, 0 };
    uint32_t ControlReg;
    int timeout;
    int flash_type = qspi_rdidcfi();
    assert(qspi_base_ptr);
    zynq_qspi_reset();

    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    qspi_polledTransfer(ReadConfCmd, Flashconf, sizeof(ReadConfCmd), QSPI_IS_INST);
    if (flash_type == MT25QL128) {
        ReadConfCmd[COMMAND_OFFSET] = READ_MT25QL128_CONF_CMD; 
    }
    for (timeout = 0; timeout < 10000; timeout++) {
        qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
        usleep(1000);
    }
    if (timeout == 10000) {
        printf("QSPI Read Status Register Time out.\n");
    } else {
        printf("Confreg: 0x%x , StatusReg: 0x%x\n", Flashconf[1], FlashStatus[1]);
    }
    return (Flashconf[1]);
}

int zynq_qspi_rdcfg(void)
{
    uchar val_cfg;
    val_cfg = read_confreg_S25FL129P();

    return (PASSED);
}

/******************************************************************************
 * Function:    sector_lock_N25Q128
 * Description: This function locks specified sectors in QSPI flash of N25Q128 type
 * Input:       secnum - number of sectors to lock, can be 0, 2, 8, 32, 128, 256
 * Output:      PASSED/FAILED
 *
 *****************************************************************************/
int sector_lock_N25Q128(int secnum)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar WriteStatusCmd[2];  /* send 2 bytes */
    uchar WriteStatus[2];
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;
    uchar ans;

    printf("\nChoose protect mode (abcdef, other keys to quit:\n");
    printf("a: SPM2 protect from top of memory array\n");
    printf("b: SPM2 protect from bottom of memory array\n");
    printf("c: SPM2 unlock all sectors\n");
    printf("d: SPM1 lock, protection status reversible\n");
    printf("e: SPM1 lock, protection status cannot be changed except by a power-up\n");
    printf("f: SPM1 unlock\n");
    printf("enter protect mode: ");
    ans = getchar();
    getchar();

    WriteStatusCmd[0] = WRITE_STATUS_CMD2;
    switch(secnum) {
    case 2:
        WriteStatusCmd[1] = 0x0B;
        break;
    case 8 :
        WriteStatusCmd[1] = 0x13;
        break;
    case 32 :
        WriteStatusCmd[1] = 0x1B;
        break;
    case 128 :
        WriteStatusCmd[1] = 0x43;
        break;
    case 256 :
        WriteStatusCmd[1] = 0x47;
        break;
    default:
        WriteStatusCmd[1] = 0x03;
        break;
    }
    switch(ans) {
    case 'a':
        break;
    case 'b':
        WriteStatusCmd[1] |= 0x20;
        break;
    case 'c':
        WriteStatusCmd[1] = 0x03;
        break;
    case 'd':
        if (sector_lock_N25Q128_SPM1(secnum, 0x01)) {
            return FAILED;
        } else {
            return PASSED;
        }
    case 'e':
        if (sector_lock_N25Q128_SPM1(secnum, 0x03)) {
            return FAILED;
        } else {
            return PASSED;
        }
    case 'f':
        if (sector_lock_N25Q128_SPM1(secnum, 0x00)) {
            return FAILED;
        } else {
            return PASSED;
        }
    default:
        return PASSED;
    }

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);

    qspi_polledTransfer(WriteStatusCmd, WriteStatus, sizeof(WriteStatusCmd), QSPI_IS_INST);

    for (timeout = 0; timeout < 10000; timeout++) {
        qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
        usleep(1000);
    }
    if (timeout == 10000) {
        printf("QSPI Read Status Register Time out.\n");
    }
    return PASSED;
}

/******************************************************************************
 * Function:    sector_lock_N25Q128_SPM1
 * Description: This function locks specified sectors in QSPI flash of N25Q128 type
 * Input:       secnum - number of sectors to lock, can be 0, 2, 8, 32, 128, 256
 *              value  - lock register value
 * Output:      PASSED/FAILED
 *
 *****************************************************************************/
int sector_lock_N25Q128_SPM1(int secnum, uchar value)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;
    uint32_t testaddr;

    testaddr = (uint32_t)gethex_answer("qspi flash offset address to lock(0x0000000 - 0x0ffffff) ", 0, 0, 0x0ffffff);

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;

    /* Auto mode */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK;
    ControlReg &= ~QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    for (i = 0; i < secnum; i++) {
        qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
        /* Setup the write lock command */
        WriteBuffer[COMMAND_OFFSET]   = WRITE_LOCK_CMD;
        WriteBuffer[ADDRESS_1_OFFSET] = (uchar)(testaddr >> 16);
        WriteBuffer[ADDRESS_2_OFFSET] = (uchar)(testaddr >> 8);
        WriteBuffer[ADDRESS_3_OFFSET] = (uchar)(testaddr & 0xFF);
        WriteBuffer[DATA_OFFSET] = value;
        qspi_polledTransfer(WriteBuffer, ReadBuffer, 5, QSPI_IS_INST);

        for (timeout = 0; timeout < 10000; timeout++) {
            qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
            if ((FlashStatus[1] & 0x01) == 0) {
               break;
            }
            usleep(1000);
        }
        if (timeout == 10000) {
            printf("QSPI Read Status Register Time out.\n");
        }
        /* Setup the read lock command */
        WriteBuffer[COMMAND_OFFSET]   = READ_LOCK_CMD;
        WriteBuffer[ADDRESS_1_OFFSET] = (uchar)(testaddr >> 16);
        WriteBuffer[ADDRESS_2_OFFSET] = (uchar)(testaddr >> 8);
        WriteBuffer[ADDRESS_3_OFFSET] = (uchar)(testaddr & 0xFF);

        qspi_polledTransfer(WriteBuffer, ReadBuffer, 5, QSPI_IS_INST);
#ifdef QSPI_DEBUG
        printf("lock reg: 0x%x \n", ReadBuffer[4]);
#endif
        testaddr += SECTOR_SIZE;
    }

    return PASSED;
}

uchar S25FL128S_read_ASP(void)
{
    uchar ReadASPCmd[] = { READ_ASP_CMD, 0, 0 };  /* must send 3 bytes */
    uint32_t ControlReg;
    uchar ASPReg;
    int flash_type = qspi_rdidcfi();
    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    /* Setup the read ASP command */
    if (flash_type == MT25QL128) {
        ReadASPCmd[COMMAND_OFFSET] = MT25QL128_READ_ASPRD; 
    }
    qspi_polledTransfer(ReadASPCmd, ReadBuffer, 3, QSPI_IS_INST);
    printf("ASPR: 0x%x%x\n", ReadBuffer[2], ReadBuffer[1]);
    ASPReg = ReadBuffer[1];

    return ASPReg;
}

int S25FL128S_write_ASP(uchar ASPR)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar WriteASPCmd[] = { WRITE_ASP_CMD, ASPR, 0xff };  /* must send 3 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;
    int flash_type = qspi_rdidcfi();
    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
    /* Setup the write ASP command */
    if (flash_type == MT25QL128) {
        WriteASPCmd[COMMAND_OFFSET] = MT25QL128_ASPP; 
    }
    qspi_polledTransfer(WriteASPCmd, WriteASPCmd, 3, QSPI_IS_INST);
    for (timeout = 0; timeout < 10000; timeout++) {
        qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
        usleep(1000);
    }
    if (timeout == 10000) {
        printf("QSPI Read Status Register Time out.\n");
    }

    return PASSED;
}

uchar S25FL128S_check_PPB(int secnum, uint32_t testaddr)
{
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;
    uint32_t addr = testaddr;
    uchar PPBarray = 0;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) << QSPI_CONFIG_PRESC_SHIFT;

    /* Auto mode */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK;
    ControlReg &= ~QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    for (i = 0; i < secnum; i++) {
        /* Setup the read PPB command */
        WriteBuffer[COMMAND_OFFSET]   = READ_PPB_CMD;
        WriteBuffer[1] = 0;
        WriteBuffer[2] = (uchar)(addr >> 16);
        WriteBuffer[3] = (uchar)(addr >> 8);
        WriteBuffer[4] = (uchar)(addr & 0xFF);
        qspi_polledTransfer(WriteBuffer, ReadBuffer, 16, QSPI_IS_INST);

        for (timeout = 0; timeout < 10000; timeout++) {
            qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
            if ((FlashStatus[1] & 0x01) == 0) {
               break;
            }
            usleep(1000);
        }
        if (timeout == 10000) {
            printf("QSPI Read Status Register Time out.\n");
            break;
        }
        if (ReadBuffer[7] == 0x00) {
            PPBarray ++;
        }
        addr += SECTOR_SIZE;
    }

    return PPBarray;
}
int S25FL128S_read_PPB(int secnum, uint32_t testaddr)
{
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;
    uint32_t addr = testaddr;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) << QSPI_CONFIG_PRESC_SHIFT;

    /* Auto mode */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK;
    ControlReg &= ~QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    for (i = 0; i < secnum; i++) {
        /* Setup the read PPB command */
        WriteBuffer[COMMAND_OFFSET]   = READ_PPB_CMD;
        WriteBuffer[1] = 0;
        WriteBuffer[2] = (uchar)(addr >> 16);
        WriteBuffer[3] = (uchar)(addr >> 8);
        WriteBuffer[4] = (uchar)(addr & 0xFF);
        qspi_polledTransfer(WriteBuffer, ReadBuffer, 16, QSPI_IS_INST);

        for (timeout = 0; timeout < 10000; timeout++) {
            qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
            if ((FlashStatus[1] & 0x01) == 0) {
               break;
            }
            usleep(1000);
        }
        if (timeout == 10000) {
            printf("QSPI Read Status Register Time out.\n");
            break;
        }
        printf("PPB of sector 0x%x is : 0x%x\n", addr, ReadBuffer[7]);
        addr += SECTOR_SIZE;
    }

    return PASSED;
}
int S25FL128S_write_PPB(int secnum, uint32_t testaddr)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i, j;
    int timeout;
    uint32_t addr = testaddr;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) << QSPI_CONFIG_PRESC_SHIFT;

    /* Auto mode */
    ControlReg |= QSPI_CONFIG_SSFORCE_MASK;
    ControlReg &= ~QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    for (i = 0; i < secnum; i++) {
        int secnum_sub = 1;
        uint32_t    seclength = SECTOR_SIZE;
        /* For the first 2 64K sectors, the lock can only be performed on every 4K */
        if (addr < QSPI_PROTECT_FW_ADDR + SECTOR_SIZE * 2 ) {
            secnum_sub = 16;
            seclength = SECTOR_SIZE_SMALL;
        }

        for (j = 0; j < secnum_sub; j++) {
            qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
            /* Setup the write PPB command */
            WriteBuffer[COMMAND_OFFSET]   = WRITE_PPB_CMD;
            WriteBuffer[1] = 0;
            WriteBuffer[2] = (uchar)(addr >> 16);
            WriteBuffer[3] = (uchar)(addr >> 8);
            WriteBuffer[4] = (uchar)(addr & 0xFF);
            qspi_polledTransfer(WriteBuffer, ReadBuffer, 5, QSPI_IS_INST);

            for (timeout = 0; timeout < 10000; timeout++) {
                qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
                if ((FlashStatus[1] & 0x01) == 0) {
                break;
                }
                usleep(1000);
            }
            if (timeout == 10000) {
                printf("QSPI Read Status Register Time out.\n");
                break;
            }

            addr += seclength;
        }
    }

    return PASSED;
}

int S25FL128S_erase_PPB(void)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int timeout;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_4 & QSPI_CONFIG_PRESC_MAXIMUM) << QSPI_CONFIG_PRESC_SHIFT;

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
    /* Setup the erase PPB command */
    WriteBuffer[COMMAND_OFFSET] = ERASE_PPB_CMD;
    qspi_polledTransfer(WriteBuffer, ReadBuffer, 1, QSPI_IS_INST);
    for (timeout = 0; timeout < 10000; timeout++) {
        qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
        usleep(1000);
    }
    if (timeout == 10000) {
        printf("QSPI Read Status Register Time out.\n");
    }

    return PASSED;
}

uchar S25FL128S_read_PPBLock(void)
{
    uchar ReadPPBLockCmd[] = { READ_PPBL_CMD, 0 };  /* must send 3 bytes */
    uint32_t ControlReg;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    /* Setup the read PLB command */
    qspi_polledTransfer(ReadPPBLockCmd, ReadBuffer, 2, QSPI_IS_INST);
    printf("PLBR: 0x%x\n", ReadBuffer[1]);

    return ReadBuffer[1];
}

int S25FL128S_write_PPBLock(void)
{
    uchar WriteEnableCmd = { WRITE_ENABLE_CMD };
    uchar ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
    uchar FlashStatus[2];
    uint32_t ControlReg;
    int i;
    int timeout;

    assert(qspi_base_ptr);
    zynq_qspi_reset();

    /* set clk prescaler */
    ControlReg = qspi_readreg(QSPI_CONFIG_OFFSET);
    ControlReg &= ~QSPI_CONFIG_PRESC_MASK;
    ControlReg |= (uint32_t)(QSPI_CLK_PRESCALE_2 & QSPI_CONFIG_PRESC_MAXIMUM) <<
                             QSPI_CONFIG_PRESC_SHIFT;

    /* Manual mode */
    ControlReg |= QSPI_CONFIG_MANSTRTEN_MASK;
    qspi_writereg(QSPI_CONFIG_OFFSET, ControlReg);

    qspi_polledTransfer(&WriteEnableCmd, ReadBuffer, sizeof(WriteEnableCmd), QSPI_IS_INST);
    /* Setup the write PPB Lock command */
    WriteBuffer[COMMAND_OFFSET] = WRITE_PPBL_CMD;
    qspi_polledTransfer(WriteBuffer, ReadBuffer, 1, QSPI_IS_INST);
    for (timeout = 0; timeout < 10000; timeout++) {
        qspi_polledTransfer(ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd), QSPI_IS_INST);
        if ((FlashStatus[1] & 0x01) == 0) {
            break;
        }
        usleep(1000);
    }
    if (timeout == 10000) {
        printf("QSPI Read Status Register Time out.\n");
    }

    return PASSED;
}
int qspi_S25FL128S_ASP(int secnum, uint32_t aspaddr)
{
    assert(qspi_base_ptr);
    uchar ans;
    uchar reg;
    uchar exit = 0;

    while (!exit) {
        printf("\nASP operations (abcdefg) other keys to quit:\n");
        printf("a: read ASP register\n");
        printf("b: select Persistent Protection Mode\n");
        printf("c: read PPB access register\n");
        printf("d: program PPB bits of sectors\n");
        printf("e: clear PPB bits of all sectors\n");
        printf("f: read PPB lock register\n");
        printf("g: write PPB lock register\n");
        printf("enter protect mode: ");
        ans = getchar();
        getchar();

        switch(ans) {
        case 'a':
            S25FL128S_read_ASP();
            break;
        case 'b':
            reg = S25FL128S_read_ASP();
            reg &= 0xFD;
            S25FL128S_write_ASP(reg);
            break;
        case 'c':
            S25FL128S_read_PPB(secnum, aspaddr);
            break;
        case 'd':
            S25FL128S_write_PPB(secnum, aspaddr);
            break;
        case 'e':
            S25FL128S_erase_PPB();
            break;
        case 'f':
            S25FL128S_read_PPBLock();
            break;
        case 'g':
            S25FL128S_write_PPBLock();
            break;
        default:
            exit = 1;
            break;
        }
    }
    return PASSED;
}




/******** History ******** 
$Log: zynq_qspi.c,v $
Revision 1.6  2018/07/23 07:02:21  easochen
Support golden image protection with Micron flash

Revision 1.5  2013/12/20 08:17:55  xiaoyizh
Comment out some printf.
Return FAILED if qspi_erase() and qspi_write() timeout.
Modified the lock method for the first 2 sectors: For the first two 64K sectors, the lock can only be performed on every 4K.

Revision 1.4  2013/09/23 07:13:48  liwwang
add S25FL128S_check_PPB function to clean up the printout of S25FL128S_read_PPB

Revision 1.3  2013/09/03 06:35:15  liwwang
Support Advanced Sector Protection of Spansion S25FL128S flash.

Revision 1.2  2013/07/16 03:22:18  liwwang
add support for sector lock and new type flash

Revision 1.1  2013/04/19 07:17:53  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/

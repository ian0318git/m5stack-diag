/* $Id: prince_ge_dma.c,v 1.3 2013/08/02 09:19:40 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_ge_dma.c,v $
 *------------------------------------------------------------------
 *
 * prince_ge_dma.c - Prince GE DMA function.
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <assert.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "common_utils.h"
#include "pcmap.h"
#include "prince_reg.h"
#include "prince_def.h"
#include "prince_ge_dma.h"
#include "nvmonvars.h"

typedef struct dev_ge_dma_t_ {
    uchar       *rx_buffer_phys;
    uchar       *rx_buffer_virt;
    ulong       *rx_bd_phys;
    ulong       *rx_bd_virt;
    ulong       *tx_bd_phys;
    ulong       *tx_bd_virt;
} dev_ge_dma_t;

static dev_ge_dma_t dev_dma;

static reg_info_t ge_dma_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"GE DMA Control",                
     0x00,      READ_ONLY,  {4}, 0x801ce107, 0x80006000},
    {"GE Tx Drings Base Hi",       
     0x04,      READ_ONLY,  {4}, 0xffff0000, 0x00000000},
    {"GE Rx Dring Base Hi",     
     0x08,      READ_ONLY,  {4}, 0xffff0000, 0x00000000}, 
    {"GE Rx Buffer Size Register",   
     0x0c,      READ_ONLY,  {4}, 0x00003ffc, 0x00000000}, 
    {"GE Rx Flow Control", 
     0x10,      READ_WRITE, {4}, 0x03ff03ff, 0x000003ff},
    {"GE Tx Dring 0 Base/Tail",
     0x20,      READ_ONLY,  {4}, 0x0000fff8, 0x0},
    {"GE Tx Dring 0 Head",       
     0x24,      READ_ONLY,  {4}, 0x000007f8, 0x0},
    {"GE Tx Dring 0 Add",     
     0x28,      READ_ONLY,  {4}, 0x000000ff, 0x0}, 
    {"GE Tx Dring 0 Status",   
     0x2c,      READ_ONLY,  {4}, 0xf0000000, 0x0}, /* Write 1 to clear */
    {"GE Tx DMA 0 Error Enable",   
     0x30,      READ_WRITE, {4}, 0xf0000000, 0x0}, 
    {"GE Tx Dring 1 Base/Tail",
     0x40,      READ_ONLY,  {4}, 0x0000fff8, 0x0},
    {"GE Tx Dring 1 Head",       
     0x44,      READ_ONLY,  {4}, 0x000007f8, 0x0},
    {"GE Tx Dring 1 Add",     
     0x48,      READ_ONLY,  {4}, 0x000000ff, 0x0}, 
    {"GE Tx Dring 1 Status",   
     0x4c,      READ_ONLY,  {4}, 0xf0000000, 0x0}, /* Write 1 to clear */
    {"GE Tx DMA 1 Error Enable",   
     0x50,      READ_WRITE, {4}, 0xf0000000, 0x0}, 
    {"GE Tx Dring 2 Base/Tail",
     0x60,      READ_ONLY,  {4}, 0x0000fff8, 0x0},
    {"GE Tx Dring 2 Head",       
     0x64,      READ_ONLY,  {4}, 0x000007f8, 0x0},
    {"GE Tx Dring 2 Add",     
     0x68,      READ_ONLY,  {4}, 0x000000ff, 0x0}, 
    {"GE Tx Dring 2 Status",   
     0x6c,      READ_ONLY,  {4}, 0xf0000000, 0x0}, /* Write 1 to clear */
    {"GE Tx DMA 2 Error Enable",   
     0x70,      READ_WRITE, {4}, 0xf0000000, 0x0}, 
    {"GE Rx Dring Base/Head",
     0x120,     READ_ONLY,  {4}, 0x0000fff8, 0x0},
    {"GE Rx Dring Tail",       
     0x124,     READ_ONLY,  {4}, 0x000007f8, 0x0},
    {"GE Rx Dring Add",     
     0x128,     READ_ONLY,  {4}, 0x000000ff, 0x0}, 
    {"GE Rx DMA Status",   
     0x12c,     READ_ONLY,  {4}, 0xf0000000, 0x0}, /* Write 1 to clear */
    {"GE Rx DMA Error Enable",   
     0x130,     READ_WRITE, {4}, 0xf0000000, 0x0}, 
    {"GE Rx DMA Overflow Count",
     0x134,     READ_ONLY,  {4}, 0xf000ffff, 0x0},
    {"END",  0x000,  0,     {0}, 0x0,        0x0},
};

static reg_info_t ge_dma_intr_ctrl_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"GE Interrupt status",                
     0x140,      READ_ONLY,  {4}, 0x00070001, 0x0},
    {"GE Rx Interrupt Enable",       
     0x144,      READ_ONLY,  {4}, 0x00ff07fe, 0x000007fe},
    {"GE Tx Interrupt Enable",       
     0x148,      READ_ONLY,  {4}, 0x00070000, 0x0},
    {"GE Error Interrupt Status",     
     0x14c,      READ_ONLY,  {4}, 0x00078001, 0x0}, 
    {"GE Error Interrupt Enable",     
     0x150,      READ_ONLY,  {4}, 0x00078001, 0x0}, 
    {"GE AXI Error Address Latch",   
     0x154,      READ_ONLY,  {4}, 0xffffffff, 0x0}, 
    {"END",  0x000,   0,     {0}, 0x0,        0x0},
};

int ge_dma_reg_rd()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    ulong *reg_p;

    base_addr = get_ge_dma_base();

    offset = gethex_answer("\nEnter register offset[0x00 to 0x154]:",
               0, 0, 0x154);

    /* all the FPGA CSR registers are 4 bytes aligned */
    offset &= 0xfffc;

    if (check_offset(offset, ge_dma_reg_table) && 
        check_offset(offset, ge_dma_intr_ctrl_reg_table)) {
        perror("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        reg_p = (ulong *)(base_addr + offset);
        printf("\n reg_p is %#x.\n ", reg_p);
        reg_data = *reg_p;
        printf("\n register value @%#x = %#x ", (base_addr + offset), reg_data);
        return (PASSED);
    }
}

int ge_dma_reg_wr()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    ulong *reg_p;

    base_addr = get_ge_dma_base();

    offset = gethex_answer("\nEnter register offset[0x00 to 0x154]:",
               0, 0, 0x154);
    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
                0, 0, 0xffffffff);

    if (check_offset(offset, ge_dma_reg_table) &&
        check_offset(offset, ge_dma_intr_ctrl_reg_table)) {
        perror("\n Offset is invalid.\n ");
        return (FAILED);
    } else {
        /* Offset is valid */
        reg_p = (ulong *)(base_addr + offset);
        *reg_p = reg_data;
        printf("\n register value @%#x = %#x ", (base_addr + offset), *reg_p);
        return (PASSED);
    }
}

int ge_dma_reg_dp()
{
    ulong base_addr = get_ge_dma_base();
    reg_info_t *reg_table_p;

    printf("GE DMA Register Dump");

    reg_table_p = &ge_dma_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    reg_table_p = &ge_dma_intr_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    return (PASSED);
}

int ge_dma_reg_test()
{
    ulong ge_dma_base = get_ge_dma_base();

    testname("GE DMA Register");

    if ((register_tests(ge_dma_base, &ge_dma_reg_table[0]) != PASSED) ||
        (register_tests(ge_dma_base, &ge_dma_intr_ctrl_reg_table[0]) !=  PASSED)) {
        cterr('f', 0, "GE DMA register test failed.");
        return (FAILED);
    }

    prpass(testpass, "GE DMA Register test passed");
    return (PASSED);
}

int ge_tx_intr_test_one(ushort tx_dring)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;
    ulong intr_en = 0;
    ulong intr_sts = 0;
    ulong mask = 0;
    int i = 0;

    assert(tx_dring < 3);
    switch (tx_dring) {
    case 0:
        intr_en = PRINCE_GE_TX0_INTR_EN;
        intr_sts = PRINCE_GE_TX0_INTR_STS;
        break;
    case 1:
        intr_en = PRINCE_GE_TX1_INTR_EN;
        intr_sts = PRINCE_GE_TX1_INTR_STS;
        break;
    case 2:
        intr_en = PRINCE_GE_TX2_INTR_EN;
        intr_sts = PRINCE_GE_TX2_INTR_STS;
        break;
    default:
        break;
    }
    
    /* Write 1 to clear the pending interrupt bit first */
    ge_dma_reg_p->ge_intr_sts |= intr_sts;

    /* Enable interrupt */
    ge_dma_reg_p->ge_tx_intr_en |= intr_en;
    printf("Enable interrupt: "
            "ge_dma_reg_p->ge_tx_intr_en @%#x = %#x\n", 
            &ge_dma_reg_p->ge_tx_intr_en, ge_dma_reg_p->ge_tx_intr_en);

    /* Force interrupt -- 
       changes all of the GE DMA interrupt status bits from W1C to RW, 
       thus allowing software to force an interrupt condition 
       by writing a 1 to the appropriate status bit. */
    ge_dma_reg_p->ge_dma_ctrl |= PRINCE_GE_FORCE_IRQ;
    printf("Force interrupt: "
            "ge_dma_reg_p->ge_dma_ctrl @%#x = %#x\n", 
            &ge_dma_reg_p->ge_dma_ctrl, ge_dma_reg_p->ge_dma_ctrl);
    printf("TX Dring %d intr_en = %#x intr_sts = %#x\n", 
            tx_dring, intr_en, intr_sts);
    ge_dma_reg_p->ge_intr_sts |= intr_sts;

    /* Wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
        if ((ge_dma_reg_p->ge_intr_sts & intr_sts) == 0) {
            break;
        } else {
            printf(".");
            usleep(100);
        }
    }

    if (i == 5000) {
        cterr('f', 0, 
              "Timeout waiting for interrupt to be cleared. "
              "intr status %#x = %#x\n", 
              &ge_dma_reg_p->ge_intr_sts,
              ge_dma_reg_p->ge_intr_sts);
        return (FAILED);
    } else {
        return (PASSED);
    }
}

int ge_rx_intr_test()
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;
    ulong mask = 0;
    int i = 0;

    /* Clear the pending interrupt bit first */
    ge_dma_reg_p->ge_intr_sts |= PRINCE_GE_RX_INTR_STS;

    /* Enable interrupt */
    ge_dma_reg_p->ge_rx_intr_en |= PRINCE_GE_RX_INTR_EN;

    /* Force interrupt -- 
       changes all of the GE DMA interrupt status bits from W1C to RW, 
       thus allowing software to force an interrupt condition 
       by writing a 1 to the appropriate status bit. */
    ge_dma_reg_p->ge_dma_ctrl |= PRINCE_GE_FORCE_IRQ;
    ge_dma_reg_p->ge_intr_sts |= PRINCE_GE_RX_INTR_STS;

    /* Wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
        if ((ge_dma_reg_p->ge_intr_sts & PRINCE_GE_RX_INTR_STS) == 0) {
            break;
        } else {
            usleep(100);
        }
    }

    if (i == 5000) {
        cterr('f', 0, 
              "Timeout waiting for interrupt to be cleared. "
              "intr status = %#x\n", 
              ge_dma_reg_p->ge_intr_sts);
        return (FAILED);
    } else {
        return (PASSED);
    }
}

int ge_dma_intr_test()
{
    ushort tx_dring;

    testname("GE DMA Interrupt");

    if (ge_dma_init(&dev_dma)) {
        cterr('f', 0, "Failed to initialize DMA");
            return (FAILED);
    }

    /* Test all 3 TX Drings */
    for (tx_dring = 0; tx_dring < PRINCE_GE_DMA_TXBD_TYPE; tx_dring++) {
        prpass(testpass, "Test Tx dring %d interrupt", tx_dring);
        if (ge_tx_intr_test_one(tx_dring)) 
            return (FAILED);
    }

    /* Test RX Drings */
    prpass(testpass, "Test Rx dring interrupt");
    if (ge_rx_intr_test()) {
        return (FAILED);
    }

    return (PASSED);
}

/* The DMA memory is allocated by kernel driver,
   so we assign the physical & virtual memory address here. */
int ge_dma_buf_alloc(dev_ge_dma_t *dev_dma_p)
{
    /* Allocate RX buffer */
    dev_dma_p->rx_buffer_phys = (uchar *)get_ge_dma_rx_phys();
    dev_dma_p->rx_buffer_virt = (uchar *)get_ge_dma_rx_virt();

    /* 
     * 1024 ingress BD, each BD has 8 bytes, RXBD must be 8KB aligned.
     */
    dev_dma_p->rx_bd_phys = (ulong *)get_ge_rxbd_phys();
    dev_dma_p->rx_bd_virt = (ulong *)get_ge_rxbd_virt();

    /* 
     * 256 x 3 egress BD, each BD has 8 bytes, TXBD must be 2KB alighed.
     */
    dev_dma_p->tx_bd_phys = (ulong *)get_ge_txbd_phys();
    dev_dma_p->tx_bd_virt = (ulong *)get_ge_txbd_virt();

    return (PASSED);
}

/* The DMA memory is allocated by kernel driver, 
   and will not be freed until module exits.
   So we just set the pointers to NULL here. */
void ge_dma_buf_free(dev_ge_dma_t *dev_dma_p)
{
    dev_dma_p->rx_buffer_phys = NULL;
    dev_dma_p->rx_buffer_virt = NULL;
    dev_dma_p->rx_bd_phys = NULL;
    dev_dma_p->rx_bd_virt = NULL;
    dev_dma_p->tx_bd_phys = NULL;
    dev_dma_p->tx_bd_virt = NULL;
}

static int ge_dma_setup_rxbd(dev_ge_dma_t *dev_dma_p)
{
    ulong bd_index = 0;
    ulong offset = 0;

    for (; bd_index < PRINCE_GE_DMA_RXBD_NUM; ++bd_index, offset += DWDS_PER_BD) {
        /* Each Rx entry should be populated with a valid buffer pointer
           and done status clear, i.e. ownership set to the DMA. */
        dev_dma_p->rx_bd_virt[offset] = 0;
        dev_dma_p->rx_bd_virt[offset + 1] = (ulong) &dev_dma_p->rx_buffer_phys
            [bd_index * PRINCE_GE_DMA_RXBD_BUF_SIZE];

        if (dev_dma_p->rx_bd_virt[offset + 1] & 0x03) {
            cterr('f',0,"Ingress buffer must be DWD aligned.");
            return (FAILED);
        }
    }

    return (PASSED);
}

int ge_dma_read(dev_ge_dma_t *dev_dma_p, volatile uchar *rx_buf_virt, ulong *buf_size)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;
    volatile uchar *data = rx_buf_virt;
    ulong pse2_gmii_rx_buf_size = 0;
    int i, j;
    ulong size;
    int rc = PASSED;

    ulong rx_qhead;
    rx_qhead = getbits(ge_dma_reg_p->ge_rx_drings_base_head,
                        GE_DMA_RX_HEAD_MASK,
                        GE_DMA_RX_HEAD_SHIFT);

    for (i = 0; i < GE_RETRY_MAX; i++) {
        /* Clear interrupt */
        if (ge_dma_reg_p->ge_intr_sts & PRINCE_GE_RX_INTR_STS) {
            ge_dma_reg_p->ge_intr_sts &= PRINCE_GE_RX_INTR_STS;
        }
        /* polling */
        if (dev_dma_p->rx_bd_virt[rx_qhead * DWDS_PER_BD] & RXBD_DONE) {
            pse2_gmii_rx_buf_size = dev_dma_p->rx_bd_virt[rx_qhead * DWDS_PER_BD] & RXBD_SIZE_MASK;
            setbits(ge_dma_reg_p->ge_rx_drings_add, 
                    GE_DMA_RX_ADD_MASK, 
                    GE_DMA_RX_ADD_SHIFT, 
                    1);
            break;
        }

        usleep(GE_POLL_DELAY);
    }

    if (i == GE_RETRY_MAX) {
        cterr('f',0,"Data receive timeout, RXBD_DONE bit is not set");
        rc = FAILED;
    } else {
        for (j = 0; j < pse2_gmii_rx_buf_size; ++j) {
            *data++ = dev_dma_p->rx_buffer_virt
                      [rx_qhead * PRINCE_GE_DMA_RXBD_BUF_SIZE + j];
        }

        *buf_size = pse2_gmii_rx_buf_size;
        rc = PASSED;
    }

    /* clean the used RXBD */
    dev_dma_p->rx_bd_virt[rx_qhead * DWDS_PER_BD] = 0;
    memset((void *) &dev_dma_p->rx_buffer_virt[rx_qhead * PRINCE_GE_DMA_RXBD_BUF_SIZE],
       0, pse2_gmii_rx_buf_size);

    return(rc);
}

/* There're 3 tx drings */
static ulong get_tx_qhead(ushort txbd_index)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;

    assert (txbd_index < PRINCE_GE_DMA_TXBD_TYPE);

    switch(txbd_index) {
    case 0:
    return getbits(ge_dma_reg_p->ge_tx_drings_0_head,
                   GE_DMA_TX_HEAD_MASK,
                   GE_DMA_TX_HEAD_SHIFT);
    case 1:
    return getbits(ge_dma_reg_p->ge_tx_drings_1_head,
                   GE_DMA_TX_HEAD_MASK,
                   GE_DMA_TX_HEAD_SHIFT);
    case 2:
    return getbits(ge_dma_reg_p->ge_tx_drings_2_head,
                   GE_DMA_TX_HEAD_MASK,
                   GE_DMA_TX_HEAD_SHIFT);
    }
}

static ulong get_tx_qtail(ushort txbd_index)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;

    assert (txbd_index < PRINCE_GE_DMA_TXBD_TYPE);

    switch(txbd_index) {
    case 0:
    return getbits(ge_dma_reg_p->ge_tx_drings_0_base_tail,
                   GE_DMA_TX_TAIL_MASK,
                   GE_DMA_TX_TAIL_SHIFT);
    case 1:
    return getbits(ge_dma_reg_p->ge_tx_drings_1_base_tail,
                   GE_DMA_TX_TAIL_MASK,
                   GE_DMA_TX_TAIL_SHIFT);
    case 2:
    return getbits(ge_dma_reg_p->ge_tx_drings_2_base_tail,
                   GE_DMA_TX_TAIL_MASK,
                   GE_DMA_TX_TAIL_SHIFT);
    }
}

static void set_tx_qtail(ushort txbd_index, ulong value)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;

    assert (txbd_index < PRINCE_GE_DMA_TXBD_TYPE);

    switch(txbd_index) {
    case 0:
        setbits(ge_dma_reg_p->ge_tx_drings_0_base_tail,
                GE_DMA_TX_TAIL_MASK,
                GE_DMA_TX_TAIL_SHIFT,
                value);
    break;
    case 1:
        setbits(ge_dma_reg_p->ge_tx_drings_1_base_tail,
                GE_DMA_TX_TAIL_MASK,
                GE_DMA_TX_TAIL_SHIFT,
                value);
    break;
    case 2:
        setbits(ge_dma_reg_p->ge_tx_drings_2_base_tail,
                GE_DMA_TX_TAIL_MASK,
                GE_DMA_TX_TAIL_SHIFT,
                value);
    }
    return;
}

static void set_tx_add(ushort txbd_index, ulong value)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;

    assert (txbd_index < PRINCE_GE_DMA_TXBD_TYPE);

    switch(txbd_index) {
    case 0:
        setbits(ge_dma_reg_p->ge_tx_drings_0_add,
                GE_DMA_TX_ADD_MASK,
                GE_DMA_TX_ADD_SHIFT,
                value);
    break;
    case 1:
        setbits(ge_dma_reg_p->ge_tx_drings_1_add,
                GE_DMA_TX_ADD_MASK,
                GE_DMA_TX_ADD_SHIFT,
                value);
    break;
    case 2:
        setbits(ge_dma_reg_p->ge_tx_drings_2_add,
                GE_DMA_TX_ADD_MASK,
                GE_DMA_TX_ADD_SHIFT,
                value);
    }
    return;
}

int ge_dma_write(dev_ge_dma_t *dev_dma_p, 
                 ushort txbd_index, 
                 volatile uchar *data, 
                 ulong tx_size)
{
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;
    ulong head, tail;
    ulong intr_mask = 0;
    int rc = PASSED;
    int i;
    
    assert (txbd_index < PRINCE_GE_DMA_TXBD_TYPE);
    switch (txbd_index) {
    case 0:
        intr_mask = PRINCE_GE_TX0_INTR_STS;
    break;
    case 1:
        intr_mask = PRINCE_GE_TX1_INTR_STS;
    break;
    case 2:
        intr_mask = PRINCE_GE_TX2_INTR_STS;
    break;
    }

    /* check egress queue is not full */
    if (diagflag_xram & D_TRACE) {
        printf("check egress queue is not full...\n");
    }
    set_tx_add(txbd_index, 0);
    head = PRINCE_GE_DMA_TXBD_NUM * txbd_index + get_tx_qhead(txbd_index);
    tail = PRINCE_GE_DMA_TXBD_NUM * txbd_index + get_tx_qtail(txbd_index);
    if (((head + 1) % PRINCE_GE_DMA_TXBD_NUM) == tail) {
        cterr('f',0,"Egress queue is full.");
            return (FAILED);
    }

    if (tx_size > PRINCE_GE_DMA_TXBD_BUF_MAX) {
        cterr('f',0,"tx buffer size must not exceed %d.", PRINCE_GE_DMA_TXBD_BUF_MAX);
            return (FAILED);
    }

    /* setup TXD */
    if (diagflag_xram & D_TRACE) {
        printf("Setting up TXBD...\n");
    }    
    dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD] = TXBD_FIRST | TXBD_LAST | TXBD_INT
                                      | (tx_size &  TXBD_SIZE_MASK);

    dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD + 1] = (ulong)data;

    /* start transmit DMA by writing ADD register */
    if (diagflag_xram & D_TRACE) {
        printf("Start transmit DMA by writing ADD register...\n");
    }
    set_tx_add(txbd_index, 1);

    for (i = 0; i < GE_RETRY_MAX; i++) {
        /* Clear interrupt */
         if (ge_dma_reg_p->ge_intr_sts & intr_mask) {
             ge_dma_reg_p->ge_intr_sts &= intr_mask;
         }

        if (dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD] & TXBD_DONE) {
            break;
        }
        usleep(GE_POLL_DELAY);
    }

    if (i == GE_RETRY_MAX) {
        cterr('f',0,"Data transmit timeout, TXBD_DONE bit is not set");
        if (DIAGFLAG & D_VERBOSE) {
            printf("\n txbd@%#x = %#x, txbd@%#x = %#x\n", 
            &dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD], 
            dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD],
            &dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD + 1],
            dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD + 1]);
        }
        return (FAILED);
    }

    /* clean used TXBD */
    dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD] = 0;
    dev_dma_p->tx_bd_virt[tail * DWDS_PER_BD + 1] = 0;

    return rc;
}

static ulong imix_sizes[] = {
    64, 256, 570, 64, 64, 570, 64,
    1514, 570, 64, 64, 570, 0
};

#define DMA_QBASE_HI_MASK       0xFFFF0000
#define DMA_TX_QBASE_MASK       0x0000E000
#define DMA_RX_QBASE_MASK       0x0000E000

int ge_dma_init(dev_ge_dma_t *dev_dma_p)
{
    int i;
    ulong base_addr = get_ge_dma_base();
    ge_dma_reg_t *ge_dma_reg_p = (ge_dma_reg_t *)base_addr;

    printf("\nInitialize DMA.\n");

    /* Assert RESET */
    setbit(ge_dma_reg_p->ge_dma_ctrl, GE_DMA_RESET_MASK);

    /* Disable TX/RX */
    clrbit(ge_dma_reg_p->ge_dma_ctrl, GE_DMA_TX2_EN_MASK);
    clrbit(ge_dma_reg_p->ge_dma_ctrl, GE_DMA_TX1_EN_MASK);
    clrbit(ge_dma_reg_p->ge_dma_ctrl, GE_DMA_TX0_EN_MASK);

    /* Disable RX */
    clrbit(ge_dma_reg_p->ge_dma_ctrl, GE_DMA_RX_EN_MASK);

    /* Allocate Rx buffer and RXBD, TXBD first. */
    if (ge_dma_buf_alloc(dev_dma_p)) {
        cterr('f',0,"Failed to allocate Buffer/BD for GE DMA");
        return (FAILED);
    }

    /* Setup the Rx and Tx descriptor ring base addresses. */
    ge_dma_reg_p->ge_rx_drings_base_hi = (ulong)dev_dma_p->rx_bd_phys & DMA_QBASE_HI_MASK;
    ge_dma_reg_p->ge_tx_drings_base_hi = (ulong)dev_dma_p->tx_bd_phys & DMA_QBASE_HI_MASK;

    ge_dma_reg_p->ge_rx_drings_base_head = (ulong)dev_dma_p->rx_bd_phys & GE_DMA_RX_BASE_OFFSET_MASK;
    ge_dma_reg_p->ge_tx_drings_0_base_tail = (ulong)dev_dma_p->tx_bd_phys & GE_DMA_TX_BASE_OFFSET_MASK;
    ge_dma_reg_p->ge_tx_drings_1_base_tail = ge_dma_reg_p->ge_tx_drings_0_base_tail + 
        BYTES_PER_BD * PRINCE_GE_DMA_TXBD_NUM * 1;
    ge_dma_reg_p->ge_tx_drings_2_base_tail = ge_dma_reg_p->ge_tx_drings_0_base_tail +
        BYTES_PER_BD * PRINCE_GE_DMA_TXBD_NUM * 2;
    if (diagflag_xram & D_TRACE) {
        printf("ge_tx_drings_0_base_tail @%#x = %#x expect %#x\n", 
            &ge_dma_reg_p->ge_tx_drings_0_base_tail, ge_dma_reg_p->ge_tx_drings_0_base_tail,
            (ulong)dev_dma_p->tx_bd_phys & GE_DMA_TX_BASE_OFFSET_MASK);
        printf("ge_tx_drings_1_base_tail @%#x = %#x expect %#x\n", 
            &ge_dma_reg_p->ge_tx_drings_1_base_tail, ge_dma_reg_p->ge_tx_drings_1_base_tail,
            ge_dma_reg_p->ge_tx_drings_0_base_tail + 
            BYTES_PER_BD * PRINCE_GE_DMA_TXBD_NUM * 1);
        printf("ge_tx_drings_2_base_tail @%#x = %#x expect %#x\n", 
            &ge_dma_reg_p->ge_tx_drings_2_base_tail, ge_dma_reg_p->ge_tx_drings_2_base_tail,
            ge_dma_reg_p->ge_tx_drings_0_base_tail + 
            BYTES_PER_BD * PRINCE_GE_DMA_TXBD_NUM * 2);

        printf("\nSetup the Rx and Tx descriptor ring base addresses.\n"
                "ge_rx_drings_base_hi @%#x = %#x "
                "ge_tx_drings_base_hi @%#x = %#x "
                "ge_rx_drings_base_head @%#x = %#x \n"
                "ge_tx_drings_0_base_tail @%#x = %#x "
                "ge_tx_drings_1_base_tail @%#x = %#x "
                "ge_tx_drings_2_base_tail @%#x = %#x\n",
                &ge_dma_reg_p->ge_rx_drings_base_hi, ge_dma_reg_p->ge_rx_drings_base_hi,
                &ge_dma_reg_p->ge_tx_drings_base_hi, ge_dma_reg_p->ge_tx_drings_base_hi,
                &ge_dma_reg_p->ge_rx_drings_base_head, ge_dma_reg_p->ge_rx_drings_base_head,
                &ge_dma_reg_p->ge_tx_drings_0_base_tail, ge_dma_reg_p->ge_tx_drings_0_base_tail,
                &ge_dma_reg_p->ge_tx_drings_1_base_tail, ge_dma_reg_p->ge_tx_drings_1_base_tail,
                &ge_dma_reg_p->ge_tx_drings_2_base_tail, ge_dma_reg_p->ge_tx_drings_2_base_tail);

        printf("DEV DMA: rx_buffer_phys = %#x rx_buffer_virt = %#x\n"
                "\t rx_bd_phys = %#x rx_bd_virt = %#x\n"
                "\t tx_bd_phys = %#x tx_bd_virt = %#x\n",
                dev_dma_p->rx_buffer_phys, dev_dma_p->rx_buffer_virt,
                dev_dma_p->rx_bd_phys, dev_dma_p->rx_bd_virt,
                dev_dma_p->tx_bd_phys, dev_dma_p->tx_bd_virt);
    }
    /* Set Rx buffer size */
    if (diagflag_xram & D_TRACE) {
        printf("Set Rx buffer size\n");
    }
    setbits(ge_dma_reg_p->ge_rx_buffer_size, 
             GE_DMA_RX_BUF_SIZE_MASK, 
             GE_DMA_RX_BUF_SIZE_SHIFT, 
             PRINCE_GE_DMA_RXBD_BUF_SIZE);

    /* Set up RXBD*/
    if (diagflag_xram & D_TRACE) {
        printf("Set up RXBD\n");
    }
    memset((void *) dev_dma_p->rx_bd_virt, 0,
       sizeof(ulong) * DWDS_PER_BD * PRINCE_GE_DMA_RXBD_NUM);

    ge_dma_setup_rxbd(dev_dma_p);

    /* Set up flow control thresholds */
    if (diagflag_xram & D_TRACE) {
        printf("Set up flow control thresholds\n");
    }
    setbits(ge_dma_reg_p->ge_rx_flow_ctrl, 
             GE_DMA_XON_TRHD_MASK, 
             GE_DMA_XON_TRHD_SHIFT, 
             152);
    setbits(ge_dma_reg_p->ge_rx_flow_ctrl, 
             GE_DMA_XOFF_TRHD_MASK, 
             GE_DMA_XOFF_TRHD_SHIFT, 
             231);

    for (i = 0; i < PRINCE_GE_DMA_TXBD_TYPE; i++) {
        set_tx_qtail(i, 0);
    }

    /* Clear the pending interrupt bit first, write 1 to clear */
    if (diagflag_xram & D_TRACE) {
        printf("Clear the pending interrupt bit first, write 1 to clear\n");
    }
    ge_dma_reg_p->ge_intr_sts = ge_dma_reg_p->ge_intr_sts;

    /* Enable Tx interrupt */
    ge_dma_reg_p->ge_tx_intr_en = PRINCE_GE_TX0_INTR_EN | 
        PRINCE_GE_TX1_INTR_EN | 
        PRINCE_GE_TX2_INTR_EN;

    /* Enable Rx interrupt */
    ge_dma_reg_p->ge_rx_intr_en |= PRINCE_GE_RX_INTR_EN;

    /* Enable TX/RX, with reset asserted */
    ge_dma_reg_p->ge_dma_ctrl = GE_DMA_RESET_MASK | GE_DMA_AXI_MTU_MASK |
        GE_DMA_FLOW_CTRL_EN_MASK | GE_DMA_RX_EN_MASK |
        GE_DMA_TX2_EN_MASK | GE_DMA_TX1_EN_MASK | GE_DMA_TX0_EN_MASK;

    /* De-assert RESET */
    clrbit(ge_dma_reg_p->ge_dma_ctrl, GE_DMA_RESET_MASK);

    for (i = 0; i < PRINCE_GE_DMA_TXBD_TYPE; i++) {
        set_tx_add(i, 0);
    }
    setbits(ge_dma_reg_p->ge_rx_drings_add, 
            GE_DMA_RX_ADD_MASK, 
            GE_DMA_RX_ADD_SHIFT, 
            0);

    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: pkt_test_pattern().
 *
 * This function implements GMII test.
 *
 * Input:  dev_dma_p      - dev pointer.
 *         tx_buf_phys    - physical address for transmit buffer
 *         tx_buf_virt    - virtual address for transmit buffer
 *         rx_buf         - virtual address for receive buffer
 * 
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int 
pkt_test_pattern(dev_ge_dma_t *dev_dma_p, 
                volatile uchar *tx_buf_phys, 
                volatile uchar *tx_buf_virt, 
                volatile uchar *rx_buf)
{
    int i, j;
    int txbd_index;
    ulong size;
    ulong rx_size;
    int rc = PASSED;
    int num_pkt_size = 0;
    for (; imix_sizes[num_pkt_size]; ++num_pkt_size);

    for (i = 0; i < PRINCE_GE_PACKET_NUM; ++i) {
        size = imix_sizes[i % num_pkt_size];

        /* Test all 3 TX Drings */
        for (txbd_index = 0; txbd_index < PRINCE_GE_DMA_TXBD_TYPE; txbd_index++) {
            /* PULL mode */
            if (ge_dma_write(dev_dma_p, txbd_index, tx_buf_phys, size)) {
                rc = FAILED;
                cterr('f', 0 , "Data transmission(PULL mode) failed at Tx Dring %d "
                      "packet %d. Packet size = %d", txbd_index, i, size);
                break;
            }

            if (DIAGFLAG & D_VERBOSE) {
                printf("\nafter write, packet size = %d, packet num = %d", size, i);
                ge_dma_reg_dp();
            }

            if (ge_dma_read(dev_dma_p, rx_buf, &rx_size)) {
                rc = FAILED;
                cterr('f', 0 , "Failed to receive packet %d, tx dring %d", i, txbd_index);
                break;
            }

            if (rx_size != size) {
                cterr('f',0,"tx/rx size mismatch on TX Dring %d packet: %d: tx-%d rx-%d",
                    txbd_index, i, size, rx_size);
                rc = FAILED;
            } else {
                for (j = 0; j < rx_size; ++j) {
                    if (tx_buf_virt[j] != rx_buf[j]) {
                        cterr('f',0,"Data mismatch at location %d in packet %d! "
                          "\nread: %#x, expect: %#x", j, i, rx_buf[j], tx_buf_virt[j]);
                        rc = FAILED;       
                        break;
                    }
                }
            }
        }
        if (rc == FAILED) {
            if (DIAGFLAG & D_VERBOSE) {
                printf("tx_buf physical is located at: %#x, "
                    "tx_buf virtual is located at: %#x, "
                    "rx_buf virtual is located at: %#X\n",
                    tx_buf_phys, tx_buf_virt, rx_buf);
                ge_dma_reg_dp();
            }
            break;
        }
    }

    return (rc);
}

/*------------------------------------------------------------------------------
 *
 * Function: prince_pkt_lpbk_test().
 *
 * This function implements prince packet loopback test. 
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int 
prince_pkt_lpbk_test()
{
    volatile uchar *tx_buf_virt;
    volatile uchar *tx_buf_phys;
    volatile uchar *rx_buf_virt;
    int i, rc;

    if (ge_dma_init(&dev_dma)) {
        cterr('f', 0, "Failed to initialize DMA");
            return (FAILED);
    }

    tx_buf_virt = (uchar *)get_ge_dma_tx_virt();
    tx_buf_phys = (uchar *)get_ge_dma_tx_phys();

    /* Allocate temperate rx buffer here */
    rx_buf_virt = (uchar *)malloc(PRINCE_GE_DMA_TXBD_BUF_MAX);
    if (!rx_buf_virt) {
        free((void *)tx_buf_virt);
        cterr('f',0,"Can't allocate memory for Rx Buffer.");
        return(FAILED);
    }

    for (i = 0; i < 1600; ++i) {
        tx_buf_virt[i] = i & 0x00FF;
    }

    msleep(1000);

    rc = pkt_test_pattern(&dev_dma, tx_buf_phys, tx_buf_virt, rx_buf_virt);

    free((void *)rx_buf_virt);

    return (rc);
}

/******** History ********
$Log: prince_ge_dma.c,v $
Revision 1.3  2013/08/02 09:19:40  xiaoyizh
Add some trace log. Fix the alignment bug in register read utility.

Revision 1.2  2013/06/25 07:56:09  xiaoyizh
Move macro definitions to header file.

Revision 1.1  2013/04/19 07:17:51  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/

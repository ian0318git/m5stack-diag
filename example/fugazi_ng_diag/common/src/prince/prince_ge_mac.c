/* $Id: prince_ge_mac.c,v 1.3 2013/08/02 09:29:53 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/prince_ge_mac.c,v $
 *------------------------------------------------------------------
 *
 * prince_ge_mac.c - Prince GE MAC function.
 *
 * Xiaoying Zhang -- Nov. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include "common.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "pcmap.h"
#include "nvsysvars.h"
#include "common_utils.h"
#include "prince_reg.h"
#include "prince_def.h"
#include "prince_ge_mac.h"
#include "prince_eth_pkt.h"

extern int prince_set_packet(int port, int speed);

/* GE MAC Register Tables */
static reg_info_t ge_mac_stat_reg_table[] =
{
/*  Register name,
    Offset,     Type,       Size, Mask,     Reset Value */
    {"Received bytes Lower",
     0x00,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Received bytes Upper",
     0x04,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Transmitted bytes Lower",
     0x08,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Transmitted bytes Upper",
     0x0c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Undersize frames Lower",
     0x10,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Undersize frames Upper",
     0x14,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Fragment frames Lower",
     0x18,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Fragment frames Upper",
     0x1c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 64 byte frames Lower",
     0x20,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 64 byte frames Upper",
     0x24,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 65-127 byte frames Lower",
     0x28,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 65-127 byte frames Upper",
     0x2c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 128-255 byte frames Lower",
     0x30,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 128-255 byte frames Upper",
     0x34,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 256-511 byte frames Lower",
     0x38,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 256-511 byte frames Upper",
     0x3c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 512-1023 byte frames Lower",
     0x40,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 512-1023 byte frames Upper",
     0x44,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 1024-Max byte frames Lower",
     0x48,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx 1024-Max byte frames Upper",
     0x4c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Oversize frames Lower Lower",
     0x50,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Oversize frames Lower Upper",
     0x54,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 64 byte frames Lower",
     0x58,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 64 byte frames Upper",
     0x5c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 65-127 byte frames Lower",
     0x60,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 65-127 byte frames Upper",
     0x64,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 128-255 byte frames Lower",
     0x68,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 128-255 byte frames Upper",
     0x6c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 256-511 byte frames Lower",
     0x70,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 256-511 byte frames Upper",
     0x74,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 512-1023 byte frames Lower",
     0x78,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 512-1023 byte frames Upper",
     0x7c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 1024-Max byte frames Lower",
     0x80,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx 1024-Max byte frames Upper",
     0x84,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Oversize frames Lower Lower",
     0x88,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Oversize frames Lower Upper",
     0x8c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Frames Lower",
     0x90,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Frames Upper",
     0x94,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Frame Check Sequence Errors Lower",
     0x98,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Frame Check Sequence Errors Upper",
     0x9c,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Broadcast Lower",
     0xa0,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Broadcast Upper",
     0xa4,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Multicast Lower",
     0xa8,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Multicast Upper",
     0xac,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Control Lower",
     0xb0,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Control Upper",
     0xb4,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Length/Type Out of Range Lower",
     0xb8,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Length/Type Out of Range Upper",
     0xbc,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good VLAN Tagged Frames Lower",
     0xc0,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good VLAN Tagged Frames Upper",
     0xc4,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Pause Frames Lower",
     0xc8,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Good Pause Frames Upper",
     0xcc,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Bad Opcode Lower",
     0xd0,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Rx Bad Opcode Upper",
     0xd4,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Frames Lower",
     0xd8,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Frames Upper",
     0xdc,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Broadcast Lower",
     0xe0,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Broadcast Upper",
     0xe4,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Multicast Lower",
     0xe8,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Multicast Upper",
     0xec,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Underrun Errors Lower",
     0xf0,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Underrun Errors Upper",
     0xf4,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Control Frames Lower",
     0xf8,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Control Frames Upper",
     0xfc,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good VLAN Tagged Frames Lower",
     0x100,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good VLAN Tagged Frames Upper",
     0x104,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Pause Frames Lower",
     0x108,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Good Pause Frames Upper",
     0x10c,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Single Collision Frames Lower",
     0x110,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Single Collision Frames Upper",
     0x114,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Multiple Collision Frames Lower",
     0x118,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Multiple Collision Frames Upper",
     0x11c,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Deferred Lower",
     0x120,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Deferred Upper",
     0x124,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Late Collisions Lower",
     0x128,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Late Collisions Upper",
     0x12c,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Excess Collisions Lower",
     0x130,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Excess Collisions Upper",
     0x134,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Excess Deferral Lower",
     0x138,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Excess Deferral Upper",
     0x13c,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Alignment Errors Lower",
     0x140,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Tx Alignment Errors Upper",
     0x144,     READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"END",     0x000,  0,  {0}, 0x0,        0x0},
};

static reg_info_t ge_mac_cfg_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"Receiver Configuration Word 0",                
     0x00,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Receiver Configuration Word 1",       
     0x04,      READ_ONLY,  {4}, 0x7f00ffff, 0x00000008},
    {"Transmitter Configuration Word",     
     0x08,      READ_ONLY,  {4}, 0x7a000000, 0x10000000}, 
    {"Flow Control Configuration Word",   
     0x0c,      READ_ONLY,  {4}, 0x60000000, 0x60000000}, 
    {"MAC Speed Configuration Word", 
     0x10,      READ_ONLY,  {4}, 0xc0000000, 0x80000000},
    {"RX Max Frame Configuration Word", 
     0x14,      READ_WRITE, {4}, 0x00007fff, 0x000007d0},
    {"TX Max Frame Configuration Word",
     0x18,      READ_WRITE, {4}, 0x00007fff, 0x000007d0},
    {"ID Register",     
     0xf8,      READ_ONLY,  {4}, 0xffff00ff, 0x05030000},
    {"Ability Register",   
     0xfc,      READ_ONLY,  {4}, 0x00000f07, 0x00000f07},
    {"END",     0x000,  0,  {0}, 0x0,        0x0},
};

static reg_info_t ge_mdio_iface_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"MDIO Configuration word 0",                
     0x00,      READ_ONLY,  {4}, 0x0000003f, 0x0},
    {"MDIO Configuration word 1",       
     0x04,      READ_ONLY,  {4}, 0x1f1fc000, 0x0},
    {"MDIO Write Data",     
     0x08,      READ_ONLY,  {4}, 0x0000ffff, 0x0}, 
    {"MDIO Read Data",   
     0x0c,      READ_ONLY,  {4}, 0x0001ffff, 0x0}, 
    {"END",     0x000,  0,  {0}, 0x0,        0x0},
};

static reg_info_t ge_mac_intr_ctrl_reg_table[] =
{
/*  Register name,                  
    Offset,     Type,       Size, Mask,     Reset Value */
    {"Interrupt status",                
     0x00,      READ_WRITE, {4}, 0xffffffff, 0x0},
    {"Interrupt pending",       
     0x10,      READ_ONLY,  {4}, 0xffffffff, 0x0},
    {"Interrupt enable",     
     0x20,      READ_WRITE, {4}, 0xffffffff, 0x0}, 
    {"Interrupt clear",   
     0x30,      WRITE_ONLY, {4}, 0xffffffff, 0x0}, 
    {"END",  0x000,  0,     {0}, 0x0,        0x0},
};

static int ge_mac_stat_reg_test()
{
    ulong ge_mac_stat_base = get_ge_mac_base() + 
                             ZYNC_GE_MAC_STAT_COUNTER_OFFSET;
    prpass(testpass, "GE MAC statistics counter registers test.");
    return (register_tests(ge_mac_stat_base, 
                          &ge_mac_stat_reg_table[0]));
}

static int ge_mac_cfg_reg_test()
{
    ulong ge_mac_cfg_base = get_ge_mac_base() + 
                            ZYNC_GE_MAC_CFG_OFFSET;
    prpass(testpass, "GE MAC configuration registers test.");
    return (register_tests(ge_mac_cfg_base, 
                          &ge_mac_cfg_reg_table[0]));
}

static int ge_mdio_iface_reg_test()
{
    ulong ge_mdio_iface_base = get_ge_mac_base() + 
                               ZYNC_GE_MDIO_INTERFACE_OFFSET;
    prpass(testpass, "GE MDIO interface registers test.\n");
    return (register_tests(ge_mdio_iface_base, 
                          &ge_mdio_iface_reg_table[0]));
}

int ge_mac_reg_test()
{
    testname("GE MAC Register");

    if ((ge_mac_cfg_reg_test() != PASSED) ||
        (ge_mdio_iface_reg_test() != PASSED)) {
        cterr('f', 0, "GE MAC register test failed.");
        return (FAILED);
    }

    prpass(testpass, "GE MAC Register test passed");
    return (PASSED);
}

int check_mac_reg_offset (ushort offset)
{
    if (offset >= ZYNC_GE_INTR_OFFSET) {
        return (check_offset(offset - ZYNC_GE_INTR_OFFSET, 
                             ge_mac_intr_ctrl_reg_table));
    } else if (offset >= ZYNC_GE_MDIO_INTERFACE_OFFSET) {
        return (check_offset(offset - ZYNC_GE_MDIO_INTERFACE_OFFSET, 
                             ge_mdio_iface_reg_table));
    } else if (offset >= ZYNC_GE_MAC_CFG_OFFSET) {
        return (check_offset(offset - ZYNC_GE_MAC_CFG_OFFSET, 
                             ge_mac_cfg_reg_table));
    } else if (offset >= ZYNC_GE_MAC_STAT_COUNTER_OFFSET) {
        return (check_offset(offset - ZYNC_GE_MAC_STAT_COUNTER_OFFSET, 
                             ge_mac_stat_reg_table));
    }

    return (FAILED);
}

int ge_mac_reg_rd ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    ulong *reg_p;

    prpass(testpass, "GE MAC Register Read");

    base_addr = get_ge_mac_base();

    offset = gethex_answer("\nEnter register offset[0x200 to 0x630]:",
               0x200, 0x200, 0x630);

    if (check_mac_reg_offset(offset)) {
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

int ge_mac_reg_wr ()
{
    ulong base_addr;
    ushort offset;
    ulong reg_data;
    ulong *reg_p;

    prpass(testpass, "GE MAC Register Write");

    base_addr = get_ge_mac_base();

    offset = gethex_answer("\nEnter register offset[0x400 to 0x630]:",
               0x400, 0x400, 0x630);
    reg_data = gethex_answer("\nEnter write value[0x0 to 0xFFFFFFFF]:", 
                0, 0, 0xffffffff);

    if (check_mac_reg_offset(offset)) {
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

int ge_mac_reg_dp()
{
    ulong base_addr = get_ge_mac_base() + ZYNC_GE_MAC_STAT_COUNTER_OFFSET;
    reg_info_t *reg_table_p = &ge_mac_stat_reg_table[0];

    prpass(testpass, "GE MAC Register Dump");

    reg_dump(base_addr, reg_table_p);

    base_addr = get_ge_mac_base() + ZYNC_GE_MAC_CFG_OFFSET;
    reg_table_p = &ge_mac_cfg_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    base_addr = get_ge_mac_base() + ZYNC_GE_MDIO_INTERFACE_OFFSET;
    reg_table_p = &ge_mdio_iface_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    base_addr = get_ge_mac_base() + ZYNC_GE_INTR_OFFSET;
    reg_table_p = &ge_mac_intr_ctrl_reg_table[0];
    reg_dump(base_addr, reg_table_p);

    return PASSED;
}

int smi_reg_dp()
{
    ulong base_addr = get_ge_mac_base() + ZYNC_GE_MDIO_INTERFACE_OFFSET;
    reg_info_t *reg_table_p = &ge_mdio_iface_reg_table[0];

    reg_dump(base_addr, reg_table_p);

    return PASSED;}

int ge_mac_mdio_read (ushort offset, ushort *reg_data)
{
    int i;

    ulong base_addr = get_ge_mac_base() + ZYNC_GE_MDIO_INTERFACE_OFFSET;

    ge_mdio_cfg_reg_t *mdio_reg_p = (ge_mdio_cfg_reg_t *)base_addr;

    /* Write MDIO Configuration Word 1 */
    mdio_reg_p->mdio_cfg_wd_1 = ((1 << MDIO_INIT_SHIFT) & MAC_MDIO_INIT ) |
        ((2 << MDIO_TX_OP_SHIFT) & MAC_MDIO_TX_OP ) |
        ((offset << MDIO_TX_REGAD_SHIFT) & MAC_MDIO_TX_REGAD ) |
        ((PRINCE_PHY_ADDR << MDIO_TX_PHYAD_SHIFT) & MAC_MDIO_TX_PHYAD );

    /* Polling for ready status, total wait time is 20ms */
    for (i = 0; i < MDIO_WAIT_LOOP; i++) {
        /* When the MDIO Ready is re-asserted the read data is ready to be read. */
        if ((mdio_reg_p->mdio_cfg_wd_1 & MAC_MDIO_READY)) {
            break;
        }
        usleep(MDIO_DELAY * 10);
    }

    if (i == MDIO_WAIT_LOOP) {
        cterr('f',0,"Timeout waiting for MDIO read to complete.");
        return (FAILED);
    }

    *reg_data = mdio_reg_p->mdio_rx_data & MAC_MDIO_RD_DATA >> MDIO_RD_DATA_SHIFT;

    return (PASSED);
}

int ge_mac_mdio_write (ushort offset, ushort *reg_data)
{
    int i;

    ulong base_addr = get_ge_mac_base() + ZYNC_GE_MDIO_INTERFACE_OFFSET;

    ge_mdio_cfg_reg_t *mdio_reg_p = (ge_mdio_cfg_reg_t *)base_addr;

    /* Write data to TX reg first */
    mdio_reg_p->mdio_tx_data = (*reg_data << MDIO_WR_DATA_SHIFT) & MAC_MDIO_RD_DATA;

    /* Write MDIO Configuration Word 1 */
    mdio_reg_p->mdio_cfg_wd_1 = ((1 << MDIO_INIT_SHIFT) & MAC_MDIO_INIT ) |
        ((1 << MDIO_TX_OP_SHIFT) & MAC_MDIO_TX_OP ) |
        ((offset << MDIO_TX_REGAD_SHIFT) & MAC_MDIO_TX_REGAD ) |
        ((PRINCE_PHY_ADDR << MDIO_TX_REGAD_SHIFT) & MAC_MDIO_TX_PHYAD );

    /* Polling for ready status, total wait time is 20ms */
    for (i = 0; i < MDIO_WAIT_LOOP; i++) {
        /* When the MDIO Ready is re-asserted, the transaction has completed. */
        if ((mdio_reg_p->mdio_cfg_wd_1 & MAC_MDIO_READY)) {
            break;
        }
        usleep(MDIO_DELAY);
    }

    if (i == MDIO_WAIT_LOOP) {
        cterr('f',0,"Timeout waiting for MDIO write to complete.");
        return (FAILED);
    }

    return (PASSED);
}

int ge_mac_init (void) 
{
    ulong base_addr;
    sys_csr_reg_t * sys_reg_p;
    ge_mac_cfg_reg_t * ge_mac_cfg_p;
    ge_mdio_cfg_reg_t * ge_mdio_cfg_p;
    ge_intr_ctrl_cfg_reg_t * ge_intr_cfg_p;

    /* Write the MAC Control Register with MTU, IFG and values, 
       clear loopback and remove reset to MAC and PHY. */
    base_addr = get_fpga_base();
    sys_reg_p = (sys_csr_reg_t *)base_addr;
    sys_reg_p->mac_ctrl |= SYS_MAC_GE_PHY_RST | SYS_MAC_GE_RST | 
                           SYS_MAC_TX_FIFO_RST | SYS_MAC_RX_FIFO_RST;
    usleep(100);
    sys_reg_p->mac_ctrl = ((GE_RX_MTU << SYS_MAC_RX_MTU_SHIFT) & SYS_MAC_RX_MTU ) |
        ((GE_IFG_MIN << SYS_MAC_GE_IFG_SHIFT) & SYS_MAC_GE_IFG ) |
        ((GE_LPBK_MAC_EXT << SYS_MAC_GE_LPBK_SHIFT) & SYS_MAC_GE_LPBK ) |
        ((0 << SYS_MAC_TX_FIFO_RST_SHIFT) & SYS_MAC_TX_FIFO_RST ) |
        ((0 << SYS_MAC_RX_FIFO_RST_SHIFT) & SYS_MAC_RX_FIFO_RST );
        ((0 << SYS_MAC_GE_PHY_RST_SHIFT) & SYS_MAC_GE_PHY_RST ) |
        ((0 << SYS_MAC_GE_RST_SHIFT) & SYS_MAC_GE_RST );
    if (diagflag_xram & D_TRACE) {
        printf("Write the MAC Control Register\n"
            "sys_reg_p->mac_ctrl @%#x = %#x expect %#x\n", 
            &sys_reg_p->mac_ctrl,
            sys_reg_p->mac_ctrl,
            ((GE_RX_MTU << SYS_MAC_RX_MTU_SHIFT) & SYS_MAC_RX_MTU ) |
            ((GE_IFG_MIN << SYS_MAC_GE_IFG_SHIFT) & SYS_MAC_GE_IFG ) |
            ((GE_LPBK_MAC_EXT << SYS_MAC_GE_LPBK_SHIFT) & SYS_MAC_GE_LPBK ) |
            ((0 << SYS_MAC_GE_PHY_RST_SHIFT) & SYS_MAC_GE_PHY_RST ) |
            ((0 << SYS_MAC_GE_RST_SHIFT) & SYS_MAC_GE_RST ));
        get_gic_spi_status1();
    }

    /* Reset MAC receiver and set Rx enable, enable Jumbo frames and VLAN */
    base_addr = get_ge_mac_base() + ZYNC_GE_MAC_CFG_OFFSET;
    ge_mac_cfg_p = (ge_mac_cfg_reg_t *)base_addr;
    ge_mac_cfg_p->rx_cfg_wd_1 |= RX_RST | RX_EN | RX_JUMBO_FRAM_EN | RX_VLAN_EN;
    if (diagflag_xram & D_TRACE) {
        printf("Reset MAC receiver and set Rx enable\n"
            "ge_mac_cfg_p->rx_cfg_wd_1 @%#x = %#x expect %#x\n", 
            &ge_mac_cfg_p->rx_cfg_wd_1,
            ge_mac_cfg_p->rx_cfg_wd_1,
            (RX_RST | RX_EN | RX_JUMBO_FRAM_EN | RX_VLAN_EN));
    }

    /* Reset MAC transmitter and set Tx enable and set IFG */
    ge_mac_cfg_p->tx_cfg |= TX_RST | TX_EN | IFG_ADJ_EN | TX_JUMBO_FRAM_EN | TX_VLAN_EN;
    if (diagflag_xram & D_TRACE) {
        printf("Reset MAC transmitter and set Tx enable and set IFG\n"
            "ge_mac_cfg_p->tx_cfg @%#x = %#x expect %#x\n", 
            &ge_mac_cfg_p->tx_cfg,
            ge_mac_cfg_p->tx_cfg,
            (TX_RST | TX_EN | IFG_ADJ_EN | TX_JUMBO_FRAM_EN | TX_VLAN_EN));
    }

    /* Enable flow control and pause frames */
    ge_mac_cfg_p->flow_ctrl = ((1 << FLOW_CTRL_RX_EN_SHIFT) & FLOW_CTRL_RX_EN ) |
        ((1 << FLOW_CTRL_TX_EN_SHIFT) & FLOW_CTRL_TX_EN );
    if (diagflag_xram & D_TRACE) {
        printf("Enable flow control and pause frames\n"
            "ge_mac_cfg_p->flow_ctrl @%#x = %#x expect %#x\n", 
            &ge_mac_cfg_p->flow_ctrl,
            ge_mac_cfg_p->flow_ctrl,
            ((1 << FLOW_CTRL_RX_EN_SHIFT) & FLOW_CTRL_RX_EN ) |
            ((1 << FLOW_CTRL_TX_EN_SHIFT) & FLOW_CTRL_TX_EN ));
    }

    /* TX MTU setting */
    ge_mac_cfg_p->tx_max_fram_cfg = ((/*0x5ee*/0x5f2 << MAC_TX_MF_LEN_SHIFT) & MAC_TX_MF_LEN ) |
        ((1 << MAC_TX_MF_EN_SHIFT) & MAC_TX_MF_EN );
    if (diagflag_xram & D_TRACE) {
        printf("TX MTU setting\n"
            "ge_mac_cfg_p->tx_max_fram_cfg @%#x = %#x expect %#x\n", 
            &ge_mac_cfg_p->tx_max_fram_cfg,
            ge_mac_cfg_p->tx_max_fram_cfg,
            ((0x5f2 << MAC_TX_MF_LEN_SHIFT) & MAC_TX_MF_LEN ) |
            ((1 << MAC_TX_MF_EN_SHIFT) & MAC_TX_MF_EN ));
    }

    /* RX max allowable size */
    ge_mac_cfg_p->rx_max_fram_cfg = ((/*0x5ee*/0x5f2 << MAC_RX_MF_LEN_SHIFT) & MAC_RX_MF_LEN ) |
        ((1 << MAC_RX_MF_EN_SHIFT) & MAC_RX_MF_EN );
    if (diagflag_xram & D_TRACE) {
        printf("RX max allowable size\n"
            "ge_mac_cfg_p->rx_max_fram_cfg @%#x = %#x expect %#x\n", 
            &ge_mac_cfg_p->rx_max_fram_cfg,
            ge_mac_cfg_p->rx_max_fram_cfg,
            ((0x5f2 << MAC_RX_MF_LEN_SHIFT) & MAC_RX_MF_LEN ) |
            ((1 << MAC_RX_MF_EN_SHIFT) & MAC_RX_MF_EN ));
    }

    /* Enable the MDIO, and set the clock divide to 0x1d */
    base_addr = get_ge_mac_base() + ZYNC_GE_MDIO_INTERFACE_OFFSET;
    ge_mdio_cfg_p = (ge_mdio_cfg_reg_t *)base_addr;
    ge_mdio_cfg_p->mdio_cfg_wd_0 = ((1 << MAC_MDIO_EN_SHIFT) & MAC_MDIO_EN ) |
        ((0x1d << MAC_MDIO_CLK_DEV_SHIFT) & MAC_MDIO_CLK_DEV );
    if (diagflag_xram & D_TRACE) {
        printf("Enable the MDIO, and set the clock divide to 0x1d\n"
            "ge_mdio_cfg_p->mdio_cfg_wd_0 @%#x = %#x expect %#x\n", 
            &ge_mdio_cfg_p->mdio_cfg_wd_0,
            ge_mdio_cfg_p->mdio_cfg_wd_0,
            ((1 << MAC_MDIO_EN_SHIFT) & MAC_MDIO_EN ) |
            ((0x1d << MAC_MDIO_CLK_DEV_SHIFT) & MAC_MDIO_CLK_DEV ));
        get_gic_spi_status1();
    }

    /* Disable Interrupt */
    base_addr = get_ge_mac_base() + ZYNC_GE_INTR_OFFSET;
    ge_intr_cfg_p = (ge_intr_ctrl_cfg_reg_t *)base_addr;
    ge_intr_cfg_p->intr_en  &= ~MAC_MDIO_INTR_MASK;

    ge_mac_reg_dp();

    return (PASSED);
}

/* Set GE MAC loopback mode */
void set_mac_loopback(int lpbk_mode)
{
    ulong base_addr = get_fpga_base();
    sys_csr_reg_t * sys_reg_p;

    sys_reg_p = (sys_csr_reg_t *)base_addr;

    if (lpbk_mode == GE_LPBK_MAC_INT) {
        sys_reg_p->mac_ctrl |= SYS_MAC_GE_LPBK;
    } else {
        sys_reg_p->mac_ctrl &= ~SYS_MAC_GE_LPBK;
    }
}

int ge_mac_intr_test()
{
    ushort data;
    ulong base_addr = get_ge_mac_base() + ZYNC_GE_INTR_OFFSET;
    ge_intr_ctrl_cfg_reg_t *ge_mac_intr_p = (ge_intr_ctrl_cfg_reg_t *)base_addr;
    ulong mask = 0;
    int i = 0;
    int rc;

    testname("GE MAC Interrupt ");

    /* Clear the pending interrupt bit first */
    ge_mac_intr_p->intr_clr |= MAC_MDIO_INTR_MASK;

    /* Enable interrupt */
    ge_mac_intr_p->intr_en |= MAC_MDIO_INTR_MASK;

    /* Start a MDIO transaction to triggle the interrupt */
    if (ge_mac_mdio_read(0, &data)) {
        cterr('f', 0, "Failed to complete an MDIO transaction");
        ge_mac_intr_p->intr_en  &= ~MAC_MDIO_INTR_MASK;
        return (FAILED);
    }

    usleep(100);

    /* Wait 1 seconds here to let interrupt to be serviced and cleared. */
    for (i = 0; i < 5000; i++) {
        if ((ge_mac_intr_p->intr_sts & MAC_MDIO_INTR_MASK) == 0) {
            break;
        } else {
            printf(".");
            usleep(100);
        }
    }

    if (i == 5000) {
        cterr('f', 0, 
              "Timeout waiting for interrupt to be cleared. "
              "intr status = %#x\n", 
              ge_mac_intr_p->intr_sts);
        rc = (FAILED);
    } else {
        rc = (PASSED);
    }

    /* Disable interrupt */
    ge_mac_intr_p->intr_en &= ~MAC_MDIO_INTR_MASK;

    return rc;
}

int ge_mac_lpbk_test()
{
    int rc = PASSED;

    testname("GE MAC Loopback");

    /* Enable MAC Internal Loopback */
    prpass(testpass, "Enable MAC Internal Loopback");
    set_mac_loopback(GE_LPBK_MAC_INT);

    /* Start Packet Loopback Test */
    prpass(testpass, "Start Packet Loopback Test...");
    rc = prince_pkt_lpbk_test();

    /* Disable MAC Internal Loopback */
    prpass(testpass, "Disable MAC Internal Loopback");
    set_mac_loopback(GE_LPBK_MAC_EXT);

    if ( rc != PASSED) {
        cterr('f', 0, "MAC Internal Loopback Failed.");
    }

    return (rc);
}

int ge_mac_lpbk_test_raw_skt()
{
    int rc = PASSED;

    testname("GE MAC Loopback");

    /* Enable MAC Internal Loopback */
    prpass(testpass, "Enable MAC Internal Loopback");
    set_mac_loopback(GE_LPBK_MAC_INT);

    /* Start Packet Loopback Test */
    prpass(testpass, "Start Packet Loopback Test...");
    rc = prince_set_packet(0, SPD_1000MBPS);

    /* Disable MAC Internal Loopback */
    prpass(testpass, "Disable MAC Internal Loopback");
    set_mac_loopback(GE_LPBK_MAC_EXT);

    if (rc != PASSED) {
        cterr('f', 0, "MAC Internal Loopback Failed.");
    }

    return (rc);
}

/******** History ********
$Log: prince_ge_mac.c,v $
Revision 1.3  2013/08/02 09:29:53  xiaoyizh
Add new MAC loopback test routine using raw socket.

Revision 1.2  2013/06/25 08:06:33  xiaoyizh
Disable interrupt before exiting from the test.

Revision 1.1  2013/04/19 07:17:51  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/

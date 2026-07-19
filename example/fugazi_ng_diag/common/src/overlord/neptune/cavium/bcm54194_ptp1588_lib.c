/* $Id: bcm54194_ptp1588_lib.c,v 1.2 2018/05/18 09:24:53 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm54194_ptp1588_lib.c,v $
 *-----------------------------------------------------------------------------
 * bcm54194_ptp1588_lib.c - BCM54194 PTP script provided by BCM FAE.
 * Neptune no need to support PTP, the script has not been verified yet.
 *
 *
 * Oct 2017, Mecca Ho
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "types.h"
#include "common.h"
#include "cvmx.h"
#include "platform_eth.h"
#include "bcm54194_api.h"

/***********************************************************************
 *
 * Function: en_1588_mii_soft_rst
 *
 * Description: Reset the RDB 0x800 to 0xAFF and 1588 block reset
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_1588_mii_soft_rst (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val;
    int rdb_offset = BCM54194_TOP_MISC_TOP_GBL_RST_REG;

    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val |= (BCM54194_TOP_MII_REG_SOFT_RST_BIT | BCM54194_1588_RESET_BIT);
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_time_sync
 *
 * Description: 1. Enable TX/RX SOP capability in 10BASE-T mode
 *              2. Per-port Time Sync enable
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_time_sync (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val;
    int rdb_offset = BCM54194_TIME_SYNC_REG;

    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val |= (BCM54194_TX_SOP_10BT_EN_BIT | BCM54194_RX_SOP_10BT_EN_BIT | BCM54194_TIME_SYNC_EN_BIT);
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    rdb_offset = BCM54194_PATTERN_GEN_CTRL_REG;//note: the script for BCM54182 is 0x45;
    rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &reg_val);
    if (rc < 0) {
        printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
        return (rc);
    }
    reg_val |= (0x1000);
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_1588_txrx
 *
 * Description: enable 1588 TX/RX functionality
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_1588_txrx (int phy_addr, boolean tx, boolean rx)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0x0;
    int rdb_offset = BCM54194_P1588_SLICE_EN_CTRL_REG;

    if (tx) {
        reg_val |= 0xFF;
    }

    if (rx) {
        reg_val |= (0xFF << 8);
    }

    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_1588_sop
 *
 * Description: Select TX/RX SOP from PHY 1588 block
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_1588_sop (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0xFFFF;
    int rdb_offset = BCM54194_P1588_SOP_SELECTION_REG;

    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_udpcksum_rxtx
 *
 * Description: P1588 RX/TX CF + Insertion Register: En_RX_TX_CS_CF_UPD
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_udpcksum_rxtx (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0xFFFF;
    int rdb_offset = 0xAC0;

    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    rdb_offset = 0xAC1;
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_timecode_sel_80bit
 *
 * Description: Select 80-bit originTimestamp Counter for Timestamp FIFO or
 *              SOP memory ingress port.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_timecode_sel_80bit (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0xFFFF;
    int rdb_offset = BCM54194_P1588_TIMECODE_SEL_REG;

    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_rsvd2_format_sel
 *
 * Description: Enable 1 GHz clock for NSE (1 ns timestamp resolution)
 *              and enable control Reserve(1) and Reserve(2) timestamp format
 *
 * When 80-bit originTimestamp Counter is enabled
 * (P1588_INBAND_CONTROL Register, bit[13] = 1ﾡﾦb0). Done on a per-port basis.
 *   ﾡE Reserved(2) field bits[31:0] = bits[31:0] from 80-bit
 *     originTimestamp Counterﾡﾦs nanoseconds field.
 *   ﾡE Reserved(1) field bits[7:4] = bits[3:0] from 80-bit
 *     originTimestamp Counterﾡﾦs seconds field.
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_rsvd2_format_sel (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0x0100;
    int rdb_offset = BCM54194_P1588_DPLL_DEBUG_SELECT_REG;

    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_ibts_txrx_da_cap
 *
 * Description: Enable INBAND-Timestamp operation for event messages.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_ibts_txrx_da_cap (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0x40BF;
    int rdb_offset = BCM54194_P1588_INBAND_CTRL_PORT_REG;

    for (rdb_offset = BCM54194_P1588_INBAND_CTRL_PORT_REG;
         rdb_offset < BCM54194_P1588_INBAND_CTRL_PORT_REG+4; rdb_offset++) {
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc < 0) {
            printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_p47_ibts_t3_sopmem_cap
 *
 * Description: P1588_INBAND_CNTL: ds_ip_macip[15:14],
 *              up_DReq[12], sopmem_cap[10],en_ibts[3:0].
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_p47_ibts_t3_sopmem_cap (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0x14BF;
    int rdb_offset = 0xAF2;

    for (rdb_offset = BCM54194_P1588_INBAND_CTRL_PORT_REG;
         rdb_offset < BCM54194_P1588_INBAND_CTRL_PORT_REG+4; rdb_offset++) {
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc < 0) {
            printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n",
                    phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: ds_ibts_clock_id_cap
 *
 * Description: P1588 MPLS label1:8 mask: Disable P07
 *              Select clock ID capture[13].
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int ds_ibts_clock_id_cap (int phy_addr)
{
    int rc, bus_id = SMI_BUS_0;
    ushort reg_val = 0x0FFF;
    int rdb_offset;
    
    for (rdb_offset = BCM54194_P1588_MPLS_LABEL1_MASK_MSB_REG;
         rdb_offset <= BCM54194_P1588_MPLS_LABEL8_MASK_MSB_REG;
         rdb_offset+=4) {
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, reg_val);
        if (rc < 0) {
    	    printf("Failed to write GE PHY, phy addr:0x%x,"
                   "RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, reg_val);
            return (rc);
        }
    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: en_txrx_sop_ts_cap
 *
 * Description: enable global timestamp capture of
 *              each TX/RX port and access of timestamp
 *              through the PHYﾡﾦs MDIO interface (FIFO
 *              registers).
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int en_txrx_sop_ts_cap (int phy_addr, boolean tx, boolean rx)
{
    int rc, bus_id = SMI_BUS_0;
    ushort tx_reg_val = 0x0, rx_reg_val = 0x0, enable_val = 0xFF;
    int rdb_offset = BCM54194_P1588_TX_SOP_TS_CAP_EN_REG;

    rdb_offset = BCM54194_P1588_TX_SOP_TS_CAP_EN_REG;
    if (tx) {
    	tx_reg_val |= enable_val;
    }
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, tx_reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, tx_reg_val);
        return (rc);
    }

    rdb_offset = BCM54194_P1588_RX_SOP_TS_CAP_EN_REG;
    if (rx) {
    	rx_reg_val |= enable_val;
    }
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, rx_reg_val);
    if (rc < 0) {
    	printf("Failed to write GE PHY, phy addr:0x%x, RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, rx_reg_val);
        return (rc);
    }

    return (PASSED);
}

/***********************************************************************
 *
 * Function: enable_bcm54194_ibts_gm_sync_t1_ts_da_cap
 *
 * Description: The script has NOT been verified yet.
 * Enable PHY 54194 PTP engine, the script is provided by BCM FAE.
 * GM sends Sync with Sop_Mem_Cap bit set in Reseved1. Egress logic capture the T1 into
 * Sop_Mem_Cap, TSFIFO is used as memory. When GM sents follow up message with
 * SopMem_Update bit is set in Reseved1 egress logic compare seq_id, Domain_num,
 * Source IP/Mac address of the follow up message, if there is a match T1 is inserted into OTS.
 * Notice: In this mode, there is no FIFO since TSFIFO is used as mem.
 * 
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 * 
 *************************************************************************/
int enable_bcm54194_ibts_gm_sync_t1_ts_da_cap (int eth_port)
{
	boolean ptp1588_tx = TRUE, ptp1588_rx = FALSE;
	boolean ptp1588_tx_ts_cap = TRUE, ptp1588_rx_ts_cap = FALSE;

	en_1588_mii_soft_rst(eth_mapping_phy_addr[eth_port]);
	en_time_sync(eth_mapping_phy_addr[eth_port]);
	en_1588_txrx(eth_mapping_phy_addr[eth_port], ptp1588_tx, ptp1588_rx);
	en_1588_sop(eth_mapping_phy_addr[eth_port]);
	en_udpcksum_rxtx(eth_mapping_phy_addr[eth_port]);
	en_timecode_sel_80bit(eth_mapping_phy_addr[eth_port]);
	en_rsvd2_format_sel(eth_mapping_phy_addr[eth_port]);
	en_ibts_txrx_da_cap(eth_mapping_phy_addr[eth_port]);
	en_txrx_sop_ts_cap(eth_mapping_phy_addr[eth_port], ptp1588_tx_ts_cap, ptp1588_rx_ts_cap);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: enable_bcm54194_ibts_sc_sync_t2_ts_da_cap
 *
 * Description: The script has NOT been verified yet.
 * Enable PHY 54194 PTP engine, the script is provided by BCM FAE.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_bcm54194_ibts_sc_sync_t2_ts_da_cap (int eth_port)
{
	boolean ptp1588_tx = FALSE, ptp1588_rx = TRUE;
	boolean ptp1588_tx_ts_cap = FALSE, ptp1588_rx_ts_cap = TRUE;

	en_1588_mii_soft_rst(eth_mapping_phy_addr[eth_port]);
	en_time_sync(eth_mapping_phy_addr[eth_port]);
	en_1588_txrx(eth_mapping_phy_addr[eth_port], ptp1588_tx, ptp1588_rx);
	en_1588_sop(eth_mapping_phy_addr[eth_port]);
	en_udpcksum_rxtx(eth_mapping_phy_addr[eth_port]);
	en_timecode_sel_80bit(eth_mapping_phy_addr[eth_port]);
	en_rsvd2_format_sel(eth_mapping_phy_addr[eth_port]);
	en_ibts_txrx_da_cap(eth_mapping_phy_addr[eth_port]);
	en_txrx_sop_ts_cap(eth_mapping_phy_addr[eth_port], ptp1588_tx_ts_cap, ptp1588_rx_ts_cap);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: enable_bcm54194_ibts_sc_dreq_t3_assist
 *
 * Description: The script has NOT been verified yet.
 * Enable PHY 54194 PTP engine, the script is provided by BCM FAE.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_bcm54194_ibts_sc_dreq_t3_assist (int eth_port)
{
	boolean ptp1588_tx = TRUE, ptp1588_rx = TRUE;
	boolean ptp1588_tx_ts_cap = FALSE, ptp1588_rx_ts_cap = FALSE;

	en_1588_mii_soft_rst(eth_mapping_phy_addr[eth_port]);
	en_time_sync(eth_mapping_phy_addr[eth_port]);
	en_1588_txrx(eth_mapping_phy_addr[eth_port], ptp1588_tx, ptp1588_rx);
	en_1588_sop(eth_mapping_phy_addr[eth_port]);
	en_udpcksum_rxtx(eth_mapping_phy_addr[eth_port]);
	en_timecode_sel_80bit(eth_mapping_phy_addr[eth_port]);
	en_rsvd2_format_sel(eth_mapping_phy_addr[eth_port]);
	en_p47_ibts_t3_sopmem_cap(eth_mapping_phy_addr[eth_port]);
    ds_ibts_clock_id_cap(eth_mapping_phy_addr[eth_port]);
    en_txrx_sop_ts_cap(eth_mapping_phy_addr[eth_port], ptp1588_tx_ts_cap, ptp1588_rx_ts_cap);
    
    return (PASSED);
}

/***********************************************************************
 *
 * Function: enable_bcm54194_ibts_gm_rx_dreq_t4
 *
 * Description: The script has NOT been verified yet.
 * Enable PHY 54194 PTP engine, the script is provided by BCM FAE.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int enable_bcm54194_ibts_gm_rx_dreq_t4 (int eth_port)
{
	boolean ptp1588_tx = FALSE, ptp1588_rx = TRUE;
	boolean ptp1588_tx_ts_cap = FALSE, ptp1588_rx_ts_cap = TRUE;

	en_1588_mii_soft_rst(eth_mapping_phy_addr[eth_port]);
	en_time_sync(eth_mapping_phy_addr[eth_port]);
	en_1588_txrx(eth_mapping_phy_addr[eth_port], ptp1588_tx, ptp1588_rx);
	en_1588_sop(eth_mapping_phy_addr[eth_port]);
	en_udpcksum_rxtx(eth_mapping_phy_addr[eth_port]);
	en_timecode_sel_80bit(eth_mapping_phy_addr[eth_port]);
	en_rsvd2_format_sel(eth_mapping_phy_addr[eth_port]);
	en_ibts_txrx_da_cap(eth_mapping_phy_addr[eth_port]);
	en_txrx_sop_ts_cap(eth_mapping_phy_addr[eth_port], ptp1588_tx_ts_cap, ptp1588_rx_ts_cap);

    return (PASSED);
}

/***********************************************************************
 *
 * Function: read_timestamp
 *
 * Description: The script has NOT been verified yet.
 * Read P1588 Time Stamp Register2:0 & Info8:0
 *
 * Inputs: port number : 0, 1 , 2, 3
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int read_timestamp (int port_num)
{
    int rc, bus_id = SMI_BUS_0, ix = 0;
    int phy_addr = ge_port_mapping_phy_addr[port_num];
    ushort ts_start_end, ts_slice_sel = (port_num << 7);
    ushort ts0, ts1, ts2, ts3;
    ushort info1, info2, info3, info4, info5, info6, info7, info8;
    ushort domain_num, msg_type, tx_rx, vlan_id;
    int rdb_offset;
    
    switch (port_num) {
        /* Only one bit can be set. Multiple ports cannot be set. */
        case GE_PORT0:
            ts_start_end = 0x0001;
            break;
        case GE_PORT1:
            ts_start_end = 0x0004;
            break;
        case GE_PORT2:
            ts_start_end = 0x0010;
            break;
        case GE_PORT3:
            ts_start_end = 0x0040;
            break;
        default:
            ts_start_end = 0x0001;
            break;
    }
    rdb_offset = BCM54194_P1588_CTRL_DEBUG_REG;
    rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, ts_slice_sel);
    if (rc < 0) {
        printf("Failed to write GE PHY, phy addr:0x%x,"
               "RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, ts_slice_sel);
        return (rc);
    }

    for (ix = 0; ix <= 0x10; ix++) {
        rdb_offset = BCM54194_P1588_PCH_TS_FIFO_RD_START_END_REG;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, ts_start_end);
        if (rc < 0) {
    	    printf("Failed to write GE PHY, phy addr:0x%x,"
                   "RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, ts_start_end);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_FIFO_0_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &ts0);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_FIFO_1_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &ts1);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TAGID_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &ts2);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_CRC8_MISMATCH_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &ts3);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_1_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info1);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_2_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info2);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_3_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info3);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_4_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info4);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_5_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info5);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_6_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info6);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_7_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info7);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        rdb_offset = BCM54194_P1588_PCH_TS_INFO_8_REG;
        rc = bcm54194_rdb_read(bus_id, phy_addr, rdb_offset, &info8);
        if (rc < 0) {
            printf("Failed to read GE PHY, phy addr:%d, RDB offset:0x%x\n", phy_addr, rdb_offset);
            return (rc);
        }

        domain_num = info1 & 0xFF00;
        msg_type = info1 & 0x001E;
        tx_rx = info1 & 0x0001;
        vlan_id = info8 & 0x0FFF;
        printf("Timestamp%x = %x_%x_%x_%x\n", ix, ts3, ts2, ts1, ts0);
        printf("Domain number = %x\n", domain_num);
        printf("Message type = %x\n", msg_type);
        printf("TX/RX = %x\n", tx_rx);
        printf("Sequence ID = %x\n", info2);
        printf("source_port_clock_ID[63:48] = %x\n", info6);
        printf("MACDA/source_port_clock_ID[47:32] = %x\n", info5);
        printf("MACDA/IP/source_port_clock_ID[31:16] = %x\n", info4);
        printf("MACDA/IP/source_port_clock_ID[15:0] = %x\n", info3);
        printf("source_port_number[15:0] = %x\n", info2);
        printf("VLAN_ID = %x\n", vlan_id);

        rdb_offset = BCM54194_P1588_PCH_TS_FIFO_RD_START_END_REG;
        rc = bcm54194_rdb_write(bus_id, phy_addr, rdb_offset, (ts_start_end << 1));
        if (rc < 0) {
    	    printf("Failed to write GE PHY, phy addr:0x%x,"
                   "RDB offset:0x%x, value=0x%#.4x\n", phy_addr, rdb_offset, ts_start_end);
            return (rc);
        }

    }
    return (PASSED);
}

/***********************************************************************
 *
 * Function: dump_bcm54194_timestamp
 *
 * Description: Read PHY 54194 PTP timestamp, the script is provided by BCM FAE.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int dump_bcm54194_timestamp (void)
{
    read_timestamp(GE_PORT0);

    return (PASSED);
}
/*-------------------------------------------------
$Log: bcm54194_ptp1588_lib.c,v $
Revision 1.2  2018/05/18 09:24:53  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2017/10/31 07:07:14  meho
Separated BCM54194 PTP1588 LIB from bcm54194_api.c.


$Endlog$
*/

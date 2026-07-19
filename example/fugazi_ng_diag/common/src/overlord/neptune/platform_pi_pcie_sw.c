/* $Id: platform_pi_pcie_sw.c,v 1.2 2018/05/18 09:24:52 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/platform_pi_pcie_sw.c,v $
 *------------------------------------------------------------------
 * Filename   : platform_pi_pcie_sw.c
 *
 * Description: Neptune PCIe switch, Pericom PI7C9X2G1616PR,
 *              related diag tests and utilities.
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "plat_defs.h"
#include "platform_plx_pcie_sw.h"
#include "dash_fpga.h"
#include "linux_pciutils.h"
#include "error.h"

/*******************************************************************************
 *                                    Externs                                  *
 *******************************************************************************
 */
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);


/*******************************************************************************
 *
 * Function   : pi_rd_reg
 * Description:	Function to read from PCIe switch registers.
 * Inputs     :	reg_addr - Address of register.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pi_rd_reg (uint32_t reg_addr)
{
    uint32_t reg_val;
    uint32_t vend_id;
    uint32_t dev_id;
    uint32_t port0_bus_num;
    char *tname = "PCI read register";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    port0_bus_num = get_pcie_bus_num (PERICOM_PCIE_SW_VID, PERICOM_PCIE_SW_DID);
    /* read config register 00h */
    reg_val = pcie_config_read(0, port0_bus_num, 0, 0, 0x00);
    dev_id = reg_val;
    dev_id = (dev_id & 0xFFFF0000) >> 16;
    vend_id = reg_val;
    vend_id = (vend_id & 0x0000FFFF);

    if ((dev_id == PERICOM_PCIE_SW_DID) && (vend_id == PERICOM_PCIE_SW_VID)) {
        prpass(testpass, "PCIe switch device id %x, vendor id %x. ", dev_id, vend_id);
        return (PASSED);
    } else {
        cterr('f', 0, "Failed to get Neptune Pericom switch device id %x vendor id %x", dev_id, vend_id);
        return (FAILED);
    }
}
/*******************************************************************************
 *
 * Function   : pi_pcie_utp_ext_lpbk_test
 * Description:	Function to test PCIe switch ports externally by running
 *              User Test Pattern(UTP) Master loopback test.
 * Inputs     :	test_port - Number of the tested port
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int pi_pcie_utp_ext_lpbk_test (uint32_t test_port)
{

    uint32_t reg_val;
    uint32_t phy_layer_cmd;
    uint32_t utp_cmd;
    uint32_t diag_data_sel;
    int err_count;
    uint32_t lpbk_ready_bit; 

    uint32_t reg_phy_layer_cmd;
    uint32_t reg_utp_cmd;
    uint32_t reg_diag_data_sel;

    uint32_t port0_bus_num;
    uint32_t utp_cmd2;
    uint32_t diag_data_sel2;
    int err_count2 = 0;

    /* Assign bus number of PLX port 0 for different platform  */
    if (is_utah_plx() || is_juno_plx()) {
        port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8618);
    } else if (is_sword()) {
        port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8617);
    } else if (is_dagger()) {
        port0_bus_num = get_pcie_bus_num (PLX_PCIE_SW_VID, PLX_PCIE_SW_DID_8604);
    }

    /* Decide phy layer cmd register offset according to ODD/EVEN
     * test port number 
     */
    if (test_port & 0x1) {
        reg_phy_layer_cmd = PHYSICAL_LAYER_CMD_ODD_P;
    } else {
        reg_phy_layer_cmd = PHYSICAL_LAYER_CMD_EVEN_P;
    }

    switch (test_port) {
    case JUNO_UTAH_NGWIC1_PLX_PCIE_P: /* Also for SWORD_NGWIC2 */
        phy_layer_cmd = PLX_PORT3_PHY_LPBK_MASTER_EN ;
        lpbk_ready_bit = PLX_PORT3_PHY_LPBK_MASTER_READY ;
        utp_cmd = PLX_SERDES12_UTP_EN ;
        diag_data_sel = PLX_SERDES4_8_12_DIAG_DATA_SELECT ;
        reg_utp_cmd = UTP_EN_SERDES8_15;
        reg_diag_data_sel = SERDES_QUAD3_DIAG_DATA;
        break;
    case JUNO_UTAH_NGWIC2_PLX_PCIE_P:
        phy_layer_cmd = PLX_PORT11_PHY_LPBK_MASTER_EN ;
        lpbk_ready_bit = PLX_PORT11_PHY_LPBK_MASTER_READY ;
        utp_cmd = PLX_SERDES13_UTP_EN ;
        diag_data_sel = PLX_SERDES5_9_13_DIAG_DATA_SELECT ;
        reg_utp_cmd = UTP_EN_SERDES8_15;
        reg_diag_data_sel = SERDES_QUAD3_DIAG_DATA;
        break;
    case JUNO_UTAH_NGWIC3_PLX_PCIE_P:
        phy_layer_cmd = PLX_PORT13_PHY_LPBK_MASTER_EN ;
        lpbk_ready_bit = PLX_PORT13_PHY_LPBK_MASTER_READY ;
        utp_cmd = PLX_SERDES14_UTP_EN ;
        diag_data_sel = PLX_SERDES14_DIAG_DATA_SELECT ;
        reg_utp_cmd = UTP_EN_SERDES8_15;
        reg_diag_data_sel = SERDES_QUAD3_DIAG_DATA;
        break;
    case UTAH_NGSM1_PLX_PCIE_P: /* Also for SWORD_NGSM1 & DAGGER_NGWIC1 */
        phy_layer_cmd = PLX_PORT1_PHY_LPBK_MASTER_EN ;
        lpbk_ready_bit = PLX_PORT1_PHY_LPBK_MASTER_READY ;
        diag_data_sel = PLX_SERDES4_8_12_DIAG_DATA_SELECT ;
        diag_data_sel2 = PLX_SERDES5_9_13_DIAG_DATA_SELECT ;
        if (is_dagger()) {
            utp_cmd = PLX8604_ODDP_UTP_EN ;
            reg_utp_cmd = UTP_EN_ODDP_PLX8604;
            reg_diag_data_sel = SERDES_QUAD2_DIAG_DATA;
        } else {
            utp_cmd = PLX_SERDES8_UTP_EN ;
            utp_cmd2 = PLX_SERDES9_UTP_EN ;
            reg_utp_cmd = UTP_EN_SERDES8_15;
            reg_diag_data_sel = SERDES_QUAD2_DIAG_DATA;
        }
        break;
    case UTAH_NGSM2_PLX_PCIE_P: /* Also for SWORD_NGWIC1  */
        phy_layer_cmd = PLX_PORT2_PHY_LPBK_MASTER_EN ;
        lpbk_ready_bit = PLX_PORT2_PHY_LPBK_MASTER_READY ;
        utp_cmd = PLX_SERDES4_UTP_EN ;
        utp_cmd2 = PLX_SERDES5_UTP_EN ;
        diag_data_sel = PLX_SERDES4_8_12_DIAG_DATA_SELECT ;
        diag_data_sel2 = PLX_SERDES5_9_13_DIAG_DATA_SELECT ;
        reg_utp_cmd = UTP_EN_SERDES0_7;
        reg_diag_data_sel = SERDES_QUAD1_DIAG_DATA;
        break;
    case DAGGER_NGWIC2_PLX_PCIE_P:
        phy_layer_cmd = PLX_PORT5_PHY_LPBK_MASTER_EN ;
        lpbk_ready_bit = PLX_PORT5_PHY_LPBK_MASTER_READY ;
        utp_cmd = PLX8604_ODDP_UTP_EN ;
        diag_data_sel = PLX_SERDES5_9_13_DIAG_DATA_SELECT ;
        reg_utp_cmd = UTP_EN_ODDP_PLX8604;
        reg_diag_data_sel = SERDES_QUAD2_DIAG_DATA;
        break;
    default:
        printf("\nThe tested port number of PCIe switch is incorrect!!");
        break;
    }

    /* Set User Test Pattern content  */
    pcie_config_write(0, port0_bus_num, 0, 0, UTP_BYTE0_3, 0x03020100);
    pcie_config_write(0, port0_bus_num, 0, 0, UTP_BYTE4_7, 0x07060504);
    pcie_config_write(0, port0_bus_num, 0, 0, UTP_BYTE8_11, 0x0b0a0908);
    pcie_config_write(0, port0_bus_num, 0, 0, UTP_BYTE12_15, 0x0f0e0d0c);
    sleep(1);

    /* Set the loopback command bit according port number */
    pcie_config_write(0, port0_bus_num, 0, 0, reg_phy_layer_cmd, phy_layer_cmd);
    sleep(1);

    /* Check loopback ready bit */
    reg_val = pcie_config_read(0, port0_bus_num, 0, 0, reg_phy_layer_cmd);
    if (!(reg_val & lpbk_ready_bit)) {
        printf("(Info: PLX port %d Loopback ready bit is not set, continue to TX.)\n", test_port);
    }

    /* Enable transmission of the 128-bit test pattern  */
    pcie_config_write(0, port0_bus_num, 0, 0, reg_utp_cmd, utp_cmd);

    /* Select serdes diagnostic data according to port number  */
    pcie_config_write(0, port0_bus_num, 0, 0, reg_diag_data_sel, diag_data_sel);
    sleep(5);

    /* Read error count of the loopback test */
    reg_val = pcie_config_read(0, port0_bus_num, 0, 0, reg_diag_data_sel);
    err_count = (int)((reg_val & UTP_ERR_COUNT_MSK) >> UTP_ERR_COUNT_OFS);
    printf("(Info: PLX port %d Loopback error count= %d)\n", test_port, err_count);

    /* for second lane of sm slot pcie ports  */
    if ((is_utah_plx() && (test_port == UTAH_NGSM1_PLX_PCIE_P)) ||  
        (is_utah_plx() && (test_port == UTAH_NGSM2_PLX_PCIE_P)) ||
        (is_sword() && (test_port == SWORD_NGSM1_PLX_PCIE_P))) {
        /* Enable transmission of the 128-bit test pattern  */
        pcie_config_write(0, port0_bus_num, 0, 0, reg_utp_cmd, utp_cmd2);

        /* Select serdes diagnostic data according to port number  */
        pcie_config_write(0, port0_bus_num, 0, 0, reg_diag_data_sel, diag_data_sel2);
        sleep(5);

        /* Read error count of the loopback test */
        reg_val = pcie_config_read(0, port0_bus_num, 0, 0, reg_diag_data_sel);
        err_count2 = (int)((reg_val & UTP_ERR_COUNT_MSK) >> UTP_ERR_COUNT_OFS);
        printf("(Info: PLX port %d, 2nd lane Loopback error count= %d)\n", test_port, err_count2);
    }

    /*Set PCIe switch back to normal mode*/
    pcie_config_write(0, port0_bus_num, 0, 0, reg_utp_cmd, 0x0);
    pcie_config_write(0, port0_bus_num, 0, 0, reg_phy_layer_cmd, 0x0);

    if ((err_count != 0) || (err_count2 != 0)) {
        //cterr('f', 0, "port %d", test_port);
        return (FAILED);
    }

    return (PASSED);
}

/*
 *------------------------------------------------------------------
 * $Log: platform_pi_pcie_sw.c,v $
 * Revision 1.2  2018/05/18 09:24:52  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.6  2017/01/10 23:42:34  ptong
 * Print item skipped msg in the mb submenu
 *
 * Revision 1.1.2.5  2017/01/04 07:41:18  leschen
 * Modify pi_rd_reg function format.
 *
 * Revision 1.1.2.4  2016/10/10 23:45:15  leschen
 * Fix Neptune PCIe register test.
 *
 * Revision 1.1.2.3  2016/06/21 21:39:07  jskow
 * Add SM4 skeleton code, add eUSB/emmc check, add msata test
 *
 * Revision 1.1.2.2  2016/06/02 22:04:01  jskow
 * Move Overlord/x86 specific files to Neptune/x86.
 *
 * Revision 1.8  2014/08/05 13:50:42  danchung
 * fix typo
 *
 * Revision 1.7  2014/05/28 14:13:09  danchung
 * Add fix for sm testcard pcie loopback test intermittent error
 *
 * Revision 1.6  2014/01/02 09:52:14  danchung
 * Fix testcard pcie loopback test fail on Dagger
 *
 * Revision 1.5  2013/12/11 11:26:47  danchung
 * Fix pcie switch looppback test error due to rommon changed
 *
 * Revision 1.4  2013/11/20 20:14:57  ptong
 * Minor change to printf statement
 *
 * Revision 1.3  2013/11/19 13:40:52  danchung
 * Support testcard pcie lpbk test for Sword
 *
 * Revision 1.2  2013/11/01 12:48:11  danchung
 * Modify pcie switch port mapping defined name
 *
 * Revision 1.1  2013/10/22 14:32:34  danchung
 * Add support for PLX PCIe switch
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

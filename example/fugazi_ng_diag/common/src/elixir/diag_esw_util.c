/* $Id: diag_esw_util.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_esw_util.c,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "ethernet.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "diag_smi_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_cpu_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"

#include "dev_98dxc25x.h"
#include "dev_88e1680.h"
#include "cpss_extserv.h"


int diag_esw_pcie_config_rd_util (void);
int diag_esw_pcie_config_wr_util (void);
int diag_esw_xcat5_internal_reg_rd_util (void);
int diag_esw_xcat5_internal_reg_wr_util (void);
int diag_esw_xcat5_reg_rd_util (void);
int diag_esw_xcat5_reg_wr_util (void);
int diag_esw_phy_reg_rd_util (void);
int diag_esw_phy_reg_wr_util (void);
int diag_esw_phy_test_mode_util (void);

/******************************************************************************
 * Function: diag_esw_xcat5_internal_reg_rd_util
 *
 * Description: Utility to read AC5 register.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_xcat5_internal_reg_rd_util (void)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x xcat5 internal register read utility function */
    esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_internal_reg_read_util(dev, esw_98dxc25x_obj_p->cpss_dev);

    return (PASSED);

}

/******************************************************************************
 *
 * Function: diag_esw_xcat5_internal_reg_wr_util
 *
 * Description: Ethernet switch xCat5 internal register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat5_internal_reg_wr_util (void)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x xcat5 internal register write function */
    esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_internal_reg_write_util(dev, esw_98dxc25x_obj_p->cpss_dev);

    return (PASSED);
	
}

/******************************************************************************
 *
 * Function: diag_esw_pcie_config_rd_util
 *
 * Description: Ethernet switch PCIE config read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_pcie_config_rd_util (void)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x pcie config read utility function */
    esw_98dxc25x_obj_p->callin_fvt->esw_pcie_config_read_util(dev);

    return (PASSED);

}

/******************************************************************************
 *
 * Function: diag_esw_pcie_config_wr_util
 *
 * Description: Ethernet switch PCIE config write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_pcie_config_wr_util (void)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x pcie config write utility function */
    esw_98dxc25x_obj_p->callin_fvt->esw_pcie_config_write_util(dev);

    return (PASSED);

}

/******************************************************************************
 *
 * Function: diag_esw_xcat5_reg_rd_util
 *
 * Description: Ethernet switch xCat5 register read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat5_reg_rd_util (void)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x xcat5 register read utility function */
    esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_reg_read_util(dev, esw_98dxc25x_obj_p->cpss_dev);

    return (PASSED);
	
}

/******************************************************************************
 *
 * Function: diag_esw_xcat5_reg_wr_util
 *
 * Description: Ethernet switch xCat5 register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_xcat5_reg_wr_util (void)
{

    dev_98dxc25x_object_t *esw_98dxc25x_obj_p = NULL;
    dev_object_t *dev;

    /* Create 98dxc25x device driver */
    esw_98dxc25x_obj_p = (dev_98dxc25x_object_t *)diag_get_esw_98dxc25x_obj(); 
    if (esw_98dxc25x_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)esw_98dxc25x_obj_p;

    /* Call 98dxc25x xcat5 register write function */
    esw_98dxc25x_obj_p->callin_fvt->esw_xcat5_reg_write_util(dev, esw_98dxc25x_obj_p->cpss_dev);

    return (PASSED);

}

/******************************************************************************
 *
 * Function: diag_esw_phy_reg_rd_util
 *
 * Description: Ethernet switch PHY register read utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_reg_rd_util (void)
{

    MAD_DEV * mad_dev;
    uint port_num, page_num, reg_num, data;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, ELIXIR_ESW_PORT_NUM-1);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);

    mad_dev = &phy_mad_88e1680;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    /* Call 88e1680 PHY register read utility function */
    if (phy_88e1680_obj_p->callin_fvt->read_phy_reg_util(dev, mad_dev, port_num, 
                                                         page_num, reg_num, &data) != PASSED) {
        cterr('f',0,"Failed to read phy reg.", port_num);
        goto _exit;
    }

    printf("PHY port %d register value @ offset %d = %#x\n", port_num, reg_num, data&0xffff);

    return (PASSED);
	
 _exit:

    return (FAILED);

}

/******************************************************************************
 *
 * Function: diag_esw_phy_reg_wr_util
 *
 * Description: Ethernet switch PHY register write utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_reg_wr_util (void)
{

    MAD_DEV * mad_dev;
    uint port_num, page_num, reg_num, data;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, ELIXIR_ESW_PORT_NUM-1);
    page_num = getdec_answer("Enter PHY page number: ", 0, 0, 18);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);
	
    mad_dev = &phy_mad_88e1680;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    /* Call 88e1680 PHY register write utility function */
    if (phy_88e1680_obj_p->callin_fvt->write_phy_reg_util(dev, mad_dev, port_num, 
                                                          page_num, reg_num, data) != PASSED) {
        cterr('f',0,"Failed to read phy reg.", port_num);
        goto _exit;
    }

    return (PASSED);
	
 _exit:

    return (FAILED);

}

/******************************************************************************
 *
 * Function: diag_esw_phy_test_mode_util
 *
 * Description: Ethernet switch PHY test mode utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_test_mode_util (void)
{

    int rc = 0;
    MAD_DEV * mad_dev;
    uint32_t test_mode = 0, test_port, actual_port_num;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    while (1) {
        printf("Ethernet Switch PHY Test Mode:\n");
        printf("[0] Normal mode.\n");
        printf("[1] Test Mode 1 - Transmit Waveform Test.\n");
        printf("[2] Test Mode 2 - Transmit Jitter Test (Master mode).\n");
        printf("[3] Test Mode 3 - Transmit Jitter Test (Slave mode).\n");
        printf("[4] Test Mode 4 - Transmit Distortion Test.\n\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter the test mode: ", 0, 0, 0xF);
        if (test_mode == 0xF) {
            break;
        }
        test_port = gethex_answer("Enter Port number: ", 0, 0, ELIXIR_ESW_PORT_NUM-1);


        /* Map panel port number (Cisco defined) to actual port number (Foxconn HW defined) */
        if (test_port % 2 == 0) {
            actual_port_num = test_port + 1;
        } else {
            actual_port_num = test_port - 1;
        }

        mad_dev = &phy_mad_88e1680;

        if ((test_mode >= 0) && (test_mode < 5)) {
            rc = phy_88e1680_obj_p->callin_fvt->set_test_mode(dev, mad_dev, actual_port_num, test_mode);
            if (rc == FAILED) {
                return (FAILED);
            } 
        } else {
            printf("Wrong test mode!\n");
        }
    }
    
    return (PASSED);

}


/*-------------------------------------------------
 * $Log: diag_esw_util.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.17  2021/05/31 10:47:07  illiu
 * Remove function diag_esw_xcat5_phy_tx_config_read/write_util
 * Modify process of PHY Test Mode Utility
 *
 * Revision 1.1.2.16  2021/04/23 02:37:02  illiu
 * Use variable cpss_dev which is a member of 98dxc25x object, instead of using local variable
 *
 * Revision 1.1.2.15  2021/04/12 08:44:41  illiu
 * Replace object-create method as object-get method (Device driver object)
 *
 * Revision 1.1.2.14  2021/03/18 08:04:33  illiu
 * 1. Replace variable phy_dev_88e1680 with phy_mad_88e1680
 * 2. Remove redundant variable
 *
 * Revision 1.1.2.13  2021/03/04 06:16:21  illiu
 * Clean up code
 *
 * Revision 1.1.2.12  2021/01/29 09:22:44  illiu
 * Map panel port number (Foxconn defined) to actual port number (Marvell defined). Related item is PHY Test Mode Utility
 *
 * Revision 1.1.2.11  2021/01/26 03:24:09  illiu
 * Modify include file because of rename nim_dm prefix file
 *
 * Revision 1.1.2.10  2020/12/04 08:37:05  illiu
 * Add 1680 PHY Test Mode Utility
 *
 * Revision 1.1.2.9  2020/11/05 06:34:55  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.8  2020/10/15 12:05:18  illiu
 * 1. Move AC5 switch init and exit process to linux_main.c(It means do init once diag application is actived and do exit once diag application is exit)
 * 2. Add port configuration process for wifi6 module(XCAT5_TO_WIFI_PORT=26) which is connected to AC5 switch
 * 3. Add nim_dm driver polling, to check if driver is ready
 * 4. Add nim_dm driver polling, to check if driver exist before doing insmod or rmmod commend
 * 5. Modify the accessed path of pcie device in diag_esw_remove_pcie_device function
 * 6. Modify marvell_cpssPpInit_xcat5 and phy_dev_88e1680_group_start_addr to be static type variable
 * 7. Move array: phy_dev_88e1680 to header file
 * 8. Remove marvell_ac5_cpss_dev_num_elixir variable, and use ELIXIR_AC5_CPSS_DEV macro directly
 * 9. Modify AC5 switch test item name: External Loopback Test ==> PHY External Loopback Test
 * 10.Remove unneeded variable: port_group, port_group_phy_num
 * 11.Modify code alignment
 *
 * Revision 1.1.2.7  2020/10/07 11:21:10  illiu
 * Clean up code
 *
 * Revision 1.1.2.6  2020/10/07 09:19:45  illiu
 * Clean up code
 *
 * Revision 1.1.2.5  2020/10/06 02:06:28  illiu
 * Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
 *
 * Revision 1.1.2.4  2020/09/28 10:35:04  illiu
 * Add below utility items:
 * 1. ESW PHY Register Read Utility
 * 2. ESW PHY Register Write Utility
 * 3. ESW 88E1680 Tx Config Read Utility
 * 4. ESW 88E1680 Tx Config Write Utility
 *
 * Revision 1.1.2.3  2020/09/26 03:33:08  illiu
 * Add below Utilities items:
 *     ESW PCI Config Read Utility
 *     ESW PCI Config Write Utility
 *     ESW xCat3 Internal Register Write Utility
 *     ESW xCat3 PP Register Read Utility
 *     ESW xCat3 PP Register Write Utility
 *     Print All PHY Counter Utility
 *     Clear All PHY Counter Utility
 *     Print xCat3 Counter Utility
 *     Clear xCat3 Counter Utility
 *     ESW Reset Default Utility
 *
 * Revision 1.1.2.2  2020/09/10 09:52:28  illiu
 * Delete 88E6390/88E6176 Switch related code
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:22  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */

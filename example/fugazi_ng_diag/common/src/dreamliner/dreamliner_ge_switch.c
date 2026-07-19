/* $Id: dreamliner_ge_switch.c,v 1.7 2018/08/06 02:57:32 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/dreamliner_ge_switch.c,v $
 *------------------------------------------------------------------
 *
 * dreamliner_ge_switch.c - This file contains functions to init and control
 *                          Marvell GE switch.
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "common_utils.h"
#include "proto.h"
#include "pci.h"
#include "strings.h"
#include "queryflags.h"
#include "plat_defs.h"
#include "dash_fpga.h"
#include "cookie_4.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>      // uintxx_t
#include <stdbool.h>     // bool type
#include <sys/mman.h>    // mmap_xxx()

#include "dreamliner.h"
#include "dreamliner_ge_switch.h"
#include "nim_dm_cpss_extserv.h"

/* workaround to avoid build error. BIT_0 to BIT_31 are defined in common.h.
   And Marvell code defines them again.
*/
#undef BIT_0
#undef BIT_1
#undef BIT_2
#undef BIT_3
#undef BIT_4
#undef BIT_5
#undef BIT_6
#undef BIT_7
#undef BIT_8
#undef BIT_9
#undef BIT_10
#undef BIT_11
#undef BIT_12
#undef BIT_13
#undef BIT_14
#undef BIT_15
#undef BIT_16
#undef BIT_17
#undef BIT_18
#undef BIT_19
#undef BIT_20
#undef BIT_21
#undef BIT_22
#undef BIT_23
#undef BIT_24
#undef BIT_25
#undef BIT_26
#undef BIT_27
#undef BIT_28
#undef BIT_29
#undef BIT_30
#undef BIT_31

#include <generic/cpssTypes.h>
#include <generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/generic/init/cpssInit.h>
#include <cpss/generic/smi/cpssGenSmi.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <prvCpssGenDiag.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/include/mainPpDrv/h/cpss/generic/port/cpssPortCtrl.h>

extern uint32_t xcat2_init();
extern int get_sgmii_port_num(uint, uint);
extern int poe_init();
extern ushort board_id;
#ifdef TACHI_INTEL
static int config_port_speed(void);
static int config_port_vlan(void);
#endif 
static int dl_pcie_config_read_util(void);
static int dl_pcie_config_write_util(void);
static int peek_internal_reg(void);
static int poke_internal_reg(void);
static int peek_phy_reg(void);
static int poke_phy_reg(void);
static int peek_fpga_reg(void);
static int poke_fpga_reg(void);
static int dl_xcat2_reg_read_util(void);
static int dl_xcat2_reg_write_util(void);
static int xcat2_reg_read(ulong offset, int size, ulong *buf, void *param);
static int xcat2_reg_write(ulong offset, int size, ulong data, void *param);
int xcat2_mac_lpbk_test_ge0(void);
int xcat2_mac_lpbk_test_ge1(void);
int dl_volt_margin(int option);

GT_UINTPTR dl_get_bus_base_addr();
unsigned long dl_get_pci_base_addr();

int phy_in_reset();
int phy_out_of_reset();
int fpga_in_reset();
int fpga_out_of_reset();

uint32_t xcat2_dev_num[3] = {0,1,2};

static reg_info_t_ext reg_ext = {4, xcat2_reg_read, xcat2_reg_write, 0};

static reg_info_t xcat2_pci_reg_table[] =
{
/*  Register name,		Offset,		Type, Size,
 *		Mask, Reset Value
 */
    {"XCAT2_USER_DEFINED_REG_0",           0x000000f0,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"XCAT2_USER_DEFINED_REG_1",           0x000000f4,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"XCAT2_USER_DEFINED_REG_2",           0x000000f8,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
    {"XCAT2_USER_DEFINED_REG_3",           0x000000fc,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&reg_ext},
     0xffffffff, 0},
};

/* submenu for xCat2 utilities */
submenu_xtable_t xcat2_util_submenu_table[] = {
    {"PCI config read",  
     (PFT)dl_pcie_config_read_util,    0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"PCI config write",  
     (PFT)dl_pcie_config_write_util,   0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"XCAT2 register read",  
     (PFT)dl_xcat2_reg_read_util,      0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"XCAT2 register write",  
     (PFT)dl_xcat2_reg_write_util,     0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"XCAT2 config register read",  
     (PFT)peek_internal_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"XCAT2 config register write",  
     (PFT)poke_internal_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"peek PHY register",  
     (PFT)peek_phy_reg,                0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"poke PHY register",  
     (PFT)poke_phy_reg,                0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"put PHY in reset",  
     (PFT)phy_in_reset,                0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"take PHY out of reset",  
     (PFT)phy_out_of_reset,            0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"put FPGA in reset",  
     (PFT)fpga_in_reset,               0, 0, (type_t(*)())is_daughter_card_present, 0, 
     (type_t(*)())0, 0},
    {"take FPGA out of reset",  
     (PFT)fpga_out_of_reset,           0, 0, (type_t(*)())is_daughter_card_present, 0, 
     (type_t(*)())0, 0},
    {"peek FPGA register",  
     (PFT)peek_fpga_reg,               0, 0, (type_t(*)())is_daughter_card_present, 0, 
     (type_t(*)())0, 0},
    {"poke FPGA register",  
     (PFT)poke_fpga_reg,               0, 0, (type_t(*)())is_daughter_card_present, 0, 
     (type_t(*)())0, 0},
    {"xCat2 MAC loopback Test through GE0",
     (PFT)xcat2_mac_lpbk_test_ge0,     0, 0, (type_t(*)())0, 0,    
     (type_t(*)())0, 0},
    {"xCat2 MAC loopback Test through GE1",
     (PFT)xcat2_mac_lpbk_test_ge1,     0, 0, (type_t(*)())0, 0,    
     (type_t(*)())0, 0},
#ifdef TACHI_INTEL
    {"config port speed",  
     (PFT)config_port_speed,    0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"config port vlan",  
     (PFT)config_port_vlan,    0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
#endif 
};

#define XCAT2_UTIL_SUBMENU_TABLE_SIZE (sizeof(xcat2_util_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t xcat2_util_primary_items[XCAT2_UTIL_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t xcat2_util_secondary_items[XCAT2_UTIL_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t xcat2_util_menu = {
    "Marvell XCAT2 switch Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    xcat2_util_primary_items,
};
menuinfo_t *xcat2_util_submenup = &xcat2_util_menu;

/*
 * Margin Utilities Menu.
 */
static submenu_xtable_t mrgn_items[] = {
    {"Set 3.3V to Normal",	(PFT)dl_volt_margin, VTG_MRGN_SET_3_3V_NORM, 0,
	(type_t(*)())0,	0, (PFT)dl_volt_margin, VTG_MRGN_SET_3_3V_NORM},
    {"Set 3.3V Margin High", (PFT)dl_volt_margin, VTG_MRGN_SET_3_3V_HI, 0,
	(type_t(*)())0,	0, (PFT)dl_volt_margin, VTG_MRGN_SET_3_3V_HI},
    {"Set 3.3V Margin Low", (PFT)dl_volt_margin, VTG_MRGN_SET_3_3V_LO, 0,
	(type_t(*)())0,	0, (PFT)dl_volt_margin, VTG_MRGN_SET_3_3V_LO},
};
#define MARGIN_MENU_TABLE_SIZE (sizeof(mrgn_items) / sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mrgn_menu_primary_items[MARGIN_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];
static mitem_t mrgn_menu_secondary_items[MARGIN_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];

static struct menuinfo margindiag = {
  "Voltage Margin Utility Menu",	    /* title */
  0,                                /* title string added by init_empty_menu */
  0,				    /* shows major flags */
  0,                                /* generic prompt */
  0,				    /* size - bumped by add_menu_item */
  mrgn_menu_primary_items,
};
static struct menuinfo *margin_util_menup = &margindiag;

/*********************************************************************
 *
 * Function: xcat2_utils()
 *
 * Description: Build the primary & secondary submenus for the
 * xcat2 switch utility menu. 
 *
 * Inputs: none       
 * Outputs: PASSED
 *
 *********************************************************************
 */
int 
xcat2_utils ()
{
    build_primary_submenu(xcat2_util_submenu_table, 
			  XCAT2_UTIL_SUBMENU_TABLE_SIZE,
			  "Marvell XCAT2 switch Utility", &xcat2_util_submenup);
    build_secondary_submenu(xcat2_util_submenu_table,
			    XCAT2_UTIL_SUBMENU_TABLE_SIZE,
			    xcat2_util_secondary_items);

    menu(&xcat2_util_menu, xcat2_util_secondary_items, '\0');
    
    return PASSED;
}

#ifdef TACHI_INTEL
/*********************************************************************
 *
 * Function: config_port_speed
 *
 * Description: configure ge switch port speed mapping to PHY
 *
 * Inputs: none
 * Outputs: PASSED
 *
 *********************************************************************
 */
static int config_port_speed (void) 
{
    int rc, dev_num, port_num, speed; 

    dev_num = 0; 
    port_num = getdec_answer("Enter the port number 0-7:", 0, 0, 7);
    speed = getdec_answer("Enter the speed number 0-10, 1-100, 2-1000:", 0, 0, 2);

    if (speed == 0) {
        speed =  CPSS_PORT_SPEED_10_E;
    } else if  (speed == 1) { 
        speed =  CPSS_PORT_SPEED_100_E;
    } else {
        speed =  CPSS_PORT_SPEED_1000_E;
    }

    rc = cpssDxChPortSpeedSet(dev_num, port_num, speed);
    if(rc != GT_OK) {
       printf("Failed to call cpssDxChPortSpeedSet() for port %d,"
       " err code %d", port_num, rc);
        return rc;
    }
    return 0 ; 
}

/*********************************************************************
 *
 * Function: config_port_vlan
 *
 * Description: configure ge switch port speed to PHY
 *
 * Inputs: none
 * Outputs: PASSED
 *
 *********************************************************************
 */
static int config_port_vlan (void)
{
    int rc, dev_num, port_num, vlan_num, addrm; 

    dev_num = 0;

    printf("PHY port:0-7; GE0:24; GE1:25; \n");
    port_num = getdec_answer("Enter port number 0-7:", 0, 0, 25);
    if ((port_num == GE0_XCAT2_PORT) ||
        (port_num == GE1_XCAT2_PORT) || 
        (port_num < 9)) { 
        vlan_num = getdec_answer("Enter vlan number 1-5:", 1, 1, 5);
        addrm = getdec_answer("Add/Remove - 1/0:", 0, 0, 1);

        if (addrm == TRUE) {
            rc = xcat2_vlan_port_add(dev_num, vlan_num, port_num); 
            if(rc != GT_OK) {
               printf("Failed to add port: %d to Vlan %d .",
                    port_num, vlan_num); 
            }

        } else {
            rc = xcat2_vlan_port_del(dev_num, vlan_num, port_num); 
            if(rc != GT_OK) {
               printf("Failed to del port: %d from Vlan %d .",
                    port_num, vlan_num); 
            }
        }
        
        printf("====Switch to PHY port 0-7==== \n"); 
        for (port_num = 0; port_num < 8; port_num++) {
            vlan_num = xcat2_vlan_port_show(dev_num, port_num);  
            printf("Port%d - vlan%d\n", port_num, vlan_num); 
        }
            printf("====Switch to host GE0(24) and GE1(25)==== \n"); 
            port_num = GE0_XCAT2_PORT; 
            vlan_num = xcat2_vlan_port_show(dev_num, port_num);  
            printf("Port%d - vlan%d\n", port_num, vlan_num); 

            port_num = GE1_XCAT2_PORT; 
            vlan_num = xcat2_vlan_port_show(dev_num, port_num);  
            printf("Port%d - vlan%d\n", port_num, vlan_num); 
        return (rc);

    } else { 
        printf("Not support this port num:%d\n", port_num); 
        return (FAILED);
    }
}

/*********************************************************************
 *
 * Function: port_vlan_add
 *
 * Description: Add specific port to a VLAN
 *
 * Inputs: port: port_number
 *         vlan_num: VLAN id number
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
*/
int port_vlan_add (int vlan_num, int port_num)
{
    uint32_t rc = 0;
    GT_U32 dev_num = xcat2_dev_num[get_slot_num()-1];

    rc = xcat2_vlan_port_add(dev_num, vlan_num, port_num);
    if(rc != GT_OK) {
        cterr('f',0,"Failed to add port: %d to Vlan %d.", port_num, vlan_num);
        return (FAILED);
    }  

    return (PASSED);

}
#endif 

/******************************************************************************
 *
 * Function   :	dl_pcie_config_read_util
 * Description:	Utility to read PCIe configuration space register.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
dl_pcie_config_read_util (void)
{
    uint32_t reg_val = 0, reg_addr = 0;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);

    dl_pcie_config_read(reg_addr, (uint32_t *)&reg_val);

    printf("\n\n Value of 0x%05X is 0x%08X.\n\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dl_pcie_config_write_util
 * Description:	Utility to write PCIe configuration space register.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dl_pcie_config_write_util (void)
{
    uint32_t reg_addr = 0, write_in = 0;

    reg_addr = gethex_answer("Enter the address:", 0, 0, 0xFFFFF);
    write_in = gethex_answer("Enter the data:", 0, 0, 0xFFFFFFFF);

    dl_pcie_config_write(reg_addr, write_in);
    printf("\n\nDone.\n");

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	dl_xcat2_reg_read_util
 * Description:	Utility to read XCAT2 memory mapped registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
dl_xcat2_reg_read_util (void)
{
    uint32_t reg_val, reg_addr;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
#ifdef DEBUG
    /* This API can read pp registers without CPSS initialization */
    cpssDxChDiagRegRead(dl_get_bus_base_addr(), 3, 0, reg_addr, (uint32_t *)&reg_val, 0);
#else
    xcat2_reg_pci_read(reg_addr, (uint32_t *)&reg_val);    
#endif
    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	dl_xcat2_reg_write_util
 * Description:	Utility to write XCAT2 memory mapped registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
dl_xcat2_reg_write_util (void)
{
    uint32_t reg_addr, write_in, reg_val;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFFFFF);
    write_in = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    xcat2_reg_pci_write(reg_addr, write_in);

    xcat2_reg_pci_read(reg_addr, (uint32_t *)&reg_val);    
    printf("\nregister at offset %#x is %#x.\n", reg_addr, reg_val);


    return (PASSED);
}

#ifdef DEBUG
/******************************************************************************
 *
 * Function   :	poke_pp_reg
 * Description:	Utility to poke pp registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
poke_pp_reg (void)
{
    uint32_t reg_val = 0, reg_addr = 0;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0x3FFFFFF);
    reg_val = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    *(unsigned int *)(dl_get_bus_base_addr() + reg_addr) = reg_val;

    printf("\n register at offset %#x is %#x.\n", reg_addr, 
	   *(unsigned int *)(dl_get_bus_base_addr() + reg_addr));

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	peek_pp_reg
 * Description:	Utility to peek pp registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
peek_pp_reg (void)
{
    uint32_t reg_val = 0, reg_addr = 0;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0x3FFFFFF);

    reg_val = *(unsigned int *)(dl_get_bus_base_addr() + reg_addr);

    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}
#endif

/******************************************************************************
 *
 * Function   :	poke_internal_reg
 * Description:	Utility to poke internal registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
poke_internal_reg (void)
{
    uint32_t reg_val = 0, reg_addr = 0;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFF);
    reg_val = gethex_answer("Enter the register data:", 0, 0, 0xFFFFFFFF);

    *(unsigned int *)(dl_get_pci_base_addr() + reg_addr) = reg_val;

    printf("\n register at offset %#x is %#x.\n", reg_addr, 
	   *(unsigned int *)(dl_get_pci_base_addr() + reg_addr));

    return (PASSED);
}

/******************************************************************************
 *
 * Function   :	peek_inernal_reg
 * Description:	Utility to peek internal registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
peek_internal_reg (void)
{
    uint32_t reg_val = 0, reg_addr = 0;

    reg_addr = gethex_answer("Enter the register offset:", 0, 0, 0xFFFFF);

    reg_val = *(unsigned int *)(dl_get_pci_base_addr() + reg_addr);

    printf("\n register at offset %#x is %#x.\n", reg_addr, reg_val);

    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	xcat2_reg_pci_read
 * Description:	Read memory mapped xCat2 internal registers through 
 *              PCI interface.
 * Inputs     :	offset - register offset
 *              *data - pointer to hold the read data 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
xcat2_reg_pci_read (uint32_t offset, uint32_t *data)
{
    uint32_t rc;
    int port_group = 0;

    rc = cpssDrvPpHwRegisterRead(xcat2_dev_num[get_slot_num()-1], port_group,
                                 offset, (GT_U32 *)data);
    return (rc);
}

/******************************************************************************
 *
 * Function   :	xcat2_reg_pci_write
 * Description:	write to memory mapped xCat2 internal registers through 
 *              PCI interface.
 * Inputs     :	offset - register offset
 *              data - data to write 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
xcat2_reg_pci_write (uint32_t offset, uint32_t data)
{
    uint32_t rc;
    int port_group = 0;

    rc = cpssDrvPpHwRegisterWrite(xcat2_dev_num[get_slot_num()-1], port_group,
                                  offset, data);

    return (rc);    
}

/******************************************************************************
 *
 * Function   :	xcat2_reg_read
 * Description:	Wrapper for XCAT2 register test. 
 * Inputs: offset - register offset
 *	   size - Number of bytes to be read. XCAT2 registers are 4 bytes
 *	   buf  - points to the data buffer to hold read data.
 *	   param - Pointer to parameter
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat2_reg_read (ulong offset, int size, ulong *buf, void *param)
{
    return (xcat2_reg_pci_read(offset, (uint32_t *)buf));
}


/******************************************************************************
 *
 * Function   :	xcat2_reg_read
 * Description:	Wrapper for XCAT2 register test. 
 * Inputs: offset - register offset
 *	   size - Number of bytes to be read. XCAT2 registers are 4 bytes
 *	   data  - write data
 *	   param - Pointer to parameter
 *
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat2_reg_write (ulong offset, int size, ulong data, void *param)
{
    return (xcat2_reg_pci_write(offset, (uint32_t)data));
}


/******************************************************************************
 *
 * Function   :	xcat2_pci_reg_test
 * Description:	xCat2 PCIe configure registers test.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
xcat2_pci_reg_test (ushort id)
{
    int32_t  rc = 0;
    uint32_t data_rd = 0;
    uint32_t dev_rd = 0,ven_rd = 0;
    uint32_t pci_data_rd = 0;
    uint32_t dev_id = 0;

    prpass(testpass, "Marvell xCat2 PCI register test, ");
    cterr_setup();
    cterr_add_component("Marvell xCat2 switch", 
			"PCIe interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check PCIe interface from the router");
    /*
     * 4 port SKU : the device ID should be 0xE75A11AB (11AB is vendor ID which is Marvell)
     * 8 port SKUs: the device ID should be 0xE61E11AB (11AB is vendor ID which is Marvell)
     */

    /* Verify XCAT2 device ID through PCIe config space access */
    dl_pcie_config_read(XCAT2_PCI_VEN_DEV_ID_OFFSET, (uint32_t *)&pci_data_rd);
    if ((board_id == NIM_ES2_8P) || (board_id == NIM_ES2_8)) {
        dev_id = (XCAT2_DEVICE_ID << 16) | XCAT2_VENDOR_ID;
        if (pci_data_rd != dev_id) {
	        cterr('f',0,"PCI config read fails. Reg offset %#x, expected %#x, read %#x\n",
	        XCAT2_PCI_VEN_DEV_ID_OFFSET, dev_id, pci_data_rd);
	        return (FAILED);
        } else {
	        printf("\nXCAT2 PCIe config read vendor/device ID = 0x%08x\n", pci_data_rd);
        }
    }

    if (board_id == NIM_ES2_4) {
        dev_id = (DEVICE_ID_4P << 16) | XCAT2_VENDOR_ID;
        if (pci_data_rd != dev_id) {
	        cterr('f',0,"PCI config read fails. Reg offset %#x, expected %#x, read %#x\n",
	        XCAT2_PCI_VEN_DEV_ID_OFFSET, dev_id, pci_data_rd);
	        return (FAILED);
        } else {
	        printf("\nXCAT2 PCIe config read vendor/device ID = 0x%08x\n", pci_data_rd);
        }
    }
    /* Verify XCAT2 device ID through PCIe memory mapped access */
    rc = xcat2_reg_pci_read(XCAT2_DEVICE_ID_REG, (uint32_t *)&dev_rd);
    if (rc) {
        cterr('f',0,"xcat2_reg_pci_read() failed, rc = 0x%x\n", rc);
    }
    rc = xcat2_reg_pci_read(XCAT2_VENDOR_ID_REG, (uint32_t *)&ven_rd);
    if (rc) {
        cterr('f',0,"xcat2_reg_pci_read() failed, rc = 0x%x\n", rc);
    }
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("XCAT2_DEVICE_ID_REG = 0x%08x\n", dev_rd);
        printf("XCAT2_VENDOR_ID_REG = 0x%08x\n", ven_rd);
    }    
    data_rd = ((dev_rd & 0x000FFFF0)<< 12) | ven_rd;
    if ((board_id == NIM_ES2_8P) || (board_id == NIM_ES2_8)) {
        dev_id = (XCAT2_DEVICE_ID << 16) | XCAT2_VENDOR_ID;
        if (data_rd != dev_id) {
	        cterr('f',0,"xCat2 memory mapped register read fails." 
	            "Reg offset 0x%08x / 0x%08x, expected 0x%08x, read 0x%08x\n",
	            XCAT2_DEVICE_ID_REG, XCAT2_VENDOR_ID_REG, dev_id, data_rd);
	        return (FAILED);
        } else {
	        printf("XCAT2 memory mapped vendor/device ID = 0x%08x\n", data_rd);
        }
    }
    if (board_id == NIM_ES2_4) {
        dev_id = (DEVICE_ID_4P << 16) | XCAT2_VENDOR_ID;
        if (data_rd != dev_id) {
	        cterr('f',0,"xCat2 memory mapped register read fails." 
	            "Reg offset 0x%08x / 0x%08x, expected 0x%08x, read 0x%08x\n",
	            XCAT2_DEVICE_ID_REG, XCAT2_VENDOR_ID_REG, dev_id, data_rd);
	        return (FAILED);
        } else {

	        printf("XCAT2 memory mapped vendor/device ID = 0x%08x\n", data_rd);
        }
    }
    return (register_tests(0, &xcat2_pci_reg_table[0])); 
}

/******************************************************************************
 *
 * Function   :	xcat2_all_reg_test
 * Description:	Test xCat2 all internal registers by calling CPSS API.
 *              Original value is restored after test completes.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
xcat2_all_reg_test (void)
{
    int rc = 0;
    GT_U32  badReg       = 0;
    GT_U32  readVal      = 0;
    GT_U32  writeVal     = 0;
    GT_BOOL testStatus   = 0;

    cterr_setup();
    prpass(testpass, "Marvell xCat2 all register test, ");
    cterr_add_component("Marvell xCat2 switch", 
			"PCIe interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check PCIe interface from the router");

    rc = cpssDxChDiagAllRegTest(xcat2_dev_num[get_slot_num()-1],
                                &testStatus,
                                &badReg,
                                &readVal,
                                &writeVal);

    if (rc == GT_OK) {
        if (testStatus == GT_FALSE) {
            cterr('f', 0, "XCAT2 switch all register test failed.\n"
		  "Reg 0x%08x, readVal 0x%08x, writeVal 0x%08x\n",
		  badReg, readVal, writeVal);
            rc = FAILED;
        } else {
            printf("\nxCat2 all register test passed !!!\n");
        }
    } else {
        cterr('f', 0, "Error returned by cpssDxChDiagAllRegTest(), rc 0x%x\n", rc);        
    }

    return (rc);
}

/******************************************************************************
 *
 * Function   :	xcat2_temperature_test
 * Description:	READ xCat2 internal temperature by calling CPSS API.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
xcat2_temperature_test (void)
{
    int rc = 0;
    GT_32  devTemp       = 0;

    cterr_setup();
    prpass(testpass, "Marvell xCat2 temperature test, ");
    cterr_add_component("Marvell xCat2 switch", 
			"PCIe interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check PCIe interface from the router");

    rc = cpssDxChDiagDeviceTemperatureGet(xcat2_dev_num[get_slot_num()-1], &devTemp);

    if (rc == GT_OK) {
        printf("\nXCAT2 device temperature = %d DegC \n\n", devTemp); 
    } else {
        cterr('f', 0, "Error returned by cpssDxChDiagDeviceTemperatureGet(), rc 0x%x\n", rc);
    }

    return (rc);
}


#ifdef DEBUG
/******************************************************************************
 *
 * Function   :	xcat2_int_mem_test
 * Description:	This test is done by invoking Marvel API in loop for all 
 *              the memory types and for AA-55, random and incremental 
 *              patterns. The test is destructive and leaves the memory 
 *              corrupted. 
 *              The xCat2 device needs to be reinitialized after test done. 
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int 
xcat2_int_mem_test (void)
{
    int rc = 0;
    GT_U32  addr       = 0;
    GT_U32  readVal    = 0;
    GT_U32  writeVal   = 0;
    GT_BOOL testStatus = 0;

    rc = cpssDxChDiagAllMemTest(xcat2_dev_num[get_slot_num()-1],
                                &testStatus,
                                &addr,
                                &readVal,
                                &writeVal);

    if (rc == GT_OK) {
        if (testStatus == GT_FALSE) {
            cterr('f',0,"xCat2 switch all memory test failed !!!\n"
		  "Memory addr 0x%08x, readVal 0x%08x, writeVal 0x%08x\n",
		  addr, readVal, writeVal);
            rc = FAILED;
        } else {
            printf("xCat2 all memory test passed !!!\n");
        }
    } else {
        cterr('f',0,"Error returned by cpssDxChDiagAllMemTest(), rc 0x%x\n", rc);
    }

    if (xcat2_reinit() != GT_OK) {
	cterr('f',0,"Failed to reinitialize xCat2 device after the internal memory test.");
	rc = FAILED;
    }

    return (rc);
}
#endif

/******************************************************************************
 *
 * Function   :	port_force_link_Set
 * Description:	set force link status for a specific port
 *              
 * Inputs     :	link: LINK_UP or LINK_DOWN
 *              port:
 *              set: TRUE or FALSE
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
port_force_link_set (port_link_status_t link, int port, boolean set)
{
    uint32_t dev_num = xcat2_dev_num[get_slot_num()-1];
    int rc;

    if (link == LINK_DOWN) {
	rc = cpssDxChPortForceLinkDownEnableSet(dev_num, port, set);
	if(rc != GT_OK) {
	    printf("Failed cpssDxChPortForceLinkDownEnableSet call, rc = %x", rc);
	    return (rc);
	}
    } else {
	rc = cpssDxChPortForceLinkPassEnableSet(dev_num, port, set);
	if(rc != GT_OK) {
	    printf("Failed cpssDxChPortForceLinkDownEnableSet call, rc = %x", rc);
	    return (rc);
	}
    }	
    return (rc);
}



/******************************************************************************
 *
 * Function   :	xcat2_port_mac_lpbk_test
 * Description:	perform GE loopback test to verify GE0 backplane connectivity
 *              between the host and Dreamliner NIM.
 *              host->GE->MAC in XCAT2->GE->host
 * Inputs     :	port_num
 *              bp_port - backplane GE port, can GE0_XCAT2_PORT or GE1_XCAT2_PORT
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
xcat2_port_mac_lpbk_test (int port_num, int bp_port)
{
#ifdef TACHI_INTEL
#else 
    int ctrl_plane_sgmii_port;
    int num_pkt = DREAMLINER_GE_BP_PACKET_NO;
#endif 
    int rc = PASSED;
    uint32_t dev_num = xcat2_dev_num[get_slot_num()-1];

    if (xcat2_config_port_pve(dev_num, bp_port, port_num)) {
	printf("Failed to configure PVE for port %d, bp_port = %d", 
	      port_num, bp_port);
	return (FAILED);
    }

    if (xcat2_port_mac_loopback_enable(dev_num, port_num)) {
	printf("Failed to enable MAC loopback for port %d, bp_port = %d", 
	      port_num, bp_port);
	return (FAILED);
    }
    msleep(200);

#ifdef TACHI_INTEL
    printf("afix --%s: Using nc command to send packet from BMC \n",
            __FUNCTION__);
#else
    /* Do SGMII loopback test. */
    ctrl_plane_sgmii_port = get_sgmii_port_num(get_slot_num(), TYPE_SWITCH);

    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt)) {
	printf("Failed XCAT2 MAC loopback for port %d", port_num);
	rc = FAILED;
    }
#endif

    if (xcat2_unconfig_port_pve(dev_num, bp_port, port_num)) {
	printf("Failed to unconfigure PVE for port %d", port_num);
	rc = FAILED;
    }

    if (xcat2_port_mac_loopback_disable(dev_num, port_num)) {
	printf("Failed to disable MAC loopback for port %d", port_num);
	return (FAILED);
    }

    return (rc);
}


/******************************************************************************
 *
 * Function   :	xcat2_mac_lpbk_test_ge0
 * Description:	perform GE loopback test to verify GE0 backplane connectivity
 *              between the host and Dreamliner NIM.
 *              host->GE0->MAC in XCAT2->GE0->host
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
xcat2_mac_lpbk_test_ge0 (void)
{
    int port_num = get_port_num();
    int i;

    clear_sw_counter();

    cterr_add_component("Marvell xCat2 switch", 
			"backplane GE0 interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check backplane GE0 interface from the router");

    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, GT_TRUE) != GT_OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    for (i = 0; i < port_num; i++) {
        prpass(testpass, "XCAT2 port %d MAC loopback test through GE0, ", i + 1);

	if (xcat2_port_mac_lpbk_test(i, GE0_XCAT2_PORT)) {
	    print_sw_counter();
	    port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, GT_FALSE);
            cterr('f', 0, "Failed XCAT2 MAC loopback through GE0 for port %d", i + 1);
	    return (FAILED);
	}
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    print_sw_counter();
	}
    }

    if (port_force_link_set(LINK_DOWN, GE1_XCAT2_PORT, GT_FALSE) != GT_OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    return PASSED;
}


/******************************************************************************
 *
 * Function   :	xcat2_mac_lpbk_test_ge1
 * Description:	perform GE loopback test to verify GE0 backplane connectivity
 *              between the host and Dreamliner NIM.
 *              host->GE1->MAC in XCAT2->GE1->host
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
xcat2_mac_lpbk_test_ge1 (void)
{
    int port_num = get_port_num();
    int i;

    clear_sw_counter(); 
    cterr_add_component("Marvell xCat2 switch", 
			"backplane GE1 interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check backplane GE1 interface from the router");

    if (port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, GT_TRUE) != GT_OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    for (i = 0; i < port_num; i++) {
        prpass(testpass, "XCAT2 port %d MAC loopback test through GE1, ", i + 1);

	if (xcat2_port_mac_lpbk_test(i, GE1_XCAT2_PORT)) {
	    print_sw_counter();
	    port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, GT_FALSE);
            cterr('f', 0, "Failed XCAT2 MAC loopback through GE1 for port %d", i + 1);
	    return (FAILED);
	}
	if ((NVRAM)->diagflag & D_VERBOSE) {
	    print_sw_counter();
	}
    }

    if (port_force_link_set(LINK_DOWN, GE0_XCAT2_PORT, GT_FALSE) != GT_OK) {
        cterr('f',0,"Failed port_force_link_set()");
        return (FAILED);
    }

    return PASSED;
}


int
led_class_config (uint32_t led_class, boolean inv_ena, boolean blk_ena, int blk_sel, boolean force_ena, uint32_t force_data)
{
    CPSS_LED_CLASS_MANIPULATION_STC led_class_params;  
    int rc;

    led_class_params.invertEnable = inv_ena;
    led_class_params.blinkEnable  = blk_ena;
    led_class_params.blinkSelect  = blk_sel;
    led_class_params.forceEnable  = force_ena;
    led_class_params.forceData    = force_data;

    rc = cpssDxChLedStreamPortGroupClassManipulationSet(xcat2_dev_num[get_slot_num()-1],
				    	CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
					0,
					CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
					led_class, &led_class_params);
    return rc;
}
    

static int led_init ()
{
    int rc;

    CPSS_LED_CONF_STC led_stream_conf;  
    CPSS_LED_GROUP_CONF_STC led_group_params;  
    GT_U32 class0Group = 0; /* led stream group used for class0 */
    GT_U32 class1Group = 1; /* led stream group used for class1 */
    GT_U32 class3Group = 2; /* led stream group used for class3 */
    GT_U32 class4Group = 3; /* led stream group used for class4 */

    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    /* for port 0 through 11 and stack port, LED interface 0 is used */
    /*Led stream configuration */
    led_stream_conf.ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
    /* link status has no effect on other indications */
    led_stream_conf.disableOnLinkDown = GT_FALSE;
    led_stream_conf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
    led_stream_conf.blink0Duration  = CPSS_LED_BLINK_DURATION_6_E;
    led_stream_conf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
    led_stream_conf.blink1Duration  = CPSS_LED_BLINK_DURATION_3_E;
    led_stream_conf.pulseStretch  = CPSS_LED_PULSE_STRETCH_3_E;
    /* the first bit in the LED stream to be driven */
    led_stream_conf.ledStart  = 168;
    /* the last bit in the LED stream to be driven */
    if (get_port_num() == 4)
	led_stream_conf.ledEnd    = 183;
    else
	led_stream_conf.ledEnd    = 199;

    /* invert LEDclk pin */
    led_stream_conf.clkInvert = GT_TRUE;
    /* This config is for none dual-media ports */
    led_stream_conf.class5select  = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
    led_stream_conf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;

    rc = cpssDxChLedStreamPortGroupConfigSet(xcat2_dev_num[get_slot_num()-1],
					     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
					     0, &led_stream_conf);
    if (rc != GT_OK)
	return rc;

    /* group 0 configuration */
    led_group_params.classA = 0x3;
    led_group_params.classB = 15;
    led_group_params.classC = 15;
    led_group_params.classD = 15;

    rc = cpssDxChLedStreamPortGroupGroupConfigSet(xcat2_dev_num[get_slot_num()-1],
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  0,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class0Group,
                                                  &led_group_params);

    if (rc != GT_OK) {
        return rc;
    }

    /* group 1 configuration */
    led_group_params.classA = 0x4;
    led_group_params.classB = 15;
    led_group_params.classC = 15;
    led_group_params.classD = 15;

    rc = cpssDxChLedStreamPortGroupGroupConfigSet(xcat2_dev_num[get_slot_num()-1],
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  0,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class1Group,
                                                  &led_group_params);

    if (rc != GT_OK) {
        return rc;
    } 
   
    /* group 2 configuration */
    led_group_params.classA = 0x0;
    led_group_params.classB = 15;
    led_group_params.classC = 15;
    led_group_params.classD = 15;

    rc = cpssDxChLedStreamPortGroupGroupConfigSet(xcat2_dev_num[get_slot_num()-1],
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  0,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class3Group,
                                                  &led_group_params);

    if (rc != GT_OK) {
        return rc;
    }

    /* group 3 configuration */
    led_group_params.classA = 0x1;
    led_group_params.classB = 15;
    led_group_params.classC = 15;
    led_group_params.classD = 15;

    rc = cpssDxChLedStreamPortGroupGroupConfigSet(xcat2_dev_num[get_slot_num()-1],
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  0,
                                                  CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E,
                                                  class4Group,
                                                  &led_group_params);

    if (rc != GT_OK) {
        return rc;
    }

    /* config LED_CTRL(GPIO32) as output first */
    value = *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG);
    value &= ~(0x1 << (GPIO_LED_CTRL - 32));
    *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG) = value;

    /* Drive GPP11(GPIO32) to low to enable the LEDs */
    value = *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG);
    value &= ~(0x1 << (GPIO_LED_CTRL - 32));
    *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG) = value;

    /* turn off all the LEDs */
       if (led_class_config(0, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 0");
	return FAILED;
    }

    if (led_class_config(1, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 1");
	return FAILED;
    }

    if (led_class_config(3, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return FAILED;
    }

    if (led_class_config(4, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 4");
	return FAILED;
    }

    return rc;
}


/******************************************************************************
 *
 * Function   :	led_test
 * Description:	Dreamliner LED test
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */

int
led_test (void)
{
    GT_U32 force_data;

    cterr_setup();
    prpass(testpass, "Dreamliner LED test, ");
    cterr_add_component("Marvell xCat2 switch");
    cterr_add_debug("Check Marvell xCat2 switch");

    if (get_port_num() == 8)
	force_data = 0xff;
    else
	force_data = 0xf;

    /* turn all LEDs off */
    if (led_class_config(0, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 0");
	return FAILED;
    }

    if (led_class_config(1, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 1");
	return FAILED;
    }

    if (led_class_config(3, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return FAILED;
    }

    if (led_class_config(4, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 4");
	return FAILED;
    }

    msleep(200);

    /* turn Link LED on */
    if (led_class_config(3, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return FAILED;
    }

    if (led_class_config(4, GT_FALSE, GT_FALSE, 0, GT_TRUE, force_data)) {
	cterr('f',0,"Failed to configure LED class 4");
	return FAILED;
    }

    msleep(ONE_SECOND);

    /* ckeck POE normal, turn on GREEN LED */
    if (led_class_config(0, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 0");
	return FAILED;
    }

    if (led_class_config(1, GT_FALSE, GT_FALSE, 0, GT_TRUE, force_data)) {
	cterr('f',0,"Failed to configure LED class 1");
	return FAILED;
    }
    msleep(ONE_SECOND);

    /* ckeck POE error, turn on YELLOW LED */
    if (led_class_config(0, GT_FALSE, GT_FALSE, 0, GT_TRUE, force_data)) {
	cterr('f',0,"Failed to configure LED class 0");
	return FAILED;
    }

    if (led_class_config(1, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 1");
	return FAILED;
    }
    msleep(ONE_SECOND);

    /* check activity blink LED */
    if (led_class_config(3, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return FAILED;
    }

    if (led_class_config(4, GT_FALSE, GT_TRUE, 0, GT_TRUE, force_data)) {
	cterr('f',0,"Failed to configure LED class 4");
	return FAILED;
    }
    msleep(ONE_SECOND);

    /* turn all LEDs off */
    if (led_class_config(0, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 0");
	return FAILED;
    }

    if (led_class_config(1, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 1");
	return FAILED;
    }

    if (led_class_config(3, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 3");
	return FAILED;
    }

    if (led_class_config(4, GT_FALSE, GT_FALSE, 0, GT_TRUE, 0)) {
	cterr('f',0,"Failed to configure LED class 4");
	return FAILED;
    }

    msleep(200);

    return (GT_OK);
}

/******************************************************************************
 *
 * Function   :	led_utils
 * Description:	Dreamliner LED Utility
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */

int 
led_utils ()
{
    int port_num;
    int led_type;
    int led_action;
    GT_U32 data1, data2;

    cterr_add_component("Marvell xCat2 switch");
    cterr_add_debug("Check Marvell xCat2 switch");

    port_num = getdec_answer("Enter the port number:", 1, 1, 8);
    led_type = getdec_answer("Control POE LED (0) or Link LED (1):", 0, 0, 1);

    /* PonCat2 port maps to external port 
       0                    2
       1                    1
       2                    4
       3                    3
       4                    6
       5                    5
       6                    8
       7                    7
    */
    if (port_num % 2)
	port_num = port_num;
    else 
	port_num -= 2;

    data1 = 0;
    data2 = 0;

    if (led_type == 0) {
	led_action =  getdec_answer("Turn POE LED Green (0), Yellow (1) or Off (2)", 
				    0, 0, 2);
	if (led_action == 0)
	    data2 = 1 << port_num;
	else if (led_action == 1)
	    data1 = 1 << port_num;

	if (led_class_config(0, GT_FALSE, GT_FALSE, 0, GT_TRUE, data1)) {
	    cterr('f',0,"Failed to configure LED class 0");
	    return FAILED;
	}

	if (led_class_config(1, GT_FALSE, GT_FALSE, 0, GT_TRUE, data2)) {
	    cterr('f',0,"Failed to configure LED class 1");
	    return FAILED;
	}
    } else {
	led_action =  getdec_answer("Turn Link LED Green (0), Blink (1) or Off (2)", 
				    0, 0, 2);

	data2 = 1 << port_num;

	if (led_class_config(3, GT_FALSE, GT_FALSE, 0, GT_TRUE, data1)) {
	    cterr('f',0,"Failed to configure LED class 3");
	    return FAILED;
	}

	if (led_action == 1) {
	    if (led_class_config(4, GT_FALSE, GT_TRUE, 0, GT_TRUE, data2)) {
		cterr('f',0,"Failed to configure LED class 4");
		return FAILED;
	    } 
	} else {
	    if (led_action == 2)
		data2 = 0;

	    if (led_class_config(4, GT_FALSE, GT_FALSE, 0, GT_TRUE, data2)) {
		cterr('f',0,"Failed to configure LED class 4");
		return FAILED;
	    }
	}
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	sw_gpp_init
 * Description:	Init the GPIO within the Marvell GE switch.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
static int
sw_gpp_init ()
{
    uint32_t value;

    /* set GPP1, GPP2 and GPP6 to be input, GPP0, GPP3, GPP4, GPP9, GPP10 and GPP11 
       to be output pins. Set unused GPP7 to be output pin */
    value = FPGA_RESET | PHY_RESET | MODULE_READY | LED_CTRL | VOLTAGE_HIGH | 
            VOLTAGE_LOW | 0x80;

    return (xcat2_reg_pci_write(GPP_IO_CTRL_REG_OFFSET, value));
}
  
/******************************************************************************
 *
 * Function   :	fpga_in_reset
 * Description:	put FPGA in reset.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
fpga_in_reset ()
{
    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    value = *(unsigned int *)(config_base + GPIO_DATA_OUT_REG);
    value &= ~(1 << GPIO_FPGA_RESET);
    *(unsigned int *)(config_base + GPIO_DATA_OUT_REG) = value;
    return (PASSED);  
}


/******************************************************************************
 *
 * Function   :	fpga_out_of_reset
 * Description:	put FPGA in reset.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
fpga_out_of_reset ()
{
    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    value = *(unsigned int *)(config_base + GPIO_DATA_OUT_REG);
    value |= (1 << GPIO_FPGA_RESET);
    *(unsigned int *)(config_base + GPIO_DATA_OUT_REG) = value;
    return (PASSED);  
}


/******************************************************************************
 *
 * Function   :	phy_in_reset
 * Description:	put PHY in reset.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int 
phy_in_reset ()
{
    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    value = *(unsigned int *)(config_base + GPIO_DATA_OUT_REG);
    value &= ~(1 << GPIO_PHY_RESET);
    *(unsigned int *)(config_base + GPIO_DATA_OUT_REG) = value;
    return (PASSED);  
}


/******************************************************************************
 *
 * Function   :	phy_out_of_reset
 * Description:	put PHY in reset.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
phy_out_of_reset ()
{
    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    value = *(unsigned int *)(config_base + GPIO_DATA_OUT_REG);
    value |= (1 << GPIO_PHY_RESET);
    *(unsigned int *)(config_base + GPIO_DATA_OUT_REG) = value;
    return (PASSED);  
}


/******************************************************************************
 *
 * Function   :	set_module_ready
 * Description:	set module ready bit.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
set_module_ready ()
{
    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    value = *(unsigned int *)(config_base + GPIO_DATA_OUT_REG);
    value |= (1 << GPIO_MODULE_RDY);
    *(unsigned int *)(config_base + GPIO_DATA_OUT_REG) = value;
    return (PASSED);  
}

/******************************************************************************
 *
 * Function   :	is_daughter_card_present
 * Description:	check GPIO bit to see if daughter card is present.
 * Inputs     :	none
 * Outputs    : TRUE/FALSE
 *
 ******************************************************************************
 */
boolean
is_daughter_card_present ()
{
    uint32_t value;
    unsigned long config_base = dl_get_pci_base_addr();

    value = *(unsigned int *)(config_base + GPIO_DATA_IN_REG);
    if ((value & (1 << GPIO_DB_PRESENT)) == (1 << GPIO_DB_PRESENT))
	return (FALSE);
    else
	return (TRUE);
}


/******************************************************************************
 *
 * Function   :	smi0_phy_init
 * Description:	init SMI master0 for PHY access.
 *              SMI0 - SMI for PHYs connected to ports 0 through 11.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi0_phy_init ()
{
    int rc, i;
    int port_num = get_port_num();
    uint32_t sum_err = 0;

    /* to fix the PHY access issue after CPSS re-init. */
    xcat2_reg_pci_write(LMS0_MISC_CONFIG_REG_OFF, 0x70000);

    for (i = 0; i < port_num; i++) {
	/* set PHY SMI address */
	rc = cpssDxChPhyPortAddrSet(xcat2_dev_num[get_slot_num()-1], i, PHY_ADDR+i);
	if (rc == PASSED) {
	    /* set SMI interface 0 for PHY ports */
	    rc = cpssDxChPhyPortSmiInterfaceSet(xcat2_dev_num[get_slot_num()-1], i, 
						CPSS_PHY_SMI_INTERFACE_0_E);
	}
	if (rc != PASSED) {
	    sum_err++;
	}
    }
    if (sum_err == 0) {
	rc = cpssDxChPhyPortSmiInit(xcat2_dev_num[get_slot_num()-1]);
	if (rc != PASSED) 
	    return (GT_FAIL);
    } else {
	return (GT_FAIL);  
    }

    return PASSED;
}

/******************************************************************************
 *
 * Function   :	smi0_read_reg
 * Description:	read from device register through SMI0 interface.
 * Inputs     :	port_num - phy port number
 *              reg_addr - device register address
 *              rd_data - point to unsigned short which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi0_read_reg (unsigned int port_num, 
	       unsigned int reg_addr, unsigned short *reg_data)
{
    return (cpssDxChPhyPortSmiRegisterRead(xcat2_dev_num[get_slot_num()-1], port_num,
					   reg_addr, reg_data));
}


/******************************************************************************
 *
 * Function   :	smi0_write_reg
 * Description:	write to device register through SMI0 interface.
 * Inputs     :	port_num - phy port number
 *              reg_addr - device register address
 *              wr_data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi0_write_reg (unsigned int port_num, 
	       unsigned int reg_addr, unsigned short wr_data)
{
    return (cpssDxChPhyPortSmiRegisterWrite(xcat2_dev_num[get_slot_num()-1], port_num,
					    reg_addr, wr_data));
}

/******************************************************************************
 *
 * Function   :	peek_phy_reg
 * Description:	Utility to peek PHY registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
peek_phy_reg (void)
{
    int port_num;
    uint16_t reg_num;
    uint16_t data;
    int ret;

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, get_port_num()-1);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);

    ret = smi0_read_reg(port_num, reg_num, &data);
    if (ret == PASSED)
	printf("PHY register value = %#x\n", data);

    return (ret);
}

/******************************************************************************
 *
 * Function   :	poke_phy_reg
 * Description:	Utility to poke PHY registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
poke_phy_reg (void)
{
    int port_num;
    uint16_t reg_num;
    uint16_t data;

    port_num = getdec_answer("Enter PHY port number: ", 0, 0, get_port_num()-1);
    reg_num = getdec_answer("Enter PHY register number: ", 0, 0, 31);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);

    return (smi0_write_reg(port_num, reg_num, data));
}

/******************************************************************************
 *
 * Function   :	smi1_init
 * Description:	init SMI master1 for FPGA access.
 *              SMI1 - connect to FPGA on daughtercard.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi1_init ()
{
    uint32_t dev_num = xcat2_dev_num[get_slot_num()-1];
    GT_STATUS rc;
    GT_U32 data;

    /* enable SMI1 fast mdc clock mode */
    rc = cpssDrvPpHwRegisterRead(dev_num, 0, PHY_ADDR_REG2_OFF, &data);
    if (rc != GT_OK) {
	return (GT_FAIL);
    }
    data |= (1 << 30);
    rc = cpssDrvPpHwRegisterWrite(dev_num, 0, PHY_ADDR_REG2_OFF, data);
    if (rc != GT_OK) {
	return (GT_FAIL);
    }  

    /* set SMI1 mdc clock to 1/16 core clock */
    rc = cpssDrvPpHwRegisterRead(dev_num, 0, LMS1_MISC_CONFIG_REG, &data);
    if (rc != GT_OK) {
	return (GT_FAIL);
    }
    data &= ~(0x3 << 15);
    rc = cpssDrvPpHwRegisterWrite(dev_num, 0, LMS1_MISC_CONFIG_REG, data);
    if (rc != GT_OK) {
	return (GT_FAIL);
    }

    return (GT_OK);
}


/******************************************************************************
 *
 * Function   :	smi1_read_reg
 * Description:	read from device register through SMI1 interface.
 * Inputs     :	
 *              reg_addr - device register address
 *              rd_data - point to unsigned short which holds regiter value
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi1_read_reg (unsigned int reg_addr, unsigned short *reg_data)
{
    return (cpssSmiRegisterReadShort(xcat2_dev_num[get_slot_num()-1], 
				     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
				     CPSS_PHY_SMI_INTERFACE_1_E,
				     FPGA_ADDR, reg_addr, reg_data));
}


/******************************************************************************
 *
 * Function   :	smi1_write_reg
 * Description:	write to device register through SMI1 interface.
 * Inputs     :	
 *              reg_addr - device register address
 *              wr_data - write data
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int
smi1_write_reg (unsigned int reg_addr, unsigned short wr_data)
{
    return (cpssSmiRegisterWriteShort(xcat2_dev_num[get_slot_num()-1], 
				     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
				     CPSS_PHY_SMI_INTERFACE_1_E,
				     FPGA_ADDR, reg_addr, wr_data));
}

/******************************************************************************
 *
 * Function   :	peek_fpga_reg
 * Description:	Utility to peek FPGA registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
peek_fpga_reg (void)
{
    uint16_t reg_num;
    uint16_t data;
    int ret;

    reg_num = gethex_answer("Enter FPGA register number: ", 0, 0, 0x1f);

    ret = smi1_read_reg(reg_num, &data);
    if (ret == PASSED)
	printf("FPGA register value = %#x\n", data);

    return (ret);
}

/******************************************************************************
 *
 * Function   :	poke_fpga_reg
 * Description:	Utility to poke FPGA registers.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
poke_fpga_reg (void)
{
    uint16_t reg_num;
    uint16_t data;

    reg_num = gethex_answer("Enter FPGA register number: ", 0, 0, 0x1f);
    data = gethex_answer("Enter write data: ", 0, 0, 0xffff);

    return (smi1_write_reg(reg_num, data));
}

/******************************************************************************
 *
 * Function   :	sw_init
 * Description:	init Marvell GE switch.
 * Inputs     :	None 
 * Outputs    : None
 *
 ******************************************************************************
 */
int
sw_init ()
{
    cterr_add_component("Marvell xCat2 switch", 
			"PCIe interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check PCIe interface from the router");

    if (xcat2_init() != PASSED) {
	cterr('f',0,"Failed XCAT2 CPSS init.");
	return (FAILED);
    }

    /* init GPIO within GE switch */
    if (sw_gpp_init() != PASSED) {
	cterr('f',0,"Failed XCAT2 gpp init.");
	return (FAILED);
    }

    /* init LED */
    if (led_init() != PASSED) {
	cterr('f',0,"Failed LED init.");
	return (FAILED);
    }

    /* init SMI master 0 and 1 */
    if (smi0_phy_init() != PASSED) {
	cterr('f',0,"Failed XCAT2 SMI0 init.");
	return (FAILED);
    }

    if (smi1_init() != PASSED) {
	cterr('f',0,"Failed XCAT2 SMI1 init.");
	return (FAILED);
    }

    /* take PHY out of reset */
    if (phy_out_of_reset() != PASSED) {
	cterr('f',0,"Failed to take PHY out of reset.");
	return (FAILED);
    }
#ifdef ian
    int value = gethex_answer("Enter Pass (1)/Failed (0): ", 0, 0, 0xffff);
    if (value ==1 ) {
        return (PASSED);  
    }else{
        return (FAILED);  
    }
#endif
    if (is_daughter_card_present() == TRUE) {
        /* take FPGA out of reset */
        if (fpga_out_of_reset() != PASSED) {
            cterr('f',0,"Failed to take FPGA out of reset.");
            return (FAILED);
        }
        sleep(1);

        /* Init POE controller when test PoE related item. */
        /*
        if (poe_init() == FAILED) {
            cterr('f',0,"Failed to init POE controller.");	    
            return (FAILED);
        }	
        */
    }
	
    return (PASSED);
}


/******************************************************************************
 *
 * Function   :	xcat2_reset
 * Description:	reset Marvell GE switch.
 * Inputs     :	None 
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************
 */
int xcat2_reset ()
{
    int rc = GT_OK;

    cterr_add_component("Marvell xCat2 switch", 
			"PCIe interface from the router");
    cterr_add_debug("Check Marvell xCat2 switch",
		    "Check PCIe interface from the router");

    rc = xcat2_soft_reset(xcat2_dev_num[get_slot_num()-1]);
    if (rc) {
        cterr('f',0,"Failed XCAT2 soft reset \n");
        return (GT_FAIL);
    } else {
        printf("XCAT2 soft reset Done !!! \n");
    }

    printf("Calling sw_init()\n");

    rc = sw_init();
    if (rc) {
        cterr('f',0,"Failed Xcat2 Switch Device Init \n");
        return (GT_FAIL);
    } else {
        printf("XCAT2 Switch Device Init Done !!! \n");
    }

    return (rc);
}


GT_UINTPTR
dl_get_bus_base_addr ()
{
    GT_UINTPTR ppregs_base, config_base;
    
    nim_dm_cpss_get_pciemap(&ppregs_base, &config_base);
#ifdef DEBUG
    printf("ppregs_base = %lx, config_base = %lx\n", ppregs_base, config_base);
    printf("ppregs_base = %#lx,  vendor_id = %#x\n", ppregs_base,
	   *(unsigned int *)(config_base + 0x50));
#endif
    return ppregs_base;
}

unsigned long 
dl_get_pci_base_addr ()
{
    GT_UINTPTR ppregs_base, config_base;
    
    nim_dm_cpss_get_pciemap(&ppregs_base, &config_base);
#ifdef DEBUG
    printf("ppregs_base = %lx, config_base = %lx\n", ppregs_base, config_base);
    printf("config_base = %#lx, vendor_id = %#x\n", config_base, 
	   *(unsigned int *)(config_base + 0x40000));
#endif
    return config_base;
}


void 
print_sw_counter ()
{
    GT_U32 data1, data2;
    uint32_t dev_num = xcat2_dev_num[get_slot_num()-1];
    int port_num = get_port_num();
    int i;

    cpssDrvPpHwRegisterRead(dev_num, 0, 0x9300018, &data1);
    cpssDrvPpHwRegisterRead(dev_num, 0, 0x930004c, &data2);
    printf("\ncounter for GE port 24: RX = 0x%x, TX = 0x%x\n", data1, data2);
    cpssDrvPpHwRegisterRead(dev_num, 0, 0x9320018, &data1);
    cpssDrvPpHwRegisterRead(dev_num, 0, 0x932004c, &data2);
    printf("counter for GE port 25: RX = 0x%x, TX = 0x%x\n", data1, data2);

    for (i = 0; i < port_num; i++) {
	if (i < 6) {
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x4010018+i*0x80, &data1);
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x401004c+i*0x80, &data2);
	} else {
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x4810018+(i-6)*0x80, &data1);
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x481004c+(i-6)*0x80, &data2);
	}   
	printf("counter for GE port %d: RX = 0x%x, TX = 0x%x\n", i + 1, data1, data2);
    }
}


void 
clear_sw_counter ()
{
    GT_U32 data1, data2;
    uint32_t dev_num = xcat2_dev_num[get_slot_num()-1];
    int port_num = get_port_num();
    int i;

    cpssDrvPpHwRegisterRead(dev_num, 0, 0x9300018, &data1);
    cpssDrvPpHwRegisterRead(dev_num, 0, 0x930004c, &data2);
    cpssDrvPpHwRegisterRead(dev_num, 0, 0x9320018, &data1);
    cpssDrvPpHwRegisterRead(dev_num, 0, 0x932004c, &data2);

    for (i = 0; i < port_num; i++) {
	if (i < 6) {
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x4010018+i*0x80, &data1);
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x401004c+i*0x80, &data2);
	} else {
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x4810018+(i-6)*0x80, &data1);
	    cpssDrvPpHwRegisterRead(dev_num, 0, 0x481004c+(i-6)*0x80, &data2);
	}   
    }
}
void
build_volt_margin_menu(int dummy)
{

    testname("Voltage Margin");

    build_primary_submenu(mrgn_items, MARGIN_MENU_TABLE_SIZE,
			  "Voltage Margin Utility Menu", &margin_util_menup);
    build_secondary_submenu(mrgn_items,  MARGIN_MENU_TABLE_SIZE,
			    mrgn_menu_secondary_items);
    menu(&margindiag, mrgn_menu_secondary_items, 0);
}


/*******************************************************************************
 *
 * Function   :	dl_volt_margin
 * Description:	Wrapper utility to set Voltage margin 
 * Inputs     :	None
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int dl_volt_margin(int option)
{
    uint32_t rc = PASSED;
#ifdef USE_GPIO
    int status;
    uint32_t reg_val;
#endif
    uint32_t value_low, value_hi;
    unsigned long config_base = dl_get_pci_base_addr();
    /* config VOLTAGE_LOW(GPIO33) as output first */
    value_low = *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG);
    value_low &= ~(0x1 << (GPIO_VOLTAGE_LOW - 32));
    *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG) = value_low;

    /* config VOLTAGE_High(GPIO23) as output first */
    value_hi = *(unsigned int *)(config_base + GPIO_DATA_OUT_ENA_REG);
    value_hi &= ~(1 << GPIO_VOLTAGE_HIGH);
    *(unsigned int *)(config_base + GPIO_DATA_OUT_ENA_REG) = value_hi;


    value_hi = *(unsigned int *)(config_base + GPIO_DATA_OUT_REG);
    value_low = *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG);

    switch(option) {
    case VTG_MRGN_SET_3_3V_NORM:
        value_hi &= ~(1 << GPIO_VOLTAGE_HIGH);
        value_low &= ~(0x1 << (GPIO_VOLTAGE_LOW - 32));
        printf("\nVoltage Margin 3.3V Normal");
	    break;
    case VTG_MRGN_SET_3_3V_HI:
        value_hi |= (1 << GPIO_VOLTAGE_HIGH);
        value_low &= ~(0x1 << (GPIO_VOLTAGE_LOW - 32));
        printf("\nVoltage Margin 3.3V High");
	    break;
    case VTG_MRGN_SET_3_3V_LO:
        value_hi &= ~(1 << GPIO_VOLTAGE_HIGH);
        value_low |= 0x1 << (GPIO_VOLTAGE_LOW - 32);
        printf("\nVoltage Margin 3.3V Low");
	    break;
    default:
	    cterr('f', 0, "vtg_mrgn() Invalid option %#x", option);
	    rc = FAILED;
	    break;
    } /* endof switch */

    *(unsigned int *)(config_base + GPIO_DATA_OUT_REG) = value_hi;
    *(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG) = value_low;

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nvalue_hi (bit 23)GPIO_DATA_OUT_REG = 0x%x ",*(unsigned int *)(config_base + GPIO_DATA_OUT_REG));
        printf("\nvalue_low(bit 1) GPIO_HIGH_DATA_OUT_REG = 0x%x ",*(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_REG));
        printf("\nGPIO_DATA_OUT_ENA_REG = 0x%x",*(unsigned int *)(config_base + GPIO_DATA_OUT_ENA_REG));
        printf("\nGPIO_HIGH_DATA_OUT_ENA_REG = 0x%x \n",*(unsigned int *)(config_base + GPIO_HIGH_DATA_OUT_ENA_REG));
    }
#ifdef USE_GPIO

    /*
     * GPP10: Margin Low (3.3V) ﾡV By default, logic 0.  To margin Low, SW to drive GPP9 to 0 and GPP10 to 1
     * GPP9:  Margin High(3.3V) ﾡV By default, logic 0.  To margin High,SW to drive GPP9 to 1 and GPP10 to 0
     */

    status = xcat2_reg_pci_read(GPP_IO_CTRL_REG_OUTPUT, &reg_val);
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        printf("\nBefor : GPP_IO_CTRL_REG_OUTPUT(0x%x) = 0x%x \n",GPP_IO_CTRL_REG_OUTPUT, reg_val);
    }

    switch(option) {
    case VTG_MRGN_SET_3_3V_NORM:
        reg_val &= ~(VOLTAGE_HIGH);
        reg_val &= ~(VOLTAGE_LOW);
        printf("\nVoltage Margin 3.3V Normal");
	    break;
    case VTG_MRGN_SET_3_3V_HI:
        reg_val |= (VOLTAGE_HIGH);
        reg_val &= ~(VOLTAGE_LOW);
        printf("\nVoltage Margin 3.3V High");
	    break;
    case VTG_MRGN_SET_3_3V_LO:
        reg_val &= ~(VOLTAGE_HIGH);
        reg_val |= (VOLTAGE_LOW);
        printf("\nVoltage Margin 3.3V Low");
	    break;
    default:
	    cterr('f', 0, "vtg_mrgn() Invalid option %#x", option);
	    rc = FAILED;
	    break;
    } /* endof switch */

    status = xcat2_reg_pci_write(GPP_IO_CTRL_REG_OUTPUT, reg_val);

    if (diagflag_xram & D_DEBUG_OPTIONS) {
        status = xcat2_reg_pci_read(GPP_IO_CTRL_REG_OUTPUT, &reg_val);
        printf("\nAfter : GPP_IO_CTRL_REG_OUTPUT(0x%x) = 0x%x \n",GPP_IO_CTRL_REG_OUTPUT, reg_val);
    }
#endif
    return (rc);  
}

/*
 *------------------------------------------------------------------
 * $Log: dreamliner_ge_switch.c,v $
 * Revision 1.7  2018/08/06 02:57:32  harrchan
 * Changing daily build server from sjc-ads-1686 to sjc-ads-9106 (CSCvk60118)
 *
 * Revision 1.6  2017/07/14 02:51:38  alpeng
 * fixed compiler warning, due to cross-compiler version was updated.
 *
 * Revision 1.5  2017/03/30 08:23:23  hondwang
 * Tachi-L brach merge
 *
 * Revision 1.4.2.1  2016/12/21 12:46:13  hondwang
 * Fix dreamliner loopback issue
 *
 * Revision 1.4  2016/10/16 12:28:15  iachang
 * Supported Goldbeach Platform.
 *
 * Revision 1.3  2016/04/20 07:06:40  benchen2
 * merge tachi branch into main trunk
 *
 * Revision 1.2.4.4  2016/05/09 01:10:01  alpeng
 * remove useless definition based on prrq suggestion
 *
 * Revision 1.2.4.3  2015/12/29 12:27:07  alpeng
 *  update dreamlienr utilities for cross functional team to setup vlan
 *
 * Revision 1.2.4.2  2015/10/05 10:21:39  alpeng
 * support single test, update loopback test
 *
 * Revision 1.2.4.1  2015/08/17 02:33:03  alpeng
 * first check in for tachi-intel test; fix smart_cookie.c and free.h
 *
 * Revision 1.2  2015/02/27 10:02:20  iachang
 *
 * Add support dreamliner NIM
 *
 * Revision 1.1.6.2  2015/02/14 07:13:53  iachang
 * Dreamliner Diag sync with main trunk.
 *
 * Revision 1.1.4.4  2015/02/09 14:40:15  iachang
 * Port number start from 1
 * 
 * Revision 1.1.4.3  2015/02/06 10:34:31  iachang
 * Moved PoE init function from board init avoid user can't into module menu.
 * 
 * Revision 1.1.4.2  2015/01/28 22:59:21  iachang
 * Dreamliner-branch2 initial check-in.
 * 
 * Revision 1.1.2.4  2015/01/22 02:18:50  iachang
 * Fixed voltage margin
 * 
 * Revision 1.1.2.3  2014/12/19 07:02:44  iachang
 * Supported voltage margin
 * 
 * Revision 1.1.2.2  2014/12/04 12:17:58  iachang
 * Fixed 4 port SKU PCI Register Test failed.
 * 
 * Revision 1.1.2.1  2014/12/02 08:04:10  iachang
 * Dreamliner Diag initial check-in.
 *------------------------------------------------------------------
 * $Endlog$
 */


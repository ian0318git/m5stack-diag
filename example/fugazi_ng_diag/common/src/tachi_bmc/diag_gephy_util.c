/* $Id: diag_gephy_util.c,v 1.3 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_gephy_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_util.c - GE PHY 88E1512 Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "diag_gephy_util.h"
#include "diag_smi_lib.h"
#include "patriot_linux/apps/common_utils.h"

int diag_gephy_util(void);

static int diag_gephy_reg_alter(void);
static int diag_gephy_reg_display(void);
static int mvl_ge_set_test_mode(void);
static int diag_gephy_dump_all_reg(void);


/* Sub Menu used for MCU utility.
 */
static submenu_xtable_t gephy_util_submenu_table[] = {
    {"Register Alter Utility", (type_t(*)())diag_gephy_reg_alter,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Register Display Utility", (type_t(*)())diag_gephy_reg_display,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Test Mode", (type_t(*)())mvl_ge_set_test_mode,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Dump All Register Utility", (type_t(*)())diag_gephy_dump_all_reg,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GEPHY_UTIL_SUBMENU_TABLE_SIZE (sizeof(gephy_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gephy_util_primary_items[GEPHY_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t gephy_util_secondary_items[GEPHY_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t gephy_util_subtest_menu = {
    "%s Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    gephy_util_primary_items,
};
menuinfo_t *gephy_util_submenup = &gephy_util_subtest_menu;

int diag_gephy_util (void)
{
    build_primary_submenu(gephy_util_submenu_table,
			              GEPHY_UTIL_SUBMENU_TABLE_SIZE,
                          "GE PHY", &gephy_util_submenup);
    build_secondary_submenu(gephy_util_submenu_table,
                            GEPHY_UTIL_SUBMENU_TABLE_SIZE,
                            gephy_util_secondary_items);    
                            
    menu(gephy_util_submenup, gephy_util_secondary_items, '\0');
    return (PASSED);
}

static int diag_gephy_reg_alter (void)
{
    int reg_addr, page_num;
    ulong reg_val;

    printf("\n\nGEPHY Register Alter\n\n");
    page_num = getdec_answer("Page offset to write", 0, 0, 250);

    /*Access Register 22 to determine which PAGE that we want to access*/
   	 if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
          MRV88E1512_PAGE_ADDRESS_REG, page_num) == FAILED) {
   	     printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
   	     return (FAILED);
   	 }

   	reg_addr = getdec_answer("Reg offset to write", 0, 0, 28);
   	reg_val    = gethex_answer("Enter new value:", 0, 0, 0xFFFFFFFF);

    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, reg_addr, reg_val)
         == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
		return (FAILED);
	}

    /* switch page 0 */
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

	return (PASSED);
}

static int diag_gephy_reg_display (void)
{
	int reg_addr, page_num, reg_data;

	printf("\n\nGEPHY Register Read\n\n");
	page_num = getdec_answer("Page offset to write", 0, 0, 250);
	reg_addr = getdec_answer("Reg offset to write", 0, 0, 28);
    
    reg_data = diag_gephy_reg_value_get(page_num, reg_addr);                    
    printf("reg_val is 0X%x\n", reg_data);
    
    return (PASSED);
}

int diag_gephy_reg_value_get(int page_num, int reg_addr)
{
    ulong reg_val;
    int ret_val;

    /*Access Register 22 to determine which PAGE that we want to access*/
   	if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, 
         MRV88E1512_PAGE_ADDRESS_REG, page_num) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
   	    return (FAILED);
   	}

    /*Access Register to read value*/
    if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID, reg_addr, &reg_val) 
         == FAILED) {
        printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
        return (FAILED);
    }
    
    /* switch page 0 */
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, 
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
       printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
       return (FAILED);
   }

    ret_val = (int)reg_val;
    return ret_val;

}

static int diag_gephy_dump_all_reg(void) 
{
    int PageNum,RegNum,reg_data, Register;
    ulong reg_val;

    for (PageNum = 0; PageNum <= 18; PageNum++) {
        /*Access Register 22 to determine which PAGE that we want to access*/
        if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
            MRV88E1512_PAGE_ADDRESS_REG, PageNum) == FAILED) {
            printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
            return (FAILED);
        }
        for (RegNum = 0; RegNum <= 28; RegNum++) {
                if(RegNum != 22){
                        switch (PageNum) {
                        case 0:
                            if (RegNum != 11 && RegNum != 12 && RegNum != 24
                               && RegNum != 25 && RegNum != 27 && RegNum != 28)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 1:
                            if (RegNum != 9 && RegNum != 10 && RegNum != 11 
                               && RegNum != 12 && RegNum != 13 
                               && RegNum != 14 && RegNum != 20
                               && RegNum != 22 && RegNum != 27 && RegNum != 28)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 2:
                            if (RegNum == 16 || RegNum == 18 || RegNum == 19 
                               || RegNum == 20 || RegNum == 21 || RegNum == 24
                               || RegNum == 25 )
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 3:
                            if (RegNum == 16 || RegNum == 18 || RegNum == 19 
                               || RegNum == 17)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 4:
                                   if (RegNum == 20)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 5:
                            if (RegNum == 16 || RegNum == 17 || RegNum == 18 
                                || RegNum == 19 ||RegNum == 20 || RegNum == 21
                                || RegNum == 23 || RegNum == 24 || RegNum == 25
                                || RegNum == 26 || RegNum == 27 || RegNum == 28)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 6:
                            if (RegNum == 16 || RegNum == 17 || RegNum == 18 
                                || RegNum == 19 ||RegNum == 20 || RegNum == 23
                                || RegNum == 24 || RegNum == 25 || RegNum == 26
                                || RegNum == 27)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 7:
                            if (RegNum == 16 || RegNum == 17 ||RegNum == 18 
                                || RegNum == 19 ||RegNum == 20 || RegNum == 21
                                || RegNum == 25 || RegNum == 26 || RegNum == 27
                                || RegNum == 28 )
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 8:
                            if (RegNum == 0 || RegNum == 1 ||RegNum == 2 
                                || RegNum == 3 ||RegNum == 8 || RegNum == 9 
                                || RegNum == 10 || RegNum == 11 || RegNum == 12
                                || RegNum == 13 ||RegNum == 14 || RegNum == 15)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 9:
                            if (RegNum == 0 || RegNum == 1 ||RegNum == 2
                               || RegNum == 3 ||RegNum == 5)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 12:
                                   if (RegNum <=15)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 14:
                            if (RegNum == 0 || RegNum == 1 ||RegNum == 2 
                                || RegNum == 3 ||RegNum == 8 
                                || RegNum == 14 || RegNum == 15)
                                           Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 17:
                            if (RegNum == 16 || RegNum == 17 ||RegNum == 18 
                                || RegNum == 19 ||RegNum == 20 || RegNum == 21
                                || RegNum == 23 || RegNum == 24 || RegNum == 25
                                || RegNum == 26 || RegNum == 27 || RegNum == 28)
                                       Register=RegNum;
                                   else
                                           Register=999;
                                   break;
                        case 18:
                            if (RegNum == 0 || RegNum == 1 || RegNum == 2
                                || RegNum == 16 ||RegNum == 17 || RegNum == 18
                                || RegNum == 19 || RegNum == 20 || RegNum == 25
                                || RegNum == 26)
                                       Register=RegNum;
                                   else
                                       Register=999;
                                   break;
                        default:
                                       Register=999;
                        }
                if (Register != 999) {
                    /*Access Register to read value*/
                    if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID, Register,
                        &reg_val) == FAILED) {
                        printf("Read Register 0X%x FAIL\n",
                        MRV88E1512M_SPECIFIC_CONTROL2_REG);
                        return (FAILED);
                    }
                    reg_data = (int)reg_val;
                    printf(" Page  %d Reg %d reg_val is 0X%x\n", PageNum,
                            RegNum, reg_data);
                }
             }
        }
    }
    return PASSED;
}

/**********************************************************************
 *
 * Function:    mvl_ge_set_test_mode
 *
 * This function: provides PHY test mode for Marvell GE PHYs.
 *
 * Input:   dev - Pointer to the Marvell GE device object
 *
 * Output:  PASSED/FAILED
 *
 **********************************************************************
 */
static int
mvl_ge_set_test_mode(void)
{
    ulong reg_val;
    int test_mode;

    /*PAGE 0*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

    /*read value of P0R9 */
    if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID, 
        MRV88E1512C_1000B_CNTL_REG, &reg_val) == FAILED) {
        printf("Read Register 0X%x FAIL\n", MRV88E1512C_1000B_CNTL_REG);
        return (FAILED);
    }

    test_mode = (reg_val & PHY_GT_CTL_TEST_MASK) >> (PHY_GT_CTL_TEST_SHIFT);

    printf("Current Test modes  %d-\n", test_mode);

    printf("\nTest modes -\n");
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");

    test_mode = gethex_answer("Enter the test mode: ", test_mode, 0, 4);

    /* Write the new data */
    reg_val &= (~PHY_GT_CTL_TEST_MASK); /* clear the test mode */
    reg_val |= (test_mode << PHY_GT_CTL_TEST_SHIFT);

    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
            MRV88E1512C_1000B_CNTL_REG, reg_val) == FAILED) {
        printf("Set register 0x%x FAIL\n", MRV88E1512C_1000B_CNTL_REG);
        return (FAILED);
    }

    /* Recover the page */
    /*Back to P0*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

    printf("Please do the hardware reset after doing text mode\n");
    return(PASSED);
}


/*---------------------------------------------------------------
$Log: diag_gephy_util.c,v $
Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2016/11/04 19:08:55  benchen2
Modify Enhanced error message

Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.5  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.4  2015/12/04 11:59:17  benchen2
fix gephy textmode

Revision 1.1.2.3  2015/09/14 07:30:43  benchen2
phy 1512 utility

Revision 1.1.2.2  2015/07/31 07:32:23  hondwang
gephy util

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/


/* $Id: nim_dreamliner.c,v 1.2 2016/04/20 08:53:59 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/intel/nim_dreamliner.c,v $
 *------------------------------------------------------------------
 * Filename:   nim_dreamliner.c 
 *
 * Description: intel nim dreamliner diag entry 
 *
 * Copyright (c) 2015-2016 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nim_test_defs.h"
#include "cross_platform.h"
#include "types.h"
#include "menu.h"
#include "common.h"

extern boolean is_daughter_card_present();
extern int xcat2_utils(void);

/* Extern tests */
extern int xcat2_pci_reg_test(void); 
extern int xcat2_all_reg_test(void); 
extern int xcat2_temperature_test(void);
extern int phy_reg_test(void);
extern int phy_intr_test(void);
extern int led_test(void);
extern int poe_intr_test(void);
extern int poe_reg_test(void);
extern int fpga_reg_test(void);
extern int spi_flash_test(void);

extern int nim_slot; 
extern int nim_types; 
extern int get_ngio_pcie_dev_bus_num(uint mod_type, uint slot);
extern unsigned int pci_config_read(unsigned int bus, unsigned int device,
                 unsigned int fn, int offset);
extern unsigned int pci_config_write(unsigned int bus, unsigned int device,
                  unsigned int fn, int offset, unsigned int value);
extern int getdec_answer(char *msgstr, uint currentval, uint min,
                       uint max);
extern int getc_answer(char *msg, char *cmpstr, char curval);
                     

extern submenu_xtable_t xcat2_util_submenu_table[]; 
extern submenu_xtable_t fpga_util_submenu_table[];
extern submenu_xtable_t phy_util_submenu_table[];
extern submenu_xtable_t poe_util_submenu_table[]; 

struct diag_menu_def nim_dl_utils_menu[] = {
    {"xcat2 utils", xcat2_util_submenu_table},  
    {"FPGA utils", fpga_util_submenu_table}, 
    {"PHY utils", phy_util_submenu_table},
    {"PoE utils", poe_util_submenu_table},
};

static struct diag_test_list nim_dl_test_list[] = {
    {"xCat2 PCI Register Test", (PFT)xcat2_pci_reg_test}, 
    {"xCat2 All Register Test", (PFT)xcat2_all_reg_test},
    {"xCat2 Temperature Test", (PFT)xcat2_temperature_test}, 
    {"Marvell PHY Register Test", (PFT)phy_reg_test},
    {"Marvell PHY interrupt Test", (PFT)phy_intr_test}, 
#if 0  /* move to BMC NIM test, using nc to trigger these tests */
    {"Marvell PHY internal loopback Test through GE0",
     (PFT)phy_internal_lpbk_test_ge0}, 
    {"Marvell PHY internal loopback Test through GE1",
     (PFT)phy_internal_lpbk_test_ge1}, 
    {"External loopback Test", (PFT)external_lpbk_test},
#endif 
    {"LED Test", (PFT)led_test},
};

static struct diag_test_list nim_dl_poe_test_list[] = {
    {"FPGA Register Test", (PFT)fpga_reg_test},
    {"POE Controller Register Test", (PFT)poe_reg_test},
    {"POE Controller interrupt Test", (PFT)poe_intr_test},
    {"SPI flash Test", (PFT)spi_flash_test},
};

ushort board_id = NIM_ES2_8P;  /* on cell 79 */

/*
 * Function: dl_pcie_test
 *
 * Description : extract dreamliner pcie tests for intel
 *
 * Inputs: slot - slot number, left for future reference.
 *
 * Output: fail - with bit expression for failure test.
 *
 */
int dl_pcie_test (int slot, int sel) {

    int cnt, size_def, size_poe, tot, rc = PASSED;

    size_def = sizeof(nim_dl_test_list)/sizeof(diag_test_list_t);
    size_poe = sizeof(nim_dl_poe_test_list)/sizeof(diag_test_list_t);
    tot = size_def + size_poe; 

    /* user select test item */
    if (sel) {
       if (sel > tot) {
           printf("Exceed maximum test items %d\n", tot);
           return (rc);
       } else if (sel > size_def) { 
           sel -= (size_def + 1);  /* start from 0 */
           for (cnt = 0; cnt < size_poe; cnt++) {
               if (cnt == sel) {
                   if (is_daughter_card_present()) {
                       rc = nim_dl_poe_test_list[cnt].xfunc();
                   } else {
                       /*
                       printf("PoE card is not detected. skipped test\n");
                       */
                   }
               }
           } 
       } else {
           sel -= 1;  /* start from 0 */
           for (cnt = 0; cnt < size_def; cnt++) {
               if (cnt == sel) {
                   rc = nim_dl_test_list[cnt].xfunc();
               }
           } 
       }
       return (rc);
    }

    for (cnt = 0; cnt < size_def; cnt++) {
        rc += nim_dl_test_list[cnt].xfunc();
    }
     
    if (is_daughter_card_present()) {
        for (cnt = 0; cnt < size_poe; cnt++) {
            rc += nim_dl_poe_test_list[cnt].xfunc();
        }
    }

    if (rc == PASSED) {
        return (PASSED);
    } else {
        return (rc);
    }
}

#if 0
int (*nim_dl_pcie_test[])(void) = {
    xcat2_pci_reg_test,
    xcat2_all_reg_test,
    xcat2_temperature_test,
    phy_reg_test,
    phy_internal_lpbk_test_ge0,
    phy_internal_lpbk_test_ge1,
    external_lpbk_test,
    phy_intr_test,
    led_test,
};

int (*nim_dl_poe_pcie_test[])(void) = {
    fpga_reg_test,
    poe_intr_test,
    poe_reg_test,
    spi_flash_test,
};


/*
 * Function: dl_pcie_test
 *
 * Description : extract dreamliner pcie tests for intel 
 *
 * Inputs: slot - slot number, left for future reference.
 *
 * Output: fail - with bit expression for failure test. 
 *              
 */
int dl_pcie_test (int slot) {

    int cnt, size, rc = PASSED; 
 
    cnt = 0;
    size = sizeof(nim_dl_pcie_test)/sizeof(*nim_dl_pcie_test); 
    for (cnt = 0; cnt < size; cnt++) {
        rc += nim_dl_pcie_test[cnt](); 
    }

    if (is_daughter_card_present()) {
        size = sizeof(nim_dl_poe_pcie_test)/sizeof(*nim_dl_poe_pcie_test); 
        for (cnt = 0; cnt < size; cnt++) {
            rc += nim_dl_poe_pcie_test[cnt](); 
        }
    }

    if (rc == PASSED) {
        return (PASSED);
    } else {
        return (rc); 
    }
}
#endif

/*
 * Function: dl_pcie_utils
 *
 * Description : an entry for dreamliner pcie utils 
 *
 * Inputs: slot - slot number, left for future reference.
 *
 * Output: rc - 0 for dummy value 
 *              
 */
int dl_pcie_utils (int slot) {

    struct diag_menu_def *utils, *tlb_start; 
    submenu_xtable_t *start, *ptr;
    int size = 0; 
    int ia = 0, option, rc = 0; 

TOP_MENU:
    size = sizeof(nim_dl_utils_menu)/sizeof(diag_menu_def_t);

    /* select dreamliner utils */
    utils = nim_dl_utils_menu; 
    tlb_start = utils; 
    while (ia < size) {
        printf("%d - %s\n", ia,  utils->name); 
        utils++;
        ia++; 
    }
    option = getdec_answer("Select item in dec: ", 0, 0, (size - 1)); 

    ia = 0; 
    utils = tlb_start; 
    while (ia < size) {
        if (ia == option) {
            ptr = utils->menu; 
            break; 
        }
        ia++;
        utils++; 
    }

START:
    /* dump and execute utils menu  */
    ptr = utils->menu; 
    ia = 0; 
    start = ptr; 
    while (ptr->x_pfunc != NULL) {
        printf("%d - %s\n", ia,  ptr->x_title); 
        ptr++; 
        ia++; 
    }
    size = ia; 

    printf("Input %d for go back to upper level menu\n", size); 
    option = getdec_answer("Select item in dec: ", 0, 0, size); 

    ia = 0; 
    if (option == size) {
        goto TOP_MENU;
    }
    ptr = start; 
    
    while (ia < size) {
        if (ia == option) {
            ptr->x_pfunc(); 

            if(getc_answer("Continue? (y/n) ","yn",'y') == 'y' ) {
                 ;
            } else {
                 return (rc);
            } 

            goto START; 
        }
        ptr++; 
        ia++; 
    }

    return (rc);
}

/*
 * Function: get_slot_num 
 *
 * Description : return nim slots
 *
 * Inputs: NONE
 *
 * Output: nim_slot - global value 
 *              
 */
int get_slot_num (void) 
{
    return (nim_slot); 
}

/*
 * Function: get_bus_num 
 *
 * Description : return ngio pcie device bus number 
 *
 * Inputs: mod_type - NIM or SM
 *         slot - slot number 
 *
 * Output: bus num
 *              
 */
int get_bus_num (uint mod_type, uint slot)
{
    return (get_ngio_pcie_dev_bus_num(mod_type, slot));
}

/*
 * Function: get_port_num 
 *
 * Description : dreamliner ge ports number
 *
 * Inputs: NONE
 *
 * Output: port - ports number, 4 or 8 ports
 *              
 */
int get_port_num (void)
{

    if (nim_types == 0) {
        return 8; 
    } else {
        return 4; 
    }
#if 0
    printf("fixme func:%s return 8, need to get info from cookie \n"
            , __FUNCTION__);
    /* it should base on board id to return 4 or 8 */
    if (board_id == NIM_ES2_4) { 
        return 4; 
    } else {
        return 8; 
    }
#endif 

}

/*
 * Function: cterr_setup
 *
 * Description : setup cterr msg for dl, used by dreamliner files 
 *
 * Inputs: NONE
 *
 * Output: NONE
 *              
 */
void cterr_setup(void) {

 //   printf("fixme func %s  return;\n", __FUNCTION__);
    return; 
}

/*
 * Function: dl_pcie_config_read
 *
 * Description : read dreamliner pcie device register 
 *
 * Inputs: offset - pcie register offset 
 *         reg_ptr - register value pointer 
 *
 * Output: NONE
 *              
 */
void dl_pcie_config_read (int offset, unsigned int *reg_ptr)
{
    unsigned int  bus;
    int device;

    bus = get_ngio_pcie_dev_bus_num(WIC_MODULE, nim_slot); 
    device = 0;

    *reg_ptr = pci_config_read(bus, device, 0, offset);
    return; 
}

/*
 * Function: dl_pcie_config_write
 *
 * Description : write dreamliner pcie device register 
 *
 * Inputs: offset - pcie register offset 
 *         reg_data - data write to register 
 *
 * Output: NONE
 *              
 */
void
dl_pcie_config_write (int offset, unsigned int reg_data)
{
    unsigned int bus;
    int device;

    bus = get_ngio_pcie_dev_bus_num(WIC_MODULE, nim_slot); 
    device = 0;

    pci_config_write(bus, device, 0, offset, reg_data);
    return; 
}

/*------------------------------------------------------------------
$Log: nim_dreamliner.c,v $
Revision 1.2  2016/04/20 08:53:59  benchen2
add tachi fru portion

Revision 1.1.2.1  2016/01/29 02:23:39  alpeng
rename files

Revision 1.1.2.8  2015/12/18 09:27:31  alpeng
fix dreamliner msg, supporting switch diag menu dynamically

Revision 1.1.2.7  2015/12/09 10:35:56  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.6  2015/11/03 09:43:51  alpeng
update dreamliner utility to support sw bridge

Revision 1.1.2.5  2015/10/05 10:21:38  alpeng
support single test, update loopback test

Revision 1.1.2.4  2015/09/30 06:02:19  alpeng
update dreamliner util, test and menu

Revision 1.1.2.3  2015/09/14 08:02:29  alpeng
update to support dreamliner

Revision 1.1.2.2  2015/08/19 08:08:18  alpeng
support both sjc-acme-v07 and sjc-foxconn-02; adding function prologue; clean up code

Revision 1.1.2.1  2015/08/17 02:33:03  alpeng
first check in for tachi-intel test; fix smart_cookie.c and free.h

$Endlog$
*/


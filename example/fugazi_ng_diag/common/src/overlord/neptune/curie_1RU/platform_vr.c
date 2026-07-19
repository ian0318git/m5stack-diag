/* $Id: platform_vr.c,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_vr.c,v $
 *---------------------------------------------------------------------------
 * platform_vr.c - Curie Voltage regulator, IR3570
 * They are connected to CPU I2C bus, not like Neptune, which are 
 * connected to FPGA I2C busses. 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *--------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include "cpu.h"
#include "platform_i2c.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "common_utils.h"
#include "platform_vr.h"  
#include "queryflags.h"   


/**************************************************************************
 *                              extern proto
 **************************************************************************/
extern uint32_t init_ir3570_i2c_struct(n2g_i2c_dev_t *, uint32_t);

/**************************************************************************
 *                              Function proto
 **************************************************************************/
static void init_i2c_struct(void);
//static n2g_i2c_if_t vtg_rgltr_i2c_if; 
static n2g_i2c_dev_t vr_i2c_dev;
static void read_vtg_rgltr_reg(void); 
static void write_vtg_rgltr_reg(void); 

static reg_info_t vtg_rgltr_reg_table[]=
{
    {"OPERATION",                        VR_OPERATION, 
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"CLEAR FAULTS",                        VR_CLEAR_FAULTS, 
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"CAPABILITY  ",                        VR_CAPABILITY, 
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"VOUT_MODE",                           VR_VOUT_MODE, 
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"VOUT_MARGIN_HIGH",                    VR_VOUT_MARGIN_HIGH,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"VOUT_MARGIN_LOW",                     VR_VOUT_MARGIN_LOW,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"STATUS_BTYE",                           VR_STATUS_BTYE,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"STATUS_WORD",                           VR_STATUS_WORD,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"STATUS_TEMPERATURE",                    VR_STATUS_TEMPERATURE, 
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"STATUS_CML",                            VR_STATUS_CML, 
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"STATUS_MFR_SPECIFIC",                   VR_STATUS_MFR_SPECIFIC,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"READ_VIN",                              VR_READ_VIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_IIN",                              VR_READ_IIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_VOUT",                             VR_READ_VOUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_IOUT",                             VR_READ_IOUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_TEMPERATURE_1",                    VR_READ_TEMPERATURE_1,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_TEMPERATURE_2",                    VR_READ_TEMPERATURE_2,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_POUT",                             VR_READ_POUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_PIN",                              VR_READ_PIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"PMBUS_REVISION",                        VR_PMBUS_REVISION,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"MFR_MODEL", /*byte conunt = 2 */          VR_MFR_MODEL,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"MFR_REVISION", /*byte conunt = 2 */       VR_MFR_REVISION,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"WRITE_REGISTER_PROCESS_CALL",             VR_WRITE_REGISTER_PROCESS_CALL,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_REGISTER_PROCESS_CALL",              VR_READ_REGISTER_PROCESS_CALL,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"GAMER COMMAND",                           VR_GAMER_COMMAND,
        WRITE_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"SET_POINTER",                             VR_SET_POINTER,
        WRITE_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"GET_POINTER",                             VR_GET_POINTER,
        READ_ONLY  | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"WRITE_REGISTER",                          VR_WRITE_REGISTER,
        WRITE_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"SET_I2C",                               VR_SET_I2C,
        READ_WRITE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"READ_EFFICIENCY",                         VR_READ_EFFICIENCY,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"MASK_STATUS_WORD",                        VR_MASK_STATUS_WORD,
        READ_WRITE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"MASK_TEMPERATUE",                         VR_MASK_TEMPERATUE,
        READ_WRITE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"MASK_CML",                                VR_MASK_CML,
        READ_WRITE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"MASK_MANUFACTURER",                      VR_MASK_MANUFACTURER,
        READ_WRITE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

/******************************************************************************
 *                                  Globals 
 ******************************************************************************/

/******************************************************************************
 *                                    Menus
 ******************************************************************************/ 
/*
 * Voltage regulator Main menu
 */
static submenu_xtable_t vtg_rgltr_menu_table[] = {
    {"Read Voltage regulator registers",   (PFT)read_vtg_rgltr_reg,      0,
     0,                                    (type_t(*)())0, 0, (PFT)0,    0},
    {"Write Voltage regulator registers",  (PFT)write_vtg_rgltr_reg,     0,
     0,                                    (type_t(*)())0, 0, (PFT)0,    0},
};

#define VTG_RGLTR_MENU_TABLE_SIZE \
        (sizeof(vtg_rgltr_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t vtg_rgltr_menu_primary_items[VTG_RGLTR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t vtg_rgltr_menu_secondary_items[VTG_RGLTR_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo vtg_rgltrdiag = {
  "Voltage Regulator Utilities Menu",     /* title */
  0,				/* title string added by init_empty_menu */
  (PFT)menu_show_dflags,	/* shows major flags */
  0,				/* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  vtg_rgltr_menu_primary_items,
};
static struct menuinfo *vtg_rgltrdiagp = &vtg_rgltrdiag;

/**************************************************************************
 *
 * Function   : build_vtg_rgltr_menu
 * Description: Build Voltage Regulator menu
 * Inputs     : None.
 * Outputs    : None.
 *
 **************************************************************************
 */
void build_vtg_rgltr_menu (void)
{
    /* init i2c struceture first */
    init_i2c_struct();

    build_primary_submenu(vtg_rgltr_menu_table, VTG_RGLTR_MENU_TABLE_SIZE,
			  "VTG Regulator Utilities Menu", &vtg_rgltrdiagp);
    build_secondary_submenu(vtg_rgltr_menu_table, VTG_RGLTR_MENU_TABLE_SIZE,
			    vtg_rgltr_menu_secondary_items);

    menu(&vtg_rgltrdiag, vtg_rgltr_menu_secondary_items, 0);
}

/*******************************************************************************
 * 
 * Function   : init_i2c_struct
 * Description: Init Voltage regulator device structure
 * Inputs     : i2c_if - Points to I2C API interface struct
 * Outputs    : None
 *
 *******************************************************************************
 */
static void init_i2c_struct (void) 
{
    unsigned int chip;

    printf("Select Voltage regulator \n"); 
    printf("VCore (VCCIN)    = 1 \n"); 
    printf("1.05V (VCCSCSUS) = 2 \n"); 
    printf("1.05V (VCCGBE)   = 3 \n"); 
    printf("1.20V            = 4 \n"); 
    chip = getdec_answer("1-4", 1, 1, 4);
    switch (chip) {
    case 1: 
        chip = MB_I2C_IR3570_VCORE;      
    break; 
    case 2: 
        chip = MB_I2C_IR3570_VCCSCSUS;       
    break; 
    case 3: 
        chip = MB_I2C_IR3570_VCCGBE;          
    break; 
    case 4: 
        chip = MB_I2C_IR3570_1_2V;         
    break; 
    /* no default, since we isolate case from 1-4; */
    }

    if (init_ir3570_i2c_struct(&vr_i2c_dev, chip) != PASSED) {
        printf("Failed to init IR3570 \n");
        /* fail through, since it is utility */   
    }
}

/**************************************************************************
 * Function   : dump_register_info
 * Description: dump naming and address of available register
 * Inputs     : reg_ptr - register table pointer 
 * Outputs    : NONE
 **************************************************************************
 */
static void dump_register_info (reg_info_t *reg_ptr)
{
    unsigned int ix = 1; 

    while(reg_ptr->offset != 0xFFFFFFFF) {  /* 0xFFFFFFFF for end of table */
        printf(" %-28s - %02x;", reg_ptr->name, reg_ptr->offset);
        ix++; 
        reg_ptr++;
        if (ix % 2) {
            printf("\n");  
        }    
    }
    printf("\n");  
}

/**************************************************************************
 * Function   : return_register_size
 * Description: return register size for bytes
 * Inputs     : offset - register offset 
 *              reg_ptr - register table pointer 
 * Outputs    : size of reg.
 **************************************************************************
 */
static int return_register_size (unsigned int offset, reg_info_t *reg_ptr)
{
    unsigned int bad_size = 0xFF; 

    while(reg_ptr->offset != 0xFFFFFFFF) { /* 0xFFFFFFFF for End of table */
        if (reg_ptr->offset == offset) {
            if (reg_ptr->mask == 0xFF) {
                return (1);  /* size is 1 */
            } else if (reg_ptr->mask == 0xFFFF) {
                return (2);  /* size is 2 */
            } else {
                return (0xFF); /* 0xff used for exit utility */
            }
        }
        reg_ptr++;    
    }
    printf("offset 0x%x is not matching with any register, exiting...\n", offset); 
    return (bad_size); 
}

/**************************************************************************
 *
 * Function   : read_vtg_rgltr_reg
 * Description: Read VTG regulator registers value
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void read_vtg_rgltr_reg (void) 
{
    unsigned int  rc, size, ia; 
    unsigned char r8[8], offset; 

    dump_register_info(&vtg_rgltr_reg_table[0]); 

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);

    /* check register is matching to table and return size for read */
    size = return_register_size(offset, &vtg_rgltr_reg_table[0]);
    if (size == 0xFF) {
        return;  /* bad offset, exit util */
    }


    /* write address before read data */
    vr_i2c_dev.wr_hd_size = 0;

    rc = api_mb_i2c_write(&vr_i2c_dev, 0, 1, 
                         (char *)&offset);
    if (rc != PASSED) { 
        printf("unable to write i2c.\n");
        return;
    }


    /* drv, offset, size, buf */
    rc = api_mb_i2c_read(&vr_i2c_dev, offset, size, 
                         (char *)r8);
    if (rc != PASSED) { 
        printf("unable to read i2c.\n");
        return;
    }

    printf("\n");
    for (ia = 0; ia < size ; ia++) {
        printf("0x%02x ", r8[ia]);
    }
} 

/**************************************************************************
 *
 * Function   : write_vtg_rgltr_reg
 * Description: Write VTG Regulator registers value
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void write_vtg_rgltr_reg (void) 
{
    unsigned int  rc, data, max, size;
    unsigned char  offset;

    dump_register_info(&vtg_rgltr_reg_table[0]); 

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);

    /* check register is matching to table and return size for read */
    size = return_register_size(offset, &vtg_rgltr_reg_table[0]);
    if (size == 0xFF) {
        return;  /* bad offset, exit util */
    }

    if (size == 1) {
        max = 0xFF;
    } else { 
        max = 0xFFFF;
    } 

    data = gethex_answer("Enter write data", 0, 0, max);

    /* using headr size = 1, to fill address before data (PMBUS protocol)*/
    vr_i2c_dev.wr_hd_size = 1;
    /* drv, offset, size, buf */
    rc = api_mb_i2c_write(&vr_i2c_dev, offset, size, 
                         (char *)&data);
    if (rc != PASSED) { 
        printf("unable to write i2c.\n");
        return;
    }
}

/******** History ********
*----------------------------------------------------
$Log: platform_vr.c,v $
Revision 1.2  2019/08/06 06:56:14  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.4  2018/12/27 07:49:54  alpeng
based on prrq comments to refine these files for message display

Revision 1.1.2.3  2018/10/16 12:11:34  alpeng
fix voltage regultor util

Revision 1.1.2.2  2018/07/19 09:27:37  alpeng
1. Moving IR3570 chips to CPU I2C bus. 2. Removed related code of i2c devices which are not support on Curie; max1617, IDT8T49N287I(sys clk), poe psu, 30w poe

Revision 1.1.2.1  2018/06/22 08:05:19  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:38  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:25:01  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.1  2018/02/02 06:45:32  leschen
Initial check in to support IR3570 voltage regulator utility.



$Endlog$
*/

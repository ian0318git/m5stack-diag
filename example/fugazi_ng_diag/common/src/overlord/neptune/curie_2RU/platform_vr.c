/* $Id: platform_vr.c,v 1.1 2020/01/09 01:02:05 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_vr.c,v $
 *---------------------------------------------------------------------------
 * platform_vr.c - Curie Voltage regulator, IR3570
 * They are connected to CPU I2C bus, not like Neptune, which are 
 * connected to FPGA I2C busses. 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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
extern uint32_t init_tps536xx_i2c_struct(n2g_i2c_dev_t *, uint32_t);

/**************************************************************************
 *                              Function proto
 **************************************************************************/
static void init_i2c_struct(void);
//static n2g_i2c_if_t vtg_rgltr_i2c_if; 
static n2g_i2c_dev_t vr_i2c_dev;
static void read_vtg_rgltr_reg(void); 
static void write_vtg_rgltr_reg(void); 

static void init_i2c_struct_2ru(void);
static void read_vtg_rgltr_reg_2ru(void);
static void write_vtg_rgltr_reg_2ru(void);
static void vtg_reg_rgltr_dump_2ru(void);

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

static reg_info_t vtg_rgltr_reg_table_2ru[]=
{
    {"PAGE",                       CURIE2RU_VR_PAGE,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"OPERATION",                  CURIE2RU_VR_OPERATION,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"CLEAR FAULTS",               CURIE2RU_VR_CLEAR_FAULTS,
        WRITE_ONLY | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"CAPABILITY  ",               CURIE2RU_VR_CAPABILITY,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"VOUT_MODE",                  CURIE2RU_VR_VOUT_MODE,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"VOUT_MARGIN_HIGH",           CURIE2RU_VR_VOUT_MARGIN_HIGH,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"VOUT_MARGIN_LOW",            CURIE2RU_VR_VOUT_MARGIN_LOW,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"STATUS_BTYE",                CURIE2RU_VR_STATUS_BTYE,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"STATUS_WORD",                CURIE2RU_VR_STATUS_WORD,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"STATUS_TEMPERATURE",         CURIE2RU_VR_STATUS_TEMPERATURE,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"STATUS_CML",                 CURIE2RU_VR_STATUS_CML,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"STATUS_MFR_SPECIFIC",        CURIE2RU_VR_STATUS_MFR_SPECIFIC,
        READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"READ_VIN",                   CURIE2RU_VR_READ_VIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_IIN",                   CURIE2RU_VR_READ_IIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_VOUT",                  CURIE2RU_VR_READ_VOUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_IOUT",                  CURIE2RU_VR_READ_IOUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_TEMPERATURE_1",         CURIE2RU_VR_READ_TEMPERATURE_1,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_POUT",                  CURIE2RU_VR_READ_POUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"READ_PIN",                   CURIE2RU_VR_READ_PIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"PMBUS_REVISION",             CURIE2RU_VR_PMBUS_REVISION,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFF, 0x00},
    {"MFR_MODEL",                  CURIE2RU_VR_MFR_MODEL,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"MFR_REVISION",               CURIE2RU_VR_MFR_REVISION,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"MFR_SPECIFIC_04",            CURIE2RU_VR_MFR_SPECIFIC_04,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"End", 0xFFFFFFFF, READ_ONLY, {0}, 0, 0},
};

static reg_info_t vtg_rgltr_reg_dump_table_2ru[]=
{
    {"VIN (V)",                   CURIE2RU_VR_READ_VIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"IIN (A)",                   CURIE2RU_VR_READ_IIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"PIN (W)",                   CURIE2RU_VR_READ_PIN,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"VOUT (V)",                  CURIE2RU_VR_MFR_SPECIFIC_04,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"IOUT (A)",                  CURIE2RU_VR_READ_IOUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"POUT (W)",                  CURIE2RU_VR_READ_POUT,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
    {"TEMPERATURE (C)",           CURIE2RU_VR_READ_TEMPERATURE_1,
        READ_ONLY  | REG_ACCESS, {(uint)REG_EXT},
        0xFFFF, 0x0000},
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

static submenu_xtable_t vtg_rgltr_menu_table_2ru[] = {
    {"Read Voltage regulator registers",   (PFT)read_vtg_rgltr_reg_2ru,      0,
     0,                                    (type_t(*)())0, 0, (PFT)0,    0},
    {"Write Voltage regulator registers",  (PFT)write_vtg_rgltr_reg_2ru,     0,
     0,                                    (type_t(*)())0, 0, (PFT)0,    0},
    {"Voltage regulator dump",             (PFT)vtg_reg_rgltr_dump_2ru,      0,
     0,                                    (type_t(*)())0, 0, (PFT)0,    0},
};

#define VTG_RGLTR_MENU_TABLE_SIZE_2RU \
        (sizeof(vtg_rgltr_menu_table_2ru) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t vtg_rgltr_menu_primary_items_2ru[VTG_RGLTR_MENU_TABLE_SIZE_2RU + MAX_BASE_ITEMS];
static mitem_t vtg_rgltr_menu_secondary_items_2ru[VTG_RGLTR_MENU_TABLE_SIZE_2RU + MAX_BASE_ITEMS];

static struct menuinfo vtg_rgltrdiag_2ru = {
  "Voltage Regulator Utilities Menu",     /* title */
  0,				/* title string added by init_empty_menu */
  (PFT)menu_show_dflags,	/* shows major flags */
  0,				/* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  vtg_rgltr_menu_primary_items_2ru,
};
static struct menuinfo *vtg_rgltrdiagp_2ru = &vtg_rgltrdiag_2ru;

/**************************************************************************
 *
 * Function   : build_vtg_rgltr_menu_2ru
 * Description: Build Curie-2RU Voltage Regulator menu
 * Inputs     : None.
 * Outputs    : None.
 *
 **************************************************************************
 */
void build_vtg_rgltr_menu_2ru (void)
{
    /* init i2c struceture first */
    init_i2c_struct_2ru();

    build_primary_submenu(vtg_rgltr_menu_table_2ru, VTG_RGLTR_MENU_TABLE_SIZE_2RU,
			  "VTG Regulator Utilities Menu", &vtg_rgltrdiagp_2ru);
    build_secondary_submenu(vtg_rgltr_menu_table_2ru, VTG_RGLTR_MENU_TABLE_SIZE_2RU,
			    vtg_rgltr_menu_secondary_items_2ru);

    menu(&vtg_rgltrdiag_2ru, vtg_rgltr_menu_secondary_items_2ru, 0);
}

/*******************************************************************************
 *
 * Function   : init_i2c_struct_2ru
 * Description: Init Voltage regulator device structure for Curie-2RU
 * Inputs     : i2c_if - Points to I2C API interface struct
 * Outputs    : None
 *
 *******************************************************************************
 */
static void init_i2c_struct_2ru (void)
{
    unsigned int chip;

    printf("Select Voltage regulator \n");
    printf("VCore/0.85_VCCSA  = 1 \n");
    printf("1.2V/1.05         = 2 \n");
    printf("1.2V/0.9V NN      = 3 \n");
    printf("1.0V              = 4 \n");
    chip = getdec_answer("1-4", 1, 1, 4);
    switch (chip) {
    case 1:
        chip = MB_I2C_TPS536XX_VCORE_0P85_VCCSA;
    break;
    case 2:
        chip = MB_I2C_TPS536XX_1P2V_1P05;
    break;
    case 3:
        chip = MB_I2C_TPS536XX_1P2V_0P9VNN;
    break;
    case 4:
        chip = MB_I2C_TPS536XX_1P0V;
    break;
    /* no default, since we isolate case from 1-4; */
    }

    if (init_tps536xx_i2c_struct(&vr_i2c_dev, chip) != PASSED) {
        printf("Failed to init TPS536XX \n");
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
 * Function   : read_vtg_rgltr_reg_2ru
 * Description: Read Curie-2RU VTG regulator registers value
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void read_vtg_rgltr_reg_2ru (void)
{
    unsigned int  rc, size, ia;
    unsigned char r8[8], offset;

    dump_register_info(&vtg_rgltr_reg_table_2ru[0]);

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);

    /* check register is matching to table and return size for read */
    size = return_register_size(offset, &vtg_rgltr_reg_table_2ru[0]);
    if (size == 0xFF) {
        return;  /* bad offset, exit util */
    }

    vr_i2c_dev.rd_hd_size = 1;

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
 * Function   : write_vtg_rgltr_reg_2ru
 * Description: Write Curie-2RU VTG Regulator registers value
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void write_vtg_rgltr_reg_2ru (void)
{
    unsigned int  rc, data, max, size;
    unsigned char  offset;

    dump_register_info(&vtg_rgltr_reg_table_2ru[0]);

    offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);

    /* check register is matching to table and return size for read */
    size = return_register_size(offset, &vtg_rgltr_reg_table_2ru[0]);
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

/**************************************************************************
 *
 * Function   : parse_pmbus_linear_word
 * Description: Parse pmbus linear word to string
 * Inputs     : lw - pmbus linear word
 *              str - result string
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void parse_pmbus_linear_word (__u8 dbl, __u8 dbh, char *str)
{
    short exponent, mantissa;
    int ret;

    exponent = (short)((char)dbh >> 3);
    mantissa = (short)(((char)dbh & 0x07) << 5) << 3 | dbl;

    if (exponent >= 0) {
        ret = mantissa * 100 * (1 << exponent);
    } else {
        ret = mantissa * 100 / (1 << (0 - exponent));
    }

    sprintf(str, "%d.%02d", ret / 100, ret % 100);
}

/**************************************************************************
 *
 * Function   : __vtg_reg_rgltr_dump_2ru
 * Description: Dump Voltage, Current, Power, and Temperature
 * Inputs     : reg_ptr - register table pointer
 *              page - channel (0 - channel A, 1 - channel B)
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void __vtg_reg_rgltr_dump_2ru (reg_info_t *reg_ptr, int page)
{
    int rc;
    unsigned char r8[8];
    reg_info_t *tmp_reg_ptr = reg_ptr;
    char str[10];

    if (page == 0) {
        printf("Channel A:\n\n");
    } else if (page == 1) {
        printf("Channel B:\n\n");
    } else {
        printf("Wrong Page\n");
        return;
    }

    /*Set Channel Page*/
    vr_i2c_dev.wr_hd_size = 1;
    rc = api_mb_i2c_write(&vr_i2c_dev, CURIE2RU_VR_PAGE, 1,
                         (char *)&page);

    /*Print Title*/
    while (tmp_reg_ptr->offset != 0xFFFFFFFF) {  /* 0xFFFFFFFF for end of table */
        printf(" %-15s", tmp_reg_ptr->name);
        tmp_reg_ptr++;
    }

    printf("\n ");

    tmp_reg_ptr = reg_ptr;

    // TODO: Advise to lock registers

    while (tmp_reg_ptr->offset != 0xFFFFFFFF) {  /* 0xFFFFFFFF for end of table */
        vr_i2c_dev.rd_hd_size = 1;

        /* drv, offset, size, buf */
        rc = api_mb_i2c_read(&vr_i2c_dev, tmp_reg_ptr->offset, 2,
                             (char *)r8);
        if (rc != PASSED) {
            printf("\n\nunable to read i2c.\n");
            return;
        }

        parse_pmbus_linear_word(r8[0], r8[1], str);
        printf(" %-15s", str);
        tmp_reg_ptr++;
    }

    // TODO: Advise to unlock registers

    printf("\n\n");
}

/**************************************************************************
 *
 * Function   : vtg_reg_rgltr_dump
 * Description: Dump Voltage, Current, Power, and Temperature both for
 *              channel A and channel B
 * Inputs     : NONE
 * Outputs    : NONE
 *
 **************************************************************************
 */
static void vtg_reg_rgltr_dump_2ru (void)
{
    __vtg_reg_rgltr_dump_2ru(&vtg_rgltr_reg_dump_table_2ru[0], 0);
    __vtg_reg_rgltr_dump_2ru(&vtg_rgltr_reg_dump_table_2ru[0], 1);
}

/*
 *-----------------------------------------------------------------------------
$Log: platform_vr.c,v $
Revision 1.1  2020/01/09 01:02:05  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */

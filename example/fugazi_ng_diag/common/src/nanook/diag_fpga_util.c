/* $Id: diag_fpga_util.c,v 1.2 2019/12/11 10:10:29 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_fpga_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_util.c - FPGA Utilities
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "queryflags.h"
#include "slot.h"
#include "dash_fpga.h"
#include "diag_fpga_util.h"
#include "platform_prom.h"


/* Function prototype */
extern int dash_set_map(int);
extern int dash_alt_mem(int argc);
extern int dash_dis_mem(int argc);
extern int dash_fil_mem(int argc);
extern int erase_config_header(int);
extern int display_sata_mux_setting(int);
extern int program_sata_mux_ngwic_setting(int);
extern int program_sata_mux_cpu_direct_setting(int);
extern int program_boot_upgrade_flag(int);
extern int program_reggio_spi_prom(void);
extern void tam_aikido_reset_utilty(void);
extern int platform_intr_test (int dummy);
extern int platform_ser_irq_intr_test(int dummy);
extern int nios_test_pwr_reg(void);
extern int nios_test_temp_reg(void);
extern int nios_test_fan_reg(void);

static int display_brd_info(int);
static int reset_dev(int);
static int power_ngio(int);
static int nios_register_test(int);

static struct mitem reggio_fpga_items[] = {
    {"Platfrom FPGA Program SPI PROM image without header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,  &zero, 0, (type_t(*)())0, 0},
    {"Platform FPGA Program SPI PROM image with header",  0, 0,
     (type_t(*)())program_reggio_spi_prom_old,   &one, 0, (type_t(*)())0, 0},
    {"Aikido Program FPGA SPI PROM image", 0, 0, 
     (type_t(*)())program_reggio_spi_prom,      &zero, 0, (type_t(*)())0, 0},
    {"Erase/Program Image Upgrade Header",  0, 0,
     (type_t(*)())program_image_upgrade_header,   &one, 0, (type_t(*)())0, 0},
    {"Set FPGA update flag",  0, 0,
     (type_t(*)())program_image_update_type,   &one, 0, (type_t(*)())0, 0},
    {"Set FPGA revision and date",  0, 0,
     (type_t(*)())set_date_revision,   &one, 0, (type_t(*)())0, 0},
    {"Display FPGA MULTI BOOT registers",  0, 0,
     (type_t(*)())display_multiboot,   &one, 0, (type_t(*)())0, 0},
    {"Display a sector", 0, 0, 
     (type_t(*)())display_prom_sector,   &zero, 0, (type_t(*)())0, 0},
    {"Test NIOS SPI",  0, 0,
     (type_t(*)())nios_test_spi_prom,  &three, 0, (type_t(*)())0, 0},
    {"Show Board type/FPGA Version",           0, 0,
     (type_t(*)())display_brd_info, &one, 0, (type_t(*)())0, 0},
    {"Erase Config header Sector",           0, 0,
     (type_t(*)())erase_config_header, &one, 0, (type_t(*)())0, 0},
    {"Display SATA mux setting",           0, 0,
     (type_t(*)())display_sata_mux_setting, &one, 0, (type_t(*)())is_overlord, 0},
    {"Program SATA MUX NGWIC Mode",           0, 0,
     (type_t(*)())program_sata_mux_ngwic_setting, &one, 0, (type_t(*)())is_overlord, 0},
    {"Program SATA MUX CPU Mode",           0, 0,
     (type_t(*)())program_sata_mux_cpu_direct_setting, &one, 0, (type_t(*)())is_overlord, 0},
    {"Program Boot Upgrade Flags",           0, 0,
     (type_t(*)())program_boot_upgrade_flag, &one, 0, (type_t(*)())0, 0},
    {"Reset internal devices",           0, 0,
     (type_t(*)())reset_dev, &zero, 0, (type_t(*)())0, 0},
    {"Reset external devices",           0, 0,
     (type_t(*)())reset_dev, &one, 0, (type_t(*)())0, 0},
    {"Enable/Disable NGIO",           0, 0,
     (type_t(*)())power_ngio, &one, 0, (type_t(*)())0, 0},
    {"rd/wr test",           0, 0,
     (type_t(*)())dash_rd_wr_test, &one, 0, (type_t(*)())0, 0},
    {"FPGA intr test", 0, 0,
     (type_t(*)())platform_intr_test,   &one, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0, 0},
    /*{"serial IRQ intr test", 0, 0,
     (type_t(*)())platform_ser_irq_intr_test,   &one, MF_CONTINUOUS | MF_DOGRP, (type_t(*)())0, 0},*/
    {"Toggle to CPLD/FPGA (default FPGA)", 0, 0, 
     (type_t(*)())dash_set_map,   &one, 0, (type_t(*)())0, 0},
    {"FPGA read", 0, 0, 
     (type_t(*)())dash_dis_mem,   &one, 0, (type_t(*)())0, 0},
    {"FPGA fill", 0, 0, 
     (type_t(*)())dash_fil_mem,   &one, 0, (type_t(*)())0, 0},
    {"FPGA alter", 0, 0, 
     (type_t(*)())dash_alt_mem,   &one, 0, (type_t(*)())0, 0},
    {"AIKIDO SPI read", 0, 0, 
     (type_t(*)())aikido_spi_read_util, &zero, 0, (type_t(*)())0, 0},
    {"AIKIDO SPI write", 0, 0, 
     (type_t(*)())aikido_spi_write_util, &zero, 0, (type_t(*)())0, 0},
    /*{"AIKIDO reset and unreset", 0, 0, 
     (type_t(*)())tam_aikido_reset_utilty, &zero, 0, (type_t(*)())0, 0},
    {"toggle flag : aikido_mailbox_flag", 0, 0, 
     (type_t(*)())aikido_flag_mailbox, &zero, 0, (type_t(*)())0, 0},
    {"toggle flag: aikido_act2_flag", 0, 0, 
     (type_t(*)())aikido_flag_act2, &zero, 0, (type_t(*)())0, 0},*/
    {"NIOS register test", 0, 0, 
     (type_t(*)())nios_register_test, &zero, 0, (type_t(*)())0, 0},
};

static struct menuinfo reggio_fpga_menu = {
    "  FPGA utility Menu",
    0,
    0,
    0,
    sizeof(reggio_fpga_items)/sizeof(struct mitem),
    reggio_fpga_items,
};

struct menuinfo *reggio_fpga_menup = &reggio_fpga_menu;

/**********************************************************************
 *
 * Function: display_brd_info
 *
 * Description: display board info, ie version number, revision number,
 *              etc...
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
display_brd_info(int val)
{
    unsigned int fpga_ver, cpld_ver, fpga_brd, cpld_brd;
    get_platform_ver(1, &fpga_ver, &cpld_ver, &fpga_brd, &cpld_brd);
    //    get_platform_prom_boot1(1);
    //    get_platform_prom_sel1(1);
    return (PASSED);
}

static int
reset_dev(int val)
{
    unsigned int c;
    c = getdec_answer("enter '1' to reset; enter '0' to unreset", 1, 0, 1);

    if (val == 1) {
        dash_reset_ext(c);
    }
    if (val == 0) {
        dash_reset_int(c);
    }
    return PASSED;
}

/**********************************************************************
 *
 * Function: power_ngio
 *
 * Description: utility to turn on/off ngio power
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int
power_ngio(int val)
{
    unsigned int b, c, d;
    struct ngio_intf_t *ngio;

    b = getdec_answer("enter module type SM/WIC [1/2]", 1, 1, 2);
    c = getdec_answer("enter slot number (first slot is 1)", 1, 1, 5);
    d = getdec_answer("power on or off [1/0]", 0, 0, 1);
    
    switch (b) {
    case 1:
        ngio = (struct ngio_intf_t *)slot_get_ngiosm(c);
        break;
    case 2:
        ngio = (struct ngio_intf_t *)slot_get_ngiowic(c);
        break;
    default:
        printf("you won't come here \n"); 
        
    }
    
    if (d) {
        ngio->on(ngio);
        if (ngio->i2c_unreset(ngio)<0) {
            cterr('f', 0, "slot%d power_ok bit not set", ngio->slot);
        }
        ngio->uart_on(ngio);
        ngio->unreset(ngio);
    } else {
        ngio->off(ngio);
        ngio->reset(ngio);
    }
    
    return PASSED;
}

/**********************************************************************
 *
 * Function: nios_register_test
 *
 * Description: utility to test registers when enable nios
 *
 * Input : val -- not used
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
nios_register_test(int val)
{
    int ret = PASSED;
    
    set_nios_mode(NIOS_NORMAL_MODE);
    if (nios_test_pwr_reg() != PASSED) {
        ret = FAILED;
    }

    if (nios_test_temp_reg() != PASSED) {
        ret = FAILED;
    }

    if (nios_test_fan_reg() != PASSED) {
        ret = FAILED;
    }
    set_nios_mode(NIOS_DISABLE_MODE);
    
    if (ret == PASSED) {
        printf("NIOS register test pass\n");
    }

    return ret;
}

/*-------------------------------------------------
 * $Log: diag_fpga_util.c,v $
 * Revision 1.2  2019/12/11 10:10:29  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
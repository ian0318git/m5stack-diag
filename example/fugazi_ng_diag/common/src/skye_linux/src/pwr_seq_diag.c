/* $Id: pwr_seq_diag.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/pwr_seq_diag.c,v $
 *------------------------------------------------------------------
 *
 * pwr_seq_diag.c: Entry file of Diag function and utilities
 *                 for power sequencer, TI UCD90120.
 *
 * May 20 2014, Paul Lin(palin2).
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "string.h"
#include "skye_i2c.h"
#include "pwr_seq_diag.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
static int util_pwr_seq_reg_rd(int);
static int util_pwr_seq_reg_wr(int);
static int volt_margin_stat_rd(uchar *, uchar *);
int        build_pwr_seq_menu(int);
int        util_set_volt_margin(void);
int        skye_dump_volt_margins(void);
int        skye_set_volt_margin(uchar, uchar);

/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int getdec_answer(char *, uint, uint, uint max);
extern int dump_readback_reg(char *, uint16_t, uint16_t, uchar *);

/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
uchar ps_mux_ch = PCA9546A_I2C_CH0;   /* Power Sequencer I2C Mux channel: 0 */

static vm_setup_info_t volt_margin_setup_tbl[] = {
    {"3.3V",              (uchar)VOLT_3P3V_PAGE},
    {"2.5V",              (uchar)VOLT_2P5V_PAGE},
    {"1.2V",              (uchar)VOLT_1P2V_PAGE},
    {"1.0V",              (uchar)VOLT_1P0V_PAGE},
    {"1.8V",              (uchar)VOLT_1P8V_PAGE},
    {"1.35V_GX0",         (uchar)VOLT_1P35V_GX0_PAGE},
    {"1.35V_GX1",         (uchar)VOLT_1P35V_GX1_PAGE},
    {"All above rails",   (uchar)VOLT_ALL_PAGE},
};

static vm_setup_info_t volt_margin_dump_tbl[] = {
    {"3.3V",         (uchar)VOLT_3P3V_PAGE},
    {"2.5V",         (uchar)VOLT_2P5V_PAGE},
    {"1.2V",         (uchar)VOLT_1P2V_PAGE},
    {"1.0V",         (uchar)VOLT_1P0V_PAGE},
    {"1.8V",         (uchar)VOLT_1P8V_PAGE},
    {"1.35V_GX0",    (uchar)VOLT_1P35V_GX0_PAGE},
    {"1.35V_GX1",    (uchar)VOLT_1P35V_GX1_PAGE},
};

static int vm_tbl_sz = (sizeof(volt_margin_dump_tbl) / sizeof(vm_setup_info_t));

static vm_setup_info_t volt_margin_state_tbl[] = {
    {"Margin High",  (uchar)VOLT_MARGIN_HIGH},
    {"Normal",       (uchar)VOLT_NORMAL},
    {"Margin Low",   (uchar)VOLT_MARGIN_LOW},
};

/******************************************************************************
 *                                 Menus
 ******************************************************************************/
/*
 * Power sequencer SubMenu Table
 */
static submenu_xtable_t pwr_seq_table[] = {
    {"Dump all Volt. Margin stat",   (PFT)skye_dump_volt_margins,   TRUE,
     0,                              (type_t(*)())0,                0,
     (type_t(*)())0,                 0},
    {"Voltage Margin Set Utility",   (PFT)util_set_volt_margin,     TRUE,
      0,                             (type_t(*)())0,                0,
      (type_t(*)())0,                0},
    {"Read Power Seq. register",     (PFT)util_pwr_seq_reg_rd,      TRUE,
     0,                              (type_t(*)())0,                0,
     (type_t(*)())0,                 0},
    {"Write Power Seq. register",    (PFT)util_pwr_seq_reg_wr,      TRUE,
     0,                              (type_t(*)())0,                0,
     (type_t(*)())0,                 0},
};

#define PWR_SEQ_TABLE_SZ \
        (sizeof(pwr_seq_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pwr_seq_primary_items[PWR_SEQ_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t pwr_seq_secondary_items[PWR_SEQ_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t pwr_seq_submenu = {
    "%s SubMenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    pwr_seq_primary_items,
};

menuinfo_t *pwr_seq_submenup = &pwr_seq_submenu;


/*******************************************************************************
 *
 * Function   : build_pwr_seq_menu
 * Description: Function to build Power Sequencer SubMenu.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
build_pwr_seq_menu (int opt)
{
    char menu_title[32];
    snprintf(menu_title, sizeof(menu_title), "Power Sequencer");

    build_primary_submenu(pwr_seq_table, PWR_SEQ_TABLE_SZ,
                          menu_title, &pwr_seq_submenup);
    build_secondary_submenu(pwr_seq_table, PWR_SEQ_TABLE_SZ,
                            pwr_seq_secondary_items);

    /* Display Utility Menu */
    menu(pwr_seq_submenup, pwr_seq_secondary_items, 0);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_pwr_seq_reg_rd
 * Description: Wrapped uility to read Power Sequencer register(s).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_pwr_seq_reg_rd (int opt)
{
    uint16_t   r_len = 0, s_off = 0;
    uchar      r_data[PS_REG_SZ];

    memset(r_data, 0, sizeof(r_data));

    s_off = gethex_answer("Enter starting Reg. offset", 0, 0, 0xFF);
    r_len = getdec_answer("Enter Data bytes you want to read",
                           1, 1, ((int)PS_REG_SZ - (int)s_off));

    /* Enable I2C Mux channel 0 */
    if (skye_i2c_mux_ctrl_reg_wr(&ps_mux_ch) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel %d.\n",
               __FUNCTION__, ps_mux_ch);
        return (FAILED);
    }

    /* Read register(s) from Power Sequencer */
    if (plat_ps_i2c_rd(s_off, r_len, r_data) != PASSED) {
        printf("%s: Failed to read register from Power Sequencer.\n",
               __FUNCTION__);
        return (FAILED);
    }

    /* Dump value of read back register(s) */
    dump_readback_reg("Power Sequencer(I2C2, CH0)", s_off, r_len, r_data);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_pwr_seq_reg_wr
 * Description: Wrapped uility to write Power Sequencer register(s).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_pwr_seq_reg_wr (int opt)
{
    uint16_t   w_len = 0, s_off = 0, ctr = 0;
    uchar      r_data[PS_REG_SZ], w_data[PS_REG_SZ];
    char       msg[PS_REG_SZ];

    memset(r_data, 0, sizeof(r_data));
    memset(w_data, 0, sizeof(w_data));
    memset(msg, 0, sizeof(msg));

    s_off = gethex_answer("Enter starting Reg. offset", 0, 0, 0xFF);
    w_len = getdec_answer("Enter Data bytes you want to write",
                           1, 1, ((int)PS_REG_SZ - (int)s_off));
    for (ctr = 0; ctr < w_len; ctr++) {
        snprintf(msg, sizeof(msg), "Enter data that you want to"
                                   " write into 0x%02X.",
                                   (s_off + ctr));
        w_data[ctr] = (uchar)gethex_answer(msg, 0, 0, 0xFF);
    }

    /* Enable I2C Mux channel 0 */
    if (skye_i2c_mux_ctrl_reg_wr(&ps_mux_ch) != PASSED) {
        printf("%s: Failed to Enable I2C Mux channel %d.\n",
               __FUNCTION__, ps_mux_ch);
        return (FAILED);
    }

    /* Write to Power Sequencer register(s). */
    if (plat_ps_i2c_wr(s_off, w_len, w_data) != PASSED) {
        printf("%s: Failed to write to Power Sequencer register(s).\n",
               __FUNCTION__);
        return (FAILED);
    }

    /* Read register(s) back from Power Sequencer */
    if (plat_ps_i2c_rd(s_off, w_len, r_data) != PASSED) {
        printf("%s: Failed to read register from Power Sequencer.\n",
               __FUNCTION__);
        return (FAILED);
    }

    /* Dump value of read back register(s) for user to reference */
    dump_readback_reg("Power Sequencer(I2C2, CH0)", s_off, w_len, r_data);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	util_set_volt_margin
 * Description:	Wrapper utility to set Voltage margin by accessing
 *              related register(s) of Power Sequencer.
 * Inputs     :	None
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
util_set_volt_margin (void)
{
    int      choice = 0, target = 0, vm_setup_sz = 0, vm_state_sz = 0, ctr = 0;
    uchar    page_num = 0, op_mode = 0;
    vm_setup_info_t *vm_setup_p, *vm_state_p;

    vm_setup_p  = &volt_margin_setup_tbl[0];
    vm_setup_sz = (sizeof(volt_margin_setup_tbl) / sizeof(vm_setup_info_t));

    vm_state_p  = &volt_margin_state_tbl[0];
    vm_state_sz = (sizeof(volt_margin_state_tbl) / sizeof(vm_setup_info_t));

    /* Dump all voltage rails current margin state */
    if (skye_dump_volt_margins() != PASSED) {
        printf("\n%s: Failed to dump all voltage rails margin state.\n",
               __FUNCTION__);
    }

    /* Display voltage rails menu & get user choice */
    printf("\nVoltage rails that Power sequencer controls:\n");
    for (ctr = 0; ctr < vm_setup_sz; ctr++, vm_setup_p++) {
        printf("%d. %s\n", ctr, vm_setup_p->name);
    }
    printf("%d. Cancel\n", vm_setup_sz);

    choice = getdec_answer("Please enter the rail you want: ",
                           vm_setup_sz, 0, vm_setup_sz);

    /* Handle User choice */
    if (choice == vm_setup_sz) {
        printf("\nCancelled by User.\n");
        return (PASSED);
    } else {
        vm_setup_p  = &volt_margin_setup_tbl[0];
        vm_setup_p += choice;
        page_num = vm_setup_p->value;
    }

    /* Display voltage magrin state menu & get user choice */
    printf("\nList of target voltage margins:\n");
    for (ctr = 0; ctr < vm_state_sz; ctr++, vm_state_p++) {
        printf("%d. %s\n", ctr, vm_state_p->name);
    }
    printf("%d. Cancel\n", vm_state_sz);

    target = getdec_answer("Please enter the target you want: ",
                           vm_state_sz, 0, vm_state_sz);

    /* Handle User choice */
    if (target == vm_state_sz) {
        printf("\nCancelled by User.\n");
        return (PASSED);
    } else {
        vm_state_p  = &volt_margin_state_tbl[0];
        vm_state_p += target;
        op_mode = vm_state_p->value;
    }

    if (skye_set_volt_margin(page_num, op_mode) != PASSED) {
        printf("\nFailed to set %s to %s.\n\n",
               vm_setup_p->name, vm_state_p->name);
        return (FAILED);
    }

    printf("\nSet %s to %s successfully.\n\n",
           vm_setup_p->name, vm_state_p->name);

    /* Dump all voltage rails margin state after changed */
    if (skye_dump_volt_margins() != PASSED) {
        printf("\n%s: Failed to dump all voltage rails margin state.\n",
               __FUNCTION__);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	volt_margin_stat_rd
 * Description:	Function to read Voltage margin stat.
 * Inputs     :	None
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
static int
volt_margin_stat_rd (uchar *page, uchar *stat)
{
    uchar    r_data = 0;
    uint16_t r_offset = 0, r_size = (uint16_t)ONE_B_REG;

    /* Choose voltage rail by setting Page (0x00). */
    r_offset = (uint16_t)PS_PAGE;
    if (plat_ps_i2c_wr(r_offset, r_size, page) != PASSED) {
        printf("%s: Failed to write 0x%02X to Power Sequencer"
               " (Command code: 0x%02X).\n",
               __FUNCTION__, *page, r_offset);
        return (FAILED);
    }

    /* Read page register to confirm write in data. */
    if (plat_ps_i2c_rd(r_offset, r_size, &r_data) != PASSED) {
        printf("%s: Failed to read from Power Sequencer"
               " (Command code: 0x%02X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_data != *page) {
        printf("%s: Failed to change power seq. page Reg.(0x%02X) to 0x%02X, "
               "it is 0x%02X now.\n",
               __FUNCTION__, r_offset, *page, r_data);
        return (FAILED);
    }

    /* Choose voltage margin target by setting Operation (0x01). */
    r_offset = (uint16_t)PS_OPERATION;
    if (plat_ps_i2c_rd(r_offset, r_size, stat) != PASSED) {
        printf("%s: Failed to read from Power Sequencer"
               " (Command code: 0x%02X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_dump_volt_margins
 * Description: Wrapped uility to dump all Voltage Margin stat.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_dump_volt_margins (void)
{
    uchar           vm_stat[PLAT_VOLT_SZ], c_stat = 0, e_chk[PLAT_VOLT_SZ];
    vm_setup_info_t *vm_p;
    int             ctr = 0;

    memset(vm_stat, 0, sizeof(vm_stat));
    memset(e_chk, 0, sizeof(e_chk));

    vm_p  = &volt_margin_dump_tbl[0];

    /* Enable I2C Mux channel 0 */
    if (skye_i2c_mux_ctrl_reg_wr(&ps_mux_ch) != PASSED) {
        cterr_db_print("%s: Failed to Enable I2C Mux channel %d.\n",
                       __FUNCTION__, ps_mux_ch);
        return (FAILED);
    }

    /* Read and record all voltage margin current stat */
    for (ctr = 0; ctr < vm_tbl_sz; ctr++, vm_p++) {
        if (volt_margin_stat_rd(&(vm_p->value), &c_stat) != PASSED) {
            cterr_db_print("%s: Failed to read %s margin stat "
                           "from Power Sequencer.\n",
                           __FUNCTION__, vm_p->name);
            e_chk[ctr] = TRUE;
            c_stat = 0;
            continue;
        }
        vm_stat[ctr] = c_stat;
        c_stat = 0;
    }

    /* Show all voltage margin current stat */
    vm_p  = &volt_margin_dump_tbl[0];

    cterr_db_print("Voltage margin current stat:\n");
    for (ctr = 0; ctr < vm_tbl_sz; ctr++, vm_p++) {
        cterr_db_print("%-9s: ", vm_p->name);

        if (e_chk[ctr] != FALSE) {
            cterr_db_print("Read Failed.\n");
            continue;
        }

        if (vm_stat[ctr] == VOLT_MARGIN_HIGH) {
            cterr_db_print("High.\n");
        } else if (vm_stat[ctr] == VOLT_MARGIN_LOW) {
            cterr_db_print("Low.\n");
        } else if (vm_stat[ctr] == VOLT_NORMAL) {
            cterr_db_print("Normal.\n");
        } else {
            cterr_db_print("Unknown Stat(0x%02X).\n", vm_stat[ctr]);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	skye_set_volt_margin
 * Description:	Function to set Voltage margin by access Skye power sequnencer
 *              related register(s).
 * Inputs     :	page_num - register page
 *              op_mode  - wanted margin mode
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_set_volt_margin (uchar page_num, uchar op_mode)
{
    uint16_t r_offset = 0, chk_val = 0;
    uint16_t reg_size = (uint16_t)ONE_B_REG;
    uchar    r_data = 0;
    int      ctr = 0, c_req = 0;
    vm_setup_info_t *vm_chk_p;


    /* Enable I2C Mux channel 0 */
    if (skye_i2c_mux_ctrl_reg_wr(&ps_mux_ch) != PASSED) {
        printf("\n%s: Failed to Enable I2C Mux channel %d.\n",
               __FUNCTION__, ps_mux_ch);
        return (FAILED);
    }

    /* Choose voltage rail by setting Page (0x00). */
    r_offset = (uint16_t)PS_PAGE;
    if (plat_ps_i2c_wr(r_offset, reg_size, (uchar*)&page_num) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Power Sequencer"
               " (Command code: 0x%02X).\n",
               __FUNCTION__, page_num, r_offset);
        return (FAILED);
    }

    /* Read page register to confirm write in data. */
    if (plat_ps_i2c_rd(r_offset, reg_size, &r_data) != PASSED) {
        printf("\n%s: Failed to read from Power Sequencer"
               " (Command code: 0x%02X).\n",
               __FUNCTION__, r_offset);
        return (FAILED);
    }

    if (r_data != page_num) {
        printf("\n%s: Failed to change page Reg(0x%02X)."
               " It is 0x%02X(expected: 0x%02X).\n",
               __FUNCTION__, r_offset, r_data, page_num);
        return (FAILED);
    }

    /* Choose voltage margin target by setting Operation (0x01). */
    r_offset = (uint16_t)PS_OPERATION;
    if (plat_ps_i2c_wr(r_offset, reg_size, (uchar*)&op_mode) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Power Sequencer"
               " (Command code: 0x%02X).\n",
               __FUNCTION__, op_mode, r_offset);
        return (FAILED);
    }

    /* Read operation register to confirm voltage magrin setup. */
    vm_chk_p  = &volt_margin_dump_tbl[0];
    if (page_num == (uchar)VOLT_ALL_PAGE) {
        c_req = vm_tbl_sz;
    } else {
        c_req = 1;

        for (ctr = 0; ctr < vm_tbl_sz; ctr++, vm_chk_p++) {
            if (vm_chk_p->value == page_num) {
                break;
            }
        }
    }

    for (ctr = 0; ctr < c_req; ctr++, vm_chk_p++) {
        r_offset = (uint16_t)PS_PAGE;
        chk_val = vm_chk_p->value;
        if (plat_ps_i2c_wr(r_offset, reg_size, (uchar*)&chk_val) != PASSED) {
            printf("\n%s: Failed to write 0x%02X to Power Sequencer"
                   " (Command code: 0x%02X).\n",
                   __FUNCTION__, chk_val, r_offset);
            return (FAILED);
        }

        /* Read page register to confirm write in data. */
        if (plat_ps_i2c_rd(r_offset, reg_size, &r_data) != PASSED) {
            printf("\n%s: Failed to read from Power Sequencer"
                   " (Command code: 0x%02X).\n",
                   __FUNCTION__, r_offset);
            return (FAILED);
        }

        if (r_data != chk_val) {
            printf("\n%s: Failed to change to %s's page. The value of power seq. "
                   "page Reg.(0x%02X) is 0x%02X now, not as expected (0x%02X).\n",
                   __FUNCTION__, vm_chk_p->name, r_offset, r_data, chk_val);
            return (FAILED);
        }

        r_data = 0;
        r_offset = (uint16_t)PS_OPERATION;
        if (plat_ps_i2c_rd(r_offset, reg_size, &r_data) != PASSED) {
            printf("\n%s: Failed to read from Power Sequencer"
                   " (Command code: 0x%02X).\n",
                   __FUNCTION__, r_offset);
            return (FAILED);
        }

        if (r_data != op_mode) {
            printf("\n%s: Failed to set %s.\n", __FUNCTION__, vm_chk_p->name);
            printf("Power sequncer Reg. 0x%02X is 0x%02X(expect: 0x%02X).\n",
                   r_offset, r_data, op_mode);

            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	skye_set_volt_margin_by_nc
 * Description:	Function to set Skye voltage margin by NC command.
 * Inputs     :	*nc_cmd - buffer to put margin setup from user
 * Outputs    :	PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_set_volt_margin_by_nc (char *nc_cmd)
{
    char            *token;
    uchar           op_mode = VOLT_NORMAL;
    vm_setup_info_t *vm_p;

    vm_p = &volt_margin_dump_tbl[0];

    /* Get the NC command set(s) */
    token = strtok(nc_cmd, ",");

    if (token == NULL) {
        printf("%s: Got NULL NC command string.\n", __FUNCTION__);
        return (FAILED);
    }

    while (token != NULL) {
        if (strcmp(token, "H") == 0) {
            op_mode = VOLT_MARGIN_HIGH;
        } else if (strcmp(token, "N") == 0) {
            op_mode = VOLT_NORMAL;
        } else if (strcmp(token, "L") == 0) {
            op_mode = VOLT_MARGIN_LOW;
        } else {
            printf("%s: Unknown command %s.\n", __FUNCTION__, token);
            return (FAILED);
        }

        /* Set voltage margin */
        if (skye_set_volt_margin(vm_p->value, op_mode) != PASSED) {
            printf("%s: Failed to set %s.\n", __FUNCTION__, vm_p->name);
            return (FAILED);
        }
        vm_p++;

        /* Get next command flag set */
        token = strtok(NULL, ",");
    }

    return (PASSED);
}


/******** History ********
$Log: pwr_seq_diag.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:34  steja
Code check-in to skye-branch2 for ER code review

-----------------------------------------------------------
Revision 1.1.2.3  2014/10/07 06:04:25  palin2
Added to set Skye voltages margin through NC command.

Revision 1.1.2.2  2014/08/28 08:03:23  palin2
Update Skye show all temp. and all voltage margin states utilities to
support enhanced error message.

Revision 1.1.2.1  2014/07/21 01:56:54  palin2
Initial check-in Skye module side Diag code.

-----------------------------------------------------------
Revision 1.1.2.3  2014/05/27 12:54:05  palin2
Updated voltage margin state check mechanism.

Revision 1.1.2.2  2014/05/26 15:23:52  palin2
1. Re-write power sequencer read/write function to make it more robust.
2. Re-write voltage margin item display method to make it more easier
   to add/delete item.

Revision 1.1.2.1  2014/05/20 17:54:50  palin2
Add power sequencer utilities and update power sequencer I2C r/w function.

-----------------------------------------------------------
$Endlog$
*/

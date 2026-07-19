/* $Id: ngwic_overdrive.c,v 1.45 2018/05/18 09:24:48 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/ngwic_overdrive.c,v $
 *------------------------------------------------------------------
 * ngwic_overdrive.c - This file contains functions for Overdrive NGWIC.
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 * linux will detect hard drives and name them as:
 * NIM: 
 * /dev/nimhdd1sda and /dev/nimhdd1sdb for hardrive 0 and 1 in slot1
 * /dev/nimhdd2sda and /dev/nimhdd2sdb for hardrive 0 and 1 in slot2
 * /dev/nimhdd3sda and /dev/nimhdd3sdb for hardrive 0 and 1 in slot3
 * SM:
 * /dev/smhdd1sda and /dev/smhdd1sdb for hardrive 0 and 1 in slot1
 * /dev/smhdd2sda and /dev/smhdd2sdb for hardrive 0 and 1 in slot2
 * /dev/smhdd3sda and /dev/smhdd3sdb for hardrive 0 and 1 in slot3
 * 
 * IO/expander: (more info see hardware spec)
 * 8 bit io exander: (offset 3 = configration regiters)
 *                   (offset 2 = ploarity inversion register)
 *                   (offset 1 = output port register)
 *                   (offset 0 = input port register)
 * 16 bit io exander: the low 8 bits are 0,2,4,and 6. high 8 bits are 1,3,5,and 7.
 *                  (offset 6/7 = configuration register)
 *                  (offset 4/5 = polarity register)
 *                  (offset 2/3 = output port register)
 *                  (offset 0/1 = inport port
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "platform_slot.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "queryflags.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "linux_usb_test.h"
#include "linux_pciutils.h"
#include "cross_platform.h"

#include <stdlib.h>
#include <unistd.h> /* stat for check file available */
#include <string.h> /* stat for check file available */

#include "dash_fpga.h" /*for fpga config */
#include "uio_utils.h" /*sticky bit */

extern int sata_tests(uchar *);
extern uint32_t pci_config_read(uint32_t, uint16_t, uint32_t, int);
extern uchar get_next_pci_exp_cap_ptri(uint32_t, uint16_t, 
                               uint32_t, uint32_t);
extern int get_platform_ver(unsigned int, unsigned int *, unsigned int *,
                            unsigned int *, unsigned int *);
extern void sata_cfg(boolean);

static void show_pci_speed(void);
static void show_info(void);
extern void dash_fillword(unsigned int *, int, unsigned int); /*sticky bit*/
static boolean get_hdd_present(int *);
static int hdd_test(int);
static int if_rdy(void);
static void pci_rdy(int mode);
static int ltc4215_register_test (void);
static int ltc4215_reg_write(void);
static int ltc4215_reg_read(void);
static int overdrive_power_off(void);
static int overdrive_pwr_off(void);
static int overdrive_pwr_on(void);
static void overdrive_cleanup(void);
static int change_mux_mode(int);
static int show_mux_mode(void);
static int reset_sata_ctrl(void);
static int unreset_sata_ctrl(void);
static int set_gpio_db_pins(void);
static int gpio_exp_write(int);
static int gpio_exp_read(int);
static void led_util(void);
static int toggle_led(int);
static void config_fpga(int);
static int config_ngio_sata_mux(int);
static int install_driver(int);
static int get_mux_mode(void);
static boolean is_pass_throu_available(void);
static void (*off_func)(void *) = NULL;

#define MODE_PASS  0
#define MODE_CTRL  1
#define OVERDRIVE_SATA_NUM  2

/* this should be platform definition */
static uint pcie_switch_bus_no = 0;

static boolean skip_setting = FALSE; 
static void (*overdrive_saved_diag_exec)(void) = NULL;
static void *oir_if;
static uint curr_slot;
static char pca_buff0[256];
static char pca_buff1[256];
static boolean is_sm_dc = FALSE;

static struct ngio_intf_t *overdrive_wic_iface;

/* possible name of hdd */
static char *linux_hdd_name[11] =
{
   "sda", "sdb", "sdc", "sdd", "sde", "sdf", "sdg", 
   "sdh", "sdi", "sdj", "sdk", 
};
static char *ngio_type_hdd[2] =
{
   "sm", "nim",
};
static int assigned_hdd_name[11];

/* addr of 8bit 0x38H >> 1; 16bit 0x48H >> 1 */
static n2g_i2c_if_t pca_i2c[] = {
    {
        .i2c_dev = 0x1C,  
    },
    {
        .i2c_dev = 0x24,
    },
};

#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)

/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Power off Overdrive NGWIC",     (PFT)overdrive_power_off,   0,   MM_1,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Overdrive NGWIC",      (PFT)overdrive_pwr_on,      0,   MM_1,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Change to SATA Pass-through Mode",   (PFT)change_mux_mode,  MODE_PASS,   MM_2,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
    {"Change to SATA Controller Mode",     (PFT)change_mux_mode,  MODE_CTRL,   MM_2,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
    {"Overdrive HDD test - PT mode (slot3 only)", (PFT)hdd_test,  MODE_PASS,   MM_3,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
    {"Overdrive HDD test - Ctrl mode",            (PFT)hdd_test,  MODE_CTRL,   MM_3,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Show Current info ",            (PFT)show_info,           0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LED utilities. ",               (PFT)led_util,            0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Rescan PCI bus",    (PFT)install_driver,            0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Test",         (PFT)ltc4215_register_test, 0,   MM_3,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Read",         (PFT)ltc4215_reg_read,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Register Write",        (PFT)ltc4215_reg_write,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Reset SATA Controller",              (PFT)reset_sata_ctrl,  0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Un-reset SATA Controller",              (PFT)unreset_sata_ctrl,  0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"8bit PCA9557 Register Write",        (PFT)gpio_exp_write,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"8bit PCA9557 Register Read",        (PFT)gpio_exp_read,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"16bit PCA9555 Register Write",        (PFT)gpio_exp_write,     1,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"16bit PCA9555 Register Read",        (PFT)gpio_exp_read,     1,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},    
    {"Config FPGA Pass-through Mode",   (PFT)config_fpga,  MODE_PASS,   0,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
    {"Config FPGA Controller Mode",   (PFT)config_fpga,  MODE_CTRL,   0,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
    {"Set NGIO SATA MUX to controller mode",   (PFT)config_ngio_sata_mux,  MODE_PASS,   0,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
    {"Set NGIO SATA MUX to passthru mode",   (PFT)config_ngio_sata_mux,  MODE_CTRL,   0,
     (type_t(*)())is_pass_throu_available, 0,	   (type_t(*)())0,          0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Overdrive Main Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;
  
/*****************************************************************************
 *
 * Function   : overdrive_fixup
 * Description: need to force idt switch to inform hotplug thta a module is
 *              detected. otherwise, hotplug might not get interrupt and won't
 *              see overdrive.
 * Inputs     : NONE 
 *             
 *              
 * Outputs    : NONE
 *
 *****************************************************************************/
void
overdrive_fixup (void)
{
    uint32_t bus = 2;
    uint16_t device_no = 0;
    uint32_t fn = 0;
    int offset;
    uint32_t data, slot;

    slot = curr_slot;
    device_no = get_wic_device_no(slot);
	
    if (access("./skip_pcie_conf", F_OK ) == 0 ) {
        printf("pci config reg 0x3F17C already modified.\n");
    } else {
        data = 0x3F17C;
        offset = 0xFF8;
        pci_config_write(bus, device_no, fn, offset, data);

        offset = 0xFFC;
        data = pci_config_read(bus, device_no, fn, offset);
        if (data != 0x14140800) {
            cterr('w', 0, "pcie config at %#x=%#x; expected 0x14140800",
                  offset, data);
        }
        data = 0x14140A00;
        pci_config_write(bus, device_no, fn, offset, data);
        data = pci_config_read(bus, device_no, fn, offset);
        if (data != 0x14140A00){
            cterr('w', 0, "pcie config at %#x=%#x; expected 0x14140A00",
                  offset, data);
        }
        system("touch ./skip_pcie_conf");
    }

}

/*************************************************************************
 * Function: overdrive_iface_test
 *
 * Test entry for Overdrive interface test.
 *      covered: I2C, GE0.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
overdrive_iface_test (void)
{

    FILE *fp;
    int reload;
    
    if (access("./pass_thru", F_OK) < 0) {
        /* test SATA on overdrive */
        if (hdd_test(MODE_CTRL)) {
            return (FAILED);
        }
    } else {
        /* use file pass_thru to tell software if we should reload hotplug module */
        if ((fp = fopen("./pass_thru", "r"))) {
            reload = 0;
            if (fscanf(fp, "reload=%d", &reload)) {
                if (reload) {
                    system("rmmod pciehp");
                    system("modprobe pciehp pciehp_force=1");
                }
            }
            fclose(fp);
        }
        if (hdd_test(MODE_PASS)) {
            return (FAILED);
        }
    }

    return (PASSED);
}

static void
set_gen_speed (int slot)
{
    int data = 0, device_no;

    if (skip_setting) {
        return;
    } else {
        device_no = get_wic_device_no(slot);
    
        if ((NVRAM)->diagflag & D_VERBOSE) {
            data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
            printf("pci offset 0x70 is %#x\n", data);
        }
        /* setpci -s 3:device 70.l=1 */
        pci_config_write(pcie_switch_bus_no, device_no, 0, 0x70, 1);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
            printf("pci offset 0x70 is %#x\n", data);
        }
    }
    return;
}

static void
set_gen_speed_dc (int slot)
{
    int data = 0, device_no;

    if (skip_setting) {
        return;
    } else {
        device_no = get_sm_device_no(slot);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
            printf("pci offset 0x70 is %#x\n", data);
        }
        /* setpci -s 3:device 70.l=1 */
        pci_config_write(pcie_switch_bus_no, device_no, 0, 0x70, 1);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
            printf("pci offset 0x70 is %#x\n", data);
        }
    }
    return;
}

static int
init_drive (int slot)
{
    uint device_id, vendor_id; 
    
    /* initialize PCIE switch bus number */

    /* only idt pcie swtich need to config pcie bus 
     * so juno-plx, utah-plx, sword, and dagger will not need to use bus num. 
     */
    if (is_utah_plx() || is_juno_plx()) {
        device_id = PLX_PCIE_SW_DID_8618;
        vendor_id = PLX_PCIE_SW_VID;
    } else if (is_sword()) {
        device_id = PLX_PCIE_SW_DID_8617;
        vendor_id = PLX_PCIE_SW_VID;
    } else if (is_dagger()) {
        device_id = PLX_PCIE_SW_DID_8604;
        vendor_id = PLX_PCIE_SW_VID;
    } else if (is_ntpn_machines() || is_vg450()) {
        device_id = PERICOM_PCIE_SW_DID;
        vendor_id = PERICOM_PCIE_SW_VID;
    } else {   /* idt pcie switch - o2, juno, utah */
        device_id = IDT_PCIE_SW_DID;
        vendor_id = IDT_PCIE_SW_VID;
    }

    pcie_switch_bus_no = get_pcie_bus_num(vendor_id, device_id);

    /* access ngio pcie swtich bus instead pcie swtich itself */
    /* so the bus number should be plus 1 */
    pcie_switch_bus_no += 1; 
  
    if (is_juno_plx() || is_utah_plx() || is_sword() || is_dagger() || is_ntpn_machines() || is_vg450()) {
        skip_setting = TRUE;
    } else {
        skip_setting = FALSE;
    }

    /* init pca for 9557 and 9555 */
    pca_init_i2c((void *)&pca_i2c[0]);
    pca_i2c[0].dev_name = "PCA9557";
    pca_i2c[0].i2c_ctrl = overdrive_wic_iface->i2c_ctrl;
    pca_i2c[0].i2c_dev = 0x1C;
    pca_i2c[0].buf = pca_buff0;

    pca_init_i2c((void *)&pca_i2c[1]);
    pca_i2c[1].dev_name = "PCA9555";
    pca_i2c[1].i2c_ctrl = overdrive_wic_iface->i2c_ctrl;
    pca_i2c[1].i2c_dev = 0x24;
    pca_i2c[1].buf = pca_buff1;

    oir_if = (void *)(overdrive_wic_iface->oir);

    if (overdrive_wic_iface->mod_type == SM_DAUGHTER_CARD) {
        set_gen_speed_dc(slot);
        is_sm_dc = TRUE; 
    } else {
        set_gen_speed(slot);
        is_sm_dc = FALSE;
    }

    /* ensure overdrive is powered on */
    if (overdrive_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Overdrive NGWIC");
        return(FAILED);
    }

    /* NOTE: on Juno PLX, we need to have unreset right after 
     * power on overdrive (less than 1 sec).
     * so we do unreset via early_unreset flag on platform_slot.c and slot.c */
#if 0 
    /* take Overdrive NGWIC out of reset before setting PCI ready bit per HW */
    overdrive_wic_iface->unreset(overdrive_wic_iface);
#endif 

    pci_rdy(get_mux_mode());

    set_gpio_db_pins();

    if (!if_rdy()) {
        printf("Device not ready. Ready bit of GPIO expander did not set");
        return(FAILED);
    }

    return(PASSED);

}
/*------------------------------------------------------------------------------
 *
 * Function: overdrive_test().
 *
 * Description: This function is the entry point for Overdrive NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int 
overdrive_test (void *wic)
{ 
    int ret_val = PASSED;

    assert(wic);
    overdrive_wic_iface = (struct ngio_intf_t *)wic;
    curr_slot = overdrive_wic_iface->slot;

    testname("Slot%d Overdirve NGWIC ", curr_slot);

    if (!off_func)
        off_func  = overdrive_wic_iface->off;

    if (!is_ntpn_machines() || is_vg450()) {
        overdrive_wic_iface->off = NULL;
    }

    if (init_drive(curr_slot)) {
        cterr('f', 0, "Failed to unreset overdrive");
        /* let code conintue for future debugging */
    }
   // msleep(9000);
    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    overdrive_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    if (overdrive_wic_iface->test_type == IFACE_TEST) {
        
	ret_val = overdrive_iface_test();
    } else if (overdrive_wic_iface->menu_display == FALSE) {
	ret_val = overdrive_iface_test();
    } else {
	menu(maindiagp, main_menu_secondary_items, '\0');
    }

    overdrive_cleanup();

    return (ret_val);
}

/**********************************************************************
 * Function: do_nothing
 *
 * Description: a function to clean up ngio->off
 *
 * Input:  None
 *
 * Output: None
 * NOTE:   So far overdrive cannot be power off, otherwise the
 *         HDD cannot be detect again. (need power cycle system)
 **********************************************************************
 */


/**********************************************************************
 * Function: overdrive_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void
overdrive_cleanup (void)
{
    assert(overdrive_wic_iface);

    if (overdrive_saved_diag_exec) {
        pre_diag_exec = overdrive_saved_diag_exec;
        overdrive_saved_diag_exec = NULL;
    }

}

/**********************************************************************
 * Function: set_gpio_db_pins.
 *
 * Description: This function will set the config register of the
 *              PCA9557 I2C device (GPIO expander) for the
 *              daughterboard related bits.
 *
 * Input:  none
 *
 * Output: PASSED/FAILED.
 **********************************************************************
 */
static int
set_gpio_db_pins (void)
{
    uchar data;
    n2g_i2c_if_t *pca, *pca1;
    uchar mask;

    pca = &pca_i2c[0]; /* 8 bit */
    pca1 = &pca_i2c[1]; /* 16bit */


    /* 8bit GPIO */
    /* Set IO bit 0, 3, 6 and 7 input, bit 1, 2, 4 and 5 output */
    if (io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }
    
    data &= ~(BIT1 | BIT2 | BIT4 | BIT5);
    data |= (BIT0 | BIT3 | BIT6 | BIT7);

    if (io_port_8bit_i2c_write(pca, CONFIGURATION_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca, CONFIGURATION_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }

#if DEBUG
    printf("\n%d,rc 8bit CONFIGURATION_REG = 0x%02x\n", __LINE__, data);
#endif 

    /* clean the inverse bit */
    mask = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    data &= ~mask;

    if (io_port_8bit_i2c_write(pca, POLARITY_INV_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x02\n");
        return (FAILED);
    }


    /* 16bit GPIO */
    /* IO port 0: bit 0 to 7 input, port 1: bit 0 t0 7 output */
    /* port 0 */
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_P0_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
    
    data |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);


    if (io_port_8bit_i2c_write(pca1, CONFIGURATION_P0_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_P0_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }

#if DEBUG
    printf("\n%d,rc 16bit CONFIGURATION_P0_REG = 0x%02x\n", __LINE__, data);
#endif 

    mask = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    data &= ~mask;

    /* clean the inverse bit */
    if (io_port_8bit_i2c_write(pca1, POLARITY_INV_P0_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x02\n");
        return (FAILED);
    }

    /* port 1 */
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_P1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }
   
    mask = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    data &= ~mask;
 
    if (io_port_8bit_i2c_write(pca1, CONFIGURATION_P1_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
        return (FAILED);
    }
 
    if (io_port_8bit_i2c_read(pca1, CONFIGURATION_P1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
        return (FAILED);
    }
#if DEBUG	
    printf("\n%d,rc 16bit CONFIGURATION_P1_REG = 0x%02x\n", __LINE__, data);
#endif

    mask = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    data &= ~mask;

    /* clean the inverse bit */
    if (io_port_8bit_i2c_write(pca1, POLARITY_INV_P1_REG, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x02\n");
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_register_test (void)
{
    int ret;

    prpass(testpass, "LTC4215 OIR Register test ");

    ret = oir_ltc4215_register_test(oir_if);
    if (ret == FAILED)
	cterr('f',0,"LTC4215 register test failed.");

    return (ret);
}


/**********************************************************************
 *
 * Function: ltc4215_reg_write
 *
 * Description: LTC4215 Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_reg_write(void)
{
    return(util_oir_ltc4215_reg_write(oir_if));
}

/**********************************************************************
 *
 * Function: ltc4215_reg_read
 *
 * Description: LTC4215 Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_reg_read(void)
{
    return(util_oir_ltc4215_reg_read(oir_if));
}

/**********************************************************************
 *
 * Function: overdrive_pwr_off
 *
 * Description: This function power off Overdrive NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
overdrive_pwr_off (void)
{
    uint8_t data = 0;

    assert(oir_if);

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(1500);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: overdrive_power_off
 *
 * Description: This function is a wrapper to power off Overdrive NGWIC.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
overdrive_power_off (void)
{
    return (overdrive_pwr_off());
}


/**********************************************************************
 *
 * Function: overdrive_pwr_on
 *
 * Description: This function power on Overdrive NGWIC.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
overdrive_pwr_on (void)
{
    uint8_t  data = 0;

    assert(overdrive_wic_iface);

    assert(oir_if);

    /* turn on board power and take I2C out of reset */
    /* jskow added to enhance power on utility */
    slot_i2c_unreset(overdrive_wic_iface, overdrive_wic_iface->slot, "WIC");
	
    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power on NGWIC module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(100);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return(FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return(FAILED);
    }
    msleep(100);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }

    /* take overdrive NGWIC out of reset */
    /* jskow added to enhance power on utility */
    overdrive_wic_iface->unreset(overdrive_wic_iface);
    
    printf("Overdrive is powered up.\n");

    return (PASSED);
}

/**********************************************************************
 * Function: reset_sata_ctrller
 *
 * Description:
 *
 * Input:  NONE. 
 *
 * Output: PASSED/FAILED.
 **********************************************************************
 */
static int
reset_sata_ctrl (void)
{
    uchar data;
    n2g_i2c_if_t *pca1;

    pca1 = &pca_i2c[1];

    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "reset SATA: Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    data &= ~BIT7;

    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT1_REG, &data) == FAILED) {
        cterr('f', 0, "reset SATA: Unable to write PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    return PASSED;

}

static int
unreset_sata_ctrl (void)
{
    uchar data;
    n2g_i2c_if_t *pca1;

    pca1 = &pca_i2c[1];

    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "unreset SATA: Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    data |= BIT7;

    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT1_REG, &data) == FAILED) {
        cterr('f', 0, "unreset SATA: Unable to write PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    return PASSED;

}

/**********************************************************************
 * Function: show_mux_mode
 *
 * Description: 
 *
 * Input:  NONE.
 *
 * Output: PASSED/FAILED.
 **********************************************************************
 */
static int 
show_mux_mode (void)
{
    uchar data;
    n2g_i2c_if_t *pca1;
    uint device_no, slot;

    slot = curr_slot;
    device_no = get_wic_device_no(slot);

    pca1 = &pca_i2c[1];

    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
   
    /* if bit 4 is set, it is controller mode */
    printf("Current mode is %s mode. \n",
         (data & BIT4) ? "Controller" : "Passive");
 
    if (skip_setting) {
        return (PASSED);
    } else {
        data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
        printf("bus 0x%x: device_no %#x: function %#x: pci reg offset 0x70=%#x\n",
               pcie_switch_bus_no, device_no, 0, data);
    }
    return (PASSED);
}

static void
pci_rdy (int mode)
{

    /* neptune with ROMMON need to set this for trigger pcie 
     * interrupt to kerenl. the storage device will show up 
     * on /dev/ 
     */
    if (is_ntpn_machines() || is_vg450()) {
        overdrive_wic_iface->pci_rdy(overdrive_wic_iface, 1);
        msleep(3000);
    }

    if (skip_setting) 
        return;
    
    if (mode == MODE_PASS) {
        overdrive_wic_iface->pci_rdy(overdrive_wic_iface, 0);
    } else {
        overdrive_wic_iface->pci_rdy(overdrive_wic_iface, 1);
    }
    
}
    
/**********************************************************************
 * Function: change_mux_mode
 *
 * Description: 
 *
 * Input:  mode - 1 : SATA Controller Mode - MODE_CTRL
 *                0 : SATA Pass-through Mode - MODE_PASS
 *
 * Output: PASSED/FAILED.
 **********************************************************************
 */
static int 
change_mux_mode (int mode)
{
    uchar data;
    n2g_i2c_if_t *pca1;
    
    pca1 = &pca_i2c[1];

    if (mode == MODE_PASS) {
        if  (overdrive_wic_iface->slot == 1 ||
            overdrive_wic_iface->slot == 2) {
            printf("PASS THRU mode not supported for slot 1 and 2\n");
            return(PASSED);
        }

    } else {
        printf("To change to controller mode, please select option 'e'.\n");
        printf("Then go back to the main menu, and re-enter Overdrive sub-menu.\n");
        printf("You may need to wait for about 15 secs. \n");
        printf("These steps are needed to witch module back to controller mode.");
        return(PASSED);
    }

    off_func(overdrive_wic_iface);
    sleep(2);  /* wait until everything is settled */
    
    /*16bit offset 4 change to ed and then fd ZZZ*/
    config_fpga(mode);

    overdrive_wic_iface->on(overdrive_wic_iface);

    overdrive_wic_iface->i2c_unreset(overdrive_wic_iface);

    msleep(1000);
    set_gpio_db_pins();

    if (reset_sata_ctrl())
        return (FAILED);

    /* when swtiching over to pass thru, linux will print out alot of messages */
    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "change mux: Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
    data &= ~BIT4;  /* pass-thru mode */
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT1_REG, &data) == FAILED) {
        cterr('f', 0, "change mux: Unable to write PCA9555 register @ 0x03\n");
        return (FAILED);
    }
        
    overdrive_wic_iface->unreset(overdrive_wic_iface);
    sleep(2); /* a delay for linux detect HDD */
    return (PASSED);
}

/**********************************************************************
 *
 * Function: gpio_exp_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int gpio_exp_write (int gpio_type)
{
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset, max_offset;

    if (gpio_type == 0) {  /* 8bit */
        pca = &pca_i2c[0];
        max_offset = 3;
    } else {   /* 16 bit */
        pca = &pca_i2c[1];
        max_offset = 7;
    }

    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, max_offset);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA register @ %#x\n", 
              __FUNCTION__,offset);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: gpio_exp_read
 * 
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 * 
 **********************************************************************
 */
static int gpio_exp_read (int gpio_type)
{
    n2g_i2c_if_t *pca;
    uchar data = 0;
    int offset, max_offset;


    if (gpio_type == 0) {  /* 8bit */
        pca = &pca_i2c[0];
        max_offset = 3;
    } else {   /* 16 bit */
        pca = &pca_i2c[1];
        max_offset = 7;
    }

    assert(pca);

    for (offset = 0; offset <= max_offset; offset++) {
        if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
            cterr('f', 0, "%s(): Unable to read PCA register @ %#x\n",
                  __FUNCTION__, offset);
            return (FAILED);
        }
        printf("\nRegister @ %#x = %#x\n", offset, data);
    }

    return (PASSED);
}


/**********************************************************************
 * Function: is_hdd_present
 *
 * Description: read io expander to check hdd is available
 *
 * Input:  hdd_num - 1 or 2  (Overdrive has two slot for HDD)
 *
 * Output:  TRUE/FALSE
 **********************************************************************
 */
static boolean
get_hdd_present (int *present)
{
    n2g_i2c_if_t *pca;
    uchar data;
    int i;

    pca = &pca_i2c[1]; /* 16 bit */

    for (i=0; i<3; i++) {
        if (io_port_8bit_i2c_read(pca, INPUT_PORT0_REG, &data, TRUE) == FAILED) {
            cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
            return (FAILED);
        }
   
        /* HDD0 is bit4, HDD1 is BIT5, bit==0 means present */
        if (data & BIT4) {
            present[0] = FALSE;
        } else {
            present[0] = TRUE;
        }
        
        if (data & BIT5) {
            present[1] = FALSE;
        } else {
            present[1] = TRUE;
        } 
        /* need BOTH to be detected before moving on (use && not ||); if not
         one of the drives maybe vacant. */
        if (present[0] && present[1])
            return(TRUE);
        sleep(1);
    }
    return(FALSE);

}

/**********************************************************************
 * Function: get_hdd_name
 *
 * Description: scan table to get hdd name from udev rule for w/r testing.
 *
 * Input:  wic_slot - current wic slot
 *         hdd_num - hdd number (overdirve has 2 slot for hdd)
 *         hdd_name - a pointer to store hdd name
 *         is_vacant - check if slot on overdirve is vacant or not
 *
 * Output:  Pass/Failed
 * NOTE:    rule : /dev/hddsd[a-b]  'a' for 1st hdd, 'b' for 2nd hdd.
 **********************************************************************
 */
static int
get_hdd_name (int wic_slot, int hdd_num, uchar *hdd_name, int is_vacant)
{
    int dev_cnt = 0, brk_cnt = 0;
    char slot_type[8];

    /* refresh device list before testing */
    system("udevtrigger");
    msleep(3000);

    /* based on rootfs script 'parse_hdd_name' and rootfs udev file:
     * /etc/udev/rules.d/11-local.roles 
     */
    if (is_sm_dc == TRUE) {
         /* sm */
         sprintf(slot_type, "%s", ngio_type_hdd[0]);
    } else {
         /* nim */
         sprintf(slot_type, "%s", ngio_type_hdd[1]);
    }

    for (brk_cnt = 0; brk_cnt < 16; brk_cnt++) {
        for (dev_cnt = 0; dev_cnt <= 10; dev_cnt++) {
            if (assigned_hdd_name[dev_cnt])
                continue;
            sprintf((char *)hdd_name, "/dev/%shdd%d%s", slot_type, wic_slot, 
                    (char *)linux_hdd_name[dev_cnt]);
            if (access((char *)hdd_name, F_OK ) != -1 ) {
                assigned_hdd_name[dev_cnt] = TRUE;
                return(PASSED);
            }  else {

            }
        }
        sleep(2);
    }
    
    return(FAILED);
}

/**********************************************************************
 * Function: get_mux_mode
 *
 * Description: read io expander to get current mode. 
 *
 * Input:  none.
 *
 * Output:  mode
 **********************************************************************
 */
static int
get_mux_mode (void)
{
    int mode;
    uchar data;
    n2g_i2c_if_t *pca1;

    pca1 = &pca_i2c[1];

    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
  
    if (data & BIT4) {
        mode = MODE_CTRL;
    } else {
        mode = MODE_PASS;
    }
    return (mode);
}

static int 
config_ngio_sata_mux (int mode)
{
    uchar data;
    n2g_i2c_if_t *pca1;

    pca1 = &pca_i2c[1];

    if (mode == MODE_PASS) {
        data &= ~BIT4;  /* pass-thru mode */
    } else {
        data |= BIT4;   /* ctrl mode */
    }

    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "change mux: Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
    printf("before switching, OUTPUT_PORT1_REG = %#x\n", data);
    
    if (io_port_8bit_i2c_write(pca1, OUTPUT_PORT1_REG, &data) == FAILED) {
        cterr('f', 0, "change mux: Unable to write PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca1, OUTPUT_PORT1_REG, &data, TRUE) == FAILED) {
        cterr('f', 0, "change mux: Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }
    printf("after switching, OUTPUT_PORT1_REG = %#x\n", data);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: config_fpga
 *
 * Description: we have different fpga setting which is decided by
 *              different mode.
 *
 * Input:  mode - current mode that user want to test.
 *
 * Output: NONE.
 *
 **********************************************************************
 */
static void config_fpga (int mode)  
{
    boolean is_passive;  

    if (mode == MODE_PASS)
       is_passive = TRUE;
    else 
       is_passive = FALSE;

    sata_cfg(is_passive);
    
    return; 
}

/**********************************************************************
 *
 * Function: hdd_test
 *
 * Description: change to pass-through mode and r/w SATA.
 *
 * Input:  mode - 1 : SATA Controller Mode - MODE_CTRL
 *                0 : SATA Pass-through Mode - MODE_PASS
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
hdd_test (int mode)
{
    int ret = FAILED;
    int hdd_num;
    int present[OVERDRIVE_SATA_NUM];
    int slot = 0;
    char hdd_name[16];

    slot = curr_slot;

    if (mode == MODE_PASS) {
        testname("Overdrive Pass-through Mode");
    } else {
        testname("Overdrive Controller Mode");
        show_pci_speed();
    }

    prpass(testpass, "slot %d ", slot);

    if (mode == MODE_PASS)
        slot = 4;

    memset(present, 0, sizeof(present));
    memset(assigned_hdd_name, 0, sizeof(assigned_hdd_name));
    get_hdd_present(present);
            
    for (hdd_num = 0; hdd_num < OVERDRIVE_SATA_NUM; hdd_num++) {
        /* check if sata present */
        if (present[hdd_num]) {
            hdd_name[0] = '\0';
            ret = get_hdd_name(slot, hdd_num, (uchar *)hdd_name, 0);
            if (ret == FAILED) {
                /* give warning if detect only one hdd */
                cterr('f',0,"Linux cannot detect harddrive%d.", hdd_num);
                return(FAILED);
            }

            prpass(testpass, "HDD%d (%s) ", hdd_num, hdd_name);
            ret = sata_tests((uchar *)hdd_name);
            if (ret == FAILED) {
                cterr('f',0,"sata %s test failed.", hdd_name);
                return(FAILED);
            } else {

            }

        } else {
            printf("Overdrive supports upto 2 physical drives. ");
            printf("One physical drive dectected in this slot.\n");
        }
    }

    if ((present[0] == 0) && (present[1] == 0)) {
        cterr('f', 0, "No harddrives found in this module.");
        return (FAILED);
    }
    return(PASSED);
}

/**********************************************************************
 *
 * Function: show_speed
 *
 * Description: show pci speed on overdirve.
 *
 * Input : None.
 *
 * Output: None
 *
 * Note: the bus num can be found using 'lspci -tv' cmd on linux. 
 **********************************************************************
 */
static void
show_pci_speed(void)
{
    int device_no, data, slot;
    
    slot = curr_slot;
    if(skip_setting) {
        return;
    } else {
        device_no = get_wic_device_no(slot);
        data = pci_config_read(pcie_switch_bus_no, device_no, 0, 0x70);
        if (data & 1) 
            prpass(testpass, "Current speed on IDT is Gen1 (PCI offset 0x70=%#x)",
                 data);
        else
            printf("IDT PCI speed is not Gen1, Offset 0x70=%#x\n", data);
    }

    return;
}

/**********************************************************************
 *
 * Function: show_info
 *
 * Description: show info for current slot. 
 *
 * Input : None.
 *
 * Output: None
 *
 **********************************************************************
 */
static void
show_info(void)
{
    printf("-----------------------\n");
    printf("HDD name and alias name\n");
    system("ls /dev/s*");
    system("ls -al /dev/hdd*sd*");

    printf("-----------------------\n");
    show_mux_mode();
    
    printf("-----------------------\n");

    if (if_rdy()) {
        printf("primary interface ready bit is set.\n");
    } else {
        printf("primary interface ready bit not set. \n");
    }

    printf("-----------------------\n");
    show_pci_speed();

    return;
}

/**********************************************************************
 *
 * Function:  toggle_led
 *
 * Description: toggle led on specific bit on io expander
 *
 * Input : led_sel - correspond to the bit on ioexpander
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
toggle_led(int led_sel)
{
    n2g_i2c_if_t *pca;
    uchar mask = 1, led_stat;

    /* 16bit io expander */
    pca = &pca_i2c[1];

    if (io_port_8bit_i2c_read(pca, OUTPUT_PORT1_REG, &led_stat, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9555 register @ 0x03\n");
        return (FAILED);
    }

    /* bit0: hdd0G, bit1:hdd0A, bit2:hdd1G, bit3:hdd1A */
    mask = mask << led_sel;

    if (led_stat & mask) {
        led_stat &= ~mask;
    } else {
        led_stat |=  mask;
    }

    if (io_port_8bit_i2c_write(pca, OUTPUT_PORT1_REG, &led_stat) == FAILED) {
        cterr('f', 0, "%s(): Unable to write PCA register OUTPUT_PORT1_REG\n",
              __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: led_util
 *
 * Description: show pci speed on overdirve.
 *
 * Input : None.
 *
 * Output: None
 *
 **********************************************************************
 */
static void
led_util(void)
{
    uchar choice;

    while (1) {
        printf("\nToggle LED utility:\n");
        printf("  0. HDD0 status Green\n");
        printf("  1. HDD0 status Amber\n");
        printf("  2. HDD1 status Green\n");
        printf("  3. HDD1 status Amber\n");
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 0, 0, 4);

        if (choice == 4)
            return;

        if(toggle_led(choice))
            return;

        continue;
    }

    return;
}


static int
install_driver (int dummy)
{
//    system("rmmod pciehp");
//    system("modprobe pciehp pciehp_force=1");
    system("echo 1 > /sys/bus/pci/devices/0000:00:01.1/remove");
    system("echo 1 > /sys/bus/pci/devices/0000:00:04.0/remove");
    system("echo 1 > /sys/bus/pci/rescan"); 
    return(PASSED);

}

static int
if_rdy (void)
{
     uchar data;
     n2g_i2c_if_t *pca;  /*8 bit */

     pca = &pca_i2c[0];

     if (io_port_8bit_i2c_read(pca, INPUT_PORT_REG, &data, TRUE) == FAILED) {
         printf("if_rdy: Unable to read PCA9555 register @ 0x03\n");
         return (FAILED);
     }

     if (data & BIT3) {
         return TRUE;
     }
     return FALSE;

}

/**********************************************************************
 *
 * Function: is_pass_throu_available
 *
 * Description: a function wrapper to check whether p
 *              ass through mode is avaiable
 *
 * Input:  none
 *
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
boolean is_pass_throu_available (void) {

    /* pass thru mode is obseleted and not support anymore */
    return (FALSE);
}

/******** History ********
$Log: ngwic_overdrive.c,v $
Revision 1.45  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.44  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.43  2016/06/16 11:56:23  alpeng
hide pass-through mode items, since it is obseleted

Revision 1.42.14.8  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.42.14.7  2017/11/27 05:59:42  leschen
Initial check in to support VG450.

Revision 1.42.14.6  2017/07/28 09:09:41  alpeng
Neptune is able to power on/off overdrive HDD properly

Revision 1.42.14.5  2017/07/19 03:51:36  alpeng
add pcie ready for neptune

Revision 1.42.14.4  2017/04/17 10:10:26  alpeng
change is_nep to is_nptn

Revision 1.42.14.3  2017/04/05 06:40:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.42.14.2  2016/12/14 08:58:15  alpeng
supporting neptune, update log

Revision 1.42.14.1  2016/06/16 11:57:37  alpeng
hide pass-through mode items, since it is obseleted

Revision 1.44  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.43  2016/06/16 11:56:23  alpeng
hide pass-through mode items, since it is obseleted

Revision 1.42  2015/07/02 16:00:02  jskow
update power on to enhance power utilities

Revision 1.41  2014/10/23 09:16:43  alpeng
support thule, rename hdd prefix with sm and nim

Revision 1.40  2014/08/26 08:48:42  bowang3
Make NIM support NGSM carrier card Thule

Revision 1.39  2014/05/20 03:48:09  alpeng
removing mutiple delay

Revision 1.38  2014/05/02 18:35:00  mcharon
change string FPGA does not detect one of the drives. new string does not imply error

Revision 1.37  2013/12/18 06:32:46  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.36  2013/12/12 06:42:03  alpeng
pci bus changed for supporting new rommon

Revision 1.35  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.34  2013/11/20 02:13:08  alpeng
skip pcie setting on sword and dagger, both of them are PLX pcie switch

Revision 1.33  2013/11/14 02:26:12  alpeng
remove pass through mode on non-overlord platform

Revision 1.32  2013/11/07 02:27:28  alpeng
Overdrive is not support pass through mode on utah

Revision 1.31  2013/11/01 04:45:38  alpeng
utah need a delay before hdd is up

Revision 1.30  2013/10/28 09:44:33  alpeng
support pci bus for utah

Revision 1.29  2013/10/08 00:25:05  alpeng
pass throught mode is not support on Juno PLX ver.

Revision 1.28  2013/10/07 03:40:13  alpeng
introducing a flag early_unreset for ngio to put reset state on early stage

Revision 1.27  2013/08/21 22:40:17  mcharon
move overdrive fix up code to main.c..other platform will not need fix up code

Revision 1.26  2013/05/22 20:04:31  mcharon
make 'power off module' option part of dogrp test

Revision 1.25  2013/05/01 20:43:23  mcharon
put overdrive in Gen1 in controller mode

Revision 1.24  2013/04/10 00:46:53  alpeng
support debug item to check primary interface status

Revision 1.23  2013/04/02 19:19:16  mcharon
don't need to clear pci ready bit in change_mux_mode. it's done from calling function

Revision 1.22  2013/03/29 05:51:10  mcharon
set pci rd bit for controller mode. unset the bit for pass thru mode

Revision 1.21  2013/02/28 17:50:04  mcharon
support pass-thru mode with reload of pciehp module

Revision 1.20  2013/02/27 22:36:22  mcharon
support controller mode without reseting platform

Revision 1.19  2012/11/29 09:50:43  alpeng
remove query, return pass directly

Revision 1.18  2012/11/28 07:48:49  alpeng
query user if they want to run SATA mode test on slot1 and slot2

Revision 1.17  2012/11/21 09:07:54  alpeng
extend the delay time after changing mode

Revision 1.16  2012/11/20 01:11:40  alpeng
an utility which is integrate show info for current slot

Revision 1.15  2012/11/15 15:43:25  alpeng
add function wqprologue

Revision 1.14  2012/11/15 15:41:02  alpeng
support 8 bytes data w/r on data

Revision 1.13  2012/11/14 09:52:58  alpeng
support hdd led status util

Revision 1.12  2012/11/13 03:45:02  alpeng
break while loop if there is no capability can be read

Revision 1.11  2012/11/12 20:35:23  mcharon
add third arg to slot_i2c_unrest to report slot num when fails..improve err reporting

Revision 1.10  2012/11/12 09:59:53  alpeng
support show pcie speed on overdrive

Revision 1.9  2012/11/06 09:48:05  alpeng
add menu item to trun on/off ngwic_disable(), mask ngwic_disable() when enter overdirve menu

Revision 1.8  2012/11/05 08:44:54  alpeng
clean up debug msg.

Revision 1.7  2012/11/02 16:06:29  alpeng
support HDD test on overdirve, add some diag item for manufacturing using

Revision 1.6  2012/11/01 09:38:40  alpeng
support SATA r/w on both modes of overdirve

Revision 1.5  2012/10/24 08:58:08  alpeng
release reset bit after reset. using warning instead of fatal when device is vacant

Revision 1.4  2012/10/23 08:02:37  alpeng
supported HDD test on overdrive

Revision 1.3  2012/10/18 10:34:53  alpeng
removing check OIR statis reg after power off wic

Revision 1.2  2012/10/18 10:15:24  alpeng
suppoer 16bit pca9555 for overdrive

Revision 1.1  2012/10/17 10:39:34  alpeng
first check in to support overdrive


$Endlog$
*/

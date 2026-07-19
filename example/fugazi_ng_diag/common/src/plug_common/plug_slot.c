/* $Id: plug_slot.c,v 1.11 2021/09/24 01:27:20 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_slot.c,v $
 *------------------------------------------------------------------
 *
 * plug_slot.c - PLUGGABLE Slot related functions 
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "nvmonvars.h"
#include "proto.h"
#include "cross_platform.h"
#include "plug_host_fpga_lib.h"
#include "plug_host_slot_modules.h"
#include "plug_slot.h"
#include "plug_lte_telit_test.h"
#include "plug_common_host_impl.h"
#include "plug_common_lib.h"
#include "cookie_4.h"
#include "plug_testcard_test.h"

static struct plug_intf_t plug[FIRST_SLOT + MAX_PLUG_SLOT_NUMBER];

static int plug_slot_enable(void *);
int plug_slot_i2c_reset(void *);
int plug_slot_i2c_unreset(void *);
static void plug_slot_disable(void *);
static int plug_slot_get_real_slot(int);
static int plug_slot_test(struct plug_intf_t *, int, int, int);
static int plug_slot_get_info(struct plug_intf_t *);
static int plug_slot_get_id(void *, char *);
static int plug_slot_enable_uart(void *);
static int plug_slot_disable_uart(void *);
static int plug_is_present(void *);
static int plug_slot_cookie_info_get(struct plug_intf_t *, int);
static int plug_module_poweroff_nontest_slot(int);
int plug_slot_i2c_poweron_unreset(struct plug_intf_t *, int, char *);
static boolean plug_common_plug_is_present(uint);
static boolean plug_common_plug_is_pwr_ok(uint);
static int plug_slot_i2c_ctrl(int);

extern void reset_errmsg_var(void);

void init_plug_info(void);
int plug_test(int);
int plug_intf_test(int);
int plug_is_pwr_ok(void *);
void print_pim_slots(void);
int plug_real_test_slot = PLUG_SLOT_1;

int plug_curr_i2c_ctrl;
extern plug_module_sku_info plug_module_sku_tbl[];
extern void plug_common_host_pcie_dev_disable(int);
extern void plug_pci_dev_remove(int);
static struct timeval plug_pwr_off_t={0,0};

/*-------------------------------------------------------------------
 *
 * Function : plug_pci_dev_remove
 * Description: a weak function for remove plug pci device on plat. 
 *
 * INPUT:  None
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
void plug_pci_dev_remove (int slot) 
    __attribute__((weak, alias("__plug_pci_dev_remove")));
void __plug_pci_dev_remove (int slot) 
{
    return;
}

/*-------------------------------------------------------------------
 *
 * Function : plug_lte_modem_pwr_down_seq
 * Description: a weak function for LTE modem power down
 *
 * INPUT:  plug_ptr - pointer to pluggable structure.
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
int plug_lte_modem_pwr_down_seq (struct plug_intf_t *)
    __attribute__((weak, alias("__plug_lte_modem_pwr_down_seq")));
int __plug_lte_modem_pwr_down_seq (struct plug_intf_t *plugp)
{
    return (FAILED); 
}

/*-------------------------------------------------------------------
 *
 * Function : plug_lte_telit_modem_pwr_down_seq
 * Description: a weak function for Telit LTE modem power down
 *
 * INPUT:  plug_ptr - pointer to pluggable structure.
 * OUTPUT: None
 * -------------------------------------------------------------------
 */
int plug_lte_telit_modem_pwr_down_seq (struct plug_intf_t *)
    __attribute__((weak, alias("__plug_lte_telit_modem_pwr_down_seq")));
int __plug_lte_telit_modem_pwr_down_seq (struct plug_intf_t *plugp)
{
    return (FAILED); 
}

/*******************************************************************************
 * Function   : init_plug_info
 * Description: Function to initialize internal data structure of pluggable
 *              modules
 *              (This function must be called during application init)
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void init_plug_info (void)
{
    int ix;

    for (ix = FIRST_SLOT; ix < MAX_PLUG_SLOT_NUMBER + FIRST_SLOT; ix++) {
        memset(&plug[ix], 0, sizeof(struct plug_intf_t));
        plug[ix].slot = ix;
        plug[ix].is_present = plug_is_present;
        plug[ix].on = plug_slot_enable;
        plug[ix].off = plug_slot_disable;
        plug[ix].i2c_reset = plug_slot_i2c_reset;
        plug[ix].i2c_unreset = plug_slot_i2c_unreset;
        plug[ix].reset = plug_slot_reset;
        plug[ix].unreset = plug_slot_unreset;
        plug[ix].uart_on = plug_slot_enable_uart;
        plug[ix].uart_off = plug_slot_disable_uart;
        plug[ix].uart_ctrl = PLUG_UART_CTRL(ix); 
        plug[ix].get_id = plug_slot_get_id;
        plug[ix].i2c_ctrl = plug_slot_i2c_ctrl(ix);
    }
}

/*-------------------------------------------------------------------
 * Function : get_max_pim_slots
 * Description: return max number of PIM slots
 * INPUT:  None
 * OUTPUT: max number of PIM slots
 * -------------------------------------------------------------------
*/
int get_max_pim_slots(void)
{
    return (plug_common_host_get_max_plug_slots());
}

/*-------------------------------------------------------------------
 * Function : print_pim_slots
 * Description: Print PIM slot info. 
 *              (Cookie ID, Serial Number, Board Revision)
 * INPUT:  None
 * OUTPUT: None
 * -------------------------------------------------------------------
*/
void print_pim_slots (void)
{
    int slotnum, max_slot_num = plug_common_host_get_max_plug_slots();
    unsigned short cookie_id = 0;
    plug_module_sku_info *plug_tbl_p = plug_module_sku_tbl;
    
    for (slotnum = FIRST_SLOT; slotnum <= max_slot_num; slotnum++) {
        cookie_id = plug_cookie_get(slotnum);

        if (plug[slotnum].id == PLUG_SLOT_VAC_ID) {
            continue;
        }

        /* Search match platform cookie table */
        while (plug_tbl_p->plug_module_name != NULL) {
            if (plug_tbl_p->cook_contype == cookie_id) {
                printf("PIM%1d: %s cookie id = 0x%4x, ",
                       slotnum, plug_tbl_p->plug_module_name, cookie_id);
                break;
            }
            plug_tbl_p++;
        }
        if (plug_tbl_p->plug_module_name == NULL) {
            printf("*** WARNING: Could not find correct PLUG module SKU info.\n");
        }

        printf("SN=%s, Rev=%c%c\n\n", plug[slotnum].serial_num, 
              (plug[slotnum].bd_rev)>>8, (plug[slotnum].bd_rev)&0xff);
        /*Graceful shutdown LTE modem before powering down the pluggable module */
        if ((cookie_id == PLUGGABLE_LTE_EM) ||
            (cookie_id == PLUGGABLE_LTE_WP7601) ||
            (cookie_id == PLUGGABLE_LTE_WP7603) ||
            (cookie_id == PLUGGABLE_LTE_WP7607) ||
            (cookie_id == PLUGGABLE_LTE_WP7608) ||
            (cookie_id == PLUGGABLE_LTE_WP7609) ||
            (cookie_id == PLUGGABLE_LTE_WP7610)) {
            plug_curr_i2c_ctrl = plug[slotnum].i2c_ctrl;
            plug_lte_modem_pwr_down_seq(&plug[slotnum]);
        } else if (cookie_id == PLUGGABLE_LTE_TELIT_LM9x0) {
            plug_curr_i2c_ctrl = plug[slotnum].i2c_ctrl;
            plug_lte_telit_modem_pwr_down_seq(&plug[slotnum]);
        }
        msleep(PLUG_PWR_OFF_DELAY);
        plug_module_power_off(slotnum);
    }

}

/*-------------------------------------------------------------------
 * Function : plug_test
 * Description: Main Entry function of Pluggable Slot Test
 * INPUT:  slot - Slot number (Starting from 1)
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
int plug_test (int slot)
{
    int real_slot = plug_slot_get_real_slot(slot);

    plug_real_test_slot = real_slot;

    return (plug_slot_test(&plug[real_slot], real_slot, slot, FULL_TEST));
}


/*-------------------------------------------------------------------
 * Function : plug_intf_test
 * Description: Main Entry function of Pluggable Slot Interface Test
 * INPUT:  slot - Slot number (Starting from 1)
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
int plug_intf_test (int slot)
{
    int real_slot = plug_slot_get_real_slot(slot);

    plug_real_test_slot = real_slot;
 
    return (plug_slot_test(&plug[real_slot], real_slot, slot, IFACE_TEST));
}


/*******************************************************************************
 * Function   : plug_host_get_slot_module
 * Description: This function essentially goes through port_module_tbl table and
 *              return module data based on cookie id
 * Inputs     : id - Cookie ID
 * Outputs    : Pointer of Plug Module Entry or Null if Cookie ID is unrecognized
 *
 *******************************************************************************
 */
struct plug_module_info *plug_host_get_slot_module (unsigned short id)
{
    int ix;

    for (ix = 0; ix < MAX_MOD_IDS; ix++) {
        if (plug_host_module_tbl[ix].id == id) {
            return (struct plug_module_info *)(&plug_host_module_tbl[ix]);
        }
    }
    return (struct plug_module_info *)(NULL);
}


/*-------------------------------------------------------------------
 * Function : plug_module_power_off
 * Description: Turn off Pluggable Slot module power
 * INPUT:  slot
 * OUTPUT: None
 * -------------------------------------------------------------------
*/
void plug_module_power_off (int slot)
{
    struct plug_intf_t *plug_p;

    plug_p = &plug[slot];
    plug_p->off(plug_p);
    msleep(PLUG_PWR_OFF_DELAY);

}


/*-------------------------------------------------------------------
 * Function : plug_cookie_get
 * Description: Get Pluggable Slot Cookie
 * INPUT:  slot - Slot number (Starting from 1)
 * OUTPUT: plug slot Cookie id
 * -------------------------------------------------------------------
*/
unsigned short plug_cookie_get (int slot)
{
    int real_slot = plug_slot_get_real_slot(slot);

    if (plug_slot_cookie_info_get(&plug[real_slot], real_slot)) {
        return (PLUG_SLOT_VAC_ID); 
    } else {
        return (plug[real_slot].id);
    }
}


/*-------------------------------------------------------------------
 * Function : plug_slot_cookie_info_get
 * Description: to get pluggable slot cookie info
 * INPUT: slot - Slot number
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int plug_slot_cookie_info_get (struct plug_intf_t *plug, int slot)
{

    if (plug_slot_get_info(plug) == FAILED) {
        prcomplete(testpass, errcount, 0);
        return (FAILED);
    }

    return (PASSED);
}


/*-------------------------------------------------------------------
 * Function : plug_module_poweroff_nontest_slot
 * Description: Power off non-test plug module 
 * INPUT: test_slot - test slot number 
 * OUTPUT: None
 * -------------------------------------------------------------------
*/
static int plug_module_poweroff_nontest_slot (int test_slot)
{
    uint ix, slot_num = 0;

    slot_num = plug_common_host_get_max_plug_slots();
    for (ix = PLUG_SLOT_1; ix <= slot_num; ix++) {
        if (ix != test_slot) {
            plug_module_power_off(ix);
        }
    }
	return (PASSED);
}

/*-------------------------------------------------------------------
 * Function : plug_slot_test
 * Description: Invoke the diagnostic for the pluggable slot test
 * INPUT: plug - ptr to struct plug_intf_t 
 *        real_slot - Slot number
 *        slot - Slot number provided by menu
 *        test_type - Full test or interface test
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int plug_slot_test (struct plug_intf_t *plug, int real_slot, 
                           int slot, int test_type)
{
    int test_err;
    char mod_str[] = PLUG_MOD_STR; 
    struct timeval t_curr = {0,0};
    int t_diff = 0;
    
    if (real_slot != slot ) { /* User opted for submenus */
        plug->menu_display = TRUE;
    } else {
        plug->menu_display = FALSE;
    }

    /* CDETS: CSCvu45403 
     * Dying gasp needs at least 12 seconds for Hyperloop to fully discharge.*/
    if ((plug->id == PLUGGABLE_LTE_TELIT_LM9x0) || (plug->id == PLUGGABLE_NR_5G_TELIT_FN980)) {
        gettimeofday(&t_curr, NULL);
        t_diff = (t_curr.tv_sec - plug_pwr_off_t.tv_sec);
        /* Wait at least 15 seconds between module power-off and power-on */
        if (t_diff < PLUG_LTE_TELIT_MIN_ACTIVE_SEC) {
            printf("Wait %d sec to power-off...\n", 
                    (PLUG_LTE_TELIT_MIN_ACTIVE_SEC - t_diff));
            sleep (PLUG_LTE_TELIT_MIN_ACTIVE_SEC - t_diff);
        }
    }

    /* power off non-test module to ensure no side effect when testing */ 
    plug_module_poweroff_nontest_slot(real_slot);

    /* Clear the test's cterr info setup before each slot test */
    reset_errmsg_var();
    testname("%s Slot %1d", mod_str,  real_slot);
    prpass(testpass, " "); /* Zero out the testpass buffer */

    if (plug_slot_get_info(plug) == FAILED) {
        prcomplete(testpass, errcount, 0);
        return (FAILED);
    }

    plug_curr_i2c_ctrl = plug->i2c_ctrl;

    plug->test_type = test_type;

    /* Invoke the diagnostic */
    if (test_type == FULL_TEST) {
        test_err = plug->diag((void *)plug);
    } else {
        if (plug->intf_diag) {
            test_err = plug->intf_diag((void *)plug);
        } else {
            test_err = PASSED;
            cterr('w', 0, "No interface test available.");
        }
    }

    if (slot != real_slot) {
        printf("\n%s Subtest Menu accumulated errors = %ld",
               plug->name, err_accum);
    }

    if (test_err == FAILED) {
        return (test_err);
    } 

    /* Not sure if we need to turn off this */
    if (!((NVRAM)->diagflag & D_POWER_ON)) {
        if (plug->off) {
            /*Add for complete testing log without USB interrupt*/
            msleep(PLUG_PWR_OFF_DELAY);
            plug->off(plug);

            /* Setup delay time to meet the power on/off sequence */
            gettimeofday(&plug_pwr_off_t, NULL);
            
            msleep(PLUG_PWR_OFF_DELAY);
        }
    }

    if (plug->id == PLUGGABLE_PCIE_TEST_CARD) {
        if (plug_testcard_pcie_post_pwr_down() == FAILED) {
            cterr('f', 0, "Workaround with PCIE power down fail");
            return (FAILED);
        }
    }

    return (test_err);
}

/*-------------------------------------------------------------------
 * Function : plug_slot_get_info
 * Description: This function essentially checks whether the module
 *              is detected and then read controller type in ACT2
 *              cookie
 * INPUT: plug - ptr to struct plug_intf_t 
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int plug_slot_get_info (struct plug_intf_t *plug)
{
    int slot;
    char type[] = PLUG_MOD_STR;
    int retry;
    char err[80];
    struct plug_module_info *plug_module_ptr;
    int status;

    /* Sanity check */
    if (plug == NULL) {
        printf("%s: Null pointer\n", __func__);
        return (FAILED);
    }

    /* 
     * to leave a blank line between network modules
     * polling print and this polling print
     */
    printf("\n");

    slot = plug->slot;

    if (!plug->is_present((void *)plug)) {
        printf("%s Slot %1d: (slot vacant)\n", type, slot);
        plug->id = PLUG_SLOT_VAC_ID;
        sprintf((char *)plug->name, "(slot vacant)");
        return (FAILED);
    } else {
        sprintf((char *)plug->name, "%s Slot %1d: id=?\n", type, slot);
    }

    /* Get ID from cookie */
    for (retry = 0, *err = '\0'; retry < PLUG_SLOT_READ_ID_RETRY; retry++) {
        /* ensure USB hub be off reset when plug power on */
        plug_common_host_usb_hub_reset(DISABLE);
        if (plug->on((void*)plug) == FAILED) {
            return (FAILED);
        }
        if (plug->i2c_unreset((void*)plug) == FAILED) {
            return (FAILED);
        }
        if ((status = plug->get_id((void *)plug, err)) == FAILED) {
            if (plug->off) {
                plug->off(plug);
            }
            sleep(PLUG_SLOT_READ_ID_DELAY_IN_SEC);
            continue;
        }
        break;
    }

    if (status == FAILED) {
        cterr('f', 0, "unable to read cookie: %s", err);
        return (FAILED);
    }

    if (plug->id == PLUG_SLOT_INVALID_ID) {
        cterr('f', 0, "%s %d: invalid cookie %#x. Is id programmed ?",
                      type, slot, plug->id);
        return (FAILED);
    }

    plug_slot_get_bd_revision(plug->cookie, &plug->bd_rev);
    plug_slot_get_pcb_serial(plug->cookie, plug->serial_num);

    /* Now retrieve module information based on ID */
    plug_module_ptr = plug_host_get_slot_module(plug->id);
    if (plug_module_ptr == NULL) {
        cterr('f', 0, "Slot %d. Card not supported. cookie id = %#x.", 
                       slot, plug->id);
        return (FAILED);
    }

    /* disable host PCIe root port to prevent PCIe link failure
     * for the platform which has PCIe but PIM test-card does not.
     */
    if (plug->id == PLUGGABLE_TEST_CARD) {
        plug_common_host_pcie_dev_disable(slot);
    }

    plug->diag = plug_module_ptr->diag;
    plug->intf_diag = plug_module_ptr->intf_diag;
    sprintf((char *)plug->name, (char *)plug_module_ptr->name);

    return (PASSED);
}


/*-------------------------------------------------------------------
 * Function : plug_slot_enable
 * Description: Enable Pluggable Modules
 * INPUT:  ptr to struct plug_intf_t 
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int plug_slot_enable (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot, ix = 0;
    uint ctrl_reg; 
    uint buf;

    slot = intf->slot; 
    /* Assign register address based on slot number */
    ctrl_reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    /* make sure ctrl reg is back in its default state before
       taking powering module */
    plug_common_host_plug_fpga_reg_read(ctrl_reg, &buf);
    buf &= ~(PLUG_UART_TX | PLUG_I2C_RESET | PLUG_RESET); 
    plug_common_host_plug_fpga_reg_write(ctrl_reg, buf);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, ctrl_reg, PLUG_PWR_EN_BIT);

    for (ix = 0; ix < PLUG_PWR_OK_TIMEOUT; ix++) {
        msleep(PLUG_PWR_EN_DELAY);
        plug_common_host_plug_fpga_reg_read(ctrl_reg, &buf);
        if (buf & PLUG_PWR_OK) {
            plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, ctrl_reg, PLUG_PWR_OK_FLT_INTR_BIT);

            plug_common_host_plug_fpga_reg_read(ctrl_reg, &buf);
            if (buf & PLUG_PWR_EN) {
                plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, ctrl_reg, PLUG_PWR_OK_FLT_INTR_BIT);
            }

            msleep(PLUG_PWR_AFTER_EN_DELAY);

            /* power fault intr enable, insert/removal intr already on */
            return (PASSED);
        }
    }

    printf("plug status register reports module power is not up.");
    printf("PLUG_PWR_EN bit4 is not set.\n");
    
    return (FAILED);
}


/*-------------------------------------------------------------------
 *
 * Function : plug_slot_disable
 * Description: power down PLUG slot
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
static void plug_slot_disable (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot;
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    /* HW suggest to add work around for curie 1RU to prevent kernel 
     * panic. 'disable pim' -> 'pci read' -> crashed '
     */
    plug_pci_dev_remove(slot); 

    /* HW suggest to assert reset signal before turning off PIM module power(CSCvs11330) */
    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_RESET_BIT);
    msleep(PLUG_RESET_DELAY);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_PWR_EN_BIT);
    
}


/*-------------------------------------------------------------------
 * Function : plug_slot_reset
 * Description: Reset plug slot
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int plug_slot_reset (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot; 
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_RESET_BIT);

    return (PASSED);
}

/*-------------------------------------------------------------------
 * Function : plug_slot_unreset
 * Description: Unreset plug slot
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int plug_slot_unreset (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot; 
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_RESET_BIT);

    return (PASSED);
}


/*-------------------------------------------------------------------
 * Function : plug_slot_enable_uart
 * Description: Enable TX UART
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
static int plug_slot_enable_uart (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot; 
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_UART_TX_EN_BIT);

    return (PASSED);
}


/*-------------------------------------------------------------------
 * Function : plug_slot_disable_uart
 * Description: Disable TX UART
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
static int plug_slot_disable_uart (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot; 
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_UART_TX_EN_BIT);

    return (PASSED);
}

/*-------------------------------------------------------------------
 * Function : plug_is_present
 * Description: Checks whether pluggable module is present or not
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: TRUE or FALSE
 * ------------------------------------------------------------------
*/
static int plug_is_present (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;

    return (plug_common_plug_is_present(intf->slot));
}

/*-------------------------------------------------------------------
 * Function : plug_is_pwr_ok
 * Description: Checks whether pluggable module is power ok or not
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: TRUE or FALSE
 * ------------------------------------------------------------------
*/
int plug_is_pwr_ok(void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;

    return (plug_common_plug_is_pwr_ok(intf->slot));
}

/*-------------------------------------------------------------------
 * Function : plug_slot_i2c_reset
 * Description: reset i2c on plug slot
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int plug_slot_i2c_reset (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot; 
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_ON, reg, PLUG_I2C_RESET_BIT);

    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function : plug_slot_i2c_unrset
 * Description: unreset i2c on plug slot
 * INPUT:  p -- pointer to struct plug_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int plug_slot_i2c_unreset (void *p)
{
    struct plug_intf_t *intf = (struct plug_intf_t *)p;
    int slot;
    uint reg;

    slot = intf->slot;

    reg = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot);

    plug_common_host_diag_fpga_reg_bitops(FPGA_BIT_OPS_OFF, reg, PLUG_I2C_RESET_BIT);

    return (PASSED);    
}


/*-------------------------------------------------------------------
 * Function : plug_slot_get_real_slot
 * Description: This function returns the real slot number based on
 *              slot number provided by menu which may be incremented
 *              by constant to distinguish a submenu entry from main
 *              menu entry
 * INPUT: slot - Slot number provided by menu 
 * OUTPUT: Real slot number
 * -------------------------------------------------------------------
*/
static int plug_slot_get_real_slot (int slot)
{
    int real_slot;

    real_slot = (slot <= MAX_PLUG_SLOT_NUMBER) ? slot : slot - MAX_PLUG_SLOT_NUMBER;

    return (real_slot);    
}


/*-------------------------------------------------------------------
 * Function : plug_slot_get_id
 * Description: This function reads controller type from cookie and
 *              store its ID into pluggable data structure
 * INPUT: *plug_ptr - Pointer to Pluggable data structure
 *        *err      - Pointer to error buffer
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int plug_slot_get_id (void *plug_ptr, char *err)
{
    struct plug_intf_t *plug = (struct plug_intf_t *)plug_ptr;
    unsigned int slot;
    uint16_t id;

    slot = plug->slot;

    if (plug_common_host_get_cookie_id(slot, PLUGGABLE_CARD, plug->cookie,
                      &id, err) == FAILED) {
        return (FAILED);
    }
    plug->id = id;

    return (PASSED);

}

/*-------------------------------------------------------------------
 * Function : slot_get_plugslot
 * Description: This function return plug slot structure point
 * INPUT: slot - plug slot
 * OUTPUT: slot matrix point
 * -------------------------------------------------------------------
*/
struct plug_intf_t *slot_get_plugslot (int slot)
{
    if (slot < FIRST_SLOT || slot > MAX_PLUG_SLOT_NUMBER) {
        assert(!" invlid slot in slot_get_plugslot\n");
    }

    return (&plug[slot]);

}

/*-----------------------------------------------------------------------------
 *  
 *  Function plug_slot_i2c_poweron_unreset
 *
 *  This function will reset plug i2c.
 *  
 *  Inputs : plug -  plug structure point.
 *           slot -  plug slot number.
 *           type -  slot type.
 *  
 *  Returns : PASSED or FAILED
 **/
int plug_slot_i2c_poweron_unreset (struct plug_intf_t *plug, int slot, char *type)
{
    int err;

    /* need to check for slot presense because this routine is also called by cookie
    *        utility */
    if (!plug->is_present(plug)) {
        printf("Vacant slot");
        return (FAILED);
    }   

    if ((err = plug->on(plug)) < 0) {
        printf("Unable to power module %s%d\n", type, slot);
        return (FAILED);
    }   

    if ((err = plug->i2c_unreset(plug)) < 0) {
        printf("Unable to unreset i2c on %s%d\n", type, slot);
        return (FAILED);
    }   
              
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_common_plug_is_present 
 * Description: Function to check PLUG module is present or not.
 * Inputs     : slot_num - Pluggable slot number (Start from 1)
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
boolean plug_common_plug_is_present (uint slot_num)
{
    uint reg_addr;
    uint reg_val = 0;
    int max_slot_num = plug_common_host_get_max_plug_slots();

    /* Sanity check whether slot_num exceeds maximum number of slot */
    if (slot_num > max_slot_num) {
        printf("%s: Slot number (%d) exceeds maximum slot number (%d)\n", 
               __func__, slot_num, max_slot_num);
        return (FALSE);
    }

    /* Calculate Register address based on slot number */
    reg_addr = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot_num);

    if (plug_common_host_plug_fpga_reg_read(reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read Pluggable FPGA Control/Status Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FALSE);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: PLUG FPGA @0x%04X: 0x%08X.\n", __FUNCTION__, reg_addr, reg_val);
        printf("%s: slot_num = 0x%08X.\n", __FUNCTION__, slot_num);
    }

    if ((reg_val & PLUG_PRSNT) != PLUG_PRSNT) {
        return (FALSE);
    }
    return (TRUE);
}


/*******************************************************************************
 * Function   : plug_common_plug_is_pwr_ok 
 * Description: Function to check if PLUG module is power ok.
 * Inputs     : slot_num - Pluggable slot number (Start from 1)
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
boolean plug_common_plug_is_pwr_ok (uint slot_num)
{
    uint reg_addr;
    uint reg_val = 0;
    int max_slot_num = plug_common_host_get_max_plug_slots();

    /* Sanity check whether slot_num exceeds maximum number of slot */
    if (slot_num > max_slot_num) {
        printf("%s: Slot number (%d) exceeds maximum slot number (%d)\n", 
               __func__, slot_num, max_slot_num);
        return (FALSE);
    }

    /* Calculate Register address based on slot number */
    reg_addr = FPGA_PLUG_OFFSET_BY_SLOT(FPGA_PLUG1_STSCTL_REG, slot_num);

    if (plug_common_host_plug_fpga_reg_read(reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read Pluggable FPGA Control/Status Reg(0x%04X).\n",
               __FUNCTION__, reg_addr);
        return (FALSE);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: PLUG FPGA @0x%04X: 0x%08X.\n", __FUNCTION__, reg_addr, reg_val);
        printf("%s: slot_num = 0x%08X.\n", __FUNCTION__, slot_num);
    }

    if ((reg_val & PLUG_PWR_OK) != PLUG_PWR_OK) {
        return (FALSE);
    }
    return (TRUE);
}

/*-------------------------------------------------------------------
 * Function : plug_slot_i2c_ctrl
 * Description: This function get i2c controller number from platform
 * INPUT: slot - plug slot number. Start from 1 
 * OUTPUT: PASSED or FAILED
 * -------------------------------------------------------------------
*/
static int plug_slot_i2c_ctrl (int slot)
{

    return (plug_common_host_i2c_ctrl(slot));
}

/*-------------------------------------------------
$Log: plug_slot.c,v $
Revision 1.11  2021/09/24 01:27:20  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.10  2021/06/02 02:56:25  alpeng
merge sears into trunk

Revision 1.9.10.1  2021/02/17 23:36:06  tshanmug
Sears PIM test back to back test delay for dying gasp

Revision 1.9  2020/06/03 08:42:35  sherliu2
Setup delay time (15 sec) between module power off/on for Hyperloop

Revision 1.8  2019/12/18 09:18:37  alpeng
1. support quack cookie rd/wr; 2. fixed new rommon break nightwatch issue; 3. a workaround for new pim testcard crashed system issue; 4. bump to v2.0.1 for curie

Revision 1.7  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.6  2019/08/15 09:27:52  shjung
Supported WP7610 PIM

Revision 1.5  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.4  2019/07/02 02:30:03  shjung
Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.3  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.2  2018/10/16 07:08:44  hondwang
plug_tc_host_sgmii_present should be platform code, modified

Revision 1.2.62.1  2018/10/15 06:50:18  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.11  2017/11/20 07:54:31  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.1.4.10  2017/11/15 11:56:06  hondwang
Fix C1109-4P slot 2 fail with IO interface issue

Revision 1.1.4.9  2017/11/13 09:05:48  hondwang
Add pluggable slot cookie info with Diag login

Revision 1.1.4.8  2017/09/21 19:30:14  hondwang
Poweroff pluggable module before testing

Revision 1.1.4.7  2017/09/19 21:40:19  hondwang
Change Pluggable USB testing power sequence

Revision 1.1.4.6  2017/09/15 21:48:34  hondwang
Power off none test slot

Revision 1.1.4.5  2017/09/09 00:47:47  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.1.4.4  2017/08/28 07:53:50  shjung
Added pluggable I/O interface test

Revision 1.1.4.3  2017/08/16 07:24:03  shjung
Reread cookie id while entering pluggable test menu and fix usb mode switching issue

Revision 1.1.4.2  2017/08/08 07:40:40  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.5  2017/08/02 05:40:41  lucywang
add LED utility of Pluggable Serial

Revision 1.1.2.4  2017/07/31 07:50:15  hondwang
Fix pluggable test log be USB disconnect interrupt

Revision 1.1.2.3  2017/07/25 03:49:47  hondwang
pluggable testcard continuous testing need power off delay

Revision 1.1.2.2  2017/07/20 12:52:38  hondwang
Add module reset pin testing and fix test card not unreset issue

Revision 1.1.2.1  2017/07/13 06:32:18  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.9  2017/07/12 06:25:13  tirawan
Turn off power enable bit when module is powered off

Revision 1.1.2.8  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/


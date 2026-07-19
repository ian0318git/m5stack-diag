/* $Id: platform_psu.c,v 1.28 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_psu.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_psu.c
 *
 * Description: PSU I2C device.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include "endians.h"
#include "common.h"
#include "platform_psu.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "nvmonvars.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "cross_platform.h"
#include "spi_cookie.h"
#include "cli_cmd.h"
#include "cookie_4.h"
#include "plat_defs.h"
#include "i2c_address.h"
#include "i2c_dev.h"
#include "dash_fpga.h"
#include "goofy_i2c.h"
#include "platform_poe_psu.h"
#include "platform_pem_utils.h" /* pem_show_cookie_x */

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
extern uint32 err_report (dev_object_t *dev, char *err_msg,
			  uint32 err_type); /* in hwic_spidey_ct3.c */
static void dev_obj_create(dev_at_object_t *, n2g_i2c_if_t *i2c_if, int);
int show_psu_cookie(int);
static int dump_psu_regs(void);
static int show_psu_reg(void);
/* static int alter_cookie(void); */
static int psu_pr_test(int submenu);

static int psu_write_reg (uint32_t offset, uint16_t size, char *data);
uint32_t get_psu_i2c_struct(n2g_i2c_if_t *, uint32_t);

uint32_t check_psu_stat(uint32_t);

extern int cookie_4_processor_x (uchar *contents, int board_type,
                               int cookie_type, int cookie_size, 
                               cli_cookie_cmd *);
extern int do_all_menu_items(struct menuinfo *);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t, uint32_t);
extern boolean has_poe_psu(uint32_t);

int get_mux_mask (uint dev_no)
    __attribute__((weak, alias("__get_mux_mask")));

int __get_mux_mask (uint dev_no)
{
    printf("WARNING!! %s in %s should not be invoked \n"
                , __FUNCTION__, __FILE__);
    return 0;
}

int set_mux_channel (n2g_i2c_dev_t *i2c_p, uint8_t mux_mask, uint32_t mux_id)
    __attribute__((weak, alias("__set_mux_channel")));

int __set_mux_channel (n2g_i2c_dev_t *i2c_p, uint8_t mux_mask, uint32_t mux_id)
{
    printf("WARNING!! %s in %s should not be invoked \n"
                , __FUNCTION__, __FILE__);
    return 0;
}

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static uint32_t psu_no_now = 0;

static int clear_psu_faults(int arg)
{
    char data = 0x0;
    psu_write_reg(0x03, 0, &data);
    return PASSED;
}

/*
 * PSU Menu
 */
static submenu_xtable_t psu_menu_table[] = {
    {"PSU cookie utility",          (PFT)show_psu_cookie,    0,
        0,                          (PFT)0, 0, (PFT)0, 0},
    {"Dump PSU registers",          (PFT)dump_psu_regs,  0,
        0,                          (PFT)0, 0, (PFT)0, 0},
    {"Show PSU register",           (PFT)show_psu_reg,   0,
        0,                          (PFT)0, 0, (PFT)0, 0},
    {"PSU Pins Test",               (PFT)psu_pr_test, TRUE,
	(MF_CONTINUOUS | MF_DOALL), (PFT)0, 0, (PFT)0, 0},
    {"Clear Faults",                (PFT)clear_psu_faults,   0,
        0,                          (PFT)0, 0, (PFT)0, 0},
};

#define PSU_MENU_TABLE_SIZE (sizeof(psu_menu_table) / \
		sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t psu_menu_primary_items[PSU_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];
static mitem_t psu_menu_secondary_items[PSU_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];

static struct menuinfo psudiag = {
    "PSU Utility Menu",		/* title */
    0,				/* title string added by init_empty_menu */
    0,				/* do not show major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    psu_menu_primary_items,
};

static struct menuinfo *psudiagp = &psudiag;

/* Bit define for the pin usage for the cpu clocks */
static psu_reg_info_t psu_regs_tbl[] = {
   {0x3B, "FAN_COMMAND_1",         2, I2C_SMBUS_WORD_DATA}, 
   {0x46, "IOUT_OC_FAULT_LIMIT",   2, I2C_SMBUS_WORD_DATA}, 
   {0x4A, "IOUT_OC_WARN_LIMIT",    2, I2C_SMBUS_WORD_DATA}, 
   {0x4F, "OT_FAULT_LIMIT",        2, I2C_SMBUS_WORD_DATA}, 
   {0x51, "OT_WARN_LIMIT",         2, I2C_SMBUS_WORD_DATA}, 
   {0x58, "VIN_UV_WARN_LIMIT",     2, I2C_SMBUS_WORD_DATA}, 
   {0x59, "VIN_UV_FAULT_LIMIT",    2, I2C_SMBUS_WORD_DATA}, 
   {0x5D, "IIN_OC_WARN_LIMIT",     2, I2C_SMBUS_WORD_DATA}, 
   {0x6B, "PIN_OP_WARN_LIMIT",     2, I2C_SMBUS_WORD_DATA}, 
   {0x79, "STATUS_WORD",           2, I2C_SMBUS_WORD_DATA}, 
   {0x7A, "STATUS_VOUT",           1, I2C_SMBUS_BYTE_DATA}, 
   {0x7B, "STATUS_IOUT",           1, I2C_SMBUS_BYTE_DATA}, 
   {0x7C, "STATUS_INPUT",          1, I2C_SMBUS_BYTE_DATA}, 
   {0x7D, "STATUS_TEMPERATURE",    1, I2C_SMBUS_BYTE_DATA}, 
   {0x7E, "STATUS_CML",            1, I2C_SMBUS_BYTE_DATA}, 
   {0x81, "STATUS_FANS_1_2",       1, I2C_SMBUS_BYTE_DATA}, 
   {0x88, "READ_VIN",              2, I2C_SMBUS_WORD_DATA}, 
   {0x89, "READ_IIN",              2, I2C_SMBUS_WORD_DATA}, 
   {0x8B, "READ_VOUT",             2, I2C_SMBUS_WORD_DATA}, 
   {0x8C, "READ_IOUT",             2, I2C_SMBUS_WORD_DATA}, 
   {0x8D, "READ_TEMPERATURE_1",    2, I2C_SMBUS_WORD_DATA}, 
   {0x8E, "READ_TEMPERATURE_2",    2, I2C_SMBUS_WORD_DATA}, 
   {0x8F, "READ_TEMPERATURE_3",    2, I2C_SMBUS_WORD_DATA}, 
   {0x90, "READ_FAN_SPEED_1",      2, I2C_SMBUS_WORD_DATA}, 
   {0x96, "READ_POUT",             2, I2C_SMBUS_WORD_DATA}, 
   {0x97, "READ_PIN",              2, I2C_SMBUS_WORD_DATA}, 
   {0x98, "PMBUS_REVISION",        1, I2C_SMBUS_BYTE_DATA}, 
   {0x99, "MFR_ID",                5, I2C_SMBUS_BLOCK_DATA}, 
   {0x9A, "MFR_MODEL",            11, I2C_SMBUS_BLOCK_DATA}, 
   {0x9B, "MFR_REVERSION",         3, I2C_SMBUS_BLOCK_DATA}, 
   {0xA0, "MFR_VIN_MIN",           2, I2C_SMBUS_WORD_DATA}, 
   {0xA1, "MFR_VIN_MAX",           2, I2C_SMBUS_WORD_DATA}, 
   {0xA2, "MFR_IIN_MAX",           2, I2C_SMBUS_WORD_DATA}, 
   {0xA3, "MFR_PIN_MAX",           2, I2C_SMBUS_WORD_DATA}, 
   {0xA4, "MFR_VOUT_MIN",          2, I2C_SMBUS_WORD_DATA}, 
   {0xA5, "MFR_VOUT_MAX",          2, I2C_SMBUS_WORD_DATA}, 
   {0xA6, "MFR_IOUT_MAX",          2, I2C_SMBUS_WORD_DATA}, 
   {0xA7, "MFR_POUT_MAX",          2, I2C_SMBUS_WORD_DATA}, 
   {0xA8, "MFR_TAMBIENT_MAX",      2, I2C_SMBUS_WORD_DATA}, 
   {0xAA, "MFR_EFFICIENCY_LL",    14, I2C_SMBUS_BLOCK_DATA}, 
   {0xAB, "MFR_EFFICIENCY_HL",    14, I2C_SMBUS_BLOCK_DATA}, 
};


/*******************************************************************************
 *
 * Function   : get_psu_i2c_struct
 * Description: To get PSU I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t get_psu_i2c_struct (n2g_i2c_if_t *i2c_if, uint32_t psu_type)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    switch (psu_type) {
    case PSU1_EEPROM:
        /* Juno is not using this code so only Utah and O2 are covered
         * here
         */
        if ((is_utah() || is_overlord() || is_ntpn_machines() || is_vg450()) ||
            is_curie_2ru()) {
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                                 MB_I2C_ADDR_PSU1_EEPROM);
        } else {
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                                 MB_I2C_ADDR_PSU1_EEPROM_SD);
        }
        break;
    case PSU1_MCNTRL:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_PSU1_MCNTRL);
        break;
    case PSU2_EEPROM:
        if (is_ntpn_machines() || is_vg450() || is_curie_2ru()) {
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                                 MB_I2C_ADDR_PSU2_EEPROM);
        } else {
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ONE,
                                                 MB_I2C_ADDR_PSU2_EEPROM);
        }
        break;
    case PSU2_MCNTRL:
        if (is_ntpn_machines() || is_vg450() || is_curie_2ru()) {
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                                 MB_I2C_ADDR_PSU2_MCNTRL);
        } else {
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ONE,
                                                 MB_I2C_ADDR_PSU2_MCNTRL);
        }
    case PEM0_EEPROM:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                             (is_sword() == TRUE) ? MB_I2C_ADDR_PSU1_EEPROM_SD : MB_I2C_ADDR_PEM0_EEPROM);
        break;
    case PEM0_MCNTRL:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_PEM0_MCNTRL);
        break;
    case PEM1_EEPROM:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, 
                                             (is_curie_1ru()) ? I2C_MUX_ZERO : I2C_MUX_ONE,
                                             MB_I2C_ADDR_PEM1_EEPROM);
        break;
    case PEM1_MCNTRL:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, 
                                             (is_curie_1ru()) ? I2C_MUX_ZERO : I2C_MUX_ONE,
                                             MB_I2C_ADDR_PEM1_MCNTRL);
        break;
    default:
        printf("%s:%d Unknown PSU related device no: %d.\n",
               __FUNCTION__, __LINE__, psu_type);
        return (FAILED);
    }

    if (tmp == NULL) {
        printf("%s:%d Failed to get PSU %d I2C interface structure.\n",
               __FUNCTION__, __LINE__, psu_type);
        return (FAILED);
    }

    memcpy(i2c_if, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/*******************************************************************************
 *
 * function   : build_psu_menu
 * Description:	Build menu for PSU.
 * Inputs     : option
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_psu_menu (uint32_t option)
{
    uint32_t menu_opt = 0;
    char    t_name[OVLD_BUF_SIZE];
	int rc = 0;
	uint8_t psu_mux_mask = 0;
	
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("option = %#x.\n", option);
    }

    /* Get the PSU number */
    psu_no_now = (option >> OVLD_PSU_OFF);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d psu_no_now = %d.\n", __FUNCTION__, __LINE__, psu_no_now);
    }

    if ((psu_no_now == 0) || (psu_no_now > MAX_NUM_PSU)) {
        cterr('f', 0, "%s:%d Invalid PSU No.(%d)",
              __FUNCTION__, __LINE__, psu_no_now);
        return;
    }

    sprintf(t_name, "PSU %d", psu_no_now);
    testname(t_name);

    /* Check PSU present */
    if (check_psu_present(psu_no_now) != TRUE) {
        return;
    }
    
    if (is_utah() || is_ntpn_machines() || is_vg450() || is_curie_1ru() || is_curie_2ru()) {
        /* Get Mux mask */
        psu_mux_mask = get_mux_mask(psu_no_now-1);
        if (psu_mux_mask == 0) {
            cterr('f', 0, "%s:%d Failed to get Mux mask).",
                          __FUNCTION__, __LINE__);
            return;
        }

        /* Setup Mux channel */
        rc = set_mux_channel(NULL, psu_mux_mask, OVLD_PSU_I2C_MUX);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                          __FUNCTION__, __LINE__, rc);
            return;
        }
    }

    build_primary_submenu(psu_menu_table, PSU_MENU_TABLE_SIZE,
			  "PSU Utility Menu", &psudiagp);
    build_secondary_submenu(psu_menu_table, PSU_MENU_TABLE_SIZE,
			    psu_menu_secondary_items);

    menu_opt = (option & OVLD_MENU_OPT_MSK);
    if (menu_opt) {
        /* Entered with submenu */
        menu(&psudiag, psu_menu_secondary_items, 0);
    } else {
        do_all_menu_items(psudiagp);
    }
}


/*******************************************************************************
 *
 * Function   : show_psu_type
 * Description:	Display PSU Controller ID.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void show_psu_type (void)
{
    uint32_t     rc = FAILED;
    uint16_t     psu_id = 0;
    n2g_i2c_if_t i2c_if;

    /* Check PSU present */
    if (check_psu_present(psu_no_now) != TRUE) {
        return;
    }

    /* Setup I2C API parameter struct */
    switch (psu_no_now) {
    case PSU_ONE:
        rc = get_psu_i2c_struct(&i2c_if, PSU1_EEPROM);
        break;
    case PSU_TWO:
        rc = get_psu_i2c_struct(&i2c_if, PSU2_EEPROM);
        break;
    default:
        cterr('f', 0, "%s: Got Unknown PSU no.(%d)\n",
                      __FUNCTION__, psu_no_now);
        return;
    }

    if (rc != PASSED) {
        return;
    } 
    
    i2c_if.buf = (char *)&psu_id;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = 0x09;

    /* Read the cookies */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to read PSU%d cookie (rc = %#x).",
                      __FUNCTION__, psu_no_now, rc);
	return;
    }

    printf("%s: PSU%d ID = 0x%04x.\n", __FUNCTION__, psu_no_now, psu_id);
}


/*********************************************************************
 *
 * Function:	cli_dev_obj_create
 *
 * Description:	CLI need to Create PSU Cookie device object for
 *		Common Device Driver.
 *
 * Inputs:	pe - Points to Max1617A device object
 *		i2c_if - Points to I2C API interface struct
 *
 * Outputs:	None
 *
 *********************************************************************
 */
void cli_psu_dev_obj_create(dev_at_object_t *pe, n2g_i2c_if_t *i2c_if, int psu_num)
{
    dev_obj_create(pe, i2c_if, psu_num);  
}

/*********************************************************************
 *
 * Function:	dev_obj_create
 *
 * Description:	Create PSU Cookie device object for
 *		Common Device Driver.
 *
 * Inputs:	pe - Points to Max1617A device object
 *		i2c_if - Points to I2C API interface struct
 *
 * Outputs:	None
 *
 *********************************************************************
 */
static void
dev_obj_create(dev_at_object_t *pe, n2g_i2c_if_t *i2c_if, int psu_num)
{
    dev_object_t *dev = (dev_object_t *)pe;

    /* Setup device struct */

    /* Create common device object */
    dev_at24c0n_create(dev, (dev_error_report_t) err_report);

    /* Setup call-out function vectors */
    pe->callout_fvt->open = n2g_i2c_open;
    pe->callout_fvt->close = n2g_i2c_close;
    pe->callout_fvt->rd = n2g_i2c_read;
    pe->callout_fvt->wr = n2g_i2c_write;

    /* Setup type and name text */
    pe->dev_type = AT24C_02;
    pe->dev_name = "PSU Cookie";

    /* Setup other struct fields */
    pe->i2c_p = i2c_if;

#ifdef PSU_DEBUG
    printf("\ni2c %#x\n", i2c_if->i2c_dev);
#endif /* PSU_DEBUG */

    i2c_if->size = sizeof(at_t);	/* Buffer size */
}


/*******************************************************************************
 *
 * Function   : show_psu_cookie
 * Description:	Display PSU Cookie Contents.
 * Inputs     : psu_num
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_psu_cookie (int psu_num)
{
    if (is_juno() || is_sword() || is_curie_1ru())
        psu_no_now = psu_num;

    return (psu_show_cookie_x(MENU_MODE, NULL));
}


/*******************************************************************************
 *
 * Function   : psu_show_cookie_x
 * Description:	Display PSU Cookie Contents. Entry for both CLI and MENU mode.
 * Inputs     : mode, TRUE is CLI ; FALSE is MENU
 *              cli cmd structure. 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int psu_show_cookie_x (boolean mode, cli_cookie_cmd *cmd)
{
    at_t data[AT24C02_MAX + 1];
    n2g_i2c_if_t i2c_if;
    uint32_t rc = FAILED;
	uint8_t psu_mux_mask = 0;
    int i;

    /* Reset data buffer */
    memset(data, 0, sizeof(data));

    /* For menu mode already check PSU is present or not */ 
    if (mode == CLI_MODE) {
        psu_no_now = cmd->slot;

        /* Rule 1 - Make sure PSU present */
        if (check_psu_present(psu_no_now) != TRUE){
            return CLI_DEVICE_IS_VACANT;
        }
        /* Rule 2 - If Input OK and 12V Output OK must be in the same stats */
        if (check_psu_stat(psu_no_now) != TRUE) {
            return CLI_DEVICE_IS_VACANT;
        }

        if (is_utah()) {
            /* Get Mux mask */
            psu_mux_mask = get_mux_mask(psu_no_now-1);
            if (psu_mux_mask == 0) {
                cterr('f', 0, "%s:%d Failed to get Mux mask).",
                          __FUNCTION__, __LINE__);
                return (FAILED);
            }

            /* Setup Mux channel */
            rc = set_mux_channel(NULL, psu_mux_mask, OVLD_PSU_I2C_MUX);
            if (rc != PASSED) {
                cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                          __FUNCTION__, __LINE__, rc);
                return (FAILED);
            }
        }
    }

    /* Juno using different method to access the cookie */
    if(is_juno() || is_sword() || is_curie_1ru())
        return (pem_show_cookie_x(mode, cmd));

    /* below is for O2 */
    /* Setup I2C API parameter struct */
    switch (psu_no_now) {
    case PSU_ONE:
            rc = get_psu_i2c_struct(&i2c_if, PSU1_EEPROM);
        break;
    case PSU_TWO:
            rc = get_psu_i2c_struct(&i2c_if, PSU2_EEPROM);
        break;
    default:
        cterr('f', 0, "%s: Got Unknown PSU number (%d).",
                      __FUNCTION__, psu_no_now);
        return (FALSE);
    }

    if (rc != PASSED) {
        return (rc);
    } 

    i2c_if.buf = (char *)&data[0];
    i2c_if.size = sizeof(data);
    i2c_if.offset = 0;

    /* Read the cookies */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: PSU%d enum %#x read cookies failed with rc = %#x",
		      __FUNCTION__, psu_no_now, i2c_if.i2c_dev, rc);
	return (FAILED);
    }

    /* Call cookie utility */
    if (cookie_4_processor_x(data, PSU_MODULE, 0, i2c_if.size, cmd) == 1) {
	/* Updated. Write them back */
	/* Do not use the block write, since EEPROM need wait between bytes */
	i2c_if.size = sizeof(at_t);	/* write 1 byte at a time */
	for (i = 0; i < sizeof(data); i++) {
	    i2c_if.offset = i;
	    i2c_if.buf = (char *)&data[i];
	    rc = n2g_i2c_write(&i2c_if);
	    if (rc != PASSED) {
		cterr('f', 0, "%s: PSU%d failed to write %#x @ %#x. rc = %#x",
                              __FUNCTION__, psu_no_now, data[i], i, rc);
		return(FAILED);
	    } /* endof rc */
	    msleep(AT24C0X_T_WR + 1);	/* Wait for write to complete */
	} /* endof for */
    } /* endof of cookie_4_processor_x */

    if (rc != PASSED) {
	cterr('f', 0, "%s: PSU%d show cookie Failed.",
                      __FUNCTION__, psu_no_now);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : check_psu_present
 * Description:	Return if the power supply is installed.
 * Inputs     : No. of PSU that will be checked
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
uint32_t check_psu_present (uint32_t psu_no)
{
    /* Notes: Juno PSU numbering is the opposite of Overlord.
     * Overlord PSU 0 is on the left hand side when viewing into the
     * power plug, and PSU 1 is on the right hand side. Juno is the
     * opposite.
     * A mistake was made at the mother board lay out. The PSU0/1 naming
     * is corrected on the chasis. The internal PSU present bits in
     * the FPGA register is still shared between the 2 platforms.
     */
    uint32_t result = FALSE, psu_present_mask = 0;
    psu_t    *psu_reg;

    /* Set related parameters based on input PSU no. */

    switch (psu_no) {
    case PSU_ONE:
        psu_present_mask = PSU1_PRESENT_MSK;
        break;
    case PSU_TWO:
        psu_present_mask = PSU2_PRESENT_MSK;
        break;
    default:
        cterr('f', 0, "%s: Got Unknown PSU number (%d).",
                      __FUNCTION__, psu_no);
        return (FALSE);
    }

    /* Read FPGA  */
    psu_reg = (psu_t *)get_platform_ps_env_base();

    /* Check if PSU is present */
    if ((psu_reg->psu_stat) & psu_present_mask) {
       result = TRUE;
    } else {
       printf("%s: PSU%d device is NOT present.\n",
                     __FUNCTION__, psu_no);
    }
    return (result);
}


/*******************************************************************************
 *
 * Function   : check_psu_stat
 * Description:	Return if the Input OK and 12V Output OK are in the same stat.
 * Inputs     : No. of PSU that will be checked
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
uint32_t check_psu_stat (uint32_t psu_no)
{
    uint32_t result = FALSE;
    uint32_t psu_ac_in_mask = 0, psu_12v_out_mask = 0;
    psu_t    *psu_reg;

    /* Set related parameters based on input PSU no. */
    switch (psu_no) {
    case PSU_ONE:
        psu_ac_in_mask = PSU1_AC_IN_OK;
        psu_12v_out_mask = PSU1_12V_OUT_OK;
        break;
    case PSU_TWO:
        psu_ac_in_mask = PSU2_AC_IN_OK;
        psu_12v_out_mask = PSU2_12V_OUT_OK;
        break;
    default:
        cterr('f', 0, "%s:%d Got Unknown PSU num (%d).",
                      __FUNCTION__, __LINE__, psu_no);
        return (FALSE);
    }

    /* Read FPGA  */
    psu_reg = (psu_t *)get_platform_ps_env_base();

    /* Check AC Input stat */
    if ((psu_reg->psu_stat) & psu_ac_in_mask) {
        if ((psu_reg->psu_stat) & psu_12v_out_mask) {
            result = TRUE;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("PSU%d AC Input and 12V Output are ready.\n", psu_no);
            }
        } else {
            printf("PSU%d 12V Output is NOT ready.\n", psu_no);
        }
    } else {
        printf("PSU%d AC Input is NOT ready.\n", psu_no);
    }

    return (result);
}


/**********************************************************************
 *
 * Function:	cli_check_ps_present
 *
 * Description:	CLI need this to Returns if the power supply is installed.
 *
 * Input:	sgpi - Goofy Master Slow General Purpose Input pin
 *
 * Output:	TRUE if installed. FALSE if not.
 *
 **********************************************************************
 */
int cli_check_ps_present(uint present)
{   
    uint32_t rc = FALSE;
    
    if (check_psu_present(present) == TRUE) {
        rc = TRUE;
    }
    
    return rc;

}

/*******************************************************************************
 *
 * Function:	psu_pr_test
 * Description:	PSU Presence checking. Refer to CSCsz33018 for more info
 *		The rules of checking -
 *		1 - PSU must be present.
 *		2 - PSU Input OK and 12V Output OK must be in the same state.
 * Inputs     : submenu - TRUE if submenu invoked.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_pr_test (int submenu)
{
    int rc = PASSED;
    char err_buf[OVLD_BUF_SIZE];

    testname("PSU Presence & Stat Detection");
    prpass(testpass, "PSU%d ", psu_no_now);

    /* Rule 1 - Make sure PSU present */
    if (check_psu_present(psu_no_now) != TRUE) {
        sprintf(err_buf, "%s: PSU%d is not present.\n",
                         __FUNCTION__, psu_no_now);
        rc = FAILED;
    }

    /* Rule 2 - If Input OK and 12V Output OK must be in the same stats */
    if (check_psu_stat(psu_no_now) != TRUE) {
        sprintf(err_buf, "%s: PSU%d is not in right stat.\n",
                         __FUNCTION__, psu_no_now);
        rc = FAILED;
    }

    if (rc != PASSED) {
	cterr('f', 0, &err_buf[0]);
    } else {
        if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
            printf("passed.\n");
        }
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : psu_read_reg
 * Description: To read register of PSU MicroController.
 * Inputs     : offset - offset of register want to read
 *              size   - size of register want to read
 *              data   - buffer to put the read back value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_read_reg (uint32_t offset, uint16_t size, char *data)
{
    n2g_i2c_if_t i2c_if;
    int          result = FAILED;

    /* Get PSU I2C interface structure */
    switch (psu_no_now) {
    case PSU_ONE:
        result = get_psu_i2c_struct(&i2c_if, PSU1_MCNTRL);
        break;
    case PSU_TWO:
        result = get_psu_i2c_struct(&i2c_if, PSU2_MCNTRL);
        break;
    default:
        printf("%s:%d Unsupported PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no_now);
        return (FAILED);
    }

    if (result != PASSED) {
        printf("%s: Failed to get PSU%d Microcontroller I2C structure.\n",
               __FUNCTION__, psu_no_now);
        return (FAILED);
    }

    /* Setup I2C API parameter struct */
    i2c_if.buf = data;
    i2c_if.size = size;
    i2c_if.offset = offset;
    
    result = n2g_i2c_read(&i2c_if);
    if (result != RC_I2C_OP_OK) {
	/* Unable to read data */
	printf("%s:%d Failed to read Reg. %#x (result = %#x).",
                __FUNCTION__, __LINE__, i2c_if.offset, result);
	return (FAILED);
    }

    return (PASSED);
}

static int psu_write_reg(uint32_t offset, uint16_t size, char *data)
{
    n2g_i2c_if_t i2c_if;
    int result = FAILED;

    /* Get PSU I2C interface structure */
    switch (psu_no_now) {
    case PSU_ONE:
        result = get_psu_i2c_struct(&i2c_if, PSU1_MCNTRL);
        break;
    case PSU_TWO:
        result = get_psu_i2c_struct(&i2c_if, PSU2_MCNTRL);
        break;
    default:
        printf("%s:%d Unsupported PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no_now);
        return (FAILED);
    }

    if (result != PASSED) {
        printf("%s: Failed to get PSU%d Microcontroller I2C structure.\n",
               __FUNCTION__, psu_no_now);
        return (FAILED);
    }

    /* Setup I2C API parameter struct */
    i2c_if.buf = data;
    i2c_if.size = size;
    i2c_if.offset = offset;

    result = n2g_i2c_write(&i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to write data */
        printf("%s:%d Failed to write Reg. %#x (result = %#x).",
               __FUNCTION__, __LINE__, i2c_if.offset, result);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : dump_psu_regs
 * Description: To dump registers of PSU.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_psu_regs (void)
{
    uint32_t       ctr = 0, offset = 0, p_ctr = 0, psu_regs_tbl_size = 0;
    uint16_t       size = 0, w_data = 0;
    uint8_t        b_data = 0;
    psu_reg_info_t *psu_reg_p;
    char          buf[32], *buffer;
    
    psu_reg_p = &psu_regs_tbl[0];
    psu_regs_tbl_size = (sizeof(psu_regs_tbl) / sizeof(psu_reg_info_t));

    /* Dump all registers listed in Table */ 
    printf("\nPSU %d MicroController Registers:\n", psu_no_now);
    for (ctr = 0; ctr < psu_regs_tbl_size; ctr++, psu_reg_p++) {
        offset = psu_reg_p->code;
        size = psu_reg_p->data_len;

        switch (psu_reg_p->type) {
        case I2C_SMBUS_BLOCK_DATA:
            memset(buf, 0, 32);
            buffer = &buf[0];
            size = (psu_reg_p->data_len + 1);   /* Based on spec */
            break;
        case I2C_SMBUS_WORD_DATA:
            w_data = 0;
            buffer = (char *)&w_data;
            break;
        case I2C_SMBUS_BYTE_DATA:
            b_data = 0;
            buffer = (char *)&b_data;
            break;
        default:
            cterr('f', 0, "%s: PSU%d I2C access type (%#x)"
                          " is NOT supported !!!",
                          __FUNCTION__, psu_no_now, psu_reg_p->type);
            return (FAILED);
        }

        if (psu_read_reg(offset, size, buffer) != PASSED) {
            cterr('f', 0, "%s: Unable to read PSU%d Regiser 0x%02X.",
                          __FUNCTION__, psu_no_now, psu_reg_p->code);
            return (FAILED);
        }
        printf("%-19s (0x%02X): ", psu_reg_p->name,
               psu_reg_p->code);

        if (psu_reg_p->type == I2C_SMBUS_BLOCK_DATA) {
            if ((psu_reg_p->code >= 0x99) &&
                (psu_reg_p->code <= 0x9B)) {
                printf("(%s) ", (buf+1));
            }

            for (p_ctr = 0; p_ctr < size; p_ctr++) {
                printf("%02X ", buf[p_ctr]);
            }
        } else if (psu_reg_p->type == I2C_SMBUS_WORD_DATA) {
            printf("0x%04X", w_data);
        } else {
            printf("0x%02X", b_data);
        }
        printf("\n");
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : show_psu_reg
 * Description: To show register of PSU MicroController that user choosed.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int show_psu_reg (void)
{
    uint32_t       ctr = 0, reg_addr = 0, psu_regs_tbl_size = 0;
    uint16_t       w_data = 0, size = 2;
    uint8_t        b_data = 0;
    psu_reg_info_t *psu_reg_p;
    char          buf[32], *buffer;
    
    buffer = (char *)&w_data;
    psu_regs_tbl_size = (sizeof(psu_regs_tbl) / sizeof(psu_reg_info_t));

    reg_addr = gethex_answer("Enter the addr. of Reg. you want to read:",
                             0x99, 0x0, 0xFF);

    psu_reg_p = &psu_regs_tbl[0];
    for (ctr = 0; ctr < psu_regs_tbl_size; ctr++, psu_reg_p++) {
        if (reg_addr == psu_reg_p->code) {
            switch (psu_reg_p->type) {
            case I2C_SMBUS_BLOCK_DATA:
                buffer = &buf[0];
                size = (psu_reg_p->data_len + 1);   /* Based on spec */
            break;
            case I2C_SMBUS_WORD_DATA:
                buffer = (char *)&w_data;
                size = psu_reg_p->data_len;
            break;
            case I2C_SMBUS_BYTE_DATA:
                buffer = (char *)&b_data;
                size = psu_reg_p->data_len;
            break;
            default:
                cterr('f', 0, "%s: PSU%d I2C acces type(%#x) "
                              "is NOT supported!!!",
                              __FUNCTION__, psu_no_now, psu_reg_p->type);
                return (FAILED);
            }
            break;
        }
    }

    /* Read PSU MicroController register */
    if (psu_read_reg(reg_addr, size, buffer) != PASSED) {
        cterr('f', 0, "%s: Unable to read PSU%d MicroController Reg. 0x%02X.",
                      __FUNCTION__, psu_no_now, reg_addr);
        return (FAILED);
    }

    printf("\nPSU%d MicroController 0x%02X = ", psu_no_now, reg_addr);
    if (size > 2) {
        for (ctr = 0; ctr < size; ctr++) {
            printf("%02X ", buf[ctr]);
        }
        printf("\n");
    } else if (size == 1) {
        printf("0x%02X\n", b_data);
    } else {
        printf("0x%04X\n", w_data);
    }

    return (PASSED);
}


#ifdef UTAH
/******************************************************************************
 *
 * Function   : psu_i2c_test_warp
 * Description: Function for warp PSU/POE PSU i2c scan test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
int psu_i2c_test_warp (void)
{
    uint psu_id;
    uint rc = FAILED, ia;
    uint8_t psu_mux_mask = 0;
    int psu_valid = FALSE;
    char data[8];
    n2g_i2c_if_t i2c_if;
    int offset, i2c_dev_mcntrl, i2c_dev_eeprom, size;
    
    /* init psu and mux setting, then perform simple i2c read */
    for (ia = OVLD_MUX_PSU0; ia < OVLD_MUX_PSU_INVALID; ia++) {
        /* check PSU and POE PSU is available or not. */
        switch (ia){
           case OVLD_MUX_PSU0:
               psu_valid = check_psu_stat(OVLD_PSU1);
               i2c_dev_mcntrl = MB_I2C_ADDR_PSU1_MCNTRL; 
               i2c_dev_eeprom = MB_I2C_ADDR_PSU1_EEPROM; 
               offset = psu_regs_tbl[27].code; /* MFR_ID */
               size = psu_regs_tbl[27].data_len + 1;
               break;
           case OVLD_MUX_PSU2:
               psu_valid = has_poe_psu(POE_PSU_ONE);
               i2c_dev_mcntrl = MB_I2C_ADDR_POE_PSU1_MCNTRL; 
               i2c_dev_eeprom = MB_I2C_ADDR_POE_PSU1_EEPROM; 
               offset = 0;
               size = sizeof(uint8_t);
               break;
           case OVLD_MUX_PSU1:
           case OVLD_MUX_PSU3:
	       /* Utah does not have the 2nd psu and poe like overlord */
	       psu_valid = FALSE;
               break;
           default: 
               printf("\n%s failure report: no such case", __FUNCTION__);
               break;
        } 
        if (psu_valid == FALSE) { /* device is not available */
            continue;
        }
        psu_id = ia - OVLD_MUX_PSU0;

        /* Get Mux mask */
        psu_mux_mask = get_mux_mask(psu_id);
        if (psu_mux_mask == 0) {
            cterr('f', 0, "%s:%d Failed to get Mux mask.",
                          __FUNCTION__, __LINE__);
            return (rc);
        }

        /* Setup Mux channel */
        rc = set_mux_channel(NULL, psu_mux_mask, OVLD_PSU_I2C_MUX);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
            return (rc);
        }
        /* Start i2c read test on PSU/POE PSU Microcontroller and EEPROM */
        /* Setup I2C API interface struct */
        i2c_if.offset = offset;
        i2c_if.i2c_bus_type = IOFPGA_I2C;
        i2c_if.i2c_dev = i2c_dev_mcntrl; // microcontroller
        i2c_if.i2c_ctrl = I2C_CTRL_FOUR;
        i2c_if.mux = I2C_MUX_ZERO;
        i2c_if.size = size; 
        memset(&data[0], 0, sizeof(data));
        i2c_if.buf = &data[0];
        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to read %#x(rc = %#x).",
                      __FUNCTION__, __LINE__, i2c_if.offset, rc);
            return (rc);
        }

        i2c_if.i2c_dev = i2c_dev_eeprom; // EEPROM
        i2c_if.offset = 0;
        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to read %#x(rc = %#x).",
                      __FUNCTION__, __LINE__, i2c_if.offset, rc);
            return (rc);
        }
    }
    return (rc);
}
#endif 

/*------------------------------------------------------------------
$Log: platform_psu.c,v $
Revision 1.28  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.27  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.26.2.2  2018/10/31 12:27:37  alpeng
curie psu is the same as Juno, fixed basic util, will fix the remaining psu utils.

Revision 1.26.2.1  2018/08/24 20:13:10  alpeng
support curie 1ru

Revision 1.26  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.25  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.24.16.6  2018/05/17 10:50:22  alpeng
 sync with trunk <trunk-051618>

Revision 1.24.16.5  2018/01/25 08:17:34  leschen
Change PSU and POE_PSU FPGA mux to zero.

Revision 1.24.16.4  2017/11/27 05:59:43  leschen
Initial check in to support VG450.

Revision 1.24.16.3  2017/07/05 06:31:15  alpeng
update fan info, update PSU and remove pem files

Revision 1.24.16.2  2017/03/13 07:49:18  leschen
Support Triton system.

Revision 1.24.16.1  2016/10/13 00:29:32  leschen
Modify for PSU utility

Revision 1.25  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.24  2015/06/23 23:11:11  ptong
Fixed PSU1 cookie util for O2. Bump O2 diag version to 11.6.1

Revision 1.23  2014/04/24 08:00:33  hroni
For Utah CLI discovery cmd, set mux channel before reading cookie

Revision 1.22  2014/01/13 09:35:48  hroni
fix psu cookie write for sword

Revision 1.21  2014/01/07 08:33:54  hroni
fix compile error

Revision 1.20  2014/01/07 05:56:06  hroni
support psu diag for sword

Revision 1.19  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.18  2013/12/04 21:43:49  ptong
Fix PSU and POE I2C scan issue on Utah

Revision 1.17  2013/12/04 01:43:18  hroni
fix poe psu detect error

Revision 1.16  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.15  2013/09/11 02:25:05  alpeng
1. support Juno fan info and display on initialize stage.
2. support fedora rootfs

Revision 1.14  2013/08/23 01:02:19  ptong
Added #ifndef UTAH for compile issue on utah

Revision 1.13  2013/08/22 07:43:29  alpeng
support both MENU and CLI PSU cookie on Juno

Revision 1.12  2013/08/15 08:04:54  alpeng
do not cterr while psu us not available

Revision 1.11  2013/08/12 11:06:38  alpeng
clean up msg, skip item before check status

Revision 1.10  2013/08/08 21:23:41  hroni
PSU/POE PSU scan test is done by reading PSU/POE PSU microcontroller and EEPROM

Revision 1.9  2013/07/25 16:57:55  hroni
check psu presence during psu scan i.e. psu_i2c_test_warp()

Revision 1.8  2013/07/18 17:17:02  mcharon
add -Wal and clean up compile warnings

Revision 1.7  2013/07/16 08:02:08  alpeng
support i2c read and read cookie for juno psu

Revision 1.6  2013/07/01 07:49:05  hroni
fix psu index number

Revision 1.5  2013/06/14 09:50:48  hroni
1. support to PSU mux
2. temporarily uses #ifdef MUX124 to turn off/on PCA9545 mux support

Revision 1.4  2013/05/31 12:51:28  danchung
Add checking board type for Juno.

Revision 1.3  2013/05/23 01:09:26  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.2  2013/05/09 19:25:19  mcharon
remove unused header files. fixed dependancy compile problem

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.15  2013/03/29 15:18:46  palin2
Add utilities to dump PSU MicroController registers for debug purpose.

Revision 1.14  2012/11/28 18:19:10  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.13  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.12  2012/11/17 01:12:40  mcharon
remove  cli (clear/set) psu_c_pid function

Revision 1.11  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.10  2012/09/10 09:09:50  alpeng
remove warning from checking PSU status

Revision 1.9  2012/09/03 09:30:30  alpeng
using VERBOSE to mask pass information

Revision 1.8  2012/08/30 07:44:57  alpeng
infrom user with warning when device is vacant on CLI discovery

Revision 1.7  2012/08/24 09:30:50  alpeng
adding check AC in and 12V output for psu cookie of CLI dicscovery cmd

Revision 1.6  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.5  2012/05/09 08:28:14  alpeng
moving FPGA I2C scan test to MB test menu

Revision 1.4  2012/05/04 08:03:32  alpeng
skip Max1617, check PoE and PoE PSU is present before I2C scan test

Revision 1.3  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

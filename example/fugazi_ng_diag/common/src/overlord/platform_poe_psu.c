/* $Id: platform_poe_psu.c,v 1.13 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_poe_psu.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_poe_psu.c
 * Description: Operation Overlord 12V PoE PSU I2C device
 *              related diag tests and utilities.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "menu.h"
#include "error.h"
#include "goofy_i2c.h"
#include "dev_csco_10698.h"
#include "cli_cmd.h"
#include "cookie_4.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "i2c_address.h"
#include "dash_fpga.h"
#include "platform_i2c.h"
#include "platform_poe_psu.h"


/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
static int get_reg_value(n2g_i2c_if_t *);
static int write_reg(n2g_i2c_if_t *);
static int dump_all_regs(void);
static int alter_reg(void);
static uint32_t get_poe_psu_i2c_struct(n2g_i2c_if_t *, uint32_t);
static uint32_t poe_psu_status_wrap(void);
static uint32_t poe_psu_pwr_wrap(void);
static uint32_t poe_psu_pwr_en_test(uint32_t);
static uint32_t poe_psu_reg_test(uint32_t);
static uint32_t poe_psu_fault_detect(uint32_t);
uint32_t check_poe_psu_present (uint32_t, uint32_t);
uint32_t poe_psu_cookie_utils(uint32_t);
int poe_psu_cookie_utils_x(uint32_t, boolean, cli_cookie_cmd *);
boolean has_poe_psu_pwr_ctrl(void);


/******************************************************************************* 
 *                                  Externs                                    *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
extern uint32_t ovld_poe_psu_pwr_control(uint32_t);
extern int set_mux_channel(n2g_i2c_dev_t *, uint8_t, uint32_t);
extern int get_mux_mask(uint);
/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static uint32_t psu_no_now = 0;
static uint32_t first_check = FALSE;

/* Registers default table */
static reg_info_t ltc4280_default_reg_tbl[] = {
    {"0x00: Control",   CNTRL_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0},
    {"0x01: Alter",     ALTER_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x7F, 0},
    {"0x02: Status",    STATUS_REG_OFFSET,
     (READ_ONLY  | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0},
    {"0x03: Fault",     FAULT_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x3F, 0},
    {"0x04: Sense",     SENSE_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0},
    {"0x05: Source",    SOURCE_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0},
    {"0x06: ADIN",      ADIN_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0},
};

/* Registers test table */
static reg_info_t reg_test_tbl[] = {
    {"0x01: Alter",     ALTER_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x7F, 0},
    {"0x03: Fault",     FAULT_REG_OFFSET,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x3F, 0},
};


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * 12V PoE PSU Main Menu
 */
static submenu_xtable_t poe_psu_table[] = {
    {"12V PoE PSU dump all registers", (PFT)dump_all_regs,        0,
         0,                            (PFT)0, 0, (PFT)0, 0},
    {"12V PoE PSU alter register",     (PFT)alter_reg,            0,
         0,                            (PFT)0, 0, (PFT)0, 0},
    {"Enable/Disable 12V PoE PSU",     (PFT)poe_psu_pwr_wrap,     0,
         0,                            (PFT)has_poe_psu_pwr_ctrl, 0, (PFT)0, 0},
    {"12V PoE PSU status info",        (PFT)poe_psu_status_wrap,  0,
         0,                            (PFT)0, 0, (PFT)0, 0},
    {"12V PoE PSU power enable test",  (PFT)poe_psu_pwr_en_test,  0,
         (MF_CONTINUOUS | MF_DOALL),   (PFT)has_poe_psu_pwr_ctrl, 0, (PFT)0, 0},
    {"12V PoE PSU registers test",     (PFT)poe_psu_reg_test,     0,
         (MF_CONTINUOUS | MF_DOALL),   (PFT)0, 0, (PFT)0, 0},
};

#define POE_PSU_TABLE_SIZE (sizeof(poe_psu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t poe_psu_primary_items[POE_PSU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t poe_psu_secondary_items[POE_PSU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo poe_psu = {
    "12V PoE PSU SubMenu",   /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    poe_psu_primary_items,
};

static struct menuinfo *poe_psu_p = &poe_psu;


/*******************************************************************************
 *
 * Function   : get_poe_psu_i2c_struct
 * Description: To get 12V PoE PSU I2C interface structure.
 * Inputs     : Pointer to save the gotten I2C interface structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t get_poe_psu_i2c_struct (n2g_i2c_if_t *i2c_if, uint32_t psu_type)
{
    n2g_i2c_if_t *tmp;

    /* init i2c_if for I2C */
    switch (psu_type) {
    case POE_PSU1_EEPROM:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR,
                         (is_utah() == TRUE || is_ntpn_machines() == TRUE || is_vg450() == TRUE
                          || is_curie_2ru() == TRUE) ?
                         I2C_MUX_ZERO : I2C_MUX_TWO, MB_I2C_ADDR_POE_PSU1_EEPROM);
        break;
    case POE_PSU1_MCNTRL:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR,
                         (is_utah() == TRUE || is_ntpn_machines() == TRUE || is_vg450() == TRUE
                          || is_curie_2ru() == TRUE) ?
                         I2C_MUX_ZERO : I2C_MUX_TWO, MB_I2C_ADDR_POE_PSU1_MCNTRL);
        break;
    case POE_PSU2_EEPROM:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR,
                         (is_utah() == TRUE || is_ntpn_machines() == TRUE || is_vg450() == TRUE
                          || is_curie_2ru() == TRUE) ?
                         I2C_MUX_ZERO : I2C_MUX_THREE, MB_I2C_ADDR_POE_PSU2_EEPROM);
        break;
    case POE_PSU2_MCNTRL:
        tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR,
                          (is_utah() == TRUE || is_ntpn_machines() == TRUE || is_vg450() == TRUE
                           || is_curie_2ru() == TRUE) ?
                         I2C_MUX_ZERO : I2C_MUX_THREE, MB_I2C_ADDR_POE_PSU2_MCNTRL);
        break;
    default:
        printf("***%s:%d Unknown 12V PoE PSU related device no: %d.\n",
               __FUNCTION__, __LINE__, psu_type);
        return (FAILED);
    }

    if (tmp == NULL) {
        printf("***%s:%d Failed to get 12V PoE PSU I2C interface structure.\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    memcpy(i2c_if, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}



/*******************************************************************************
 *
 * Function   : build_poe_psu_menu
 * Description:	Build SubMenu for 12V PoE PSU test.
 * Inputs     : psu_no - PSU I2C device number
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_poe_psu_menu (uint32_t option)
{
    char t_name[ERR_BUF_SIZE];
    uint32_t menu_opt = 0;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("option = %#x.\n", option);
    }

    /* Get the PSU number */
    psu_no_now = (option >> OVLD_POE_PSU_OFF);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d 12V PoE PSU Num = %d.\n",
               __FUNCTION__, __LINE__, psu_no_now);
    }

    if ((psu_no_now == 0) || (psu_no_now > MAX_NUM_POE_PSU)) {
        cterr('f', 0, "%s:%d Invalid 12V PoE PSU No.(%d)",
              __FUNCTION__, __LINE__, psu_no_now);
        return;
    }

    sprintf(t_name, "12V PoE PSU %d", psu_no_now);
    testname(t_name);

    /* Check PoE PSU present */
    if (check_poe_psu_present(psu_no_now, FULL_MODE) != TRUE) {
        return;
    }

#ifdef MUX124
#ifdef UTAH
    int rc;
    uint8_t psu_mux_mask = 0;
    /* 
     * Get Mux mask 
     * Utah only support 1 POE PSU, 
     * the POE PSU ID for PSU mux is SFP_PSU_TWO (2) 
     */
    psu_mux_mask = get_mux_mask(SFP_PSU_TWO);
    if (psu_mux_mask == 0) {
        cterr('f', 0, "%s:%d Failed to get Mux mask.",
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
#endif
#endif

    if (is_ntpn_machines() || is_vg450() || is_curie_2ru()) {
        int rc;
        uint8_t psu_mux_mask = 0;
        /* 
         * Get Mux mask 
         * Neptune support 2 POE PSU, 
         * the POE PSU ID for PSU mux is SFP_PSU_THREE (3) 
         */
        psu_mux_mask = get_mux_mask(psu_no_now + 1);
        if (psu_mux_mask == 0) {
            cterr('f', 0, "%s:%d Failed to get Mux mask.",
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

    /* Build Menu */
    build_primary_submenu(poe_psu_table, POE_PSU_TABLE_SIZE,
			  "12V PoE PSU SubMenu", &poe_psu_p);
    build_secondary_submenu(poe_psu_table, POE_PSU_TABLE_SIZE,
			    poe_psu_secondary_items);

    menu_opt = (option & OVLD_MENU_OPT_MSK);
    if (menu_opt) {
        /* Entered with submenu */
        menu(&poe_psu, poe_psu_secondary_items, 0);
    } else {
        do_all_menu_items(poe_psu_p);
    }
}


/*******************************************************************************
 *
 * Function   : poe_psu_cookie_utils
 * Description:	12V PoE PSU Cookie Utilities.
 * Inputs     : psu_no - expected PoE PSU number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t poe_psu_cookie_utils (uint32_t psu_no)
{
    if (is_usd_machines()) {
        printf("\n(Altering cookie data field is not supported in this platform.)\n");
    }
    return (poe_psu_cookie_utils_x(psu_no, MENU_MODE, NULL));
}


/*******************************************************************************
 *
 * Function   : poe_psu_cookie_utils_x
 * Description:	12V PoE PSU Cookie Utilities.
 * Inputs     : mode, TRUE is CLI ; FALSE is MENU
 *              cli cmd structure. 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int poe_psu_cookie_utils_x (uint32_t psu_no, boolean mode, cli_cookie_cmd *cmd)
{
    char        data[AT24C02_MAX + 1];
    n2g_i2c_if_t i2c_if;
    uint32_t     rc = FAILED;
    int          i;

    if (is_curie_2ru()) {
        int _rc;
        uint8_t psu_mux_mask = 0;

        psu_mux_mask = get_mux_mask(psu_no + 1);
        if (psu_mux_mask == 0) {
            cterr('f', 0, "%s:%d Failed to get Mux mask.",
                          __FUNCTION__, __LINE__);
            return (FAILED);
        }

        /* Setup Mux channel */
        _rc = set_mux_channel(NULL, psu_mux_mask, OVLD_PSU_I2C_MUX);
        if (_rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                          __FUNCTION__, __LINE__, rc);
            return (FAILED);
        }
    }

    /* Reset data buffer */
    memset(data, 0, sizeof(data));

    /* Setup I2C API parameter struct */
    switch (psu_no) {
    case POE_PSU_ONE:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU1_EEPROM);
        break;
    case POE_PSU_TWO:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU2_EEPROM);
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no);
        return (FALSE);
    }

    if (rc != PASSED) {
        return (rc);
    } 

    i2c_if.buf = &data[0];
    i2c_if.size = sizeof(data);
    i2c_if.offset = 0;

    if (is_ntpn_machines() || is_vg450()) {
        i2c_if.mux = I2C_MUX_ZERO;
    }

    /* Read the cookies */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
	cterr('f', 0, "%s: 12V PoE PSU%d read cookie failed.(rc = %#x)",
		      __FUNCTION__, psu_no, rc);
	return (FAILED);
    }

    /* Call cookie utility */
    if (cookie_4_processor_x((uchar *)data, PSU_MODULE, 0, i2c_if.size, cmd) == 1) {
	/* Updated. Write them back */
	/* Do not use the block write, since EEPROM need wait between bytes */
	i2c_if.size = sizeof(at_t);	/* write 1 byte at a time */
	for (i = 0; i < sizeof(data); i++) {
	    i2c_if.offset = i;
	    i2c_if.buf = &data[i];
	    rc = n2g_i2c_write(&i2c_if);
	    if (rc != PASSED) {
		cterr('f', 0, "show_cookie() write %#x @ %#x failed. rc = %#x",
				data[i], i, rc);
		return(FAILED);
	    } /* endof rc */
	    msleep(AT24C0X_T_WR + 1);	/* Wait for write to complete */
	} /* endof for */
    } /* endof of cookie_4_processor_x */

    if (rc != PASSED) {
	cterr('f', 0, "show_cookie() Device show failed");
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : check_poe_psu_present
 * Description:	Return if the expected 12V PoE PSU is installed.
 * Inputs     : psu_no - No. of PoE PSU that will be checked
 *              option - test mode (QUICK_MODE & FULL_MODE)
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
uint32_t check_poe_psu_present (uint32_t psu_no, uint32_t option)
{
    uint32_t result = FALSE;
    uint32_t poe_psu_present_mask = 0, poe_psu_outpower_mask = 0;
    psu_t    *psu_reg;

    /* Set related parameters based on input PSU no. */
    switch (psu_no) {
    case POE_PSU_ONE:
        poe_psu_present_mask = POE_PSU1_PRESENT_MSK;
        poe_psu_outpower_mask = POE_PSU1_OUT_OK_MSK;
        break;
    case POE_PSU_TWO:
        poe_psu_present_mask = POE_PSU2_PRESENT_MSK;
        poe_psu_outpower_mask = POE_PSU2_OUT_OK_MSK;
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no);
        return (FALSE);
    }

    /* Read FPGA  */
    psu_reg = (psu_t *)get_platform_ps_env_base();

    /* Check if the expected PoE PSU is present */
    if ((psu_reg->poe_psu_stat) & poe_psu_present_mask) {
       if (option == QUICK_MODE) {
           return (TRUE); 
       }

       if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
           printf("PoE PSU%d is Present, ", psu_no);

           /* If PoE PSU is present, then check if 
            * output signal is asserted.
            */
           if ((psu_reg->poe_psu_stat) & poe_psu_outpower_mask) {
               printf("and output power is Enabled.\n");
           } else {
               printf("and output power is Disabled.\n");
           }
       }

       /* when the system is juno, sword or dagger, don't try to access
        * PoE PSU micro controller since it will only exist on Overlord and Utah
        */
       if (is_overlord() || is_utah()) {
           /* PoE PSU Fault detect */
           poe_psu_fault_detect(psu_no);
       }

       result = TRUE;
    } else {
       if (option == QUICK_MODE) {
           return (FALSE); 
       }

       printf("PoE PSU%d is NOT present.\n", psu_no);
    }
    return (result);
}


/*******************************************************************************
 *
 * Function   : poe_psu_fault_detect
 * Description: Function to detect PoE PSU status.
 * Inputs     : No. of PoE PSU that will be checked
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t poe_psu_fault_detect (uint32_t psu_no)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     rc = FAILED;
    uint8_t      retry_ctr = 0;
    char         reg_val = 0, data = 0;

    /* Init device structure */
    switch (psu_no) {
    case POE_PSU_ONE:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU1_MCNTRL);
        break;
    case POE_PSU_TWO:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU2_MCNTRL);
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no);
        return (FAILED);
    }

    if (rc != PASSED) {
        return (rc);
    }

#ifdef UTAH
    {
        uint psu_mux_chan, psu_id;
	uint8_t psu_mux_mask = 0;

        if (is_utah() && (psu_no == POE_PSU_ONE)) {
	    /* Utah only has PSU_ONE
	     */
	    psu_mux_chan = OVLD_MUX_PSU2;
	    psu_id = psu_mux_chan - OVLD_MUX_PSU0;

	    /* Get Mux mask */
	    psu_mux_mask = get_mux_mask(psu_id);
	    if (psu_mux_mask == 0) {
	        cterr('f', 0, "%s:%d Failed to get Mux mask (rc = %#x).",
		      __FUNCTION__, __LINE__, rc);
		return (rc);
	    }

	    /* Setup Mux channel */
	    rc = set_mux_channel(NULL, psu_mux_mask, OVLD_PSU_I2C_MUX);
	    if (rc != PASSED) {
	        cterr('f', 0, "%s:%d Failed to set Mux channel (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
		return (rc);
	    }
	}
    }
#endif //UTAH

    /* Get Fault Register value */
    i2c_if.offset = FAULT_REG_OFFSET;
    i2c_if.buf = &reg_val;

    /* Based on HW team's request, try to clean-up FAULT register 
     * first after system power cycle.
     */
    if (first_check == TRUE) {
        for (retry_ctr = 0; retry_ctr < POE_PSU_MAX_RETRY; retry_ctr++) {
            rc = get_reg_value(&i2c_if);
            if (rc != PASSED) {
                printf("%s%d: *** FAILED to read out Fault Register value 1st time.\n",
                       __FUNCTION__, __LINE__);
                return (FAILED);
            }

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("[%d] reg_val = %#x.\n", retry_ctr, reg_val);
            }

            if (reg_val == POE_PSU_NO_FAULT) {
                break;
            }

            data = POE_PSU_NO_FAULT;
            i2c_if.buf = &data;
            if ((rc = write_reg(&i2c_if)) != PASSED) {
                return (FAILED);
            }
        }
    }

    i2c_if.buf = &reg_val;
    rc = get_reg_value(&i2c_if);
    if (rc != PASSED) {
        printf("%s%d: *** FAILED to read out Fault Register value.\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("reg_val = %#x.\n", reg_val);
    }

    /* Report the detected Faults */
    if (reg_val) {
        printf("***PoE PSU%d Fault detection report:\n", psu_no);
        if (reg_val & FET_SHORT_OCCUR) {
            printf("FET Short Fault occurred.\n");
        }

        if (reg_val & POWER_BAD_OCCUR) {
            printf("Power Bad Fault occurred.\n");
        }

        if (reg_val & OVERCUR_OCCUR) {
            printf("Overcurrent Fault occurred.\n");
        }

        if (reg_val & UNDERVOLT_OCCUR) {
            printf("Undervoltage Fault occurred.\n");
        }

        if (reg_val & OVERVOLT_OCCUR) {
            printf("Overvoltage Fault occurred.\n");
        }
        printf("\n");
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : poe_psu_status_wrap
 * Description: Function wrap to show PoE PSU status.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t poe_psu_status_wrap (void)
{
    uint32_t rc = FAILED;

    rc = check_poe_psu_present(psu_no_now, FULL_MODE);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : dump_all_regs
 * Description: Dump all of 12V PoE PSU controller(LTC4280) registers.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_all_regs (void)
{
    n2g_i2c_if_t i2c_if;
    uint32_t     ctr = 0;
    int          rc = FAILED;
    char        reg_val[POE_PSU_BUF_SIZE], data = 0;
    reg_info_t   *reg_info_p;

    /* Init device structure */
    switch (psu_no_now) {
    case POE_PSU_ONE:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU1_MCNTRL);
        break;
    case POE_PSU_TWO:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU2_MCNTRL);
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no_now);
        return (FALSE);
    }

    if (rc != PASSED) {
        return (rc);
    }

    /* Reset data buffer */
    memset(reg_val, 0, sizeof(reg_val));


    /* Get Registers value */
    for (ctr = 0; ctr < POE_PSU_BUF_SIZE; ctr++) {
        i2c_if.offset = ctr;
        i2c_if.buf = &data;

        if (is_ntpn_machines() || is_vg450()) {
            i2c_if.mux = I2C_MUX_ZERO;
        }

        rc = get_reg_value(&i2c_if);
        if (rc != PASSED) {
            return (FAILED);
        }

        reg_val[ctr] = data;
        data = 0;
    }

    /* Dump Registers value */
    reg_info_p = &ltc4280_default_reg_tbl[0];

    printf("\n12V PoE PSU%d registers:\n", psu_no_now);
    for (ctr = 0; ctr < POE_PSU_BUF_SIZE; ctr++, reg_info_p++) {
         printf("%-32s = 0x%.2X\n", reg_info_p->name, reg_val[ctr]);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : get_reg_value
 * Description: Get expected 12V PoE PSU register value
 * Inputs     : *i2c_if - pointer of i2c interface data structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_reg_value (n2g_i2c_if_t *i2c_if)
{
    int rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"vm_write: buf is null");
    }

    /* Get Registers value */
    rc = n2g_i2c_read(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        /* Unable to read data */
        printf("%s:%d Failed to read %s register 0x%02x(rc = %#x).\n",
               __FUNCTION__, __LINE__, i2c_if->dev_name, i2c_if->offset, rc);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : write_reg
 * Description: Write the expected data to 12V PoE PSU register
 * Inputs     : *i2c_if - pointer of i2c interface data structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int write_reg (n2g_i2c_if_t *i2c_if)
{
    uint32_t rc = FAILED;

    if (!i2c_if->buf) {
        assert(!"vm_write: buf is null");
    }

    rc = n2g_i2c_write(i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("%s:%d Failed to write %s to %s register 0x%02x(rc = %#x).\n",
               __FUNCTION__, __LINE__, i2c_if->buf, i2c_if->dev_name,
               i2c_if->offset, rc);
        msleep(REN_I2C_PROC_TIME);      /* Env MCU I2C cycle time */
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);  /* Env MCU I2C cycle time */
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : alter_reg
 * Description: Alter the expected 12V PoE PSU Register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int alter_reg (void)
{
    uint32_t     rc = FAILED, i, offset;
    reg_info_t   *reg_table_p;
    uint8_t      tmp_mask;
    char         data = 0;
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API parameter struct */
    printf("\n12V PoE PSU%d Writable Reg. number:\n", psu_no_now);
    for (i = 0, reg_table_p = &ltc4280_default_reg_tbl[0];
                i < (sizeof(ltc4280_default_reg_tbl) / sizeof(reg_info_t));
                i++, reg_table_p++) {
        if (!(reg_table_p->type & READ_ONLY)) {
            /* Read writeable */
            printf("   %02x - %s\n", reg_table_p->offset,
                                     reg_table_p->name);
        }
    }

    offset = gethex_answer("Enter the register number:", 0, 0,
                           (sizeof(ltc4280_default_reg_tbl) / sizeof(reg_info_t)) - 1);

    /* Got the register offset. Check if writeable */
    for (i = 0, reg_table_p = &ltc4280_default_reg_tbl[0];
                i < (sizeof(ltc4280_default_reg_tbl) / sizeof(reg_info_t));
                i++, reg_table_p++) {
        if (reg_table_p->offset != offset) {
            continue;
        }

        /* Found the offset */
        if (reg_table_p->type & READ_ONLY) {
            /* read only */
            cterr('f', 0, "Read only register");
            return (FAILED);
        }

        /* Valid offset and writeable */
        tmp_mask = reg_table_p->mask; /* get the mask for user to alter reg. */
        break;
    }

    /* Get the original value of expected reg. */
    /* Init device structure */
    switch (psu_no_now) {
    case POE_PSU_ONE:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU1_MCNTRL);
        break;
    case POE_PSU_TWO:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU2_MCNTRL);
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no_now);
        return (FALSE);
    }

    if (rc != PASSED) {
        return (rc);
    }

    i2c_if.offset = offset;
    i2c_if.buf = &data;

    if (is_ntpn_machines() || is_vg450()) {
        i2c_if.mux = I2C_MUX_ZERO;
    }

    if ((rc = get_reg_value(&i2c_if)) != PASSED) {
        return (FAILED);
    }

    data = gethex_answer("Enter the 16-bit data:", data, 0, tmp_mask);

    i2c_if.buf = &data;
    if ((rc = write_reg(&i2c_if)) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : poe_psu_pwr_wrap
 * Description: Function to Enable/Disable PoE PSU power.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t poe_psu_pwr_wrap (void) {
    uint32_t rc = FAILED, option = 1;

    option = getdec_answer("\n[0]:Disable or [1]:Enable 12V PoE PSU:", 1, 0, 1);

    if (option) {
        rc = ovld_poe_psu_pwr_control(ENABLE);
    } else {
        rc = ovld_poe_psu_pwr_control(DISABLE);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : ovld_check_poe_psu_wrap
 * Description: Function wrap to check all PoE PSUs' status on Overlord.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
uint32_t ovld_check_poe_psu_wrap (void)
{
    uint32_t rc = FAILED, test_ctr = 0;
    uint32_t max_poe_psu = MAX_NUM_POE_PSU;

    if (is_usd_machines()) {
        max_poe_psu = POE_PSU_ONE;
    }

    for (test_ctr = POE_PSU_ONE; test_ctr <= max_poe_psu; test_ctr++) {

        first_check = TRUE;

        rc = check_poe_psu_present(test_ctr, FULL_MODE);

        first_check = FALSE;
    }
    return (rc);
}


/*******************************************************************************
 *
 * Function   : poe_psu_reg_test
 * Description: Test read/writeable registers.
 * Inputs     : option - for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t poe_psu_reg_test (uint32_t option)
{
    uint32_t     rc = FAILED, reg_ctr = 0, ctr = 0;
    char         original_data = 0, reg_data = 0, chk_data = 0;
    uint8_t      test_ctr = 0, temp = 0, data = 0, mask = 0;
    n2g_i2c_if_t i2c_if;
    reg_info_t   *reg_ptr;
    char         err_buf[ERR_BUF_SIZE];

    testname("12V PoE PSU Registers");

    /* Init device structure */
    switch (psu_no_now) {
    case POE_PSU_ONE:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU1_MCNTRL);
        break;
    case POE_PSU_TWO:
        rc = get_poe_psu_i2c_struct(&i2c_if, POE_PSU2_MCNTRL);
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no_now);
        return (FALSE);
    }

    prpass(testpass, "PoE PSU%d ", psu_no_now);

    if (rc != PASSED) {
        return (rc);
    }

    if (is_ntpn_machines() || is_vg450()) {
        i2c_if.mux = I2C_MUX_ZERO;
    }

    /* Register test */
    for (reg_ctr = 0, reg_ptr = &reg_test_tbl[0];
         reg_ctr < (sizeof(reg_test_tbl) / sizeof(reg_info_t));
         reg_ctr++, reg_ptr++) {

         if ((reg_ptr->type & (READ_ONLY | WRITE_ONLY)) == READ_WRITE) {
            i2c_if.offset = reg_ptr->offset;
            i2c_if.buf = &original_data;

            /* check result of read data */
            rc = get_reg_value(&i2c_if);
            if (rc != PASSED) {
                sprintf(err_buf, "Ripple one FAILED to read from %s.",
                        reg_ptr->name);
                break;
            }

            /*
             * Ripple 1 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(i2c_if.size)*8); test_ctr++) {
                temp = (1 << test_ctr);
                if (!temp) {
                    continue;
                }

                reg_data = temp;
                i2c_if.buf = &reg_data;

                rc = write_reg(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple one FAILED on write to %s.",
                            reg_ptr->name);
                    break;
                }

                msleep(500);

                i2c_if.buf = &chk_data;
                rc = get_reg_value(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple one FAILED on read back %s.",
                            reg_ptr->name);
                    break;
                }

                if ((chk_data & 0xFF) != (temp & 0xFF)) {
                    rc = FAILED;
                    sprintf(err_buf, "%s Reg. Ripple one test FAILED, "
                                     "read back = 0x%02x and expected = 0x%02x.",
                                     reg_ptr->name, chk_data, temp);
                    break;
                }
            }

            /* leave the for loop of reg_ptr */
            if (rc != PASSED) {
                break;
            }

            /*
             * Ripple 0 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(i2c_if.size) * 8); test_ctr++) {
                temp = (1 << test_ctr);
                if (!temp) {
                    continue;
                }

                temp = (~(1 << test_ctr));

                reg_data = temp;
                i2c_if.buf = &reg_data;

                rc = write_reg(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple zero FAILED on write to %s.",
                            reg_ptr->name);
                    break;
                }

                msleep(500);

                i2c_if.buf = &chk_data;
                rc = get_reg_value(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Ripple zero test FAILED on read back"
                                     " from %s.", reg_ptr->name);
                    break;
                }

                if ((chk_data & 0xFF) != (temp & 0xFF)) {
                    rc = FAILED;
                    sprintf(err_buf, "%s Reg. Ripple zero test FAILED, read back"
                                     " = 0x%02x, and expected = 0x%02x.",
                                     reg_ptr->name, chk_data, temp);
                    break;
                }
            }

            /* leave the for loop of reg_ptr*/
            if (rc != PASSED) {
                break;
            }

            /*
             * Pattern test
             */
            data = (uint8_t)POE_PSU_PATTERN;

            for (ctr = 0; ctr < 2; ctr++) {
                /* build mask of size for pattern */
                for (test_ctr = 0; test_ctr < (sizeof(i2c_if.size)*8); test_ctr++) {
                    mask |= (1 << test_ctr);
                }
                temp = data & mask;
                if (!temp) {
                    continue;
                }

                reg_data = temp;
                i2c_if.buf = &reg_data;

                rc = write_reg(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Pattern test FAILED on write to %s.",
                            reg_ptr->name);
                    break;
                }

                msleep(500);

                i2c_if.buf = &chk_data;
                rc = get_reg_value(&i2c_if);
                if (rc != PASSED) {
                    sprintf(err_buf, "Pattern test FAILED to read back %s.",
                            reg_ptr->name);
                    break;
                }

                if ((chk_data & 0xFF) != (temp & 0xFF)) {
                    rc = FAILED;
                    sprintf(err_buf, "%s Reg. Pattern test FAILED, read back "
                                     "= 0x%04x, and expected = 0x%04x.",
                                     reg_ptr->name, chk_data, temp);
                    break;
                }

                data = (uint8_t)(~POE_PSU_PATTERN); /* complemrent data pattern */
            } /* for (ix = 0; ix < 2; ix++) */

        /* leave the for loop of reg_ptr */
        if (rc != PASSED) {
            break;
        }

        /*
         * Restore Reset value
         */
        i2c_if.buf = &original_data;

        rc = write_reg(&i2c_if);
        if (rc != PASSED) {
            sprintf(err_buf, "Restore value FAILED %s.", reg_ptr->name);
            break;
        }

    } /*if ((reg_table_p->type & (READ_ONLY | WRITE_ONLY))*/
    } 

    if (rc != PASSED) {
        cterr('f', 0, err_buf);
    } /* endof if rc */

    if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
        printf("passed.\n");
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : has_poe_psu
 * Description:	Check if PoE PSU is present.
 * Inputs     : psu_no - PSU number that want to check
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
boolean has_poe_psu (uint32_t psu_no)
{
    /* Check PoE PSU present, Poe PSU module can not be used in Juno */
    if ((check_poe_psu_present(psu_no, QUICK_MODE) == TRUE) && !is_juno()) {
        return (TRUE);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   : has_poe_psu_pwr_ctrl
 * Description:	Check if system support PoE PSU power control.
 * Inputs     : NONE 
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
boolean has_poe_psu_pwr_ctrl (void)
{
    /* Neptune doesn't has ability and not necessary to control POE PSU power by MCU */
    if (is_ntpn_machines() || is_vg450()) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}


/*******************************************************************************
 *
 * Function   : poe_psu_pwr_en_test
 * Description:	Do 12V PoE PSU power enable/disable test.
 * Inputs     : option - for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32_t poe_psu_pwr_en_test (uint32_t option)
{
    uint32_t poe_psu_outpower_mask = 0;
    psu_t    *psu_reg;

    testname("12V PoE PSU Power Enable");

    /* Set related parameters based on input PSU no. */
    switch (psu_no_now) {
    case POE_PSU_ONE:
        poe_psu_outpower_mask = POE_PSU1_OUT_OK_MSK;
        break;
    case POE_PSU_TWO:
        poe_psu_outpower_mask = POE_PSU2_OUT_OK_MSK;
        break;
    default:
        printf("*** %s:%d Got Unknown PoE PSU no.(%d)\n",
               __FUNCTION__, __LINE__, psu_no_now);
        return (FALSE);
    }

    prpass(testpass, "PoE PSU%d ", psu_no_now);

    /* Read FPGA  */
    psu_reg = (psu_t *)get_platform_ps_env_base();

    /* 1. Disable PoE PSU power */
    if (ovld_poe_psu_pwr_control(DISABLE) != PASSED) {
        cterr('f', 0, "%s:%d Failed to disable PoE PSU power",
              __FUNCTION__, __LINE__);
        return (FAILED);
    }

    msleep(POE_PSU_WAIT_TIME);

    /* 2. Read FPGA PoE PSU status reg.(+0x20) to see
     *    if PoE PSU power is disabled.
     */
    if ((psu_reg->poe_psu_stat) & poe_psu_outpower_mask) {
        cterr('f', 0, "%s:%d PoE PSU power is not expected"
                      "(PoE PSU power is enabled now)",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* 3. Enable PoE PSU power */
    if (ovld_poe_psu_pwr_control(ENABLE) != PASSED) {
        cterr('f', 0, "%s:%d Failed to enable PoE PSU power",
              __FUNCTION__, __LINE__);
        return (FAILED);
    }

    msleep(POE_PSU_WAIT_TIME);

    /* 4. Read FPGA PoE PSU status reg.(+0x20) to see
     *    if PoE PSU power is enabled.
     */
    if (!((psu_reg->poe_psu_stat) & poe_psu_outpower_mask)) {
        cterr('f', 0, "%s:%d PoE PSU power is not expected"
                      "(PoE PSU power is disabled now)",
                      __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
        printf("passed.\n");
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: platform_poe_psu.c,v $
Revision 1.13  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.12  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.11.28.3  2018/01/25 08:17:34  leschen
Change PSU and POE_PSU FPGA mux to zero.

Revision 1.11.28.2  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.11.28.1  2017/08/03 02:57:21  leschen
Support Neptune POE PSU utility.

Revision 1.11  2014/11/13 11:34:49  danchung
CSCur67546:Provide the function to detect the POE ability of platform for
           ngio module

Revision 1.10  2014/07/16 11:08:48  danchung
fix the warning message on sword with new AC+IP POE PSU

Revision 1.9  2014/06/27 08:04:49  danchung
Don't try to access PoE PSU module when the AC+IP supply is plugged in Juno

Revision 1.8  2014/03/06 06:24:46  hroni
before displaying menu of alter poe psu cookie, notify user USD platform does not support altering cookie data.

Revision 1.7  2014/01/22 10:32:52  hroni
fix Utah POE PSU access

Revision 1.6  2013/12/05 22:52:52  ptong
Bump version to 3.2. Fixed Utah PSU, POE support and PCIe bus change dur to ROMMON UTAHBRINGUP4

Revision 1.5  2013/12/04 21:43:49  ptong
Fix PSU and POE I2C scan issue on Utah

Revision 1.4  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.3  2013/07/18 17:17:02  mcharon
add -Wal and clean up compile warnings

Revision 1.2  2013/06/14 09:50:47  hroni
1. support to PSU mux
2. temporarily uses #ifdef MUX124 to turn off/on PCA9545 mux support

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.13  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.12  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.11  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.10  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.9  2012/05/04 19:50:53  mcharon
add declaration for check_poe_psu_presnet to fix compile err

Revision 1.8  2012/05/04 08:03:32  alpeng
skip Max1617, check PoE and PoE PSU is present before I2C scan test

Revision 1.7  2012/04/25 02:01:50  palin2
Add code to clean-up FAULT register before first read based on HW's request.

Revision 1.6  2012/04/17 16:37:47  palin2
Add 12V PoE PSU power enable test.

Revision 1.5  2012/04/17 14:14:06  palin2
Add 12V PoE PSU cookie utility support.

Revision 1.4  2012/04/16 15:29:26  palin2
Update 12V PoE PSU tests and utilities based on HW team's request:
1) Add "Registers test" support.
2) Add "PoE PSU" info into bootlog message.
3) Add utility to verified FPGA related PoE PSU detect function.

Revision 1.3  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/

/*------------------------------------------------------------------
 * Filename:    platform_psu.c
 *
 * Description: PSU I2C device.
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "nvmonvars.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "cli_cmd.h"
#include "diag_fpga.h"
#include "diag_i2c_addr.h"
#include "dev_at24c0n.h"
#include "linux_main.h"
#include "platform_psu.h"
#include "i2c_dev.h"
#include "diag_fpga_i2c_lib.h"


/******************************************************************************
 *                            Marco                                           *
 ******************************************************************************/
#define PSU_INTR_CHK_RETRY_TIMES 10
#define PSU_INTR_CHK_WAIT_TIME 200 /* 200 ms */

#define PSU_REG_CLR_FAULTS 0x3 /* Clear PSU alert interrupt */
#define PSU_REG_INVALID    0x4 /* Unsupported register address to trigger
                                  PSU alert interrupt */

#define PSU_READ_TEMPERATURE_1 0x8D
#define PSU_READ_TEMPERATURE_2 0x8E
#define PSU_READ_TEMPERATURE_3 0x8F
#define PSU_READ_FAN_SPEED_1 0x90

/****************************************************************************** 
 *                            Function Prototypes                             * 
 ******************************************************************************/
static int psu_pr_test (int);
extern int cookie_4_processor_x(uchar *, int, int, int, cli_cookie_cmd *);
static int psu_read_reg (int, uint32_t, uint16_t, char *);
static int show_psu_reg (void);
static int dump_psu_regs (void);
static int psu_intr_test (int);


/****************************************************************************** 
 *                             Global Variables                               * 
 ******************************************************************************/


/*
 * PSU Menu
 */
static submenu_xtable_t psu_menu_table[] = {
    {"PSU cookie utility",          (PFT)show_psu_cookie, 0,
        0,                          (PFT)0, 0, (PFT)0, 0},

    {"Dump PSU registers",          (PFT)dump_psu_regs, 0,
        0,                          (PFT)0, 0, (PFT)0, 0},

    {"Show PSU register",           (PFT)show_psu_reg, 0,
        0,                          (PFT)0, 0, (PFT)0, 0},
};

static submenu_xtable_t psu_test_menu_table[] = {
    {"PSU Pins Test",               (PFT)psu_pr_test, 0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL), (PFT)0, 0, (PFT)0, 0},

    {"PSU Interrupt Test",          (PFT)psu_intr_test, 0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL), (PFT)0, 0, (PFT)0, 0},
};

#define PSU_MENU_TABLE_SIZE (sizeof(psu_menu_table) / \
        sizeof(submenu_xtable_t))

#define PSU_TEST_MENU_TABLE_SIZE (sizeof(psu_test_menu_table) / \
        sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t psu_menu_primary_items[PSU_MENU_TABLE_SIZE +
       MAX_BASE_ITEMS];
static mitem_t psu_menu_secondary_items[PSU_MENU_TABLE_SIZE +
       MAX_BASE_ITEMS];
static mitem_t psu_test_menu_primary_items[PSU_TEST_MENU_TABLE_SIZE +
       MAX_BASE_ITEMS];
static mitem_t psu_test_menu_secondary_items[PSU_TEST_MENU_TABLE_SIZE +
       MAX_BASE_ITEMS];

static struct menuinfo psudiag = {
    "PSU Utility Menu",     /* title */
    0,              /* title string added by init_empty_menu */
    0,              /* do not show major flags */
    0,              /* generic prompt */
    0,              /* size -- bumped by add_menu_item() */
    psu_menu_primary_items,
};

static struct menuinfo psu_test_menu = {
    "PSU Test Menu",     /* title */
    0,              /* title string added by init_empty_menu */
    0,              /* do not show major flags */
    0,              /* generic prompt */
    0,              /* size -- bumped by add_menu_item() */
    psu_test_menu_primary_items,
};

static struct menuinfo *psudiagp = &psudiag;
static struct menuinfo *psutest_menup = &psu_test_menu;

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
        case PSU0_EEPROM:
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                                 MB_I2C_ADDR_PSU_EEPROM);
            break;

        case PSU0_MCNTRL:
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ZERO,
                                                 MB_I2C_ADDR_PSU_MCCTLR);
            break;

        case PSU1_EEPROM:
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ONE,
                                                 MB_I2C_ADDR_PSU_EEPROM);
            break;
        case PSU1_MCNTRL:
            tmp = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_FOUR, I2C_MUX_ONE,
                                                 MB_I2C_ADDR_PSU_MCCTLR);
            break;
        default:
            printf("%s:%d Unknown PSU related device no: %d.\n",
                   __func__, __LINE__, psu_type);
            return (FAILED);
    }

    if (tmp == NULL) {
        printf("%s:%d Failed to get PSU %d I2C interface structure.\n",
               __func__, __LINE__, psu_type);
        return (FAILED);
    }

    memcpy(i2c_if, tmp, sizeof(n2g_i2c_if_t));

    return (PASSED);
}


/*******************************************************************************
 *
 * function   : build_psu_test_menu
 * Description:	Build test menu for PSU.
 * Inputs     : option
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_psu_test_menu (uint32_t menu_opt)
{
    build_primary_submenu(psu_test_menu_table, PSU_TEST_MENU_TABLE_SIZE,
			  "PSU Test Menu", &psutest_menup);
    build_secondary_submenu(psu_test_menu_table, PSU_TEST_MENU_TABLE_SIZE,
			    psu_menu_secondary_items);

    if (menu_opt) {
        /* Entered with submenu */
        menu(&psu_test_menu, psu_test_menu_secondary_items, 0);
    } else {
        do_all_menu_items(psutest_menup);
    }
}


/*******************************************************************************
 *
 * function   : build_psu_util_menu
 * Description:	Build utility menu for PSU.
 * Inputs     : option
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_psu_util_menu (uint32_t option)
{
    build_primary_submenu(psu_menu_table, PSU_MENU_TABLE_SIZE,
			  "PSU Utility Menu", &psudiagp);
    build_secondary_submenu(psu_menu_table, PSU_MENU_TABLE_SIZE,
			    psu_menu_secondary_items);

    menu(&psudiag, psu_menu_secondary_items, 0);
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
    uint32_t     psu_no;
    uint16_t     psu_id = 0;
    n2g_i2c_if_t i2c_if;

    psu_no = gethex_answer("Select PSU: 0/PSU0, 1/PSU1: ", 0, 0, 1);

    /* Check PSU present */
    if (check_psu_present(psu_no) != TRUE) {
        return;
    }

    /* Setup I2C API parameter struct */
    switch (psu_no) {
    case PSU_ZERO:
        rc = get_psu_i2c_struct(&i2c_if, PSU0_EEPROM);
        break;
    case PSU_ONE:
        rc = get_psu_i2c_struct(&i2c_if, PSU1_EEPROM);
        break;
    default:
        cterr('f', 0, "%s: Got Unknown PSU no.(%d)\n",
                      __func__, psu_no);
        return;
    }

    if (rc != PASSED) {
        return;
    } 
    
    i2c_if.buf = (char *)&psu_id;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = 0xF;

    /* Read the cookies */
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to read PSU%d cookie (rc = %#x).",
                      __func__, psu_no, rc);
	return;
    }

    printf("%s: PSU%d ID = 0x%04x.\n", __func__, psu_no, psu_id);
}


/*******************************************************************************
 *
 * Function   : show_psu_cookie
 * Description:	Display PSU Cookie Contents.
 * Inputs     : opt -- not used.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_psu_cookie (int opt)
{
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
    int psu_no, ix;

    psu_no = gethex_answer("Select PSU: 0/PSU0, 1/PSU1: ", 0, 0, 1);

    /* Check PSU present */
    if (check_psu_present(psu_no) != TRUE) {
        return (FAILED);
    }

    /* Reset data buffer */
    memset(data, 0, sizeof(data));

    /* For menu mode already check PSU is present or not */ 
    if (mode == CLI_MODE) {
        psu_no = cmd->slot;

        /* Rule 1 - Make sure PSU present */
        if (check_psu_present(psu_no) != TRUE){
            return CLI_DEVICE_IS_VACANT;
        }
        /* Rule 2 - If Input OK and 12V Output OK must be in the same stats */
        if (check_psu_stat(psu_no) != TRUE) {
            return CLI_DEVICE_IS_VACANT;
        }
    }

    /* Setup I2C API parameter struct */
    switch (psu_no) {
    case PSU_ZERO:
            rc = get_psu_i2c_struct(&i2c_if, PSU0_EEPROM);
        break;
    case PSU_ONE:
            rc = get_psu_i2c_struct(&i2c_if, PSU1_EEPROM);
        break;
    default:
        cterr('f', 0, "%s: Got Unknown PSU number (%d).",
                      __func__, psu_no);
        return (FAILED);
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
              __func__, psu_no, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    /* Call cookie utility */
    if (cookie_4_processor_x(data, PSU_MODULE, 0, i2c_if.size, cmd) == 1) {
        /* Updated. Write them back */
        /* Do not use the block write, since EEPROM need wait between bytes */
        i2c_if.size = sizeof(at_t);	/* write 1 byte at a time */
        for (ix = 0; ix < sizeof(data); ix++) {
            i2c_if.offset = ix;
            i2c_if.buf = (char *)&data[ix];
            rc = n2g_i2c_write(&i2c_if);
            if (rc != PASSED) {
                cterr('f', 0, "%s: PSU%d failed to write %#x @ %#x. rc = %#x",
                      __func__, psu_no, data[ix], ix, rc);
                return(FAILED);
            } /* endof rc */
            msleep(AT24C0X_T_WR + 1);	/* Wait for write to complete */
        } /* endof for */
    } /* endof of cookie_4_processor_x */

    if (rc != PASSED) {
        cterr('f', 0, "%s: PSU%d show cookie Failed.",
              __func__, psu_no);
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
    uint32_t result = FALSE, psu_present_mask = 0;
    psu_t    *psu_reg;

    /* Set related parameters based on input PSU no. */

    switch (psu_no) {
        case PSU_ZERO:
            psu_present_mask = PSU1_PRESENT_MSK;
            break;
        case PSU_ONE:
            psu_present_mask = PSU2_PRESENT_MSK;
            break;
        default:
            cterr('f', 0, "%s: Got Unknown PSU number (%d).",
                      __func__, psu_no);
            return (FALSE);
    }

    /* Read FPGA  */
    psu_reg = (psu_t *)get_platform_ps_env_base();

    /* Check if PSU is present */
    if ((psu_reg->psu_stat) & psu_present_mask) {
       result = TRUE;
    } else {
       printf("%s: PSU%d device is NOT present.\n",
                     __func__, psu_no);
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
        case PSU_ZERO:
            psu_ac_in_mask = PSU1_AC_IN_OK;
            psu_12v_out_mask = PSU1_12V_OUT_OK;
            break;
        case PSU_ONE:
            psu_ac_in_mask = PSU2_AC_IN_OK;
            psu_12v_out_mask = PSU2_12V_OUT_OK;
            break;
        default:
            cterr('f', 0, "%s:%d Got Unknown PSU num (%d).",
                      __func__, __LINE__, psu_no);
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


/*******************************************************************************
 *
 * Function:	psu_pr_test
 * Description:	PSU Presence checking. Refer to CSCsz33018 for more info
 *		The rules of checking -
 *		1 - PSU must be present.
 *		2 - PSU Input OK and 12V Output OK must be in the same state.
 * Inputs     : opt -- not used.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_pr_test (int opt)
{
    int rc = PASSED;
    char err_buf[PHOENIX_INFO_BUF_SIZE];
    uint32_t psu_no;

    testname("PSU Presence & Stat Detection");

    for (psu_no = 0; psu_no < MAX_NUM_PSU; psu_no++) {
        rc = PASSED;
        prpass(testpass, "PSU%d ", psu_no);

        /* Rule 1 - Make sure PSU present */
        if (check_psu_present(psu_no) != TRUE) {
            sprintf(err_buf, "%s: PSU%d is not present.\n",
                             __func__, psu_no);
            rc = FAILED;
            goto pr_test_fail;
        }

        /* Rule 2 - If Input OK and 12V Output OK must be in the same stats */
        if (check_psu_stat(psu_no) != TRUE) {
            sprintf(err_buf, "%s: PSU%d is not in right stat.\n",
                             __func__, psu_no);
            rc = FAILED;
        }

pr_test_fail:
        if (rc != PASSED) {
            cterr('f', 0, &err_buf[0]);
        } else {
            if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
                printf("passed.\n");
            }
        }
        prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : psu_write_reg
 * Description: To write register of PSU MicroController.
 * Inputs     : offset - offset of register want to read
 *              size   - size of register want to read
 *              data   - buffer to write value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_write_reg (int psu_no, uint32_t offset, uint16_t size, char *data)
{
    n2g_i2c_if_t i2c_if;
    int result = FAILED;

    /* Get PSU I2C interface structure */
    switch (psu_no) {
        case PSU_ZERO:
            result = get_psu_i2c_struct(&i2c_if, PSU0_MCNTRL);
            break;
        case PSU_ONE:
            result = get_psu_i2c_struct(&i2c_if, PSU1_MCNTRL);
            break;
        default:
            printf("%s:%d Unsupported PSU no.(%d)\n",
                   __func__, __LINE__, psu_no);
            return (FAILED);
    }

    if (result != PASSED) {
        printf("%s: Failed to get PSU%d Microcontroller I2C structure.\n",
               __func__, psu_no);
        return (FAILED);
    }

    /* Setup I2C API parameter struct */
    i2c_if.buf = data;
    i2c_if.size = size;
    i2c_if.offset = offset;

    result = n2g_i2c_write(&i2c_if);
    if (result != RC_I2C_OP_OK) {
        /* Unable to write data */
        printf("%s:%d Failed to read Reg. %#x (result = %#x).",
                __func__, __LINE__, i2c_if.offset, result);
        return (FAILED);
    }

    return (PASSED);
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
static int psu_read_reg (int psu_no, uint32_t offset, uint16_t size, char *data)
{
    n2g_i2c_if_t i2c_if;
    int result = FAILED;

    /* Get PSU I2C interface structure */
    switch (psu_no) {
        case PSU_ZERO:
            result = get_psu_i2c_struct(&i2c_if, PSU0_MCNTRL);
            break;
        case PSU_ONE:
            result = get_psu_i2c_struct(&i2c_if, PSU1_MCNTRL);
            break;
        default:
            printf("%s:%d Unsupported PSU no.(%d)\n",
                   __func__, __LINE__, psu_no);
            return (FAILED);
    }

    if (result != PASSED) {
        printf("%s: Failed to get PSU%d Microcontroller I2C structure.\n",
               __func__, psu_no);
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
                __func__, __LINE__, i2c_if.offset, result);
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
    uint32_t ctr = 0, offset = 0, p_ctr = 0, psu_regs_tbl_size = 0;
    uint16_t size = 0, w_data = 0;
    uint8_t b_data = 0;
    psu_reg_info_t *psu_reg_p;
    char buf[32], *buffer;
    int psu_no;

    psu_no = gethex_answer("Select PSU: 0/PSU0, 1/PSU1: ", 0, 0, 1);

    /* Check PSU present */
    if (check_psu_present(psu_no) != TRUE) {
        return (FAILED);
    }
    
    psu_reg_p = &psu_regs_tbl[0];
    psu_regs_tbl_size = (sizeof(psu_regs_tbl) / sizeof(psu_reg_info_t));

    /* Dump all registers listed in Table */ 
    printf("\nPSU %d MicroController Registers:\n", psu_no);
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
                          __func__, psu_no, psu_reg_p->type);
                return (FAILED);
        }

        if (psu_read_reg(psu_no, offset, size, buffer) != PASSED) {
            cterr('f', 0, "%s: Unable to read PSU%d Regiser 0x%02X.",
                          __func__, psu_no, psu_reg_p->code);
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
    int psu_no;

    psu_no = gethex_answer("Select PSU: 0/PSU0, 1/PSU1: ", 0, 0, 1);

    /* Check PSU present */
    if (check_psu_present(psu_no) != TRUE) {
        return (FAILED);
    }
    
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
                              __func__, psu_no, psu_reg_p->type);
                return (FAILED);
            }
            break;
        }
    }

    /* Read PSU MicroController register */
    if (psu_read_reg(psu_no, reg_addr, size, buffer) != PASSED) {
        cterr('f', 0, "%s: Unable to read PSU%d MicroController Reg. 0x%02X.",
                      __func__, psu_no, reg_addr);
        return (FAILED);
    }

    printf("\nPSU%d MicroController 0x%02X = ", psu_no, reg_addr);
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


/*******************************************************************************
 *
 * Function   : force_psu_interrupt
 * Description:	force psu to assert interrupt.
 * Inputs     : No. of PSU that will assert interrupt
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int force_psu_interrupt(int psu)
{
    char data = 0x0;
    return psu_write_reg(psu, PSU_REG_INVALID, 1, &data);
}


/*******************************************************************************
 *
 * Function   : clear_psu_interrupt
 * Description:	clear psu interrupt.
 * Inputs     : No. of PSU that will be clear
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int clear_psu_interrupt(int psu)
{
    char data = 0x0;
    return psu_write_reg(psu, PSU_REG_CLR_FAULTS, 1, &data);
}


/*******************************************************************************
 *
 * Function   : check_psu_interrupt
 * Description:	Return if the power supply asserts interrupt.
 * Inputs     : No. of PSU that will be checked
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
static int check_psu_interrupt(int psu)
{
    psu_t *psu_reg;
    uint32_t mask = 0;

    switch (psu) {
        case PSU_ZERO:
            mask = PSU1_INTERRUPT_STAT;
            break;

        case PSU_ONE:
            mask = PSU2_INTERRUPT_STAT;
            break;

        default:
            cterr('f', 0, "%s: Got Unknown PSU number (%d).",
                      __func__, psu);
            return (FALSE);
    }

    /* Read FPGA  */
    psu_reg = (psu_t *)get_platform_ps_env_base();


    /* Check if PSU asserts interrupt */
    if ((psu_reg->psu_stat) & mask) {
       return (TRUE);
    }

    return (FALSE);
}


/*******************************************************************************
 *
 * Function   : psu_intr_test
 * Description: Check PSU interrupt pin is good or not.
 * Inputs     : opt -- reserved
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int psu_intr_test (int opt)
{
    int retry = 0, psu_no;
    int rc = PASSED;
    char err_buf[PHOENIX_INFO_BUF_SIZE];

    testname("PSU Interrupt Detection");

    for (psu_no = 0; psu_no < MAX_NUM_PSU; psu_no++) {
        rc = PASSED;
        prpass(testpass, "PSU%d ", psu_no);

        /* Make sure PSU present */
        if (check_psu_present(psu_no) != TRUE) {
            sprintf(err_buf, "%s: PSU%d is not present.\n",
                             __func__, psu_no);
            rc = FAILED;
            goto intr_test_fail;
        }

        /* Force to clear interrupt first */
        while (check_psu_interrupt(psu_no)) {
            clear_psu_interrupt(psu_no);
            if (retry++ > PSU_INTR_CHK_RETRY_TIMES) {
                sprintf(err_buf, "%s:%d: Can't clear PSU%d interrupt\n",
                        __func__, __LINE__, psu_no);
                rc = FAILED;
                goto intr_test_fail;
            }
            msleep(PSU_INTR_CHK_WAIT_TIME);
        }

        /* Force PSU interrupt */
        retry = 0;
        while (!check_psu_interrupt(psu_no)) {
            force_psu_interrupt(psu_no);
            if (retry++ > PSU_INTR_CHK_RETRY_TIMES) {
                sprintf(err_buf, "%s:%d: Can't receive PSU%d interrupt\n",
                        __func__, __LINE__, psu_no);
                rc = FAILED;
                goto intr_test_fail;
            }
            msleep(PSU_INTR_CHK_WAIT_TIME);
        }

        /* Clear interrupt */
        retry = 0;
        while (check_psu_interrupt(psu_no)) {
            clear_psu_interrupt(psu_no);
            if (retry++ > PSU_INTR_CHK_RETRY_TIMES) {
                sprintf(err_buf, "%s:%d: Can't clear PSU%d interrupt\n",
                        __func__, __LINE__, psu_no);
                rc = FAILED;
                goto intr_test_fail;
            }
            msleep(PSU_INTR_CHK_WAIT_TIME);
        }

intr_test_fail:
        if (rc == PASSED) {
            if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
                printf("passed.\n");
            }
        } else {
            cterr('f', 0, &err_buf[0]);
        }
        prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : psu_reg_cnvt_data_2bytes
 * Description: Convert 2-bytes register data
 *
 *                Data Byte High    |  Data Byte Low
 *              | 7 6 5 4 3 | 2 1 0 | 7 6 5 4 3 2 1 0 |
 *              |->   N   <-|->         Y           <-|
 *
 *              For example:
 *                0xDB26 = 0b1101101100100110
 *                N = 0b11011 = -5
 *                Y = 0b01100100110 = 806
 *                result = Y*(2^N) = 806*(2^-5) = 25.1875
 *
 * Inputs     : data -- register data
 * Outputs    : None.
 *
 *******************************************************************************
 */
static float psu_reg_cnvt_data_2bytes(uint16_t data)
{
    uint16_t dataN;
    uint8_t valN;
    uint8_t signN;
    float val, valY;

    dataN = (data & 0xF800) >> 11;
    valN = (uint8_t)(0xF & dataN);
    signN = (uint8_t)(0x10 & dataN);
    valY = data & 0x7FF;

    if (signN) {
        valN = (~valN & 0xF) + 1;
        val = valY / (1 << valN);
    } else {
        val = valY * (1 << valN);
    }

    return val;
}


/*******************************************************************************
 *
 * Function   : psu_show_temp_info
 * Description: Show PSU's temperature information.
 * Inputs     : psu_no -- which PSU
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void psu_show_temp_info(int psu_no)
{
    int ix;
    uint32_t offset = PSU_READ_TEMPERATURE_1;
    uint16_t data;
    uint16_t size = sizeof(data);
    float val;

    for (ix = 1; ix <= 3; ix++, offset++) {
        if (PASSED != psu_read_reg(psu_no, offset, size, (char *)&data)) {
            printf("PSU_READ_TEMPERATURE_%d: Invalid\n", ix);
            continue;
        }

        val = psu_reg_cnvt_data_2bytes(data);
        printf("PSU_READ_TEMPERATURE_%d: %.4f Celcius\n", ix, val);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : psu_show_fan_speed
 * Description: Show PSU's fan speed.
 * Inputs     : psu_no -- which PSU
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void psu_show_fan_speed(int psu_no)
{
    uint32_t offset = PSU_READ_FAN_SPEED_1;
    uint16_t data;
    uint16_t size = sizeof(data);
    float val;

    if (PASSED != psu_read_reg(psu_no, offset, size, (char *)&data)) {
        printf("PSU_READ_FAN_SPEED_1: Invalid\n");
        return;
    }

    val = psu_reg_cnvt_data_2bytes(data);
    printf("PSU_READ_FAN_SPEED_1: %u RPM.\n", (uint32_t)val);

    return;
}


/*******************************************************************************
 *
 * Function   : psu_show_env_info
 * Description: Show PSU's environment information.
 * Inputs     : None.
 * Outputs    : None.
 *
 *******************************************************************************
 */
void psu_show_env_info(void)
{
    int psu_no;
    uint32_t present;

    for (psu_no = PSU_ZERO; psu_no < MAX_NUM_PSU ; psu_no++) {
        printf("\nPSU %d:\n", psu_no);

        /* Check PSU present */
        present = check_psu_present(psu_no);
        printf("Present: %s\n", present?"Yes":"No");
        if (present != TRUE) {
            continue;
        }

        /* Show PSU temperature */
        psu_show_temp_info(psu_no);

        /* Show PSU FAN speed */
        psu_show_fan_speed(psu_no);
    }

    return;
}

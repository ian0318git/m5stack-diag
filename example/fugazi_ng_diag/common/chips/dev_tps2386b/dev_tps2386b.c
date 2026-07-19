/* $Id: dev_tps2386b.c,v 1.2 2019/01/10 06:30:25 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_tps2386b/dev_tps2386b.c,v $
 *------------------------------------------------------------------
 * Filename   :	dev_tps2386b.c
 *
 * Copyright (c) 2018 - 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/time.h>
#include "defs.h"
#include "common.h"
#include "dev_tps2386b.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#include "byteswap.h"
#include "nvmonvars.h"

/*******************************************************************************
 *                             Function Prototypes                             *
 ******************************************************************************/
int           tps2386b_dev_create(dev_object_t *, dev_error_report_t);
static uint32 dev_tps2386b_attach(dev_object_t *);
static int    dev_i2c_rd(ulong, int, ulong *, void *);
static int    dev_i2c_wr(ulong, int, ulong, void *);
static uint32 dev_tps2386b_detach(dev_object_t *);
static uint32 dev_tps2386b_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_tps2386b_restart(dev_object_t *);
static uint32 dev_tps2386b_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void   dev_tps2386b_destroy(dev_object_t **);
static int    dev_tps2386b_util_dump_reg(dev_object_t *);
static int    dev_tps2386b_util_read_reg(dev_object_t *);
static int    dev_tps2386b_util_write_reg(dev_object_t *);
static int    dev_tps2386b_test_reg(dev_object_t *);
static int    dev_tps2386b_test_intr(dev_object_t *);
static int    dev_tps2386b_chk_intr_assert(void *);
static int    dev_tps2386b_chk_intr_deassert(void *);
static int    dev_tps2386b_util_detect_pwr (dev_object_t *, int, int);
static int    dev_tps2386b_reset_all_port(dev_object_t *, int, int);
static int    dev_tps2386b_cfg_semi_auto_mode(dev_object_t *);
static int    dev_tps2386b_ena_detection_class(dev_object_t *);
static int    dev_tps2386b_ena_port_pwr(dev_object_t *, int, int);
static int    dev_tps2386b_cfg_plus_mode(dev_object_t *, int, int);
static int    dev_tps2386b_cfg_cut_current(dev_object_t *, int, int);
static int    dev_tps2386b_cfg_ieee_pwr(dev_object_t *, int, int);
static int    dev_tps2386b_util_show_pwr_stat (dev_object_t *, int, int);
static void   dev_tps2386b_show_status_title (void);
static void   dev_tps2386b_clr_port_status(void);
static int    dev_tps2386b_get_port_op_mode(dev_object_t *, int, int);
static int    dev_tps2386b_get_port_detection_class(dev_object_t *, int, int);
static int    dev_tps2386b_get_port_pwr_status(dev_object_t *, int, int);
static int    dev_tps2386b_detect_port_current(dev_object_t *, int, int);
static int    dev_tps2386b_get_port_current(dev_object_t *, int, int);
static int    dev_tps2386b_get_port_voltage(dev_object_t *, int, int);
static int    dev_tps2386b_get_port_power(int, int);
static void   dev_tps2386b_show_port_status(dev_object_t *, int, int);
static void   dbg_dev_tps2386b_show_port_status(dev_object_t *);
static int    dev_tps2386b_polling_reg(dev_object_t *, int, ulong, ulong);

/*******************************************************************************
 *                               Global variables                              *
 ******************************************************************************/
/* Port status struture declaration, supported 4 ports */
static tps2386b_port_status_t port_status[TPS2386B_ALL_4PORT];

/* Registers test table */
static reg_info_t_ext tps2386b_reg_ext = {TPS2386B_REG_ONE_BYTE_ACCESS,
                                          dev_i2c_rd, dev_i2c_wr, 0};

/* TPS2386B register test table */
static reg_info_t tps2386b_reg_test_table[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    /* Interrupt Mask: 0x1 */
    {"Interrupt Mask", TPS2386B_INTR_MASK_REG, TPS2386B_REG_RW_FLAG,
     {(unsigned long)&tps2386b_reg_ext}, REG_MASK_INTR_MASK, INI_VAL_INTR_MASK},
    /* Operating Mode: 0x12*/
    {"Operating Mode", TPS2386B_OP_MODE_REG,
     TPS2386B_REG_RW_FLAG,
     {(unsigned long)&tps2386b_reg_ext}, REG_MASK_OP_MODE, INI_VAL_OP_MODE},
    {"END", END_VAL, END_VAL, {END_VAL}, END_VAL, END_VAL},
};

/* TPS2386B Register Map */
static tps2386b_reg_info_t tps2386b_reg_dump_table[] = {
    {"Interrupt",                     TPS2386B_INTR_REG,            READ_ONLY},
    {"Interrupt Mask",                TPS2386B_INTR_MASK_REG,       READ_WRITE},
    {"Power Event",                   TPS2386B_PWREVENT_REG,        READ_ONLY},
    {"Detect Event",                  TPS2386B_DET_REG,             READ_ONLY},
    {"Fault Event",                   TPS2386B_FAULTEVENT_REG,      READ_ONLY},
    {"Start Event",                   TPS2386B_STARTEVENT_REG,      READ_ONLY},
    {"Supply Event",                  TPS2386B_SUPPLYEVENT_REG,     READ_ONLY},
    {"Port1 Status",                  TPS2386B_P1_STATUS_REG,       READ_ONLY},
    {"Port2 Status",                  TPS2386B_P2_STATUS_REG,       READ_ONLY},
    {"Port3 Status",                  TPS2386B_P3_STATUS_REG,       READ_ONLY},
    {"Port4 Status",                  TPS2386B_P4_STATUS_REG,       READ_ONLY},
    {"Power Status",                  TPS2386B_PWR_STAT_REG,        READ_ONLY},
    {"I2C Slave Addr.",               TPS2386B_I2C_SLVADDR_REG,     READ_ONLY},
    {"Operating mode",                TPS2386B_OP_MODE_REG,         READ_WRITE},
    {"Disconnect Enable",             TPS2386B_DISCONN_EN_REG,      READ_WRITE},
    {"Detect/Class Enable",           TPS2386B_DETCLA_EN_REG,       READ_WRITE},
    {"Port Power Disable",            TPS2386B_PORT_PWR_DIS_REG,    READ_WRITE},
    {"Timing Config",                 TPS2386B_TIMING_CONF_REG,     READ_WRITE},
    {"General Mask",                  TPS2386B_GENERAL_MASK_REG,    READ_WRITE},
    {"Detect/Class Restart",          TPS2386B_DETCLA_RESTART_REG,  WRITE_ONLY},
    {"Power Enable",                  TPS2386B_PWR_EN_REG,          WRITE_ONLY},
    {"RESET",                         TPS2386B_RESET_REG,           WRITE_ONLY},
    {"ID",                            TPS2386B_ID_REG,              READ_WRITE},
    {"ICUT 21 CONFIG",                TPS2386B_POLICE21_CONFIG_REG, READ_WRITE},
    {"ICUT 43 CONFIG",                TPS2386B_POLICE43_CONFIG_REG, READ_WRITE},
    {"IEEE Power Enable",             TPS2386B_IEEE_PWR_ENA_REG,    READ_WRITE},
    {"Power on fault",                TPS2386B_PWRON_FAULT_REG,     READ_WRITE},
    {"Temperature",                   TPS2386B_TEMP_REG,            READ_ONLY},
    {"Input Voltage(LSB)",            TPS2386B_IN_VOLT_LSB_REG,     READ_ONLY},
    {"Input Voltage(MSB)",            TPS2386B_IN_VOLT_MSB_REG,     READ_ONLY},
    {"Port 1 Current(LSB)",           TPS2386B_P1_CURR_LSB_REG,     READ_ONLY},
    {"Port 1 Voltage(MSB)",           TPS2386B_P1_VOLT_MSB_REG,     READ_ONLY},
    {"Port 2 Current(LSB)",           TPS2386B_P2_CURR_LSB_REG,     READ_ONLY},
    {"Port 2 Voltage(MSB)",           TPS2386B_P2_VOLT_MSB_REG,     READ_ONLY},
    {"Port 3 Current(LSB)",           TPS2386B_P3_CURR_LSB_REG,     READ_ONLY},
    {"Port 3 Voltage(MSB)",           TPS2386B_P3_VOLT_MSB_REG,     READ_ONLY},
    {"Port 4 Current(LSB)",           TPS2386B_P4_CURR_LSB_REG,     READ_ONLY},
    {"Port 4 Voltage(MSB)",           TPS2386B_P4_VOLT_MSB_REG,     READ_ONLY},
    {"PoE Plus",                      TPS2386B_POEPLUS_REG,         READ_WRITE},
    {"Firmware Revision",             TPS2386B_FW_REV_REG,          READ_ONLY},
    {"I2C Watchdog",                  TPS2386B_I2C_WD_REG,          READ_WRITE},
    {"Device ID",                     TPS2386B_DEV_ID_REG,          READ_WRITE},
    {"Cool Down",                     TPS2386B_COOL_DOWN_REG,       READ_WRITE},
};

INFO_TB *op_mode_info_tb[INFO_SIZE_OP_MODE]  = {"OFF",           /* Index: 0 */ 
                                                "Manual",        /* Index: 1 */
                                                "Semi-Auto",     /* Index: 2 */
                                                "Semi-Auto"};    /* Index: 3 */

INFO_TB *port_onoff_info_tb[INFO_SIZE_ONOFF] = {"OFF",           /* Index: 0 */
                                                "ON"};           /* Index: 1 */

INFO_TB *detection_info_tb[INFO_SIZE_DET]    = {"Unknown",       /* Index: 0 */
                                                "Short-circuit", /* Index: 1 */
                                                "Reserved",      /* Index: 2 */
                                                "Too Low",       /* Index: 3 */
                                                "Valid",         /* Index: 4 */
                                                "Too High",      /* Index: 5 */
                                                "Open-circuit",  /* Index: 6 */
                                                "Reserved",      /* Index: 7 */
                                                "MOSFET fault"}; /* Index: 8 */

INFO_TB *class_info_tb[INFO_SIZE_CLASS]      = {"Unknown",       /* Index: 0 */
                                                "Class1",        /* Index: 1 */
                                                "Class2",        /* Index: 2 */
                                                "Class3",        /* Index: 3 */
                                                "Class4",        /* Index: 4 */
                                                "Reserved",      /* Index: 5 */
                                                "Class0",        /* Index: 6 */
                                                "Overcurrent"};  /* Index: 7 */

/******************************************************************************
 * Function   : tps2386b_dev_create
 *
 * Description:	Create object with various device function.
 * Inputs     : *dev          - dev_object_t pointer to the TPS2386B device
 *		err_report_fn - error reporting function pointer
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
int tps2386b_dev_create (dev_object_t *dev, dev_error_report_t err_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_tps2386b_object_t *tps2386b = (dev_tps2386b_object_t *)dev;

    /* Allocate memory for the device object */
    dev_fvt = (dev_object_fvt_t *)(malloc(sizeof(dev_object_fvt_t)));
    if (dev_fvt == NULL) {
        /* Unable to allocate memory */
        err_report_fn(dev, "malloc failed at dev_create.", TPS2386B_DEV_STATE);
	return (FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    tps2386b->base.dev_object_fvt->dev_attach            = dev_tps2386b_attach;
    tps2386b->base.dev_object_fvt->dev_detach            = dev_tps2386b_detach;
    tps2386b->base.dev_object_fvt->dev_reconfig_needed   = dev_tps2386b_reconfig;
    tps2386b->base.dev_object_fvt->dev_restart           = dev_tps2386b_restart;
    tps2386b->base.dev_object_fvt->dev_error_report      = err_report_fn;
    tps2386b->base.dev_object_fvt->dev_collect_crashinfo = dev_tps2386b_crsh;
    tps2386b->base.dev_object_fvt->dev_destroy           = dev_tps2386b_destroy;
    tps2386b->base.dev_object_fvt->dev_name              = "TPS2386B";

    tps2386b->callin_fvt = (tps2386b_callin_fvt_t *)
                           (malloc(sizeof(tps2386b_callin_fvt_t)));
    tps2386b->callout_fvt = (tps2386b_callout_fvt_t *)
                            (malloc(sizeof(tps2386b_callout_fvt_t)));

    tps2386b->base.dev_state = DEV_STATE_CREATE;
    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_attach
 *
 * Description:	Attach TPS2386B device for use.
 *              This function will initialize, setup all necessary pointers
 *              and bring the chip to operation.
 * Inputs     : Pointer to the TPS2386B device object
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32 dev_tps2386b_attach (dev_object_t *dev)
{
    dev_tps2386b_object_t *tps2386b = (dev_tps2386b_object_t *)dev;

    if (tps2386b->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "callin_fvt is NULL", TPS2386B_ATTACH);
        return (FAILED);
    }

    if (tps2386b->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "callout_fvt is NULL", TPS2386B_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    tps2386b->callin_fvt->register_test      = dev_tps2386b_test_reg;
    tps2386b->callin_fvt->interrupt_test     = dev_tps2386b_test_intr;
    tps2386b->callin_fvt->util_read_reg      = dev_tps2386b_util_read_reg;
    tps2386b->callin_fvt->util_write_reg     = dev_tps2386b_util_write_reg;
    tps2386b->callin_fvt->util_dump_register = dev_tps2386b_util_dump_reg;
    tps2386b->callin_fvt->util_show_pwr_stat = dev_tps2386b_util_show_pwr_stat;
    tps2386b->callin_fvt->util_detect_pwr    = dev_tps2386b_util_detect_pwr;

    tps2386b->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_detach
 *
 * Description:	detach the device specific functions from the caller.
 *              All of the device specific function are connected to the
 *              dev_do_nothing() function, except for the dev_attach() function.
 *
 *              Also, the dev_state must be assigned the value of
 *              DEV_STATE_DETACH.
 *
 *              Since, some platforms may want to detach the device, but not
 *              release memory resources(via a free () in the dev_destroy()),
 *              this function can be executed to accomplish this task. However,
 *              a detached device need to be re-attached(via dev_attach())
 *              before it can be used again.
 * Inputs     : Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static uint32 dev_tps2386b_detach (dev_object_t *dev)
{
    dev_tps2386b_object_t *tps2386b = (dev_tps2386b_object_t *)dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, tps2386b->base.dev_object_fvt);

    tps2386b->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);
}

/**i***************************************************************************
 * Function   : dev_tps2386b_reconfig
 *
 * Description:	A scratch function for reconfig.
 * Inputs     : *dev            - dev_object_t pointer to the TPS2386B device
 *		*context_handle - a device/platform specific context handle
 *		*reconfig       - a pointer to a boolean
 * Outputs    : PASSED/FAILED, context information and a boolean value.
 *              The boolean value shall be set to TRUE if the device must be
 *              reconfigured from scratch and it shall be set to FALSE otherwise
 * Assumptions:	The dev_attach() function has been called and successfully
 ******************************************************************************/
static uint32 dev_tps2386b_reconfig (dev_object_t *dev, void *context_handle,
                                  boolean *reconfig)
{
    *reconfig = FALSE;   /* No need to reconfig from scratch */
    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *              the device or changing its configuration.
 *              For example, during a failover event.
 *
 *              Change the state of the device from its current state
 *              to an initial state. Also, dev_state must be assigned the
 *              value of DEV_STATE_INIT.
 * Inputs     : *dev - dev_object_t pointer to TPS2386B device
 * Outputs    : PASSED/FAILED
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 ******************************************************************************/
static uint32 dev_tps2386b_restart (dev_object_t *dev)
{
    dev_tps2386b_object_t *tps2386b = (dev_tps2386b_object_t *)dev;

    tps2386b->base.dev_state = DEV_STATE_INIT;

    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *              Print data to the crash log (via the provide print error) using
 *              the appropriate verbisity level requested by the host
 * Inputs     : *dev      - dev_object_t pointer to the TPS2386B device
 *		dev_print - A crash print function vector.
 *              verbosity - A verbosity level.
 * Outputs    : PASSED/FAILED
 * Assumptions:	A device print function vector has been provided by the host
 *              platform which implements the crash logging functionality. It
 *              could be the mechanism to log info to the Compact Flash before
 *              the device crash and now retrieve them. The dev_attch()
 *              function has been called and successfully executed.
 ******************************************************************************/
static uint32 dev_tps2386b_crsh (dev_object_t *dev, print_fn_t dev_print,
                              dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("%s(): No Crash info available for TPS2386B\n", __func__);
    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 * Inputs     : **dev - dev_object_t pointer to the TPS2386B device
 * Outputs    : none
 * Assumptions:	The dev_attch() function has been called and successfully.
 ******************************************************************************/
static void dev_tps2386b_destroy (dev_object_t **dev)
{
    dev_tps2386b_object_t *tps2386b;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    tps2386b = (dev_tps2386b_object_t *)*dev;

    if (tps2386b->callout_fvt) {
        free(tps2386b->callout_fvt);   /* Free callout struct */
    }

    if (tps2386b->callin_fvt) {
        free(tps2386b->callin_fvt);   /* Free callin struct */
    }

    free(tps2386b->base.dev_object_fvt);   /* Free dev_object_t */
}

/******************************************************************************
 * Function   : dev_tps2386b_util_read_reg
 *
 * Description:	Utility to read TPS2386B register
 * Inputs     : *dev - dev_object_t pointer to the TPS2386B device
 * Outputs    : PASSED/FAILED
 * Assumptions:	The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 ******************************************************************************/
static int dev_tps2386b_util_read_reg (dev_object_t *dev)
{
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)dev;
    uchar reg_addr = 0;
    ulong reg_val = 0;
    int byte_size = TPS2386B_REG_ONE_BYTE_ACCESS;

    reg_addr = (uchar)gethex_answer("\nEnter Reg. address(0 ~ 0x45) ",
                                    TPS2386B_START_REG, 
                                    TPS2386B_START_REG,
                                    TPS2386B_END_REG);

    if (dev_i2c_rd((ulong)reg_addr,
                   byte_size,
                   &reg_val,
                   (void *)tps2386b_p) != PASSED) {
        printf("%s(): Failed to read register %#x.\n", __func__, reg_addr);
        return (FAILED);
    }

    printf("%s reg. 0x%02X = 0x%X\n",
           tps2386b_p->base.dev_object_fvt->dev_name,
           reg_addr,
           (uchar)reg_val);

    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_util_write_reg
 *
 * Description:	Utility to write TPS2386B register
 * Inputs     : *dev - dev_object_t pointer to the TPS2386B device
 * Outputs    : PASSED/FAILED
 * Assumptions:	create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 ******************************************************************************/
static int dev_tps2386b_util_write_reg (dev_object_t *dev)
{
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)dev;
    uchar reg_addr = 0, wr_val = 0;
    ulong reg_val = 0;
    int byte_size = TPS2386B_REG_ONE_BYTE_ACCESS;

    reg_addr = (uchar)gethex_answer("\nEnter register offset in hex(0 ~ 0x45)",
                                    TPS2386B_START_REG, 
                                    TPS2386B_START_REG,
                                    TPS2386B_END_REG);

    if (dev_i2c_rd((ulong)reg_addr,
                   byte_size,
                   &reg_val,
                   (void *)tps2386b_p) != PASSED) {
        printf("%s(): Failed to read register %#x.\n", __func__, reg_addr);
        return (FAILED);
    }

    wr_val = (uchar)gethex_answer("Enter the alter value in hex",
                                  reg_val, 0, 0xFF);

    if (dev_i2c_wr((ulong)reg_addr,
                   byte_size,
                   (ulong)wr_val,
                   (void *)tps2386b_p) != PASSED) {
        printf("%s(): Failed to write register 0x%02X.\n", __func__, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (dev_i2c_rd((ulong)reg_addr,
                   byte_size,
                   &reg_val,
                   (void *)tps2386b_p) != PASSED) {
        printf("%s(): Failed to read register %#x.\n", __func__, reg_addr);
        return (FAILED);
    }

    printf("Done writing 0x%02X to %s reg. 0x%02X, its value = 0x%02X now.\n",
           wr_val,
           tps2386b_p->base.dev_object_fvt->dev_name,
           reg_addr,
           (uchar)reg_val);

    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_util_dump_reg
 * 
 * Description:	Function to dump all TPS2386B reigsters.
 * Inputs     : *dev - dev_object_t pointer to TPS2386B device
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_util_dump_reg (dev_object_t *dev)
{
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)dev;
    ulong reg_addr = 0;
    ulong reg_val = 0;
    int ix, byte_size = TPS2386B_REG_ONE_BYTE_ACCESS;
    int table_size = (sizeof(tps2386b_reg_dump_table) / 
                      sizeof(tps2386b_reg_info_t));
    tps2386b_reg_info_t *reg_p = tps2386b_reg_dump_table;

    for (ix = 0; ix < table_size; ix++, reg_p++) 
    {
        reg_addr = reg_p->reg_addr;

        /* read register*/
        if (dev_i2c_rd(reg_addr,
                       byte_size,
                       &reg_val,
                       (void *)tps2386b_p) != PASSED) {
            printf("%s(): Failed to read register %02x\n", 
                   __func__, (int)reg_addr);
            return (FAILED);
        }
        printf("%-30s reg(0x%02x): 0x%02x\n", 
               reg_p->reg_name, (int)reg_addr, (uchar)reg_val);
    }

    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_test_reg
 *
 * Description: Function to test TPS2386B register read/write function
 * Inputs     : *dev - dev_object_t pointer to TPS2386B device
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_test_reg (dev_object_t *dev)
{
    char err_buf[TPS2386B_ERR_BUF_SIZE];
    reg_info_t *test_reg_p;

    /* Init pointer to beginning of register table */
    test_reg_p = &tps2386b_reg_test_table[0];
    test_reg_p->size.ext->param = (void *)dev;

    if (register_tests(0, test_reg_p) != PASSED) {
        sprintf(err_buf, "%s:%d:TPS2386b register test Failed", 
                         __func__, __LINE__);
        DEV_ERROR_REPORT(dev, err_buf, TPS2386B_REG_TEST);
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 * Function   : dev_tps2386b_test_intr
 *
 * Description: Function to test TPS2386B Interrupt Pin
 * Inputs     : *dev - dev_object_t pointer to TPS2386B device
 * Outputs    : PASSED/FAILED
 * Note: For more details of interrupt procedure, please check
 *       the specification that "TPS23861 IEEE 802.3at Quad Port 
 *       Power-over-Ethernet PSE Controller"
 *       Please refer to the following page:
 *       P.49, Interrupt Enable Register(0x1) SUPEN field(bit[7])
 ******************************************************************************/
static int dev_tps2386b_test_intr (dev_object_t *dev)
{
    ulong reg_offset = TPS2386B_INTR_MASK_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0, wr_data = 0x0, original_data = 0;

    /* 1. Read original data of TPS2386B Interrupt Enable Register(0x1)*/
    if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
        printf("%s(): Failed to read register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }
    original_data = rd_data;

    /* 1. If the interrupt is asserted already, force clear first */
    if ((original_data & TPS2386B_ENA_SUPEN_INTR) == TPS2386B_ENA_SUPEN_INTR) {
        /* 1.1. Clear Interrupt Pin with setting SUPEN field(bit[8]) as 0 
         *    (writing 0x0 into Interrupt Enable Register) 
         *    deu to default value is 0x80. */
        wr_data = TPS2386B_DIS_SUPEN_INTR;
        if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
            printf("%s(): Failed to write register 0x%ld\n", 
                   __func__, reg_offset);
            return (FAILED);
        }
        
        /* 1.2. Checking INT pin is de-asserted */
        if (dev_tps2386b_chk_intr_deassert(dev) != PASSED) {
            printf("%s(): The interrupt pin is not de-asserted!!\n\n", 
                   __func__);
            return (FAILED);
        }
    }

    /* 2. Trigger Interrupt Pin with 
     *    setting SUPEN field(bit[8]) as 1 
     *    (writing 0x80 into Interrupt Enable Register) */
    wr_data = TPS2386B_ENA_SUPEN_INTR;
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    /* 3. Checking INT pin is asserted */
    if (dev_tps2386b_chk_intr_assert(dev) != PASSED) {
        printf("%s(): The interrupt pin is not asserted!!\n\n", __func__);
        return (FAILED);
    }

    /* 4. Clear Interrupt Pin with 
     *    setting SUPEN field(bit[8]) as 0 
     *    (writing 0x0 into Interrupt Enable Register) */
    wr_data = TPS2386B_DIS_SUPEN_INTR;
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    /* 5. Checking INT pin is de-asserted */
    if (dev_tps2386b_chk_intr_deassert(dev) != PASSED) {
        printf("%s(): The interrupt pin is not de-asserted!!\n\n", __func__);
        return (FAILED);
    }

    /* 6. Recover Interrupt Enable Register with original data */
    wr_data = original_data;
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Function   : dev_i2c_rd
 *
 * Description: Function to read TPS2386B register by I2C.
 * Inputs     : offset - offset of register to be written
 *              size   - only support one byte
 *              *buf   - pointer to the data buffer to be read
 *              param  - pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_i2c_rd (ulong offset, int size, ulong *buf, void *param)
{
    uint32 retv = FAILED;
    char err_buf[TPS2386B_ERR_BUF_SIZE];
    n2g_i2c_if_t i2c_if;
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)param;
    ushort reg_val = 0;

    /* Setup the interface struct for I2C API read */
    i2c_if.offset = offset;
    i2c_if.buf = (char *)&reg_val;
    i2c_if.size = TPS2386B_REG_ONE_BYTE_ACCESS; 
    i2c_if.i2c_bus_type = tps2386b_p->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = tps2386b_p->i2c_p->i2c_dev;

    /* Call the I2C Read API */
    retv = tps2386b_p->callout_fvt->rd(&i2c_if);
    if (retv != PASSED) {
	/* Read Failed */
        sprintf(err_buf, "%s(): Failed to read reg. %lx(return code %#x)\n",
                         __func__, offset, retv);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, TPS2386B_I2C_RD);
	return (FAILED);
    }

    *buf = (ulong)reg_val;
    return (PASSED);
}

/******************************************************************************
 * Function   : dev_i2c_wr
 *
 * Description: Function to write TPS2386B register by I2C.
 * Inputs     : offset - offset of register to be read
 *              size   - number of bytes to be read
 *              data   - write data
 *              param  - Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_i2c_wr (ulong offset, int size, ulong data, void *param)
{
    uint32 ret_val = FAILED;
    n2g_i2c_if_t i2c_if;
    char err_buf[TPS2386B_ERR_BUF_SIZE];
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)param;
    ushort wr_data =(ushort)data;

    /* Setup the interface struct for I2C API write */
    i2c_if.offset = offset;
    i2c_if.buf = (char *)&wr_data;
    i2c_if.size = TPS2386B_REG_ONE_BYTE_ACCESS;
    i2c_if.i2c_bus_type = tps2386b_p->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = tps2386b_p->i2c_p->i2c_dev;

    /* Call the I2C Write API */
    ret_val = tps2386b_p->callout_fvt->wr(&i2c_if);
    if (ret_val != PASSED) {
	/* Write Failed */
        sprintf(err_buf, "%s(): Failed to write reg. %lx(return code %#x)\n",
                         __func__, offset, ret_val);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, TPS2386B_I2C_WR);
	return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_chk_intr_assert
 *
 * Description: Function to check Interrupt Pin is asserted
 * Inputs     : dev - Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_chk_intr_assert (void *dev)
{
    int rc = FAILED, ix;
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)dev;

    /* While platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = tps2386b_p->callout_fvt->chk_intr_assert();
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }
    return (rc);
}

/*******************************************************************************
 * Function   : dev_tps2386b_chk_intr_deassert
 *
 * Description: Function to check Interrupt Pin is de-asserted
 * Inputs     : dev - Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_chk_intr_deassert (void *dev)
{
    int rc = FAILED, ix;
    dev_tps2386b_object_t *tps2386b_p = (dev_tps2386b_object_t *)dev;

    /* While platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < INTR_POLLING_ROUND; ix++) 
    {
        rc = tps2386b_p->callout_fvt->chk_intr_deassert();
        if (rc == PASSED) {
            break;
        }
        msleep(INTR_POLLING_PERIOD);
    }
    return (rc);
}

/*******************************************************************************
 * Function   : dev_tps2386b_util_show_pwr_stat
 *
 * Description: Function to show port power status
 * Inputs     : dev    - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_util_show_pwr_stat (dev_object_t *dev, 
                                       int s_port, int e_port)
{
    /* Show the title of status */
    dev_tps2386b_show_status_title();
    
    /* Clear the port status */
    dev_tps2386b_clr_port_status();

    /* Get port operation mode with offset:0x12 */
    if (dev_tps2386b_get_port_op_mode(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to get operation mode\n", __func__, __LINE__);
    }
   
    /* Get port status(Dection/Class) with offset:0x0c / 0x0d / 0x0e / 0x0f */
    if (dev_tps2386b_get_port_detection_class(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to get Detection/Class\n", __func__, __LINE__);
    }

    /* Get port power status with offset:0x10 */
    if (dev_tps2386b_get_port_pwr_status(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to get power status\n", __func__, __LINE__);
    }

    /* Check the port current is not 0 with offset:0x30 / 0x34 / 0x38 / 0x3c */
    if (dev_tps2386b_detect_port_current(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to detect current\n", __func__, __LINE__);
    }

    /* Get stable port current with offset:0x30 / 0x34 / 0x38 / 0x3c */
    if (dev_tps2386b_get_port_current(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to get current\n", __func__, __LINE__);
    }

    /* Get port voltage with offset:0x32 / 0x36 / 0x3a / 0x3e */    
    if (dev_tps2386b_get_port_voltage(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to get current\n", __func__, __LINE__);
    }

    /* Get port power */ 
    if (dev_tps2386b_get_port_power(s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to get current\n", __func__, __LINE__);
    }

    /* Show port number , operation mode, power ON/OFF, detection/class, 
     * power, voltage, current */
    dev_tps2386b_show_port_status(dev, s_port, e_port); 

    /* Debug information dump */
    if ((NVRAM)->diagflag & D_VERBOSE) {
        dbg_dev_tps2386b_show_port_status(dev); 
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_show_status_title
 *
 * Description: Function to show title of status
 * Inputs     : dev - Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static void dev_tps2386b_show_status_title (void)
{
    int ix;

    printf("\n\n");
    printf("Port   Mode       ON/OFF Detect/Class  Power(W)"
           "  Volt(V)   Current(mA)\n");
    for (ix = 0; ix < 70; ix++) {
        printf("=");
    }
    printf("\n");
}

/*******************************************************************************
 * Function   : dev_tps2386b_clr_port_status 
 *
 * Description: Function to clear all fileds of port status as default
 * Inputs     : NONE
 * Outputs    : NONE
 ******************************************************************************/
static void dev_tps2386b_clr_port_status (void)
{
    int ix, base;
    
    for (ix = TPS2386B_PORT1; ix <= TPS2386B_PORT4; ix++)
    {
        base = ix - 1;
        port_status[base].port_number      = ix; /* port 1~4 */
        port_status[base].port_op_mode     = DEFAULT_OP_MODE;
        port_status[base].port_detection   = DEFAULT_DETECTION;
        port_status[base].port_class       = DEFAULT_CLASS;
        port_status[base].port_power_good  = PORT_PWR_OFF;
        port_status[base].port_onoff  = PORT_PORT_OFF;
        port_status[base].port_current     = NO_CURRENT;
        port_status[base].port_voltage     = NO_VOLTAGE;
        port_status[base].port_power       = NO_POWER;
    }
}

/*******************************************************************************
 * Function   : dev_tps2386b_get_port_op_mode
 *
 * Description: Function to get port operation mode with offset:0x12
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_port - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_get_port_op_mode (dev_object_t *dev, 
                                          int s_port, int e_port)
{
    /* Operation mode register, 0x12 */
    ulong reg_offset = TPS2386B_OP_MODE_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0;
    int ix, base;

    /* Operation mode register description:
     * =================================================
     * = D7   D6   | D5   D4   | D3   D2   | D1   D0   =
     * = P4M1 P4M0 | P3M1 P3M0 | P2M1 P2M0 | P1M1 P1M0 =
     * =================================================
     *
     * M1 M0    Operating Mode
     * 0  0  -> Off
     * 0  1  -> Manual
     * 1  X  -> Semi Auto
     */
   
    /* read register */
    if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
        printf("%s(): Failed to read register 0x%x\n", 
                __func__, (uchar)reg_offset);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d:Operqation Mode reg:0x%x value = 0x%x\n",
               __func__, __LINE__, (uchar)reg_offset, (uchar)rd_data);
    }

    /* data parsing */
    for (ix = s_port; ix <= e_port; ix++)
    {
        base = ix - 1; /* range:0~3 */
        port_status[base].port_op_mode = (uchar)(rd_data >> (base * 2)) &
                                         (uchar)REG_MASK_FOR_OP_MODE;
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_get_port_detection_class
 *
 * Description: Function to get port detection/class with offset:
 *              0x0C / 0x0D / 0x0E / 0x0F
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_port - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_get_port_detection_class (dev_object_t *dev,
                                                  int s_port, int e_port)
{
    ulong reg_offset;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0;
    int ix, base;

    /* data parsing */
    for (ix = s_port; ix <= e_port; ix++)
    {
        base = ix - 1; /* range:0~3 */

        reg_offset = (uchar)REG_SHIFT_STATUS_REG(ix);
        /* read register */
        if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
            printf("%s(): Failed to read register 0x%x\n", 
                    __func__, (uchar)reg_offset);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d:Port%d Status reg:0x%x value = 0x%x\n",
                   __func__, __LINE__, ix, (uchar)reg_offset, (uchar)rd_data);
        }

        /* data parsing for Detection */
        port_status[base].port_detection = (uchar)(rd_data & 
                                                   REG_MASK_FOR_DETECTION);
        /* data parsing for Class */
        port_status[base].port_class = (uchar)((rd_data & REG_MASK_FOR_CLASS) >>
                                               REG_SHIFT_CLASS);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_get_port_pwr_status
 *
 * Description: Function to get power status with offset:0x10
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_get_port_pwr_status (dev_object_t *dev,
                                             int s_port, int e_port)
{
    /* Operation mode register, 0x10 */
    ulong reg_offset = TPS2386B_PWR_STAT_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0;
    int ix, base;

    /* Power status register description:
     * ================================================
     * ==== D7   D6   D5   D4   |  D3   D2   D1   D0  =
     * ==== PG4  PG3  PG2  PG1  |  PE4  PE3  PE2  PE1 =
     * ================================================
     * PG                       | PE
     * 0  -> Power is good      | 0  -> Port is ON
     * 1  -> Power is not good  | 1  -> Port is OFF
     */

    /* read register */
    if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
        printf("%s(): Failed to read register 0x%x\n", 
                __func__, (uchar)reg_offset);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d:Power Status reg:0x%x value = 0x%x\n",
               __func__, __LINE__, (uchar)reg_offset, (uchar)rd_data);
    }

    for (ix = s_port; ix <= e_port; ix++)
    {
        base = ix - 1; /* range:0~3 */
        /* data parsing for power good */
        port_status[base].port_power_good = (uchar)(rd_data >> 
                                                    REG_SHIFT_PG(ix)) &
                                            (uchar)REG_MASK_FOR_PG;

        /* data parsing for port On/Off */
        port_status[base].port_onoff = (uchar)(rd_data >> 
                                               REG_SHIFT_PE(ix)) &
                                       (uchar)REG_MASK_FOR_PE;
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_detect_port_current
 *
 * Description: Function to detect port current with offset:
 *              0x30 / 0x34 / 0x38 / 0x3C
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_detect_port_current (dev_object_t *dev, 
                                             int s_port, int e_port)
{
    ulong reg_offset;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0;
    int ix, jx, kx, base;

    for (ix = s_port; ix <= e_port; ix++)
    {
        /* no need to detect current due to the port is off */
        base = ix - 1;
        if (port_status[base].port_onoff == PORT_PORT_OFF) {
            continue;
        }

        /* read LSB of current register, 0x30/0x34/0x38/0x3C */
        reg_offset = (uchar)REG_SHIFT_CURRENT_REG_LSB(ix);
        /* clear buffer before read */
        rd_data = 0;

        for (jx = 0; jx < TPS2386B_MAX_POLLING; jx++)
        {
            printf("\rDetecting power...");
            fflush(stdout);

            /* read register */
            if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
                printf("%s(): Failed to read register 0x%x\n", 
                        __func__, (uchar)reg_offset);
                return (FAILED);
            }

            if (rd_data != 0) {
                /* PD starts polling power, wait PD to get expected power.
                 * Confirmed with PSE vendor, no way for PSE to know PD
                 * polls enough power.
                 * So here wait 5 seconds from PD starts to poll.
                 * Note:
                 * PD: Power Device, such as PoE tester.
                 * */
                for (kx = 0; kx < TPS2386B_WAIT_PD_READY_MAX; kx++) {
                    printf(".");
                    fflush(stdout);
                    msleep(TPS2386B_WAIT_PD_READY_INTVAL);
                }
                break;
            }
            msleep(TPS2386B_POLLING_INTERVAL);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_get_port_current
 *
 * Description: Function to get port current with offset:
 *                   P1   / P2   / P3   / P4
 *              LSB: 0x30 / 0x34 / 0x38 / 0x3C
 *              MSB: 0x31 / 0x35 / 0x39 / 0x3D
 * Inputs     : dev  - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_get_port_current (dev_object_t *dev, 
                                          int  s_prot, int e_port)
{
    ulong reg_offset;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0;
    double current_msb = 0.0, current_lsb = 0.0;
    int ix, base;

    for (ix = s_prot; ix <= e_port; ix++)
    {
        /* no need to detect current due to the port is off */
        base = ix - 1; /* 0~3 */
        if (port_status[base].port_onoff == PORT_PORT_OFF) {
            continue;
        }

        /* clear buffer before read */
        rd_data = 0;

        /* =====================================================
         * = read LSB of current register, 0x30/0x34/0x38/0x3C =
         * ===================================================== */
        reg_offset = (uchar)REG_SHIFT_CURRENT_REG_LSB(ix);
        /* read register */
        if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
            printf("%s(): Failed to read register 0x%x\n", 
                    __func__, (uchar)reg_offset);
            return (FAILED);
        }
        /* get the LSB of current value */
        current_lsb = (double)rd_data;

        /* clear buffer before read */
        rd_data = 0;
        
        /* =====================================================
         * = read MSB of current register, 0x31/0x35/0x39/0x3D =
         * ===================================================== */
        reg_offset = (uchar)REG_SHIFT_CURRENT_REG_MSB(ix);
        /* read register */
        if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
            printf("%s(): Failed to read register 0x%x\n", 
                    __func__, (uchar)reg_offset);
            return (FAILED);
        }
        /* get the MSB of current value */
        current_msb = (double)(rd_data << REG_SHIFT_MSB_CURRENT);

        /* merge MSB + LSB */
        port_status[base].port_current = (current_msb + current_lsb) *
                                         (double)CURRENT_RESOLUTION;

        /* uni translation from uA to mA */
        port_status[base].port_current /= (double)CURRENT_UNI_TRANS; 
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_get_port_voltage
 *
 * Description: Function to get port voltage with offset:
 *              0x32 / 0x36 / 0x3A / 0x3E
 *                   P1   / P2   / P3   / P4
 *              LSB: 0x32 / 0x36 / 0x3A / 0x3E
 *              MSB: 0x33 / 0x37 / 0x3B / 0x3F
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_get_port_voltage (dev_object_t *dev, 
                                          int s_port, int e_port)
{
    ulong reg_offset;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0;
    double volt_msb = 0.0, volt_lsb = 0.0;
    int ix, base;

    for (ix = s_port; ix <= e_port; ix++)
    {
        /* no need to detect current due to the port is off */
        base = ix - 1; /* 0~3 */
        if (port_status[base].port_onoff == PORT_PORT_OFF) {
            continue;
        }

        /* clear buffer before read */
        rd_data = 0;

        /* =====================================================
         * = read LSB of voltage register, 0x32/0x36/0x3A/0x3E =
         * ===================================================== */
        reg_offset = (uchar)REG_SHIFT_VOLTAGE_REG_LSB(ix);
        /* read register */
        if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
            printf("%s(): Failed to read register 0x%x\n", 
                    __func__, (uchar)reg_offset);
            return (FAILED);
        }
        /* get the LSB of current value */
        volt_lsb = (double)rd_data;

        /* clear buffer before read */
        rd_data = 0;
        
        /* =====================================================
         * = read LSB of voltage register, 0x33/0x37/0x3B/0x3F =
         * ===================================================== */
        reg_offset = (uchar)REG_SHIFT_VOLTAGE_REG_MSB(ix);
        /* read register */
        if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
            printf("%s(): Failed to read register 0x%x\n", 
                    __func__, (uchar)reg_offset);
            return (FAILED);
        }
        /* get the MSB of current value */
        volt_msb = (double)(rd_data << REG_SHIFT_MSB_VOLTAGE);

        /* merge MSB + LSB */
        port_status[base].port_voltage = (volt_msb + volt_lsb) *
                                         (double)VOLTAGE_RESOLUTION;

        /* uni translation from mV to V */
        port_status[base].port_voltage /= (double)VOLTAGE_UNI_TRANS; 
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_get_port_power
 *
 * Description: Function to get port power
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_get_port_power (int s_port, int e_port)
{
    int ix, base;

    for (ix = s_port; ix <= e_port; ix++)
    {
        /* no need to detect current due to the port is off */
        base = ix - 1; /* 0~3 */
        if (port_status[base].port_onoff == PORT_PORT_OFF) {
            continue;
        }
        /* Power calculation by formula: P = I * V */
        port_status[base].port_power = (double)(port_status[base].port_current *
                                                port_status[base].port_voltage);

        /* uni translation from mW to W */
        port_status[base].port_power /= POWER_UNI_TRANS;
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_show_port_status
 *
 * Description: Function to show port status as following:
 *              port number, operation mode, power ON/OFF, detection, class, 
 *              power, voltage, current
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : NONE
 ******************************************************************************/
static void dev_tps2386b_show_port_status (dev_object_t *dev,
                                           int s_port, int e_port)
{
    int ix, base, info_index;

    for (ix = s_port; ix <= e_port; ix++)
    {
       base = ix - 1;
       /* ====================
          = show port number = 
          ==================== */
       printf("\rPort%d  ", port_status[base].port_number);

       /* =======================
          = show operation mode = 
          ======================= */
       info_index = port_status[base].port_op_mode;
       if (CHECK_OP_MODE(info_index)) {
           printf("%-11s", op_mode_info_tb[info_index]);
       } else {
           printf("\n%s:%d:Invalid op mode:0x%x\n", __func__, __LINE__, 
                   port_status[base].port_op_mode);
           continue;
       }
       
       /* ====================
          = show port ON/OFF = 
          ==================== */
       info_index = port_status[base].port_onoff;
       if (CHECK_PORT_ONOFF(info_index)) {
           printf("%-7s", port_onoff_info_tb[info_index]);
       } else {
           printf("\n%s:%d:Invalid port on_off:0x%x\n", __func__, __LINE__, 
                   port_status[base].port_onoff);
           continue;
       }

       /* ================================
          = show port detection or class = 
          ================================ */
       info_index = port_status[base].port_detection;
       if (CHECK_DETECTION(info_index)) {
           if(CHECK_DETECT_VALID(info_index)) {
               /* show class info if valid detection */
               info_index = port_status[base].port_class;
               if (CHECK_CLASS(info_index)) {
                   printf("%-14s", class_info_tb[info_index]);
               } else {
                   printf("\n%s:%d:Invalid class:0x%x\n", __func__, __LINE__,
                   port_status[base].port_class);
                   continue;
               }
           } else {
               /* show detection info */
               printf("%-14s", detection_info_tb[info_index]);
           }
       } else {
           printf("\n%s:%d:Invalid detection:0x%x\n", __func__, __LINE__, 
                   port_status[base].port_detection);
           continue;
       }
   
       /* ========================================
          = show port power, voltage and current = 
          ======================================== */
        printf("%.2lf(W)   %.2lf(V)   %.2lf(mA)",
               port_status[base].port_power,
               port_status[base].port_voltage,
               port_status[base].port_current);
        
       printf("\n");
    }
}

/*******************************************************************************
 * Function   : dbg_dev_tps2386b_show_port_status
 *
 * Description:  A debugging function to show port status as following:
 *              port number, operation mode, power ON/OFF, detection, class, 
 *              power, voltage, current
 * Inputs     : dev - Pointer to TPS2386B device object
 * Outputs    : NONE
 ******************************************************************************/
static void dbg_dev_tps2386b_show_port_status (dev_object_t *dev)
{
    int ix, base;
    
    printf("\n");
    for (ix = TPS2386B_PORT1; ix <= TPS2386B_PORT4; ix++)
    {
        base = ix - 1;
        printf("Port%d status :\n",      port_status[base].port_number);
        printf("    op mode   :0x%x\n",  port_status[base].port_op_mode);
        printf("    detection :0x%x\n",  port_status[base].port_detection);
        printf("    class     :0x%x\n",  port_status[base].port_class);
        printf("    power good:0x%x\n",  port_status[base].port_power_good);
        printf("    port onoff:0x%x\n",  port_status[base].port_onoff);
        printf("    current   :%.2lf mA\n", port_status[base].port_current);
        printf("    voltage   :%.2lf Volt\n", port_status[base].port_voltage);
        printf("    power     :%.2lf Watt\n", port_status[base].port_power);
    }
}

/*******************************************************************************
 * Function   : dev_tps2386b_util_detect_pwr
 *
 * Description: Function to detect port power
 * Inputs     : dev    - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_util_detect_pwr (dev_object_t *dev, 
                                       int s_port, int e_port)
{
    /* reset all port with offset:0x1A */
    if (dev_tps2386b_reset_all_port(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to reset all port\n", __func__, __LINE__);
    }
    
    /* config all port as semi-auto op mode with offset:0x12 */
    if (dev_tps2386b_cfg_semi_auto_mode(dev) != PASSED) {
        printf("%s:%d:Failed to config semi-auto mode\n", __func__, __LINE__);
    }

    /* enable all port detection and class with offset:0x14 */
    if (dev_tps2386b_ena_detection_class(dev) != PASSED) {
        printf("%s:%d:Failed to enable detection/class\n", __func__, __LINE__);
    }

    /* enable port power with offset:0x0C/0x0D/0x0E/0x0F */
    if (dev_tps2386b_ena_port_pwr(dev, s_port, e_port) != PASSED) {
        printf("%s:%d:Failed to enable port power\n", __func__, __LINE__);
    }

    /* show port power status */
    dev_tps2386b_util_show_pwr_stat(dev, s_port, e_port);
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_reset_all_port
 *
 * Description: Function to reset all port with offset:0x1A
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_reset_all_port(dev_object_t *dev, 
                                       int s_prot, int e_port)
{
    ulong reg_offset  = TPS2386B_RESET_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong wr_data = 0x0, pattern = TPS2386B_STAT_REG_RST_VALUE;
    int ix;

    /* reset all port with writing data:0x10 into reg:0x1A */
    wr_data = TPS2386B_ALL_PORT_RST;
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    /* wait 10ms from Reset to Start condition */
    msleep(TPS2386B_RST_WAIT_TIME);

    /*  check all port status(detection/class) should be 0 */
    for (ix = s_prot; ix <= e_port; ix++)
    {
        reg_offset = REG_SHIFT_STATUS_REG(ix);

        /* polling status register with given port */
        if (dev_tps2386b_polling_reg(dev, COMPARE_EQL, 
                                     reg_offset, pattern) != PASSED) {
            printf("%s:%d:Port:%d status reg:0x%x is not reset\n",
                   __func__, __LINE__, ix, (uchar)reg_offset); 
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_cfg_semi_auto_mode
 *
 * Description: Function to config Semi-Auto mode with offset:0x12
 * Inputs     : dev - Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_cfg_semi_auto_mode(dev_object_t *dev)
{
    ulong reg_offset  = TPS2386B_OP_MODE_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong wr_data = 0x0, pattern = CFG_ALL_PORT_SEMI_AUTO_MODE;

    /* config all port as Auto-Semi mode with writing data:0xAA into reg:0x12 */
    wr_data = pattern;
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    /* polling operation mode register with pattern:0xAA */
    if (dev_tps2386b_polling_reg(dev, COMPARE_EQL, 
                                 reg_offset, pattern) != PASSED) {
        printf("%s:%d:Operation mode reg:0x%x is not in Auto-Semi mode\n",
               __func__, __LINE__, (uchar)reg_offset); 
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_ena_detection_class
 *
 * Description: Function to enable detection and class with offset:0x14
 * Inputs     : dev - Pointer to TPS2386B device object
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_ena_detection_class(dev_object_t *dev)
{
    ulong reg_offset  = TPS2386B_DETCLA_EN_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong wr_data = 0x0, pattern = ENA_ALL_PORT_DETECTION_CLASS;

    /* enable all port detection/class with writing data:0xFF into reg:0x14 */
    wr_data = pattern;
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    /* polling operation mode register with pattern:0xAA */
    if (dev_tps2386b_polling_reg(dev, COMPARE_EQL, 
                                 reg_offset, pattern) != PASSED) {
        printf("%s:%d:Detect/Class Enable reg:0x%x is not enable\n",
               __func__, __LINE__, (uchar)reg_offset); 
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_ena_port_pwr 
 *
 * Description: Function to enable port power
 * Inputs     : dev - Pointer to TPS2386B device object
 *              s_prot - start port
 *              e_port - end port
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_ena_port_pwr(dev_object_t *dev, int s_port, int e_port)
{
    int ix, jx, base, detect_buf = 0x0, class_buf = 0x0;
    
    printf("\n");
    /* Clear the port status */
    dev_tps2386b_clr_port_status();

    for (ix = s_port; ix <= e_port; ix++)
    {
        base = ix - 1;
        /* polling status register wihthin 1 second,
         * Detection should be valid:0x4; Class should be Class0~4 */
        printf("\rPort%d is not detected", ix);
        for (jx = 0; jx < STATUS_POLLING_ROUND; jx++)
        {
            /* Get port status(Dection/Class) with offset:0x0C/0x0D/0x0E/0x0F */
            if (dev_tps2386b_get_port_detection_class(dev,
                                                      s_port,
                                                      e_port) != PASSED) {
                printf("%s:%d:Failed to get Detection/Class\n", 
                       __func__, __LINE__);
            }
     
            /* check detection/class */
            detect_buf = port_status[base].port_detection;
            class_buf = port_status[base].port_class;
            if (CHECK_DETECT_VALID(detect_buf) && 
                CHECK_CLASS_VALID(class_buf)) {
                printf("\rPort%d is detected, Class: %s\n", 
                       ix, class_info_tb[class_buf]);

                /* config power PoE plus mode with offset:0x40
                 * Class 0~3, plus mode disable
                 * Class 4  , plus mode enable  */
                dev_tps2386b_cfg_plus_mode(dev, ix, class_buf);

                /* config ICUT(cutting current) with offset:0x1E or 0x1F 
                 * Class 0~3, ICUT = 320 mA
                 * Class 4  , ICUT = 640 mA */
                dev_tps2386b_cfg_cut_current(dev, ix, class_buf);

                /* config IEEE PWR Enable with offset:0x23
                 * Class 0~3, Type1 IEEE Power enable 
                 * Class 4  , Type2 IEEE Power enable */
                dev_tps2386b_cfg_ieee_pwr(dev, ix, class_buf);

                break;
            }
            msleep(STATUS_POLLING_PERIOD);
        }
        printf("\n");
    }

    return (PASSED);
}

/*******************************************************************
 * Function:    dev_tps2386b_polling_reg
 *
 * Description: Polling a specific register.
 * Input:	dev              - pointer to the Marvell GE device.
 *              compare_op       - COMPARE_AND/COMPARE_EQL
 *		offset           - register offset.
 *		pattern          - a test pattern for data checking
 * Returns:     PASSED/FAILED
 *******************************************************************/
static int dev_tps2386b_polling_reg (dev_object_t *dev, int compare_op, 
                                     ulong reg_offset, ulong pattern)
{
    int polling_rc = FAILED, ix, size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data;
    
    for (ix = 0; ix < TPS2386B_POLLING_ROUND; ix++)
    {
        /* read register */
        if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
            printf("%s(): Failed to read register 0x%x\n", 
                    __func__, (uchar)reg_offset);
            return (FAILED);
        }

        /* compare data with pattern */
        if (compare_op == COMPARE_AND) {
            if ((rd_data & pattern) == 0) {
                polling_rc = PASSED;
                break;
            }
        } else if(compare_op == COMPARE_EQL) {
            if (rd_data == pattern) {
                polling_rc = PASSED;
                break;
            }
        } else {
            if ((rd_data & pattern) == pattern) {
                polling_rc = PASSED;
                break;
            }
        }

        /* put a delay for hardware preparation */
        msleep(TPS2386B_POLLING_PERIOD);
    }
    
    /* checking polling result */ 
    if (polling_rc != PASSED) {
            printf("%s:%d:Polling: offset:0x%x fail, "
                   "pattern:0x%x, read data:0x%x\n",  __FUNCTION__, __LINE__, 
                   (uchar)reg_offset, (uchar)pattern, (uchar)rd_data);
            return (FAILED);
    }

    return (polling_rc);
}

/*******************************************************************************
 * Function   : dev_tps2386b_cfg_plus_mode
 *
 * Description: Function to config PoE plus mode with offset:0x40
 * Inputs     : dev - Pointer to TPS2386B device object
 *              port - port number (range:1~4)
 *              class - port class (valid range:CLASS0~4)
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_cfg_plus_mode(dev_object_t *dev, int port, int class)
{
    ulong reg_offset  = TPS2386B_POEPLUS_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0, wr_data = 0x0;
    
    /* PoE plus register description:
     * ==================================================
     * = D7     D6      D5     D4      D3  D2  D1  D0   =
     * = PoE_P4 PoE_P3  PoE_P2 PoE_P1  X   X   X   TPON =
     * ==================================================
     * PoE_PX
     * 0      -> Disable plus mode
     * 1      -> Enable plus mode
     */

    /* read register */
    if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
        printf("%s(): Failed to read register 0x%x\n", 
                __func__, (uchar)reg_offset);
        return (FAILED);
    }

    /* config power PoE plus mode with offset:0x40
     * Class 0~3, plus mode disable
     * Class 4  , plus mode enable  */
    if (CHECK_CLASS_IS_4(class)) {
        /* set corresponding port bit as 1, enable */
        /* set bit[7|6|5|4] as 1 per port, bit[0] as 1 */
        wr_data = rd_data | REG_SHIFT_PORT_ENA_PLUS(port) | PORT_PLUS_TPON;
    } else if (CHECK_CLASS_IS_0TO3(class)) {
        /* set corresponding port bit as 0, enable */
        /* set bit[7|6|5|4] as 0 per port, bit[0] as 1 */
        wr_data = (rd_data & REG_SHIFT_PORT_DIS_PLUS(port)) | PORT_PLUS_TPON;
    } else {
        printf("%s:%d:Invalid class:0x%x\n", __func__, __LINE__, class);
        return (FAILED);
    }

    /* write register */
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_cfg_cut_current
 *
 * Description: Function to config ICUT(cutting current with offset:0x1E/0x1F)
 * Inputs     : dev - Pointer to TPS2386B device object
 *              port - port number (range:1~4)
 *              class - port class (valid range:CLASS 0~4)
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_cfg_cut_current(dev_object_t *dev, int port, int class)
{
    ulong reg_offset;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong rd_data = 0x0, wr_data = 0x0;
    ushort icut_pol_data = 0x0;

    /* Police 21 configuration register(0x1E) description:
     * =============================================
     * = D7 ~ D4             | D3 ~ D0             =
     * = Port 2 police value | Port 1 police value =
     * =============================================
     *
     * Police 43 configuration register(0x1F) description:
     * =============================================
     * = D7 ~ D4             | D3 ~ D0             =
     * = Port 4 police value | Port 3 police value =
     * =============================================
     *
     * ICUT(cutting current) = (N * IC_step) + IC_offs
     *
     * N = Port Police Value
     * IC_step =  20mA, if plus mode is disable
     *         =  40mA, if plus mode is enable
     * IC_offs =  20mA, if plus mode is disable
     *         = 320mA, if plus mode is enable
     *
     * Class 0~3, plus mode is disable,
     *     ICUT = 320mA = (N * 20) + 20 where N = 0xf
     *
     * Class 4  , plus mode is enable
     *     ICUT = 640mA = (N * 40) + 320 where N = 0x8
     */
    if (TPS2386B_PORT21(port)) {
        reg_offset = TPS2386B_POLICE21_CONFIG_REG;
    } else if (TPS2386B_PORT43(port)) {
        reg_offset = TPS2386B_POLICE43_CONFIG_REG;
    } else {
        printf("%s:%d:Invalid port:%d\n", __func__, __LINE__, port);
        return (FAILED);
    }

    /* read register */
    if (dev_i2c_rd(reg_offset, size, &rd_data, dev) != PASSED) {
        printf("%s(): Failed to read register 0x%x\n", 
                __func__, (uchar)reg_offset);
        return (FAILED);
    }

    /* clear corresponding POL per port */
    wr_data = rd_data & REG_SHIFT_CLR_POL(port);

    if (CHECK_CLASS_IS_4(class)) {
        /* class 4, ICUT = 640mA */
        icut_pol_data = ICUT_POL_640MA;
    } else if (CHECK_CLASS_IS_0TO3(class)) {
        /* class 0~3, ICUT = 320mA */
        icut_pol_data = ICUT_POL_320MA;
    } else {
        printf("%s:%d:Invalid class:%d\n", __func__, __LINE__, class);
        return (FAILED);
    }

    /* config ICUT pol value to corresponding nibble */
    if (TPS2386B_PORT31(port)) {
        wr_data |= icut_pol_data;
    } else {
        wr_data |= (icut_pol_data << REG_SHIFT_POL_DATA);
    }

    /* write register */
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : dev_tps2386b_cfg_ieee_pwr 
 *
 * Description: Function to config IEEE power with offset:0x23
 * Inputs     : dev - Pointer to TPS2386B device object
 *              port - port number (range:1~4)
 *              class - port class (valid range:CLASS 0~4)
 * Outputs    : PASSED/FAILED
 ******************************************************************************/
static int dev_tps2386b_cfg_ieee_pwr(dev_object_t *dev, int port, int class)
{
    ulong reg_offset = TPS2386B_IEEE_PWR_ENA_REG;
    int size = TPS2386B_REG_ONE_BYTE_ACCESS;
    ulong wr_data = 0x0, pattern = 0x0;

    /* IEEE Power Enable register description:
     * =====================================================================
     * = D7      D6      D5      D4      | D3      D2      D1      D0      =
     * = Type2 IEEE Power Enable         | Type1 IEEE Power Enable         =
     * = T2_PON4 T2_PON3 T2_PON2 T2_PON1 | T1_PON4 T1_PON3 T1_PON2 T1_PON1 =
     * =====================================================================
     * Tips:
     * 1. This is a "write-only" register. Don't read it or always get 0 value.
     * 2. If the given port is configured as Type1 or Type 2, 
     *    the type can't be changed to the other.
     * 3. The type can be changed after reset.
     */

    /* config IEEE PWR Enable with offset:0x23
     * Class 0~3, Type1 IEEE Power enable 
     * Class 4  , Type2 IEEE Power enable */
    if (CHECK_CLASS_IS_4(class)) {
        wr_data = ENA_IEEE_PWR_T2(port);
    } else if (CHECK_CLASS_IS_0TO3(class)) {
        wr_data = ENA_IEEE_PWR_T1(port);
    } else {
        printf("%s:%d:Invalid class:0x%x\n", __func__, __LINE__, class);
        return (FAILED);
    }

    /* write register */
    if (dev_i2c_wr(reg_offset, size, wr_data, dev) != PASSED) {
        printf("%s(): Failed to write register 0x%ld\n", __func__, reg_offset);
        return (FAILED);
    }

    /* After activated the IEEE power with offset:0x23, the power status
     * register(0x10) will not be updated simultaneously. Hence, should polling
     * power status to check "power is good and port is on". */

    /* polling power status register(0x10) with pattern:0x11/0x22/0x44/0x88 */
    reg_offset = TPS2386B_PWR_STAT_REG;
    pattern = REG_SHIFT_PWR_STAT(port);
    if (dev_tps2386b_polling_reg(dev, COMPARE_AND_EQL, 
                                 reg_offset, pattern) != PASSED) {
        printf("%s:%d:Port%d power is enable but power status is incorrect\n",
               __func__, __LINE__, port);
        return (FAILED);
    }
    
    printf("Port%d IEEE power is enable\n", port);
    
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: dev_tps2386b.c,v $
 * Revision 1.2  2019/01/10 06:30:25  wilbhuan
 * The beginning of TI TPS2386B PoE PSE Controller device driver.
 *
 *-------------------------------------------------
 */

/* $Id: dev_ilp.c,v 1.3 2013/11/26 08:40:32 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_ilp/dev_ilp.c,v $
 *------------------------------------------------------------------
 *
 * File name: dev_ilp.c
 *
 * Description: The device driver of In Line Power (PoE)
 *              (ltc4659, TI sn2385d, Si3456D)
 *              Source: lediag and Firebee
 *
 * April 2010 by tirawan 
 *
 * Copyright (c) 2009-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "error.h"
#include "defs.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_ilp.h"

/***********************************************************************
 *  Macro Processor Definitions
 ************************************************************************/

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

void dev_ilp_create(dev_object_t *, dev_error_report_t);

static uint32 dev_ilp_attach(dev_object_t *);
static uint32 ilp_dev_show (dev_object_t *, print_fn_t, dev_show_cmd_e);
static uint32 ilp_init(dev_object_t *);
static uint32 is_ilp_present(dev_object_t *);
static uint32 ilp_power_ports(dev_object_t *);
static uint32 ilp_clear_events(dev_object_t *);
static void print_ilp_mode(dev_object_t *, uchar, int);
static void print_ilp_port_status(uchar);
static uint32 ilp_dc_disc(dev_object_t *);
static uint32 ilp_ac_disc(dev_object_t *);
static uint32 ilp_alter_regs(dev_object_t *);
static uint32 ilp_show_regs(dev_object_t *);
static uint32 ilp_display_regs(dev_object_t *);
static uint32 ilp_register_test(dev_object_t *);
static uint32 ilp_force_interrupt(dev_object_t *);
static uint32 ilp_poe_2x_mode(dev_object_t *);
static uint32 ilp_poe_2x_enable(dev_object_t *, int, int);

/***********************************************************************
 *  Extern Functions Declaration
 ************************************************************************/

extern void msleep(int);

/***********************************************************************
 *  Global Variable
 ************************************************************************/

static dev_object_fvt_t      ilp_fvt;
static dev_ilp_callin_fvt_t  ilp_callin;
static dev_ilp_callout_fvt_t ilp_callout;

static char err_msg[500];
static char *buf = err_msg;

/* Structure for ILP registers test */
/*  name, offset, type, I2C addr,  mask, default value */

static ilp_reg_info_t ilp_reg_test_tbl[] = {
    { "Interrupt Mask       ", ILP_INT_MASK,            READ_WRITE,
       0xFF, 0xE4 },
    { "Operating Mode       ", ILP_OPERATING_MODE,      READ_WRITE,
       0xFF, 0xFF },
    { "Disconnect Enable    ", ILP_DISCONNECT_ENABLE,   READ_WRITE,
       0x0F, 0xF0 },
    { "Detect/Class Enable  ", ILP_DETECT_CLASS_ENABLE, READ_ONLY,
       0xFF, 0xFF },
    { "Timing Config        ", ILP_TIMING_CONFIG,       READ_ONLY,
       0x3F, 0x00 },
    { "Misc Config          ", ILP_MISC_CONFIG,         READ_ONLY,
       0x80, 0x00 },
    { "end                  ", 0xFF, 0xFF, 0x00, 0x00},
};

/***********************************************************************
 *  Functions
 ************************************************************************/

/* ******************************************************
 *
 * Function: ilp_poe_2x_enable
 *
 * Description: Enable or Diable PoE 2x mode (support 802.3at, 
                30W power).
 *
 * Input:    dev - pointer to the register struct.
 *           port_num - Port Number 
 *           enable - 1 to enable, 0 to disable.
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_poe_2x_enable (dev_object_t *dev, int port_num, int enable)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar addr, rd_data, wr_data;

    if (((port_num % PORTS_PER_ILP) == 0) ||
        ((port_num % PORTS_PER_ILP) == 1)){
        addr = ILP_ICUT_P1_P2; /* Port 0, 1 */
    } else {
        addr = ILP_ICUT_P3_P4; /* Port 2, 3 */
    }

    if (ilp->callout_fvt->i2c_read(addr, 
                                   &rd_data, sizeof(rd_data)
                                   , TRUE) == FAILED) {
        return (FAILED);
    }

    if (enable == TRUE) {
        wr_data = rd_data | (ICUT_MAX << (port_num % 2) * 4);
    } else {
        wr_data = rd_data & ~(ICUT_MAX << (port_num % 2) * 4);
    }

    /* Set iCut Limit */
    if (ilp->callout_fvt->i2c_write(addr, 
                                    &wr_data, 1) == FAILED) {
        return (FAILED);
    } 

    /* Set the 2x power plus (0x40) */
    if (ilp->callout_fvt->i2c_read(ILP_2X_MODE, 
                                   &rd_data, sizeof(rd_data)
                                   , TRUE) == FAILED) {
        return (FAILED);
    }
    wr_data = rd_data | (POWER_2X << port_num);
    if (ilp->callout_fvt->i2c_write(ILP_2X_MODE, 
                                    &wr_data, 1) == FAILED) {
        return (FAILED);
    } 

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_poe_2x_mode
 *
 * Description: Enable or Diable PoE 2x mode (support 802.3at, 
 *              30W power).
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_poe_2x_mode (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uint32 max_port;
    int port_num;
    uchar user_input;

    /* Check if PoE is present */
    if (is_ilp_present(dev) == FALSE) {
        sprintf(buf, "In Line Power card is not installed");
        DEV_ERROR_REPORT(dev, buf, DEV_ILP_NOT_PRESENT);
        return (FAILED);
    }

    /* Get max. port from platform */
    max_port = ilp->callout_fvt->get_max_port();

    printf("This utility is to enable/diable PoE 2X mode.\n");

    user_input = getc_answer("To enable (e) or diable (d)? (q) to exit", 
                             "edq", 'e');

    if (user_input == 'q') {
         printf("exit\n");
         return (PASSED);
    }

    if (user_input == 'e') { /* enable */
        for (port_num = 0; port_num < max_port; port_num++) {       
            if (ilp_poe_2x_enable(dev, port_num, 1) == FAILED) {
                return (FAILED);
            }
        }
    } else { /* disable */
        for (port_num = 0; port_num < max_port; port_num++) {       
            if (ilp_poe_2x_enable(dev, port_num, 0) == FAILED) {
                return (FAILED);
            }
        }
    }

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_force_interrupt
 *
 * Description: This function forces ILP chip to generate
 *              interrupt toward host CPU.
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_force_interrupt (dev_object_t *dev)
{
    uchar i2c_data = 0;
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    int timeout;
    
    /* Power off PoE before test */
    i2c_data = POWER_OFF_1;
    if (ilp->callout_fvt->i2c_write(ILP_POWER_ENABLE_PB, 
                                    &i2c_data, 1) == FAILED) {
        return (FAILED);
    } 

    /* Wait for power down and add 10ms delay */
    for (timeout = 0; timeout < ILP_PORT_OFF_TIMEOUT; timeout++) {
        if (ilp->callout_fvt->i2c_read(ILP_POWER_STATUS, 
                                        &i2c_data, 1, TRUE) == FAILED) {
            return (FAILED);
        } 

        if (i2c_data & (POWER_GOOD_1 | POWER_ENABLE_1)) {
            /* The power is still on */
            msleep(1);
        } else {
            /* The power is down now */
            msleep(10);   /* Delay per Silab suggestion */
            break;
        }
    }

    if (timeout == ILP_PORT_OFF_TIMEOUT) {
        cterr('f', 0, "ILP power down failed.");
        return (FAILED);
    }

    /* Configure to manual mode */
    i2c_data = MANUAL;
    if (ilp->callout_fvt->i2c_write(ILP_OPERATING_MODE, 
                                    &i2c_data, 1) == FAILED) {
        return (FAILED);
    } 

    /* Clear intr by reading detect event CoR (0x05) */
    if (ilp->callout_fvt->i2c_read(ILP_DETECT_EVENT_COR, 
                                    &i2c_data, 1, TRUE) == FAILED) {
        return (FAILED);
    } 

    /* Set int mask for detection done (0x08) */
    i2c_data = DETECT_COMPLETE;
    if (ilp->callout_fvt->i2c_write(ILP_INT_MASK, 
                                    &i2c_data, 1) == FAILED) {
        return (FAILED);
    } 


    /* restart detection to generate detect complete intr (0x18) */
    i2c_data = DETECT_ENABLE_1;
    if (ilp->callout_fvt->i2c_write(ILP_DET_CLASS_RESTART_PB, 
                                    &i2c_data, 1) == FAILED) {
        return (FAILED);
    } 

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_register_test
 *
 * Description: For each register from reg_ptr, this function checks for
 *              accessibility and does a ripple 1 and a ripple 0 test if
 *              applicable (not all registers are W/R register).
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_register_test (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    ilp_reg_info_t * reg_ptr = ilp_reg_test_tbl;
    int  ix;
    uchar temp, readval, data, save_val;

    prpass(testpass, "PoE Registers");

    while (reg_ptr->type != 0xFF) {
        /* Backup the current value */
        if (ilp->callout_fvt->i2c_read(reg_ptr->offset, 
                                       &save_val, 1, TRUE) == FAILED) {
            return (FAILED);
        }
        /*
         * Test a register if it's a R/W register
         */
        if (reg_ptr->type == READ_WRITE) {
            /*
             * ripple 1 test
             */
            for (ix = 0; ix < I2C_REG_WIDTH; ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                if (ilp->callout_fvt->i2c_write(reg_ptr->offset, 
                                                &temp, 1) == FAILED) {
                    return (FAILED);
                } 
                if (ilp->callout_fvt->i2c_read(reg_ptr->offset, 
                                               &readval, 1, TRUE) == FAILED) {
                    return (FAILED);
                }
                if ((readval & reg_ptr->mask) != temp) {
                    sprintf(buf, "Ripple one test failed when accessing %s "
                                 "Register at %#x."
                                 "\nExpect: %#x, Read: %#x.",
                                 reg_ptr->name, reg_ptr->offset,
                                 temp, readval);
                    DEV_ERROR_REPORT(dev, buf, DEV_ILP_REGISTER_TEST);
                    return (FAILED);
                }
            }

            /*
             * ripple 0 test
             */
            for (ix = 0; ix < I2C_REG_WIDTH; ix++) {
                temp = (1 << ix) & reg_ptr->mask;
                if (!temp) {
                    continue;
                }
                temp = (~(1 << ix)) & reg_ptr->mask;
                if (ilp->callout_fvt->i2c_write(reg_ptr->offset, 
                                                &temp, 1) == FAILED) {
                    return (FAILED);
                } 
                if (ilp->callout_fvt->i2c_read(reg_ptr->offset, 
                                               &readval, 1, TRUE) == FAILED) {
                    return (FAILED);
                }
                if ((readval & reg_ptr->mask) != temp) {
                    sprintf(buf, "Ripple zero test failed when accessing %s "
                                 "Register at %#x."
                                 "\nExpect: %#x, Read: %#x.",
                                 reg_ptr->name, reg_ptr->offset,
                                 temp, readval);
                    DEV_ERROR_REPORT(dev, buf, DEV_ILP_REGISTER_TEST);
                    return (FAILED);
                }
            }

            /*
             * pattern test
             */
            data = (uchar)PATTERN;
            for (ix = 0;ix < 2; ix++){
                temp = data & reg_ptr->mask;
                if (ilp->callout_fvt->i2c_write(reg_ptr->offset, 
                                                &temp, 1) == FAILED) {
                    return (FAILED);
                } 
                if (ilp->callout_fvt->i2c_read(reg_ptr->offset, 
                                               &readval, 1, TRUE) == FAILED) {
                    return (FAILED);
                }
                if ((readval & reg_ptr->mask) != temp) {
                    sprintf(buf, "Pattern test failed when accessing %s "
                                 "Register at %#x."
                                 "\nExpect: %#x, Read: %#x.",
                                 reg_ptr->name, reg_ptr->offset,
                                 temp, readval);
                    DEV_ERROR_REPORT(dev, buf, DEV_ILP_REGISTER_TEST);
                    return (FAIL);
                }

                data = (uchar)~PATTERN; /* complement data pattern */
            }

            /*
             * restore reset value
             */
            if (ilp->callout_fvt->i2c_write(reg_ptr->offset, 
                                            &save_val, 1) == FAILED) {
                return (FAILED);
            }
        }
        reg_ptr++;
    }

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_ac_disc
 *
 * Description: Test LTC AC Disconnect of a port on ILP. Need IEEE 802.3af
 *              compliant device. ILP interrupt generated by LTC is tested.
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_ac_disc (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar ltc_port, write_val,read_val;
    uint32 maxport;
    uchar op_mode, power_status, new_power_status, detect_class, int_mask;
    char msgbuf[80];

    printf("AC Disconnect may not work for legacy Cisco PD");
    printf(" (none IEEE 802.3af compliant).\n");

    if (is_ilp_present(dev) == FALSE) {
        printf("In Line Power card is not installed\n");
        return (FAILED);
    }

    maxport = ilp->callout_fvt->get_max_port();

    sprintf(msgbuf, "Attach PD, then enter port to be tested [0 - %d].",
            maxport - 1);

    ltc_port = gethex_answer(msgbuf, 0, 0, maxport - 1);

    /* Power on port */
    if (ilp->callout_fvt->i2c_read(ILP_OPERATING_MODE,
                                   &op_mode, sizeof(op_mode), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    op_mode &= ~(MODE_MASK << (ltc_port * 2));

    op_mode |= (MODE_AUTO << (ltc_port * 2));
    /* update the new mode */
    if (ilp->callout_fvt->i2c_write(ILP_OPERATING_MODE,
                                    &op_mode, sizeof(op_mode)) == FAILED) {
        return (FAILED);
    }

    if (ilp->callout_fvt->i2c_read(ILP_POWER_STATUS,
                                   &power_status, sizeof(power_status), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    power_status &= POWER_GOOD_MASK;

    /* Enable detect and class */
    if (ilp->callout_fvt->i2c_read(ILP_DETECT_CLASS_ENABLE,
                                   &detect_class, sizeof(detect_class), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    detect_class |= ((CLASS_ENABLE_1 + DETECT_ENABLE_1) << ltc_port);
    if (ilp->callout_fvt->i2c_write(ILP_DETECT_CLASS_ENABLE,
                                    &detect_class, 
                                    sizeof(detect_class)) == FAILED) {
        return (FAILED);
    }
    msleep(500);        /* 460 ms Detection Time tDETECT */

    /* Power on. Setup expected power status */
    power_status |= (POWER_GOOD_1 << ltc_port);
    write_val = (POWER_ENABLE_1 << ltc_port);
    if (ilp->callout_fvt->i2c_write(ILP_POWER_ENABLE_PB,
                                    &write_val, 
                                    sizeof(write_val)) == FAILED) {
        return (FAILED);
    }

    msleep(500);       /* wait for the power good status to settle */
    if (ilp->callout_fvt->i2c_read(ILP_POWER_STATUS,
                                   &new_power_status, 
                                   sizeof(new_power_status), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }

    if (power_status != (new_power_status & POWER_GOOD_MASK)) {
        /* Unable to power on */
        DEV_ERROR_REPORT(dev, "Unable to power on port",
                         DEV_ILP_UNABLE_POWER_ON);
        return (FALSE);
    }

    /* Setup LTC AC Disconnect Interrupt */
    write_val = (AC_DISCON_EN_1 << ltc_port);
    if (ilp->callout_fvt->i2c_write(ILP_DISCONNECT_ENABLE,
                                    &write_val, 
                                    sizeof(write_val)) == FAILED) {
        return (FAILED);
    }

    /* Enable Disconnect interrupt */
    if (ilp->callout_fvt->i2c_read(ILP_INT_MASK,
                                   &int_mask, 
                                   sizeof(int_mask), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }

    write_val = (int_mask | DISCONNECT);
    if (ilp->callout_fvt->i2c_write(ILP_INT_MASK,
                                    &write_val, 
                                    sizeof(write_val)) == FAILED) {
        return (FAILED);
    }

    msleep(500);        /* Wait for 500ms */

    /*Clear the ILP_FAULT_EVENT */
    if (ilp->callout_fvt->i2c_read(ILP_FAULT_EVENT_COR,
                                   &read_val, 
                                   sizeof(read_val), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }

    /* Request user to remove the cable */
    if (getc_answer("Remove the PD cable and then enter y", "yn", 'y')
                    != 'y') {
        DEV_ERROR_REPORT(dev, "Skipped", WARNING | DEV_ILP_SKIPPED);
        return (FAILED);
    }

    /* Check for AC Disconnect */
    msleep(2 * ONE_SECOND);        /* Wait for 2 seconds */
    if (ilp->callout_fvt->i2c_read(ILP_FAULT_EVENT,
                                   &read_val, 
                                   sizeof(read_val), 
                                   TRUE) == FAILED) {
        return (FAILED);
    }

    if((read_val >> ltc_port) != 0x10) {
        printf("Did not receive any Disconnect Status!!!!!\n");
    } else {
        printf("Received Port %d Disconnect\n", ltc_port);
    }

    return (PASSED);
}

/* ******************************************************
 *
 * Function: ilp_dc_disc
 *
 * Description: Enable or disable ILP DC Disconnect one port at a time,
 *              or all ports at a time.
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_dc_disc (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    int port, maxport;
    uchar enable, disc, write_val;

    if (is_ilp_present(dev) == FALSE) {
        printf("In Line Power card is not installed\n");
        return (FAILED);
    }

    if (getc_answer("DC disconnect all ports? (y/n)", "yn", 'y') == 'y') {
        /* Enable/Disable all ports */
        /* Read current Disconnect Enable register */
        if (ilp->callout_fvt->i2c_read(ILP_DISCONNECT_ENABLE,
                                       &disc, sizeof(disc), TRUE) == FAILED) {
            return (FAILED);
        }

        enable = getc_answer("Enable or Disable DC Disconnect? (e/d)",
                             "ed", 'd');

        if (enable == 'd') {
            /* Disable */
            write_val = (disc & (~DC_DIS_ALL));
            if (ilp->callout_fvt->i2c_write(ILP_DISCONNECT_ENABLE, &write_val, 
                                            sizeof(write_val)) == FAILED) {
                return (FAILED);
            }
        } else {
            /* Enable */
            write_val = (disc | DC_DIS_ALL); 
            if (ilp->callout_fvt->i2c_write(ILP_DISCONNECT_ENABLE, &write_val, 
                                            sizeof(write_val)) == FAILED) {
                return (FAILED);
            }
        }
    } else {
        /* Enable/Disable individual port */
        maxport = ilp->callout_fvt->get_max_port(); 

        /* Enable/Disable individual port */
        for (port = 0; port < maxport; port++) {
            /* First LTC */
            if (ilp->callout_fvt->i2c_read(ILP_DISCONNECT_ENABLE, &disc, 
                                           sizeof(disc), TRUE) == FAILED) {
                return (FAILED);
            }
            printf(" Port %d: \n", port);
            enable = getc_answer("Enable or Disable DC Disconnect? (e/d)",
                                        "ed", 'd');
            if (enable == 'd') {
                /* Disable */
                disc &= (~(DC_DIS_ENABLE_1 << (port % 4)));
            } else {
                /* Enable */
                disc |= (DC_DIS_ENABLE_1 << (port % 4));
            }
            if (ilp->callout_fvt->i2c_write(ILP_DISCONNECT_ENABLE, &disc, 
                                            sizeof(disc)) == FAILED) {
                return (FAILED);
            }
        }
    }

    return (PASSED);
}

/* ******************************************************
 *
 * Function: ilp_clear_events
 *
 * Description: Clear a given ILP events.
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_clear_events (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar events, sizeof_events;

    sizeof_events = sizeof(events);

    /* Read to clear the LTC events */
    if (ilp->callout_fvt->i2c_read(ILP_POWER_EVENT_COR,
          &events, sizeof_events, TRUE) == FAILED) {
        return (FAILED);
    }
    
    if (ilp->callout_fvt->i2c_read(ILP_DETECT_EVENT_COR,
         &events, sizeof_events, TRUE) == FAILED) {
        return (FAILED);
    }
    
    if (ilp->callout_fvt->i2c_read(ILP_FAULT_EVENT_COR,
          &events, sizeof_events, TRUE) == FAILED) {
        return (FAILED);
    }
    
    if (ilp->callout_fvt->i2c_read(ILP_TSTART_EVENT_COR,
          &events, sizeof_events, TRUE) == FAILED) {
        return (FAILED);
    }
    
    if (ilp->callout_fvt->i2c_read(ILP_SUPPLY_EVENT_COR,
          &events, sizeof_events, TRUE) == FAILED) {
        return (FAILED);
    }
    
    return (PASSED);
}


/* ******************************************************
 *
 * Function: print_ilp_mode
 *
 * Description: Display the operating mode as indicated by the ILP device
 *
 * Input:    data - ILP operating mode register content.
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static void print_ilp_mode (dev_object_t *dev, uchar mode, int new_line)
{
    switch (mode) {
    case SHUTDOWN:
        printf("shutdown");
        break;
    case MANUAL:
        printf("manual");
        break;
    case SEMIAUTO:
        printf("semiauto");
        break;
    case MODE_AUTO:
        printf("auto");
        break;
    default:
        sprintf(buf, "%s(): Invalid mode %#x", __FUNCTION__, mode);
        DEV_ERROR_REPORT(dev, buf, DEV_ILP_INVALID_MODE);
        break;
    }

    if (new_line) {
        printf("\n");
    }
}  


/* ******************************************************
 *
 * Function: print_ilp_port_status
 *
 * Description: Display port status as indicated by the ILP
 *
 * Input:    data - ILP port status register content.
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static void print_ilp_port_status (uchar data)
{
    printf("        Class        = ");
    switch ((data & CLASS_MASK) >> 4) {
    case UNKNOWN_CLASS:
        printf("class status unknown        ");
        break;
    case CLASS1:
        printf("class 1                        ");
        break;
    case CLASS2:
        printf("class 2                        ");
        break;
    case CLASS3:
        printf("class 3                        ");
        break;
    case CLASS4:
        printf("class 4                        ");
        break;
    case CLASS0:
        printf("class 0                        ");
        break;
    case OVERCURRENT:
        printf("overcurrent                ");
        break;
    default:
        printf("undefined - read as 0        ");
        break;
    }

    printf("Detect        = ");
    switch (data & DETECT_MASK) {
    case UNKNOWN_DETECT:
        printf("detect status unknown");
        break;
    case SHORT_CIRCUIT:
        printf("short circuit (<1V)");
        break;
    case RLOW:
        printf("RLOW (<15K)");
        break;
    case DETECT_GOOD:
        printf("detect good (15K < R < 33K)");
        break;
    case RHIGH:
        printf("RHIGH (>33K)");
        break;
    case OPEN_CIRCUIT:
        printf("open circuit");
        break;
    default:
        printf("reserved");
        break;
    }
    printf("\n");
}


/* ******************************************************
 *
 * Function: ilp_power_ports
 *
 * Description: Power on or off all ports of a given ILP.
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_power_ports (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar op_mode, port_opmode, power_status, new_power_status, port_status;
    uchar detect_event, write_val;
    static uchar phone_type[PORTS_PER_ILP];
    uchar curr_lsb = 0, curr_msb = 0, volt_lsb = 0, volt_msb = 0;
    uint power;
    uint port, base_port = 0, cisco_phone, user_input;

    uint32_t test_port = 0, test_end = 0;
    test_port = getdec_answer("\nWhich port you want to test[0, 1, or '2' for all]:", 0, 0, 2);
    
    if ((test_port == 0) || (test_port == 1)) {
        test_end = (test_port + 1);
    } else {
        test_end = ilp->callout_fvt->get_max_port(); 
    }

    if (is_ilp_present(dev) == FALSE) {
        printf("In Line Power card is not installed\n");
        return (FAILED);
    }

    /* Clear pending LTC events */
    if (ilp_clear_events(dev) == FAILED) { 
        return (FAILED);
    }

    /* Check for the operating mode for each ports. */
    if (ilp->callout_fvt->i2c_read(ILP_OPERATING_MODE,
                                   &op_mode, 
                                   sizeof(op_mode), TRUE) == FAILED) {
        return (FAILED);
    }


    for (port = test_port; port < test_end; port++) {
        printf("\nILP port %d: Mode = ", port );
        print_ilp_mode(dev, (op_mode >> (port * 2)) & MODE_MASK, FALSE);
        if (((op_mode >> (port * 2)) & MODE_MASK) == SHUTDOWN) {
            /* Shutdown mode cannot power on/off the port. Change the mode */
            printf("\n To power on or off the port, ");
            printf(" the operating mode cannot be in the shutdown mode.\n");
            printf("The operating mode code: ");
            printf(" %d for Manual. %d for Semi-auto. %d for Auto\n",
                        MANUAL, SEMIAUTO, MODE_AUTO);
            /* Shutdown mode is 00, so clearing is not needed. Otherwise;*/
            /* Clear Shutdown mode*/
            op_mode &= (~(MODE_MASK << (port * 2)));
            port_opmode = getdec_answer("Enter the new operating mode:",
                                SEMIAUTO, MANUAL, MODE_AUTO) & MODE_MASK;
            op_mode |= (port_opmode << (port * 2));
            /* update the new mode */ 
            if (ilp->callout_fvt->i2c_write(ILP_OPERATING_MODE, &op_mode, 
                                            sizeof(op_mode)) == FAILED) {
                return (FAILED);
            }
        }
    }

    /* Enable Disconnect */
    write_val = DC_DIS_ALL;
    if (ilp->callout_fvt->i2c_write(ILP_DISCONNECT_ENABLE,
                                    &write_val, sizeof(write_val)) == FAILED) {
        return (FAILED);
    }

    printf("\n\nWARNING: Perform PoE power on with loopback cable(s) "
           "may damage the PHY component!! \n"  
           "         Please make sure you have correct PD(s).\n");

    user_input = getc_answer("To power on (i) or off (o)? (q) to exit", 
                             "ioq", 'i');
    if (user_input == 'q') {
        return (PASSED);
    }

    /* Power On the port */
    if (user_input == 'i') {
        for (port = test_port; port < test_end; port++) {
            cisco_phone = FALSE;        /* Assume 802.3af */
            /* Get the operating mode of the port */
            port_opmode = (op_mode >> (port * 2)) & MODE_MASK;
            if (ilp->callout_fvt->i2c_read(ILP_POWER_STATUS, &power_status, 
                                           sizeof(power_status), 
                                           TRUE) == FAILED) {
                return (FAILED);
            }
            power_status &= POWER_GOOD_MASK;
            printf("\n Port %d powered %-3s. \n", base_port + port,
                    (power_status & (POWER_GOOD_1 << port)) ? "On" : "Off");
            if (power_status & (POWER_GOOD_1 << port)) {
                /* Already powered on */
                printf("Power already on. ");
                printf("Reading the Class/Detect set earlier\n");
            } else {
                /* Was powered off */
                phone_type[port] = NO_PHONE;   /* Init as no phone */
                /* Enable Class and Detect */
                printf("Powering on port %d ...\n", base_port + port);
                write_val = ((CLASS_ENABLE_1 + DETECT_ENABLE_1) << port);
                if (ilp->callout_fvt->i2c_write(ILP_DET_CLASS_RESTART_PB, 
                                                &write_val, 
                                                sizeof(write_val)) == FAILED) {
                    return (FAILED);
                }
                msleep(600);        /* 460 ms Detect time tDETECT */

                /* Check for detect complete */
                if (ilp->callout_fvt->i2c_read(ILP_DETECT_EVENT, 
                                               &detect_event, 
                                               sizeof(detect_event),
                                               TRUE) == FAILED) {
                    return (FAILED);
                }
                if (detect_event & (DETECT_COMPLETE_1 << port)) {
                    /* Detect complete */
                    /* Wait for Class to complete */
                    switch (port_opmode) {
                    case MANUAL:
                        /* Manual mode */
                        msleep(300);        /* 265 ms tCLASS */
                        break;
                    default:
                        /* Semi-auto and Auto mode */
                        printf("\rUp to 5 seconds delay");
                        msleep(5000);        /* 47 ms tCLASS But Cisco IP
                                         * phone needs more time */
                        break;
                    }

                    /* Check for class complete */
                    if (ilp->callout_fvt->i2c_read(ILP_DETECT_EVENT, 
                                                   &detect_event, 
                                                   sizeof(detect_event),
                                                   TRUE) == FAILED) {
                        return (FAILED);
                    }
                    if (detect_event & (CLASS_COMPLETE_1 << port)) {
                        /* Class complete */
                        phone_type[port] = IEEE_PHONE;
                    } else {
                        /* Unable to get the class */
                        /* Check for short circuit */
                        if (ilp->callout_fvt->i2c_read(ILP_PORT1_STATUS, 
                                                       &port_status, 
                                                       sizeof(port_status),
                                                       TRUE) == FAILED) {
                            return (FAILED);
                        }
                        if ((port_status & DETECT_MASK) == SHORT_CIRCUIT) {
                            printf("\rWarning: Short circuit detected.\n"
                                   "Please make sure no loopback cable and "
                                   "check the circuit.\n");
                            phone_type[port] = NO_PHONE;
                            return (FAILED);
                        }
                        /* Check for Cisco PD */
                        if (ilp->callout_fvt->phone_detect(base_port + port)) {
                            printf("\rCisco PD detected\n");
                            cisco_phone = TRUE;
                            phone_type[port] = CISCO_PHONE;
                        } else {
                            /* Not Cisco phone */
                            DEV_ERROR_REPORT(dev, "Unable to read the class",
                                             DEV_ILP_UNABLE_READ_CLASS);
                            phone_type[port] = NO_PHONE;
                        }
                    }
                } else {
                    /* Unable to detect */
                    /* Check for short circuit */
                    if (ilp->callout_fvt->i2c_read(ILP_PORT1_STATUS, 
                                                   &port_status, 
                                                   sizeof(port_status),
                                                   TRUE) == FAILED) {
                        return (FAILED);
                    }
                    if ((port_status & DETECT_MASK) == SHORT_CIRCUIT) {
                        printf("\rWarning: Short circuit detected.\n"
                               "Please make sure no loopback cable and "
                               "check the circuit.\n");
                        phone_type[port] = NO_PHONE;
                        return (FAILED);
                    }
                    /* Check for Cisco PD */
                    if (ilp->callout_fvt->phone_detect(base_port + port)) {
                        printf("\rCisco PD detected\n");
                        cisco_phone = TRUE;
                        phone_type[port] = CISCO_PHONE;
                    } else {
                        /* Not Cisco phone */
                        phone_type[port] = NO_PHONE;
                        if (port_opmode == MANUAL) {
                            /* For manual mode, Detect does not have to
                             * complete 
                             */
                            DEV_ERROR_REPORT(dev, 
                                      "Detect not complete in manual mode",
                                      WARNING | DEV_ILP_MANUAL_MODE);
                        } else {
                            /* For Semi-auto or Auto mode, Detect has to
                                                     complete */
                            DEV_ERROR_REPORT(dev, "Unable to detect",
                                             DEV_ILP_UNABLE_DETECT);
                        } /* endof if port_opmode */
                    } /* endof if phone_detect */
                } /* endof if (detect_event) */

                /* If Cisco PD, check for the mode */
                if ((cisco_phone == TRUE) && (port_opmode == MODE_AUTO)) {
                    /* Cisco phone in auto mode, change it to semi-auto mode */
                    printf("Cisco PD cannot operate in Auto mode.");
                    printf(" Switching to Semiauto mode\n");
                    /* clear the mode bit */
                    op_mode &= (~(MODE_MASK << (port * 2)));

                    port_opmode = SEMIAUTO;
                    op_mode |= (port_opmode << (port * 2));
                    if (ilp->callout_fvt->i2c_write(ILP_OPERATING_MODE, 
                                                    &op_mode, 
                                                    sizeof(op_mode)) 
                                                    == FAILED) {
                        return (FAILED);
                    }
                    msleep(10);
                }

                /* Power on the port. If in auto mode, and if class complete,
                        power on is automatic */
                if ((port_opmode != MODE_AUTO)
                    || (detect_event & (CLASS_COMPLETE_1 << port))) {
                    /* Power on the port */
                    write_val = (POWER_ON_1 << port);
                    if (ilp->callout_fvt->i2c_write(ILP_POWER_ENABLE_PB, 
                                                    &write_val, 
                                                    sizeof(write_val))
                                                    == FAILED) {
                        return (FAILED);
                    }
                }

            } /* endof if (power_status) */

            if (cisco_phone == FALSE) {
                /* Read Class from Port status register */
                if (ilp->callout_fvt->i2c_read(port + ILP_PORT1_STATUS, 
                                               &port_status, 
                                               sizeof(port_status),
                                               TRUE) == FAILED) {
                    return (FAILED);
                }
                if (port_opmode == MANUAL) {
                    printf("Manual mode. ");
                    printf("Class/Detect shown may not be accurate\n");
                }
                print_ilp_port_status(port_status); 
            }

            /* setup expected power status */
            power_status |= (POWER_GOOD_1 << port);

            /* wait for the power good status to settle tDIS - 720 ms max */
            msleep(800);
            if (ilp->callout_fvt->i2c_read(ILP_POWER_STATUS, &new_power_status, 
                                           sizeof(new_power_status), 
                                           TRUE) == FAILED) {
                return (FAILED);
            }
            if ((power_status & (POWER_GOOD_1 << port)) != (new_power_status &
                 POWER_GOOD_MASK & (POWER_GOOD_1 << port))) {
                /* Not the expected power status. turn on yellow LED */
                ilp->callout_fvt->ilp_led(port, ILP_YELLOW_LED); 
                sprintf(buf, "Expect power status 0x%2X, received 0x%2X",
                             power_status, new_power_status & POWER_GOOD_MASK);
                DEV_ERROR_REPORT(dev, buf, DEV_ILP_POWER_STATUS);
            } else {
                /* power state verified */
                if (power_status & (POWER_GOOD_1 << port)) {
                    /* Power on. Turn on green LED */
                    ilp->callout_fvt->ilp_led(port, ILP_GREEN_LED); 
                } else {
                    /* Power off. Turn off LED */
                    ilp->callout_fvt->ilp_led(port, ILP_LED_OFF); 
                }
            } /* endof if (power_status) */
        } /* endof for */
        msleep(5000); /* 5 sec delay before display power */
    } else {
        /* Power off */
        for (port = 0; port < ilp->callout_fvt->get_max_port(); port++) {
            phone_type[port]= NO_PHONE;
            printf("Powering off port %d ...\n", port);
            
            power_status &= ~(POWER_GOOD_1 << port);
            write_val = (POWER_OFF_1 << port);
            if (ilp->callout_fvt->i2c_write(ILP_POWER_ENABLE_PB, &write_val, 
                                            sizeof(write_val)) == FAILED) {
                return (FAILED);
            }

            /* Power off. Turn off led */
        }
    }

    /* Summarize the port status*/
    printf("\n\nPort\tMode\tOn/Off\tPhone\tClass\tPower(Watt)\t\n");
    for (port = 0; port < ilp->callout_fvt->get_max_port(); port++) {
        /* Print port num */
        printf("%d\t", port);

        if (ilp->callout_fvt->i2c_read(ILP_OPERATING_MODE, &op_mode, 
                                       sizeof(op_mode), 
                                       TRUE) == FAILED) {
            return (FAILED);
        }

        switch ((op_mode >> (port * 2)) & MODE_MASK) {
        case SHUTDOWN:
            printf("shutdown\t");
            break;
        case MANUAL:
            printf("manual\t");
            break;
        case SEMIAUTO:
            printf("semiauto\t");
            break;
        case MODE_AUTO:
            printf("auto\t");
            break;
        default:
            printf("unknown\t ");
            break;
        }

        /* Print Power Status On/Off */
        if (ilp->callout_fvt->i2c_read(ILP_POWER_STATUS, &power_status, 
                                       sizeof(power_status), 
                                       TRUE) == FAILED) {
            return (FAILED);
        }
        power_status &= POWER_GOOD_MASK;
        printf("%-3s\t", (power_status & (POWER_GOOD_1 << port)) ? 
               "On" : "Off");

        /* Print Phone Type */
        switch (phone_type[port]) {
        case NO_PHONE:
            printf("None\n");
            continue;
        case CISCO_PHONE:
            printf("Cisco\t");
            break;
        case IEEE_PHONE:
            printf("IEEE\t");
            break;
        default:
            printf("unknown\n");
            continue;
        }

        /* Print Class Status */
        if (ilp->callout_fvt->i2c_read(ILP_PORT1_STATUS + port, &port_status, 
                                       sizeof(port_status), 
                                       TRUE) == FAILED) {
            return (FAILED);
        }
        switch ((port_status & CLASS_MASK) >> 4) {
        case UNKNOWN_CLASS:
            printf("unknown\t");
            break;
        case CLASS1:
            printf("1\t");
            break;
        case CLASS2:
            printf("2\t");
            break;
        case CLASS3:
            printf("3\t");
            break;
        case CLASS4:
            printf("4\t");
            break;
        case CLASS0:
            printf("0\t");
            break;
        case OVERCURRENT:
            printf("overcurrent\t");
            break;
        default:
            printf("\t");
            break;
        }

        /* Print Power */
        if (power_status & (POWER_GOOD_1 << port)) { /* print if power on*/
            if (ilp->callout_fvt->i2c_read(ILP_IDC_P1_LSB + 4 * port, 
                                           &curr_lsb, 
                                           sizeof(curr_lsb), 
                                           TRUE) == FAILED) {
                return (FAILED);
            }
            if (ilp->callout_fvt->i2c_read(ILP_IDC_P1_MSB + 4 * port, 
                                           &curr_msb, 
                                           sizeof(curr_msb), 
                                           TRUE) == FAILED) {
                return (FAILED);
            }
            if (ilp->callout_fvt->i2c_read(ILP_VDC_P1_LSB + 4 * port, 
                                           &volt_lsb, 
                                           sizeof(volt_lsb), 
                                           TRUE) == FAILED) {
                return (FAILED);
            }
            if (ilp->callout_fvt->i2c_read(ILP_VDC_P1_MSB + 4 * port, 
                                           &volt_msb, 
                                           sizeof(volt_msb), 
                                           TRUE) == FAILED) {
                return (FAILED);
            }

            power = ((uint)curr_lsb + ((uint)curr_msb << 8)) * 
                    ((uint)volt_lsb + ((uint)volt_msb << 8)) / 1000 * 361;
            printf("%2d.%02dw\n", (power/1000000), (power%1000000));
        } else {
            printf("\n");
        }
    }

    return (PASSED);
}



/**********************************************************************
 *
 * Function: ilp_init
 *
 * This function initialize the Inline Power card.  tSTART and tICUT of
 * are initialized to 120 ms. Pending Supply event interrupt
 * is cleared.
 *
 * Input : pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 **********************************************************************
 */
static uint32 ilp_init (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar ilp_reg;

    /* Change tSTART and tICUT to 120 ms */
    ilp_reg = (TSTART_120MS + TICUT_120MS);
    if (ilp->callout_fvt->i2c_write(ILP_TIMING_CONFIG, &ilp_reg,
                                    sizeof(ilp_reg)) == FAILED) {
        return (FAILED);
    }

    /* OSC Fail in Supply Event of ILP will be set,
        after reset released */
    /* Read Supply Event register to clear it */
    if (ilp->callout_fvt->i2c_read(ILP_SUPPLY_EVENT_COR, &ilp_reg, 
                                   sizeof(ilp_reg), TRUE) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: is_ilp_present
 *
 * Check if the inline power card is installed
 *
 * Input :   pointer to the register struct.
 *
 * Output: TRUE - installed. FALSE - not present.
 *
 **********************************************************************
 */
static uint32 is_ilp_present (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar power_status = 0;

    prpass(testpass, "ILP Presence Detect");
    if (ilp->callout_fvt->i2c_read((uchar)ILP_POWER_STATUS, 
                                  &power_status, 1, TRUE) == FAILED ) {
        sprintf(buf, "ILP Module is absent or not responding.");
        DEV_ERROR_REPORT(dev, buf, WARNING | DEV_ILP_NOT_PRESENT);
        return (FALSE);
    } else {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nInline Power is PRESENT.\n");
        }
        return (TRUE);
    }
}


/*****************************************************************
 * Name: ilp_dev_show
 *
 * Description: Provide platforms with a mechanism to display some common
 *              device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the ILP device
 *        A device print function vector
 *        A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *****************************************************************/
static uint32 ilp_dev_show (dev_object_t *dev, print_fn_t dev_print, 
                            dev_show_cmd_e cmd)
{
    switch (cmd) {
    case DEV_SHOW_ALL:
    case DEV_SHOW_REGISTERS:
        if (ilp_show_regs(dev)) {
            return (FAILED);
        }
        break;
    default:
        assert(!"ilp_dev_show");
    }

    return (PASSED);
}


/* ******************************************************
 *
 * Function: ilp_display_regs
 *
 * Description: Display one of ILP's registers 
 *
 * Input:   pointer to the register struct. 
 *
 * Outputs:  None.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_display_regs (dev_object_t *dev)
{
    uchar val = 0;
    uint offset;
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;

    offset = ILP_INTERRUPT;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_INTERRUPT               reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_INT_MASK;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_INT_MASK                reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_EVENT;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_POWER_EVENT             reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_EVENT_COR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_POWER_EVENT_COR         reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DETECT_EVENT;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_DETECT_EVENT            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DETECT_EVENT_COR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_DETECT_EVENT_COR        reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_FAULT_EVENT;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_FAULT_EVENT             reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_FAULT_EVENT_COR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_FAULT_EVENT_COR         reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_TSTART_EVENT;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_TSTART_EVENT            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_TSTART_EVENT_COR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_TSTART_EVENT_COR        reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_SUPPLY_EVENT;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_SUPPLY_EVENT            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_SUPPLY_EVENT_COR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_SUPPLY_EVENT_COR        reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT1_STATUS;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PORT1_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT2_STATUS;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PORT2_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT3_STATUS;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PORT3_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PORT4_STATUS;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PORT4_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_STATUS;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_POWER_STATUS            reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_PIN_STATUS;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PIN_STATUS              reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_OPERATING_MODE;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_OPERATING_MODE          reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DISCONNECT_ENABLE;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_DISCONNECT_ENABLE       reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DETECT_CLASS_ENABLE;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_DETECT_CLASS_ENABLE     reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_TIMING_CONFIG;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_TIMING_CONFIG           reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_MISC_CONFIG;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_MISC_CONFIG             reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_DET_CLASS_RESTART_PB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_DET_CLASS_RESTART_PB    reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_POWER_ENABLE_PB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_POWER_ENABLE_PB         reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_GLOBAL_PB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_GLOBAL_PB               reg %#.2x = %#.2x\n", offset, val);
    
    offset = ILP_ID;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_ID                      reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PWRPR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PWRPR                   reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_ICUT_P1_P2;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_ICUT_P1_P2              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_ICUT_P3_P4;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_ICUT_P3_P4              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P1_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P1_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P1_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P1_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P1_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P1_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P1_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P1_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P2_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P2_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P2_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P2_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P2_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P2_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P2_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P2_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P3_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P3_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P3_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P3_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P3_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P3_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P3_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P3_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P4_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P4_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_IDC_P4_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_IDC_P4_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P4_LSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P4_LSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_VDC_P4_MSB;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_VDC_P4_MSB              reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_2X_MODE;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_2X_MODE                 reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_FIRMWARE_REV;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_FIRMWARE_REV            reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_WATCHDOG;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_WATCHDOG                reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_DEVICE_ID;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_DEVICE_ID               reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PUSHBOTTON1;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PUSHBOTTON1             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PUSHBOTTON2;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PUSHBOTTON2             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PUSHBOTTON3;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PUSHBOTTON3             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PUSHBOTTON4;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PUSHBOTTON4             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PWR_PD_DIS1;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PWR_PD_DIS1             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PWR_PD_DIS2;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PWR_PD_DIS2             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PWR_PD_DIS3;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PWR_PD_DIS3             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PWR_PD_DIS4;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PWR_PD_DIS4             reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_PORT_RST_INTR;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_PORT_RST_INTR           reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_INTR_CNTL;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_INTR_CNTL               reg %#.2x = %#.2x\n", offset, val);

    offset = ILP_TIMER_VALUE;
    if (ilp->callout_fvt->i2c_read(offset, 
                                   &val, 
                                   sizeof(val),
                                   TRUE) == FAILED) {
        return (FAILED);
    }
    printf("ILP_TIMER_VALUE             reg %#.2x = %#.2x\n", offset, val);

    return (PASSED);;
}


/* ******************************************************
 *
 * Function: ilp_alter_regs
 *
 * Description: Change ILP register
 *
 * Input:    pointer to the register struct.
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_alter_regs (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;
    uchar offset, data, read_data;
	
    if (is_ilp_present(dev) == FALSE) {
        printf("In Line Power card is not installed\n");
        return (FAILED);
    }

    offset = gethex_answer("Enter in hex the register offset",
                           0, 0, ILP_MAX_REG);

    if (ilp->callout_fvt->i2c_read(offset, &read_data,
                                   sizeof(read_data), TRUE) == FAILED) {
        return (FAILED);
    }
    data = gethex_answer("Enter in hex the pattern to be written",
                         read_data, 0, 0xFF);

    return (ilp->callout_fvt->i2c_write((uchar)offset,
                                        &data, sizeof(data)));
}


/* ******************************************************
 *
 * Function: ilp_show_regs
 *
 * Description: Display ILP's all LTC's registers
 *
 * Input: pointer to the register struct. 
 *
 * Outputs:  PASSED - No errors encountered.
 *             FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************
 */
static uint32 ilp_show_regs (dev_object_t *dev)
{
    if (is_ilp_present(dev) == FALSE) {
        printf("In Line Power card is not installed\n");
        return (FAILED);
    }
    return (ilp_display_regs(dev));
}


/*****************************************************************
 * Name: dev_ilp_attach
 *
 * Description: Attach the ILP Common Driver for use. This
 *              function will initialize and setup all necessary pointers
 *              and bring the chip to operation.
 *              If memory space is required for this device, malloc() should be
 *              called in this function and return FAILED if malloc() operation
 *              fails. The malloc() is called only once at the first entrance
 *              when dev_state=DEV_STATE_CREATE. Once dev_state is changed,
 *              no malloc() is allowed.
 *
 * Input: Pointer to the ILP device object
 *
 * Returns: none
 *****************************************************************
 */
static uint32 dev_ilp_attach (dev_object_t *dev)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;

    ilp->base.dev_state = DEV_STATE_ATTACH;

    ilp->callin_fvt = &ilp_callin;
    ilp->callin_fvt->is_ilp_present  = is_ilp_present;
    ilp->callin_fvt->ilp_init        = ilp_init;
    ilp->callin_fvt->ilp_power_port  = ilp_power_ports;
    ilp->callin_fvt->dc_disc         = ilp_dc_disc;
    ilp->callin_fvt->ac_disc         = ilp_ac_disc;
    ilp->callin_fvt->show_ilp_regs   = ilp_show_regs;
    ilp->callin_fvt->alter_ilp_reg   = ilp_alter_regs;
    ilp->callin_fvt->register_test   = ilp_register_test;
    ilp->callin_fvt->poe_2x_mode     = ilp_poe_2x_mode;
    ilp->callin_fvt->force_interrupt = ilp_force_interrupt;
    
    /* init the callout function */
    ilp->callout_fvt = &ilp_callout;

    return (PASSED);
}


/*****************************************************************
 * Name: dev_ilp_create
 *
 * Description: Newly create object with various device function
 *              point to "do nothing" and then initialize all of the
 *              appropriate function.
 *
 * Input: dev_object_t pointer to the Linear 4259 device
 *        dev_error_report_t This is a callout function provided
 *        by/for the platform
 *
 * Returns: none
 *****************************************************************
 */
void dev_ilp_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_ilp_object_t * const ilp = (dev_ilp_object_t *)dev;

    ilp->base.dev_state = DEV_STATE_CREATE;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, &ilp_fvt);

    ilp->base.dev_object_fvt->dev_attach = dev_ilp_attach;
    ilp->base.dev_object_fvt->dev_error_report = error_report_fn;
    ilp->base.dev_object_fvt->dev_name = "ILP Common Driver";
    ilp->base.dev_object_fvt->dev_show = ilp_dev_show;
}


/******** History ******** 
$Log: dev_ilp.c,v $
Revision 1.3  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.2  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/

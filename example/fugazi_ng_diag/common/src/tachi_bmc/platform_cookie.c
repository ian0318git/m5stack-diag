/* $Id: platform_cookie.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_cookie.c,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.c - Platform cookie 
 *
 * June 2015, Times Huang ported from Overlord
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "proto.h"
#include "error.h"
#include "menu.h"
#include "cookie_4.h"
#include "cross_platform.h"
#include "platform_cookie.h"
#include "platform_i2c.h"
#include "plat_defs.h"
#include "diag_i2c_api.h"
#include "diag_fpga_lib.h"
#include "ngio.h"

void i2c_act2_reset(sc_context *);

static int i2c_act2_write_bytes(sc_context *, char *, int);
static int i2c_act2_read_bytes(sc_context *, char *);

static char smc_buf[80];
static char i2c_err[80];

extern struct ngio_intf_t *slot_get_ngiowic(int);
extern int slot_i2c_unreset(struct ngio_intf_t *, int, char *);

/**********************************************************************
 *
 * Function: plat_init_smart_eeprom_context
 *
 * Description:
 *           intializes sc_context.
 * Input:  con_p   - pointer to sc_context
 *         type    - type of module (ie, aim, mb, wic, etc)
 *         slot    - slot
 *         cookie_p- pointer to eeprom data
 *
 * Output: PASS/FAIL
 *
 **********************************************************************
 */
int plat_init_smart_eeprom_context (sc_context *con_p, uchar type,
                                    uchar slot, uchar *cookie_p)
{
    int retval = PASSED;
    struct ngio_intf_t *ngiowic;

    *i2c_err = '\0';
    
    con_p->info_string = smc_buf;
    
    switch (type) {
    case MOTHER_BOARD: /* I2C interface */
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C; 
        con_p->dev_if_p->parm2 = (uint8_t)MB_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)0;    /* TBI */
        con_p->dev_if_p->parm4 = (uint8_t)I2C_CTRL_ZERO;   /* TBI */
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "MB");
        break;
    case WIC_MODULE:
    case WIC_DAUGHTER_CARD:
        ngiowic = (struct ngio_intf_t *)slot_get_ngiowic(slot);
        if (slot_i2c_unreset(ngiowic, slot, "NIM") == FAILED) {
            printf("%s: NIM I2C Unreset failed\n", __func__);
            return (FAILED);
        }
        con_p->type = type;
        con_p->slot = slot;        /* slot = 0 */
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C; 
        con_p->dev_if_p->parm2 = (uint8_t)NIM_I2C_ADDR_ACT2;   
        con_p->dev_if_p->parm3 = (uint8_t)0;
        con_p->dev_if_p->parm4 = get_wic_i2c_ctrl(slot);
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "NIM");
        break;
    case PSU_MODULE:
        con_p->type = type;
        con_p->slot = slot ;
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C; 
        con_p->dev_if_p->parm2 = (uint8_t)POE_I2C_ADDR_ACT2;
        con_p->dev_if_p->parm3 = (uint8_t)0;
        con_p->dev_if_p->parm4 = (uint8_t)I2C_CTRL_NIGHT;
        con_p->dev_if_p->interface = SCC_I2C_IF;
        sprintf((char *)smc_buf, "PoE");
        break;

    case ISP_CARD:
        con_p->type = type;
        con_p->slot = slot ;
        con_p->cookie_contents = cookie_p;
        con_p->quack_read_2bytes = (PFT)i2c_act2_read_bytes;
        con_p->quack_write_2bytes = (PFT)i2c_act2_write_bytes;
        con_p->quack_reset = (PFT)i2c_act2_reset;
        con_p->dev_if_p->parm1 = (uint8_t)IOFPGA_I2C; 
        con_p->dev_if_p->parm2 = (uint8_t)get_daughter_card_i2c_addr(slot);
        con_p->dev_if_p->parm3 = (uint8_t)0;
        con_p->dev_if_p->parm4 = (uint8_t)get_daughter_card_i2c_ctrl(slot);
        con_p->dev_if_p->interface = SCC_I2C_IF;
        if (slot == POE_CARD) {
            sprintf((char *)smc_buf, "PoE");
        } else { /* RADI_CARD */
            sprintf((char *)smc_buf, "RAID");
        }
        break;
    default:
        cterr('f',0,"in plat_init_smart_eeprom_context: Not a supported "
              "Smart EEPROM type %d", type);
        assert(!"plat_init_eeprom: invalid argument");
        retval = FAILED;
    }

    return (retval);
}


/**************************************************************************
 *
 * Name: i2c_act2_reset
 *
 * Description: This function implementes a reset to Quack chip by
 *              reset the line for 50ms then unreset it
 *
 * Inputs: con - pointer to sc_context
 *
 * Outputs: None
 *
 *************************************************************************/
void i2c_act2_reset (sc_context *con_p)
{
    unsigned int reg, reset, slot; 
    if (con_p->type == MOTHER_BOARD) {
        printf("Resetting ACT2 Motherboard...");
        fflush(stdout);
        diag_fpga_reg_or(FPGA_EXT_RESET_REG, FPGA_ACT2_RESET);
        msleep(ACT2_RESET_UNRESET_DELAY);
        diag_fpga_reg_nand(FPGA_EXT_RESET_REG, FPGA_ACT2_RESET);
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
    } else if (con_p->type == ISP_CARD) {
        printf("Resetting RAID ACT2...");
        fflush(stdout);
        diag_fpga_reg_write(FPGA_PCIE_STS_CTL_REG, NGIO_PRSNT);
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
    } else if (con_p->type == WIC_MODULE)  {
        slot = con_p->slot;
        switch (slot) {
        case NIM1: 
            reg = FPGA_NIM1_STSCTL_REG; 
            reset = NGIO_I2C_RESET; 
        break; 
        case NIM2: 
            reg = FPGA_NIM2_STSCTL_REG; 
            reset = NGIO_I2C_RESET; 
        break; 
        case NIM3: 
            reg = FPGA_NIM3_STSCTL_REG; 
            reset = NGIO_I2C_RESET; 
        break; 
        }    
        printf("Resetting ACT2 NIM%d...", slot);
        fflush(stdout);
        diag_fpga_reg_or(reg, reset); 
        msleep(ACT2_RESET_UNRESET_DELAY);
        diag_fpga_reg_nand(reg, reset); 
        msleep(ACT2_UNRESET_DELAY);
        printf("Done\n");
        fflush(stdout);
    }
    return;
}


/**************************************************************************
 *
 * Name: i2c_act2_read_bytes
 *
 * Description: Read bytes from the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         read_buffer - buffer to hold the data
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int i2c_act2_read_bytes (sc_context *con_p, char *rx_buffer)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int ix;

    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;
    
    n2g_i2c_if_p->size = 4; /* default of read 2 byte routine */
    n2g_i2c_if_p->buf = rx_buffer;
    n2g_i2c_if_p->offset = -1;

    for (ix = 0; ix < QUACK_RETRY; ix++) {
        if ((ret_status = n2g_i2c_read (n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
}

/**************************************************************************
 *
 * Name: i2c_act2_write_bytes
 *
 * Description: Write bytes to the I2C interface
 *
 * Inputs: con_p   - Pointer to sc_context
 *         tx_buffer - pointer to the command to be sent
 *         tx_size - size of the command
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int i2c_act2_write_bytes (sc_context *con_p, char *tx_buffer, int tx_size)
{
    uint32_t ret_status, error_flag = PASSED;
    n2g_i2c_if_t *n2g_i2c_if_p, n2g_i2c_if_ds;
    int ix;

    memset(&n2g_i2c_if_ds, 0, sizeof(n2g_i2c_if_t));
    n2g_i2c_if_p = &n2g_i2c_if_ds;
    n2g_i2c_if_p->i2c_bus_type = con_p->dev_if_p->parm1;
    n2g_i2c_if_p->i2c_dev = con_p->dev_if_p->parm2;
    n2g_i2c_if_p->mux = con_p->dev_if_p->parm3;
    n2g_i2c_if_p->i2c_ctrl = con_p->dev_if_p->parm4;
    
    n2g_i2c_if_p->size = tx_size;
    n2g_i2c_if_p->buf = tx_buffer;
    n2g_i2c_if_p->offset = -1;

    for (ix = 0; ix < QUACK_RETRY; ix++) {
        if ((ret_status = n2g_i2c_write(n2g_i2c_if_p)) != RC_I2C_OP_OK) {
            sprintf(i2c_err,  i2c_err_str(ret_status));
            error_flag = FAILED;
            sleep(1);
        } else {
            return (PASSED);
        }
    }

    /* still need close it after this point */
    return (error_flag);
}

void clean_smart_eeprom_context (sc_context *con_p)
{
	printf("To be developed...\n");
}

/*---------------------------------------------------------------
$Log: platform_cookie.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.14  2016/04/18 07:00:47  benchen2
according to prrq fix isp define

Revision 1.1.2.13  2016/01/18 12:07:27  alpeng
act2 reset before access it for nim, following MB act2

Revision 1.1.2.12  2016/01/18 07:02:28  alpeng
update cookie info for read mac

Revision 1.1.2.11  2015/11/13 08:05:42  benchen2
modify raid card act2

Revision 1.1.2.10  2015/11/02 10:22:56  tirawan
Add PoE Cookie Utility

Revision 1.1.2.9  2015/10/28 07:55:04  benchen2
add raid act2 cookies utility

Revision 1.1.2.8  2015/09/30 09:15:30  benchen2
add poe act2

Revision 1.1.2.7  2015/09/18 06:58:54  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.6  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.5  2015/08/31 06:42:08  tirawan
Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip

Revision 1.1.2.4  2015/08/30 05:57:35  tirawan
To support NIM ACT2 R/W access using TAM library

Revision 1.1.2.3  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.2  2015/08/11 07:44:28  meho
Added f35 nim tests.

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project


$Endlog$
*/

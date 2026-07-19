/* $Id: ngio.c,v 1.3 2020/01/09 01:02:37 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/ngio.c,v $
 *------------------------------------------------------------------
 * ngio.c  check EDCS 1108257 section 2.7.5 for NGIO init sequence
 *
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 * 
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "proto.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "common.h"
#include "slot.h"
#include "ngio.h"
#include "pca.h"
#include "nvmonvars.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "linux_pciutils.h"
#include "diag_fpga_lib.h"

/* Extern fucntions */
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);

/* static functions */
static int ngio_i2c_unreset(int);


/*-------------------------------------------------------------------
 *
 * Function : ngio_i2c_unreset
 * Description: unreset i2c of a module
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
static int ngio_i2c_unreset (int reg)
{
    int ia, buf; 

    diag_fpga_reg_read(reg, &buf);
    buf &= ~NGIO_I2C_RESET;
    diag_fpga_reg_write(reg, buf);

    for (ia = 0; ia < 60; ia++) {
        diag_fpga_reg_read(reg, &buf);
        if (buf & NGIO_PWR_OK) {
            break;
        }
        msleep(10);
    }

    diag_fpga_reg_read(reg, &buf);
    if (buf & NGIO_PWR_OK) {
        msleep(I2C_UNRESET_DELAY);
        return 0;
    }

    printf("ngio status register reports module power is not up.");
    printf(" NGIO_PWR_OK bit16 is not set.\n");
    fflush(stdout);
    return -1;

}

/*-------------------------------------------------------------------
 *
 * Function : ngio_enable
 * Description: power on  modual
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED if power on successfully 
 * -------------------------------------------------------------------
*/
static int
ngio_enable (int ctrl_reg, int intr_reg)
{
    int buf; 

    /* make sure ctrl reg is back in its default state before
       taking powering module */
    diag_fpga_reg_read(ctrl_reg, &buf);
    buf &= ~(NGIO_UART_TX) | (NGIO_I2C_RESET | NGIO_RESET); 
    diag_fpga_reg_write(ctrl_reg, buf);

    diag_fpga_reg_read(ctrl_reg, &buf);
    buf |= NGIO_PWR_EN;
    diag_fpga_reg_write(ctrl_reg, buf);
    msleep(30);

    diag_fpga_reg_read(ctrl_reg, &buf);
    if (buf & NGIO_PWR_EN) {
        /* rails need to be powered up within 500m of NGIO_PWR_EN
           being set to 1
         */
        diag_fpga_reg_read(intr_reg, &buf);
        buf |= NGIO_FLT_INTR;
        diag_fpga_reg_write(intr_reg, buf);
        msleep(ENABLE_DELAY);

        /* power fault intr enable, insert/removal intr already on */
        return 0;
    }

    printf("ngio status register reports module power is not up.");
    printf("NGIO_PWR_EN bit4 is not set.\n");
    
    return(-1);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_present 
 * Description: check if WIC is present
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: TRUE if wic is present; FALSE otherwise
 * -------------------------------------------------------------------
*/
int
ngiowic_present (void *p)
{
    int slot, buf, reg;
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    assert(intf->slot);
    
    slot = intf->slot; 
    switch (slot) {
    case NIM1 :
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break; 
    }
 
    diag_fpga_reg_read(reg, &buf);
    return (buf & NGIO_PRSNT);
}

/*-------------------------------------------------------------------
 *
 * Function : is_ngiowic_i2c_unreset
 * Description: check ngiowic i2c reset status.
 * INPUT:  p -- pointer to struct ngio_intf_t
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
is_ngiowic_i2c_unreset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, buf, reg;
    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1 :
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    if (buf & NGIO_I2C_RESET) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_reset
 * Description: reset wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_reset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, buf, reg;
    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1 :
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf |= NGIO_RESET; 
    diag_fpga_reg_write(reg, buf);

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_unreset
 * Description: unreset wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_unreset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, buf, reg;
    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1 :
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf  &= ~(NGIO_RESET | NGIO_PCI_RDY);
    diag_fpga_reg_write(reg, buf);

    diag_fpga_reg_read(reg, &buf);
    buf  |= NGIO_SRC_SEL;
    diag_fpga_reg_write(reg, buf);

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_i2c_reset
 * Description: reset i2c on wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_i2c_reset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg, buf;

    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1:
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf |= NGIO_I2C_RESET;
    diag_fpga_reg_write(reg, buf);

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_i2c_unrset
 * Description: unreset i2c on wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_i2c_unreset (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg;

    assert(intf->slot);    

    slot = intf->slot; 
    switch (slot) {
    case NIM1: 
        reg = FPGA_NIM1_STSCTL_REG; 
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default: 
        printf("Not support slot%d\n", slot);
    break;
    }
    
    return (ngio_i2c_unreset(reg));
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic enable
 * Description: power up wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_enable (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, ctrl_reg, intr_reg; 
    assert(intf->slot);    

    slot = intf->slot; 
    switch(slot) {
    case NIM1:
        ctrl_reg = FPGA_NIM1_STSCTL_REG;
        intr_reg = FPGA_NIM1_INTEN_REG; 
    break;
    case NIM2:
        ctrl_reg = FPGA_NIM2_STSCTL_REG;
	intr_reg = FPGA_NIM2_INTEN_REG; 
    break;
    case NIM3:
        ctrl_reg = FPGA_NIM3_STSCTL_REG;
	intr_reg = FPGA_NIM3_INTEN_REG; 
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    return (ngio_enable(ctrl_reg, intr_reg));
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_disable
 * Description: power down wic
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiowic_disable (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, ctrl_reg, intr_reg, buf;
    assert(intf->slot);

    slot = intf->slot;
    switch(slot) {
    case NIM1:
        ctrl_reg = FPGA_NIM1_STSCTL_REG;
        intr_reg = FPGA_NIM1_INTEN_REG;
    break;
    case NIM2:
        ctrl_reg = FPGA_NIM2_STSCTL_REG;
	intr_reg = FPGA_NIM2_INTEN_REG; 
    break;
    case NIM3:
        ctrl_reg = FPGA_NIM3_STSCTL_REG;
	intr_reg = FPGA_NIM3_INTEN_REG; 
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    ngiowic_reset(p);
    ngiowic_i2c_reset(p);
    
    diag_fpga_reg_read(ctrl_reg, &buf);
    buf &= ~(NGIO_PWR_EN); 
    diag_fpga_reg_write(ctrl_reg, buf);

    diag_fpga_reg_read(ctrl_reg, &buf);
    buf &= ~(NGIO_PCI_RDY | NGIO_UART_TX);
    diag_fpga_reg_write(ctrl_reg, buf);

    msleep(ENABLE_DELAY);

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_enable_intr
 * Description: enable wic intr
 * INPUT:  dev -- slot number starting with 1; intr_type -- type of intr
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiowic_enable_intr (int dev, int intr_type)
     
{
    int slot, reg, buf;

    slot = dev; 
    /* interrupt start from slot 1 */
    switch (dev) {
    case NIM1:
        reg = FPGA_NIM1_INTEN_REG; 
    break;
    case NIM2:
        reg = FPGA_NIM2_INTEN_REG; 
    break;
    case NIM3:
        reg = FPGA_NIM3_INTEN_REG; 
    break;    
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf |= intr_type; 
    diag_fpga_reg_write(reg, buf);

    return; 
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_disable intr
 * Description: disable wic intr
 * INPUT:  dev -- slot number starting with 1; intr_type -- type of intr
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiowic_disable_intr (int dev, int intr_type)
{
    int slot, reg, buf;

    slot = dev;
    /* interrupt start from slot 1 */
    switch (dev) {
    case NIM1:
        reg = FPGA_NIM1_INTEN_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_INTEN_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_INTEN_REG;
    break;    
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf &= ~intr_type;
    diag_fpga_reg_write(reg, buf);

    return;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_enable_uart
 * Description: enable wic uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_enable_uart (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg, buf;

    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1:
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf |= NGIO_UART_TX;
    diag_fpga_reg_write(reg, buf);

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiowic_disable_uart
 * Description: disale wic uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiowic_disable_uart (void *p)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg, buf;

    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1:
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf &= ~NGIO_UART_TX;
    diag_fpga_reg_write(reg, buf);

    return (OK);

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_present
 * Description: check if daughter  is present
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_present (void *intf)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)intf;
    struct ngio_intf_t *parent;
    unsigned char data;

    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;
    assert(parent->pca);
    
    if (io_port_8bit_i2c_read(parent->pca, 0, &data, 0)) {
        return ERROR;
    }

    if (data & 0x1) {
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_reset
 * Description: reset daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_reset (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;

    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;

    if (io_port_8bit_i2c_read(parent->pca, CONFIGURATION_REG, &data, 0)) {
        printf("unable to read from config reg of daughter card expander\n");
        return ERROR;
    }

    /* configure as output pin */
    data &= ~DB_RESET_L;
    if (io_port_8bit_i2c_write(parent->pca, CONFIGURATION_REG, &data)) {
        printf("unable to write to config reg of daughter card expander\n");
        return ERROR;
    }

    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("unable to read from output port reg of daughter card expnder\n");
        return ERROR;
    }
    
    data &= ~(DB_RESET_L); 

    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("unable to write to output port reg of daughter card expnder\n");
        return ERROR;
    }
    return PASSED;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_unreset
 * Description: unreset daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_unreset (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;

    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);

    parent = dc->pc;

    if (io_port_8bit_i2c_read(parent->pca, CONFIGURATION_REG, &data, 0)) {
        printf("fail reading from config reg of daughter card expander\n");
        return ERROR;
    }

    /* configure as output pin */
    data &= ~DB_RESET_L;
    if (io_port_8bit_i2c_write(parent->pca, CONFIGURATION_REG, &data)) {
        printf("fail writing to config reg of daughter card expander\n");
        return ERROR;
    }
    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("fail reading from output port reg of daughter card expander\n");
        return ERROR;
    }

    data |= DB_RESET_L;

    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("fail writing to output port reg of daughter card expander\n");
        return ERROR;
    }
    return PASSED;

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_i2c_rset
 * Description: do nothing. can't reset i2c on daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_i2c_reset (void *p)
{
    assert(!"not supported");
    return ERROR;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_i2c_unrset
 * Description: do nothing. can't unreset i2c on daughter card
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_i2c_unreset (void *p)
{
    printf("FIX ME: %s %d\n", __FILE__, __LINE__);
    assert(!"not supported");
    return ERROR;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_enable
 * Description: do nothing. daughter card is alwasy on
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_enable (void *p)
{
    assert(!"not supported");
    return ERROR;
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_disable
 * Description: do nothing. daughter card is alwasy on
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiodc_disable (void *p)
{
    ngiodc_reset(p);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_enable_intr
 * Description: do nothing
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiodc_enable_intr (int dev, int intr_type)
{
    assert(!"not supported");

}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_disable_intr
 * Description: disable daughter card intr
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
void
ngiodc_disable_intr (int dev, int intr_type)
{
    assert(!"not supported");
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodc_enalbe_uart
 * Description: enale duaghter card uart
 * INPUT:  dev -- slot number starting with 1
 *         intr type -- type of interupt
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_enable_uart (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;
    
    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;
    assert(parent->pca);
    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("fail to read from output port reg of daughter card expander\n");
        return ERROR;
    }
    data &= ~0x10;
    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("fail to write to outputport reg of daughter card expander\n");
        return ERROR;
    }

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : ngiodisable_uart
 * Description: dsiable daughet card uart
 * INPUT:  p -- pointer to struct ngio_intf_t 
 * OUTPUT: PASSED
 * -------------------------------------------------------------------
*/
int
ngiodc_disable_uart (void *p)
{
    struct ngio_intf_t *dc = (struct ngio_intf_t *)p;
    struct ngio_intf_t *parent;
    unsigned char data;
    
    assert(dc);
    assert(dc->pc);
    assert(dc->pc->pca);
    
    parent = dc->pc;
    assert(parent->pca);

    if (io_port_8bit_i2c_read(parent->pca, OUTPUT_PORT_REG, &data, 0)) {
        printf("fail to read from outputport reg of daughter card expander\n");
        return ERROR;
    }
    data |= 0x10;
    if (io_port_8bit_i2c_write(parent->pca, OUTPUT_PORT_REG, &data)) {
        printf("fail to write to outputport reg of daughter card expander\n");
        return ERROR;
    }

    return (OK);
}

/*-------------------------------------------------------------------
 *
 * Function : clr_oir_intr
 * Description: clear OIR inter
 * INPUT:  io -- pointer to struct ng_t ;
 *         buf -- pointer to message
 *         type -- type of module
 *         slot_num -- slot number
 * OUTPUT: NONE
 *         
 * -------------------------------------------------------------------
*/
static void
clr_oir_intr (char *buf, int type, int slot_num)
{
    *buf = '\0';
    char str[32];
    int databuf, reg; 
    
    switch (type) {
    case SM_MODULE: 
        sprintf(str, "SM%d", slot_num);
        break;
    case WIC_MODULE:
        sprintf(str, "WIC%d", slot_num);
        switch (slot_num) {
        case NIM1 :
            reg = FPGA_NIM1_STSCTL_REG;
        break;
	case NIM2:
	    reg = FPGA_NIM2_STSCTL_REG;
	    break;
	case NIM3:
	    reg = FPGA_NIM3_STSCTL_REG;
	    break;
        default:
            printf("Not support slot%d\n", slot_num);
        break;
        }
    break;
    default:
        *str = '\0';
    break;
    }
        
    diag_fpga_reg_read(reg, &databuf);
    if (databuf & NGIO_FLT_INTR) {
        databuf |= NGIO_FLT_INTR;
        diag_fpga_reg_write(reg, databuf);
        sprintf(buf, "pwr fault ");

        diag_fpga_reg_read(reg, &databuf);
        printf("***power fault %s; ngio status @%#x=%#x***\n", str,
               reg, databuf);
    }
    
    diag_fpga_reg_read(reg, &databuf);
    if (databuf & NGIO_RMV_INTR) {
        databuf |= NGIO_RMV_INTR;
        diag_fpga_reg_write(reg, databuf);
        slot_clear_cookie_id(type, slot_num);
        sprintf(buf, "removal");
    }
    
    diag_fpga_reg_read(reg, &databuf);
    if (databuf & NGIO_INS_INTR) {
        databuf |= NGIO_INS_INTR;
        slot_clear_cookie_id(type, slot_num);
        sprintf(buf, "insertion");
    }
    
    return;
}

/*-------------------------------------------------------------------
 *
 * Function : oir_wic1_intr_hndlr
 * Description: intr hndlr for wic1
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
oir_wic1_intr_hndlr (int irq, void *p)
{
    char buf[48];

    clr_oir_intr(buf, WIC_MODULE, 1);
    if (*buf && ((NVRAM)->diagflag & D_VERBOSE)) {
        printf("\n\n****NGIOWIC1 %s OIR\n\n", buf);
    }

    return;
}
/*-------------------------------------------------------------------
 *
 * Function : clr_all_oir_intr
 * Description: clear all possible OIR intr
 * INPUT:  irq - irq number; p -- pointer to struct ngio_intf_t 
 * OUTPUT: NONE
 * -------------------------------------------------------------------
*/
void
clr_all_oir_intr (void)
{
    char buf[48];
    clr_oir_intr(buf, WIC_MODULE, 1);
    return;
}

void
ngiowic_pci_rdy (void *p, int on)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg, buf;

    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1:
        reg = FPGA_NIM1_STSCTL_REG;
    break;
    case NIM2:
        reg = FPGA_NIM2_STSCTL_REG;
    break;
    case NIM3:
        reg = FPGA_NIM3_STSCTL_REG;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);

    
    if (on) {
        buf |= NGIO_SRC_SEL;
        buf |= NGIO_PCI_RDY;
    } else {
        buf &= ~NGIO_PCI_RDY;
    }
    diag_fpga_reg_write(reg, buf);
    return;
}

int
ngio_sync_out_enable (void *p, int mask)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg, buf;

    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1:
        reg = FPGA_NIM1_STNC_TRIG_CTRL;
    break;
    case NIM2:
        reg = FPGA_NIM2_STNC_TRIG_CTRL;
    break;
    case NIM3:
        reg = FPGA_NIM3_STNC_TRIG_CTRL;
    break;
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf |= mask;
    diag_fpga_reg_write(reg, buf);
    
    return(PASSED);
}

int
ngio_sync_out_disable (void *p, int mask)
{
    struct ngio_intf_t *intf = (struct ngio_intf_t *)p;
    int slot, reg, buf;

    assert(intf->slot);

    slot = intf->slot;
    switch (slot) {
    case NIM1:
        reg = FPGA_NIM1_STNC_TRIG_CTRL;
    break;
    case NIM2:
        reg = FPGA_NIM2_STNC_TRIG_CTRL;
    break;
    case NIM3:
        reg = FPGA_NIM3_STNC_TRIG_CTRL;
    break;    
    default:
        printf("Not support slot%d\n", slot);
    break;
    }

    diag_fpga_reg_read(reg, &buf);
    buf &= ~mask;
    diag_fpga_reg_write(reg, buf);

    return(PASSED);
}
int ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                            const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed)
    __attribute__((weak, alias("__ngio_cfg_eth_port_speed")));
int __ngio_cfg_eth_port_speed(uint mod_type, uint slot,
                              const ngio_eth_speed_t *new_speed, ngio_eth_speed_t *old_speed)
{
    return (FALSE);
}

/******** History ******** 
$Log: ngio.c,v $
Revision 1.3  2020/01/09 01:02:37  jiajliu
Merge Curie 2RU to main trunk

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.2  2015/09/26 08:00:41  alpeng
add pci rdy during init ngio

Revision 1.1.2.1  2015/07/24 06:59:58  alpeng
Add ngio.c to support NIM test

$Endlog$
*/
